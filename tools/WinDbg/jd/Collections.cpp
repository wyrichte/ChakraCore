#include "stdafx.h"

#ifdef JD_PRIVATE
#include "Collections.h"
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

#if ENABLE_MARK_OBJ
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
            previous[node] = null;

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
    while (previous[node] != null)
    {
        result.push_back(node->Key);
        node = previous[node];
    }

    std::reverse(result.begin(), result.end());
    return result;
}

// Instantiate templates
template Graph<ULONG64, RecyclerGraphNodeAux>;
#endif

#endif