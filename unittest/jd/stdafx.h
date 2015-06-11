//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include "DiagAssertion.h"
#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlfile.h>
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

#include "IEUtest.h"
#include "UTestHelper.h"
#include "werdump.h"
#include "MockDataTarget.h"
#include "ut_jsdiag.h"
#include "SinkDebugSite.h"
