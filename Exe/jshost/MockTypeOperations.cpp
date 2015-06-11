/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

HRESULT MockTypeOperations::SetInstanceUnknown(Var instance, IUnknown* pUnk)
{
    HRESULT hr = S_OK;

    void* buffer;
    IfFailGo(ScriptDirect::JsVarToExtension(instance, &buffer));

    static_cast<IUnknown**>(buffer)[0] = pUnk;
    pUnk->AddRef(); // Saved a reference in extension buffer

Error:
    return hr;
}

HRESULT MockTypeOperations::GetInstanceUnknown(Var instance, IUnknown** ppUnk)
{
    HRESULT hr = S_OK;

    void* buffer;
    IfFailGo(ScriptDirect::JsVarToExtension(instance, &buffer));

    *ppUnk = static_cast<IUnknown**>(buffer)[0];
    (*ppUnk)->AddRef();

Error:
    return hr;
}

void __cdecl MockTypeOperations::FinalizeInstanceUnknown(void* address, FinalizeMode mode)
{
    switch (mode)
    {
    case FinalizeMode_Finalize:
        break;

    case FinalizeMode_Dispose:
        {
            Var instance = address;
            CComPtr<IUnknown> pUnk;
            if (SUCCEEDED(GetInstanceUnknown(instance, &pUnk)))
            {
                // Explictly Release() the instance unknown pointer
                ((IUnknown*)pUnk)->Release();
            }
        }
        break;

    default:
        Assert(false);
        break;
    }
}

MockTypeOperations::MockTypeOperations()
    : m_operationUsage()
{
}

MockTypeOperations::~MockTypeOperations()
{
}

STDMETHODIMP MockTypeOperations::QueryInterface(
    REFIID riid,
    void **ppvObj)
{
    IfNullReturnError(ppvObj, E_POINTER);

    QI_IMPL_INTERFACE(IUnknown);
    QI_IMPL_INTERFACE(ITypeOperations);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP MockTypeOperations::GetOperationUsage(
    OperationUsage *usageRef)
{
    *usageRef = m_operationUsage;
    return NOERROR;
}

STDMETHODIMP MockTypeOperations::HasOwnProperty(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL *result)
{
    *result = FALSE;
    return NO_ERROR;
}

STDMETHODIMP MockTypeOperations::GetOwnProperty(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var *value,
    BOOL *propertyPresent)
{
    *propertyPresent = FALSE;
    return NO_ERROR;
}

STDMETHODIMP MockTypeOperations::GetPropertyReference(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var *value,
    BOOL *propertyPresent)
{
    *propertyPresent = FALSE;
    return NO_ERROR;
}

STDMETHODIMP MockTypeOperations::SetProperty(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var value,
    BOOL *result)
{
    *result = FALSE;
    return NO_ERROR;
}

STDMETHODIMP MockTypeOperations::SetPropertyWithAttributes(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var value,
    PropertyAttributes attributes,
    SideEffects effects,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::DeleteProperty(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::HasOwnItem(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var index,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::GetOwnItem(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var index,
    Var *value,
    BOOL *itemPresent)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::SetItem(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var index,
    Var value,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::DeleteItem(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var index,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::GetEnumerator(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    BOOL enumNonEnumerable,
    BOOL enumSymbols,
    IVarEnumerator **enumerator)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::IsEnumerable(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::IsWritable(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::IsConfigurable(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL *result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::SetEnumerable(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL value)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::SetWritable(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL value)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::SetConfigurable(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    BOOL value)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::SetAccessors(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var getter,
    Var setter)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::GetAccessors(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var* getter,
    Var* setter,
    BOOL* result)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::GetSetter(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    PropertyId propertyId,
    Var* setter,
    DescriptorFlags* flags)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::GetItemSetter(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var index,
    Var* setter,
    DescriptorFlags* flags)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::Equals(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var other,
    BOOL *result)
{
    *result = (instance == other);
    return S_OK;
}

STDMETHODIMP MockTypeOperations::StrictEquals(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var other,
    BOOL *result)
{
    *result = (instance == other);
    return S_OK;
}

STDMETHODIMP MockTypeOperations::QueryObjectInterface(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    REFIID riid,
    void** ppvObj)
{
    HRESULT hr = S_OK;

    CComPtr<IUnknown> pUnk;
    IfFailGo(GetInstanceUnknown(instance, &pUnk));

    hr = pUnk->QueryInterface(riid, ppvObj);

Error:
    return hr;
}

STDMETHODIMP MockTypeOperations::GetInitializer(
    InitializeMethod * initializer,
    int * initSlotCapacity,
    BOOL * hasAccessors)
{
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::GetFinalizer(
    FinalizeMethod * finalizer)
{
    *finalizer = FinalizeInstanceUnknown;
    return S_OK;
}

STDMETHODIMP MockTypeOperations::HasInstance(
    IActiveScriptDirect* scriptDirect,
    Var constructor,
    Var instance,
    BOOL* result)
{
    *result = TRUE;
    return S_OK;
}

STDMETHODIMP MockTypeOperations::GetNamespaceParent(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    Var* namespaceParent)
{
    *namespaceParent = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP MockTypeOperations::CrossDomainCheck(
    IActiveScriptDirect* scriptDirect,
    Var instance,
    BOOL* result)
{
    *result = FALSE;
    return S_OK;
}

STDMETHODIMP MockTypeOperations::GetHeapObjectInfo(
	IActiveScriptDirect* scriptDirect,
    Var instance, 
    ProfilerHeapObjectInfoFlags flags,
    HostProfilerHeapObject** result,
    HeapObjectInfoReturnResult* results)
{
    *result = NULL;
    *results = HeapObjectInfoReturnResult_NoResult;
    return E_NOTIMPL;
}


