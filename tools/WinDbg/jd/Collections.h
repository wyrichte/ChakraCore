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

    int Count() const
    {
        return items.size();
    }

    template <typename Fn>
    void Map(Fn func)
    {
        for (auto it = items.begin(); it != items.end(); it++)
        {
            func(*it);
        }
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
    void Map(Fn func)
    {
        for (auto it = _map.begin(); it != _map.end(); it++)
        {
            EntryType entry = (*it);
            func(entry.first, entry.second);
        }
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
    void MapEdges(Fn func)
    {
        Edges.Map(func);
    }

    template <typename Fn>
    void MapPredecessors(Fn func)
    {
        Predecessors.Map(func);
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

    template <class Fn>
    void MapNodes(Fn fn)
    {
        _nodes.Map([&](TKey& key, NodeType* node)
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

            this->MapNodes([&](TKey&, NodeType* node)
            {
                node->MapEdges([&] (NodeType* toNode)
                {
                    g_Ext->Out("0x%P => 0x%P\n", node->Key, toNode->Key);
                    fprintf(f, "G.add_edge('0x%p'", node->Key);
                    fprintf(f, ", '0x%p')\n", toNode->Key);
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
            this->MapNodes([&](TKey&, NodeType* node)
            {
                node->MapEdges([&](NodeType* toNode)
                {
                    //g_Ext->Out("0x%P => 0x%P\n", node->Key, toNode->Key);
                    ULONG64 fromPointer = node->Key;
                    ULONG64 toPointer = toNode->Key;

                    fprintf(f, "G.add_edge(\"0x%p\", \"0x%p\")\n", fromPointer, toPointer);
                });
            });

            fclose(f);
        }
    }

    ~Graph()
    {
        _nodes.Map([] (const TKey& key, const NodeType* node)
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
    RecyclerGraphNodeAux() { isScanned = false; }

    uint objectSize;
    bool isScanned;
};

// Simple sorted map class
// Insertion is O(n) so use only with a small buffer size
// The advantage here is that the underlying datastructure is always stored sorted
// so sorted traversal is also O(n)
template <typename TKey, typename TValue, int BufferSize>
class SortedBuffer
{
    struct KVP
    {
        TKey key;
        TValue value;
    };
public:
    SortedBuffer() :
        _min(TValue()),
        _tail(-1)
    {
        for (int i = 0; i < BufferSize; i++) {
            _buffer[i].key = TKey();
            _buffer[i].value = TValue();
        }
    }

    void Add(TKey key, TValue value)
    {
        // If the key already exists, remove it
        for (int i = 0; i <= _tail; i++)
        {
#if DBG_SB
            g_Ext->Out("Checking whether %d == %d\n", _buffer[i].key, key);
#endif
            // If the key's already in here, remove it first
            if (_buffer[i].key == key) {
                for (int j = i; j < _tail; j++) {
#if DBG_SB
                    g_Ext->Out("Moving %d to %d\n", j + 1, j);
#endif

                    _buffer[j] = _buffer[j + 1];
                }
                if (_tail >= 0) { _tail--; }
                break;
            }
        }

        // Check if new value is greater than whats in the buffer so far
        if (_tail + 1 < BufferSize || value > _min)
        {
            int i = 0;
            // Find a spot to insert the new item
            while (_buffer[i].value > value)
            {
                i++;
            }

            // See if we have hit our size limit
            if (_tail + 1 < BufferSize) _tail++;

            // Start from the end, move every item to its next slot
            int j = _tail;
            while (j > i)
            {
                _buffer[j] = _buffer[j - 1];
                j--;
            }

            // Insert the new item into its chosen slot
            _buffer[i].key = key;
            _buffer[i].value = value;

            // Update the cached smallest value
            _min = _buffer[_tail].value;
        }
    }

    template <class Fn>
    void Map(Fn fn)
    {
        for (int i = 0; i <= _tail; i++) {
            fn(_buffer[i].key, _buffer[i].value);
        }
    }

    int Count() { return _tail + 1; }

private:
    TValue _min;
    int _tail;
    KVP _buffer[BufferSize];
};

#endif
