//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"
#include "ChakraEngine.h"

namespace JsStaticAPI
{
    IActiveScriptDirect* Legacy::GetChakraEngineIASD(__in ChakraEngine *chakraEngine)
    {
        HRESULT hr = E_FAIL; // init in failure mode to protect against paths which did not explicitly succeed
        IActiveScriptDirect *pIASD = chakraEngine->GetActiveScriptDirect(&hr);
        if (!pIASD)
        {
            FATAL_ON_FAILED_API_RESULT(hr);
        }

        return pIASD;
    }
}
