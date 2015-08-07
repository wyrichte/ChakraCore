/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "StdAfx.h"
#include "..\..\lib\winrt\autoHSTRING.h"
#include "..\..\Lib\Common\Memory\AutoPtr.h"

[uuid("63109BE8-1A17-4abc-9F19-AF4A3AA7AC1B")]
class IWineryFactory : public IInspectable
{
public:
    STDMETHOD(CreateWinery)(__in int val, __deref_out IInspectable **ppWinery);
};

DelayLoadWinRt * TestUtilities::m_WinRTLibrary = NULL;
Js::DelayLoadWinRtString * TestUtilities::m_WinRTStringLibrary = NULL;

IInspectable * TestUtilities::ActivateRuntimeClassInstance(const WCHAR * runtimeClassName)
{
    if ((m_WinRTStringLibrary != nullptr) && (m_WinRTStringLibrary->IsAvailable())
        && (m_WinRTLibrary != nullptr) && (m_WinRTLibrary->IsAvailable()))
    {
        HSTRING typeName = nullptr;
        IActivationFactory* pActivationFactory = nullptr;
        IInspectable* pInspectable = nullptr;

        HRESULT hr = m_WinRTStringLibrary->WindowsCreateString(runtimeClassName, (UINT32)wcslen(runtimeClassName), &typeName);
        if (FAILED(hr))
        {
            return nullptr;
        }
        AutoHSTRING typeNameStr(m_WinRTStringLibrary);
        typeNameStr.Initialize(typeName);

        hr = m_WinRTLibrary->GetActivationFactory(typeNameStr.Get(), __uuidof(IActivationFactory), (LPVOID *)&pActivationFactory);
        if (FAILED(hr))
        {
            return nullptr;
        }

        hr = pActivationFactory->ActivateInstance(&pInspectable);
        if (FAILED(hr))
        {
            return nullptr;
        }
        return pInspectable;
    }

    return nullptr;
}

IInspectable * TestUtilities::GetAnimalInstance()
{
    return ActivateRuntimeClassInstance(L"Animals.Animal");
}

IInspectable * TestUtilities::GetRestrictedErrorAccessInstance()
{
    return ActivateRuntimeClassInstance(L"Winery.WinRTErrorTests.RestrictedErrorAccessInstance");
}

IInspectable * TestUtilities::GetRWineryInstance()
{
    static const WCHAR RuntimeClassName[] = L"Winery.RWinery";

    if ((m_WinRTStringLibrary != nullptr) && (m_WinRTStringLibrary->IsAvailable())
        && (m_WinRTLibrary != nullptr) && (m_WinRTLibrary->IsAvailable()))
    {
        HSTRING typeName = nullptr;
        IWineryFactory* pActivationFactory = nullptr;
        IInspectable* pInspectable = nullptr;

        HRESULT hr = m_WinRTStringLibrary->WindowsCreateString(RuntimeClassName, (UINT32)wcslen(RuntimeClassName), &typeName);
        if (FAILED(hr))
        {
            return nullptr;
        }
        AutoHSTRING typeNameStr(m_WinRTStringLibrary);
        typeNameStr.Initialize(typeName);

        hr = m_WinRTLibrary->GetActivationFactory(typeNameStr.Get(), __uuidof(IWineryFactory), (LPVOID *)&pActivationFactory);
        if (FAILED(hr))
        {
            return nullptr;
        }

        hr = pActivationFactory->CreateWinery(1, &pInspectable);
        if (FAILED(hr))
        {
            return nullptr;
        }
        return pInspectable;
    }

    return nullptr;
}

