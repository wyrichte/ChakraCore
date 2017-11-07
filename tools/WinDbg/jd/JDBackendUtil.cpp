//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "JDBackendUtil.h"
ExtRemoteTyped JDBackendUtil::GetFunctionBodyFromFunc(ExtRemoteTyped func)
{
    if (func.HasField("m_jnFunction"))
    {
        // Before OOP JIT
        return func.Field("m_jnFunction");
    }

    // After OOP JIT
    ExtRemoteTyped functionBodyAddr = func.Field("m_workItem.m_jitBody.m_bodyData.functionBodyAddr");
    return JDRemoteTyped::FromPtrWithVtable(functionBodyAddr.GetPtr());
}

EXT_CLASS_BASE::PropertyNameReader JDBackendUtil::GetPropertyNameReaderFromFunc(ExtRemoteTyped func)
{
    return EXT_CLASS_BASE::PropertyNameReader(GetExtension()->IsJITServer() ? ExtRemoteTyped("(void *)0") :
        RemoteFunctionBody(JDBackendUtil::GetFunctionBodyFromFunc(func)).GetThreadContext());
}