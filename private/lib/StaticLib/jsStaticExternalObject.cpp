//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"
#include "ExternalObject.h"
#include "CustomExternalType.h"

namespace JsStaticAPI
{
    HTYPE ExternalObject::GetTypeFromVar(Var instance)
    {
        return ((Js::CustomExternalObject *)instance)->GetType();
    }

    void ** ExternalObject::TypeToExtension(HTYPE instance)
    {
        return (void**)(((char*)instance) + sizeof(Js::CustomExternalType));
    }

    void ** ExternalObject::VarToExtension(Var instance)
    {
        return (void**)(((char*)instance) + sizeof(Js::CustomExternalObject));
    }

    Var ExternalObject::ExtensionToVar(void * buffer)
    {
        Assert(buffer != nullptr);
        return (Var)(((char*)buffer) - sizeof(Js::CustomExternalObject));
    }

    HRESULT __stdcall ExternalObject::BuildDOMDirectFunction(
        IActiveScriptDirect* activeScriptDirect,
        Var signature,
        ScriptMethod entryPoint,
        PropertyId nameId,
        UINT64 flags,
        UCHAR length,
        Var* jsFunction)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);

        return scriptEngineBase->BuildDOMDirectFunction(signature, entryPoint, nameId, flags, length, jsFunction);
    }
}