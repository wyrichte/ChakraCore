
#include "stdafx.h"
#include "ScriptDirectUnitTests.h"

Var MyObjectConstructor(Var function, CallInfo callInfo, Var* args)
{
    return args[0];
}

Var MyCallerTest(Var function, CallInfo callInfo, Var* args)
{
    IServiceProvider* serviceProvider;
    IActiveScriptDirect* scriptDirect = NULL;
    HRESULT hr = scriptDirect->GetServiceProvider(&serviceProvider);
    if (FAILED(hr))
    {
        return NULL;
    }
    serviceProvider->Release();
    return NULL;

}

void PrintVar(IActiveScriptDirect* activeScriptDirect, Var varToPrint)
{
    HRESULT hr;
    BSTR bstr;
    hr = activeScriptDirect->VarToString(varToPrint, &bstr);
    if (SUCCEEDED(hr))
    {
        printf("%S\n", bstr);
        SysFreeString(bstr);
    }
    else
    {
        Print("VarToString failed");
    }
}

HRESULT TestBasicFastDOM(IActiveScriptDirect* activeScriptDirect)
{
    HRESULT hr;
    Var newVar = NULL;
    CallInfo callInfo = {0, CallFlags_None};
    BOOL wasPropertySet = FALSE, wasPropertyPresent = FALSE;
    Var topFunc;
    hr = activeScriptDirect->Parse(L"new Number(1234567);", &topFunc);
    IfFailedReturn(hr);
    Var varResult;
    hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*servicerProvider*/ NULL, &varResult);
    IfFailedReturn(hr);
    PrintVar(activeScriptDirect, varResult);

    ITypeOperations* defaultScriptOperations;
    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    IfFailedReturn(hr);

    Var functionName;
    LPWSTR funName = L"my function name";
    hr = activeScriptDirect->StringToVar(funName, static_cast<int>(wcslen(funName)), &functionName);
    IfFailedReturn(hr);

    PropertyId constructorId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"constructor", &constructorId);
    IfFailedReturn(hr);

    Var constructor;
    hr = activeScriptDirect->CreateConstructor(NULL, MyObjectConstructor, constructorId, FALSE, &constructor);
    IfFailedReturn(hr);

    PropertyId prototypeId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"prototype", &prototypeId);
    IfFailedReturn(hr);

    BOOL wasProtoAvailable = FALSE;
    Var prototype;
    hr = defaultScriptOperations->GetOwnProperty(activeScriptDirect, constructor, prototypeId, &prototype, &wasProtoAvailable);
    if (FAILED(hr) || !wasProtoAvailable)
    {
        return hr;
    }

    HTYPE externalType;
    hr = activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, prototype, NULL, NULL, FALSE, constructorId, false, &externalType);
    IfFailedReturn(hr);

    Var externalVar;
    hr = activeScriptDirect->CreateTypedObject(externalType, 0, FALSE, &externalVar);
    IfFailedReturn(hr);

    Var globalObject;
    hr = activeScriptDirect->GetGlobalObject((Var*)&globalObject);
    IfFailedReturn(hr);

    CComPtr<IJavascriptOperations> jsOps;
    hr=activeScriptDirect->GetJavascriptOperations(&jsOps);

    IfFailedReturn(hr);

    PropertyId testInstanceOfId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"testInstanceOf", &testInstanceOfId);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = jsOps->SetProperty(activeScriptDirect, globalObject, testInstanceOfId, topFunc);
    if (FAILED(hr))
    {
        return hr;
    }

    Var callerTestFunc;
    hr = activeScriptDirect->Parse(L"(function f2(){if (testInstanceOf instanceof testInstanceOf) return TRUE;})()", &callerTestFunc);
    IfFailedReturn(hr);

    hr = activeScriptDirect->Execute(callerTestFunc, callInfo, NULL, /*serviceProvider*/ NULL,&varResult);
    IfFailedReturn(hr);
    PrintVar(activeScriptDirect, varResult);

    JavascriptTypeId typeId1;
    JavascriptTypeId typeId2;

    if (FAILED(activeScriptDirect->GetTypeIdForType(externalType, &typeId1)) ||
        FAILED(activeScriptDirect->GetTypeIdForVar(externalVar, &typeId2)) ||
        typeId1 != typeId2)
    {
        hr = E_FAIL;
        return hr;
    }

    PropertyId propertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"hello", &propertyId);
    IfFailedReturn(hr);

    LPCWSTR propertyName;
    hr = activeScriptDirect->GetPropertyName(propertyId, &propertyName);
    printf("propertyname is %S \n", propertyName);

    printf("throug interface access\n");
    hr = defaultScriptOperations->SetProperty(activeScriptDirect, externalVar, propertyId,topFunc, &wasPropertySet);
    IfFailedReturn(hr);

    if (wasPropertySet)
    {
        hr = defaultScriptOperations->GetOwnProperty(activeScriptDirect, externalVar, propertyId, &newVar, &wasPropertyPresent);
        IfFailedReturn(hr);
        PrintVar(activeScriptDirect, newVar);
    }

    if (wasPropertyPresent)
    {
        BOOL isSameObject;
        hr = jsOps->Equals(activeScriptDirect, topFunc, newVar, &isSameObject);
        IfFailedReturn(hr);
        if (isSameObject)
        {
            printf("correct: same object\n");
        }
        else
        {
            printf("incorrect retrival of object\n");
        }
        BOOL enumerable;
        hr = defaultScriptOperations->IsEnumerable(activeScriptDirect, externalVar, propertyId, &enumerable);
        printf("isnumerable =%d\n", enumerable);

        BOOL wasPropertyDelete;
        hr = defaultScriptOperations->DeleteProperty(activeScriptDirect, externalVar, propertyId, &wasPropertyDelete);
    }
    else
    {
        printf("incorrect: can't find property");
    }


    // Create a function object that points to the MyCallerTest built-in.
    Var functionVar;
    hr = activeScriptDirect->CreateConstructor(NULL, MyCallerTest, constructorId, FALSE, &functionVar);
    IfFailedReturn(hr);

    // Add it to the GlobalObject.
    hr = activeScriptDirect->GetOrAddPropertyId(L"MyCallerTest", &propertyId);
    IfFailedReturn(hr);

    hr = defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, propertyId, functionVar, &wasPropertySet);
    if (FAILED(hr) || !wasPropertySet)
    {
        return hr;
    }

    hr = activeScriptDirect->SetHostObject(externalVar, externalVar);
    if (FAILED(hr))
    {
        printf("SetHostObject failed: %x \n", hr);
        return hr;
    }

    Var hostObject;
    hr = activeScriptDirect->GetHostObject(&hostObject);
    if (FAILED(hr))
    {
        printf("GetHostObject failed: %x \n", hr);
        return hr;
    }

    if (hostObject != externalVar)
    {
        hr = E_FAIL;
        return hr;
    }

    Var jsFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(NULL, MyCallerTest, propertyId, -1, 0, &jsFunction);
    IfFailedReturn(hr);

    BSTR functionBSTRName;
    hr = activeScriptDirect->VarToString(jsFunction, &functionBSTRName);
    if (FAILED(hr))
    {
        return hr;
    }

    printf("direct function name is %ls\n", functionBSTRName);
    SysFreeString(functionBSTRName);

    hr = activeScriptDirect->VarToString(functionVar, &functionBSTRName);
    IfFailedReturn(hr);

    printf("constrctor function name is %ls\n", functionBSTRName);
    SysFreeString(functionBSTRName);

    hr = activeScriptDirect->VarToString(externalVar, &functionBSTRName);
    IfFailedReturn(hr);

    printf("createtypedobject function name is %ls\n", functionBSTRName);
    SysFreeString(functionBSTRName);


    // test for FTL inherited typeId for instanceOf type validation

    {
        uint slot_idx = 1;
        PropertyId propId = 0x456;
        const int inheritedIds[] = { 0x2000, 0x3000, 0x4000 };
    hr = activeScriptDirect->CreateType(TypeId_Unspecified, inheritedIds, 3, prototype, NULL, defaultScriptOperations, FALSE, propId, false, &externalType);
        hr = activeScriptDirect->CreateType(0x2000, inheritedIds, 3, prototype, NULL, defaultScriptOperations, FALSE, propId, false, &externalType);
        IfFailedReturn(hr);

    hr = activeScriptDirect->CreateTypedObject(externalType, sizeof(void*)*(slot_idx+1), FALSE, &externalVar);
        hr = activeScriptDirect->CreateTypedObject(externalType, sizeof(void*)*(slot_idx + 1), FALSE, &externalVar);
        IfFailedReturn(hr);


        void* slotAddr = nullptr;
        JavascriptTypeId typeId;
        hr = activeScriptDirect->VarToExtension(externalVar, &slotAddr, &typeId);
        IfFailedReturn(hr);

        ((void**)slotAddr)[slot_idx] = jsFunction;

    {
        Var getter, setter;
        hr = activeScriptDirect->GetTypedObjectSlotAccessor(0x3000, propId, slot_idx, &getter, &setter);
        IfFailedReturn(hr);

        hr = defaultScriptOperations->SetAccessors(activeScriptDirect, externalVar, propId, getter, setter);
        IfFailedReturn(hr);

        Var value;
        BOOL exist = FALSE;
        hr = defaultScriptOperations->GetPropertyReference(activeScriptDirect, externalVar, propId, &value, &exist);
        IfFailedReturn(hr);

        if (value != jsFunction)
        {
            return E_FAIL;
        }
        printf("inherited typeid test passed 1\n");
    }

    {
        uint slot_idx = 1;
        PropertyId propId = 0x567;
        const int inheritedIds[] = { 0x2000, 0x3000, 0x4000 };
        hr = activeScriptDirect->CreateType(0x2001, inheritedIds, 3, prototype, NULL, defaultScriptOperations, FALSE, propId, false, &externalType);
        IfFailedReturn(hr);

        hr = activeScriptDirect->CreateTypedObject(externalType, sizeof(void*)*(slot_idx + 1), FALSE, &externalVar);
        IfFailedReturn(hr);


        void* slotAddr = nullptr;
        JavascriptTypeId typeId;
        hr = activeScriptDirect->VarToExtension(externalVar, &slotAddr, &typeId);
        IfFailedReturn(hr);

        ((void**)slotAddr)[slot_idx] = jsFunction;
        Var getter, setter;
        hr = activeScriptDirect->GetTypedObjectSlotAccessor(0x3001, propId, slot_idx, &getter, &setter);
        IfFailedReturn(hr);

        hr = defaultScriptOperations->SetAccessors(activeScriptDirect, externalVar, propId, getter, setter);
        IfFailedReturn(hr);

        Var value;
        BOOL exist = FALSE;
        hr = defaultScriptOperations->GetPropertyReference(activeScriptDirect, externalVar, propId, &value, &exist);
        if (hr == S_OK)
        {
            // expect fail
            return E_FAIL;
        }
        printf("inherited typeid test passed 2\n");
    }

    return NOERROR;
};

