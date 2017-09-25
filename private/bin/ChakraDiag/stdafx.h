//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// This is supposed to exclude rarely-used .h files from windows.h and thus make build faster.
// See https://osgwiki.com/wiki/NOT_LEAN_AND_MEAN for details.
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "intsafe.h"
#include "psapi.h"
#include <stdio.h>

#if defined(DBG)
#define DiagAssert(exp)   \
    do { \
        if (!(exp)) \
        { \
            fprintf(stderr, "ASSERTION (%s, line %d) %s %s\n", __FILE__, __LINE__, _CRT_STRINGIZE(exp), #exp); \
            fflush(stderr); \
            DebugBreak(); \
        } \
    } while(0)
#else
#define DiagAssert(exp)             ((void)0)
#endif //defined(DBG)

#define ATLASSERT DiagAssert

// NOTE: This is a workaround to account for the fact that atlsd.lib cannot be linked to in
// OneCoreUAP builds because OpenFileMappingA doesn't exist in any APISet. To get around this,
// we undefine _DEBUG so that we don't need to link against atlsd.lib
// TODO: Remove this workaround when OpenFileMappingA is exposed through an APISet
// TODO: More: OpenFileMappingA is marked as legacy, will eventually be removed in next windows release.
// OpenFileMappingW is kept normal, need to wait for the onecore decision on ATL
// we can take the dependency on OpenFileMappingA for this release, but we will have a dependency on api-ms-win-core-kernel32-legacy-l1-1-2.dll
// which makes unittest painful. so still not link to atlsd.lib
#if defined(_DEBUG)
#define JS_ATL_DEBUG
#undef _DEBUG
#endif
#pragma warning(push)
#pragma warning(disable:4838) // conversion from 'int' to 'UINT' requires a narrowing conversion
#include "atlbase.h"
#pragma warning(pop)
#include <atlcoll.h>
#include <atlcom.h>
#include <atlstr.h>
#ifdef JS_ATL_DEBUG
#define _DEBUG
#undef JS_ATL_DEBUG
#endif

// DIAG_DAC indicates runtime #include's that they are compiling under Manual DAC for diagnostics.
#ifndef DIAG_DAC
#define DIAG_DAC
#endif

// In DAC we need to access all fields, so make everything public in Runtime.
#ifdef private
#undef private
#endif
#define private public

#ifdef protected
#undef protected
#endif
#define protected public

#include "EdgeScriptDirect.h"
#include "RuntimeCore.h"

#undef private
#undef protected
#undef DIAG_DAC

//----------------------------------------------------------------------------------------------
// WARNING: Do not #include any runtime headers after "DiagAssertion.h". Runtime headers redefines runtime specific Assert.
//----------------------------------------------------------------------------------------------
#include "DiagAssertion.h"


#ifndef VALIDATE_RUNTIME_VERSION
#define VALIDATE_RUNTIME_VERSION (0) // Enable before release
#endif

#define IfFailRet(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)

#define _ATL_APARTMENT_THREADED

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define DIAG_PHASE_OFF1(phase) RemoteConfiguration::GetInstance()->PhaseOff1(phase)
#define DIAG_PHASE_ON1(phase) RemoteConfiguration::GetInstance()->PhaseOn1(phase)
#define DIAG_CONFIG_FLAG(flag) (((Js::Configuration*)RemoteConfiguration::GetInstance()->ToTargetPtr())->flags.##flag##)
#else
    // All config flags are off by default.
#define DIAG_PHASE_OFF1(phase) (false)
#define DIAG_PHASE_ON1(phase) (false)
#define DIAG_CONFIG_FLAG(flag) (false)
#endif ENABLE_DEBUG_CONFIG_OPTIONS

#if defined(DBG) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
// Note: uncomment this for troubleshooting, 1 - basic dump, 2 - verbose.
//#define DUMP_FRAMES_LEVEL 2
#endif

// Forward reference
namespace JsDiag
{
    class RemoteStackWalker;
    class DebugClient;
    class RemoteStackFrame;

    enum Globals : unsigned
    {
#define ENTRY(field, name) Globals_##name##,
#include "DiagGlobalList.h"
#undef ENTRY
    Globals_Max
    };
    const int Globals_Count = (int)Globals_Max;
}

#include "thrownew.h"
#include "jscript9diag.h"
#include "jscript9diagprivate.h"
#include "mshtmldac.h"
#include "dbghelp.h"
#include "DbgEng.h"
#include "werdump.h"
#include "DiagConstants.h"
#include "AutoPtr.h"
#include "DiagAutoPtr.h"
#include "guids.h"
#include "DiagException.h"
#include "VirtualReader.h"
#include "RemoteBuffer.h"

#include "dllfunc.h"
#include "jsdiag.h"
#include "DAC.h"
#include "DiagProvider.h"
#include "DebugClient.h"
#include "RemoteStackFrame.h"
#include "InternalStackFrame.h"
#include "RemoteStackFrameEnumerator.h"
#include "RemoteInlineFrameWalker.h"
#include "RemoteStackWalker.h"
#include "RemoteBuffer.inl"
#include "Serializer.h"
#include "ScriptDump.h"
#include "ScriptDumpReader.h"
#include "ScriptState.h"
#include "rterror.h"

// .inl files
#include "Serializer.inl"
