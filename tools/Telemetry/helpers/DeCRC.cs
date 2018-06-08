using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chakra.Utils
{
    public class DeCRC
    {
        private static Dictionary<UInt64, string> crcToStrings = new Dictionary<UInt64, string>();

        static DeCRC()
        {
            crcToStrings.Add(CRC32("_None"), "_None");
            crcToStrings.Add(CRC32("_Max"), "_Max");
            Populate(ParseBuiltIns());
            Populate(ParseRejitReasons());
            Populate(ParseLangFeatures());
            Populate(ParseBailoutReasons());
            Populate(ParseRecyclerWaitReasons());
            Populate(ParseRecyclerSizeEntries());
        }

        public static string GetStringForCRC(UInt64 crc)
        {
            string val;
            crcToStrings.TryGetValue(crc, out val);
            if (val == null)
            {
                val = "Unknown(" + crc + ")";
            }
            return val;
        }

        public static UInt64 CRC32(string s)
        {
            byte[] bytes = Encoding.ASCII.GetBytes(s);
            return CRC32(bytes);
        }

        private static void Populate(IEnumerable<string> vals)
        {
            foreach (string s in vals)
            {
                crcToStrings.Add(CRC32(s), s);
            }
        }

        private static IEnumerable<string> ParseBailoutReasons()
        {
            List<string> result = new List<string>();
            string[] lines = BailOutKind_dot_h.Split(new[] { '\r', '\n' });
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i].Trim();
                if (line.Length > 0)
                {
                    if (line.StartsWith("BAIL_OUT_KIND("))
                    {
                        // lop off leading "ENTRY_BUILTIN("
                        line = line.Substring("BAIL_OUT_KIND(".Length);

                        // lop off everything after first comma
                        line = line.Substring(0, line.LastIndexOf(','));

                        result.Add(line.Trim());

                    }
                    else if (line.StartsWith("BAIL_OUT_KIND_VALUE("))
                    {
                        // lop off leading "ENTRY_BUILTIN("
                        line = line.Substring("BAIL_OUT_KIND_VALUE(".Length);

                        // lop off everything after first comma
                        line = line.Substring(0, line.LastIndexOf(','));

                        result.Add(line.Trim());
                    }
                    else if (line.StartsWith("BAIL_OUT_KIND_VALUE_LAST("))
                    {
                        // lop off leading "ENTRY_BUILTIN("
                        line = line.Substring("BAIL_OUT_KIND_VALUE_LAST(".Length);

                        // lop off everything after first comma
                        line = line.Substring(0, line.LastIndexOf(','));

                        result.Add(line.Trim());
                    }
                }
            }
            return result;
        }

        private static string BailOutKind_dot_h = @"
//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#if !defined(BAIL_OUT_KIND) || !defined(BAIL_OUT_KIND_VALUE) || !defined(BAIL_OUT_KIND_VALUE_LAST)
    #error BAIL_OUT_KIND, BAIL_OUT_KIND_VALUE, and BAIL_OUT_KIND_VALUE_LAST must be defined before including this file.
#endif
               /* kind */                           /* allowed bits */
