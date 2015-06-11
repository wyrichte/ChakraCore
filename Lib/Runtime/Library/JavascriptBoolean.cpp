//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{    
    Var JavascriptBoolean::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");    

        // SkipDefaultNewObject function flag should have revent the default object
        // being created, except when call true a host dispatch
        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == null
            || JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch);

        BOOL value;

        if (args.Info.Count > 1)
        {
            value = JavascriptConversion::ToBoolean(args[1], scriptContext) ? true : false;
        }
        else
        {
            value = false;
        }

        if (!(callInfo.Flags & CallFlags_New))
        {
            return scriptContext->GetLibrary()->CreateBoolean(value);
        }

        return scriptContext->GetLibrary()->CreateBooleanObject(value);
    }

    // Boolean.prototype.valueOf as described in ES6 spec (draft 24) 19.3.3.3
    Var JavascriptBoolean::EntryValueOf(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if(JavascriptBoolean::Is(args[0]))
        {
            return args[0];
        }
        else if (JavascriptBooleanObject::Is(args[0]))
        {
            JavascriptBooleanObject* booleanObject = JavascriptBooleanObject::FromVar(args[0]);
            return scriptContext->GetLibrary()->CreateBoolean(booleanObject->GetValue());
        }
        else
        {
            return TryInvokeRemotelyOrThrow(EntryValueOf, scriptContext, args, JSERR_This_NeedBoolean, L"Boolean.prototype.valueOf");
        }
    }

    // Boolean.prototype.toString as described in ES6 spec (draft 24) 19.3.3.2
    Var JavascriptBoolean::EntryToString(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        AssertMsg(args.Info.Count, "Should always have implicit 'this'.");
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        BOOL bval;
        Var aValue = args[0];
        if(JavascriptBoolean::Is(aValue))
        {
            bval = JavascriptBoolean::FromVar(aValue)->GetValue();
        }
        else if (JavascriptBooleanObject::Is(aValue))
        {
            JavascriptBooleanObject* booleanObject = JavascriptBooleanObject::FromVar(aValue);
            bval = booleanObject->GetValue();
        }
        else
        {
            return TryInvokeRemotelyOrThrow(EntryToString, scriptContext, args, JSERR_This_NeedBoolean, L"Boolean.prototype.toString");
        }
        
        return bval ? scriptContext->GetLibrary()->GetTrueDisplayString() : scriptContext->GetLibrary()->GetFalseDisplayString();
    }

    RecyclableObject * JavascriptBoolean::CloneToScriptContext(ScriptContext* requestContext)
    {
        if (this->GetValue())
        {
            return requestContext->GetLibrary()->GetTrue();
        }
        return requestContext->GetLibrary()->GetFalse();
    }

    Var JavascriptBoolean::TryInvokeRemotelyOrThrow(JavascriptMethod entryPoint, ScriptContext * scriptContext, Arguments & args, long errorCode, PCWSTR varName)
    {
        if (JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch)
        {
            Var result;
            if (RecyclableObject::FromVar(args[0])->InvokeBuiltInOperationRemotely(entryPoint, args, &result))
            {
                return result;
            }
        }
        // Don't error if we disabled implicit calls
        if(scriptContext->GetThreadContext()->RecordImplicitException())
        {
            JavascriptError::ThrowTypeError(scriptContext, errorCode, varName);
        }
        else
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }
} // namespace Js
