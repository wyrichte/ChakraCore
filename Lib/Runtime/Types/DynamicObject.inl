//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
#ifdef RECYCLER_STRESS
    // Only enable RecyclerTrackStress on DynamicObject
    template <class T> bool IsRecyclerTrackStressType() { return false; }
    template <> inline bool IsRecyclerTrackStressType<DynamicObject>() { return true; }
#endif

    template <class T>
    __inline T * DynamicObject::NewObject(Recycler * recycler, DynamicType * type)
    {
        size_t inlineSlotsSize = type->GetTypeHandler()->GetInlineSlotsSize();
        if (inlineSlotsSize)
        {
#ifdef RECYCLER_STRESS
            if (Js::Configuration::Global.flags.RecyclerTrackStress && IsRecyclerTrackStressType<T>())
            {
                return RecyclerNewTrackedLeafPlusZ(recycler, inlineSlotsSize, T, type);
            }            
#endif
            return RecyclerNewPlusZ(recycler, inlineSlotsSize, T, type);
        }
        else
        {
#ifdef RECYCLER_STRESS
            if (Js::Configuration::Global.flags.RecyclerTrackStress && IsRecyclerTrackStressType<T>())
            {
                return RecyclerNewTrackedLeaf(recycler, T, type);
            }            
#endif
            return RecyclerNew(recycler, T, type);
        }
    }

    __inline DynamicObject * DynamicObject::New(Recycler * recycler, DynamicType * type)
    {
        return NewObject<DynamicObject>(recycler, type);
    }

    inline bool DynamicObject::Is(Var aValue)
    {
        return RecyclableObject::Is(aValue) && (RecyclableObject::FromVar(aValue)->GetTypeId() == TypeIds_Object);
    }

    inline DynamicObject* DynamicObject::FromVar(Var aValue)
    {
//        AssertMsg(Is(aValue), "Ensure var is actually a 'DynamicObject'");
        RecyclableObject* obj = RecyclableObject::FromVar(aValue);
        AssertMsg(obj->DbgIsDynamicObject(), "Ensure instance is actually a DynamicObject");
        Assert(DynamicType::Is(obj->GetTypeId()));
        return static_cast<DynamicObject*>(obj);
    }

    inline ArrayObject* DynamicObject::EnsureObjectArray()
    {
        if (!HasObjectArray())
        {
            ScriptContext* scriptContext = GetScriptContext();
            ArrayObject* objArray = scriptContext->GetLibrary()->CreateArray(0, SparseArraySegmentBase::SMALL_CHUNK_SIZE);
            SetObjectArray(objArray);
        }
        Assert(HasObjectArray());
        return GetObjectArrayOrFlagsAsArray();
    }

    inline void DynamicObject::SetObjectArray(ArrayObject* objArray)
    {
        Assert(!IsAnyArray(this));

        DeoptimizeObjectHeaderInlining();

        this->objectArray = objArray;
        if (objArray)
        {
            if (!this->IsExtensible()) // sync objectArray isExtensible
            {
                objArray->PreventExtensions();
            }

            // sync objectArray is prototype
            if ((this->GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag) != 0)
            {
                objArray->SetIsPrototype();
            }
        }
    }

    inline bool DynamicObject::HasNonEmptyObjectArray() const 
    { 
        return HasObjectArray() && GetObjectArrayOrFlagsAsArray()->GetLength() > 0; 
    }

    // Check if a typeId is of any array type (JavascriptArray or ES5Array).
    inline bool DynamicObject::IsAnyArrayTypeId(TypeId typeId)
    {
        return JavascriptArray::Is(typeId) || typeId == TypeIds_ES5Array;
    }

    // Check if a Var is either a JavascriptArray* or ES5Array*.
    inline bool DynamicObject::IsAnyArray(const Var aValue)
    {
        return IsAnyArrayTypeId(JavascriptOperators::GetTypeId(aValue));
    }

    // Check if this instance is of any array type, used for debug
    inline bool DynamicObject::IsArrayInstance()
    {
        return JavascriptArray::Is(this) || ES5Array::Is(this);
    }

    inline BOOL DynamicObject::HasObjectArrayItem(uint32 index)
    {
        return HasObjectArray() && GetObjectArrayOrFlagsAsArray()->HasItem(index);
    }

    inline BOOL DynamicObject::DeleteObjectArrayItem(uint32 index, PropertyOperationFlags flags)
    {
        if(HasObjectArray())
        {
            return GetObjectArrayOrFlagsAsArray()->DeleteItem(index, flags);
        }
        return true;
    }

    inline BOOL DynamicObject::GetObjectArrayItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return HasObjectArray() && GetObjectArrayOrFlagsAsArray()->GetItem(originalInstance, index, value, requestContext);
    }

    inline DescriptorFlags DynamicObject::GetObjectArrayItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext)
    {
        return HasObjectArray() ? GetObjectArrayOrFlagsAsArray()->GetItemSetter(index, setterValue, requestContext) : None;
    }

    inline BOOL DynamicObject::SetObjectArrayItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        const auto result = EnsureObjectArray()->SetItem(index, value, flags);

        // We don't track non-enumerable items in object arrays.  Any object with an object array reports having 
        // enumerable properties.  See comment in DynamicObject::GetHasNoEnumerableProperties.
        //SetHasNoEnumerableProperties(false);

        return result;
    }

    inline BOOL DynamicObject::SetObjectArrayItemWithAttributes(uint32 index, Var value, PropertyAttributes attributes)
    {
        const auto result = EnsureObjectArray()->SetItemWithAttributes(index, value, attributes);

        // We don't track non-enumerable items in object arrays.  Any object with an object array reports having 
        // enumerable properties.  See comment in DynamicObject::GetHasNoEnumerableProperties.
        //if (attributes & PropertyEnumerable)
        //{
        //    SetHasNoEnumerableProperties(false);
        //}

        if(!(attributes & PropertyWritable) && result)
        {
            InvalidateHasOnlyWritableDataPropertiesInPrototypeChainCacheIfPrototype();
        }
        return result;
    }

    inline BOOL DynamicObject::SetObjectArrayItemAttributes(uint32 index, PropertyAttributes attributes)
    {
        const auto result = HasObjectArray() && GetObjectArrayOrFlagsAsArray()->SetItemAttributes(index, attributes);

        // We don't track non-enumerable items in object arrays.  Any object with an object array reports having 
        // enumerable properties.  See comment in DynamicObject::GetHasNoEnumerableProperties.
        //if (attributes & PropertyEnumerable)
        //{
        //    SetHasNoEnumerableProperties(false);
        //}

        if(!(attributes & PropertyWritable) && result)
        {
            InvalidateHasOnlyWritableDataPropertiesInPrototypeChainCacheIfPrototype();
        }
        return result;
    }

    inline BOOL DynamicObject::SetObjectArrayItemWritable(PropertyId propertyId, BOOL writable)
    {
        const auto result = HasObjectArray() && GetObjectArrayOrFlagsAsArray()->SetWritable(propertyId, writable);
        if(!writable && result)
        {
            InvalidateHasOnlyWritableDataPropertiesInPrototypeChainCacheIfPrototype();
        }
        return result;
    }

    inline BOOL DynamicObject::SetObjectArrayItemAccessors(uint32 index, Var getter, Var setter)
    {
        const auto result = EnsureObjectArray()->SetItemAccessors(index, getter, setter);
        if(result)
        {
            InvalidateHasOnlyWritableDataPropertiesInPrototypeChainCacheIfPrototype();
        }
        return result;
    }

    inline void DynamicObject::InvalidateHasOnlyWritableDataPropertiesInPrototypeChainCacheIfPrototype()
    {
        if(GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag)
        {
            // No need to invalidate store field caches for non-writable properties here.  We're dealing
            // with numeric properties only, and we never cache these in add property inline caches.

            // If this object is used as a prototype, the has-only-writable-data-properties-in-prototype-chain cache needs to be
            // invalidated here since the type handler of 'objectArray' is not marked as being used as a prototype
            GetType()->GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
        }
    }

    inline bool DynamicObject::HasLockedType() const 
    {
        return this->GetDynamicType()->GetIsLocked();
    }

    inline bool DynamicObject::HasSharedType() const 
    {
        return this->GetDynamicType()->GetIsShared();
    }

    inline bool DynamicObject::HasSharedTypeHandler() const 
    {
        return this->GetTypeHandler()->GetIsShared();
    }

    inline void DynamicObject::ReplaceType(DynamicType * type)
    {
        Assert(!type->isLocked || type->GetTypeHandler()->GetIsLocked());
        Assert(!type->isShared || type->GetTypeHandler()->GetIsShared());

        //For now, i have added only Aux Slot -> so new inlineSlotCapacity should be 2.
        AssertMsg(DynamicObject::IsTypeHandlerCompatibleForObjectHeaderInlining(this->GetTypeHandler(), type->GetTypeHandler()),
            "Object is ObjectHeaderInlined and should have compatible TypeHandlers for proper transition");
	
        this->type = type;
    }
    
    inline DWORD DynamicObject::GetOffsetOfAuxSlots()
    {
        return offsetof(DynamicObject, auxSlots);
    }

    inline DWORD DynamicObject::GetOffsetOfObjectArray()
    {
        return offsetof(DynamicObject, objectArray);
    }

    inline DWORD DynamicObject::GetOffsetOfType()
    {
        return offsetof(DynamicObject, type);
    }

    __inline void DynamicObject::EnsureSlots(int oldCount, int newCount, ScriptContext * scriptContext, DynamicTypeHandler * newTypeHandler)
    {
        this->GetTypeHandler()->EnsureSlots(this, oldCount, newCount, scriptContext, newTypeHandler);
    }

    __inline void DynamicObject::EnsureSlots(int newCount, ScriptContext * scriptContext)
    {
        EnsureSlots(GetTypeHandler()->GetSlotCapacity(), newCount, scriptContext);
    }
    
    __inline Var DynamicObject::GetSlot(int index)
    {
        return this->GetTypeHandler()->GetSlot(this, index);
    }

    __inline Var DynamicObject::GetInlineSlot(int index)
    {
        return this->GetTypeHandler()->GetInlineSlot(this, index);
    }

    __inline Var DynamicObject::GetAuxSlot(int index)
    {
        return this->GetTypeHandler()->GetAuxSlot(this, index);
    }

#if DBG
    __inline void DynamicObject::SetSlot(PropertyId propertyId, bool allowLetConst, int index, Var value)
    {
        this->GetTypeHandler()->SetSlot(this, propertyId, allowLetConst, index, value);
    }

    __inline void DynamicObject::SetInlineSlot(PropertyId propertyId, bool allowLetConst, int index, Var value)
    {
        this->GetTypeHandler()->SetInlineSlot(this, propertyId, allowLetConst, index, value);
    }

    __inline void DynamicObject::SetAuxSlot(PropertyId propertyId, bool allowLetConst, int index, Var value)
    {
        this->GetTypeHandler()->SetAuxSlot(this, propertyId, allowLetConst, index, value);
    }
#else
    __inline void DynamicObject::SetSlot(int index, Var value)
    {
        this->GetTypeHandler()->SetSlot(this, index, value);
    }

    __inline void DynamicObject::SetInlineSlot(int index, Var value)
    {
        this->GetTypeHandler()->SetInlineSlot(this, index, value);
    }

    __inline void DynamicObject::SetAuxSlot(int index, Var value)
    {
        this->GetTypeHandler()->SetAuxSlot(this, index, value);
    }
#endif
}
