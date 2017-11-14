//  Copyright (c) Microsoft Corporation. All rights reserved.

// disable non-standard extension warning. This is necessary to use the 'override' 
// specifier in code that compiles at Warning level 4
#pragma warning(disable: 4481)

#include <windows.h>

//WinRT Related Includes
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <windowsstringp.h>
#include <roerrorapip.h>
#include <roerrorapi.h>
#include <minwin\sddl.h>

extern "C"
BOOL
WINAPI
QuirkIsEnabled(__in ULONG QuirkId);