HRESULT TestDOMToString(IActiveScriptDirect* activeScriptDirect)
{
    Var constructor;
    HRESULT hr;
    HTYPE prototypeHandle;
    PropertyId propertyId;
    BOOL wasPropertySet;
    CallInfo callInfo = {0, CallFlags_None};
    printf("test dom toString\n");
    ITypeOperations* defaultScriptOperations;
    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    IfFailedReturn(hr);

    // Add it to the GlobalObject.
    hr = activeScriptDirect->GetOrAddPropertyId(L"my class", &propertyId);
    IfFailedReturn(hr);

    hr = activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, NULL, NULL, NULL, FALSE, propertyId, false, &prototypeHandle);
    IfFailedReturn(hr);

    Var prototype;
    hr = activeScriptDirect->CreateTypedObject(prototypeHandle, 0, FALSE, &prototype);
    IfFailedReturn(hr);

    hr = activeScriptDirect->GetOrAddPropertyId(L"my class", &propertyId);
    IfFailedReturn(hr);

    hr = activeScriptDirect->CreateConstructor(prototype, NULL, propertyId, FALSE, &constructor);
    IfFailedReturn(hr);

    Var instanceObject;
    hr = activeScriptDirect->CreateTypedObject(prototypeHandle, 0, FALSE, &instanceObject);
    IfFailedReturn(hr);

    Var  globalObject;
    hr = activeScriptDirect->GetGlobalObject((Var*)&globalObject);
    IfFailedReturn(hr);

    // Add it to the GlobalObject.
    hr = activeScriptDirect->GetOrAddPropertyId(L"MytoStringTest", &propertyId);
    IfFailedReturn(hr);
    hr = defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, propertyId, constructor, &wasPropertySet);
    IfFailedReturn(hr);

        // Add it to the GlobalObject.
    hr = activeScriptDirect->GetOrAddPropertyId(L"toStringInstance", &propertyId);
    IfFailedReturn(hr);
    hr = defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, propertyId, instanceObject, &wasPropertySet);
    if (FAILED(hr) || !wasPropertySet)
    {
        goto LError;
    }


    Var topFunc;
    hr = activeScriptDirect->Parse(L"WScript.Echo(MytoStringTest.toString());WScript.Echo(MytoStringTest.prototype.toString()); WScript.Echo(toStringInstance.toString());", &topFunc);
    IfFailedReturn(hr);
    Var varResult;
    hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*serviceProvider*/ NULL, &varResult);
    IfFailedReturn(hr);
    PrintVar(activeScriptDirect, varResult);

