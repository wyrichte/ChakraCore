//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    __inline Var JavascriptBoolean::OP_LdTrue(ScriptContext*scriptContext)
    {
        return scriptContext->GetLibrary()->GetTrue();
    }

    __inline Var JavascriptBoolean::OP_LdFalse(ScriptContext* scriptContext)
    {
        return scriptContext->GetLibrary()->GetFalse();
    }

    inline bool JavascriptBoolean::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Boolean;
    }

    inline JavascriptBoolean* JavascriptBoolean::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptBoolean'");
        
        return static_cast<JavascriptBoolean *>(RecyclableObject::FromVar(aValue));
    }

    inline Js::Var JavascriptBoolean::ToVar(BOOL fValue, ScriptContext* scriptContext)
    {
        return
            fValue ?
            scriptContext->GetLibrary()->GetTrue() :
            scriptContext->GetLibrary()->GetFalse();
    }
} // namespace Js
