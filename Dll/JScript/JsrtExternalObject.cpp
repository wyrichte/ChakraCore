//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <StdAfx.h>
#include "JsrtExternalObject.h"
#include "JsrtCustomEnumerator.h"

JsrtExternalType::JsrtExternalType(Js::ScriptContext* scriptContext, JsExternalTypeDescription * typeDescription) 
    : Js::DynamicType(
        scriptContext, 
        scriptContext->CreateTypeId(), 
        typeDescription->prototype ?
            (Js::RecyclableObject *)typeDescription->prototype :
            scriptContext->GetLibrary()->GetObjectPrototype(),
        nullptr, 
        Js::SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), 
        true, 
        true)
    , typeDescription(*typeDescription)
{
    // We don't know anything for certain about the type of properties an external object might have
    this->GetTypeHandler()->ClearHasOnlyWritableDataProperties();
    if (GetTypeHandler()->GetFlags() & Js::DynamicTypeHandler::IsPrototypeFlag)
    {
        scriptContext->GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
    }
    this->flags |= TypeFlagMask_CanHaveInterceptors;
}

JsrtExternalObject::JsrtExternalObject(JsrtExternalType * type, void *data) :
    slot(data), 
    Js::DynamicObject(type)
{
}

bool JsrtExternalObject::Is(Js::Var value)
{
    if (Js::TaggedNumber::Is(value))
    {
        return false;
    }

    return (VirtualTableInfo<JsrtExternalObject>::HasVirtualTable(value));
}

JsrtExternalObject * JsrtExternalObject::FromVar(Js::Var value)
{
    Assert(Is(value));
    return static_cast<JsrtExternalObject *>(value);
}

void JsrtExternalObject::Finalize(bool isShutdown)
{
    JsExternalTypeDescription *typeDescription = this->GetExternalType()->GetExternalTypeDescription();
    if (NULL != typeDescription->finalizeCallback)
    {
        if (typeDescription->version == 0)
        {
            // This version is used by JsCreateExternalObject and by internal projects that
            // used the interceptor API when it was private in IE11 and the signature was
            // different.
            ((JsFinalizeCallback)typeDescription->finalizeCallback)(this->slot);
        }
        else
        {
            typeDescription->finalizeCallback((JsValueRef)this);
        }
    }
}

void JsrtExternalObject::Dispose(bool isShutdown)
{    
}

void * JsrtExternalObject::GetSlotData() const
{
    return this->slot;
}

void JsrtExternalObject::SetSlotData(void * data)
{
    this->slot = data;
}

BOOL JsrtExternalObject::HasProperty(PropertyId propertyId)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return (attributes & JsPropertyAttributeInvalid) != JsPropertyAttributeInvalid;
        }
    }

    return DynamicObject::HasProperty(propertyId);
}

BOOL JsrtExternalObject::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->getCallback)
    {
        bool handled = false;
        Js::PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

        Js::ScriptContext * scriptContext = this->GetScriptContext();
        ThreadContext * threadContext = scriptContext->GetThreadContext();

        // Reject implicit call
        if (threadContext->IsDisableImplicitCall())
        {
            *value = requestContext->GetLibrary()->GetNull();
            threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
            return TRUE;
        }

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->getCallback((JsValueRef)originalInstance, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), (JsValueRef *)value);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return *value != JS_INVALID_REFERENCE;
        }
        else
        {
            // if we need to go through ITypeOperation first to find property, it is possible for the DOM side 
            // to add property without going through jscript side of code. We need to disable the prototype 
            // lookup as well, because we cannot guarantee if we'll see the property in this object next round.
            // We might get property from current object instead of prototype object without any change in jscript code.
            Js::PropertyValueInfo::DisablePrototypeCache(info, this); // We can't cache the property
        }
    }

    return DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
}

BOOL JsrtExternalObject::GetProperty(Var originalInstance, Js::JavascriptString* propertyNameString, Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    Js::PropertyRecord const * propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return JsrtExternalObject::GetProperty(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
}

BOOL JsrtExternalObject::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    return GetProperty(originalInstance, propertyId, value, info, requestContext);
}

