//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "edgejsStatic.h"
#include "ScriptDirectUnitTests.h"

HRESULT VerifyTypeId(Var obj, JavascriptTypeId typeId)
{
    wprintf(_u("verify typeid == %d : "), typeId);
    if (JsStaticAPI::DataConversion::GetTypeId(obj) == typeId)
    {
        wprintf(_u("SUCCEEDED\n"));
        return NOERROR;
    }
    else
    {
        wprintf(_u("FAILED\n"));
        return E_FAIL;
    }
}

HRESULT VerifyVarToInt(MyScriptDirectTests* myTest)
{
    double tests[][2] =
    {
        // input, expected output
        { 18446744075857035279.545, INT_MIN }, //  > (2^64)
        { -18446744075857035279.545, INT_MIN }, // < -(2^64)
        { 8589934592.345, 0 }, // > (2^32)
        { -8589934592.345, 0 }, // < -(2^32),
        { 524288.3434, 524288}, // < (2^32)
        { -524288.435, -524288 } // > -(2^32)
    };

    Var myDouble;
    int actualOutput;
    int numOfTests = sizeof(tests) / (2 * sizeof(double));
    for (int i = 0; i < numOfTests; i++)
    {
        myTest->GetScriptDirectNoRef()->DoubleToVar(tests[i][0], &myDouble);
        JsStaticAPI::DataConversion::VarToInt(myDouble, &actualOutput);
        if (actualOutput != tests[i][1])
        {
            wprintf(_u("verify DataConversion::VarToInt for double failed, expecting: %d, actual: %d.\n"), (int)tests[i][1], actualOutput);
            return E_FAIL;
        }
        else
        {
            wprintf(_u("verify DataConversion::VarToInt for double succeeded.\n"));
        }
    }
    return NOERROR;
}

void RunLibraryObjectTests(MyScriptDirectTests* myTest)
{
    Var myUndefined, myNull, myTrue, myFalse, myGlobal;
    myUndefined = JsStaticAPI::JavascriptLibrary::GetUndefined(myTest->GetScriptDirectNoRef());
    myNull = JsStaticAPI::JavascriptLibrary::GetNull(myTest->GetScriptDirectNoRef());
    myTrue = JsStaticAPI::JavascriptLibrary::GetTrue(myTest->GetScriptDirectNoRef());
    myFalse = JsStaticAPI::JavascriptLibrary::GetFalse(myTest->GetScriptDirectNoRef());
    myGlobal = JsStaticAPI::JavascriptLibrary::GetGlobalObject(myTest->GetScriptDirectNoRef());

    HRESULT hr = VerifyTypeId(myUndefined, TypeIds_Undefined);
    if (SUCCEEDED(hr))
    {
        hr = VerifyTypeId(myNull, TypeIds_Null);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyTypeId(myTrue, TypeIds_Boolean);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyTypeId(myFalse, TypeIds_Boolean);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyTypeId(myGlobal, TypeIds_GlobalObject);
    }

    if (SUCCEEDED(hr))
    {
        IActiveScriptDirect* scriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(myUndefined);
        if (scriptDirect != myTest->GetScriptDirectNoRef())
        {
            wprintf(_u("library object test failed\n"));
        }
        else
        {
            wprintf(_u("library object test succeeded\n"));
        }
    }
}

HRESULT VerifyBool(Var inVar, BOOL expected)
{
    BOOL result;
    HRESULT hr = JsStaticAPI::DataConversion::VarToBOOL(inVar, &result);
    if (FAILED(hr) || (expected ^ result))
    {
        wprintf(_u("verify bool failed, expecting: %d\n"), expected);
        return E_FAIL;
    }
    else
    {
        wprintf(_u("verify bool succeeded\n"));
        return NOERROR;
    }
}

HRESULT VerifyIASD(Var inVar, IActiveScriptDirect* activeScriptDirect)
{
    IActiveScriptDirect* iasdFromVar = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(inVar);
    if (iasdFromVar != activeScriptDirect)
    {
        wprintf(_u("FAILED: IASD from Var is not consistent with original IASD"));
        return E_FAIL;
    }
    return NOERROR;
}

