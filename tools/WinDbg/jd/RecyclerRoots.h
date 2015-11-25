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

class RootPointers : public Addresses
{
public:
    friend class RecyclerObjectGraph;

    RootPointers(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler) :
        _heapBlockHelper(extension, recycler),
        _recycler(recycler)
    {
    }

    bool TryAdd(ULONG64 address)
    {
        if (address != 0 && _heapBlockHelper.IsAlignedAddress(address))
        {
            ULONG64 heapBlock = _heapBlockHelper.FindHeapBlock(address, _recycler, false);

            if (heapBlock != NULL && _addresses.count(address) == 0)
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
        _addresses.insert(address);
    }

private:
    ExtRemoteTyped _recycler;
    HeapBlockHelper _heapBlockHelper;
};

template <typename Fn>
void MapPinnedObjects(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, const Fn& callback);

// Represents the recycler object graph
// Encapsulates logic to scan remote recycler objects
class RecyclerObjectGraph
{
public:
    typedef Graph<ULONG64, RecyclerGraphNodeAux> GraphImplType;
    typedef GraphImplType::NodeType GraphImplNodeType;

    RecyclerObjectGraph(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler, bool verbose = false);
    ~RecyclerObjectGraph();
    void Construct(Addresses& roots);

    void DumpForPython(const char* filename);
    void DumpForJs(const char* filename);
#if ENABLE_MARK_OBJ
    void FindPathTo(RootPointers& roots, ULONG64 address, ULONG64 root);
#endif

    template <class Fn>
    void MapNodes(Fn fn)
    {
        _objectGraph.MapNodes(fn);
    }

protected:

    void MarkObject(ULONG64 address, ULONG64 prev);
    ULONG64 GetLargeObjectSize(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress);
    void ScanBytes(ULONG64 address, ULONG64 size);
    void PushMark(ULONG64 object, ULONG64 prev);

    typedef std::pair<ULONG64, ULONG64> MarkStackEntry;

    std::stack<MarkStackEntry> _markStack;
    GraphImplType _objectGraph;

    HeapBlockHelper _heapBlockHelper;
    ExtRemoteTyped _recycler;
    EXT_CLASS_BASE* _ext;
    bool _verbose;
};
#endif
