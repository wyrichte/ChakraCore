#pragma once

#ifdef JD_PRIVATE
#include "Collections.h"
#include "RemoteRecycler.h"

struct PinnedObjectEntry
{
    ULONG64 address;
    int pinnedCount;
};

template <typename TPointerType>
class PinnedObjectMap
{
public:
    PinnedObjectMap(RemoteRecycler recycler, bool pinRecordWithStacks):
        _pinRecordsWithStacks(pinRecordWithStacks)
    {
        ExtRemoteTyped transientPinnedObject = recycler.GetExtRemoteTyped().Field("transientPinnedObject");
        ExtRemoteTyped pinnedObjectMap = recycler.GetExtRemoteTyped().Field("pinnedObjectMap");
        _hasPendingUnpinnedObject = recycler.GetExtRemoteTyped().Field("hasPendingUnpinnedObject").GetStdBool();
        _transientPinnedObject = transientPinnedObject.GetPtr();
        _pinnedObjectEntries = pinnedObjectMap.Field("table").GetPtr();
        _pinnedObjectTableSize = pinnedObjectMap.Field("size").GetUlong();
    }

    template <class Fn>
    void Map(Fn fn);

private:
    bool _hasPendingUnpinnedObject;
    bool _pinRecordsWithStacks;
    int _currentIndex;
    ULONG64 _transientPinnedObject;
    ULONG64 _pinnedObjectEntries;
    ULONG _pinnedObjectTableSize;
};

enum RootType : unsigned char
{
    RootTypeNone            = 0x00,
    RootTypePinned          = 0x01,
    RootTypeStack           = 0x02,
    RootTypeRegister        = 0x04,
    RootTypeArena           = 0x08,
    RootTypeImplicit        = 0x10,

    RootTypeTransient       = RootTypeStack | RootTypeRegister
};

class RootTypeUtils
{
public:
    static inline bool IsAnyRootType(RootType type)
    {
        return static_cast<uint>(type) != 0;
    }

    static inline bool IsType(RootType type, RootType target)
    {
        RootType result = static_cast<RootType>(
            static_cast<uint>(type) &
            static_cast<uint>(target));
        return result == target;
    }

    static inline bool IsOnlyType(RootType type, RootType target)
    {
        uint result =
            static_cast<uint>(type) &
            ~static_cast<uint>(target);
        return result != 0;
    }

    static inline RootType CombineTypes(RootType a, RootType b)
    {
        RootType type = static_cast<RootType>(
            static_cast<uint>(a) |
            static_cast<uint>(b));
        return type;
    }

    static inline bool IsNonTransientRootType(RootType type)
    {
        return (static_cast<uint>(type) & ~RootTypeTransient) != 0;
    }
};

class RecyclerObjectGraph;
class Addresses
{
    friend class RootPointerReader;

protected:
    stdext::hash_map<ULONG64, RootType> _addresses;

public:
    template <typename Fn>
    void Map(const Fn& fn)
    {
        for (auto it = _addresses.begin(); it != _addresses.end(); ++it)
        {
            fn(it->first);
        }
    }

    void Insert(ULONG64 address, RootType rootType)
    {
        if (Contains(address))
        {
            auto found = _addresses.find(address);
            RootType existingRootType = found->second;

            rootType = RootTypeUtils::CombineTypes(rootType, existingRootType);
        }

        auto entry = std::pair<ULONG64, RootType>(address, rootType);
        _addresses.insert(entry);
    }

    RootType GetRootType(ULONG64 address)
    {
        if (Contains(address))
        {
            auto found = _addresses.find(address);
            RootType existingRootType = found->second;
            return existingRootType;
        }

        return RootType::RootTypeNone;
    }

    bool Contains(ULONG64 address)
    {
        return _addresses.find(address) != _addresses.end();
    }

    int Count()
    {
        return (int)_addresses.size();
    }
};

class RootPointerReader
{
public:
    typedef void(*RootAddedCallback)(ULONG64, void*);

    static void DefaultRootAddedCallback(ULONG64, void* context)
    {
    }

    RootPointerReader(RemoteRecycler recycler, RootAddedCallback callback = DefaultRootAddedCallback) :
        _recycler(recycler),
        m_addresses(new Addresses()),
        m_rootAddedCallback(callback)
    {
    }

    bool TryAdd(ULONG64 address, RootType rootType, void* context = nullptr)
    {
        if (address != NULL && _recycler.IsAlignedAddress(address))
        {
            RemoteHeapBlock * remoteHeapBlock = _recycler.GetHeapBlockMap().FindHeapBlock(address);

            // TODO: Validate it is a valid object pointer?
            if (remoteHeapBlock && m_addresses->_addresses.count(address) == 0)
            {
#if DBG
                if (g_Ext->m_PtrSize == 4)
                {
                    Assert(address < MAXULONG32);
                }
#endif
                Add(address, rootType);

                m_rootAddedCallback(address, context);

                return true;
            }
        }

        return false;
    }

    void Add(ULONG64 address, RootType rootType)
    {
        // Assert(_addresses.count(address) == 0);
        m_addresses->Insert(address, rootType);
    }

    Addresses * DetachAddresses()
    {
        return m_addresses.release();
    }

    void ScanRegisters(bool print = true);
    void ScanStack(RemoteRecycler& recycler, ULONG64 stackTop, bool print = true, bool showScriptContext = false);
    void ScanArenaData(ULONG64 arenaDataPtr);
    void ScanArena(ULONG64 arena, bool verbose);
    void ScanArenaMemoryBlocks(ExtRemoteTyped blocks);
    void ScanArenaBigBlocks(ExtRemoteTyped blocks);
    void ScanObject(ULONG64 object, ULONG64 bytes, RootType rootType);
    void ScanImplicitRoots(bool print = true);
private:

    RootAddedCallback m_rootAddedCallback;
    std::auto_ptr<Addresses> m_addresses;
    RemoteRecycler _recycler;
};

template <typename Fn>
void MapPinnedObjects(ExtRemoteTyped recycler, const Fn& callback);

// Use this template to get around the following nested type definition not being available in this file:
// - RecyclerObjectGraph::GraphImplNodeType
template <typename TNode>
void FormatPointerFlags(char *buffer, uint bufferLength, TNode *node);

ULONG64 GetStackTop();
#endif
