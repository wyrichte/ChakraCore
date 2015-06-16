//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    __inline void InlineCache::CacheLocal(
        Type *const type,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        Type *const typeWithoutProperty,
        int requiredAuxSlotCapacity,
        ScriptContext *const requestContext)
    {
        Assert(type);
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyIndex != Constants::NoSlot);
        Assert(requestContext);
        Assert(type->GetScriptContext() == requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        Assert(requiredAuxSlotCapacity >= 0 && requiredAuxSlotCapacity < 0x01 << RequiredAuxSlotCapacityBitCount);
        // Store field and load field caches are never shared so we should never have a prototype cache morphing into an add property cache.
        // We may, however, have a flags cache (setter) change to add property cache.
        Assert(typeWithoutProperty == null || !IsProto());

        requestContext->RegisterAsScriptContextWithInlineCaches();

        // Add cache into a store field cache list if required, but not there yet.
        if (typeWithoutProperty != null && invalidationListSlotPtr == null)
        {
            // Note, this can throw due to OOM, so we need to do it before the inline cache is set below.
            requestContext->RegisterStoreFieldInlineCache(this, propertyId);
        }

        if (isInlineSlot)
        {
            u.local.type = type;
            u.local.typeWithoutProperty = typeWithoutProperty;
        }
        else
        {
            u.local.type = TypeWithAuxSlotTag(type);
            u.local.typeWithoutProperty = typeWithoutProperty ? TypeWithAuxSlotTag(typeWithoutProperty) : null;
        }

        u.local.isLocal = true;
        u.local.slotIndex = propertyIndex;
        u.local.requiredAuxSlotCapacity = requiredAuxSlotCapacity;

        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            Output::Print(L"IC::CacheLocal, %s: ", requestContext->GetPropertyName(propertyId)->GetBuffer());
            Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    __inline void InlineCache::CacheProto(
        DynamicObject *const prototypeObjectWithProperty,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        const bool isMissing,
        Type *const type,
        ScriptContext *const requestContext)
    {
        Assert(prototypeObjectWithProperty);
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyIndex != Constants::NoSlot);
        Assert(type);
        Assert(requestContext);
        Assert(prototypeObjectWithProperty->GetScriptContext() == requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        // We don't want to optimize missing property accesses when running in the language service, because missing values are treated and tracked differently there.
        Assert(!BinaryFeatureControl::LanguageService() || (!isMissing && prototypeObjectWithProperty != requestContext->GetLibrary()->GetMissingPropertyHolder()));

        // This is an interesting quirk.  In the browser Chakra's global object cannot be used directly as a prototype, because
        // the global object (referenced either as window or as this) always points to the host object.  Thus, when we retrieve
        // a property from Chakra's global object the prototypeObjectWithProperty != info->GetInstance() and we will never cache
        // such property loads (see CacheOperators::CachePropertyRead).  However, in jc.exe or jshost.exe the only global object
        // is Chakra's global object, and so prototypeObjectWithProperty == info->GetInstance() and we can cache.  Hence, the 
        // assert below is only correct when running in the browser.
        // Assert(prototypeObjectWithProperty != prototypeObjectWithProperty->type->GetLibrary()->GetGlobalObject());

        // Store field and load field caches are never shared so we should never have an add property cache morphing into a prototype cache.
        Assert(!IsLocal() || u.local.typeWithoutProperty == null);

        requestContext->RegisterAsScriptContextWithInlineCaches();

        // Add cache into a proto cache list if not there yet.
        if (invalidationListSlotPtr == null)
        {
            // Note, this can throw due to OOM, so we need to do it before the inline cache is set below.
            requestContext->RegisterProtoInlineCache(this, propertyId);
        }

        u.proto.prototypeObject = prototypeObjectWithProperty;
        u.proto.isProto = true;
        u.proto.isMissing = isMissing;
        u.proto.slotIndex = propertyIndex;
        if (isInlineSlot)
        {
            u.proto.type = type;
        }
        else
        {
            u.proto.type = TypeWithAuxSlotTag(type);
        }

        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        Assert(u.proto.isMissing == (uint16)(u.proto.prototypeObject == requestContext->GetLibrary()->GetMissingPropertyHolder()));

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            Output::Print(L"IC::CacheProto, %s: ", requestContext->GetPropertyName(propertyId)->GetBuffer());
            Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    // TODO (InlineCacheCleanup): When simplifying inline caches due to not sharing between loads and stores, create two
    // separate methods CacheSetter and CacheGetter.
    __inline void InlineCache::CacheAccessor(
        const bool isGetter,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        Type *const type,
        DynamicObject *const object,
        const bool isOnProto,
        ScriptContext *const requestContext)
    {
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyIndex != Constants::NoSlot);
        Assert(type);
        Assert(object);
        Assert(requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        // It is possible that prototype is from a different scriptContext than the original instance. We don't want to cache
        // in this case. 
        Assert(type->GetScriptContext() == requestContext);

        requestContext->RegisterAsScriptContextWithInlineCaches();

        if (isOnProto && invalidationListSlotPtr == null)
        {
            // Note, this can throw due to OOM, so we need to do it before the inline cache is set below.
            if (!isGetter)
            {
                // If the setter is on a prototype, this cache must be invalidated whenever proto
                // caches are invalidated, so we must register it here.  Note that store field inline
                // caches are invalidated any time proto caches are invalidated.
                requestContext->RegisterStoreFieldInlineCache(this, propertyId);
            }
            else
            {
                requestContext->RegisterProtoInlineCache(this, propertyId);
            }
        }

        u.accessor.isAccessor = true;
        // TODO (PersistentInlineCaches): Consider removing the flag altogether and just have a bit indicating
        // whether the cache itself is a store field cache (isStore?).
        u.accessor.flags = isGetter ? InlineCacheGetterFlag : InlineCacheSetterFlag;
        u.accessor.isOnProto = isOnProto;
        u.accessor.type = isInlineSlot ? type : TypeWithAuxSlotTag(type);
        u.accessor.slotIndex = propertyIndex;
        u.accessor.object = object;

        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            Output::Print(L"IC::CacheAccessor, %s: ", requestContext->GetPropertyName(propertyId)->GetBuffer());
            Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    template<
        bool CheckLocal,
        bool CheckProto,
        bool CheckAccessor,
        bool CheckMissing,
        bool ReturnOperationInfo>
    __inline bool InlineCache::TryGetProperty(
        Var const instance,
        RecyclableObject *const propertyObject,
        const PropertyId propertyId,
        Var *const propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo)
    {
        CompileAssert(CheckLocal || CheckProto || CheckAccessor);
        Assert(!ReturnOperationInfo || operationInfo);
        CompileAssert(!ReturnOperationInfo || (CheckLocal && CheckProto && CheckAccessor));
        Assert(instance);
        Assert(propertyObject);
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyValue);
        Assert(requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

        Type *const type = propertyObject->GetType();

        if(CheckLocal && type == u.local.type)
        {
            Assert(propertyObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            *propertyValue = DynamicObject::FromVar(propertyObject)->GetInlineSlot(u.local.slotIndex);
            Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(*propertyValue))) ||
                *propertyValue == JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext) || 
                *propertyValue == JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext));
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Local;
                operationInfo->slotType = SlotType_Inline;
            }
            return true;
        }

        if(CheckLocal && TypeWithAuxSlotTag(type) == u.local.type)
        {
            Assert(propertyObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            *propertyValue = DynamicObject::FromVar(propertyObject)->GetAuxSlot(u.local.slotIndex);
            Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(*propertyValue))) ||
                *propertyValue == JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext) || 
                *propertyValue == JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext));
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Local;
                operationInfo->slotType = SlotType_Aux;
            }
            return true;
        }

        if (CheckProto && type == u.proto.type && !this->u.proto.isMissing)
        {
            Assert(u.proto.prototypeObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            *propertyValue = u.proto.prototypeObject->GetInlineSlot(u.proto.slotIndex);
            Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(*propertyValue))) ||
                *propertyValue == JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext) ||
                *propertyValue == JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext));
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Proto;
                operationInfo->slotType = SlotType_Inline;
            }
            return true;
        }

        if (CheckProto && TypeWithAuxSlotTag(type) == u.proto.type && !this->u.proto.isMissing)
        {
            Assert(u.proto.prototypeObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            *propertyValue = u.proto.prototypeObject->GetAuxSlot(u.proto.slotIndex);
            // TODO: This assert often results in Assert(RootObjectBase::Is(object)) inside GetRootProperty, which is misleading 
            // when the problem is that GetProperty returned a different value than propertyValue. Consider reworking it here and elsewhere.
            Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(*propertyValue))) ||
                *propertyValue == JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext) || 
                *propertyValue == JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext));
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Proto;
                operationInfo->slotType = SlotType_Aux;
            }
            return true;
        }

        if(CheckAccessor && type == u.accessor.type)
        {
            Assert(propertyObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(u.accessor.flags & InlineCacheGetterFlag);

            RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetInlineSlot(u.accessor.slotIndex));

            *propertyValue = JavascriptOperators::CallGetter(function, instance, requestContext);

            // Can't assert because the getter could have a side effect
#ifdef CHKGETTER
            Assert(JavascriptOperators::Equal(*propertyValue, JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext), requestContext));
