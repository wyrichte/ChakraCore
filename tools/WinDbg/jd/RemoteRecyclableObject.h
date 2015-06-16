//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

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

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
