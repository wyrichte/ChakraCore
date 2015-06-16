//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#define InlineSlotCountIncrement (HeapConstants::ObjectGranularity / sizeof(Var))

#define MaxPreInitializedObjectTypeInlineSlotCount 16
#define MaxPreInitializedObjectHeaderInlinedTypeInlineSlotCount \
    (Js::DynamicTypeHandler::GetObjectHeaderInlinableSlotCapacity() + MaxPreInitializedObjectTypeInlineSlotCount)
#define PreInitializedObjectTypeCount ((MaxPreInitializedObjectTypeInlineSlotCount / InlineSlotCountIncrement) + 1)
CompileAssert(MaxPreInitializedObjectTypeInlineSlotCount <= USHRT_MAX);

class ScriptSite;

#ifdef ENABLE_PROJECTION
namespace Projection
{
    class ProjectionContext;
}
#endif

namespace Authoring
{
    class JavascriptLibraryAccessor;
}

namespace Js
{
    typedef RecyclerFastAllocator<JavascriptNumber, LeafBit> RecyclerJavascriptNumberAllocator;

    class UndeclaredBlockVariable : public RecyclableObject
    {
        friend class JavascriptLibrary;
        UndeclaredBlockVariable(Type* type) : RecyclableObject(type) { }
    };

    struct CacheForCopyOnAccessArraySegments
    {
        static const uint32 MAX_SIZE = 31;
        SparseArraySegment<int32> *cache[MAX_SIZE];
        int count;

        uint32 AddSegment(SparseArraySegment<int32> *segment)
        {
            cache[count++] = segment;
            return count;
        }

        SparseArraySegment<int32> *GetSegmentByIndex(byte index)
        {
            Assert(index <= MAX_SIZE);
            return cache[index - 1];
        }

        bool IsNotOverHardLimit()
        {
            return count < MAX_SIZE;
        }

        bool IsNotFull()
        {
            return count < CONFIG_FLAG(CopyOnAccessArraySegmentCacheSize);
        }

        bool IsValidIndex(int index)
        {
            return count && index && index <= count;
        }

#if ENABLE_DEBUG_CONFIG_OPTIONS
        uint32 GetCount()
        {
            return count;
        }
#endif
    };

    template <typename T>
    struct StringTemplateCallsiteObjectComparer
    {
        static bool Equals(T x, T y)
        {
            static_assert(false, "Unexpected type T");
        }
        static hash_t GetHashCode(T i)
        {
            static_assert(false, "Unexpected type T");
        }
    };

    template <>
    struct StringTemplateCallsiteObjectComparer<ParseNodePtr>
    {
        static bool Equals(ParseNodePtr x, RecyclerWeakReference<Js::RecyclableObject>* y);
        static bool Equals(ParseNodePtr x, ParseNodePtr y);
        static hash_t GetHashCode(ParseNodePtr i);
    };

    template <>
    struct StringTemplateCallsiteObjectComparer<RecyclerWeakReference<Js::RecyclableObject>*>
    {
        static bool Equals(RecyclerWeakReference<Js::RecyclableObject>* x, RecyclerWeakReference<Js::RecyclableObject>* y);
        static bool Equals(RecyclerWeakReference<Js::RecyclableObject>* x, ParseNodePtr y);
        static hash_t GetHashCode(RecyclerWeakReference<Js::RecyclableObject>* o);
    };

    class JavascriptLibrary : public JavascriptLibraryBase
    {
        friend class ScriptSite;
        friend class GlobalObject;
        friend class ScriptContext;
        friend class EngineInterfaceObject;
        friend class Authoring::JavascriptLibraryAccessor;
#ifdef ENABLE_PROJECTION
        friend class Projection::ProjectionContext;
#endif
        static const wchar_t* domBuiltinPropertyNames[];

    public:
        CacheForCopyOnAccessArraySegments *cacheForCopyOnAccessArraySegments;

        static DWORD GetScriptContextOffset() { return offsetof(JavascriptLibrary, scriptContext); }
        static DWORD GetUndeclBlockVarOffset() { return offsetof(JavascriptLibrary, undeclBlockVarSentinel); }
        static DWORD GetEmptyStringOffset() { return offsetof(JavascriptLibrary, emptyString); }
        static DWORD GetUndefinedValueOffset() { return offsetof(JavascriptLibrary, undefinedValue); }
        static DWORD GetNullValueOffset() { return offsetof(JavascriptLibrary, nullValue); }
        static DWORD GetBooleanTrueOffset() { return offsetof(JavascriptLibrary, booleanTrue); }
        static DWORD GetBooleanFalseOffset() { return offsetof(JavascriptLibrary, booleanFalse); }
        static DWORD GetNegativeZeroOffset() { return offsetof(JavascriptLibrary, negativeZero); }
        static DWORD GetNumberTypeStaticOffset() { return offsetof(JavascriptLibrary, numberTypeStatic); }
        static DWORD GetStringTypeStaticOffset() { return offsetof(JavascriptLibrary, stringTypeStatic); }
        static DWORD GetObjectTypesOffset() { return offsetof(JavascriptLibrary, objectTypes); }
        static DWORD GetObjectHeaderInlinedTypesOffset() { return offsetof(JavascriptLibrary, objectHeaderInlinedTypes); }
        static DWORD GetRegexTypeOffset() { return offsetof(JavascriptLibrary, regexType); }
        static DWORD GetArrayConstructorOffset() { return offsetof(JavascriptLibrary, arrayConstructor); }
        static DWORD GetPositiveInfinityOffset() { return offsetof(JavascriptLibrary, positiveInfinite); }
        static DWORD GetNaNOffset() { return offsetof(JavascriptLibrary, nan); }
        static DWORD GetNativeIntArrayTypeOffset() { return offsetof(JavascriptLibrary, nativeIntArrayType); }
        static DWORD GetCopyOnAccessNativeIntArrayTypeOffset() { return offsetof(JavascriptLibrary, copyOnAccessNativeIntArrayType); }
        static DWORD GetNativeFloatArrayTypeOffset() { return offsetof(JavascriptLibrary, nativeFloatArrayType); }
        static DWORD GetVTableAddressesOffset() { return offsetof(JavascriptLibrary, vtableAddresses); }
        static DWORD GetConstructorCacheDefaultInstanceOffset() { return offsetof(JavascriptLibrary, constructorCacheDefaultInstance); }
        static DWORD GetAbsDoubleCstOffset() { return offsetof(JavascriptLibrary, absDoubleCst); }
        static DWORD GetUintConvertConstOffset() { return offsetof(JavascriptLibrary, uintConvertConst); }
        static DWORD GetBuiltinFunctionsOffset() { return offsetof(JavascriptLibrary, builtinFunctions); }
        static DWORD GetJnHelperMethodsOffset() { return offsetof(JavascriptLibrary, jnHelperMethods); }
        static DWORD GetCharStringCacheOffset() { return offsetof(JavascriptLibrary, charStringCache); }
        static DWORD GetCharStringCacheAOffset() { return GetCharStringCacheOffset() + CharStringCache::GetCharStringCacheAOffset(); }
        const  JavascriptLibraryBase* GetLibraryBase() const { return static_cast<const JavascriptLibraryBase*>(this); }
        void SetGlobalObject(GlobalObject* globalObject) {globalObject = globalObject; }
        static const unsigned int DOM_BUILTIN_MAX_SLOT_COUNT  = 100;

        typedef bool (CALLBACK *PromiseContinuationCallback)(Var task, void *callbackState);

        Var GetUndeclBlockVar() const { return undeclBlockVarSentinel; }
        bool IsUndeclBlockVar(Var var) const { return var == undeclBlockVarSentinel; }

    private:
        Recycler * recycler;

        UndeclaredBlockVariable* undeclBlockVarSentinel;