LError:
    return hr;

}

HRESULT TestArray(IActiveScriptDirect* activeScriptDirect)
{
    HRESULT hr = E_FAIL;
    Var arrayInstance;
    PropertyId propertyId;
    BOOL wasPropertySet;
    CallInfo callInfo = {0, CallFlags_None};
    printf("test array\n");
    ITypeOperations* defaultScriptOperations;
    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    if (FAILED(hr))
    {
        return hr;
    }

    IfFailedReturn(activeScriptDirect->CreateArrayObject(20, &arrayInstance));

    Var  globalObject;
    IfFailedReturn(activeScriptDirect->GetGlobalObject((Var*)&globalObject));

    // Add it to the GlobalObject.
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(L"arrayInstance", &propertyId));
    hr = defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, propertyId, arrayInstance, &wasPropertySet);
    if (!wasPropertySet)
    {
        hr = E_FAIL;
    }

    IfFailedReturn(hr);

    Var topFunc;
    IfFailedReturn(activeScriptDirect->Parse(L"var sum =0; for (i = 0; i < 20; i++) {arrayInstance[i] = i; sum += arrayInstance[i]}; WScript.Echo('sum is' + sum);", &topFunc));
    Var varResult;
    hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*serviceProvider*/ NULL, &varResult);

    IfFailedReturn(hr);
    BOOL isArray;
    hr = activeScriptDirect->IsArrayObject(arrayInstance, &isArray);
    IfFailedReturn(hr);
    if (!isArray)
    {
        return E_FAIL;
    }

    IfFailedReturn(activeScriptDirect->IsArrayObject(globalObject, &isArray));
    if (isArray)
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT PrintDate(IActiveScriptDirect* activeScriptDirect, Var varDate)
{
    HRESULT hr = NOERROR;

    // Print Date.toISOString to be locale-independent
    Var toISOString;
    {
        Var topFunc;
        hr = activeScriptDirect->Parse(L"Date.prototype.toISOString", &topFunc);
        IfFailedReturn(hr);

        CallInfo callInfo = {0, CallFlags_None};
        hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*servicerProvider*/NULL, &toISOString);
        IfFailedReturn(hr);
    }

    Var strDate;
    {
        Var args[1] = {varDate};
        CallInfo callInfo = {_countof(args), CallFlags_None};
        hr = activeScriptDirect->Execute(toISOString, callInfo, args, /*servicerProvider*/NULL, &strDate);
        IfFailedReturn(hr);
    }

    PrintVar(activeScriptDirect, strDate);
    return hr;
}

