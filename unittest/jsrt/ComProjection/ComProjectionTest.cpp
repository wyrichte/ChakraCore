//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ComProjectionTest.h"

const IID GUID_NULL = {};

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{        
    CLSID ComProjectionTest::fsoClsid = { 0x0D43FE01, 0xF093, 0x11CF, { 0x89, 0x40, 0x00, 0xA0, 0xC9, 0x05, 0x42, 0x28 } };

    ComProjectionTest::ComProjectionTest() : runtime(NULL)
    {
    }

    bool ComProjectionTest::Setup()
    {
        if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        {
            return false;
        }

        if (!VERIFY_IS_TRUE(JsCreateRuntime(JsRuntimeAttributeAllowScriptInterrupt, NULL, &this->runtime) == JsNoError))
        {
            return false;
        }
        
        if (!VERIFY_IS_TRUE(JsCreateContext(this->runtime, &this->context) == JsNoError))
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

    bool ComProjectionTest::Cleanup()
    {        
        if (this->runtime != NULL)
        {
            JsSetCurrentContext(NULL);
            JsDisposeRuntime(this->runtime);
        }

        CoUninitialize();

        return true;
    }

    // Basic COM projectsion test. Create an FSO object, call it from script to load some more script
    // and dynamically evaluate it.
    void ComProjectionTest::BasicTest()
    {        
        CComPtr<IUnknown> punk = NULL;        
        VERIFY_IS_TRUE(SUCCEEDED(CoCreateInstance(fsoClsid, NULL, CLSCTX_INPROC_SERVER, __uuidof(IUnknown), (void **)&punk)));
        
        CComVariant var = punk;                
        JsValueRef fso = JS_INVALID_REFERENCE;
        JsValueRef global = JS_INVALID_REFERENCE;

        VERIFY_IS_TRUE(JsVariantToValue(&var, &fso) == JsNoError);
        VERIFY_IS_TRUE(JsGetGlobalObject(&global) == JsNoError);

        JsPropertyIdRef fsoName;

        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"fso", &fsoName) == JsNoError);
        VERIFY_IS_TRUE(JsSetProperty(global, fsoName, fso, true) == JsNoError);        

        LPCWSTR script = LoadScriptFile(L"ComProjectionBasic.js");
        String path = GetCurrentModulePath();
        JsValueRef currentPath = JS_INVALID_REFERENCE;
        JsPropertyIdRef currentPathName;

        VERIFY_IS_TRUE(JsPointerToString(path, wcslen(path), &currentPath) == JsNoError);
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"currentPath", &currentPathName) == JsNoError);
        VERIFY_IS_TRUE(JsSetProperty(global, currentPathName, currentPath, true) == JsNoError);

        JsValueRef result = JS_INVALID_REFERENCE;

        VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);
        LPCWSTR str = NULL;
        size_t length;
        VERIFY_IS_TRUE(JsStringToPointer(result, &str, &length) == JsNoError);
        VERIFY_IS_TRUE(wcscmp(str, L"hello world!") == 0);
    }

    // Tests core marshalling scenarios.
    void ComProjectionTest::IDispatchToObjectTest()
    {
        // Create a test COM object.
        TestComponent * t = new TestComponent();
        CComVariant var = (IDispatch *)t;
        JsValueRef test = JS_INVALID_REFERENCE;
        JsValueRef global = JS_INVALID_REFERENCE;

        // Create a projection object.
        VERIFY_IS_TRUE(JsVariantToValue(&var, &test) == JsNoError);
        VERIFY_IS_TRUE(JsGetGlobalObject(&global) == JsNoError);

        JsPropertyIdRef name;

        // Set the projection object as a global property.
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"test", &name) == JsNoError);
        VERIFY_IS_TRUE(JsSetProperty(global, name, test, true) == JsNoError);
        
        // Call script that sets and gets properties of various kinds on the
        // projection object.
        LPCWSTR script = LoadScriptFile(L"IDispatchToObject.js");
        JsValueRef result = JS_INVALID_REFERENCE;

        VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", &result) == JsNoError);   
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);

        // The previous script set a callback on the projection object. Test that 
        // projection objects (both directions, HostDispatch and JavascriptDispatch)
        // are thread agile.
        DWORD threadId;
        void * state[2] = { t, this->context };
        HANDLE thread = CreateThread(NULL, 0xFFFF, DoIDispatchToObjectTestCallback, &state, 0, &threadId);

        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        
        VERIFY_IS_TRUE(JsSetCurrentContext(this->context) == JsNoError);
        
        t->Release();
    }

    DWORD ComProjectionTest::DoIDispatchToObjectTestCallback(void * state)
    {
        TestComponent * t = (TestComponent *)((void **)state)[0];
        JsContextRef context = (JsContextRef)((void **)state)[1];        
        CComVariant arg1 = L"pi";
        CComVariant arg2 = 3.141592;

        VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);
        VERIFY_SUCCEEDED(t->DoCallback(&arg1, &arg2));
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);

        return 0;
    }

    void ComProjectionTest::ValueToVariantTest()
    {
        // Load a JS test object into a object (JsValueRef)
        LPCWSTR script = LoadScriptFile(L"ValueToVariant.js");
        JsValueRef global = JS_INVALID_REFERENCE;
        JsPropertyIdRef name;
        JsValueRef object = JS_INVALID_REFERENCE;
        CComVariant var;
        CComVariant value;

        VERIFY_IS_TRUE(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", NULL) == JsNoError);

        VERIFY_IS_TRUE(JsGetGlobalObject(&global) == JsNoError);
        VERIFY_IS_TRUE(JsGetPropertyIdFromName(L"test", &name) == JsNoError);
        VERIFY_IS_TRUE(JsGetProperty(global, name, &object) == JsNoError);
        
        // Get var (VARIANT) from object
        VERIFY_IS_TRUE(JsValueToVariant(object, &var) == JsNoError);
        VERIFY_IS_TRUE(var.vt == VT_DISPATCH);

        // Get own and proto properties
        VERIFY_IS_TRUE(GetDispatchProperty(var.pdispVal, L"num1", &value));
        VERIFY_IS_TRUE(value.vt == VT_R8);
        VERIFY_IS_TRUE(value.dblVal == 3.141592);

        VERIFY_IS_TRUE(GetDispatchProperty(var.pdispVal, L"str1", &value));
        VERIFY_IS_TRUE(value.vt == VT_BSTR);
        VERIFY_IS_TRUE(value == CComVariant("world"));

        VERIFY_IS_TRUE(GetDispatchProperty(var.pdispVal, L"num", &value));
        VERIFY_IS_TRUE(value.vt == VT_I4);
        VERIFY_IS_TRUE(value.lVal == 123456);

        VERIFY_IS_TRUE(GetDispatchProperty(var.pdispVal, L"str", &value));
        VERIFY_IS_TRUE(value.vt == VT_BSTR);
        VERIFY_IS_TRUE(value == CComVariant("hello"));   

        VERIFY_IS_TRUE(GetDispatchProperty(var.pdispVal, L"bool", &value));
        VERIFY_IS_TRUE(value.vt == VT_BOOL);
        VERIFY_IS_TRUE(value.boolVal == VARIANT_TRUE);

        // Set own and proto properties
        value = "999999";
        
        VERIFY_IS_TRUE(SetDispatchProperty(var.pdispVal, L"num", &value));
        VERIFY_IS_TRUE(SetDispatchProperty(var.pdispVal, L"num1", &value));

        CComQIPtr<IDispatchEx> dispEx = var.pdispVal;
        VERIFY_IS_TRUE(SetDispatchProperty(dispEx, L"num2", &value));

        // Call a function on var from a different thread
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);

        DWORD threadId;
        void * state[2] = { &var, this->context };
        HANDLE thread = CreateThread(NULL, 0xFFFF, DoValueToVariantTestCallback, &state, 0, &threadId);

        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);

        VERIFY_IS_TRUE(JsSetCurrentContext(this->context) == JsNoError);
    }

    DWORD ComProjectionTest::DoValueToVariantTestCallback(void * state)
    {
        VARIANT * var = (VARIANT *)((void **)state)[0];
        JsContextRef context = (JsContextRef)((void **)state)[1];        
        CComVariant args[] = { L"it", "do" };
        CComVariant result;
        
        VERIFY_IS_TRUE(JsSetCurrentContext(context) == JsNoError);
        VERIFY_IS_TRUE(InvokeDispatchMethod(var->pdispVal, L"func", args, 2, &result));
        VERIFY_IS_TRUE(result.vt == VT_BSTR);
        VERIFY_IS_TRUE(result == CComVariant("doit"));
        VERIFY_IS_TRUE(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);

        return 0;
    }
    
    bool ComProjectionTest::SetDispatchProperty(IDispatch * object, BSTR name, VARIANT * value)
    {
        DISPID id;
        DISPID idNamed = DISPID_PROPERTYPUT;
        DISPPARAMS params = { value, &idNamed, 1, 1 };

        VERIFY_SUCCEEDED(object->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &id));        
        VERIFY_SUCCEEDED(object->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &params, value, NULL, NULL));  

        return true;
    }

    bool ComProjectionTest::SetDispatchProperty(IDispatchEx * object, BSTR name, VARIANT * value)
    {
        DISPID id;
        DISPID idNamed = DISPID_PROPERTYPUT;
        DISPPARAMS params = { value, &idNamed, 1, 1 };

        VERIFY_SUCCEEDED(object->GetDispID(name, fdexNameEnsure | fdexNameCaseSensitive , &id));
        VERIFY_SUCCEEDED(object->InvokeEx(id, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL));        

        return true;
    }

    bool ComProjectionTest::GetDispatchProperty(IDispatch * object, BSTR name, VARIANT * value)
    {
        DISPID id;       
        DISPPARAMS params = { NULL, NULL, 0, 0 };

        VERIFY_SUCCEEDED(object->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &id));        
        VERIFY_SUCCEEDED(object->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, value, NULL, NULL));

        return true;
    }

    bool ComProjectionTest::InvokeDispatchMethod(IDispatch * object, BSTR name, VARIANT * args, int cargs, VARIANT * result)
    {
        DISPID id;       
        DISPPARAMS params = { args, NULL, cargs, 0 };

        VERIFY_SUCCEEDED(object->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &id));
        VERIFY_SUCCEEDED(object->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, result, NULL, NULL));

        return true;
    }
}
