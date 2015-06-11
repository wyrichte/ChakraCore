//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

namespace DevTests 
{
    namespace Repros
    {
        namespace WebUI
        {
            class WebUIActivationFactory :
                public Microsoft::WRL::ActivationFactory<IWebUIActivationStatics>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable)
                {
                    *ppInspectable = nullptr;
                    return E_NOTIMPL;
                }
 
                // IWebApplicationStatics
                virtual HRESULT STDMETHODCALLTYPE add_Activated( 
                    __RPC__in_opt IActivatedEventHandler *handler,
                    __RPC__out EventRegistrationToken *token)
                { return _evtActivated.Add(handler, token); }                        
 
                virtual HRESULT STDMETHODCALLTYPE remove_Activated( 
                    EventRegistrationToken token)
                { return _evtActivated.Remove(token); }  

                virtual HRESULT STDMETHODCALLTYPE FireActivatedEvent()
                { _evtActivated.InvokeAll(nullptr, nullptr); return S_OK; }
 
            private:
                Microsoft::WRL::EventSource<IActivatedEventHandler> _evtActivated;

            };

            class WebUIActivationServer :
                public Microsoft::WRL::RuntimeClass<IInspectable>
            {
                InspectableClass(L"DevTests.Repros.WebUI.WebUIActivation", BaseTrust);
            };
        }
    }
}