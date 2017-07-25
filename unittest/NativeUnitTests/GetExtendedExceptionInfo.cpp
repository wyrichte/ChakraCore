// Copyright (C) Microsoft. All rights reserved.
// GetExtendedErrorInfo.cpp: Defines test for IActiveScriptErrorEx::GetExtendedERrorInfo functionality
//

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"

bool g_OnScriptErrorCallbackSucceeded = false;
bool g_VerboseOutput = true;

void FreeExcepInfo(EXCEPINFO *pei)
{
    if (pei->bstrSource)
        SysFreeString(pei->bstrSource);
    if (pei->bstrDescription)
        SysFreeString(pei->bstrDescription);
    if (pei->bstrHelpFile)
        SysFreeString(pei->bstrHelpFile);
    memset(pei, 0, sizeof(*pei));
}

void FreeExcepInfo(ExtendedExceptionInfo *pei)
{
    FreeExcepInfo(&pei->exceptionInfo);
    CoTaskMemFree((LPVOID)(pei->errorType.typeText));
    for (UINT i=0; i < pei->callStack.frameCount; i++)
    {
        if (pei->callStack.frames[i].activeScriptDirect != NULL)
        {
            pei->callStack.frames[i].activeScriptDirect->Release();
        }
        CoTaskMemFree((LPVOID)(pei->callStack.frames[i].functionName));
    }
    if (pei->callStack.frames)
    {
        CoTaskMemFree((LPVOID)(pei->callStack.frames));
    }
    memset(pei, 0, sizeof(*pei));
}

void TestCreateErrorObjectSucceedsWithValidErrorType(MyScriptDirectTests* mytest)
{
    Var errorObject;
    HRESULT hr = mytest->CreateErrorObject(JavascriptError, 0, _u("An error occured"), &errorObject);
    SUCCEEDED(hr) ? Print("PASS: TestCreateErrorObjectSucceedsWithValidErrorType\n", true) : Print("FAILED: TestCreateErrorObjectSucceedsWithValidErrorType\n", false);
}

void TestCreateErrorObjectFailsWithInvalidErrorType(MyScriptDirectTests* mytest)
{
    Var errorObject;
    HRESULT hr = mytest->CreateErrorObject(JsErrorType(-1), 0, _u("An error occured"), &errorObject);
    FAILED(hr) ? Print("PASS: TestCreateErrorObjectFailsWithInvalidErrorType\n", true) : Print("FAILED: TestCreateErrorObjectFailsWithInvalidErrorType\n", false);
}

//
// struct to hold one test info, used by RunOnScriptErrorTests to run multiple tests on one script
//
struct OnScriptErrorTest
{
    std::string testName;
    FN_OnScriptError callback;
    bool expectedCallbackResult;

    OnScriptErrorTest(std::string testName, FN_OnScriptError callback = NULL, bool expectedCallbackResult = true):
        testName(testName), callback(callback), expectedCallbackResult(expectedCallbackResult)
    {
    }

    bool IsEmpty() const { return callback == NULL; }
};
OnScriptErrorTest END_TEST("END");

va_list g_onScriptErrorTests; // global test list

HRESULT STDMETHODCALLTYPE ScriptErrorTestsCallback(IActiveScriptError *errorInfo, void* context)
{
    HRESULT hr = S_OK;

    for (;;)
    {
        const OnScriptErrorTest& test = *va_arg(g_onScriptErrorTests, OnScriptErrorTest*);
        if (test.IsEmpty()) // END of test list
        {
            break;
        }

        bool testSucceeded = false;
        g_OnScriptErrorCallbackSucceeded = false;
        std::stringstream testResult;

        try
        {
            hr = test.callback(errorInfo, context);
            IfFailedReturn(hr);

            if (g_OnScriptErrorCallbackSucceeded != test.expectedCallbackResult)
            {
                testResult << "FAILED: " << test.testName << " callback expected " << test.expectedCallbackResult << ", got " << g_OnScriptErrorCallbackSucceeded << endl;
            }
            else
            {
                testSucceeded = true;
                testResult << "PASS: " << test.testName << endl;
            }
        }
        catch (...)
        {
            testResult << "FAILED: " << test.testName << " caught unexpected exception" << endl;
        }
        Print(testResult.str(), testSucceeded);
    }

    return hr;
}

