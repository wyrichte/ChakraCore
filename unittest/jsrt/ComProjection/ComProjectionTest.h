//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{
    class ComProjectionTest
    {
    public:
        ComProjectionTest();

    private:
        BEGIN_TEST_CLASS(ComProjectionTest)
            TEST_CLASS_PROPERTY(L"Description", L"COM Projection tests")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()

        TEST_METHOD_SETUP(Setup);
        TEST_METHOD_CLEANUP(Cleanup);

        TEST_METHOD(BasicTest);
        TEST_METHOD(IDispatchToObjectTest);
        TEST_METHOD(ValueToVariantTest);

    private:
        static DWORD WINAPI DoIDispatchToObjectTestCallback(void * state);
        static DWORD WINAPI DoValueToVariantTestCallback(void * state);
        static bool SetDispatchProperty(IDispatch * object, BSTR name, VARIANT * value);
        static bool SetDispatchProperty(IDispatchEx * object, BSTR name, VARIANT * value);
        static bool GetDispatchProperty(IDispatch * object, BSTR name, VARIANT * value);
        static bool InvokeDispatchMethod(IDispatch * object, BSTR name, VARIANT * args, int cargs, VARIANT * result);

    private:
        JsRuntimeHandle runtime;
        JsContextRef context;
        static CLSID fsoClsid;
    };   
}