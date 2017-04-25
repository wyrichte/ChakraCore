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
}