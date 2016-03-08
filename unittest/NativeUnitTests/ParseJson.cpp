// Copyright (C) Microsoft. All rights reserved.
//

#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"

struct JSONTestData
{
    LPCWSTR jsonText;
    int length; 
    Var* var;
    bool(*verify)(const JSONTestData&, HRESULT);
};

Var nullVar;

void printData(const JSONTestData &data, HRESULT hr)
{
    std::wcout << _u("JSON Text:") << std::wstring(data.jsonText == nullptr ? _u("<nullptr>") : data.jsonText)
        << _u(", HR:") << hex << hr << _u(", Var:");
    if (data.var == nullptr)
    {
        std::wcout << _u("<nullptr>");
    }
    else if (data.var == nullVar)
    {
        std::wcout << _u("<nullVar>");
    }
    else
    {
        std::wcout << _u("<Var>");
    }
    std::wcout << endl;
}

HRESULT TestBasicActiveScriptDirectParseJson(MyScriptDirectTests* myTests)
{
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();

    activeScriptDirect->GetNull(&nullVar);

    auto TestJson = [&](JSONTestData& data)->bool
    {
        UINT len = data.length < 0 ? (UINT)wcslen(data.jsonText) : data.length;
        HRESULT hr = activeScriptDirect->ParseJson(data.jsonText, len, data.var);
        printData(data, hr);
        return data.verify(data, hr);
    };

    Var varOut;
    
    JSONTestData testdata[] = {
        { _u(""), -1, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr != S_OK && *(data.var) == nullVar; } },
        { _u("{\"a\":1}"), -1, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == S_OK && *(data.var) != nullVar; } },
        { _u("adsfa"), -1, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{  return hr != S_OK && *(data.var) == nullVar; } },
        { nullptr, 0, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == E_INVALIDARG && *(data.var) == nullVar; } },
        { _u("{a:0}"), -1, (Var*)nullptr, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == E_INVALIDARG && data.var == nullptr; } },
        { _u("{\"a\":1}}}}}"), 7, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == S_OK && *(data.var) != nullVar; } },
    };

    int failedCount = 0;

    for (int i = 0; i < _countof(testdata); i++)
    {
        StartTest(myTests, "Test basic IActiveScriptDirect::ParseJson functionality...");
        if (TestJson(testdata[i]))
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

void RunParseJsonTest(MyScriptDirectTests* myTests)
{
    HRESULT hr;

    try
    {
        hr = TestBasicActiveScriptDirectParseJson(myTests);

        if (FAILED(hr))
        {
            Print("Test failed", false);
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

