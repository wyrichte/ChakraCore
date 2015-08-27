//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{    
    class RentalThreadingTest
    {
        class FiboCalculator
        { 
        public:
            FiboCalculator(ThreadPool * pool, double stopAt);
            ~FiboCalculator();

            void Start();
            void Wait();
            double GetResult();

        private:
            void FiboContinue();
            static JsValueRef __stdcall FiboCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

        private:         
            HANDLE wait;
            bool pendingWork;
            JsRuntimeHandle runtime;
            JsValueRef nextFibo;            
            JsContextRef context;
            ThreadPool * pool;
            double result;
            double stopAt;
        };

    private:
        BEGIN_TEST_CLASS(RentalThreadingTest)
            TEST_CLASS_PROPERTY(L"Description", L"jsrt.dll rental threading test")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()

        TEST_METHOD(DoTest);     

        static JsValueRef GetUndefined()
        {
            JsValueRef undefined;
            VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);
            return undefined;
        }

    private:
        static const int fiboCount;  
        static const double fiboStopAt;
    };
}