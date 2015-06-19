// Copyright (C) Microsoft. All rights reserved.
#pragma once

#include "edgescriptdirect.h"
#include <wrl.h>
#include <windows.foundation.h>

using namespace Microsoft::WRL;
using namespace Windows::Foundation;

class DelegateWrapper : public RuntimeClass<
    RuntimeClassFlags<RuntimeClassType::ClassicCom>,
    IEventDelegateWrapper>
{
public:

    static HRESULT CreateInstance(
        _In_ JsProjectionEnqueueCallback callback, 
        _In_opt_ void *callbackState, 
        __deref_out DelegateWrapper **delegateWrapperOut);

    // IDelegateWrapper methods
    // RegisterPriorityItem creates a priority delegate wrapper around the original delegate.
    STDMETHODIMP RegisterPriorityItem(
        _In_ IUnknown *originalDelegate,
        _In_ REFIID originalDelegateInterfaceID,
        _COM_Outptr_ IUnknown **priorityDelegate);

    // IEventDelegateWrapper methods
    // RegisterPriorityItem creates an event delegate wrapper around the original delegate.
    STDMETHODIMP RegisterEventItem(
        _In_ IUnknown *originalDelegateAsIUnknown,
        _In_ REFIID originalDelegateInterfaceID,
        _COM_Outptr_ IUnknown **eventDelegateAsIUnknown
    );
    
    // Constructor must be public for WRL::Make
    DelegateWrapper();

protected:
    
    // For now only support one async call.  This could be extended to be an array of contexts to support
    // multiple outstanding calls.
    AsyncStatus m_asyncStatus;
    ComPtr<IAsyncOperation<IInspectable*>> m_asyncInfo;
    ComPtr<IAsyncOperationCompletedHandler<IInspectable*>> m_asyncCompletionHandler;
    
    ComPtr<IInspectable> m_sender;
    ComPtr<IInspectable> m_args;
    ComPtr<IEventHandler<IInspectable*>> m_eventCompletionHandler;
    
    JsProjectionEnqueueCallback m_callback;
    void *m_callbackState;

    STDMETHODIMP Initialize(
        _In_ JsProjectionEnqueueCallback callback, 
        _In_opt_ void *callbackState
    );

    // The dtor is protected as the only correct method of creation is via CreateInstance.
    virtual ~DelegateWrapper();
};