BOOL JsrtExternalObject::SetProperty(PropertyId propertyId, Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->setCallback)
    {
        bool handled = false;
        Js::PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

        Js::ScriptContext * scriptContext = this->GetScriptContext();
        ThreadContext * threadContext = scriptContext->GetThreadContext();

        // Reject implicit call
        if (threadContext->IsDisableImplicitCall())
        {
            threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
            return TRUE;
        }

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->setCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), JsPropertyAttributeInvalid, (JsValueRef)value);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::SetProperty(propertyId, value, flags, info);
}

BOOL JsrtExternalObject::SetProperty(Js::JavascriptString* propertyNameString, Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    Js::PropertyRecord const * propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return JsrtExternalObject::SetProperty(propertyRecord->GetPropertyId(), value, flags, info);
}

BOOL JsrtExternalObject::SetPropertyWithAttributes(PropertyId propertyId, Var value, Js::PropertyAttributes attributes, Js::PropertyValueInfo* info, Js::PropertyOperationFlags flags, Js::SideEffects possibleSideEffects)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->setCallback)
    {
        bool handled = false;
        Js::PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

        Js::ScriptContext * scriptContext = this->GetScriptContext();
        ThreadContext * threadContext = scriptContext->GetThreadContext();

        // Reject implicit call
        if (threadContext->IsDisableImplicitCall())
        {
            threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
            return TRUE;
        }

        Assert((attributes & ~(JsPropertyAttributeEnumerable | JsPropertyAttributeConfigurable | JsPropertyAttributeWritable | JsPropertyAttributeNoRedeclare | JsPropertyAttributeConst)) == 0);

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->setCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), (JsPropertyAttributes)attributes, (JsValueRef)value);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, info);
}

BOOL JsrtExternalObject::InitProperty(PropertyId propertyId, Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    return JsrtExternalObject::SetProperty(propertyId, value, Js::PropertyOperation_None, info);
}

BOOL JsrtExternalObject::DeleteProperty(PropertyId propertyId, Js::PropertyOperationFlags flags)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->deleteCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();
        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->deleteCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId));
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::DeleteProperty(propertyId, flags);
}

BOOL JsrtExternalObject::HasItem(uint32 index)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryIndexedCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryIndexedCallback((JsValueRef)this, index, &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return (attributes & JsPropertyAttributeInvalid) != JsPropertyAttributeInvalid;
        }
    }

    return DynamicObject::HasItem(index);
}

BOOL JsrtExternalObject::GetItem(Var originalInstance, uint32 index, Var* value, Js::ScriptContext * requestContext)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->getIndexedCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();
        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->getIndexedCallback((JsValueRef)originalInstance, index, (JsValueRef *)value);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::GetItem(originalInstance, index, value, requestContext);
}

BOOL JsrtExternalObject::GetItemReference(Var originalInstance, uint32 index, Var* value, Js::ScriptContext * requestContext)
{
    return JsrtExternalObject::GetItem(originalInstance, index, value, requestContext);
}

BOOL JsrtExternalObject::SetItem(uint32 index, Var value, Js::PropertyOperationFlags flags)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->setIndexedCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();
        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->setIndexedCallback((JsValueRef)this, index, (JsValueRef)value);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::SetItem(index, value, flags);
}

BOOL JsrtExternalObject::DeleteItem(uint32 index, Js::PropertyOperationFlags flags)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->deleteIndexedCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();
        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->deleteIndexedCallback((JsValueRef)this, index);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::DeleteItem(index, flags);
}

BOOL JsrtExternalObject::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, Js::ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->enumerateCallback)
    {
        bool handled = false;

        Js::ScriptContext * scriptContext = this->GetScriptContext();
        JsMoveNextCallback moveNextCallback = nullptr;
        JsGetCurrentCallback enumeratorCallback = nullptr;
        JsEndEnumerationCallback finishCallback = nullptr;
        void *data = nullptr;

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->enumerateCallback((JsValueRef)this, enumNonEnumerable != 0, &moveNextCallback, &enumeratorCallback, &finishCallback, &data);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            *enumerator = RecyclerNewFinalized(scriptContext->GetRecycler(), JsrtCustomEnumerator, scriptContext, enumNonEnumerable != 0, enumSymbols, moveNextCallback, enumeratorCallback, finishCallback, data, this);
            return TRUE;
        }
    }
    return DynamicObject::GetEnumerator(enumNonEnumerable, enumerator, requestContext, preferSnapshotSemantics, enumSymbols);
}

