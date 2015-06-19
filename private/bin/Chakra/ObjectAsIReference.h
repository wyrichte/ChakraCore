//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents a marshaled Javascript array as a IVectorView in JavaScript

namespace Projection
{
    class ProjectionContext;

    Declare_InspectableImpl_Extern_VTable(g_IReferenceVtable);

    // *******************************************************
    // Represents a projection of an Array as IIterable Methods
    // *******************************************************
    class ObjectAsIReference sealed : public CUnknownImpl
    {
    private:
        bool isArray;
        ObjectAsIPropertyValue *m_pPropertyValue;
        MetadataStringId getValueId;

        FinalizableTypedArrayContents *finalizableTypedArrayContents;

    private:
        ObjectAsIReference(ProjectionContext *projectionContext, PFN_VTABLE_ENTRY *pvtbl);
        virtual ~ObjectAsIReference();

        HRESULT Initialize(
            __in bool isArray,
            __in size_t typeStorageSize,
            __in_bcount(typeStorageSize) byte *typeStorage,
            __in RtCONCRETETYPE elementType);

        HRESULT ObjectAsIReference::GetPropertyValue(void **ppv);

    public:
        static HRESULT CreateIReference(
            __in ProjectionContext *projectionContext, 
            __in size_t typeStorageSize,
            __in_bcount(typeStorageSize) byte *typeStorage,
            __in RtCONCRETETYPE elementType,
            __out ObjectAsIReference **newObjectAsIReference);

        static HRESULT CreateIReferenceArray(
            __in ProjectionContext *projectionContext, 
            __in size_t typeStorageSize,
            __in_bcount(typeStorageSize) byte *typeStorage,
            __in RtCONCRETETYPE elementType,
            __out ObjectAsIReference **newObjectAsIReferenceArray);

        //
        // IUnknown members
        //
        CUnknownMethodNoError_Def(QueryInterface, REFIID riid, void **ppv);

        //
        // IInspectable Methods
        //
        CUnknownMethodNoError_Def(GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);

        //
        // IReference/Array members
        //
#pragma warning(push)
#pragma warning(disable:28285)
#pragma warning(disable:28202)
        CUnknownMethodImpl_Def(ObjectAsIReference, get_ValueIReference, __out_bcount(finalizableTypedArrayContents->elementType->storageSize) byte *value);
        CUnknownMethodImpl_Def(ObjectAsIReference, get_ValueIReferenceArray, __out UINT32 *length, __out_bcount(sizeof(LPVOID)) byte *value);
#pragma warning(pop)

        void MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect) override;
        virtual void MarkForClose() override
        {
            if (m_pPropertyValue != nullptr)
            {
                m_pPropertyValue->MarkForClose();
            }

            __super::MarkForClose();
        }

        HRESULT get_Value(__in size_t valueSize, __in_bcount(valueSize) byte *value, __in RtCONCRETETYPE elementType = nullptr, __in uint32 numberOfElements = 0, __in size_t srcValueSize = 0, __in_bcount_opt(srcValueSize) byte *srcValue = nullptr);

        uint32 GetNumberOfElements() { return finalizableTypedArrayContents->numberOfElements; }
        bool IsElementTypeEnumType() { return finalizableTypedArrayContents->elementType->typeCode == tcEnumType; }
        bool IsArray() { return isArray; }
        LPCWSTR GetFullElementTypeName();

        virtual USHORT GetWinrtTypeFlags() override;
        UINT GetHeapObjectRelationshipInfoSize() override;
        void FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo) override;
    };
};