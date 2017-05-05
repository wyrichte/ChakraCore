/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "edgejsstatic.h"

#define ReturnUndefOnFailed(expr, undef) do { if (FAILED(expr)) { return undef; }} while(FALSE)

MockDomObjectManager g_domObjectManager;
const int MockMapDomObject::TypdID_DOMMap = 4000;
const int MockSetDomObject::TypdID_DOMSet = 5000;

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

    MockArrayDomObject *mydomObj = static_cast<MockArrayDomObject*>(g_domObjectManager.GetDomObjectFromVar(instance));
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
    MockArrayDomObject *mydomObj = static_cast<MockArrayDomObject*>(g_domObjectManager.GetDomObjectFromVar(instance));
    if (mydomObj != nullptr)
    {
        *value = mydomObj->FetchGetObject(index, itemPresent);
        return S_OK;
    }
    return E_FAIL;
}

MockObject::MockObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations)
    : m_myDomVar(domVar), m_activeScriptDirect(activeScriptDirect), m_typeOperation(operations)
{
    m_activeScriptDirect->GetTypeIdForVar(domVar, &m_typeId);
}

MockObject::~MockObject()
{
    if (m_typeOperation)
    {
        delete m_typeOperation;
    }
}

Var MockObject::GetUndefined()
{
    Var undefined = nullptr;
    m_activeScriptDirect->GetUndefined(&undefined);
    return undefined;
}

HRESULT MockObject::ThrowException(IActiveScriptDirect* activeScriptDirect, LPCWSTR text, HRESULT hr)
{
    if (activeScriptDirect)
    {
        Var exceptionObject;
        if (activeScriptDirect->CreateErrorObject(JavascriptTypeError, hr, text, &exceptionObject) == S_OK)
        {
            return activeScriptDirect->ThrowException(exceptionObject);
        }
    }
    return E_FAIL;
}

HRESULT MockObject::ThrowException(Var function, LPCWSTR text, HRESULT hr)
{
    return MockObject::ThrowException(JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function), text, hr);
}

HRESULT MockObject::ThrowException(LPCWSTR text, HRESULT hr)
{
    return MockObject::ThrowException(m_activeScriptDirect, text, hr);
}

MockArrayDomObject::MockArrayDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations)
    : MockObject(activeScriptDirect, domVar, operations)
{
}

