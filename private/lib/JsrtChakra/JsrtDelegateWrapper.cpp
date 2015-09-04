// Copyright (c) Microsoft Corporation.  All rights reserved.

#include "JsrtChakraPch.h"
#include "JsrtDelegateWrapper.h"


STDMETHODIMP CheckApartment()
{
    HRESULT hr = S_OK;
    APTTYPE aptType = {};
    APTTYPEQUALIFIER aptTypeQualifier = {};

    IfFailedReturn(CoGetApartmentType(&aptType, &aptTypeQualifier));

    // We don't support running on STA, in order to support this we would need to
    // put the COM pointers into the GlobalInterfaceTable before we pass the 
    // context to the caller.  This is not necessary for most scenarios.  It is only
    // useful when the caller wants to post messages to the COM thread at different
    // priorities than default, WWA does this with it's delegate wrapper implementation.
    
    if (aptType != APTTYPE_MTA)
    {
        // We are not running in an implicit (uninitialized) or explicit MTA thread.
        return RPC_E_WRONG_THREAD;
    }

    return S_OK;
}

// RegisterPriorityItem is called with originalDelegate set to the completion handler for a WinRT async call.  
// This means that originalDelegate is either IAsyncOperationCompletedHandler<resultType> or   
// IAsyncOperationWithProgressCompletedHandler<resultType, progressType> where resultType and progressType   
// are unknown. Because it is a templated interface we don't know the IID and can't QI for it, but the vtable for them  
// is the same as far as we're concerned.  //  // They both have (beyond the normal QI, AddRef, and Release methods) only one additional Invoke method:  
//  virtual HRESULT STDMETHODCALLTYPE Invoke(  
//      __in IAsyncOperation<resultType> *asyncInfo,   
//      __in Windows::Foundation::AsyncStatus status);  
// or  
//  virtual HRESULT STDMETHODCALLTYPE Invoke(  
//      __in IAsyncOperationWithProgress<resultType, progressType> *asyncInfo,   
//      __in Windows::Foundation::AsyncStatus status);  
//  
// Although we don't know the size or type of resultType or progressType, it doesn't matter, because we just need to   
// preserve the pointer value for when we call the real Invoke call on the other side.  

typedef RuntimeClass <
                RuntimeClassFlags<RuntimeClassType::ClassicCom>,
                IAsyncOperationCompletedHandler<IInspectable*>,
                IAgileObject,
                FtmBase >
            AsyncCompletionWrapperParent;

// AsyncCompletionWrapper wraps the delegate Invoke call of an async completion.  Upon
// an invoke it posts a callback to the provided JsEnqueueProjectionCallback and
// does a direct invoke on the original delegate when the caller calls our
// JsProjectionCallback on the proper thread.
class AsyncCompletionWrapper : public AsyncCompletionWrapperParent
{
public:
    AsyncCompletionWrapper() :
        m_originalCompletedHandlerIid(GUID_NULL),
        m_thread(0),
        m_callback(nullptr),
        m_callbackState(nullptr)
    {
    }

    STDMETHODIMP Initialize(
        _In_ JsProjectionEnqueueCallback callback,
        _In_opt_ void *callbackState,
        _In_ IUnknown *originalCompletedHandlerUnknown,
        _In_ GUID originalCompletedHandlerIid
    )
    {
        HRESULT hr = S_OK;
        ComPtr<IUnknown> spOriginalCompletedHandlerUnknown = originalCompletedHandlerUnknown;
        ComPtr<IAsyncOperationCompletedHandler<IInspectable*>> spOriginalCompletedHandler;

        IfFailedReturn(spOriginalCompletedHandlerUnknown.CopyTo(
                                originalCompletedHandlerIid,
                                &spOriginalCompletedHandler));    
                
        m_thread = GetCurrentThreadId();
        m_callback = callback;
        m_callbackState = callbackState;
        m_originalCompletedHandler = spOriginalCompletedHandler;
        m_originalCompletedHandlerIid = originalCompletedHandlerIid;

        return S_OK;
    }

