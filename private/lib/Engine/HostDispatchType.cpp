/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#include "hostdispatchenumerator.h"

Js::RecyclableObject * HostDispatch::GetPrototypeSpecial()
{    
    Var result;
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;

    if (InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::GetPrototypeOf.GetOriginalEntryPoint(), args, &result))
    {
        return Js::RecyclableObject::FromVar(result);
    }   
    return GetType()->GetPrototype();
}

Js::PropertyQueryFlags HostDispatch::HasPropertyQuery(PropertyId propertyId)
{
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return Js::PropertyQueryFlags::Property_NotFound;
    }

    return this->HasProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer()) ? Js::PropertyQueryFlags::Property_Found : Js::PropertyQueryFlags::Property_NotFound;
}

BOOL HostDispatch::HasOwnProperty(PropertyId propertyId)
{
    return this->HasProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
}

Js::RecyclableObject* HostDispatch::GetModuleRootCallerObject()
{
    // Try to be bug-compatible with v5.8, which uses the current root object (possibly a ModuleRoot)
    // to do the lookup. We need to walk the stack to find out which module we're running in.
    Js::ScriptContext* scriptContext = GetScriptContext();
    Js::GlobalObject* globalObject = scriptContext->GetGlobalObject();
    Js::JavascriptFunction *func = nullptr;
    if (Js::JavascriptStackWalker::GetCaller(&func, scriptContext))
    {
        // Found a JS caller. Get its module ID.

        Assert(func && func->GetFunctionProxy());
        Js::ModuleID moduleID = func->GetFunctionProxy()->GetUtf8SourceInfo()->GetSrcInfo()->moduleID;
        if (moduleID != kmodGlobal)
        {
            // We're not in the global module. Use the current ModuleRoot.
            Js::HostObjectBase * hostObject = globalObject->GetHostObject();
            Js::RecyclableObject *instance = Js::RecyclableObject::FromVar(hostObject->GetModuleRoot(moduleID));
            return instance;
        }
    }
    return NULL;
}

BOOL HostDispatch::GetPropertyFromRootObject(Var originalInstance, PropertyId propertyId, Var *value, Js::ScriptContext* requestContext, BOOL* wasGetAttempted)
{
    // Try to short-circuit the DOM lookup for global JS properties through "window".
    *wasGetAttempted = FALSE;
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    Js::GlobalObject * globalObj = scriptContext->GetGlobalObject();

    // IE9+ mode, or we're in the global module.
    if (globalObj->HasOwnProperty(propertyId))
    {
        *wasGetAttempted = TRUE;
        return globalObj->GetProperty(originalInstance, propertyId, value, NULL, requestContext);
    }

    return FALSE;
}

BOOL HostDispatch::GetPropertyReferenceFromRootObject(Var originalInstance, PropertyId propertyId, Var *value, Js::ScriptContext* requestContext, BOOL* wasGetAttempted)
{
    // Try to short-circuit the DOM lookup for global JS properties through "window".
    *wasGetAttempted = FALSE;

    Js::ScriptContext * scriptContext = this->GetScriptContext();
    Js::GlobalObject * globalObj = scriptContext->GetGlobalObject(); 

    // IE9+ mode, or we're in the global module.
    if (globalObj->HasOwnProperty(propertyId))
    {
        *wasGetAttempted = TRUE;
        return globalObj->GetPropertyReference(originalInstance, propertyId, value, NULL, requestContext);
    }

    return FALSE;
}


Js::PropertyQueryFlags HostDispatch::GetPropertyQuery(Var originalInstance, PropertyId propertyId, Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{    
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    BOOL result, wasGetAttempted = FALSE;
    if(IsGlobalDispatch()  && scriptContext->CanOptimizeGlobalLookup())
    {
        if (this->GetPropertyFromRootObject(originalInstance, propertyId, value, requestContext, &wasGetAttempted))
        {
            Assert(wasGetAttempted);
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
            return Js::PropertyQueryFlags::Property_Found;
        }
        if (wasGetAttempted)
        {
            return Js::PropertyQueryFlags::Property_NotFound;
        }
    }

    ThreadContext * threadContext = scriptContext->GetThreadContext();

    // Reject implicit call
    if (threadContext->IsDisableImplicitCall())
    {        
        *value = requestContext->GetLibrary()->GetNull();
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return Js::PropertyQueryFlags::Property_Found;
    }

    result = this->GetValue(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), value);
    if (result)
    {
        *value = Js::CrossSite::MarshalVar(requestContext, *value);
    }
    return Js::JavascriptConversion::BooleanToPropertyQueryFlags(result);
}

