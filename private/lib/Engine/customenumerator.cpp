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

    Var CustomEnumerator::GetCurrentIndex()
    {
        Var currentName = NULL;
        ScriptContext* scriptContext = GetScriptContext();
        if (varEnumerator == nullptr)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        HRESULT hr = E_FAIL;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            hr = varEnumerator->GetCurrentName(&currentName);
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext);

        if (SUCCEEDED(hr))
        {
            return CrossSite::MarshalVar(GetScriptContext(), currentName);
        }
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var CustomEnumerator::GetCurrentValue()
    {
        return GetScriptContext()->GetLibrary()->GetUndefined();
    }

    BOOL CustomEnumerator::MoveNext(Js::PropertyAttributes* attributes)
    {
        BOOL itemsAvailable = FALSE;
        ScriptContext* scriptContext = GetScriptContext();
        if (varEnumerator == nullptr)
        {
            return FALSE;
        }

        if (attributes != nullptr)
        {
            *attributes = PropertyEnumerable;
        }

        HRESULT hr = E_FAIL;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            ::PropertyAttributes externalAttributes = PropertyAttributes_Enumerable;
            hr = varEnumerator->MoveNext(&itemsAvailable, &externalAttributes);
            if (attributes != nullptr && externalAttributes & PropertyAttributes_Enumerable)
            {
                *attributes = PropertyEnumerable;
            }
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        if (SUCCEEDED(hr))
        {
            return itemsAvailable;
        }
        return FALSE;
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
