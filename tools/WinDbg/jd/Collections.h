//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

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
template <typename T, typename TAllocator>
class SingleLinkList
{
public:
    SingleLinkList() : head(nullptr), count(0) {};
    ~SingleLinkList()
    {
        if (TAllocator::NeedFree)
        {
            Node * node = head;
            while (node != nullptr)
            {
                Node * nextNode = node->next;
                delete node;
                node = nextNode;
            }
        }
    }

    void Add(const T& item, TAllocator& allocator)
    {        
        head = new (allocator.Alloc(sizeof(Node))) Node(item, head);
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

template <typename T, typename CollectionT, typename TAllocator>
class PointerCollection
{
public:
    PointerCollection() : data(nullptr) {}
    ~PointerCollection()
    {
        if (TAllocator::NeedFree)
        {
            if (IsMultiple())
            {
                delete GetCollection();
            }
        }
    }
    void Add(const T& item, TAllocator& allocator)
    {
        if (this->data == nullptr)
        {
            this->data = item;
            return;
        }
        EnsureCollection(allocator)->Add(item, allocator);
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
    CollectionT * EnsureCollection(TAllocator& allocator)
    {
        if (IsMultiple())
        {
            return GetCollection();
        }

        auto _collection = new (allocator.Alloc(sizeof(CollectionT))) CollectionT();
        _collection->Add(data, allocator);
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

template <typename T, typename TAllocator>
class Array
{
public:
    Array() : size(0), buffer(nullptr) {};
    ~Array()
    {
        if (TAllocator::NeedFree)
        {
            if (buffer)
            {
                delete[] buffer;
            }
        }
    }

    void Initialize(Set<T> const& items, TAllocator& allocator)
    {
        Assert(size == 0);
        Assert(buffer == nullptr);
        T * newBuffer = new (allocator.Alloc(sizeof(T) * items.Count())) T[items.Count()];
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

template <typename TKey, typename TData, typename TAllocator>
class GraphNode : public TData
{
public:
    GraphNode(TKey key) : TData(key) {}
    
    void SetSuccessors(Set<GraphNode *> const& set, TAllocator& allocator)
    {
        successors.Initialize(set, allocator);
        set.MapAll([this, &allocator](GraphNode * node)
        {
            node->predecessors.Add(this, allocator);
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
    bool MapSuccessors(Fn func)
    {
        return successors.Map(func);
    }

    template <typename Fn>
    bool MapPredecessors(Fn func)
    {
        return predecessors.Map(func);
    }

    template <typename Fn>
    void MapAllSuccessors(Fn func)
    {
        successors.MapAll(func);
    }

    template <typename Fn>
    void MapAllPredecessors(Fn func)
    {
        predecessors.MapAll(func);
    }
    
private:
   
    PointerCollection<GraphNode *, SingleLinkList<GraphNode *, TAllocator>, TAllocator> predecessors;
    Array<GraphNode *, TAllocator> successors;
};

class DefaultAllocator
{
public:
    static const bool NeedFree = true;

    void * Alloc(size_t size)
    {
        return new byte[size];
    }

    void Free(void * ptr)
    {
        delete ptr;
    }
};

class ReservedPageAllocator
{
public:
    static const bool NeedFree = false;

    ReservedPageAllocator();
    ~ReservedPageAllocator();

    void * Alloc(size_t size);
    void Free(void * ptr);
private:
    static const size_t segmentSize = 64 * 1024;
    struct ReservedSegment
    {
        ReservedSegment * next;
        char * nextAlloc;
        size_t freeSize;
    };

    static ReservedSegment * AllocSegment(size_t size);

    struct LargeAlloc
    {
        LargeAlloc * next;
    };
    ReservedSegment * head;
    LargeAlloc * largeHead;
};

template <typename TKey, typename TData, typename TAllocator = DefaultAllocator>
class Graph
{
public:
    typedef GraphNode<TKey, TData, TAllocator> NodeType;

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
        NodeType * node = new (allocator.Alloc(sizeof(NodeType))) NodeType(key);
        if (!node)
        {
            g_Ext->ThrowOutOfMemory();
        }
        
        _nodes.Add(node);
        return node;
    }

    NodeType * FindNode(const TKey& key)
    {   
        if (NodeType::IsLegalAddress(key))
        {
            NodeType node = NodeType(key);
            return _nodes.Get(&node);
        }
        return nullptr;
    }

    void AddEdges(NodeType * nodeFrom, Set<NodeType *> const& successors)
    {
        nodeFrom->SetSuccessors(successors, this->allocator);
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
            EmitEdges([=](char rootChar, ULONG64 fromPointer, ULONG64 toPointer)
            {
                if (rootChar)
                {
                    fprintf(f, g_Ext->m_PtrSize == 8 ? "G.add_edge('0x%c', '0x%016llX')\n" : "G.add_edge('0x%c', '0x%08llX')\n", rootChar, fromPointer);
                }
                else
                {
                    fprintf(f, g_Ext->m_PtrSize == 8 ? "G.add_edge('0x%016llX', '0x%016llX')\n" : "G.add_edge(\'0x%08llX', '0x%08llX')\n", fromPointer, toPointer);
                }
            });          
            fclose(f);
            g_Ext->Out("Finished saving to '%s'\n", filename);
        }
        else
        {
            g_Ext->Err("Could not open '%s'\n", filename);
        }
    }

    // Export to Js
    // Uses JsNetworkX library's format
    void ExportToJs(const char* filename)
    {
        FILE* f = fopen(filename, "w+");
        if (f != nullptr)
        {
            EmitEdges([=](char rootChar, ULONG64 fromPointer, ULONG64 toPointer)
            {
                if (rootChar)
                {
                    fprintf(f, g_Ext->m_PtrSize == 8 ? "G.add_edge(\"0x%c\", \"0x%016llX\")\n" : "G.add_edge(\"0x%c\", \"0x%08llX\")\n", rootChar, fromPointer);
                }
                else
                {
                    fprintf(f, g_Ext->m_PtrSize == 8 ? "G.add_edge(\"0x%016llX\", \"0x%016llX\")\n" : "G.add_edge(\"0x%08llX\", \"0x%08llX\")\n", fromPointer, toPointer);
                }
            });
            fclose(f);
            g_Ext->Out("Finished saving to '%s'\n", filename);
        }
        else
        {
            g_Ext->Err("Could not open '%s'\n", filename);
        }
    }    
    
    // Export to CSV
    // nodes and edges in separate .csv files
    // nodes: Addr,Size,HasVTbl,Type,TypeId
    // edges: source, destPointer
    bool ExportToCsv(const char* filename)
    {        
        char outputFileName[_MAX_PATH];
        if (strcpy_s(outputFileName, filename) != 0)
        {
            g_Ext->Err("Could not copy '%s'\n", filename);
            return false;
        }
        if (strcat_s(outputFileName, ".nodes.csv") != 0)
        {
            g_Ext->Err("Could not append '%s' to '%s'\n", ".nodes.csv", outputFileName);
            return false;
        }
        FILE * f = fopen(outputFileName, "w+");
        if (f == nullptr)
        {
            g_Ext->Err("Could not open '%s'\n", outputFileName);
            return false;
        }

        fprintf(f, "Addr,Size,HasVTbl,Type,TypeId\n");
        this->MapAllNodes([=](NodeType * node)
        {
            fprintf(f, g_Ext->m_PtrSize == 8 ? "0x%016llX" : "0x%08llX", node->Key());
            fprintf(f, ",%d", node->GetObjectSize());
            fprintf(f, node->HasVtable() ? ",1" : ",0");
            fprintf(f, ",\"%s\"", node->HasTypeInfo() && !node->IsPropagated()? node->GetTypeNameOrField() : "");
            /* TODO: TypeId */
            fprintf(f, ",");
            fprintf(f, "\n");
        });

        fclose(f);

        if (strcpy_s(outputFileName, filename) != 0)
        {
            g_Ext->Err("Could not copy '%s'\n", filename);
            return false;
        }
        if (strcat_s(outputFileName, ".edges.csv") != 0)
        {
            g_Ext->Err("Could not append '%s' to '%s'\n", ".edges.csv", outputFileName);
            return false;
        }

        f = fopen(outputFileName, "w+");
        if (f == nullptr)
        {
            g_Ext->Err("Could not open '%s'\n", outputFileName);
            return false;
        }
        EmitEdges([=](char rootChar, ULONG64 fromPointer, ULONG64 toPointer)
        {
            if (rootChar)
            {
                fprintf(f, g_Ext->m_PtrSize == 8? "%c,0x%016llX\n" : "%c,0x%08llX\n", rootChar, fromPointer);
            }
            else
            {
                fprintf(f, g_Ext->m_PtrSize == 8 ? "0x%016llX,0x%016llX\n": "0x%08llX,0x%08llX\n", fromPointer, toPointer);
            }
        });
        fclose(f);
        g_Ext->Out("Finished saving to '%s'\n", filename);
        return true;
    }

    Graph() : edgeCount(0) {};
    ~Graph()
    {
        if (TAllocator::NeedFree)
        {
            _nodes.MapAll([this](const NodeType* node)
            {
                node->~NodeType();
                allocator.Free((void *)node);
            });
        }
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
    template <typename Fn>
    void EmitEdges(Fn emitOneEdge)
    {
        this->MapAllNodes([&](NodeType* node)
        {
            ULONG64 fromPointer = node->Key();
            RootType rootType = node->GetRootType();
            if (RootTypeUtils::IsType(rootType, RootType::RootTypePinned))
            {
                emitOneEdge('P', fromPointer, 0);
            }
            if (RootTypeUtils::IsType(rootType, RootType::RootTypeStack))
            {
                emitOneEdge('S', fromPointer, 0);
            }
            if (RootTypeUtils::IsType(rootType, RootType::RootTypeRegister))
            {
                emitOneEdge('R', fromPointer, 0);
            }
            if (RootTypeUtils::IsType(rootType, RootType::RootTypeArena))
            {
                emitOneEdge('A', fromPointer, 0);
            }
            if (RootTypeUtils::IsType(rootType, RootType::RootTypeImplicit))
            {
                emitOneEdge('I', fromPointer, 0);
            }

            node->MapAllSuccessors([&](NodeType* toNode)
            {
                ULONG64 toPointer = toNode->Key();
                emitOneEdge(0, fromPointer, toPointer);
            });
        });
    }

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

    TAllocator allocator;
};

#include "RecyclerObjectTypeInfo.h"

struct RecyclerGraphNodeData
{
#if DBG
    friend RecyclerObjectGraph;
#endif

    RecyclerGraphNodeData(ULONG64 address) :
        address(address),
        depth(0),
        rootType(RootType::RootTypeNone)
    {
        Assert(IsLegalAddress(address));
        ClearTypeInfo();
    }

    ULONG64 Key() const { return address; }
    uint GetObjectSize() const { return objectSize; }
    void SetObjectSize(uint size) { objectSize = size; }
    uint GetDepth() const { return depth; }
    void SetDepth(uint d) { this->depth = d; }

    bool HasTypeInfo() const { return GetTypeName() != nullptr; }
    void SetTypeInfo(char const * typeName, char const * typeNameOrField, RecyclerObjectTypeInfo::Flags flags, ULONG64 javascriptLibrary);
    void ClearTypeInfo() { typeInfo = nullptr; }
    
    bool HasVtable() const { return typeInfo ? typeInfo->HasVtable() : false; }
    bool IsPropagated() const { return typeInfo? typeInfo->IsPropagated() : false; }
    bool OverrideVtable() const { return typeInfo ? typeInfo->IsPropagated() : false; }
    const char * GetTypeName() const { return typeInfo? typeInfo->GetTypeName() : nullptr; }
    const char * GetTypeNameOrField() const { return typeInfo? typeInfo->GetTypeNameOrField() : nullptr; }
    ULONG64 GetAssociatedJavascriptLibrary() const { return typeInfo ? typeInfo->GetAssociatedJavascriptLibrary() : 0; }
    bool IsRoot() const { return RootTypeUtils::IsAnyRootType(rootType); }
    RootType GetRootType() const { return rootType; }
    void AddRootType(RootType rootType) { this->rootType = RootTypeUtils::CombineTypes(this->rootType, rootType); }

    static bool IsLegalAddress(ULONG64 address)
    {
        return (address & 0xFF00000000000000) == 0;
    }

private:
    RootType rootType : 8;
    const ULONG64 address : 56;
    uint objectSize;
    uint depth;
    RecyclerObjectTypeInfo * typeInfo;
};