Var MockArrayDomObject::EntryAddObject(Var function, CallInfo callInfo, Var* args)
{
    MockArrayDomObject *domObj = static_cast<MockArrayDomObject*>(g_domObjectManager.GetDomObjectFromVar(args[0]));

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
    MockArrayDomObject *domObj = static_cast<MockArrayDomObject*>(g_domObjectManager.GetDomObjectFromVar(args[0]));
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

MockObject* MockDomObjectManager::GetDomObjectFromVar(Var obj)
{
    if (obj == nullptr || varToDomObject.size() == 0)
    {
        return nullptr;
    }
    auto it = varToDomObject.find(obj);
    if (it == varToDomObject.end())
    {
        return nullptr;
    }

    MockObject * domObj = it->second;
    if (!domObj || domObj->m_myDomVar != obj)
    {
        return nullptr;
    }

    return domObj;
}


void MockDomObjectManager::AddVarToObject(Var obj, MockObject * domObject)
{
    varToDomObject.insert(std::pair<Var, MockObject*>(obj, domObject));
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

MockPairDomObject::MockPairDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations)
    : MockObject(activeScriptDirect, domVar, operations)
{
}

Var MockPairDomObject::Set(Var function, int typeIdToValidate, LPCWSTR errorMessage, CallInfo callInfo, Var* args)
{
    MockObject *obj = g_domObjectManager.GetDomObjectFromVar(args[0]);
    if (obj == nullptr || ((int)obj->m_typeId) != typeIdToValidate)
    {
        MockObject::ThrowException(function, errorMessage);
        return MockDomObjectManager::GetUndefined(function);
    }

    MockPairDomObject *domObj = static_cast<MockPairDomObject*>(obj);

    if (callInfo.Count >= 3)
    {
        Var key = args[1];
        Var value = args[2];

        if (domObj->m_objectMap.size() == 0)
        {
            domObj->m_objectMap.insert(std::pair<Var, Var>(key, value));
        }
        else
        {
            auto it = domObj->m_objectMap.find(key);
            if (it != domObj->m_objectMap.end())
            {
                it->second = value;
            }
            else
            {
                domObj->m_objectMap.insert(std::pair<Var, Var>(key, value));
            }
        }
    }
    return MockDomObjectManager::GetUndefined(function);
}

Var MockPairDomObject::Get(Var function, int typeIdToValidate, LPCWSTR errorMessage, CallInfo callInfo, Var* args)
{
    MockObject *obj = g_domObjectManager.GetDomObjectFromVar(args[0]);
    if (obj == nullptr || ((int)obj->m_typeId) != typeIdToValidate)
    {
        MockObject::ThrowException(function, errorMessage);
        return MockDomObjectManager::GetUndefined(function);
    }

    MockPairDomObject *domObj = static_cast<MockPairDomObject*>(obj);

    if (callInfo.Count >= 2)
    {
        Var key = args[1];
        if (key && domObj->m_objectMap.size() > 0)
        {
            auto it = domObj->m_objectMap.find(key);
            if (it != domObj->m_objectMap.end())
            {
                return it->second;
            }
        }
    }
    return MockDomObjectManager::GetUndefined(function);
}

void MockPairDomObject::InitIterator(Var instance, int typeIdToValidate, Var iterator, LPCWSTR errorMessage)
{
    MockObject *obj = g_domObjectManager.GetDomObjectFromVar(instance);
    if (obj == nullptr || ((int)obj->m_typeId) != typeIdToValidate)
    {
        MockObject::ThrowException(instance, errorMessage);
        return;
    }

    MockPairDomObject *domObj = static_cast<MockPairDomObject*>(obj);
    DomPairIterator *mapIterator = new DomPairIterator(domObj); // This iterator will be deleted in the Next function.

    Assert(iterator);
    void **buffer = (void **)JsStaticAPI::JavascriptLibrary::CustomIteratorToExtension(iterator);
    buffer[0] = (void *)mapIterator;
}

bool MockPairDomObject::Next(Var iterator, Var *key, Var *value)
{
    Assert(iterator != nullptr);
    void **buffer = (void **)JsStaticAPI::JavascriptLibrary::CustomIteratorToExtension(iterator);
    bool ret = false;
    if (buffer[0] != nullptr)
    {
        DomPairIterator *mapIterator = static_cast<DomPairIterator*>(buffer[0]);
        ret = mapIterator->Next(key, value);
        if (!ret)
        {
            // Done with the iterator, delete it.
            delete buffer[0];
            buffer[0] = nullptr;
        }
    }
    return ret;
}

HRESULT MockPairDomObject::CreateBaseIteratorPrototype(IActiveScriptDirect *activeScriptDirect, int typeIdToValidate, LPCWSTR propName, LPCWSTR stringTag, Var *proto)
{
    HRESULT hr = S_OK;

    PropertyId objectPropId;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(propName, &objectPropId));

    HTYPE externalType;
    Var domVarObject = nullptr;
    CComPtr<ITypeOperations> defaultScriptOperations;

    IfFailedReturn(activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations));

    Var iteratorPrototype = JsStaticAPI::JavascriptLibrary::GetIteratorPrototype(activeScriptDirect);
    Assert(iteratorPrototype != nullptr);

    IfFailedReturn(activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, iteratorPrototype, NULL, defaultScriptOperations, FALSE, objectPropId, false, &externalType));
    IfFailedReturn(activeScriptDirect->CreateTypedObject(externalType, 0, TRUE, &domVarObject));

    Var nextFunction = JsStaticAPI::JavascriptLibrary::CreateIteratorNextFunction(activeScriptDirect, typeIdToValidate);
    Assert(nextFunction != nullptr);

    IfFailedReturn(MockDomObjectManager::AddMemberFunction(activeScriptDirect, defaultScriptOperations, _u("next"), nextFunction, domVarObject));

    PropertyId symbolToStringTagId = JsStaticAPI::JavascriptLibrary::GetPropertyIdSymbolToStringTag(activeScriptDirect);
    Var toStringTagValue;
    IfFailedReturn(activeScriptDirect->StringToVar(stringTag, static_cast<int>(wcslen(stringTag)), &toStringTagValue));
    BOOL result = false;
    IfFailedReturn(defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, domVarObject, symbolToStringTagId, toStringTagValue, PropertyAttributes_Configurable, SideEffects_None, &result));

    *proto = domVarObject;

    return hr;
}

