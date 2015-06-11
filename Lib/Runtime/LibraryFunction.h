//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

/*
 * These are javascript library functions that might be inlined
 * by the JIT.
 *
 * Notes:
 * - the argc is the number of args to pass to InlineXXX call, e.g. 2 for Math.pow and 2 for String.CharAt.
 * - TODO: consider having dst/src1/src2 in separate columns rather than bitmask, this seems to be better for design but we won't be able to see 'all float' by single check.
 * - TODO: enable string inlines when string type spec is avaiable
 *
 *               target         name                argc  flags
 */
LIBRARY_FUNCTION(Math,          Abs,                1,    BIF_TypeSpecSrcAndDstToFloatOrInt)
LIBRARY_FUNCTION(Math,          Acos,               1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Asin,               1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Atan,               1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Atan2,              2,    BIF_TypeSpecAllToFloat)
LIBRARY_FUNCTION(Math,          Ceil,               1,    BIF_TypeSpecDstToInt | BIF_TypeSpecSrc1ToFloat)
LIBRARY_FUNCTION(String,        CodePointAt,        2,    /*BIF_TypeSpecDstToInt    | BIF_TypeSpecSrc1ToString |*/ BIF_TypeSpecSrc2ToInt | BIF_UseSrc0)
LIBRARY_FUNCTION(String,        CharAt,             2,    BIF_UseSrc0)
LIBRARY_FUNCTION(String,        CharCodeAt,         2,    BIF_UseSrc0)
LIBRARY_FUNCTION(String,        Concat,             15,   BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(String,        FromCharCode,       1,    BIF_None)
LIBRARY_FUNCTION(String,        FromCodePoint,      1,    BIF_None)
LIBRARY_FUNCTION(String,        IndexOf,            3,    BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(String,        LastIndexOf,        3,    BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(String,        Link,               2,    BIF_UseSrc0)
LIBRARY_FUNCTION(String,        LocaleCompare,      2,    BIF_UseSrc0)
LIBRARY_FUNCTION(String,        Match,              2,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        Replace,            3,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        Search,             2,    BIF_UseSrc0)
LIBRARY_FUNCTION(String,        Slice,              3,    BIF_UseSrc0 | BIF_VariableArgsNumber) 
LIBRARY_FUNCTION(String,        Split,              3,    BIF_UseSrc0 | BIF_VariableArgsNumber | BIF_IgnoreDst) 
LIBRARY_FUNCTION(String,        Substr,             3,    BIF_UseSrc0 | BIF_VariableArgsNumber) 
LIBRARY_FUNCTION(String,        Substring,          3,    BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(String,        ToLocaleLowerCase,  1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        ToLocaleUpperCase,  1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        ToLowerCase,        1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        ToUpperCase,        1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        Trim,               1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        TrimLeft,           1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(String,        TrimRight,          1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(Math,          Cos,                1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Exp,                1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Floor,              1,    BIF_TypeSpecDstToInt | BIF_TypeSpecSrc1ToFloat)
LIBRARY_FUNCTION(Math,          Log,                1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Max,                2,    BIF_TypeSpecSrcAndDstToFloatOrInt)
LIBRARY_FUNCTION(Math,          Min,                2,    BIF_TypeSpecSrcAndDstToFloatOrInt)
LIBRARY_FUNCTION(Math,          Pow,                2,    BIF_TypeSpecAllToFloat)
LIBRARY_FUNCTION(Math,          Imul,               2,    BIF_TypeSpecAllToInt)
LIBRARY_FUNCTION(Math,          Clz32,              1,    BIF_TypeSpecAllToInt)
LIBRARY_FUNCTION(Array,         Push,               2,    BIF_UseSrc0 | BIF_IgnoreDst | BIF_TypeSpecSrc1ToFloatOrInt) 
LIBRARY_FUNCTION(Array,         Pop,                1,    BIF_UseSrc0 | BIF_TypeSpecDstToFloatOrInt)
LIBRARY_FUNCTION(Math,          Random,             0,    BIF_TypeSpecDstToFloat)
LIBRARY_FUNCTION(Math,          Round,              1,    BIF_TypeSpecDstToInt | BIF_TypeSpecSrc1ToFloat)
LIBRARY_FUNCTION(Math,          Sin,                1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Sqrt,               1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Math,          Tan,                1,    BIF_TypeSpecUnaryToFloat)
LIBRARY_FUNCTION(Array,         Concat,             15,   BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(Array,         IndexOf,            2,    BIF_UseSrc0)
LIBRARY_FUNCTION(Array,         IsArray,            1,    BIF_VariableArgsNumber) //TODO (radua): implement as a typeId check
LIBRARY_FUNCTION(Array,         Join,               2,    BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(Array,         LastIndexOf,        3,    BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(Array,         Reverse,            1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(Array,         Shift,              1,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(Array,         Slice,              3,    BIF_UseSrc0 | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(Array,         Splice,             15,   BIF_UseSrc0 | BIF_VariableArgsNumber | BIF_IgnoreDst)
LIBRARY_FUNCTION(Array,         Unshift,            15,   BIF_UseSrc0 | BIF_VariableArgsNumber | BIF_IgnoreDst)
LIBRARY_FUNCTION(Function,      Apply,              3,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(Function,      Call,               15,   BIF_UseSrc0 | BIF_IgnoreDst | BIF_VariableArgsNumber)
LIBRARY_FUNCTION(GlobalObject,  ParseInt,           1,    BIF_IgnoreDst)
LIBRARY_FUNCTION(RegExp,        Exec,               2,    BIF_UseSrc0 | BIF_IgnoreDst)
LIBRARY_FUNCTION(Math,          Fround,             1,    BIF_TypeSpecUnaryToFloat)
#if 0
// TODO: Implement inlining of the following ES6 Math functions
LIBRARY_FUNCTION(Math,          Log10,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Log2,               1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Log1p,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Expm1,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Cosh,               1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Sinh,               1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Tanh,               1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Acosh,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Asinh,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Atanh,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Hypot,              2,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Trunc,              1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Sign,               1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Cbrt,               1,    /* TODO: What flags should this function have? */)
LIBRARY_FUNCTION(Math,          Clz32,              1,    /* TODO: What flags should this function have? */)
#endif
// Note: 1st column is currently used only for debug tracing.
