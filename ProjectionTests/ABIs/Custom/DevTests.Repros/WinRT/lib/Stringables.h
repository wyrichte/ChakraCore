//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#pragma warning(disable:4100)

namespace DevTests 
{
    namespace Repros
    {
        namespace Stringables
        {
            class SimpleStringable :
                public Microsoft::WRL::RuntimeClass<ISimpleStringable, Windows::Foundation::IStringable>
            {
                InspectableClass(RuntimeClass_DevTests_Repros_Stringables_SimpleStringable, BaseTrust);

            public:
                SimpleStringable()
                {
                }

                ~SimpleStringable() 
                {
                }
                
                IFACEMETHOD(ToString)(__out HSTRING* value)
                {
                    if (!value) { return E_POINTER; }

                    const wchar_t *name = RuntimeClass_DevTests_Repros_Stringables_SimpleStringable;
                    HRESULT hr = WindowsCreateString(name, static_cast<UINT32>(::wcslen(name)), value);

                    return hr;
                }
            };
        }
    }
}