        DynamicType * generatorConstructorPrototypeObjectType;
        DynamicType * constructorPrototypeObjectType;
        DynamicType * heapArgumentsType;
        DynamicType * activationObjectType;
        DynamicType * arrayType;
        DynamicType * nativeIntArrayType;
        DynamicType * copyOnAccessNativeIntArrayType;
        DynamicType * nativeFloatArrayType;
        DynamicType * arrayBufferType;
        DynamicType * dataViewType;
        DynamicType * typedArrayType;
        DynamicType * int8ArrayType;
        DynamicType * uint8ArrayType;
        DynamicType * uint8ClampedArrayType;
        DynamicType * int16ArrayType;
        DynamicType * uint16ArrayType;
        DynamicType * int32ArrayType;
        DynamicType * uint32ArrayType;
        DynamicType * float32ArrayType;
        DynamicType * float64ArrayType;
        DynamicType * int64ArrayType;
        DynamicType * uint64ArrayType;
        DynamicType * boolArrayType;
        DynamicType * charArrayType;
        DynamicType * pixelArrayType;
        StaticType * booleanTypeStatic;
        DynamicType * booleanTypeDynamic;
        DynamicType * dateType;
        DynamicType * winrtDateType;
        StaticType * variantDateType;
        DynamicType * symbolTypeDynamic;
        StaticType * symbolTypeStatic;
        DynamicType * iteratorResultType;
        DynamicType * arrayIteratorType;
        DynamicType * mapIteratorType;
        DynamicType * setIteratorType;
        DynamicType * stringIteratorType;
        DynamicType * promiseType;
        DynamicType * javascriptEnumeratorIteratorType;

        JavascriptFunction* builtinFunctions[BuiltinFunction::Count];

        INT_PTR vtableAddresses[VTableValue::Count];
        ConstructorCache *constructorCacheDefaultInstance;
        __declspec(align(16)) const BYTE *absDoubleCst;
        double const *uintConvertConst;
        const void *const*jnHelperMethods;

        // Function Types
        DynamicTypeHandler * functionTypeHandler;
        DynamicTypeHandler * functionWithPrototypeTypeHandler;
        DynamicType * externalFunctionWithDeferredPrototypeType;
        DynamicType * wrappedFunctionWithDeferredPrototypeType;
        DynamicType * stdCallFunctionWithDeferredPrototypeType;
        DynamicType * idMappedFunctionWithPrototypeType;
        DynamicType * externalConstructorFunctionWithDeferredPrototypeType;
        DynamicType * boundFunctionType;
        DynamicType * typedObjectSlotGetterFunctionTypes[DOM_BUILTIN_MAX_SLOT_COUNT];
        DynamicType * typedObjectSlotSetterFunctionTypes[DOM_BUILTIN_MAX_SLOT_COUNT];
        DynamicType * regexConstructorType;
        DynamicType * crossSiteDeferredPrototypeFunctionType;
        DynamicType * crossSiteIdMappedFunctionWithPrototypeType;
        DynamicType * crossSiteExternalConstructFunctionWithPrototypeType;

        StaticType  * enumeratorType;
        DynamicType * errorType;
        DynamicType * evalErrorType;
        DynamicType * rangeErrorType;
        DynamicType * referenceErrorType;
        DynamicType * syntaxErrorType;
        DynamicType * typeErrorType;
        DynamicType * uriErrorType;
        DynamicType * winrtErrorType;
        StaticType  * numberTypeStatic;
        StaticType  * int64NumberTypeStatic;
        StaticType  * uint64NumberTypeStatic;

#ifdef SIMD_JS_ENABLED
        // SIMD
        StaticType * simdFloat32x4TypeStatic;
        StaticType * simdInt32x4TypeStatic;
        StaticType * simdFloat64x2TypeStatic;
#endif

        DynamicType * numberTypeDynamic;
        DynamicType * objectTypes[PreInitializedObjectTypeCount];
        DynamicType * objectHeaderInlinedTypes[PreInitializedObjectTypeCount];
        DynamicType * regexType;
        DynamicType * regexResultType;
        StaticType  * stringTypeStatic;
        DynamicType * stringTypeDynamic;
        DynamicType * mapType;
        DynamicType * setType;
        DynamicType * weakMapType;
        DynamicType * weakSetType;
        DynamicType * proxyType;
        StaticType  * withType;
        DynamicType * SpreadArgumentType;
        PropertyDescriptor defaultPropertyDescriptor;

        JavascriptString* nullString;
        JavascriptString* emptyString;
        JavascriptString* quotesString;
        JavascriptString* whackString;
        JavascriptString* objectDisplayString;
        JavascriptString* stringTypeDisplayString;
        JavascriptString* errorDisplayString;
        JavascriptString* functionPrefixString;
        JavascriptString* generatorFunctionPrefixString;
        JavascriptString* functionDisplayString;
        JavascriptString* xDomainFunctionDisplayString;
        JavascriptString* undefinedDisplayString;
        JavascriptString* nanDisplayString;
        JavascriptString* nullDisplayString;
        JavascriptString* unknownDisplayString;
        JavascriptString* commaDisplayString;
        JavascriptString* commaSpaceDisplayString;
        JavascriptString* trueDisplayString;
        JavascriptString* falseDisplayString;
        JavascriptString* lengthDisplayString;
        JavascriptString* invalidDateString;
        JavascriptString* objectTypeDisplayString;
        JavascriptString* functionTypeDisplayString;
        JavascriptString* booleanTypeDisplayString;
        JavascriptString* numberTypeDisplayString;

#ifdef SIMD_JS_ENABLED
        // SIMD
        JavascriptString* simdFloat32x4DisplayString;
        JavascriptString* simdFloat64x2DisplayString;
        JavascriptString* simdInt32x4DisplayString;
#endif

        JavascriptString* symbolTypeDisplayString;
        JavascriptString* debuggerDeadZoneBlockVariableString;

        DynamicType * extensionEnumeratorType;
        DynamicType * moduleRootType;

        StaticType * dispMemberProxyType;
        StaticType * hostDispatchType;
        DynamicType * hostObjectType;
        DynamicObject* missingPropertyHolder;

        StaticType* throwErrorObjectType;

        NullEnumerator *nullEnumerator;

        PropertyStringCacheMap* propertyStringMap;
        RecyclerWeakReference<ForInObjectEnumerator>*  cachedForInEnumerator;

        ConstructorCache* builtInConstructorCache;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        JavascriptFunction* debugObjectFaultInjectionCookieGetterFunction;
        JavascriptFunction* debugObjectFaultInjectionCookieSetterFunction;
#endif

        JavascriptFunction* evalFunctionObject;
        JavascriptFunction* arrayPrototypeValuesFunction;
        JavascriptFunction* parseIntFunctionObject;
        JavascriptFunction* parseFloatFunctionObject;
        JavascriptFunction* arrayPrototypeToStringFunction;
        JavascriptFunction* arrayPrototypeToLocaleStringFunction;
        JavascriptFunction* identityFunction;
        JavascriptFunction* throwerFunction;
        JavascriptFunction* hostPromiseContinuationFunction;

        JavascriptFunction* objectValueOfFunction;
        JavascriptFunction* objectToStringFunction;

#ifdef SIMD_JS_ENABLED
        // SIMD
        // Float32x4
        JavascriptFunction* simdFloat32x4ToStringFunction;
        // Float64x2
        JavascriptFunction* simdFloat64x2ToStringFunction;
        // Int32x4
        JavascriptFunction* simdInt32x4ToStringFunction;
#endif

        mutable CharStringCache charStringCache;

        PromiseContinuationCallback nativeHostPromiseContinuationFunction;
        void *nativeHostPromiseContinuationFunctionState;

        typedef JsUtil::BaseHashSet<RecyclerWeakReference<RecyclableObject>*, Recycler, PowerOf2SizePolicy, RecyclerWeakReference<RecyclableObject>*, StringTemplateCallsiteObjectComparer> StringTemplateCallsiteObjectList;

