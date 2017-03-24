/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "edgejsstatic.h"

#define ReturnUndefOnFailed(expr, undef) do { if (FAILED(expr)) { return undef; }} while(FALSE)

MockDomObjectManager g_domObjectManager;

MockPassTypeOperations::MockPassTypeOperations(ITypeOperations* defaultScriptOperations)
    : m_defaultScriptOperations(defaultScriptOperations)
{}

MockPassTypeOperations::~MockPassTypeOperations()
{}

STDMETHODIMP MockPassTypeOperations::QueryInterface(REFIID riid, void** ppvObj)
{
    IfNullReturnError(ppvObj, E_POINTER);

    QI_IMPL_INTERFACE(IUnknown);
    QI_IMPL_INTERFACE(ITypeOperations);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

// ITypeOperations
STDMETHODIMP MockPassTypeOperations::GetOperationUsage(OperationUsage* usage)
{
    return m_defaultScriptOperations->GetOperationUsage(usage);
}
STDMETHODIMP MockPassTypeOperations::HasOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in PropertyId propertyId, __out BOOL* result)
{
    return m_defaultScriptOperations->HasOwnProperty(pActiveScriptDirect, instance, propertyId, result);
}
STDMETHODIMP MockPassTypeOperations::GetOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var* value, BOOL* propertyPresent)
{
    return m_defaultScriptOperations->GetOwnProperty(pActiveScriptDirect, instance, propertyId, value, propertyPresent);
}
STDMETHODIMP MockPassTypeOperations::GetPropertyReference(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in PropertyId propertyId, __out Var* value, __out BOOL* propertyPresent)
{
    return m_defaultScriptOperations->GetPropertyReference(pActiveScriptDirect, instance, propertyId, value, propertyPresent);
}
STDMETHODIMP MockPassTypeOperations::SetProperty(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var value, BOOL* result)
{
    return m_defaultScriptOperations->SetProperty(pActiveScriptDirect, instance, propertyId, value, result);
}
STDMETHODIMP MockPassTypeOperations::SetPropertyWithAttributes(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var value, PropertyAttributes attributes, SideEffects effects, BOOL* result)
{
    return m_defaultScriptOperations->SetPropertyWithAttributes(pActiveScriptDirect, instance, propertyId, value, attributes, effects, result);
}
STDMETHODIMP MockPassTypeOperations::DeleteProperty(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result)
{
    return m_defaultScriptOperations->DeleteProperty(pActiveScriptDirect, instance, propertyId, result);
}
STDMETHODIMP MockPassTypeOperations::HasOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, BOOL* result)
{
    return m_defaultScriptOperations->HasOwnItem(pActiveScriptDirect, instance, index, result);
}
STDMETHODIMP MockPassTypeOperations::GetOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, Var* value, BOOL* itemPresent)
{
    return m_defaultScriptOperations->GetOwnItem(pActiveScriptDirect, instance, index, value, itemPresent);
}
STDMETHODIMP MockPassTypeOperations::SetItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, Var value, BOOL* result)
{
    return m_defaultScriptOperations->SetItem(pActiveScriptDirect, instance, index, value, result);
}
STDMETHODIMP MockPassTypeOperations::DeleteItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, BOOL* result)
{
    return m_defaultScriptOperations->DeleteItem(pActiveScriptDirect, instance, index, result);
}
STDMETHODIMP MockPassTypeOperations::GetEnumerator(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, BOOL enumNonEnumerable, __in BOOL enumSymbols, IVarEnumerator** enumerator)
{
    return m_defaultScriptOperations->GetEnumerator(pActiveScriptDirect, instance, enumNonEnumerable, enumSymbols, enumerator);
}
STDMETHODIMP MockPassTypeOperations::IsEnumerable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result)
{
    return m_defaultScriptOperations->IsEnumerable(pActiveScriptDirect, instance, propertyId, result);
}
STDMETHODIMP MockPassTypeOperations::IsWritable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result)
{
    return m_defaultScriptOperations->IsWritable(pActiveScriptDirect, instance, propertyId, result);
}
STDMETHODIMP MockPassTypeOperations::IsConfigurable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result)
{
    return m_defaultScriptOperations->IsConfigurable(pActiveScriptDirect, instance, propertyId, result);
}
STDMETHODIMP MockPassTypeOperations::SetEnumerable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL value)
{
    return m_defaultScriptOperations->SetEnumerable(pActiveScriptDirect, instance, propertyId, value);
}
STDMETHODIMP MockPassTypeOperations::SetWritable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL value)
{
    return m_defaultScriptOperations->SetWritable(pActiveScriptDirect, instance, propertyId, value);
}
STDMETHODIMP MockPassTypeOperations::SetConfigurable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL value)
{
    return m_defaultScriptOperations->SetConfigurable(pActiveScriptDirect, instance, propertyId, value);
}
STDMETHODIMP MockPassTypeOperations::SetAccessors(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var getter, Var setter)
{
    return m_defaultScriptOperations->SetAccessors(pActiveScriptDirect, instance, propertyId, getter, setter);
}
STDMETHODIMP MockPassTypeOperations::GetAccessors(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var* getter, Var* setter, BOOL* result)
{
    return m_defaultScriptOperations->GetAccessors(pActiveScriptDirect, instance, propertyId, getter, setter, result);
}
STDMETHODIMP MockPassTypeOperations::GetSetter(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var* setter, DescriptorFlags* flags)
{
    return m_defaultScriptOperations->GetSetter(pActiveScriptDirect, instance, propertyId, setter, flags);
}
STDMETHODIMP MockPassTypeOperations::GetItemSetter(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, Var* setter, DescriptorFlags* flags)
{
    return m_defaultScriptOperations->GetItemSetter(pActiveScriptDirect, instance, index, setter, flags);
}
STDMETHODIMP MockPassTypeOperations::Equals(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var other, BOOL* result)
{
    return m_defaultScriptOperations->Equals(pActiveScriptDirect, instance, other, result);
}
STDMETHODIMP MockPassTypeOperations::StrictEquals(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var other, BOOL* result)
{
    return m_defaultScriptOperations->StrictEquals(pActiveScriptDirect, instance, other, result);
}
STDMETHODIMP MockPassTypeOperations::QueryObjectInterface(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, REFIID riid, void** ppvObj)
{
    return m_defaultScriptOperations->QueryObjectInterface(pActiveScriptDirect, instance, riid, ppvObj);
}
STDMETHODIMP MockPassTypeOperations::GetInitializer(InitializeMethod * initializer, int * initSlotCapacity, BOOL * hasAccessors)
{
    return E_NOTIMPL;
}
STDMETHODIMP MockPassTypeOperations::GetFinalizer(FinalizeMethod * finalizer)
{
    return E_NOTIMPL;
}
STDMETHODIMP MockPassTypeOperations::HasInstance(__in IActiveScriptDirect* pActiveScriptDirect, Var constructor, Var instance, BOOL* result)
{
    return E_NOTIMPL;
}
STDMETHODIMP MockPassTypeOperations::GetNamespaceParent(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __out Var* namespaceParent)
{
    return E_NOTIMPL;
}
STDMETHODIMP MockPassTypeOperations::CrossDomainCheck(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __out BOOL* result)
{
    *result = FALSE;
    return S_OK;
}
STDMETHODIMP MockPassTypeOperations::GetHeapObjectInfo(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in ProfilerHeapObjectInfoFlags flags, __out HostProfilerHeapObject** result, __out HeapObjectInfoReturnResult* returnResult)
{
    return m_defaultScriptOperations->GetHeapObjectInfo(pActiveScriptDirect, instance, flags, result, returnResult);
}

