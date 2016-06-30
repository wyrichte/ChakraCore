// Copyright (C) Microsoft. All rights reserved.
//

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"

typedef struct
{
    LPCWSTR executeCode;
    JsNativeValueType valueType;
    HRESULT expectedHr;
    ULONG expectedLength;
    ULONG expectedSize;
    void* result;
    void* expectedResult;
} NativeArrayTestData;

int result1[] = { 1,2,3,4 };
float result2[] = { 1, 2, 3, 4 };
LPCWSTR result3[] = { _u("a"), _u("b"), _u("c") };
int result4[] = { 0, 0, 0, 0 };

NativeArrayTestData testdata[] = {
    {_u("(function() {return [1,2,3,4]})();"), JsInt32Type, S_OK, 4, 4, nullptr, (void*)&result1},
    { _u("(function() {return [1,2,3,4]})();"), JsFloatType, S_OK, 4, 4, nullptr, (void*)&result2 },
    { _u("(function() {var a = {'0':1,'1':2,'2':3,'3':4}; a.length = 4; return a})();"), JsInt32Type, S_OK, 4, 4, nullptr, (void*)&result1 },
    { _u("(function() {var a = {'0':1,'1':2,'2':3,'3':4}; a.length = 4; return a})();"), JsFloatType, S_OK, 4, 4, nullptr, (void*)&result2 },
    {_u("(function() {return ['a', 'b', 'c' ]})();"), JsNativeStringType, S_OK, 3, sizeof(JsNativeString), nullptr, (void*)&result3},
    { _u("(function() {return ['1', '2', '3', '4' ]})();"), JsInt32Type, S_OK, 4, 4, nullptr, (void*)&result1 },
    { _u("(function() {return ['1', '2', '3', '4' ]})();"), JsFloatType, S_OK, 4, 4, nullptr, (void*)&result2 },
    { _u("(function() {var a = {'0':1,'1':2,'2':3,'3':4}; a.length = 2147483647; return a})();"), JsInt32Type, E_INVALIDARG, 4, 4, nullptr, (void*)&result1 },
    { _u("(function() {var a = ['a', 'b', 'c' ]; a.length = 2147483647; return a})();"), JsInt32Type, E_INVALIDARG, 4, 4, nullptr, (void*)&result1 },
    { _u("(function() {var a = ['a', 'b', 'c' ]; return a})();"), JsInt32Type, S_OK, 3, 4, nullptr, (void*)&result4 },
};
#define delta(left,right) (left > right? left-right: right-left)
HRESULT TestBasicActiveScriptDirectVarToNativeArray(MyScriptDirectTests* myTests)
{
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();
    
    auto TestNativeArray = [&](NativeArrayTestData& data)->HRESULT
    {
        UINT length, elementSize;
        HRESULT hr;
        CallInfo callInfo = { 0, CallFlags_None };
        Var topFunc;
        hr = activeScriptDirect->Parse((LPWSTR)data.executeCode, &topFunc);
        IfFailedReturn(hr);
        Var varResult;
        hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*servicerProvider*/ NULL, &varResult);
        IfFailedReturn(hr);

        hr = activeScriptDirect->VarToNativeArray(varResult, data.valueType, (byte**)&data.result, &length, &elementSize);
        if (hr != data.expectedHr)
        {
            printf("test failed: %x in %S\n", hr, data.executeCode);
            return E_FAIL;
        }
        if (SUCCEEDED(hr))
        {
            if (length != data.expectedLength || elementSize != data.expectedSize)
            {
                printf("test result is wrong: %S %d size is %d\n", data.executeCode, length, elementSize);
                return E_FAIL;
            }
            for (uint i = 0; i < length; i++)
            {
                switch (data.valueType)
                {
                case JsInt8Type:
                    if (((char*)data.result)[i] != ((char*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((char*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsUint8Type:
                    if (((unsigned char*)data.result)[i] != ((unsigned char*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((unsigned char*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsInt16Type:
                    if (((short*)data.result)[i] != ((short*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((short*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsUint16Type:
                    if (((unsigned short*)data.result)[i] != ((unsigned short*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((unsigned short*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsInt32Type:
                    if (((int*)data.result)[i] != ((int*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((int*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsUint32Type:
                    if (((unsigned int*)data.result)[i] != ((unsigned int*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((unsigned int*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsInt64Type:
                    if (((__int64*)data.result)[i] != ((__int64*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((__int64*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsUint64Type:
                    if (((unsigned __int64*)data.result)[i] != ((unsigned __int64*)data.expectedResult)[i])
                    {
                        printf("test result is wrong: %S [%d] is %d\n", data.executeCode, i, ((unsigned __int64*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsFloatType:
                    if (delta(((float*)data.result)[i] , ((float*)data.expectedResult)[i]) > 0.01)
                    {
                        printf("test result is wrong: %S [%d] is %f\n", data.executeCode, i, ((float*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsDoubleType:
                    if (delta(((double*)data.result)[i] , ((double*)data.expectedResult)[i]) < 0.01)
                    {
                        printf("test result is wrong: %S [%d] is %f\n", data.executeCode, i, ((double*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                case JsNativeStringType:
                    if (wcscmp(((JsNativeString*)data.result)[i].str,((LPCWSTR*)data.expectedResult)[i]) != 0)
                    {
                        printf("test result is wrong: %S [%d] is %s\n", data.executeCode, i, ((LPCWSTR*)data.result)[i]);
                        return E_FAIL;
                    }
                    break;
                }
            }
            CoTaskMemFree(data.result);
            data.result = nullptr;
        }
        return NOERROR;
    };

    UINT failedCount = 0;
    for (int i = 0; i < _countof(testdata); i++)
    {
        StartTest(myTests, "Test basic IActiveScriptDirect::VarToNativeArray functionality...");
        if (TestNativeArray(testdata[i]) == S_OK)
        {
            FinishTest(myTests, "Done.", S_OK);
        }
        else
        {
            failedCount++;
            FinishTest(myTests, "Failed.", E_FAIL);
        }
    }

    return failedCount == 0 ? S_OK : E_FAIL;
}

void RunVarToNativeArrayTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestBasicActiveScriptDirectVarToNativeArray(myTests);

        if (FAILED(hr))
        {
            printf("Test failed", false);
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
