//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include <windowsstringp.h>
#include <wrl\implements.h>
#include "Fabrikam.h"

namespace Fabrikam
{
    namespace Kitchen
    {
        class KitchenServer :
            public ::Microsoft::WRL::RuntimeClass<IKitchen>
        {
            InspectableClass(RuntimeClass_Fabrikam_Kitchen_Kitchen, BaseTrust);
        public:
            // Initialize is not projected.  
            STDMETHOD(Initialize)();

        private:
        };

        class KitchenFactory :
            public ::Microsoft::WRL::ActivationFactory<>
        {
        public:
            IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable);
        };
    }
}