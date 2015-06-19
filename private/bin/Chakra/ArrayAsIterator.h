//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents a marshaled Javascript array as a IIterator in JavaScript

namespace Projection
{
    class ProjectionContext;

    Declare_InspectableImpl_Extern_VTable(g_IteratorVtable);

    // *******************************************************
    // Represents a projection of an Array as IIterator Methods
    // *******************************************************
    class ArrayAsIterator sealed : public CUnknownImpl
    {
    private:
        Js::JavascriptArray *m_pUnderlyingArray;

        uint m_uCurrentIndex;
        RtRUNTIMEINTERFACECONSTRUCTOR iterator;
        RtCONCRETETYPE elementType;

    private:
        ArrayAsIterator(ProjectionContext *projectionContext);
        virtual ~ArrayAsIterator();

        HRESULT Initialize(
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterator
            );

    public:
        static HRESULT Create(
            __in ProjectionContext *projectionContext, 
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterator,
            __out ArrayAsIterator **newArrayAsIterator);

        //
        // IIterator members
        //
#pragma warning(push)
#pragma warning(disable:28202)
#pragma warning(disable:28285)
        CUnknownMethodImpl_Def(ArrayAsIterator, get_Current, __out_bcount(elementType->storageSize) byte * current);
        CUnknownMethodImpl_Def(ArrayAsIterator, GetMany, __in unsigned int capacity, __out_bcount(elementType->storageSize * capacity) byte *items, __RPC__out unsigned int *actual);
#pragma warning(pop)
        CUnknownMethodImpl_Def(ArrayAsIterator, get_HasCurrent, __RPC__out boolean *hasCurrent);
        CUnknownMethodImpl_Def(ArrayAsIterator, MoveNext, __RPC__out boolean *hasCurrent);

        void MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect) override;

        virtual USHORT GetWinrtTypeFlags() override;
        UINT GetHeapObjectRelationshipInfoSize() override;
        void FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo) override;
    };
};