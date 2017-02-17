//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------


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
JDRemoteTyped RemoteUtf8SourceInfo::GetDeferredFunctionsDictionary()
{
    if (!GetExtension()->IsJScript9())
    {
        return utf8SourceInfo.Field("m_deferredFunctionsDictionary");
    }
    return JDRemoteTyped("(void *)0");
}

JDRemoteTyped RemoteUtf8SourceInfo::GetFunctionBodyDictionary()
{
    return utf8SourceInfo.Field("functionBodyDictionary");
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------