BAIL_OUT_KIND(BailOutInvalid,                       IR::BailOutOnResultConditions | IR::BailOutForArrayBits | IR::BailOutForDebuggerBits | IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutIntOnly,                       IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutNumberOnly,                    IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutPrimitiveButString,            IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutOnImplicitCalls,               IR::BailOutForArrayBits)
BAIL_OUT_KIND(BailOutOnImplicitCallsPreOp,          (IR::BailOutOnResultConditions | IR::BailOutForArrayBits | IR::BailOutMarkTempObject) & ~IR::BailOutOnArrayAccessHelperCall )
BAIL_OUT_KIND(BailOutOnNotPrimitive,                IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutOnMemOpError,                  IR::BailOutForArrayBits)
BAIL_OUT_KIND(BailOutOnInlineFunction,              0)
BAIL_OUT_KIND(BailOutOnNoProfile,                   0)
BAIL_OUT_KIND(BailOutOnPolymorphicInlineFunction,   0)
BAIL_OUT_KIND(BailOutOnFailedPolymorphicInlineTypeCheck,   0)
BAIL_OUT_KIND(BailOutShared,                        0)
BAIL_OUT_KIND(BailOutOnNotArray,                    IR::BailOutOnMissingValue)
BAIL_OUT_KIND(BailOutOnNotNativeArray,              IR::BailOutOnMissingValue)
BAIL_OUT_KIND(BailOutConventionalTypedArrayAccessOnly, IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutOnIrregularLength,             IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutCheckThis,                     0)
BAIL_OUT_KIND(BailOutOnTaggedValue,                 0)
BAIL_OUT_KIND(BailOutFailedTypeCheck,               IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutFailedEquivalentTypeCheck,     IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutInjected,                      0)
BAIL_OUT_KIND(BailOutExpectingInteger,              0)
BAIL_OUT_KIND(BailOutExpectingString,               0)
BAIL_OUT_KIND(BailOutFailedInlineTypeCheck,         IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutFailedFixedFieldTypeCheck,     IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutFailedFixedFieldCheck,         0)
BAIL_OUT_KIND(BailOutFailedEquivalentFixedFieldTypeCheck,     IR::BailOutMarkTempObject)
BAIL_OUT_KIND(BailOutOnFloor,                       0)
BAIL_OUT_KIND(BailOnModByPowerOf2,                  0)
BAIL_OUT_KIND(BailOnIntMin,                         0)
BAIL_OUT_KIND(BailOnDivResultNotInt,                IR::BailOutOnDivByZero | IR::BailOutOnDivOfMinInt | IR::BailOutOnNegativeZero)
BAIL_OUT_KIND(BailOnSimpleJitToFullJitLoopBody,     0)
BAIL_OUT_KIND(BailOutFailedCtorGuardCheck,          0)
BAIL_OUT_KIND(BailOutOnFailedHoistedBoundCheck,     0)
BAIL_OUT_KIND(LazyBailOut,                          0)
BAIL_OUT_KIND(BailOutOnFailedHoistedLoopCountBasedBoundCheck, 0)
BAIL_OUT_KIND(BailOutForGeneratorYield,             0)
BAIL_OUT_KIND(BailOutOnException,                   0)
BAIL_OUT_KIND(BailOutOnEarlyExit,                   0)
BAIL_OUT_KIND(BailOutKindEnd,                       0)

// One bailout instruction can have multiple of the following reasons for bailout combined with any of the above. These tell
// what additional checks must be done to determine whether to bail out.
BAIL_OUT_KIND(BailOutKindBitsStart, 0) // fake bail out kind to indicate start index for kinds below

#define BAIL_OUT_KIND_BIT_START 10      // We can have 2^10 or 1024 bailout condition above
// ======================
// Result condition bits
// ======================
#define BAIL_OUT_KIND_RESULT_CONDITIONS_BIT_START BAIL_OUT_KIND_BIT_START
BAIL_OUT_KIND_VALUE(BailOutOnOverflow, 1 << (BAIL_OUT_KIND_RESULT_CONDITIONS_BIT_START + 0))
BAIL_OUT_KIND_VALUE(BailOutOnMulOverflow, 1 << (BAIL_OUT_KIND_RESULT_CONDITIONS_BIT_START + 1))
BAIL_OUT_KIND_VALUE(BailOutOnNegativeZero, 1 << (BAIL_OUT_KIND_RESULT_CONDITIONS_BIT_START + 2))
BAIL_OUT_KIND_VALUE(BailOutOnPowIntIntOverflow, 1 << (BAIL_OUT_KIND_RESULT_CONDITIONS_BIT_START + 3))
BAIL_OUT_KIND_VALUE(BailOutOnResultConditions, BailOutOnOverflow | BailOutOnMulOverflow | BailOutOnNegativeZero | BailOutOnPowIntIntOverflow)

// ================
// Array bits
// ================
#define BAIL_OUT_KIND_ARRAY_BIT_START BAIL_OUT_KIND_RESULT_CONDITIONS_BIT_START + 4
BAIL_OUT_KIND_VALUE(BailOutOnMissingValue, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 0))
BAIL_OUT_KIND_VALUE(BailOutConventionalNativeArrayAccessOnly, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 1))
BAIL_OUT_KIND_VALUE(BailOutConvertedNativeArray, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 2))
BAIL_OUT_KIND_VALUE(BailOutOnArrayAccessHelperCall, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 3))
BAIL_OUT_KIND_VALUE(BailOutOnInvalidatedArrayHeadSegment, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 4))
BAIL_OUT_KIND_VALUE(BailOutOnInvalidatedArrayLength, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 5))
BAIL_OUT_KIND_VALUE(BailOnStackArgsOutOfActualsRange, 1 << (BAIL_OUT_KIND_ARRAY_BIT_START + 6))
BAIL_OUT_KIND_VALUE(    BailOutForArrayBits,    (        BailOutOnMissingValue |        BailOutConventionalNativeArrayAccessOnly |        BailOutConvertedNativeArray |        BailOutOnArrayAccessHelperCall |        BailOutOnInvalidatedArrayHeadSegment |        BailOutOnInvalidatedArrayLength |        BailOnStackArgsOutOfActualsRange    ))

