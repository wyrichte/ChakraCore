//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#if DBG
#else // DBG
#pragma warning(disable: 4189)  // initialized but unused variable (e.g. variable that may only used by assert)
#endif

#define Unused(var) var;

//TODO: remove this define, once updated corhdr.h reaches branch
#define ofNoTransform 0x00001000	// Disable automatic transforms of .winmd files.

#define PREVENT_COPY(ClassName) \
    private: \
        ClassName(const ClassName & copy); \
        ClassName & operator =(const ClassName & rhs);

#define QI_IMPL(name, intf)\
    if (IsEqualIID(riid, name))\
{   \
    *ppvObj = static_cast<intf *>(this); \
    static_cast<intf *>(this)->AddRef();\
    return NOERROR;\
}\

#define QI_IMPL_INTERFACE(intf) \
    QI_IMPL(__uuidof(intf), intf)

#define ARGUMENTS(callInfo, args) \
    va_list _argptr; \
    va_start(_argptr, callInfo); \
    Var* args = (Var*)_argptr; \

#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedGoLabel(expr, label) do { hr = (expr); if (FAILED(hr)) { if(hr != E_ABORT) { DebuggerController::LogError(L"%s Hr:0x%x", _TEXT(#expr), hr); } goto label;  } } while (FALSE)
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)

#define IfFailedGoLabel_NoLog(expr, label) do { hr = (expr); if (FAILED(hr)) { goto label; } } while (FALSE)
#define IfFailedGo_NoLog(expr) IfFailedGoLabel_NoLog(expr, LReturn)

#define IfFailGo(expr) IfFailedGoLabel(hr = (expr), Error)
#define IfFailGo_NoLog(expr) IfFailedGoLabel_NoLog(hr = (expr), Error)

#define IfFailRet(expr) IfFailedReturn(expr)

#define ODS(x) OutputDebugString(x)
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFUNCTION__ WIDEN(__FUNCTION__)
#define WM_USER_PAGE_LOADED     ((WM_USER) + 1)
#define WM_USER_QUIT            ((WM_USER) + 2)
#define WM_USER_DEBUG_MESSAGE   ((WM_USER) + 3)


#define WIN32_LEAN_AND_MEAN 1

#define ENABLE_TEST_HOOKS 1
#include "CommonDefines.h"
#include "DiagAssertion.h"

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <assert.h>
#include <strsafe.h>
#include <cor.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <string>

// A hack to get around linker errors
#ifdef _DEBUG
#define _DEBUG_WAS_DEFINED
#undef _DEBUG
#endif
#include <atlbase.h>
#include <atlsafe.h>
#ifdef _DEBUG_WAS_DEFINED
#define _DEBUG
#undef _DEBUG_WAS_DEFINED
#endif

#include <functional>
#include "Psapi.h"

#include <roapi.h>
#include <winstring.h>
#include <shlwapi.h>
#include <wininet.h>
#include <mshtml.h>
#include <mshtmdid.h>
#include <mshtmhst.h>
#include <mshtmpid.h>
#include <ieguidp.h>
#include <interned.h>
#include <mshtmcid.h>

#include "intsafe.h"
#include "activscp.h"
#include "activaut.h"
#include "activscp_private.h"
#include "activdbg.h"
#include "activdbg100.h"
#include "activdbg_private.h"
#include "edgescriptdirect.h"
#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"
#include "EdgeJsHostNativeTest.h"
#include "ThreadMessage.h"
#include "MessageQueue.h"
#include "phscriptsite.h"
#include "ScriptDebugEvent.h"
#include "ScriptDebugNodeSource.h"
#include "DebuggerController.h"
#include "DebuggerCore.h"
#include "ComObject.h"
#include "ScriptDirectHelper.h"
#include "MockSCAContext.h"
#include "MockTypeOperations.h"
#include "MockImageData.h"
#include "sca.h"
#include "ScriptProfiler.h"
#include "WScriptFastDom.h"
#include "DiagnosticsHelper.h"
#include <shdeprecated.h>  
#include <DocObjectService.h>

#ifdef LANGUAGE_SERVICE_TEST
#include "ScriptDirectAuthor.h"
#include "LanguageService.h"
#include "LanguageServiceTestDriver.h"
#endif
        
#include "TestHooks.h"
#include "TestUtilitiesReferences.h"
#include "TestUtilities.h"
#include "Helpers.h"
#include "HostConfigFlags.h"
#include "Jscript9Interface.h"
#include "delegatewrapper.h"
#include "HtmlWinSink.h"
#include "HtmlAppWindow.h"
#include "HtmlApp.h"

typedef HRESULT (*DoOneIterationPtr)(BSTR filename);
void __stdcall PrintUsage();
int ExecuteTests(int argc, __in_ecount(argc) LPWSTR argv[], DoOneIterationPtr pfDoOneIteration, bool useJScript9 = false);
int ExecuteLSTests(int argc, __in_ecount(argc) LPWSTR argv[]);
int _cdecl ExecuteHtmlTests(int argc, __in_ecount(argc) LPWSTR argv[]);

//--- BEGIN HtmlHost
void PrintDISPID(DISPID dispidMember);
void PrintBSTR(BSTR bstr);
void QuitHtmlHost();
extern HINSTANCE g_hInst;
extern LPAPP g_pApp;
//--- END HtmlHost

extern LPWSTR dbgBaselineFilename;
extern HINSTANCE jscriptLibrary;
extern IGlobalInterfaceTable * git;
EXTERN_C CRITICAL_SECTION hostThreadMapCs;
extern bool IsRunningUnderJdtest;

HRESULT CreateEngineThread(HANDLE * thread, HANDLE * terminateThreadEvent = 0);
HRESULT CreateNewEngine(HANDLE thread, JsHostActiveScriptSite ** scriptSite, bool freeAtShutdown, bool actAsDiagnosticsHost, bool isPrimary, WORD domainId);
HRESULT ShutdownEngine(JsHostActiveScriptSite* scriptSite);

// BEGIN debug operations
void PerformSourceRundown();
void FastDomDebugAttach(Var function);
void FastDomDebugDetach(Var function);
void FastDomStartProfiler(Var function);
void FastDomStopProfiler(Var function);
void FastDomEdit(Var function, IDebugDocumentText*, ULONG startOffset, ULONG length, PCWSTR editContent, ULONG newLength);

void DispatchDebugAttach(IDispatch* function);
void DispatchDebugDetach(IDispatch* function);
void DispatchStartProfiler(IDispatch* function);
void DispatchStopProfiler(IDispatch* function);
// END  debug operations

class AutoCriticalSection 
{
public:
    AutoCriticalSection(CRITICAL_SECTION* cs) {m_cs = cs; EnterCriticalSection(cs); };
    ~AutoCriticalSection() { Assert(m_cs!= NULL); LeaveCriticalSection(m_cs); }
private:
    CRITICAL_SECTION* m_cs;
};
