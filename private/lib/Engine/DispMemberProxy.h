/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

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
        const char16 *name) :
        HostDispatch(hostVariant, type)
    {
        m_id = id;
        m_pwszName = name;
    }

    DispMemberProxy(        
        RefCountedHostVariant* refCountHostVariant, 
        DISPID id, 
        Js::StaticType * type,
        const char16 *name) :
        HostDispatch(refCountHostVariant, type)
    {
        m_id = id;
        m_pwszName = name;
    }

    static Js::Var DefaultInvoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var ProfileInvoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);

    const char16 *GetName()
    {
        return m_pwszName;
    }

    virtual Js::PropertyQueryFlags HasPropertyQuery(Js::PropertyId propertyId) override;
    virtual Js::PropertyQueryFlags GetPropertyQuery(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual Js::PropertyQueryFlags GetPropertyQuery(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual Js::PropertyQueryFlags GetPropertyReferenceQuery(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    virtual BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
    virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;
    virtual BOOL DeleteProperty(Js::JavascriptString *propertyNameString, Js::PropertyOperationFlags flags) override;
    virtual BOOL SetAccessor(Js::PropertyId propertyId, Js::Var setter, Js::Var getter);
    virtual Var GetTypeOfString(Js::ScriptContext * requestContext) override;
    virtual RecyclableObject * CloneToScriptContext(Js::ScriptContext* requestContext) override;

protected:
    DEFINE_VTABLE_CTOR(DispMemberProxy, HostDispatch);
    
private:
    DISPID m_id;
    const char16 * m_pwszName;
};