Var TestUtilities::ActivateRuntimeClassAndConvertToVar(RuntimeClassFactoryFunction fnFactory, Var function, CallInfo callInfo, Var dynamo)
{
    IActiveScriptDirect * scriptDirect = nullptr;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return nullptr;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return nullptr;
    }

    if ((m_WinRTStringLibrary != nullptr) && (m_WinRTStringLibrary->IsAvailable())
        && (m_WinRTLibrary != nullptr) && (m_WinRTLibrary->IsAvailable()))
    {
        Var varInstance;
        IInspectable * pInspectable = fnFactory();
        if (pInspectable != nullptr)
        {
            HRESULT hr = scriptDirect->InspectableUnknownToVar(pInspectable, &varInstance);
            pInspectable->Release();
            if (SUCCEEDED(hr))
            {
                return varInstance;
            }
        }
    }
    return nullObject;
}

Var TestUtilities::AnimalToVar(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    return ActivateRuntimeClassAndConvertToVar(&GetAnimalInstance, function, callInfo, dynamo);
}

Var TestUtilities::VectorIntToVar(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return NULL;
    }

    if ((m_WinRTStringLibrary != NULL) && (m_WinRTStringLibrary->IsAvailable()))
    {
        Var vector;
        Animals::IGetVector * pAnimal = NULL;
        __FIVector_1_int * pVector = NULL;

        IInspectable * pInspectableAnimal = GetAnimalInstance();
        if (pInspectableAnimal == nullptr) { return nullObject; }

        HRESULT hr = pInspectableAnimal->QueryInterface(__uuidof(Animals::IGetVector), (LPVOID *)&pAnimal);
        if (FAILED(hr)) { return nullObject; } 

        hr = pAnimal->GetVector(&pVector);
        if (FAILED(hr)) { return nullObject; } 

        hr = scriptDirect->InspectableUnknownToVar(pVector, &vector);
        if (FAILED(hr)) { return nullObject; } 
        
        return vector;
    }

    return nullObject;
}

Var TestUtilities::GetRestrictedStringFromError(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var undefinedObject;
    if (FAILED(scriptDirect->GetUndefined(&undefinedObject)))
    {
        return NULL;
    }

    HRESULT hr;

    args = &args[1];

    if (callInfo.Count < 2) { return undefinedObject; }
    BSTR errorSz;
    Var errorString;
    Var errorVar = args[0];
    hr = JScript9Interface::GetRestrictedString(errorVar, &errorSz);
    if (nullptr != errorSz)
    {
        HRESULT hr = scriptDirect->StringToVar(errorSz, SysStringLen(errorSz), &errorString);
        SysFreeString(errorSz);
        if (FAILED(hr)) { return undefinedObject; }
        return errorString;
    }
    
    return undefinedObject;
}

Var TestUtilities::GetCapabilitySidFromError(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var undefinedObject;
    if (FAILED(scriptDirect->GetUndefined(&undefinedObject)))
    {
        return NULL;
    }

    HRESULT hr;

    args = &args[1];

    if (callInfo.Count < 2) { return undefinedObject; }
    BSTR capabilitySz;
    Var capabilityString;
    Var errorVar = args[0];
    hr = JScript9Interface::GetCapabilitySid(errorVar, &capabilitySz);
    if (nullptr != capabilitySz)
    {
        HRESULT hr = scriptDirect->StringToVar(capabilitySz, SysStringLen(capabilitySz), &capabilityString);
        SysFreeString(capabilitySz);
        if (FAILED(hr)) { return undefinedObject; }
        return capabilityString;
    }
    
    return undefinedObject;
}

Var TestUtilities::GetMemoryFootprintOfRC(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var undefinedObject;
    if (FAILED(scriptDirect->GetUndefined(&undefinedObject)))
    {
        return NULL;
    }

    HRESULT hr;

    args = &args[1];

    if (callInfo.Count < 2) { return undefinedObject; }
    INT32 gcPressure;
    Var gcPressureVar;
    LPCWSTR typeNameSz;
    unsigned int typeNameLen;
    hr = scriptDirect->VarToRawString(args[0], &typeNameSz, &typeNameLen);
    if(FAILED(hr))
    {
        return undefinedObject;
    }
    BOOL result = JScript9Interface::GetMemoryFootprintOfRC(scriptDirect, typeNameSz, &gcPressure);
    if (result)
    {
        hr = scriptDirect->IntToVar(gcPressure, &gcPressureVar);
        if (FAILED(hr)) { return undefinedObject; }
        return gcPressureVar;
    }
    
    return undefinedObject;
}

