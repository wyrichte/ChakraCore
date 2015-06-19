/*++

Module Name:

    jsthunk.h

Abstract:

    JScript thunks without writable executable memory, based closely on ATL thunks.

Author:

    Jay Krell (jaykrell) 22-Aug-2013

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (__cdecl *JsFunc0_t)(void*, ...); // basically Js::JavascriptMethod, except for AsmJs case
typedef void* (__cdecl *JsFunc1_t)(void**);     // InterpreterThunk
struct JsThunkData_t; // opaque

JsThunkData_t*
__stdcall
JsThunk_AllocateData(
    void
    );

void
__stdcall
JsThunk_InitData(
    JsThunkData_t* ThunkData,
    void* Func1 // JsFunc1_t
    );

void* // JsFunc0_t
__stdcall
JsThunk_DataToCode(
    void* ThunkData
    );

void
__stdcall
JsThunk_Cleanup(
    void* ThunkData
    );

bool
__stdcall
JsThunk_Is(
    void* Code // Data works too
    );

unsigned
__stdcall
JsThunk_GetSize(
    void
    );

void
__stdcall
JsThunk_CleanupDefer(
    PSINGLE_LIST_ENTRY DeferredCleanup,
    void* ThunkData
    );

void
__stdcall
JsThunk_CleanupFinish(
    PSINGLE_LIST_ENTRY DeferredCleanup
    );

#ifdef __cplusplus
} /* extern "C" */
#endif
