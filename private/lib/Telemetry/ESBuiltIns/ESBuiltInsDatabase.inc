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



