//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

RefCountedHostVariant::RefCountedHostVariant(HostVariant* hostVariant):
    hostVariant(hostVariant),
    refCount(1)
{
}

ULONG RefCountedHostVariant::Release()
{
    refCount--;
    Assert((long)refCount >= 0);
    if (refCount == 0)
    {
        // disconnect the hostVariant
        // let the recycler clean it up.
        hostVariant = NULL;
    }
    return refCount;
}

void RefCountedHostVariant::Finalize(bool isShutdown)
{
}

void RefCountedHostVariant::Dispose(bool isShutdown) 
{
    // all refcount should have been released during finalize phrase.
    Assert(isShutdown || hostVariant == NULL);
}
