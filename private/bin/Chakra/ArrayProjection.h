//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

namespace Projection
{
    class ArrayObjectInstance;

    // *******************************************************
    // Represents a projection of an ABIType_Array in JavaScript
    // *******************************************************
    class ArrayProjection sealed : public ProjectionTypeOperations
    {
        friend class ProjectionMarshaler;
    private:
        RtCONCRETETYPE elementType;
        ProjectionContext* projectionContext;
        HTYPE m_hTypeRef;
        MetadataStringId getItemAtId;

        ArrayProjection(ProjectionContext* projectionContext, RtCONCRETETYPE elementType);
        ~ArrayProjection();
        HRESULT Initialize();

    public:
        static HRESULT EnsureProjection(
            __in RtCONCRETETYPE elementType,
            __in ProjectionContext* projectionContext, 
            __out ArrayProjection** ppNewInstance);

        static HRESULT CreateArrayProjectionObject(
            __in RtCONCRETETYPE elementType, 
            __in ProjectionContext* projectionContext, 
            __in_xcount(length * elementType->storageSize) byte* pArrayBlockPointer,
            __in uint length,
            __in uint32 readArrayLength,
            __in bool fOwnBuffer,
            __out Var* newInstance,
            __in bool validContents);

        static HRESULT CreateArrayProjectionObject(
            __in RtCONCRETETYPE elementType, 
            __in ProjectionContext* projectionContext, 
            __in_xcount(length * elementType->storageSize) byte* pArrayBlockPointer,
            __in uint length,
            __in bool fOwnBuffer,
            __out Var* newInstance)
        {
            return ArrayProjection::CreateArrayProjectionObject(elementType, projectionContext, pArrayBlockPointer, length, length, fOwnBuffer, newInstance, true);
        }

        HTYPE GetTypeRef() {return m_hTypeRef; }
        virtual ProjectionType GetProjectionType() override {return ArrayProjectionType;}
        static BOOL Is(Var instance);
        static ArrayObjectInstance *GetArrayObjectInstance(Var instance);
        // TODO: support all types using typed array.
        static BOOL SupportTypedArray(RtCONCRETETYPE elementType)
        {
            Assert(!(BasicType::Is(elementType) && BasicType::From(elementType)->typeCor <= ELEMENT_TYPE_VOID));
            return BasicType::Is(elementType) && BasicType::From(elementType)->typeCor <= ELEMENT_TYPE_R8;
        }

        static BOOL NeedConversion(RtCONCRETETYPE elementType, Js::Var typedArray);

#if DBG_DUMP
        static LPCWSTR TypedArrayName(Js::PFNCreateTypedArray createTypedArrayFunc);
        static LPCWSTR TypedArrayName(Js::Var typedArray);
#endif

        Js::ScriptContext * GetScriptContext() { return projectionContext->GetScriptContext(); }
        PropertyId GetPropertyId() { return elementType->fullTypeNameId; }

        uint32 GetLength(__in Var instance);
        HRESULT GetNumericOwnProperty(__in Var instance, __in uint32 index, __out Var *value, __out BOOL *result, bool fIndexByVar = false, Var indexVar = nullptr);
        HRESULT HasNumericOwnProperty(__in Var instance, __in uint32 index, __out BOOL *result, bool fIndexByVar = false, Var indexVar = nullptr);
        HRESULT SetNumericOwnProperty(__in Var instance, __in uint32 index, __out Var value, __out BOOL *result, bool fIndexByVar = false, Var indexVar = nullptr);
        HRESULT DeleteNumericOwnProperty(__in Var instance, __in uint32 index, __out BOOL *result, bool fIndexByVar = false, Var indexVar = nullptr);

        // *** Overrides of DefaultScriptOperations
        virtual HRESULT STDMETHODCALLTYPE HasOwnProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE GetOwnProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var *value,
            /* [out] */ BOOL *propertyPresent) override;

