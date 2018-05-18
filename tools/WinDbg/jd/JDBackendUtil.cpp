//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "JDBackendUtil.h"
JDRemoteTyped JDBackendUtil::GetFunctionBodyFromFunc(JDRemoteTyped func)
{
    if (func.HasField("m_jnFunction"))
    {
        // Before OOP JIT
        return func.Field("m_jnFunction");
    }

    // After OOP JIT
    JDRemoteTyped functionBodyAddr = func.Field("m_workItem.m_jitBody.m_bodyData.functionBodyAddr");
    return JDRemoteTyped::FromPtrWithVtable(functionBodyAddr.GetPtr());
}

EXT_CLASS_BASE::PropertyNameReader JDBackendUtil::GetPropertyNameReaderFromFunc(JDRemoteTyped func)
{
    return EXT_CLASS_BASE::PropertyNameReader(GetExtension()->IsJITServer() ? JDRemoteTyped::NullPtr() :
        RemoteFunctionBody(JDBackendUtil::GetFunctionBodyFromFunc(func)).GetThreadContext());
}