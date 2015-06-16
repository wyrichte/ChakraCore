//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptRegExp::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_RegEx;
    }

    inline JavascriptRegExp* JavascriptRegExp::FromVar(Var aValue) 
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptRegExp'");
        
        return static_cast<JavascriptRegExp *>(RecyclableObject::FromVar(aValue));
    }

    inline void JavascriptRegExp::SetRegex(UnifiedRegex::RegexPattern* pattern)
    {
        this->pattern = pattern;
    }

    inline JavascriptRegExp* JavascriptRegExp::GetJavascriptRegExp(Var var, ScriptContext* scriptContext)
    {
        if (JavascriptRegExp::Is(var))
        {
            return JavascriptRegExp::FromVar(var);
        }

        if (JavascriptOperators::GetTypeId(var) == TypeIds_HostDispatch)
        {
            TypeId remoteTypeId;
            RecyclableObject* reclObj = RecyclableObject::FromVar(var);
            reclObj->GetRemoteTypeId(&remoteTypeId);
            if (remoteTypeId == TypeIds_RegEx)
            {
                return static_cast<JavascriptRegExp *>(reclObj->GetRemoteObject());
            }
        }

        return nullptr;
    }
} 
