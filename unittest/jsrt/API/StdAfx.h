//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <process.h>
#include "wexstring.h"
#include "wextestclass.h"
#include "Helpers.h"

// Edge-mode binaries and build, use edge-mode JsRT
#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"
#include "jsrtprivate.h"