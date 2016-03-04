;//---------------------------------------------------------------------------
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

namespace Memory{};
using namespace Memory;

class StackBackTrace;

#include "DiagAssertion.h"

// TODO (doilij): remove JD's dependency on ATL
#pragma push_macro("_DEBUG")
#undef _DEBUG
#include <atlbase.h>
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
typedef wchar_t wchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef signed char sbyte;
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned char byte;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

typedef uintptr_t uintptr;
typedef intptr_t intptr;

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
