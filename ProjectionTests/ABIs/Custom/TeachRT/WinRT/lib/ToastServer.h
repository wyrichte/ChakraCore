//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "Fabrikam.h"

namespace Fabrikam
{
    namespace Kitchen
    {
        // DO NOT give your server class the same namespace qualified name
        // or non-qualified name as the runtime class that it's intended
        // to implement. This will mitigate against possible naming conflicts
        // with the future C++ consumption model.
        class ToastServer :
            public Microsoft::WRL::RuntimeClass<Fabrikam::Kitchen::IToast>
        {
            InspectableClass(L"Fabrikam.Kitchen.Toast", TrustLevel::BaseTrust)
        public:

            // non-ABI method for setting up private state of Toast
            HRESULT PrivateInitialize(HSTRING hstrMessage);

            // DevX & UEX Coding standards require Hungarian notation. WinRT
            // API guidelines prohibit it. This is because the hungarian type
            // prefixes may be misleading when mapped into different languages. 
            // The way to resolve this is to specify non-prefixed names in the 
            // ReXML or IDL files, but use hungarian parameter names in your 
            // server implementation files.

            // IToast::Message property (read)
            IFACEMETHOD(get_Message)(__out HSTRING *phstrMessage) override;
                
        private:

            Windows::Internal::String _hstrMessage;
        };

        // This class doesn't use the ActivatableClass macro because the only way to 
        // acquire one is via a method call on other objects. Only define the macro if
        // the class should be directly instantiable via ActivateInstance.
        // ActivatableClass(Toaster)
    }
}
