//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    // the name is per type. other external type can potentially add per-instance name
    JavascriptString* ExternalObject::GetClassName(ScriptContext * requestContext)
    {
        return requestContext->GetPropertyString(GetNameId());
    }

    BOOL ExternalObject::HasProperty(PropertyId propertyId)
    {
        // we don't throw in hasProperty, but possibly throw in GetProperty
        // this is consistent with HostDispatch code path as well.
        return DynamicObject::HasProperty(propertyId);
    }

    BOOL ExternalObject::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {        
        if (!this->VerifyObjectAlive()) return FALSE;
        originalInstance = CrossSite::MarshalVar(GetScriptContext(), originalInstance);
        BOOL result = DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
        if (result)
        {
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
        }
        return result;
    }

    BOOL ExternalObject::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {        
        if (!this->VerifyObjectAlive()) return FALSE;
        BOOL result = DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
        if (result)
        {
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
        }
        return result;
    }

    BOOL ExternalObject::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {        
        if (!this->VerifyObjectAlive()) return FALSE;
        originalInstance = CrossSite::MarshalVar(GetScriptContext(), originalInstance);
        BOOL result = DynamicObject::GetPropertyReference(originalInstance, propertyId, value, info, requestContext);
        if (result)
        {
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
        }
        return result;
    }

    BOOL ExternalObject::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) 
    {        
        if (!this->VerifyObjectAlive()) return FALSE;
        value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more informaiton.
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_NonFixedValue);
        return DynamicObject::SetProperty(propertyId, value, flags, info);
    }

    BOOL ExternalObject::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) 
    {        
        if (!this->VerifyObjectAlive()) return FALSE;
        value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more informaiton.
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_NonFixedValue);
        return DynamicObject::SetProperty(propertyNameString, value, flags, info);
    }

    BOOL ExternalObject::SetInternalProperty(PropertyId internalPropertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) 
    {
        // Before we added this override in ExternalObject we would go directly to DynamicObject::SetInternalProperty, which does not
        // do the following two.  Let's leave them out to maintain consistency.
        //if (!this->VerifyObjectAlive()) return FALSE;
        //value = Js::CrossSite::MarshalVar(GetScriptContext(), value);

        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more informaiton.
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_NonFixedValue);
        return DynamicObject::SetInternalProperty(internalPropertyId, value, flags, info);
    }

    BOOL ExternalObject::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more informaiton.
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_NonFixedValue);
        return DynamicObject::InitProperty(propertyId, value, flags, info);
    }

    BOOL ExternalObject::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more informaiton.
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_NonFixedValue);
        return DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
    }

    BOOL ExternalObject::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {        
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::DeleteProperty(propertyId, flags);
    }

    BOOL ExternalObject::HasItem(uint32 index)
    {
        // we don't throw in hasItem, but possibly throw in GetItem
        // this is consistent with HostDispatch code path as well.
        return DynamicObject::HasItem(index);
    }

    BOOL ExternalObject::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) 
    {     
        if (!this->VerifyObjectAlive()) return FALSE;
        originalInstance = CrossSite::MarshalVar(GetScriptContext(), originalInstance);
        BOOL result = DynamicObject::GetItem(originalInstance, index, value, requestContext);
        if (result)
        {
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
        }
        return result;
    }

    BOOL ExternalObject::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        originalInstance = CrossSite::MarshalVar(GetScriptContext(), originalInstance);
        BOOL result = DynamicObject::GetItemReference(originalInstance, index, value, requestContext);
        if (result)
        {
            *value = Js::CrossSite::MarshalVar(requestContext, *value);
        }
        return result;
    }

    DescriptorFlags ExternalObject::GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext)
    {
        if (!this->VerifyObjectAlive()) return DescriptorFlags::None;
        DescriptorFlags result = DynamicObject::GetItemSetter(index, setterValue, requestContext);
        if (result & Accessor)
        {
            if (*setterValue)
            {
                *setterValue = Js::CrossSite::MarshalVar(requestContext, *setterValue);
            }
        }
        return result;
    }

    BOOL ExternalObject::SetItem(uint32 index, Var value, PropertyOperationFlags flags) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
        return DynamicObject::SetItem(index, value, flags);
    }

    BOOL ExternalObject::DeleteItem(uint32 index, PropertyOperationFlags flags) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::DeleteItem(index, flags);
    }

    BOOL ExternalObject::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        BOOL result = DynamicObject::GetEnumerator(enumNonEnumerable, enumerator, requestContext, preferSnapshotSemantics, enumSymbols);
        if (result)
        {
            JavascriptEnumerator* jsEnumerator = JavascriptEnumerator::FromVar(*enumerator);
            if (requestContext != GetScriptContext())
            {
                jsEnumerator->MarshalToScriptContext(requestContext);
            }
        }
        return result;
    }

    BOOL ExternalObject::IsWritable(PropertyId propertyId)
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::IsWritable(propertyId);
    }

    BOOL ExternalObject::IsConfigurable(PropertyId propertyId) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::IsConfigurable(propertyId);
    }

    BOOL ExternalObject::IsEnumerable(PropertyId propertyId) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::IsEnumerable(propertyId);
    }

    BOOL ExternalObject::SetEnumerable(PropertyId propertyId, BOOL value) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::SetEnumerable(propertyId, value);
    }

    BOOL ExternalObject::SetWritable(PropertyId propertyId, BOOL value) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::SetWritable(propertyId, value);
    }

    BOOL ExternalObject::SetConfigurable(PropertyId propertyId, BOOL value) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::SetConfigurable(propertyId, value);
    }

    BOOL ExternalObject::SetAttributes(PropertyId propertyId, PropertyAttributes attributes)
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::SetAttributes(propertyId, attributes);
    }

    BOOL ExternalObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        if (getter)
        {
            getter = CrossSite::MarshalVar(GetScriptContext(), getter);
        }
        if (setter)
        {
            setter = CrossSite::MarshalVar(GetScriptContext(), setter);
        }
        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more informaiton.
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_NonFixedValue);
        return DynamicObject::SetAccessors(propertyId, getter, setter, flags);
    }

    BOOL ExternalObject::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive()) return FALSE;

        BOOL result = DynamicObject::GetAccessors(propertyId, getter, setter, requestContext);
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

    DescriptorFlags ExternalObject::GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (!this->VerifyObjectAlive()) return DescriptorFlags::None;

        DescriptorFlags result = DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
        if (result & Accessor)
        {
            if (*setterValue)
            {
                *setterValue = Js::CrossSite::MarshalVar(requestContext, *setterValue);
            }
        }

        return result;
    }

    DescriptorFlags ExternalObject::GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (!this->VerifyObjectAlive()) return DescriptorFlags::None;

        DescriptorFlags result = DynamicObject::GetSetter(propertyNameString, setterValue, info, requestContext);
        if (result & Accessor)
        {
            if (*setterValue)
            {
                *setterValue = Js::CrossSite::MarshalVar(requestContext, *setterValue);
            }
        }
        return result;
    }

    HRESULT ExternalObject::QueryObjectInterface(REFIID riid, void **ppvObj) 
    {
        if (!this->IsObjectAlive())
        {
            return E_ACCESSDENIED;
        }
        return DynamicObject::QueryObjectInterface(riid, ppvObj);
    }

    BOOL ExternalObject::Equals(Var other, BOOL* value, ScriptContext * requestContext) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return DynamicObject::Equals(other, value, requestContext);
    }

    BOOL ExternalObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"{...}");
        return TRUE;
    }

    BOOL ExternalObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"[Object");
        JavascriptString *pString = GetClassName(requestContext);
        if (pString)
        {
            stringBuilder->AppendCppLiteral(L", ");
            stringBuilder->Append(pString->GetString(), pString->GetLength());
        }
        stringBuilder->Append(L']');

        return TRUE;
    }

    void ExternalObject::RemoveFromPrototype(ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive()) return;
        DynamicObject::RemoveFromPrototype(requestContext);
    }

    void ExternalObject::AddToPrototype(ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive()) return;
        DynamicObject::AddToPrototype(requestContext);
    }

    void ExternalObject::SetPrototype(RecyclableObject* newPrototype)
    {
        if (!this->VerifyObjectAlive()) return;
        newPrototype = (RecyclableObject*)Js::CrossSite::MarshalVar(GetScriptContext(), newPrototype);
        return DynamicObject::SetPrototype(newPrototype);
    }

    BOOL ExternalObject::StrictEquals(Var other, BOOL* value, ScriptContext * requestContext) 
    {
        if (!this->VerifyObjectAlive()) return FALSE;
        return this == other;
    }

    void ExternalType::Initialize(JavascriptMethod entryPoint)
    {
        // If caller specifies an entry point, wrap it in the thunk so we can do additional check, calldir leavescriptstart/leavescriptend on the methods.
        if (entryPoint != nullptr)
        {
            nativeMethod = entryPoint;
            SetEntryPoint(ExternalEntryThunk);
        }

        // We don't know anything for certain about the type of properties an external object might have
        this->GetTypeHandler()->ClearHasOnlyWritableDataProperties();
        if(GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag)
        {
            GetScriptContext()->GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
        }
    }

    ExternalType::ExternalType(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, JavascriptMethod entryPoint, 
        DynamicTypeHandler * typeHandler, bool isLocked, bool isShared, ITypeOperations * operations, PropertyId nameId) 
        : DynamicType(scriptContext, typeId, prototype, entryPoint, typeHandler, isLocked, isShared), nameId(nameId), operations(operations)
    {
        this->flags |= TypeFlagMask_External | TypeFlagMask_CanHaveInterceptors;
        Initialize(entryPoint);
    }

     ExternalType::ExternalType(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, JavascriptMethod entryPoint,  
         DynamicTypeHandler * typeHandler, bool isLocked, bool isShared,  PropertyId nameId) 
         : DynamicType(scriptContext, typeId, prototype, entryPoint, typeHandler, isLocked, isShared), nameId(nameId), operations(nullptr) 
     {
         this->flags |= TypeFlagMask_External | TypeFlagMask_CanHaveInterceptors;
         Initialize(entryPoint);
     }

    Var __cdecl ExternalType::ExternalEntryThunk(RecyclableObject* recyclableObject, CallInfo callInfo, ...)
    {
        RUNTIME_ARGUMENTS(args, callInfo);
        ExternalObject* externalObject = Js::ExternalObject::FromVar(recyclableObject);
        ExternalType* externalType = static_cast<ExternalType*>(externalObject->GetType());
        ScriptContext* scriptContext = externalType->GetScriptContext();
        Assert(!scriptContext->GetThreadContext()->IsDisableImplicitException());
        scriptContext->VerifyAlive();
        if (args.Info.Count == 0)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NullOrUndefined);
        }
        // We need to verify "this" pointer is active as well. The problem is that DOM prototype functions are
        // the same across multiple frames, and caller can do function.call(closedthis) 
        if (!Js::TaggedNumber::Is(args.Values[0]))
        {
            Js::RecyclableObject::FromVar(args.Values[0])->GetScriptContext()->VerifyAlive();
        }

        Var result = scriptContext->GetLibrary()->GetUndefined();
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
            // translate direct host for fastDOM.
            if (args.Values[0] == scriptContext->GetGlobalObject() || 
                args.Values[0] == scriptContext->GetLibrary()->GetUndefined())
            {
                args.Values[0] = scriptContext->GetGlobalObject()->GetDirectHostObject();
            }
            
            // Don't do stack probe since BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION does that for us already
            result = JavascriptFunction::CallFunction<false>(recyclableObject, externalType->nativeMethod, args);
            if ( nullptr == result )
            {
                result = scriptContext->GetLibrary()->GetUndefined();
            }
            else
            {
                result = CrossSite::MarshalVar(scriptContext, result);
            }
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext);
        return result;
    }

    Var __cdecl ExternalType::CrossSiteExternalEntryThunk(RecyclableObject* recyclableObject, CallInfo callInfo, ...)
    {
        RUNTIME_ARGUMENTS(args, callInfo);
        
        DynamicObject * dynamicObject = DynamicObject::FromVar(recyclableObject);
        ScriptContext * targetScriptContext = dynamicObject->GetScriptContext();
        targetScriptContext->VerifyAliveWithHostContext(!dynamicObject->IsExternal(), 
            ThreadContext::GetContextForCurrentThread()->GetPreviousHostScriptContext());
        return CrossSite::CommonThunk(recyclableObject, ExternalType::ExternalEntryThunk, args);
    }

}
