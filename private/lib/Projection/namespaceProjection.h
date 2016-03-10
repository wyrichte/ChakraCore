//---------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

namespace Projection
{
    //****************************************************************************
    // Represents a projection of a namespace in JavaScript
    //****************************************************************************
    /*
    NamespaceProjection represent a namespace imported from WinRT. there is one instance of NamespaceProjection per
    sub namespace. The projection is the base for ITypeOperation for objects created from the namespace. 
    Each instance of NamespaceProject contain an instance of TypeFActoryProject, which contains information about
    the metadata of the interface.Currently there is one interface per factory.
    */
    class NamespaceProjection : public ProjectionTypeOperations
    {
        friend class VectorArray;
    protected:
        BOOL m_isExtensible;           // True if this namespace and its sub-namespaces should be extensible
        LPCWSTR m_pszFullName;         // The full name for this namespace (ie. Windows.Devices)
        ImmutableList<LPCWSTR> *m_directChildren;
        ImmutableList<LPCWSTR> *m_alreadyFullyProjectedPropertiesIfConfigurable;    // tracks all ever-projected properties on the namespace object, when configurable mode is enabled

        // JS Object members
        HTYPE m_type;               // The type for the underlying JS object
        Var m_instance;             // A created instance for the underlying JS object
        ProjectionContext* projectionContext;

    public:
        NamespaceProjection(ProjectionContext* projectionContext, BOOL preLoad);
        ~NamespaceProjection();
        ArenaAllocator *ProjectionAllocator() { return projectionContext->ProjectionAllocator(); }
        ProjectionContext *GetProjectionContext() { return projectionContext; }
        void Initialize(LPCWSTR fullName, LPCWSTR partialName, Var parentOject, bool fEnumerable = true);

        virtual ProjectionType GetProjectionType() override {return NamespaceProjectionType;}

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

        // *** ABI Projector Helper Properties
        HTYPE GetJSType();
        Var GetJSInstance();
        LPCWSTR GetFullName();
        BOOL GetIsExtensible();
        ImmutableList<LPCWSTR> * GetDirectChildren();
        static BOOL Is(Var instance);

    private:
        HRESULT EnsureHasProperty(IActiveScriptDirect* scriptDirect, Var instance, PropertyId propertyId);
        HRESULT AddDirectChildTypesFromMetadata(IUnknown* metadataImport, BOOL isVersioned);
        void ReleaseAll(__in DWORD unkCount, __in_ecount(unkCount) IUnknown ** toRelease);
        HRESULT GetDirectNamespaceChildren();

        void RecordAlreadyFullyProjectedProperties(ImmutableList<LPCWSTR>* names);
        void RecordAlreadyFullyProjectedProperty(LPCWSTR name);
        BOOL IsAlreadyFullyProjectedProperty(LPCWSTR name);

#ifdef PROJECTION_METADATA_TRACE
        void Trace(const char16 *form, ...) const
        {
            if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ProjectionMetadataPhase))
            {
                va_list argptr;
                va_start(argptr, form);
                Output::Print(_u("NamespaceProjection: "));
                Output::VPrint(form, argptr);
                Output::Flush();
            }
        }
#endif
    };

}