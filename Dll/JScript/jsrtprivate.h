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
const enum JsErrorCode JsErrorNotDebuggingContext = (JsErrorCode)(JsErrorIdleNotEnabled + 7);

/// <summary>
///     Create runtime with IE12 version.
/// </summary>
const JsRuntimeVersion JsRuntimeVersion12 = (JsRuntimeVersion)2;

/// <summary>
///     Calling <c>JsSetException</c> will also dispatch the exception to the script debugger
///     (if any) giving the debugger a chance to break on the exception.
/// </summary>
const JsRuntimeAttributes JsRuntimeDispatchSetExceptionsToDebugger = (JsRuntimeAttributes)0x00000020;

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
typedef enum JsPropertyAttributes
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
};

/// <summary>
///     A callback to move a property enumerator to the next property.
/// </summary>
/// <param name="data">
///     The external data that was passed in when creating the property enumerator.
/// </param>
/// <returns><c>true</c> if there is another property, <c>false</c> otherwise.</returns>
typedef bool (CALLBACK *JsMoveNextCallback)(_In_opt_ void *data);

/// <summary>
///     A callback to get the name or symbol of the current property in the property
///     enumerator.
/// </summary>
/// <param name="data">
///     The external data that was passed in when creating the property enumerator.
/// </param>
/// <returns>The name or symbol of the current property.</returns>
typedef JsValueRef(CALLBACK *JsGetCurrentCallback)(_In_opt_ void *data);

/// <summary>
///     A callback that indicates property enumeration has ended.
/// </summary>
/// <param name="data">
///     The external data that was passed in when creating the property enumerator.
/// </param>
typedef void (CALLBACK *JsEndEnumerationCallback)(_In_opt_ void *data);

/// <summary>
///     A callback to create a property enumerator for the external object.
/// </summary>
/// <remarks>
///     <para>
///     If the callback handles the request, then all of the callbacks must be returned.
///     </para>
///     <para>
///     Properties indexed by name and by symbol should both be returned from the
///     enumerator. The runtime will filter the result list based on the request type.
///     </para>
/// </remarks>
/// <param name="object">The external object whose properties are to be enumerated.</param>
/// <param name="includeNonEnumerable">
///     Whether the enumeration should include non-enumerable properties.
/// </param>
/// <param name="moveNext">A callback that moves the enumeration forward.</param>
/// <param name="getCurrent">A callback that gets the current property name.</param>
/// <param name="endEnumeration">A callback that ends the enumeration.</param>
/// <param name="data">Data that represents the enumerator. May be null.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsEnumeratePropertiesCallback)(_In_ JsValueRef object, _In_ bool includeNonEnumerable, _Out_ JsMoveNextCallback *moveNext, _Out_ JsGetCurrentCallback *getCurrent, _Out_ JsEndEnumerationCallback *endEnumeration, _Out_opt_ void **data);

/// <summary>
///     A callback to query the attributes of an external object's property.
/// </summary>
/// <remarks>
///     If the callback wishes to indicate the property does not exist, it should return
///     <c>true</c> with a value of <c>JsPropertyAttributeInvalid</c>.
/// </remarks>
/// <param name="object">The external object whose property is to be queried.</param>
/// <param name="propertyId">The property ID of the property name being queried.</param>
/// <param name="result">The attributes of the property, if the query is handled.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsNamedPropertyQueryCallback)(_In_ JsValueRef object, _In_ JsPropertyIdRef propertyId, _Out_ JsPropertyAttributes *result);

/// <summary>
///     A callback to query the value of an external object's property.
/// </summary>
/// <remarks>
///     If the callback wishes to indicate the property does not exist, it should return
///     JS_INVALID_REFERENCE as the result.
/// <param name="object">The external object whose property is to be retrieved.</param>
/// <param name="propertyId">The property ID of the property name being retrieved.</param>
/// <param name="result">The value of the property, if the query is handled.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsNamedPropertyGetCallback)(_In_ JsValueRef object, _In_ JsPropertyIdRef propertyId, _Out_ JsValueRef *result);

