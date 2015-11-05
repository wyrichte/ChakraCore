// Copyright (C) Microsoft. All rights reserved. 
// FastDOMNative.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <sstream>
#include "windows.h"
#include "shlwapi.h"
#include "MyScriptDirectNative.h"
#include "Verifier.h"
#include "FakeMSHTML.h"
#include "ProfileCacheTests.h"


using namespace std;

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);
std::string ConvertNewlinesToPrintable(const std::string& s);
std::string Summary(int result);
void Print(std::string message, bool result);
void Print(std::string message);

void RunFastDomTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify);
void RunGetExtendedExceptionInfoTests(MyScriptDirectTests* mytest);
void RunStackTraceInlineTests(MyScriptDirectTests* mytest);
void RunRegexTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify);
void RunTypedArrayTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify);
void RunTypedArrayES6Tests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify);
void RunInt64Tests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify);
void RunBasicTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify);
void RunHeapEnumTest(MyScriptDirectTests* mytest);
void RunProfileCacheTests(JsHostNativeTestArguments* args);
void RunVersionTest();
void RunScriptDiagSerializationTest();
void RunJsDirectTest(MyScriptDirectTests* mytest);
void RunJsDirectNoScriptScopeTests(MyScriptDirectTests* myTests);
void RunJsDirectNoScriptScopeErrorCaseTests(MyScriptDirectTests* myTests);
void RunJsDirectNoScriptScopeFailfastTest(MyScriptDirectTests* myTests);
void RunAsyncDebugTest(MyScriptDirectTests* myTests);
void RunStaticLibVerificationTest();
void RunVarToNativeArrayTest(MyScriptDirectTests* myTests);
void RunCreatePromiseTest(MyScriptDirectTests* myTests);
void RunParseJsonTest(MyScriptDirectTests* myTests);
void RunVarToNativeArrayTest(MyScriptDirectTests* myTests);
void RunExternalFuncFpcwTest(MyScriptDirectTests* mytest);
void StartTest(MyScriptDirectTests* myTests, std::string message);
void FinishTest(MyScriptDirectTests* myTests, std::string message, HRESULT result);
