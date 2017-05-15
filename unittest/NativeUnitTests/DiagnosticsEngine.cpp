#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"
#include "EdgeJsStatic.h"

HRESULT TestDebugEval(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test basic IActiveScriptDirect::GetDebugEval functionality...");

    HRESULT hr;
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();

    Var debugEvalFunc = JsStaticAPI::JavascriptLibrary::GetDebugEval(activeScriptDirect);

    Var result = NULL;
    Var isLibraryCodeVar = NULL;
    Var evalCodeVar = NULL;
    Var undefinedVar = NULL;
    LPWSTR debugEvalCode = _u("x = 10");
    activeScriptDirect->BOOLToVar(false, &isLibraryCodeVar);
    activeScriptDirect->StringToVar(debugEvalCode, static_cast<int>(wcslen(debugEvalCode)), &evalCodeVar);
    activeScriptDirect->GetUndefined(&undefinedVar);
    Var argsForDebugEval1[3] = { undefinedVar, evalCodeVar, isLibraryCodeVar };
    CallInfo callInfoForDebugEval1 = { _countof(argsForDebugEval1), CallFlags_None };
    hr = activeScriptDirect->Execute(debugEvalFunc, callInfoForDebugEval1, argsForDebugEval1, /*serviceProvider*/NULL, &result);
    if (FAILED(hr))
    {
        Print("FAILED: Failed to execute DebugEval!");
        return E_FAIL;
    }

    evalCodeVar = NULL;
    debugEvalCode = _u("x");
    activeScriptDirect->StringToVar(debugEvalCode, static_cast<int>(wcslen(debugEvalCode)), &evalCodeVar);
    Var argsForDebugEval2[3] = { undefinedVar, evalCodeVar, isLibraryCodeVar };
    CallInfo callInfoForDebugEval2 = { _countof(argsForDebugEval2), CallFlags_None };
    hr = activeScriptDirect->Execute(debugEvalFunc, callInfoForDebugEval2, argsForDebugEval2, /*serviceProvider*/NULL, &result);
    if (FAILED(hr))
    {
        Print("FAILED: Failed to execute DebugEval while retrieving the result!");
        return E_FAIL;
    }

    int resultInt = -1;
    activeScriptDirect->VarToInt(result, &resultInt);
    if (resultInt != 10)
    {
        Print("FAILED: Method returned an unexpeted value");
        return E_FAIL;
    }

    return NOERROR;
}

void RunDiagnosticsEngineTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestDebugEval(myTests);
        if (FAILED(hr))
        {
            Print("DebugEval Test failed", false);
        }
        else
        {
            Print("PASSED", true);
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
