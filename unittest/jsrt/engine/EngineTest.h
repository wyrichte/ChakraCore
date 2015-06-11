//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

using namespace WEX::Common;

#define CMPBUF_SIZE 0xFFFF

namespace JsrtUnitTests
{
    class AutoHandle
    {
    public:
        AutoHandle() : handle(INVALID_HANDLE_VALUE)
        {
        }

        ~AutoHandle()
        {
            Close();
        }

        bool IsValid() const
        {
            return (this->handle != INVALID_HANDLE_VALUE);
        }

        AutoHandle & operator=(HANDLE handle)
        {
            this->handle = handle;
            return *this;
        }

        void Close()
        {
            if (this->IsValid())
            {
                CloseHandle(this->handle);
                this->handle = INVALID_HANDLE_VALUE;
            }
        }

        operator const HANDLE()
        {
            return this->handle;
        }

    private:
        HANDLE handle;
    };

    class EngineTest
    {    
    private:
        BEGIN_TEST_CLASS(EngineTest)
            TEST_CLASS_PROPERTY(L"Description", L"Runs jscript9 unittests using jsrt.dll")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()   

        BEGIN_TEST_METHOD(RunScript)
            TEST_METHOD_PROPERTY(L"DataSource", L"Table:Tests.xml#Table")
        END_TEST_METHOD()

    private:
        bool CompareFiles(LPCWSTR filename1, LPCWSTR filename2);
        static JsValueRef __stdcall EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
        JsValueRef Echo(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount);
        void WriteOutput(LPCWSTR toWrite);

    private:
        AutoHandle output;
        static char cmpbuf1[CMPBUF_SIZE];
        static char cmpbuf2[CMPBUF_SIZE];
        
    };
}