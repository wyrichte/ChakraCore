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

    static RecyclerObjectGraph * New(RemoteRecycler recycler, RemoteThreadContext * threadContext,
        ULONG64 stackTop,
        RecyclerObjectGraph::TypeInfoFlags typeInfoFlags = RecyclerObjectGraph::TypeInfoFlags::None);

    ~RecyclerObjectGraph();

    void DumpForPython(const char* filename);
    void DumpForJs(const char* filename);
    void DumpForCsv(const char* filename);
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
    RecyclerObjectGraph(RemoteRecycler recycler, bool verbose = false);
    void Construct(RemoteRecycler recycler, Addresses& roots);
    void EnsureTypeInfo(RemoteThreadContext * threadContext, RecyclerObjectGraph::TypeInfoFlags typeInfoFlags);    
    static ULONG64 InferJavascriptLibrary(RecyclerObjectGraph::GraphImplNodeType* node, JDRemoteTyped remoteTyped, char const * simpleTypeName);

    static ULONG64 TryMatchDynamicType(char const * type, RecyclerObjectGraph::GraphImplNodeType* pred, char const * field, RecyclerObjectGraph::GraphImplNodeType* node);
    static ULONG64 TryInferFunctionProxyJavascriptLibrary(JDRemoteTyped& remoteTyped);
    static ULONG64 TryInferVarJavascriptLibrary(JDRemoteTyped& remoteTyped);

    typedef std::pair<RemoteHeapBlock *, GraphImplNodeType *> MarkStackEntry;
    struct ConstructData
    {
        ConstructData(RemoteRecycler recycler) : recycler(recycler), hbm(recycler.GetHeapBlockMap()) {};
        RemoteRecycler recycler;
        RemoteHeapBlockMap hbm;
        std::stack<MarkStackEntry> markStack;
    };
    void ClearTypeInfo();
    void MarkObject(ConstructData& constructData, ULONG64 address, Set<GraphImplNodeType *> * successors, RootType rootType);
    void ScanBytes(ConstructData& constructData, RemoteHeapBlock * remoteHeapBlock, GraphImplNodeType * node);

    GraphImplType _objectGraph;
    bool _verbose;
    bool m_trident;
    bool m_hasTypeName;
    bool m_hasTypeNameAndFields;
    bool m_interior;
};

#endif
