//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteUtf8SourceInfo
{
public:
    RemoteUtf8SourceInfo(ExtRemoteTyped const& utf8SourceInfo) : utf8SourceInfo(utf8SourceInfo) {};

private:
    ExtRemoteTyped utf8SourceInfo;
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------