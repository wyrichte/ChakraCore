#pragma once

#ifdef JD_PRIVATE
#include "Collections.h"

struct PinnedObjectEntry
{
    ULONG64 address;
    int pinnedCount;
};

template <typename TPointerType>
class PinnedObjectMap
{
public:
    PinnedObjectMap(ExtRemoteTyped recycler, bool pinRecordWithStacks):
        _pinRecordsWithStacks(pinRecordWithStacks)
    {
        ExtRemoteTyped transientPinnedObject = recycler.Field("transientPinnedObject");
        ExtRemoteTyped pinnedObjectMap = recycler.Field("pinnedObjectMap");

        _transientPinnedObject = transientPinnedObject.GetPtr();
        _pinnedObjectEntries = pinnedObjectMap.Field("table").GetPtr();
        _pinnedObjectTableSize = pinnedObjectMap.Field("size").GetUlong();
    }

    template <class Fn>
    void Map(Fn fn);

private:

    bool _pinRecordsWithStacks;
    int _currentIndex;
    ULONG64 _transientPinnedObject;
    ULONG64 _pinnedObjectEntries;
    ULONG _pinnedObjectTableSize;
};


class RecyclerObjectGraph;
class Addresses
{
    friend class RootPointerReader;
protected:
    stdext::hash_set<ULONG64> _addresses;
public :
    template <typename Fn>
    void Map(const Fn& fn)
    {
        for (auto it = _addresses.begin(); it != _addresses.end(); it++)
        {
            fn(*it);
        }
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
    RootPointerReader(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler) :
        _heapBlockHelper(extension, recycler),
        _recycler(recycler),
        m_addresses(new Addresses())
    {
    }

    bool TryAdd(ULONG64 address)
    {
        if (address != 0 && _heapBlockHelper.IsAlignedAddress(address))
        {
            RemoteHeapBlock * remoteHeapBlock = _heapBlockHelper.FindHeapBlock(address, _recycler);

            // TODO: Validate it is a valid object pointer?
            if (remoteHeapBlock && m_addresses->_addresses.count(address) == 0)
            {
#if DBG
                if (g_Ext->m_PtrSize == 4)
                {
                    Assert(address < MAXULONG32);
                }
#endif
                Add(address);
                return true;
            }
        }

        return false;
    }

    void Add(ULONG64 address)
    {
        //Assert(_addresses.count(address) == 0);
        m_addresses->_addresses.insert(address);
    }

    Addresses * DetachAddresses()
    {
        return m_addresses.release();
    }

    void ScanRegisters(EXT_CLASS_BASE* ext, bool print = true);
    void ScanStack(EXT_CLASS_BASE* ext, ExtRemoteTyped& recycler, bool print = true);
    void ScanArenaData(ULONG64 arenaDataPtr);
    void ScanArena(ULONG64 arena, bool verbose);
    void ScanArenaMemoryBlocks(ExtRemoteTyped blocks);
    void ScanArenaBigBlocks(ExtRemoteTyped blocks);
    void ScanObject(ULONG64 object, size_t bytes);
    void ScanImplicitRoots(bool print = true);
private:
    std::auto_ptr<Addresses> m_addresses;
    ExtRemoteTyped _recycler;
    HeapBlockHelper _heapBlockHelper;
};

template <typename Fn>
void MapPinnedObjects(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, const Fn& callback);

#endif