HRESULT TestVarToDate(IActiveScriptDirect* activeScriptDirect)
{
    printf("test VarToDate/DateToVar\n");
    HRESULT hr = NOERROR;
    CallInfo callInfo = {0, CallFlags_None};

    Var topFunc;
    hr = activeScriptDirect->Parse(L"new Date('2011-06-20T22:54:51.244Z')", &topFunc);
    IfFailedReturn(hr);

    Var varDate;
    hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*servicerProvider*/NULL, &varDate);
    IfFailedReturn(hr);

    // Print initial Date
    PrintDate(activeScriptDirect, varDate);

    // Verify VarToDate returns E_INVALIDARG when called on non-JavascriptDate object
    {
        double d;
        hr = activeScriptDirect->VarToDate(topFunc, &d);
        if (hr != E_INVALIDARG)
        {
            printf("Fail: VarToDate did not return E_INVALIDARG on a non-JavascriptDate object: %x\n", hr);
        }
    }

    // Verify VarToDate result is the same as ToDouble conversion
    double d1;
    hr = activeScriptDirect->VarToDouble(varDate, &d1);
    IfFailedReturn(hr);

    double d2;
    hr = activeScriptDirect->VarToDate(varDate, &d2);
    IfFailedReturn(hr);

    if (d1 != d2)
    {
        printf("Fail: VarToDouble and VarToDate results are different -- %lf %lf\n", d1, d2);
    }

    // Verify round-trip to the same date
    hr = activeScriptDirect->DateToVar(d2, &varDate);
    IfFailedReturn(hr);
    PrintDate(activeScriptDirect, varDate);

    return hr;
}

