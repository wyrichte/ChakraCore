//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1
//TODO: remove this define, once updated corhdr.h reaches branch 
#define ofNoTransform 0x00001000	// Disable automatic transforms of .winmd files.

#include <strsafe.h>
#include "Common.h"
#include "WinRT\WinRTLib.h"
#include "WinRT\SortProjection.h"
#include "StringUtils.h"
#include "macros.h"

