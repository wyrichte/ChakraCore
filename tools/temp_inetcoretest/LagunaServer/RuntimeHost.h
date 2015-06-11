#pragma once

#include <list>
extern LONG tot;
using namespace JsrtTestLib;
using namespace JsRT;

class IHostObjectProvider;
class StressManager;

typedef JsRT::Value (CALLBACK * JsCallback)(JsRT::Arguments const& arguments);

// RuntimeHost: manages a Laguna Runtime, including the lifetime of the Runtime, as well
// as the threading model for callbacks between the host objects and Javascript.
class RuntimeHost : public TestLibObject
{
public:
    // HostData: data the stress manager supplies to the RuntimeHost.
    class HostData : public TestLibObject
    {
        friend class RuntimeHost;

        std::list<IThreadPool*> m_pools;
        StressManager *m_pManager;

        void _Cleanup();
    public:
        virtual ULONG Release();
        void AddThreadPool(IThreadPool *pool);
        static HRESULT Create(StressManager *manager, HostData** data);
    };

    // CallbackArguments: represents the Javascript arguments a host issues to a Javascript callback function.
    class CallbackArguments : public TestLibObject
    {
        friend class RuntimeHost;
        std::vector<void*> args;
    public:
        virtual ULONG Release();
        void Add(void* arg);
        static HRESULT Create(CallbackArguments **obj);
    };

    // Adds a host callback into the main Context's global object.
    HRESULT AddObject(Object object, LPCWSTR name);

    // Executes a script.
    HRESULT RunScript(LPCWSTR script);
    HRESULT RunScript(LPCWSTR script, unsigned char *byteCode);

    // Queues a callback to the specified Javascript function, with the specified arguments.
    HRESULT QueueWorkItem(Function func, CallbackArguments *arguments);

    // Sets the current context for the thread
    HRESULT SetCurrentContext();
    HRESULT ClearCurrentContext();

    // Locks/unlocks the Runtime.
    HRESULT Enter();
    HRESULT Leave();

    HRESULT WaitForCompletion();

    void SetHostObjectProvider(IHostObjectProvider *provider);
    HostData *GetHostData() { m_pHostData->AddRef(); return m_pHostData; }

    ULONG Release();
    ULONG AddRef();

    static HRESULT Create(HostData *data, IJsRuntimeProvider* provider, RuntimeHost **host);

private:

    struct CallbackData
    {
        CallbackData(Function func) : jsFunction(func) { }

        CallbackArguments *jsArgs;
        RuntimeHost *host;
        Function jsFunction;
    };

    // Processes the work item queue to completion.
    void ProcessWorkItemQueue();

    RuntimeHost();

    IJsRuntimeProvider* m_runtimeProvider;
    IJsRuntime* m_Runtime;
    IJsContext* m_Context;

    bool m_isProcessingQueue;
    std::list<CallbackData*> m_workItemQueue;
    IHostObjectProvider *m_pObjectProvider;
    IMonitor *m_pMonitor;
    HostData *m_pHostData;
    
    void _Cleanup();
};