// Copyright (C) Microsoft. All rights reserved. 
// ScriptDirectUnitTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"

std::wstring StringToWString(const std::string& s)
{
    std::wstring temp(s.begin(),s.end());
    return temp; 
}

std::string WStringToString(const std::wstring& s)
{
    int slength = (int)s.length() + 1;
    int len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, s.c_str(), slength, 0, 0, 0, 0);
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, s.c_str(), slength, buf, len, 0, 0);
    std::string r(buf);
    delete[] buf;
    return r;
}

std::string ConvertNewlinesToPrintable(const std::string& s)
{
    string t = s;
    string::size_type pos = 0;
    do
    {
        pos = t.find('\n', pos);   
        if (pos == string::npos) 
        {
            break;
        }
        t.replace(pos, 1, "\n            ");
        pos += 2;
    }
    while(TRUE);
    return t;
}

std::string Summary(int result)
{
    static int failed_count=0;
    static int total_count=0;
    if(result==1)
    {
        total_count++;
        return "";
    }
    else if(result==0)
    {
        total_count++;
        failed_count++;
        return "";
    }
    else
    {
        std::stringstream str;
        str<<"Total Test Cases  "<<total_count<<"\n"<<"Passed:  "<<(total_count-failed_count)<<"\n"<<"Failed   "<<failed_count<<"\n"<<endl;
        cout<<str.str()<<endl;
        if(failed_count>0)
        {
            return "fail";
        }
        else
        {
            return "pass";
        }
    }
}

void Print(std::string message, bool result)
{

    if(result)
    {
        cout<<message<<endl;
        Summary(1);
    }
    else
    {

        cout<<message<<endl; 
        Summary(0);
    }
}

void Print(std::string message)
{
    cout<<message<<endl; 

}

void StartTest(MyScriptDirectTests* myTests, std::string message)
{
    Print(message);
    myTests->Start();
}

void FinishTest(MyScriptDirectTests* myTests, std::string message, HRESULT result)
{
    Print(message, SUCCEEDED(result));
    myTests->End();
}

HRESULT RunNativeTest(JsHostNativeTestArguments* jsHostArgs)
{
    MyScriptDirectTests test(jsHostArgs);
    Verifier<MyScriptDirectTests> verify(&test);

    if (jsHostArgs->flagCount == 0)
    {
        Print("NativeUnitTests::RunNativeTests called with no test suite selection\n");
        return E_FAIL;
    }
    if (_wcsicmp(jsHostArgs->flags[0], _u("FastDOM")) == 0)
    {
        Print("\n\nRunning FastDom Tests\n");
        RunFastDomTests(&test, &verify);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("GetExtendedExceptionInfo")) == 0)
    {
        Print("\n\nRunning GetExtendedExceptionInfo Tests\n");
        RunGetExtendedExceptionInfoTests(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("StackTraceInline")) == 0)
    {
        Print("\n\nRunning StackTraceInline Tests\n");
        RunStackTraceInlineTests(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("Regex")) == 0)
    {
        Print("\n\nRunning Regex Tests\n");
        RunRegexTests(&test, &verify);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("TypedArray")) == 0)
    {
        Print("\n\nRunning TypedArray Tests\n");
        RunTypedArrayTests(&test, &verify);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("TypedArrayES6")) == 0)
    {
        Print("\n\nRunning TypedArray ES6 Tests\n");
        RunTypedArrayES6Tests(&test, &verify);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("Int64")) == 0)
    {
        Print("\n\nRunning Int64 Tests\n");
        RunInt64Tests(&test, &verify);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("BasicTest")) == 0)
    {
        Print("\n\nRunning Basic Tests\n");
        RunBasicTests(&test, &verify);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("HeapEnum")) == 0)
    {
        Print("\n\nRunning HeapEnum Tests\n");
        RunHeapEnumTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("ProfileCacheTests")) == 0)
    {
        Print("\n\nRunning ProfileCacheTests Tests\n");
        RunProfileCacheTests(jsHostArgs);
    } 
    else if (_wcsicmp(jsHostArgs->flags[0], _u("VersionTest")) == 0)
    {
        Print("\n\nRunning version tests\n");
        RunVersionTest();
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("ScriptDiagSerialization")) == 0)
    {
        Print("\n\nRunning JScript9Diag serialization tests\n");
        RunScriptDiagSerializationTest();
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("DirectAPITest")) == 0)
    {
        Print("\n\nRunning version tests\n");
        RunJsDirectTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("DirectAPINoScriptScopeTest")) == 0)
    {
        Print("\n\nRunning DirectAPINoScriptScopeTest tests\n");
        RunJsDirectNoScriptScopeTests(&test);
        return S_OK;
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("DirectAPINoScriptScopeErrorCaseTest")) == 0)
    {
        Print("\n\nRunning DirectAPINoScriptScopeErrorCaseTest tests\n");
        RunJsDirectNoScriptScopeErrorCaseTests(&test);
        return S_OK;
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("DirectAPINoScriptScopeFailfastTest")) == 0)
    {
        Print("\n\nRunning DirectAPINoScriptScopeFailfastTest tests\n");
        RunJsDirectNoScriptScopeFailfastTest(&test);
        return S_OK;
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("DirectAPIDisableNoScriptScopeTest")) == 0)
    {
        Print("\n\nRunning DirectAPIDisableNoScriptScopeTest tests\n");
        RunJsDirectDisableNoScriptScopeTest(&test);
        return S_OK;
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("AsyncDebugTest")) == 0)
    {
        Print("\n\nRunning AsyncDebugging tests\n");
        RunAsyncDebugTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("CreatePromiseTest")) == 0)
    {
        Print("\n\nRunning CreatePromise tests\n");
        RunCreatePromiseTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("JsonTest")) == 0)
    {
        Print("\n\nRunning JsonTest tests\n");
        RunJSONTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("ObjectTest")) == 0)
    {
        Print("\n\nRunning ObjectTest tests\n");
        RunObjectTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("PromiseResolveAndThenTest")) == 0)
    {
        Print("\n\nRunning PromiseResolveAndThen tests\n");
        RunPromiseResolveAndThenTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("ParseJsonTest")) == 0)
    {
        Print("\n\nRunning ParseJson tests\n");
        RunParseJsonTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("DiagnosticsEngineTest")) == 0)
    {
        Print("\n\nRunning DiagnosticsEngine tests\n");
        RunDiagnosticsEngineTest(&test);
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("StaticLibVerificationTest")) == 0)
    {
        Print("\n\nRunning static library verification tests\n");
        RunStaticLibVerificationTest();
        return S_OK;
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("VarToNativeArrayTest")) == 0)
    {
        Print("\n\nRunning VarTonativeArray tests\n");
        RunVarToNativeArrayTest(&test);
        return S_OK;
    }
    else if (_wcsicmp(jsHostArgs->flags[0], _u("RunExternalFuncFpcwTest")) == 0)
    {
        Print("\n\nRunning RunExternalFuncFpcwTest tests\n");
        RunExternalFuncFpcwTest(&test);
        return S_OK;
    }
    else
    {
        Print("NativeUnitTests::RunNativeTests called with invalid test suite selection ");
        Print(WStringToString(jsHostArgs->flags[0]));
        Print("\n");
        return E_FAIL;
    }

    std::string result=Summary(2);
    if(result.compare("pass")==0)
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}
