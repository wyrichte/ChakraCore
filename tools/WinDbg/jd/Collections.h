//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE
#include <hash_set>
#include <vector>
#include <hash_map>
#include <stdio.h>

template <typename T>
struct DefaultValue
{
    static T Get();
};

template <typename T>
struct DefaultValue<T*>
{
    static T* Get()
    {
        return nullptr;
    }
};

// Wrappers for some STL classes to make it nicer
template <typename T>
class Set
{
public:
    void Add(const T& item)
    {
        items.insert(item);
    }

    bool Contains(const T& item)
    {
        auto it = items.find(item);

        return (it != items.end());
    }

    T Get(const T& item)
    {
        auto it = items.find(item);

        return (it != items.end() ? (*it) : T());
    }

    bool Union(const Set<T>& other)
    {
        bool changed = false;

        other.Map([&](const T& item)
        {
            if (!this->Contains(item))
            {
                items.insert(item);
                changed = true;
            }
        });

        return changed;
    }

    size_t Count() const
    {
        return items.size();
    }

    template <typename Fn>
    bool Map(Fn func)
    {
        for (auto it = items.begin(); it != items.end(); it++)
        {
            if (func(*it))
            {
                return true;
            }
        }
        return false;
    }

    template <typename Fn>
    void MapAll(Fn func)
    {
        Map([func](const T& item)
        {
            func(item);
            return false;
        });
    }

private:
    stdext::hash_set<T> items;
};

template <typename TKey, typename TValue>
class HashMap
{
public:
    typedef std::pair<TKey, TValue> EntryType;

    void Add(const TKey& key, const TValue& value)
    {
        EntryType entry;
        entry.first = key;
        entry.second = value;
        _map.insert(entry);
    }

    bool Contains(const TKey& key)
    {
        return (_map.find(key) != _map.end());
    }

    TValue Get(const TKey& key)
    {
        auto iter = _map.find(key);

        if (iter == _map.end())
        {
            return DefaultValue<TValue>::Get();
        }

        return (*iter).second;
    }

    const int Count()
    {
        return (int)_map.size();
    }

    template <typename Fn>
    bool Map(Fn func)
    {
        for (auto it = _map.begin(); it != _map.end(); it++)
        {
            EntryType entry = (*it);
            if (func(entry.first, entry.second))
            {
                return true;
            }
        }
        return false;
    }

    template <typename Fn>
    void MapAll(Fn func)
    {
        Map([func](const TKey& key, const TValue& value)
        {
            func(key, value);
            return false;
        });
    }
private:
    stdext::hash_map<TKey, TValue> _map;
};

template <typename TKey, typename TAux>
class GraphNode
{
public:
    typedef GraphNode<TKey, TAux> Node;

    TKey Key;

    template <typename Fn>
    bool MapEdges(Fn func)
    {
        return Edges.Map(func);
    }

    template <typename Fn>
    bool MapPredecessors(Fn func)
    {
        return Predecessors.Map(func);
    }

    template <typename Fn>
    void MapAllEdges(Fn func)
    {
        Edges.MapAll(func);
    }

    template <typename Fn>
    void MapAllPredecessors(Fn func)
    {
        Predecessors.MapAll(func);
    }

    Set<Node*> Edges;
    Set<Node*> Predecessors;
    TAux aux;
};

template <typename TKey, typename TAux>
class Graph
{
public:
    typedef GraphNode<TKey, TAux> NodeType;
    typedef std::pair<TKey, NodeType*> EntryType;

    NodeType* GetNode(const TKey& key)
    {
        NodeType* node = _nodes.Get(key);
        if (node == nullptr)
        {
            node = new NodeType;
            if (!node)
            {
                g_Ext->ThrowOutOfMemory();
            }

            node->Key = key;
            _nodes.Add(key, node);
        }

        return node;
    }
    
    NodeType * FindNode(const TKey& key)
    {
        return _nodes.Get(key);
    }

    void AddEdge(const TKey& from, const TKey& to)
    {
        NodeType* nodeFrom = GetNode(from);
        NodeType* nodeTo = GetNode(to);

        if (nodeFrom != nodeTo)
        {
            nodeFrom->Edges.Add(nodeTo);
            nodeTo->Predecessors.Add(nodeFrom);
        }
    }

    void AddEdge(const TKey& from, NodeType *nodeTo)
    {
        NodeType* nodeFrom = GetNode(from);

        if (nodeFrom != nodeTo)
        {
            nodeFrom->Edges.Add(nodeTo);
            nodeTo->Predecessors.Add(nodeFrom);
        }
    }

    template <class Fn>
    bool MapNodes(Fn fn)
    {
        return _nodes.Map([&](const TKey& key, NodeType* node)
        {
            return fn(key, node);
        });
    }

    template <class Fn>
    void MapAllNodes(Fn fn)
    {
        _nodes.MapAll([&](const TKey& key, NodeType* node)
        {
            fn(key, node);
        });
    }

    // Export to Python
    // Uses NetworkX library's format
    void ExportToPython(const char* filename)
    {
        FILE* f = fopen(filename, "w+");
        if (f != nullptr)
        {
            fprintf(f, "import networkx as nx\n");
            fprintf(f, "G = nx.DiGraph()\n");

            this->MapAllNodes([&](const TKey&, NodeType* node)
            {
                node->MapAllEdges([&] (NodeType* toNode)
                {
                    ULONG64 fromPointer = node->Key;
                    ULONG64 toPointer = toNode->Key;
                    if (g_Ext->m_PtrSize == 8)
                    {
                        fprintf(f, "G.add_edge('0x%016llX', '0x%016llX')\n", fromPointer, toPointer);
                    }
                    else
                    {
                        fprintf(f, "G.add_edge('0x%08X', '0x%08X')\n", fromPointer, toPointer);
                    }
                });
            });

            fclose(f);
        }
    }

    // Export to Js
    // Uses JsNetworkX library's format
    void ExportToJs(const char* filename)
    {
        FILE* f = fopen(filename, "w+");
        if (f != nullptr)
        {
            this->MapAllNodes([&](const TKey&, NodeType* node)
            {
                node->MapAllEdges([&](NodeType* toNode)
                {
                    ULONG64 fromPointer = node->Key;
                    ULONG64 toPointer = toNode->Key;

                    if (g_Ext->m_PtrSize == 8)
                    {
                        fprintf(f, "G.add_edge(\"0x%016llX\", \"0x%016llX\")\n", fromPointer, toPointer);
                    }
                    else
                    {
                        fprintf(f, "G.add_edge(\"0x%08X\", \"0x%08X\")\n", fromPointer, toPointer);
                    }
                });
            });

            fclose(f);
        }
    }

    ~Graph()
    {
        _nodes.MapAll([] (const TKey& key, const NodeType* node)
        {
            delete node;
        });
    }

    int Count()
    {
        return _nodes.Count();
    }

#if ENABLE_MARK_OBJ
    std::vector<TKey> FindPath(const TKey& from, const TKey& to);
#endif

private:
    HashMap<TKey, NodeType*> _nodes;
};

struct RecyclerGraphNodeAux
{
    RecyclerGraphNodeAux() 
    { 
        isScanned = false; 
        typeName = nullptr; 
        typeNameOrField = nullptr;
        hasVtable = false;
        isPropagated = false;
    }

    uint objectSize;    
    const char * typeName;
    const char * typeNameOrField;
    bool hasVtable;
    bool isPropagated;
    bool isScanned;
    bool isRoot;
};

#endif
