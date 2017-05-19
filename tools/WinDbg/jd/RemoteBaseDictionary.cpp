//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

JDRemoteTyped RemoteBaseDictionary::GetBuckets()
{
    return baseDictionary.Field("buckets");
}

JDRemoteTyped RemoteBaseDictionary::GetEntries()
{
    return baseDictionary.Field("entries");
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
