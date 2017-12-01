//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class HostObject :public Js::HostObjectBase
{    
public:
    HostObject(Js::ScriptContext * scriptContext, IDispatch* pDispatch, Js::DynamicType * type);    

    HostDispatch * GetHostDispatch() const { return hostDispatch; }

    virtual Js::PropertyQueryFlags HasPropertyQuery(Js::PropertyId propertyId) override;
    virtual Js::PropertyQueryFlags GetPropertyQuery(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext ) override;
    virtual Js::PropertyQueryFlags GetPropertyQuery(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext ) override;
    virtual Js::PropertyQueryFlags GetPropertyReferenceQuery(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
    virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;
    virtual BOOL DeleteProperty(Js::JavascriptString *propertyNameString, Js::PropertyOperationFlags flags) override;
    virtual Js::PropertyQueryFlags GetItemReferenceQuery(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * requestContext) override;
    virtual BOOL GetAccessors(PropertyId propertyId, Var* getter, Var* setter, Js::ScriptContext * requestContext) override;
    virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags = Js::PropertyOperation_None) override;
    virtual Js::PropertyQueryFlags HasItemQuery(uint32 index) override;
    virtual Js::PropertyQueryFlags GetItemQuery(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * requestContext) override;
    virtual BOOL SetItem(__in uint32 index, __in Js::Var value, __in Js::PropertyOperationFlags flags) override;
    virtual BOOL ToPrimitive(Js::JavascriptHint hint, Js::Var* value, Js::ScriptContext * requestContext) override;
    virtual BOOL Equals(__in Js::Var other, __out BOOL* value, Js::ScriptContext * requestContext) override;
    virtual BOOL StrictEquals(__in Js::Var other, __out BOOL* value, Js::ScriptContext * requestContext) override;
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