/// <summary>
///     A callback to set the value or attributes of an external object's property.
/// </summary>
/// <remarks>
///     The callback can set both the value and attributes of a property, or only one of the two.
///     If only the value is being set, the attributes will be <c>JsPropertyAttributeInvalid</c>.
///     If only the attributes are being set, the value will be <c>JS_INVALID_REFERENCE</c>.
/// </remarks>
/// <param name="object">The external object whose property is to be set.</param>
/// <param name="propertyId">The property ID of the property name being set.</param>
/// <param name="attributes">
///     The new attributes of the property, if the query is handled. If the attributes are not
///     being set, this will be <c>JsPropertyAttributeInvalid</c>.
/// </param>
/// <param name="value">
///     The new value of the property, if the query is handled. If the value is not being set, this
///     will be <c>JS_INVALID_REFERENCE</c>.
/// </param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsNamedPropertySetCallback)(_In_ JsValueRef object, _In_ JsPropertyIdRef propertyId, _In_ JsPropertyAttributes attributes, _In_opt_ JsValueRef value);

/// <summary>
///     A callback to delete an external object's property.
/// </summary>
/// <param name="object">The external object whose property is to be deleted.</param>
/// <param name="propertyId">The property ID of the property name being deleted.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsNamedPropertyDeleteCallback)(_In_ JsValueRef object, _In_ JsPropertyIdRef propertyId);

/// <summary>
///     A callback to query the attributes of an external object's indexed property.
/// </summary>
/// <remarks>
///     If the callback wishes to indicate the property does not exist, it should return
///     <c>true</c> with a value of <c>JsPropertyAttributeInvalid</c>.
/// </remarks>
/// <param name="object">The external object whose property is to be queried.</param>
/// <param name="index">The index being queried.</param>
/// <param name="result">The attributes of the property, if the query is handled.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsIndexedPropertyQueryCallback)(_In_ JsValueRef object, _In_ unsigned int index, _Out_ JsPropertyAttributes *result);

/// <summary>
///     A callback to query the value of an external object's indexed property.
/// </summary>
/// <param name="object">The external object whose property is to be retrieved.</param>
/// <param name="index">The index being retrieved.</param>
/// <param name="result">The value of the property, if the query is handled.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsIndexedPropertyGetCallback)(_In_ JsValueRef object, _In_ unsigned index, _Out_ JsValueRef *result);

/// <summary>
///     A callback to set the value of an external object's indexed property.
/// </summary>
/// <param name="object">The external object whose property is to be set.</param>
/// <param name="index">The index being set.</param>
/// <param name="value">The new value of the property, if the query is handled.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsIndexedPropertySetCallback)(_In_ JsValueRef object, _In_ unsigned index, _In_ JsValueRef value);

/// <summary>
///     A callback to delete an external object's indexed property.
/// </summary>
/// <param name="object">The external object whose property is to be deleted.</param>
/// <param name="index">The index being deleted.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsIndexedPropertyDeleteCallback)(_In_ JsValueRef object, _In_ unsigned index);

/// <summary>
///     A callback to determine equality between two values.
/// </summary>
/// <param name="object">The external object which is being compared.</param>
/// <param name="other">The object to compare it to.</param>
/// <param name="result"><c>true</c> if the objects are equal, <c>false</c> otherwise.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsEqualsCallback)(_In_ JsValueRef object, _In_ JsValueRef other, _Out_ bool *result);

/// <summary>
///     A callback to determine strict equality between two values.
/// </summary>
/// <param name="object">The external object which is being compared.</param>
/// <param name="other">The object to compare it to.</param>
/// <param name="result"><c>true</c> if the objects are equal, <c>false</c> otherwise.</param>
/// <returns>
///     <c>true</c> if the callback handled the request, <c>false</c> if the default object handler
///     should be used.
/// </returns>
typedef bool (CALLBACK *JsStrictEqualsCallback)(_In_ JsValueRef object, _In_ JsValueRef other, _Out_ bool *result);

/// <summary>
///     A finalizer callback for an object with an external type.
/// </summary>
/// <param name="object">The external object which is being finalized.</param>
typedef void (CALLBACK *JsTypedFinalizeCallback)(_In_ JsValueRef object);

/// <summary>
///     Version of the type description.
/// </summary>
typedef enum JsExternalTypeDescriptionVersion
{
    /// <summary>
    ///     A type description compatible with IE12.
    /// </summary>
    JsTypeDescriptionVersion12 = 12,
};

/// <summary>
///     A description of an external JavaScript type.
/// </summary>
struct JsExternalTypeDescription {
    /// <summary>
    ///     The version of the type description.
    /// </summary>
    JsExternalTypeDescriptionVersion version;

    /// <summary>
    ///     The name of the class.
    /// </summary>
    /// <remarks>
    ///     If <c>JS_INVALID_REFERENCE</c> is specified, the default class name will be used.
    /// </remarks>
    JsPropertyIdRef className;

