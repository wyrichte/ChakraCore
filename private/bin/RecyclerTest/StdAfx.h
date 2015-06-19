//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "TargetVer.h"

#include <windows.h>
#include <winbase.h>
#include <oleauto.h>
#pragma warning(disable:4985)
#include <intrin.h>


#include <wtypes.h>
#include <Common.h>
#include <Commoninl.h>
#include <stdio.h>
#include <tchar.h>

#include "harness.h"
#include "reporting.h"
#include "tracker.h"
#include "list.h"
#include "tests.h"

#ifdef RECYCLER_PAGE_HEAP
extern int g_pageHeapModeType;
#endif