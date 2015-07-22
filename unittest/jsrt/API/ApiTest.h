//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#pragma once
#include <float.h>
namespace JsrtUnitTests
{
    template<JsRuntimeAttributes attributes>
    class ApiTest
    {
    public:
        ApiTest() :
        runtime(NULL),
        hMonitor(NULL),
        threadHandle(NULL),
        isScriptActive(FALSE)
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

        TEST_METHOD(ValidateArgumentsJsIdle)
        {
            VERIFY_IS_TRUE(JsIdle(nullptr) == JsErrorNullArgument);
        }

        TEST_METHOD(ReferenceCountingTest)
        {
            JsContextRef context;

            VERIFY_IS_TRUE(JsGetCurrentContext(&context) == JsNoError);

            VERIFY_IS_TRUE(JsAddRef(context, nullptr) == JsNoError);
            VERIFY_IS_TRUE(JsRelease(context, nullptr) == JsNoError);

            JsValueRef undefined;

            VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);

            VERIFY_IS_TRUE(JsSetCurrentContext(nullptr) == JsNoError);
            VERIFY_IS_TRUE(JsAddRef(undefined, nullptr) == JsErrorNoCurrentContext);
            VERIFY_IS_TRUE(JsRelease(undefined, nullptr) == JsErrorNoCurrentContext);

            VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);
            VERIFY_IS_TRUE(JsAddRef(undefined, nullptr) == JsNoError);
            VERIFY_IS_TRUE(JsRelease(undefined, nullptr) == JsNoError);

