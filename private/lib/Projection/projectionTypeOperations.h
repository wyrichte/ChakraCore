//---------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Base interface for ITypeOperations in projection. This is used
// to provide projection specified information that is not available in default ITypeOperations.
// Use a private IID to idenitify the internal ITypeOperations.


// Projection type operations code paths are always called from the engine, and can safely assume a caller.
// This should be used instead of BEGIN_JS_RUNTIME_CALL_EX in all projection type operation cases.
#define BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext) \
        BEGIN_ENTER_SCRIPT(scriptContext, /*doCleanup*/false, /*isCallRoot*/false, /*hasCaller*/true) \
        {


namespace Projection
{
    enum ProjectionType
    {
        NamespaceProjectionType,
        ArrayProjectionType,
        InspectableProjectionType
    };

    const IID IID_IProjectionTypeOperations = { /* b6842441-1bd3-4927-bdb8-6fddeb33cf91 */
        0xb6842441,
        0x1bd3,
        0x4927,
        {0xbd, 0xb8, 0x6f, 0xdd, 0xeb, 0x33, 0xcf, 0x91}
    };

    bool IsWinRTType(Js::CustomExternalObject *ceo);
    bool IsWinRTConstructorFunction(Js::RecyclableObject *ro);

    ITypeOperations *GetDefaultTypeOperations();

    class ProjectionTypeOperations : public ITypeOperations
    {
        static OperationUsage projectionTypeOperationUsage;

    public:
        ProjectionTypeOperations();
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)  ;

        virtual ULONG STDMETHODCALLTYPE AddRef( void) override;

        virtual ULONG STDMETHODCALLTYPE Release() override sealed;
        virtual ProjectionType GetProjectionType() = 0;

        // *** Overrides of DefaultScriptOperations
        virtual HRESULT STDMETHODCALLTYPE GetOperationUsage(
            /* [out] */ OperationUsage *flags) override;

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

        virtual HRESULT STDMETHODCALLTYPE Equals(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var other,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE StrictEquals(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var other,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE QueryObjectInterface(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ REFIID riid,
            /* [out] */ void **ppvObj) override;

        virtual HRESULT STDMETHODCALLTYPE GetInitializer(
            /* [out] */ InitializeMethod * initializer,
            /* [out] */ int * initSlotCapacity,
            /* [out] */ BOOL * hasAccessors) override;

        virtual HRESULT STDMETHODCALLTYPE GetFinalizer(
            /* [out] */ FinalizeMethod * finalizer) override;

        virtual HRESULT STDMETHODCALLTYPE HasInstance(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var constructor,
            /* [in] */ Var instance,
            /* [out] */ BOOL* result) override;

        virtual HRESULT STDMETHODCALLTYPE GetNamespaceParent(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [out] */ Var* namespaceParent) override;

        virtual HRESULT STDMETHODCALLTYPE CrossDomainCheck(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [out] */ BOOL* result) override;

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

    private:
        ULONG refCount;
    };
}