Var TestUtilities::GetSystemStringFromHr(Var function, CallInfo callInfo, Var* args)
{    
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return NULL;
    }

    HRESULT hr;

    args = &args[1];

    if (callInfo.Count < 2) { return nullObject; }
    Var hrVar = args[0];
    HRESULT hrValue;
    if (FAILED(scriptDirect->VarToInt(hrVar, (__int32*)&hrValue))) { return nullObject; }


    LPWSTR szMsg = JScript9Interface::GetSystemStringFromHr(hrValue);

    if (nullptr == szMsg)
    {
        return nullObject;
    }

    Var result;
    hr = scriptDirect->StringToVar(szMsg, wcslen(szMsg), &result);
    LocalFree(szMsg);
    if (FAILED(hr)) { return nullObject; }
    return result;
}

DevTests::SimpleTestNamespace::ISimpleInterface * TestUtilities::GetAndUpdateSimpleClass(IActiveScriptDirect * scriptDirect, Var instance)
{
    Assert(scriptDirect != nullptr);

    IJavascriptOperations * pOperations = nullptr;
    if (FAILED(scriptDirect->GetJavascriptOperations(&pOperations)))
    {
        return nullptr;
    }

    AutoReleasePtr<IJavascriptOperations> autoReleaseOperations(pOperations);

    DevTests::SimpleTestNamespace::ISimpleInterface * spSimple = nullptr;
    if (FAILED(pOperations->QueryObjectInterface(scriptDirect, instance, __uuidof(DevTests::SimpleTestNamespace::ISimpleInterface), (void **)&spSimple)))
    {
        return nullptr;
    }

    HRESULT hr;

    LPCWSTR strMessageSent = L"Goodbye";
    HSTRING hstrMessage = NULL;
    m_WinRTStringLibrary->WindowsCreateString(strMessageSent, wcslen(strMessageSent), &hstrMessage);
    hr = spSimple->SetMessage(hstrMessage);
    IfFailedGo(hr);
    m_WinRTStringLibrary->WindowsDeleteString(hstrMessage);
    hr = spSimple->GetMessage(&hstrMessage);
    IfFailedGo(hr);
    LPCWSTR strMessageReceived = m_WinRTStringLibrary->WindowsGetStringRawBuffer(hstrMessage, nullptr);
    m_WinRTStringLibrary->WindowsDeleteString(hstrMessage);
    Assert(wcscmp(strMessageSent, strMessageReceived) == 0);

    int value;
    hr = spSimple->put_Value(217);
    IfFailedGo(hr);
    hr = spSimple->get_Value(&value);
    IfFailedGo(hr);
    Assert(value == 217);

    return spSimple;

LReturn:
    spSimple->Release();
    return nullptr;
}

Var TestUtilities::UpdateSimpleClassAndReturnAsVar(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return NULL;
    }

    args = &args[1];

    if (callInfo.Count < 2) { return nullObject; }
    Var instance = args[0];

    DevTests::SimpleTestNamespace::ISimpleInterface * spSimple = GetAndUpdateSimpleClass(scriptDirect, instance);
    if (spSimple == nullptr)
    {
        return nullObject;
    }

    AutoReleasePtr<DevTests::SimpleTestNamespace::ISimpleInterface> autoReleaseSimple(spSimple);

    Var simpleVar;
    if (FAILED(scriptDirect->InspectableUnknownToVar(spSimple, &simpleVar)))
    {
        simpleVar = nullObject;
    }

    return simpleVar;
}