            JsPropertyIdRef foo;

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"foo", &foo) == JsNoError);
            VERIFY_IS_TRUE(JsAddRef(foo, nullptr) == JsNoError);
            VERIFY_IS_TRUE(JsRelease(foo, nullptr) == JsNoError);
        }

        TEST_METHOD(ObjectsAndPropertiesTest1)
        {                      
            // Create an object and set some own properties on it
            JsValueRef object;        
            VERIFY_IS_TRUE(JsCreateObject(&object) == JsNoError);

            JsPropertyIdRef name1;
            const wchar_t* name;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"stringProperty1", &name1) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyNameFromId(name1, &name) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(name, L"stringProperty1"));

            JsPropertyIdType propertyIdType;
            VERIFY_IS_TRUE(JsGetPropertyIdType(name1, &propertyIdType) == JsNoError);
            VERIFY_IS_TRUE(propertyIdType == JsPropertyIdTypeString);

            JsPropertyIdRef name2;        
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"stringProperty2", &name2) == JsNoError);

            JsValueRef value1;        
            VERIFY_IS_TRUE(JsPointerToString(L"value1", wcslen(L"value1"), &value1) == JsNoError);

            JsValueRef value2;        
            VERIFY_IS_TRUE(JsPointerToString(L"value1", wcslen(L"value1"), &value2) == JsNoError);

            VERIFY_IS_TRUE(JsSetProperty(object, name1, value1, true) == JsNoError);        
            VERIFY_IS_TRUE(JsSetProperty(object, name2, value2, true) == JsNoError);

            JsValueRef value1Check;        
            VERIFY_IS_TRUE(JsGetProperty(object, name1, &value1Check) == JsNoError);
            VERIFY_IS_TRUE(value1 == value1Check);

            JsValueRef value2Check;        
            VERIFY_IS_TRUE(JsGetProperty(object, name2, &value2Check) == JsNoError);
            VERIFY_IS_TRUE(value1 == value1Check);        
        }

        TEST_METHOD(ObjectsAndPropertiesTest2)
        {        
            // Run a script to setup some globals
            LPCWSTR script;
            JsValueRef function;

            VERIFY_IS_TRUE((script = LoadScriptFile(L"ObjectsAndProperties2.js")) != NULL);
            VERIFY_IS_TRUE(JsParseScript(script, JS_SOURCE_CONTEXT_NONE, L"", &function) == JsNoError);
            VERIFY_IS_TRUE(JsCallFunction(function, nullptr, 0, nullptr) == JsNoError);

            // Get proto properties
            JsValueRef circle;        
            VERIFY_IS_TRUE(JsRunScript(L"new Circle()", JS_SOURCE_CONTEXT_NONE, L"", &circle) == JsNoError);

            JsPropertyIdRef name;        
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"color", &name) == JsNoError);

            JsValueRef value;        
            VERIFY_IS_TRUE(JsGetProperty(circle, name, &value) == JsNoError);

            JsValueRef asString;        
            VERIFY_IS_TRUE(JsConvertValueToString(value, &asString) == JsNoError);

            LPCWSTR str;
            size_t length;
            VERIFY_IS_TRUE(JsStringToPointer(asString, &str, &length) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(str, L"white"));
        }

        TEST_METHOD(CrossContextSetPropertyTest)
        {
            bool hasExternalData;
            JsContextRef oldContext, secondContext, testContext;
            JsValueRef secondValueRef, secondObjectRef, jsrtExternalObjectRef, mainObjectRef;
            JsPropertyIdRef idRef;
            JsValueRef indexRef;
            VERIFY_IS_TRUE(JsGetCurrentContext(&oldContext) == JsNoError);
            VERIFY_IS_TRUE(JsCreateObject(&mainObjectRef) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"prop1", &idRef) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(this->runtime, &secondContext) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(secondContext) == JsNoError);
            VERIFY_IS_TRUE(JsCreateObject(&secondObjectRef) == JsNoError);

            // Verify the main object is from first context
            VERIFY_IS_TRUE(JsGetContextOfObject(mainObjectRef, &testContext) == JsNoError);
            VERIFY_IS_TRUE(testContext == oldContext);

            // Create external Object in 2nd context which will be accessed in main context.
            VERIFY_IS_TRUE(JsCreateExternalObject((void *)0xdeadbeef, ExternalObjectFinalizeCallback, &jsrtExternalObjectRef) == JsNoError);
            VERIFY_IS_TRUE(JsIntToNumber(1, &secondValueRef) == JsNoError);
            VERIFY_IS_TRUE(JsIntToNumber(2, &indexRef) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(oldContext) == JsNoError);

            // Verify the second object is from second context
            VERIFY_IS_TRUE(JsGetContextOfObject(secondObjectRef, &testContext) == JsNoError);
            VERIFY_IS_TRUE(testContext == secondContext);

            VERIFY_IS_TRUE(JsSetProperty(mainObjectRef, idRef, secondValueRef, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(mainObjectRef, idRef, secondObjectRef, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetIndexedProperty(mainObjectRef, indexRef, secondValueRef) == JsNoError);
            VERIFY_IS_TRUE(JsSetIndexedProperty(mainObjectRef, indexRef, secondObjectRef) == JsNoError);
            VERIFY_IS_TRUE(JsSetPrototype(jsrtExternalObjectRef, mainObjectRef) == JsNoError);
            VERIFY_IS_TRUE(JsHasExternalData(jsrtExternalObjectRef, &hasExternalData) == JsNoError);
            VERIFY_IS_TRUE(hasExternalData);
        }

        TEST_METHOD(CrossContextFunctionCall) 
        {
            /*
            1. function changeFoo() { foo = 100 }
            2. CreateContext
            3. Set f : changeFoo in newContext
            4. Call f() from newContext
            */
            JsContextRef oldContext, secondContext;
            JsValueRef functionRef, functionResultRef, globalRef, globalNewCtxRef, valueRef;
            JsPropertyIdRef propertyIdFRef, propertyIdFooRef, propertyIdChangeFooRef;
            int answer;

            VERIFY_IS_TRUE(JsGetCurrentContext(&oldContext) == JsNoError);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"f", &propertyIdFRef) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"foo", &propertyIdFooRef) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"changeFoo", &propertyIdChangeFooRef) == JsNoError);

            //1. function changeFoo() { foo = 100 }
            VERIFY_IS_TRUE(JsRunScript(L"foo = 3; function changeFoo() { foo = 100 }", JS_SOURCE_CONTEXT_NONE, L"", &functionResultRef) == JsNoError);

            VERIFY_IS_TRUE(JsGetGlobalObject(&globalRef) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(globalRef, propertyIdChangeFooRef, &functionRef) == JsNoError);

            //2. CreateContext
            VERIFY_IS_TRUE(JsCreateContext(this->runtime, &secondContext) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(secondContext) == JsNoError);

            //3. Set f : changeFoo in newContext
            VERIFY_IS_TRUE(JsGetGlobalObject(&globalNewCtxRef) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(globalNewCtxRef, propertyIdFRef, functionRef, false) == JsNoError);

            //4. Call 'f()' from newContext
            VERIFY_IS_TRUE(JsRunScript(L"f()", JS_SOURCE_CONTEXT_NONE, L"", &functionResultRef) == JsNoError);

            //5. Change context to oldContext
            VERIFY_IS_TRUE(JsSetCurrentContext(oldContext) == JsNoError);

            //6. Verify foo == 100
            VERIFY_IS_TRUE(JsGetProperty(globalRef, propertyIdFooRef, &valueRef) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToInt(valueRef, &answer) == JsNoError);
            VERIFY_IS_TRUE(answer == 100);
        }

        TEST_METHOD(ExternalDataOnJsrtContextTest)
        {
            int i = 5;
            void *j;
            JsContextRef currentContext;

            VERIFY_IS_TRUE(JsGetCurrentContext(&currentContext) == JsNoError);
            VERIFY_IS_TRUE(JsSetContextData(currentContext, &i) == JsNoError);
            VERIFY_IS_TRUE(JsGetContextData(currentContext, &j) == JsNoError);
            VERIFY_IS_TRUE(static_cast<int*>(j) == &i);
        }

        static void ExternalObjectFinalizeCallback(void *data)
        {
            VERIFY_IS_TRUE(data == (void *)0xdeadbeef);
        }

        TEST_METHOD(ExternalObjectDataTest)
        {
            JsValueRef object;
            bool hasExternalData;
            JsValueRef protoObject;

            VERIFY_IS_TRUE(JsCreateObject(&protoObject) == JsNoError);
            VERIFY_IS_TRUE(JsCreateExternalObject((void *)0xdeadbeef, ExternalObjectFinalizeCallback, &object) == JsNoError);
            VERIFY_IS_TRUE(JsSetPrototype(object, protoObject) == JsNoError);
            VERIFY_IS_TRUE(JsHasExternalData(object, &hasExternalData) == JsNoError);
            VERIFY_IS_TRUE(hasExternalData);

            void *externalData;
            VERIFY_IS_TRUE(JsGetExternalData(object, &externalData) == JsNoError);
            VERIFY_IS_TRUE(externalData == (void *)0xdeadbeef);

            object = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsCollectGarbage(this->runtime) == JsNoError);
        }

        TEST_METHOD(ArrayAndItemTest)
        {        
            // Create some arrays
            JsValueRef array1;
            JsValueRef array2;
            size_t length;
            VERIFY_IS_TRUE(JsCreateArray(0, &array1) == JsNoError);
            VERIFY_IS_TRUE(JsGetArrayLength(array1, &length) == JsNoError);
            VERIFY_IS_TRUE(length == 0);
            VERIFY_IS_TRUE(JsCreateArray(100, &array2) == JsNoError);
            VERIFY_IS_TRUE(JsGetArrayLength(array2, &length) == JsNoError);
            VERIFY_IS_TRUE(length == 100);

            // Create an object we'll treat like an array
            JsValueRef object;        
            VERIFY_IS_TRUE(JsCreateObject(&object) == JsNoError);

            // Create an index value to use
            JsValueRef index;
            VERIFY_IS_TRUE(JsDoubleToNumber(3, &index) == JsNoError);

            JsValueRef value1;
            VERIFY_IS_TRUE(JsPointerToString(L"value1", wcslen(L"value1"), &value1) == JsNoError);
            
            // Do some index-based manipulation
            bool result;
            VERIFY_IS_TRUE(JsHasIndexedProperty(array1, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsHasIndexedProperty(array2, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            
            VERIFY_IS_TRUE(JsSetIndexedProperty(array1, index, value1) == JsNoError);
            VERIFY_IS_TRUE(JsSetIndexedProperty(array2, index, value1) == JsNoError);
            VERIFY_IS_TRUE(JsSetIndexedProperty(object, index, value1) == JsNoError);

            VERIFY_IS_TRUE(JsHasIndexedProperty(array1, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsHasIndexedProperty(array2, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);

            JsValueRef value2;
            VERIFY_IS_TRUE(JsGetIndexedProperty(array1, index, &value2) == JsNoError);
            VERIFY_IS_TRUE(JsStrictEquals(value1, value2, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsGetIndexedProperty(array2, index, &value2) == JsNoError);
            VERIFY_IS_TRUE(JsStrictEquals(value1, value2, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsGetIndexedProperty(object, index, &value2) == JsNoError);
            VERIFY_IS_TRUE(JsStrictEquals(value1, value2, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);

            VERIFY_IS_TRUE(JsDeleteIndexedProperty(array1, index) == JsNoError);
            VERIFY_IS_TRUE(JsDeleteIndexedProperty(array2, index) == JsNoError);
            VERIFY_IS_TRUE(JsDeleteIndexedProperty(object, index) == JsNoError);
            
            VERIFY_IS_TRUE(JsHasIndexedProperty(array1, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsHasIndexedProperty(array2, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, index, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
        }
        
        TEST_METHOD(EqualsTest)
        {        
            // Create some values
            JsValueRef number1;
            VERIFY_IS_TRUE(JsDoubleToNumber(1, &number1) == JsNoError);
            JsValueRef number2;
            VERIFY_IS_TRUE(JsDoubleToNumber(2, &number2) == JsNoError);
            JsValueRef stringa;
            VERIFY_IS_TRUE(JsPointerToString(L"1", wcslen(L"1"), &stringa) == JsNoError);
            JsValueRef stringb;
            VERIFY_IS_TRUE(JsPointerToString(L"1", wcslen(L"1"), &stringb) == JsNoError);

            bool result;
            VERIFY_IS_TRUE(JsEquals(number1, number1, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsEquals(number1, number2, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsEquals(number1, stringa, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsEquals(number1, stringb, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsEquals(number2, stringa, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsEquals(number2, stringb, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsEquals(stringa, stringb, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            
            VERIFY_IS_TRUE(JsStrictEquals(number1, number1, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
            VERIFY_IS_TRUE(JsStrictEquals(number1, number2, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsStrictEquals(number1, stringa, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsStrictEquals(number1, stringb, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsStrictEquals(number2, stringa, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsStrictEquals(number2, stringb, &result) == JsNoError);
            VERIFY_IS_TRUE(result == false);
            VERIFY_IS_TRUE(JsStrictEquals(stringa, stringb, &result) == JsNoError);
            VERIFY_IS_TRUE(result == true);
        }

        
        TEST_METHOD(LanguageTypeConversionTest)
        {
            JsValueRef value;
            JsValueType type;
            JsValueRef asString;
            LPCWSTR str;        
            size_t length;

            // Number

            VERIFY_IS_TRUE(JsDoubleToNumber(3.141592, &value) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsNumber);

            double dbl = 0.0;        
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_IS_TRUE(dbl == 3.141592);

            VERIFY_IS_TRUE(JsPointerToString(L"3.141592", wcslen(L"3.141592"), &asString) == JsNoError);
            VERIFY_IS_TRUE(JsConvertValueToNumber(asString, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_IS_TRUE(dbl == 3.141592);

            int intValue;
            VERIFY_IS_TRUE(JsNumberToInt(value, &intValue) == JsNoError);
            VERIFY_ARE_EQUAL(3, intValue);

            VERIFY_IS_TRUE(JsDoubleToNumber(2147483648.1, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToInt(value, &intValue) == JsNoError);
            VERIFY_ARE_EQUAL(INT_MIN, intValue);
            VERIFY_IS_TRUE(JsDoubleToNumber(-2147483649.1, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToInt(value, &intValue) == JsNoError);
            VERIFY_ARE_EQUAL(2147483647, intValue);

            // String

            VERIFY_IS_TRUE(JsDoubleToNumber(3.141592, &value) == JsNoError);
            VERIFY_IS_TRUE(JsConvertValueToString(value, &asString) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(asString, &str, &length) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(str, L"3.141592"));

            VERIFY_IS_TRUE(JsGetTrueValue(&value) == JsNoError);

            VERIFY_IS_TRUE(JsConvertValueToString(value, &asString) == JsNoError);               
            VERIFY_IS_TRUE(JsGetValueType(asString, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsString);
            VERIFY_IS_TRUE(JsStringToPointer(asString, &str, &length) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(str, L"true"));

            // Undefined

            VERIFY_IS_TRUE(JsGetUndefinedValue(&value) == JsNoError);        
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsUndefined);

            // Null

            VERIFY_IS_TRUE(JsGetNullValue(&value) == JsNoError);        
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsNull);

            // Boolean

            VERIFY_IS_TRUE(JsGetTrueValue(&value) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsBoolean);

            VERIFY_IS_TRUE(JsGetFalseValue(&value) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsBoolean);

            bool boolValue;

            VERIFY_IS_TRUE(JsBoolToBoolean(true, &value) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsBoolean);
            VERIFY_IS_TRUE(JsBooleanToBool(value, &boolValue) == JsNoError);
            VERIFY_IS_TRUE(boolValue == true);

            VERIFY_IS_TRUE(JsPointerToString(L"true", wcslen(L"true"), &asString) == JsNoError);
            VERIFY_IS_TRUE(JsConvertValueToBoolean(asString, &value) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsBoolean);
            VERIFY_IS_TRUE(JsBooleanToBool(value, &boolValue) == JsNoError);
            VERIFY_IS_TRUE(boolValue == true);

            // Object

            VERIFY_IS_TRUE(JsCreateObject(&value) == JsNoError);        
            VERIFY_IS_TRUE(JsGetValueType(value, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsObject);
        }

        TEST_METHOD(ExternalFunctionTest)
        {
            JsValueRef function;
            VERIFY_IS_TRUE(JsCreateFunction(ExternalFunctionCallback, nullptr, &function) == JsNoError);
            JsValueRef undefined;
            VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);
            JsValueRef args[] = { undefined };
            VERIFY_IS_TRUE(JsCallFunction(function, args, 1, nullptr) == JsNoError);
            JsValueRef result;
            VERIFY_IS_TRUE(JsConstructObject(function, args, 1, &result) == JsNoError);
        }

        TEST_METHOD(ExternalFunctionTestForFPCW)
        {
            // This test is explicitely disabled because we will hit Assert in LeaveScript since we don't 
            // restore FPCW because of perf reasons. Once we start restoring, enable this test case.
            BEGIN_TEST_METHOD_PROPERTIES()
                TEST_METHOD_PROPERTY(L"Disabled", L"Yes")
            END_TEST_METHOD_PROPERTIES()

            JsValueRef function;
            VERIFY_IS_TRUE(JsCreateFunction(ExternalFunctionCallbackForFpcw, nullptr, &function) == JsNoError);
            JsValueRef undefined;
            VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);
            JsValueRef args[] = { undefined };
            VERIFY_IS_TRUE(JsCallFunction(function, args, 1, nullptr) == JsNoError);
            JsValueRef result;
            VERIFY_IS_TRUE(JsConstructObject(function, args, 1, &result) == JsNoError);
        }

        TEST_METHOD(ExternalFunctionWithScriptAbortionTest)
        {
            if (!(attributes & JsRuntimeAttributeAllowScriptInterrupt))
            {
                VERIFY_IS_TRUE(JsDisableRuntimeExecution(runtime) == JsErrorCannotDisableExecution);
                return;
            }

            if (hMonitor == NULL)
            {
                hMonitor = CreateEvent(NULL, FALSE, FALSE, NULL);
            }
            if (threadHandle == NULL)
            {
                threadHandle = (HANDLE)_beginthreadex(NULL, 0, &ApiTest::StaticThreadProc, this, 0, NULL);
            }

            JsValueRef preScriptAbortFunction, postScriptAbortFunction;
            JsValueRef exception;

            for (int i = 0; i < _countof(terminationTests); i++)
            {
                BeginScriptExecution();
                SignalMonitor();
                VERIFY_IS_TRUE(JsCreateFunction(ExternalFunctionPreScriptAbortionCallback, nullptr, &preScriptAbortFunction) == JsNoError);
                VERIFY_IS_TRUE(JsCreateFunction(ExternalFunctionPostScriptAbortionCallback, nullptr, &postScriptAbortFunction) == JsNoError);
                JsValueRef scriptTextArg;

                wchar_t *scriptText = const_cast<wchar_t *>(terminationTests[i]);
                VERIFY_IS_TRUE(JsPointerToString(scriptText, wcslen(scriptText), &scriptTextArg) == JsNoError);
                JsValueRef args[] = { scriptTextArg };

                VERIFY_IS_TRUE(JsCallFunction(preScriptAbortFunction, args, 1, nullptr) == JsErrorScriptTerminated);

                bool isDisabled;
                VERIFY_IS_TRUE(JsIsRuntimeExecutionDisabled(runtime, &isDisabled) == JsNoError);
                VERIFY_IS_TRUE(isDisabled);


                VERIFY_IS_TRUE(JsCallFunction(postScriptAbortFunction, args, 1, nullptr) == JsErrorInDisabledState);

                VERIFY_IS_TRUE(JsGetAndClearException(&exception) == JsErrorInDisabledState);
                VERIFY_IS_TRUE(JsEnableRuntimeExecution(runtime) == JsNoError);
                EndScriptExecution();
            }
            SignalMonitor();
            WaitForSingleObject(threadHandle, INFINITE);
            hMonitor = NULL;
            threadHandle = NULL;
        }

        TEST_METHOD(ExternalFunctionNameTest)
        {
            auto testConstructorName = [=](JsValueRef function, PCWCHAR expectedName, size_t expectedNameLength)
            {
                JsValueRef undefined;
                VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);
                JsValueRef args[] = { undefined };

                JsValueRef obj, constructor, name;
                JsPropertyIdRef propId;
                VERIFY_IS_TRUE(JsConstructObject(function, args, 1, &obj) == JsNoError);
                VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"constructor", &propId) == JsNoError);
                VERIFY_IS_TRUE(JsGetProperty(obj, propId, &constructor) == JsNoError);
                VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"name", &propId) == JsNoError);
                VERIFY_IS_TRUE(JsGetProperty(constructor, propId, &name) == JsNoError);

                PCWCHAR actualName;
                size_t actualNameLength;
                VERIFY_IS_TRUE(JsStringToPointer(name, &actualName, &actualNameLength) == JsNoError);
                VERIFY_ARE_EQUAL(expectedNameLength, actualNameLength);
                VERIFY_IS_TRUE(wcscmp(expectedName, actualName) == 0);
            };

            JsValueRef function;
            VERIFY_IS_TRUE(JsCreateFunction(ExternalFunctionCallback, nullptr, &function) == JsNoError);
            testConstructorName(function, L"", 0);

            wchar_t name[] = L"FooName";
            JsValueRef nameString;
            VERIFY_IS_TRUE(JsPointerToString(name, _countof(name) - 1, &nameString) == JsNoError);
            VERIFY_IS_TRUE(JsCreateNamedFunction(nameString, ExternalFunctionCallback, nullptr, &function) == JsNoError);
            testConstructorName(function, name, _countof(name) - 1);
        }

        TEST_METHOD(ErrorHandlingTest)
        {
            // Run a script to setup some globals
            LPCWSTR script;
            VERIFY_IS_TRUE((script = LoadScriptFile(L"ErrorHandling.js")) != NULL);
            VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsNoError);

            JsValueRef global;        
            VERIFY_IS_TRUE(JsGetGlobalObject(&global) == JsNoError);

            JsPropertyIdRef name;
            JsValueRef result;
            JsValueRef exception;        
            JsValueRef function;
            JsValueType type;

            // throw from script, handle in host        
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"throwAtHost", &name) == JsNoError);               
            VERIFY_IS_TRUE(JsGetProperty(global, name, &function) == JsNoError);        
            VERIFY_IS_TRUE(JsCallFunction(function, NULL, 0, &result) == JsErrorScriptException);        
            VERIFY_IS_TRUE(JsGetAndClearException(&exception) == JsNoError);

            // setup a host callback for the next couple of tests
            VERIFY_IS_TRUE(JsCreateFunction(ErrorHandlingCallback, nullptr, &result) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(result, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsFunction);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"callHost", &name) == JsNoError);        
            VERIFY_IS_TRUE(JsSetProperty(global, name, result, true) == JsNoError);

            // throw from host callback, catch in script        
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"callHostWithTryCatch", &name) == JsNoError);        
            VERIFY_IS_TRUE(JsGetProperty(global, name, &function) == JsNoError);        
            VERIFY_IS_TRUE(JsCallFunction(function, NULL, 0, &result) == JsNoError);

            // throw from host callback, through script, handle in host        
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"callHostWithNoTryCatch", &name) == JsNoError);        
            VERIFY_IS_TRUE(JsGetProperty(global, name, &function) == JsNoError);        
            VERIFY_IS_TRUE(JsCallFunction(function, NULL, 0, &result) == JsErrorScriptException);
        }        

        TEST_METHOD(EngineFlagTest)
        {
            JsValueRef ret;        
            VERIFY_IS_TRUE(JsRunScript(L"new ActiveXObject('Scripting.FileSystemObject')", JS_SOURCE_CONTEXT_NONE, L"", &ret) == JsErrorScriptException);
            VERIFY_IS_TRUE(JsGetAndClearException(&ret) == JsNoError);

        }

        TEST_METHOD(ExceptionHandlingTest)
        {
            bool value;
            JsValueRef exception;
            JsValueType type;

            VERIFY_IS_TRUE(JsHasException(&value) == JsNoError);
            VERIFY_IS_TRUE(value == false);
            VERIFY_IS_TRUE(JsRunScript(L"throw new Error()", JS_SOURCE_CONTEXT_NONE, L"", NULL) == JsErrorScriptException);
            VERIFY_IS_TRUE(JsHasException(&value) == JsNoError);
            VERIFY_IS_TRUE(value == true);
            VERIFY_IS_TRUE(JsGetAndClearException(&exception) == JsNoError);
            VERIFY_IS_TRUE(JsHasException(&value) == JsNoError);
            VERIFY_IS_TRUE(value == false);
            VERIFY_IS_TRUE(JsGetValueType(exception, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsError);
            VERIFY_IS_TRUE(JsSetException(exception) == JsNoError);
            VERIFY_IS_TRUE(JsHasException(&value) == JsNoError);
            VERIFY_IS_TRUE(value == true);
        }

        TEST_METHOD(ScriptCompileErrorTest)
        {                                
            JsValueRef error;

            VERIFY_IS_TRUE(JsRunScript(
                L"if (x > 0) { \n" \
                L"  x = 1;     \n" \
                L"}}",
                JS_SOURCE_CONTEXT_NONE, L"", NULL) == JsErrorScriptCompile);           
            VERIFY_IS_TRUE(JsGetAndClearException(&error) == JsNoError);

            JsPropertyIdRef property;
            JsValueRef value;
            LPCWSTR str;
            size_t length;

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"message", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &str, &length) == JsNoError);
            VERIFY_ARE_EQUAL(String(L"Syntax error"), str);

            double dbl;
            
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"line", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_ARE_EQUAL(2, dbl);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"column", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_ARE_EQUAL(1, dbl);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"length", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_ARE_EQUAL(1, dbl);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"source", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &str, &length) == JsNoError); 
            VERIFY_ARE_EQUAL(String(L"}}"), str);

            VERIFY_IS_TRUE(JsRunScript(
                L"var for = 10;\n",
                JS_SOURCE_CONTEXT_NONE, L"", NULL) == JsErrorScriptCompile);           
            VERIFY_IS_TRUE(JsGetAndClearException(&error) == JsNoError);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"message", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &str, &length) == JsNoError);
            VERIFY_ARE_EQUAL(String(L"The use of a keyword for an identifier is invalid"), str);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"line", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_ARE_EQUAL(0, dbl);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"column", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_ARE_EQUAL(4, dbl);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"length", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &dbl) == JsNoError);
            VERIFY_ARE_EQUAL(3, dbl);

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"source", &property) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(error, property, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &str, &length) == JsNoError); 
            VERIFY_ARE_EQUAL(String(L"var for = 10;"), str);
        }

        TEST_METHOD(ScriptTerminationTest)
        {
            // can't interrupt if scriptinterrupt is disabled.
            if (!(attributes & JsRuntimeAttributeAllowScriptInterrupt))
            {
                VERIFY_IS_TRUE(JsDisableRuntimeExecution(runtime) == JsErrorCannotDisableExecution);
                return;
            }
            if (hMonitor == NULL)
            {
                hMonitor = CreateEvent(NULL, FALSE, FALSE, NULL);
            }
            if (threadHandle == NULL)
            {
                threadHandle = (HANDLE)_beginthreadex(NULL, 0, &ApiTest::StaticThreadProc, this, 0, NULL);
            }
            JsValueRef result;                
            JsValueRef exception;
            for (int i = 0; i < _countof(terminationTests); i++)
            {
                BeginScriptExecution();
                SignalMonitor();
                VERIFY_IS_TRUE(JsRunScript(terminationTests[i], JS_SOURCE_CONTEXT_NONE, L"", &result) == JsErrorScriptTerminated);
                bool isDisabled;
                VERIFY_IS_TRUE(JsIsRuntimeExecutionDisabled(runtime, &isDisabled) == JsNoError);
                VERIFY_IS_TRUE(isDisabled);
                VERIFY_IS_TRUE(JsRunScript(terminationTests[i], JS_SOURCE_CONTEXT_NONE, L"", &result) == JsErrorInDisabledState);
                VERIFY_IS_TRUE(JsGetAndClearException(&exception) == JsErrorInDisabledState);
                VERIFY_IS_TRUE(JsEnableRuntimeExecution(runtime) == JsNoError);
                EndScriptExecution();
            }
            SignalMonitor();
            WaitForSingleObject(threadHandle, INFINITE);
            hMonitor = NULL;
            threadHandle = NULL;
        }

        TEST_METHOD(ObjectTests)
        {
            LPCWSTR script = L"x = { a: \"abc\", b: \"def\", c: \"ghi\" }; x";
            JsValueRef result;
            JsValueRef propertyNames;

            VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);
            VERIFY_IS_TRUE(JsGetOwnPropertyNames(result, &propertyNames) == JsNoError);
            
            for (int index = 0; index < 3; index++)
            {
                JsValueRef indexValue;
                VERIFY_IS_TRUE(JsDoubleToNumber(index, &indexValue) == JsNoError);

                JsValueRef nameValue;
                VERIFY_IS_TRUE(JsGetIndexedProperty(propertyNames, indexValue, &nameValue) == JsNoError);

                const wchar_t *name;
                size_t length;
                VERIFY_IS_TRUE(JsStringToPointer(nameValue, &name, &length) == JsNoError);

                VERIFY_IS_TRUE(length == 1);
                VERIFY_IS_TRUE(name[0] == ('a' + index));
            }
        }

        TEST_METHOD(SymbolTests)
        {
            JsValueRef object;
            JsValueRef symbol1;
            JsValueRef string1;
            JsValueRef symbol2;
            JsValueRef value;
            JsPropertyIdRef propertyId;
            JsValueRef outValue;
            JsValueRef propertySymbols;
            const wchar_t* name;
            JsPropertyIdType propertyIdType;

            VERIFY_IS_TRUE(JsCreateObject(&object) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromSymbol(object, &propertyId) == JsErrorPropertyNotSymbol);

            VERIFY_IS_TRUE(JsPointerToString(L"abc", 3, &string1) == JsNoError);
            VERIFY_IS_TRUE(JsCreateSymbol(string1, &symbol1) == JsNoError);

            VERIFY_IS_TRUE(JsCreateSymbol(JS_INVALID_REFERENCE, &symbol2) == JsNoError);

            VERIFY_IS_TRUE(JsIntToNumber(1, &value) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromSymbol(symbol1, &propertyId) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyNameFromId(propertyId, &name) == JsErrorPropertyNotString);
            VERIFY_IS_TRUE(JsGetPropertyIdType(propertyId, &propertyIdType) == JsNoError);
            VERIFY_IS_TRUE(propertyIdType == JsPropertyIdTypeSymbol);

            JsValueRef symbol11;
            bool resultBool;
            VERIFY_IS_TRUE(JsGetSymbolFromPropertyId(propertyId, &symbol11) == JsNoError);
            VERIFY_IS_TRUE(JsEquals(symbol1, symbol11, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsStrictEquals(symbol1, symbol11, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);


            VERIFY_IS_TRUE(JsSetProperty(object, propertyId, value, true) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(object, propertyId, &outValue) == JsNoError);
            VERIFY_IS_TRUE(value == outValue);
            
            VERIFY_IS_TRUE(JsGetPropertyIdFromSymbol(symbol2, &propertyId) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(object, propertyId, value, true) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(object, propertyId, &outValue) == JsNoError);
            VERIFY_IS_TRUE(value == outValue);

            size_t intLength;
            JsValueType type;
            JsValueRef index;

            VERIFY_IS_TRUE(JsGetOwnPropertySymbols(object, &propertySymbols) == JsNoError);
            VERIFY_IS_TRUE(JsGetArrayLength(propertySymbols, &intLength) == JsNoError);
            VERIFY_IS_TRUE(intLength == 2);

            VERIFY_IS_TRUE(JsIntToNumber(0, &index) == JsNoError);
            VERIFY_IS_TRUE(JsGetIndexedProperty(propertySymbols, index, &outValue) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(outValue, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsSymbol);
            VERIFY_IS_TRUE(JsGetPropertyIdFromSymbol(outValue, &propertyId) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(object, propertyId, &outValue) == JsNoError);
            VERIFY_IS_TRUE(value == outValue);

            VERIFY_IS_TRUE(JsIntToNumber(1, &index) == JsNoError);
            VERIFY_IS_TRUE(JsGetIndexedProperty(propertySymbols, index, &outValue) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(outValue, &type) == JsNoError);
            VERIFY_IS_TRUE(type == JsSymbol);
            VERIFY_IS_TRUE(JsGetPropertyIdFromSymbol(outValue, &propertyId) == JsNoError);
            VERIFY_IS_TRUE(JsGetProperty(object, propertyId, &outValue) == JsNoError);
            VERIFY_IS_TRUE(value == outValue);
        }

        TEST_METHOD(ByteCodeTest)
        {
            LPCWSTR script = L"function test() { return true; }; test();";
            JsValueRef result;
            JsValueType type;
            bool boolValue;
            BYTE *compiledScript = nullptr;
            DWORD scriptSize = 0;

            VERIFY_IS_TRUE(JsSerializeScript(script, compiledScript, &scriptSize) == JsNoError);
            compiledScript = new BYTE[scriptSize];
            DWORD newScriptSize = scriptSize;
            VERIFY_IS_TRUE(JsSerializeScript(script, compiledScript, &newScriptSize) == JsNoError);
            VERIFY_IS_TRUE(newScriptSize == scriptSize);
            VERIFY_IS_TRUE(JsRunSerializedScript(script, compiledScript, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(result, &type) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &boolValue) == JsNoError);
            VERIFY_IS_TRUE(boolValue);

            JsRuntimeHandle second;
            JsContextRef secondContext, current;

            VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &second) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(second, &secondContext) == JsNoError);
            VERIFY_IS_TRUE(JsGetCurrentContext(&current) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(secondContext) == JsNoError);

            VERIFY_IS_TRUE(JsRunSerializedScript(script, compiledScript, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(result, &type) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &boolValue) == JsNoError);
            VERIFY_IS_TRUE(boolValue);

            VERIFY_IS_TRUE(JsSetCurrentContext(current) == JsNoError);
            VERIFY_IS_TRUE(JsDisposeRuntime(second) == JsNoError);

            delete compiledScript;
        }

        TEST_METHOD(ContextCleanupTest)
        {
            JsRuntimeHandle rt;
            VERIFY_IS_TRUE(JsCreateRuntime(attributes, NULL, &rt) == JsNoError);

            JsContextRef c1, c2, c3, c4, c5, c6, c7;

            // Create a bunch of contexts
            
            VERIFY_IS_TRUE(JsCreateContext(rt, &c1) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(rt, &c2) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(rt, &c3) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(rt, &c4) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(rt, &c5) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(rt, &c6) == JsNoError);
            VERIFY_IS_TRUE(JsCreateContext(rt, &c7) == JsNoError);

            // Clear references to some, then collect, causing them to be collected
            c1 = NULL;
            c3 = NULL;
            c5 = NULL;
            c7 = NULL;
            
            VERIFY_IS_TRUE(JsCollectGarbage(rt) == JsNoError);

            // Now dispose
            VERIFY_IS_TRUE(JsDisposeRuntime(rt) == JsNoError);
        }

        TEST_METHOD(ObjectMethodTest)
        {
            JsValueRef proto;
            JsValueRef object;

            VERIFY_IS_TRUE(JsCreateObject(&proto) == JsNoError);
            VERIFY_IS_TRUE(JsCreateObject(&object) == JsNoError);
            VERIFY_IS_TRUE(JsSetPrototype(object, proto) == JsNoError);

            JsValueRef objectProto;

            VERIFY_IS_TRUE(JsGetPrototype(object, &objectProto) == JsNoError);
            VERIFY_IS_TRUE(proto == objectProto);

            JsPropertyIdRef propertyId;
            bool hasProperty;
            JsValueRef deleteResult;

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"foo", &propertyId) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(object, propertyId, object, true) == JsNoError);
            VERIFY_IS_TRUE(JsHasProperty(object, propertyId, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsDeleteProperty(object, propertyId, true, &deleteResult) == JsNoError);
            VERIFY_IS_TRUE(JsHasProperty(object, propertyId, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);

            bool canExtend;
            VERIFY_IS_TRUE(JsGetExtensionAllowed(object, &canExtend) == JsNoError);
            VERIFY_IS_TRUE(canExtend);
            VERIFY_IS_TRUE(JsPreventExtension(object) == JsNoError);
            VERIFY_IS_TRUE(JsGetExtensionAllowed(object, &canExtend) == JsNoError);
            VERIFY_IS_TRUE(!canExtend);
            VERIFY_IS_TRUE(JsSetProperty(object, propertyId, object, true) == JsErrorScriptException);
            JsValueRef exception;
            VERIFY_IS_TRUE(JsGetAndClearException(&exception) == JsNoError);
            VERIFY_IS_TRUE(JsHasProperty(object, propertyId, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);
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

        TEST_METHOD(DisableEval)
        {
            JsValueRef result;
            JsErrorCode error = JsRunScript(L"eval(\"1 + 2\")", JS_SOURCE_CONTEXT_NONE, L"", &result);

            if (!(attributes & JsRuntimeAttributeDisableEval))
            {
                VERIFY_IS_TRUE(error == JsNoError);
            }
            else
            {
                VERIFY_IS_TRUE(error == JsErrorScriptEvalDisabled);
            }

            error = JsRunScript(L"new Function(\"return 1 + 2\")", JS_SOURCE_CONTEXT_NONE, L"", &result);

            if (!(attributes & JsRuntimeAttributeDisableEval))
            {
                VERIFY_IS_TRUE(error == JsNoError);
            }
            else
            {
                VERIFY_IS_TRUE(error == JsErrorScriptEvalDisabled);
            }
        }

        static void CALLBACK PromiseContinuationCallback(JsValueRef task, void *callbackState)
        {
            VERIFY_IS_NOT_NULL(callbackState);
            VERIFY_IS_NULL(*(void **)callbackState);
            // All the task need to finish async, so we need to save the callback.
            *(void **)callbackState = task;
        }

        TEST_METHOD(Promises)
        {
            JsValueRef result;
            JsValueType valueType;
            JsValueRef task = JS_INVALID_REFERENCE;
            JsValueRef callback = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsSetPromiseContinuationCallback(PromiseContinuationCallback, &callback) == JsNoError);
            VERIFY_IS_TRUE(JsRunScript(
                L"new Promise(" \
                L"  function(resolve, reject) {" \
                L"      resolve('basic:success');" \
                L"  }" \
                L").then (" \
                L"  function () { return new Promise(" \
                L"    function(resolve, reject) { " \
                L"      resolve('second:success'); " \
                L"      })" \
                L"  }" \
                L");", JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);
            VERIFY_IS_NOT_NULL(callback);

            // first then handler was queued
            task = callback;
            callback = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsGetValueType(task, &valueType) == JsNoError);
            VERIFY_IS_TRUE(valueType == JsFunction);
            VERIFY_IS_TRUE(JsCallFunction(task, nullptr, 0, &result) == JsNoError);

            // the second promise resolution was queued
            task = callback;
            callback = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsGetValueType(task, &valueType) == JsNoError);
            VERIFY_IS_TRUE(valueType == JsFunction);
            VERIFY_IS_TRUE(JsCallFunction(task, nullptr, 0, &result) == JsNoError);

            // second then handler was queued.
            task = callback;
            callback = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsGetValueType(task, &valueType) == JsNoError);
            VERIFY_IS_TRUE(valueType == JsFunction);
            VERIFY_IS_TRUE(JsCallFunction(task, nullptr, 0, &result) == JsNoError);

            // we are done; no more new task are queue.
            VERIFY_IS_TRUE(callback == JS_INVALID_REFERENCE);
        }

        TEST_METHOD(ArrayBuffer)
        {
            for (int type = JsArrayTypeInt8; type <= JsArrayTypeFloat64; type++)
            {
                int size = -1;

                switch (type)
                {
                case JsArrayTypeInt16:
                    size = sizeof(__int16);
                    break;
                case JsArrayTypeInt8:
                    size = sizeof(__int8);
                    break;
                case JsArrayTypeUint8:
                case JsArrayTypeUint8Clamped:
                    size = sizeof(unsigned __int8);
                    break;
                case JsArrayTypeUint16:
                    size = sizeof(unsigned __int16);
                    break;
                case JsArrayTypeInt32:
                    size = sizeof(__int32);
                    break;
                case JsArrayTypeUint32:
                    size = sizeof(unsigned __int32);
                    break;
                case JsArrayTypeFloat32:
                    size = sizeof(float);
                    break;
                case JsArrayTypeFloat64:
                    size = sizeof(double);
                    break;
                }

                // ArrayBuffer
                JsValueRef arrayBuffer;
                JsValueType valueType;
                BYTE *originBuffer;
                unsigned int originBufferLength;

                VERIFY_IS_TRUE(JsCreateArrayBuffer(16 * size, &arrayBuffer) == JsNoError);
                VERIFY_IS_TRUE(JsGetValueType(arrayBuffer, &valueType) == JsNoError);
                VERIFY_ARE_EQUAL(JsValueType::JsArrayBuffer, valueType);

                VERIFY_IS_TRUE(JsGetArrayBufferStorage(arrayBuffer, &originBuffer, &originBufferLength) == JsNoError);
                VERIFY_ARE_EQUAL(16 * size, (int)originBufferLength);

                // TypedArray
                JsValueRef typedArray;

                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)type, arrayBuffer, /*byteOffset*/size, /*length*/12, &typedArray) == JsNoError);
                VERIFY_IS_TRUE(JsGetValueType(typedArray, &valueType) == JsNoError);
                VERIFY_ARE_EQUAL(JsValueType::JsTypedArray, valueType);

                BYTE *buffer;
                unsigned int bufferLength;
                JsTypedArrayType arrayType;
                int elementSize;

                VERIFY_IS_TRUE(JsGetTypedArrayStorage(typedArray, &buffer, &bufferLength, &arrayType, &elementSize) == JsNoError);
                VERIFY_ARE_EQUAL(originBuffer + size, buffer);
                VERIFY_ARE_EQUAL(12 * size, (int)bufferLength);
                VERIFY_ARE_EQUAL(type, arrayType);
                VERIFY_ARE_EQUAL(size, elementSize);

                // DataView
                JsValueRef dataView;

                VERIFY_IS_TRUE(JsCreateDataView(arrayBuffer, /*byteOffset*/3, /*byteLength*/13, &dataView) == JsNoError);
                VERIFY_IS_TRUE(JsGetValueType(dataView, &valueType) == JsNoError);
                VERIFY_ARE_EQUAL(JsValueType::JsDataView, valueType);

                VERIFY_IS_TRUE(JsGetDataViewStorage(dataView, &buffer, &bufferLength) == JsNoError);
                VERIFY_ARE_EQUAL(originBuffer + 3, buffer);
                VERIFY_ARE_EQUAL(13, (int)bufferLength);

                // InvalidArgs Get...
                JsValueRef bad;
                VERIFY_IS_TRUE(JsIntToNumber(5, &bad) == JsNoError);

                VERIFY_IS_TRUE(JsGetArrayBufferStorage(typedArray, &buffer, &bufferLength) == JsErrorInvalidArgument);
                VERIFY_IS_TRUE(JsGetArrayBufferStorage(dataView, &buffer, &bufferLength) == JsErrorInvalidArgument);
                VERIFY_IS_TRUE(JsGetArrayBufferStorage(bad, &buffer, &bufferLength) == JsErrorInvalidArgument);

                VERIFY_IS_TRUE(JsGetTypedArrayStorage(arrayBuffer, &buffer, &bufferLength, &arrayType, &elementSize) == JsErrorInvalidArgument);
                VERIFY_IS_TRUE(JsGetTypedArrayStorage(dataView, &buffer, &bufferLength, &arrayType, &elementSize) == JsErrorInvalidArgument);
                VERIFY_IS_TRUE(JsGetTypedArrayStorage(bad, &buffer, &bufferLength, &arrayType, &elementSize) == JsErrorInvalidArgument);

                VERIFY_IS_TRUE(JsGetDataViewStorage(arrayBuffer, &buffer, &bufferLength) == JsErrorInvalidArgument);
                VERIFY_IS_TRUE(JsGetDataViewStorage(typedArray, &buffer, &bufferLength) == JsErrorInvalidArgument);
                VERIFY_IS_TRUE(JsGetDataViewStorage(bad, &buffer, &bufferLength) == JsErrorInvalidArgument);

                // InvalidArgs Create...
                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)(type + 100), arrayBuffer, /*byteOffset*/size, /*length*/12, &typedArray) == JsErrorInvalidArgument); // bad array type
                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)type, JS_INVALID_REFERENCE, /*byteOffset*/size, /*length*/12, &typedArray) == JsErrorInvalidArgument);  // byteOffset should be 0
                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)type, dataView, /*byteOffset*/size, /*length*/12, &typedArray) == JsErrorInvalidArgument);              // byteOffset should be 0
                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)type, bad, /*byteOffset*/size, /*length*/12, &typedArray) == JsErrorInvalidArgument);                   // byteOffset should be 0
                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)type, dataView, /*byteOffset*/0, /*length*/12, &typedArray) == JsErrorInvalidArgument); // length should be 0
                VERIFY_IS_TRUE(JsCreateTypedArray((JsTypedArrayType)type, bad, /*byteOffset*/0, /*length*/12, &typedArray) == JsErrorInvalidArgument);      // length should be 0
                VERIFY_IS_TRUE(JsCreateDataView(typedArray, /*byteOffset*/size, /*byteLength*/12, &dataView) == JsErrorInvalidArgument);    // must from arrayBuffer
                VERIFY_IS_TRUE(JsCreateDataView(dataView, /*byteOffset*/size, /*byteLength*/12, &dataView) == JsErrorInvalidArgument);      // must from arrayBuffer
                VERIFY_IS_TRUE(JsCreateDataView(bad, /*byteOffset*/size, /*byteLength*/12, &dataView) == JsErrorInvalidArgument);           // must from arrayBuffer
            }
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

        TEST_METHOD(Interception)
        {
            const wchar_t *interception = L"interception";
            const wchar_t *string = L"string";
            JsValueRef object;
            bool hasExternalData;
            JsPropertyIdRef interceptionId;
            JsValueRef prototypeRef;
            JsExternalTypeDescription typeDesc;

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(interception, &interceptionId) == JsNoError);
            VERIFY_IS_TRUE(JsCreateObject(&prototypeRef) == JsNoError);

            memset(&typeDesc, 0, sizeof(JsExternalTypeDescription));
            typeDesc.version = JsTypeDescriptionVersion12;
            typeDesc.className = interceptionId;
            typeDesc.prototype = prototypeRef;
            typeDesc.enumerateCallback = PropertyEnumerateCallback;
            typeDesc.queryCallback = PropertyQueryCallback;
            typeDesc.getCallback = PropertyGetCallback;
            typeDesc.setCallback = PropertySetCallback;
            typeDesc.deleteCallback = PropertyDeleteCallback;
            typeDesc.queryIndexedCallback = IndexedPropertyQueryCallback;
            typeDesc.getIndexedCallback = IndexedPropertyGetCallback;
            typeDesc.setIndexedCallback = IndexedPropertySetCallback;
            typeDesc.deleteIndexedCallback = IndexedPropertyDeleteCallback;
            typeDesc.equalsCallback = EqualsCallback;
            typeDesc.strictEqualsCallback = StrictEqualsCallback;
            typeDesc.finalizeCallback = FinalizeCallback;

            JsExternalTypeRef type;
            VERIFY_IS_TRUE(JsCreateExternalType(&typeDesc, &type) == JsNoError);
            VERIFY_IS_TRUE(JsCreateTypedExternalObject(type, (void *)string, &object) == JsNoError);

            // Verify the external data
            VERIFY_IS_TRUE(JsHasExternalData(object, &hasExternalData) == JsNoError);
            VERIFY_IS_TRUE(hasExternalData);

            // Verify the class name
            JsValueRef className;
            const wchar_t *classNameString;
            size_t classNameLength;
            VERIFY_IS_TRUE(JsConvertValueToString(object, &className) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(className, &classNameString, &classNameLength) == JsNoError);
            VERIFY_IS_TRUE(wcscmp(classNameString, L"[object interception]") == 0);

            // Verify the prototype
            JsValueRef objectPrototype;
            VERIFY_IS_TRUE(JsGetPrototype(object, &objectPrototype) == JsNoError);
            VERIFY_IS_TRUE(objectPrototype == prototypeRef);

            JsPropertyIdRef aName;
            JsPropertyIdRef bName;
            JsPropertyIdRef cName;
            JsPropertyIdRef dName;
            JsPropertyIdRef eName;
            JsPropertyIdRef fName;

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"a", &aName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"b", &bName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"c", &cName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"d", &dName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"e", &eName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"f", &fName) == JsNoError);

            JsValueRef values[6];

            for (int i = 0; i < 6; i++)
            {
                VERIFY_IS_TRUE(JsIntToNumber(i + 1, &values[i]) == JsNoError);
            }

            // Verify set
            VERIFY_IS_TRUE(JsSetProperty(object, aName, values[0], false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(object, bName, values[1], false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(object, cName, values[2], false) == JsNoError);

            // Verify set with attributes
            JsValueRef propertyDescriptor;
            JsValueRef configurableName;
            JsValueRef writableName;
            JsValueRef enumerableName;
            JsValueRef valueName;
            JsValueRef trueValue;

            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"configurable", &configurableName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"writable", &writableName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"enumerable", &enumerableName) == JsNoError);
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"value", &valueName) == JsNoError);

            VERIFY_IS_TRUE(JsBoolToBoolean(true, &trueValue) == JsNoError);

            bool resultBool;

            VERIFY_IS_TRUE(JsCreateObject(&propertyDescriptor) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, configurableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, writableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, enumerableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, valueName, values[3], false) == JsNoError);
            VERIFY_IS_TRUE(JsDefineProperty(object, dName, propertyDescriptor, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);

            VERIFY_IS_TRUE(JsCreateObject(&propertyDescriptor) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, configurableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, writableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, enumerableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, valueName, values[4], false) == JsNoError);
            VERIFY_IS_TRUE(JsDefineProperty(object, eName, propertyDescriptor, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);

            VERIFY_IS_TRUE(JsCreateObject(&propertyDescriptor) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, configurableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, writableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, enumerableName, trueValue, false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(propertyDescriptor, valueName, values[5], false) == JsNoError);
            VERIFY_IS_TRUE(JsDefineProperty(object, fName, propertyDescriptor, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);

            // Verify have
            bool hasProperty;
            VERIFY_IS_TRUE(JsHasProperty(object, aName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, bName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, cName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, dName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, eName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, fName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);

            // Verify get
            JsValueRef value;
            double number;
            VERIFY_IS_TRUE(JsGetProperty(object, aName, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 1);
            VERIFY_IS_TRUE(JsGetProperty(object, bName, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 2);
            VERIFY_IS_TRUE(JsGetProperty(object, cName, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 3);
            VERIFY_IS_TRUE(JsGetProperty(object, dName, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 4);
            VERIFY_IS_TRUE(JsGetProperty(object, eName, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 5);
            VERIFY_IS_TRUE(JsGetProperty(object, fName, &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 6);

            // Verify delete
            JsValueRef result;
            VERIFY_IS_TRUE(JsDeleteProperty(object, aName, false, &result) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsDeleteProperty(object, bName, false, &result) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsDeleteProperty(object, cName, false, &result) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsDeleteProperty(object, dName, false, &result) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsDeleteProperty(object, eName, false, &result) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsDeleteProperty(object, fName, false, &result) == JsNoError);
            VERIFY_IS_TRUE(JsBooleanToBool(result, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);

            VERIFY_IS_TRUE(JsHasProperty(object, aName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, bName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, cName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, dName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, eName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasProperty(object, fName, &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);

            // Verify indexed set
            VERIFY_IS_TRUE(JsSetIndexedProperty(object, values[0], values[0]) == JsNoError);
            VERIFY_IS_TRUE(JsSetIndexedProperty(object, values[1], values[1]) == JsNoError);
            VERIFY_IS_TRUE(JsSetIndexedProperty(object, values[2], values[2]) == JsNoError);

            // Verify indexed have
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, values[0], &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, values[1], &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, values[2], &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);

            // Verify get
            VERIFY_IS_TRUE(JsGetIndexedProperty(object, values[0], &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 1);
            VERIFY_IS_TRUE(JsGetIndexedProperty(object, values[1], &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 2);
            VERIFY_IS_TRUE(JsGetIndexedProperty(object, values[2], &value) == JsNoError);
            VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
            VERIFY_IS_TRUE(number == 3);

            // Verify delete
            VERIFY_IS_TRUE(JsDeleteIndexedProperty(object, values[0]) == JsNoError);
            VERIFY_IS_TRUE(JsDeleteIndexedProperty(object, values[1]) == JsNoError);
            VERIFY_IS_TRUE(JsDeleteIndexedProperty(object, values[2]) == JsNoError);

            VERIFY_IS_TRUE(JsHasIndexedProperty(object, values[0], &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, values[1], &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(hasProperty);
            VERIFY_IS_TRUE(JsHasIndexedProperty(object, values[2], &hasProperty) == JsNoError);
            VERIFY_IS_TRUE(!hasProperty);

            // Verify equals
            VERIFY_IS_TRUE(JsEquals(object, values[0], &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsStrictEquals(object, values[0], &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);

            // Verify enumeration
            VERIFY_IS_TRUE(JsSetProperty(object, aName, values[0], false) == JsNoError);
            VERIFY_IS_TRUE(JsSetProperty(object, cName, values[2], false) == JsNoError);

            JsValueRef names;
            const wchar_t *name;
            size_t length;
            JsValueRef index;
            size_t arrayLength;
            VERIFY_IS_TRUE(JsGetOwnPropertyNames(object, &names) == JsNoError);
            VERIFY_IS_TRUE(JsGetArrayLength(names, &arrayLength) == JsNoError);
            VERIFY_IS_TRUE(arrayLength == 3);
            VERIFY_IS_TRUE(JsIntToNumber(0, &index) == JsNoError);
            VERIFY_IS_TRUE(JsGetIndexedProperty(names, index, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &name, &length) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(name, L"b"));
            VERIFY_IS_TRUE(JsIntToNumber(1, &index) == JsNoError);
            VERIFY_IS_TRUE(JsGetIndexedProperty(names, index, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &name, &length) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(name, L"a"));
            VERIFY_IS_TRUE(JsIntToNumber(2, &index) == JsNoError);
            VERIFY_IS_TRUE(JsGetIndexedProperty(names, index, &value) == JsNoError);
            VERIFY_IS_TRUE(JsStringToPointer(value, &name, &length) == JsNoError);
            VERIFY_IS_TRUE(!wcscmp(name, L"c"));

            JsValueRef symbols;
            JsValueType valueType;
            VERIFY_IS_TRUE(JsGetOwnPropertySymbols(object, &symbols) == JsNoError);
            VERIFY_IS_TRUE(JsGetArrayLength(symbols, &arrayLength) == JsNoError);
            VERIFY_IS_TRUE(arrayLength == 1);
            VERIFY_IS_TRUE(JsIntToNumber(0, &index) == JsNoError);
            VERIFY_IS_TRUE(JsGetIndexedProperty(symbols, index, &value) == JsNoError);
            VERIFY_IS_TRUE(JsGetValueType(value, &valueType) == JsNoError);
            VERIFY_IS_TRUE(valueType == JsSymbol);

            // bug367066
            memset(&typeDesc, 0, sizeof(JsExternalTypeDescription));
            typeDesc.version = JsTypeDescriptionVersion12;

            VERIFY_IS_TRUE(JsCreateExternalType(&typeDesc, &type) == JsNoError);
            VERIFY_IS_TRUE(JsCreateTypedExternalObject(type, (void *)string, &object) == JsNoError);
            VERIFY_IS_TRUE(JsEquals(object, object, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);
            VERIFY_IS_TRUE(JsStrictEquals(object, object, &resultBool) == JsNoError);
            VERIFY_IS_TRUE(resultBool);

            // Test compatibility with IE11
            typeDesc.version = (JsExternalTypeDescriptionVersion)0;
            typeDesc.finalizeCallback = OldFinalizeCallback;

            VERIFY_IS_TRUE(JsCreateExternalType(&typeDesc, &type) == JsNoError);
            VERIFY_IS_TRUE(JsCreateTypedExternalObject(type, nullptr, &object) == JsNoError);
            object = JS_INVALID_REFERENCE;
            VERIFY_IS_TRUE(JsCollectGarbage(this->runtime) == JsNoError);
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
            VERIFY_ARE_EQUAL(0, called);
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
            auto verifyIndexedPropertiesEqual = [](JsValueRef a, JsValueRef b, int len)
            {
                for (int i = 0; i < len; i++)
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
                VERIFY_IS_TRUE(JsSetIndexedPropertiesToExternalData(obj, buffer, type, length) == JsNoError);

                bool tmpHas;
                VERIFY_IS_TRUE(JsHasIndexedPropertiesExternalData(obj, &tmpHas) == JsNoError);
                VERIFY_IS_TRUE(tmpHas);

                void* tmpData;
                JsTypedArrayType tmpType;
                unsigned int tmpLength;
                VERIFY_IS_TRUE(JsGetIndexedPropertiesExternalData(obj, &tmpData, &tmpType, &tmpLength) == JsNoError);
                VERIFY_ARE_EQUAL(static_cast<void*>(buffer), tmpData);
                VERIFY_ARE_EQUAL(type, tmpType);
                VERIFY_ARE_EQUAL(static_cast<unsigned int>(length), tmpLength);

                verifyIndexedPropertiesEqual(typedArray, obj, length);

                for (int k = 0; k < static_cast<int>(length); k++)
                {
                    JsValueRef index, value;
                    VERIFY_IS_TRUE(JsIntToNumber(k, &index) == JsNoError);
                    VERIFY_IS_TRUE(JsIntToNumber(k * 100, &value) == JsNoError);
                    VERIFY_IS_TRUE(JsSetIndexedProperty(obj, index, value) == JsNoError);
                }
                verifyIndexedPropertiesEqual(typedArray, obj, length);
            }
        }

    private:
        class SimpleEnumerator
        {
        private:
            JsMoveNextCallback defaultMoveNext;
            JsGetCurrentCallback defaultGetCurrent;
            JsEndEnumerationCallback defaultEndEnumeration;
            void *defaultData;
            int index;

        public:
            SimpleEnumerator(JsMoveNextCallback defaultMoveNext, JsGetCurrentCallback defaultGetCurrent, JsEndEnumerationCallback defaultEndEnumeration, void *defaultData) :
                index(-1),
                defaultMoveNext(defaultMoveNext),
                defaultGetCurrent(defaultGetCurrent),
                defaultEndEnumeration(defaultEndEnumeration),
                defaultData(defaultData)
            {
            }

            static bool MoveNext(void *data)
            {
                SimpleEnumerator *value = (SimpleEnumerator *)data;
                if (value->index < 2)
                {
                    value->index++;
                }

                return value->index < 2 || value->defaultMoveNext(value->defaultData);
            }

            static JsValueRef GetCurrent(void *data)
            {
                SimpleEnumerator *value = (SimpleEnumerator *)data;
                JsValueRef current;

                if (value->index == -1)
                {
                    JsGetUndefinedValue(&current);
                    return current;
                }

                if (value->index == 0)
                {
                    JsPointerToString(L"b", 1, &current);
                    return current;
                }

                if (value->index == 1)
                {
                    JsCreateSymbol(JS_INVALID_REFERENCE, &current);
                    return current;
                }

                return value->defaultGetCurrent(value->defaultData);
            }

            static void EndEnumeration(void *data)
            {
                SimpleEnumerator *value = (SimpleEnumerator *)data;
                value->defaultEndEnumeration(value->defaultData);
            }
        };

        static bool PropertyEnumerateCallback(JsValueRef object, bool includeNonEnumerable, JsMoveNextCallback *moveNext, JsGetCurrentCallback *enumerator, JsEndEnumerationCallback *finish, void **data)
        {
            JsMoveNextCallback defaultMoveNext;
            JsGetCurrentCallback defaultGetCurrent;
            JsEndEnumerationCallback defaultEndEnumeration;
            void *defaultData;
            const JsExternalTypeDescription *defaultDesc;
            VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
            VERIFY_IS_TRUE(defaultDesc->enumerateCallback(object, includeNonEnumerable, &defaultMoveNext, &defaultGetCurrent, &defaultEndEnumeration, &defaultData) == true);
            *data = new SimpleEnumerator(defaultMoveNext, defaultGetCurrent, defaultEndEnumeration, defaultData);
            *moveNext = SimpleEnumerator::MoveNext;
            *enumerator = SimpleEnumerator::GetCurrent;
            *finish = SimpleEnumerator::EndEnumeration;
            return true;
        }

        static bool PropertyQueryCallback(JsValueRef object, JsPropertyIdRef propertyId, JsPropertyAttributes *result)
        {
            JsPropertyIdRef aName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"a", &aName) == JsNoError);

            if (propertyId == aName)
            {
                // "a" is not handled
                return false;
            }

            JsPropertyIdRef bName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"b", &bName) == JsNoError);

            if (propertyId == bName)
            {
                *result = (JsPropertyAttributes)(JsPropertyAttributeConfigurable | JsPropertyAttributeWritable | JsPropertyAttributeEnumerable);
                return true;
            }

            JsPropertyIdRef cName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"c", &cName) == JsNoError);

            if (propertyId == cName)
            {
                // "c" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->queryCallback(object, propertyId, result) == true);
                return true;
            }

            JsPropertyIdRef dName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"d", &dName) == JsNoError);

            if (propertyId == dName)
            {
                // "d" is not handled
                return false;
            }

            JsPropertyIdRef eName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"e", &eName) == JsNoError);

            if (propertyId == eName)
            {
                // "e" we handle
                *result = (JsPropertyAttributes)(JsPropertyAttributeConfigurable | JsPropertyAttributeWritable | JsPropertyAttributeEnumerable);
                return true;
            }

            JsPropertyIdRef fName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"f", &fName) == JsNoError);

            if (propertyId == fName)
            {
                // "f" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->queryCallback(object, propertyId, result) == true);
                return true;
            }

            return false;
        }

        static bool PropertyGetCallback(JsValueRef object, JsPropertyIdRef propertyId, JsValueRef *result)
        {
            JsPropertyIdRef aName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"a", &aName) == JsNoError);

            if (propertyId == aName)
            {
                // "a" is not handled
                return false;
            }

            JsPropertyIdRef bName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"b", &bName) == JsNoError);

            if (propertyId == bName)
            {
                // "b" we handle
                VERIFY_IS_TRUE(JsIntToNumber(2, result) == JsNoError);
                return true;
            }

            JsPropertyIdRef cName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"c", &cName) == JsNoError);

            if (propertyId == cName)
            {
                // "c" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->getCallback(object, propertyId, result) == true);
                return true;
            }

            JsPropertyIdRef dName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"d", &dName) == JsNoError);

            if (propertyId == dName)
            {
                // "d" is not handled
                return false;
            }

            JsPropertyIdRef eName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"e", &eName) == JsNoError);

            if (propertyId == eName)
            {
                // "e" we handle
                VERIFY_IS_TRUE(JsIntToNumber(5, result) == JsNoError);
                return true;
            }

            JsPropertyIdRef fName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"f", &fName) == JsNoError);

            if (propertyId == fName)
            {
                // "f" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->getCallback(object, propertyId, result) == true);
                return true;
            }

            return false;
        }

        static bool PropertySetCallback(JsValueRef object, JsPropertyIdRef propertyId, JsPropertyAttributes attributes, JsValueRef value)
        {
            JsPropertyIdRef aName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"a", &aName) == JsNoError);

            if (propertyId == aName)
            {
                VERIFY_IS_TRUE(attributes == JsPropertyAttributeInvalid);
                VERIFY_IS_TRUE(value != JS_INVALID_REFERENCE);
                // "a" is not handled
                return false;
            }

            JsPropertyIdRef bName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"b", &bName) == JsNoError);

            if (propertyId == bName)
            {
                VERIFY_IS_TRUE(attributes == JsPropertyAttributeInvalid);
                VERIFY_IS_TRUE(value != JS_INVALID_REFERENCE);
                // "b" we handle
                double number;
                VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
                VERIFY_IS_TRUE(number == 2);
                return true;
            }

            JsPropertyIdRef cName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"c", &cName) == JsNoError);

            if (propertyId == cName)
            {
                VERIFY_IS_TRUE(attributes == JsPropertyAttributeInvalid);
                VERIFY_IS_TRUE(value != JS_INVALID_REFERENCE);
                // "c" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->setCallback(object, propertyId, attributes, value) == true);
                return true;
            }

            JsPropertyIdRef dName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"d", &dName) == JsNoError);

            if (propertyId == dName)
            {
                VERIFY_IS_TRUE(attributes == (JsPropertyAttributeConfigurable | JsPropertyAttributeWritable | JsPropertyAttributeEnumerable));
                VERIFY_IS_TRUE(value != JS_INVALID_REFERENCE);
                // "d" is not handled
                return false;
            }

            JsPropertyIdRef eName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"e", &eName) == JsNoError);

            if (propertyId == eName)
            {
                VERIFY_IS_TRUE(attributes == (JsPropertyAttributeConfigurable | JsPropertyAttributeWritable | JsPropertyAttributeEnumerable));
                VERIFY_IS_TRUE(value != JS_INVALID_REFERENCE);
                // "e" we handle
                double number;
                VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
                VERIFY_IS_TRUE(number == 5);
                return true;
            }

            JsPropertyIdRef fName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"f", &fName) == JsNoError);

            if (propertyId == fName)
            {
                VERIFY_IS_TRUE(attributes == (JsPropertyAttributeConfigurable | JsPropertyAttributeWritable | JsPropertyAttributeEnumerable));
                VERIFY_IS_TRUE(value != JS_INVALID_REFERENCE);
                // "f" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->setCallback(object, propertyId, attributes, value) == true);
                return true;
            }

            return false;
        }

        static bool PropertyDeleteCallback(JsValueRef object, JsPropertyIdRef propertyId)
        {
            JsPropertyIdRef aName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"a", &aName) == JsNoError);

            if (propertyId == aName)
            {
                // "a" is not handled
                return false;
            }

            JsPropertyIdRef bName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"b", &bName) == JsNoError);

            if (propertyId == bName)
            {
                // "b" we handle
                return true;
            }

            JsPropertyIdRef cName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"c", &cName) == JsNoError);

            if (propertyId == cName)
            {
                // "c" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->deleteCallback(object, propertyId) == true);
                return true;
            }

            JsPropertyIdRef dName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"d", &dName) == JsNoError);

            if (propertyId == dName)
            {
                // "d" is not handled
                return false;
            }

            JsPropertyIdRef eName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"e", &eName) == JsNoError);

            if (propertyId == eName)
            {
                // "e" we handle
                return true;
            }

            JsPropertyIdRef fName;
            VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"f", &fName) == JsNoError);

            if (propertyId == fName)
            {
                // "f" we pass through
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->deleteCallback(object, propertyId) == true);
                return true;
            }

            return false;
        }

        static bool IndexedPropertyQueryCallback(JsValueRef object, unsigned int index, JsPropertyAttributes *result)
        {
            if (index == 1)
            {
                return false;
            }

            if (index == 2)
            {
                *result = JsPropertyAttributeNone;
                return true;
            }

            if (index == 3)
            {
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->queryIndexedCallback(object, index, result) == true);
                return true;
            }

            return false;
        }

        static bool IndexedPropertyGetCallback(JsValueRef object, unsigned index, JsValueRef *result)
        {
            if (index == 1)
            {
                return false;
            }

            if (index == 2)
            {
                VERIFY_IS_TRUE(JsIntToNumber(2, result) == JsNoError);
                return true;
            }

            if (index == 3)
            {
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->getIndexedCallback(object, index, result) == true);
                return true;
            }

            return false;
        }

        static bool IndexedPropertySetCallback(JsValueRef object, unsigned index, JsValueRef value)
        {
            if (index == 1)
            {
                return false;
            }

            if (index == 2)
            {
                double number;
                VERIFY_IS_TRUE(JsNumberToDouble(value, &number) == JsNoError);
                VERIFY_IS_TRUE(number == 2);
                return true;
            }

            if (index == 3)
            {
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->setIndexedCallback(object, index, value) == true);
                return true;
            }

            return false;
        }

        static bool IndexedPropertyDeleteCallback(JsValueRef object, unsigned index)
        {
            if (index == 1)
            {
                return false;
            }

            if (index == 2)
            {
                return true;
            }

            if (index == 3)
            {
                const JsExternalTypeDescription *defaultDesc;
                VERIFY_IS_TRUE(JsGetDefaultTypeDescription(&defaultDesc) == JsNoError);
                VERIFY_IS_TRUE(defaultDesc->deleteIndexedCallback(object, index) == true);
                return true;
            }

            return false;
        }

        static bool EqualsCallback(JsValueRef /* object */, JsValueRef /* other */, bool *result)
        {
            *result = true;
            return true;
        }

        static bool StrictEqualsCallback(JsValueRef /* object */, JsValueRef /* other */, bool *result)
        {
            *result = true;
            return true;
        }

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

        static JsValueRef ErrorHandlingCallback(JsValueRef /* function */, bool /* isConstructCall */, JsValueRef * /* args */, USHORT /* cargs */, void * /* callbackState */)
        {        
            JsValueRef result;                

            VERIFY_IS_TRUE(JsRunScript(L"new Error()", JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);         
            VERIFY_IS_TRUE(JsSetException(result) == JsNoError);

            return NULL;
        }

        static unsigned int  StaticThreadProc(LPVOID lpParameter)
        {
            DWORD ret = (DWORD)-1;
            ApiTest * apiTest = (ApiTest *)lpParameter;
            ret = apiTest->ThreadProc();

            return ret;
        }

        void BeginScriptExecution() { isScriptActive = true; }
        void EndScriptExecution() { isScriptActive = false; }
        void SignalMonitor() { SetEvent(hMonitor); }
        unsigned int ThreadProc()
        {
            while (1)
            {
                WaitForSingleObject(hMonitor, INFINITE);
                // TODO: have a generic stopping mechanism.
                if (isScriptActive)
                {
                    Sleep(ApiTest::waitTime);
                    VERIFY_IS_TRUE(JsDisableRuntimeExecution(runtime) == JsNoError);
                }
                else
                {
                    CloseHandle(hMonitor);
                    break;
                }
            }
            return 0;
        }

        static bool IsNaN(JsValueRef a)
        {
            bool equals;
            VERIFY_IS_TRUE(JsStrictEquals(a, a, &equals) == JsNoError);
            return !equals;
        };

    private:
        JsRuntimeHandle runtime;
        HANDLE threadHandle;
        HANDLE hMonitor;
        BOOL isScriptActive;
        JsRuntimeAttributes jsRuntimeAttributes;
        static const LPCWSTR terminationTests[];
        static const int waitTime = 1000;
    };
}
