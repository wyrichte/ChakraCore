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
        class CookieServer :
            public Microsoft::WRL::RuntimeClass<
                Fabrikam::Kitchen::ICookie>
        {
            InspectableClass(RuntimeClass_Fabrikam_Kitchen_Cookie, BaseTrust);

        public:

            // DevX & UEX Coding standards require Hungarian notation. WinRT
            // API guidelines prohibit it. This is because the hungarian type
            // prefixes may be misleading when mapped into different languages. 
            // The way to resolve this is to specify non-prefixed names in the 
            // ReXML or IDL files, but use hungarian parameter names in your 
            // server implementation files.

            CookieServer() : 
                _doneness(Fabrikam::Kitchen::CookieDoneness_Raw)
            {}

            STDMETHOD(put_Doneness)(
                __in CookieDoneness doneness);

            // ICookie::get_Doneness
            IFACEMETHOD(get_Doneness)(
                __out CookieDoneness *pDoneness) override;

        private:

            CookieDoneness _doneness;
        };
    }
}
