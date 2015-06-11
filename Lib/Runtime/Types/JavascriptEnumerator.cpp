//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptEnumerator::JavascriptEnumerator(ScriptContext* scriptContext) : RecyclableObject(scriptContext->GetLibrary()->GetEnumeratorType())
    {
        Assert(scriptContext != NULL);
    }

    bool JavascriptEnumerator::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Enumerator;
    }

    JavascriptEnumerator* JavascriptEnumerator::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptEnumerator'");
        
        return static_cast<JavascriptEnumerator *>(RecyclableObject::FromVar(aValue));
    }
}