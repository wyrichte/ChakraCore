//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once
#include <float.h>
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4127) // conditional expression is constant
namespace JsrtUnitTests
{
    template<JsRuntimeAttributes attributes>
    class ApiTest
    {
    public:
        ApiTest() :
        runtime(NULL)
    {
    }
        
    private:
        BEGIN_TEST_CLASS(ApiTest)
            TEST_CLASS_PROPERTY(L"Description", L"jsrt.dll C API tests")
            TEST_CLASS_PROPERTY(L"Parallel", L"true")
        END_TEST_CLASS()

        TEST_METHOD_SETUP(Setup)
        {
            // Create runtime, context and set current context
            if (!VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &this->runtime) == JsNoError))
            {
                return false;
            }

            JsValueRef context;        
            if (!VERIFY_IS_TRUE(JsCreateContext(this->runtime, &context) == JsNoError))
            {
                return false;
            }

            JsSetCurrentContext(context);
            JsValueRef setContext;
            if (!VERIFY_IS_TRUE(JsGetCurrentContext(&setContext) == JsNoError) ||
                !VERIFY_IS_TRUE(setContext == context))
            {
                return false;
            }

            return true;
        }

        TEST_METHOD_CLEANUP(Cleanup)
        {        
            if (this->runtime != NULL)
            {
                JsSetCurrentContext(NULL);
                JsDisposeRuntime(this->runtime);
            }

            return true;
        }

        TEST_METHOD(WeakReferences)
        {
            JsValueRef object;
            VERIFY_IS_TRUE(JsCreateObject(&object) == JsNoError);

            JsWeakContainerRef weakContainer;
            VERIFY_IS_TRUE(JsCreateWeakContainer(object, &weakContainer) == JsNoError);

            VERIFY_IS_TRUE(JsCollectGarbage(runtime) == JsNoError);

            bool isValid;
            VERIFY_IS_TRUE(JsIsReferenceValid(weakContainer, &isValid) == JsNoError);
            VERIFY_IS_TRUE(isValid);

            JsValueRef containedObject;
            VERIFY_IS_TRUE(JsGetReference(weakContainer, &containedObject) == JsNoError);
            VERIFY_IS_TRUE(containedObject == object);

#if 0
            // Temporarily disabling the portion of the test that relies on timely GC.
            // This is unreliable and is causing random SNAP failures.
            object = JS_INVALID_REFERENCE;
            containedObject = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsCollectGarbage(runtime) == JsNoError);

            VERIFY_IS_TRUE(JsIsReferenceValid(weakContainer, &isValid) == JsNoError);
            VERIFY_IS_TRUE(!isValid);
            VERIFY_IS_TRUE(JsGetReference(weakContainer, &containedObject) == JsNoError);
            VERIFY_IS_TRUE(containedObject == JS_INVALID_REFERENCE);
