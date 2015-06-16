//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

enum TypeId
{
        TypeIds_Undefined = 0,
        TypeIds_Null = 1,

        TypeIds_UndefinedOrNull =  TypeIds_Null,

        TypeIds_Boolean = 2,

        // backend typeof() == "number" is true for typeIds
        // between TypeIds_FirstNumberType <= typeId <= TypeIds_LastNumberType
        TypeIds_Integer = 3,
        TypeIds_FirstNumberType = TypeIds_Integer,
        TypeIds_Number = 4,
        TypeIds_Int64Number = 5,
        TypeIds_UInt64Number = 6,
        TypeIds_LastNumberType = TypeIds_UInt64Number,

        TypeIds_String = 7,
        TypeIds_Symbol = 8,

        TypeIds_LastToPrimitiveType = TypeIds_Symbol,

        TypeIds_Enumerator = 9,
        TypeIds_VariantDate = 10,

        // SIMD types
        TypeIds_SIMDFloat32x4 = 11,
        TypeIds_SIMDFloat64x2 = 12,
        TypeIds_SIMDInt32x4 = 13,

        TypeIds_LastJavascriptPrimitiveType = TypeIds_SIMDInt32x4,

        TypeIds_HostDispatch = 14,
        TypeIds_WithScopeObject = 15,
        TypeIds_UndeclBlockVar = 16,

        TypeIds_LastStaticType = TypeIds_UndeclBlockVar,

        TypeIds_Proxy = 17,
        TypeIds_Function = 18,

        //
        // The backend expects only objects whose typeof() === "object" to have a
        // TypeId >= TypeIds_Object. Only 'null' is a special case because it
        // has a static type.
        //
        TypeIds_Object = 19,
        TypeIds_Array = 20,
        TypeIds_ArrayFirst = TypeIds_Array,
        TypeIds_NativeIntArray = 21,
        TypeIds_CopyOnAccessNativeIntArray = 22,
        TypeIds_NativeFloatArray = 23,
        TypeIds_ArrayLast = TypeIds_NativeFloatArray,
        TypeIds_Date = 24,
        TypeIds_RegEx = 25,
        TypeIds_Error = 26,
        TypeIds_BooleanObject = 27,
        TypeIds_NumberObject = 28,
        TypeIds_StringObject = 29,
        TypeIds_ExtensionEnumerator = 30,
        TypeIds_Arguments = 31,
        TypeIds_ES5Array = 32,
        TypeIds_PixelArray = 33,
        TypeIds_ArrayBuffer = 34,
        TypeIds_Int8Array = 35,
        TypeIds_TypedArrayMin = TypeIds_Int8Array,
        TypeIds_TypedArraySCAMin = TypeIds_Int8Array, // Min SCA supported TypedArray TypeId
        TypeIds_Uint8Array = 36,
        TypeIds_Uint8ClampedArray = 37,
        TypeIds_Int16Array = 38,
        TypeIds_Uint16Array = 39,
        TypeIds_Int32Array = 40,
        TypeIds_Uint32Array = 41,
        TypeIds_Float32Array = 42,
        TypeIds_Float64Array = 43,
        TypeIds_TypedArraySCAMax = TypeIds_Float64Array, // Max SCA supported TypedArray TypeId
        TypeIds_Int64Array = 44,
        TypeIds_Uint64Array = 45,
        TypeIds_CharArray = 46,
        TypeIds_BoolArray = 47,
        TypeIds_TypedArrayMax = TypeIds_BoolArray,
        TypeIds_EngineInterfaceObject = 48,
        TypeIds_DataView = 49,
        TypeIds_WinRTDate = 50,
        TypeIds_Map = 51,
        TypeIds_Set = 52,
        TypeIds_WeakMap = 53,
        TypeIds_WeakSet = 54,
        TypeIds_SymbolObject = 55,
        TypeIds_ArrayIterator = 56,
        TypeIds_MapIterator = 57,
        TypeIds_SetIterator = 58,
        TypeIds_StringIterator = 59,
        TypeIds_JavascriptEnumeratorIterator = 60,
        TypeIds_Generator = 61,
        TypeIds_Promise = 62,

        TypeIds_LastBuiltinDynamicObject = TypeIds_Promise,
        TypeIds_GlobalObject = 63,
        TypeIds_ModuleRoot = 64,
        TypeIds_LastTrueJavascriptObjectType = TypeIds_ModuleRoot,

        TypeIds_HostObject = 65,
        TypeIds_ActivationObject = 66,
        TypeIds_SpreadArgument = 67,

        TypeIds_Limit //add a new TypeId before TypeIds_Limit or before TypeIds_LastTrueJavascriptObjectType
};