Js::PropertyQueryFlags HostDispatch::GetPropertyQuery(Var originalInstance, Js::JavascriptString* propertyNameString, Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    // TODO: Consider flowing string propertyNameString through logic for GetProperty instead of obtaining
    // or creating a PropertyRecord here. Would require extending GetPropertyFromRootObject and HasOwnProperty
    // to accept JavascriptString instead of PropertyId.
    Js::PropertyRecord const * propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return HostDispatch::GetPropertyQuery(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
}

Js::PropertyQueryFlags HostDispatch::GetPropertyReferenceQuery(Js::Var originalInstance,PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{    
    Js::ScriptContext * scriptContext = GetScriptContext();
    BOOL result, wasGetAttempted = FALSE;
    if(IsGlobalDispatch() && scriptContext->CanOptimizeGlobalLookup())
    {            
        if (this->GetPropertyReferenceFromRootObject(originalInstance, propertyId, value, requestContext, &wasGetAttempted))
        {
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
            Assert(wasGetAttempted);
            return Js::PropertyQueryFlags::Property_Found;
        }
        if (wasGetAttempted)
        {
            return Js::PropertyQueryFlags::Property_NotFound;
        }
    }    
    ThreadContext * threadContext = scriptContext->GetThreadContext();

    // Reject implicit call
    if (threadContext->IsDisableImplicitCall())
    {        
        *value = requestContext->GetLibrary()->GetNull();
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return Js::PropertyQueryFlags::Property_Found;
    }

    result = this->GetPropertyReference(scriptContext->GetPropertyName(propertyId)->GetBuffer(), value);
    if (result)
    {
        *value = Js::CrossSite::MarshalVar(requestContext, *value);
    }
    return Js::JavascriptConversion::BooleanToPropertyQueryFlags(result);
}

BOOL HostDispatch::SetProperty(PropertyId propertyId, Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    return SetPropertyCore(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), value, flags, info);
}

BOOL HostDispatch::SetProperty(Js::JavascriptString* propertyNameString, Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    // We need a BSTR like string with the byte length preceding the string buffer.
    // Just get or create the PropertyRecord which is already laid out this way.
    Js::PropertyRecord const* propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return SetPropertyCore(propertyRecord->GetBuffer(), value, flags, info);
}

BOOL HostDispatch::SetPropertyCore(const char16* propertyName, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    // Reject implicit call (for IR::BailOutOnImplicitCallsPreOp generate by type hardcoding)
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    ThreadContext * threadContext = scriptContext->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }
    value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
    return this->PutValue(propertyName, value);
}

BOOL HostDispatch::SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags)
{    
    Js::ScriptContext* scriptContext = GetScriptContext();
    if (getter)
    {
        getter = Js::CrossSite::MarshalVar(scriptContext, getter);
    }
    if (setter)
    {
        setter = Js::CrossSite::MarshalVar(scriptContext, setter);
    }
    return this->SetAccessors(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), getter, setter);
}

BOOL HostDispatch::GetAccessors(PropertyId propertyId, Var* getter, Var* setter, Js::ScriptContext * requestContext)
{    
    BOOL result = this->GetAccessors(requestContext->GetPropertyName(propertyId)->GetBuffer(), getter, setter, requestContext);
    if (result)
    {
        if (*getter)
        {
            *getter = Js::CrossSite::MarshalVar(requestContext, *getter);
        }
        if (*setter)
        {
            *setter = Js::CrossSite::MarshalVar(requestContext, *setter);
        }
    }
    return result;
}