        // Used to store a list of template callsite objects.
        // We use the raw strings in the callsite object (or a string template parse node) to identify unique callsite objects in the list.
        // See abstract operation GetTemplateObject in ES6 Spec (RC1) 12.2.8.3
        StringTemplateCallsiteObjectList* stringTemplateCallsiteObjectList;

        // This list contains types ensured to have only writable data properties in it and all objects in its prototype chain
        // (i.e., no readonly properties or accessors). Only prototype objects' types are stored in the list. When something
        // in the script context adds a readonly property or accessor to an object that is used as a prototype object, this
        // list is cleared. The list is also cleared before garbage collection so that it does not keep growing, and so, it can
        // hold strong references to the types.
        //
        // The cache is used by the type-without-property local inline cache. When setting a property on a type that doens't
        // have the property, to determine whether to promote the object like an object of that type was last promoted, we need
        // to ensure that objects in the prototype chain have not acquired a readonly property or setter (ideally, only for that
        // property ID, but we just check for any such property). This cache is used to avoid doing this many times, especially
        // when the prototype chain is not short.
        //
        // This list is only used to invalidate the status of types. The type itself contains a boolean indicating whether it
        // and prototypes contain only writable data properties, which is reset upon invalidating the status.
        JsUtil::List<Type *> *typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain;

        uint64 randSeed;
        bool inProfileMode;
        bool inDispatchProfileMode;

        JavascriptFunction * AddFunctionToLibraryObjectWithPrototype(GlobalObject * globalObject, PropertyId propertyId, FunctionInfo * functionInfo, int length, DynamicObject * prototype = null, DynamicType * functionType = null);

        JavascriptFunction * AddFunctionToLibraryObject(DynamicObject* object, PropertyId propertyId, FunctionInfo * functionInfo, int length, PropertyAttributes attributes = PropertyBuiltInMethodDefaults);
        JavascriptFunction * AddFunctionToLibraryObjectWithName(DynamicObject* object, PropertyId propertyId, PropertyId nameId, FunctionInfo * functionInfo, int length);
        void AddAccessorsToLibraryObject(DynamicObject* object, PropertyId propertyId, FunctionInfo * getterFunctionInfo, FunctionInfo * setterFunctionInfo);
        void AddAccessorsToLibraryObjectWithName(DynamicObject* object, PropertyId propertyId, PropertyId nameId, FunctionInfo * getterFunctionInfo, FunctionInfo * setterFunctionInfo);

        template <size_t N>
        JavascriptFunction * AddFunctionToLibraryObjectWithPropertyName(DynamicObject* object, const wchar_t(&propertyName)[N], FunctionInfo * functionInfo, int length);

        bool isHybridDebugging; // If this library is in hybrid debugging mode
        bool isLibraryReadyForHybridDebugging; // If this library is ready for hybrid debugging (library objects using deferred type handler have been un-deferred)

        bool IsHybridDebugging() const { return isHybridDebugging; }
        void EnsureLibraryReadyForHybridDebugging();
        DynamicObject* EnsureReadyIfHybridDebugging(DynamicObject* obj);
        template <class T> T* EnsureReadyIfHybridDebugging(T* obj)
        {
            return (T*)EnsureReadyIfHybridDebugging((DynamicObject*)obj);
        }

        static SimpleTypeHandler<1> SharedPrototypeTypeHandler;
        static SimpleTypeHandler<1> SharedFunctionWithoutPrototypeTypeHandler;
        static SimpleTypeHandler<1> SharedFunctionWithPrototypeTypeHandlerV11;
        static SimpleTypeHandler<2> SharedFunctionWithPrototypeTypeHandler;
        static SimpleTypeHandler<1> SharedFunctionWithLengthTypeHandler;
        static SimpleTypeHandler<1> SharedIdMappedFunctionWithPrototypeTypeHandler;
        static MissingPropertyTypeHandler MissingPropertyHolderTypeHandler;

        static SimplePropertyDescriptor SharedFunctionPropertyDescriptors[2];
        static SimplePropertyDescriptor HeapArgumentsPropertyDescriptorsV11[2];
        static SimplePropertyDescriptor HeapArgumentsPropertyDescriptors[3];
        static SimplePropertyDescriptor FunctionWithLengthAndPrototypeTypeDescriptors[2];

        void CopyOnWriteSpecialGlobals(ScriptContext * scriptContext, GlobalObject * globalObject, JavascriptLibrary * originalLibrary);
        void CopyOnWriteGlobal(ScriptContext * scriptContext, GlobalObject * globalObject, JavascriptLibrary * originalLibrary);

    public:
        static const ObjectInfoBits EnumFunctionClass = EnumClass_1_Bit;

        static void InitializeProperties(ThreadContext * threadContext);

        JavascriptLibrary(GlobalObject* globalObject) :
                              JavascriptLibraryBase(globalObject),
                              inProfileMode(false),
                              inDispatchProfileMode(false),
                              propertyStringMap(nullptr),
                              parseIntFunctionObject(nullptr),
                              evalFunctionObject(nullptr),
                              parseFloatFunctionObject(nullptr),
                              arrayPrototypeToLocaleStringFunction(nullptr),
                              arrayPrototypeToStringFunction(nullptr),
                              identityFunction(nullptr),
                              throwerFunction(nullptr),
                              cachedForInEnumerator(nullptr),
                              cacheForCopyOnAccessArraySegments(nullptr),
#ifdef ENABLE_PROJECTION
                              pfArrayBufferFromExternalObject(nullptr),
#endif
                              isHybridDebugging(false),
                              isLibraryReadyForHybridDebugging(false),
                              referencedPropertyRecords(nullptr),
                              stringTemplateCallsiteObjectList(nullptr)
        {
            globalObject = globalObject;
        }

        void Initialize(ScriptContext* scriptContext, GlobalObject * globalObject);
        void Uninitialize();
        GlobalObject* GetGlobalObject() const { return globalObject; }
        ScriptContext* GetScriptContext() const { return scriptContext; }

        void CopyOnWriteFrom(ScriptContext * scriptContext, GlobalObject * globalObject, JavascriptLibrary * originalLibrary);

