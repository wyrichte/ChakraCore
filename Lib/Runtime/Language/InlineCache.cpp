//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    bool InlineCache::NeedsToBeRegisteredForProtoInvalidation() const
    {
        return (IsProto() || IsGetterAccessorOnProto());
    }

    bool InlineCache::NeedsToBeRegisteredForStoreFieldInvalidation() const
    {
        return (IsLocal() && this->u.local.typeWithoutProperty != null) || IsSetterAccessorOnProto();
    }

#if DEBUG
    bool InlineCache::NeedsToBeRegisteredForInvalidation() const
    {
        int howManyInvalidationsNeeded =
            (int)NeedsToBeRegisteredForProtoInvalidation() +
            (int)NeedsToBeRegisteredForStoreFieldInvalidation();
        Assert(howManyInvalidationsNeeded <= 1);
        return howManyInvalidationsNeeded > 0;
    }

    void InlineCache::VerifyRegistrationForInvalidation(const InlineCache* cache, ScriptContext* scriptContext, Js::PropertyId propertyId)
    {
        bool needsProtoInvalidation = cache->NeedsToBeRegisteredForProtoInvalidation();
        bool needsStoreFieldInvalidation = cache->NeedsToBeRegisteredForStoreFieldInvalidation();
        int howManyInvalidationsNeeded = (int)needsProtoInvalidation + (int)needsStoreFieldInvalidation;
        bool hasListSlotPtr = cache->invalidationListSlotPtr != null;
        bool isProtoRegistered = hasListSlotPtr ? scriptContext->GetThreadContext()->IsProtoInlineCacheRegistered(cache, propertyId) : false;
        bool isStoreFieldRegistered = hasListSlotPtr ? scriptContext->GetThreadContext()->IsStoreFieldInlineCacheRegistered(cache, propertyId) : false;
        int howManyRegistrations = (int)isProtoRegistered + (int)isStoreFieldRegistered;

        Assert(howManyInvalidationsNeeded <= 1);
        Assert((howManyInvalidationsNeeded == 0) || hasListSlotPtr)
        Assert(!needsProtoInvalidation || isProtoRegistered);
        Assert(!needsStoreFieldInvalidation || isStoreFieldRegistered);
        Assert(!hasListSlotPtr || howManyRegistrations > 0);
        Assert(!hasListSlotPtr || (*cache->invalidationListSlotPtr) == cache);
    }

    // Confirm inline cache miss against instance property lookup info.
    bool InlineCache::ConfirmCacheMiss(const Type * oldType, const PropertyValueInfo* info) const
    {
        return u.local.type != oldType
            && u.proto.type != oldType
            && (u.accessor.type != oldType || info == NULL || u.accessor.flags != info->GetFlags());
    }
#endif

#if DBG_DUMP
    void InlineCache::Dump()
    {
        if (this->u.local.isLocal)
        {
            Output::Print(L"LOCAL { types: 0x%X -> 0x%X, slot = %d, list slot ptr = 0x%X }",
                this->u.local.typeWithoutProperty,
                this->u.local.type,
                this->u.local.slotIndex,
                this->invalidationListSlotPtr
                );
        }
        else if (this->u.proto.isProto)
        {
            Output::Print(L"PROTO { type = 0x%X, prototype = 0x%X, slot = %d, list slot ptr = 0x%X }",
                this->u.proto.type,
                this->u.proto.prototypeObject,
                this->u.proto.slotIndex,
                this->invalidationListSlotPtr
                );
        }
        else if (this->u.accessor.isAccessor)
        {
            Output::Print(L"FLAGS { type = 0x%X, object = 0x%X, flag = 0x%X, slot = %d, list slot ptr = 0x%X }",
                this->u.accessor.type,
                this->u.accessor.object,
                this->u.accessor.slotIndex,
                this->u.accessor.flags,
                this->invalidationListSlotPtr
                );
        }
        else
        {
            Assert(this->u.accessor.type == 0);
            Assert(this->u.accessor.slotIndex == 0);
            Output::Print(L"uninitialized");
        }
    }

    void PolymorphicInlineCache::Dump()
    {
        for (uint i = 0; i < size; ++i)
        {
            if (!inlineCaches[i].IsEmpty())
            {
                Output::Print(L"  %d: ", i);
                inlineCaches[i].Dump();
                Output::Print(L"\n");
            }
        }
    }
