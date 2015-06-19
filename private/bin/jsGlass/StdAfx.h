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
#pragma warning(disable:4100)

#if DBG==1
#define __DONOTUSEDIRECTLY__ALWAYSBREAK() NT_ASSERT(FALSE) // Plug into Windows' common assert infrastructure on chk builds.
#else
#define __DONOTUSEDIRECTLY__ALWAYSBREAK() __debugbreak();
#endif

// REVIEW: [sebazim] These asserts are also defined on Free builds and never break unless a debugger is 
// attached. This should probably be changed to match the default behavior of an assert: always break on
// check builds only, regardless of if a debugger is attached.
#define AssertMsg(f, ...) \
    do \
    { \
        if (!(f)) \
        { \
            if (IsDebuggerPresent()) \
            { \
                __DONOTUSEDIRECTLY__ALWAYSBREAK(); \
            } \
        } \
    } while (0)

#define IfFailGo(expr) IfFailGoto(expr, Error)

#define IfFailGoto(expr, label) \
 do {                           \
  hr = (expr);                  \
  if (FAILED (hr)) {            \
   goto label;                  \
  }                             \
 } while (0)

#define IfFalseGo(expr, HR) \
 do {                       \
  if (!(expr)) {            \
   hr = (HR);               \
   goto Error;              \
  }                         \
 } while (0)

#define IfNullGo(expr, result) \
 do {                          \
    if ((expr) == NULL) {      \
        IfFailGo ((result));   \
    }                          \
 } while (0)


#define IfFlagSet(value ,flag) ((value & flag) == flag)

#ifndef USE_VC_CRT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>        // added for ntassert.h
#include <ntassert.h>
#else
#include "..\..\..\published\internal\inc\ntassert.h"
#endif

#include "targetver.h"
#include <process.h>
#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>
#undef DEBUG

#include "activdbg.h"
#include "activdbg100.h"
#include "activscp.h"
#include "activscp_private.h"
#include "activaut.h"
#include "edgescriptDirect.h"
#include "threadmessage.h"
#include "scriptsite.h"
#ifdef DIRECT_AUTHOR_TEST
#include "ScriptDirectAuthor.h"
#endif
#include "authoringhost.h"
#include "main.h"

#define SCRIPTPROP_INVOKEVERSIONING         0x00004000
