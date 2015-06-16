//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE

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
#endif