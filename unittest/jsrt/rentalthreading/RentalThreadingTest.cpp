//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ThreadPool.h"
#include "RentalThreadingTest.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{           
    const int RentalThreadingTest::fiboCount = 64;
    const double RentalThreadingTest::fiboStopAt = 100.0;            

    RentalThreadingTest::FiboCalculator::FiboCalculator(ThreadPool * pool, double stopAt) :
        pool(pool), 
        stopAt(stopAt),
        wait(NULL), 
        pendingWork(false), 
        result(0.0), 
        nextFibo(),
        context()
    {
        VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeAllowScriptInterrupt, NULL, &this->runtime) == JsNoError);
        VERIFY_IS_TRUE(JsCreateContext(this->runtime, &this->context) == JsNoError);
        this->wait = CreateEvent(NULL, TRUE, FALSE, NULL);
        
        VERIFY_IS_TRUE(JsSetCurrentContext(this->context) == JsNoError);
        LPCWSTR script = LoadScriptFile(L"Fibonacci.js");
        VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsNoError);

        JsValueRef global;
        VERIFY_IS_TRUE(JsGetGlobalObject(&global) == JsNoError);

        JsPropertyIdRef nextFiboProperty;
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"nextFibo", &nextFiboProperty) == JsNoError);

        VERIFY_IS_TRUE(JsGetProperty(global, nextFiboProperty, &this->nextFibo) == JsNoError);
        
        JsValueRef num1;
        VERIFY_IS_TRUE(JsDoubleToNumber(0, &num1) == JsNoError);
        JsValueRef num2;
        VERIFY_IS_TRUE(JsDoubleToNumber(1, &num2) == JsNoError);

        JsPropertyIdRef num1Property;
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"num1", &num1Property) == JsNoError);
        JsPropertyIdRef num2Property;
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"num2", &num2Property) == JsNoError);

        VERIFY_IS_TRUE(JsSetProperty(global, num1Property, num1, true) == JsNoError);
        VERIFY_IS_TRUE(JsSetProperty(global, num2Property, num2, true) == JsNoError);
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
    }

    RentalThreadingTest::FiboCalculator::~FiboCalculator()
    {
        VERIFY_IS_TRUE(JsDisposeRuntime(this->runtime) == JsNoError);
        CloseHandle(this->wait);
    }

    void RentalThreadingTest::FiboCalculator::Start()
    {             
        this->pendingWork = true;
        this->pool->QueueWorkItem(&FiboCalculator::FiboContinue, this);
    }

    void RentalThreadingTest::FiboCalculator::Wait()
    {
        WaitForSingleObject(this->wait, INFINITE);
    }

    double RentalThreadingTest::FiboCalculator::GetResult()
    {             
        return this->result;
    }

    void RentalThreadingTest::FiboCalculator::FiboContinue()
    {     
        {
            VERIFY_IS_TRUE(JsSetCurrentContext(this->context) == JsNoError);
            JsValueRef fiboCallback;
            VERIFY_IS_TRUE(JsCreateFunction(FiboCallback, this, &fiboCallback) == JsNoError);
            JsValueRef args[2] = { GetUndefined(), fiboCallback };
            VERIFY_IS_TRUE(JsCallFunction(this->nextFibo, args, 2, nullptr) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
        }
        
        if (this->pendingWork)
        {
            this->pool->QueueWorkItem(&FiboCalculator::FiboContinue, this);
        }
        else
        {
            SetEvent(this->wait);
        }        
    }

    JsValueRef RentalThreadingTest::FiboCalculator::FiboCallback(JsValueRef /* callee */, bool /* isConstructCall */, JsValueRef *arguments, unsigned short /* argumentCount */, void *callbackState)
    {
        FiboCalculator * _this = (FiboCalculator *)callbackState;
        
        double dbl2;
        VERIFY_IS_TRUE(JsNumberToDouble(arguments[1], &dbl2) == JsNoError);

        if (dbl2 < _this->stopAt)
        {
            _this->pendingWork = true;
        }
        else
        {
            _this->result = dbl2;
            _this->pendingWork = false;
        }

        return JS_INVALID_REFERENCE;
    }

    void RentalThreadingTest::DoTest()
    {
        ThreadPool pool(4, 4);
        FiboCalculator * calculators[fiboCount];

        for (int i = 0; i < fiboCount; i++)
        {
            calculators[i] = new FiboCalculator(&pool, fiboStopAt);            
        }

        for (int i = 0; i < fiboCount; i++)
        {
            calculators[i]->Start();         
        }

        for (int i = 0; i < fiboCount; i++)
        {
            calculators[i]->Wait();         
        }

        for (int i = 0; i < fiboCount; i++)
        {
            VERIFY_IS_TRUE(calculators[i]->GetResult() == 144);
        }

        for (int i = 0; i < fiboCount; i++)
        {
            delete calculators[i];
        }
    }
}
