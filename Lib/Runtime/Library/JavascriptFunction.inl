//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline FunctionProxy *JavascriptFunction::GetFunctionProxy() const
    {
        Assert(functionInfo != null);
        return functionInfo->GetFunctionProxy();
    }

    inline ParseableFunctionInfo *JavascriptFunction::GetParseableFunctionInfo() const
    {
        Assert(functionInfo != null);
        return functionInfo->GetParseableFunctionInfo();
    }

    inline DeferDeserializeFunctionInfo *JavascriptFunction::GetDeferDeserializeFunctionInfo() const
    {
        Assert(functionInfo != null);
        return functionInfo->GetDeferDeserializeFunctionInfo();
    }

    inline FunctionBody *JavascriptFunction::GetFunctionBody() const
    {
        Assert(functionInfo != null);
        return functionInfo->GetFunctionBody();
    }

    inline BOOL JavascriptFunction::IsScriptFunction() const
    {
        Assert(functionInfo != null);
        return functionInfo->HasBody();
    }

    inline bool JavascriptFunction::Is(Var aValue)
    {
        if (JavascriptOperators::GetTypeId(aValue) == TypeIds_Function)
        {
            return true;
        }
        return false;
    }

    inline JavascriptFunction* JavascriptFunction::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptFunction'");
        
        return static_cast<JavascriptFunction *>(RecyclableObject::FromVar(aValue));
    }

    inline BOOL JavascriptFunction::IsStrictMode()
    {
        FunctionProxy * proxy = this->GetFunctionProxy();
        return proxy && proxy->EnsureDeserialized()->GetIsStrictMode();
    }

    inline BOOL JavascriptFunction::IsLambda() const
    {
        return this->GetFunctionInfo()->IsLambda();
    }

    inline BOOL JavascriptFunction::IsConstructor() const
    {
        return this->GetFunctionInfo()->IsConstructor();
    }

#if DBG
    /* static */
    inline bool JavascriptFunction::IsBuiltinProperty(Var objectWithProperty, PropertyIds propertyId)
    {
        return ScriptFunction::Is(objectWithProperty)
            && (propertyId == PropertyIds::length || propertyId == PropertyIds::arguments || propertyId == PropertyIds::caller);
    }
#endif
} // namespace Js
