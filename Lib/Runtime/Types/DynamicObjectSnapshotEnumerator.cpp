//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    JavascriptEnumerator* DynamicObjectSnapshotEnumerator<T, enumNonEnumerable, enumSymbols>::New(ScriptContext* scriptContext, DynamicObject* object)
    {
        DynamicObjectSnapshotEnumerator* enumerator = RecyclerNew(scriptContext->GetRecycler(), DynamicObjectSnapshotEnumerator, scriptContext);
        enumerator->Initialize(object);
        return enumerator;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    Var DynamicObjectSnapshotEnumerator<T, enumNonEnumerable, enumSymbols>::GetCurrentAndMoveNextFromArray(PropertyId& propertyId, PropertyAttributes* attributes)
    {
        if (arrayEnumerator)
        {
            Var currentIndex = arrayEnumerator->GetCurrentAndMoveNext(propertyId, attributes);
            if(currentIndex != NULL)
            {
                return currentIndex;
            }
            arrayEnumerator = NULL;
        }

        return null;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    JavascriptString * DynamicObjectSnapshotEnumerator<T, enumNonEnumerable, enumSymbols>::GetCurrentAndMoveNextFromObject(T& index, PropertyId& propertyId, PropertyAttributes* attributes)
    {
        JavascriptString* propertyString = null;
        auto newIndex = objectIndex;
        do
        {
            newIndex++;
            if (!object->FindNextProperty(newIndex, &propertyString, &propertyId, attributes, GetTypeToEnumerate(), !enumNonEnumerable, /*enumSymbols*/enumSymbols) || newIndex >= initialPropertyCount)
            {
                newIndex--;
                propertyString = null;
                break;
            }
        }
        while (Js::IsInternalPropertyId(propertyId));

        index = newIndex;
        return propertyString;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    Var DynamicObjectSnapshotEnumerator<T, enumNonEnumerable, enumSymbols>::GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes)
    {
        Var currentIndex = GetCurrentAndMoveNextFromArray(propertyId, attributes);
        return (currentIndex != null)? currentIndex :
            this->GetCurrentAndMoveNextFromObject(objectIndex, propertyId, attributes);
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    void DynamicObjectSnapshotEnumerator<T, enumNonEnumerable, enumSymbols>::Reset()
    {
        __super::Reset();
        initialPropertyCount = object->GetPropertyCount();
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols>
    void DynamicObjectSnapshotEnumerator<T, enumNonEnumerable, enumSymbols>::Initialize(DynamicObject* object)
    {
        __super::Initialize(object);
        initialPropertyCount = object->GetPropertyCount();
    }

    template class DynamicObjectSnapshotEnumerator<PropertyIndex, true, true>;
    template class DynamicObjectSnapshotEnumerator<PropertyIndex, true, false>;
    template class DynamicObjectSnapshotEnumerator<PropertyIndex, false, true>;
    template class DynamicObjectSnapshotEnumerator<PropertyIndex, false, false>;
    template class DynamicObjectSnapshotEnumerator<BigPropertyIndex, true, true>;
    template class DynamicObjectSnapshotEnumerator<BigPropertyIndex, true, false>;
    template class DynamicObjectSnapshotEnumerator<BigPropertyIndex, false, true>;
    template class DynamicObjectSnapshotEnumerator<BigPropertyIndex, false, false>;
}