ArrayCollectionTypeOperations::ArrayCollectionTypeOperations(ITypeOperations* defaultScriptOperations) :
    MockPassTypeOperations(defaultScriptOperations)
{
}

STDMETHODIMP ArrayCollectionTypeOperations::GetOperationUsage(__out OperationUsage* usage)
{
    MockPassTypeOperations::GetOperationUsage(usage);

    usage->useWhenPropertyNotPresent = (OperationFlags)
        (usage->useWhenPropertyNotPresent |
            OperationFlag_hasOwnProperty |
            OperationFlag_getOwnProperty |
            OperationFlag_hasOwnItem |
            OperationFlag_getOwnItem);

    usage->useWhenPropertyNotPresentInPrototypeChain = OperationFlagsForNamespaceOrdering_allGetPropertyOperations;

    return S_OK;
}

STDMETHODIMP ArrayCollectionTypeOperations::HasOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, PropertyId propertyId, __out BOOL* result)
{
    Var value = nullptr;
    return GetOwnProperty(pActiveScriptDirect, instance, propertyId, &value, result);
}

STDMETHODIMP ArrayCollectionTypeOperations::GetOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, PropertyId propertyId, __out Var* value, __out BOOL* propertyPresent)
{
    *propertyPresent = FALSE;
    HRESULT hr = E_FAIL;

    MockArrayDomObject *mydomObj = g_domObjectManager.GetDomObjectFromVar(instance);
    if (mydomObj != nullptr)
    {
        PCWSTR propName = nullptr;
        hr = pActiveScriptDirect->GetPropertyName(propertyId, &propName);
        if (hr == S_OK && wcscmp(propName, _u("length")) == 0)
        {
            uint count = (uint)mydomObj->m_objectVars.size();
            if (count > 0 && count < INT_MAX)
            {
                pActiveScriptDirect->IntToVar(count, value);
                *propertyPresent = TRUE;
                return S_OK;
            }
        }
    }
    return E_FAIL;
}

