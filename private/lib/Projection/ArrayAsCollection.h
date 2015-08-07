//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Helper methods for array as collection
namespace Projection
{
    class ProjectionContext;
    class NamespaceProjection;

    // *******************************************************
    // Represents a projection of an Array as Collection helper methods
    // *******************************************************
    class ArrayAsCollection
    {
    public:
        static uint GetLength(Js::JavascriptArray *pArray);
        static void SetLength(Js::JavascriptArray *pArray, uint length);

        static bool IsArrayInstance(Var objectVar)
        {
            return Js::JavascriptArray::Is(objectVar) || Js::ES5Array::Is(objectVar);
        }

        static HRESULT GetAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index, __out Var *returnValue);
        static HRESULT GetAt(__in ProjectionContext *projectionContext, RtCONCRETETYPE elementType, __in Js::JavascriptArray *pArray, __in unsigned index, __out_bcount(elementType->storageSize) byte *returnValue);
        static HRESULT SetAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index, __in Var varValue, bool fAllowAppend = false);
        static HRESULT InsertAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index, __in Var varValue);
        static HRESULT Append(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in Var varValue);
        static HRESULT RemoveAt(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in unsigned index);
        static HRESULT RemoveAtEnd(__in Js::JavascriptArray *pArray);
        static HRESULT Clear(__in Js::JavascriptArray *pArray);
        static HRESULT IndexOf(__in ProjectionContext *projectionContext, __in Js::JavascriptArray *pArray, __in_opt Var varValue, __out unsigned *index, __out boolean *found);
        static HRESULT GetMany(__in ProjectionContext *projectionContext, RtCONCRETETYPE elementType, __in Js::JavascriptArray *pArray, __in unsigned int startIndex, __in unsigned int capacity, __out_bcount(elementType->storageSize * capacity) byte *items, __RPC__out unsigned int *actual);
        static HRESULT ReplaceAll(__in ProjectionContext *projectionContext, RtCONCRETETYPE elementType, __in Js::JavascriptArray *pArray, __in unsigned int count, __in_bcount(elementType->storageSize * count) byte *value, MetadataStringId replaceAllId);
    };
};