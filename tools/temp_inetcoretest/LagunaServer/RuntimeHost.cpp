#include "stdafx.h"
using namespace JsRT;

RuntimeHost::RuntimeHost() : m_isProcessingQueue(false)
{
}

HRESULT RuntimeHost::Create(HostData *data, IJsRuntimeProvider* provider, RuntimeHost **host)
{
    HRESULT hr = S_OK;
    RuntimeHost *tmpHost = NULL;
    IMonitor *tmpMonitor = NULL;

    IFCPTR(host);
    IFCOOM(tmpHost = new RuntimeHost());

    tmpHost->m_runtimeProvider = provider;
    tmpHost->m_Runtime = provider->GetRuntime();
    tmpHost->m_Context = provider->GetContext(tmpHost->m_Runtime);

    IFC(CreateMonitor(&tmpMonitor));

    *host = tmpHost;
    tmpHost = NULL;

    (*host)->m_pMonitor = tmpMonitor;
    data->AddRef();
    (*host)->m_pHostData = data;

    tmpMonitor = NULL;

    (*host)->m_pObjectProvider = NULL;
    
Cleanup:
    if(tmpHost != NULL)
    {
        assert(false);
        delete tmpHost;
    }

    if(tmpMonitor != NULL)
    {
        assert(false);
        delete tmpMonitor;
    }

    return hr;
}

HRESULT RuntimeHost::RunScript(LPCWSTR script, unsigned char *byteCode)
{
    HRESULT hr = S_OK;

    IFC(Enter());
    {
        Context::Scope scope(m_Context->ToContext());
        Context::RunCompiled(script, byteCode);
    }

Cleanup:
    IFC(Leave());
    return hr;
}

HRESULT RuntimeHost::RunScript(LPCWSTR script)
{
    HRESULT hr = S_OK;

    IFC(Enter());
    {
        Context::Scope scope(m_Context->ToContext());
        Context::Run(script);
    }

Cleanup:
    IFC(Leave());
    return hr;
}

HRESULT RuntimeHost::AddObject(Object object, LPCWSTR name)
{
    HRESULT hr = S_OK;

    IFC(Enter());
    {
        Context::Scope scope(m_Context->ToContext());
        Object::Global().PutProperty(PropertyId::New(name), object);
    }

Cleanup:
    IFC(Leave());
    return hr;
}

HRESULT RuntimeHost::Enter()
{
    return m_pMonitor->Enter();
}

HRESULT RuntimeHost::Leave()
{
    return m_pMonitor->Leave();
}

void RuntimeHost::ProcessWorkItemQueue()
{
    CallbackData *data = NULL;
    bool done = false;

    // AddRef, because we'll need access to data structures that may otherwise be cleaned up once
    // the last work item is processed.
    this->AddRef();

    do
    {
        // Pop an item off the queue (locked).
        Enter();
        {
            if(m_workItemQueue.size() == 0)
            {
                done = true;
            }
            else
            {
                data = m_workItemQueue.front();
                m_workItemQueue.pop_front();
            }
        }
        Leave();

        if(done)
            break;

        // Process the work item.
        if(data != NULL)
        {
            try
            {
                // This could be hoisted out of the loop for today's scenarios, but we may support multiple contexts in the future.
                Context::Scope scope(data->host->m_Context->ToContext());

                JsValueRef result;
                JsCallFunction(data->jsFunction.Reference(), &data->jsArgs->args[0], data->jsArgs->args.size(), &result);
            }
            catch(JsRT::Exception ex)
            {
                // TODO: report the actual exception
                data->host->m_pHostData->m_pManager->Error(L"RuntimeHost::InternalCallback failed with an exception");
            }

            // Clean up the work object
            data->jsFunction.Release();
            data->jsArgs->Release();
            data->host->Release();
            delete data;
        }

    } while(!done);

    // Finished, allow cleanup.
    this->Release();
}

HRESULT RuntimeHost::QueueWorkItem(Function func, CallbackArguments *arguments)
{
    HRESULT hr = S_OK;
    CallbackData *data = NULL;

    IFC(Enter());
    
    // Create the new workitem and enqueue it.
    func.AddRef();
    IFCOOM(data = new CallbackData(func));
    this->AddRef();
    data->host = this;
    data->jsArgs = arguments;
    m_workItemQueue.push_back(data);

    // If the queue is already being processed, end here.
    if(m_isProcessingQueue)
    {
        goto Cleanup;
    }
    else
    {
        m_isProcessingQueue = true;

        m_pHostData->m_pools.front()->ExecuteAsync(IThreadPool::MakeMethodCallback<RuntimeHost, &RuntimeHost::ProcessWorkItemQueue>(), this, NULL);
    }

Cleanup:
    Leave();
    return hr;
}

ULONG RuntimeHost::AddRef()
{
    ULONG count = TestLibObject::AddRef();
    return count;
}

ULONG RuntimeHost::Release()
{
    ULONG count = TestLibObject::Release();
    if (count == 1)
    {
        // TODO, this is not quite right.
        m_pObjectProvider->Teardown();
    }
    else if (count == 0)
    {
        _Cleanup();
        
        delete this;
    }

    return count;
}

void RuntimeHost::_Cleanup()
{
    if (m_pObjectProvider != NULL)
    {
        m_pObjectProvider->Release();
    }
    
    m_pMonitor->Release();
    m_runtimeProvider->PutContext(m_Context);
    m_runtimeProvider->PutRuntime(m_Runtime);

    m_pHostData->m_pManager->RemoveHost(this);
    m_pHostData->Release();
}

ULONG RuntimeHost::HostData::Release()
{
    ULONG count = TestLibObject::Release();

    if(count == 0)
    {
        _Cleanup();

        delete this;
    }

    return count;
}

HRESULT RuntimeHost::SetCurrentContext()
{
    JsSetCurrentContext(m_Context->ToContext().Reference());
    return S_OK;
}

HRESULT RuntimeHost::ClearCurrentContext()
{
    JsSetCurrentContext(NULL);
    return S_OK;
}

void RuntimeHost::HostData::_Cleanup()
{
    for each(IThreadPool* pool in m_pools)
    {
        pool->Release();
    }
}

HRESULT RuntimeHost::HostData::Create(StressManager *manager, RuntimeHost::HostData **data)
{
    HRESULT hr = S_OK;
    HostData *tmp = NULL;
    IFCOOM(tmp = new HostData());

    tmp->m_pManager = manager;

    *data = tmp;

Cleanup:
    return hr;
}

void RuntimeHost::HostData::AddThreadPool(IThreadPool *pool)
{
    pool->AddRef();
    m_pools.push_back(pool);
}

HRESULT RuntimeHost::CallbackArguments::Create(CallbackArguments **obj)
{
    *obj = new CallbackArguments();
    
    // this-pointer is always undefined.
    (*obj)->Add((void*)Value::Undefined().Reference());
    return S_OK;
}

void RuntimeHost::CallbackArguments::Add(void *arg)
{
    JsAddRef(arg, nullptr);
    args.push_back(arg);
}


ULONG RuntimeHost::CallbackArguments::Release()
{
    ULONG count = TestLibObject::Release();
    if(count == 0)
    {
        for each(void *arg in args)
        {
            JsRelease(arg, nullptr);
        }
        delete this;
    }
    return count;
}

void RuntimeHost::SetHostObjectProvider(IHostObjectProvider *provider) 
{ 
    provider->AddRef(); 
    m_pObjectProvider = provider; 
};