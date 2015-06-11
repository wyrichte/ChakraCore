//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ThreadServiceTest.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{           
    bool ThreadServiceTest::sawCallback = false;
    
    bool ThreadServiceTest::SubmitBackgroundWorkToThreadPool(JsBackgroundWorkItemCallback callback, void * callbackData)
    {
        VERIFY_IS_TRUE(callback != NULL && callbackData != NULL);
        
        sawCallback = true;

        ThreadPoolCallbackContext * c = new ThreadPoolCallbackContext();
        c->callback = callback;
        c->callbackData = callbackData;
        
        BOOL success = TrySubmitThreadpoolCallback(ThreadPoolCallback, c, NULL);
        VERIFY_IS_TRUE(!!success);
        
        return true;
    }
    
    bool ThreadServiceTest::FailBackgroundWorkRequest(JsBackgroundWorkItemCallback callback, void * callbackData)
    {
        VERIFY_IS_TRUE(callback != NULL && callbackData != NULL);
        
        sawCallback = true;
        
        // Always fail the request and force work in-thread
        return false;
    }

    
    void ThreadServiceTest::ThreadPoolCallback(PTP_CALLBACK_INSTANCE /* Instance */, void * Context)
    {
        ThreadPoolCallbackContext * c = (ThreadPoolCallbackContext *)Context;
        c->callback(c->callbackData);

        delete c;
    }
    
    void ThreadServiceTest::DoThreadPoolTest()
    {
        DoTest(SubmitBackgroundWorkToThreadPool);
    }

    void ThreadServiceTest::DoAlwaysDenyRequestTest()
    {
        DoTest(FailBackgroundWorkRequest);
    }
    
    void ThreadServiceTest::DoTest(JsThreadServiceCallback threadService)
    {
        sawCallback = false;
        
        VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeAllowScriptInterrupt, threadService, &this->runtime) == JsNoError);
        VERIFY_IS_TRUE(JsCreateContext(this->runtime, &this->context) == JsNoError);
        
        VERIFY_IS_TRUE(JsSetCurrentContext(this->context) == JsNoError);
        LPCWSTR script = LoadScriptFile(L"Splay.js");
        VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsNoError);
        
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
        VERIFY_IS_TRUE(JsDisposeRuntime(this->runtime) == JsNoError);

        // Ensure that at least one callback happened
        VERIFY_IS_TRUE(sawCallback);
    }
}
