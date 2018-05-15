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
    return JDRemoteTyped::NullPtr();
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetDeferredFunctionsDictionary()
{
    if (!GetExtension()->IsJScript9())
    {
        return utf8SourceInfo.Field("m_deferredFunctionsDictionary");
    }
    return JDRemoteTyped::NullPtr();
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetFunctionBodyDictionary()
{
    return utf8SourceInfo.Field("functionBodyDictionary");
}

ExtRemoteTyped RemoteUtf8SourceInfo::GetExtRemoteTyped()
{
    return utf8SourceInfo.GetExtRemoteTyped();
}

bool RemoteUtf8SourceInfo::HasBoundedPropertyRecordHashSet()
{
    return utf8SourceInfo.HasField("boundedPropertyRecordHashSet");
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetBoundedPropertyRecordHashSet()
{
    return utf8SourceInfo.Field("boundedPropertyRecordHashSet");
}
