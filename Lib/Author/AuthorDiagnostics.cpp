//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

using namespace Authoring;

template<typename TElement>
class SimpleList
{
public:
    static void Prepend(TElement*& head, TElement* element)
    {
        Assert(element);
        if(head) 
        {
            element->Next = head;
            head = element;
        } 
        else
        {
            head = element;
            element->Next = nullptr;
        }
    }

    template<typename THandler>
    static void ForEach(TElement* head, THandler handler)
    {
        auto current = head;
        while(current)
        {
            handler(current);
            current = current->Next;
        }
    }
};

//
//  TypeStats
//  

static TypeStats* allTypes = nullptr;

TypeStats::TypeStats(LPCWSTR typeName)
    : Next(nullptr), TypeName(typeName), Instances(0)
{
    SimpleList<TypeStats>::Prepend(allTypes, this);
}

void TypeStats::IncrementInstances()
{
    ::InterlockedIncrement(&Instances);
}

void TypeStats::DecrementInstances()
{
    ::InterlockedDecrement(&Instances);
}

void TypeStats::Dump()
{
    Output::Print(L">>>> COM type: %s instances#: %d\n", TypeName, Instances);
}

#ifdef MEMORY_LEAK_DIAGNOSTICS
class WhyAliveGraph
{
private:
    struct KeptAliveBy
    {
        void *parentObject;
        KeptAliveBy *next;
        KeptAliveBy(void *parentObject, KeptAliveBy *next) : parentObject(parentObject), next(next) { }
    };
    typedef JsUtil::BaseDictionary<void *, KeptAliveBy *, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> ReferenceMap;

    ReferenceMap referenceMap;
    ArenaAllocator *alloc;
    void *watch;
    void *watchObject;
    int seen;

    bool AddReference(wchar_t const *objectName, void *objectAddress, void *referenceAddress)
    {
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);

        KeptAliveBy *parentChain;
        bool watched = referenceAddress == watch;
        if (watched) 
            seen++;
        if (watchObject && objectAddress == watchObject)
            seen++;
        if (!referenceMap.TryGetValue(referenceAddress, &parentChain))
            parentChain = nullptr;
        if (!parentChain || parentChain->parentObject != objectAddress)
        {
            void *parent = objectAddress;
            if (!parent && objectName)
            {
                auto len = wcslen(objectName) + 1;
                auto t = (wchar_t*)alloc->Alloc((len + 1) * sizeof(wchar_t));
                ::memcpy_s(t, len * sizeof(wchar_t), objectName, len * sizeof(wchar_t));
                parent = (void *)((int)t | 1);
            }
            KeptAliveBy *newParent = Anew(alloc, KeptAliveBy, parent, parentChain);
            referenceMap.Item(referenceAddress, newParent);
        }
        return watched;
    }

    static WhyAliveGraph *current;
    static bool AddReferenceFunc(wchar_t const *objectName, void *objectAddress, void *referenceAddress)
    {
        return current->AddReference(objectName, objectAddress, referenceAddress);
    }