BOOL JsrtExternalObject::IsEnumerable(PropertyId propertyId)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return (attributes & JsPropertyAttributeEnumerable) == JsPropertyAttributeEnumerable;
        }
    }

    return DynamicObject::IsEnumerable(propertyId);
}

BOOL JsrtExternalObject::IsWritable(PropertyId propertyId)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return (attributes & JsPropertyAttributeWritable) == JsPropertyAttributeWritable;
        }
    }

    return DynamicObject::IsWritable(propertyId);
}

BOOL JsrtExternalObject::IsConfigurable(PropertyId propertyId)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return (attributes & JsPropertyAttributeConfigurable) == JsPropertyAttributeConfigurable;
        }
    }

    return DynamicObject::IsConfigurable(propertyId);
}

BOOL JsrtExternalObject::SetEnumerable(PropertyId propertyId, BOOL value)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            JsPropertyIdRef propertyIdRef = (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId);
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, propertyIdRef, &attributes);

            if (handled)
            {
                if (value)
                {
                    attributes = (JsPropertyAttributes)(attributes | JsPropertyAttributeEnumerable);
                }
                else
                {
                    attributes = (JsPropertyAttributes)(attributes & ~JsPropertyAttributeEnumerable);
                }

                bool setHandled = this->GetExternalType()->GetExternalTypeDescription()->setCallback((JsValueRef)this, propertyIdRef, attributes, JS_INVALID_REFERENCE);
                Assert(setHandled);
            }
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::SetEnumerable(propertyId, value);
}

BOOL JsrtExternalObject::SetWritable(PropertyId propertyId, BOOL value)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            JsPropertyIdRef propertyIdRef = (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId);
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, propertyIdRef, &attributes);

            if (handled)
            {
                if (value)
                {
                    attributes = (JsPropertyAttributes)(attributes | JsPropertyAttributeWritable);
                }
                else
                {
                    attributes = (JsPropertyAttributes)(attributes & ~JsPropertyAttributeWritable);
                }

                bool setHandled = this->GetExternalType()->GetExternalTypeDescription()->setCallback((JsValueRef)this, propertyIdRef, attributes, JS_INVALID_REFERENCE);
                Assert(setHandled);
            }
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::SetWritable(propertyId, value);
}

BOOL JsrtExternalObject::SetConfigurable(PropertyId propertyId, BOOL value)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            JsPropertyIdRef propertyIdRef = (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId);
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, propertyIdRef, &attributes);

            if (handled)
            {
                if (value)
                {
                    attributes = (JsPropertyAttributes)(attributes | JsPropertyAttributeConfigurable);
                }
                else
                {
                    attributes = (JsPropertyAttributes)(attributes & ~JsPropertyAttributeConfigurable);
                }

                bool setHandled = this->GetExternalType()->GetExternalTypeDescription()->setCallback((JsValueRef)this, propertyIdRef, attributes, JS_INVALID_REFERENCE);
                Assert(setHandled);
            }
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return TRUE;
        }
    }

    return DynamicObject::SetConfigurable(propertyId, value);
}

// TODO: WARNING: slow perf as it calls GetConfigurable/Enumerable/Writable individually.
BOOL JsrtExternalObject::SetAttributes(PropertyId propertyId, Js::PropertyAttributes attributes)
{
    return
        this->SetConfigurable(propertyId, attributes & PropertyConfigurable) &&
        this->SetEnumerable(propertyId, attributes & PropertyEnumerable) &&
        this->SetWritable(propertyId, attributes & PropertyWritable);
    // Note that although technically it's not valid to set writable to accessor properties,
    // internally we keep writable as true for accessors, and take it out before returning
    // property descriptor to the user. In other words, (1) it's OK to do and (2) we need it
    // as this is low-level call.
}

BOOL JsrtExternalObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return FALSE;
        }
    }

    return DynamicObject::SetAccessors(propertyId, getter, setter);
}

BOOL JsrtExternalObject::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, Js::ScriptContext * requestContext)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return FALSE;
        }
    }

    return DynamicObject::GetAccessors(propertyId, getter, setter, requestContext);
}

