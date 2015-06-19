// Syntax: TYPENAME(HasLibraryConstructor,TypeName)

TYPENAME(1,Array)
TYPENAME(1,ArrayBuffer)
TYPENAME(1,Boolean)
TYPENAME(0,DataView)
TYPENAME(1,Date)
TYPENAME(1,Error)
TYPENAME(1,Float32Array)
TYPENAME(1,Float64Array)
TYPENAME(1,Function)
TYPENAME(0,Generator)
TYPENAME(1,GeneratorFunction)
TYPENAME(0,Global)
TYPENAME(1,Int16Array)
TYPENAME(1,Int32Array)
TYPENAME(1,Int8Array)
TYPENAME(0,JSON)
TYPENAME(1,Map)
TYPENAME(0,Math)
TYPENAME(1,Number)
TYPENAME(1,Object)
TYPENAME(1,Promise)
TYPENAME(1,Proxy)
TYPENAME(0,Reflect)
TYPENAME(1,RegExp)
TYPENAME(1,Set)
TYPENAME(1,String)
TYPENAME(0,StringIterator)
TYPENAME(1,Symbol)
TYPENAME(1,Uint16Array)
TYPENAME(1,Uint32Array)
TYPENAME(1,Uint8Array)
TYPENAME(1,Uint8ClampedArray)
TYPENAME(1,WeakMap)
TYPENAME(1,WeakSet)

// Syntax: TYPENAME_MAP(Js::TypeId,TypeName)
// This list maps Js::TypeId to ESBuiltInTypeNameId. These entries do not have to be sorted in any particular order.
// 1. Open EdgeJavascriptTypeId.h
// 2. Use this RegEx F+R (excluding backticks)
// 2.1. `        (\w+) = (\d+),` -> `TYPENAME_MAP( $1,  )`
// 3. Entries that I'm not sure of are commented out using /*...*/
// 4. Finally add the correct TypeName from above as the second argument.

TYPENAME_MAP( TypeIds_Undefined, Object )
TYPENAME_MAP( TypeIds_Null, Object )
TYPENAME_MAP( TypeIds_Boolean, Boolean )
TYPENAME_MAP( TypeIds_Integer, Number )
TYPENAME_MAP( TypeIds_Number, Number )
TYPENAME_MAP( TypeIds_Int64Number, Number )
TYPENAME_MAP( TypeIds_UInt64Number, Number )
TYPENAME_MAP( TypeIds_String, String )
TYPENAME_MAP( TypeIds_Symbol, Symbol )
/*TYPENAME_MAP( TypeIds_Enumerator, Object )*/
TYPENAME_MAP( TypeIds_VariantDate, Date )
TYPENAME_MAP( TypeIds_HostDispatch, Object )
TYPENAME_MAP( TypeIds_WithScopeObject, Object )
/*TYPENAME_MAP( TypeIds_UndeclBlockVar, Object )*/
TYPENAME_MAP( TypeIds_Proxy, Object )
TYPENAME_MAP( TypeIds_Function, Function )
TYPENAME_MAP( TypeIds_Object, Object )
TYPENAME_MAP( TypeIds_Array, Array )
TYPENAME_MAP( TypeIds_NativeIntArray, Int32Array )
TYPENAME_MAP( TypeIds_NativeFloatArray, Float32Array )
TYPENAME_MAP( TypeIds_Date, Date )
TYPENAME_MAP( TypeIds_RegEx, RegExp )
TYPENAME_MAP( TypeIds_Error, Error )
TYPENAME_MAP( TypeIds_BooleanObject, Boolean )
TYPENAME_MAP( TypeIds_NumberObject, Number )
TYPENAME_MAP( TypeIds_StringObject, String )
/*TYPENAME_MAP( TypeIds_ExtensionEnumerator,  )*/
/*TYPENAME_MAP( TypeIds_Arguments, Array? )*/
TYPENAME_MAP( TypeIds_ES5Array, Array )
TYPENAME_MAP( TypeIds_PixelArray, Array )
TYPENAME_MAP( TypeIds_ArrayBuffer, ArrayBuffer )
TYPENAME_MAP( TypeIds_Int8Array, Int8Array )
TYPENAME_MAP( TypeIds_Uint8Array, Uint8Array )
TYPENAME_MAP( TypeIds_Uint8ClampedArray, Uint8ClampedArray )
TYPENAME_MAP( TypeIds_Int16Array, Int16Array )
TYPENAME_MAP( TypeIds_Uint16Array, Uint16Array )
TYPENAME_MAP( TypeIds_Int32Array, Int32Array )
TYPENAME_MAP( TypeIds_Uint32Array, Uint32Array )
TYPENAME_MAP( TypeIds_Float32Array, Float32Array )
TYPENAME_MAP( TypeIds_Float64Array, Float64Array )
TYPENAME_MAP( TypeIds_Int64Array, Array )
TYPENAME_MAP( TypeIds_Uint64Array, Array )
TYPENAME_MAP( TypeIds_CharArray, Array )
TYPENAME_MAP( TypeIds_BoolArray, Array )
/*TYPENAME_MAP( TypeIds_EngineInterfaceObject,  )*/
TYPENAME_MAP( TypeIds_DataView, DataView )
/*TYPENAME_MAP( TypeIds_WinRTDate, Date? )*/
TYPENAME_MAP( TypeIds_Map, Map )
TYPENAME_MAP( TypeIds_Set, Set )
TYPENAME_MAP( TypeIds_WeakMap, WeakMap )
TYPENAME_MAP( TypeIds_WeakSet, WeakSet )
TYPENAME_MAP( TypeIds_SymbolObject, Symbol )
/*TYPENAME_MAP( TypeIds_ArrayIterator,  )*/
/*TYPENAME_MAP( TypeIds_MapIterator,  )*/
/*TYPENAME_MAP( TypeIds_SetIterator,  )*/
TYPENAME_MAP( TypeIds_StringIterator, StringIterator )
/*TYPENAME_MAP( TypeIds_JavascriptEnumeratorIterator,  )*/
TYPENAME_MAP( TypeIds_Generator, Generator )
TYPENAME_MAP( TypeIds_Promise, Promise )
TYPENAME_MAP( TypeIds_GlobalObject, Global )
/*TYPENAME_MAP( TypeIds_ModuleRoot,  )*/
/*TYPENAME_MAP( TypeIds_HostObject,  )*/
/*TYPENAME_MAP( TypeIds_ActivationObject,  )*/
