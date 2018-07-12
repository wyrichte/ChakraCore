//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

class RemoteJavascriptLibrary
{
public:
    RemoteJavascriptLibrary(ULONG64 address);

    RemoteScriptContext GetScriptContext();

    void PrintStateAndLink(bool showNull);
    void PrintLink(bool showNull = false);

    bool IsInternal();
    bool IsClosed();

    static const ULONG64 GlobalLibrary = (ULONG64)-1;
private:
    JDRemoteTyped javascriptLibrary;
};