    // IUnknown methods (the rest are implemented by parent helper class)      
    virtual STDMETHODIMP QueryInterface(
        _In_ REFIID riid,
        _COM_Outptr_ void **ppvObject)
    {
        IID qiid = riid;

        // The original delegate interface ID is a kind of IAsyncOperationCompletedHandler<T>.          
        // Since we don't need to know the specifics pretend like the QI was for our          
        // kind of IAsyncOperationCompletedHandler<IInspectable*>.
        if (riid == m_originalCompletedHandlerIid)
        {
            qiid = __uuidof(IAsyncOperationCompletedHandler<IInspectable*>);
        }

        // Otherwise delegate to the parent helper class to do the actual QI work.
        return AsyncCompletionWrapperParent::QueryInterface(qiid, ppvObject);
    }
        
    // IAsyncOperationCompletedHandler methods
    virtual STDMETHODIMP Invoke(
        _In_opt_ IAsyncOperation<IInspectable*> *asyncInfo,
        _In_ AsyncStatus asyncStatus)
    {
        if (m_thread == GetCurrentThreadId()) 
        {
            // Already on the originating thread, simply invoke.
            m_originalCompletedHandler->Invoke(asyncInfo, asyncStatus);
        }
        else 
        {
            AsyncCompletionContext* prepareAsyncCompletionContext = new AsyncCompletionContext();
            
            IfNullReturnError(prepareAsyncCompletionContext, E_OUTOFMEMORY);

            prepareAsyncCompletionContext->m_originalCompletedHandler = m_originalCompletedHandler;
            prepareAsyncCompletionContext->m_asyncInfo = asyncInfo;
            prepareAsyncCompletionContext->m_asyncStatus = asyncStatus;
            prepareAsyncCompletionContext->m_thread = m_thread;

            // this lambda represents a JsProjectionCallback, give it to the caller to invoke
            // on the correct thread.
            m_callback([](JsProjectionCallbackContext context)
                        {
                            AsyncCompletionContext* runAsyncCompletionContext = (AsyncCompletionContext*)context;
                            
                            Assert(runAsyncCompletionContext->m_thread == GetCurrentThreadId());

                            // invoke the original delegate now that we are on the correct thread.
                            runAsyncCompletionContext->m_originalCompletedHandler.Get()->Invoke(
                                    runAsyncCompletionContext->m_asyncInfo.Get(),
                                    runAsyncCompletionContext->m_asyncStatus
                                );

                            delete runAsyncCompletionContext;
                            runAsyncCompletionContext = nullptr;
                        },                        
                        prepareAsyncCompletionContext,
                        m_callbackState);
        }

        return S_OK;
    }

    
protected:

    typedef struct _AsyncCompletionContext 
    {
        ComPtr<IAsyncOperationCompletedHandler<IInspectable*>> m_originalCompletedHandler;
        ComPtr<IAsyncOperation<IInspectable*>> m_asyncInfo;
        AsyncStatus m_asyncStatus;
        DWORD m_thread;
    } AsyncCompletionContext;

    // Ctor and dtor should only be called by WRL helper. Regular callers use AddRef and Release.      
    virtual ~AsyncCompletionWrapper()
    {
    }

private:

    ComPtr<IAsyncOperationCompletedHandler<IInspectable*>> m_originalCompletedHandler;
    
    GUID m_originalCompletedHandlerIid;

    DWORD m_thread;

    JsProjectionEnqueueCallback m_callback;
    void* m_callbackState;

    // Disable default copy ctor and operator=      
    AsyncCompletionWrapper(const AsyncCompletionWrapper&);  
    const AsyncCompletionWrapper& operator=(const AsyncCompletionWrapper&);
};

// RegisterEventItem is called with originalDelegate set to the event handler for a WinRT event invocation.  
// This means that originalDelegate is either IEventHandler<T> ITypedEventHandler<U, T> where the types are unkown. 
// Because it is a templated interface we don't know the IID and can't QI for it, but the vtable for them  
// is the same as far as we're concerned.  
// They both have (beyond the normal QI, AddRef, and Release methods) only one additional Invoke method:  
// 
// virtual HRESULT STDMETHODCALLTYPE Invoke( 
//      _In_ IInspectable *sender, 
//      _In_ T_abi args)
// or  
// virtual HRESULT STDMETHODCALLTYPE Invoke(
//      _In_ TSender_abi sender, 
//      _In_ TArgs_abi args) 
//
// Although we don't know the size or type of sender and args, it doesn't matter, because we just need to   
// preserve the pointer value for when we call the real Invoke call on the other side.  

