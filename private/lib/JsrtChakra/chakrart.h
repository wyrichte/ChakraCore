//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
/// \mainpage Chakra Hosting API Reference
///
/// Chakra is Microsoft's JavaScript engine. It is an integral part of Internet Explorer but can
/// also be hosted independently by other applications. This reference describes the APIs available
/// to applications to host Chakra.

/// \file
/// \brief The base Chakra hosting API.
///
/// This file contains a flat C API layer. This is the API exported by chakra.dll.

#ifdef _MSC_VER
#pragma once
#endif  // _MSC_VER

#ifndef _JSRT_
#error "You should include <jsrt.h> instead of <jsrt9.h> or <chakrart.h>."
#endif

#ifdef _JSRT9_H_
#error "It is invalid to include both jscript9-mode and edge-mode JsRT headers.  To include edge, use #define USE_EDGEMODE_JSRT and then include jsrt.h, and link against chakrart.lib.  To use jscript9 mode, include jsrt.h and link against jsrt.lib."
#endif

#ifndef _CHAKRART_H_
#define _CHAKRART_H_

#if NTDDI_VERSION >= NTDDI_WIN7

#include <oaidl.h>
#include <Inspectable.h>

#pragma region Application Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)

#include "chakracommon.h"


    /// <summary>
    ///     Version of the runtime.
    /// </summary>
    typedef enum _JsRuntimeVersion
    {
        /// <summary>
        ///     Create runtime with highest version present on the machine at runtime.
        /// </summary>
        JsRuntimeVersionEdge = -1,
    } JsRuntimeVersion;

    /// <summary>
    ///     Create runtime with IE10 version.
    /// </summary>
    __declspec(deprecated("Only JsRuntimeVersionEdge is supported when targeting the edge-mode JavaScript Runtime header.  To use jscript9 mode, which supports targeting docmode 10 or 11, omit USE_EDGEMODE_JSRT before including jsrt.h, and link against jsrt.lib.  For more information, go to https://go.microsoft.com/fwlink/?LinkId=522493")) const JsRuntimeVersion JsRuntimeVersion10 = (JsRuntimeVersion)0;
    /// <summary>
    ///     Create runtime with IE11 version.
    /// </summary>
    __declspec(deprecated("Only JsRuntimeVersionEdge is supported when targeting the edge-mode JavaScript Runtime header.  To use jscript9 mode, which supports targeting docmode 10 or 11, omit USE_EDGEMODE_JSRT before including jsrt.h, and link against jsrt.lib.  For more information, go to https://go.microsoft.com/fwlink/?LinkId=522493")) const JsRuntimeVersion JsRuntimeVersion11 = (JsRuntimeVersion)1;



    /// <summary>
    ///     Starts debugging in the current context.
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     The host should make sure that CoInitializeEx is called with COINIT_MULTITHREADED or COINIT_APARTMENTTHREADED at least once before using this API
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsStartDebugging();

    /// <summary>
    ///     Creates a JavaScript value that is a projection of the passed in <c>VARIANT</c>.
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     The projected value can be used by script to call a COM automation object from script.
    ///     Hosts are responsible for enforcing COM threading rules.
    ///     </para>
    ///     <para>
    ///     Requires an active script context.
    ///     </para>
    /// </remarks>
    /// <param name="variant">A <c>VARIANT</c> to be projected.</param>
    /// <param name="value">A JavaScript value that is a projection of the <c>VARIANT</c>.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsVariantToValue(
        _In_ VARIANT *variant,
        _Out_ JsValueRef *value);

    /// <summary>
    ///     Initializes the passed in <c>VARIANT</c> as a projection of a JavaScript value.
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     The projection <c>VARIANT</c> can be used by COM automation clients to call into the
    ///     projected JavaScript object.
    ///     </para>
    ///     <para>
    ///     Requires an active script context.
    ///     </para>
    /// </remarks>
    /// <param name="object">A JavaScript value to project as a <c>VARIANT</c>.</param>
    /// <param name="variant">
    ///     A pointer to a <c>VARIANT</c> struct that will be initialized as a projection.
    /// </param>
    STDAPI_(JsErrorCode)
        JsValueToVariant(
        _In_ JsValueRef object,
        _Out_ VARIANT *variant);

    /// <summary>
    ///     Creates a JavaScript value that is a projection of the passed in <c>IInspectable</c> pointer.
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     The projected value can be used by script to call an IInspectable object.
    ///     Hosts are responsible for enforcing COM threading rules.
    ///     </para>
    ///     <para>
    ///     Requires an active script context.
    ///     </para>
    /// </remarks>
    /// <param name="inspectable">A <c>IInspectable</c> to be projected.</param>
    /// <param name="value">A JavaScript value that is a projection of the <c>IInspectable</c>.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsInspectableToObject(
        _In_ IInspectable  *inspectable,
        _Out_ JsValueRef *value);

    /// <summary>
    ///     Unwraps a JavaScript object to an <c>IInspectable</c> pointer
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     Hosts are responsible for enforcing COM threading rules.
    ///     </para>
    ///     <para>
    ///     Requires an active script context.
    ///     </para>
    /// </remarks>
    /// <param name="value">A JavaScript value that should be projected to <c>IInspectable</c>.</param>
    /// <param name="inspectable">A <c>IInspectable</c> value of the object.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsObjectToInspectable(
        _In_ JsValueRef value,
        _Out_ IInspectable  **inspectable);

    /// <summary>
    ///     The context passed into application callback, JsProjectionEnqueueCallback, from Jsrt and
    ///     then passed back to Jsrt in the provided callback, JsProjectionCallback, by the application
    ///     on the correct thread.
    /// </summary>
    /// <remarks>
    ///     Requires calling JsSetProjectionEnqueueCallback to receive callbacks.
    /// </remarks>
    typedef void *JsProjectionCallbackContext;

    /// <summary>
    ///     The Jsrt callback which should be called with the context passed to JsProjectionEnqueueCallback on
    ///     the correct thread.
    /// </summary>
    /// <remarks>
    ///     Requires calling JsSetProjectionEnqueueCallback to receive callbacks.
    /// </remarks>
    /// <param name="jsContext">The context originally received by a call to JsProjectionEnqueueCallback.</param>
    typedef void (CALLBACK *JsProjectionCallback)(_In_ JsProjectionCallbackContext jsContext);

    /// <summary>
    ///     The application callback which is called by Jsrt when a projection API is completed on
    ///     a different thread than the original.
    /// </summary>
    /// <remarks>
    ///     Requires calling JsSetProjectionEnqueueCallback to receive callbacks.
    /// </remarks>
    /// <param name="jsCallback">The callback to be invoked on the original thread.</param>
    /// <param name="jsContext">The Jsrt context that must be passed into jsCallback.</param>
    /// <param name="callbackState">The state passed to <c>JsSetProjectionEnqueueCallback</c>.</param>
    typedef void (CALLBACK *JsProjectionEnqueueCallback)(_In_ JsProjectionCallback jsCallback, _In_ JsProjectionCallbackContext jsContext, _In_opt_ void *callbackState);

    /// <summary>
    ///     Sets the callback to be used in order to invoke a projection completion back to the
    ///     callers required thread.
    /// </summary>
    /// <remarks>
    ///     <para>Requires an active script context.</para>
    ///     <para>
    ///     The caller must be running in an different thread in the MTA or free threaded apartment.
    ///     When running on STA,COM manages the return to the required thread.
    ///     </para>
    /// </remarks>
    /// <param name="projectionEnqueueCallback">
    ///     The callback that will be invoked any time a projection completion occurs on a background thread.
    /// </param>
    /// <param name="callbackState">
    ///     User provided state that will be passed back to the callback.
    /// </param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsSetProjectionEnqueueCallback(
        _In_ JsProjectionEnqueueCallback projectionEnqueueCallback,
        _In_opt_ void *callbackState);

    /// <summary>
    ///     Project a UWP namespace.
    /// </summary>
    /// <remarks>
    ///     <para>Requires an active script context.</para>
    ///     <para>WinRT was the platform name before Universal Windows Platform (UWP).</para>
    /// </remarks>
    /// <param name="namespaceName">The UWP namespace to be projected.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsProjectWinRTNamespace(
        _In_z_ const wchar_t *namespaceName);

