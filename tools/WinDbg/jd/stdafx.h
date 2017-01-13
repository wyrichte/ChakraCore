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
#if defined(_MSC_VER) && _MSC_VER <= 1800 // VS2013?
#define THREAD_LOCAL __declspec(thread)
#else // VS2015+, linux Clang etc.
#define THREAD_LOCAL thread_local
#endif // VS2013?

namespace Memory{};
using namespace Memory;

class StackBackTrace;

#include "Assertions.h"
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

// ----------------------------------------------------------------------------
// Runtime headers using "Field" macro go here
#include "WriteBarrierMacros.h"
#include "DataStructures\BitVector.h"

#ifdef JD_PRIVATE
// include headers that recycler needs
#include "core\api.h"
#include "common\MathUtil.h"
#include "core\CriticalSection.h"
#include "DataStructures\Comparer.h"
#include "DataStructures\SizePolicy.h"
#include "core\SysInfo.h"
#include "core\AllocSizeMath.h"

// Include recycler headers
#include "Exceptions\Throw.h"
#include "Memory\AllocationPolicyManager.h"
#include "Memory\Allocator.h"
#include "DataStructures\SList.h"
#include "DataStructures\DList.h"
#include "DataStructures\DoublyLinkedListElement.h"
#include "Memory\VirtualAllocWrapper.h"
#include "Memory\MemoryTracking.h"
#include "Memory\PageAllocator.h"
#include "Memory\IdleDecommitPageAllocator.h"
#include "Memory\FreeObject.h"
#include "Memory\HeapConstants.h"
#include "Memory\HeapBlock.h"
#include "Memory\SmallHeapBlockAllocator.h"
#include "Memory\SmallNormalHeapBlock.h"
#include "Memory\SmallLeafHeapBlock.h"
#include "Memory\SmallFinalizableHeapBlock.h"

#if DBG
class HeapAllocator {}; // LargeHeapBlock use BVSparse<HeapAllocator>
#endif

#include "Memory\LargeHeapBlock.h"
#include "Memory\HeapBucket.h"
#include "Memory\SmallLeafHeapBucket.h"
#include "Memory\SmallNormalHeapBucket.h"
#include "Memory\SmallFinalizableHeapBucket.h"
#include "Memory\LargeHeapBucket.h"
#include <Memory\heapinfo.h>

#ifndef _M_X64
#define _M_X64  // force x64 so we get HeapBlockMap64
#include "Memory\HeapBlockMap.h"
#undef _M_X64
#else
#include "Memory\HeapBlockMap.h"
#endif

#endif  // JD_PRIVATE

#undef Field
#undef FieldNoBarrier
// undef "Field" to avoid conflicting with dbgeng function name
// ----------------------------------------------------------------------------

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
