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
        TypeIds_SafeArray = 8,
        TypeIds_Symbol = 9,
        
        TypeIds_LastToPrimitiveType = TypeIds_Symbol,

        TypeIds_Enumerator = 10,
        TypeIds_VariantDate = 11,
        
        // SIMD types
        TypeIds_SIMDFloat32x4 = 12,
        TypeIds_SIMDFloat64x2 = 13,
        TypeIds_SIMDInt32x4 = 14,

        TypeIds_LastJavascriptPrimitiveType = TypeIds_SIMDInt32x4,

        TypeIds_HostDispatch = 15,
        TypeIds_WithScopeObject = 16,
        TypeIds_UndeclBlockVar = 17,

        TypeIds_LastStaticType = TypeIds_UndeclBlockVar,

        TypeIds_Proxy = 18,
        TypeIds_Function = 19,

        //
        // The backend expects only objects whose typeof() === "object" to have a
        // TypeId >= TypeIds_Object. Only 'null' is a special case because it
        // has a static type.
        //
        TypeIds_Object = 20,
        TypeIds_Array = 21,
        TypeIds_ArrayFirst = TypeIds_Array,
        TypeIds_NativeIntArray = 22,
        TypeIds_CopyOnAccessNativeIntArray = 23,
        TypeIds_NativeFloatArray = 24,
        TypeIds_ArrayLast = TypeIds_NativeFloatArray,
        TypeIds_Date = 25,
        TypeIds_RegEx = 26,
        TypeIds_Error = 27,
        TypeIds_BooleanObject = 28,
        TypeIds_NumberObject = 29,
        TypeIds_StringObject = 30,
        TypeIds_ExtensionEnumerator = 31,
        TypeIds_SafeArrayObject = 32,
        TypeIds_Arguments = 33,
        TypeIds_ES5Array = 34,
        TypeIds_PixelArray = 35,
        TypeIds_ArrayBuffer = 36,
        TypeIds_Int8Array = 37,
        TypeIds_TypedArrayMin = TypeIds_Int8Array,
        TypeIds_TypedArraySCAMin = TypeIds_Int8Array, // Min SCA supported TypedArray TypeId
        TypeIds_Uint8Array = 38,
        TypeIds_Uint8ClampedArray = 39,
        TypeIds_Int16Array = 40,
        TypeIds_Uint16Array = 41,
        TypeIds_Int32Array = 42,
        TypeIds_Uint32Array = 43,
        TypeIds_Float32Array = 44,
        TypeIds_Float64Array = 45,
        TypeIds_TypedArraySCAMax = TypeIds_Float64Array, // Max SCA supported TypedArray TypeId
        TypeIds_Int64Array = 46,
        TypeIds_Uint64Array = 47,
        TypeIds_CharArray = 48,
        TypeIds_BoolArray = 49,
        TypeIds_TypedArrayMax = TypeIds_BoolArray,
        TypeIds_EngineInterfaceObject = 50,
        TypeIds_DataView = 51,
        TypeIds_WinRTDate = 52,
        TypeIds_Map = 53,
        TypeIds_Set = 54,
        TypeIds_WeakMap = 55,
        TypeIds_WeakSet = 56,
        TypeIds_SymbolObject = 57,
        TypeIds_ArrayIterator = 58,
        TypeIds_MapIterator = 59,
        TypeIds_SetIterator = 60,
        TypeIds_StringIterator = 61,
        TypeIds_JavascriptEnumeratorIterator = 62,
        TypeIds_Generator = 63,
        TypeIds_Promise = 64,

        TypeIds_LastBuiltinDynamicObject = TypeIds_Promise,
        TypeIds_GlobalObject = 65,
        TypeIds_ModuleRoot = 66,
        TypeIds_LastTrueJavascriptObjectType = TypeIds_ModuleRoot,

        TypeIds_HostObject = 67,
        TypeIds_ActivationObject = 68,
        TypeIds_SpreadArgument = 69,
        
        TypeIds_Limit //add a new TypeId before TypeIds_Limit or before TypeIds_LastTrueJavascriptObjectType
};