Js::DescriptorFlags HostDispatch::GetSetter(PropertyId propertyId, Js::Var *setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    // Reject implicit call
    ThreadContext * threadContext = requestContext->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        *setterValue = requestContext->GetLibrary()->GetNull();
        Js::PropertyValueInfo::SetNoCache(info, this);
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return Js::DescriptorFlags::Accessor;
    }

    return GetSetterCore(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), setterValue, info, requestContext);
}

Js::DescriptorFlags HostDispatch::GetSetter(Js::JavascriptString* propertyNameString, Js::Var *setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    return GetSetterCore(propertyNameString->GetSz(), setterValue, info, requestContext);
}

Js::DescriptorFlags HostDispatch::GetSetterCore(const char16* propertyName, Var* setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    // TODO: what to do with requestScriptContext?    
    Js::Var getter;
    Js::PropertyValueInfo::SetNoCache(info, this);
    // in HostDispatch case, we can only have setter here (can't have the case of having property but not setter)
    if(this->GetAccessors(propertyName, &getter, setterValue, requestContext))
    {
        if (*setterValue)
        {
            *setterValue = Js::CrossSite::MarshalVar(requestContext, *setterValue);
        }
        return (Js::DescriptorFlags)1; // Accessor
    }

    // TODO: Get descriptor...
    return (Js::DescriptorFlags)0; // None
}

Js::DescriptorFlags HostDispatch::GetItemSetter(uint32 index, Js::Var* setterValue, Js::ScriptContext* requestContext)
{
    Js::JavascriptString *propertyName = requestContext->GetIntegerString(index);
    Js::Var getter;
    // in HostDispatch case, we can only have setter here (can't have the case of having property but not setter)
    if (this->GetAccessors(propertyName->GetSz(), &getter, setterValue, requestContext))
    {
        if (*setterValue)
        {
            *setterValue = Js::CrossSite::MarshalVar(requestContext, *setterValue);
        }
        return (Js::DescriptorFlags)1; // Accessor
    }

    // TODO: Get descriptor...
    return (Js::DescriptorFlags)0; // None
}

BOOL HostDispatch::InitProperty(PropertyId propertyId, Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{   
    // TODO: Populate info
    value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }

    return this->PutValue(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), value);
}

BOOL HostDispatch::DeleteProperty(PropertyId propertyId, Js::PropertyOperationFlags flags)
{
    if (GetScriptContext()->GetThreadContext()->IsDisableImplicitCall())
    {                
        GetScriptContext()->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }
    return this->DeleteProperty(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());
}

BOOL HostDispatch::DeleteProperty(Js::JavascriptString *propertyNameString, Js::PropertyOperationFlags flags)
{
    if (GetScriptContext()->GetThreadContext()->IsDisableImplicitCall())
    {
        GetScriptContext()->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }
    Js::PropertyRecord const *propertyRecord = nullptr;
    if (Js::JavascriptOperators::ShouldTryDeleteProperty(this, propertyNameString, &propertyRecord))
    {
        Assert(propertyRecord);
        return DeleteProperty(propertyRecord->GetPropertyId(), flags);
    }

    return TRUE;
}

BOOL HostDispatch::ToPrimitive(Js::JavascriptHint hint, Var* value, Js::ScriptContext * requestContext)
{    
    BOOL result = this->GetDefaultValue(hint, value);
    if (result)
    {
        *value = Js::CrossSite::MarshalVar(requestContext, *value);

        // If GetDefaultValue returned us another HostDispatch (the case with "Shell.Application".namespace), 
        // JavascriptConversion::ToString will Assert that value we return should have been a primitive (not a HostDispatch).
        // We detect this here, and return false so JavascriptConversion::ToPrimitive can throw an error instead of Asserting.
        // For more info see BLUE: 147013.
        if (HostDispatch::Is(*value))
        {
            return false;
        }
    }
    return result;
}

