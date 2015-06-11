//  Copyright (c) Microsoft Corporation. All rights reserved.

// ChefServer.h

#include <stdafx.h>
#include <windowsstringp.h>
#include <wrl\implements.h>
#include "Fabrikam.h" // Generated from IDL/by RexPP.  

namespace Fabrikam
{
    namespace Kitchen
    {
        class ChefServer:
            public Microsoft::WRL::RuntimeClass<IChef>
        {
            InspectableClass(RuntimeClass_Fabrikam_Kitchen_Chef, BaseTrust);

        public:
            IFACEMETHOD(get_Name)(__out HSTRING *phstrName);
            
            // Initialize is not projected.  
            STDMETHOD(Initialize)(__in HSTRING name, __in IKitchen *pKitchen);

            IFACEMETHOD(MakeBreakfastToaster)(__in IToaster *pToaster, __out int *cost);

            IFACEMETHOD(MakeBreakfastToasterInt)(__in IToaster *pToaster, 
                __in int howMany,
                __out int *cost);

            IFACEMETHOD(MakeBreakfastToasterDouble)(__in IToaster *pToaster,
                __in double howMany,
                __out int *cost);

            IFACEMETHOD(get_Role)(__out ChefRole* value);

            IFACEMETHOD(put_Role)(__in ChefRole value);

            IFACEMETHOD(get_Capabilities)(__out ChefCapabilities* value);

            IFACEMETHOD(put_Capabilities)(__in ChefCapabilities value);

            IFACEMETHOD(add_MultipleToastCompleteCollection)( 
                __in Fabrikam::Kitchen::IMultipleToastCompleteCollectionHandler *inHandler,
                __out EventRegistrationToken *pCookie) override;

            IFACEMETHOD(remove_MultipleToastCompleteCollection)( 
                __in EventRegistrationToken iCookie) override;

            IFACEMETHOD(add_MultipleToastCompleteArray)( 
                __in Fabrikam::Kitchen::IMultipleToastCompleteArrayHandler *inHandler,
                __out EventRegistrationToken *pCookie) override;

            IFACEMETHOD(remove_MultipleToastCompleteArray)( 
                __in EventRegistrationToken iCookie) override;

            IFACEMETHOD(add_MakeToastRoundOff)( 
                __in Fabrikam::Kitchen::IMakeToastRoundOffHandler *inHandler,
                __out EventRegistrationToken *pCookie) override;

            IFACEMETHOD(remove_MakeToastRoundOff)( 
                __in EventRegistrationToken iCookie) override;

        private:
            Microsoft::WRL::ComPtr<IKitchen> _spKitchen;
            Microsoft::WRL::ComPtr<IPricingModel> _spExtension;
            Windows::Internal::String _strName;
            ChefCapabilities _capabilities;
            ChefRole _role;
            Microsoft::WRL::EventSource<IMultipleToastCompleteCollectionHandler> _evtCollectionToastComplete;
            Microsoft::WRL::EventSource<IMultipleToastCompleteArrayHandler> _evtArrayToastComplete;
            bool fCheckRoundOff;
            Microsoft::WRL::EventSource<IMakeToastRoundOffHandler> _evtBreakFastToasterRoundOff;
        };

        class ChefFactory :
            public Microsoft::WRL::ActivationFactory<IChefFactory>
        {
        public:
            IFACEMETHOD(CreateChef)(__in HSTRING name, 
                __in IKitchen *pKitchen, 
                __deref_out IChef ** ppChef);

            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
        };
    }
}