typedef RuntimeClass <
                RuntimeClassFlags<RuntimeClassType::ClassicCom>,
                Windows::Foundation::IEventHandler<IInspectable*>,
                IAgileObject,
                FtmBase >
            EventHandlerWrapperParent;

// EventHandlerWrapper wraps the delegate Invoke call of an Event raise.  Upon
// an invoke it posts a callback to the provided JsEnqueueProjectionCallback and
// does a direct invoke on the original delegate when the caller calls our
// JsProjectionCallback on the proper thread.
class EventHandlerWrapper : public EventHandlerWrapperParent
{
public:
    EventHandlerWrapper() :
        m_originalEventHandlerIid(GUID_NULL),
        m_thread(0),
        m_callback(nullptr),
        m_callbackState(nullptr)
    {
    }
    
    STDMETHODIMP Initialize(
        _In_ JsProjectionEnqueueCallback callback,
        _In_opt_ void *callbackState,
        _In_ IUnknown *originalEventHandlerUnknown,
        _In_ GUID originalEventHandlerIid
    )
    {
        HRESULT hr = S_OK;
        ComPtr<IUnknown> spOriginalEventHandlerUnknown = originalEventHandlerUnknown;
        ComPtr<IEventHandler<IInspectable*>> spOriginalEventHandler;

        IfFailedReturn(spOriginalEventHandlerUnknown.CopyTo(
                                originalEventHandlerIid,
                                &spOriginalEventHandler));    

        m_thread = GetCurrentThreadId();
        m_callback = callback;
        m_callbackState = callbackState;
        m_originalEventHandler = spOriginalEventHandler;
        m_originalEventHandlerIid = originalEventHandlerIid;

        return S_OK;
    }

    // IUnknown methods (the rest are implemented by parent helper class)      
    virtual STDMETHODIMP QueryInterface(
        _In_ REFIID riid,
        _COM_Outptr_ void **ppvObject)
    {
        IID qiid = riid;

        // The original delegate interface ID is a kind of IEventHandler<T>.          
        // Since we don't need to know the specifics pretend like the QI was for our          
        // kind of IAsyncOperationCompletedHandler<IInspectable*>.
        if (riid == m_originalEventHandlerIid)
        {
            qiid = __uuidof(IEventHandler<IInspectable*>);
        }

        // Otherwise delegate to the parent helper class to do the actual QI work.          
        return EventHandlerWrapperParent::QueryInterface(qiid, ppvObject);
    }
        
    // IEventHandler methods.
    virtual STDMETHODIMP Invoke(
        _In_ IInspectable *sender,
        _In_ IInspectable *args
    )
    {
        if (m_thread == GetCurrentThreadId()) 
        {
            // Already on the originating thread, simply invoke.
            m_originalEventHandler->Invoke(sender, args);
        }
        else 
        {
            EventHandlerContext* prepareEventHandlerContext = new EventHandlerContext();
            
            IfNullReturnError(prepareEventHandlerContext, E_OUTOFMEMORY);

            prepareEventHandlerContext->m_originalEventHandler = m_originalEventHandler;
            prepareEventHandlerContext->m_sender = sender;
            prepareEventHandlerContext->m_args = args;
            prepareEventHandlerContext->m_thread = m_thread;

            // this lambda represents a JsProjectionCallback, give it to the caller to invoke
            // on the correct thread.
            m_callback([](JsProjectionCallbackContext context)
                        {
                            EventHandlerContext* runEventHandlerContext = (EventHandlerContext*)context;

                            Assert(runEventHandlerContext->m_thread == GetCurrentThreadId());

                            // invoke the original delegate now that we are on the correct thread.
                            runEventHandlerContext->m_originalEventHandler.Get()->Invoke(
                                    runEventHandlerContext->m_sender.Get(),
                                    runEventHandlerContext->m_args.Get()
                                );

                            delete runEventHandlerContext;
                            runEventHandlerContext = nullptr;
                        },                        
                        prepareEventHandlerContext,
                        m_callbackState);
        }

        return S_OK;
    }

protected:

