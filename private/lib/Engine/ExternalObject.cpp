//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    // ExternalObject is similar to JavascriptDispatch that the scriptSite should not go
    // away when there is active ExternalObject alive. Use similar logic here to AddRef/Release
    // scriptSite
    ExternalObject::ExternalObject(ExternalType* type
#if DBG
                    , UINT byteCount
#endif
        ) :
        DynamicObject(type), finalizer(NULL), cachedJavascriptDispatch(NULL)
#if DBG_EXTRAFIELD
        , additionalByteCount(byteCount)
#endif
    {
        Assert(type->IsExternal());
    }

    BOOL ExternalObject::IsObjectAlive()
    {
        return !this->GetScriptContext()->IsClosed();
    }

    BOOL ExternalObject::VerifyObjectAlive()
    {
        ScriptContext* scriptContext = GetScriptContext();
        if (!scriptContext->VerifyAlive())
        {
            return FALSE;
        }

        // Perform an extended host object invalidation check only for external host objects.
        if (scriptContext->IsInvalidatedForHostObjects())
        {
            if (!scriptContext->GetThreadContext()->RecordImplicitException())
                return FALSE;
            Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
        }
        return TRUE;
    }

    PropertyId ExternalObject::GetNameId() const
    {
        return ((ExternalType *)this->GetType())->GetNameId();
    }

    DynamicType* ExternalObject::DuplicateType()
    {
        return RecyclerNew(this->GetScriptContext()->GetRecycler(), ExternalType,
            ((ExternalType *)this->GetDynamicType()));
    }

    // This is a special API for fastDOM to reinitialize an existing
    // Var to be used in a different script site. The example is that a
    // document is created from a site, doc.open() might cause the switch of
    // markup and lead to old site being close, but application can still
    // the same Var. We might need to initialize the Var to be used in the
    // new context.
    HRESULT ExternalObject::Reinitialize(ExternalType* newType, BOOL keepProperties)
    {
        // Save the old type before resetting the object.
        DynamicType * oldType = this->GetDynamicType();
        HRESULT hr = NOERROR;
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            ResetObject(newType, keepProperties);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);

        if (FAILED(hr))
        {
            return hr;
        }

        ScriptContext* scriptContext = newType->GetScriptContext();
        ScriptSite* requestScriptSite = ScriptSite::FromScriptContext(scriptContext);
        if (!oldType->GetScriptContext()->IsClosed())
        {
            ScriptSite* oldScriptSite = ScriptSite::FromScriptContext(oldType->GetScriptContext());
            if (oldScriptSite != requestScriptSite)
            {
                ScriptSite::DispatchMap * dispMap = oldScriptSite->GetDispatchMap();
                JavascriptDispatch* jsdisp = NULL;
                if (dispMap != NULL && dispMap->TryGetValue(this, &jsdisp))
                {
                    return jsdisp->ResetToScriptSite(requestScriptSite);
                }
            }
        }
        else
        {
            Js::CustomExternalObject * ceo = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(this);
            if (ceo)
            {
                ceo->CacheJavascriptDispatch(NULL);
            }
        }
        return NOERROR;
    }

    void ExternalObject::MarshalToScriptContext(ScriptContext* scriptContext)
    {
        Assert(Js::JavascriptConversion::IsCallable(this));
        GetDynamicType()->SetEntryPoint(Js::ExternalType::CrossSiteExternalEntryThunk);
    }
}

