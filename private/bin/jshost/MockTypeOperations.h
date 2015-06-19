/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class MockTypeOperations:
    public ComObjectRoot,
    public ITypeOperations
{
private:
    OperationUsage m_operationUsage;

public:
    static HRESULT SetInstanceUnknown(Var instance, IUnknown* pUnk);
    static HRESULT GetInstanceUnknown(Var instance, IUnknown** ppUnk);
    static void __cdecl FinalizeInstanceUnknown(void *address, FinalizeMode mode);

    MockTypeOperations();
    ~MockTypeOperations();

    STDMETHODIMP QueryInterface(
        REFIID riid,
        void **ppvObj) ;

    STDMETHODIMP GetOperationUsage(
        OperationUsage *flags);

    STDMETHODIMP HasOwnProperty(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL *result);

    STDMETHODIMP GetOwnProperty(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var *value,
        BOOL *propertyPresent);

    STDMETHODIMP GetPropertyReference(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var *value,
        BOOL *propertyPresent);

    STDMETHODIMP SetProperty(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var value,
        BOOL *result);

    STDMETHODIMP SetPropertyWithAttributes(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var value,
        PropertyAttributes attributes,
        SideEffects effects,
        BOOL *result);

    STDMETHODIMP DeleteProperty(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL *result);

    STDMETHODIMP HasOwnItem(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var index,
        BOOL *result);

    STDMETHODIMP GetOwnItem(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var index,
        Var *value,
        BOOL *itemPresent);

    STDMETHODIMP SetItem(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var index,
        Var value,
        BOOL *result);

    STDMETHODIMP DeleteItem(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var index,
        BOOL *result);

    STDMETHODIMP GetEnumerator(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        BOOL enumNonEnumerable,
        BOOL enumSymbols,
        IVarEnumerator **enumerator);

    STDMETHODIMP IsEnumerable(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL *result);

    STDMETHODIMP IsWritable(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL *result);

    STDMETHODIMP IsConfigurable(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL *result);

    STDMETHODIMP SetEnumerable(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL value);

    STDMETHODIMP SetWritable(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL value);

    STDMETHODIMP SetConfigurable(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        BOOL value);

    STDMETHODIMP SetAccessors(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var getter,
        Var setter);

    STDMETHODIMP GetAccessors(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var* getter,
        Var* setter,
        BOOL* result);

    STDMETHODIMP GetSetter(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        PropertyId propertyId,
        Var* setter,
        DescriptorFlags* flags);

    STDMETHODIMP GetItemSetter(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var index,
        Var* setter,
        DescriptorFlags* flags);

    STDMETHODIMP Equals(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var other,
        BOOL *result);

    STDMETHODIMP StrictEquals(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var other,
        BOOL *result);

    STDMETHODIMP QueryObjectInterface(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        REFIID riid,
        void** ppvObj);

    STDMETHODIMP GetInitializer(
        InitializeMethod * initializer,
        int * initSlotCapacity,
        BOOL * hasAccessors);

    STDMETHODIMP GetFinalizer(
        FinalizeMethod * finalizer);

    STDMETHODIMP HasInstance(
        IActiveScriptDirect* scriptDirect,
        Var constructor,
        Var instance,
        BOOL* result);

    STDMETHODIMP GetNamespaceParent(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        Var* namespaceParent);

    STDMETHODIMP CrossDomainCheck(
        IActiveScriptDirect* scriptDirect,
        Var instance,
        BOOL* result);

    STDMETHODIMP GetHeapObjectInfo(
        IActiveScriptDirect* scriptDirect,
        Var instance, 
        ProfilerHeapObjectInfoFlags flags,
        HostProfilerHeapObject** result,
        HeapObjectInfoReturnResult* results);
};
