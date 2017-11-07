//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "Fabrikam.h"
#include "gitevent.h"   // this is in $(INTERNAL_SDK_INC_PATH)\winrt; \

using namespace Microsoft::WRL;

namespace Fabrikam
{
    namespace Kitchen
    {
        class ApplianceElectricityConsumptionReporter :
            public Microsoft::WRL::RuntimeClass<Fabrikam::Kitchen::IApplianceElectricityConsumptionReporter>
        {
            InspectableClass(L"Fabrikam.Kitchen.IApplianceElectricityConsumptionReporter", BaseTrust);

        private:
            HSTRING _applianceName;
            Microsoft::WRL::EventSource<IApplianceSwitchOnHandler> _evtApplianceSwitchOnEvent;
            Microsoft::WRL::EventSource<IApplianceSwitchOffHandler> _evtApplianceSwitchOffEvent;
            
        public:
            ApplianceElectricityConsumptionReporter(HSTRING applianceName);
            ~ApplianceElectricityConsumptionReporter();

            IFACEMETHOD(get_ApplianceName)( 
                __out HSTRING *name) override;

            IFACEMETHOD(add_ApplianceSwitchOnEvent)( 
                __in Fabrikam::Kitchen::IApplianceSwitchOnHandler * switchOnHandler,
                __out EventRegistrationToken *pCookie) override;
                
            IFACEMETHOD(remove_ApplianceSwitchOnEvent)( 
                __in EventRegistrationToken iCookie) override;
                
            IFACEMETHOD(add_ApplianceSwitchOffEvent)( 
                __in Fabrikam::Kitchen::IApplianceSwitchOffHandler * switchOffHandler,
                __out EventRegistrationToken *pCookie) override;
                
            IFACEMETHOD(remove_ApplianceSwitchOffEvent)( 
                __in EventRegistrationToken iCookie) override;

            IFACEMETHOD(GetSameElectricityReporter)(
                __out IApplianceElectricityConsumptionReporter **outElectricityReporter) override
            {
                return this->QueryInterface(IID_IApplianceElectricityConsumptionReporter, (void **)outElectricityReporter);
            }

            void InvokeSwitchOn(HSTRING msg);
            void InvokeSwitchOff(HSTRING msg, UINT32 unitCount);


        };

