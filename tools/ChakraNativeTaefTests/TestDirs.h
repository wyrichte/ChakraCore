// Copyright (c) Microsoft Corporation. All rights reserved.

DEFINE_TEST_DIR(InlineCaches, L"");
DEFINE_TEST_DIR(jd, L"exclude_serialized");
DEFINE_TEST_DIR(NativeUnitTests, L"");

// Long-running tests which rarely fail can be excluded from
// SNAP runs via the exclude_snap and / or exclude_drt tags.
// Be careful when using these tags.
DEFINE_TEST_DIR(SunSpider, L"exclude_snap");
DEFINE_TEST_DIR_(SunSpider1_0_2, SunSpider1.0.2, L"exclude_snap");
DEFINE_TEST_DIR(SunSpiderFunctionality, L"exclude_snap");
DEFINE_TEST_DIR(V8, L"exclude_snap");
DEFINE_TEST_DIR(V8_Functionality, L"exclude_snap");

DEFINE_TEST_DIR(LetConst, L"");
DEFINE_TEST_DIR(Basics, L"");
DEFINE_TEST_DIR(Generated, L"");
DEFINE_TEST_DIR(Closures, L"");
DEFINE_TEST_DIR(Strings, L"");
DEFINE_TEST_DIR(Date, L"");
DEFINE_TEST_DIR(EH, L"");
DEFINE_TEST_DIR(Error, L"");
DEFINE_TEST_DIR(Boolean, L"");
DEFINE_TEST_DIR(Number, L"");
DEFINE_TEST_DIR(ControlFlow, L"");
DEFINE_TEST_DIR(Math, L"");
DEFINE_TEST_DIR(Array, L"");
DEFINE_TEST_DIR(TaggedIntegers, L"");
DEFINE_TEST_DIR(TaggedFloats, L"");
DEFINE_TEST_DIR(Optimizer, L"");
DEFINE_TEST_DIR(Function, L"");
DEFINE_TEST_DIR(Object, L"");
DEFINE_TEST_DIR(Regex, L"");
DEFINE_TEST_DIR(Prototypes, L"");
DEFINE_TEST_DIR(GlobalFunctions, L"");
DEFINE_TEST_DIR(Operators, L"");
DEFINE_TEST_DIR(Conversions, L"");
DEFINE_TEST_DIR(RWC, L"");
DEFINE_TEST_DIR(VT_DATE, L"");
DEFINE_TEST_DIR(Lib, L"");
DEFINE_TEST_DIR(DirectAuthor, L"exclude_apollo");
DEFINE_TEST_DIR(JSON, L"");
DEFINE_TEST_DIR(Intl, L"");
DEFINE_TEST_DIR(Debugger, L"exclude_serialized");
DEFINE_TEST_DIR(DebuggerQA, L"exclude_serialized");
DEFINE_TEST_DIR(Bugs, L"");
DEFINE_TEST_DIR(es5, L"");
DEFINE_TEST_DIR(strict, L"");
DEFINE_TEST_DIR(Utf8, L"");
DEFINE_TEST_DIR(Authoring, L"");
DEFINE_TEST_DIR(UnifiedRegex, L"");
DEFINE_TEST_DIR(typedarray, L"");
DEFINE_TEST_DIR(sca, L"");
DEFINE_TEST_DIR(crossthread, L"");
DEFINE_TEST_DIR(bailout, L"");
DEFINE_TEST_DIR(Loop, L"");
DEFINE_TEST_DIR(StackTrace, L"");
DEFINE_TEST_DIR(Miscellaneous, L"");
DEFINE_TEST_DIR(fieldopts, L"");
DEFINE_TEST_DIR(FixedFields, L"");
DEFINE_TEST_DIR(HeapEnum, L"");
DEFINE_TEST_DIR(rejit, L"exclude_nonative");
DEFINE_TEST_DIR(inlining, L"");
DEFINE_TEST_DIR(UnitTestFramework, L"");
DEFINE_TEST_DIR(InternalProfile, L"");
DEFINE_TEST_DIR(DynamicCode, L"");
DEFINE_TEST_DIR(es6, L"");
DEFINE_TEST_DIR(switchStatement, L"");
DEFINE_TEST_DIR(AsyncDebug, L"exclude_serialized");
DEFINE_TEST_DIR(Profiler, L"");
DEFINE_TEST_DIR(host, L"");
DEFINE_TEST_DIR(stackfunc, L"exclude_serialized");
DEFINE_TEST_DIR(iasd, L"");
DEFINE_TEST_DIR(KrakenFunctionality, L"");
DEFINE_TEST_DIR(PerfHint, L"exclude_serialized,exclude_snap");
DEFINE_TEST_DIR(LanguageServicesQA, L"exclude_apollo,exclude_serialized,exclude_amd64,exclude_arm,exclude_arm64");
DEFINE_TEST_DIR(AsmJs, L"exclude_apollo,exclude_serialized,exclude_amd64,exclude_arm,exclude_arm64");
DEFINE_TEST_DIR(AsmJsFloat, L"exclude_apollo,exclude_serialized,exclude_amd64,exclude_arm,exclude_arm64");

#undef DEFINE_TEST_DIR
#undef DEFINE_TEST_DIR_
