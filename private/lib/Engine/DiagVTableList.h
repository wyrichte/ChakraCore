//
// This file is included multiple times to list vtable entries needed by jscript9diag.
//

#ifndef ENTRY
#define ENTRY(s)
#endif

#ifndef MARK_ENTRY
#define MARK_ENTRY(mark, entry)
#endif

//
// projection vtable entries
//
#ifndef PROJECTION_ENTRY
#define PROJECTION_ENTRY(s) ENTRY(s)
#endif
PROJECTION_ENTRY(ProjectionObjectInstance)
PROJECTION_ENTRY(EventHandlingProjectionObjectInstance)
#undef PROJECTION_ENTRY

//
// ActivationObject vtable entries
//
ENTRY(ActivationObject)
ENTRY(ActivationObjectEx)
ENTRY(PseudoActivationObject)
ENTRY(BlockActivationObject)

MARK_ENTRY(FirstActivationObject,   ActivationObject)
MARK_ENTRY(LastActivationObject,    BlockActivationObject)

//
// Function vtable entries
//
ENTRY(JavascriptFunction)
ENTRY(BoundFunction)

MARK_ENTRY(FirstNoSourceJavascriptFunction,   JavascriptFunction)
MARK_ENTRY(LastNoSourceJavascriptFunction,    BoundFunction)

//
// Misc vtable entries
//
ENTRY(JavascriptRegExpConstructor)
ENTRY(FunctionBody)

//
// type handlers
//
#ifndef TYPEHANDLER_ENTRY
#define TYPEHANDLER_ENTRY(s) ENTRY(s)
#endif
TYPEHANDLER_ENTRY(NonProtoNullTypeHandler)
TYPEHANDLER_ENTRY(ProtoNullTypeHandler)
TYPEHANDLER_ENTRY(SimpleTypeHandlerSize1)
TYPEHANDLER_ENTRY(SimpleTypeHandlerSize2)
TYPEHANDLER_ENTRY(SimplePathTypeHandler)
TYPEHANDLER_ENTRY(PathTypeHandler)
TYPEHANDLER_ENTRY(SimpleDictionaryTypeHandler)
TYPEHANDLER_ENTRY(SimpleDictionaryTypeHandlerNotExtensible)
TYPEHANDLER_ENTRY(BigSimpleDictionaryTypeHandler)
TYPEHANDLER_ENTRY(BigSimpleDictionaryTypeHandlerNotExtensible)
TYPEHANDLER_ENTRY(SimpleDictionaryUnorderedPropertyRecordKeyedTypeHandler)
TYPEHANDLER_ENTRY(SimpleDictionaryUnorderedPropertyRecordKeyedTypeHandlerNotExtensible)
TYPEHANDLER_ENTRY(BigSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandler)
TYPEHANDLER_ENTRY(BigSimpleDictionaryUnorderedPropertyRecordKeyedTypeHandlerNotExtensible)
TYPEHANDLER_ENTRY(SimpleDictionaryUnorderedStringKeyedTypeHandler)
TYPEHANDLER_ENTRY(SimpleDictionaryUnorderedStringKeyedTypeHandlerNotExtensible)
TYPEHANDLER_ENTRY(BigSimpleDictionaryUnorderedStringKeyedTypeHandler)
TYPEHANDLER_ENTRY(BigSimpleDictionaryUnorderedStringKeyedTypeHandlerNotExtensible)
TYPEHANDLER_ENTRY(DictionaryTypeHandler)
TYPEHANDLER_ENTRY(BigDictionaryTypeHandler)
TYPEHANDLER_ENTRY(ES5ArrayTypeHandler)
TYPEHANDLER_ENTRY(BigES5ArrayTypeHandler)


MARK_ENTRY(FirstTypeHandler,    NonProtoNullTypeHandler)
MARK_ENTRY(LastTypeHandler,     BigES5ArrayTypeHandler)
#undef TYPEHANDLER_ENTRY

//
// strings
//
#ifndef STRING_ENTRY
#define STRING_ENTRY(s) ENTRY(s)
#endif
#ifndef STRING_ENTRY_TEMPLATE
#define STRING_ENTRY_TEMPLATE(s, c, ...) STRING_ENTRY(s)
#endif
STRING_ENTRY(LiteralString)
STRING_ENTRY(ArenaLiteralString)
STRING_ENTRY(SingleCharString)
STRING_ENTRY(PropertyString)
STRING_ENTRY(ConcatString)
STRING_ENTRY(JSONString)
STRING_ENTRY_TEMPLATE(ConcatStringN2, ConcatStringN, 2)
STRING_ENTRY_TEMPLATE(ConcatStringN4, ConcatStringN, 4)
STRING_ENTRY_TEMPLATE(ConcatStringN7, ConcatStringN, 7)
STRING_ENTRY(ConcatStringBuilder)
STRING_ENTRY_TEMPLATE(ConcatStringWrappingSB, ConcatStringWrapping, _u('['), _u(']'))
STRING_ENTRY_TEMPLATE(ConcatStringWrappingB,  ConcatStringWrapping, _u('{'), _u('}'))
STRING_ENTRY_TEMPLATE(ConcatStringWrappingQ,  ConcatStringWrapping, _u('"'), _u('"'))
STRING_ENTRY(ConcatStringMulti)
STRING_ENTRY(CompoundString)
STRING_ENTRY(BufferStringBuilderWritableString)
STRING_ENTRY(SubString)
STRING_ENTRY(LiteralStringWithPropertyStringPtr)

MARK_ENTRY(FirstString,         LiteralString)
MARK_ENTRY(LastString,          LiteralStringWithPropertyStringPtr)
#undef STRING_ENTRY
#undef STRING_ENTRY_TEMPLATE

#undef MARK_ENTRY
#undef ENTRY
