//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

typedef int (__cdecl *MainFunc)(int argc, __in_ecount(argc) LPWSTR argv[]);
bool UseLargeAddresses(int& argc, __in_ecount(argc) LPWSTR argv[]);
int TestLargeAddress(int argc, __in_ecount(argc) LPWSTR argv[], MainFunc pfunc);


