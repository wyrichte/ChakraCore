#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"
#include "EdgeJsStatic.h"

HRESULT TestBasicActiveScriptDirectObjectFreeze(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test basic Object.freeze functionality...");

    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();

    // Create an object
    Var obj = nullptr;
    Var createObjFunc = nullptr;
    activeScriptDirect->Parse(L"(function () { obj = {a : 1, b : 2 }; return obj; })()", &createObjFunc);
    CallInfo callInfoForCreateObjFunc = { 0, CallFlags_None };
    activeScriptDirect->Execute(createObjFunc, callInfoForCreateObjFunc, nullptr, /*serviceProvider*/NULL, &obj);

    // Get the Object.freeze function
    Var objFreezeFunc = nullptr;
    objFreezeFunc = JsStaticAPI::JavascriptLibrary::GetObjectFreeze(activeScriptDirect);
    BOOL funcVerificationResult;
    activeScriptDirect->IsVarFunctionObject(objFreezeFunc, &funcVerificationResult);
    if (!funcVerificationResult)
    {
        Print("FAILED: Invalid Object.freeze function", false);
    }

    // Call Object.freeze
    Var outputObj = nullptr;
    Var argsForObjFreeze[2] = { JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect), obj };
    CallInfo callInfoForObjFreeze = { _countof(argsForObjFreeze), CallFlags_None };
    HRESULT hr = activeScriptDirect->Execute(objFreezeFunc, callInfoForObjFreeze, argsForObjFreeze, /*serviceProvider*/NULL, &outputObj);
    if (FAILED(hr) || outputObj != obj)
    {
        Print("FAILED: Error while executing Object.freeze", false);
    }

    // Try deleting a property from the frozen object in strict mode
    Var deletePropFunc = nullptr;
    Var result;
    activeScriptDirect->Parse(L"(function () { \"use strict\"; try { delete obj.a; } catch (e) { return e instanceof TypeError; } return false;})()", &deletePropFunc);
    CallInfo callInfoForDeletePropFunc = { 0, CallFlags_None };
    activeScriptDirect->Execute(deletePropFunc, callInfoForDeletePropFunc, nullptr, /*ServiceProvider*/NULL, &result);

    BOOL convertedRestult = FALSE;
    JsStaticAPI::DataConversion::VarToBOOL(result, &convertedRestult);
    if (convertedRestult == TRUE)
    {
        Print("PASSED", true);
        return NOERROR;
    }
    else
    {
        Print("FAILED: TypeError was not thrown while trying to delete a property from a frozen object in strict mode", false);
        return E_FAIL;
    }
}

void RunObjectTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestBasicActiveScriptDirectObjectFreeze(myTests);

        if (FAILED(hr))
        {
            Print("FAILED: Object.freeze test failed", false);
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