//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "DiagAssertion.h"

// NOTE: This is a workaround for not linking atlsd.lib

#if defined(_DEBUG)
#define JS_ATL_DEBUG
#undef _DEBUG
#endif
#include <atlbase.h>
#include <atlcom.h>
#ifdef JS_ATL_DEBUG
#define _DEBUG
#undef JS_ATL_DEBUG
#endif

#include <string>
#include <vector>
#include <map>
#include <comdef.h>

#ifdef UNICODE
typedef std::wstring string;
#else
typedef std::string string;
#endif

#define DEBUG_UNICODE_MACROS
#include "dbghelp.h"
#include "dbgeng.h"
#include "DiagException.h"

extern bool g_autoBreakpoints;
extern bool g_targetedTest;
extern bool g_dynamicAttach;
extern LPCTSTR g_dbgBaselineFilename;

#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedGoLabel(expr, label) do { hr = (expr); if (FAILED(hr)) { goto label; } } while (FALSE)
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)

// The following will assign to hr
#define IfFailGo(expr) IfFailedGoLabel(hr = (expr), Error)

#define IfFailError(expr, ...) if(FAILED(expr)) { DebuggerController::LogError(__VA_ARGS__); }

#define IfNullGo(expr, result) \
 do {                          \
    if ((expr) == NULL) {      \
        IfFailGo ((result));   \
    }                          \
 } while (0)

#define IfFalseAssertReturn(expr)  \
do { \
    if (!(expr)) { \
        AssertMsg(FALSE, #expr); \
        return ; \
    } \
} while (FALSE)

#define IfFalseAssertReturnEFAIL(expr)  \
do { \
    if (!(expr)) { \
        AssertMsg(FALSE, #expr); \
        return E_FAIL; \
    } \
} while (FALSE)

#include "edgescriptDirect.h"
#include "jscript9diag.h"
#include "jscript9diagprivate.h"
#include "UTestHelper.h"
#include "ScriptDebugEvent.h"
#include "RemoteData.h"
#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"
#include "debuggercontroller.h"
#include "SimpleDebugger.h"
#include "werdump.h"
#include "MockDataTarget.h"
