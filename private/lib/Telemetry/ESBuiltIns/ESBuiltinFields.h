//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

// This is a sort of master file where you can define builtins that you'd like some
// telemetry data for. It is then included in a bunch of different other files, and
// each of those defines preprocessor macros appropriately such that this file gets
// massaged into the desired format for that include location.

// Format is:
// ENTRY_BUILTIN(           -- macro key
//  ES5/6/7                 -- EMCAScript version feature was added (barely used)
//  Array/Object/etc        -- base object on which your method resides
//  Constructor/Prototype   -- is your method "X.afunc()" or "X.prototype.afunc()"?
//  functionname            -- "afunc" - the name of your function (get caps right)
// )
//
// If you have a language feature that you want to count the number of parses for:
// ENTRY_LANGFEATURE(
//  ES5/6/7                 -- EMCAScript version feature was added (barely used)
//  pointname               -- the name for the telemetry point
// )
//
// If you have some other telemetry counter that you'd like to track on a per-load,
// per script-context basis:
// ENTRY_TELPOINT(
//  pointname               -- the name for the telemetry point
// )

// Array builtins
// We're especially concerned about these, due to the prevalence of arrays in perf-
// important scenarios
BLOCK_START(Array, 14)
ENTRY_BUILTIN(ES5, Array, Constructor, isArray)
ENTRY_BUILTIN(ES5, Array, Prototype, indexOf)
ENTRY_BUILTIN(ES5, Array, Prototype, includes)
ENTRY_BUILTIN(ES5, Array, Prototype, every)
ENTRY_BUILTIN(ES5, Array, Prototype, filter)
ENTRY_BUILTIN(ES5, Array, Prototype, forEach)
ENTRY_BUILTIN(ES5, Array, Prototype, lastIndexOf)
ENTRY_BUILTIN(ES5, Array, Prototype, map)
ENTRY_BUILTIN(ES5, Array, Prototype, reduce)
ENTRY_BUILTIN(ES5, Array, Prototype, reduceRight)
ENTRY_BUILTIN(ES5, Array, Prototype, some)
ENTRY_BUILTIN(ES7, Array, Prototype, contains)
ENTRY_BUILTIN(ES7, Array, Constructor, observe)
ENTRY_BUILTIN(ES7, Array, Constructor, unobserve)
BLOCK_END()

// Object builtins
BLOCK_START(Object, 18)
ENTRY_BUILTIN(ES5, Object, Constructor, defineProperty)
ENTRY_BUILTIN(ES5, Object, Constructor, defineProperties)
ENTRY_BUILTIN(ES5, Object, Constructor, create)
ENTRY_BUILTIN(ES5, Object, Constructor, seal)
ENTRY_BUILTIN(ES5, Object, Constructor, freeze)
ENTRY_BUILTIN(ES5, Object, Constructor, preventExtensions)
ENTRY_BUILTIN(ES5, Object, Constructor, isSealed)
ENTRY_BUILTIN(ES5, Object, Constructor, isFrozen)
ENTRY_BUILTIN(ES5, Object, Constructor, isExtensible)
ENTRY_BUILTIN(ES5, Object, Constructor, getOwnPropertyNames)
ENTRY_BUILTIN(ES5, Object, Constructor, getPrototypeOf)
ENTRY_BUILTIN(ES5, Object, Constructor, keys)
ENTRY_BUILTIN(ES6, Object, Constructor, getOwnPropertySymbols)
ENTRY_BUILTIN(ES7, Object, Constructor, values)
ENTRY_BUILTIN(ES7, Object, Constructor, entries)
ENTRY_BUILTIN(ES7, Object, Constructor, getOwnPropertyDescriptors)
ENTRY_BUILTIN(ES7, Object, Constructor, observe)
ENTRY_BUILTIN(ES7, Object, Constructor, unobserve)
BLOCK_END()

// Date builtins
BLOCK_START(Date, 1)
ENTRY_BUILTIN(ES5, Date, Prototype, toISOString)
BLOCK_END()

// Function builtins
BLOCK_START(Function, 1)
ENTRY_BUILTIN(ES5, Function, Prototype, bind)
BLOCK_END()

// String builtins
// Also very important for some perf scenarios
BLOCK_START(String, 11)
ENTRY_BUILTIN(ES5, String, Prototype, trim)
ENTRY_BUILTIN(ES6, String, Prototype, startsWith)
ENTRY_BUILTIN(ES6, String, Prototype, endsWith)
ENTRY_BUILTIN(ES6, String, Prototype, contains)
ENTRY_BUILTIN(ES6, String, Prototype, repeat)
ENTRY_BUILTIN(ES7, String, Prototype, padStart)
ENTRY_BUILTIN(ES7, String, Prototype, padEnd)
ENTRY_BUILTIN(ES7, String, Prototype, at)
//ENTRY_BUILTIN(ES7, String, Prototype, leftPad)
//ENTRY_BUILTIN(ES7, String, Prototype, lPad)
//ENTRY_BUILTIN(ES7, String, Prototype, padLeft)
//ENTRY_BUILTIN(ES7, String, Prototype, padRight)
//ENTRY_BUILTIN(ES7, String, Prototype, rightPad)
//ENTRY_BUILTIN(ES7, String, Prototype, rPad)
ENTRY_BUILTIN(ES7, String, Prototype, substr)
ENTRY_BUILTIN(ES7, String, Prototype, trimLeft)
ENTRY_BUILTIN(ES7, String, Prototype, trimRight)
BLOCK_END()

