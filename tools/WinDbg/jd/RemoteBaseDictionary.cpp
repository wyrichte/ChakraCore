//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


JDRemoteTyped RemoteBaseDictionary::GetBuckets()
{
    return baseDictionary.Field("buckets");
}

JDRemoteTyped RemoteBaseDictionary::GetEntries()
{
    return baseDictionary.Field("entries");
}
