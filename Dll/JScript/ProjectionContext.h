//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for the ProjectionContext
//  This class is used to hold Projection related global fields
//  that is common in the same script engine. There should be only
//  one instance of ProjectionContext per ScriptEngine, and it's
//  freed when ScriptEngine is deleted. 

#pragma once


namespace Projection
{
    class NamespaceProjection;

    typedef JsUtil::BaseDictionary<LPCWSTR, NamespaceProjection*, ArenaAllocator, PrimeSizePolicy> NAMESPACEMAP;
    typedef JsUtil::BaseDictionary<GUID, Metadata::Assembly*, ArenaAllocator, PrimeSizePolicy> ASSEMBLYMAP;
    typedef JsUtil::BaseDictionary<PropertyId, Metadata::TypeDefProperties*, ArenaAllocator, PowerOf2SizePolicy> TYPEDEFPROPERTIESMAP;
    
    class ProjectionWriter;
    class ProjectionAsyncDebug;

    class ProjectionContext sealed : ProjectionModel::ITypeResolver, Metadata::IStringConverter, IProjectionContext
    {
    private:
        ArenaAllocator* projectionAllocator; 
        ArenaAllocator* tempAlloc;
        IActiveScriptProjectionHost* m_pProjectionHost;
        IActiveScriptProjectionTelemetryHost* m_projectionTelemetryHost;
        BOOL m_isConfigurable;
        BOOL ignoreWebHidden;
        NAMESPACEMAP* m_namespaces;
        ASSEMBLYMAP* metadata;
        TYPEDEFPROPERTIESMAP* typeDefs;
        IDelegateWrapper* m_delegateWrapper;

        ThreadContext *threadContext;
        ScriptSite* scriptSite; 
        Js::ScriptContext* scriptContext;
        ProjectionModel::ProjectionBuilder * builder;
        bool supportsWeakDelegate;
        ProjectionAsyncDebug* m_projectionAsyncDebug;

        ProjectionWriter * writer;

        HRESULT GetTypeFromTypeNameParts(__in DWORD typeNamePartsCount, __in_ecount(typeNamePartsCount) HSTRING *typeNameParts, __in RtTYPE *type, DWORD *readParts);

    public:
        ProjectionContext(ScriptSite* scriptSite, ThreadContext *threadContext);
        ~ProjectionContext();
        HRESULT Initialize();
        HRESULT SetProjectionHost(IActiveScriptProjectionHost * host, BOOL isConfigurable, DWORD targetVersion, IDelegateWrapper* delegateWrapper);
        HRESULT ResetDelegateWrapper(IDelegateWrapper* newDelegateWrapper);
        /* Projection related public methods */
        ArenaAllocator* ProjectionAllocator() const { return projectionAllocator; };
        Js::ScriptContext* GetScriptContext() const { return scriptContext; }
        ScriptSite* GetScriptSite() const { return scriptSite; }
        ThreadContext* GetThreadContext() const { return threadContext; };
        ScriptEngine * GetScriptEngine() const; 
        ProjectionAsyncDebug* GetProjectionAsyncDebug() { return m_projectionAsyncDebug; }
        DWORD GetTargetVersion() const;

        bool SupportsWeakDelegate() const { return supportsWeakDelegate; }
        void DoNotSupportWeakDelegate() { supportsWeakDelegate = false; }

        IDelegateWrapper* GetDelegateWrapper() const { return m_delegateWrapper; }

        // *** ABI Projector Helper Methods ***
        NamespaceProjection * CreateSubNamespace(Var parentObject, LPCWSTR fullName, LPCWSTR partialName, BOOL preLoad);
        void DeleteSubNamespace(_In_z_ LPCWSTR fullName);
        HRESULT ReserveNamespace(LPCWSTR name, BOOL isExtensible);
        void SetTempAlloc(ArenaAllocator* tempAlloc) { this->tempAlloc = tempAlloc; }
        ArenaAllocator* GetTempAlloc() const { return this->tempAlloc; }

        ProjectionWriter * GetProjectionWriter();
        ProjectionWriter * GetExistingProjectionWriter() const { return writer; }
        BOOL AreProjectionPrototypesConfigurable() const { return m_isConfigurable; }

        Js::DelayLoadWinRtRoParameterizedIID* GetRoParameterizedIIDDelayLoad();

        HRESULT GetExprFromConcreteTypeName(HSTRING concreteTypeName, RtEXPR *expr);
        HRESULT GetExpr(MetadataStringId typeId, MetadataStringId fullNameId, LPCWSTR fullName, regex::ImmutableList<RtTYPE> * genericParameters, RtEXPR * expr);

        HRESULT ResolveTypeName(MetadataStringId typeId, LPCWSTR typeName, Metadata::TypeDefProperties ** typeDef) override;

        MetadataStringId IdOfString(LPCWSTR sz) override;
        LPCWSTR StringOfId(MetadataStringId id) override;

        HRESULT CreateMetadataAssembly(IMetaDataImport2* pMetaDataImport, Metadata::Assembly ** ppAssembly, bool isVersioned = false);

        HRESULT TypeInstanceCreated(__in IUnknown *unknown);
        HRESULT CreateTypeFactoryInstance(LPCWSTR typeName, IID factoryID, IUnknown** instance);
        HRESULT GetTypeMetaDataInformation(LPCWSTR typeName, IUnknown** metaDataInformation, DWORD* typeDefToken, BOOL* isVersioned);
        HRESULT GetNamespaceChildren(__in LPCWSTR fullNamespace, __out DWORD* metaDataImportCount, 
            __deref_out_ecount(*metaDataImportCount) IUnknown*** metaDataImport, 
            __out DWORD* childrenNamespacesCount, __deref_out_ecount(*childrenNamespacesCount) LPWSTR** childrenNamespaces,
            __deref_out_ecount(*metaDataImportCount) BOOL** metaDataImportIsVersioned);

        HRESULT MarkForClose();
        HRESULT Close();

        ProjectionModel::ProjectionBuilder * GetProjectionBuilder() const { return builder; }
        Metadata::IStringConverter * GetStringConverter() { return this; }

        BOOL IgnoreWebHidden() const { return ignoreWebHidden; }
        void SetIgnoreWebHidden(BOOL shouldIgnore) { ignoreWebHidden = shouldIgnore; }

        BOOL EnforceAllowForWeb() const { return scriptContext->GetConfig()->GetHostType() == Js::HostTypeWebview; }

        BOOL IsWinRTConstructorAllowed() const { return scriptContext->GetConfig()->IsWinRTConstructorAllowed(); }

        // Pre-computed ids for common string literals
        MetadataStringId indexOfId;
        MetadataStringId setAtId;
        MetadataStringId insertAtId;
        MetadataStringId appendId;
        MetadataStringId replaceAllId;
        MetadataStringId getItemAtId;
        MetadataStringId toStringId;

        void ClearCaches();
        
#ifdef PROJECTION_METADATA_TRACE
        void Trace(const wchar_t *form, ...) const
        {
            if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ProjectionMetadataPhase))
            {
                va_list argptr;
                va_start(argptr, form);
                Output::Print(L"ProjectionContext: ");
                Output::VPrint(form, argptr);
                Output::Flush();
            }
        }
#endif

        void IncrementSQMCount(DWORD dataID, DWORD count);
    };
};
