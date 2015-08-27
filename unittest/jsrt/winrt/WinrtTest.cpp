//-----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include <winstring.h>
#include <roapi.h>
#include "WinRTTest.h"
#include <windows.foundation.h>
#include <windows.globalization.h>

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{
    bool WinRTTest::Setup()
    {
        if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        {
            return false;
        }
        return true;
    }
    
    bool WinRTTest::Cleanup()
    {        
        CoUninitialize();

        return true;
    }

    void WinRTTest::BasicTest()
    {        
        WCHAR* fileNames[] = {L"equals.js"};
        WCHAR* results[] = {L"[object Windows.Foundation.Collections.IMapChangedEventArgs`1<String>]"};
        //            LPCWSTR scriptCode = L"var ps = new Windows.Foundation.Collections.PropertySet(); ps.addEventListener('mapchanged', function(ev){WScript.Echo(ev);WScript.Echo(ev.target);});";
        for (int i = 0; i < sizeof(fileNames)/sizeof(WCHAR*); i++)
        {
            LPCWSTR script = LoadScriptFile(fileNames[i]);


            // Create the runtime
            JsRuntimeHandle runtime;
            VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeNone, NULL, &runtime) == JsNoError);

            // Create and initialize the script context
            JsContextRef context;
            JsValueRef result;

            VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
            VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);

            VERIFY_IS_TRUE(JsProjectWinRTNamespace(L"Windows") == JsNoError);
            VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);

            wchar_t* stringContent;
            size_t stringLength;
            VERIFY_IS_TRUE(JsStringToPointer(result, (const wchar_t**)&stringContent, &stringLength) == JsNoError);
            wprintf(L"%s\n", stringContent);
            VERIFY_IS_TRUE(wcscmp(stringContent, results[i]) == 0);
            
            VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);  
            VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);  
        }
    }

    typedef struct _jsCall {
        JsProjectionCallback jsCallback;
        JsProjectionCallbackContext jsContext;
        HANDLE event;
    } jsCall;

    // filename: The file to run.
    // scriptResult: The expected output of the script.
    // asyncResult: The expected value of 'injectedResult' object after async call executes.
    void RunProjectionCallbackTest(
        _In_ PCWSTR fileName,
        _In_ PCWSTR scriptResult,
        _In_ PCWSTR asyncResult
    )
    {
        LPCWSTR script = LoadScriptFile(fileName);

        // Create the runtime
        JsRuntimeHandle runtime;
        VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeNone, NULL, &runtime) == JsNoError);

        // Create and initialize the script context
        JsContextRef context;
        JsValueRef result;
        JsValueRef function;

        VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
        VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);

        // Inject a global property injectedResult to get async results.
        JsValueRef globalObject; 
        VERIFY_IS_TRUE(JsGetGlobalObject(&globalObject) == JsNoError);

        JsPropertyIdRef resultPropertyId; 
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"injectedResult", &resultPropertyId) == JsNoError);

        JsValueRef undefined;
        VERIFY_IS_TRUE(JsGetUndefinedValue(&undefined) == JsNoError);
        VERIFY_IS_TRUE(JsSetProperty(globalObject, resultPropertyId, undefined, false) == JsNoError);

        HANDLE event = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
        
        VERIFY_IS_NOT_NULL(event);

        jsCall outstandingCall = {};
        outstandingCall.event = event;
        VERIFY_IS_TRUE(JsSetProjectionEnqueueCallback([] (JsProjectionCallback jsCallback, JsProjectionCallbackContext jsContext, void *callbackState) {
                jsCall* call = (jsCall*) callbackState;
                VERIFY_IS_TRUE(call->jsCallback == nullptr);
                VERIFY_IS_TRUE(call->jsContext == nullptr);
                VERIFY_IS_TRUE(call->event != nullptr);
                call->jsCallback = jsCallback;
                call->jsContext = jsContext;
                SetEvent(call->event);
            },
            &outstandingCall) == JsNoError);
            
        VERIFY_IS_TRUE(JsProjectWinRTNamespace(L"Windows") == JsNoError);

        // Run the script.
        VERIFY_IS_TRUE(JsParseScript(script, JS_SOURCE_CONTEXT_NONE, L"", &function) == JsNoError);
        JsValueRef args[] = { WinRTTest::GetUndefined() };
        VERIFY_IS_TRUE(JsCallFunction(function, args, _countof(args), &result) == JsNoError);

        // Validate the script result.
        wchar_t* stringContent;
        size_t stringLength;
        VERIFY_IS_TRUE(JsStringToPointer(result, (const wchar_t**)&stringContent, &stringLength) == JsNoError);
        wprintf(L"%s\n", stringContent);
        VERIFY_IS_TRUE(wcscmp(stringContent, scriptResult) == 0);
        
        if (asyncResult != nullptr) 
        {
            // wait for the async call to come in.
            VERIFY_IS_TRUE(WaitForSingleObjectEx(outstandingCall.event, 10000, FALSE) == WAIT_OBJECT_0);
            
            // execute the async call.
            outstandingCall.jsCallback(outstandingCall.jsContext);

            // Validate the async result, the is the value injectedResult is set to.
            JsValueRef resultVariable;
            VERIFY_IS_TRUE(JsGetProperty(globalObject, resultPropertyId, &resultVariable) == JsNoError);

            VERIFY_IS_TRUE(JsStringToPointer(resultVariable, (const wchar_t**)&stringContent, &stringLength) == JsNoError);
            wprintf(L"%s\n", stringContent);
            VERIFY_IS_TRUE(wcscmp(stringContent, asyncResult) == 0);
        }

        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);  
        VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);        
    }

    void WinRTTest::ProjectionCallbackEventTest()
    {
        // Run a script which sets an event handler on a PropertySet and then modifies the set.
        // NOTE: The event is executed sync to the map change.
        RunProjectionCallbackTest(L"event.js",
                        L"map changed",
                        nullptr);
    }

    void WinRTTest::ProjectionCallbackAsyncTest()
    {
        // Run a script which attempts to read a file from the documents folder.  We expect
        // the file not to be found.
        RunProjectionCallbackTest(L"async.js",
                        L"result is pending",
                        L"does not exist");
    }

    void WinRTTest::ProjectionCallbackErrorTest()
    {
        // Create the runtime
        JsRuntimeHandle runtime;
        VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeNone, NULL, &runtime) == JsNoError);

        // Create and initialize the script context
        JsContextRef context;
        
        VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
        VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);
        
        JsProjectionEnqueueCallback callback = [] (JsProjectionCallback jsCallback, void *callbackState, JsProjectionCallbackContext jsContext) -> void { };

        VERIFY_IS_TRUE(JsSetProjectionEnqueueCallback(nullptr, nullptr) == JsErrorNullArgument);
        VERIFY_IS_TRUE(JsSetProjectionEnqueueCallback(callback, nullptr) == JsNoError);
        VERIFY_IS_TRUE(JsSetProjectionEnqueueCallback(callback, nullptr) == JsCannotSetProjectionEnqueueCallback);

        VERIFY_IS_TRUE(JsProjectWinRTNamespace(L"Windows") == JsNoError);

        VERIFY_IS_TRUE(JsSetProjectionEnqueueCallback(callback, nullptr) == JsCannotSetProjectionEnqueueCallback);
        
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);          
        VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);
    }

    void WinRTTest::InspectableTest()
    {        
        HRESULT hr;
        Windows::Foundation::IPropertyValueStatics * propertyFactory = nullptr;
        IInspectable* propertyValue = nullptr;
        HSTRING factoryName;
        hr = WindowsCreateString(L"Windows.Foundation.PropertyValue", wcslen(L"Windows.Foundation.PropertyValue"), &factoryName);
        if (SUCCEEDED(hr))
        {
            hr = RoGetActivationFactory(factoryName, Windows::Foundation::IID_IPropertyValueStatics, (void**)&propertyFactory);
            WindowsDeleteString(factoryName);
        }
        if (SUCCEEDED(hr))
        {
            propertyFactory->CreateDouble(0.5, &propertyValue);
        }


        // Create the runtime
        JsRuntimeHandle runtime;
        VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeNone, NULL, &runtime) == JsNoError);

        // Create and initialize the script context
        JsContextRef context;
        JsValueRef builtinVar;

        VERIFY_IS_TRUE(JsCreateContext(runtime, &context) == JsNoError);
        VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);
        VERIFY_IS_TRUE(JsProjectWinRTNamespace(L"Windows") == JsNoError);

        VERIFY_IS_TRUE(JsInspectableToObject(propertyValue, &builtinVar) == JsNoError);

        double retValue;
        VERIFY_IS_TRUE(JsNumberToDouble(builtinVar, &retValue) == JsNoError);
        VERIFY_IS_TRUE(retValue == 0.5);

        if (propertyFactory) 
        {
            propertyFactory->Release();
        }
        if (propertyValue)
        {
            propertyValue->Release();
        }

        Windows::Globalization::DateTimeFormatting::IDateTimeFormatterStatics* dtFormatterFactory = nullptr;
        IInspectable* dtFormatterInspectable = nullptr;
        HSTRING dtFormatterFactoryName;
        hr = WindowsCreateString(L"Windows.Globalization.DateTimeFormatting.DateTimeFormatter", wcslen(L"Windows.Globalization.DateTimeFormatting.DateTimeFormatter"), &dtFormatterFactoryName);
        if (SUCCEEDED(hr))
        {
            hr = RoGetActivationFactory(dtFormatterFactoryName, Windows::Globalization::DateTimeFormatting::IID_IDateTimeFormatterStatics, (void**)&dtFormatterFactory);
            WindowsDeleteString(dtFormatterFactoryName);
        }
        Windows::Globalization::DateTimeFormatting::IDateTimeFormatter* dtFormatter = nullptr;
        if (SUCCEEDED(hr))
        {
            dtFormatterFactory->get_LongDate(&dtFormatter);
        }
        JsValueRef dtFormatterVar;
        VERIFY_IS_TRUE(JsObjectToInspectable(builtinVar, &dtFormatterInspectable) == JsErrorObjectNotInspectable);
        VERIFY_IS_TRUE(JsInspectableToObject(dtFormatter, &dtFormatterVar) == JsNoError);
        VERIFY_IS_TRUE(JsObjectToInspectable(dtFormatterVar, &dtFormatterInspectable) == JsNoError);
        VERIFY_IS_TRUE(dtFormatterInspectable != nullptr);
        HSTRING value;
        VERIFY_IS_TRUE(((Windows::Globalization::DateTimeFormatting::IDateTimeFormatter*)dtFormatterInspectable)->get_Clock(&value) == S_OK);
        
        if (dtFormatterFactory) 
        {
            dtFormatterFactory->Release();
        }
        if (dtFormatter)
        {
            dtFormatter->Release();
        }
        if (dtFormatterInspectable)
        {
            dtFormatterInspectable->Release();
        }

        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);  
        VERIFY_IS_TRUE(JsDisposeRuntime(runtime) == JsNoError);  
    }
}
