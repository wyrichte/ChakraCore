//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteScriptContext;
class RemoteJavascriptFunction;
class RemoteRecyclableObject
{
public:
    RemoteRecyclableObject();
    RemoteRecyclableObject(ULONG64 address);
    RemoteRecyclableObject(ExtRemoteTyped const& o);
    bool IsJavascriptFunction();
    RemoteJavascriptFunction AsJavascriptFunction();

    ULONG64 GetPtr();
    void Print(bool printSlotIndex, int depth);
    bool DumpPossibleExternalSymbol(char const * typeName, bool makeLink = true, bool showScriptContext = false);
    
    JDRemoteTyped GetType();
    RemoteScriptContext GetScriptContext();

	void PrintSimpleVarValue();
protected:
    char const * GetTypeName();
    char const * GetTypeIdEnumString();

    JDRemoteTyped object;
    char const * typeName;
};
