/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class MockPassTypeOperations : public ITypeOperations
{
public:
    MockPassTypeOperations(ITypeOperations* defaultScriptOperations);
    ~MockPassTypeOperations();

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        // This object is new/delete directly
        return 1;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        return 1;
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

    // ITypeOperations
    STDMETHODIMP GetOperationUsage(OperationUsage* usage) override;
    STDMETHODIMP HasOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in PropertyId propertyId, __out BOOL* result);
    STDMETHODIMP GetOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var* value, BOOL* propertyPresent);

    STDMETHODIMP GetPropertyReference(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in PropertyId propertyId, __out Var* value, __out BOOL* propertyPresent);
    STDMETHODIMP SetProperty(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var value, BOOL* result);
    STDMETHODIMP SetPropertyWithAttributes(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var value, PropertyAttributes attributes, SideEffects effects, BOOL* result);
    STDMETHODIMP DeleteProperty(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result);
    STDMETHODIMP HasOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, BOOL* result);
    STDMETHODIMP GetOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, Var* value, BOOL* itemPresent);
    STDMETHODIMP SetItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, Var value, BOOL* result);
    STDMETHODIMP DeleteItem(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, BOOL* result);
    STDMETHODIMP GetEnumerator(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, BOOL enumNonEnumerable, __in BOOL enumSymbols, IVarEnumerator** enumerator);
    STDMETHODIMP IsEnumerable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result);
    STDMETHODIMP IsWritable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result);
    STDMETHODIMP IsConfigurable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL* result);
    STDMETHODIMP SetEnumerable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL value);
    STDMETHODIMP SetWritable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL value);
    STDMETHODIMP SetConfigurable(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, BOOL value);
    STDMETHODIMP SetAccessors(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var getter, Var setter);
    STDMETHODIMP GetAccessors(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var* getter, Var* setter, BOOL* result);
    STDMETHODIMP GetSetter(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, PropertyId propertyId, Var* setter, DescriptorFlags* flags);
    STDMETHODIMP GetItemSetter(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var index, Var* setter, DescriptorFlags* flags);
    STDMETHODIMP Equals(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var other, BOOL* result);
    STDMETHODIMP StrictEquals(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, Var other, BOOL* result);
    STDMETHODIMP QueryObjectInterface(__in IActiveScriptDirect* pActiveScriptDirect, Var instance, REFIID riid, void** ppvObj);
    STDMETHODIMP GetInitializer(InitializeMethod * initializer, int * initSlotCapacity, BOOL * hasAccessors);
    STDMETHODIMP GetFinalizer(FinalizeMethod * finalizer);
    STDMETHODIMP HasInstance(__in IActiveScriptDirect* pActiveScriptDirect, Var constructor, Var instance, BOOL* result);
    STDMETHODIMP GetNamespaceParent(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __out Var* namespaceParent);
    STDMETHODIMP CrossDomainCheck(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __out BOOL* result);
    STDMETHODIMP GetHeapObjectInfo(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in ProfilerHeapObjectInfoFlags flags, __out HostProfilerHeapObject** result, __out HeapObjectInfoReturnResult* returnResult);

private:
    CComPtr<ITypeOperations> m_defaultScriptOperations;
};

class ArrayCollectionTypeOperations : public MockPassTypeOperations
{
public:
    ArrayCollectionTypeOperations(ITypeOperations* defaultScriptOperations);
    STDMETHODIMP GetOperationUsage(__out OperationUsage* usage) override;
    STDMETHODIMP HasOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, PropertyId propertyId, __out BOOL* result) override;
    STDMETHODIMP GetOwnProperty(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, PropertyId propertyId, __out Var* value, __out BOOL* propertyPresent) override;
    STDMETHODIMP HasOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in Var index, __out BOOL* result) override;
    STDMETHODIMP GetOwnItem(__in IActiveScriptDirect* pActiveScriptDirect, __in Var instance, __in Var index, __out Var* value, __out BOOL* itemPresent) override;
};

class MockArrayDomObject
{
public:
    MockArrayDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations);
    ~MockArrayDomObject();

    static Var EntryAddObject(Var function, CallInfo callInfo, Var* args);
    static Var EntryGetObject(Var function, CallInfo callInfo, Var* args);
    static Var EntryForEach(Var function, CallInfo callInfo, Var* args);
    static Var EntryKeys(Var function, CallInfo callInfo, Var* args);
    static Var EntryValues(Var function, CallInfo callInfo, Var* args);
    static Var EntryEntries(Var function, CallInfo callInfo, Var* args);

    Var GetUndefined();
    void AddObject(Var obj);
    Var FetchGetObject(Var index, BOOL *itemPresent);

    Var m_myDomVar;
    std::vector<Var> m_objectVars;
    IActiveScriptDirect* m_activeScriptDirect;
    JavascriptTypeId m_typeId;
    ITypeOperations* m_typeOperation;
};

class MockDomObjectManager
{
public :
    ~MockDomObjectManager();

    MockArrayDomObject* GetDomObjectFromVar(Var obj);
    void AddVarToObject(Var obj, IActiveScriptDirect *scriptDirect, ITypeOperations* operation);

    static Var TempDomArrayConstructor(Var function, CallInfo callInfo, Var* args)
    {
        return args[0];
    }

    typedef Var ScriptFunctionObj(Var f, CallInfo c, Var* a);

    static HRESULT Initialize(IActiveScript *activeScript);
    static Var CreateDomArrayObject(Var function, CallInfo callInfo, Var* args);

    static Var GetUndefined(Var obj);

private:
    static HRESULT AddDomArrayObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect);
    static HRESULT AddMemberVar(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, LPCWSTR prop, ScriptFunctionObj fn, Var obj, PropertyId secondPropId = 0);

    std::map<Var, MockArrayDomObject *> varToDomArrayObject;
};