MockSetDomObject::MockSetDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations)
    : MockPairDomObject(activeScriptDirect, domVar, operations)
{
}

Var MockSetDomObject::EntrySet(Var function, CallInfo callInfo, Var* args)
{
    return MockPairDomObject::Set(function, MockSetDomObject::TypdID_DOMSet, _u("Current object is not a DOMSetObject"), callInfo, args);
}

Var MockSetDomObject::EntryGet(Var function, CallInfo callInfo, Var* args)
{
    return MockPairDomObject::Get(function, MockSetDomObject::TypdID_DOMSet, _u("Current object is not a DOMSetObject"), callInfo, args);
}

void MockSetDomObject::InitIterator(Var instance, Var iterator)
{
    MockPairDomObject::InitIterator(instance, MockSetDomObject::TypdID_DOMSet, iterator, _u("Current object is not a DOMSetObject"));
}

bool MockSetDomObject::Next(Var iterator, Var *key, Var *value)
{
    return MockPairDomObject::Next(iterator, key, value);
}

HRESULT MockSetDomObject::CreateSetIteratorPrototype(IActiveScriptDirect *activeScriptDirect, Var *proto)
{
    return MockPairDomObject::CreateBaseIteratorPrototype(activeScriptDirect, MockSetDomObject::TypdID_DOMSet, _u("SetIteratorPrototype"), _u("SetIterator"), proto);
}

HRESULT MockSetDomObject::CreateSetPrototypeObject(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, Var *prototype)
{
    HRESULT hr = S_OK;
    Var protoypeExternalVar = g_domObjectManager.GetSetPrototypeObject();
    if (protoypeExternalVar == nullptr)
    {
        HTYPE externalType;

        PropertyId objectProtoPropId;
        IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(_u("DOMSetObjPrototype"), &objectProtoPropId));
        IfFailedReturn(activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, nullptr, NULL, defaultScriptOperations, FALSE, objectProtoPropId, false, &externalType));
        IfFailedReturn(activeScriptDirect->CreateTypedObject(externalType, 0, TRUE, &protoypeExternalVar));


        IfFailedReturn(MockDomObjectManager::AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("Set"), MockSetDomObject::EntrySet, protoypeExternalVar));
        IfFailedReturn(MockDomObjectManager::AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("Get"), MockSetDomObject::EntryGet, protoypeExternalVar));

        Var setIteratorPrototype;
        IfFailedReturn(CreateSetIteratorPrototype(activeScriptDirect, &setIteratorPrototype));

        IfFailedReturn(MockPairDomObject::AddIteratorProperties(activeScriptDirect, defaultScriptOperations, MockSetDomObject::TypdID_DOMSet, protoypeExternalVar, setIteratorPrototype,
            &MockSetDomObject::InitIterator, &MockSetDomObject::Next));

        g_domObjectManager.SetSetPrototypeObject(protoypeExternalVar);
    }

    *prototype = protoypeExternalVar;
    return hr;
}

