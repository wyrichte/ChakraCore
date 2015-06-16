//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE

RemoteRecyclableObject::RemoteRecyclableObject() {}
RemoteRecyclableObject::RemoteRecyclableObject(ExtRemoteTyped const& o) : object(o) {};

ExtRemoteTyped
RemoteRecyclableObject::GetTypeId()
{
    return object.Field("type.typeId");
}

bool
RemoteRecyclableObject::IsJavascriptFunction()
{
    return strcmp(JDUtil::GetEnumString(GetTypeId()), "TypeIds_Function") == 0;
}

RemoteJavascriptFunction
RemoteRecyclableObject::AsJavascriptFunction()
{
    if (!IsJavascriptFunction())
    {
        return RemoteJavascriptFunction();
    }
    return object.GetPtr();
}

#endif