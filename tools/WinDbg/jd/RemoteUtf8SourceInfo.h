//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteScriptContext;
class RemoteUtf8SourceInfo
{
public:
    RemoteUtf8SourceInfo(ExtRemoteTyped const& utf8SourceInfo) : utf8SourceInfo(utf8SourceInfo) {};
    
    JDRemoteTyped GetSrcInfo();
    JDRemoteTyped GetLineOffsetCache();
    RemoteBaseDictionary GetDeferredFunctionsDictionary();
    RemoteBaseDictionary GetFunctionBodyDictionary();
    
    RemoteScriptContext GetScriptContext();

    ExtRemoteTyped GetExtRemoteTyped();

private:
    JDRemoteTyped utf8SourceInfo;
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------