#endif

    bool EquivalentTypeSet::Contains(const Js::Type * type, uint16* pIndex) const
    {
        for (uint16 ti = 0; ti < this->count; ti++)
        {
            if (this->types[ti] == type)
            {
                if (pIndex)
                {
                    *pIndex = ti;
                }
                return true;
            }
        }
        return false;
    }

    bool EquivalentTypeSet::AreIdentical(EquivalentTypeSet * left, EquivalentTypeSet * right)
    {
        if (!left->GetSortedAndDuplicatesRemoved())
        {
            left->SortAndRemoveDuplicates();
        }
        if (!right->GetSortedAndDuplicatesRemoved())
        {
            right->SortAndRemoveDuplicates();
        }

        Assert(left->GetSortedAndDuplicatesRemoved() && right->GetSortedAndDuplicatesRemoved());

        if (left->count != right->count)
        {
            return false;
        }

        if (memcmp(left->types, right->types, left->count * sizeof(Type*)) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool EquivalentTypeSet::IsSubsetOf(EquivalentTypeSet * left, EquivalentTypeSet * right)
    {
        if (!left->GetSortedAndDuplicatesRemoved())
        {
            left->SortAndRemoveDuplicates();
        }
        if (!right->GetSortedAndDuplicatesRemoved())
        {
            right->SortAndRemoveDuplicates();
        }

        if (left->count > right->count)
        {
            return false;
        }

        // Try to find each left type in the right set.
        int j = 0;
        for (int i = 0; i < left->count; i++)
        {
            bool found = false;
            for (; j < right->count; j++)
            {
                if (left->types[i] < right->types[j])
                {
                    // Didn't find the left type. Fail.
                    return false;
                }
                if (left->types[i] == right->types[j])
                {
                    // Found the left type. Continue to the next left/right pair.
                    found = true;
                    j++;
                    break;
                }
            }
            Assert(j <= right->count);
            if (j == right->count && !found)
            {
                // Exhausted the right set without finding the current left type.
                return false;
            }
        }
        return true;
    }


    void EquivalentTypeSet::SortAndRemoveDuplicates()
    {
        uint16 oldCount = this->count;
        uint16 i;

        // sorting
        for (i = 1; i < oldCount; i++)
        {
            uint16 j = i;
            while (j > 0 && (this->types[j - 1] > this->types[j]))
            {
                Type* tmp = this->types[j];
                this->types[j] = this->types[j - 1];
                this->types[j - 1] = tmp;
            }
        }
        
        // removing duplicate types from the sorted set
        i = 0;
        for (uint16 j = 1; j < oldCount; j++)
        {
            if (this->types[i] != this->types[j])
            {
                this->types[++i] = this->types[j];
            }
        }
        this->count = ++i;
        for (i; i < oldCount; i++)
        {
            this->types[i] = null;
        }

        this->sortedAndDuplicatesRemoved = true;
    }

    ConstructorCache ConstructorCache::DefaultInstance;

#if DBG_DUMP
    void ConstructorCache::Dump() const
    {
        Output::Print(L"guard value or type = 0x%p, script context = 0x%p, pending type = 0x%p, slots = %d, inline slots = %d, populated = %d, polymorphic = %d, update cache = %d, update type = %d, skip default = %d, no return = %d", 
            this->GetRawGuardValue(), this->GetScriptContext(), this->GetPendingType(), this->GetSlotCount(), this->GetInlineSlotCount(), 
            this->IsPopulated(), this->IsPolymorphic(), this->GetUpdateCacheAfterCtor(), this->GetTypeUpdatePending(),
            this->GetSkipDefaultNewObject(), this->GetCtorHasNoExplicitReturnValue());
    }
#endif

}
