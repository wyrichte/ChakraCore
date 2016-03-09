//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE
#include <hash_set>
#include <vector>
#include <hash_map>
#include <stdio.h>
#include "RecyclerRoots.h"

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
template <typename T, class Traits = std::less<T>>
class Set
{
public:
    bool Add(const T& item)
    {
        auto i = items.insert(item);
        return i.second;
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

    bool Union(const Set& other)
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
    bool Map(Fn func) const
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
    void MapAll(Fn func) const
    {
        Map([func](const T& item)
        {
            func(item);
            return false;
        });
    }
private:
    std::set<T, Traits> items;
};

template <typename T, class Traits=stdext::hash_compare<T, std::less<T> > >
class HashSet
{
public:
    bool Add(const T& item)
    {
        auto i = items.insert(item);
        return i.second;
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

    bool Union(const HashSet& other)
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
    stdext::hash_set<T, Traits> items;
};

template <typename TKey, typename TValue>
class HashMap
{
public:
    typedef std::pair<TKey, TValue> EntryType;

    void Add(const TKey& key, const TValue& value)
    {
        _map.insert(std::make_pair(key, value));
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

// Singlely Link List
template <typename T>
class SingleLinkList
{
public:
    SingleLinkList() : head(nullptr), count(0) {};
    ~SingleLinkList()
    {
        Node * node = head;
        while (node != nullptr)
        {
            Node * nextNode = node->next;
            delete node;
            node = nextNode;
        }
    }

    void Add(const T& item)
    {        
        head = new Node(item, head);
        count++;
    }

    uint Count()
    {
        return count;
    }

    template <typename Fn>
    bool Map(Fn func)
    {
        for (Node * node = head; node != nullptr; node = node->next)
        {
            if (func(node->data))
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
    uint count;
    struct Node
    {
        Node(const T& data, Node * next) : data(data), next(next) {};        
        T data;
        Node * next;
    } * head;
};

template <typename T, typename CollectionT>
class PointerCollection
{
public:
    PointerCollection() : data(nullptr) {}
    ~PointerCollection()
    {
        if (IsMultiple())
        {
            delete GetCollection();
        }
    }
    void Add(const T& item)
    {
        if (this->data == nullptr)
        {
            this->data = item;
            return;
        }
        EnsureCollection()->Add(item);
    }

    uint Count()
    {
        if (!IsMultiple())
        {
            return this->data != nullptr;
        }
        return GetCollection()->Count();
    }

    template <typename Fn>
    bool Map(Fn func)
    {
        if (!IsMultiple())
        {
            if (this->data)
            {
                return func(this->data);
            }
            return false;
        }
        return GetCollection()->Map(func);
    }

    template <typename Fn>
    void MapAll(Fn func)
    {
        if (!IsMultiple())
        {
            if (this->data)
            {
                func(this->data);
            }
        }
        else
        {
            GetCollection()->MapAll(func);
        }
    }
private:
    CollectionT * EnsureCollection()
    {
        if (IsMultiple())
        {
            return GetCollection();
        }

        auto _collection = new CollectionT();
        _collection->Add(data);
        this->collection = ((uintptr_t)_collection) | 1;
        return _collection;
    }
    bool IsMultiple()
    {
        return (this->collection & 1) != 0;
    }
    CollectionT * GetCollection()
    {
        Assert(IsMultiple());
        return (CollectionT *)(this->collection & ~1);
    }

    union
    {
        T data;
        uintptr_t collection;
    };
};

template <typename T>
class Array
{
public:
    Array() : size(0), buffer(nullptr) {};
    ~Array()
    {
        if (buffer)
        {
            delete[] buffer;
        }
    }

    void Initialize(Set<T> const& items)
    {
        Assert(size == 0);
        Assert(buffer == nullptr);
        T * newBuffer = new T[items.Count()];
        uint count = 0;
        items.MapAll([newBuffer, &count](T const& item)
        {
            newBuffer[count++] = item;
        });
        buffer = newBuffer;
        size = count;
    }

    uint Count()
    {
        return size;
    }

    template <class Fn>
    bool Map(Fn fn)
    {
        for (uint i = 0; i < size; i++)
        {
            if (fn(buffer[i]))
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
    uint size;
    T * buffer;
};

template <typename TKey, typename TData>
class GraphNode : public TData
{
public:
    GraphNode(TKey key) : TData(key) {}
    
    void SetSuccessors(Set<GraphNode *> const& set)
    {
        successors.Initialize(set);
        set.MapAll([this](GraphNode * node)
        {
            node->predecessors.Add(this);
        });
    }

    uint GetSuccessorCount()
    {
        return successors.Count();
    }

    uint GetPredecessorCount() 
    {
        return predecessors.Count();
    }

    template <typename Fn>
    bool MapEdges(Fn func)
    {
        return successors.Map(func);
    }

    template <typename Fn>
    bool MapPredecessors(Fn func)
    {
        return predecessors.Map(func);
    }

    template <typename Fn>
    void MapAllEdges(Fn func)
    {
        successors.MapAll(func);
    }

    template <typename Fn>
    void MapAllPredecessors(Fn func)
    {
        predecessors.MapAll(func);
    }
    
private:
   
    PointerCollection<GraphNode *, SingleLinkList<GraphNode *>> predecessors;
    Array<GraphNode *> successors;
};

template <typename TKey, typename TData>
class Graph
{
public:
    typedef GraphNode<TKey, TData> NodeType;

    NodeType* GetNode(const TKey& key)
    {
        NodeType* node = FindNode(key);
        if (node == nullptr)
        {
            return AddNode(key);
        }

        return node;
    }
    
    NodeType* AddNode(const TKey& key)
    {
        Assert(FindNode(key) == nullptr);
        NodeType * node = new NodeType(key);
        if (!node)
        {
            g_Ext->ThrowOutOfMemory();
        }
        
        _nodes.Add(node);
        return node;
    }

    NodeType * FindNode(const TKey& key)
    {   
        NodeType node = NodeType(key);
        return _nodes.Get(&node);
    }

    void AddEdges(NodeType * nodeFrom, Set<NodeType *> const& successors)
    {
        nodeFrom->SetSuccessors(successors);
        edgeCount += (uint)successors.Count();
    }

    template <class Fn>
    bool MapNodes(Fn fn)
    {
        return _nodes.Map([&](NodeType* node)
        {
            return fn(node);
        });
    }

    template <class Fn>
    void MapAllNodes(Fn fn)
    {
        _nodes.MapAll([&](NodeType* node)
        {
            fn(node);
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

            this->MapAllNodes([&](NodeType* node)
            {
                node->MapAllEdges([&] (NodeType* toNode)
                {
                    ULONG64 fromPointer = node->Key();
                    ULONG64 toPointer = toNode->Key();
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
            this->MapAllNodes([&](NodeType* node)
            {
                node->MapAllEdges([&](NodeType* toNode)
                {
                    ULONG64 fromPointer = node->Key();
                    ULONG64 toPointer = toNode->Key();

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

    // TODO (doilij) refactor all of these export commands (they share a common structure)
    // Export to CSV
    // sourcePointer, destPointer
    void ExportToCsv(const char* filename)
    {
        FILE* f = fopen(filename, "w+");
        if (f != nullptr)
        {
            this->MapAllNodes([&](NodeType* node)
            {
                node->MapAllEdges([&](NodeType* toNode)
                {
                    ULONG64 fromPointer = node->Key();
                    ULONG64 toPointer = toNode->Key();

                    if (g_Ext->m_PtrSize == 8)
                    {
                        fprintf(f, "0x%016llX,0x%016llX\n", fromPointer, toPointer);
                    }
                    else
                    {
                        fprintf(f, "0x%08X,0x%08X\n", fromPointer, toPointer);
                    }
                });
            });

            fclose(f);
        }
    }

    // Export to CSV Extended
    void ExportToCsvExtended(EXT_CLASS_BASE *ext, const char* filename)
    {
        // display some info on the console about the data layout
        ext->Out("CSVX column info:\n");
        ext->Out("    sourcePointer, destPointer, sourceTypeId, destTypeId, sourceTypeName, destTypeName, sourceRootFlags, destRootFlags\n");

        FILE* f = fopen(filename, "w+");
        if (f != nullptr)
        {
            this->MapAllNodes([&](NodeType* node)
            {
                node->MapAllEdges([&](NodeType* toNode)
                {
                    ULONG64 fromPointer = node->Key();
                    ULONG64 toPointer = toNode->Key();

                    if (g_Ext->m_PtrSize == 8)
                    {
                        fprintf(f, "0x%016llX,0x%016llX", fromPointer, toPointer);
                    }
                    else
                    {
                        fprintf(f, "0x%08X,0x%08X", fromPointer, toPointer);
                    }

                    fprintf(f, ","); // comma following the first two fields

                    ULONG64 fromPointerVtable = GetPointerAtAddress(fromPointer);
                    ULONG64 toPointerVtable = GetPointerAtAddress(toPointer);

                    // TODO (doilij) implement fromAddrTypeId column
                    fprintf(f, ",");

                    // TODO (doilij) implement toAddrTypeId column
                    fprintf(f, ",");

                    fprintf(f, "\"%s\",", ext->GetTypeNameFromVTable(fromPointerVtable).c_str());
                    fprintf(f, "\"%s\",", ext->GetTypeNameFromVTable(toPointerVtable).c_str());

                    // print node flags information for node and toNode
                    const uint flagsBufferLength = 8; // space for 7 flags plus NULL
                    char flagsBuffer[flagsBufferLength];
                    FormatPointerFlags(flagsBuffer, flagsBufferLength, node);
                    fprintf(f, "\"%s\",", flagsBuffer);
                    FormatPointerFlags(flagsBuffer, flagsBufferLength, toNode);
                    fprintf(f, "\"%s\"\n", flagsBuffer);
                });
            });

            fclose(f);
        }
    }

    Graph() : edgeCount(0) {};
    ~Graph()
    {
        _nodes.MapAll([] (const NodeType* node)
        {
            delete node;
        });
    }

    size_t GetNodeCount()
    {
        return (uint)_nodes.Count();
    }

    uint GetEdgeCount()
    {
        return edgeCount;
    }

#if ENABLE_MARK_OBJ
    std::vector<TKey> FindPath(const TKey& from, const TKey& to);
#endif

private:
    class HashCompare
    {
    public:
        stdext::hash_compare<TKey> comp;
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        size_t operator()(const NodeType * _Key) const
        {
            return comp(_Key->Key());
        }
        bool operator( )(
            const NodeType * _Key1,
            const NodeType * _Key2
            ) const
        {
            return comp(_Key1->Key(), _Key2->Key());
        }
    };
    HashSet<NodeType*, HashCompare> _nodes;
    uint edgeCount;
};

#include "RecyclerObjectTypeInfo.h"

struct RecyclerGraphNodeData
{
#if DBG
    friend RecyclerObjectGraph;
#endif

    RecyclerGraphNodeData(ULONG64 address) :
        address(address),
      
        rootType(RootType::RootTypeNone)
    {
        Assert((address & 0xFF00000000000000) == 0);
        ClearTypeInfo();
    }

    ULONG64 Key() const { return address; }
    uint GetObjectSize() const { return objectSize; }
    void SetObjectSize(uint size) { objectSize = size; }

    bool HasTypeInfo() const { return GetTypeName() != nullptr; }
    void SetTypeInfo(char const * typeName, char const * typeNameOrField, bool hasVtable, bool isPropagated);
    void ClearTypeInfo() { typeInfo = nullptr; }
    
    bool HasVtable() const { return typeInfo ? typeInfo->HasVtable() : false; }
    bool IsPropagated() const { return typeInfo? typeInfo->IsPropagated() : false; }
    const char * GetTypeName() const { return typeInfo? typeInfo->GetTypeName() : nullptr; }
    const char * GetTypeNameOrField() const { return typeInfo? typeInfo->GetTypeNameOrField() : nullptr; }

    bool IsRoot() const { return RootTypeUtils::IsAnyRootType(rootType); }
    RootType GetRootType() const { return rootType; }
    void AddRootType(RootType rootType) { this->rootType = RootTypeUtils::CombineTypes(this->rootType, rootType); }

private:
    RootType rootType : 8;
    const ULONG64 address : 56;
    uint objectSize;
    RecyclerObjectTypeInfo * typeInfo;
};

#endif