public:
    WhyAliveGraph(ArenaAllocator *alloc): alloc(alloc), referenceMap(alloc, 100000), watch(nullptr), watchObject(nullptr), seen(0) { }

    void Initialize(Recycler *recycler, void *watch = nullptr) 
    {
        this->watch = watch;
        seen = 0;
        current = this;
        RecyclerObjectGraphDumper::Param param = { AddReferenceFunc, false, true };        
        recycler->DumpObjectGraph(&param);
    }

    template<typename THandler>
    void EachParent(void *object, THandler handler)
    {
        typedef JsUtil::BaseDictionary<void *, void *, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> ReportedMap;
        struct Stack 
        {
        private:
            struct StackItem 
            {
                void *reference;
                StackItem *next;
                StackItem(void *reference, StackItem *next): reference(reference), next(next) { }
            };
            StackItem *top;
            ArenaAllocator *alloc;
        public:
            Stack(ArenaAllocator *alloc): alloc(alloc) { }
            void Push(void *reference)
            {
                top = Anew(alloc, StackItem, reference, top);
            }
            void *Pop()
            {
                if (top)
                {
                    void *result = top->reference;
                    top = top->next;
                    return result;
                }
                return nullptr;
            }
        };

        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);
        ReportedMap reported(alloc, 10000);
        Stack stack(alloc);
        stack.Push(object);
        while (true)
        {
            void *current = stack.Pop();
            if (!current) break;
            if (reported.ContainsKey(current)) continue;
            reported.Add(current, current);
            KeptAliveBy *parentChain;
            referenceMap.TryGetValue(current, &parentChain);
            for (KeptAliveBy *currentParent = parentChain; currentParent; currentParent = currentParent->next)
            {
                void *parentObject = currentParent->parentObject;
                handler(current, parentObject);
                if (parentObject && !reported.ContainsKey(parentObject))
                    stack.Push(parentObject);
            }
        }
    }

    struct Chain
    {
        Chain *next;
        void *object;
        Chain(Chain *next, void *object): next(next), object(object) { }
    };

    Chain *ShortestChain(ArenaAllocator *alloc, void *object, Recycler *recycler)
    {

        struct Search
        {
            Search *next;
            Chain *chain;
            Search(Search *next, Chain *chain): next(next), chain(chain) { }
        };

        typedef JsUtil::BaseDictionary<void *, void *, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> TrackingMap;
        
        Search *searches = Anew(alloc, Search, nullptr, Anew(alloc, Chain, nullptr, object));
        auto searching = Anew(alloc, TrackingMap, alloc);

        Chain *shortest = nullptr;
        bool progress = true;
        while (!shortest && progress) 
        {
            progress = false;

            // Step all searches
            for (Search *search = searches; search; search = search->next)
            {
                // Step the search
                KeptAliveBy *parentChain;
                if (search->chain->object && referenceMap.TryGetValue(search->chain->object, &parentChain))
                {
                    if (parentChain)
                    {
                        bool first = true;
                        for (KeptAliveBy *current = parentChain; current; current = current->next) 
                        {
                            auto parent = current->parentObject;
                            if (!searching->ContainsKey(parent))
                            {
                                progress = true;
                                searching->Add(parent, parent);
                                if (first)
                                {
                                    // Step this search by the first parent, and create new searches, with for each of the other parents;
                                    search->chain = Anew(alloc, Chain, search->chain, parent);
                                    first = false;
                                }
                                else
                                {
                                    searches = Anew(alloc, Search, searches, Anew(alloc, Chain, search->chain, parent));
                                }
                            }
                        }
                    }
                }
                else
                {
                    shortest = search->chain;
                    break;
                }
            }
        }

        return shortest;
    }

    void PrintChain(ThreadContext *threadContext, Chain *chain)
    {
        if (chain)
        {
            auto recycler = threadContext->GetRecycler();
            for (auto current = chain; current; current = current->next)
                if ((int)current->object & 1)
                    Output::Print(L"%s\n", (wchar_t*)((int)current->object & ~1));
                else if (!current->object)
                {
                    Output::Print(L"null\n");
                }
                else
                {
                    recycler->DumpObjectDescription(current->object);
                    Output::Print(L"\n");
                }
        }
    }
};
WhyAliveGraph* WhyAliveGraph::current = nullptr;
#endif

//
//  AuthorDiagnosticsImpl
//  

class AuthorDiagnosticsImpl: public SimpleComObjectWithAlloc<IAuthorDiagnostics>
{
    class AllocInfo: public InnerComObject<IAuthorAllocInfo>
    {
        ComStringField m_tag;
        ComStringField m_category;
        int           m_size;
        int           m_count;
    public:
        AllocInfo(AuthorDiagnosticsImpl* parent, LPCWSTR tag, LPCWSTR category, int size, int count)
            :InnerComObject<IAuthorAllocInfo>(parent), m_size(size), m_count(count)
        {
            Assert(parent != NULL);
            m_tag.Set(parent->Alloc(), tag);
            m_category.Set(parent->Alloc(), category);
        }
        // IAuthorAllocInfo
        STRING_PROPERTY_IMPL(Tag, m_tag)
        SIMPLE_PROPERTY_IMPL(Size, int, m_size)
        SIMPLE_PROPERTY_IMPL(Count, int, m_count)
        STRING_PROPERTY_IMPL(Category, m_category)
    }; // End AllocInfo

    Js::ScriptContext* m_scriptContext;

    typedef Collection<IAuthorAllocInfoSet,IAuthorAllocInfo> AllocInfoSet;
public:
    AuthorDiagnosticsImpl(Js::ScriptContext* scriptContext)
        : SimpleComObjectWithAlloc<IAuthorDiagnostics>(scriptContext->GetThreadContext()->GetPageAllocator(), L"LS - AuthorDiagnostics"), 
        m_scriptContext(scriptContext) 
    { 
    }

#ifdef MEMORY_LEAK_DIAGNOSTICS
    static HRESULT WhyIsThisAlive(ThreadContext *threadContext, void *reference)
    {
        HRESULT hr = S_OK;
        
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);

