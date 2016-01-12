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

    RecyclerObjectGraph(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler, bool verbose = false);
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
    bool m_trident;
    bool m_hasTypeName;
    bool m_hasTypeNameAndFields;
};


#endif