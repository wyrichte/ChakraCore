//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


JDRemoteTyped RemoteUtf8SourceInfo::GetSrcInfo()
{
    return utf8SourceInfo.Field("m_srcInfo");
}

RemoteScriptContext RemoteUtf8SourceInfo::GetScriptContext()
{
    return utf8SourceInfo.Field("m_scriptContext");
}

JDRemoteTyped RemoteUtf8SourceInfo::GetLineOffsetCache()
{
    if (!GetExtension()->IsJScript9())
    {
        return utf8SourceInfo.Field("m_lineOffsetCache");
    }
    return JDRemoteTyped("(void *)0");
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetDeferredFunctionsDictionary()
{
    if (!GetExtension()->IsJScript9())
    {
        return utf8SourceInfo.Field("m_deferredFunctionsDictionary");
    }
    return JDRemoteTyped("(void *)0");
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetFunctionBodyDictionary()
{
    return utf8SourceInfo.Field("functionBodyDictionary");
}

ExtRemoteTyped RemoteUtf8SourceInfo::GetExtRemoteTyped()
{
    return utf8SourceInfo;
}