void RunConversionTests(MyScriptDirectTests* myTest)
{
    Var myUndefined, myNull, myTrue, myFalse, myGlobal;
    myUndefined = JsStaticAPI::JavascriptLibrary::GetUndefined(myTest->GetScriptDirectNoRef());
    myNull = JsStaticAPI::JavascriptLibrary::GetNull(myTest->GetScriptDirectNoRef());
    myTrue = JsStaticAPI::JavascriptLibrary::GetTrue(myTest->GetScriptDirectNoRef());
    myFalse = JsStaticAPI::JavascriptLibrary::GetFalse(myTest->GetScriptDirectNoRef());
    myGlobal = JsStaticAPI::JavascriptLibrary::GetGlobalObject(myTest->GetScriptDirectNoRef());

    HRESULT hr;
    hr = VerifyBool(myUndefined, FALSE);
    if (SUCCEEDED(hr))
    {
        hr = VerifyBool(myNull, FALSE);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyBool(myTrue, TRUE);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyBool(myFalse, FALSE);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyBool(myGlobal, TRUE);
    }
    if (SUCCEEDED(hr))
    {
        hr = VerifyVarToInt(myTest);
    }
    if (SUCCEEDED(hr))
    {
        wprintf(_u("verify IASD\n"));
        hr = VerifyIASD(myUndefined, myTest->GetScriptDirectNoRef());
    }
}

void RunJsDirectTest(MyScriptDirectTests* myTests)
{
    wprintf(_u("run js direct API test\n"));
    RunLibraryObjectTests(myTests);
    RunConversionTests(myTests);
}

#ifndef NOSCRIPTSCOPE_CHECK_HRESULT
#define NOSCRIPTSCOPE_CHECK_HRESULT(expr, expected) \
    hr = expr; \
    if (expected == hr) \
    { \
        wprintf(_u("SUCCESS: ") #expr _u(" // HRESULT == ") #expected _u("\n")); \
    } \
    else \
    { \
        wprintf(_u("FAILURE: ") #expr _u(" // HRESULT != ") #expected _u("\n")); \
    }
#ifndef NOSCRIPTSCOPE_CHECK_BOOL
#define NOSCRIPTSCOPE_CHECK_BOOL(which, expected, message) \
    wprintf(which == expected \
        ? _u("SUCCESS: ") #which _u(" ") message _u("\n") \
        : _u("FAILURE: ") #which _u(" not ") message _u("\n"));

IJavascriptThreadService* GetJavascriptThreadService(MyScriptDirectTests* myTests)
{
    IJavascriptThreadProperty *threadProperty = myTests->GetThreadService();
    IJavascriptThreadService *threadService = nullptr;
    HRESULT hresult = threadProperty->QueryInterface(__uuidof(IJavascriptThreadService), /* out */ reinterpret_cast<void **>(&threadService));

    if (S_OK == hresult)
    {
        // Release extra addref from QI and return
        threadService->Release();
        return threadService;
    }

    // The tests will fail when a nullptr is used so we'll see the problem show up right away.
    return nullptr;
}

void RunNoFailFastScopeTest(MyScriptDirectTests* myTests)
{
    HRESULT hr = S_OK;

    // noScriptScope == false, so this call should not fail
    myTests->ParseAndExecute(_u("var x;"), S_OK); // this should cause EnterScriptObject

    IJavascriptThreadService *threadService = GetJavascriptThreadService(myTests);

    // ensure set and reset works
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, true), S_OK);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, false), S_OK);
    myTests->ParseAndExecute(_u("var x;"), S_OK); // this should cause EnterScriptObject

    wprintf(_u("RunNoFailFastScopeTest: This SHOULD be printed out as the EnterScript earlier should NOT fail.\n"));
}

void RunJsDirectNoScriptScopeTests(MyScriptDirectTests* myTests)
{
    myTests->InitThreadService();

    bool noScriptScope = false;
    HRESULT hr = S_OK; // for NOSCRIPTSCOPE_CHECK_* macros

    IJavascriptThreadService *threadService = GetJavascriptThreadService(myTests);

    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(threadService, &noScriptScope), S_OK);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, false, "initialized to false");

    // set and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, true), S_OK);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(threadService, &noScriptScope), S_OK);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true");

    // reset and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, false), S_OK);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(threadService, &noScriptScope), S_OK);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, false, "reset to false");

    RunNoFailFastScopeTest(myTests);
}

