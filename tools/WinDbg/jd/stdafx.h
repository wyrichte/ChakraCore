//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define RECYCLER_WRITE_BARRIER
#define BUCKETIZE_MEDIUM_ALLOCATIONS 1
#define SMALLBLOCK_MEDIUM_ALLOC 1
#define RECYCLER_PAGE_HEAP
#define LARGEHEAPBLOCK_ENCODING 1

#define _NO_CVCONST_H

#define DECLSPEC_GUARD_OVERFLOW __declspec(guard(overflow))

namespace Memory{};
using namespace Memory;

class StackBackTrace;

#include "DiagAssertion.h"

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

#include <objbase.h>
#include "edgescriptDirect.h"
#include "jscript9diag.h"
#include "jscript9diagprivate.h"
#include <engextcpp.hpp>
#include <string>
#include <hash_map>
#include <Warnings.h>
#include <comdef.h>
#include <strsafe.h>

// From Common.h
#include "CommonTypedefs.h"

namespace Output
{
    void Print(LPCWSTR format, ...);
}

// For 64-bit
const uint64 FloatTag_Value      = 0xFFFCull << 48;

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

#ifndef JD_PRIVATE
#define JD_IS_PUBLIC    true
#else
#define JD_IS_PUBLIC    false
#endif

#define PropertyDeleted         0x08

#if defined(_M_X64) || defined(_M_ARM64)
#define _M_X64_OR_ARM64 1
#endif

#include "DataStructures\BitVector.h"
#include "time.h"
#include "UTestHelper.h"
#include "ComObject.h"
#include "Nullable.h"
#include "RemoteTypeHandler.h"
#include "ScriptDebugSite.h"

#include "ExtRemoteTypedUtil.h"
#include "JDRemoteTyped.h"

#include "RecyclerCachedData.h"
#include "jd.h"
#include "TypeHandlerPropertyNameReader.h"

#ifdef JD_PRIVATE
#include "dbghelp.h"
#include "dbgeng.h"
#include "werdump.h"
#include "MockDataTarget.h"
#include "DiagException.h"
#include "BasePtr.h"
#include "AutoPtr.h"
#include "DiagAutoPtr.h"
#include "thrownew.h"
#include "Serializer.h"
#include "Serializer.inl"
#include "ScriptDump.h"
#include "ScriptDumpReader.h"
#endif
