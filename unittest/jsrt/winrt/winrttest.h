//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{  
    class WinRTTest
    {
    private:
        BEGIN_TEST_CLASS(WinRTTest)
            TEST_CLASS_PROPERTY(L"Description", L"WinRT support test")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()
        TEST_METHOD_SETUP(Setup);
        TEST_METHOD_CLEANUP(Cleanup);
        
        TEST_METHOD(BasicTest);
        TEST_METHOD(ProjectionCallbackEventTest);
        TEST_METHOD(ProjectionCallbackAsyncTest);
        TEST_METHOD(ProjectionCallbackErrorTest);
        TEST_METHOD(InspectableTest);

    public:
        static JsValueRef GetUndefined()
        {
            JsValueRef undefined;
            VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);
            return undefined;
        }
    };
};

