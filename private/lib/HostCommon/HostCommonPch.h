//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#define QI_IMPL(name, intf)\
    if (IsEqualIID(riid, name))\
{   \
    *ppvObj = static_cast<intf *>(this); \
    static_cast<intf *>(this)->AddRef();\
    return NOERROR;\
}\

#define Assert assert

#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; }
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)

// The following will assign to hr
#define IfFailGo(expr) IfFailedGoLabel(hr = (expr), Error)

#define ENABLE_TEST_HOOKS 1
#include "CommonDefines.h"

#include <stdio.h>
#include <string>
#include <string>
#include <map>
#include <io.h>
#include <assert.h>
#include <strsafe.h>
#include <atlbase.h>
#include <Psapi.h>

#include <roapi.h>
#include <winstring.h>
#include <shlwapi.h>
#include <comutil.h>
#include <wininet.h>

#include "intsafe.h"
#include "activscp.h"
#include "activscp_private.h"
#include "activdbg.h"
#include "edgescriptdirect.h"
#include "HostConfigFlags.h"
#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"
#include "DebuggerController.h"

#include "TestHooks.h"

#include "JScript9Interface.h"