//ES6 builtins
//Math builtins
BLOCK_START(Math, 17)
ENTRY_BUILTIN(ES6, Math, Constructor, log10)
ENTRY_BUILTIN(ES6, Math, Constructor, log1p)
ENTRY_BUILTIN(ES6, Math, Constructor, log2)
ENTRY_BUILTIN(ES6, Math, Constructor, expm1)
ENTRY_BUILTIN(ES6, Math, Constructor, sinh)
ENTRY_BUILTIN(ES6, Math, Constructor, cosh)
ENTRY_BUILTIN(ES6, Math, Constructor, tanh)
ENTRY_BUILTIN(ES6, Math, Constructor, asinh)
ENTRY_BUILTIN(ES6, Math, Constructor, acosh)
ENTRY_BUILTIN(ES6, Math, Constructor, atanh)
ENTRY_BUILTIN(ES6, Math, Constructor, hypot)
ENTRY_BUILTIN(ES6, Math, Constructor, cbrt)
ENTRY_BUILTIN(ES6, Math, Constructor, trunc)
ENTRY_BUILTIN(ES6, Math, Constructor, sign)
ENTRY_BUILTIN(ES6, Math, Constructor, imul)
ENTRY_BUILTIN(ES6, Math, Constructor, clz32)
ENTRY_BUILTIN(ES6, Math, Constructor, fround)
BLOCK_END()

// Number builtins
BLOCK_START(Number, 4)
ENTRY_BUILTIN(ES6, Number, Constructor, isNaN)
ENTRY_BUILTIN(ES6, Number, Constructor, isFinite)
ENTRY_BUILTIN(ES6, Number, Constructor, isInteger)
ENTRY_BUILTIN(ES6, Number, Constructor, isSafeInteger)
BLOCK_END()

// RegEx builtins
ENTRY_TELPOINT(ES6_RegexSymbolMatch)
ENTRY_TELPOINT(ES6_RegexSymbolSearch)
ENTRY_TELPOINT(ES6_RegexSymbolReplace)
ENTRY_TELPOINT(ES6_RegexSymbolSplit)

//Ctors
ENTRY_TELPOINT(ES6_Proxy)
ENTRY_TELPOINT(ES6_Symbol)
ENTRY_TELPOINT(ES6_Map)
ENTRY_TELPOINT(ES6_WeakMap)
ENTRY_TELPOINT(ES6_WeakSet)
ENTRY_TELPOINT(ES6_Set)
ENTRY_TELPOINT(ES6_Promise)

//TypedArray Methods
BLOCK_START(TypedArray, 23)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, from)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, of)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, copyWithin)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, entries)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, every)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, fill)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, filter)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, find)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, findIndex)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, forEach)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, includes)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, indexOf)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, join)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, keys)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, lastIndexOf)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, map)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, reduce)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, reduceRight)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, reverse)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, some)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, sort)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, subarray)
ENTRY_BUILTIN(ES6, TypedArray, Prototype, values)
BLOCK_END()

// Language Features
ENTRY_LANGFEATURE(ES6, Let)
ENTRY_LANGFEATURE(ES6, Lambda)
ENTRY_LANGFEATURE(ES6, StrictModeFunction)
ENTRY_LANGFEATURE(ES6, Super)
ENTRY_LANGFEATURE(ES6, Class)
ENTRY_LANGFEATURE(ES6, AsmJSFunction)
ENTRY_LANGFEATURE(ES6, StringTemplates)
ENTRY_LANGFEATURE(ES6, Const)
ENTRY_LANGFEATURE(ES6, Generator)
ENTRY_LANGFEATURE(ES6, Rest)
ENTRY_LANGFEATURE(ES6, SpreadFeature)
ENTRY_LANGFEATURE(ES6, UnicodeRegexFlag)
ENTRY_LANGFEATURE(ES6, StickyRegexFlag)
ENTRY_LANGFEATURE(ES6, DefaultArgFunction)

// ES7 stuff, formerly in BuiltinsDatabase.inc
BLOCK_START(Other, 12)
ENTRY_BUILTIN(ES7, Float32Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Float64Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Int16Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Int32Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Int8Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Map, Prototype, toJSON)
ENTRY_BUILTIN(ES7, RegExp, Constructor, escape)
ENTRY_BUILTIN(ES7, Set, Prototype, toJSON)
ENTRY_BUILTIN(ES7, Uint16Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Uint32Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Uint8Array, Prototype, includes)
ENTRY_BUILTIN(ES7, Uint8ClampedArray, Prototype, includes)
BLOCK_END()

// Some global stuff we want to track
BLOCK_START(Global, 1)
ENTRY_BUILTIN(Wasm, Global, Prototype, WebAssembly)
BLOCK_END()
