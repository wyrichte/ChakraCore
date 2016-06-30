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
    enum TypeInfoFlags
    {
        None,
        Infer,
        Trident
    };
    typedef Graph<ULONG64, RecyclerGraphNodeData> GraphImplType;
    typedef GraphImplType::NodeType GraphImplNodeType;

    static RecyclerObjectGraph * New(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext,
        RecyclerObjectGraph::TypeInfoFlags typeInfoFlags = RecyclerObjectGraph::TypeInfoFlags::None);

    ~RecyclerObjectGraph();

    void DumpForPython(const char* filename);
    void DumpForJs(const char* filename);
    void DumpForCsv(const char* filename);
    void DumpForCsvExtended(EXT_CLASS_BASE *ext, const char* filename);
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

    uint GetNodeCount()
    {
        return (uint)_objectGraph.GetNodeCount();
    }

    int GetEdgeCount()
    {
        return _objectGraph.GetEdgeCount();
    }


    template <typename Sort, typename Fn>
    bool MapSorted(Fn fn)
    {
        std::set<GraphImplNodeType*, Sort> sortedNodes;
        MapAllNodes([&](GraphImplNodeType* node)
        {
            sortedNodes.insert(node);
        });

        for (auto i = sortedNodes.begin(); i != sortedNodes.end(); i++)
        {
            if (fn((*i)))
            {
                return true;
            }
        }
        return false;
    }
protected:
    RecyclerObjectGraph(EXT_CLASS_BASE* extension, JDRemoteTyped recycler, bool verbose = false);
    void Construct(ExtRemoteTyped& heapBlockMap, Addresses& roots);
    void EnsureTypeInfo(RecyclerObjectGraph::TypeInfoFlags typeInfoFlags);

    void ClearTypeInfo();
    void MarkObject(ULONG64 address, Set<GraphImplNodeType *> * successors, RootType rootType);
    void ScanBytes(RemoteHeapBlock * remoteHeapBlock, GraphImplNodeType * node);

    typedef std::pair<RemoteHeapBlock *, GraphImplNodeType *> MarkStackEntry;

    std::stack<MarkStackEntry> _markStack;
    GraphImplType _objectGraph;

    RemoteHeapBlockMap m_hbm;
    EXT_CLASS_BASE* _ext;
    HeapBlockAlignmentUtility _alignmentUtility;
    bool _verbose;
    bool m_trident;
    bool m_hasTypeName;
    bool m_hasTypeNameAndFields;
    bool m_interior;
};

#endif