STDMETHODIMP ArrayCollectionTypeOperations::HasOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in Var index, __out BOOL* result)
{
    Var value = nullptr;
    return GetOwnItem(pActiveScriptDirect, instance, index, &value, result);
}

STDMETHODIMP ArrayCollectionTypeOperations::GetOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in Var index, __out Var* value, __out BOOL* itemPresent)
{
    MockArrayDomObject *mydomObj = g_domObjectManager.GetDomObjectFromVar(instance);
    if (mydomObj != nullptr)
    {
        *value = mydomObj->FetchGetObject(index, itemPresent);
        return S_OK;
    }
    return E_FAIL;
}

MockArrayDomObject::MockArrayDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations)
    : m_myDomVar(domVar), m_activeScriptDirect(activeScriptDirect), m_typeOperation(operations)
{
    m_activeScriptDirect->GetTypeIdForVar(domVar, &m_typeId);
}

MockArrayDomObject::~MockArrayDomObject()
{
    if (m_typeOperation)
    {
        delete m_typeOperation;
    }
}

Var MockArrayDomObject::EntryAddObject(Var function, CallInfo callInfo, Var* args)
{
    MockArrayDomObject *domObj = g_domObjectManager.GetDomObjectFromVar(args[0]);

    if (domObj != nullptr && callInfo.Count >= 2)
    {
        Var val = args[1];
        domObj->AddObject(val);
        return domObj->GetUndefined();
    }

    return MockDomObjectManager::GetUndefined(function);
}

Var MockArrayDomObject::EntryGetObject(Var function, CallInfo callInfo, Var* args)
{
    MockArrayDomObject *domObj = g_domObjectManager.GetDomObjectFromVar(args[0]);
    if (domObj && callInfo.Count >= 2)
    {
        BOOL itemPresent;
        return domObj->FetchGetObject(args[1], &itemPresent);
    }

    return MockDomObjectManager::GetUndefined(function);
}

Var MockArrayDomObject::EntryForEach(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *scriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var forEachFunction = JsStaticAPI::JavascriptLibrary::GetArrayForEachFunction(scriptDirect);
    if (forEachFunction != nullptr)
    {
        Var varResult = nullptr;
        if (scriptDirect->Execute(forEachFunction, callInfo, args, NULL, &varResult) == S_OK)
        {
            return varResult;
        }
    }

    return MockDomObjectManager::GetUndefined(function);
}

Var MockArrayDomObject::EntryKeys(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *scriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var keysFunction = JsStaticAPI::JavascriptLibrary::GetArrayKeysFunction(scriptDirect);
    if (keysFunction != nullptr)
    {
        Var varResult = nullptr;
        if (scriptDirect->Execute(keysFunction, callInfo, args, NULL, &varResult) == S_OK)
        {
            return varResult;
        }
    }

    return JsStaticAPI::JavascriptLibrary::GetUndefined(scriptDirect);
}

