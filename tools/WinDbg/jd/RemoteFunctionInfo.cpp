//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

bool
RemoteFunctionInfo::HasBody()
{
    if (functionInfo.HasField("functionBodyImpl"))
    {
        return JDUtil::GetWrappedField(functionInfo, "functionBodyImpl").GetPtr() != 0;
    }
    return functionInfo.Field("hasBody").GetUchar() != 0; // older version
}

RemoteFunctionBody
RemoteFunctionInfo::GetFunctionBody()
{
    if (!HasBody())
    {
        return RemoteFunctionBody(0);
    }
    // TODO: Proxy check?
    if (functionInfo.HasField("functionBodyImpl"))
    {
        return JDUtil::GetWrappedField(functionInfo, "functionBodyImpl").GetPtr();
    }
    return functionInfo.GetPtr();
}

ULONG64
RemoteFunctionInfo::GetOriginalEntryPoint()
{
    return functionInfo.Field("originalEntryPoint").GetPtr();
}