void RunOnScriptErrorTests(MyScriptDirectTests* mytest, LPCWSTR codeToRun, ...
    /* OnScriptErrorTest *test1, OnScriptErrorTest *test2, ..., OnScriptErrorTest *END_TEST */)
{
    va_start(g_onScriptErrorTests, codeToRun);

    std::stringstream testResult;

    mytest->Start();
    mytest->SetOnScriptError(ScriptErrorTestsCallback, mytest);
    if (g_VerboseOutput)
    {
        testResult << "    Executing: " << endl << "\t    " << ConvertNewlinesToPrintable(WStringToString(codeToRun)) << endl;
        Print(testResult.str());
        testResult.str("");
    }

    try
    {
        mytest->ParseAndExecute(codeToRun, SCRIPT_E_REPORTED);
    }
    catch (...)
    {
        testResult << "FAILED: codeToRun caught unexpected exception" << endl;
        Print(testResult.str(), false);
        testResult.str("");
    }
    mytest->End();

    va_end(g_onScriptErrorTests);
}

void RunOneOnScriptErrorTest(MyScriptDirectTests* mytest, std::string testName, FN_OnScriptError callBack, LPCWSTR codeToRun, bool expectedCallbackResult=true)
{
    bool testSucceeded = false;
    g_OnScriptErrorCallbackSucceeded = false;
    std::stringstream testResult;
    mytest->Start();
    mytest->SetOnScriptError(callBack, mytest);
    if (g_VerboseOutput)
    {
        testResult << "    Executing: " << endl << "\t    " << ConvertNewlinesToPrintable(WStringToString(codeToRun)) << endl;
        Print(testResult.str());
        testResult.str("");
    }
    try
    {
        mytest->ParseAndExecute(codeToRun, SCRIPT_E_REPORTED);
        if (g_OnScriptErrorCallbackSucceeded != expectedCallbackResult)
        {
            testResult << "FAILED: " << testName << " callback expected " << expectedCallbackResult << ", got " << g_OnScriptErrorCallbackSucceeded << endl;
        }
        else
        {
            testSucceeded = true;
            testResult << "PASS: " << testName << endl;
        }
    }
    catch (...)
    {
        testResult << "FAILED: " << testName << " caught unexpected exception" << endl;
    }
    Print(testResult.str(), testSucceeded);
    mytest->End();
}

HRESULT STDMETHODCALLTYPE TestOnScriptErrorCalled_Callback(IActiveScriptError *errorInfo, void* context)
{
    g_OnScriptErrorCallbackSucceeded = true;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE TestOnScriptErrorCanQIForOSEEx_Callback(IActiveScriptError *errorInfo, void* context)
{
    IActiveScriptErrorEx *errorInfoEx = NULL;
    HRESULT hr = errorInfo->QueryInterface(__uuidof(IActiveScriptErrorEx), (void**)&errorInfoEx);
    IfFailedReturn(hr);

    g_OnScriptErrorCallbackSucceeded = true;
    errorInfoEx->Release();
    return hr;
}

// This is to ensure we don't regress existing functionality
HRESULT STDMETHODCALLTYPE TestOnScriptErrorGetExceptionInfo_Callback(IActiveScriptError *errorInfo, void* context)
{
    EXCEPINFO exInfo;
    HRESULT hr = errorInfo->GetExceptionInfo(&exInfo);
    IfFailedReturn(hr);

    std::stringstream testResult;
    // let the baseline diff ensure that these results haven't changed
    testResult << "\texInfo.bstrSource: " << WStringToString(exInfo.bstrSource) << endl <<
                  "\texInfo.bstrDescription: " << WStringToString(exInfo.bstrDescription) << endl <<
                  "\texInfo.scode: " << hex << exInfo.scode;
    Print(testResult.str());
    g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && exInfo.scode == 0x800a139e);
    FreeExcepInfo(&exInfo);
    return hr;
}