#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#pragma endregion

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include <activprof.h>

    /// <summary>
    ///     Starts profiling in the current context.
    /// </summary>
    /// <remarks>
    ///     Requires an active script context.
    /// </remarks>
    /// <param name="callback">The profiling callback to use.</param>
    /// <param name="eventMask">The profiling events to callback with.</param>
    /// <param name="context">A context to pass to the profiling callback.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsStartProfiling(
        _In_ IActiveScriptProfilerCallback *callback,
        _In_ PROFILER_EVENT_MASK eventMask,
        _In_ unsigned long context);

    /// <summary>
    ///     Stops profiling in the current context.
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     Will not return an error if profiling has not started.
    ///     </para>
    ///     <para>
    ///     Requires an active script context.
    ///     </para>
    /// </remarks>
    /// <param name="reason">
    ///     The reason for stopping profiling to pass to the profiler callback.
    /// </param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsStopProfiling(
        _In_ HRESULT reason);

    /// <summary>
    ///     Enumerates the heap of the current context.
    /// </summary>
    /// <remarks>
    ///     <para>
    ///     While the heap is being enumerated, the current context cannot be removed, and all calls to
    ///     modify the state of the context will fail until the heap enumerator is released.
    ///     </para>
    ///     <para>
    ///     Requires an active script context.
    ///     </para>
    /// </remarks>
    /// <param name="enumerator">The heap enumerator.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsEnumerateHeap(
        _Out_ IActiveScriptProfilerHeapEnum **enumerator);

    /// <summary>
    ///     Returns a value that indicates whether the heap of the current context is being enumerated.
    /// </summary>
    /// <remarks>
    ///     Requires an active script context.
    /// </remarks>
    /// <param name="isEnumeratingHeap">Whether the heap is being enumerated.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    STDAPI_(JsErrorCode)
        JsIsEnumeratingHeap(
        _Out_ bool *isEnumeratingHeap);

#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#pragma endregion

#endif // NTDDI_VERSION

#endif // _CHAKRART_H_
