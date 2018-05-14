//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#ifndef ENTRY_BUILTIN
#error ENTRY_BUILTIN is not defined
#endif

// Array Builtins
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


// Object builtins
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


// Date builtins
ENTRY_BUILTIN(ES5, Date, Prototype, toISOString)

// Function builtins
ENTRY_BUILTIN(ES5, Function, Prototype, bind)


// String builtins
ENTRY_BUILTIN(ES5, String, Prototype, trim)
ENTRY_BUILTIN(ES6, String, Prototype, startsWith)
ENTRY_BUILTIN(ES6, String, Prototype, endsWith)
ENTRY_BUILTIN(ES6, String, Prototype, contains)
ENTRY_BUILTIN(ES6, String, Prototype, repeat)
ENTRY_BUILTIN(ES7, String, Prototype, padStart)
ENTRY_BUILTIN(ES7, String, Prototype, padEnd)
ENTRY_BUILTIN(ES7, String, Prototype, at)
ENTRY_BUILTIN(ES7, String, Prototype, substr)
ENTRY_BUILTIN(ES7, String, Prototype, trimLeft)
ENTRY_BUILTIN(ES7, String, Prototype, trimRight)


//ES6 builtins
//Math builtins
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

// Number builtins
ENTRY_BUILTIN(ES6, Number, Constructor, isNaN)
ENTRY_BUILTIN(ES6, Number, Constructor, isFinite)
ENTRY_BUILTIN(ES6, Number, Constructor, isInteger)
ENTRY_BUILTIN(ES6, Number, Constructor, isSafeInteger)


//TypedArray Methods
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

// ES7 stuff, formerly in BuiltinsDatabase.inc
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

// Some global stuff we want to track
ENTRY_BUILTIN(Wasm, Global, Prototype, WebAssembly)

// Intl
ENTRY_BUILTIN(Intl, Collator, Prototype, compare)
ENTRY_BUILTIN(Intl, String, Prototype, localeCompare)
ENTRY_BUILTIN(Intl, NumberFormat, Prototype, format)
ENTRY_BUILTIN(Intl, NumberFormat, Prototype, formatToParts)
ENTRY_BUILTIN(Intl, Number, Prototype, toLocaleString)
ENTRY_BUILTIN(Intl, DateTimeFormat, Prototype, format)
ENTRY_BUILTIN(Intl, DateTimeFormat, Prototype, formatToParts)
ENTRY_BUILTIN(Intl, Date, Prototype, toLocaleString)
ENTRY_BUILTIN(Intl, PluralRules, Prototype, select)
