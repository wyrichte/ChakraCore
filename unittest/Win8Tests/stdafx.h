/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include "jsauto.h"
#include <math.h>

#define IFR(x)	if(FAILED(x)) { printf("... failed %x\n", x); return -1; } 
#define ASRT(x)  if(!(x))  { printf("... failed \n"); return -1; }
#define IFN(x)  if((x) == NULL) { printf("... failed\n"); return -1; }
#define IFRET(x) if (FAILED(x)) { return false; }
#define IFRET_ARG(x) if (FAILED(x)) { return x; }
