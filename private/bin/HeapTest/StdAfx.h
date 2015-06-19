//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// #define HEAP_ALLOC 1

#include "TargetVer.h"

#ifdef __cplusplus
 extern "C" {
#endif
#include <nt.h>
#include <ntrtl_x.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
 }
 #endif

#include <windows.h>
#include <winbase.h>
#include <oleauto.h>
#if defined(USE_VC_CRT)
#pragma warning(disable:4985)
#include <intrin.h>
#pragma intrinsic (_InterlockedIncrement)
#else
extern "C"
{
char __cdecl
_InterlockedOr8 (
    __inout char volatile *Destination,
    __in    char Value
    );
char __cdecl
_InterlockedAnd8 (
    __inout char volatile *Destination,
    __in    char Value
    );
}
#endif

#include <wtypes.h>
#include <Runtime.h>
#include <stdio.h>
#include <tchar.h>

