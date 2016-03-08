//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "DiagAssertion.h"

#if defined(_DEBUG)
#define JS_ATL_DEBUG
#undef _DEBUG
#endif
#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlfile.h>
#ifdef JS_ATL_DEBUG
#define _DEBUG
#undef JS_ATL_DEBUG
#endif

#include <algorithm>
#include <set>
#include <string>
#include <hash_map>
#include <comdef.h>
#include <strsafe.h>

#include "edgescriptDirect.h"
#include "jscript9diag.h"
#include "jscript9diagPrivate.h"

#define DEBUG_UNICODE_MACROS
#include <engextcpp.hpp>
#include <dbghelp.h>
#include "Core/CommonTypedefs.h"

#define IfNullReturnError(expr, hrError) \
    do {                        \
        if ((expr) == NULL) {   \
            return (hrError);   \
        }                       \
    } while(0)

#define IfFailGo(expr) IfFailGoto(expr, Error)

#define IfFailGoto(expr, label) \
    do {                        \
        hr = (expr);            \
        if (FAILED (hr)) {      \
            goto label;         \
        }                       \
    } while (0)

#define IfNullGo(expr, result)  \
    do {                        \
        if ((expr) == NULL) {   \
            IfFailGo(result);   \
        }                       \
    } while (0)

#ifdef UNICODE
typedef std::wstring string;
#else
typedef std::string string;
#endif

#define IfFailedAssertReturn(expr)  \
do { \
    hr = (expr); \
    if (FAILED(hr)) { \
        AssertMsg(FALSE, #expr); \
        return ; \
    } \
} while (FALSE)

#define IfFalseAssertReturn(expr)  \
do { \
    if (!(expr)) { \
        AssertMsg(FALSE, #expr); \
        return ; \
    } \
} while (FALSE)

#define IfFalseAssertMsgReturn(expr, msg)  \
do { \
    if (!(expr)) { \
        AssertMsg(FALSE, msg); \
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

#define ReturnIfHRNotEqual(expr1, expr2)  { \
    const HRESULT _hr1_ = (expr1); \
    const HRESULT _hr2_ = (expr2); \
    do { \
        if (_hr1_ != _hr2_) { \
            AssertMsg(FALSE, "HRESULTs are not equal"); \
            return ; \
        } \
    } while (FALSE); \
}

#define ExpressionNoThrow( exp ) \
try \
{ \
    exp; \
} \
catch (...) \
{ \
    m_fLastTestCaseFailed = true; \
    wprintf(_u("'%ls' has thrown an exception but it should not have"), _u(#exp)); \
    return; \
}

#include "UTestHelper.h"
#include "werdump.h"
#include "MockDataTarget.h"
#include "ut_jsdiag.h"
#include "SinkDebugSite.h"
