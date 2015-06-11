//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

DEFINE_VALIDATE_HAS_VTABLE_CTOR(Js::RecyclableObject);

namespace Js
{
    void PropertyValueInfo::SetCacheInfo(PropertyValueInfo* info, InlineCache *const inlineCache)
    {
        Assert(info);
        Assert(inlineCache);

        info->functionBody = nullptr;
        info->inlineCache = inlineCache;
        info->polymorphicInlineCache = nullptr;
        info->inlineCacheIndex = Js::Constants::NoInlineCacheIndex;
        info->allowResizingPolymorphicInlineCache = false;
    }

    void PropertyValueInfo::SetCacheInfo(
        PropertyValueInfo* info,
        FunctionBody *const functionBody,
        InlineCache *const inlineCache,
        const InlineCacheIndex inlineCacheIndex,
        const bool allowResizingPolymorphicInlineCache)
    {
        Assert(info);
        Assert(functionBody);
        Assert(inlineCache);
        Assert(inlineCacheIndex < functionBody->GetInlineCacheCount());

        info->functionBody = functionBody;
        info->inlineCache = inlineCache;
        info->polymorphicInlineCache = nullptr;
        info->inlineCacheIndex = inlineCacheIndex;
        info->allowResizingPolymorphicInlineCache = allowResizingPolymorphicInlineCache;
    }

    void PropertyValueInfo::SetCacheInfo(
        PropertyValueInfo* info,
        FunctionBody *const functionBody,
        PolymorphicInlineCache *const polymorphicInlineCache,
        const InlineCacheIndex inlineCacheIndex,
        const bool allowResizingPolymorphicInlineCache)
    {
        Assert(info);
        Assert(functionBody);
        Assert(polymorphicInlineCache);
        Assert(inlineCacheIndex < functionBody->GetInlineCacheCount());

        info->functionBody = functionBody;
        info->inlineCache = nullptr;
        info->polymorphicInlineCache = polymorphicInlineCache;
        info->inlineCacheIndex = inlineCacheIndex;
        info->allowResizingPolymorphicInlineCache = allowResizingPolymorphicInlineCache;
    }

    void PropertyValueInfo::ClearCacheInfo(PropertyValueInfo* info)
    {
        if (info != NULL)
        {
            info->functionBody = nullptr;
            info->inlineCache = nullptr;
            info->polymorphicInlineCache = nullptr;
            info->inlineCacheIndex = Constants::NoInlineCacheIndex;
            info->allowResizingPolymorphicInlineCache = true;
        }
    }

#if DBG || defined(PROFILE_TYPES)
    // Used only by the GlobalObject, because it's typeHandler can't be fully initialized
    // with the globalobject which is currently being created.
    RecyclableObject::RecyclableObject(DynamicType * type, ScriptContext * scriptContext) : type(type)
    {
#if DBG_EXTRAFIELD
        dtorCalled = false;
#ifdef HEAP_ENUMERATION_VALIDATION
        m_heapEnumValidationCookie = 0;
#endif
#endif
        Assert(type->GetTypeId() == TypeIds_GlobalObject);
        RecordAllocation(scriptContext);
    }

    void RecyclableObject:: RecordAllocation(ScriptContext * scriptContext)
    {
#ifdef PROFILE_TYPES
        TypeId typeId = this->GetType()->GetTypeId();
        if (typeId < sizeof(scriptContext->instanceCount)/sizeof(int))
        {
            scriptContext->instanceCount[typeId]++;
        }
#endif
    }
#endif

    uint32
    RecyclableObject::GetOffsetOfType()
    {
        return offsetof(RecyclableObject, type);
    }

    RecyclableObject * RecyclableObject::CloneToScriptContext(ScriptContext* requestContext)
    {
        switch (JavascriptOperators::GetTypeId(this))
        {
        case TypeIds_Undefined:
            return requestContext->GetLibrary()->GetUndefined();
        case TypeIds_Null:
            return requestContext->GetLibrary()->GetNull();
        case TypeIds_Number:
            return RecyclableObject::FromVar(JavascriptNumber::CloneToScriptContext(this, requestContext));
        default:
            AssertMsg(FALSE, "shouldn't clone for other types");
            Js::JavascriptError::ThrowError(requestContext, VBSERR_InternalError);
        }
    }

#if defined(PROFILE_RECYCLER_ALLOC) && defined(RECYCLER_DUMP_OBJECT_GRAPH)
    bool RecyclableObject::DumpObjectFunction(type_info const * typeinfo, bool isArray, void * objectAddress)
    {
        if (isArray)
        {
            // Don't deal with array
            return false;
        }

        Output::Print(L"%S{%x} %p", typeinfo->name(), ((RecyclableObject *)objectAddress)->GetTypeId(), objectAddress);
        return true;
    }
#endif

    BOOL RecyclableObject::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        // TODO: It appears as though this is never called. Some types (such as JavascriptNumber) don't override this, but they
        // also don't expect properties to be set on them. Need to review this and see if we can make this pure virtual or
        // Assert(false) here. In any case, this should be SetProperty, not InitProperty.
        Assert(false);

        bool isForce = (flags & PropertyOperation_Force) != 0;
        bool throwIfNotExtensible = (flags & PropertyOperation_ThrowIfNotExtensible) != 0;
        if (!isForce)
        {
            // throwIfNotExtensible is only relevant to DynamicObjects
            Assert(!throwIfNotExtensible);
        }

        return 
            this->InitProperty(propertyId, value, flags) &&
            this->SetAttributes(propertyId, attributes);
    }

    void RecyclableObject::ThrowIfCannotDefineProperty(PropertyId propId, PropertyDescriptor descriptor)
    {        
        // Do nothing
    }
    
    BOOL RecyclableObject::GetDefaultPropertyDescriptor(PropertyDescriptor& descriptor)
    {
        // By default, when GetOwnPropertyDescriptor is called for a nonexistent property,
        // return undefined.
        return false;
    }

    HRESULT RecyclableObject::QueryObjectInterface(REFIID riid, void **ppvObj)
    { 
        Assert(!this->GetScriptContext()->GetThreadContext()->IsScriptActive()); 
        return E_NOINTERFACE; 
    }
    RecyclableObject* RecyclableObject::GetThisObjectOrUnWrap()
    {
        if (WithScopeObject::Is(this))
        {
            return WithScopeObject::FromVar(this)->GetWrappedObject();
        }
        return this;
    }


} // namespace Js
