//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IVectorView in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    uint ArrayAsCollection::GetLength(Js::JavascriptArray *pArray)
    {
        return pArray->GetLength();
    }

    void ArrayAsCollection::SetLength(Js::JavascriptArray *pArray, uint length)
    {
        pArray->SetLength(length);
    }

    HRESULT ArrayAsCollection::GetAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index, __out Var *returnValue)
    {
        IfNullReturnError(pArray, E_POINTER);
        IfNullReturnError(returnValue, E_POINTER);

        uint length = ArrayAsCollection::GetLength(pArray);
        if (index >= length)
        {
            return E_BOUNDS;
        }

        // Get Item  at index
        Js::ScriptContext *pScriptContext = projectionContext->GetScriptContext();
        Var varIndex = Js::JavascriptNumber::ToVar(index, pScriptContext);
        *returnValue = Js::JavascriptOperators::OP_GetElementI(pArray, varIndex, pScriptContext);

        return S_OK;
    }

    HRESULT ArrayAsCollection::GetAt(__in ProjectionContext *projectionContext, RtCONCRETETYPE elementType, __in Js::JavascriptArray *pArray, __in unsigned index, __out_bcount(elementType->storageSize) byte *returnValue)
    {
        IfNullReturnError(returnValue, E_POINTER);
        Var varItem = nullptr;
        HRESULT hr = ArrayAsCollection::GetAt(projectionContext, pArray, index, &varItem);
        IfFailedReturn(hr);

        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
        marshal.WriteInType(varItem, elementType, returnValue, elementType->storageSize, true);
        return S_OK;
    }

    HRESULT ArrayAsCollection::SetAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index, __in Var varValue, bool fAllowAppend)
    {
        IfNullReturnError(pArray, E_POINTER);

        uint length = ArrayAsCollection::GetLength(pArray);
        if (index > length || (!fAllowAppend && index == length))
        {
            return E_BOUNDS;
        }

        // set Item  at index
        Js::ScriptContext *pScriptContext = projectionContext->GetScriptContext();
        Var varIndex = Js::JavascriptNumber::ToVar(index, pScriptContext);
        BOOL fSet = Js::JavascriptOperators::OP_SetElementI(pArray, varIndex, varValue, pScriptContext);

        return fSet ? S_OK : E_FAIL;
    }

    HRESULT ArrayAsCollection::InsertAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index, __in Var varValue)
    {
        IfNullReturnError(pArray, E_POINTER);

        uint length = ArrayAsCollection::GetLength(pArray);
        if (index >= length)
        {
            return E_BOUNDS;
        }

        for (uint uIndex = length ; uIndex > index; uIndex--)
        {
            Var varItem = nullptr;
            HRESULT hr = ArrayAsCollection::GetAt(projectionContext, pArray, uIndex - 1, &varItem);
            IfFailedReturn(hr);

            hr = ArrayAsCollection::SetAt(projectionContext, pArray, uIndex, varItem, true);
            IfFailedReturn(hr);
        }

        // set Item  at index
        return ArrayAsCollection::SetAt(projectionContext, pArray, index, varValue);
    }

    HRESULT ArrayAsCollection::Append(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in Var varValue)
    {
        IfNullReturnError(pArray, E_POINTER);

        // set Item  at the length
        uint length = ArrayAsCollection::GetLength(pArray);
        return ArrayAsCollection::SetAt(projectionContext, pArray, length, varValue, true);
    }

    HRESULT ArrayAsCollection::RemoveAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index)
    {
        IfNullReturnError(pArray, E_POINTER);

        uint length = ArrayAsCollection::GetLength(pArray);
        if (index >= length || length == 0)
        {
            return E_BOUNDS;
        }

        for (index = index + 1; index < length; index++)
        {
            Var varItem = nullptr;
            HRESULT hr = ArrayAsCollection::GetAt(projectionContext, pArray, index, &varItem);
            IfFailedReturn(hr);

            hr = ArrayAsCollection::SetAt(projectionContext, pArray, index - 1, varItem, true);
            IfFailedReturn(hr);
        }

        // Set length as length - 1
        ArrayAsCollection::SetLength(pArray, length - 1);
        return S_OK;
    }

    HRESULT ArrayAsCollection::RemoveAtEnd(__in Js::JavascriptArray *pArray)
    {
        IfNullReturnError(pArray, E_POINTER);

        uint length = ArrayAsCollection::GetLength(pArray);
        if (length == 0)
        {
            return E_BOUNDS;
        }

        // Set length as length - 1
        ArrayAsCollection::SetLength(pArray, length - 1);
        return S_OK;
    }

    HRESULT ArrayAsCollection::Clear(__in Js::JavascriptArray *pArray)
    {
        IfNullReturnError(pArray, E_POINTER);

        // Set length as length to 0
        ArrayAsCollection::SetLength(pArray, 0);
        return S_OK;
    }

    HRESULT ArrayAsCollection::IndexOf(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in_opt Var varValue, __out unsigned *index, __out boolean *found)
    {
        *found = false;
        *index = UINT_MAX;
        Var element = nullptr;
        bool isSearchTaggedInt = Js::TaggedInt::Is(varValue);

        // If length = 0 the item  is not found return with S_OK
        uint32 length = ArrayAsCollection::GetLength(pArray);
        for (uint32 iIndex = 0; iIndex < length; iIndex++)
        {
            if (!pArray->DirectGetItemAtFull(iIndex, &element))
            {
                continue;
            }

            if (isSearchTaggedInt &&  Js::TaggedInt::Is(element))
            {
                if (element == varValue)
                {
                    *found = true;
                    *index = iIndex;
                    break;
                }
                continue;
            }

            if (Js::JavascriptOperators::StrictEqual(element, varValue, projectionContext->GetScriptContext()))
            {
                *found = true;
                *index = iIndex;
                break;
            }
        }

        // We either found the item  or didnt but in any case its a success.
        return S_OK;
    }

    HRESULT ArrayAsCollection::GetMany(__in ProjectionContext *projectionContext, RtCONCRETETYPE elementType, __in Js::JavascriptArray *pArray, __in unsigned int startIndex, __in unsigned int capacity, __out_bcount(elementType->storageSize * capacity) byte *items, __RPC__out unsigned int *actual)
    {
        IfNullReturnError(pArray, E_POINTER);
        IfNullReturnError(actual, E_POINTER);

        if (capacity > 0)
        {
            IfNullReturnError(items, E_POINTER);
        }
        else
        {
            *actual = 0;
            return S_OK;
        }

        uint32 length = ArrayAsCollection::GetLength(pArray);
        if (startIndex >= length)
        {
            return E_BOUNDS;
        }

        unsigned int copyEndIndex = startIndex + capacity;
        if (copyEndIndex > length)
        {
            copyEndIndex = length;
        }

        // FillArray pattern (filling the out buffer) - release existing
        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
        for (unsigned int index = startIndex; index < copyEndIndex; index++)
        {
            Var varItem = nullptr;
            HRESULT hr = ArrayAsCollection::GetAt(projectionContext, pArray, index, &varItem);
            IfFailedReturn(hr);

            marshal.WriteInType(varItem, elementType, items, elementType->storageSize, true);
            items = items + elementType->storageSize;
        }

        *actual = copyEndIndex - startIndex;
        return S_OK;
    }

    HRESULT ArrayAsCollection::ReplaceAll(__in ProjectionContext *projectionContext, RtCONCRETETYPE elementType, __in Js::JavascriptArray *pArray, __in unsigned int count, __in_bcount(elementType->storageSize * count) byte *value, MetadataStringId replaceAllId)
    {
        IfNullReturnError(pArray, E_POINTER);

        if (count > 0)
        {
            IfNullReturnError(value, E_POINTER);
        }

        HRESULT hr = ArrayAsCollection::Clear(pArray);
        IfFailedReturn(hr);

        // Pass Array (reading from the buffer)
        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);

        for (unsigned int index = 0; index < count; index++)
        {
            Var varItem = marshal.ReadOutType(nullptr, elementType, true, value, elementType->storageSize, replaceAllId);

            hr = ArrayAsCollection::SetAt(projectionContext, pArray, index, varItem, true);
            IfFailedReturn(hr);
            value = value + elementType->storageSize;
        }

        return hr;
    }
}