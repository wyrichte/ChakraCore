#include "stdafx.h"

#include "Collections.h"

void RecyclerGraphNodeData::SetTypeInfo(char const * typeName, char const * typeNameOrField, RecyclerObjectTypeInfo::Flags flags, ULONG64 javascriptLibrary)
{
    typeInfo = GetExtension()->recyclerObjectTypeInfoCache.GetRecyclerObjectTypeInfo(typeName, typeNameOrField, flags, javascriptLibrary);
}

#if ENABLE_MARK_OBJ
#include <queue>

#define Infinity 0x7fffffff

#ifdef GRAPH_DBG
#define GraphTrace(...) printf(__VA_ARGS__)
#else
#define GraphTrace(...)
#endif


template <typename TKey, typename TAux>
struct QueueElement
{
    GraphNode<TKey, TAux>* node;
    int distance;

    bool operator<(const QueueElement& other) const
    {
        return distance > other.distance;
    }
};


// Implements Dijkstra's algorithm to find shortest path
template <typename TKey, typename TAux>
std::vector<TKey> 
Graph<TKey, TAux>::FindPath(const TKey& from, const TKey& to)
{
    NodeType* fromNode = this->GetNode(from);
    NodeType* toNode = this->GetNode(to);

    std::vector<TKey> result;
    if (!fromNode || !toNode)
    {
        GraphTrace("Node not found");
        return result;
    }

    if (fromNode == toNode)
    {
        result.push_back(from);
        return result;
    }

    stdext::hash_map<NodeType*, int> distance;
    stdext::hash_map<NodeType*, NodeType*> previous;
    std::priority_queue<QueueElement<TKey, TAux>> queue;

    _nodes.Map([&] (const TKey& key, NodeType* node)
    {
        if (node != fromNode)
        {
            QueueElement<TKey, TAux> item;
            item.node = node;
            item.distance = Infinity;

            distance[node] = Infinity;
            previous[node] = nullptr;

            if (node->Edges.Count() == 0 &&
                node->Predecessors.Count() == 0)
            {
                g_Ext->Out("Unreachable node 0x%p- likely root with no references. Skipping.\n", node->Key);
            }
            else
            {
                queue.push(item);
            }
        }
    });

    distance[fromNode] = 0;
    QueueElement<TKey, TAux> item;
    item.node = fromNode;
    item.distance = 0;

    queue.push(item);

    int i = 0;
    while (!queue.empty())
    {
        i++;
        GraphTrace("Count: %d\n", queue.size());
        item = queue.top();
        queue.pop();

        GraphTrace("Item: %d\n", item.node->Key);
        GraphTrace("Distance: %d\n", item.distance);
        if (item.distance == Infinity)
        {
            g_Ext->Out("Unreachable node 0x%p reached. No path exists to target. I is %d, size is %d\n", item.node->Key, i, queue.size());

            auto top = queue.top();
            g_Ext->Out("Top distance is %d\n", top.distance);
            g_Ext->Out("Top node is %d\n", top.node->Key);
            return std::vector<TKey>();
        }

        item.node->Edges.Map([&] (GraphNode<TKey, TAux>* node)
        {
            QueueElement<TKey, TAux> neighbour;
            neighbour.node = node;
            neighbour.distance = distance[node];

            GraphTrace("Neighbour: %d, distance: %d\n", node->Key, neighbour.distance);
            GraphTrace("New distance: %d\n", item.distance + 1);

            if ((item.distance + 1) < neighbour.distance)
            {
                neighbour.distance = distance[node] = item.distance + 1;
                previous[node] = item.node;
                queue.push(neighbour);
            }
        });
    }

    auto node = toNode;
    while (previous[node] != nullptr)
    {
        result.push_back(node->Key);
        node = previous[node];
    }

    std::reverse(result.begin(), result.end());
    return result;
}

#endif

void SingleReservedSegment::Initialize(char * segmentStart, char const * segmentEnd, char * commitEnd)
{
    this->segmentStart = segmentStart;
    this->segmentEnd = segmentEnd;
    this->nextAlloc = segmentStart;
    this->commitEnd = commitEnd;
}

void * SingleReservedSegment::Alloc(size_t size)
{
    char * mem = this->nextAlloc;
    char * newNextAlloc = mem + size;
    if (newNextAlloc > commitEnd)
    {
        if (newNextAlloc > segmentEnd)
        {
            return nullptr;
        }
        size_t commitSize = JDUtil::Align<size_t>(newNextAlloc - commitEnd, 4096);
        if (commitEnd != ::VirtualAlloc(commitEnd, commitSize, MEM_COMMIT, PAGE_READWRITE))
        {
            g_Ext->ThrowOutOfMemory();
        }
        this->commitEnd += commitSize;
    }
    this->nextAlloc = newNextAlloc;
    return mem;
}

ReservedPageAllocator::ReservedPageAllocator()
{
    head = AllocSegment(0);
    largeHead = nullptr;
}

ReservedPageAllocator::~ReservedPageAllocator()
{
    ReservedSegment * curr = head;
    while (curr)
    {
        ReservedSegment * next = curr->next;
        ::VirtualFree(curr, 0, MEM_RELEASE);
        curr = next;
    }

    LargeAlloc * largeCurr = largeHead;
    while (largeCurr)
    {
        LargeAlloc * largeNext = largeCurr->next;
        delete largeCurr;
        largeCurr = largeNext;
    }
}

ReservedPageAllocator::ReservedSegment * ReservedPageAllocator::AllocSegment(size_t size)
{
    char * segment = (char *)::VirtualAlloc(0, segmentSize, MEM_RESERVE, PAGE_READWRITE);
    if (segment == nullptr)
    {
        g_Ext->ThrowOutOfMemory();
    }

    size_t commitSize = JDUtil::Align<size_t>(size + sizeof(ReservedSegment), 4096);
    ReservedSegment * newSegment = (ReservedSegment *)::VirtualAlloc(segment, commitSize, MEM_COMMIT, PAGE_READWRITE);
    if (newSegment == nullptr)
    {
        ::VirtualFree(segment, 0, MEM_RELEASE);
        g_Ext->ThrowOutOfMemory();
    }
    newSegment->Initialize((char *)(newSegment + 1), segment + segmentSize, segment + commitSize);
    return newSegment;
}

void * ReservedPageAllocator::Alloc(size_t size)
{
    if (size > segmentSize)
    {
        LargeAlloc * alloc = (LargeAlloc *)new byte[size + sizeof(LargeAlloc)];
        alloc->next = this->largeHead;
        this->largeHead = alloc;
        return alloc + 1;
    }
    void * mem = head->Alloc(size);
    if (mem == nullptr)
    {
        ReservedSegment * newSegment = AllocSegment(size);
        newSegment->next = head;
        head = newSegment;
        mem = head->Alloc(size);
    }  
    return mem;
}

void ReservedPageAllocator::Free(void * ptr)
{
    // Do nothing;
}
