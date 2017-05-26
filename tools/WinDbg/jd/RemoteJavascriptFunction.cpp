//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

RemoteJavascriptFunction::RemoteJavascriptFunction() {}
RemoteJavascriptFunction::RemoteJavascriptFunction(ExtRemoteTyped const& o) : 
    RemoteRecyclableObject(o) 
{}

RemoteJavascriptFunction::RemoteJavascriptFunction(ULONG64 ptr) :
    RemoteRecyclableObject(ExtRemoteTyped("(Js::JavascriptFunction *)@$extin", ptr))
{

}

bool
RemoteJavascriptFunction::IsScriptFunction()
{
    return GetFunctionInfo().HasBody();
}
   

RemoteScriptFunction
RemoteJavascriptFunction::AsScriptFunction()
{
    if (!IsScriptFunction())
    {
        return RemoteScriptFunction();
    }
    return object.GetPtr();
}

RemoteFunctionInfo 
RemoteJavascriptFunction::GetFunctionInfo()
{
    return object.Field("functionInfo");
}

void
RemoteJavascriptFunction::Print()
{
    RemoteFunctionInfo functionInfo = this->GetFunctionInfo();
    if (functionInfo.HasBody())
    {
        RemoteFunctionBody functionBody = functionInfo.GetFunctionBody();
        g_Ext->Out(_u("  [FunctionBody] "));
        functionBody.PrintNameAndNumberWithLink();
        g_Ext->Out(_u(" "));
        functionBody.PrintByteCodeLink();
        g_Ext->Out("\n");
    }
    else
    {
        std::string symbol = GetSymbolForOffset(functionInfo.GetOriginalEntryPoint());
        if (!symbol.empty())
        {
            g_Ext->Out("  [NativeEntry] %s", symbol.c_str());
        }
        else
        {
            object.Field("functionInfo").OutFullValue();
        }
    }
}