MockMapDomObject::MockMapDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations)
    : MockPairDomObject(activeScriptDirect, domVar, operations)
{
}

Var MockMapDomObject::EntrySet(Var function, CallInfo callInfo, Var* args)
{
    return MockPairDomObject::Set(function, MockMapDomObject::TypdID_DOMMap, _u("Current object is not a DOMMapObject"), callInfo, args);
}

Var MockMapDomObject::EntryGet(Var function, CallInfo callInfo, Var* args)
{
    return MockPairDomObject::Get(function, MockMapDomObject::TypdID_DOMMap, _u("Current object is not a DOMMapObject"), callInfo, args);
}

void MockMapDomObject::InitIterator(Var instance, Var iterator)
{
    MockPairDomObject::InitIterator(instance, MockMapDomObject::TypdID_DOMMap, iterator, _u("Current object is not a DOMMapObject"));
}

bool MockMapDomObject::Next(Var iterator, Var *key, Var *value)
{
    return MockPairDomObject::Next(iterator, key, value);
}

DomPairIterator::DomPairIterator(MockPairDomObject *map)
    : m_pairObject(map)
{
    m_iter = m_pairObject->m_objectMap.begin();
}

bool DomPairIterator::Next(Var *key, Var *value)
{
    if (value)
    {
        *value = nullptr;
    }
    if (key)
    {
        *key = nullptr;
    }

    if (m_pairObject->m_objectMap.size() == 0 || m_iter == m_pairObject->m_objectMap.end())
    {
        return false;
    }

    if (key)
    {
        *key = m_iter->first;
    }

    if (value)
    {
        *value = m_iter->second;
    }

    ++m_iter;

    return true;
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

HRESULT MockDomObjectManager::AddMemberFunction(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, LPCWSTR prop, Var function, Var obj)
{
    HRESULT hr = S_OK;
    PropertyId propertyId;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(prop, &propertyId));

    BOOL result = false;
    return defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, obj, propertyId, function, PropertyAttributes_Enumerable, SideEffects_None, &result);
}

HRESULT MockDomObjectManager::CreateDomCtor(IActiveScriptDirect *activeScriptDirect, LPCWSTR name, ScriptFunctionObj entrypoint, Var *ctor)
{
    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;
    Var globalObject = nullptr;

    IfFailedReturn(activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations));
    IfFailedReturn(activeScriptDirect->GetGlobalObject(&globalObject));

    PropertyId ctorPropId;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(name, &ctorPropId));

    Var domConstructor;
    IfFailedReturn(activeScriptDirect->CreateConstructor(NULL, entrypoint, ctorPropId, TRUE, &domConstructor));

    BOOL result = FALSE;
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, globalObject, ctorPropId, domConstructor, static_cast<PropertyAttributes>(PropertyAttributes_Writable | PropertyAttributes_Configurable), SideEffects_None, &result);
    *ctor = domConstructor;
    return hr;
}

template <typename T>
HRESULT MockObject::CreateObject(IActiveScriptDirect *activeScriptDirect, Var prototype, LPCWSTR name, JavascriptTypeId typeId, MockPassTypeOperations* operations, Var *varObject)
{
    Assert(activeScriptDirect != nullptr);
    Assert(prototype != nullptr);
    Assert(name != nullptr);
    Assert(operations != nullptr);
    Assert(varObject != nullptr);

    *varObject = nullptr;
    HRESULT hr = S_OK;

    PropertyId objectPropId;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(name, &objectPropId));

    HTYPE externalType;
    Var domVarObject = nullptr;
    IfFailedReturn(activeScriptDirect->CreateType(typeId, nullptr, 0, prototype, NULL, operations, FALSE, objectPropId, false, &externalType));
    IfFailedReturn(activeScriptDirect->CreateTypedObject(externalType, 0, TRUE, &domVarObject));

    MockObject *domObj = new T(activeScriptDirect, domVarObject, operations);

    g_domObjectManager.AddVarToObject(domVarObject, domObj);
    *varObject = domVarObject;
    return hr;
}