Var TestUtilities::UpdateSimpleClassAndReturnAsVarByAlternateInterface(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return NULL;
    }

    HRESULT hr;

    args = &args[1];

    if (callInfo.Count < 2) { return nullObject; }
    Var instance = args[0];

    DevTests::SimpleTestNamespace::ISimpleInterface * spSimple = GetAndUpdateSimpleClass(scriptDirect, instance);
    if (spSimple == nullptr)
    {
        return nullObject;
    }

    AutoReleasePtr<DevTests::SimpleTestNamespace::ISimpleInterface> autoReleaseSimple(spSimple);

    DevTests::SimpleTestNamespace::IEmptyInterface * spEmpty = nullptr;
    hr = spSimple->QueryInterface(__uuidof(DevTests::SimpleTestNamespace::IEmptyInterface), (void**)&spEmpty);
    
    if (FAILED(hr))
    {
        return nullObject;
    }

    AutoReleasePtr<DevTests::SimpleTestNamespace::IEmptyInterface> autoReleaseEmpty(spEmpty);

    Var simpleVar;
    if (FAILED(scriptDirect->InspectableUnknownToVar(spEmpty, &simpleVar)))
    {
        simpleVar = nullObject;
    }

    return simpleVar;
}

Var TestUtilities::VarToDispExTest(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);
    
    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return NULL;
    }

    HRESULT hr;

    args = &args[1];

    if (callInfo.Count < 2) { return nullObject; }
    Var instance = args[0];

    IDispatchEx * pdispex = nullptr;
    hr = scriptDirect->VarToDispEx(instance, &pdispex);
    LPCWSTR retMsg;
    if (E_INVALIDARG == hr)
    {
        retMsg = L"Call failed with hr of E_INVALIDARG";
    }
    else if (FAILED(hr))
    {
        retMsg = L"Call failed with hr other than E_INVALIDARG";
    }
    else
    {
        retMsg = L"Call succeeded";
        pdispex->Release();
    }

    Var result;
    hr = scriptDirect->StringToVar(retMsg, wcslen(retMsg), &result);
    if (FAILED(hr)) { return nullObject; }
    return result;
}

Var TestUtilities::ClearAllProjectionCaches(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var nullObject;
    if (FAILED(scriptDirect->GetNull(&nullObject)))
    {
        return NULL;
    }
    JScript9Interface::ClearAllProjectionCaches(scriptDirect);
    return nullObject;
}

Var TestUtilities::QueryPerformanceCounter(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    LARGE_INTEGER count;
    ::QueryPerformanceCounter(&count);
    Var var;
    scriptDirect->Int64ToVar(count.QuadPart, &var);
    return var;
}


Var TestUtilities::QueryPerformanceFrequency(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    LARGE_INTEGER count;
    ::QueryPerformanceFrequency(&count);
    Var var;
    scriptDirect->Int64ToVar(count.QuadPart, &var);
    return var;   
}

Var TestUtilities::DoNotSupportWeakDelegate(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var undefinedObject;
    if (FAILED(scriptDirect->GetUndefined(&undefinedObject)))
    {
        return NULL;
    }

    JScript9Interface::DoNotSupportWeakDelegate(scriptDirect);
    return undefinedObject;
}

Var TestUtilities::SupportsWeakDelegate(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = NULL;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return NULL;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var undefinedObject;
    if (FAILED(scriptDirect->GetUndefined(&undefinedObject)))
    {
        return NULL;
    }

    BOOL supportsWeakDelegate = JScript9Interface::SupportsWeakDelegate(scriptDirect);
    Var supportsWeakDelegateVar;
    HRESULT hr = scriptDirect->BOOLToVar(supportsWeakDelegate, &supportsWeakDelegateVar);
    if (FAILED(hr)) { return undefinedObject; }
    return supportsWeakDelegateVar;
}

Var TestUtilities::GetHostType(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    IActiveScriptDirect * scriptDirect = nullptr;
    if (FAILED(JScript9Interface::JsVarToScriptDirect(dynamo, &scriptDirect)))
    {
        return nullptr;
    }

    AutoReleasePtr<IActiveScriptDirect> autoReleaseScriptDirect(scriptDirect);

    Var undefinedObject;
    if (FAILED(scriptDirect->GetUndefined(&undefinedObject)))
    {
        return nullptr;
    }

    int hostType = -1;
    if (FAILED(JScript9Interface::GetHostTypeFlag(&hostType)))
    {
        return undefinedObject;
    }

    Var hostTypeVar;
    if (FAILED(scriptDirect->IntToVar(hostType, &hostTypeVar)))
    {
        return undefinedObject;
    }

    return hostTypeVar;
}