HRESULT TestSYSTEMTIMEConversions(IActiveScriptDirect* activeScriptDirect)
{
    printf("test VarToSYSTEMTIME/VarToSYSTEMTIME\n");
    HRESULT hr = NOERROR;
    Var varDate = NULL;
    SYSTEMTIME t1, t2;

#define IfNotInvalidArgReturn(FNAME, EXPR)                                                     \
    do {                                                                                       \
        hr = (EXPR);                                                                           \
        if (hr != E_INVALIDARG)                                                                \
        {                                                                                      \
            printf("Fail: "FNAME" did not return E_INVALIDARG on a NULL parameter: %x\n", hr); \
            if (varDate != NULL) PrintDate(activeScriptDirect, varDate);                       \
            return hr;                                                                         \
        }                                                                                      \
    } while(FALSE) // End #define

    GetLocalTime(&t1);

    // Verify SYSTEMTIMEToVar returns E_INVALIDARG when called with NULL arguments
    hr = activeScriptDirect->SYSTEMTIMEToVar(NULL, NULL);
    IfNotInvalidArgReturn("SYSTEMTIMEToVar", hr);
    hr = activeScriptDirect->SYSTEMTIMEToVar(&t1, NULL);
    IfNotInvalidArgReturn("SYSTEMTIMEToVar", hr);
    hr = activeScriptDirect->SYSTEMTIMEToVar(NULL, &varDate);
    IfNotInvalidArgReturn("SYSTEMTIMEToVar", hr);

    hr = activeScriptDirect->SYSTEMTIMEToVar(&t1, &varDate);
    IfFailedReturn(hr);

    // Convert to double and back again (ensure the SYSTEMTIME functions aren't doing any extra conversion)
    double d;
    hr = activeScriptDirect->VarToDate(varDate, &d);
    IfFailedReturn(hr);
    hr = activeScriptDirect->DateToVar(d, &varDate);

    // Verify SYSTEMTIMEToVar returns E_INVALIDARG when called with NULL arguments
    hr = activeScriptDirect->VarToSYSTEMTIME(NULL, NULL);
    IfNotInvalidArgReturn("VarToSYSTEMTIME", hr);
    hr = activeScriptDirect->VarToSYSTEMTIME(varDate, NULL);
    IfNotInvalidArgReturn("VarToSYSTEMTIME", hr);
    hr = activeScriptDirect->VarToSYSTEMTIME(NULL, &t2);
    IfNotInvalidArgReturn("VarToSYSTEMTIME", hr);

    // Verify VarToSystemTime only accepts date objects
    Var notADate;
    hr = activeScriptDirect->Parse(L"new Number(1234567);", &notADate);
    IfFailedReturn(hr);
    hr = activeScriptDirect->VarToSYSTEMTIME(notADate, &t2);
    IfNotInvalidArgReturn("VarToSYSTEMTIME", hr);

    hr = activeScriptDirect->VarToSYSTEMTIME(varDate, &t2);
    IfFailedReturn(hr);

    // Verify conversions give us back the same time as the original
    if (memcmp(&t1, &t2, sizeof(SYSTEMTIME) != 0))
    {
        printf("SYSTEMTIMEToVar / VarToSYSTEMTIME conversions FAILED\n");
        return E_FAIL;
    }
    return hr;
#undef IfNotInvalidArgReturn
}

