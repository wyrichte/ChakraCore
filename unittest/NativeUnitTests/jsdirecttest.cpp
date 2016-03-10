//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "edgejsStatic.h"
#include "ScriptDirectUnitTests.h"

HRESULT VerifyTypeId(Var obj, JavascriptTypeId typeId)
{
    printf("verify typeid == %d : ", typeId);
    if (JsStaticAPI::DataConversion::GetTypeId(obj) == typeId)
    {
        printf("SUCCEEDED\n");
        return NOERROR;
    }
    else
    {
        printf("FAILED\n");
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
            printf("verify DataConversion::VarToInt for double failed, expecting: %d, actual: %d.\n", (int)tests[i][1], actualOutput);
            return E_FAIL;
        }
        else
        {
            printf("verify DataConversion::VarToInt for double succeeded.\n");
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
            printf("library object test failed\n");
        }
        else
        {
            printf("library object test succeeded\n");
        }
    }
}

HRESULT VerifyBool(Var inVar, BOOL expected)
{
    BOOL result;
    HRESULT hr = JsStaticAPI::DataConversion::VarToBOOL(inVar, &result);
    if (FAILED(hr) || (expected ^ result))
    {
        printf("verify bool failed, expecting: %d\n", expected);
        return E_FAIL;
    }
    else
    {
        printf("verify bool succeeded\n");
        return NOERROR;
    }
}

HRESULT VerifyIASD(Var inVar, IActiveScriptDirect* activeScriptDirect)
{
    IActiveScriptDirect* iasdFromVar = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(inVar);
    if (iasdFromVar != activeScriptDirect)
    {
        printf("FAILED: IASD from Var is not consistent with original IASD");
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
        printf("verify IASD\n");
        hr = VerifyIASD(myUndefined, myTest->GetScriptDirectNoRef());
    }
}

void RunJsDirectTest(MyScriptDirectTests* myTests)
{
    printf("run js direct API test\n");
    RunLibraryObjectTests(myTests);
    RunConversionTests(myTests);
}

#ifndef NOSCRIPTSCOPE_CHECK_HRESULT
#define NOSCRIPTSCOPE_CHECK_HRESULT(expr, expected) \
    hr = expr; \
    if (expected == hr) \
    { \
        printf("SUCCESS: " #expr " // HRESULT == " #expected "\n"); \
    } \
    else \
    { \
        printf("FAILURE: " #expr " // HRESULT != " #expected "\n"); \
    }
#ifndef NOSCRIPTSCOPE_CHECK_BOOL
#define NOSCRIPTSCOPE_CHECK_BOOL(which, expected, message) \
    printf(which == expected \
        ? "SUCCESS: " #which " " message "\n" \
        : "FAILURE: " #which " not " message "\n");

void RunNoFailFastScopeTest(MyScriptDirectTests* myTests)
{
    HRESULT hr = S_OK;

    // noScriptScope == false, so this call should not fail
    myTests->ParseAndExecute(_u("var x;"), S_OK); // this should cause EnterScriptObject

    // ensure set and reset works
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), true), S_OK);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), false), S_OK);
    myTests->ParseAndExecute(_u("var x;"), S_OK); // this should cause EnterScriptObject

    printf("RunNoFailFastScopeTest: This SHOULD be printed out as the EnterScript earlier should NOT fail.\n");
}

void RunJsDirectNoScriptScopeTests(MyScriptDirectTests* myTests)
{
    myTests->InitThreadService();

    bool noScriptScope = false;
    HRESULT hr = S_OK;

    //
    // using myTests->GetThreadService()
    //

    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(myTests->GetThreadService(), &noScriptScope), S_OK);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, false, "initialized to false");

    // set and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), true), S_OK);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(myTests->GetThreadService(), &noScriptScope), S_OK);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true");

    // reset and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), false), S_OK);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(myTests->GetThreadService(), &noScriptScope), S_OK);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, false, "reset to false");

    RunNoFailFastScopeTest(myTests);
}

