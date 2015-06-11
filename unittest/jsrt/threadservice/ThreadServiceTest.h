//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{    
    class ThreadServiceTest
    {
    private:
        BEGIN_TEST_CLASS(ThreadServiceTest)
            TEST_CLASS_PROPERTY(L"Description", L"jsrt.dll thread service test")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()

        TEST_METHOD(DoThreadPoolTest);     
        TEST_METHOD(DoAlwaysDenyRequestTest);

    private:
        JsRuntimeHandle runtime;
        JsContextRef context;
        static bool sawCallback;

    private:
        void DoTest(JsThreadServiceCallback threadService);
            
        static bool CALLBACK SubmitBackgroundWorkToThreadPool(JsBackgroundWorkItemCallback callback, void * callbackData);
        static bool CALLBACK FailBackgroundWorkRequest(JsBackgroundWorkItemCallback callback, void * callbackData);   

        static void CALLBACK ThreadPoolCallback(PTP_CALLBACK_INSTANCE Instance, void * Context);

        struct ThreadPoolCallbackContext
        {
            JsBackgroundWorkItemCallback callback;
            void * callbackData;
        };
    };
}