Var TestUtilities::RestrictedErrorAccessInstanceToVar(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    return ActivateRuntimeClassAndConvertToVar(&GetRestrictedErrorAccessInstance, function, callInfo, dynamo);
}

Var TestUtilities::RWineryToVar(Var function, CallInfo callInfo, Var* args)
{
    Var dynamo = args[0];
    return ActivateRuntimeClassAndConvertToVar(&GetRWineryInstance, function, callInfo, dynamo);
}

void TestUtilities::Initialize(IActiveScriptDirect* activeScriptDirect, DelayLoadWinRt * winRTLibrary, Js::DelayLoadWinRtString * winRTStringLibrary)
{
    HRESULT hr = S_OK;
    IJavascriptOperations * javascriptOperations = NULL;

    m_WinRTLibrary = winRTLibrary;
    m_WinRTStringLibrary = winRTStringLibrary;

    hr = activeScriptDirect->GetJavascriptOperations(&javascriptOperations);
    IfFailedGo(hr);

    Var globalObject;
    hr = activeScriptDirect->GetGlobalObject(&globalObject);
    IfFailedGo(hr);

    // add TestUtilities object to the root.
    PropertyId testUtilitiesPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"TestUtilities", &testUtilitiesPropertyId);
    IfFailedGo(hr);
    Var testUtilitiesObject = NULL;
    hr = activeScriptDirect->CreateObject(&testUtilitiesObject);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, globalObject, testUtilitiesPropertyId, testUtilitiesObject);
    IfFailedGo(hr);

    // Add AnimalToVar function to TestUtilities
    PropertyId animalToVarPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"AnimalToVar", &animalToVarPropertyId);
    IfFailedGo(hr);
    PropertyId lengthPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var animalToVarFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::AnimalToVar, animalToVarPropertyId, -1, 0, &animalToVarFunction);
    IfFailedGo(hr);
    Var lengthValue;
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, animalToVarFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, animalToVarPropertyId, animalToVarFunction);
    IfFailedGo(hr);

    // DO NOT ADD UNTIL WIN8: 236649 is resolved
    // Add VectorIntToVar function to TestUtilities
    // function = NULL;
    // func = NULL;
    // id = scriptContext->GetOrAddPropertyId(L"VectorIntToVar", (UINT32)wcslen(L"VectorIntToVar"));
    // hr = scriptContext->GetActiveScriptDirect()->BuildDOMDirectFunction(nullptr, TestUtilities::VectorIntToVar, id, -1, 0, &func);
    // if (FAILED(hr)) { return; }
    // function = Js::JavascriptFunction::FromVar(func);
    // Js::JavascriptOperators::InitProperty(function, Js::PropertyIds::length, Js::TaggedInt::ToVarUnchecked(0));
    // Js::JavascriptOperators::InitProperty(testUtilitiesObject, id, function);

    // Add GetRestrictedStringFromError function to TestUtilities
    PropertyId getRestrictedStringFromErrorPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"GetRestrictedStringFromError", &getRestrictedStringFromErrorPropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var getRestrictedStringFromErrorFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::GetRestrictedStringFromError, getRestrictedStringFromErrorPropertyId, -1, 0, &getRestrictedStringFromErrorFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, getRestrictedStringFromErrorFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, getRestrictedStringFromErrorPropertyId, getRestrictedStringFromErrorFunction);
    IfFailedGo(hr);

    // Add GetCapabilitySidFromError function to TestUtilities
    PropertyId getCapabilitySidFromErrorPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"GetCapabilitySidFromError", &getCapabilitySidFromErrorPropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var getCapabilitySidFromErrorFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::GetCapabilitySidFromError, getCapabilitySidFromErrorPropertyId, -1, 0, &getCapabilitySidFromErrorFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, getCapabilitySidFromErrorFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, getCapabilitySidFromErrorPropertyId, getCapabilitySidFromErrorFunction);
    IfFailedGo(hr);

    // Add GetMemoryFootprintOfRC function to TestUtilities
    PropertyId getMemoryFootprintOfRCPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"GetMemoryFootprintOfRC", &getMemoryFootprintOfRCPropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var getMemoryFootprintOfRCFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::GetMemoryFootprintOfRC, getMemoryFootprintOfRCPropertyId, -1, 0, &getMemoryFootprintOfRCFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, getMemoryFootprintOfRCFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, getMemoryFootprintOfRCPropertyId, getMemoryFootprintOfRCFunction);
    IfFailedGo(hr);

    // Add DoNotSupportWeakDelegate function to TestUtilities
    PropertyId doNotSupportWeakDelegateId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"DoNotSupportWeakDelegate", &doNotSupportWeakDelegateId);
    IfFailedGo(hr);
    Var doNotSupportWeakDelegateFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::DoNotSupportWeakDelegate, doNotSupportWeakDelegateId, -1, 0, &doNotSupportWeakDelegateFunction);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, doNotSupportWeakDelegateId, doNotSupportWeakDelegateFunction);
    IfFailedGo(hr);

    // Add SupportsWeakDelegate function to TestUtilities
    PropertyId supportsWeakDelegateId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"SupportsWeakDelegate", &supportsWeakDelegateId);
    IfFailedGo(hr);
    Var supportsWeakDelegateFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::SupportsWeakDelegate, supportsWeakDelegateId, -1, 0, &supportsWeakDelegateFunction);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, supportsWeakDelegateId, supportsWeakDelegateFunction);
    IfFailedGo(hr);

    // Add GetSystemStringFromHr function to TestUtilities
    PropertyId getSystemStringFromHrPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"GetSystemStringFromHr", &getSystemStringFromHrPropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var getSystemStringFromHrFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::GetSystemStringFromHr, getSystemStringFromHrPropertyId, -1, 0, &getSystemStringFromHrFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, getSystemStringFromHrFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, getSystemStringFromHrPropertyId, getSystemStringFromHrFunction);
    IfFailedGo(hr);

    // Add UpdateSimpleClassAndReturnAsVar function to TestUtilities
    PropertyId updateSimpleClassAndReturnAsVarPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"UpdateSimpleClassAndReturnAsVar", &updateSimpleClassAndReturnAsVarPropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var updateSimpleClassAndReturnAsVarFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::UpdateSimpleClassAndReturnAsVar, updateSimpleClassAndReturnAsVarPropertyId, -1, 0, &updateSimpleClassAndReturnAsVarFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, updateSimpleClassAndReturnAsVarFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, updateSimpleClassAndReturnAsVarPropertyId, updateSimpleClassAndReturnAsVarFunction);
    IfFailedGo(hr);

    // Add UpdateSimpleClassAndReturnAsVarByAlternateInterface function to TestUtilities
    PropertyId updateSimpleClassAndReturnAsVarByAlternateInterfacePropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"UpdateSimpleClassAndReturnAsVarByAlternateInterface", &updateSimpleClassAndReturnAsVarByAlternateInterfacePropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var updateSimpleClassAndReturnAsVarByAlternateInterfaceFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::UpdateSimpleClassAndReturnAsVarByAlternateInterface, updateSimpleClassAndReturnAsVarByAlternateInterfacePropertyId, -1, 0, &updateSimpleClassAndReturnAsVarByAlternateInterfaceFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, updateSimpleClassAndReturnAsVarByAlternateInterfaceFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, updateSimpleClassAndReturnAsVarByAlternateInterfacePropertyId, updateSimpleClassAndReturnAsVarByAlternateInterfaceFunction);
    IfFailedGo(hr);

    // Add VarToDispExTest function to TestUtilities
    PropertyId varToDispExTestPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"VarToDispExTest", &varToDispExTestPropertyId);
    IfFailedGo(hr);
    hr = activeScriptDirect->GetOrAddPropertyId(L"length", &lengthPropertyId);
    IfFailedGo(hr);
    Var varToDispExTestFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::VarToDispExTest, varToDispExTestPropertyId, -1, 0, &varToDispExTestFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(1, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, varToDispExTestFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, varToDispExTestPropertyId, varToDispExTestFunction);
    IfFailedGo(hr);

    // Add ClearAllProjectionCaches function to TestUtilities
    PropertyId clearAllProjectionCachesId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"ClearAllProjectionCaches", &clearAllProjectionCachesId);
    IfFailedGo(hr);
    Var clearAllProjectionCachesFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::ClearAllProjectionCaches, clearAllProjectionCachesId, -1, 0, &clearAllProjectionCachesFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(0, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, clearAllProjectionCachesFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, clearAllProjectionCachesId, clearAllProjectionCachesFunction);
    IfFailedGo(hr);

    // Add QueryPerformanceCounter function to TestUtilities
    PropertyId queryPerformanceCounterId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"QueryPerformanceCounter", &queryPerformanceCounterId);
    IfFailedGo(hr);
    Var queryPerformanceCounterFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::QueryPerformanceCounter, queryPerformanceCounterId, -1, 0, &queryPerformanceCounterFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(0, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, queryPerformanceCounterFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, queryPerformanceCounterId, queryPerformanceCounterFunction);
    IfFailedGo(hr);

    // Add QueryPerformanceFrequency function to TestUtilities
    PropertyId queryPerformanceFrequencyId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"QueryPerformanceFrequency", &queryPerformanceFrequencyId);
    IfFailedGo(hr);
    Var queryPerformanceFrequencyFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::QueryPerformanceFrequency, queryPerformanceFrequencyId, -1, 0, &queryPerformanceFrequencyFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(0, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, queryPerformanceFrequencyFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, queryPerformanceFrequencyId, queryPerformanceFrequencyFunction);
    IfFailedGo(hr);

    // Add GetHostType function to TestUtilities
    PropertyId getHostTypeId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"GetHostType", &getHostTypeId);
    IfFailedGo(hr);
    Var getHostTypeFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::GetHostType, getHostTypeId, -1, 0, &getHostTypeFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(0, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, getHostTypeFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, getHostTypeId, getHostTypeFunction);
    IfFailedGo(hr);

    // Add RestrictedErrorAccessInstanceToVar function to TestUtilities
    PropertyId restrictedErrorAccessInstanceToVarId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"RestrictedErrorAccessInstanceToVar", &restrictedErrorAccessInstanceToVarId);
    IfFailedGo(hr);
    Var restrictedErrorAccessInstanceToVarFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::RestrictedErrorAccessInstanceToVar, restrictedErrorAccessInstanceToVarId, -1, 0, &restrictedErrorAccessInstanceToVarFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(0, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, restrictedErrorAccessInstanceToVarFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, restrictedErrorAccessInstanceToVarId, restrictedErrorAccessInstanceToVarFunction);
    IfFailedGo(hr);

    // Add RWineryToVar function to TestUtilities
    PropertyId rwineryToVarId;
    hr = activeScriptDirect->GetOrAddPropertyId(L"RWineryToVar", &rwineryToVarId);
    IfFailedGo(hr);
    Var rwineryToVarFunction;
    hr = activeScriptDirect->BuildDOMDirectFunction(nullptr, TestUtilities::RWineryToVar, rwineryToVarId, -1, 0, &rwineryToVarFunction);
    IfFailedGo(hr);
    hr = activeScriptDirect->IntToVar(0, &lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, rwineryToVarFunction, lengthPropertyId, lengthValue);
    IfFailedGo(hr);
    hr = javascriptOperations->SetProperty(activeScriptDirect, testUtilitiesObject, rwineryToVarId, rwineryToVarFunction);
    IfFailedGo(hr);

LReturn:
    if (javascriptOperations)
    {
        javascriptOperations->Release();
    }

    return;
}
