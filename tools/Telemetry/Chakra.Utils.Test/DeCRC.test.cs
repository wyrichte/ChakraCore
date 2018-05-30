using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Chakra.Utils;


namespace Chakra.Utils.Test
{
    [TestClass]
    public class DeCRCTest
    {
        [TestMethod]
        public void TestBuiltInCRCs()
        {
            string[] inputs = {
                "Array_Constructor_isArray",
                "Array_Prototype_indexOf",
                "Array_Prototype_includes",
                "Array_Prototype_every",
                "Array_Prototype_filter",
                "Array_Prototype_forEach",
                "Array_Prototype_lastIndexOf",
                "Array_Prototype_map",
                "Array_Prototype_reduce",
                "Array_Prototype_reduceRight",
                "Array_Prototype_some",
                "Array_Prototype_contains",
                "Array_Constructor_observe",
                "Array_Constructor_unobserve",
                "Object_Constructor_defineProperty",
                "Object_Constructor_defineProperties",
                "Object_Constructor_create",
                "Object_Constructor_seal",
                "Object_Constructor_freeze",
                "Object_Constructor_preventExtensions",
                "Object_Constructor_isSealed",
                "Object_Constructor_isFrozen",
                "Object_Constructor_isExtensible",
                "Object_Constructor_getOwnPropertyNames",
                "Object_Constructor_getPrototypeOf",
                "Object_Constructor_keys",
                "Object_Constructor_getOwnPropertySymbols",
                "Object_Constructor_values",
                "Object_Constructor_entries",
                "Object_Constructor_getOwnPropertyDescriptors",
                "Object_Constructor_observe",
                "Object_Constructor_unobserve",
                "Date_Prototype_toISOString",
                "Function_Prototype_bind",
                "String_Prototype_trim",
                "String_Prototype_startsWith",
                "String_Prototype_endsWith",
                "String_Prototype_contains",
                "String_Prototype_repeat",
                "String_Prototype_padStart",
                "String_Prototype_padEnd",
                "String_Prototype_at",
                "String_Prototype_substr",
                "String_Prototype_trimLeft",
                "String_Prototype_trimRight",
                "Math_Constructor_log10",
                "Math_Constructor_log1p",
                "Math_Constructor_log2",
                "Math_Constructor_expm1",
                "Math_Constructor_sinh",
                "Math_Constructor_cosh",
                "Math_Constructor_tanh",
                "Math_Constructor_asinh",
                "Math_Constructor_acosh",
                "Math_Constructor_atanh",
                "Math_Constructor_hypot",
                "Math_Constructor_cbrt",
                "Math_Constructor_trunc",
                "Math_Constructor_sign",
                "Math_Constructor_imul",
                "Math_Constructor_clz32",
                "Math_Constructor_fround",
                "Number_Constructor_isNaN",
                "Number_Constructor_isFinite",
                "Number_Constructor_isInteger",
                "Number_Constructor_isSafeInteger",
                "TypedArray_Prototype_from",
                "TypedArray_Prototype_of",
                "TypedArray_Prototype_copyWithin",
                "TypedArray_Prototype_entries",
                "TypedArray_Prototype_every",
                "TypedArray_Prototype_fill",
                "TypedArray_Prototype_filter",
                "TypedArray_Prototype_find",
                "TypedArray_Prototype_findIndex",
                "TypedArray_Prototype_forEach",
                "TypedArray_Prototype_includes",
                "TypedArray_Prototype_indexOf",
                "TypedArray_Prototype_join",
                "TypedArray_Prototype_keys",
                "TypedArray_Prototype_lastIndexOf",
                "TypedArray_Prototype_map",
                "TypedArray_Prototype_reduce",
                "TypedArray_Prototype_reduceRight",
                "TypedArray_Prototype_reverse",
                "TypedArray_Prototype_some",
                "TypedArray_Prototype_sort",
                "TypedArray_Prototype_subarray",
                "TypedArray_Prototype_values",
                "Float32Array_Prototype_includes",
                "Float64Array_Prototype_includes",
                "Int16Array_Prototype_includes",
                "Int32Array_Prototype_includes",
                "Int8Array_Prototype_includes",
                "Map_Prototype_toJSON",
                "RegExp_Constructor_escape",
                "Set_Prototype_toJSON",
                "Uint16Array_Prototype_includes",
                "Uint32Array_Prototype_includes",
                "Uint8Array_Prototype_includes",
                "Uint8ClampedArray_Prototype_includes",
                "Global_Prototype_WebAssembly",
                "Collator_Prototype_compare",
                "String_Prototype_localeCompare",
                "NumberFormat_Prototype_format",
                "NumberFormat_Prototype_formatToParts",
                "Number_Prototype_toLocaleString",
                "DateTimeFormat_Prototype_format",
                "DateTimeFormat_Prototype_formatToParts",
                "Date_Prototype_toLocaleString",
                "PluralRules_Prototype_select"
            };

            foreach (string s in inputs)
            {
                Assert.AreEqual(s, DeCRC.GetStringForCRC(DeCRC.CRC32(s)));
            }
        }