static HRESULT ParseAndExecute(IActiveScriptDirect* activeScriptDirect, LPCWSTR src, Var* result)
{
    HRESULT hr = S_OK;
    CallInfo callInfo = {0, CallFlags_None};
    Var topFunc;

    IfFailedReturn(activeScriptDirect->Parse(const_cast<LPWSTR>(src), &topFunc));
    IfFailedReturn(activeScriptDirect->Execute(topFunc, callInfo, NULL, /*servicerProvider*/NULL, result));
    return hr;
}
static HRESULT AssertIsPrimitiveType(IActiveScriptDirect* activeScriptDirect, LPCWSTR msg, Var instance, BOOL isPrimitiveType)
{
    HRESULT hr = S_OK;

    BOOL res;
    IfFailedReturn(activeScriptDirect->IsPrimitiveType(instance, &res));

    if (res != isPrimitiveType)
    {
        printf("Fail: IsPrimitiveType(%S) expected %s, actual %s\n", msg,
            isPrimitiveType ? "TRUE" : "FALSE", res ? "TRUE" : "FALSE");
    }
    return hr;
}
static HRESULT AssertIsPrimitiveType(IActiveScriptDirect* activeScriptDirect, LPCWSTR msg, LPCWSTR src, BOOL isPrimitiveType)
{
    HRESULT hr = S_OK;
    Var var = NULL;

    IfFailedReturn(ParseAndExecute(activeScriptDirect, src, &var));
    IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, msg, var, isPrimitiveType));
    return hr;
}
static HRESULT AssertIsPrimitiveType(IActiveScriptDirect* activeScriptDirect, LPCWSTR src, BOOL isPrimitiveType)
{
    HRESULT hr = S_OK;
    Var var = NULL;

    IfFailedReturn(ParseAndExecute(activeScriptDirect, src, &var));
    CComBSTR bstr;
    IfFailedReturn(activeScriptDirect->VarToString(var, &bstr));
    IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, bstr, var, isPrimitiveType));
    return hr;
}
static HRESULT AssertIsPrimitiveType(IActiveScriptDirect* activeScriptDirect,
                                     _In_reads_(count) LPCWSTR* src, int count, BOOL isPrimitiveType)
{
    HRESULT hr = S_OK;
    for (int i = 0; i < count; i++) {
        IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, src[i], isPrimitiveType));
    }
    return hr;
}
static HRESULT TestIsPrimitiveType(IActiveScriptDirect* activeScriptDirect)
{
    printf("test IsPrimitiveType\n");
    HRESULT hr = NOERROR;

    LPCWSTR primitives[] = {
        L"undefined", L"null", L"true", L"false",
        L"0", L"+0", L"-0", L"123",
        L"1.5", L"-123.4", L"Number.NaN", L"Infinity", L"-Infinity",
        L"\"\"", L"\"string value\""
    };
    IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, primitives, _countof(primitives), TRUE));

    Var var;
    IfFailedReturn(activeScriptDirect->Int64ToVar(0x10F0F0F0Fll, &var));
    IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, L"int64", var, TRUE));
    IfFailedReturn(activeScriptDirect->UInt64ToVar(0x10F0F0F0Full, &var));
    IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, L"uint64", var, TRUE));

    LPCWSTR nonPrimitives[] = {
        L"new Boolean(true)", L"new Boolean(false)",
        L"new Number(123)",
        L"new Number(-123.5)",
        L"new String(\"string object\")",
        L"({})", L"[0, 1]", L"new Object()", L"new Array()", L"new Date('2011-06-20T22:54:51.244Z')"
    };
    IfFailedReturn(AssertIsPrimitiveType(activeScriptDirect, nonPrimitives, _countof(nonPrimitives), FALSE));

    return hr;
}

