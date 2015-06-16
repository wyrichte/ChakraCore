//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#ifdef GARBAGE_COLLECTION_DIAGNOSTICS
void WhyIsThisAlive(ThreadContext *threadContext, void *reference);
#endif

namespace Authoring
{

    HRESULT AuthoringFactory::GetColorizer(IAuthorColorizeText **result)
    {
        STDMETHOD_PREFIX;

        // Force a thread context
        if (!ThreadBoundThreadContextManager::EnsureContextForCurrentThread())
        {
            return E_FAIL;
        }

        *result = new Colorizer(this->GetAuthoringScriptContext());

        STDMETHOD_POSTFIX;
    }

    HRESULT AuthoringFactory::RegisterFile(IAuthorFile *file, IAuthorFileHandle **result)
    {
        STDMETHOD_PREFIX;

        ValidateArg(file && result);

        *result = new AuthoringFileHandle(this, file);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (PHASE_TRACE1(Js::JSLSPhase) && *result != nullptr && file != nullptr)
        {
            long fileLength;
            file->GetLength(&fileLength);
            CComBSTR name;
            file->GetDisplayName(&name);
            if (!name) name = L"<no name>";
            OUTPUT_TRACE(Js::JSLSPhase, L"AuthoringFactory::RegisterFile : url : %ls, length %d\n", name, fileLength);
        }
#endif

        STDMETHOD_POSTFIX;
    }

    HRESULT AuthoringFactory::GetFileAuthoring(IAuthorFileContext* context, IAuthorFileAuthoring **result)
    {
        STDMETHOD_PREFIX;

        ValidateArg(context && result);

        if (!m_authoringServicesInfo)
        {
            m_authoringServicesInfo.Assign(AuthoringServicesInfo::New(this->GetAuthoringScriptAllocator(), new ScriptContextManager(this)));
        }
        Assert(m_authoringServicesInfo);
        
        *result = new FileAuthoring(this, m_authoringServicesInfo, this->GetAuthoringScriptContext(), context);

        ValidateAlloc(*result);

        STDMETHOD_POSTFIX;
    }

    void AuthoringFactory::CloseLanguageService()
    {
        if (m_authoringServicesInfo)
        {
            m_authoringServicesInfo->NotifyEngineClosing();
            m_authoringServicesInfo.ReleaseAndNull();
        }

        if (m_recyclerCallback)
        {
            auto threadContext = this->GetAuthoringScriptContext()->GetThreadContext();
            Assert(threadContext);
            threadContext->RemoveRecyclerCollectCallBack(m_recyclerCallback);
        }

        m_host.ReleaseAndNull();
    } 

    void AuthoringFactory::DecommitUnusedPages(PageAllocator *pageAllocator)
    {
        Assert(pageAllocator);

        pageAllocator->SuspendIdleDecommit();
        pageAllocator->DecommitNow();
        pageAllocator->ResumeIdleDecommit();
    }

#ifdef GARBAGE_COLLECTION_DIAGNOSTICS
    typedef JsUtil::BaseDictionary<void *, void *, HeapAllocator, PrimeSizePolicy, RecyclerPointerComparer> ParentMap;
    
    ParentMap *wasAllocated = nullptr;
    ParentMap *nowAllocated = nullptr;

    bool UpdateCallback(const wchar_t *name, void *parent, void *object) 
    {
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);

        nowAllocated->Item(object, parent);
        return false;
    }

    void UpdateAllocated(ThreadContext *threadContext)
    {
        ExceptionCheck::ClearHandledExceptionType();
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);
        Recycler *recycler = threadContext->GetRecycler();
        auto saved = ExceptionCheck::Save();
        nowAllocated = new ParentMap(&HeapAllocator::Instance);
        RecyclerObjectGraphDumper::Param param = { UpdateCallback, false, true };
        recycler->DumpObjectGraph(&param);
        int count = 0;

        auto dumpDescription = [&](wchar_t const *description, void *address) {
            Output::Print(description);
            if (address)
                recycler->DumpObjectDescription(address);
            else
                Output::Print(L"null");
            Output::Print(L"  ");
        };
        if (wasAllocated)
        {
            void *candidate = nullptr;
            nowAllocated->Map([&](void *key, void *value) {
                if (!wasAllocated->ContainsKey(key))
                {
                    auto parent = value;
                    auto parentParent = nowAllocated->Lookup(parent, nullptr);
                    dumpDescription(L"Object: ", key);
                    dumpDescription(L"Parent: ", parent);
                    dumpDescription(L"Gramps: ", parentParent);
                    Output::Print(L"\n");
                    count++;
                    candidate = value;
                }
            });
            Output::Print(L"-----------------------------------------------------\n");
            Output::Print(L"%d allocations left\n\n", count);
            delete wasAllocated;

            if (candidate)
                WhyIsThisAlive(threadContext, candidate);
        }
        wasAllocated = nowAllocated;
        ExceptionCheck::Restore(saved);
    }