        Recycler * GetRecycler() const { return recycler; }
        Var GetPI() { return pi; }
        Var GetNaN() { return nan; }
        Var GetNegativeInfinite() { return negativeInfinite; }
        Var GetPositiveInfinite() { return positiveInfinite; }
        Var GetMaxValue() { return maxValue; }
        Var GetMinValue() { return minValue; }
        Var GetNegativeZero() { return negativeZero; }
        RecyclableObject* GetUndefined() { return undefinedValue; }
        RecyclableObject* GetNull() { return nullValue; }
        JavascriptBoolean* GetTrue() { return booleanTrue; }
        JavascriptBoolean* GetFalse() { return booleanFalse; }
        Var GetTrueOrFalse(BOOL value) { return value ? booleanTrue : booleanFalse; }
        JavascriptSymbol* GetSymbolHasInstance() { return symbolHasInstance; }
        JavascriptSymbol* GetSymbolIsConcatSpreadable() { return symbolIsConcatSpreadable; }
        JavascriptSymbol* GetSymbolIterator() { return symbolIterator; }
        JavascriptSymbol* GetSymbolSpecies() { return symbolSpecies; }
        JavascriptSymbol* GetSymbolToPrimitive() { return symbolToPrimitive; }
        JavascriptSymbol* GetSymbolToStringTag() { return symbolToStringTag; }
        JavascriptSymbol* GetSymbolUnscopables() { return symbolUnscopables; }
        JavascriptString* GetNullString() { return nullString; }
        JavascriptString* GetEmptyString() const;
        JavascriptString* GetWhackString() { return whackString; }
        JavascriptString* GetUndefinedDisplayString() { return undefinedDisplayString; }
        JavascriptString* GetNaNDisplayString() { return nanDisplayString; }
        JavascriptString* GetQuotesString() { return quotesString; }
        JavascriptString* GetNullDisplayString() { return nullDisplayString; }
        JavascriptString* GetUnknownDisplayString() { return unknownDisplayString; }
        JavascriptString* GetCommaDisplayString() { return commaDisplayString; }
        JavascriptString* GetCommaSpaceDisplayString() { return commaSpaceDisplayString; }
        JavascriptString* GetTrueDisplayString() { return trueDisplayString; }
        JavascriptString* GetFalseDisplayString() { return falseDisplayString; }
        JavascriptString* GetLengthDisplayString() { return lengthDisplayString; }
        JavascriptString* GetObjectDisplayString() { return objectDisplayString; }
        JavascriptString* GetStringTypeDisplayString() { return stringTypeDisplayString; }
        JavascriptString* GetErrorDisplayString() const { return errorDisplayString; }
        JavascriptString* GetFunctionPrefixString() { return functionPrefixString; }
        JavascriptString* GetGeneratorFunctionPrefixString() { return generatorFunctionPrefixString; }
        JavascriptString* GetFunctionDisplayString() { return functionDisplayString; }
        JavascriptString* GetXDomainFunctionDisplayString() { return xDomainFunctionDisplayString; }
        JavascriptString* GetInvalidDateString() { return invalidDateString; }
        JavascriptString* GetObjectTypeDisplayString() const { return objectTypeDisplayString; }
        JavascriptString* GetFunctionTypeDisplayString() const { return functionTypeDisplayString; }
        JavascriptString* GetBooleanTypeDisplayString() const { return booleanTypeDisplayString; }
        JavascriptString* GetNumberTypeDisplayString() const { return numberTypeDisplayString; }

#ifdef SIMD_JS_ENABLED
        JavascriptString* GetSIMDFloat32x4DisplayString() const { return simdFloat32x4DisplayString; }
        JavascriptString* GetSIMDFloat64x2DisplayString() const { return simdFloat64x2DisplayString; }
        JavascriptString* GetSIMDInt32x4DisplayString()   const { return simdInt32x4DisplayString; }
#endif

        JavascriptString* GetSymbolTypeDisplayString() const { return symbolTypeDisplayString; }
        JavascriptString* GetDebuggerDeadZoneBlockVariableString() { Assert(debuggerDeadZoneBlockVariableString); return debuggerDeadZoneBlockVariableString; }
        JavascriptRegExp* CreateEmptyRegExp();
        JavascriptFunction* GetObjectConstructor() const {return objectConstructor; }
        JavascriptFunction* GetBooleanConstructor() const {return booleanConstructor; }
        JavascriptFunction* GetDateConstructor() const {return dateConstructor; }
        JavascriptFunction* GetFunctionConstructor() const {return functionConstructor; }
        JavascriptFunction* GetNumberConstructor() const {return numberConstructor; }
        JavascriptRegExpConstructor* GetRegExpConstructor() const {return regexConstructor; }
        JavascriptFunction* GetStringConstructor() const {return stringConstructor; }
        JavascriptFunction* GetArrayBufferConstructor() const {return arrayBufferConstructor; }
        JavascriptFunction* GetPixelArrayConstructor() const {return pixelArrayConstructor; }
        JavascriptFunction* GetErrorConstructor() const { return errorConstructor; }
        JavascriptFunction* GetTypedArrayConstructor() const { return typedArrayConstructor; }
        JavascriptFunction* GetInt8ArrayConstructor() const {return Int8ArrayConstructor; }
        JavascriptFunction* GetUint8ArrayConstructor() const {return Uint8ArrayConstructor; }
        JavascriptFunction* GetInt16ArrayConstructor() const {return Int16ArrayConstructor; }
        JavascriptFunction* GetUint16ArrayConstructor() const {return Uint16ArrayConstructor; }
        JavascriptFunction* GetInt32ArrayConstructor() const {return Int32ArrayConstructor; }
        JavascriptFunction* GetUint32ArrayConstructor() const {return Uint32ArrayConstructor; }
        JavascriptFunction* GetFloat32ArrayConstructor() const {return Float32ArrayConstructor; }
        JavascriptFunction* GetFloat64ArrayConstructor() const {return Float64ArrayConstructor; }
        JavascriptFunction* GetWeakMapConstructor() const {return weakMapConstructor; }
        JavascriptFunction* GetMapConstructor() const {return mapConstructor; }
        JavascriptFunction* GetSetConstructor() const {return  setConstructor; }
        JavascriptFunction* GetSymbolConstructor() const {return symbolConstructor; }
        JavascriptFunction* GetWinRTPromiseConstructor();
        JavascriptFunction* GetEvalFunctionObject() { return evalFunctionObject; }
        JavascriptFunction* GetArrayPrototypeValuesFunction() { return arrayPrototypeValuesFunction; }
        DynamicObject* GetMathObject() const {return mathObject; }
        DynamicObject* GetJSONObject() const {return JSONObject; }
        DynamicObject* GetINTLObject() const {return IntlObject; }
        DynamicObject* GetReflectObject() const { return reflectObject; }
        const PropertyDescriptor* GetDefaultPropertyDescriptor() const { return &defaultPropertyDescriptor; }
        DynamicObject* GetMissingPropertyHolder() const { return missingPropertyHolder; }


#ifdef ENABLE_INTL_OBJECT
        void ResetIntlObject();
        void EnsureIntlObjectReady();
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        DynamicType * GetDebugDisposableObjectType() { return debugDisposableObjectType; }
        DynamicType * GetDebugFuncExecutorInDisposeObjectType() { return debugFuncExecutorInDisposeObjectType; }
#endif

        StaticType  * GetBooleanTypeStatic() const { return booleanTypeStatic; }
        DynamicType * GetBooleanTypeDynamic() const { return booleanTypeDynamic; }
        DynamicType * GetDateType() const { return dateType; }
        DynamicType * GetWinRTDateType() const { return winrtDateType; }
        DynamicType * GetBoundFunctionType() const { return boundFunctionType; }
        DynamicType * GetRegExpConstructorType() const { return regexConstructorType; }
        StaticType  * GetEnumeratorType() const { return enumeratorType; }
        DynamicType * GetSpreadArgumentType() const { return SpreadArgumentType; }
        StaticType  * GetWithType() const { return withType; }
        DynamicType * GetErrorType() const { return errorType; }
        DynamicType * GetEvalErrorType() const { return evalErrorType; }
        DynamicType * GetRangeErrorType() const { return rangeErrorType; }
        DynamicType * GetReferenceErrorType() const { return referenceErrorType; }
        DynamicType * GetSyntaxErrorType() const { return syntaxErrorType; }
        DynamicType * GetTypeErrorType() const { return typeErrorType; }
        DynamicType * GetURIErrorType() const { return uriErrorType; }
        DynamicType * GetWinRTErrorType() const { return winrtErrorType; }
        StaticType  * GetNumberTypeStatic() const { return numberTypeStatic; }
        StaticType  * GetInt64TypeStatic() const { return int64NumberTypeStatic; }
        StaticType  * GetUInt64TypeStatic() const { return uint64NumberTypeStatic; }
        DynamicType * GetNumberTypeDynamic() const { return numberTypeDynamic; }
        DynamicType * GetPromiseType() const { return promiseType; }

#ifdef SIMD_JS_ENABLED
        // SIMD
        StaticType* GetSIMDFloat32x4TypeStatic() const { return simdFloat32x4TypeStatic; }
        StaticType* GetSIMDFloat64x2TypeStatic() const { return simdFloat64x2TypeStatic; }
        StaticType* GetSIMDInt32x4TypeStatic()   const { return simdInt32x4TypeStatic; }
#endif

