//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ApiTest.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{
    template ApiTest<JsRuntimeAttributeNone>;
    template ApiTest<JsRuntimeAttributeDisableBackgroundWork>;
    template ApiTest<JsRuntimeAttributeAllowScriptInterrupt>;
    template ApiTest<JsRuntimeAttributeEnableIdleProcessing>;
    template ApiTest<JsRuntimeAttributeDisableNativeCodeGeneration>;
    template ApiTest<JsRuntimeAttributeDisableEval>;
    template ApiTest<(JsRuntimeAttributes) (JsRuntimeAttributeDisableBackgroundWork | JsRuntimeAttributeAllowScriptInterrupt | JsRuntimeAttributeEnableIdleProcessing)>;

#define TERMINATION_TESTS \
        L"for (i=0; i<200; i = 20) {"              \
        L"    var a = new Int8Array(800);"         \
        L"}",                                      \
                                                   \
        L"function nextFunc() { "                  \
        L"    throw 'hello'"                       \
        L"};"                                      \
        L"for (i=0; i<200; i = 20)  { "            \
        L"    try {"                               \
        L"        nextFunc();"                     \
        L"    } "                                  \
        L"    catch(e) {}"                         \
        L"}",                                      \
                                                   \
        L"function nextFunc() {"                   \
        L"    bar = bar + nextFunc.toString();"    \
        L"};"                                      \
        L"bar = '';"                               \
        L"for (i=0; i<200; i = 20) {"              \
        L"    nextFunc()"                          \
        L"}",                                      \
                                                   \
        L"while(1);",                              \
                                                   \
        L"function foo(){}"                        \
        L"do{"                                     \
        L"    foo();"                              \
        L"}while(1);",                             \
                                                   \
        L"(function foo(){"                        \
        L"    do {"                                \
        L"        if (foo) continue;"              \
        L"        if (!foo) break;"                \
        L"    } while(1); "                        \
        L"})();",                                  \
                                                   \
        L"(function foo(a){"                       \
        L"    while (a){"                          \
        L" L1:"                                    \
        L"        do {"                            \
        L"            while(1) {"                  \
        L"                continue L1;"            \
        L"            }"                           \
        L"            a = 0;"                      \
        L"        } while(0);"                     \
        L"    }"                                   \
        L"})(1);",                                 \
                                                   \
        L"(function (){"                           \
        L"    while (1) {"                         \
        L"        try {"                           \
        L"            throw 0;"                    \
        L"            break;"                      \
        L"        }"                               \
        L"        catch(e) {"                      \
        L"            if (!e) continue;"           \
        L"        }"                               \
        L"        break;"                          \
        L"    }"                                   \
        L"})();"

    // only static const integral value can be initialized in declaration.
    LPCWSTR const ApiTest<JsRuntimeAttributeNone>::terminationTests[] = { TERMINATION_TESTS };
    LPCWSTR const ApiTest<JsRuntimeAttributeDisableBackgroundWork>::terminationTests[] = { TERMINATION_TESTS };
    LPCWSTR const ApiTest<JsRuntimeAttributeAllowScriptInterrupt>::terminationTests[] = { TERMINATION_TESTS };
    LPCWSTR const ApiTest<JsRuntimeAttributeEnableIdleProcessing>::terminationTests[] = { TERMINATION_TESTS };
    LPCWSTR const ApiTest<JsRuntimeAttributeDisableNativeCodeGeneration>::terminationTests[] = { TERMINATION_TESTS };
    LPCWSTR const ApiTest<JsRuntimeAttributeDisableEval>::terminationTests[] = { TERMINATION_TESTS };
    LPCWSTR const ApiTest<(JsRuntimeAttributes) (JsRuntimeAttributeDisableBackgroundWork | JsRuntimeAttributeAllowScriptInterrupt | JsRuntimeAttributeEnableIdleProcessing)>::terminationTests[] = { TERMINATION_TESTS };
}