void RunJsDirectNoScriptScopeErrorCaseTests(MyScriptDirectTests* myTests)
{
    myTests->InitThreadService();

    bool noScriptScope = false;
    HRESULT hr = S_OK;

    // some arbitrary pointer which is the wrong kind of IUnknown
    IUnknown *pWrongKind = static_cast<IUnknown *>(myTests->GetScriptDirectNoRef());

    //
    // using nullptr
    //

    // set and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(nullptr, true), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(nullptr, &noScriptScope), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true when passed an invalid argument");

    // reset and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(nullptr, false), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(nullptr, &noScriptScope), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true when passed an invalid argument");

    //
    // using pWrongKind
    //

    // set and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(pWrongKind, true), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(pWrongKind, &noScriptScope), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true when passed an invalid argument");

    // reset and query noScriptScope
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(pWrongKind, false), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::IsNoScriptScope(pWrongKind, &noScriptScope), E_INVALIDARG);
    NOSCRIPTSCOPE_CHECK_BOOL(noScriptScope, true, "set to true when passed an invalid argument");
}

void RunJsDirectNoScriptScopeFailfastTest(MyScriptDirectTests* myTests)
{
    myTests->InitThreadService();

    JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), true);
    myTests->ParseAndExecute(_u("var x;"), S_OK); // this should cause EnterScriptObject
    printf("This should not be printed out as the EnterScript earlier should have failed fast.\n");
}

void RunJsDirectDisableNoScriptScopeTest(MyScriptDirectTests* myTests)
{
    // TODO (doilij): DisableNoScriptScope is a temporary workaround to unblock integration of NoScriptScope into TreeWriter

    myTests->InitThreadService();

    HRESULT hr = S_OK;
    IActiveScriptDirect *x = myTests->GetScriptDirectNoRef();

    // set up variables
    VARIANT variant;
    Var var = nullptr;
    variant.vt = VT_R8;
    variant.dblVal = 3.14;

    // NoScriptScope == false
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), false), S_OK);
    x->ChangeTypeToVar(&variant, &var); // should pass even before fix
    printf("When NoScriptScope==false, this should pass.\n");

    // NoScriptScope == true
    NOSCRIPTSCOPE_CHECK_HRESULT(JsStaticAPI::JavascriptLibrary::SetNoScriptScope(myTests->GetThreadService(), true), S_OK);
    x->ChangeTypeToVar(&variant, &var); // would fail before fix, expect to pass now
    printf("When NoScriptScope==true, and DisableNoScriptScope workaround is in place, this should pass.\n");
}

#undef NOSCRIPTSCOPE_CHECK_BOOL
#endif
#undef NOSCRIPTSCOPE_CHECK_HRESULT
#endif

void RunStaticLibVerificationTest()
{
    JsStaticAPI::BinaryVerificationData binaryVerificationData;
    JsStaticAPI::DataConversion::FillInBinaryVerificationData(&binaryVerificationData);
    printf("majorVersion: %d\n", binaryVerificationData.majorVersion);
    printf("minorVersion: %d\n", binaryVerificationData.minorVersion);
    printf("scriptEngineBaseSize: %d\n", binaryVerificationData.scriptEngineBaseSize);
    printf("scriptEngineBaseOffset: %d\n", binaryVerificationData.scriptEngineBaseOffset);
    printf("scriptContextBaseSize: %d\n", binaryVerificationData.scriptContextBaseSize);
    printf("scriptContextBaseOffset: %d\n", binaryVerificationData.scriptContextBaseOffset);
    printf("javascriptLibraryBaseSize: %d\n", binaryVerificationData.javascriptLibraryBaseSize);
    printf("javascriptLibraryBaseOffset: %d\n", binaryVerificationData.javascriptLibraryBaseOffset);
    printf("customExternalObjectSize: %d\n", binaryVerificationData.customExternalObjectSize);
    printf("typeOffset: %d\n", binaryVerificationData.typeOffset);
    printf("typeIdOffset: %d\n", binaryVerificationData.typeIdOffset);
    printf("taggedIntSize: %d\n", binaryVerificationData.taggedIntSize);
    printf("javascriptNumberSize: %d\n", binaryVerificationData.javascriptNumberSize);
    printf("TypeIdLimit: %d\n", binaryVerificationData.typeIdLimit);
    printf("numberUtilitiesBaseSize: %d\n", binaryVerificationData.numberUtilitiesBaseSize);
    printf("numberUtilitiesBaseOffset: %d\n", binaryVerificationData.numberUtilitiesBaseOffset);
}