#endif
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Getter;
                operationInfo->slotType = SlotType_Inline;
            }
            return true;
        }

        if(CheckAccessor && TypeWithAuxSlotTag(type) == u.accessor.type)
        {
            Assert(propertyObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(u.accessor.flags & InlineCacheGetterFlag);

            RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetAuxSlot(u.accessor.slotIndex));

            *propertyValue = JavascriptOperators::CallGetter(function, instance, requestContext);

            // Can't assert because the getter could have a side effect
#ifdef CHKGETTER
            Assert(JavascriptOperators::Equal(*propertyValue, JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext), requestContext));
#endif
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Getter;
                operationInfo->slotType = SlotType_Aux;
            }
            return true;
        }

        if (CheckMissing && type == u.proto.type && this->u.proto.isMissing)
        {
            Assert(u.proto.prototypeObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            *propertyValue = u.proto.prototypeObject->GetInlineSlot(u.proto.slotIndex);
            // TODO: This assert often results in Assert(RootObjectBase::Is(object)) inside GetRootProperty, which is misleading 
            // when the problem is that GetProperty returned a different value than propertyValue. Consider reworking it here and elsewhere.
            Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(*propertyValue))) ||
                *propertyValue == JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext) ||
                *propertyValue == JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext));

#ifdef MISSING_PROPERTY_STATS
            if (PHASE_STATS1(MissingPropertyCachePhase))
            {
                requestContext->RecordMissingPropertyHit();
            }
#endif

            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Proto;
                operationInfo->slotType = SlotType_Inline;
            }
            return true;
        }

        if (CheckMissing && TypeWithAuxSlotTag(type) == u.proto.type && this->u.proto.isMissing)
        {
            Assert(u.proto.prototypeObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            *propertyValue = u.proto.prototypeObject->GetAuxSlot(u.proto.slotIndex);
            // TODO: This assert often results in Assert(RootObjectBase::Is(object)) inside GetRootProperty, which is misleading 
            // when the problem is that GetProperty returned a different value than propertyValue. Consider reworking it here and elsewhere.
            Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(*propertyValue))) ||
                *propertyValue == JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext) ||
                *propertyValue == JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext));

#ifdef MISSING_PROPERTY_STATS
            if (PHASE_STATS1(MissingPropertyCachePhase))
            {
                requestContext->RecordMissingPropertyHit();
            }