#endif

    void AuthoringFactory::CleanupPropertyCaches()
    {
        if (m_authoringServicesInfo) 
        {
            auto scriptContextManager = m_authoringServicesInfo->GetScriptContextManager();
            if (scriptContextManager)
            {
                scriptContextManager->CleanupScriptPropertyCaches();
            }
        }
    }

    HRESULT AuthoringFactory::Cleanup(VARIANT_BOOL exhaustive)
    {
        STDMETHOD_PREFIX;

        auto threadContext = this->GetAuthoringScriptContext()->GetThreadContext();

        Assert(threadContext);

#ifdef LOG_CLEANUP_TIMES
        double startTime = threadContext->GetHiResTimer()->Now();
#endif

        // Perform any deletes that came from threads other than the language service thread.
        PerformScheduledReleases();

        if (exhaustive || !m_inCleanup)
        {
            // Only start a cleanup if we are not already in one or if we are no going to be exhaustive.

            auto recycler = threadContext->GetRecycler();
            auto pageAllocator = threadContext->GetPageAllocator();
        
            Assert(recycler);
        
            if (exhaustive)
            {
                // Weak references take two collections to go away. The first pass will nill the weak reference handle
                // the second pass will collect the weak reference itself.
                recycler->CollectNow<CollectNowDefaultLSCleanup>();
            }
        
            // This forces the property string map used to turn PropertyId's to a string to release weak reference
            // handles to release weak reference handles to strings no longer in use. Normally this will not happen.
            CleanupPropertyCaches();

            // This forces the thread to release property records for strings that are no longer in use. This will
            // happen eventually but this forces it to happen now.
            threadContext->ForceCleanPropertyMap();

            if (exhaustive || !StartConcurrentCleanup(threadContext))
            {
                // Force the garbage collector to collect now.
                recycler->CollectNow<CollectNowDefaultLSCleanup>();

                // Force the page alloctor to give-up pages it is holding onto to avoid otherwise unnecessary calls to
                // VirtualAlloc/VirtualFree. 
                DecommitUnusedPages(pageAllocator);
    
#ifdef GARBAGE_COLLECTION_DIAGNOSTICS
                UpdateAllocated(threadContext);
#endif
            }
        }

#ifdef LOG_CLEANUP_TIMES
        double endTime = threadContext->GetHiResTimer()->Now();

        Output::Print(L"Cleanup time: %f\n", endTime - startTime);
#endif

        STDMETHOD_POSTFIX;        
    }

    HRESULT AuthoringFactory::SetHost(IAuthorServiceHost *host)
    {
        STDMETHOD_PREFIX;

        this->m_host.Assign(host);

        STDMETHOD_POSTFIX;
    }

    HRESULT AuthoringFactory::Work()
    {
        STDMETHOD_PREFIX;

        // This method is called by the host in response to calling RequestWork().

        if (m_inCleanup)
        {
            // We are in a concurrent collection, request the recycler to finish the
            // next step of the concurrent collection by calling FinishConcurrent().
            auto threadContext = this->GetAuthoringScriptContext()->GetThreadContext();
            
            Assert(threadContext);
        
            auto recycler = threadContext->GetRecycler();
            recycler->FinishConcurrent<FinishConcurrentLanguageService>();
        }

        STDMETHOD_POSTFIX;
    }

    bool AuthoringFactory::StartConcurrentCleanup(ThreadContext *threadContext)
    {
        // Requires the host implement RequestWork();
        if (!m_host) 
            return false;

        // Install the recycler callback if not already installed.
        if (!this->m_recyclerCallback)
        {
            this->m_recyclerCallback = threadContext->AddRecyclerCollectCallBack(RecyclerCallback, this);
        }

        // Let the recycler callback know that we are in a concurrent cleanup and it
        // should "move the recycler along" in reponse to waits.
        this->m_inCleanup = true;

        // Start a concurrent collection.
        auto recycler = threadContext->GetRecycler();
        Assert(recycler);
        recycler->CollectNow<CollectNowConcurrent>();

        return true;
    }

    void AuthoringFactory::RecyclerCallback(void *context, RecyclerCollectCallBackFlags flags)
    {
        Assert(context);

        auto factory = reinterpret_cast<AuthoringFactory *>(context);
        if (factory->m_inCleanup)
        {
            if ((flags & Collect_End) == Collect_End)
            {
                factory->m_inCleanup = false;
            }
            else if ((flags & Collect_Wait) == Collect_Wait)
            {
                // We are in some random thread (most likely the concurent collection thread but 
                // not the lanuguage service thread).                 
                
                // The garbage collector finished a task and is now waiting to be told when to
                // finish its work. We need to call Recycler::FinishConcurrent() but in the 
                // language service thread, not this one.

                // To get back to the language service thread we request the host to call us. The 
                // host (if it is provided) is prepared to receive calls to RequestWork() on any 
                // thread. It will then post a message of some kind to the language service thread 
                // to call IAuthoringServices::Work(). In Work() we will tell the recycler it is a 
                // good time to finish its work.

                // Eventually we will receive a Collect_End and then stop paying attention to the
                // recycler. At that point we should have gone through one complete collection.
                if (factory->m_host)
                {
                    // This will request the host to call Work().
                    factory->m_host->RequestWork();
                }
            }
        }
    }
}