template <bool retrieveObjectInfo>
HRESULT STDMETHODCALLTYPE TestOnScriptErrorCompileError_Callback(IActiveScriptError *errorInfo, void* context)
{
    IActiveScriptErrorEx *errorInfoEx = NULL;
    HRESULT hr = errorInfo->QueryInterface(__uuidof(IActiveScriptErrorEx), (void**)&errorInfoEx);
    IfFailedReturn(hr);

    if (retrieveObjectInfo)
    {
       hr = VerifyThrownObject(errorInfoEx, context);
    }

    ExtendedExceptionInfo exInfo = { 0 };
    if (SUCCEEDED(hr))
    {
        hr = errorInfoEx->GetExtendedExceptionInfo(&exInfo);
    }
    errorInfoEx->Release();
    IfFailedReturn(hr);

    std::stringstream testResult;
    testResult << "Compile Error:" << endl;
    testResult << "\texInfo.errorType.typeNumber: " << exInfo.errorType.typeNumber << endl;
    if (exInfo.errorType.typeText != NULL) {
        testResult << "\texInfo.errorType.typeText: " << WStringToString(exInfo.errorType.typeText) << endl;
    } else {
        testResult << "\texInfo.errorType.typeText: " << "(null)" << endl;
    }
    testResult << "\texInfo.callStack.frameCount: " << exInfo.callStack.frameCount << endl;
    if (exInfo.callStack.frames == NULL) {
        testResult << "\texInfo.callStack.frames: " << "NULL" << endl;
    } else {
        testResult << "\texInfo.callStack.frames: " << "Must be NULL, but it is not!" << endl;
    }
    testResult << "\texInfo.bstrSource: " << WStringToString(exInfo.exceptionInfo.bstrSource) << endl;
    testResult << "\texInfo.bstrDescription: " << WStringToString(exInfo.exceptionInfo.bstrDescription) << endl;
    testResult << "\texInfo.scode: " << hex << exInfo.exceptionInfo.scode << endl;
    testResult << endl;
    Print(testResult.str());

    g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && exInfo.flags == ExtendedExceptionInfo_Available);
    FreeExcepInfo(&exInfo);
    return hr;
}

HRESULT STDMETHODCALLTYPE VerifyThrownObject(IActiveScriptErrorEx* errorInfoEx, void* context)
{
    const LPWSTR codeToRun = _u("WScript.Echo('unhandled exception info'); if (typeof _testResult == 'object' && _testResult.number == -2146828260) { WScript.Echo(_testResult.description); } else { for (i in _testResult) WScript.Echo('result.' + i + '=' + _testResult[i]); WScript.Echo('result.description=' + _testResult.description); WScript.Echo('result.number=' + _testResult.number); WScript.Echo('result.stack=' + _testResult.stack);}");
    Var thrownObject;
    HRESULT hr;
    hr = errorInfoEx->GetThrownObject(&thrownObject);
    if (SUCCEEDED(hr) && thrownObject != nullptr)
    {
        MyScriptDirectTests* myTest = static_cast<MyScriptDirectTests*>(context);
        myTest->SetProperty(myTest->GetGlobalObject(), _u("_testResult"), thrownObject);
        myTest->ParseAndExecute(codeToRun, NOERROR);
    }
    return hr;
}

template <JsErrorType errorType, bool retrieveObjectInfo>
HRESULT STDMETHODCALLTYPE TestOnScriptErrorGetsExpectedError_Callback(IActiveScriptError *errorInfo, void* context)
{
    IActiveScriptErrorEx *errorInfoEx = NULL;
    HRESULT hr = errorInfo->QueryInterface(__uuidof(IActiveScriptErrorEx), (void**)&errorInfoEx);
    IfFailedReturn(hr);

    if (retrieveObjectInfo)
    {
       hr = VerifyThrownObject(errorInfoEx, context);
    }

    ExtendedExceptionInfo exInfo = { 0 };
    if (SUCCEEDED(hr))
    {
        hr = errorInfoEx->GetExtendedExceptionInfo(&exInfo);
    }
    errorInfoEx->Release();
    IfFailedReturn(hr);

    std::stringstream testResult;
    testResult << "    Uncaught exception:" << endl;
    if (g_VerboseOutput)
    {
        testResult <<
            "\texInfo.errorType.typeNumber: " << exInfo.errorType.typeNumber << endl <<
            "\texInfo.errorType.typeText: " << WStringToString(exInfo.errorType.typeText) << endl <<
            "\texInfo.bstrSource: " << WStringToString(exInfo.exceptionInfo.bstrSource) << endl <<
            "\texInfo.bstrDescription: " << WStringToString(exInfo.exceptionInfo.bstrDescription) << endl <<
            "\texInfo.scode: " << hex << exInfo.exceptionInfo.scode << endl ;
        testResult << endl;
    }
    testResult << "\texpected error type " << errorType << ", got: " << exInfo.errorType.typeNumber <<
        " (" +  WStringToString(exInfo.errorType.typeText) << ": " << WStringToString(exInfo.exceptionInfo.bstrDescription) << ")" << endl;
    Print(testResult.str());

    g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && exInfo.flags == ExtendedExceptionInfo_Available && exInfo.errorType.typeNumber == errorType);
    FreeExcepInfo(&exInfo);
    return hr;
}

