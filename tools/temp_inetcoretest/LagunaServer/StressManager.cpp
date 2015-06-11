#include "stdafx.h"


StressManager::StressManager(LPCWSTR hostProviderName_, int requestCount_, int threadCount_, int runtimeCount_, int precompile_, IJsRuntimeProvider* runtime_provider) : 
    requestCount(requestCount_), 
    runtimeCount(runtimeCount_),
    createdHostCount(0),
    completedHostCount(0),
    id(0),
    m_pThreadPool(NULL), 
    precompile(precompile_),
    hostProviderName(hostProviderName_),
    runtimeProvider(runtime_provider)
{
    CreateThreadPool(&m_pThreadPool);
    if (threadCount_ > 0)
    {
        m_pThreadPool->SetPoolSize(threadCount_, threadCount_);
    }
}

StressManager::~StressManager()
{

    for each(unsigned char* cache in m_bytecode)
    {
        delete[] cache;
    }

    m_pThreadPool->Release();
}

HRESULT StressManager::RemoveHost(void *host)
{
    PostNewRequest();
    
    if (InterlockedIncrement(&completedHostCount) == requestCount)
    {
        SetEvent(completionEvent);
    }
    
    return S_OK;
}

HRESULT StressManager::CreateHost(RuntimeHost **host)
{
    HRESULT hr = S_OK;

    RuntimeHost::HostData *hostData = NULL;
    RuntimeHost *tmpHost = NULL;
    *host = NULL;

    IFC(RuntimeHost::HostData::Create(this, &hostData));
    hostData->AddThreadPool(m_pThreadPool);
    IFC(RuntimeHost::Create(hostData, runtimeProvider, &tmpHost));

    // Set the current context, since the provider factory may need to create objects.
    IFC(tmpHost->SetCurrentContext());

    IHostObjectProvider::pf_ProviderFactory pf_factory = NULL;
    if (hostProviderName != NULL)
    {
        IHostObjectProvider *prov = NULL;
        IFC(IHostObjectProvider::GetProviderFactory(hostProviderName, &pf_factory));

        IFC(pf_factory(tmpHost, id++, &prov));

        prov->Release();
    }
    
    hostData->Release();

    *host = tmpHost;

Cleanup:
    if(*host == NULL)
    {
        Error(L"failed to create RuntimeHost");
    }
    if(tmpHost != NULL)
    {
        tmpHost->ClearCurrentContext();
    }

    return hr;
}

void StressManager::Error(__in wchar_t *message)
{
    // TODO: allow app to override this behavior.  For now, just be cautious and exit the app,
    // because we're using this for perf and functional validation.
    printf("ERROR: %S\n", message);
    exit(1);
}

void StressManager::CreateHostAndExecute()
{
    try 
    {
        RuntimeHost *obj = NULL;

        CreateHost(&obj);
        if(!precompile)
        {
            obj->RunScript(m_scripts[rand() % m_scripts.size()].c_str());
        }
        else
        {
            assert(m_scripts.size() == m_bytecode.size());
            int id = rand() % m_scripts.size();
            obj->RunScript(m_scripts[id].c_str(), m_bytecode[id]);
        }
        obj->Release();
    }
    catch(JsRT::Exception ex)
    {
        Error(L"CreateHostAndExecute failed with an exception");
    }
}

void StressManager::PostNewRequest()
{
    if (InterlockedIncrement(&createdHostCount) <= requestCount)
    {
        // Queue another request
        m_pThreadPool->ExecuteAsync(IThreadPool::MakeMethodCallback<StressManager, &StressManager::CreateHostAndExecute>(), this, NULL);
    }
}

HRESULT StressManager::AddScript(__in const wchar_t *script)
{
    HRESULT hr = S_OK;

    m_scripts.push_back(std::wstring(script));

    if(precompile)
    {
        Runtime runtime = Runtime::New();
        Context context = Context::New(runtime);
        Context::Scope scope(context);

        unsigned long bytesNeeded = Context::Compile(script, NULL, 0);
        unsigned char *buf = new unsigned char[bytesNeeded];
        Context::Compile(script, buf, bytesNeeded);
        
        m_bytecode.push_back(buf);

        runtime.Dispose();
    }

    return hr;
}

HRESULT StressManager::Run()
{
    HRESULT hr = S_OK;

    completionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    createdHostCount = 0;

    // Queue up work for specified number of runtimes
    for (int i = 0; i < runtimeCount; i++)
    {
        PostNewRequest();
    }

    WaitForSingleObject(completionEvent, INFINITE);

    // Quick sanity check
    // Total createdHostCount should be requestCount + runtimeCount, 
    // since we should increment it one extra time at the end for each runtime
    if (createdHostCount != (requestCount + runtimeCount))
    {
        Error(L"Internal error on total host count");
    }

Cleanup:
    return hr;
}