// ================
// Debug bits
// ================
#define BAIL_OUT_KIND_DEBUG_BIT_START BAIL_OUT_KIND_ARRAY_BIT_START + 7
// Forced bailout by ThreadContext::m_forceInterpreter, e.g. for async break when we enter a function.
BAIL_OUT_KIND_VALUE(BailOutForceByFlag, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 0))

// When a function has a breakpoint, we need to bail out when we enter/return back to it.
BAIL_OUT_KIND_VALUE(BailOutBreakPointInFunction, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 1))

// Used for stepping/on return from func. Bails out when current frame addr is greater than DebuggingFlags.m_stepEffectiveFrameBase.
BAIL_OUT_KIND_VALUE(BailOutStackFrameBase, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 2))

// When we return to a frame in which a value of a local was changed.
BAIL_OUT_KIND_VALUE(BailOutLocalValueChanged, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 3))

// Unconditional bailout, e.g. for the 'debugger' statement.
BAIL_OUT_KIND_VALUE(BailOutExplicit, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 4))
BAIL_OUT_KIND_VALUE(BailOutStep, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 5))
BAIL_OUT_KIND_VALUE(BailOutIgnoreException, 1 << (BAIL_OUT_KIND_DEBUG_BIT_START + 6))

BAIL_OUT_KIND_VALUE(BailOutForDebuggerBits, BailOutForceByFlag | BailOutBreakPointInFunction | BailOutStackFrameBase | BailOutLocalValueChanged | BailOutExplicit | BailOutStep | BailOutIgnoreException)

// ======================
// Div Src Condition Bits
// ======================
#define BAIL_OUT_KIND_DIV_SRC_CONDITIONS_BIT_START BAIL_OUT_KIND_DEBUG_BIT_START + 7
BAIL_OUT_KIND_VALUE(BailOutOnDivByZero, 1 << (BAIL_OUT_KIND_DIV_SRC_CONDITIONS_BIT_START + 0))
BAIL_OUT_KIND_VALUE(BailOutOnDivOfMinInt, 1 << (BAIL_OUT_KIND_DIV_SRC_CONDITIONS_BIT_START + 1))
BAIL_OUT_KIND_VALUE(BailOutOnDivSrcConditions, BailOutOnDivByZero | BailOutOnDivOfMinInt)

#define BAIL_OUT_KIND_MISC_BIT_START BAIL_OUT_KIND_DIV_SRC_CONDITIONS_BIT_START + 2
BAIL_OUT_KIND_VALUE(BailOutMarkTempObject, 1 << (BAIL_OUT_KIND_MISC_BIT_START + 0))


BAIL_OUT_KIND_VALUE_LAST(BailOutKindBits, BailOutMarkTempObject | BailOutOnDivSrcConditions | BailOutOnResultConditions | BailOutForArrayBits | BailOutForDebuggerBits)

// Help caller undefine the macros
#undef BAIL_OUT_KIND
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
";



        private static string RejitReasons_dot_h = @"