    /// <summary>
    ///     The prototype of the class.
    /// </summary>
    /// <remarks>
    ///     If <c>JS_INVALID_REFERENCE</c> is specified, the default prototype will be used.
    /// </remarks>
    JsValueRef prototype;

    /// <summary>
    ///     A callback for property enumeration. Can be null.
    /// </summary>
    JsEnumeratePropertiesCallback enumerateCallback;

    /// <summary>
    ///     A callback for property attributes. Can be null.
    /// </summary>
    JsNamedPropertyQueryCallback queryCallback;

    /// <summary>
    ///     A callback for property value retrieval. Can be null.
    /// </summary>
    JsNamedPropertyGetCallback getCallback;

    /// <summary>
    ///     A callback for property value assignment or changing property attributes. Can be
    ///     null.
    /// </summary>
    JsNamedPropertySetCallback setCallback;

    /// <summary>
    ///     A callback for property deletion. Can be null.
    /// </summary>
    JsNamedPropertyDeleteCallback deleteCallback;

    /// <summary>
    ///     A callback for indexed property attributes. Can be null.
    /// </summary>
    JsIndexedPropertyQueryCallback queryIndexedCallback;

    /// <summary>
    ///     A callback for indexed property value retrieval. Can be null.
    /// </summary>
    JsIndexedPropertyGetCallback getIndexedCallback;

    /// <summary>
    ///     A callback for indexed property value assignment. Can be null.
    /// </summary>
    JsIndexedPropertySetCallback setIndexedCallback;

    /// <summary>
    ///     A callback for indexed property deletion. Can be null.
    /// </summary>
    JsIndexedPropertyDeleteCallback deleteIndexedCallback;

    /// <summary>
    ///     A callback for equality comparison. Can be null.
    /// </summary>
    JsEqualsCallback equalsCallback;

    /// <summary>
    ///     A callback for strict equality comparison. Can be null.
    /// </summary>
    JsStrictEqualsCallback strictEqualsCallback;

    /// <summary>
    ///     A callback for object finalization. Can be null.
    /// </summary>
    JsTypedFinalizeCallback finalizeCallback;
};

/// <summary>
///     An external type.
/// </summary>
typedef JsRef JsExternalTypeRef;

/// <summary>
///     Creates a new external object type.
/// </summary>
/// <remarks>
///     Requires an active script context.
/// </remarks>
/// <param name="typeDescription">A description of the external type.</param>
/// <param name="type">The new external type.</param>
/// <returns>JsNoError if a value was returned, a failure code otherwise.</returns>
STDAPI_(JsErrorCode)
JsCreateExternalType(
_In_ JsExternalTypeDescription *typeDescription,
_Out_ JsExternalTypeRef *type);

/// <summary>
///     Creates a typed external <c>Object</c>.
/// </summary>
/// <remarks>
///     Requires an active script context.
/// </remarks>
/// <param name="type">The external type of the object.</param>
/// <param name="data">External data to store in the object. May be null.</param>
/// <param name="object">The new <c>Object</c>.</param>
/// <returns>JsNoError if a value was returned, a failure code otherwise.</returns>
STDAPI_(JsErrorCode)
JsCreateTypedExternalObject(
_In_ JsExternalTypeRef type,
_In_opt_ void *data,
_Out_ JsValueRef *object);

/// <summary>
///     Returns the external type, if any, of an object.
/// </summary>
/// <remarks>
///     Requires an active script context.
/// </remarks>
/// <param name="object">The object whose external type is to be returned.</param>
/// <param name="type">
///     The object's external type. If the object has no external type, this will be
///     <c>JS_INVALID_REFERENCE</c>.
/// </param>
/// <returns>JsNoError if a value was returned, a failure code otherwise.</returns>
STDAPI_(JsErrorCode)
JsGetExternalType(
_In_ JsValueRef object,
_Out_ JsExternalTypeRef *type);

/// <summary>
///     Returns a type description of the default operators.
/// </summary>
/// <remarks>
///     The default type description can be used in type callbacks to perform operations on
///     objects without recursing into further type callbacks.
/// </remarks>
/// <param name="defaultDescription">The default type description.</param>
/// <returns>JsNoError if a value was returned, a failure code otherwise.</returns>
STDAPI_(JsErrorCode)
JsGetDefaultTypeDescription(
_Out_ const JsExternalTypeDescription **defaultDescription);

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
