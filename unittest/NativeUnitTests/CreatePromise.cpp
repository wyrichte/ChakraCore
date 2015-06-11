// Copyright (C) Microsoft. All rights reserved.
//

#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"

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