        DynamicType * GetObjectLiteralType(uint16 requestedInlineSlotCapacity);
        DynamicType * GetObjectHeaderInlinedLiteralType(uint16 requestedInlineSlotCapacity);
        DynamicType * GetObjectType() const { return objectTypes[0]; }
        DynamicType * GetObjectHeaderInlinedType() const { return objectHeaderInlinedTypes[0]; }
        StaticType  * GetSymbolTypeStatic() const { return symbolTypeStatic; }
        DynamicType * GetSymbolTypeDynamic() const { return symbolTypeDynamic; }
        DynamicType * GetProxyType() const { return proxyType; }
        DynamicType * GetJavascriptEnumeratorIteratorType() const { return javascriptEnumeratorIteratorType; }
        DynamicType * GetHeapArgumentsObjectType() const { return heapArgumentsType; }
        DynamicType * GetActivationObjectType() const { return activationObjectType; }
        DynamicType * GetArrayType() const { return arrayType; }
        DynamicType * GetNativeIntArrayType() const { return nativeIntArrayType; }
        DynamicType * GetCopyOnAccessNativeIntArrayType() const { return copyOnAccessNativeIntArrayType; }
        DynamicType * GetNativeFloatArrayType() const { return nativeFloatArrayType; }
        DynamicType * GetPixelArrayType() const { return pixelArrayType; }
        DynamicType * GetRegexType() const { return regexType; }
        DynamicType * GetRegexResultType() const { return regexResultType; }
        DynamicType * GetArrayBufferType() const { return arrayBufferType; }
        StaticType  * GetStringTypeStatic() const { AssertMsg(stringTypeStatic, "Where's stringTypeStatic?"); return stringTypeStatic; }
        DynamicType * GetStringTypeDynamic() const { return stringTypeDynamic; }
        StaticType  * GetVariantDateType() const { return variantDateType; }
        void EnsureDebugObject(DynamicObject* newDebugObject);
        DynamicObject* GetDebugObject() const { Assert(debugObject != null); return debugObject; }
        DynamicType * GetMapType() const { return mapType; }
        DynamicType * GetSetType() const { return setType; }
        DynamicType * GetWeakMapType() const { return weakMapType; }
        DynamicType * GetWeakSetType() const { return weakSetType; }
        DynamicType * GetArrayIteratorType() const { return arrayIteratorType; }
        DynamicType * GetMapIteratorType() const { return mapIteratorType; }
        DynamicType * GetSetIteratorType() const { return setIteratorType; }
        DynamicType * GetStringIteratorType() const { return stringIteratorType; }
        JavascriptFunction* GetDefaultAccessorFunction() const { return defaultAccessorFunction; }
        JavascriptFunction* GetStackTraceAccessorFunction() const { return stackTraceAccessorFunction; }
        JavascriptFunction* GetThrowTypeErrorAccessorFunction() const { return throwTypeErrorAccessorFunction; }
        JavascriptFunction* GetThrowTypeErrorCallerAccessorFunction() const { return throwTypeErrorCallerAccessorFunction; }
        JavascriptFunction* GetThrowTypeErrorCalleeAccessorFunction() const { return throwTypeErrorCalleeAccessorFunction; }
        JavascriptFunction* GetThrowTypeErrorArgumentsAccessorFunction() const { return throwTypeErrorArgumentsAccessorFunction; }
        JavascriptFunction* Get__proto__getterFunction() const { return __proto__getterFunction; }
        JavascriptFunction* Get__proto__setterFunction() const { return __proto__setterFunction; }

        JavascriptFunction* GetObjectValueOfFunction() const { return objectValueOfFunction; }
        JavascriptFunction* GetObjectToStringFunction() const { return objectToStringFunction; }

#ifdef SIMD_JS_ENABLED
        // SIMD
        JavascriptFunction* GetSIMDFloat32x4ToStringFunction() const { return simdFloat32x4ToStringFunction;  }
        JavascriptFunction* GetSIMDFloat64x2ToStringFunction() const { return simdFloat64x2ToStringFunction; }
        JavascriptFunction* GetSIMDInt32x4ToStringFunction()   const { return simdInt32x4ToStringFunction; }
#endif

        JavascriptFunction* GetDebugObjectNonUserGetterFunction() const { return debugObjectNonUserGetterFunction; }
        JavascriptFunction* GetDebugObjectNonUserSetterFunction() const { return debugObjectNonUserSetterFunction; }

        void SetDebugObjectNonUserAccessor(FunctionInfo *funcGetter, FunctionInfo *funcSetter);

        JavascriptFunction* GetDebugObjectDebugModeGetterFunction() const { return debugObjectDebugModeGetterFunction; }
        void SetDebugObjectDebugModeAccessor(FunctionInfo *funcGetter);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        JavascriptFunction* GetDebugObjectFaultInjectionCookieGetterFunction() const { return debugObjectFaultInjectionCookieGetterFunction; }
        JavascriptFunction* GetDebugObjectFaultInjectionCookieSetterFunction() const { return debugObjectFaultInjectionCookieSetterFunction; }
        void SetDebugObjectFaultInjectionCookieGetterAccessor(FunctionInfo *funcGetter, FunctionInfo *funcSetter);
#endif

        JavascriptFunction* GetArrayPrototypeToStringFunction() const { return arrayPrototypeToStringFunction; }
        JavascriptFunction* GetArrayPrototypeToLocaleStringFunction() const { return arrayPrototypeToLocaleStringFunction; }
        JavascriptFunction* GetIdentityFunction() const { return identityFunction; }
        JavascriptFunction* GetThrowerFunction() const { return throwerFunction; }

        void InitializeHostPromiseContinuationFunction();
        JavascriptFunction* GetHostPromiseContinuationFunction();
        void SetNativeHostPromiseContinuationFunction(PromiseContinuationCallback function, void *state);
        void EnqueueTask(Var taskVar);

        HeapArgumentsObject* CreateHeapArguments(Var frameObj, uint formalCount);
        JavascriptArray* CreateArray();
        JavascriptArray* CreateArray(uint32 length);
        JavascriptArray *CreateArrayOnStack(void *const stackAllocationPointer);
        JavascriptNativeIntArray* CreateNativeIntArray();
        JavascriptNativeIntArray* CreateNativeIntArray(uint32 length);
        JavascriptCopyOnAccessNativeIntArray* CreateCopyOnAccessNativeIntArray();
        JavascriptCopyOnAccessNativeIntArray* CreateCopyOnAccessNativeIntArray(uint32 length);
        JavascriptNativeFloatArray* CreateNativeFloatArray();
        JavascriptNativeFloatArray* CreateNativeFloatArray(uint32 length);
        JavascriptArray* CreateArray(uint32 length, uint32 size);
        ArrayBuffer* CreateArrayBuffer(uint32 length);
        ArrayBuffer* CreateArrayBuffer(byte* buffer, uint32 length);
        ArrayBuffer* CreateProjectionArraybuffer(uint32 length);
        ArrayBuffer* CreateProjectionArraybuffer(byte* buffer, uint32 length);
        DataView* CreateDataView(ArrayBuffer* arrayBuffer, uint32 offSet, uint32 mappedLength);

        template <typename TypeName, bool clamped>
        inline DynamicType* GetTypedArrayType(TypeName);