Var ObjectEntryPoint(Var method, CallInfo callInfo, Var* args)
{
    Print("we are in the object entry\n");
    return args[0];
}

static HRESULT TestCallable(IActiveScriptDirect* activeScriptDirect)
{
    HRESULT hr;
    HTYPE typeRef = nullptr;
    Var instance = nullptr;
    PropertyId propertyId;
    Print("Test callable object");
    hr = activeScriptDirect->GetOrAddPropertyId(L"foo", &propertyId);
    if (SUCCEEDED(hr))
    {
        hr = activeScriptDirect->CreateType(0, nullptr, 0, nullptr, ObjectEntryPoint, nullptr, false, propertyId, true, &typeRef);
    }

    if (SUCCEEDED(hr))
    {
        hr = activeScriptDirect->CreateTypedObject(typeRef, 0, true, &instance);
    }

    Var varResult = nullptr;
    if (SUCCEEDED(hr))
    {
        Var args[1];
        args[0] = instance;
        CallInfo callInfo = {1, CallFlags_None};
        hr = activeScriptDirect->Execute(instance, callInfo, args, nullptr, &varResult);
    }


    Print("Test non-callable object");
    if (SUCCEEDED(hr))
    {
        hr = activeScriptDirect->CreateObject(&instance);
    }

    if (SUCCEEDED(hr))
    {
        Var args[1];
        args[0] = instance;
        CallInfo callInfo = {1, CallFlags_None};
        hr = activeScriptDirect->Execute(instance, callInfo, args, nullptr, &varResult);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            Print("FAILED");
        }
        else
        {
            hr = NOERROR;
        }
    }

    return hr;
}

void RunBasicTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    HRESULT hr;
    try
    {
        Print("Test basic IActiveScirptDirect functionality");
        hr = TestBasicFastDOM(mytest->GetScriptDirectNoRef());
        if (SUCCEEDED(hr))
        {
            Print("Test ToString for fastdom");
            hr = TestDOMToString(mytest->GetScriptDirectNoRef());
        }

        if (SUCCEEDED(hr))
        {
            Print("Test array");
            hr = TestArray(mytest->GetScriptDirectNoRef());
        }

        if (SUCCEEDED(hr))
        {
            hr = TestVarToDate(mytest->GetScriptDirectNoRef());
        }

        if (SUCCEEDED(hr))
        {
            hr = TestSYSTEMTIMEConversions(mytest->GetScriptDirectNoRef());
        }

        if (SUCCEEDED(hr))
        {
            hr = TestIsPrimitiveType(mytest->GetScriptDirectNoRef());
        }

        if (SUCCEEDED(hr))
        {
            hr = TestCallable(mytest->GetScriptDirectNoRef());
        }


        if (FAILED(hr))
        {
            Print("Test failed", false);
        }
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
}