#endif
        }


        static void CALLBACK CallOnceCallback(_In_opt_ void *data)
        {
            bool* pValue = reinterpret_cast<bool*>(data);
            VERIFY_IS_FALSE(*pValue);
            *pValue = true;
        }

        TEST_METHOD(ExternalArrayBuffer)
        {
            BYTE originBuffer[] = {0x2, 0x1, 0x4, 0x3, 0x6, 0x5, 0x8, 0x7};
            const unsigned int originBufferLength = sizeof(originBuffer);

            JsValueRef externalArrayBuffer;
            JsValueType valueType;
            BYTE *buffer;
            unsigned int bufferLength;

            bool called = false;
            VERIFY_IS_TRUE(JsCreateExternalArrayBuffer(originBuffer, originBufferLength, CallOnceCallback, &called, &externalArrayBuffer) == JsNoError);

            VERIFY_IS_TRUE(JsGetValueType(externalArrayBuffer, &valueType) == JsNoError);
            VERIFY_ARE_EQUAL(JsValueType::JsArrayBuffer, valueType);
            VERIFY_IS_TRUE(JsGetArrayBufferStorage(externalArrayBuffer, &buffer, &bufferLength) == JsNoError);
            VERIFY_ARE_EQUAL(originBuffer, buffer);
            VERIFY_ARE_EQUAL(originBufferLength, bufferLength);

            VERIFY_IS_FALSE(called);
            VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
            VERIFY_IS_TRUE(called);
        }

        //
        // Object BeforeCollect callback tests
        //
        static const int OBJECTBEFORECOLLECT_GC_ITERATIONS = 5;

        // Test basic ObjectBeforeCollect callback with a normal object
        TEST_METHOD(ObjectBeforeCollect_Basic)
        {
            int called = 0;
            CreateObject([&](JsRef obj)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef)
                {
                    ++called;
                    return true; // collect
                });
            });

            VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
            VERIFY_ARE_EQUAL(1, called);
        }

        // Test basic ObjectBeforeCollect callback revive behavior
        TEST_METHOD(ObjectBeforeCollect_Basic_Revive)
        {
            int called = 0;
            CreateObject([&](JsRef obj)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef)
                {
                    return ++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS; // collect after OBJECTBEFORECOLLECT_GC_ITERATIONS
                });
            });

            for (int i = 0; i < OBJECTBEFORECOLLECT_GC_ITERATIONS; i++)
            {
                VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                VERIFY_ARE_EQUAL(i + 1, called); // called in every iteration
            }
            for (int i = 0; i < OBJECTBEFORECOLLECT_GC_ITERATIONS; i++)
            {
                VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                VERIFY_ARE_EQUAL(OBJECTBEFORECOLLECT_GC_ITERATIONS, called); // callback should no longer be called
            }
        }

        // Common test helper using an external object to verify collect behavior with finalizer
        template <class Config, class Test>
        void ObjectBeforeCollectTest(const Config& config, const Test& test)
        {
            int called = 0;
            bool finalized = false;

            CreateObject([&](JsRef ref)
            {
                config(ref, called, finalized);
            }, [&]()
            {
                finalized = true;
            });

            test(called, finalized); // This should ensure object finalize
            VERIFY_IS_TRUE(finalized);
            {
                // Further GC should have no effect on beforeCollect callback/finalizer
                const int oldCalled = called;
                finalized = false;
                for (int i = 0; i < OBJECTBEFORECOLLECT_GC_ITERATIONS; i++)
                {
                    VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                    VERIFY_ARE_EQUAL(oldCalled, called);
                    VERIFY_IS_TRUE(!finalized);
                }
            }
        }
        template <class Config>
        void ObjectBeforeCollectTest(const Config& config)
        {
            // Default "test" calls GC OBJECTBEFORECOLLECT_GC_ITERATIONS times and assumes collect in the last iteration. Shared by several tests.
            ObjectBeforeCollectTest(config, [this](int& called, bool& finalized)
            {               
                for (int i = 0; i < OBJECTBEFORECOLLECT_GC_ITERATIONS; i++)
                {
                    VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                    VERIFY_ARE_EQUAL(i + 1, called);
                    VERIFY_IS_TRUE(i < OBJECTBEFORECOLLECT_GC_ITERATIONS - 1 ? !finalized : finalized); // only finalize in the last iteration
                }
            });
        }

        // Test ObjectBeforeCollect callback with an external object, verify collect behavior through finalizer
        TEST_METHOD(ObjectBeforeCollect_Revive_0)
        {
            ObjectBeforeCollectTest([](JsRef obj, int& called, bool& finalized)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef ref)
                {
                    return ++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS; // collect after OBJECTBEFORECOLLECT_GC_ITERATIONS
                });
            });
        }

        // Test revive followed by clearing callback
        TEST_METHOD(ObjectBeforeCollect_Revive_1)
        {
            ObjectBeforeCollectTest([](JsRef obj, int& called, bool& finalized)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef ref)
                {
                    if (++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS)
                    {
                        VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(ref, nullptr, nullptr) == JsNoError); // Clear callback in OBJECTBEFORECOLLECT_GC_ITERATIONS call. Should behave as NoOp.
                        return true; // collect after OBJECTBEFORECOLLECT_GC_ITERATIONS
                    }
                    return false;
                });
            });
        }

        template <class Config>
        void ObjectBeforeCollect_Clear_Common(const Config& config)
        {
            ObjectBeforeCollectTest(config, [this](int& called, bool& finalized)
            {
                // Suppose no weak callback registered at all. Collect in one GC.
                VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                VERIFY_ARE_EQUAL(0, called); // no callback invoked
                VERIFY_IS_TRUE(finalized);
            });
        }
        // Register null callback is no-op
        TEST_METHOD(ObjectBeforeCollect_Clear_0)
        {
            ObjectBeforeCollect_Clear_Common([](JsValueRef obj, int& called, bool& finalized)
            {
                VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(obj, &called, nullptr) == JsNoError); // Clear callback
            });
        }
        // Register a callback then clear it is no-op
        TEST_METHOD(ObjectBeforeCollect_Clear_1)
        {
            ObjectBeforeCollect_Clear_Common([](JsValueRef obj, int& called, bool& finalized)
            {
                VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(obj, &called, objectBeforeCollectCallback_Revive) == JsNoError); // Set callback
                VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(obj, &called, nullptr) == JsNoError); // Clear callback
            });
        }
        static void CALLBACK objectBeforeCollectCallback_Revive(_In_ JsRef ref, _In_opt_ void *callbackState)
        {
            if (++*reinterpret_cast<int*>(callbackState) < OBJECTBEFORECOLLECT_GC_ITERATIONS)
            {
                VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(ref, callbackState, objectBeforeCollectCallback_Revive) == JsNoError); // Revive
            }
        }

        // Callback calls JsSetContext
        TEST_METHOD(ObjectBeforeCollect_JsSetCurrentContextNull)
        {
            int called = 0;
            CreateObject([&](JsRef obj)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef)
                {
                    VERIFY_IS_TRUE(JsSetCurrentContext(nullptr) == JsNoError);
                    ++called;
                    return true; // collect
                });
            });

            VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
            VERIFY_ARE_EQUAL(1, called);
        }

        // Callback calls JsSetContext
        TEST_METHOD(ObjectBeforeCollect_JsSetCurrentContext)
        {
            int called = 0;
            CreateObject([&](JsRef obj)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef ref)
                {
                    JsContextRef testContext;
                    VERIFY_IS_TRUE(JsGetContextOfObject(ref, &testContext) == JsNoError);
                    VERIFY_IS_TRUE(JsSetCurrentContext(testContext) == JsNoError);
                    ++called;
                    return true; // collect
                });
            });
            VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
            VERIFY_ARE_EQUAL(1, called);
        }

        // Mix with JsAddRef/JsRelease
        TEST_METHOD(ObjectBeforeCollect_AddRelease_0)
        {
            ObjectBeforeCollectTest([this](JsRef obj, int& called, bool& finalized)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef)
                {
                    return ++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS; // collect after OBJECTBEFORECOLLECT_GC_ITERATIONS
                });

                VERIFY_IS_TRUE(JsAddRef(obj, nullptr) == JsNoError); // explicit ref

                for (int i = 0; i < OBJECTBEFORECOLLECT_GC_ITERATIONS; i++) {
                    VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                    VERIFY_ARE_EQUAL(0, called); // should not be called, since we have explicit ref
                    VERIFY_IS_TRUE(!finalized); // should not be finalized, since we have explicit ref
                }

                VERIFY_IS_TRUE(JsRelease(obj, nullptr) == JsNoError); // release explicit ref
            });
        }

        // Callback calls JsAddRef
        TEST_METHOD(ObjectBeforeCollect_AddRelease_1)
        {
            ObjectBeforeCollectTest([this](JsRef obj, int& called, bool& finalized)
            {
                JsValueRef data = JS_INVALID_REFERENCE;
                VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(obj, &data, objectBeforeCollectCallback_AddRelease_1) == JsNoError);
                obj = nullptr;

                VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
                VERIFY_IS_TRUE(data != JS_INVALID_REFERENCE); // data now holds ref
                VERIFY_IS_TRUE(!finalized);

                SetObjectBeforeCollectCallback(data, [&](JsRef) { return ++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS; /*collect after OBJECTBEFORECOLLECT_GC_ITERATIONS*/ });
                VERIFY_IS_TRUE(JsRelease(data, nullptr) == JsNoError); // Release ref added by callback
                data = nullptr;
            });
        }
        static void CALLBACK objectBeforeCollectCallback_AddRelease_1(_In_ JsRef ref, _In_opt_ void *callbackState)
        {
            VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(ref, callbackState, objectBeforeCollectCallback_AddRelease_1) == JsNoError); // Dummy, revive
            VERIFY_IS_TRUE(JsAddRef(ref, nullptr) == JsNoError); // AddRef
            *reinterpret_cast<JsRef*>(callbackState) = ref; // Return the ref
        }

        // Callback accesses external data
        TEST_METHOD(ObjectBeforeCollect_ExternalData)
        {
            void* oldData = nullptr;
            char* someData = "some data";

            ObjectBeforeCollectTest([&](JsRef obj, int& called, bool& finalized)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef ref)
                {
                    // We have existing external data (our finalizer copy from CreateObject).
                    // Save it and change to "someData". Restore it before final collect.
                    bool hasExternalData;
                    void* data = nullptr;
                    VERIFY_IS_TRUE(JsHasExternalData(ref, &hasExternalData) == JsNoError);
                    VERIFY_IS_TRUE(hasExternalData);
                    VERIFY_IS_TRUE(JsGetExternalData(ref, &data) == JsNoError && data != nullptr);

                    if (oldData == nullptr)
                    {
                        oldData = data;
                        VERIFY_IS_TRUE(JsSetExternalData(ref, someData) == JsNoError);
                    }
                    else
                    {
                        VERIFY_IS_TRUE(data == someData);
                    }

                    if (++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS) // collect after OBJECTBEFORECOLLECT_GC_ITERATIONS
                    {
                        VERIFY_IS_TRUE(JsSetExternalData(ref, oldData) == JsNoError); // restore oldData (our finalizer copy)
                        return true;
                    }
                    return false;
                });
            });
        }

        // Callback calls unsupported API
        TEST_METHOD(ObjectBeforeCollect_UnsupportedOperation)
        {
            ObjectBeforeCollectTest([](JsRef obj, int& called, bool& finalized)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef ref)
                {
                    JsValueRef value;
                    VERIFY_IS_TRUE(JsCreateObject(&value) == JsErrorInObjectBeforeCollectCallback);
                    return ++called >= OBJECTBEFORECOLLECT_GC_ITERATIONS; // collect after OBJECTBEFORECOLLECT_GC_ITERATIONS
                });
            });
        }

        // Test shutdown behavior
        TEST_METHOD(ObjectBeforeCollect_ResetContextInFinalizer)
        {
            int called = 0;
            bool finalized = false;
            CreateObject([&](JsRef obj)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef)
                {
                    ++called;
                    return true;
                });
            }, [&]()
            {
                finalized = true;
                JsSetCurrentContext(nullptr);
            });
            // manually shutdown
            JsSetCurrentContext(NULL);
            JsDisposeRuntime(this->runtime);
            this->runtime = nullptr;

            VERIFY_ARE_EQUAL(1, called);
            VERIFY_IS_TRUE(finalized);
        }

        // Test shutdown behavior
        TEST_METHOD(ObjectBeforeCollect_Shutdown)
        {
            int called = 0;
            bool finalized = false;
            CreateObject([&](JsRef obj)
            {
                SetObjectBeforeCollectCallback(obj, [&](JsRef)
                {
                    ++called;
                    return true;
                });
            }, [&]()
            {
                finalized = true;
            });

            // manually shutdown
            JsSetCurrentContext(NULL);
            JsDisposeRuntime(this->runtime);
            this->runtime = nullptr;

            VERIFY_ARE_EQUAL(1, called);
            VERIFY_IS_TRUE(finalized);
        }

        // Verify that JsContext doesn't get collected prematurely
        // as it is pinned to javascriptLibrary.
        TEST_METHOD(ObjectBeforeCollect_Context)
        {
            int called = 0;
            {
                // Set callback on current context
                JsValueRef context;
                VERIFY_IS_TRUE(JsGetCurrentContext(&context) == JsNoError);
                SetObjectBeforeCollectCallback(context, [&](JsRef)
                {
                    ++called;
                    return true;
                });
            }

            JsSetCurrentContext(NULL); // Release current context
            VERIFY_IS_TRUE(JsPrivateCollectGarbageSkipStack(this->runtime) == JsNoError);
            VERIFY_ARE_EQUAL(1, called);
        }

        // Create a JS object and configure it.
        template <class Config>
        static void CreateObject(const Config& config)
        {
            JsValueRef obj;
            VERIFY_IS_TRUE(JsCreateObject(&obj) == JsNoError);
            config(obj);
        }
        // Create an external object with finalizer and configure it.
        template <class Config, class Finalize>
        static void CreateObject(const Config& config, const Finalize& finalize)
        {
            JsValueRef obj;
            VERIFY_IS_TRUE(JsCreateExternalObject(new Finalize(finalize), FinalizeCallback<Finalize>, &obj) == JsNoError); // Create a _new_ copy of "finalize" as callbackState
            config(obj);
        }
        template <class Fn>
        static void SetObjectBeforeCollectCallback(JsRef obj, const Fn& f)
        {
            VERIFY_IS_TRUE(JsSetObjectBeforeCollectCallback(obj, new Fn(f), ObjectBeforeCollectCallback<Fn>) == JsNoError); // Create a _new_ copy of "f" as callbackState
        }
        // Hook lambda to callback
        template <class Fn>
        static void CALLBACK ObjectBeforeCollectCallback(_In_ JsRef ref, _In_opt_ void *callbackState)
        {
            CallAndDelete<Fn>(callbackState, [ref](const Fn& f)
            {
                if (!f(ref)) // true to collect, false to revive
                {
                    SetObjectBeforeCollectCallback(ref, f);
                }
            });
        }
        template <class Fn>
        static void CALLBACK FinalizeCallback(_In_opt_ void *data)
        {
            CallAndDelete<Fn>(data);
        }
        template <class Fn, class Call>
        static void CallAndDelete(void *callbackState, const Call& call)
        {
            Fn* f = reinterpret_cast<Fn*>(callbackState);
            call(*f);
            delete f;
        }
        template <class Fn>
        static void CallAndDelete(void *callbackState)
        {
            CallAndDelete<Fn>(callbackState, [](const Fn& f) { f(); });
        }

        TEST_METHOD(SetIndexedPropertiesToExternalData)
        {
            auto verifyIndexedPropertiesEqual = [](JsValueRef a, JsValueRef b, unsigned int len)
            {
                for (unsigned int i = 0; i < len; i++)
                {
                    JsValueRef index, itemA, itemB;
                    VERIFY_IS_TRUE(JsIntToNumber(i, &index) == JsNoError);
                    VERIFY_IS_TRUE(JsGetIndexedProperty(a, index, &itemA) == JsNoError);
                    VERIFY_IS_TRUE(JsGetIndexedProperty(b, index, &itemB) == JsNoError);

                    bool equals;
                    VERIFY_IS_TRUE(JsStrictEquals(itemA, itemB, &equals) == JsNoError);
                    VERIFY_IS_TRUE(equals || (IsNaN(itemA) && IsNaN(itemB)));
                }
            };

            JsValueRef baseArray;
            size_t length;
            VERIFY_IS_TRUE(JsRunScript(L"[-12.3, -500, -256, -255, -1, 0, 1, 200, 255, 256, 500, 1.2, false, new Boolean(true), NaN, 'String', {}]",
                JS_SOURCE_CONTEXT_NONE, L"", &baseArray) == JsNoError);
            VERIFY_IS_TRUE(JsGetArrayLength(baseArray, &length) == JsNoError);
            
            unsigned int uiLength = static_cast<unsigned int>(length);
            VERIFY_ARE_EQUAL(length, uiLength);

            const JsTypedArrayType types[] =
            {
                JsArrayTypeInt8,
                JsArrayTypeUint8,
                JsArrayTypeUint8Clamped,
                JsArrayTypeInt16,
                JsArrayTypeUint16,
                JsArrayTypeInt32,
                JsArrayTypeUint32,
                JsArrayTypeFloat32,
                JsArrayTypeFloat64
            };

            for (int i = 0; i < _countof(types); i++)
            {
                const JsTypedArrayType type = types[i];

                JsValueRef typedArray;
                VERIFY_IS_TRUE(JsCreateTypedArray(type, baseArray, 0, 0, &typedArray) == JsNoError);

                BYTE* buffer;
                unsigned int bufferLength;
                VERIFY_IS_TRUE(JsGetTypedArrayStorage(typedArray, &buffer, &bufferLength, nullptr, nullptr) == JsNoError);

                JsValueRef obj;
                VERIFY_IS_TRUE(JsCreateObject(&obj) == JsNoError);
                VERIFY_IS_TRUE(JsSetIndexedPropertiesToExternalData(obj, buffer, type, uiLength) == JsNoError);

                bool tmpHas;
                VERIFY_IS_TRUE(JsHasIndexedPropertiesExternalData(obj, &tmpHas) == JsNoError);
                VERIFY_IS_TRUE(tmpHas);

                void* tmpData;
                JsTypedArrayType tmpType;
                unsigned int tmpLength;
                VERIFY_IS_TRUE(JsGetIndexedPropertiesExternalData(obj, &tmpData, &tmpType, &tmpLength) == JsNoError);
                VERIFY_ARE_EQUAL(static_cast<void*>(buffer), tmpData);
                VERIFY_ARE_EQUAL(type, tmpType);
                VERIFY_ARE_EQUAL(uiLength, tmpLength);

                verifyIndexedPropertiesEqual(typedArray, obj, uiLength);

                for (unsigned int k = 0; k < uiLength; k++)
                {
                    JsValueRef index, value;
                    VERIFY_IS_TRUE(JsIntToNumber(k, &index) == JsNoError);
                    VERIFY_IS_TRUE(JsIntToNumber(k * 100, &value) == JsNoError);
                    VERIFY_IS_TRUE(JsSetIndexedProperty(obj, index, value) == JsNoError);
                }
                verifyIndexedPropertiesEqual(typedArray, obj, uiLength);
            }
        }

    private:

        static void FinalizeCallback(JsValueRef object)
        {
            VERIFY_IS_TRUE(object != JS_INVALID_REFERENCE);
        }
        
        static void OldFinalizeCallback(void *data)
        {
            VERIFY_IS_TRUE(data == nullptr);
        }

        static JsValueRef ExternalFunctionCallbackForFpcw(JsValueRef /* function */, bool /* isConstructCall */, JsValueRef * /* args */, USHORT /* cargs */, void * /* callbackState */)
        {
#if _M_IX86
            unsigned int i = 0;
            _control87(i, _MCW_EM);
#else
            unsigned int i = 0;
            _controlfp_s(0, i, _MCW_EM);
#endif
            return NULL;
        }

        static JsValueRef ExternalFunctionCallback(JsValueRef /* function */, bool /* isConstructCall */, JsValueRef * /* args */, USHORT /* cargs */, void * /* callbackState */)
        {        
            return NULL;
        }

        static JsValueRef ExternalFunctionPreScriptAbortionCallback(JsValueRef /* function */, bool /* isConstructCall */, JsValueRef * args /* args */, USHORT /* cargs */, void * /* callbackState */)
        {
            JsValueRef result;
            const wchar_t *scriptText;
            size_t scriptTextLen;

            VERIFY_IS_TRUE(JsStringToPointer(args[0], &scriptText, &scriptTextLen) == JsNoError);
            VERIFY_IS_TRUE(JsRunScript(scriptText, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsErrorScriptTerminated);
            return NULL;
        }

        static JsValueRef ExternalFunctionPostScriptAbortionCallback(JsValueRef /* function */, bool /* isConstructCall */, JsValueRef * args /* args */, USHORT /* cargs */, void * /* callbackState */)
        {
            JsValueRef result;
            const wchar_t *scriptText;
            size_t scriptTextLen;

            VERIFY_IS_TRUE(JsStringToPointer(args[0], &scriptText, &scriptTextLen) == JsNoError);
            VERIFY_IS_TRUE(JsRunScript(scriptText, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsErrorInDisabledState);
            return NULL;
        }

        static bool IsNaN(JsValueRef a)
        {
            bool equals;
            VERIFY_IS_TRUE(JsStrictEquals(a, a, &equals) == JsNoError);
            return !equals;
        };

    private:
        JsRuntimeHandle runtime;
    };
}
