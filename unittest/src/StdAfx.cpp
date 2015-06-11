/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// stdafx.cpp : source file that includes just the standard includes
// JnConsole.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "DefaultCommonExternalApi.cpp"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// Stubbing to satisfy the linker
// If we need ETW rundown in ut_jscript, need to refactor the rundown 
// to its own lib to link with this dll
void EtwCallbackApi::OnSessionChange(ULONG controlCode, PVOID callbackContext)
{
}
