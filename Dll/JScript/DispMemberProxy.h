/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

// DispMemberProxy represents a temporary lvalue formed by pairing an IDispatch
// with a DISPID that identifies an IDispatch member. The pair is primarily used to call member functions
// of host objects. It is not intended to represent the value of a host object in JS code.
class DispMemberProxy : public HostDispatch
{
public:
    DispMemberProxy(        
        HostVariant* hostVariant, 
        DISPID id, 
        Js::StaticType * type,
        const wchar_t *name) :
        HostDispatch(hostVariant, type)
    {
        m_id = id;
        m_pwszName = name;
    }

    DispMemberProxy(        
        RefCountedHostVariant* refCountHostVariant, 
        DISPID id, 
        Js::StaticType * type,
        const wchar_t *name) :
        HostDispatch(refCountHostVariant, type)
    {
        m_id = id;
        m_pwszName = name;
    }

    static Js::Var DefaultInvoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var ProfileInvoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);

    const wchar_t *GetName()
    {
        return m_pwszName;
    }

    virtual BOOL HasProperty(Js::PropertyId propertyId) override;
    virtual BOOL GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
    virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;
    virtual BOOL SetAccessor(Js::PropertyId propertyId, Js::Var setter, Js::Var getter);
    virtual Var GetTypeOfString(Js::ScriptContext * requestContext) override;
    virtual Var InvokePut(Js::Arguments args) override;
    virtual RecyclableObject * CloneToScriptContext(Js::ScriptContext* requestContext) override;

protected:
    DEFINE_VTABLE_CTOR(DispMemberProxy, HostDispatch);
    
private:
    DISPID m_id;
    const wchar_t * m_pwszName;
};