HRESULT MockArrayDomObject::CreateArrayObject(IActiveScriptDirect *activeScriptDirect, Var prototype, ITypeOperations* defaultOperations, Var *varObject)
{
    Assert(defaultOperations != nullptr);
    ArrayCollectionTypeOperations * operations = new ArrayCollectionTypeOperations(defaultOperations);
    return MockObject::CreateObject<MockArrayDomObject>(activeScriptDirect, prototype, _u("DOMArrayObj"), TypeId_Unspecified, operations, varObject);
}

HRESULT MockMapDomObject::CreateMapObject(IActiveScriptDirect *activeScriptDirect, Var prototype, ITypeOperations* defaultOperations, Var *varObject)
{
    Assert(defaultOperations != nullptr);
    MockPassTypeOperations * operations = new MockPassTypeOperations(defaultOperations);
    return MockObject::CreateObject<MockMapDomObject>(activeScriptDirect, prototype, _u("DOMMapObj"), MockMapDomObject::TypdID_DOMMap, operations, varObject);
}

HRESULT MockSetDomObject::CreateSetObject(IActiveScriptDirect *activeScriptDirect, Var prototype, ITypeOperations* defaultOperations, Var *varObject)
{
    Assert(defaultOperations != nullptr);
    MockPassTypeOperations * operations = new MockPassTypeOperations(defaultOperations);
    return MockObject::CreateObject<MockSetDomObject>(activeScriptDirect, prototype, _u("DOMSetObj"), MockSetDomObject::TypdID_DOMSet, operations, varObject);
}

HRESULT MockPairDomObject::AddIteratorProperties(IActiveScriptDirect *activeScriptDirect,
    ITypeOperations* defaultScriptOperations,
    int typeIdToValidate, Var instance, Var iteratorPrototype,
    InitIteratorFunction initFn, NextFunction nextFn)
{
    HRESULT hr = S_OK;
    Var entriesFunction = JsStaticAPI::JavascriptLibrary::CreateExternalEntriesFunction(
        activeScriptDirect,
        typeIdToValidate,
        sizeof(void*),
        iteratorPrototype, initFn, nextFn);

    PropertyId symbolIterator = JsStaticAPI::JavascriptLibrary::GetPropertyIdSymbolIterator(activeScriptDirect);
    IfFailedReturn(MockDomObjectManager::AddMemberFunction(activeScriptDirect, defaultScriptOperations, _u("entries"), entriesFunction, instance));

    BOOL result = FALSE;
    IfFailedReturn(defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect,
        instance, symbolIterator, entriesFunction, PropertyAttributes_Enumerable, SideEffects_None, &result));

    Var keysFunction = JsStaticAPI::JavascriptLibrary::CreateExternalKeysFunction(
        activeScriptDirect,
        typeIdToValidate,
        sizeof(void*),
        iteratorPrototype, initFn, nextFn);

    IfFailedReturn(MockDomObjectManager::AddMemberFunction(activeScriptDirect, defaultScriptOperations, _u("keys"), keysFunction, instance));

    Var valuesFunction = JsStaticAPI::JavascriptLibrary::CreateExternalValuesFunction(
        activeScriptDirect,
        typeIdToValidate,
        sizeof(void*),
        iteratorPrototype, initFn, nextFn);
    IfFailedReturn(MockDomObjectManager::AddMemberFunction(activeScriptDirect, defaultScriptOperations, _u("values"), valuesFunction, instance));

    return hr;
}

