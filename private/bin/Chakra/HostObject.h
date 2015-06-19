//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class HostObject :public Js::HostObjectBase
{    
public:
    HostObject(Js::ScriptContext * scriptContext, IDispatch* pDispatch, Js::DynamicType * type);    

    HostDispatch * GetHostDispatch() const { return hostDispatch; }

    virtual BOOL HasProperty(Js::PropertyId propertyId) override;
    virtual BOOL GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext ) override;
    virtual BOOL GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext ) override;
    virtual BOOL GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
    virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;
    virtual BOOL GetItemReference(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * requestContext) override;
    virtual BOOL GetAccessors(PropertyId propertyId, Var* getter, Var* setter, Js::ScriptContext * requestContext) override;
    virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags = Js::PropertyOperation_None) override;
    virtual BOOL HasItem(uint32 index) override;
    virtual BOOL GetItem(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * requestContext) override;
    virtual BOOL SetItem(__in uint32 index, __in Js::Var value, __in Js::PropertyOperationFlags flags) override;
    virtual BOOL ToPrimitive(Js::JavascriptHint hint, Js::Var* value, Js::ScriptContext * requestContext) override;
    virtual BOOL Equals(Js::Var other, BOOL* value, Js::ScriptContext * requestContext) override;
    virtual BOOL StrictEquals(Js::Var other, BOOL* value, Js::ScriptContext * requestContext) override;
    virtual Js::Var GetTypeOfString(Js::ScriptContext * requestContext) override;
    virtual Js::ModuleRoot * GetModuleRoot(Js::ModuleID) override;
    virtual Js::Var GetHostDispatchVar() override;
    virtual Var GetNamespaceParent(Js::Var aChild) override;
    virtual HRESULT QueryObjectInterface(REFIID riid, void** ppvObj) override;
    void SetNeedToCheckOtherItem() { needToCheckOtherItem = TRUE; }

protected:
    DEFINE_VTABLE_CTOR(HostObject, HostObjectBase);

private:
    HostDispatch* hostDispatch;        
    BOOL needToCheckOtherItem;

    BOOL TryGetDispId(Js::PropertyId, DISPID *pDispId);
    void CacheDispId(Js::PropertyId, DISPID dispId);

    typedef JsUtil::BaseDictionary<Js::PropertyId, int32, Recycler, PowerOf2SizePolicy> DispIdCacheDictionaryType;
    DispIdCacheDictionaryType * dispIdCache;
};

