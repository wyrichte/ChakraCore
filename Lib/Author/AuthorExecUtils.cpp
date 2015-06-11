//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    HRESULT CallRootFunction(Js::JavascriptFunction * function, Js::Arguments args, Js::Var *result)
    {
        METHOD_PREFIX;

        Js::ScriptContext * scriptContext = function->GetScriptContext();
        try
        {
            *result = function->CallRootFunction(args, scriptContext);
        }
        catch(Js::JavascriptExceptionObject *)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            // Ignore this error.
        }
        catch(Js::InternalErrorException)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            hr = E_FAIL;
        }
        catch(Js::OutOfMemoryException)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            hr = E_OUTOFMEMORY;
        }
        catch(Js::NotImplementedException)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            hr = E_NOTIMPL;
        }

        METHOD_POSTFIX;
    }

    HRESULT GetPropertyOfImpl(Js::RecyclableObject* object, Js::PropertyId id, Js::ScriptContext *scriptContext, Js::Var *result)
    {
        METHOD_PREFIX;

        try
        {
            *result = Js::JavascriptOperators::GetProperty(object, id, scriptContext, NULL);
        }
        catch(Js::JavascriptExceptionObject *)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            // Ignore this error.
        }
        catch(Js::InternalErrorException)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            hr = E_FAIL;
        }
        catch(Js::OutOfMemoryException)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            hr = E_OUTOFMEMORY;
        }
        catch(Js::NotImplementedException)
        {
            *result = scriptContext->GetLibrary()->GetUndefined();
            hr = E_NOTIMPL;
        }

        METHOD_POSTFIX;

    }
    
    HRESULT GetPropertyOf(Js::RecyclableObject* object, Js::PropertyId id, Js::ScriptContext *scriptContext, Js::Var* result)
    {
        if (scriptContext->GetThreadContext()->IsScriptActive())
            return GetPropertyOfImpl(object, id, scriptContext, result);
        else
        {
            BEGIN_ENTER_SCRIPT(scriptContext, false, false, false);
            {
                return GetPropertyOfImpl(object, id, scriptContext, result);
            }
            END_ENTER_SCRIPT;
        }
    }

}