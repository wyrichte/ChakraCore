//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "CommonDefines.h"
#include "DiagAssertion.h"

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <assert.h>
#include <strsafe.h>

#include "Psapi.h"

#include "intsafe.h"
#include "activscp.h"
#include "activaut.h"
#include "activscp_private.h"
#include "activdbg.h"
#include "activdbg100.h"
#include "activdbg_private.h"

#include "edgescriptdirect.h"
#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"

#include "targetver.h"
#include "TestHooks.h"
#include "Jscript9Interface.h"
