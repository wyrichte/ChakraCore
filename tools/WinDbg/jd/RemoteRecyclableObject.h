//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteJavascriptFunction;
class RemoteRecyclableObject
{
public:
    RemoteRecyclableObject();
    RemoteRecyclableObject(ExtRemoteTyped const& o);
    bool IsJavascriptFunction();
    RemoteJavascriptFunction AsJavascriptFunction();
protected:
    ExtRemoteTyped GetTypeId();
    ExtRemoteTyped object;
};
