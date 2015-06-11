// Copyright (C) Microsoft Corporation. All rights reserved.
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// disable non-standard extension warning. This is necessary to use the 'override' 
// specifier in code that compiles at Warning level 4
#pragma warning(disable: 4481)

// Windows and CRT includes
#include <windows.h>

// WinRT Headers
#include <WinRT.h>
#include <wrl\client.h>
#include <animals.h>