Var MockArrayDomObject::EntryValues(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *scriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var valuesFunction = JsStaticAPI::JavascriptLibrary::GetArrayValuesFunction(scriptDirect);
    if (valuesFunction != nullptr)
    {
        Var varResult = nullptr;
        if (scriptDirect->Execute(valuesFunction, callInfo, args, NULL, &varResult) == S_OK)
        {
            return varResult;
        }
    }

    return JsStaticAPI::JavascriptLibrary::GetUndefined(scriptDirect);
}

Var MockArrayDomObject::EntryEntries(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *scriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var entriesFunction = JsStaticAPI::JavascriptLibrary::GetArrayEntriesFunction(scriptDirect);
    if (entriesFunction != nullptr)
    {
        Var varResult = nullptr;
        if (scriptDirect->Execute(entriesFunction, callInfo, args, NULL, &varResult) == S_OK)
        {
            return varResult;
        }
    }

    return JsStaticAPI::JavascriptLibrary::GetUndefined(scriptDirect);
}

Var MockArrayDomObject::GetUndefined()
{
    Var undefined = nullptr;
    m_activeScriptDirect->GetUndefined(&undefined);
    return undefined;
}

void MockArrayDomObject::AddObject(Var obj)
{
    m_objectVars.push_back(obj);
}

Var MockArrayDomObject::FetchGetObject(Var varIndex, BOOL *itemPresent)
{
    int index;
    if (m_activeScriptDirect->VarToInt(varIndex, &index) == S_OK && (uint)index < m_objectVars.size())
    {
        *itemPresent = TRUE;
        return m_objectVars[index];
    }

    *itemPresent = FALSE;
    return GetUndefined();
}

MockArrayDomObject* MockDomObjectManager::GetDomObjectFromVar(Var obj)
{
    if (obj == nullptr || varToDomArrayObject.size() == 0)
    {
        return nullptr;
    }
    auto it = varToDomArrayObject.find(obj);
    if (it == varToDomArrayObject.end())
    {
        nullptr;
    }

    MockArrayDomObject * domObj = it->second;
    if (!domObj || domObj->m_myDomVar != obj)
    {
        return nullptr;
    }

    // TODO : typeId safety

    return domObj;
}

void MockDomObjectManager::AddVarToObject(Var obj, IActiveScriptDirect *scriptDirect, ITypeOperations* operation)
{
    MockArrayDomObject *domObj = new MockArrayDomObject(scriptDirect, obj, operation);
    varToDomArrayObject.insert(std::pair<Var, MockArrayDomObject*>(obj, domObj));
}

Var MockDomObjectManager::GetUndefined(Var obj)
{
    IActiveScriptDirect *scriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(obj);
    if (scriptDirect != nullptr)
    {
        return JsStaticAPI::JavascriptLibrary::GetUndefined(scriptDirect);
    }

    return nullptr;
}

HRESULT MockDomObjectManager::AddMemberVar(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, LPCWSTR prop, ScriptFunctionObj fn, Var obj, PropertyId secondPropId)
{
    HRESULT hr = S_OK;
    PropertyId propertyId;
    Var func = nullptr;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(prop, &propertyId));
    IfFailedReturn(activeScriptDirect->BuildDOMDirectFunction(nullptr, fn, propertyId, -1, 0, &func));

    BOOL result = false;
    IfFailedReturn(defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, obj, propertyId, func, PropertyAttributes_Enumerable, SideEffects_None, &result));
    if (secondPropId != 0)
    {
        hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, obj, secondPropId, func, PropertyAttributes_Enumerable, SideEffects_None, &result);
    }
    return hr;
}