        virtual HRESULT STDMETHODCALLTYPE GetPropertyReference(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var *value,
            /* [out] */ BOOL *propertyPresent) override;

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE DeleteProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE GetOwnItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ Var *value,
            /* [out] */ BOOL *itemPresent) override;

        virtual HRESULT STDMETHODCALLTYPE SetItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [in] */ Var value,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE DeleteItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE GetEnumerator(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ BOOL enumNonEnumerable,
            /* [in] */ BOOL enumSymbols,
            /* [out] */ IVarEnumerator **enumerator) override;

        virtual HRESULT STDMETHODCALLTYPE IsEnumerable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE IsWritable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE IsConfigurable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE SetEnumerable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value) override;

        virtual HRESULT STDMETHODCALLTYPE SetWritable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value) override;

        virtual HRESULT STDMETHODCALLTYPE SetConfigurable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value) override;

        virtual HRESULT STDMETHODCALLTYPE GetAccessors(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var* getter,
            /* [out] */ Var* setter,
            /* [out] */ BOOL* result) override;

        virtual HRESULT STDMETHODCALLTYPE SetAccessors(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var getter,
            /* [in] */ Var setter) override;

        virtual HRESULT STDMETHODCALLTYPE GetSetter( 
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags) override;

        virtual HRESULT STDMETHODCALLTYPE GetHeapObjectInfo(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance, 
            /* [in] */ ProfilerHeapObjectInfoFlags flags,
            /* [out] */ HostProfilerHeapObject** result,
            /* [out] */ HeapObjectInfoReturnResult* returnResult) override;

        virtual HRESULT STDMETHODCALLTYPE HasOwnItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance, 
            /* [in] */ Var index,
            /* [out] */ BOOL* result) override;

        virtual HRESULT STDMETHODCALLTYPE GetItemSetter(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index, 
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags) override;

        virtual HRESULT STDMETHODCALLTYPE SetPropertyWithAttributes(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ PropertyAttributes attributes,
            /* [in] */ SideEffects effects, 
            /* [out] */ BOOL* result) override;
    };

    class ArrayObjectInstance : public Js::CustomExternalObject
    {
        friend class ProjectionMarshaler;

    public:
        static HRESULT Create(__in HTYPE hTypeRef, __in MetadataStringId getItemAtId, __in RtCONCRETETYPE elementType, __in byte* pArrayBlockPointer, __in uint length, __in ProjectionContext *projectionContext, __out ArrayObjectInstance** ppNewArrayInstance);
        static HRESULT GetItemAt(__in_xcount(length * elementType->storageSize) byte *pArrayBlockPointer, __in uint length, __in ProjectionContext *projectionContext, __in RtCONCRETETYPE elementType, __in uint32 index, __in MetadataStringId getItemAtId, __out Var *value, __out BOOL *result);
        static HRESULT SetItemAt(__in_xcount(length * elementType->storageSize) byte *pArrayBlockPointer, __in uint length, __in ProjectionContext *projectionContext, __in RtCONCRETETYPE elementType, __in uint32 index, __in Var value, __in bool fReleaseExistingItem, __out BOOL *result);
        
        HRESULT GetItemAt(__in ProjectionContext *projectionContext, __in uint32 index, __out Var *value, __out BOOL *result, bool fIndexByVar, Var indexVar);
        HRESULT SetItemAt(__in ProjectionContext *projectionContext, __in uint32 index, __in Var value, __out BOOL *result, bool fIndexByVar, Var indexVar);

        HRESULT GetFullHeapObjectInfo(HostProfilerHeapObject** result, HeapObjectInfoReturnResult* returnResult);

        uint32 GetLength() { return finalizableTypedArrayContents->numberOfElements; }
        byte *GetArrayBlock() { return finalizableTypedArrayContents->typedArrayBuffer; }
        PropertyId GetPropertyId() { return finalizableTypedArrayContents->elementType->fullTypeNameId; }

    protected:
        DEFINE_VTABLE_CTOR(ArrayObjectInstance, Js::CustomExternalObject);

    private:
        ArrayObjectInstance(FinalizableTypedArrayContents *finalizableTypedArrayContents, HTYPE hTypeRef, MetadataStringId getItemAtId);
        FinalizableTypedArrayContents *finalizableTypedArrayContents;
        MetadataStringId getItemAtId;
    };

};