class T {}; // dummy class as can't seem to specify multiple nontype parameters w/o a type parameter
enum FrameCountType
{
    exactMatch,
    lessThan,
    greaterThan
};
enum DumpStackType
{
    noDumpStack,
    dumpStack
};

// For the expected results can require
//    1) exact match on number of frames and stack trace output
//    2) just exact match on number of frames
//    3) no more or no less than a certain number of frames.
//
// Generally most tests are (1), OOM tests are (2) and SO tests are (3).
//
template <class T, int expectedFrameCount, FrameCountType frameCountType, DumpStackType dumpStackValue, bool retrieveObjectInfo>
HRESULT STDMETHODCALLTYPE TestOnScriptErrorGetsExpectedCallStack_Callback(IActiveScriptError *errorInfo, void* context)
{
    IActiveScriptErrorEx *errorInfoEx = NULL;
    HRESULT hr = errorInfo->QueryInterface(__uuidof(IActiveScriptErrorEx), (void**)&errorInfoEx);
    IfFailedReturn(hr);

    if (retrieveObjectInfo)
    {
       hr = VerifyThrownObject(errorInfoEx, context);
    }

    ExtendedExceptionInfo exInfo = { 0 };
    if (SUCCEEDED(hr))
    {
        hr = errorInfoEx->GetExtendedExceptionInfo(&exInfo);
    }
    errorInfoEx->Release();
    IfFailedReturn(hr);

    std::stringstream testResult;
    testResult << "    Uncaught exception:" << endl;
    UINT frameCountToDump = frameCountType == exactMatch ? expectedFrameCount : 30;
    if (g_VerboseOutput && dumpStackValue==dumpStack)
    {
        if (frameCountToDump > 0)
        {
            testResult << "\tCall stack: " << endl;
            for (UINT i = 0; i < exInfo.callStack.frameCount && i < frameCountToDump; i++)
            {
                CallStackFrame* frame = &exInfo.callStack.frames[i];
                testResult << "\t    " << WStringToString(frame->functionName) << "(" << frame->lineNumber << ", " << frame->characterPosition << ")" << endl ;
            }
            testResult << endl;
        }
        testResult << endl;
    }
    testResult << "\texInfo.bstrDescription: " << WStringToString(exInfo.exceptionInfo.bstrDescription) << endl;
    if (frameCountType == exactMatch)
    {
        testResult << "\texpected stack frame count " << expectedFrameCount << ", got: " << exInfo.callStack.frameCount << endl;
        g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && exInfo.flags == ExtendedExceptionInfo_Available && exInfo.callStack.frameCount  == expectedFrameCount);
    }
    else if (frameCountType == greaterThan)
    {
        testResult << "\texpected stack frame count > " << expectedFrameCount << ", got: ";
        if (exInfo.callStack.frameCount < expectedFrameCount)
        {
            testResult << exInfo.callStack.frameCount << endl;
        }
        else
        {
            testResult << "at least that" << endl;
        }
        g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && exInfo.flags == ExtendedExceptionInfo_Available && exInfo.callStack.frameCount > expectedFrameCount);
    }
    else
    {
        testResult << "\texpected stack frame count < " << expectedFrameCount << ", got: ";
        if (exInfo.callStack.frameCount >= expectedFrameCount)
        {
            testResult << exInfo.callStack.frameCount << endl;
        }
        else
        {
            testResult << "no more than that" << endl;
        }
        g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && exInfo.flags == ExtendedExceptionInfo_Available && exInfo.callStack.frameCount < expectedFrameCount);
    }
    Print(testResult.str());

    FreeExcepInfo(&exInfo);
    return hr;
}

