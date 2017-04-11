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

class MockObject
{
public:
    MockObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations);
    virtual ~MockObject();
    Var GetUndefined();

    template <typename T>
    static HRESULT CreateObject(IActiveScriptDirect *activeScriptDirect, Var prototype, LPCWSTR name, JavascriptTypeId typeId, MockPassTypeOperations* operations, Var *varObject);

    HRESULT ThrowException(LPCWSTR text, HRESULT hr = E_INVALIDARG);
    static HRESULT ThrowException(Var function, LPCWSTR text, HRESULT hr = E_INVALIDARG);
    static HRESULT ThrowException(IActiveScriptDirect* activeScriptDirect, LPCWSTR text, HRESULT hr = E_INVALIDARG);

    Var m_myDomVar;
    IActiveScriptDirect* m_activeScriptDirect;
    JavascriptTypeId m_typeId;
    ITypeOperations* m_typeOperation;
};

class MockArrayDomObject : public MockObject
{
public:
    MockArrayDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations);
    ~MockArrayDomObject() {};

    static Var EntryAddObject(Var function, CallInfo callInfo, Var* args);
    static Var EntryGetObject(Var function, CallInfo callInfo, Var* args);
    static Var EntryForEach(Var function, CallInfo callInfo, Var* args);
    static Var EntryKeys(Var function, CallInfo callInfo, Var* args);
    static Var EntryValues(Var function, CallInfo callInfo, Var* args);
    static Var EntryEntries(Var function, CallInfo callInfo, Var* args);

    void AddObject(Var obj);
    Var FetchGetObject(Var index, BOOL *itemPresent);

    static HRESULT CreateArrayObject(IActiveScriptDirect *activeScriptDirect, Var prototype, ITypeOperations* defaultOperations, Var *varObject);
    std::vector<Var> m_objectVars;
};

typedef void(*InitIteratorFunction)(Var, Var);
typedef bool (*NextFunction)(Var, Var *, Var *);

class MockPairDomObject : public  MockObject
{
public:
    MockPairDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations);
    ~MockPairDomObject() {}

    static Var Get(Var function, int typeIdToValidate, LPCWSTR errorMessage, CallInfo callInfo, Var* args);
    static Var Set(Var function, int typeIdToValidate, LPCWSTR errorMessage, CallInfo callInfo, Var* args);

    static void InitIterator(Var instance, int typeIdToValidate, Var iterator, LPCWSTR errorMessage);
    static bool Next(Var iterator, Var *key, Var *value);
    static HRESULT CreateBaseIteratorPrototype(IActiveScriptDirect *activeScriptDirect, int typeIdToValidate, LPCWSTR propName, LPCWSTR stringTag, Var *proto);
    static HRESULT AddIteratorProperties(IActiveScriptDirect *activeScriptDirect,
        ITypeOperations* defaultScriptOperations,
        int typeIdToValidate, Var instance, Var iteratorPrototype,
        InitIteratorFunction initFn, NextFunction nextFn);

    std::map<Var, Var> m_objectMap;
};

class MockMapDomObject : public MockPairDomObject
{
public:
    MockMapDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations);

    static Var EntrySet(Var function, CallInfo callInfo, Var* args);
    static Var EntryGet(Var function, CallInfo callInfo, Var* args);

    static HRESULT CreateMapObject(IActiveScriptDirect *activeScriptDirect, Var prototype, ITypeOperations* defaultOperations, Var *varObject);
    static HRESULT CreateMapPrototypeObject(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultOperations, Var *prototype);
    static HRESULT CreateMapIteratorPrototype(IActiveScriptDirect *activeScriptDirect, Var *proto);

    static void InitIterator(Var instance, Var iterator);
    static bool Next(Var iterator, Var *key, Var *value);

    static const int TypdID_DOMMap;
};

class MockSetDomObject : public MockPairDomObject
{
public:
    MockSetDomObject(IActiveScriptDirect *activeScriptDirect, Var domVar, ITypeOperations *operations);

    static Var EntrySet(Var function, CallInfo callInfo, Var* args);
    static Var EntryGet(Var function, CallInfo callInfo, Var* args);

    static HRESULT CreateSetObject(IActiveScriptDirect *activeScriptDirect, Var prototype, ITypeOperations* defaultOperations, Var *varObject);
    static HRESULT CreateSetPrototypeObject(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultOperations, Var *prototype);
    static HRESULT CreateSetIteratorPrototype(IActiveScriptDirect *activeScriptDirect, Var *proto);

    static void InitIterator(Var instance, Var iterator);
    static bool Next(Var iterator, Var *key, Var *value);

    static const int TypdID_DOMSet;
};

class DomPairIterator
{
public:
    DomPairIterator(MockPairDomObject *map);

    bool Next(Var *key, Var *value);
    MockPairDomObject *m_pairObject;
    std::map<Var, Var>::iterator m_iter;
};

class MockDomObjectManager
{
public :
    MockDomObjectManager()
        : m_mapPrototypeObject(nullptr), m_setPrototypeObject(nullptr)
    { }

    ~MockDomObjectManager();

    MockObject* GetDomObjectFromVar(Var obj);
    void AddVarToObject(Var obj, MockObject * domObject);

    Var GetMapPrototypeObject() { return m_mapPrototypeObject;  }
    void SetMapPrototypeObject(Var o) { m_mapPrototypeObject = o; }
    Var GetSetPrototypeObject() { return m_setPrototypeObject; }
    void SetSetPrototypeObject(Var o) { m_setPrototypeObject = o; }

    static Var TempDomConstructor(Var function, CallInfo callInfo, Var* args)
    {
        return args[0];
    }

    typedef Var ScriptFunctionObj(Var f, CallInfo c, Var* a);

    static HRESULT Initialize(IActiveScript *activeScript);
    static Var CreateDomArrayObject(Var function, CallInfo callInfo, Var* args);
    static Var CreateDomMapObject(Var function, CallInfo callInfo, Var* args);
    static Var CreateDomSetObject(Var function, CallInfo callInfo, Var* args);

    static Var GetUndefined(Var obj);

    static HRESULT AddMemberVar(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, LPCWSTR prop, ScriptFunctionObj fn, Var obj, PropertyId secondPropId = 0);
    static HRESULT AddMemberFunction(IActiveScriptDirect *activeScriptDirect, ITypeOperations* defaultScriptOperations, LPCWSTR prop, Var function, Var obj);

private:
    static HRESULT CreateDomCtor(IActiveScriptDirect *activeScriptDirect, LPCWSTR name, ScriptFunctionObj entrypoint, Var *ctor);

    static HRESULT DomCreatorFunction(IActiveScriptDirect *activeScriptDirect, LPCWSTR name, ScriptFunctionObj entrypoint);
    static HRESULT AddDomArrayObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect);
    static HRESULT AddDomMapObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect);
    static HRESULT AddDomSetObjectCreatorFunction(IActiveScriptDirect *activeScriptDirect);

    std::map<Var, MockObject *> varToDomObject;
    Var m_mapPrototypeObject;
    Var m_setPrototypeObject;
};

