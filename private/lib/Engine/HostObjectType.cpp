/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"

BOOL HostObject::HasProperty(Js::PropertyId propertyId)
{        
    DISPID dispId;
    BOOL fCached;
    fCached = this->TryGetDispId(propertyId, &dispId);
    if (!fCached)
    {
        if (!this->hostDispatch->GetDispIdForProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &dispId))
        {
            if (needToCheckOtherItem)
            {
                ScriptSite* scriptSite = hostDispatch->GetScriptSite();
                JsUtil::List<HostDispatch*, ArenaAllocator>* globalDispatches = scriptSite->GetGlobalDispatches();
                Assert(globalDispatches->Count() > 0);
                for (int i = 0; i < globalDispatches->Count(); i++)
                {
                    if (globalDispatches->Item(i)->GetDispIdForProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &dispId))
                    {
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }
        this->CacheDispId(propertyId, dispId);
    }
    return TRUE;
}

BOOL HostObject::GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{    
    ThreadContext * threadContext = requestContext->GetThreadContext();

    // Reject implicit call
    if (threadContext->IsDisableImplicitCall())
    {        
        *value = requestContext->GetLibrary()->GetNull();
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }

    DISPID dispId;
    BOOL fCached;
    fCached = this->TryGetDispId(propertyId, &dispId);
    if (!fCached)
    {
        if (!this->hostDispatch->GetDispIdForProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &dispId))
        {
            if (needToCheckOtherItem)
            {
                ScriptSite* scriptSite = hostDispatch->GetScriptSite();
                JsUtil::List<HostDispatch*, ArenaAllocator>* globalDispatches = scriptSite->GetGlobalDispatches();
                Assert(globalDispatches->Count() > 0);
                for (int i = 0; i < globalDispatches->Count(); i++)
                {
                    if (globalDispatches->Item(i)->GetDispIdForProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &dispId))
                    {
                        return globalDispatches->Item(i)->GetValueByDispId(dispId, value);
                    }
                }

            }
            return FALSE;
        }
    }
    if (this->hostDispatch->GetValueByDispId(dispId, value))
    {
        if (!fCached)
        {
            this->CacheDispId(propertyId, dispId);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL HostObject::GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{    
    // TODO: Consider flowing string propertyNameString through logic for GetProperty instead of obtaining
    // or creating a PropertyRecord here. Would require changing DispIdCacheDictionaryType to key by
    // PropertyRecord* instead of PropertyId, so that keys can be compared to strings directly. Would also
    // require extending TryGetDispId and CacheDispId to take JavascriptString. In CacheDispId it would
    // have to convert the string to a PropertyRecord, thus negating the benefit of skipping over that,
    // so analysis should be done to understand if there is actually room for a perf gain here.
    // Same goes for SetProperty.
    Js::PropertyRecord const * propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return HostObject::GetProperty(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
}

BOOL HostObject::GetAccessors(PropertyId propertyId, Var* getter, Var* setter, Js::ScriptContext * requestContext)
{    
    return this->hostDispatch->GetAccessors(requestContext->GetPropertyName(propertyId)->GetBuffer(), getter, setter, requestContext);
}

BOOL HostObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags)
{    
     return this->hostDispatch->SetAccessors(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), getter, setter);

}

BOOL HostObject::GetPropertyReference(Js::Var originalInstance,Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    ThreadContext * threadContext = requestContext->GetThreadContext();

    // Reject implicit call
    if (threadContext->IsDisableImplicitCall())
    {        
        *value = requestContext->GetLibrary()->GetNull();
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }

    HostDispatch* hostDispatch = this->GetHostDispatch();
    DISPID dispId;
    BOOL fCached;
    fCached = this->TryGetDispId(propertyId, &dispId);
    if (!fCached)
    {
        if (!hostDispatch->GetDispIdForProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &dispId))
        {
            if (needToCheckOtherItem)
            {
                ScriptSite* scriptSite = hostDispatch->GetScriptSite();
                JsUtil::List<HostDispatch*, ArenaAllocator>* globalDispatches = scriptSite->GetGlobalDispatches();
                Assert(globalDispatches->Count() > 0);
                for (int i = 0; i < globalDispatches->Count(); i++)
                {
                    if (globalDispatches->Item(i)->GetDispIdForProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &dispId))
                    {
                        hostDispatch = globalDispatches->Item(i);
                        hostDispatch->GetReferenceByDispId(dispId, value, GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }
    }

    if (!fCached)
    {
        this->CacheDispId(propertyId, dispId);
    }
    hostDispatch->GetReferenceByDispId(dispId, value, GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
    return TRUE;
}

BOOL HostObject::SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    // Reject implicit call (for IR::BailOutOnImplicitCallsPreOp generate by type hardcoding)
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    ThreadContext * threadContext = scriptContext->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }
    DISPID dispId;
    BOOL fCached;
    fCached = this->TryGetDispId(propertyId, &dispId);
    if (!fCached)
    {
        if (!this->hostDispatch->EnsureDispIdForProperty(scriptContext->GetPropertyName(propertyId)->GetBuffer(), &dispId))
        {
            return FALSE;
        }
    }
    if (this->hostDispatch->PutValueByDispId(dispId, value))
    {
        if (!fCached)
        {
            this->CacheDispId(propertyId, dispId);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL HostObject::SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    // TODO: Consider flowing string propertyNameString through logic for SetProperty instead of obtaining
    // or creating a PropertyRecord here. Would require changing DispIdCacheDictionaryType to key by
    // PropertyRecord* instead of PropertyId, so that keys can be compared to strings directly. Would also
    // require extending TryGetDispId and CacheDispId to take JavascriptString. In CacheDispId it would
    // have to convert the string to a PropertyRecord, thus negating the benefit of skipping over that,
    // so analysis should be done to understand if there is actually room for a perf gain here.
    // Same goes for GetProperty.
    Js::PropertyRecord const * propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return HostObject::SetProperty(propertyRecord->GetPropertyId(), value, flags, info);
}

BOOL HostObject::InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    return HostObject::SetProperty(propertyId, value, flags, info);
}

BOOL HostObject::DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags)
{    
    DISPID dispId;
    if (this->TryGetDispId(propertyId, &dispId))
    {
        return this->hostDispatch->DeletePropertyByDispId(dispId);
    }
    return this->hostDispatch->DeleteProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
}

BOOL HostObject::HasItem(uint32 index)
{
    return this->hostDispatch->HostDispatch::HasItem(index);
}

BOOL HostObject::GetItem(
        Js::Var originalInstance,
        __in uint32 index, 
        __out Js::Var* value,
        __in Js::ScriptContext* requestContext)
{    
    return this->hostDispatch->HostDispatch::GetItem(originalInstance, index, value, requestContext);
}

BOOL HostObject::SetItem(
        __in uint32 index, 
        __in Js::Var value,
        __in Js::PropertyOperationFlags flags)
{
    
    return this->hostDispatch->HostDispatch::SetItem(index, value, flags);
}

BOOL HostObject::GetItemReference(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * requestContext)
{    
    return this->hostDispatch->HostDispatch::GetItemReference(originalInstance, index, value, requestContext);
}

BOOL HostObject::ToPrimitive(Js::JavascriptHint hint, Js::Var* value, Js::ScriptContext * requestContext)
{    
    return this->hostDispatch->HostDispatch::GetDefaultValue(hint, value);
}

BOOL HostObject::StrictEquals(__in Js::Var other, __out BOOL* value, Js::ScriptContext * requestContext)
{        
    return this->hostDispatch->HostDispatch::StrictEquals(other, value, requestContext);
}

BOOL HostObject::Equals(__in Js::Var other, __out BOOL* value, Js::ScriptContext * requestContext)
{        
    return this->hostDispatch->HostDispatch::Equals(other, value, requestContext);
}
// Have to override this, otherwise we get 'function'.
Var HostObject::GetTypeOfString(Js::ScriptContext * requestContext)
{
    return requestContext->GetLibrary()->GetObjectTypeDisplayString();
}

Js::Var HostObject::GetHostDispatchVar()
{
    return hostDispatch; 
}

HRESULT HostObject::QueryObjectInterface(REFIID riid, void** ppvObj)
{    
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    HRESULT hr = NOERROR;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = this->hostDispatch->HostDispatch::QueryObjectInterface(riid, ppvObj);
        if (FAILED(hr))
        {
            hr = Js::DynamicObject::QueryObjectInterface(riid, ppvObj);
        }
    }
    END_LEAVE_SCRIPT(scriptContext);
    return hr;
}