BOOL HostDispatch::GetEnumerator(Js::JavascriptStaticEnumerator * enumerator, Js::EnumeratorFlags flags, Js::ScriptContext* requestContext, Js::ForInCache * forInCache)
{    
    if (!this->CanSupportIDispatchEx())
    {
        enumerator->Clear(flags, requestContext);
        return FALSE;
    }    
    HostDispatch * currentHostDispatch;
    if (GetScriptContext() != requestContext)
    {
        currentHostDispatch = HostDispatch::Create(requestContext, this->GetDispatchNoRef());
    }
    else
    {
        currentHostDispatch = this;
    }
    return enumerator->Initialize(RecyclerNew(requestContext->GetRecycler(), HostDispatchEnumerator, currentHostDispatch),
        nullptr, nullptr, flags, requestContext, nullptr);
}

BOOL HostDispatch::StrictEquals(__in Var other, __out BOOL* value, Js::ScriptContext * requestContext)
{
    if (this == other)
    {
        *value = TRUE;
        return TRUE;
    }

    HostDispatch *right;
    Js::TypeId rightType = Js::JavascriptOperators::GetTypeId(other);    

    if (rightType == Js::TypeIds_GlobalObject)
    {
        Js::GlobalObject* globalObject = (Js::GlobalObject*)other;
        if (globalObject->GetHostObject())
        {
            right = ((HostObject*)globalObject->GetHostObject())->GetHostDispatch();
        }
        else
        {
            *value = FALSE;
            return FALSE;
        }
    }
    else if (rightType == Js::TypeIds_HostDispatch)
    {
        right = (HostDispatch*)other;
    }
    else
    {
        AssertMsg(FALSE, "shouldn't be here");
        *value = FALSE;
        return TRUE;
    }

    return HostDispatch::EqualsHelper(this, right, value, TRUE);
}

BOOL HostDispatch::Equals(__in Var other, __out BOOL* value, Js::ScriptContext * requestContext)
{
    if (this == other)
    {
        *value = TRUE;
        return TRUE;
    }
       
    Js::TypeId otherType = Js::JavascriptOperators::GetTypeId(other);
    
    switch (otherType)
    {
    case Js::TypeIds_Undefined:
    case Js::TypeIds_Null:
        *value = FALSE;
        return TRUE;

    case Js::TypeIds_HostDispatch:
        {            
            HostDispatch* right = (HostDispatch*)other;
            return HostDispatch::EqualsHelper(this, right, value, FALSE);
        }
    case Js::TypeIds_GlobalObject:
        {
            Js::GlobalObject* globalObject = (Js::GlobalObject*)other;
            if (globalObject->GetHostObject())
            {
                HostDispatch* pGlobalHostDispatch = ((HostObject*)globalObject->GetHostObject())->GetHostDispatch();
                return this->Equals(pGlobalHostDispatch, value, requestContext);
            }
            *value = FALSE;
            return FALSE;
        }
    case Js::TypeIds_WithScopeObject:
        AssertMsg(false, "WithScopeObjects should not be exposed");
        *value = FALSE;
        break;

    case Js::TypeIds_Object:
    case Js::TypeIds_ActivationObject:
    case Js::TypeIds_VariantDate:
        {
            // == on a variant always returns false.  Putting this in a 
            // switch in each .Equals to prevent a perf hit by adding an
            // if/branch to JavascriptOperators::Equal_Full
            *value = FALSE;
            break;
        }
    default:
        {          
            Var  aLeft;

            if (Js::DynamicType::Is(otherType))
            {
                aLeft = static_cast<Var>(this);
                (*value) = Js::JavascriptOperators::Equal_Full(other, aLeft, requestContext);
                break;
            }

            if (this->GetDefaultValue(Js::JavascriptHint::None, &aLeft, FALSE))
            {
                *value = Js::JavascriptOperators::Equal_Full(aLeft, other, requestContext);
            }
            else
            {
                *value = FALSE;
            }
            break;
        }
    }
    return TRUE;
}
// Have to override this, otherwise we get 'function'.
Var HostDispatch::GetTypeOfString(Js::ScriptContext * requestContext)
{
    // the dispatch object can be a wrapped Jscript object. 
    // Cally GetTypeOfString() on the remote object would return a string object created on the other context. 
    // Marshalling the remote string via a bstr wold be unnecessary expensive.
    // TypeOf() returns 'object' for all objects, except functions. Use the remote type ID to build the string here. 
    // Note that we need to check if it is the old engine representation of a function, in which case we need to return function
    Js::TypeId remoteTypeId;
    IUnknown* punkOldEngine = NULL;
    if (requestContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        requestContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return requestContext->GetLibrary()->GetObjectTypeDisplayString();
    }
    HRESULT hr = EnsureDispatch();
    if (FAILED(hr))
    {
        return requestContext->GetLibrary()->GetUnknownDisplayString();
    }
    if ((Js::JavascriptOperators::GetRemoteTypeId(this, &remoteTypeId) &&
        remoteTypeId == Js::TypeIds_Function)
        || SUCCEEDED(HostDispatch::QueryInterfaceWithLeaveScript(GetHostVariant()->GetIDispatchAddress(), IID_OldJscripEngine, (void**)&punkOldEngine, requestContext)))
    {
        if (punkOldEngine)
            punkOldEngine->Release();
        return requestContext->GetLibrary()->GetFunctionTypeDisplayString();
    }

    return requestContext->GetLibrary()->GetObjectTypeDisplayString();
}

