//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    CustomEnumerator::CustomEnumerator(ScriptContext* scriptContext, IVarEnumerator* inVarEnumerator, CustomExternalObject* customObject):
        JavascriptEnumerator(scriptContext),
        customObject(customObject)
    {
        varEnumerator = inVarEnumerator;
        varEnumerator->AddRef();
    }

    void CustomEnumerator::Dispose(bool isShutdown)
    {
        if (varEnumerator)
        {
            if (!isShutdown)
            {
                varEnumerator->Release();
                varEnumerator = NULL;
            }
            else
            {
                LEAK_REPORT_PRINT(_u("CustomEnumerator %p: Finalize not called on shutdown (IVarEnumerator %p)\n"),
                    this, varEnumerator);
            }
        }
    }

    JavascriptString* CustomEnumerator::MoveAndGetNext(Js::PropertyId& propertyId, Js::PropertyAttributes* attributes)    
    {
        BOOL itemsAvailable = FALSE;
        ScriptContext* scriptContext = GetScriptContext();
        propertyId = Js::Constants::NoProperty;
        if (varEnumerator == nullptr)
        {
            return nullptr;
        }

        if (attributes != nullptr)
        {
            *attributes = PropertyNone;
        }

        HRESULT hr = E_FAIL;
        Var currentName = NULL;
        ::PropertyAttributes externalAttributes = PropertyAttributes_Enumerable;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            hr = varEnumerator->MoveNext(&itemsAvailable, &externalAttributes);
            if (SUCCEEDED(hr) && itemsAvailable)
            {
                hr = varEnumerator->GetCurrentName(&currentName);
            }
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)

        if (attributes != nullptr)
        {
            *attributes = (Js::PropertyAttributes)externalAttributes;
        }
        if (SUCCEEDED(hr) && currentName)
        {
            return (JavascriptString*)CrossSite::MarshalVar(GetScriptContext(), currentName);
        }
        return nullptr;
    }

    void CustomEnumerator::Reset()
    {
        Dispose(/*isShutdown*/ false);

        ScriptContext* scriptContext = GetScriptContext();
        HRESULT hr = E_FAIL;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            hr = customObject->GetTypeOperations()->GetEnumerator(scriptContext->GetActiveScriptDirect(), customObject, FALSE, FALSE, &varEnumerator);
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        AssertMsg(SUCCEEDED(hr), "Getting enumerator failed");
    }
};
