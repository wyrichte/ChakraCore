//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{    
    template<JsRuntimeAttributes attributes>
    class MemoryPolicyTest
    {
    private:
        BEGIN_TEST_CLASS(MemoryPolicyTest)
            TEST_CLASS_PROPERTY(L"Description", L"memory policy test")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()

        TEST_METHOD(BasicTest)
        {
            WCHAR* fileNames[] = {L"UnboundedMemory.js", L"arrayTest.js", L"arraybuffer.js"};
            for (int i = 0; i < sizeof(fileNames)/sizeof(WCHAR*); i++)
            {
                LPCWSTR script = LoadScriptFile(fileNames[i]);

                // Create the runtime
                JsRuntimeHandle runtime;
                VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &runtime) == JsNoError);

                // Set memory limit
                VERIFY_IS_TRUE(JsSetRuntimeMemoryLimit(runtime, MemoryLimit) == JsNoError);

                size_t memoryLimit;
                size_t memoryUsage;

                VERIFY_IS_TRUE(JsGetRuntimeMemoryLimit(runtime, &memoryLimit) == JsNoError);
                VERIFY_IS_TRUE(memoryLimit == MemoryLimit);
                VERIFY_IS_TRUE(JsGetRuntimeMemoryUsage(runtime, &memoryUsage) == JsNoError);
                VERIFY_IS_TRUE(memoryUsage < MemoryLimit);

                // Create and initialize the script context
                JsContextRef context;
                VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
                VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);

                // Invoke the script
                VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsErrorScriptException);
                ValidateOOMException();
               

                VERIFY_IS_TRUE(JsGetRuntimeMemoryLimit(runtime, &memoryLimit) == JsNoError);
                VERIFY_IS_TRUE(memoryLimit == MemoryLimit);
                VERIFY_IS_TRUE(JsGetRuntimeMemoryUsage(runtime, &memoryUsage) == JsNoError);
                VERIFY_IS_TRUE(memoryUsage <= MemoryLimit);
                // first case (UnboundedMemory) have unlimited growth; we can have test property flag later if needed.
                if (i != 0)
                {
                    VERIFY_IS_TRUE(JsSetRuntimeMemoryLimit(runtime, 0xffffffff) == JsNoError);
                    VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsNoError);
                }

                // Destroy the runtime
                VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
                VERIFY_IS_TRUE(JsCollectGarbage(runtime) == JsNoError);
                VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);
            }
        }

        TEST_METHOD(MemoryCallbackTest)
        {
            LPCWSTR script = LoadScriptFile(L"UnboundedMemory.js");
            // Create the runtime
            JsRuntimeHandle runtime;
            VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &runtime) == JsNoError);

            // Set memory limit
            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(runtime, this, MemoryPolicyTest::MemoryAllocationCallback) == JsNoError);

            size_t memoryUsage;

            VERIFY_IS_TRUE(JsGetRuntimeMemoryUsage(runtime, &memoryUsage) == JsNoError);
            VERIFY_IS_TRUE(memoryUsage < MemoryLimit);
            this->totalAllocationSize = memoryUsage;

            // Create and initialize the script context
            JsContextRef context;
            VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);

            // Invoke the script
            VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsErrorScriptException);
            ValidateOOMException();

            VERIFY_IS_TRUE(JsGetRuntimeMemoryUsage(runtime, &memoryUsage) == JsNoError);
            VERIFY_IS_TRUE(memoryUsage <= MemoryLimit);

            // Destroy the runtime
            VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
            VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);
        }

        TEST_METHOD(OOSTest) {
          JsPropertyIdRef property;
          JsValueRef value, exception;
          LPCWSTR str;
          size_t length;

          LPCWSTR script = LoadScriptFile(L"oos.js");
          // Create the runtime
          JsRuntimeHandle runtime;
          VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &runtime) == JsNoError);

          // Create and initialize the script context
          JsContextRef context;
          VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
          VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);

          // Invoke the script
          VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsErrorScriptException);

          // Verify we got OOS
          VERIFY_IS_TRUE(JsGetAndClearException(&exception) == JsNoError);
          VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"message", &property) == JsNoError);
          VERIFY_IS_TRUE(JsGetProperty(exception, property, &value) == JsNoError);
          VERIFY_IS_TRUE(JsStringToPointer(value, &str, &length) == JsNoError);
          VERIFY_ARE_EQUAL(String(L"Out of stack space"), str);

          // Destroy the runtime
          VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
          VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);
        }

        void ValidateOOMException()
        {
            JsValueRef exception;
            JsErrorCode errorCode = JsGetAndClearException(&exception);
            if (errorCode == JsNoError)
            {
                JsPropertyIdRef property;
                JsValueRef value;
                LPCWSTR str;
                size_t length;

                VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"message", &property) == JsNoError);
                VERIFY_IS_TRUE(JsGetProperty(exception, property, &value) == JsNoError);
                VERIFY_IS_TRUE(JsStringToPointer(value, &str, &length) == JsNoError);
                VERIFY_ARE_EQUAL(String(L"Out of memory"), str);
            }
            else
            {
                // If we don't have enough memory to create error object, then GetAndClearException might 
                // fail and return ErrorInvalidArgument. Check if we don't get any other error code.
                VERIFY_IS_TRUE(errorCode == JsErrorInvalidArgument);
            }
        }

        TEST_METHOD(NegativeTest)
        {
            this->totalAllocationSize = 0;
            // Create the runtime
            JsRuntimeHandle runtime;
            VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &runtime) == JsNoError);

            // after setting callback, you can set limit
            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(runtime, this, MemoryPolicyTest::MemoryAllocationCallback) == JsNoError);
            VERIFY_IS_TRUE(JsSetRuntimeMemoryLimit(runtime, MemoryLimit) == JsNoError);

            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(runtime, NULL, NULL) == JsNoError);
            // now we can set limit
            VERIFY_IS_TRUE(JsSetRuntimeMemoryLimit(runtime, MemoryLimit) == JsNoError);
            // and we can set the callback again. 
            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(runtime, this, MemoryPolicyTest::MemoryAllocationCallback) == JsNoError);

            VERIFY_IS_TRUE(JsSetRuntimeMemoryLimit(runtime, (size_t)-1) == JsNoError);
            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(runtime, this, MemoryPolicyTest::MemoryAllocationCallback) == JsNoError);

            // Destroy the runtime
            VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
            VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);
        }

        static const size_t MemoryLimit = 10 * 1024 * 1024;
        static bool __stdcall MemoryAllocationCallback(LPVOID context, JsMemoryEventType allocationEvent, size_t allocationSize)
        {
            VERIFY_IS_TRUE(context != NULL);
            MemoryPolicyTest* memoryPolicyTest = static_cast<MemoryPolicyTest*>(context);
            switch (allocationEvent)
            {
            case JsMemoryAllocate:
                memoryPolicyTest->totalAllocationSize += allocationSize;
                if (memoryPolicyTest->totalAllocationSize > MemoryPolicyTest::MemoryLimit)
                {
                    return FALSE;
                }
                return TRUE;
                break;
            case JsMemoryFree:
                memoryPolicyTest->totalAllocationSize -= allocationSize;
                break;
            case JsMemoryFailure:
                memoryPolicyTest->totalAllocationSize -= allocationSize;

                break;
            default:
                VERIFY_IS_TRUE(FALSE);
            }
            return TRUE;
        }

        size_t totalAllocationSize;
    };
}