BOOL HostDispatch::HasInstance(Js::Var instance, Js::ScriptContext* scriptContext, Js::IsInstInlineCache* inlineCache)
{
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    HRESULT hr = EnsureDispatch();
    //
    // Throw TypeError if this is not a constructor HostDispatch (supports .prototype.isPrototypeOf)
    //
    if (FAILED(hr) || Js::JavascriptOperators::GetTypeId(this) != Js::TypeIds_HostDispatch)
    {
        return __super::HasInstance(instance, scriptContext); // Throw TypeError
    }
    
    HostVariant* hostVariant = GetHostVariant();

        // Make sure the object on the other side is a JavascriptDispatch object
    IJavascriptDispatchRemoteProxy* dispatchProxy;
    hr = HostDispatch::QueryInterfaceWithLeaveScript(hostVariant->varDispatch.pdispVal, IID_IJavascriptDispatchRemoteProxy, (void**)&dispatchProxy, scriptContext);
    if (SUCCEEDED(hr))
    {
        VARIANT varInstance;
        VariantInit(&varInstance);
            
        EXCEPINFO ei;
        memset(&ei, 0, sizeof(ei));

        BOOL result = FALSE;
        hr = DispatchHelper::MarshalJsVarToVariantNoThrowWithLeaveScript(instance, &varInstance, scriptContext);
        if (SUCCEEDED(hr))
        {
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                DispatchExCaller* pdc = NULL;
                hr = scriptSite->GetDispatchExCaller(&pdc);
                if (SUCCEEDED(hr))
                {
                    hr = dispatchProxy->HasInstance(varInstance, &result, &ei, pdc);
                    scriptSite->ReleaseDispatchExCaller(pdc);
                }
            }
            END_LEAVE_SCRIPT(scriptContext);
        }

        dispatchProxy->Release();

        if (FAILED(hr))
        {
            HandleDispatchError(hr, &ei);
        }
        else
        {
            return result;
        }
    }

    // REVIEW: avoid the marshalling
    Js::Var prototype;
    if (!this->GetValue(scriptContext->GetPropertyName(Js::PropertyIds::prototype)->GetBuffer(), &prototype)
        || Js::JavascriptOperators::GetTypeId(prototype) != Js::TypeIds_HostDispatch)
    {
        return __super::HasInstance(instance, scriptContext); // Throw TypeError
    }
    HostDispatch* prototypeDispatch = static_cast<HostDispatch*>(prototype);
    Js::Var prototypeProxy;
    if (!prototypeDispatch->GetPropertyReference(scriptContext->GetPropertyName(Js::PropertyIds::isPrototypeOf)->GetBuffer(), &prototypeProxy)
        || Js::JavascriptOperators::GetTypeId(prototypeProxy) != Js::TypeIds_HostDispatch)
    {
        return __super::HasInstance(instance, scriptContext); // Throw TypeError
    }

    if (Js::JavascriptOperators::GetTypeId(instance) != Js::TypeIds_HostDispatch)
    {
        return FALSE;
    }
    HostDispatch* instanceDispatch = static_cast<HostDispatch*>(instance);
    return instanceDispatch->IsInstanceOf(prototypeProxy);
}

