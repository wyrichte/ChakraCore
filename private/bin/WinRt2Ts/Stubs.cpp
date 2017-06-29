//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

const IID GUID_NULL = {};

// This is consumed by AutoSystemInfo. AutoSystemInfo is in Chakra.Common.Core.lib, which is linked
// into multiple binaries. The hosting binary provides the implementation of this function.
bool GetDeviceFamilyInfo(
    _Out_opt_ ULONGLONG* /*pullUAPInfo*/,
    _Out_opt_ ULONG* /*pulDeviceFamily*/,
    _Out_opt_ ULONG* /*pulDeviceForm*/)
{
    return false;
}

#if DBG && defined(RECYCLER_VERIFY_MARK)
bool IsLikelyRuntimeFalseReference(char* objectStartAddress, size_t offset,
    const char* typeName)
{
    return false;
}
#endif
