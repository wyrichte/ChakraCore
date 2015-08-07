//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#if _M_AMD64 || defined(_M_ARM32_OR_ARM64)
Declare_Extern_ClassMethodImpl(ArrayAsVector, IndexOf);
Declare_Extern_ClassMethodImpl(ArrayAsVector, SetAt);
Declare_Extern_ClassMethodImpl(ArrayAsVector, InsertAt); 
Declare_Extern_ClassMethodImpl(ArrayAsVector, Append);
#endif 

// Description: Represents a marshaled Javascript array as a IVector in JavaScript
namespace Projection
{
    Declare_InspectableImpl_Extern_VTable(g_VectorVtable);
    Declare_InspectableImpl_Extern_VTable(g_VectorViewVtable);

    // *******************************************************
    // Represents a projection of an Array as IVector Methods
    // *******************************************************
    class ArrayAsVector sealed : public CUnknownImpl
    {
    private:
        Js::JavascriptArray *m_pUnderlyingArray;
        
        bool m_fReadOnly; // whether this one is just a VectorView(ReadOnly) or Vector(Read/Write)

        IID m_iidIterable;
        ArrayAsIterable *m_pIterable;
        ArrayAsVector *m_pVectorView;

        RtRUNTIMEINTERFACECONSTRUCTOR iterable;
        RtRUNTIMEINTERFACECONSTRUCTOR vector;
        RtCONCRETETYPE elementType;

        size_t sizeOnStackOfElement;

        MetadataStringId indexOfId;
        MetadataStringId setAtId;
        MetadataStringId insertAtId;
        MetadataStringId appendId;
        MetadataStringId replaceAllId;

    private:
        ArrayAsVector(ProjectionContext *projectionContext, bool fReadOnly);
        virtual ~ArrayAsVector();

        HRESULT Initialize(
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR vector);

        IID GetIterableGuid();
        HRESULT GetIterable(void **ppv);

        HRESULT SupportsWrite() { return (m_fReadOnly) ? E_UNEXPECTED : S_OK; }

    public:
        static HRESULT Create(
            __in ProjectionContext *projectionContext, 
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR vector, 
            __in bool fReadOnly,
            __out ArrayAsVector **newArrayAsVector);

        //
        // IUnknown members
        //
        CUnknownMethodNoError_Def(QueryInterface, REFIID riid, void **ppv);

        //
        // IInspectable Methods
        //
        CUnknownMethodNoError_Def(GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);

        //
        // IVector members
        //
#pragma warning(push)
#pragma warning(disable:28202)
#pragma warning(disable:28285)
        CUnknownMethodImpl_Def(ArrayAsVector, GetAt, __in unsigned index, __out_bcount(elementType->storageSize) byte *returnValue);
        CUnknownMethodImpl_Def(ArrayAsVector, GetMany, __in unsigned int startIndex, __in unsigned int capacity, __out_bcount(elementType->storageSize * capacity) byte *items, __RPC__out unsigned int *actual);
        CUnknownMethodImpl_Def(ArrayAsVector, ReplaceAll, __in unsigned int count, __out_bcount(elementType->storageSize * count) byte *value);
#pragma warning(pop)
        CUnknownMethodImpl_Def(ArrayAsVector, get_Size, __out unsigned *returnValue);
        CUnknownMethodImpl_Def(ArrayAsVector, GetView, __deref_out_opt IUnknown **returnValue);
        CUnknownMethodImpl_ArgT_Def(ArrayAsVector, IndexOf);
        CUnknownMethodImpl_ArgT_Def(ArrayAsVector, SetAt);
        CUnknownMethodImpl_ArgT_Def(ArrayAsVector, InsertAt); 
        CUnknownMethodImpl_Def(ArrayAsVector, RemoveAt, __in unsigned index);
        CUnknownMethodImpl_ArgT_Def(ArrayAsVector, Append);
        CUnknownMethodImpl_NoArgs_Def(ArrayAsVector, RemoveAtEnd);
        CUnknownMethodImpl_NoArgs_Def(ArrayAsVector, Clear);

        void MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect) override;
        virtual void MarkForClose() override
        {
            if (m_pIterable != nullptr)
            {
                m_pIterable->MarkForClose();
            }

            __super::MarkForClose();
        }

        virtual USHORT GetWinrtTypeFlags() override;
        UINT GetHeapObjectRelationshipInfoSize() override;
        void FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo) override;
    };
};