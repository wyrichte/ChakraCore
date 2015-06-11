//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    DEFINE_RECYCLER_TRACKER_PERF_COUNTER(DynamicObject);
    DEFINE_RECYCLER_TRACKER_WEAKREF_PERF_COUNTER(DynamicObject);

    DynamicObject::DynamicObject(DynamicType * type, const bool initSlots) :
        RecyclableObject(type),
        auxSlots(null),
        objectArray(null)
    {
        Assert(!UsesObjectArrayOrFlagsAsFlags());
        if(initSlots)
        {
            InitSlots(this);
        }
        else
        {
            Assert(type->GetTypeHandler()->GetInlineSlotCapacity() == type->GetTypeHandler()->GetSlotCapacity());
        }
    }

    DynamicObject::DynamicObject(DynamicType * type, ScriptContext * scriptContext) :
#if DBG || defined(PROFILE_TYPES)
        RecyclableObject(type, scriptContext),
#else
        RecyclableObject(type),
#endif
        auxSlots(null),
        objectArray(null)
    {
        Assert(!UsesObjectArrayOrFlagsAsFlags());
        InitSlots(this, scriptContext);
    }

    DynamicObject::DynamicObject(DynamicObject * instance) :
        RecyclableObject(instance->type),
        auxSlots(instance->auxSlots),       // TODO: stack allocate aux Slots
        objectArray(instance->objectArray)  // copying the array should copy the array flags and array call site index as well
    {
        DynamicTypeHandler * typeHandler = this->GetTypeHandler();

        // TODO: stack allocate aux Slots
        Assert(typeHandler->IsObjectHeaderInlinedTypeHandler() || !ThreadContext::IsOnStack(this->auxSlots));
        int propertyCount = typeHandler->GetPropertyCount();
        int inlineSlotCapacity = GetTypeHandler()->GetInlineSlotCapacity();
        int inlineSlotCount = min(inlineSlotCapacity, propertyCount);
        Var * srcSlots = reinterpret_cast<Var*>(reinterpret_cast<size_t>(instance) + typeHandler->GetOffsetOfInlineSlots());
        Var * dstSlots = reinterpret_cast<Var*>(reinterpret_cast<size_t>(this) + typeHandler->GetOffsetOfInlineSlots());
#if !FLOATVAR
        ScriptContext * scriptContext = this->GetScriptContext();
#endif
        for (int i = 0; i < inlineSlotCount; i++)
        {
            // TODO: when we support assigning a mark temp object as property to another mark temp object
            // we will need to box them too.
            // TODO: Beware of bailout in the middle of initializing a object literal.  Some slots
            // might not be initialized yet.            
            // dstSlots[i] = JavascriptOperators::BoxStackInstance(srcSlots[i], this->GetScriptContext());

#if !FLOATVAR
            // Currently we only support temp numbers assigned to stack objects
            dstSlots[i] = JavascriptNumber::BoxStackNumber(srcSlots[i], scriptContext);
#else
            dstSlots[i] = srcSlots[i];
#endif
            
        }

        if (propertyCount > inlineSlotCapacity)
        {
            uint auxSlotCount = propertyCount - inlineSlotCapacity;

            for (uint i = 0; i < auxSlotCount; i++)
            {
                // TODO: when we support assigning a mark temp object as property to another mark temp object
                // we will need to box them too.
                // TODO: Beware of bailout in the middle of initializing a object literal.  Some slots
                // might not be initialized yet.
                // auxSlots[i] = JavascriptOperators::BoxStackInstance(instance->auxSlots[i], scriptContext);

#if !FLOATVAR
                // Currently we only support temp numbers assigned to stack objects
                auxSlots[i] = JavascriptNumber::BoxStackNumber(instance->auxSlots[i], scriptContext);
#else
                auxSlots[i] = instance->auxSlots[i];
#endif
            }
        }
    }

    bool
    DynamicObject::GetIsExtensible() const
    {
        return this->GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsExtensibleFlag;
    }

    Var
    DynamicObject::GetNextProperty(PropertyIndex& index, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols)
    {
        JavascriptString* propertyString= null;
        PropertyId propertyId = Constants::NoProperty;

        if (!this->GetTypeHandler()->FindNextProperty(this->GetScriptContext(), index, &propertyString, &propertyId, nullptr, this->GetType(), typeToEnumerate, requireEnumerable, enumSymbols))
        {
            return null;
        }

        Assert(propertyString);
        Assert(propertyId);
        if (VirtualTableInfo<PropertyString>::HasVirtualTable(propertyString))
        {
            PropertyCache const* cache = ((PropertyString*)propertyString)->GetPropertyCache();
            if (cache && cache->type == this->GetType())
            {
                if (cache->isInlineSlot)
                {
                    return this->GetInlineSlot(cache->dataSlotIndex);
                }
                else
                {
                    return this->GetAuxSlot(cache->dataSlotIndex);
                }
            }
        }

        Var value = null;
        BOOL result;
        if (propertyId != Constants::NoProperty)
        {
            result = this->GetProperty(this, propertyId, &value, NULL, this->GetScriptContext());
        }
        else
        {
            result = this->GetProperty(this, propertyString, &value, NULL, this->GetScriptContext());
        }
        Assert(result);

        return value;
    }

    Var
    DynamicObject::GetNextProperty(BigPropertyIndex& index, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols)
    {
        JavascriptString* propertyString = null;
        PropertyId propertyId = Constants::NoProperty;

        if (!this->GetTypeHandler()->FindNextProperty(this->GetScriptContext(), index, &propertyString, &propertyId, nullptr, this->GetType(), typeToEnumerate, requireEnumerable, enumSymbols))
        {
            return null;
        }

        Assert(propertyString);
        Assert(propertyId);
        //PropertyCache* cache = propertyString->GetPropertyCache();
        //if (cache && cache->type == this->GetType())
        //{
        //    return this->slots[cache->dataSlotIndex];
        //}

        Var value = null;
        BOOL result;
        if (propertyId != Constants::NoProperty)
        {
            result = this->GetProperty(this, propertyId, &value, NULL, this->GetScriptContext());
        }
        else
        {
            result = this->GetProperty(this, propertyString, &value, NULL, this->GetScriptContext());
        }
        Assert(result);

        return value;
    }

    BOOL
    DynamicObject::FindNextProperty(PropertyIndex& index, __out JavascriptString** propertyString, __out PropertyId* propertyId, __out PropertyAttributes* attributes, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols) const
    {
        if (index == Constants::NoSlot)
        {
            return FALSE;
        }
        return this->GetTypeHandler()->FindNextProperty(this->GetScriptContext(), index, propertyString, propertyId, attributes, this->GetType(), typeToEnumerate, requireEnumerable, enumSymbols);
    }

    BOOL
    DynamicObject::FindNextProperty(BigPropertyIndex& index, __out JavascriptString** propertyString, __out PropertyId* propertyId, __out PropertyAttributes* attributes, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols) const
    {
        if (index == Constants::NoBigSlot)
        {
            return FALSE;
        }
        return this->GetTypeHandler()->FindNextProperty(this->GetScriptContext(), index, propertyString, propertyId, attributes, this->GetType(), typeToEnumerate, requireEnumerable, enumSymbols);
    }

    DynamicObject* DynamicObject::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        DynamicType *type = scriptContext->GetLibrary()->CreateObjectTypeNoCache(RecyclableObject::FromVar(scriptContext->CopyOnWrite(this->GetPrototype())), this->GetTypeId());
        return CopyOnWriteObject<DynamicObject>::New(recycler, type, this, scriptContext);
    }

    BOOL
    DynamicObject::HasDeferredTypeHandler() const
    {
        return this->GetTypeHandler()->IsDeferredTypeHandler();
    }

    void
    DynamicObject::EnsureObjectReady()
    {
        this->GetTypeHandler()->EnsureObjectReady(this);
    }

    DynamicTypeHandler *
    DynamicObject::GetTypeHandler() const
    {
        return this->GetDynamicType()->GetTypeHandler();
    }

    uint16 DynamicObject::GetOffsetOfInlineSlots() const
    {
        return this->GetDynamicType()->GetTypeHandler()->GetOffsetOfInlineSlots();
    }

    void
    DynamicObject::SetTypeHandler(DynamicTypeHandler * typeHandler, bool hasChanged)
    {
        if (hasChanged && this->HasLockedType())
        {
            this->ChangeType();
        }
        this->GetDynamicType()->typeHandler = typeHandler;
    }

    DynamicType* DynamicObject::DuplicateType()
    {
        return RecyclerNew(GetRecycler(), DynamicType, this->GetDynamicType());
    }

    /*
    *   DynamicObject::IsTypeHandlerCompatibleForObjectHeaderInlining
    *   -   Checks if the TypeHandlers are compatible for transition from oldTypeHandler to newTypeHandler
    */
    bool DynamicObject::IsTypeHandlerCompatibleForObjectHeaderInlining(DynamicTypeHandler * oldTypeHandler, DynamicTypeHandler * newTypeHandler)
    {
        Assert(oldTypeHandler);
        Assert(newTypeHandler);

        return
            oldTypeHandler->GetInlineSlotCapacity() == newTypeHandler->GetInlineSlotCapacity() ||
            (
                oldTypeHandler->IsObjectHeaderInlinedTypeHandler() &&
                newTypeHandler->GetInlineSlotCapacity() ==
                    oldTypeHandler->GetInlineSlotCapacity() - DynamicTypeHandler::GetObjectHeaderInlinableSlotCapacity()
            );
    }

    bool DynamicObject::IsObjectHeaderInlinedTypeHandlerUnchecked() const
    {
        return this->GetTypeHandler()->IsObjectHeaderInlinedTypeHandlerUnchecked();
    }

    bool DynamicObject::IsObjectHeaderInlinedTypeHandler() const
    {
        return this->GetTypeHandler()->IsObjectHeaderInlinedTypeHandler();
    }

    bool DynamicObject::DeoptimizeObjectHeaderInlining()
    {
        if(!IsObjectHeaderInlinedTypeHandler())
        {
            return false;
        }

        if (PHASE_TRACE1(Js::ObjectHeaderInliningPhase))
        {
            Output::Print(L"ObjectHeaderInlining: Deoptimizing the object.\n");
            Output::Flush();
        }

        PathTypeHandlerBase *const oldTypeHandler = PathTypeHandlerBase::FromTypeHandler(GetTypeHandler());
        SimplePathTypeHandler *const newTypeHandler = oldTypeHandler->DeoptimizeObjectHeaderInlining(GetLibrary());

        const PropertyIndex newInlineSlotCapacity = newTypeHandler->GetInlineSlotCapacity();
        DynamicTypeHandler::AdjustSlots(
            this,
            newInlineSlotCapacity,
            newTypeHandler->GetSlotCapacity() - newInlineSlotCapacity);

        DynamicType *const newType = DuplicateType();
        newType->typeHandler = newTypeHandler;
        newType->ShareType();
        type = newType;
        return true;
    }

    void DynamicObject::ChangeType()
    {
        Assert(!GetDynamicType()->GetIsShared() || GetTypeHandler()->GetIsShared());
        this->type = this->DuplicateType();
    }

    void DynamicObject::ChangeTypeIf(const Type* oldType)
    {
        if (this->type == oldType)
        {
            ChangeType();
        }
    }

    DynamicObjectFlags DynamicObject::GetArrayFlags() const
    {
        Assert(IsAnyArray(const_cast<DynamicObject *>(this)));
        Assert(UsesObjectArrayOrFlagsAsFlags()); // an array object never has another internal array
        return arrayFlags & DynamicObjectFlags::AllArrayFlags;
    }

    DynamicObjectFlags DynamicObject::GetArrayFlags_Unchecked() const // do not use except in extreme circumstances
    {
        return arrayFlags & DynamicObjectFlags::AllArrayFlags;
    }

    void DynamicObject::InitArrayFlags(const DynamicObjectFlags flags)
    {        
        Assert(IsAnyArray(this));
        Assert(this->objectArray == null);            
        Assert((flags & DynamicObjectFlags::ObjectArrayFlagsTag) == DynamicObjectFlags::ObjectArrayFlagsTag);
        Assert((flags & ~DynamicObjectFlags::AllFlags) == DynamicObjectFlags::None);
        this->arrayFlags = flags;     
    }

    void DynamicObject::SetArrayFlags(const DynamicObjectFlags flags)
    {
        Assert(IsAnyArray(this));
        Assert(UsesObjectArrayOrFlagsAsFlags()); // an array object never has another internal array
        // Make sure we don't attempt to set any flags outside of the range of array flags.
        Assert((arrayFlags & ~DynamicObjectFlags::AllArrayFlags) == DynamicObjectFlags::ObjectArrayFlagsTag);
        Assert((flags & ~DynamicObjectFlags::AllArrayFlags) == DynamicObjectFlags::None);
        arrayFlags = flags | DynamicObjectFlags::ObjectArrayFlagsTag;          
    }

    ProfileId DynamicObject::GetArrayCallSiteIndex() const
    {
        Assert(IsAnyArray(const_cast<DynamicObject *>(this)));
        return arrayCallSiteIndex;
    }

    void DynamicObject::SetArrayCallSiteIndex(ProfileId profileId)
    {
        Assert(IsAnyArray(this));
        arrayCallSiteIndex = profileId;
    }

    void DynamicObject::SetIsPrototype()
    {
        DynamicTypeHandler* currentTypeHandler = this->GetTypeHandler();
        Js::DynamicType* oldType = this->GetDynamicType();

#if DBG
        bool wasShared = currentTypeHandler->GetIsShared();
        bool wasPrototype = (currentTypeHandler->GetFlags() & DynamicTypeHandler::IsPrototypeFlag) != 0;
        Assert(!DynamicTypeHandler::IsolatePrototypes() || !currentTypeHandler->RespectsIsolatePrototypes() || !currentTypeHandler->GetIsOrMayBecomeShared() || !wasPrototype);
#endif

        // If this handler is not shared and it already has a prototype flag then we must have taken the required
        // type transition (if any) earlier when the singleton object first became a prototype.
        if ((currentTypeHandler->GetFlags() & (DynamicTypeHandler::IsSharedFlag | DynamicTypeHandler::IsPrototypeFlag)) == DynamicTypeHandler::IsPrototypeFlag)
        {
            Assert(this->GetObjectArray() == null || (this->GetObjectArray()->GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag) != 0);
            return;
        }

        currentTypeHandler->SetIsPrototype(this);
        // Get type handler again, in case it got changed by SetIsPrototype.
        currentTypeHandler = this->GetTypeHandler();

        // Set the object array as an prototype as well, so if it is an ES5 array, we will disable the array set element fast path
        ArrayObject * objectArray = this->GetObjectArray();
        if (objectArray)
        {
            objectArray->SetIsPrototype();
        }

#if DBG            
        Assert(currentTypeHandler->SupportsPrototypeInstances());
        Assert(!DynamicTypeHandler::IsolatePrototypes() || !currentTypeHandler->RespectsIsolatePrototypes() || !currentTypeHandler->GetIsOrMayBecomeShared());
        Assert((wasPrototype && !wasShared) || !DynamicTypeHandler::ChangeTypeOnProto() || !currentTypeHandler->RespectsChangeTypeOnProto() || this->GetDynamicType() != oldType);
#endif

        // If we haven't changed type we must explicitly invalidate store field inline caches to avoid properties
        // getting added to this prototype object on the fast path without proper invalidation.
        if (this->GetDynamicType() == oldType)
        {
            currentTypeHandler->InvalidateStoreFieldCachesForAllProperties(this->GetScriptContext());
        }
    }

    bool
    DynamicObject::LockType()
    {
        return this->GetDynamicType()->LockType();
    }

    bool
    DynamicObject::ShareType()
    {
        return this->GetDynamicType()->ShareType();
    }

    void
    DynamicObject::ResetObject(DynamicType* newType, BOOL keepProperties)
    {
        Assert(newType != NULL);
        Assert(!keepProperties || (!newType->GetTypeHandler()->IsDeferredTypeHandler() && newType->GetTypeHandler()->GetPropertyCount() == 0));

        // This is what's going on here.  The newType comes from the (potentially) new script context, but the object is
        // described by the old type handler, so we want to keep that type handler.  We set the new type on the object, but
        // then re-set the type handler of that type back to the old type handler.  In the process, we may actually change
        // the type of the object again (if the new type was locked) via DuplicateType, the new new type will then also be
        // from the new script context.
        DynamicType * oldType = this->GetDynamicType();
        DynamicTypeHandler* oldTypeHandler = oldType->GetTypeHandler();

        // TODO: Because we've disabled fixed properties on DOM objects, we don't need to rely on a type change here to 
        // invalidate fixed properties.  Apparently, under some circumstances (with F12 tools enabled) an object which 
        // is already in the new context can be reset and newType == oldType.  This seems like a bug in Trident.  If we
        // re-enable fixed properties on DOM object we'll have to investigate and address this issue.
        // Assert(newType != oldType);
        // We only expect DOM objects to ever be reset and we explicitly disable fixed properties on DOM objects.
        Assert(!oldTypeHandler->HasAnyFixedProperties());

        this->type = newType;
        if (!IsAnyArray(this))
        {
            this->objectArray = nullptr;
        }
        oldTypeHandler->ResetTypeHandler(this);
        Assert(this->GetScriptContext() == newType->GetScriptContext());

        if (this->GetTypeHandler()->IsDeferredTypeHandler())
        {
            return;
        }

        if (!keepProperties)
        {
            this->GetTypeHandler()->SetAllPropertiesToUndefined(this, false);
        }

        // Fix blue bug 445811: Targeted fix; Marshalling cannot handle non-Var values, so extract
        // the two internal property values that could appear on a CEO, clear them to null which
        // marshalling does handle, and then restore them after marshalling.  Neither property's
        // data needs marshalling because:
        //  1. StackTrace's data does not contain references to JavaScript objects that would need marshalling.
        //  2. Values in the WeakMapKeyMap can only be accessed by the WeakMap object that put them there.  If
        //     that WeakMap is marshalled it will take care of any necessary marshalling of the value by virtue
        //     of being wrapped in CrossSite<>.

        Var stackTraceValue = nullptr;
        if (this->GetInternalProperty(this, InternalPropertyIds::StackTrace, &stackTraceValue, nullptr, nullptr))
        {
            this->SetInternalProperty(InternalPropertyIds::StackTrace, nullptr, PropertyOperation_None, nullptr);
        }

        Var weakMapKeyMapValue = nullptr;
        if (this->GetInternalProperty(this, InternalPropertyIds::WeakMapKeyMap, &weakMapKeyMapValue, nullptr, nullptr))
        {
            this->SetInternalProperty(InternalPropertyIds::WeakMapKeyMap, nullptr, PropertyOperation_Force, nullptr);
        }

        Var mutationBpValue = nullptr;
        if (this->GetInternalProperty(this, InternalPropertyIds::MutationBp, &mutationBpValue, nullptr, nullptr))
        {
            this->SetInternalProperty(InternalPropertyIds::MutationBp, nullptr, PropertyOperation_Force, nullptr);
        }

        if (keepProperties)
        {
            this->GetTypeHandler()->MarshalAllPropertiesToScriptContext(this, this->GetScriptContext(), false); 

            // TODO: We need to marshal Var in object arrays, but we have no way to do that  yet, as the object array
            // is accessed directly and not thru the type handler.

            if (stackTraceValue)
            {
                this->SetInternalProperty(InternalPropertyIds::StackTrace, stackTraceValue, PropertyOperation_None, nullptr);
            }
            if (weakMapKeyMapValue)
            {
                this->SetInternalProperty(InternalPropertyIds::WeakMapKeyMap, weakMapKeyMapValue, PropertyOperation_Force, nullptr);
            }
            if (mutationBpValue)
            {
                this->SetInternalProperty(InternalPropertyIds::MutationBp, mutationBpValue, PropertyOperation_Force, nullptr);
            }
        }
    }

    bool
    DynamicObject::GetHasNoEnumerableProperties()
    {
        this->GetTypeHandler()->EnsureObjectReady(this);
        if (!this->GetDynamicType()->GetHasNoEnumerableProperties())
        {
            return false;
        }
        if (HasObjectArray() || (JavascriptArray::Is(this) && JavascriptArray::FromVar(this)->GetLength() != 0))
        {
            // TODO: Currently, all numeric properties are enumerable, and the type doesn't keep track of them
            // Need to change this when numeric properties has attributes
            return false;
        }
        return true;
    }

    bool
    DynamicObject::SetHasNoEnumerableProperties(bool value)
    {
        return this->GetDynamicType()->SetHasNoEnumerableProperties(value);
    }

    BigPropertyIndex
    DynamicObject::GetPropertyIndexFromInlineSlotIndex(uint inlineSlotIndex)
    {
        return this->GetTypeHandler()->GetPropertyIndexFromInlineSlotIndex(inlineSlotIndex);
    }

    BigPropertyIndex
    DynamicObject::GetPropertyIndexFromAuxSlotIndex(uint auxIndex)
    {
        return this->GetTypeHandler()->GetPropertyIndexFromAuxSlotIndex(auxIndex);
    }

    BOOL 
    DynamicObject::GetAttributesWithPropertyIndex(PropertyId propertyId, BigPropertyIndex index, PropertyAttributes * attributes)
    {
        return this->GetTypeHandler()->GetAttributesWithPropertyIndex(this, propertyId, index, attributes);
    }

    RecyclerWeakReference<DynamicObject>* DynamicObject::CreateWeakReferenceToSelf()
    {
        Assert(!ThreadContext::IsOnStack(this));
        return GetRecycler()->CreateWeakReferenceHandle(this);
    }

    DynamicObject * 
    DynamicObject::BoxStackInstance(DynamicObject * instance)
    {
        Assert(ThreadContext::IsOnStack(instance));
        // On the stack, the we reserved a pointer before the object as to store the boxed value
        DynamicObject ** boxedInstanceRef = ((DynamicObject **)instance) - 1;
        DynamicObject * boxedInstance = *boxedInstanceRef;
        if (boxedInstance)
        {
            return boxedInstance;
        }

        size_t inlineSlotsSize = instance->GetTypeHandler()->GetInlineSlotsSize();
        if (inlineSlotsSize)
        {
            boxedInstance = RecyclerNewPlusZ(instance->GetRecycler(), inlineSlotsSize, DynamicObject, instance);
        }
        else
        {
            boxedInstance = RecyclerNew(instance->GetRecycler(), DynamicObject, instance);
        }
        
        *boxedInstanceRef = boxedInstance;
        return boxedInstance;
    }