Var MockDomObjectManager::CreateDomArrayObject(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *activeScriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var undef = JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect);

    CComPtr<IJavascriptOperations> jsOps;
    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;

    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    ReturnUndefOnFailed(hr, undef);

    hr = activeScriptDirect->GetJavascriptOperations(&jsOps);
    ReturnUndefOnFailed(hr, undef);

    Var globalObject;
    hr = activeScriptDirect->GetGlobalObject(&globalObject);
    ReturnUndefOnFailed(hr, undef);

    PropertyId ctorPropId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("DOMArrayCtor"), &ctorPropId);
    ReturnUndefOnFailed(hr, undef);

    Var domConstructor;
    hr = activeScriptDirect->CreateConstructor(NULL, TempDomArrayConstructor, ctorPropId, TRUE, &domConstructor);
    ReturnUndefOnFailed(hr, undef);

    BOOL result = FALSE;
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, globalObject, ctorPropId, domConstructor, static_cast<PropertyAttributes>(PropertyAttributes_Writable | PropertyAttributes_Configurable), SideEffects_None, &result);
    ReturnUndefOnFailed(hr, undef);

    HTYPE externalType;
    Var protoypeExternalVar = nullptr;

    PropertyId objectProtoPropId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("DOMArrayObjPrototype"), &objectProtoPropId);

    hr = activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, nullptr, NULL, defaultScriptOperations, FALSE, objectProtoPropId, false, &externalType);
    ReturnUndefOnFailed(hr, undef);

    hr = activeScriptDirect->CreateTypedObject(externalType, 0, TRUE, &protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);


    hr = AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("AddObject"), MockArrayDomObject::EntryAddObject, protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    hr = AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("ItemAt"), MockArrayDomObject::EntryGetObject, protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    hr = AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("forEach"), MockArrayDomObject::EntryForEach, protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    hr = AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("keys"), MockArrayDomObject::EntryKeys, protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    PropertyId symbolIterator = JsStaticAPI::JavascriptLibrary::GetPropertyIdSymbolIterator(activeScriptDirect);

    hr = AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("values"), MockArrayDomObject::EntryValues, protoypeExternalVar, symbolIterator);
    ReturnUndefOnFailed(hr, undef);

    hr = AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("entries"), MockArrayDomObject::EntryEntries, protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    PropertyId ctorId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("constructor"), &ctorId);
    ReturnUndefOnFailed(hr, undef);

    // Adding constructor property
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, protoypeExternalVar, ctorId, domConstructor, PropertyAttributes_None, SideEffects_None, &result);
    ReturnUndefOnFailed(hr, undef);

    PropertyId objectPropId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("DOMArrayObj"), &objectPropId);
    ReturnUndefOnFailed(hr, undef);

    ArrayCollectionTypeOperations * operations = new ArrayCollectionTypeOperations(defaultScriptOperations);
    operations;

    HTYPE externalType1;
    Var domVarObject = nullptr;
    hr = activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, protoypeExternalVar, NULL, operations, FALSE, objectPropId, false, &externalType1);
    ReturnUndefOnFailed(hr, undef);

    hr = activeScriptDirect->CreateTypedObject(externalType1, 0, TRUE, &domVarObject);
    ReturnUndefOnFailed(hr, undef);

    g_domObjectManager.AddVarToObject(domVarObject, activeScriptDirect, operations);
    return domVarObject;
}

HRESULT MockDomObjectManager::Initialize(IActiveScript *activeScript)
{
    HRESULT hr = S_OK;
    CComPtr<IActiveScriptDirect> activeScriptDirect = nullptr;
    IfFailedReturn(activeScript->QueryInterface(&activeScriptDirect));

    return AddDomArrayObjectCreatorFunction(activeScriptDirect);
}

HRESULT MockDomObjectManager::AddDomArrayObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect)
{
    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;
    Var globalObject = nullptr;

    IfFailedReturn(activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations));
    IfFailedReturn(activeScriptDirect->GetGlobalObject(&globalObject));

    PropertyId ctorPropId;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(_u("CreateDomArrayObject"), &ctorPropId));

    Var domConstructor = nullptr;
    hr = activeScriptDirect->CreateConstructor(nullptr, MockDomObjectManager::CreateDomArrayObject, ctorPropId, TRUE, &domConstructor);
    IfFailedReturn(hr);

    BOOL result = FALSE;
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, globalObject, ctorPropId, domConstructor, static_cast<PropertyAttributes>(PropertyAttributes_Writable | PropertyAttributes_Configurable), SideEffects_None, &result);

    return hr;
}

MockDomObjectManager::~MockDomObjectManager()
{
    for (auto iterator = varToDomArrayObject.begin(); iterator != varToDomArrayObject.end(); iterator++)
    {
        delete iterator->second;
    }
}

