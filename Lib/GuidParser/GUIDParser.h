//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

extern "C"
{
    HRESULT TryParseGUID(_In_z_ PCWSTR g, _Out_ GUID* result);
    HRESULT TryGUIDToString(_In_ const GUID* g, _Out_writes_z_(length) PWSTR result, size_t length);
}


