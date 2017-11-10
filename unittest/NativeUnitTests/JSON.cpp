#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"
#include "EdgeJsStatic.h"

HRESULT TestBasicActiveScriptDirectJSONStringify(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test basic Json.stringify functionality...");

    HRESULT hr;
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();

    // Create an object form a string through JSON.parse
    Var createObjFunc;
    Var obj;
    activeScriptDirect->Parse(L"(function () { originalStr = \"{\\\"a\\\":1,\\\"b\\\":2}\"; return JSON.parse(originalStr); })()", &createObjFunc);
    Var argsForCreateObjFunc[1] = { JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect) };
    CallInfo callInfoForCreateObjFunc = { _countof(argsForCreateObjFunc), CallFlags_None };
    activeScriptDirect->Execute(createObjFunc, callInfoForCreateObjFunc, argsForCreateObjFunc, /*serviceProvider*/NULL, &obj);

    // Get the JSON.stringify function
    Var jsonStringifyFunc;
    jsonStringifyFunc = JsStaticAPI::JavascriptLibrary::GetJSONStringify(activeScriptDirect);
    BOOL funcVerificationResult;
    activeScriptDirect->IsVarFunctionObject(jsonStringifyFunc, &funcVerificationResult);
    if (!funcVerificationResult)
    {
        Print("FAILED: Invalid JSON.stringify function", false);
    }

    // Call JSON.stringify and get the string result
    Var actualStr;
    Var argsForJsonStringify[2] = { JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect), obj };
    CallInfo callInfoForJsonStringify = { _countof(argsForJsonStringify), CallFlags_None };
    hr = activeScriptDirect->Execute(jsonStringifyFunc, callInfoForJsonStringify, argsForJsonStringify, /*serviceProvider*/NULL, &actualStr);
    if (FAILED(hr))
    {
        Print("FAILED: Error while executing JSON.stringify", false);
    }

    // Compare the result with the actual value
    BSTR bstr;
    hr = activeScriptDirect->VarToString(actualStr, &bstr);
    if (SUCCEEDED(hr))
    {
        if (0 == wcscmp(bstr, L"{\"a\":1,\"b\":2}"))
        {
            Print("PASSED", true);
        }
        SysFreeString(bstr);
        return NOERROR;
    }
    else
    {
        Print("FAILED: VarToString failed");
        return hr;
    }
}

void RunJSONTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestBasicActiveScriptDirectJSONStringify(myTests);

        if (FAILED(hr))
        {
            Print("FAILED: JSON.stringify test failed", false);
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