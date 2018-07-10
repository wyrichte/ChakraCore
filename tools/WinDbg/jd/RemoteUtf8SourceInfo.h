//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteScriptContext;
class RemoteUtf8SourceInfo
{
public:
    static bool TryGetUtf8SourceInfoFromPointer(ULONG64 pointer, RemoteUtf8SourceInfo& remoteUtf8SourceInfo);

    RemoteUtf8SourceInfo() {};
    RemoteUtf8SourceInfo(JDRemoteTyped const& utf8SourceInfo) : utf8SourceInfo(utf8SourceInfo) {};

    JDRemoteTyped GetLineOffsetCache();
    RemoteBaseDictionary GetDeferredFunctionsDictionary();
    RemoteBaseDictionary GetFunctionBodyDictionary();

    bool HasBoundedPropertyRecordHashSet();
    RemoteBaseDictionary GetBoundedPropertyRecordHashSet();

    RemoteScriptContext GetScriptContext();

    ExtRemoteTyped GetExtRemoteTyped();

    // SourceContextInfo
    ULONG GetSourceContextId();
    void PrintSourceUrl();
    void PrintSource(ULONG64 startOffset = 0, ULONG length = -1);
    void SaveSource(char const * filename, ULONG64 startOffset = 0, ULONG length = -1);
private:
    bool GetSourceBuffer(ExtBuffer<CHAR>& sourceBuffer, ULONG64 startOffset, ULONG length);
    JDRemoteTyped GetSourceContextInfo();

    JDRemoteTyped utf8SourceInfo;
};
