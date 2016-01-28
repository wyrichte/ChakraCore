#pragma once

#include <stack>
#include "jdrecycler.h"
#include "Collections.h"
#ifdef JD_PRIVATE

// Represents the recycler object graph
// Encapsulates logic to scan remote recycler objects
class RecyclerObjectGraph
{
public:
    typedef Graph<ULONG64, RecyclerGraphNodeAux> GraphImplType;
    typedef GraphImplType::NodeType GraphImplNodeType;

    RecyclerObjectGraph(EXT_CLASS_BASE* extension, JDRemoteTyped recycler, bool verbose = false);
    ~RecyclerObjectGraph();
    void Construct(Addresses& roots);

    void DumpForPython(const char* filename);
    void DumpForJs(const char* filename);
#if ENABLE_MARK_OBJ
    void FindPathTo(RootPointers& roots, ULONG64 address, ULONG64 root);
#endif

    template <class Fn>
    bool MapNodes(Fn fn)
    {
        return _objectGraph.MapNodes(fn);
    }

    template <class Fn>
    void MapAllNodes(Fn fn)
    {
        _objectGraph.MapAllNodes(fn);
    }

    GraphImplNodeType * FindNode(ULONG64 address)
    {
        return _objectGraph.FindNode(address);
    }

    ULONG64 NumNodes()
    {
        return _objectGraph.Count();
    }

    void EnsureTypeInfo(bool infer, bool trident, bool verbose);

protected:
    void ClearTypeInfo();
    void MarkObject(ULONG64 address, ULONG64 prev);
    void ScanBytes(RemoteHeapBlock * remoteHeapBlock, HeapObjectInfo const& info);

    typedef std::pair<RemoteHeapBlock *, HeapObjectInfo> MarkStackEntry;

    std::stack<MarkStackEntry> _markStack;
    GraphImplType _objectGraph;

    RemoteHeapBlockMap m_hbm;
    HeapBlockHelper _heapBlockHelper;
    JDRemoteTyped _recycler;
    EXT_CLASS_BASE* _ext;
    bool _verbose;
    bool m_trident;
    bool m_hasTypeName;
    bool m_hasTypeNameAndFields;
    bool m_interior;
};


#endif