    typedef struct _EventHandlerContext 
    {
        ComPtr<IEventHandler<IInspectable*>> m_originalEventHandler;
        ComPtr<IInspectable> m_sender;
        ComPtr<IInspectable> m_args;
        DWORD m_thread;
    } EventHandlerContext;

    // Ctor and dtor should only be called by WRL helper. Regular callers use AddRef and Release.      
    virtual ~EventHandlerWrapper()
    {
    }

private:

    ComPtr<IEventHandler<IInspectable*>> m_originalEventHandler;
    GUID m_originalEventHandlerIid;

    ComPtr<DelegateWrapper> m_delegateWrapper;

    DWORD m_thread;

    JsProjectionEnqueueCallback m_callback;
    void* m_callbackState;

    // Disable default copy ctor and operator=      
    EventHandlerWrapper(const EventHandlerWrapper&);  
    const EventHandlerWrapper& operator=(const EventHandlerWrapper&);
};

// RegisterPriorityItem wraps a delegate for WinRT async completions in an object
// which allows the Jsrt API caller to post the invocation to the correct thread before
// calling invoke.
HRESULT DelegateWrapper::RegisterPriorityItem(
    _In_ IUnknown *originalDelegateAsIUnknown,
    _In_ REFIID originalDelegateInterfaceID,
    _COM_Outptr_ IUnknown **priorityDelegateAsIUnknown)
{
    HRESULT hr = S_OK;
    *priorityDelegateAsIUnknown = nullptr;
    
    auto priorityDelegate = Make<AsyncCompletionWrapper>();
    IfNullReturnError(priorityDelegate, E_OUTOFMEMORY);

    IfFailedReturn(priorityDelegate->Initialize(m_callback, m_callbackState, originalDelegateAsIUnknown, originalDelegateInterfaceID));

    IfFailedReturn(priorityDelegate.CopyTo<IUnknown>(priorityDelegateAsIUnknown));   

    return S_OK;
}

// RegisterEventItem wraps a delegate for WinRT event firing in an object
// which allows the Jsrt API caller to post the invocation to the correct thread before
// calling invoke.
HRESULT DelegateWrapper::RegisterEventItem(
    _In_ IUnknown *originalDelegateAsIUnknown,
    _In_ REFIID originalDelegateInterfaceID,
    _COM_Outptr_ IUnknown **eventDelegateAsIUnknown)
{
    HRESULT hr = S_OK;
    *eventDelegateAsIUnknown = nullptr;
    
    auto eventDelegate = Make<EventHandlerWrapper>();
    IfNullReturnError(eventDelegate, E_OUTOFMEMORY);

    IfFailedReturn(eventDelegate->Initialize(m_callback, m_callbackState, originalDelegateAsIUnknown, originalDelegateInterfaceID));

    IfFailedReturn(eventDelegate.CopyTo<IUnknown>(eventDelegateAsIUnknown));

    return S_OK;
}

HRESULT DelegateWrapper::CreateInstance(
    _In_ JsProjectionEnqueueCallback callback,
    _In_opt_ void *callbackState,
    _Outptr_ DelegateWrapper **delegateWrapperOut)
{
    HRESULT hr = S_OK;
        
    IfNullReturnError(callback, E_INVALIDARG);
    IfNullReturnError(delegateWrapperOut, E_INVALIDARG);

    *delegateWrapperOut = nullptr;

    auto delegateWrapper = Make<DelegateWrapper>();
    IfNullReturnError(delegateWrapper, E_OUTOFMEMORY);
    IfFailedReturn(delegateWrapper->Initialize(callback, callbackState));

    *delegateWrapperOut = delegateWrapper.Detach();
    return S_OK;
}

DelegateWrapper::DelegateWrapper()
{
    m_callback = nullptr;
    m_callbackState = nullptr;
}

HRESULT DelegateWrapper::Initialize(
    _In_ JsProjectionEnqueueCallback callback, 
    _In_opt_ void *callbackState)
{
    HRESULT hr = S_OK;

    IfFailedReturn(CheckApartment());

    m_callback = callback;
    m_callbackState = callbackState;

    return S_OK;
}

DelegateWrapper::~DelegateWrapper()
{
    m_callback = nullptr;
    m_callbackState = nullptr;
}