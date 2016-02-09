/*++

Module Name:

    jsthunk_thunks.cpp

Abstract:

    JScript thunks without writable executable memory, based closely on ATL thunks.
    This is a separate file in order to compile without /Gy, to keep the thunks ordered
    and their size measurable.

Author:

    Jay Krell (jaykrell) 22-Aug-2013

--*/

#include "jsthunk_private.h"

// Try to always get the same codegen, to reduce test matrix.

#pragma optimize("gs", on)

#undef THUNK
#define THUNK(n) IMPL(n)
THUNKS
#pragma optimize("", on)