        // DO NOT give your server class the same namespace qualified name
        // or non-qualified name as the runtime class that it's intended
        // to implement. This will mitigate against possible naming conflicts
        // with the future C++ consumption model.
        class ToasterServer :
            public Microsoft::WRL::RuntimeClass<
                Fabrikam::Kitchen::IToaster, 
                Fabrikam::Kitchen::IAppliance>
        {
            InspectableClass(RuntimeClass_Fabrikam_Kitchen_Toaster, BaseTrust);

        public:
            ToasterServer();
            ~ToasterServer();

            // DevX & UEX Coding standards require Hungarian notation. WinRT
            // API guidelines prohibit it. This is because the hungarian type
            // prefixes may be misleading when mapped into different languages. 
            // The way to resolve this is to specify non-prefixed names in the 
            // ReXML or IDL files, but use hungarian parameter names in your 
            // server implementation files.

            // IAppliance::get_Size
            IFACEMETHOD(get_Size)(__out Dimensions *pdims) override;
                
            // IAppliance::put_Size
            IFACEMETHOD(put_Size)(Dimensions dims) override;

            // IAppliance::get_ElectricityReporter
            IFACEMETHOD(get_ElectricityReporter)(
                __out IApplianceElectricityConsumptionReporter **electricityReporter) override;

            // IToaster::Toast
            IFACEMETHOD(MakeToast)(
                __in  HSTRING hstrMessage, 
                __deref_out Fabrikam::Kitchen::IToast **ppToast) override;

            IFACEMETHOD(add_ToastCompleteEvent)( 
                __in Fabrikam::Kitchen::IToastCompleteHandler *clickHandler,
                __out EventRegistrationToken *pCookie) override;
                
            IFACEMETHOD(remove_ToastCompleteEvent)( 
                __in EventRegistrationToken iCookie) override;
                
            IFACEMETHOD(add_ToastStartEvent)( 
                __in Fabrikam::Kitchen::IToastStartHandler *clickHandler,
                __out EventRegistrationToken *pCookie) override;
                
            IFACEMETHOD(remove_ToastStartEvent)( 
                __in EventRegistrationToken iCookie) override;

            IFACEMETHOD(PreheatInBackground)(
                __in Fabrikam::Kitchen::IToastCompleteHandler* onPreheatComplete) override;

            IFACEMETHOD(add_PreheatCompleteBackground)( 
                __in Fabrikam::Kitchen::IToastCompleteHandler *inHandler,
                __out EventRegistrationToken *pCookie) override;

            IFACEMETHOD(remove_PreheatCompleteBackground)( 
                __in EventRegistrationToken iCookie) override;

            IFACEMETHOD(InvokePreheatCompleteBackgroundEvents)(__in Fabrikam::Kitchen::IToaster *sender) override;

            IFACEMETHOD(PreheatInBackgroundWithSmuggledDelegate)(
                __in Fabrikam::Kitchen::IToastCompleteHandler* onPreheatComplete) override;

            IFACEMETHOD(GetSameToaster)(
                __out Fabrikam::Kitchen::IToaster** outToaster) override
            {
                return this->QueryInterface(IID_IToaster, (void **)outToaster);
            }

            IFACEMETHOD(add_PreheatStart)( 
                __in Fabrikam::Kitchen::IToasterPreheatStartHandler *inHandler,
                __out EventRegistrationToken *pCookie) override;

            IFACEMETHOD(remove_PreheatStart)( 
                __in EventRegistrationToken iCookie) override;

            IFACEMETHOD(add_IndirectToastCompleteEvent)(
                __in IToastCompleteHandler * clickHandler,
                __out EventRegistrationToken * eventCookie) override;

            IFACEMETHOD(remove_IndirectToastCompleteEvent)(__in EventRegistrationToken eventCookie) override;

            IFACEMETHOD(get_IndirectToaster)(__out IToaster **toaster) override;
            IFACEMETHOD(put_IndirectToaster)(__in IToaster *toaster) override;

            IFACEMETHOD(IndirectMakeToast)(__in HSTRING message, __out IToast** toast) override;

            IFACEMETHOD(add_RootedToastCompleteEvent)(
                __in IToastCompleteHandler * clickHandler,
                __out EventRegistrationToken * eventCookie) override;

            IFACEMETHOD(remove_RootedToastCompleteEvent)(__in EventRegistrationToken eventCookie) override;

            IFACEMETHOD(get_RootedHandler)(__out IToastCompleteHandler ** clickHandler) override;
            IFACEMETHOD(put_RootedHandler)(__in IToastCompleteHandler * clickHandler) override;
            IFACEMETHOD(InvokeRootedHandler)(__in IToaster *sender, __in IToast *toast) override;

        private:
            ApplianceElectricityConsumptionReporter *_electricityReporter;
            Microsoft::WRL::EventSource<IToastCompleteHandler> _evtToastComplete;
            Microsoft::WRL::EventSource<IToastStartHandler> _evtToastStart;
            Microsoft::WRL::EventSource<IToasterPreheatStartHandler> _evtToasterPreheatStart;
            // cannot use regular WRL event source becase the delegate is added on the ASTA thread and 
            // invoked from the background thread, must hold the delegates in the GIT so the correct
            // localization happens when Invokd from the background thread (i.e. the background thread
            // invokes via a proxy to the delegate
            //Microsoft::WRL::EventSource<IToastCompleteHandler> _evtToasterPreheatBackground;
            Windows::Internal::GitEventSource<IToastCompleteHandler> _evtToasterPreheatBackground;

            Dimensions _dims;
            IToaster * _indirectToaster;
            IToastCompleteHandler *_rootedHandler;
        };

        // The activatable class macro must be defined *inside* the same namespace
        // as the class that it's describing, and the server name must be specified
        // as the unqualified name. This is a limitation of the macro mechanics used
        // to make the server activatable.
        // ActivatableClass(Toaster)
    }
}