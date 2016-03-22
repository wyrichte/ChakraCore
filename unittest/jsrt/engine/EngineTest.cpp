//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "EngineTest.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{
    char EngineTest::cmpbuf1[CMPBUF_SIZE];
    char EngineTest::cmpbuf2[CMPBUF_SIZE];

    void EngineTest::RunScript()
    {
        String testLocation;

        if (SUCCEEDED(TestData::TryGetValue(L"Location", testLocation)))
        {
            WCHAR sdxRoot[_MAX_PATH];

            DWORD stored = GetEnvironmentVariableW(L"SDXROOT", sdxRoot, _MAX_PATH);
            if (!stored || stored >= _MAX_PATH)
            {
                VERIFY_FAIL(L"Error retrieving SDXROOT environment variable");
                return;
            }

            String basePath(sdxRoot);

            basePath += L"\\onecoreuap\\inetcore\\jscript\\";

            String outputFile = basePath + testLocation + L".out";
            this->output = CreateFile(outputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            if (!this->output.IsValid())
            {
                VERIFY_FAIL(L"can't open output file");
                return;
            }

            LPCWSTR script = LoadScriptFileWithPath(basePath + L"\\" + testLocation + L".js");

            if (script == NULL)
            {
                return;
            }

            JsRuntimeHandle runtime;
            VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeAllowScriptInterrupt, NULL, &runtime) == JsNoError);

            JsContextRef context;
            VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);

            JsValueRef wscript;
            VERIFY_IS_TRUE(JsCreateObject(&wscript) == JsNoError);

            JsValueRef echo;
            VERIFY_IS_TRUE(JsCreateFunction(EchoCallback, this, &echo) == JsNoError);

            JsPropertyIdRef echoProperty;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"Echo", &echoProperty) == JsNoError);

            VERIFY_IS_TRUE(JsSetProperty(wscript, echoProperty, echo, true) == JsNoError);

            JsPropertyIdRef wscriptProperty;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"WScript", &wscriptProperty) == JsNoError);

            JsValueRef global;
            VERIFY_IS_TRUE(JsGetGlobalObject(&global) == JsNoError);

            VERIFY_IS_TRUE(JsSetProperty(global, wscriptProperty, wscript, true) == JsNoError);

            VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsNoError);

            VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
            VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);
            this->output.Close();

            String baselineFile = basePath + testLocation + L".baseline";
            VERIFY_IS_TRUE(CompareFiles(baselineFile, outputFile));
        }
    }

    JsValueRef EngineTest::EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
    {
        EngineTest * _this = (EngineTest *)callbackState;
        return _this->Echo(callee, isConstructCall, arguments, argumentCount);
    }

    JsValueRef EngineTest::Echo(JsValueRef /* callee */, bool /* isConstructCall */, JsValueRef *arguments, unsigned short argumentCount)
    {
        for (unsigned int i = 1; i < argumentCount; i++)
        {
            if (i > 1)
            {
                WriteOutput(L" ");
            }

            JsValueRef value;
            VERIFY_IS_TRUE(JsConvertValueToString(arguments[i], &value) == JsNoError);

            const wchar_t *string;
            size_t length;
            VERIFY_IS_TRUE(JsStringToPointer(value, &string, &length) == JsNoError);

            WriteOutput(string);
        }

        WriteOutput(L"\r\n");

        return JS_INVALID_REFERENCE;
    }

    void EngineTest::WriteOutput(LPCWSTR toWrite)
    {
        DWORD written;
        int len = wcslen(toWrite);
        char * toWriteNarrow = (char *)malloc(len + 1);

        if (toWriteNarrow == NULL)
        {
            VERIFY_FAIL(L"out of memory");
            return;
        }

        ::wcstombs(toWriteNarrow, toWrite, len + 1);

        if (!WriteFile(this->output, toWriteNarrow, len, &written, NULL))
        {
            VERIFY_FAIL(L"error writing to output file");
        }

        free(toWriteNarrow);
    }

    bool EngineTest::CompareFiles(LPCWSTR filename1, LPCWSTR filename2)
    {
        AutoHandle h1, h2;     // automagically closes open handles
        DWORD count1, count2;
        DWORD size1, size2;

        h1 = CreateFile(filename1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (!h1.IsValid())
        {
            VERIFY_FAIL(String(L"Unable to open file %s").Format(filename1));
            return false;
        }

        h2 = CreateFile(filename2, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (!h2.IsValid())
        {
            VERIFY_FAIL(String(L"Unable to open file %s").Format(filename1));
            return false;
        }

        // Short circuit by first checking for different file lengths.

        // assume <4GB files!
        size1 = GetFileSize(h1, NULL);
        if (size1 == 0xFFFFFFFF)
        {
            VERIFY_FAIL(String(L"Unable to get file size for %s").Format(filename1));
            return false;
        }

        size2 = GetFileSize(h2, NULL);
        if (size2 == 0xFFFFFFFF)
        {
            VERIFY_FAIL(String(L"Unable to get file size for %s").Format(filename1));
            return false;
        }

        // not equal; don't bother reading the files
        if (size1 != size2)
        {
            return false;
        }

        do
        {
            if (!ReadFile(h1, cmpbuf1, CMPBUF_SIZE, &count1, NULL) ||
                !ReadFile(h2, cmpbuf2, CMPBUF_SIZE, &count2, NULL))
            {
                VERIFY_FAIL(String(L"ReadFile failed doing compare of %s and %s").Format(filename1, filename2));;
                return false;
            }

            if (count1 != count2 || memcmp(cmpbuf1, cmpbuf2, count1) != 0)
            {
                return false;
            }
        }
        while (count1 == CMPBUF_SIZE);

        return true;
    }
}