REJIT_REASON(None)
REJIT_REASON(Forced)
REJIT_REASON(RuntimeStatsEnabled)
REJIT_REASON(AggressiveIntTypeSpecDisabled)
REJIT_REASON(AggressiveMulIntTypeSpecDisabled)
REJIT_REASON(DivIntTypeSpecDisabled)
REJIT_REASON(TrackIntOverflowDisabled)
REJIT_REASON(FloatTypeSpecDisabled)
REJIT_REASON(ImplicitCallFlagsChanged)
REJIT_REASON(FailedPolymorphicInlineeTypeCheck)
REJIT_REASON(InlineeChanged)
REJIT_REASON(CheckThisDisabled)
REJIT_REASON(LossyIntTypeSpecDisabled)
REJIT_REASON(MemOpDisabled)
REJIT_REASON(FailedTypeCheck)
REJIT_REASON(FailedFixedFieldTypeCheck)
REJIT_REASON(FailedFixedFieldCheck)
REJIT_REASON(FailedEquivalentTypeCheck)
REJIT_REASON(FailedEquivalentFixedFieldTypeCheck)
REJIT_REASON(CtorGuardInvalidated)
REJIT_REASON(ArrayCheckHoistDisabled)
REJIT_REASON(ArrayMissingValueCheckHoistDisabled)
REJIT_REASON(ArrayAccessHelperCallEliminationDisabled)
REJIT_REASON(ExpectingNativeArray)
REJIT_REASON(ConvertedNativeArray)
REJIT_REASON(ArrayAccessNeededHelperCall)
REJIT_REASON(JsArraySegmentHoistDisabled)
REJIT_REASON(TypedArrayTypeSpecDisabled)
REJIT_REASON(ExpectingConventionalNativeArrayAccess)
REJIT_REASON(LdLenIntSpecDisabled)
REJIT_REASON(FailedTagCheck)
REJIT_REASON(BoundCheckHoistDisabled)
REJIT_REASON(LoopCountBasedBoundCheckHoistDisabled)
REJIT_REASON(AfterLoopBodyRejit)
REJIT_REASON(DisableSwitchOptExpectingInteger)
REJIT_REASON(DisableSwitchOptExpectingString)
REJIT_REASON(InlineApplyDisabled)
REJIT_REASON(InlineSpreadDisabled)
REJIT_REASON(FloorInliningDisabled)
REJIT_REASON(ModByPowerOf2)
REJIT_REASON(NoProfile)
REJIT_REASON(PowIntIntTypeSpecDisabled)
REJIT_REASON(DisableStackArgOpt)
REJIT_REASON(OptimizeTryFinallyDisabled)
";

        private static IEnumerable<string> ParseRejitReasons()
        {
            List<string> result = new List<string>();
            string[] lines = RejitReasons_dot_h.Split(new[] { '\r', '\n' });
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i].Trim();
                if (line.StartsWith("REJIT_REASON("))
                {
                    // lop off leading "ENTRY_BUILTIN("
                    line = line.Substring("REJIT_REASON(".Length);

                    // lop off trailing ")"
                    line = line.Substring(0, line.LastIndexOf(')'));

                    result.Add(line.Trim());
                }
            }
            return result;
        }

        // copy/paste from /chakra/private/lib/Telemetry/ScriptContext/ESBuiltIns.h
        private static string ESBuiltIns_dot_h = @"
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
";

        private static IEnumerable<string> ParseBuiltIns()
        {
            List<string> result = new List<string>();
            string[] lines = ESBuiltIns_dot_h.Split(new[] { '\r', '\n' });
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i].Trim();
                if (line.StartsWith("ENTRY_BUILTIN("))
                {
                    // lop off leading "ENTRY_BUILTIN("
                    line = line.Substring("ENTRY_BUILTIN(".Length);

                    // lop off trailing ")"
                    line = line.Substring(0, line.LastIndexOf(')'));

                    string[] parts = line.Split(',');
                    result.Add(parts[1].Trim() + "_" + parts[2].Trim() + "_" + parts[3].Trim());
                }
            }
            return result;
        }

        // copy/paste from /chakra/private/lib/Telemetry/ScriptContext/LanguageFeatures.h
        private static string langFeatures_dot_h = @"
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

ENTRY_LANGFEATURE(ES6, RegexSymbolMatch)
ENTRY_LANGFEATURE(ES6, RegexSymbolSearch)
ENTRY_LANGFEATURE(ES6, RegexSymbolReplace)
ENTRY_LANGFEATURE(ES6, RegexSymbolSplit)

//Ctors
ENTRY_LANGFEATURE(ES6, Proxy)
ENTRY_LANGFEATURE(ES6, Symbol)
ENTRY_LANGFEATURE(ES6, Map)
ENTRY_LANGFEATURE(ES6, WeakMap)
ENTRY_LANGFEATURE(ES6, WeakSet)
ENTRY_LANGFEATURE(ES6, Set)
ENTRY_LANGFEATURE(ES6, Promise)
";

        private static IEnumerable<string> ParseLangFeatures()
        {
            List<string> result = new List<string>();
            string[] lines = langFeatures_dot_h.Split(new[] { '\r', '\n' });
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i].Trim();
                if (line.StartsWith("ENTRY_LANGFEATURE("))
                {
                    // lop off leading "ENTRY_BUILTIN("
                    line = line.Substring("ENTRY_LANGFEATURE(".Length);

                    // lop off trailing ")"
                    line = line.Substring(0, line.LastIndexOf(')'));

                    string[] parts = line.Split(',');
                    result.Add(parts[0].Trim() + "_" + parts[1].Trim());
                }
            }
            return result;
        }

        // copy/paste from /chakra/core/lib/Common/Memory/RecyclerWaitReasonInc.h
        private static string recyclerWaitReason_dot_h= @"