Js::DescriptorFlags JsrtExternalObject::GetSetter(PropertyId propertyId, Var* setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

        Js::ScriptContext * scriptContext = this->GetScriptContext();
        ThreadContext * threadContext = scriptContext->GetThreadContext();

        // Reject implicit call
        if (threadContext->IsDisableImplicitCall())
        {
            *setterValue = requestContext->GetLibrary()->GetNull();
            threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
            return Js::DescriptorFlags::Accessor;
        }

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryCallback((JsValueRef)this, (JsPropertyIdRef)scriptContext->GetPropertyName(propertyId), &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return Js::DescriptorFlags::None;
        }
    }

    return DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
}

Js::DescriptorFlags JsrtExternalObject::GetSetter(Js::JavascriptString* propertyNameString, Var* setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext)
{
    Js::PropertyRecord const * propertyRecord;
    this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
    return JsrtExternalObject::GetSetter(propertyRecord->GetPropertyId(), setterValue, info, requestContext);
}

Js::DescriptorFlags JsrtExternalObject::GetItemSetter(uint32 index, Var* setterValue, Js::ScriptContext* requestContext)
{
    JsPropertyAttributes attributes = JsPropertyAttributeInvalid;

    if (this->GetExternalType()->GetExternalTypeDescription()->queryCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->queryIndexedCallback((JsValueRef)this, index, &attributes);
        }
        END_INTERCEPTOR(scriptContext)

        if (handled)
        {
            return Js::DescriptorFlags::None;
        }
    }

    return DynamicObject::GetItemSetter(index, setterValue, requestContext);
}

BOOL JsrtExternalObject::Equals(Js::Var other, BOOL* returnResult, Js::ScriptContext * requestContext)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->equalsCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();
        bool result = false;

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->equalsCallback((JsValueRef)this, (JsValueRef)other, &result);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            *returnResult = result;
            return TRUE;
        }
    }

    return DynamicObject::Equals(other, returnResult, requestContext);
}

BOOL JsrtExternalObject::StrictEquals(Js::Var other, BOOL* returnResult, Js::ScriptContext * requestContext)
{
    if (this->GetExternalType()->GetExternalTypeDescription()->strictEqualsCallback)
    {
        bool handled = false;
        Js::ScriptContext * scriptContext = this->GetScriptContext();
        bool result = false;

        BEGIN_INTERCEPTOR(scriptContext)
        {
            handled = this->GetExternalType()->GetExternalTypeDescription()->strictEqualsCallback((JsValueRef)this, (JsValueRef)other, &result);
        }
        END_INTERCEPTOR(scriptContext);

        if (handled)
        {
            *returnResult = result;
            return TRUE;
        }
    }

    return this == other;
}

Js::JavascriptString* JsrtExternalObject::GetClassName(Js::ScriptContext * requestContext)
{
    Js::PropertyRecord *propertyRecord = (Js::PropertyRecord *)this->GetExternalType()->GetExternalTypeDescription()->className;

    if (propertyRecord != nullptr)
    {
        return requestContext->GetPropertyString(propertyRecord->GetPropertyId());
    }
    else
    {
        return requestContext->GetLibrary()->GetObjectDisplayString();
    }
}

BOOL JsrtExternalObject::GetDiagValueString(Js::StringBuilder<ArenaAllocator>* stringBuilder, Js::ScriptContext* requestContext)
{
    stringBuilder->AppendCppLiteral(L"{...}");
    return TRUE;
}

BOOL JsrtExternalObject::GetDiagTypeString(Js::StringBuilder<ArenaAllocator>* stringBuilder, Js::ScriptContext* requestContext)
{
    stringBuilder->AppendCppLiteral(L"[Object");
    Js::JavascriptString *pString = GetClassName(requestContext);
    if (pString)
    {
        stringBuilder->AppendCppLiteral(L", ");
        stringBuilder->Append(pString->GetString(), pString->GetLength());
    }
    stringBuilder->Append(L']');

    return TRUE;
}

Js::DynamicType* JsrtExternalObject::DuplicateType()
{
    return RecyclerNew(this->GetScriptContext()->GetRecycler(), JsrtExternalType,
        this->GetExternalType());
}