        ArenaAllocator localAlloc(L"ls:WhyAlive", threadContext->GetPageAllocator(), nullptr);
        WhyAliveGraph graph(&localAlloc);
        graph.Initialize(threadContext->GetRecycler(), reference);
        auto chain = graph.ShortestChain(&localAlloc, reference, threadContext->GetRecycler());
        graph.PrintChain(threadContext, chain);
        return hr;
    }
#endif

    //
    // IAuthorDiagnostics interface
    //

    HRESULT STDMETHODCALLTYPE get_AllocStats(__out IAuthorAllocInfoSet** value) 
    { 
        IfNullReturnError(value, E_POINTER); 
        HRESULT hr = S_OK;
        *value = nullptr;
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            auto allocInfoSet = Anew(Alloc(), AllocInfoSet, this, Alloc());
    #ifdef PROFILE_MEM
            MemoryProfiler::GetArenaMemoryUsage([&] (LPCWSTR arenaName, ArenaMemoryDataSummary *memoryUsage)
            {
                auto item = Anew(Alloc(), AllocInfo, this, arenaName, L"ARENA", memoryUsage->total.allocatedBytes, memoryUsage->arenaCount);
                allocInfoSet->Add(item);
            });
    #endif
            SimpleList<TypeStats>::ForEach(allTypes, [&] (TypeStats* typeStats) {
                auto item = Anew(Alloc(), AllocInfo, this, typeStats->TypeName, L"Object", 0, typeStats->Instances);
                allocInfoSet->Add(item);
            });

            allocInfoSet->AddRef();
            *value = allocInfoSet; 
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        return hr; 
    } 

    STDMETHOD(ForceGC)()
    {
        HRESULT hr = S_OK;
        m_scriptContext->GetThreadContext()->GetRecycler()->CollectNow<CollectionFlags::CollectNowExhaustive>();

#ifdef MEMORY_LEAK_DIAGNOSTICS
        hr = WhyIsThisAlive(m_scriptContext->GetThreadContext(), nullptr);
#endif
        return hr;
    }
};

#ifdef MEMORY_LEAK_DIAGNOSTICS
void WhyIsThisAlive(ThreadContext *threadContext, void *reference)
{
    AuthorDiagnosticsImpl::WhyIsThisAlive(threadContext, reference);
}
#endif

TYPE_STATS(AuthorDiagnosticsImpl, L"AuthorDiagnosticsImpl")
TYPE_STATS(AuthorDiagnosticsImpl::AllocInfo, L"AuthorDiagnosticsImpl::AllocInfo")
TYPE_STATS(AuthorDiagnosticsImpl::AllocInfoSet, L"AllocInfoSet")

IAuthorDiagnostics* AuthorDiagnostics::CreateInstance(Js::ScriptContext* scriptContext)
{
    return new AuthorDiagnosticsImpl(scriptContext);
}

void AuthorDiagnostics::DumpTypes()
{
    SimpleList<TypeStats>::ForEach(allTypes, [&] (TypeStats* typeStats) {
        typeStats->Dump();
    });
}

void AuthorDiagnostics::DumpMemoryUsage()
{
#ifdef PROFILE_MEM
    int total = 0;
    MemoryProfiler::GetArenaMemoryUsage([&] (LPCWSTR arenaName, ArenaMemoryDataSummary *memoryUsage)
    {
        total += memoryUsage->total.allocatedBytes;
        Output::Print(L">>> Arena: %s, memory allocated %d\n", arenaName, memoryUsage->total.allocatedBytes);       
    });
    Output::Print(L">>> TOTAL: %d\n", total);
#else
    Output::Print(L"Memory diagnostics not enabled\n");
#endif
}

int AuthorDiagnostics::TotalMemoryOf(LPCWSTR name)
{
#ifdef PROFILE_MEM
    int total = 0;
    MemoryProfiler::GetArenaMemoryUsage([&] (LPCWSTR arenaName, ArenaMemoryDataSummary *memoryUsage)
    {
        if (::wcscmp(arenaName, name))
            total += memoryUsage->total.allocatedBytes;
    });
    return total;
#else
    return 0;
#endif
}
