#pragma once

#include <stack>
#include "jdrecycler.h"
#include "Collections.h"


// Represents the recycler object graph
// Encapsulates logic to scan remote recycler objects
class RecyclerLibraryGraph;
class RecyclerObjectGraph
{
public:
    enum TypeInfoFlags
    {
        None,
        Infer,
        Trident
    };
    typedef Graph<ULONG64, RecyclerGraphNodeData, ReservedPageAllocator> GraphImplType;
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

    uint GetDepth()
    {
        return this->m_maxDepth + 1;
    }

    template <typename Sort, typename FnInclude, typename FnSorted>
    bool MapSorted(FnInclude fnInclude, FnSorted fnSorted)
    {
        std::auto_ptr<GraphImplNodeType *> nodes(new GraphImplNodeType *[GetNodeCount()]);
        uint filteredCount = 0;
        MapAllNodes([&](GraphImplNodeType* node)
        {
            if (fnInclude(node))
            {
                nodes.get()[filteredCount++] = node;
            }
        });

        std::sort(nodes.get(), nodes.get() + filteredCount, Sort());

        for (uint i = 0; i < filteredCount; i++)
        {
            if (fnSorted(nodes.get()[i]))
            {
                return true;
            }
        }
        return false;
    }

    RecyclerLibraryGraph * GetLibraryGraph();
protected:
    RecyclerObjectGraph(RemoteRecycler recycler, bool verbose = false);
    void Construct(RemoteRecycler recycler, Addresses& roots);
    void EnsureTypeInfo(RemoteRecycler recycler, RemoteThreadContext * threadContext, RecyclerObjectGraph::TypeInfoFlags typeInfoFlags);    
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

        // Use a deque to do a breath first walk of the graph so we can calculate the depth of the a node
        std::deque<MarkStackEntry> markStack;
    };
    void ClearTypeInfo();
    void MarkObject(ConstructData& constructData, ULONG64 address, Set<GraphImplNodeType *> * successors, RootType rootType, uint depth);
    void ScanBytes(ConstructData& constructData, RemoteHeapBlock * remoteHeapBlock, GraphImplNodeType * node);

    RecyclerLibraryGraph * libraryGraph;
    GraphImplType _objectGraph;
    bool _verbose;
    bool m_trident;
    bool m_hasTypeName;
    bool m_hasTypeNameAndFields;
    bool m_interior;
    uint m_maxDepth;
};

