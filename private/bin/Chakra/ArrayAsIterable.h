//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents a marshaled Javascript array as a IVectorView in JavaScript

namespace Projection
{
    class ProjectionContext;
    class ArrayAsIterator;

    Declare_InspectableImpl_Extern_VTable(g_IterableVtable);

    // *******************************************************
    // Represents a projection of an Array as IIterable Methods
    // *******************************************************
    class ArrayAsIterable sealed : public CUnknownImpl
    {
        friend class ArrayAsVector;

    private:
        Js::JavascriptArray *m_pUnderlyingArray;

        IID m_iidVectorOrView;
        ArrayAsVector *m_pVectorOrView;

        RtRUNTIMEINTERFACECONSTRUCTOR iterable;
        RtRUNTIMEINTERFACECONSTRUCTOR iterator;
    private:
        ArrayAsIterable(ProjectionContext *projectionContext);
        virtual ~ArrayAsIterable();

        HRESULT Initialize(
            __in Js::JavascriptArray *pUnderlyingArray,
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterable, 
            __in_opt IID iidVectorOrView = GUID_NULL, 
            __in_opt ArrayAsVector *pVectorOrView = NULL);

        bool IsVectorOrView() { return m_pVectorOrView != NULL; }

    public:
        static HRESULT Create(
            __in ProjectionContext *projectionContext, 
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterable,
            __out ArrayAsIterable **newArrayAsIterable,
            __in_opt IID iidVectorOrView = GUID_NULL, 
            __in_opt ArrayAsVector *pVectorOrView = NULL);

        //
        // IUnknown members
        //
        CUnknownMethodNoError_Def(QueryInterface, REFIID riid, void **ppv);
        CUnknownMethod_ULONGReturn_Def(AddRef);
        CUnknownMethod_ULONGReturn_Def(Release);

        //
        // IInspectable Methods
        //
        CUnknownMethodNoError_Def(GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);

        //
        // IIterable members
        //
        CUnknownMethodImpl_Def(ArrayAsIterable, First, __out IUnknown **first);

        void MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect) override;

        virtual USHORT GetWinrtTypeFlags() override;
        UINT GetHeapObjectRelationshipInfoSize() override;
        void FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo) override;
    };
};