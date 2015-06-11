//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    __inline Var DynamicTypeHandler::GetSlot(DynamicObject * instance, int index)
    {
        if (index < inlineSlotCapacity)
        {
            Var * slots = reinterpret_cast<Var*>(reinterpret_cast<size_t>(instance) + offsetOfInlineSlots);
            Var value = slots[index];
            Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
            return value;
        }
        else
        {
            Var value = instance->auxSlots[index - inlineSlotCapacity];
            Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
            return value;            
        }
    }

    __inline Var DynamicTypeHandler::GetInlineSlot(DynamicObject * instance, int index)
    {
        AssertMsg(index >= (int)(offsetOfInlineSlots / sizeof(Var)), "index should be relative to the address of the object");
        Assert(index - (int)(offsetOfInlineSlots / sizeof(Var)) < this->GetInlineSlotCapacity());
        Var * slots = reinterpret_cast<Var*>(instance);
        Var value = slots[index];
        Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
        return value;
    }

    __inline Var DynamicTypeHandler::GetAuxSlot(DynamicObject * instance, int index)
    {
        // We should only assign stack value only to an stack object (current mark temp number in mark temp object)
        
        Assert(index < GetSlotCapacity() - GetInlineSlotCapacity());
        Var value = instance->auxSlots[index];
        Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
        return value;
    }

#if DBG
    __inline void DynamicTypeHandler::SetSlot(DynamicObject* instance, PropertyId propertyId, bool allowLetConst, int index, Var value)
#else
    __inline void DynamicTypeHandler::SetSlot(DynamicObject* instance, int index, Var value)
#endif
    {
        Assert(index < GetSlotCapacity());
        Assert(propertyId == Constants::NoProperty || CanStorePropertyValueDirectly(instance, propertyId, allowLetConst));
        SetSlotUnchecked(instance, index, value);
    }

    __inline void DynamicTypeHandler::SetSlotUnchecked(DynamicObject * instance, int index, Var value)
    {
        // We should only assign stack value only to an stack object (current mark temp number in mark temp object)
        Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
        uint16 inlineSlotCapacity = instance->GetTypeHandler()->GetInlineSlotCapacity();
        uint16 offsetOfInlineSlots = instance->GetTypeHandler()->GetOffsetOfInlineSlots();
        int slotCapacity = instance->GetTypeHandler()->GetSlotCapacity();

        if (index < inlineSlotCapacity)
        {
            Var * slots = reinterpret_cast<Var*>(reinterpret_cast<size_t>(instance) + offsetOfInlineSlots);
            slots[index] = value;
        }
        else
        {
            Assert((index - inlineSlotCapacity) < (slotCapacity - inlineSlotCapacity));
            instance->auxSlots[index - inlineSlotCapacity] = value;
        }
    }

#if DBG
    __inline void DynamicTypeHandler::SetInlineSlot(DynamicObject* instance, PropertyId propertyId, bool allowLetConst, int index, Var value)
#else
    __inline void DynamicTypeHandler::SetInlineSlot(DynamicObject* instance, int index, Var value)
#endif
    {
        // We should only assign stack value only to an stack object (current mark temp number in mark temp object)
        Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
        AssertMsg(index >= (int)(offsetOfInlineSlots / sizeof(Var)), "index should be relative to the address of the object");
        Assert(index - (int)(offsetOfInlineSlots / sizeof(Var)) < this->GetInlineSlotCapacity());
        Assert(propertyId == Constants::NoProperty || CanStorePropertyValueDirectly(instance, propertyId, allowLetConst));
        Var * slots = reinterpret_cast<Var*>(instance);
        slots[index] = value;
    }

#if DBG
    __inline void DynamicTypeHandler::SetAuxSlot(DynamicObject* instance, PropertyId propertyId, bool allowLetConst, int index, Var value)
#else
    __inline void DynamicTypeHandler::SetAuxSlot(DynamicObject* instance, int index, Var value)
#endif
    {
        // We should only assign stack value only to an stack object (current mark temp number in mark temp object)
        Assert(ThreadContext::IsOnStack(instance) || !ThreadContext::IsOnStack(value) || TaggedNumber::Is(value));
        Assert(index < GetSlotCapacity() - GetInlineSlotCapacity());
        Assert(propertyId == Constants::NoProperty || CanStorePropertyValueDirectly(instance, propertyId, allowLetConst));
        instance->auxSlots[index] = value;
    }
}