#endif

            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Proto;
                operationInfo->slotType = SlotType_Aux;
            }
            return true;
        }

        return false;
    }

    __inline bool InlineCache::PretendTryGetProperty(Type *const type, PropertyCacheOperationInfo * operationInfo)
    {
        if(type == u.local.type)
        {
            operationInfo->cacheType = CacheType_Local;
            operationInfo->slotType = SlotType_Inline;
            return true;
        }

        if(TypeWithAuxSlotTag(type) == u.local.type)
        {
            operationInfo->cacheType = CacheType_Local;
            operationInfo->slotType = SlotType_Aux;
            return true;
        }

        if (type == u.proto.type)
        {
            operationInfo->cacheType = CacheType_Proto;
            operationInfo->slotType = SlotType_Inline;
            return true;
        }

        if (TypeWithAuxSlotTag(type) == u.proto.type)
        {
            operationInfo->cacheType = CacheType_Proto;
            operationInfo->slotType = SlotType_Aux;
            return true;
        }

        if (type == u.accessor.type)
        {
            Assert(u.accessor.flags & InlineCacheGetterFlag);

            operationInfo->cacheType = CacheType_Getter;
            operationInfo->slotType = SlotType_Inline;
            return true;
        }

        if (TypeWithAuxSlotTag(type) == u.accessor.type)
        {
            Assert(u.accessor.flags & InlineCacheGetterFlag);

            operationInfo->cacheType = CacheType_Getter;
            operationInfo->slotType = SlotType_Aux;
            return true;
        }

        return false;
    }

    template<
        bool CheckLocal,
        bool CheckLocalTypeWithoutProperty,
        bool CheckAccessor,
        bool ReturnOperationInfo>
    __inline bool InlineCache::TrySetProperty(
        RecyclableObject *const object,
        const PropertyId propertyId,
        Var propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        const PropertyOperationFlags propertyOperationFlags)
    {
        CompileAssert(CheckLocal || CheckLocalTypeWithoutProperty || CheckAccessor);
        Assert(!ReturnOperationInfo || operationInfo);
        CompileAssert(!ReturnOperationInfo || (CheckLocal && CheckLocalTypeWithoutProperty && CheckAccessor));
        Assert(object);
        Assert(propertyId != Constants::NoProperty);
        Assert(requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

#if DBG
        const bool isRoot = (propertyOperationFlags & PropertyOperation_Root) != 0;

        bool canSetField;           // To verify if we can set a field on the object
        Var setterValue = nullptr;
        {   // We need to disable implicit call to ensure the check doesn't cause unwanted side effects in debug code
            // Save old disableImplicitFlags and implicitCallFlags and disable implicit call and exception
            ThreadContext * threadContext = requestContext->GetThreadContext();
            DisableImplicitFlags disableImplicitFlags = *threadContext->GetAddressOfDisableImplicitFlags();
            Js::ImplicitCallFlags implicitCallFlags = threadContext->GetImplicitCallFlags();
            threadContext->ClearImplicitCallFlags();
            *threadContext->GetAddressOfDisableImplicitFlags() = DisableImplicitCallAndExceptionFlag;

            DescriptorFlags flags = DescriptorFlags::None;
            canSetField = !JavascriptOperators::CheckPrototypesForAccessorOrNonWritablePropertySlow(object, propertyId, &setterValue, &flags, isRoot, requestContext);
            if (threadContext->GetImplicitCallFlags() != Js::ImplicitCall_None)
            {
                canSetField = true; // If there was implict call, inconclusive. Disable debug check.
                setterValue = nullptr;
            }
            else if ((flags & Accessor) == Accessor)
            {
                Assert(setterValue != nullptr);
            }

            // Restore old disableImplicitFlags and implicitCallFlags
            *threadContext->GetAddressOfDisableImplicitFlags() = disableImplicitFlags;
            threadContext->SetImplicitCallFlags(implicitCallFlags);
        }
#endif

        Type *const type = object->GetType();

        if(CheckLocal && type == u.local.type)
        {
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(u.local.slotIndex, true));
            Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(u.local.slotIndex, true));
            Assert(object->CanStorePropertyValueDirectly(propertyId, isRoot));
            DynamicObject::FromVar(object)->SetInlineSlot(SetSlotArgumentsRoot(propertyId, isRoot, u.local.slotIndex, propertyValue));
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Local;
                operationInfo->slotType = SlotType_Inline;
            }
            Assert(canSetField);
            return true;
        }

        if(CheckLocal && TypeWithAuxSlotTag(type) == u.local.type)
        {
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(u.local.slotIndex, false));
            Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(u.local.slotIndex, false));
            Assert(object->CanStorePropertyValueDirectly(propertyId, isRoot));
            DynamicObject::FromVar(object)->SetAuxSlot(SetSlotArgumentsRoot(propertyId, isRoot, u.local.slotIndex, propertyValue));
            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Local;
                operationInfo->slotType = SlotType_Aux;
            }
            Assert(canSetField);
            return true;
        }

        if(CheckLocalTypeWithoutProperty && type == u.local.typeWithoutProperty)
        {
            // CAREFUL! CheckIfPrototypeChainHasOnlyWritableDataProperties may do allocation that triggers GC and
            // clears this cache, so save any info that is needed from the cache before calling those functions.
            Type *const typeWithProperty = u.local.type;
            const PropertyIndex propertyIndex = u.local.slotIndex;

#if DBG
            uint16 newAuxSlotCapacity = u.local.requiredAuxSlotCapacity;
#endif
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(typeWithProperty);
            Assert(DynamicType::Is(typeWithProperty->GetTypeId()));
            Assert(((DynamicType*)typeWithProperty)->GetIsShared());
            Assert(((DynamicType*)typeWithProperty)->GetTypeHandler()->IsPathTypeHandler());
            AssertMsg(!((DynamicType*)u.local.typeWithoutProperty)->GetTypeHandler()->GetIsPrototype(),  "Why did we cache a property add for a prototype?");
            Assert(((DynamicType*)typeWithProperty)->GetTypeHandler()->CanStorePropertyValueDirectly((const DynamicObject*)object, propertyId, isRoot));

            DynamicObject *const dynamicObject = DynamicObject::FromVar(object);

            // If we're adding a property to an inlined slot, we should never need to adjust auxiliary slot array size.
            Assert(newAuxSlotCapacity == 0);

            dynamicObject->type = typeWithProperty;

            Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, true));
            Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, true));

            dynamicObject->SetInlineSlot(SetSlotArgumentsRoot(propertyId, isRoot, propertyIndex, propertyValue));

            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_LocalWithoutProperty;
                operationInfo->slotType = SlotType_Inline;
            }
            Assert(canSetField);
            return true;
        }

        if(CheckLocalTypeWithoutProperty && TypeWithAuxSlotTag(type) == u.local.typeWithoutProperty)
        {
            // CAREFUL! CheckIfPrototypeChainHasOnlyWritableDataProperties or AdjustSlots may do allocation that triggers GC and
            // clears this cache, so save any info that is needed from the cache before calling those functions.
            Type *const typeWithProperty = TypeWithoutAuxSlotTag(u.local.type);
            const PropertyIndex propertyIndex = u.local.slotIndex;
            uint16 newAuxSlotCapacity = u.local.requiredAuxSlotCapacity;

            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(typeWithProperty);
            Assert(DynamicType::Is(typeWithProperty->GetTypeId()));
            Assert(((DynamicType*)typeWithProperty)->GetIsShared());
            Assert(((DynamicType*)typeWithProperty)->GetTypeHandler()->IsPathTypeHandler());
            AssertMsg(!((DynamicType*)TypeWithoutAuxSlotTag(u.local.typeWithoutProperty))->GetTypeHandler()->GetIsPrototype(),  "Why did we cache a property add for a prototype?");
            Assert(((DynamicType*)typeWithProperty)->GetTypeHandler()->CanStorePropertyValueDirectly((const DynamicObject*)object, propertyId, isRoot));

            DynamicObject *const dynamicObject = DynamicObject::FromVar(object);

            if (newAuxSlotCapacity > 0)
            {
                DynamicTypeHandler::AdjustSlots(
                    dynamicObject,
                    static_cast<DynamicType *>(typeWithProperty)->GetTypeHandler()->GetInlineSlotCapacity(),
                    newAuxSlotCapacity);
            }

            dynamicObject->type = typeWithProperty;

            Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, false));
            Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, false));

            dynamicObject->SetAuxSlot(SetSlotArgumentsRoot(propertyId, isRoot, propertyIndex, propertyValue));

            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_LocalWithoutProperty;
                operationInfo->slotType = SlotType_Aux;
            }
            Assert(canSetField);
            return true;
        }

        if(CheckAccessor && type == u.accessor.type)
        {
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(u.accessor.flags & InlineCacheSetterFlag);

            RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetInlineSlot(u.accessor.slotIndex));

            Assert(setterValue == nullptr || setterValue == function);
            Js::JavascriptOperators::CallSetter(function, object, propertyValue, requestContext);

            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Setter;
                operationInfo->slotType = SlotType_Inline;
            }
            return true;
        }

        if(CheckAccessor && TypeWithAuxSlotTag(type) == u.accessor.type)
        {
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(u.accessor.flags & InlineCacheSetterFlag);

            RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetAuxSlot(u.accessor.slotIndex));

            Assert(setterValue == nullptr || setterValue == function);
            Js::JavascriptOperators::CallSetter(function, object, propertyValue, requestContext);

            if (ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Setter;
                operationInfo->slotType = SlotType_Aux;
            }
            return true;
        }

        return false;
    }

    __inline bool InlineCache::PretendTrySetProperty(Type *const type, Type *const oldType, PropertyCacheOperationInfo * operationInfo)
    {
        if(oldType == u.local.typeWithoutProperty)
        {
            operationInfo->cacheType = CacheType_LocalWithoutProperty;
            operationInfo->slotType = SlotType_Inline;
            return true;
        }

        if(TypeWithAuxSlotTag(oldType) == u.local.typeWithoutProperty)
        {
            operationInfo->cacheType = CacheType_LocalWithoutProperty;
            operationInfo->slotType = SlotType_Aux;
            return true;
        }

        if(type == u.local.type)
        {
            operationInfo->cacheType = CacheType_Local;
            operationInfo->slotType = SlotType_Inline;
            return true;
        }

        if(TypeWithAuxSlotTag(type) == u.local.type)
        {
            operationInfo->cacheType = CacheType_Local;
            operationInfo->slotType = SlotType_Aux;
            return true;
        }

        if(type == u.accessor.type)
        {
            if(u.accessor.flags & InlineCacheSetterFlag)
            {
                operationInfo->cacheType = CacheType_Setter;
                operationInfo->slotType = SlotType_Inline;
                return true;
            }
        }

        if(TypeWithAuxSlotTag(type) == u.accessor.type)
        {
            if(u.accessor.flags & InlineCacheSetterFlag)
            {
                operationInfo->cacheType = CacheType_Setter;
                operationInfo->slotType = SlotType_Aux;
                return true;
            }
        }

        return false;
    }

    __inline bool InlineCache::GetGetterSetter(Type *const type, RecyclableObject **callee)
    {
        Type *const taggedType = TypeWithAuxSlotTag(type);
        *callee = null;

        if(u.accessor.flags & (InlineCacheGetterFlag | InlineCacheSetterFlag))
        {
            if (type == u.accessor.type)
            {
                *callee = RecyclableObject::FromVar(u.accessor.object->GetInlineSlot(u.accessor.slotIndex));
                return true;
            }
            else if(taggedType == u.accessor.type)
            {
                *callee = RecyclableObject::FromVar(u.accessor.object->GetAuxSlot(u.accessor.slotIndex));
                return true;
            }
        }
        return false;
    }

    __inline bool InlineCache::GetCallApplyTarget(RecyclableObject* obj, RecyclableObject **callee)
    {
        Type *const type = obj->GetType();
        Type *const taggedType = TypeWithAuxSlotTag(type);
        *callee = null;

        if (IsLocal())
        {
            if (type == u.local.type)
            {
                const Var objectAtInlineSlot = DynamicObject::FromVar(obj)->GetInlineSlot(u.local.slotIndex);
                if (!Js::TaggedNumber::Is(objectAtInlineSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtInlineSlot);
                    return true;
                }
            }
            else if (taggedType == u.local.type)
            {
                const Var objectAtAuxSlot = DynamicObject::FromVar(obj)->GetAuxSlot(u.local.slotIndex);
                if (!Js::TaggedNumber::Is(objectAtAuxSlot))
                {
                    *callee = RecyclableObject::FromVar(DynamicObject::FromVar(obj)->GetAuxSlot(u.local.slotIndex));
                    return true;
                }
            }
            return false;
        }
        else if (IsProto())
        {
            if (type == u.proto.type)
            {
                const Var objectAtInlineSlot = u.proto.prototypeObject->GetInlineSlot(u.proto.slotIndex);
                if (!Js::TaggedNumber::Is(objectAtInlineSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtInlineSlot);
                    return true;
                }
            }
            else if (taggedType == u.proto.type)
            {
                const Var objectAtAuxSlot = u.proto.prototypeObject->GetAuxSlot(u.proto.slotIndex);
                if (!Js::TaggedNumber::Is(objectAtAuxSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtAuxSlot);
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    inline void InlineCache::Clear()
    {
        // IsEmpty() is a quick check to see that the cache is not populated, it only checks u.local.type, which does not
        // guarantee that the proto or flags cache would not hit. So Clear() must still clear everything.

        u.local.type = null;
        u.local.isLocal = true;
        u.local.typeWithoutProperty = null;
    }

    InlineCache *InlineCache::Clone(Js::PropertyId propertyId, ScriptContext* scriptContext)
    {        
        Assert(scriptContext);

        InlineCacheAllocator* allocator = scriptContext->GetInlineCacheAllocator();
        // Important to zero the allocated cache to be sure CopyTo doesn't see garbage 
        // when it uses the next pointer.
        InlineCache* clone = AllocatorNewZ(InlineCacheAllocator, allocator, InlineCache);
        CopyTo(propertyId, scriptContext, clone);
        return clone;
     }

    bool InlineCache::TryGetFixedMethodFromCache(Js::FunctionBody* functionBody, uint cacheId, Js::JavascriptFunction** pFixedMethod)
    {
        Assert(pFixedMethod);

        if (IsEmpty())
        {
            return false;
        }
        Js::Type * propertyOwnerType = null;
        bool isLocal = IsLocal();
        bool isProto = IsProto();
        if (isLocal)
        {
            propertyOwnerType = TypeWithoutAuxSlotTag(this->u.local.type);
        }
        else if (isProto)
        {
            // TODO (InlineCacheCleanup): For loads from proto, we could at least grab the value from protoObject's slot
            // (given by the cache) and see if its a function. Only then, does it make sense to check with the type handler.
            propertyOwnerType = this->u.proto.prototypeObject->GetType();
        }
        else
        {
            propertyOwnerType = this->u.accessor.object->GetType();
        }

        Assert(propertyOwnerType != null);

        if (Js::DynamicType::Is(propertyOwnerType->GetTypeId()))
        {
            Js::DynamicTypeHandler* propertyOwnerTypeHandler = ((Js::DynamicType*)propertyOwnerType)->GetTypeHandler();
            Js::PropertyId propertyId = functionBody->GetPropertyIdFromCacheId(cacheId);
            Js::PropertyRecord const * const methodPropertyRecord = functionBody->GetScriptContext()->GetPropertyName(propertyId);

            Var fixedMethod = null;
            bool isUseFixedProperty;
            if (isLocal || isProto)
            {
                isUseFixedProperty = propertyOwnerTypeHandler->TryUseFixedProperty(methodPropertyRecord, &fixedMethod, Js::FixedPropertyKind::FixedMethodProperty, functionBody->GetScriptContext());
            }
            else
            {
                isUseFixedProperty = propertyOwnerTypeHandler->TryUseFixedAccessor(methodPropertyRecord, &fixedMethod, Js::FixedPropertyKind::FixedAccessorProperty, this->IsGetterAccessor(), functionBody->GetScriptContext());
            }
            AssertMsg(fixedMethod == null || Js::JavascriptFunction::Is(fixedMethod), "The fixed value should have been a Method !!!");
            *pFixedMethod = reinterpret_cast<JavascriptFunction*>(fixedMethod);
            return isUseFixedProperty;
        }

        return false;
    }

    void InlineCache::CopyTo(PropertyId propertyId, ScriptContext * scriptContext, InlineCache * const clone)
    {
        DebugOnly(VerifyRegistrationForInvalidation(this, scriptContext, propertyId));
        DebugOnly(VerifyRegistrationForInvalidation(clone, scriptContext, propertyId));
        Assert(clone != null);

        // Note, the Register methods can throw due to OOM, so we need to do it before the inline cache is copied below.
        if (this->invalidationListSlotPtr != null && clone->invalidationListSlotPtr == null)
        {
            if (this->NeedsToBeRegisteredForProtoInvalidation())
            {
                scriptContext->RegisterProtoInlineCache(clone, propertyId);
            }
            else if (this->NeedsToBeRegisteredForStoreFieldInvalidation())
            {
                scriptContext->RegisterStoreFieldInlineCache(clone, propertyId);
            }
        }

        clone->u = this->u;
        
        DebugOnly(VerifyRegistrationForInvalidation(clone, scriptContext, propertyId));
    }

    __inline PolymorphicInlineCache * PolymorphicInlineCache::New(uint16 size, FunctionBody * functionBody)
    {
        ScriptContext * scriptContext = functionBody->GetScriptContext();
        InlineCache * inlineCaches = AllocatorNewArrayZ(InlineCacheAllocator, scriptContext->GetInlineCacheAllocator(), InlineCache, size);
#ifdef POLY_INLINE_CACHE_SIZE_STATS
        scriptContext->GetInlineCacheAllocator()->LogPolyCacheAlloc(size * sizeof(InlineCache));
#endif
        PolymorphicInlineCache * polymorphicInlineCache = RecyclerNewFinalizedLeaf(scriptContext->GetRecycler(), PolymorphicInlineCache, inlineCaches, size, functionBody);

        // Insert the cache into finalization list.  We maintain this linked list of polymorphic caches because when we allocate
        // a larger cache, the old one might still be used by some code on the stack.  Consequently, we can't release
        // the inline cache array back to the arena allocator.  The list is leaf-allocated and so does not keep the
        // old caches alive.  As soon as they are collectible, their finalizer releases the inline cache array to the arena.
        polymorphicInlineCache->prev = null;
        polymorphicInlineCache->next = functionBody->GetPolymorphicInlineCachesHead();
        if (polymorphicInlineCache->next)
        {
            polymorphicInlineCache->next->prev = polymorphicInlineCache;
        }
        functionBody->SetPolymorphicInlineCachesHead(polymorphicInlineCache);

        return polymorphicInlineCache;
    }

    template<bool IsAccessor>
    __inline bool PolymorphicInlineCache::HasDifferentType(
        const bool isProto,
        const Type * type,
        const Type * typeWithoutProperty) const
    {
        Assert(!IsAccessor && !isProto || !typeWithoutProperty);

        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        if (inlineCaches[inlineCacheIndex].HasDifferentType<IsAccessor>(isProto, type, typeWithoutProperty))
        {
            return true;
        }

        if (!IsAccessor && !isProto && typeWithoutProperty)
        {
            inlineCacheIndex = GetInlineCacheIndexForType(typeWithoutProperty);
            return inlineCaches[inlineCacheIndex].HasDifferentType<IsAccessor>(isProto, type, typeWithoutProperty);
        }

        return false;
    }

    __inline bool PolymorphicInlineCache::HasType_Flags(const Type * type) const
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        return inlineCaches[inlineCacheIndex].HasType_Flags(type);
    }

    template<
        bool CheckLocal,
        bool CheckProto,
        bool CheckAccessor>
    inline void PolymorphicInlineCache::CloneInlineCacheToEmptySlotInCollision(Type * const type, uint inlineCacheIndex)
    {
        if (CheckLocal && (inlineCaches[inlineCacheIndex].u.local.type == type || inlineCaches[inlineCacheIndex].u.local.type == TypeWithAuxSlotTag(type))) 
        {
            return; 
        }
        if (CheckProto && (inlineCaches[inlineCacheIndex].u.proto.type == type || inlineCaches[inlineCacheIndex].u.proto.type == TypeWithAuxSlotTag(type)))
        { 
            return;
        }
        if (CheckAccessor && (inlineCaches[inlineCacheIndex].u.accessor.type == type || inlineCaches[inlineCacheIndex].u.accessor.type == TypeWithAuxSlotTag(type)))
        {  
            return;
        }

        // Collision is with a cache having a different type.

        uint tryInlineCacheIndex = GetNextInlineCacheIndex(inlineCacheIndex);

        // Iterate over the inline caches in the polymorphic cache, stop when:
        //   1. an empty inline cache is found, or
        //   2. a cache already populated with the incoming type is found, or
        //   3. all the inline caches have been looked at.
        while (!inlineCaches[tryInlineCacheIndex].IsEmpty() && tryInlineCacheIndex != inlineCacheIndex)
        {
            if (CheckLocal && (inlineCaches[tryInlineCacheIndex].u.local.type == type || inlineCaches[tryInlineCacheIndex].u.local.type == TypeWithAuxSlotTag(type)))
            {
                break;
            }
            if (CheckProto && (inlineCaches[tryInlineCacheIndex].u.proto.type == type || inlineCaches[tryInlineCacheIndex].u.proto.type == TypeWithAuxSlotTag(type)))
            {
                Assert(GetInlineCacheIndexForType(inlineCaches[tryInlineCacheIndex].u.proto.type) == inlineCacheIndex);
                break;
            }
            if (CheckAccessor && (inlineCaches[tryInlineCacheIndex].u.accessor.type == type || inlineCaches[tryInlineCacheIndex].u.accessor.type == TypeWithAuxSlotTag(type)))
            {
                Assert(GetInlineCacheIndexForType(inlineCaches[tryInlineCacheIndex].u.accessor.type) == inlineCacheIndex);
                break;
            }
            tryInlineCacheIndex = GetNextInlineCacheIndex(tryInlineCacheIndex);
        }
        if (tryInlineCacheIndex != inlineCacheIndex)
        {
            if (inlineCaches[inlineCacheIndex].invalidationListSlotPtr != null)
            {
                Assert(*(inlineCaches[inlineCacheIndex].invalidationListSlotPtr) == &inlineCaches[inlineCacheIndex]);
                if (inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr != null)
                {
                    Assert(*(inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr) == &inlineCaches[tryInlineCacheIndex]);
                }
                else
                {
                    inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr = inlineCaches[inlineCacheIndex].invalidationListSlotPtr;
                    *(inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr) = &inlineCaches[tryInlineCacheIndex];
                    inlineCaches[inlineCacheIndex].invalidationListSlotPtr = null;
                }
            }
            inlineCaches[tryInlineCacheIndex].u = inlineCaches[inlineCacheIndex].u;
            // Let's clear the cache slot on which we had the collision.  We might have stolen the invalidationListSlotPtr, so it may not pass VerifyRegistrationForInvalidation.
            // Besides, It will be repopulated with the incoming data, and registered for invalidation, if necessary.           
            inlineCaches[inlineCacheIndex].Clear();
        }
    }

    __inline void PolymorphicInlineCache::CacheLocal(
        Type *const type,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        Type *const typeWithoutProperty,
        int requiredAuxSlotCapacity,
        ScriptContext *const requestContext)
    {
        // Let's not waste polymorphic cache slots by caching both the type without property and type with property. If the
        // cache is used for both adding a property and setting the existing property, then those instances will cause both
        // types to be cached. Until then, caching both types proactively here can unnecessarily trash useful cached info
        // because the types use different slots, unlike a monomorphic inline cache.
        if(!typeWithoutProperty)
        {
            uint inlineCacheIndex = GetInlineCacheIndexForType(type);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
            bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
            if (!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
            {
                if(!inlineCaches[inlineCacheIndex].IsEmpty() && !inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
                {
                    if (inlineCaches[inlineCacheIndex].IsLocal())
                    {
                        CloneInlineCacheToEmptySlotInCollision<true,false,false>(type, inlineCacheIndex);
                    }
                    else if (inlineCaches[inlineCacheIndex].IsProto())
                    {
                        CloneInlineCacheToEmptySlotInCollision<false,true,false>(type, inlineCacheIndex);
                    }
                    else
                    {
                        CloneInlineCacheToEmptySlotInCollision<false,false,true>(type, inlineCacheIndex);
                    }
                }
            }

            inlineCaches[inlineCacheIndex].CacheLocal(
                type, propertyId, propertyIndex, isInlineSlot, nullptr, requiredAuxSlotCapacity, requestContext);
#if DBG_DUMP
            if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
            {
                Output::Print(L"PIC::CacheLocal, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
                inlineCaches[inlineCacheIndex].Dump();
                Output::Print(L"\n");
                Output::Flush();
            }
#endif
            PHASE_PRINT_INTRUSIVE_TESTTRACE1(
                Js::PolymorphicInlineCachePhase,
                L"TestTrace PIC: CacheLocal, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
        }
        else
        {
            uint inlineCacheIndex = GetInlineCacheIndexForType(typeWithoutProperty);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
            bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
            inlineCaches[inlineCacheIndex].CacheLocal(
                type, propertyId, propertyIndex, isInlineSlot, typeWithoutProperty, requiredAuxSlotCapacity, requestContext);
#if DBG_DUMP
            if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
            {
                Output::Print(L"PIC::CacheLocal, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
                inlineCaches[inlineCacheIndex].Dump();
                Output::Print(L"\n");
                Output::Flush();
            }
#endif
            PHASE_PRINT_INTRUSIVE_TESTTRACE1(
                Js::PolymorphicInlineCachePhase,
                L"TestTrace PIC: CacheLocal, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
        }
    }

    __inline void PolymorphicInlineCache::CacheProto(
        DynamicObject *const prototypeObjectWithProperty,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        const bool isMissing,
        Type *const type,
        ScriptContext *const requestContext)
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
        bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
        if(!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
        {
            if(!inlineCaches[inlineCacheIndex].IsEmpty() && !inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
            {
                if (inlineCaches[inlineCacheIndex].IsLocal())
                {
                    CloneInlineCacheToEmptySlotInCollision<true,false,false>(type, inlineCacheIndex);
                }
                else if (inlineCaches[inlineCacheIndex].IsProto())
                {
                    CloneInlineCacheToEmptySlotInCollision<false,true,false>(type, inlineCacheIndex);
                }
                else
                {
                    CloneInlineCacheToEmptySlotInCollision<false,false,true>(type, inlineCacheIndex);
                }
            }
        }

        inlineCaches[inlineCacheIndex].CacheProto(
            prototypeObjectWithProperty, propertyId, propertyIndex, isInlineSlot, isMissing, type, requestContext);

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
        {
            Output::Print(L"PIC::CacheProto, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
            inlineCaches[inlineCacheIndex].Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
        PHASE_PRINT_INTRUSIVE_TESTTRACE1(
            Js::PolymorphicInlineCachePhase,
            L"TestTrace PIC: CacheProto, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
    }

    __inline void PolymorphicInlineCache::CacheAccessor(
        const bool isGetter,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        Type *const type,
        DynamicObject *const object,
        const bool isOnProto,
        ScriptContext *const requestContext)
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
        bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
        if(!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
        {
            if(!inlineCaches[inlineCacheIndex].IsEmpty() && !inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
            {
                if (inlineCaches[inlineCacheIndex].IsLocal())
                {
                    CloneInlineCacheToEmptySlotInCollision<true,false,false>(type, inlineCacheIndex);
                }
                else if (inlineCaches[inlineCacheIndex].IsProto())
                {
                    CloneInlineCacheToEmptySlotInCollision<false,true,false>(type, inlineCacheIndex);
                }
                else
                {
                    CloneInlineCacheToEmptySlotInCollision<false,false,true>(type, inlineCacheIndex);
                }
            }
        }

        inlineCaches[inlineCacheIndex].CacheAccessor(isGetter, propertyId, propertyIndex, isInlineSlot, type, object, isOnProto, requestContext);

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
        {
            Output::Print(L"PIC::CacheAccessor, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
            inlineCaches[inlineCacheIndex].Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
        PHASE_PRINT_INTRUSIVE_TESTTRACE1(
            Js::PolymorphicInlineCachePhase,
            L"TestTrace PIC: CacheAccessor, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
    }

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
    template <typename TDelegate>
    inline bool PolymorphicInlineCache::CheckClonedInlineCache(uint inlineCacheIndex, TDelegate mapper)
    {
        bool success = false;
        uint tryInlineCacheIndex = GetNextInlineCacheIndex(inlineCacheIndex);
        do
        {
            if (inlineCaches[tryInlineCacheIndex].IsEmpty())
            {
                break;
            }
            success = mapper(tryInlineCacheIndex);
            if (success)
            {
                Assert(inlineCaches[inlineCacheIndex].invalidationListSlotPtr == null || *inlineCaches[inlineCacheIndex].invalidationListSlotPtr == &inlineCaches[inlineCacheIndex]);
                Assert(inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr == null || *inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr == &inlineCaches[tryInlineCacheIndex]);

                // Swap inline caches, including their invalidationListSlotPtrs.
                InlineCache temp = inlineCaches[tryInlineCacheIndex];
                inlineCaches[tryInlineCacheIndex] = inlineCaches[inlineCacheIndex];
                inlineCaches[inlineCacheIndex] = temp;

                // Fix up invalidationListSlotPtrs to point to their owners.
                if (inlineCaches[inlineCacheIndex].invalidationListSlotPtr != null)
                {
                    *inlineCaches[inlineCacheIndex].invalidationListSlotPtr = &inlineCaches[inlineCacheIndex];
                }
                if (inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr != null)
                {
                    *inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr = &inlineCaches[tryInlineCacheIndex];
                }

                break;
            }
            tryInlineCacheIndex = GetNextInlineCacheIndex(tryInlineCacheIndex);

        } while(tryInlineCacheIndex != inlineCacheIndex);

        return success;
    }
#endif

    template<
        bool CheckLocal,
        bool CheckProto,
        bool CheckAccessor,
        bool CheckMissing,
        bool IsInlineCacheAvailable,
        bool ReturnOperationInfo>
    __inline bool PolymorphicInlineCache::TryGetProperty(
        Var const instance,
        RecyclableObject *const propertyObject,
        const PropertyId propertyId,
        Var *const propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        InlineCache *const inlineCacheToPopulate)
    {
        Assert(!IsInlineCacheAvailable || inlineCacheToPopulate);
        Assert(!ReturnOperationInfo || operationInfo);

        Type * const type = propertyObject->GetType();
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        InlineCache *cache = &inlineCaches[inlineCacheIndex];

#ifdef INLINE_CACHE_STATS
        bool isEmpty = false;
        if(PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            isEmpty = cache->IsEmpty();
        }
#endif
        bool result = cache->TryGetProperty<CheckLocal, CheckProto, CheckAccessor, CheckMissing, ReturnOperationInfo>(
                            instance, propertyObject, propertyId, propertyValue, requestContext, operationInfo);

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
        if (!result && !cache->IsEmpty())
        {
            result = CheckClonedInlineCache(inlineCacheIndex, [&] (uint tryInlineCacheIndex) -> bool
            {
                cache = &inlineCaches[tryInlineCacheIndex];
                return cache->TryGetProperty<CheckLocal, CheckProto, CheckAccessor, CheckMissing, ReturnOperationInfo>(
                        instance, propertyObject, propertyId, propertyValue, requestContext, operationInfo);
            });
        }
#endif

        if(IsInlineCacheAvailable && result)
        {
            cache->CopyTo(propertyId, requestContext, inlineCacheToPopulate);
        }

#ifdef INLINE_CACHE_STATS
        if(PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            bool collision = !result && !isEmpty;
            this->functionBody->GetScriptContext()->LogCacheUsage(this, /*isGet*/ true, propertyId, result, collision);
        }
#endif

        return result;
    }

    __inline bool PolymorphicInlineCache::PretendTryGetProperty(
        Type *const type,
        PropertyCacheOperationInfo * operationInfo)
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        return inlineCaches[inlineCacheIndex].PretendTryGetProperty(type, operationInfo);
    }

    template<
        bool CheckLocal,
        bool CheckLocalTypeWithoutProperty,
        bool CheckAccessor,
        bool IsInlineCacheAvailable,
        bool ReturnOperationInfo>
    __inline bool PolymorphicInlineCache::TrySetProperty(
        RecyclableObject *const object,
        const PropertyId propertyId,
        Var propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        InlineCache *const inlineCacheToPopulate,
        const PropertyOperationFlags propertyOperationFlags)
    {
        Assert(!IsInlineCacheAvailable || inlineCacheToPopulate);
        Assert(!ReturnOperationInfo || operationInfo);

        Type * const type = object->GetType();
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        InlineCache *cache = &inlineCaches[inlineCacheIndex];

#ifdef INLINE_CACHE_STATS
        bool isEmpty = false;
        if(PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            isEmpty = cache->IsEmpty();
        }
#endif
        bool result = cache->TrySetProperty<CheckLocal, CheckLocalTypeWithoutProperty, CheckAccessor, ReturnOperationInfo>(
                object, propertyId, propertyValue, requestContext, operationInfo, propertyOperationFlags);

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
        if (!result && !cache->IsEmpty())
        {
            result = CheckClonedInlineCache(inlineCacheIndex, [&] (uint tryInlineCacheIndex) -> bool
            {
                cache = &inlineCaches[tryInlineCacheIndex];
                return cache->TrySetProperty<CheckLocal, CheckLocalTypeWithoutProperty, CheckAccessor, ReturnOperationInfo>(
                    object, propertyId, propertyValue, requestContext, operationInfo, propertyOperationFlags);
            });
        }
#endif

        if(IsInlineCacheAvailable && result)
        {
            cache->CopyTo(propertyId, requestContext, inlineCacheToPopulate);
        }

#ifdef INLINE_CACHE_STATS
        if(PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            bool collision = !result && !isEmpty;
            this->functionBody->GetScriptContext()->LogCacheUsage(this, /*isGet*/ false, propertyId, result, collision);
        }
#endif

        return result;
    }

    __inline bool PolymorphicInlineCache::PretendTrySetProperty(
        Type *const type,
        Type *const oldType,
        PropertyCacheOperationInfo * operationInfo)
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        return inlineCaches[inlineCacheIndex].PretendTrySetProperty(type, oldType, operationInfo);
    }

    void PolymorphicInlineCache::CopyTo(PropertyId propertyId, ScriptContext* scriptContext, PolymorphicInlineCache *const clone)
    {
        Assert(clone);

        clone->ignoreForEquivalentObjTypeSpec = this->ignoreForEquivalentObjTypeSpec;
        clone->cloneForJitTimeUse = this->cloneForJitTimeUse;

        for (uint i = 0; i < GetSize(); ++i)
        {
            Type * type = inlineCaches[i].GetType();
            if (type)
            {
                uint inlineCacheIndex = clone->GetInlineCacheIndexForType(type);

                // When copying inline caches from one polymorphic cache to another, types are again hashed to get the corresponding indices in the new polymorphic cache.
                // This might lead to collision in the new cache. We need to try to resolve that collision.
                if(!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
                {
                    if (!clone->inlineCaches[inlineCacheIndex].IsEmpty() && !clone->inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
                    {
                        if (clone->inlineCaches[inlineCacheIndex].IsLocal())
                        {
                            clone->CloneInlineCacheToEmptySlotInCollision<true, false, false>(type, inlineCacheIndex);
                        }
                        else if (clone->inlineCaches[inlineCacheIndex].IsProto())
                        {
                            clone->CloneInlineCacheToEmptySlotInCollision<false, true, false>(type, inlineCacheIndex);
                        }
                        else
                        {
                            clone->CloneInlineCacheToEmptySlotInCollision<false, false, true>(type, inlineCacheIndex);
                        }
                    
                    }
                }
                inlineCaches[i].CopyTo(propertyId, scriptContext, &clone->inlineCaches[inlineCacheIndex]);
            }
        }
    }

    ConstructorCache* ConstructorCache::EnsureValidInstance(ConstructorCache* currentCache, ScriptContext* scriptContext)
    {
        Assert(currentCache != null);

        ConstructorCache* newCache = currentCache;

        // If the old cache has been invalidated, we need to create a new one to avoid incorrectly re-validating
        // caches that may have been hard-coded in the JIT-ed code with different prototype and type.  However, if
        // the cache is already polymorphic, it will not be hard-coded, and hence we don't need to allocate a new
        // one - in case the prototype property changes frequently.
        if (ConstructorCache::IsDefault(currentCache) || (currentCache->IsInvalidated() && !currentCache->IsPolymorphic()))
        {
            // Review (jedmiad): I don't think we need to zero the struct, since we initialize each field.
            newCache = RecyclerNew(scriptContext->GetRecycler(), ConstructorCache);
            // TODO: Consider marking the cache as polymorphic only if the prototype and type actually changed.  In fact,
            // if they didn't change we could reuse the same cache and simply mark it as valid.  Not really true.  The cache
            // might have been invalidated due to a property becoming read-only.  In that case we can't re-validate an old
            // monomorphic cache.  We must allocate a new one.
            newCache->content.isPolymorphic = currentCache->content.isPopulated && currentCache->content.hasPrototypeChanged;
        }

        // If we kept the old invalidated cache, it better be marked as polymorphic.
        Assert(!newCache->IsInvalidated() || newCache->IsPolymorphic());

        // If the cache was polymorphic, we shouldn't have allocated a new one.
        Assert(!currentCache->IsPolymorphic() || newCache == currentCache);

        return newCache;
    }

    __inline void IsInstInlineCache::Set(Type * instanceType, JavascriptFunction * function, JavascriptBoolean * result)
    {
        this->type = instanceType;
        this->function = function;
        this->result = result;
    }

    __inline void IsInstInlineCache::Clear()
    {
        this->type = NULL;
        this->function = NULL;
        this->result = NULL;
    }

    __inline void IsInstInlineCache::Unregister(ScriptContext * scriptContext)
    {
        scriptContext->GetThreadContext()->UnregisterIsInstInlineCache(this, this->function);
    }

    __inline bool IsInstInlineCache::TryGetResult(Var instance, JavascriptFunction * function, JavascriptBoolean ** result)
    {
        // In order to get the result from the cache we must have a function instance.
        Assert(function != NULL);

        if (this->function == function &&
            this->type == RecyclableObject::FromVar(instance)->GetType())
        {
            if (result != null)
            {
                (*result = this->result);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    __inline void IsInstInlineCache::Cache(Type * instanceType, JavascriptFunction * function, JavascriptBoolean *  result, ScriptContext * scriptContext)
    {  
        // In order to populate the cache we must have a function instance.
        Assert(function != null);

        // We assume the following invariant: (cache->function != null) => script context is registered as having some populated instance-of inline caches and
        // this cache is registered with thread context for invalidation.
        if (this->function == function)
        {
            Assert(scriptContext->IsRegisteredAsScriptContextWithIsInstInlineCaches());
            Assert(scriptContext->IsIsInstInlineCacheRegistered(this, function));
            this->Set(instanceType, function, result);
        }
        else
        {
            if (this->function != null) 
            {
                Unregister(scriptContext);
                Clear();
            }

            scriptContext->RegisterAsScriptContextWithIsInstInlineCaches();
            // If the cache's function is not null, the cache must have been registered already.  No need to register again.  
            // In fact, ThreadContext::RegisterIsInstInlineCache, would assert if we tried to re-register the same cache (to enforce the invariant above).
            // Review (jedmiad): What happens if we run out of memory inside RegisterIsInstInlieCache?
            scriptContext->RegisterIsInstInlineCache(this, function);   
            this->Set(instanceType, function, result);
        }
    }

}

