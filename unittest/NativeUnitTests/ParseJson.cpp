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
    std::wcout << L"JSON Text:" << std::wstring(data.jsonText == nullptr ? L"<nullptr>" : data.jsonText)
        << L", HR:" << hex << hr << L", Var:";
    if (data.var == nullptr)
    {
        std::wcout << L"<nullptr>";
    }
    else if (data.var == nullVar)
    {
        std::wcout << L"<nullVar>";
    }
    else
    {
        std::wcout << L"<Var>";
    }
    std::wcout << endl;
}

HRESULT TestBasicActiveScriptDirectParseJson(MyScriptDirectTests* myTests)
{
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();

    activeScriptDirect->GetNull(&nullVar);

    auto TestJson = [&](JSONTestData& data)->bool
    {
        UINT len = data.length < 0 ? wcslen(data.jsonText) : data.length;
        HRESULT hr = activeScriptDirect->ParseJson(data.jsonText, len, data.var);
        printData(data, hr);
        return data.verify(data, hr);
    };

    Var varOut;
    
    JSONTestData testdata[] = {
        { L"", -1, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr != S_OK && *(data.var) == nullVar; } },
        { L"{\"a\":1}", -1, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == S_OK && *(data.var) != nullVar; } },
        { L"adsfa", -1, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{  return hr != S_OK && *(data.var) == nullVar; } },
        { nullptr, 0, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == E_INVALIDARG && *(data.var) == nullVar; } },
        { L"{a:0}", -1, (Var*)nullptr, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == E_INVALIDARG && data.var == nullptr; } },
        { L"{\"a\":1}}}}}", 7, &varOut, [](const JSONTestData &data, HRESULT hr)->bool{ return hr == S_OK && *(data.var) != nullVar; } },
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

