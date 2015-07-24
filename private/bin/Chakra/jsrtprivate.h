// Copyright (C) Microsoft. All rights reserved.
#ifdef _MSC_VER
#pragma once
#endif  // _MSC_VER
#if NTDDI_VERSION >= NTDDI_WIN7

#include "jsrt.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \file
/// \brief The base Chakra hosting API. These are APIs that are not finalized to be published yet.
///
/// This file contains a flat C API layer. This is the API exported by chakra.dll.

    // TODO: this set of constants are moved from jsrt.h, JsErrorCode section, to jsrtprivate.h
    // becuase we don't want to release this new features in Threshold yet. At time of feature
    // release we should move these definitions back to jsrt.h
/// <summary>
///     The context cannot be taken out of a debug state because it is not in a debug state.
/// </summary>
const JsErrorCode JsErrorNotDebuggingContext = (JsErrorCode)(JsErrorIdleNotEnabled + 7);

STDAPI_(JsErrorCode)
JsSerializeNativeScript(
    _In_z_ const wchar_t *script,
    _In_reads_opt_(functionTableSize) BYTE *functionTable,
    _In_ int functionTableSize,
    _Out_writes_to_opt_(*bufferSize, *bufferSize) BYTE *buffer,
    _Inout_ unsigned long *bufferSize);

/// <summary>
///     The attributes of a property.
/// </summary>
typedef enum _JsPropertyAttributes
{
    /// <summary>
    ///     The property has no special attributes.
    /// </summary>
    JsPropertyAttributeNone = 0x00000000,
    /// <summary>
    ///     The property is enumerable.
    /// </summary>
    JsPropertyAttributeEnumerable = 0x00000001,
    /// <summary>
    ///     The property is configurable.
    /// </summary>
    JsPropertyAttributeConfigurable = 0x00000002,
    /// <summary>
    ///     The property is writable.
    /// </summary>
    JsPropertyAttributeWritable = 0x00000004,
    /// <summary>
    ///     The property cannot be redeclared. (Applies to <c>let</c> and <c>const</c> variables.)
    /// </summary>
    JsPropertyAttributeNoRedeclare = 0x00000040,
    /// <summary>
    ///     The property is a constant value. (Applies to <c>const</c> variables.)
    /// </summary>
    JsPropertyAttributeConst = 0x00000080,
    /// <summary>
    ///     An invalid property attribute.
    /// </summary>
    JsPropertyAttributeInvalid = 0xFFFFFFFF,
} JsPropertyAttributes;

/// <summary>
///     Stops debugging in the current context.
/// </summary>
/// <returns>
///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
/// </returns>
STDAPI_(JsErrorCode)
JsStopDebugging();

/// <summary>
///     A container that holds a weak reference to an object.
/// </summary>
/// <remarks>
///     <para>
///     A weak container holds a reference to an object but doesn't keep the object alive. The
///     host can query the container to see whether the reference is still alive, and can retrieve
///     the underlying reference if it is.
///     </para>
///     <para>
///     Note that the weak container is, itself, an object which is managed by the garbage
///     collector. As such, it can be collected if there are no outstanding references to it.
///     </para>
/// </remakrs>
typedef void *JsWeakContainerRef;

/// <summary>
///     Creates a weak reference container.
/// </summary>
/// <param name="ref">The object to create a weak container reference for.</param>
/// <param name="weakContainerRef">The new weak reference container.</param>
/// <returns>
///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
/// </returns>
STDAPI_(JsErrorCode)
JsCreateWeakContainer(
_In_ JsRef ref,
_Out_ JsWeakContainerRef *weakContainerRef);

/// <summary>
///     Returns a value that indicates whether the reference in the container is still valid.
/// </summary>
/// <param name="weakContainerRef">The weak reference container.</param>
/// <param name="isValid">If reference is still valid, <c>true</c>, <c>false</c> otherwise.</param>
/// <returns>
///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
/// </returns>
STDAPI_(JsErrorCode)
JsIsReferenceValid(
_In_ JsWeakContainerRef weakContainerRef,
_Out_ bool *isValid);

/// <summary>
///     Gets the value in the container.
/// </summary>
/// <param name="weakContainerRef">The weak reference container.</param>
/// <param name="ref">
///     If the reference is still valid, the reference in the container,
///     <c>JS_INVALID_REFERENCE</c> otherwise.
/// </param>
/// <returns>
///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
/// </returns>
STDAPI_(JsErrorCode)
JsGetReference(
_In_ JsWeakContainerRef weakContainerRef,
_Out_ JsRef *ref);


/// <summary>
///     Gets the length of an array value.
/// </summary>
/// <remarks>
///     Requires an active script context.
/// </remarks>
/// <param name="arrayValue">The array value to get the length of.</param>
/// <param name="length">The length of the array.</param>
/// <returns>
///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
/// </returns>
STDAPI_(JsErrorCode)
JsGetArrayLength(
_In_ JsValueRef arrayValue,
_Out_ size_t *length);

/// <summary>
///     Collect garbage without scanning the stack for roots. Private API for testing.
/// </summary>
STDAPI_(JsErrorCode)
  JsPrivateCollectGarbageSkipStack(
  _In_ JsRuntimeHandle runtime);

#ifdef __cplusplus
};
#endif

#endif