BOOL HostDispatch::IsWritable(PropertyId propertyId)
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    HRESULT hr = EnsureDispatch();
    if (FAILED(hr))
    {
        return FALSE;
    }

    Js::GlobalObject * globalObj = scriptContext->GetGlobalObject();
    if(IsGlobalDispatch() && scriptContext->CanOptimizeGlobalLookup())
    {
        if(globalObj->HasOwnProperty(propertyId))
        {
            return globalObj->IsWritable(propertyId);
        }
    }

    return true;
}

BOOL HostDispatch::IsEnumerable(PropertyId propertyId)
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    HRESULT hr = EnsureDispatch();
    if (FAILED(hr))
    {
        return FALSE;
    }

    Js::GlobalObject * globalObj = scriptContext->GetGlobalObject();
    if(IsGlobalDispatch() && scriptContext->CanOptimizeGlobalLookup())
    {
        if(globalObj->HasOwnProperty(propertyId))
        {
            return globalObj->IsEnumerable(propertyId);
        }
    }

    //For now follow IE8 if its a setter\getter then return false else true
    Var getter;
    Var setter;
    return !this->GetAccessors(GetScriptContext()->GetPropertyName(propertyId)->GetBuffer(), &getter, &setter, GetScriptContext());
}

BOOL HostDispatch::IsConfigurable(PropertyId propertyId)
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    HRESULT hr = EnsureDispatch();
    if (FAILED(hr))
    {
        return FALSE;
    }

    Js::GlobalObject * globalObj = scriptContext->GetGlobalObject();
    if(IsGlobalDispatch() && scriptContext->CanOptimizeGlobalLookup())
    {
        if(globalObj->HasOwnProperty(propertyId))
        {
            return globalObj->IsConfigurable(propertyId);
        }
    }

    return true;
}

BOOL HostDispatch::Seal()
{
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }

    return InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::Seal.GetOriginalEntryPoint(), args, NULL);
}

BOOL HostDispatch::Freeze()
{
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }

    return InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::Freeze.GetOriginalEntryPoint(), args, NULL);
}

BOOL HostDispatch::PreventExtensions()
{
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;

    return InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::PreventExtensions.GetOriginalEntryPoint(), args, NULL);
}

BOOL HostDispatch::IsSealed()
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    Var result;
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;

    if (!InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::IsSealed.GetOriginalEntryPoint(), args, &result))
    {
        return FALSE;
    }

    return Js::JavascriptBoolean::FromVar(result)->GetValue();
}

BOOL HostDispatch::IsFrozen()
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    Var result;
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;

    if (!InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::IsFrozen.GetOriginalEntryPoint(), args, &result))
    {
        return FALSE;
    }

    return Js::JavascriptBoolean::FromVar(result)->GetValue();
}

BOOL HostDispatch::IsExtensible()
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    Var result;
    Js::Var values[2];
    Js::CallInfo info(2);
    Js::Arguments args(info, values);
    values[0] = this->GetLibrary()->GetNull(); // we can just pass null as the built-in implementation doesn't use "this"
    values[1] = this;

    if (!InvokeBuiltInOperationRemotely(Js::JavascriptObject::EntryInfo::IsExtensible.GetOriginalEntryPoint(), args, &result))
    {
        return FALSE;
    }

    return Js::JavascriptBoolean::FromVar(result)->GetValue();
}

Var HostDispatch::InvokePut( Js::Arguments args)
{    
    return this->InvokeByDispId(args, DISPID_VALUE, true);
}

__inline BOOL HostDispatch::IsGlobalDispatch()
{
    Js::GlobalObject * globalObj = GetScriptContext()->GetGlobalObject();
    Js::RecyclableObject * hostObject = globalObj->GetHostObject();
    // We still can have HostDispatch in FastDOM world.
    if (hostObject == NULL)
    {
        return FALSE;
    }
    return GetDispatchNoRef() == static_cast<HostObject*>(hostObject)->GetHostDispatch()->GetDispatchNoRef();
}