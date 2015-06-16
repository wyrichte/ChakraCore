//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE
bool
RemoteFunctionInfo::HasBody()
{
    if (functionInfo.HasField("functionBodyImpl"))
    {
        return functionInfo.Field("functionBodyImpl").GetPtr() != 0;
    }
    return functionInfo.Field("hasBody").GetUchar() != 0; // older version
}

RemoteFunctionBody
RemoteFunctionInfo::GetFunctionBody()
{
    if (!HasBody())
    {
        return RemoteFunctionBody();
    }
    // TODO: Proxy check?
    if (functionInfo.HasField("functionBodyImpl"))
    {
        return functionInfo.Field("functionBodyImpl").GetPtr();
    }
    return functionInfo.GetPtr();
}

ULONG64
RemoteFunctionInfo::GetOriginalEntryPoint()
{
    return functionInfo.Field("originalEntryPoint").GetPtr();
}
#endif