HRESULT MockMapDomObject::CreateMapPrototypeObject(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, Var *prototype)
{
    HRESULT hr = S_OK;
    Var protoypeExternalVar = g_domObjectManager.GetMapPrototypeObject();
    if (protoypeExternalVar == nullptr)
    {
        HTYPE externalType;
        PropertyId objectProtoPropId;
        IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(_u("DOMMapObjPrototype"), &objectProtoPropId));
        IfFailedReturn(activeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, nullptr, NULL, defaultScriptOperations, FALSE, objectProtoPropId, false, &externalType));
        IfFailedReturn(activeScriptDirect->CreateTypedObject(externalType, 0, TRUE, &protoypeExternalVar));


        IfFailedReturn(MockDomObjectManager::AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("Set"), MockMapDomObject::EntrySet, protoypeExternalVar));
        IfFailedReturn(MockDomObjectManager::AddMemberVar(activeScriptDirect, defaultScriptOperations, _u("Get"), MockMapDomObject::EntryGet, protoypeExternalVar));

        Var mapIteratorPrototype;
        IfFailedReturn(CreateMapIteratorPrototype(activeScriptDirect, &mapIteratorPrototype));

        IfFailedReturn(MockPairDomObject::AddIteratorProperties(activeScriptDirect, defaultScriptOperations, MockMapDomObject::TypdID_DOMMap, protoypeExternalVar, mapIteratorPrototype,
            &MockMapDomObject::InitIterator, &MockMapDomObject::Next));

        g_domObjectManager.SetMapPrototypeObject(protoypeExternalVar);
    }
    *prototype = protoypeExternalVar;
    return hr;
}

HRESULT MockMapDomObject::CreateMapIteratorPrototype(IActiveScriptDirect *activeScriptDirect, Var *proto)
{
    return MockPairDomObject::CreateBaseIteratorPrototype(activeScriptDirect, MockMapDomObject::TypdID_DOMMap, _u("MapIteratorPrototype"), _u("MapIterator"), proto);
}

Var MockDomObjectManager::CreateDomArrayObject(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *activeScriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var undef = JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect);

    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;

    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    ReturnUndefOnFailed(hr, undef);

    Var globalObject;
    hr = activeScriptDirect->GetGlobalObject(&globalObject);
    ReturnUndefOnFailed(hr, undef);

    Var domConstructor;
    hr = CreateDomCtor(activeScriptDirect, _u("DOMArrayCtor"), TempDomConstructor, &domConstructor);
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

    BOOL result = FALSE;
    // Adding constructor property
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, protoypeExternalVar, ctorId, domConstructor, PropertyAttributes_None, SideEffects_None, &result);
    ReturnUndefOnFailed(hr, undef);

    Var domVarObject;
    hr = MockArrayDomObject::CreateArrayObject(activeScriptDirect, protoypeExternalVar, defaultScriptOperations, &domVarObject);
    ReturnUndefOnFailed(hr, undef);
    return domVarObject;
}

Var MockDomObjectManager::CreateDomMapObject(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *activeScriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var undef = JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect);

    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;

    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    ReturnUndefOnFailed(hr, undef);

    Var domConstructor;
    hr = CreateDomCtor(activeScriptDirect, _u("DOMMapCtor"), TempDomConstructor, &domConstructor);
    ReturnUndefOnFailed(hr, undef);

    Var protoypeExternalVar = nullptr;

    hr = MockMapDomObject::CreateMapPrototypeObject(activeScriptDirect, defaultScriptOperations, &protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    PropertyId ctorId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("constructor"), &ctorId);
    ReturnUndefOnFailed(hr, undef);

    BOOL result = FALSE;
    // Adding constructor property
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, protoypeExternalVar, ctorId, domConstructor, PropertyAttributes_None, SideEffects_None, &result);
    ReturnUndefOnFailed(hr, undef);

    Var domVarObject;
    hr = MockMapDomObject::CreateMapObject(activeScriptDirect, protoypeExternalVar, defaultScriptOperations, &domVarObject);
    ReturnUndefOnFailed(hr, undef);
    return domVarObject;
}