        template<> inline DynamicType* GetTypedArrayType<int8,false>(int8) { return int8ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<uint8,false>(uint8) { return uint8ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<uint8,true>(uint8) { return uint8ClampedArrayType; };
        template<> inline DynamicType* GetTypedArrayType<int16,false>(int16) { return int16ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<uint16,false>(uint16) { return uint16ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<int32,false>(int32) { return int32ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<uint32,false>(uint32) { return uint32ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<float,false>(float) { return float32ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<double,false>(double) { return float64ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<int64,false>(int64) { return int64ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<uint64,false>(uint64) { return uint64ArrayType; };
        template<> inline DynamicType* GetTypedArrayType<bool,false>(bool) { return boolArrayType; };

        DynamicType* GetCharArrayType() { return charArrayType; };

        JavascriptPixelArray* CreatePixelArray(uint32 length);
        bool GetPixelArrayBuffer(Var instance, BYTE **ppBuffer, UINT *pBufferLength);

        //
        // This method would be used for creating array literals, when we really need to create a huge array
        // Avoids checks at runtime.
        //
        JavascriptArray*            CreateArrayLiteral(uint32 length);
        JavascriptNativeIntArray*   CreateNativeIntArrayLiteral(uint32 length);
        JavascriptNativeIntArray*   CreateCopyOnAccessNativeIntArrayLiteral(ArrayCallSiteInfo *arrayInfo, FunctionBody *functionBody, const Js::AuxArray<int32> *ints);
        JavascriptNativeFloatArray* CreateNativeFloatArrayLiteral(uint32 length);

        JavascriptBoolean* CreateBoolean(BOOL value);
        JavascriptDate* CreateDate();
        JavascriptDate* CreateDate(double value);
        JavascriptDate* CreateDate(SYSTEMTIME* pst);
        JavascriptMap* CreateMap();
        JavascriptSet* CreateSet();
        JavascriptWeakMap* CreateWeakMap();
        JavascriptWeakSet* CreateWeakSet();
        JavascriptError* CreateError();
        JavascriptError* CreateError(DynamicType* errorType, BOOL isExternal = FALSE);
        JavascriptError* CreateExternalError(ErrorTypeEnum errorTypeEnum);
        JavascriptError* CreateEvalError();
        JavascriptError* CreateRangeError();
        JavascriptError* CreateReferenceError();
        JavascriptError* CreateSyntaxError();
        JavascriptError* CreateTypeError();
        JavascriptError* CreateURIError();
        JavascriptError* CreateStackOverflowError();
        JavascriptError* CreateOutOfMemoryError();
        JavascriptError* CreateWinRTError();
        JavascriptSymbol* CreateSymbol(JavascriptString* description);
        JavascriptSymbol* CreateSymbol(const wchar_t* description, int descriptionLength);
        JavascriptSymbol* CreateSymbol(const PropertyRecord* propertyRecord);
        JavascriptPromise* CreatePromise();
        JavascriptGenerator* CreateGenerator(Arguments& args, ScriptFunction* scriptFunction, RecyclableObject* prototype);
        JavascriptFunction* CreateNonProfiledFunction(FunctionInfo * functionInfo);
        template <class MethodType>
        JavascriptExternalFunction* CreateIdMappedExternalFunction(MethodType entryPoint, DynamicType *pPrototypeType);
        JavascriptExternalFunction* CreateExternalConstructor(Js::JavascriptMethod entryPoint, PropertyId nameId, RecyclableObject * prototype);
        JavascriptExternalFunction* CreateExternalConstructor(Js::JavascriptMethod entryPoint, PropertyId nameId, InitializeMethod method, unsigned short deferredTypeSlots, bool hasAccessors);
        JavascriptWinRTFunction* CreateIdMappedWinRTFunction(DynamicType * type, WinRTFunctionInfo * functionInfo, Var signature);
        JavascriptWinRTFunction* CreateIdMappedWinRTConstructorFunction(DynamicType * type, WinRTFunctionInfo * functionInfo, Var signature);
        static DynamicTypeHandler * GetDeferredPrototypeGeneratorFunctionTypeHandler(ScriptContext* scriptContext);
        DynamicType * CreateDeferredPrototypeGeneratorFunctionType(JavascriptMethod entrypoint, bool isShared = false);
        static DynamicTypeHandler * GetDeferredPrototypeFunctionTypeHandler(ScriptContext* scriptContext);
        static DynamicTypeHandler * GetDeferredBoundFunctionTypeHandler(ScriptContext* scriptContext);
        DynamicTypeHandler * GetDeferredFunctionTypeHandler();
        DynamicType * CreateDeferredPrototypeFunctionType(JavascriptMethod entrypoint, bool isShared = false);
        DynamicType * CreateFunctionType(JavascriptMethod entrypoint, RecyclableObject* prototype = nullptr);
        DynamicType * CreateFunctionWithLengthType(FunctionInfo * functionInfo);
        DynamicType * CreateFunctionWithLengthAndPrototypeType(FunctionInfo * functionInfo);
        DynamicType * CreateFunctionWithLengthType(DynamicObject * prototype, FunctionInfo * functionInfo);
        DynamicType * CreateFunctionWithLengthAndPrototypeType(DynamicObject * prototype, FunctionInfo * functionInfo);
        ScriptFunction * CreateScriptFunction(FunctionProxy* proxy);
        RuntimeFunction * CloneBuiltinFunction(RuntimeFunction* func);
        ScriptFunctionWithInlineCache * CreateScriptFunctionWithInlineCache(FunctionProxy* proxy);
        GeneratorVirtualScriptFunction * CreateGeneratorVirtualScriptFunction(FunctionProxy* proxy);
        JavascriptTypedObjectSlotAccessorFunction* CreateTypedObjectSlotGetterFunction(unsigned int slotIndex, FunctionInfo* functionInfo, int typeId, PropertyId nameId);
        JavascriptTypedObjectSlotAccessorFunction* CreateTypedObjectSlotSetterFunction(unsigned int slotIndex, FunctionInfo* functionInfo, int typeId, PropertyId nameId);
        DynamicType * CreateGeneratorType(RecyclableObject* prototype);

        NullEnumerator * GetNullEnumerator() { return nullEnumerator; }
#if 0
        JavascriptNumber* CreateNumber(double value);
#endif
        JavascriptNumber* CreateNumber(double value, RecyclerJavascriptNumberAllocator * numberAllocator);
        JavascriptGeneratorFunction* CreateGeneratorFunction(JavascriptMethod entryPoint, GeneratorVirtualScriptFunction* scriptFunction);
        JavascriptExternalFunction* CreateExternalFunction(JavascriptMethod entryPointer, PropertyId nameId, Var signature);
        JavascriptExternalFunction* CreateExternalFunction(JavascriptMethod entryPointer, Var nameId, Var signature);
        inline JavascriptExternalFunction* CreateStdCallExternalFunction(StdCallJavascriptMethod entryPointer, PropertyId nameId, void *callbackState);
        inline JavascriptExternalFunction* CreateStdCallExternalFunction(StdCallJavascriptMethod entryPointer, Var nameId, void *callbackState);
        inline JavascriptWinRTFunction* CreateWinRTFunction(JavascriptMethod entryPoint, PropertyId nameId, Var signature, bool fConstructor);
        JavascriptPromiseCapabilitiesExecutorFunction* CreatePromiseCapabilitiesExecutorFunction(JavascriptMethod entryPoint, JavascriptPromiseCapability* capability);
        JavascriptPromiseResolveOrRejectFunction* CreatePromiseResolveOrRejectFunction(JavascriptMethod entryPoint, JavascriptPromise* promise, bool isReject);
        JavascriptPromiseReactionTaskFunction* CreatePromiseReactionTaskFunction(JavascriptMethod entryPoint, JavascriptPromiseReaction* reaction, Var argument);
        JavascriptPromiseResolveThenableTaskFunction* CreatePromiseResolveThenableTaskFunction(JavascriptMethod entryPoint, JavascriptPromise* promise, RecyclableObject* thenable, RecyclableObject* thenFunction);
        JavascriptPromiseAllResolveElementFunction* CreatePromiseAllResolveElementFunction(JavascriptMethod entryPoint, uint32 index, JavascriptArray* values, JavascriptPromiseCapability* capabilities, JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper* remainingElements);
        JavascriptExternalFunction* CreateWrappedExternalFunction(JavascriptExternalFunction* wrappedFunction);
        JavascriptNumber* CreateCodeGenNumber(CodeGenNumberAllocator *alloc, double value);
        DynamicObject* CreateGeneratorConstructorPrototypeObject();
        DynamicObject* CreateConstructorPrototypeObject(JavascriptFunction * constructor);
        DynamicObject* CreateObject(const bool allowObjectHeaderInlining = false, const PropertyIndex requestedInlineSlotCapacity = 0);
        DynamicObject* CreateObject(DynamicTypeHandler * typeHandler);
        DynamicObject* CreateActivationObject();
        DynamicObject* CreatePseudoActivationObject();
        DynamicObject* CreateBlockActivationObject();
        DynamicType* CreateObjectType(RecyclableObject* prototype, Js::TypeId typeId, uint16 requestedInlineSlotCapacity);
        DynamicType* CreateObjectTypeNoCache(RecyclableObject* prototype, Js::TypeId typeId);
        DynamicType* CreateObjectType(RecyclableObject* prototype, uint16 requestedInlineSlotCapacity);
        DynamicObject* CreateObject(RecyclableObject* prototype, uint16 requestedInlineSlotCapacity = 0);

        typedef JavascriptString* LibStringType; // used by diagnostics template
        template< size_t N > JavascriptString* CreateStringFromCppLiteral(const wchar_t (&value)[N]) const;
        template<> JavascriptString* CreateStringFromCppLiteral(const wchar_t (&value)[1]) const; // Specialization for empty string
        template<> JavascriptString* CreateStringFromCppLiteral(const wchar_t (&value)[2]) const; // Specialization for single-char strings
        PropertyString* CreatePropertyString(const Js::PropertyRecord* propertyRecord);
        PropertyString* CreatePropertyString(const Js::PropertyRecord* propertyRecord, ArenaAllocator *arena);

        JavascriptVariantDate* CreateVariantDate(const double value);

        JavascriptBooleanObject* CreateBooleanObject(BOOL value);
        JavascriptBooleanObject* CreateBooleanObject();
        JavascriptNumberObject* CreateNumberObjectWithCheck(double value);
        JavascriptNumberObject* CreateNumberObject(Var number);
        JavascriptStringObject* CreateStringObject(JavascriptString* value);
        JavascriptStringObject* CreateStringObject(const wchar_t* value, charcount_t length);
        JavascriptSymbolObject* CreateSymbolObject(JavascriptSymbol* value);
        JavascriptArrayIterator* CreateArrayIterator(Var iterable, JavascriptArrayIteratorKind kind);
        JavascriptMapIterator* CreateMapIterator(JavascriptMap* map, JavascriptMapIteratorKind kind);
        JavascriptSetIterator* CreateSetIterator(JavascriptSet* set, JavascriptSetIteratorKind kind);
        JavascriptStringIterator* CreateStringIterator(JavascriptString* string);
        JavascriptRegExp* CreateRegExp(UnifiedRegex::RegexPattern* pattern);

        DynamicObject* CreateIteratorResultObject(Var value, Var done);
        DynamicObject* CreateIteratorResultObjectValueFalse(Var value);
        DynamicObject* CreateIteratorResultObjectUndefinedTrue();

        RecyclableObject* CreateThrowErrorObject(JavascriptError* error);

        StaticType * GetDispMemberProxyType() { return dispMemberProxyType; }
        StaticType * GetHostDispatchType() { return hostDispatchType; }
        DynamicType * GetHostObjectType() { return hostObjectType;}

        DynamicType * GetModuleRootType() { return moduleRootType; }
        DynamicType * GetExtensionEnumeratorType() { return extensionEnumeratorType; }

        void SetCrossSiteForSharedFunctionType(JavascriptFunction * function, bool useSlotAccessCrossSiteThunk);

        uint64 GetRandSeed(){return randSeed;}
        void SetRandSeed(uint64 rs){randSeed = rs;}

        void SetProfileMode(bool fSet);
        void SetDispatchProfile(bool fSet, JavascriptMethod dispatchInvoke);
        void SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke);

        static bool IsCopyOnAccessArrayCallSite(JavascriptLibrary *lib, ArrayCallSiteInfo *arrayInfo, uint32 length);
        static bool IsCachedCopyOnAccessArrayCallSite(const JavascriptLibrary *lib, ArrayCallSiteInfo *arrayInfo);
        template <typename T>
        static void CheckAndConvertCopyOnAccessNativeIntArray(const T instance);

        void EnsureStringTemplateCallsiteObjectList();
        void AddStringTemplateCallsiteObject(RecyclableObject* callsite);
        RecyclableObject* TryGetStringTemplateCallsiteObject(ParseNodePtr pnode);
        RecyclableObject* TryGetStringTemplateCallsiteObject(RecyclableObject* callsite);

#if DBG_DUMP
        static const wchar_t* GetStringTemplateCallsiteObjectKey(Var callsite);
#endif

#ifdef ENABLE_NATIVE_CODEGEN
        inline JavascriptFunction** GetBuiltinFunctions();
        inline INT_PTR* GetVTableAddresses();
        static BuiltinFunction GetBuiltinFunctionForPropId(PropertyId id);
        static BuiltinFunction GetBuiltInForFuncInfo(FunctionInfo* funcInfo);
#if DBG
        static void CheckRegisteredBuiltIns(JavascriptFunction** builtInFuncs)
        {
            byte count = BuiltinFunction::Count;
            for(byte index = 0; index < count; index++)
            {
                Assert(!builtInFuncs[index] || (index == GetBuiltInForFuncInfo(builtInFuncs[index]->GetFunctionInfo())));
            }
        }
#endif
        static BOOL CanFloatPreferenceFunc(BuiltinFunction index);
        static BOOL IsFltFunc(BuiltinFunction index);
        static bool IsFloatFunctionCallsite(BuiltinFunction index, size_t argc);
        static bool IsFltBuiltInConst(PropertyId id);
        static size_t GetArgCForBuiltIn(BuiltinFunction index)
        {
            Assert(index < _countof(JavascriptLibrary::LibraryFunctionArgC));
            return JavascriptLibrary::LibraryFunctionArgC[index];
        }
        static BuiltInFlags GetFlagsForBuiltIn(BuiltinFunction index)
        {
            Assert(index < _countof(JavascriptLibrary::LibraryFunctionFlags));
            return (BuiltInFlags)JavascriptLibrary::LibraryFunctionFlags[index];
        }
        static BuiltinFunction GetBuiltInInlineCandidateId(Js::OpCode opCode);
        static BuiltInArgSpecizationType GetBuiltInArgType(BuiltInFlags flags, BuiltInArgShift argGroup);
        static bool IsTypeSpecRequired(BuiltInFlags flags)
        {
            return GetBuiltInArgType(flags, BuiltInArgShift::BIAS_Src1) || GetBuiltInArgType(flags, BuiltInArgShift::BIAS_Src2) || GetBuiltInArgType(flags, BuiltInArgShift::BIAS_Dst);
        }
#if ENABLE_DEBUG_CONFIG_OPTIONS
        static wchar_t* GetNameForBuiltIn(BuiltinFunction index)
        {
            Assert(index < _countof(JavascriptLibrary::LibraryFunctionName));
            return JavascriptLibrary::LibraryFunctionName[index];
        }
#endif
#endif

        PropertyStringCacheMap* EnsurePropertyStringMap();
        PropertyStringCacheMap* GetPropertyStringMap() { return this->propertyStringMap; }

        ForInObjectEnumerator* JavascriptLibrary::GetAndClearForInEnumeratorCache();
        void SetForInEnumeratorCache(ForInObjectEnumerator* enumerator);

        void TypeAndPrototypesAreEnsuredToHaveOnlyWritableDataProperties(Type *const type);
        void NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();

        HRESULT EnsureReadyIfHybridDebugging(bool isScriptEngineReady = true);

        CharStringCache& GetCharStringCache() { return charStringCache;  }
        static JavascriptLibrary * FromCharStringCache(CharStringCache * cache)
        {
            return (JavascriptLibrary *)((uintptr)cache - offsetof(JavascriptLibrary, charStringCache));
        }

    private:
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        // Declare fretest/debug properties here since otherwise it can cause
        // a mismatch between fre mshtml and fretest jscript9 causing undefined behavior

        DynamicType * debugDisposableObjectType;
        DynamicType * debugFuncExecutorInDisposeObjectType;
#endif

        void InitializePrototypes();
        void InitializeTypes();
        void InitializeGlobal(GlobalObject * globalObject);

        static void __cdecl InitializeArrayConstructor(DynamicObject* arrayConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeArrayPrototype(DynamicObject* arrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeArrayBufferConstructor(DynamicObject* arrayBufferConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeArrayBufferPrototype(DynamicObject* arrayBufferPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeDataViewConstructor(DynamicObject* dataViewConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeDataViewPrototype(DynamicObject* dataViewPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeEvalErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeEvalErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeRangeErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeRangeErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeReferenceErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeReferenceErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSyntaxErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSyntaxErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeTypeErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeTypeErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeURIErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeURIErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeWinRTErrorConstructor(DynamicObject* constructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeWinRTErrorPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeTypedArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeTypedArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt8ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt8ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint8ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint8ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint8ClampedArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint8ClampedArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt16ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt16ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint16ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint16ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt32ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt32ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint32ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint32ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeFloat32ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeFloat32ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeFloat64ArrayConstructor(DynamicObject* typedArrayConsructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeFloat64ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeInt64ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeUint64ArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeBoolArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeCharArrayPrototype(DynamicObject* prototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializePixelArrayConstructor(DynamicObject* pixelArrayConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializePixelArrayPrototype(DynamicObject* arrayPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeBooleanConstructor(DynamicObject* booleanConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeBooleanPrototype(DynamicObject* booleanPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSymbolConstructor(DynamicObject* symbolConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSymbolPrototype(DynamicObject* symbolPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeDateConstructor(DynamicObject* dateConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeDatePrototype(DynamicObject* datePrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeProxyConstructor(DynamicObject* proxyConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeProxyPrototype(DynamicObject* proxyPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeFunctionConstructor(DynamicObject* functionConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeFunctionPrototype(DynamicObject* functionPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        void InitializeDebug();
        void InitializeComplexThings();
        void InitializeStaticValues();
        static void __cdecl InitializeMathObject(DynamicObject* mathObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
#ifdef SIMD_JS_ENABLED
        // SIMD
        static void __cdecl InitializeSIMDObject(DynamicObject* simdObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
#endif

        static void __cdecl InitializeNumberConstructor(DynamicObject* numberConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeNumberPrototype(DynamicObject* numberPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeObjectConstructor(DynamicObject* objectConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeObjectPrototype(DynamicObject* objectPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeRegexConstructor(DynamicObject* regexConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeRegexPrototype(DynamicObject* regexPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeStringConstructor(DynamicObject* stringConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeStringPrototype(DynamicObject* stringPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeJSONObject(DynamicObject* JSONObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeEngineInterfaceObject(DynamicObject* engineInterface, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeReflectObject(DynamicObject* reflectObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
#ifdef ENABLE_INTL_OBJECT
        static void __cdecl InitializeIntlObject(DynamicObject* IntlEngineObject, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
#endif
        void InitializeWinRTPromiseConstructor();

        static void __cdecl InitializeMapConstructor(DynamicObject* mapConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeMapPrototype(DynamicObject* mapPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSetConstructor(DynamicObject* setConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSetPrototype(DynamicObject* setPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeWeakMapConstructor(DynamicObject* weakMapConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeWeakMapPrototype(DynamicObject* weakMapPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeWeakSetConstructor(DynamicObject* weakSetConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeWeakSetPrototype(DynamicObject* weakSetPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeArrayIteratorPrototype(DynamicObject* arrayIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeMapIteratorPrototype(DynamicObject* mapIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeSetIteratorPrototype(DynamicObject* setIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeStringIteratorPrototype(DynamicObject* stringIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeJavascriptEnumeratorIteratorPrototype(DynamicObject* javascriptEnumeratorIteratorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializePromiseConstructor(DynamicObject* promiseConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializePromisePrototype(DynamicObject* promisePrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static void __cdecl InitializeGeneratorFunctionConstructor(DynamicObject* generatorFunctionConstructor, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeGeneratorFunctionProtoype(DynamicObject* generatorFunctionPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeGeneratorProtoype(DynamicObject* generatorPrototype, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        RuntimeFunction* CreateBuiltinConstructor(FunctionInfo * functionInfo, DynamicTypeHandler * typeHandler, DynamicObject* prototype = nullptr);
        RuntimeFunction* DefaultCreateFunction(FunctionInfo * functionInfo, int length, DynamicObject * prototype, DynamicType * functionType, PropertyId nameId);
        RuntimeFunction* DefaultCreateFunction(FunctionInfo * functionInfo, int length, DynamicObject * prototype, DynamicType * functionType, Var nameId);
        JavascriptFunction* AddFunction(DynamicObject* object, PropertyId propertyId, RuntimeFunction* function);
        void AddMember(DynamicObject* object, PropertyId propertyId, Var value);
        void AddMember(DynamicObject* object, PropertyId propertyId, Var value, PropertyAttributes attributes);
        JavascriptString* CreateEmptyString();

        void InitializeDiagnosticsScriptObject(DiagnosticsScriptObject* newDiagnosticsScriptObject);

        static void __cdecl InitializeGeneratorFunction(DynamicObject* function, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        template<bool addPrototype>
        static void __cdecl InitializeFunction(DynamicObject* function, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

#ifdef ENABLE_NATIVE_CODEGEN
        static size_t LibraryFunctionArgC[BuiltinFunction::Count + 1];
        static int LibraryFunctionFlags[BuiltinFunction::Count + 1];   // returns enum BuiltInFlags.
#if ENABLE_DEBUG_CONFIG_OPTIONS
        static wchar_t* LibraryFunctionName[BuiltinFunction::Count + 1];
#endif
#endif

#ifdef ENABLE_PROJECTION
    public:
        typedef HRESULT (*ArrayBufferFromExternalObject)(__in RecyclableObject *obj,
                                                         __out ArrayBuffer **ppArrayBuffer);
        ArrayBufferFromExternalObject getpfArrayBufferFromExternalObject() { return pfArrayBufferFromExternalObject; }
    private:
        ArrayBufferFromExternalObject pfArrayBufferFromExternalObject;
#endif

    public:
        virtual void Finalize(bool isShutdown) override
        {
            __super::Finalize(isShutdown);
            if (this->referencedPropertyRecords != null)
            {
                RECYCLER_PERF_COUNTER_SUB(PropertyRecordBindReference, this->referencedPropertyRecords->Count());
            }
        }
    private:
        typedef JsUtil::BaseHashSet<Js::PropertyRecord const *, Recycler, PowerOf2SizePolicy> ReferencedPropertyRecordHashSet;
        ReferencedPropertyRecordHashSet* referencedPropertyRecords;

        ReferencedPropertyRecordHashSet * EnsureReferencedPropertyRecordList()
        {
            ReferencedPropertyRecordHashSet* pidList = this->referencedPropertyRecords;
            if (pidList == null)
            {
                pidList = RecyclerNew(this->recycler, ReferencedPropertyRecordHashSet, this->recycler, 173);
                this->referencedPropertyRecords = pidList;
            }
            return pidList;
        }

        ReferencedPropertyRecordHashSet * GetReferencedPropertyRecordList() const
        {
            return this->referencedPropertyRecords;
        }
    };
}
