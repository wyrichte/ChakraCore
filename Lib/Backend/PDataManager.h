//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#if PDATA_ENABLED

class PDataManager
{
public:
    static void RegisterPdata(RUNTIME_FUNCTION* pdataStart, _In_ const ULONG_PTR functionStart, _In_ const ULONG_PTR functionEnd, _Out_opt_ PVOID* pdataTable, ULONG entryCount = 1, ULONG maxEntryCount = 1);
    static void UnregisterPdata(RUNTIME_FUNCTION* pdata);
};

#endif