template <class T, int Line, int Col>
HRESULT STDMETHODCALLTYPE TestOnScriptErrorGetsExpectedLineCol_Callback(IActiveScriptError *errorInfo, void* context)
{
    DWORD sourceContext;
    ULONG lineNumber;
    LONG characterPosition;
    HRESULT hr = errorInfo->GetSourcePosition(&sourceContext, &lineNumber, &characterPosition);
    std::stringstream testResult;

    IfFailedReturn(hr);
    testResult << "\texpected line:col " << Line << ":" << Col << ", got " << lineNumber << ":"  << characterPosition << endl;
    Print(testResult.str());

    g_OnScriptErrorCallbackSucceeded = (SUCCEEDED(hr) && lineNumber==Line && characterPosition == Col);
    return hr;
}

void RunGetExtendedExceptionInfoTests(MyScriptDirectTests* mytest)
{
    WCHAR* simpleThrow = _u("throw new Error");
    WCHAR* nestedThrow = _u("function foo() \n{\n    foo2(); \n}\nfunction foo2() \n{\n    foo3(); \n}\nfunction foo3() \n{\n    baz(); \n}\n function baz() \n{\n    var tttt;  throw new Error('Thrown from baz');\n}\nfoo();");
    WCHAR* nestedThrowWithFinally = _u("function foo() \n{\n    foo2(); \n}\n\nfunction foo2() \n{\n    \n    try\n    {\n        foo3();\n    }\n    finally\n    {\n        var a=5;\n    }\n}\n\nfunction foo3() \n{\n    baz(); \n}\n function baz() \n{\n    var tttt;  throw new Error('Thrown from baz');\n}\nfoo();");
    WCHAR* nestedReThrow = _u("function foo() \n{\n    foo2(); \n}\nfunction foo2() \n{\n    foo3(); \n}\nfunction foo3() \n{\n    try\n    {\n        baz();\n    }\n        catch(e)\n    {\n        throw e;\n    }\n}\n function baz() \n{\n    var tttt;  throw new Error('Thrown from baz');\n}\nfoo();");
    WCHAR* nestedReThrowOnly = _u("function foo() \n{\n    foo2(); \n}\nfunction foo2() \n{\n    try\n    {\n    foo3(); \n    }\n        catch(e)\n    {\n        throw e;\n    }\n}\nfunction foo3() \n{\n    try\n    {\n        baz();\n    }\n        catch(e)\n    {\n        throw e;\n    }\n}\n function baz() \n{\n    var tttt;  throw new Error('reThrown from baz');\n}\nfoo();");
    WCHAR* nestedEvalThrow = _u("function foo() \n{ \n    baz(); \n}\nfunction baz() \n{ \n    eval('function evalbar() { throw new RangeError(); } function evalfoo() { evalbar(); } evalfoo();');\n}\nfoo();");
    WCHAR* throwOOM =  _u("var a = new Array(); \na[0] = 'string'; \na[1] = 'aaa'; \nfor (var i=2; ; i++ ) \n{ \n    a[i] = a[i-1] + a[i-2];\n}");
    WCHAR* throwSO = _u("function foo() \n{\n    foo(); \n}\nfoo();");
    WCHAR* throwSO2 = _u("function foobaz()\n{\n    foo();\n}\nfunction baz()\n{\n    foobaz();\n}\nfunction foo()\n{\n    baz();\n}\nfoo();");
    WCHAR* throwSO3 = _u("function bigStack()\n{\n    var i = 0;\n    function inner()\n    {\n        if (i == 30) return\n        ++i;\n        inner();\n    }\n    inner();\n}\nfunction foobaz()\n{\n    bigStack();\n    foo();\n}\nfunction baz()\n{\n    foobaz();\n}\nfunction foo()\n{\n    baz();\n}\nfoo();");
    WCHAR* reThrowSO = _u("try {\n function foo() \n{\n    foo(); \n}\nfoo(); } \n catch(e) {\n throw e};\n ");
    WCHAR* throwOOMSO1 = _u("var OOM = (function() {\n    var a = new Array();\n    a[0] = 'string';\n    a[1] = 'aaa';\n    var i = 2;\n    return function() {\n        for (;;i++) {\n            a[i] = a[i-1] + a[i-2];\n        }\n    };\n})();\n\nfunction foo() \n{\n    try \n    {\n        foo(); \n    }\n    finally\n    {\n        OOM();\n    }\n}\nfoo();");
    WCHAR* throwOOMSO2 = _u("var callCount = 0;\nvar unwindCount = 0;\nvar throwOOM = true;\nfunction OOM()\n{\n    unwindCount++;\n    if (!throwOOM || (callCount - unwindCount > 30 && unwindCount < 500))\n        return;\n    throwOOM = false;\n    var a = new Array(); \n    a[0] = 'string'; \n    a[1] = 'aaa'; \n    for (var i=2; ; i++ ) \n    { \n        a[i] = a[i-1] + a[i-2];\n    }\n}\nfunction foo() \n{\n    try \n    {\n        callCount++;\n        foo(); \n    }\n    finally\n    {\n        OOM();\n    }\n}\nfoo();");
    WCHAR* nestedThrowWithArguments = _u("function g(x) {\n    f(null, undefined, NaN, true, 1, 'a', { as: 3 }, 23.23, 1e-4, 1, 1, 1, 1, 1, 1, 1);\n}\nfunction f(x) {\n    throw Error('mm');\n}\nfunction h() {\n    g();\n}\nvar s = new String('qqq');\neval('h(s, 123);');\n");
    WCHAR* nestedThrowWithArguments2 = _u("function a() {\n    b(1);\n}\nfunction b(x, y, z) {\n    c(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);\n}\nfunction c() {\n    d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);\n}\nfunction d() {\n    e(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);\n}\nfunction e() {\n    f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21);\n}\nfunction f() {\n    g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);\n}\nfunction g() {\n    h(NaN, null, undefined, 'abacaba', 1, 1.3, 1e5, {}, { a: 4 }, new String('a'), true, NaN, null, undefined, 'abacaba', 1, 1.3, 1e5, {}, { a: 4 }, new String('a'), true);\n}\nfunction h(a,b,c,d,e,f,g,h,i,j,k,l) {\n    throw new Error('Error inside of fucntions with a lot of arguments which has to be displayed');\n}\na();\n");

    TestCreateErrorObjectSucceedsWithValidErrorType(mytest);
    TestCreateErrorObjectFailsWithInvalidErrorType(mytest);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorCalled", TestOnScriptErrorCalled_Callback, simpleThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorCanQIForOSEEx", TestOnScriptErrorCanQIForOSEEx_Callback, simpleThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfo", TestOnScriptErrorGetExceptionInfo_Callback, nestedThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, false>, nestedThrowWithArguments);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments2", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 9, exactMatch, dumpStack, false>, nestedThrowWithArguments2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, true>, nestedThrowWithArguments);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments2", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 9, exactMatch, dumpStack, true>, nestedThrowWithArguments2);

    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetNoExceptionInfoOnCompileError", TestOnScriptErrorCompileError_Callback<false>, _u("var e = 'string;"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetNoExceptionInfoOnCompileError", TestOnScriptErrorCompileError_Callback<true>, _u("var e = 'string;"));

    //Test error types
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedErrorOnOOM", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, false>, throwOOM);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedErrorOnStackOverflow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, false>, throwSO);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedErrorOnOOM", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, true>, throwOOM);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedErrorOnStackOverflow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, true>, throwSO);

    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, false>, _u("throw new Error"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedEvalErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptEvalError, false>, _u("throw new EvalError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedRangeErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptRangeError, false>, _u("throw new RangeError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedRangeErrorOnArrayLengthSetNegative", TestOnScriptErrorGetsExpectedError_Callback<JavascriptRangeError, false>, _u("var a = new Array(); a.length = -1;"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedReferenceErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptReferenceError, false>, _u("throw new ReferenceError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedSyntaxErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptSyntaxError, false>, _u("throw new SyntaxError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedSyntaxErrorOnBadInput", TestOnScriptErrorGetsExpectedError_Callback<JavascriptSyntaxError, false>, _u("eval('var function a=1');"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedTypeErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptTypeError, false>, _u("throw new TypeError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedURIErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptURIError, false>, _u("throw new URIError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedCustomErrorOnThrowString", TestOnScriptErrorGetsExpectedError_Callback<CustomError, false>, _u("throw 'Custom error'"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedCustomErrorOnThrowString", TestOnScriptErrorGetsExpectedError_Callback<CustomError, false>, _u("throw {a: 'aaa', b: 'bbb'}"));

    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, true>, _u("throw new Error"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedEvalErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptEvalError, true>, _u("throw new EvalError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedRangeErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptRangeError, true>, _u("throw new RangeError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedRangeErrorOnArrayLengthSetNegative", TestOnScriptErrorGetsExpectedError_Callback<JavascriptRangeError, true>, _u("var a = new Array(); a.length = -1;"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedReferenceErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptReferenceError, true>, _u("throw new ReferenceError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedSyntaxErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptSyntaxError, true>, _u("throw new SyntaxError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedSyntaxErrorOnBadInput", TestOnScriptErrorGetsExpectedError_Callback<JavascriptSyntaxError, true>, _u("eval('var function a=1');"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedTypeErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptTypeError, true>, _u("throw new TypeError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedURIErrorOnExplicitThrow", TestOnScriptErrorGetsExpectedError_Callback<JavascriptURIError, true>, _u("throw new URIError"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedCustomErrorOnThrowString", TestOnScriptErrorGetsExpectedError_Callback<CustomError, true>, _u("throw 'Custom error'"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedCustomErrorOnThrowString", TestOnScriptErrorGetsExpectedError_Callback<CustomError, true>, _u("throw {a: 'aaa', b: 'bbb'}"));

    //Test stack traces
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, false>, nestedThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedThrowWithFinally", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, false>, nestedThrowWithFinally);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnReThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 10, exactMatch, dumpStack, false>, nestedReThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedReThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 9, exactMatch, dumpStack, false>, nestedReThrowOnly);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnReThrowSO", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 2, exactMatch, dumpStack, false>, reThrowSO);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedEvalThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 6, exactMatch, dumpStack, false>, nestedEvalThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 12, exactMatch, dumpStack, false>, throwSO);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO2a", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 33, lessThan, noDumpStack, false>, throwSO2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO2b", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 29, greaterThan, noDumpStack, false>, throwSO2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO3", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 29, greaterThan, noDumpStack, false>, throwSO3);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnOOM", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 1, exactMatch, noDumpStack, false>, throwOOM);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnOOMSO2", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 20, greaterThan, noDumpStack, false>, throwOOMSO2);

    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, true>, nestedThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedThrowWithFinally", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, true>, nestedThrowWithFinally);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnReThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 10, exactMatch, dumpStack, true>, nestedReThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedReThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 9, exactMatch, dumpStack, true>, nestedReThrowOnly);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnReThrowSO", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 2, exactMatch, dumpStack, true>, reThrowSO);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnNestedEvalThrow", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 6, exactMatch, dumpStack, true>, nestedEvalThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 12, exactMatch, dumpStack, true>, throwSO);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO2a", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 33, lessThan, noDumpStack, true>, throwSO2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO2b", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 29, greaterThan, noDumpStack, true>, throwSO2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnSO3", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 29, greaterThan, noDumpStack, true>, throwSO3);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnOOM", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 1, exactMatch, noDumpStack, true>, throwOOM);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedStackTraceOnOOMSO2", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 20, greaterThan, noDumpStack, true>, throwOOMSO2);

    //This one runs very slow. Run the script once and apply multiple tests to save time.
    OnScriptErrorTest OOMTest1 = OnScriptErrorTest("TestOnScriptErrorGetsExpectedErrorOnOOMSO1", TestOnScriptErrorGetsExpectedError_Callback<JavascriptError, false>);
    OnScriptErrorTest OOMTest2 = OnScriptErrorTest("TestOnScriptErrorGetsExpectedStackTraceOnOOMSO1", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 3, exactMatch, noDumpStack, false>);
    RunOnScriptErrorTests(mytest,
        throwOOMSO1,
        &OOMTest1,
        &OOMTest2,
        &END_TEST);

    //Run these to check manually that don't generate a stack trace.
    // JenH TODO: Figure out a way to test automagically
    RunOneOnScriptErrorTest(mytest, "TestOnScriptNoErrorWithCatchInPlaceSimple", TestOnScriptErrorCalled_Callback,
        _u("try \n{\n    foo(); \n} \ncatch(e) \n{\n    \n}"), false);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptNoErrorWithCatchInPlaceNested", TestOnScriptErrorCalled_Callback,
        _u("try \n{ \n    foo();\n} \ncatch(e) \n{\n   \n}\nfunction foo() \n{\n    try\n    { \n        throw new Error;\n    }\n    catch(e) \n    {\n   \n    }\n    throw new Error;\n}"), false);


    // Regression tests
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedLineColOnSimpleThrow", TestOnScriptErrorGetsExpectedLineCol_Callback<T, 0, 0>, simpleThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedLineColOnSimpleThrowPrecedingNL", TestOnScriptErrorGetsExpectedLineCol_Callback<T, 1, 1>,
        _u("\n throw Error"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedLineColOnSimpleThrowWithNLs", TestOnScriptErrorGetsExpectedLineCol_Callback<T, 2, 7>,
        _u("\n\n var a;throw new Error"));
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedLineColOnNestedThrow", TestOnScriptErrorGetsExpectedLineCol_Callback<T, 14, 15>, nestedThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedLineColOnThrowFromEval", TestOnScriptErrorGetsExpectedLineCol_Callback<T, 0, 21>, nestedEvalThrow);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetsExpectedLineColOnStackOverflow", TestOnScriptErrorGetsExpectedLineCol_Callback<T, 2, 4>, throwSO);

 }

