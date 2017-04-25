// Copyright (C) Microsoft. All rights reserved.
//

#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"
#include "EdgeJsStatic.h"

HRESULT TestExpr(BOOL toTest)
{
    if (!toTest)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT TestBasicActiveScriptDirectCreatePromise(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test basic IActiveScriptDirect::CreatePromise functionality...");

    HRESULT hr;
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();
    std::stringstream buf;

    Var promise = nullptr;
    Var resolve = nullptr;
    Var reject = nullptr;

    IfFailGoto(activeScriptDirect->CreatePromise(&promise, &resolve, &reject), error);

    IfFailGoto(TestExpr(promise != nullptr), error);
    IfFailGoto(TestExpr(resolve != nullptr), error);
    IfFailGoto(TestExpr(reject != nullptr), error);

    BOOL result;

    IfFailGoto(activeScriptDirect->IsObjectCallable(resolve, &result), error);
    IfFailGoto(TestExpr(result), error);
    IfFailGoto(activeScriptDirect->IsObjectCallable(reject, &result), error);
    IfFailGoto(TestExpr(result), error);

error:
    FinishTest(myTests, "Done.", hr);

    return hr;
}

Var obj = nullptr;

Var ResolveHandler(Var function, CallInfo callInfo, Var* args)
{
    if (callInfo.Count != 2)
    {
        Print("FAILED: Wrong number of arguments passed to the resolve handler", false);
    }
    else if (obj == nullptr || obj != args[1])
    {
        Print("FAILED: Resolve handler received a wrong object", false);
    }
    else
    {
        Print("PASSED", true);
    }

    return args[1];
}

HRESULT TestBasicActiveScriptDirectPromiseResolveAndThen(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test basic IActiveScriptDirect::PromiseResolveAndThen functionality...");

    HRESULT hr;
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();

    Var promiseResolveFunction = JsStaticAPI::JavascriptLibrary::GetPromiseResolve(activeScriptDirect);
    Var promiseThenFunction = JsStaticAPI::JavascriptLibrary::GetPromiseThen(activeScriptDirect);

    // Verify both methods are valid javascript functions.
    BOOL funcVerificationResult;
    activeScriptDirect->IsVarFunctionObject(promiseResolveFunction, &funcVerificationResult);
    if (!funcVerificationResult)
    {
        Print("FAILED: Invalid Promise.Resolve function", false);
    }
    activeScriptDirect->IsVarFunctionObject(promiseThenFunction, &funcVerificationResult);
    if (!funcVerificationResult)
    {
        Print("FAILED: Invalid Promise.prototype.then function", false);
    }

    activeScriptDirect->CreateArrayObject(3, &obj);

    // Call resolve function which returns a proxy.
    Var argsForResolve[2] = { JsStaticAPI::JavascriptLibrary::GetPromiseConstructor(activeScriptDirect), obj }, result;
    CallInfo callInfoForResolve = { _countof(argsForResolve), CallFlags_None };
    hr = activeScriptDirect->Execute(promiseResolveFunction, callInfoForResolve, argsForResolve, /*servicerProvider*/NULL, &result);
    if (FAILED(hr))
    {
        Print("FAILED: Error while executing resolve", false);
    }
    Var resolvedPromise = result;

    // Function to initialize the result variable.
    Var thenResultInitializer;
    activeScriptDirect->Parse(L"function thenResultInitializer(v) { expectedResult = v; }", &thenResultInitializer);
    // Call the method to initialize the result value to undefined.
    Var argsForThenResultInitializer[1] = { obj };
    CallInfo callInfoForThenResultInitializer = { _countof(argsForThenResultInitializer), CallFlags_None };
    activeScriptDirect->Execute(thenResultInitializer, callInfoForThenResultInitializer, argsForThenResultInitializer, /*serviceProvider*/NULL, &result);

    // Resolve handler for then which saves the result in a global var.
    Var thenResolveHandler, thenRejectHandler;
    PropertyId propId;
    activeScriptDirect->GetOrAddPropertyId(_u("ResolveHandler"), &propId);
    activeScriptDirect->BuildDOMDirectFunction(NULL, ResolveHandler, propId, -1, 0, &thenResolveHandler);

    // Reject handler for then.
    activeScriptDirect->Parse(L"function rejectHandler(error) { print('FAILED'); }", &thenRejectHandler);

    // Call then with the handlers.
    Var argsForThen[3] = { resolvedPromise, thenResolveHandler, thenRejectHandler };
    CallInfo callInfoForThen = { _countof(argsForThen), CallFlags_None };
    hr = activeScriptDirect->Execute(promiseThenFunction, callInfoForThen, argsForThen, /*serviceProvider*/NULL, &result);
    if (FAILED(hr))
    {
        Print("FAILED: Error while executing then", false);
    }

    return NOERROR;
}

void RunCreatePromiseTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestBasicActiveScriptDirectCreatePromise(myTests);

        if (FAILED(hr))
        {
            Print("Test failed", false);
        }
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
 }

void RunPromiseResolveAndThenTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestBasicActiveScriptDirectPromiseResolveAndThen(myTests);
        if (FAILED(hr))
        {
            Print("FAILED: PromiseResolveAndThen test failed", false);
        }
        else
        {
            Print("Passed", true);
        }
    }
    catch (std::string message)
    {
        Print(message, false);
    }
    catch (exception ex)
    {
        Print(ex.what(), false);
    }
}