void RunJsDirectNoScriptScopeErrorCaseTests(MyScriptDirectTests* myTests)
{
    myTests->InitThreadService();

    IJavascriptThreadService *threadService = GetJavascriptThreadService(myTests);

    bool noScriptScope = false;
    HRESULT hr = S_OK;

    //
    // using nullptr
    //

    JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, false);

    // set and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(nullptr, true), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(nullptr, &noScriptScope), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true when passed an invalid argument");

    JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, false);

    // reset and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(nullptr, false), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(nullptr, &noScriptScope), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true when passed an invalid argument");
}

void RunJsDirectNoScriptScopeFailfastTest(MyScriptDirectTests* myTests)
{
    myTests->InitThreadService();

    IJavascriptThreadService *threadService = GetJavascriptThreadService(myTests);

    JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, true);
    myTests->ParseAndExecute(_u("var x;"), S_OK); // this should cause EnterScriptObject
    wprintf(_u("This should not be printed out as the EnterScript earlier should have failed fast.\n"));
}

void RunJsDirectDisableNoScriptScopeTest(MyScriptDirectTests* myTests)
{
    // This test covers behavior that previously required the DisableNoScriptScope workaround.
    // The behavior was fixed and the workaround was removed in commit 0d5a69e6 (<yongqu>).

    myTests->InitThreadService();

    HRESULT hr = S_OK;
    IActiveScriptDirect *x = myTests->GetScriptDirectNoRef();

    // set up variables
    VARIANT variant;
    Var var = nullptr;
    variant.vt = VT_R8;
    variant.dblVal = 3.14;

    IJavascriptThreadService *threadService = GetJavascriptThreadService(myTests);

    // NoScriptScope == false
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, false), S_OK);
    x->ChangeTypeToVar(&variant, &var); // should pass even before fix
    wprintf(_u("When NoScriptScope==false, this should pass.\n"));

    // NoScriptScope == true
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(threadService, true), S_OK);
    x->ChangeTypeToVar(&variant, &var); // would fail before fix, expect to pass now
    wprintf(_u("When NoScriptScope==true, this should still pass.\n"));
}

#undef NOSCRIPTSCOPE_CHECK_BOOL
#endif
#undef NOSCRIPTSCOPE_CHECK_HRESULT
#endif

void RunStaticLibVerificationTest()
{
    JsStaticAPI::BinaryVerificationData binaryVerificationData;
    JsStaticAPI::DataConversion::FillInBinaryVerificationData(&binaryVerificationData);
    wprintf(_u("Please be careful when you change the baseline here as it will break edge browser unless edge is built with matching source in SD\n"));
    wprintf(_u("Please add yongqu to codereview when you change this file\n"));
    wprintf(_u("majorVersion: %d\n"), binaryVerificationData.majorVersion);
    wprintf(_u("minorVersion: %d\n"), binaryVerificationData.minorVersion);
    wprintf(_u("scriptEngineBaseSize: %d\n"), binaryVerificationData.scriptEngineBaseSize);
    wprintf(_u("scriptEngineBaseOffset: %d\n"), binaryVerificationData.scriptEngineBaseOffset);
    wprintf(_u("scriptContextBaseSize: %d\n"), binaryVerificationData.scriptContextBaseSize);
    wprintf(_u("scriptContextBaseOffset: %d\n"), binaryVerificationData.scriptContextBaseOffset);
    wprintf(_u("javascriptLibraryBaseSize: %d\n"), binaryVerificationData.javascriptLibraryBaseSize);
    wprintf(_u("javascriptLibraryBaseOffset: %d\n"), binaryVerificationData.javascriptLibraryBaseOffset);
    wprintf(_u("customExternalObjectSize: %d\n"), binaryVerificationData.customExternalObjectSize);
    wprintf(_u("typeOffset: %d\n"), binaryVerificationData.typeOffset);
    wprintf(_u("typeIdOffset: %d\n"), binaryVerificationData.typeIdOffset);
    wprintf(_u("taggedIntSize: %d\n"), binaryVerificationData.taggedIntSize);
    wprintf(_u("javascriptNumberSize: %d\n"), binaryVerificationData.javascriptNumberSize);
    wprintf(_u("TypeIdLimit: %d\n"), binaryVerificationData.typeIdLimit);
    wprintf(_u("numberUtilitiesBaseSize: %d\n"), binaryVerificationData.numberUtilitiesBaseSize);
    wprintf(_u("numberUtilitiesBaseOffset: %d\n"), binaryVerificationData.numberUtilitiesBaseOffset);
}