        [TestMethod]
        public void TestBailOutCRCs()
        {
            string[] inputs = {
                "BailOutInvalid",
                "BailOutIntOnly",
                "BailOutNumberOnly",
                "BailOutPrimitiveButString",
                "BailOutOnImplicitCalls",
                "BailOutOnImplicitCallsPreOp",
                "BailOutOnNotPrimitive",
                "BailOutOnMemOpError",
                "BailOutOnInlineFunction",
                "BailOutOnNoProfile",
                "BailOutOnPolymorphicInlineFunction",
                "BailOutOnFailedPolymorphicInlineTypeCheck",
                "BailOutShared",
                "BailOutOnNotArray",
                "BailOutOnNotNativeArray",
                "BailOutConventionalTypedArrayAccessOnly",
                "BailOutOnIrregularLength",
                "BailOutCheckThis",
                "BailOutOnTaggedValue",
                "BailOutFailedTypeCheck",
                "BailOutFailedEquivalentTypeCheck",
                "BailOutInjected",
                "BailOutExpectingInteger",
                "BailOutExpectingString",
                "BailOutFailedInlineTypeCheck",
                "BailOutFailedFixedFieldTypeCheck",
                "BailOutFailedFixedFieldCheck",
                "BailOutFailedEquivalentFixedFieldTypeCheck",
                "BailOutOnFloor",
                "BailOnModByPowerOf2",
                "BailOnIntMin",
                "BailOnDivResultNotInt",
                "BailOnSimpleJitToFullJitLoopBody",
                "BailOutFailedCtorGuardCheck",
                "BailOutOnFailedHoistedBoundCheck",
                "LazyBailOut",
                "BailOutOnFailedHoistedLoopCountBasedBoundCheck",
                "BailOutForGeneratorYield",
                "BailOutOnException",
                "BailOutOnEarlyExit",
                "BailOutKindEnd",
                "BailOutKindBitsStart",
                "BailOutOnMulOverflow",
                "BailOutOnNegativeZero",
                "BailOutOnPowIntIntOverflow",
                "BailOutOnResultConditions",
                "BailOutConventionalNativeArrayAccessOnly",
                "BailOutConvertedNativeArray",
                "BailOutOnArrayAccessHelperCall",
                "BailOutOnInvalidatedArrayHeadSegment",
                "BailOutOnInvalidatedArrayLength",
                "BailOnStackArgsOutOfActualsRange",
                "BailOutForArrayBits",
                "BailOutForceByFlag",
                "BailOutBreakPointInFunction",
                "BailOutStackFrameBase",
                "BailOutLocalValueChanged",
                "BailOutExplicit",
                "BailOutStep",
                "BailOutIgnoreException",
                "BailOutForDebuggerBits",
                "BailOutOnDivOfMinInt",
                "BailOutOnDivSrcConditions",
                "BailOutMarkTempObject",
                "BailOutKindBits"};

            foreach (string s in inputs)
            {
                Assert.AreEqual(s, DeCRC.GetStringForCRC(DeCRC.CRC32(s)));
            }
        }


        [TestMethod]
        public void TestRejitCRCs()
        {

            string[] inputs = {
                "None",
                "Forced",
                "RuntimeStatsEnabled",
                "AggressiveIntTypeSpecDisabled",
                "AggressiveMulIntTypeSpecDisabled",
                "DivIntTypeSpecDisabled",
                "TrackIntOverflowDisabled",
                "FloatTypeSpecDisabled",
                "ImplicitCallFlagsChanged",
                "FailedPolymorphicInlineeTypeCheck",
                "InlineeChanged",
                "CheckThisDisabled",
                "LossyIntTypeSpecDisabled",
                "MemOpDisabled",
                "FailedTypeCheck",
                "FailedFixedFieldTypeCheck",
                "FailedFixedFieldCheck",
                "FailedEquivalentTypeCheck",
                "FailedEquivalentFixedFieldTypeCheck",
                "CtorGuardInvalidated",
                "ArrayCheckHoistDisabled",
                "ArrayMissingValueCheckHoistDisabled",
                "ArrayAccessHelperCallEliminationDisabled",
                "ExpectingNativeArray",
                "ConvertedNativeArray",
                "ArrayAccessNeededHelperCall",
                "JsArraySegmentHoistDisabled",
                "TypedArrayTypeSpecDisabled",
                "ExpectingConventionalNativeArrayAccess",
                "LdLenIntSpecDisabled",
                "FailedTagCheck",
                "BoundCheckHoistDisabled",
                "LoopCountBasedBoundCheckHoistDisabled",
                "AfterLoopBodyRejit",
                "DisableSwitchOptExpectingInteger",
                "DisableSwitchOptExpectingString",
                "InlineApplyDisabled",
                "InlineSpreadDisabled",
                "FloorInliningDisabled",
                "ModByPowerOf2",
                "NoProfile",
                "PowIntIntTypeSpecDisabled",
                "DisableStackArgOpt",
                "OptimizeTryFinallyDisabled",
};
            foreach (string s in inputs)
            {
                Assert.AreEqual(s, DeCRC.GetStringForCRC(DeCRC.CRC32(s)));
            }
        }



        [TestMethod]
        public void TestLangFeatureCRCs()
        {

            string[] inputs = {
                "ES6_Let",
                "ES6_Lambda",
                "ES6_StrictModeFunction",
                "ES6_Super",
                "ES6_Class",
                "ES6_AsmJSFunction",
                "ES6_StringTemplates",
                "ES6_Const",
                "ES6_Generator",
                "ES6_Rest",
                "ES6_SpreadFeature",
                "ES6_UnicodeRegexFlag",
                "ES6_StickyRegexFlag",
                "ES6_DefaultArgFunction",
                "ES6_RegexSymbolMatch",
                "ES6_RegexSymbolSearch",
                "ES6_RegexSymbolReplace",
                "ES6_RegexSymbolSplit",
                "ES6_Proxy",
                "ES6_Symbol",
                "ES6_Map",
                "ES6_WeakMap",
                "ES6_WeakSet",
                "ES6_Set",
                "ES6_Promise"};

            foreach (string s in inputs)
            {
                Assert.AreEqual(s, DeCRC.GetStringForCRC(DeCRC.CRC32(s)));
            }
        }
    }
}
