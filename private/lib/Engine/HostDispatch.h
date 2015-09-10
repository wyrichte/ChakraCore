//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

enum BuiltInOperation;
interface ITracker;
class HostVariant;
class RefCountedHostVariant;

// HostDispatch is used to represent IDispatch interfaces passed to us by the host,
// including named root items. They contain all the logic required to get/set/invoke host objects.
class HostDispatch : public Js::RecyclableObject
{
    friend class ScriptEngine;
    friend class ScriptSite;
    friend class HostObject;

    struct StackNode
    {
        const wchar_t* Name;
        StackNode* Next;

        StackNode(const wchar_t* name, StackNode* next)
        {
            this->Name = name;
            this->Next = next;
        }
    };

    StackNode* cycleStack;
    BOOL IsGetDispIdCycle(const wchar_t* name);

public:
    static HostDispatch * Create(Js::ScriptContext * scriptContext, LPCOLESTR itemName);
    static HostDispatch* Create(Js::ScriptContext * scriptContext, VARIANT* variant);
    static HostDispatch * Create(Js::ScriptContext * scriptContext, IDispatch* pdisp, BOOL tryTracker = TRUE);
    static HostDispatch * Create(Js::ScriptContext * scriptContext, ITracker* tracker);

    virtual RecyclableObject* GetPrototypeSpecial() override;
    virtual BOOL HasOwnProperty(PropertyId propertyId) override;
    virtual BOOL HasProperty(Js::PropertyId propertyId) override;
    virtual BOOL GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
    virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;
    virtual void ThrowIfCannotDefineProperty(Js::PropertyId propId, Js::PropertyDescriptor descriptor);
    virtual void ThrowIfCannotGetOwnPropertyDescriptor(Js::PropertyId propId);
    virtual BOOL GetDefaultPropertyDescriptor(Js::PropertyDescriptor& descriptor);
    virtual BOOL HasItem(uint32 index) override;
    virtual BOOL GetItemReference(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, Js::ScriptContext * requestContext) override;
    virtual BOOL GetItem(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, Js::ScriptContext * requestContext) override;
    virtual Js::DescriptorFlags GetItemSetter(uint32 index, Var* setterValue, Js::ScriptContext* requestContext) override;
    virtual BOOL SetItem(__in uint32 index, __in Js::Var value, __in Js::PropertyOperationFlags flags) override;
    virtual BOOL DeleteItem(uint32 index, Js::PropertyOperationFlags flags) override;
    virtual BOOL ToPrimitive(Js::JavascriptHint hint, Js::Var* value, Js::ScriptContext * requestContext) override;
    virtual BOOL Equals(Js::Var other, BOOL* value, Js::ScriptContext * requestContext) override;
    virtual BOOL StrictEquals(Js::Var other, BOOL* value, Js::ScriptContext * requestContext) override sealed;
    virtual BOOL GetEnumerator(__in BOOL enumNonEnumerable, __out Js::Var* enumerator, Js::ScriptContext * requestContext, __in bool preferSnapshotSemantics = true, __in bool enumSymbols = false) override;
    virtual BOOL SetAccessors(Js::PropertyId propertyId, Js::Var getter, Js::Var setter, Js::PropertyOperationFlags flags = Js::PropertyOperation_None) override;
    virtual BOOL GetAccessors(Js::PropertyId propertyId, Js::Var* getter, Js::Var* setter, Js::ScriptContext * requestContext) override;
    virtual Js::DescriptorFlags GetSetter(PropertyId propertyId, Var* setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual Js::DescriptorFlags GetSetter(Js::JavascriptString* propertyNameString, Var* setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual Var  GetTypeOfString(Js::ScriptContext * requestContext) override;
    virtual BOOL HasInstance(Js::Var instance, Js::ScriptContext* scriptContext, Js::IsInstInlineCache* inlineCache = NULL) override;
    virtual BOOL IsWritable(PropertyId propertyId)  override;
    virtual BOOL IsEnumerable(PropertyId propertyId)  override;
    virtual BOOL IsConfigurable(PropertyId propertyId)  override;
    virtual BOOL Seal() override;
    virtual BOOL Freeze() override;
    virtual BOOL PreventExtensions() override;
    virtual BOOL IsSealed() override;
    virtual BOOL IsFrozen() override;
    virtual BOOL IsExtensible() override;
    virtual HRESULT QueryObjectInterface(REFIID riid, void** ppvObj) override;
    virtual Var InvokePut(Js::Arguments args) override;
    virtual BOOL GetRemoteTypeId(Js::TypeId* typeId) override;
    virtual Js::DynamicObject* GetRemoteObject() override;
    virtual RecyclableObject * CloneToScriptContext(Js::ScriptContext* requestContext) override;
    virtual BOOL ToString(Js::Var* value, Js::ScriptContext* scriptContext) override;

    static BOOL Is(Var instance);
    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override sealed;

    ScriptSite* GetScriptSite() const { return scriptSite; }
    IDispatch* GetDispatchNoRef();
    IDispatch* GetDispatch();
    BOOL CanSupportIDispatchEx() const;
    static IDispatch*& FastGetDispatchNoRef(HostVariant* hostVariant);

    BOOL HasProperty(const wchar_t * name);
    BOOL GetValue(const wchar_t * name, Js::Var *pValue);
    BOOL GetPropertyReference(const wchar_t * name, Js::Var *pValue);
    BOOL PutValue(const wchar_t * name, Js::Var value);
    BOOL DeleteProperty(const wchar_t * name);
    BOOL GetDefaultValue(Js::JavascriptHint hint, Js::Var* value, BOOL throwException = TRUE);
    BOOL SetAccessors(const wchar_t * name, Js::Var getter, Js::Var setter);
    BOOL GetAccessors(const wchar_t * name, Js::Var* getter, Js::Var* setter, Js::ScriptContext * requestContext);

    BOOL GetDispIdForProperty(const wchar_t * name, DISPID *pDispId);
    BOOL EnsureDispIdForProperty(const wchar_t * name, DISPID *pDispId);

    BOOL GetValueByDispId(DISPID dispId, Js::Var *pValue);
    void GetReferenceByDispId(DISPID dispId, Js::Var *pValue, const wchar_t * name);
    BOOL PutValueByDispId(DISPID dispId, Js::Var value);
    BOOL DeletePropertyByDispId(DISPID dispId);

    Js::Var InvokeByDispId(Js::Arguments args, DISPID id, BOOL fIsPut = false);
    BOOL InvokeBuiltInOperationRemotely(Js::JavascriptMethod entryPoint, Js::Arguments args, Js::Var* result) override sealed;
    void RemoveFromPrototype(Js::ScriptContext * requestContext) override;
    void AddToPrototype(Js::ScriptContext * requestContext) override;
    void SetPrototype(RecyclableObject* newPrototype) override;

    HRESULT InvokeMarshaled(DISPID id, WORD invokeFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei);

    static Js::Var Invoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    BOOL IsInstanceOf(Js::Var prototypeProxy);
    
    VARIANT* GetVariant() const;
    static __declspec(noreturn) void HandleDispatchError(Js::ScriptContext * scriptContext, HRESULT hr, EXCEPINFO* exceptInfo);
    static HRESULT QueryInterfaceWithLeaveScript(IUnknown* obj, REFIID iid, void** returnObj, Js::ScriptContext* scriptContext);

#if DBG
    Js::ScriptContext * GetScriptContext();
#endif

protected:
    DEFINE_VTABLE_CTOR(HostDispatch, RecyclableObject);
    HostDispatch(HostVariant* hostVariant, Js::StaticType * type);
    HostDispatch(RefCountedHostVariant* refCountedHostVariant, Js::StaticType* type);

    RefCountedHostVariant* refCountedHostVariant;

    __declspec(noreturn) void HandleDispatchError(HRESULT hr, EXCEPINFO* exceptInfo);


private:
    ScriptSite * scriptSite;
    LIST_ENTRY linkList;

    IDispatch* GetIDispatchAddress();
    __inline HRESULT EnsureDispatch();

    HRESULT GetHostVariantWrapper(__out HostVariant** ppHostVariant);

    HRESULT CallInvokeEx(DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei);
    HRESULT CallInvoke(DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei);
    HRESULT GetDispID(LPCWSTR psz, ULONG flags, DISPID *pid);
    HRESULT GetIDsOfNames(LPCWSTR psz, DISPID *pid);
    HostVariant* GetHostVariant() const;
    HRESULT QueryObjectInterfaceInScript(REFIID riid, void** ppvObj);

    __inline BOOL IsGlobalDispatch();

    static Var CreateDispatchWrapper(Var object, Js::ScriptContext * sourceScriptContext, Js::ScriptContext * destScriptContext);
    static BOOL EqualsHelper(HostDispatch *left, HostDispatch *right, BOOL *value, BOOL strictEqual);
    BOOL GetPropertyFromRootObject(Var originalInstance, PropertyId propertyId, Var *value, Js::ScriptContext* requestContext, BOOL* wasGetAttempted);
    BOOL GetPropertyReferenceFromRootObject(Var originalInstance, PropertyId propertyId, Var *value, Js::ScriptContext* requestContext, BOOL* wasGetAttempted);
    Js::RecyclableObject* GetModuleRootCallerObject();
    static BOOL GetBuiltInOperationFromEntryPoint(Js::JavascriptMethod entryPoint, BuiltInOperation* operation);
    static BOOL InvokeBuiltInOperationRemotelyHelper(Js::JavascriptMethod entryPoint, Js::Var* result);

    BOOL SetPropertyCore(const wchar_t* propertyName, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info);
    Js::DescriptorFlags GetSetterCore(const wchar_t* propertyName, Var* setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext);
};
