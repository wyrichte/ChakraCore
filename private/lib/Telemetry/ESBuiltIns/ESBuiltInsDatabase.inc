// Syntax: BUILTIN(Object,Location,PropertyName,PropertyKind,ESVersion)
// This list MUST be sorted in Object,PropertyName ASC order!

// Follow these instructions if this file is being generated from a CSV file from the E&I Team's ECMAScript properties spreadsheet:
// This is the RegEx-based F+R I use to convert the CSV into this file:
// 1. `\r\n` -> `)\r\nBUILTIN(`.
// 2. `ES(\d).*\)` -> `ES$1)`.
// 3. `Proprietary.*\)` -> `Proprietary)`.
// 4. `Global,` -> `GlobalObject,`.
// 5. Trim the first line (headers) and the last line (empty BUILTIN macro).
// 6. Remove rows that were removed from the spec (e.g.Reflect.Loader and Reflect.Realm) if present.
// 7. Replace:
// 7.1. `,delete,` -> `,delete_,`
// 7.2. `,return,` -> `,return_,`
// 7.3. `,throw,` -> `,throw_,`
// 7.4. `,catch,` -> `,catch_,`

// After creating this file from the spreadsheet or CSV file you must perform these steps in-order.
// 1. Re-generate TraceLogList.inc (`cscript TraceLogList.Generator.js ESBuiltInsDatabase.inc TraceLogList.inc`)
// 2. Re-generate ESBuiltInsTypeNames.inc (a manual process, unfortunately, instructions contained within)
// 3. Re-generate ESBuiltInsTypeNames.trie.inc (`cscript ESBuiltInsTypeNames.trie.Generator.js ESBuiltInsTypeNames.inc ESBuiltInsTypeNames.trie.inc value valueLength`)


BUILTIN(Array,Prototype,contains,Function,ES7)
BUILTIN(Array,Prototype,includes,Function,ES7)
BUILTIN(Array,Constructor,observe,Function,ES7)
BUILTIN(Array,Constructor,unobserve,Function,ES7)
BUILTIN(ArrayBuffer,Constructor,transfer,Function,ES7)
BUILTIN(Float32Array,Prototype,includes,Function,ES7)
BUILTIN(Float64Array,Prototype,includes,Function,ES7)
BUILTIN(Int16Array,Prototype,includes,Function,ES7)
BUILTIN(Int32Array,Prototype,includes,Function,ES7)
BUILTIN(Int8Array,Prototype,includes,Function,ES7)
BUILTIN(Map,Prototype,toJSON,Function,ES7)
BUILTIN(Object,Constructor,getOwnPropertyDescriptors,Function,ES7)
BUILTIN(Object,Constructor,observe,Function,ES7)
BUILTIN(Object,Constructor,unobserve,Function,ES7)
BUILTIN(RegExp,Constructor,escape,Function,ES7)
BUILTIN(Set,Prototype,toJSON,Function,ES7)
BUILTIN(String,Prototype,at,Function,ES7)
BUILTIN(String,Prototype,leftPad,Function,ES7)
BUILTIN(String,Prototype,lPad,Function,ES7)
BUILTIN(String,Prototype,padLeft,Function,ES7)
BUILTIN(String,Prototype,padRight,Function,ES7)
BUILTIN(String,Prototype,rightPad,Function,ES7)
BUILTIN(String,Prototype,rPad,Function,ES7)
BUILTIN(String,Prototype,substr,Function,Proprietary)
BUILTIN(String,Prototype,trimLeft,Function,ES7)
BUILTIN(String,Prototype,trimRight,Function,ES7)
BUILTIN(Uint16Array,Prototype,includes,Function,ES7)
BUILTIN(Uint32Array,Prototype,includes,Function,ES7)
BUILTIN(Uint8Array,Prototype,includes,Function,ES7)
BUILTIN(Uint8ClampedArray,Prototype,includes,Function,ES7)