P(WaitReasonNone)
P(RescanMark)
P(DoParallelMark)
P(RequestConcurrentCallbackWrapper)
P(CollectOnConcurrentThread)
P(FinishConcurrentCollect)
P(Other)
";

        private static IEnumerable<string> ParseRecyclerWaitReasons()
        {
            List<string> result = new List<string>();
            string[] lines = recyclerWaitReason_dot_h.Split(new[] { '\r', '\n' });
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i].Trim();
                if (line.StartsWith("P("))
                {
                    // lop off leading "P("
                    line = line.Substring("P(".Length);

                    // lop off trailing ")"
                    line = line.Substring(0, line.LastIndexOf(')'));
                    string val = "GC_UI_THREAD_BLOCKED_" + line;
                    result.Add(val);
                }
            }
            return result;
        }


        // copy/paste from /chakra/private/lib/Telemetry/Recycler/RecyclerSizeEntries.h
        private static string recyclerSizeEntries_dot_h = @"
RECYCLER_SIZE_NO_SUBFIELD(processAllocaterUsedBytes_start)
RECYCLER_SIZE_NO_SUBFIELD(processAllocaterUsedBytes_end)
RECYCLER_SIZE_NO_SUBFIELD(processCommittedBytes_start)
RECYCLER_SIZE_NO_SUBFIELD(processCommittedBytes_end)

RECYCLER_SIZE_SUBFIELD(threadPageAllocator_start, committedBytes)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_start, usedBytes)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_start, reservedBytes)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_start, numberOfSegments)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_end, committedBytes)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_end, usedBytes)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_end, reservedBytes)
RECYCLER_SIZE_SUBFIELD(threadPageAllocator_end, numberOfSegments)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_start, committedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_start, usedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_start, reservedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_start, numberOfSegments)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_end, committedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_end, usedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_end, reservedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLeafPageAllocator_end, numberOfSegments)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_start, committedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_start, usedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_start, reservedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_start, numberOfSegments)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_end, committedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_end, usedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_end, reservedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerLargeBlockPageAllocator_end, numberOfSegments)

#ifdef RECYCLER_WRITE_BARRIER_ALLOC_SEPARATE_PAGE

RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_start, committedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_start, usedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_start, reservedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_start, numberOfSegments)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_end, committedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_end, usedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_end, reservedBytes)
RECYCLER_SIZE_SUBFIELD(recyclerWithBarrierPageAllocator_end, numberOfSegments)
";

        private static IEnumerable<string> ParseRecyclerSizeEntries()
        {
            // .net regex expressions to parse the above:
            //    new Regex(@"^\s*RECYCLER_SIZE_NO_SUBFIELD\(\s*(.*)\s*\)\s*$", RegexOptions.Multiline);
            //    new Regex(@"^\s*RECYCLER_SIZE_SUBFIELD\(\s*(.*)\s*,\s*(.*)\)\s*$", RegexOptions.Multiline);

            List<string> result = new List<string>();
            string[] lines = recyclerSizeEntries_dot_h.Split(new[] { '\r', '\n' });
            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i].Trim();
                if (line.StartsWith("RECYCLER_SIZE_NO_SUBFIELD("))
                {


                    // lop off leading "RECYCLER_SIZE_NO_SUBFIELD("
                    line = line.Substring("RECYCLER_SIZE_NO_SUBFIELD(".Length);

                    // lop off trailing ")"
                    line = line.Substring(0, line.LastIndexOf(')'));
                    result.Add(line);
                }

                if (line.StartsWith("RECYCLER_SIZE_SUBFIELD("))
                {
                    // lop off leading "RECYCLER_SIZE_SUBFIELD("
                    line = line.Substring("RECYCLER_SIZE_SUBFIELD(".Length);

                    // lop off trailing ")"
                    line = line.Substring(0, line.LastIndexOf(')'));

                    string[] parts = line.Split(',');
                    result.Add(parts[0].Trim() + "_" + parts[1].Trim());
                }
            }
            return result;
        }

        // To save on data size, we transmit the CRC32 hashes of the names for fields in the associative
        // arrays. This reduces the volume of data that we send, and simplifies some handling. We need a
        // way to convert back, so we include the headers that define the symbols above, and then do the
        // same operation to re-generate the hash.
        private static readonly UInt64[] crc_table = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

        private static UInt64 CRC32(byte[] input)
        {
            UInt64 crc = (UInt64)(0xFFFFFFFF);
            int i = 0;
            while (i < input.Length && input[i] != '\0')
            {
                crc = (crc >> 8) ^ crc_table[(crc ^ input[i]) & 0xFF];
                i++;
            }
            return crc ^ (UInt64)(0xFFFFFFFF);
        }

    }
}
