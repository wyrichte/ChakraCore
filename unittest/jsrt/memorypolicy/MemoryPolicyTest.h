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

        TEST_METHOD(ContextLeak)
        {
            JsRuntimeHandle jsRuntime = JS_INVALID_RUNTIME_HANDLE;
            this->totalAllocationSize = 0;
            JsErrorCode errorCode = ::JsCreateRuntime(
                (JsRuntimeAttributes)(JsRuntimeAttributeAllowScriptInterrupt | JsRuntimeAttributeDisableEval | JsRuntimeAttributeDisableNativeCodeGeneration),
                nullptr,
                &jsRuntime);
            VERIFY_IS_TRUE(errorCode == JsNoError);
            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(jsRuntime, this, MemoryPolicyTest::MemoryAllocationCallback) == JsNoError);
            for (ULONG i = 0; i < 1000; ++i)
            {
                JsContextRef jsContext = JS_INVALID_REFERENCE;
                errorCode = ::JsCreateContext(jsRuntime, &jsContext);
                VERIFY_IS_TRUE(errorCode == JsNoError);
                errorCode = ::JsSetCurrentContext(jsContext);
                VERIFY_IS_TRUE(errorCode == JsNoError);
                errorCode = ::JsSetCurrentContext(JS_INVALID_REFERENCE);
                VERIFY_IS_TRUE(errorCode == JsNoError);
                if (((ULONG)(i % 100)) * 100 == 0)
                {
                    JsCollectGarbage(jsRuntime);
                }
            }
            VERIFY_IS_TRUE(JsDisposeRuntime(jsRuntime) == JsNoError);
        }

        TEST_METHOD(ContextLeak1)
        {
            JsRuntimeHandle jsRuntime = JS_INVALID_RUNTIME_HANDLE;
            this->totalAllocationSize = 0;
            JsErrorCode errorCode = ::JsCreateRuntime(
                (JsRuntimeAttributes)(JsRuntimeAttributeAllowScriptInterrupt | JsRuntimeAttributeDisableEval | JsRuntimeAttributeDisableNativeCodeGeneration),
                nullptr,
                &jsRuntime);
            VERIFY_IS_TRUE(errorCode == JsNoError);
            VERIFY_IS_TRUE(JsSetRuntimeMemoryAllocationCallback(jsRuntime, this, MemoryPolicyTest::MemoryAllocationCallback) == JsNoError);
            for (ULONG i = 0; i < 1000; ++i)
            {
                JsContextRef jsContext = JS_INVALID_REFERENCE;
                JsContextRef jsContext1 = JS_INVALID_REFERENCE;
                JsValueRef jsVal1 = JS_INVALID_REFERENCE, jsGO2 = JS_INVALID_REFERENCE;
                JsPropertyIdRef propertyId;
                VERIFY_IS_TRUE(::JsCreateContext(jsRuntime, &jsContext) == JsNoError);
                VERIFY_IS_TRUE(::JsSetCurrentContext(jsContext) == JsNoError);
                VERIFY_IS_TRUE(::JsCreateObject(&jsVal1) == JsNoError);
                VERIFY_IS_TRUE(::JsGetPropertyIdFromName(L"test", &propertyId) == JsNoError);
                VERIFY_IS_TRUE(::JsCreateContext(jsRuntime, &jsContext1) == JsNoError);
                VERIFY_IS_TRUE(::JsSetCurrentContext(jsContext1) == JsNoError);
                VERIFY_IS_TRUE(::JsGetGlobalObject(&jsGO2) == JsNoError);
                VERIFY_IS_TRUE(::JsSetProperty(jsGO2, propertyId, jsVal1, true) == JsNoError);
                VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
                if (((ULONG)(i % 100)) * 100 == 0)
                {
                    jsVal1 = JS_INVALID_REFERENCE;
                    jsGO2 = JS_INVALID_REFERENCE;
                    jsContext = JS_INVALID_REFERENCE;
                    jsContext1 = JS_INVALID_REFERENCE;
                    JsCollectGarbage(jsRuntime);
                }
            }
            VERIFY_IS_TRUE(JsDisposeRuntime(jsRuntime) == JsNoError);
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