#ifdef RECYCLER_STRESS
    void DynamicObject::Finalize(bool isShutdown)
    {
        // If -RecyclerTrackStress is enabled, DynamicObject will be allocated as Track (and thus Finalize too).
        // Just ignore this.
        if (Js::Configuration::Global.flags.RecyclerTrackStress)
        {
            return;
        }

        RecyclableObject::Finalize(isShutdown);
    }

    void DynamicObject::Dispose(bool isShutdown)
    { 
        // If -RecyclerTrackStress is enabled, DynamicObject will be allocated as Track (and thus Finalize too).
        // Just ignore this.
        if (Js::Configuration::Global.flags.RecyclerTrackStress)
        {
            return;
        }

        RecyclableObject::Dispose(isShutdown);
    }
    
    void DynamicObject::Mark(Recycler *recycler)
    { 
        // If -RecyclerTrackStress is enabled, DynamicObject will be allocated as Track (and thus Finalize too).
        // Process the mark now.
        
        if (Js::Configuration::Global.flags.RecyclerTrackStress)
        {
            size_t inlineSlotsSize = this->GetDynamicType()->GetTypeHandler()->GetInlineSlotsSize();
            size_t objectSize = sizeof(DynamicObject) + inlineSlotsSize;
            void ** obj = (void **)this;
            void ** objEnd = obj + (objectSize / sizeof(void *));
            
            do
            {
                recycler->TryMarkNonInterior(*obj, null);
                obj++;
            } while (obj != objEnd);

            return;
        }

        RecyclableObject::Mark(recycler);
    }
#endif

} // namespace Js