void RunStackTraceInlineTests(MyScriptDirectTests* mytest)
{
    WCHAR* nestedThrowWithArguments = _u("function g(x) {\n    f(null, undefined, NaN, true, 1, 'a', { as: 3 }, 23.23, 1e-4, 1, 1, 1, 1, 1, 1, 1);\n}\nfunction f(x) {\n    throw Error('mm');\n}\nfunction h() {\n    g();\n}\nvar s = new String('qqq');\neval('h(s, 123);');\n");
    WCHAR* nestedThrowWithArguments2 = _u("function a() {\n    b(1);\n}\nfunction b(x, y, z) {\n    c(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);\n}\nfunction c() {\n    d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);\n}\nfunction d() {\n    e(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);\n}\nfunction e() {\n    f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21);\n}\nfunction f() {\n    g(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);\n}\nfunction g() {\n    h(NaN, null, undefined, 'abacaba', 1, 1.3, 1e5, {}, { a: 4 }, new String('a'), true, NaN, null, undefined, 'abacaba', 1, 1.3, 1e5, {}, { a: 4 }, new String('a'), true);\n}\nfunction h(a,b,c,d,e,f,g,h,i,j,k,l) {\n    throw new Error('Error inside of fucntions with a lot of arguments which has to be displayed');\n}\na();\n");
    WCHAR* nestedThrowWithArguments3 = _u("function f(x) {\n    throw new Error('x'); \n}\nf(3);");

    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, false>, nestedThrowWithArguments);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments2", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 9, exactMatch, dumpStack, false>, nestedThrowWithArguments2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments3", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 2, exactMatch, dumpStack, false>, nestedThrowWithArguments3);

    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 5, exactMatch, dumpStack, true>, nestedThrowWithArguments);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments2", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 9, exactMatch, dumpStack, true>, nestedThrowWithArguments2);
    RunOneOnScriptErrorTest(mytest, "TestOnScriptErrorGetExceptionInfoWithArguments3", TestOnScriptErrorGetsExpectedCallStack_Callback<T, 2, exactMatch, dumpStack, true>, nestedThrowWithArguments3);
}
