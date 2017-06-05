//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

RemoteJavascriptFunction::RemoteJavascriptFunction() {}
RemoteJavascriptFunction::RemoteJavascriptFunction(ExtRemoteTyped const& o) : 
    RemoteRecyclableObject(o) 
{}

RemoteJavascriptFunction::RemoteJavascriptFunction(ULONG64 ptr) :
    RemoteRecyclableObject(ptr)
{
	if (!this->IsJavascriptFunction())
	{
		g_Ext->ThrowStatus(E_INVALIDARG, "RemoteJavascriptFunction used with non-function %p", ptr);
	}
}

bool
RemoteJavascriptFunction::IsScriptFunction()
{
    return GetFunctionInfo().HasBody();
}

bool
RemoteJavascriptFunction::IsBoundFunction()
{
	return object.HasField("targetFunction");
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

RemoteRecyclableObject
RemoteJavascriptFunction::GetTargetFunction()
{
	return object.Field("targetFunction");
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
            g_Ext->Out("  [NativeEntry] %s\n", symbol.c_str());
        }
        else
        {
            object.Field("functionInfo").OutFullValue();
        }

		if (this->IsBoundFunction())
		{
			g_Ext->Out("  [TargetFunction] ");
			this->GetTargetFunction().PrintSimpleVarValue();
			g_Ext->Out("\n");
		}
    }
}