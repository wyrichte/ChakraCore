//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline RecyclableObject::RecyclableObject(Type * type) : type(type)
    {
#if DBG_EXTRAFIELD
        dtorCalled = false;
#ifdef HEAP_ENUMERATION_VALIDATION
        m_heapEnumValidationCookie = 0;
#endif
#endif
#if DBG || defined(PROFILE_TYPES)
        RecordAllocation(type->GetScriptContext());
#endif
    }

#if INT32VAR
    __inline bool RecyclableObject::Is(Var aValue)
    {
        AssertMsg(aValue != null, "RecyclableObject::Is aValue is null");

        return (((uintptr)aValue) >> VarTag_Shift) == 0;
    }
#else
    __inline bool RecyclableObject::Is(Var aValue)
    {
        AssertMsg(aValue != null, "RecyclableObject::Is aValue is null");

        return (((uintptr) aValue) & AtomTag) == AtomTag_Object;
    }
#endif

    __inline RecyclableObject* RecyclableObject::FromVar(const Js::Var aValue)
    {
        AssertMsg(AtomTag_Object == 0, "Ensure GC objects do not need to be marked");
        AssertMsg(Is(aValue), "Ensure instance is a RecyclableObject");
        AssertMsg(!TaggedNumber::Is(aValue), "Tagged value being used as RecyclableObject");

        return reinterpret_cast<RecyclableObject *>(aValue);
    }

    inline TypeId RecyclableObject::GetTypeId() const 
    { 
        return this->GetType()->GetTypeId(); 
    }

    inline RecyclableObject* RecyclableObject::GetPrototype() const 
    { 
        Type* type = GetType();
        if (!type->HasSpecialPrototype())
        {
            return type->GetPrototype();
        }
        return const_cast<RecyclableObject*>(this)->GetPrototypeSpecial();
    }

    __inline RecyclableObject* RecyclableObject::GetPrototypeSpecial()
    {
        AssertMsg(GetType()->GetTypeId() == TypeIds_Null, "Do not use this function."); 
        return nullptr; 
    }

    inline JavascriptMethod RecyclableObject::GetEntryPoint() const 
    { 
        return this->GetType()->GetEntryPoint(); 
    }

    inline JavascriptLibrary* RecyclableObject::GetLibrary() const 
    { 
        return this->GetType()->GetLibrary(); 
    }

    inline Recycler* RecyclableObject::GetRecycler() const
    {
        return this->GetLibrary()->GetRecycler();
    }

    inline void RecyclableObject::SetIsPrototype() 
    { 
        if (DynamicType::Is(this->GetTypeId()))
        {
            DynamicObject* dynamicThis = DynamicObject::FromVar(this);
            dynamicThis->SetIsPrototype();      // Call the DynamicObject::SetIsPrototype
        }
    }    

    inline bool RecyclableObject::HasOnlyWritableDataProperties()
    {
        if (DynamicType::Is(this->GetTypeId()))
        {
            DynamicObject* obj = DynamicObject::FromVar(this);
            return obj->GetTypeHandler()->GetHasOnlyWritableDataProperties() &&
                (!obj->HasObjectArray() || obj->GetObjectArrayOrFlagsAsArray()->HasOnlyWritableDataProperties());
        }

        return true;
    }

    inline void RecyclableObject::ClearWritableDataOnlyDetectionBit()
    {
        if (DynamicType::Is(this->GetTypeId()))
        {
            DynamicObject* obj = DynamicObject::FromVar(this);
            obj->GetTypeHandler()->ClearWritableDataOnlyDetectionBit();
            if (obj->HasObjectArray())
            {
                obj->GetObjectArrayOrFlagsAsArray()->ClearWritableDataOnlyDetectionBit();
            }
        }
    }

    inline bool RecyclableObject::IsWritableDataOnlyDetectionBitSet()
    {
        if (DynamicType::Is(this->GetTypeId()))
        {
            DynamicObject* obj = DynamicObject::FromVar(this);
            return obj->GetTypeHandler()->IsWritableDataOnlyDetectionBitSet() ||
                (obj->HasObjectArray() && obj->GetObjectArrayOrFlagsAsArray()->IsWritableDataOnlyDetectionBitSet());
        }
        
        return false;
    }

    inline ScriptContext* RecyclableObject::GetScriptContext() const
    {
        return this->GetLibrary()->GetScriptContext();
    }

    inline RecyclableObject* RecyclableObject::GetProxiedObjectForHeapEnum()
    { 
        Assert(this->GetScriptContext()->IsHeapEnumInProgress()); 
        return NULL; 
    }

    inline BOOL RecyclableObject::IsExternal() const
    {
        Assert(this->IsExternalVirtual() == this->GetType()->IsExternal());
        return this->GetType()->IsExternal();
    }

    inline BOOL RecyclableObject::SkipsPrototype() const
    { 
        Assert(this->DbgSkipsPrototype() == this->GetType()->SkipsPrototype());
        return this->GetType()->SkipsPrototype(); 
    }

    inline BOOL RecyclableObject::CanHaveInterceptors() const
    {
#if !defined(USED_IN_STATIC_LIB)
        Assert(this->DbgCanHaveInterceptors() == this->GetType()->CanHaveInterceptors());
#endif
        return this->GetType()->CanHaveInterceptors();
    }
}

