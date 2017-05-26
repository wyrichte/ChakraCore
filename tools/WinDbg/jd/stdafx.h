//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define _NO_CVCONST_H

// TODO (doilij): remove JD's dependency on ATL
#pragma push_macro("_DEBUG")
#undef _DEBUG
#pragma warning(push)
#pragma warning(disable:4838) // conversion from 'int' to 'UINT' requires a narrowing conversion
#include <atlbase.h>
#pragma warning(pop)
#include <atlcoll.h>
#include <atlcom.h>
#pragma pop_macro("_DEBUG")

#include "edgescriptDirect.h"
#include "jscript9diag.h"
#include "jscript9diagprivate.h"
#include <engextcpp.hpp>
#include <string>
#include <hash_map>
#include <Warnings.h>

// -----------------------------------------------------------------------------------------
// Chakra references
#include "Assertions.h"
#include "DiagAssertion.h"

#include "CommonTypedefs.h"

// For 64-bit
const uint64 FloatTag_Value      = 0xFFFCull << 48;

#define PropertyDeleted         0x08

#include "common\MathUtil.h"
// -----------------------------------------------------------------------------------------

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

#define QI_IMPL(name, intf)     \
    if (IsEqualIID(riid, name)) \
{                               \
    *ppvObj = static_cast<intf *>(this);    \
    static_cast<intf *>(this)->AddRef();    \
    return NOERROR;             \
}

#define QI_IMPL_INTERFACE(intf) \
    QI_IMPL(__uuidof(intf), intf)

#include "time.h"
#include "UTestHelper.h"
#include "Nullable.h"
#include "RemoteTypeHandler.h"
#include "ExtRemoteTypedUtil.h"
#include "JDRemoteTyped.h"
#include "RemoteBaseDictionary.h"

#include "RecyclerCachedData.h"
#include "jd.h"
#include "TypeHandlerPropertyNameReader.h"

#include "dbghelp.h"
#include "RemoteVar.h"
