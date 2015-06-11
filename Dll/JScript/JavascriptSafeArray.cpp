//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    bool JavascriptSafeArray::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_SafeArray;
    }

    JavascriptSafeArray* JavascriptSafeArray::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSafeArray'");
        return static_cast<JavascriptSafeArray *>(RecyclableObject::FromVar(aValue));
    }

    RecyclableObject * JavascriptSafeArray::CloneToScriptContext(ScriptContext* requestContext)     
    {
        return RecyclerNewLeafZ(requestContext->GetRecycler(), JavascriptSafeArray, ScriptSite::FromScriptContext(requestContext)
                                    , m_vt, m_safeArray, requestContext->GetLibrary()->GetJavascriptSafeArrayType());
    }

    uint JavascriptSafeArray::GetSafeArrayLength()
    {
        if (!m_safeArray || m_safeArray->cDims == 0)
        {
            return 0;
        }
        uint len = 1;
        for (uint i = 0; i < m_safeArray->cDims; i++ )
        {
            len = UInt32Math::Mul(len, m_safeArray->rgsabound[i].cElements, Js::Throw::OutOfMemory);
        }
        return len;
    }

    uint JavascriptSafeArray::GetSafeArraySize()
    {
        return (!m_safeArray) ? 0 : UInt32Math::Mul(GetSafeArrayLength(), m_safeArray->cbElements, Js::Throw::OutOfMemory);
    }
}