Var MockDomObjectManager::CreateDomSetObject(Var function, CallInfo callInfo, Var* args)
{
    IActiveScriptDirect *activeScriptDirect = JsStaticAPI::DataConversion::VarToScriptDirectNoRef(function);
    Var undef = JsStaticAPI::JavascriptLibrary::GetUndefined(activeScriptDirect);

    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;

    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    ReturnUndefOnFailed(hr, undef);

    Var domConstructor;
    hr = CreateDomCtor(activeScriptDirect, _u("DOMSetCtor"), TempDomConstructor, &domConstructor);
    ReturnUndefOnFailed(hr, undef);

    Var protoypeExternalVar = nullptr;

    hr = MockSetDomObject::CreateSetPrototypeObject(activeScriptDirect, defaultScriptOperations, &protoypeExternalVar);
    ReturnUndefOnFailed(hr, undef);

    PropertyId ctorId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("constructor"), &ctorId);
    ReturnUndefOnFailed(hr, undef);

    BOOL result = FALSE;
    // Adding constructor property
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, protoypeExternalVar, ctorId, domConstructor, PropertyAttributes_None, SideEffects_None, &result);
    ReturnUndefOnFailed(hr, undef);

    Var domVarObject;
    hr = MockSetDomObject::CreateSetObject(activeScriptDirect, protoypeExternalVar, defaultScriptOperations, &domVarObject);
    ReturnUndefOnFailed(hr, undef);
    return domVarObject;
}

HRESULT MockDomObjectManager::Initialize(IActiveScript *activeScript)
{
    HRESULT hr = S_OK;
    CComPtr<IActiveScriptDirect> activeScriptDirect = nullptr;
    IfFailedReturn(activeScript->QueryInterface(&activeScriptDirect));

    IfFailedReturn(AddDomArrayObjectCreatorFunction(activeScriptDirect));
    IfFailedReturn(AddDomMapObjectCreatorFunction(activeScriptDirect));
    return AddDomSetObjectCreatorFunction(activeScriptDirect);
}

HRESULT MockDomObjectManager::DomCreatorFunction(IActiveScriptDirect *activeScriptDirect, LPCWSTR name, ScriptFunctionObj entrypoint)
{
    HRESULT hr = S_OK;
    CComPtr<ITypeOperations> defaultScriptOperations;
    Var globalObject = nullptr;

    IfFailedReturn(activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations));
    IfFailedReturn(activeScriptDirect->GetGlobalObject(&globalObject));

    PropertyId ctorPropId;
    IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(name, &ctorPropId));

    Var domConstructor = nullptr;
    hr = activeScriptDirect->CreateConstructor(nullptr, entrypoint, ctorPropId, TRUE, &domConstructor);
    IfFailedReturn(hr);

    BOOL result = FALSE;
    hr = defaultScriptOperations->SetPropertyWithAttributes(activeScriptDirect, globalObject, ctorPropId, domConstructor, static_cast<PropertyAttributes>(PropertyAttributes_Writable | PropertyAttributes_Configurable), SideEffects_None, &result);

    return hr;
}

HRESULT MockDomObjectManager::AddDomArrayObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect)
{
    return MockDomObjectManager::DomCreatorFunction(activeScriptDirect, _u("CreateDomArrayObject"), MockDomObjectManager::CreateDomArrayObject);
}

HRESULT MockDomObjectManager::AddDomMapObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect)
{
    return MockDomObjectManager::DomCreatorFunction(activeScriptDirect, _u("CreateDomMapObject"), MockDomObjectManager::CreateDomMapObject);
}

HRESULT MockDomObjectManager::AddDomSetObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect)
{
    return MockDomObjectManager::DomCreatorFunction(activeScriptDirect, _u("CreateDomSetObject"), MockDomObjectManager::CreateDomSetObject);
}

MockDomObjectManager::~MockDomObjectManager()
{
    for (auto iterator = varToDomObject.begin(); iterator != varToDomObject.end(); iterator++)
    {
        delete iterator->second;
    }
}
