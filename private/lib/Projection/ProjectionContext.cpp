//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// General projection related context information for the script engine
// *******************************************************


#include "ProjectionPch.h"

#ifdef PROJECTION_METADATA_TRACE
#define TRACE_METADATA(...) { Trace(__VA_ARGS__); }
#else
#define TRACE_METADATA(...)
#endif


namespace Projection
{
    Var ReleaseMethodThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        auto func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptFunction::FromVar(method);
        auto scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(scriptContext->GetThreadContext()->IsScriptActive());   
        
        if(args.Info.Count < 2)
        {
            // If no object provided to release, then return.
            return scriptContext->GetLibrary()->GetUndefined();
        }

        ProjectionObjectInstance *instance = GetProjectionObjectInstanceFromVarNoThrow(args[1]);

        if (instance == nullptr)
        {
            // It isnt projection instance then we dont support this function
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedInspectableObject);
        }

        if (instance->GetUnknown() == nullptr)
        {
            // Already released
            Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_AlreadyReleasedInspectableObject);
        }

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("MemoryTrace: Explicit release of projection object (msReleaseWinRTObject)\n"));
            Output::Flush();
        }
#endif
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            instance->ReleaseNativePointers(false, false);
        }
        END_LEAVE_SCRIPT(scriptContext)
        return scriptContext->GetLibrary()->GetUndefined();
    }

    ProjectionObjectInstance *GetNonDisposedProjectionObjectInstance(Js::ScriptContext *scriptContext, Var arg)
    {
        ProjectionObjectInstance *instance = GetProjectionObjectInstanceFromVarNoThrow(arg);
        if (instance == nullptr)
        {
            // It isnt projection instance then we dont support this function
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedInspectableObject);
        }

        if (instance->GetUnknown() == nullptr)
        {
            // Already released
            Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject);
        }

        return instance;
    }

    Var GetWeakWinRTPropertyThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        auto func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto scriptContext = function->GetScriptContext();
        auto projectionContext = (ProjectionContext *)function->GetSignature();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(scriptContext->GetThreadContext()->IsScriptActive());   
        
        if(args.Info.Count < 3)
        {
            // If not both parameters passed : object from which to get the weak property and propertyName
            Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, _u("msGetWeakWinRTProperty"));
        }

        // Param1
        ProjectionObjectInstance *instance = GetNonDisposedProjectionObjectInstance(scriptContext, args[1]);

        // Param2
        Js::JavascriptString * propertyNameString = Js::JavascriptConversion::ToString(args[2], scriptContext);
        LPCWSTR propertyName = propertyNameString->GetSz();
        const Js::PropertyRecord *propertyNameRecord = nullptr;
        scriptContext->FindPropertyRecord(propertyName, Js::JavascriptString::GetBufferLength(propertyName), &propertyNameRecord);

        // Get the value
        return projectionContext->GetProjectionWriter()->GetWeakWinRTProperty(instance, propertyNameRecord);
    }

    Var SetWeakWinRTPropertyThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        auto func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        auto scriptContext = function->GetScriptContext();
        auto projectionContext = (ProjectionContext *)function->GetSignature();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(scriptContext->GetThreadContext()->IsScriptActive());   
        
        if(args.Info.Count < 4)
        {
            // If not all three parameters passed : object from which to set the weak property, propertyName and actual property value
            Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, _u("msSetWeakWinRTProperty"));
        }

        // Param1
        ProjectionObjectInstance *instance = GetNonDisposedProjectionObjectInstance(scriptContext, args[1]);

        // Param2
        Js::JavascriptString * propertyNameString = Js::JavascriptConversion::ToString(args[2], scriptContext);
        LPCWSTR propertyName = propertyNameString->GetSz();
        const Js::PropertyRecord *propertyNameRecord = nullptr;
        scriptContext->GetOrAddPropertyRecord(propertyName, Js::JavascriptString::GetBufferLength(propertyName), &propertyNameRecord);
        
        // Set the value
        projectionContext->GetProjectionWriter()->SetWeakWinRTProperty(instance, propertyNameRecord, args[3]);

        return scriptContext->GetLibrary()->GetUndefined();
    }

    ProjectionContext::ProjectionContext(ScriptSite* scriptSite, ThreadContext *threadContext):
        m_namespaces(nullptr), 
        metadata(nullptr),
        typeDefs(nullptr),
        m_pProjectionHost(nullptr),
        projectionAllocator(nullptr),
        tempAlloc(nullptr),
        scriptSite(scriptSite),
        threadContext(threadContext),
        builder(nullptr),
        writer(nullptr),
        m_delegateWrapper(nullptr),
        m_projectionTelemetryHost(nullptr),
        m_projectionAsyncDebug(nullptr),
        m_isConfigurable(false)
        ,ignoreWebHidden(false)
    {
        Assert(scriptSite != nullptr);
        scriptSite->AddRef();
        
        // Having this flag in projectionContext, will help us disable this feature easily.
        supportsWeakDelegate = true;

        scriptContext = scriptSite->GetScriptSiteContext();
#if DBG_DUMP
        // Create a new ProjectionMemoryInformation if there isn't already one registered by threadContext.
        IProjectionContextMemoryInfo* memInfo = threadContext->GetProjectionContextMemoryInformation();

        if (!memInfo)
            memInfo = new ProjectionMemoryInformation();

        threadContext->RegisterProjectionMemoryInformation(memInfo);
#endif
    }

    ProjectionContext::~ProjectionContext()
    {
        threadContext->RemoveFromPendingClose(this);

        scriptSite->Release();
        scriptSite = nullptr;

        if (m_delegateWrapper != nullptr)
        {
            m_delegateWrapper->Release();
            m_delegateWrapper = nullptr;
        }

        ClearCaches();

        if (nullptr != m_pProjectionHost)
        {
            m_pProjectionHost->Release();
            m_pProjectionHost = nullptr;
        }


        if (nullptr != projectionAllocator)
        {
            HeapDelete(projectionAllocator);
            projectionAllocator = nullptr;
        }

        if (nullptr != m_projectionTelemetryHost)
        {
            m_projectionTelemetryHost->Release();
            m_projectionTelemetryHost = nullptr;
        }

        m_projectionAsyncDebug = nullptr;
        m_namespaces = nullptr;
        metadata = nullptr;
        typeDefs = nullptr;
        builder = nullptr;
    }
    
    ScriptEngine * ProjectionContext::GetScriptEngine() const 
    {
        return scriptSite->GetScriptEngine();
    }

    void ProjectionContext::ClearCaches()
    {
        if (nullptr != m_namespaces)
        {
            m_namespaces->Clear();
        }

        if (nullptr != metadata)
        {
            metadata->Clear();
        }

        if (nullptr != typeDefs)
        {
            typeDefs->Clear();
        }

        if (nullptr != builder)
        {
            builder->ClearCaches();
        }

    }

    HRESULT ProjectionContext::Initialize()
    {
        HRESULT hr = S_OK;

        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            projectionAllocator = HeapNew(ArenaAllocator, _u("Projection"), threadContext->GetPageAllocator(), Js::Throw::OutOfMemory);
            m_namespaces = Anew(projectionAllocator, NAMESPACEMAP, projectionAllocator);

            PropertyId releaseMethodId = IdOfString(_u("msReleaseWinRTObject"));

            auto releaseMethod = this->CreateWinRTFunction(reinterpret_cast<Js::JavascriptMethod>(ReleaseMethodThunk), releaseMethodId, nullptr, false);
            Var varLength = Js::JavascriptNumber::ToVar(1, scriptContext);
            releaseMethod->SetPropertyWithAttributes(Js::PropertyIds::length, varLength, PropertyNone, NULL, Js::PropertyOperation_None, Js::SideEffects_None);
            scriptContext->GetGlobalObject()->SetPropertyWithAttributes(releaseMethodId, releaseMethod, PropertyBuiltInMethodDefaults, nullptr);

            PropertyId getWeakWinRTPropertyId = IdOfString(_u("msGetWeakWinRTProperty"));
            auto getWeakWinRTProperty = this->CreateWinRTFunction(reinterpret_cast<Js::JavascriptMethod>(GetWeakWinRTPropertyThunk), getWeakWinRTPropertyId, this, false);
            varLength = Js::JavascriptNumber::ToVar(2, scriptContext);
            getWeakWinRTProperty->SetPropertyWithAttributes(Js::PropertyIds::length, varLength, PropertyNone, NULL, Js::PropertyOperation_None, Js::SideEffects_None);
            scriptContext->GetGlobalObject()->SetPropertyWithAttributes(getWeakWinRTPropertyId, getWeakWinRTProperty, PropertyBuiltInMethodDefaults, nullptr);

            PropertyId setWeakWinRTPropertyId = IdOfString(_u("msSetWeakWinRTProperty"));
            auto setWeakWinRTProperty = this->CreateWinRTFunction(reinterpret_cast<Js::JavascriptMethod>(SetWeakWinRTPropertyThunk), setWeakWinRTPropertyId, this, false);
            varLength = Js::JavascriptNumber::ToVar(3, scriptContext);
            setWeakWinRTProperty->SetPropertyWithAttributes(Js::PropertyIds::length, varLength, PropertyNone, NULL, Js::PropertyOperation_None, Js::SideEffects_None);
            scriptContext->GetGlobalObject()->SetPropertyWithAttributes(setWeakWinRTPropertyId, setWeakWinRTProperty, PropertyBuiltInMethodDefaults, nullptr);

            m_projectionAsyncDebug = Anew(projectionAllocator, ProjectionAsyncDebug, this);

            getItemAtId = IdOfString(_u("getItemAt"));
            indexOfId = IdOfString(_u("indexOf"));
            setAtId = IdOfString(_u("setAt"));
            insertAtId = IdOfString(_u("insertAt"));
            appendId = IdOfString(_u("append"));
            replaceAllId = IdOfString(_u("replaceAll"));
            toStringId = IdOfString(_u("toString"));

            projectionExternalLibrary = RecyclerNew(threadContext->GetRecycler(), ProjectionExternalLibrary);
            projectionExternalLibrary->Initialize(scriptContext->GetLibrary());

            TRACE_METADATA(_u("WinRT Projections initialized\n"));
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr)
        return hr;
    }

    HRESULT ProjectionContext::ArrayBufferFromExternalObject(__in Js::RecyclableObject *obj,
        __out Js::ArrayBuffer **ppArrayBuffer)
    {
        return ProjectionObjectInstance::ArrayBufferFromExternalObjectDispatch(obj, ppArrayBuffer);
    }

    ProjectionWriter * ProjectionContext::GetProjectionWriter()
    {
        if (nullptr == writer)
        {
            writer = Anew(projectionAllocator, ProjectionWriter, this);
        }
        return writer;
    }

    Js::DelayLoadWinRtRoParameterizedIID * ProjectionContext::GetRoParameterizedIIDDelayLoad()
    {
        return threadContext->GetWinRTRoParameterizedIIDLibrary();
    }

    HRESULT ProjectionContext::CreateMetadataAssembly(IMetaDataImport2* pMetaDataImport, Metadata::Assembly ** ppAssembly, bool isVersioned)
    {
        IfNullReturnError(ppAssembly, E_INVALIDARG);

        EnsureProjectionBuilder();

        if (metadata == nullptr)
        {
            metadata = Anew(projectionAllocator, ASSEMBLYMAP, projectionAllocator);
        }

        char16 assemblyName[MAX_PATH];
        DWORD assemblyNameLength;
        GUID   assemblyGuid;

        HRESULT hr = pMetaDataImport->GetScopeProps(assemblyName, _countof(assemblyName), &assemblyNameLength, &assemblyGuid);
        IfFailedReturn(hr);

        if(!metadata->TryGetValue(assemblyGuid, ppAssembly))
        {
            *ppAssembly = Anew(projectionAllocator, Metadata::Assembly, pMetaDataImport, this, projectionAllocator, isVersioned);
            metadata->Add(assemblyGuid, *ppAssembly);

            TRACE_METADATA(_u("Metadata loaded for assembly %s\n"), assemblyName); 
        }
        else
        {
            TRACE_METADATA(_u("Assembly %s already has metadata loaded\n"), assemblyName);
        }

        return hr;
    }
    
    HRESULT ProjectionContext::GetExprFromConcreteTypeName(HSTRING concreteTypeName, RtEXPR *expr)
    {
        Assert(concreteTypeName != NULL);
        Assert(expr != NULL);

        EnsureProjectionBuilder();

        UINT32 length;
        Js::DelayLoadWinRtString *winrtStringDelayLoad = threadContext->GetWinRTStringLibrary();
        PCWSTR passedInTypeName = winrtStringDelayLoad->WindowsGetStringRawBuffer(concreteTypeName, &length);
        auto typeId = IdOfString(passedInTypeName);
        *expr = builder->TryGetExprOfTypeId(typeId);
        if(*expr)
        {
            return S_OK;
        }

        JS_ETW(EventWriteJSCRIPT_PROJECTION_GETEXPRFROMCONCRETETYPENAME_START(passedInTypeName));

        DWORD typeNamePartsCount = 0;
        HSTRING *typeNameParts = nullptr;
        HRESULT hr = threadContext->GetWinRTTypeResolutionLibrary()->RoParseTypeName(concreteTypeName, &typeNamePartsCount, &typeNameParts);
        IfFailedReturn(hr);
        Assert(typeNamePartsCount > 0);

        // First Part Name is the fullName and rest are the instantiation types for the parameterized type
        ImmutableList<RtTYPE> *genericParameters = ImmutableList<RtTYPE>::Empty();
        auto tail = genericParameters;
        for (DWORD index = 1; index < typeNamePartsCount; )
        {
            RtTYPE type = nullptr;
            DWORD readParts = 0;
            // This would make sure we release all the HSTRINGs as well.
            hr = GetTypeFromTypeNameParts(typeNamePartsCount - index, typeNameParts + index, &type, &readParts);
            if (FAILED(hr))
            {
                // Release the remaining HSTRINGs and free the array before returning
                for (DWORD i = index; i < typeNamePartsCount; i++)
                {
                    winrtStringDelayLoad->WindowsDeleteString(typeNameParts[i]);
                }
                winrtStringDelayLoad->WindowsDeleteString(typeNameParts[0]);
                CoTaskMemFree(typeNameParts);
                return hr;
            }
            
            // Release all read HSTRINGs
            while (readParts != 0)
            {
                __analysis_assume(index + readParts < typeNamePartsCount);
                winrtStringDelayLoad->WindowsDeleteString(typeNameParts[index]);
                index++;
                readParts--;
            }

            genericParameters = genericParameters->Append(type, projectionAllocator, &tail);
        }

        PCWSTR fullTypeName = winrtStringDelayLoad->WindowsGetStringRawBuffer(typeNameParts[0], &length);
        hr = GetExpr(typeId, IdOfString(fullTypeName), fullTypeName, genericParameters, expr);
        winrtStringDelayLoad->WindowsDeleteString(typeNameParts[0]);

        CoTaskMemFree(typeNameParts);

        JS_ETW(EventWriteJSCRIPT_PROJECTION_GETEXPRFROMCONCRETETYPENAME_STOP(passedInTypeName));

        return hr;
    }

 
    HRESULT ProjectionContext::GetTypeFromTypeNameParts(__in DWORD typeNamePartsCount, __in_ecount(typeNamePartsCount) HSTRING *typeNameParts, __in RtTYPE *type, DWORD *readParts)
    {
        Assert(threadContext->IsScriptActive());

        Assert(type != nullptr);
        Assert(readParts != nullptr);
        Assert(typeNamePartsCount > 0);
        Assert(typeNameParts != nullptr);

        UINT32 length;
        PCWSTR fullTypeName = threadContext->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(typeNameParts[0], &length);
        *readParts = 1;

        JS_ETW(EventWriteJSCRIPT_PROJECTION_GETTYPEFROMTYPENAMEPARTS_START(fullTypeName));

        // Check for basic and known types
        *type = builder->GetBasicAndKnownTypeByName(fullTypeName);
        if (*type)
        {
            return S_OK;
        }

        Metadata::TypeDefProperties * typeDef = nullptr;
        HRESULT hr = ResolveTypeName(IdOfString(fullTypeName), fullTypeName, &typeDef);
        IfFailedReturn(hr);

        ImmutableList<RtTYPE> *genericParameters = ImmutableList<RtTYPE>::Empty();
        auto tail = genericParameters;
        for (size_t index = 0; index < typeDef->genericParameterTokens->Count(); index++)
        {
            DWORD genericParameterTypeReadPartsCount = 0;
            RtTYPE genericParameterType = nullptr;
            // Read the generic types
            hr = GetTypeFromTypeNameParts(typeNamePartsCount - *readParts, typeNameParts + *readParts, &genericParameterType, &genericParameterTypeReadPartsCount);
            IfFailedReturn(hr);

            genericParameters = genericParameters->Append(genericParameterType, projectionAllocator, &tail);
            *readParts = *readParts + genericParameterTypeReadPartsCount;
        }

        *type = builder->TypeOfToken(typeDef->td, typeDef->assembly, genericParameters, false);
        JS_ETW(EventWriteJSCRIPT_PROJECTION_GETTYPEFROMTYPENAMEPARTS_STOP(fullTypeName));

        return S_OK;
    }

    void ProjectionContext::EnsureProjectionBuilder()
    {
        if (builder != nullptr)
        {
            return;
        }

        builder = Anew(
            projectionAllocator,
            ProjectionModel::ProjectionBuilder,
            this,
            this,
            projectionAllocator,
            this->GetTargetVersion(),
            scriptContext->GetConfig()->IsWinRTAdaptiveAppsEnabled());
        builder->SetIgnoreWebHidden(IgnoreWebHidden());
        builder->SetEnforceAllowForWeb(EnforceAllowForWeb());
    }

    HRESULT ProjectionContext::GetExpr(MetadataStringId typeId, MetadataStringId fullNameId, LPCWSTR fullName, ImmutableList<RtTYPE> * genericParameters, RtEXPR* expr)
    {
        Assert(threadContext->IsScriptActive());
        Assert(expr != NULL);
        Assert(fullNameId != MetadataStringIdNil);

        Metadata::TypeDefProperties * typeDef = nullptr;
        HRESULT hr = ResolveTypeName(fullNameId, fullName, &typeDef);
        IfFailedReturn(hr);

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_RESOLVETYPE_START())
        {
            LPCWSTR fullNameString = fullName? fullName : StringOfId(fullNameId);
            EventWriteJSCRIPT_PROJECTION_RESOLVETYPE_START(fullNameString);
        }
#endif
        *expr = builder->ExprOfToken(typeId, typeDef->td, typeDef->assembly, genericParameters);

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_RESOLVETYPE_STOP())
        {
            LPCWSTR fullNameString = fullName? fullName : StringOfId(fullNameId);
            EventWriteJSCRIPT_PROJECTION_RESOLVETYPE_STOP(fullNameString);
        }
#endif
        return S_OK;
    }

    // ABI Projector Helper Methods
    NamespaceProjection * ProjectionContext::CreateSubNamespace(Var parentObject, LPCWSTR fullName, LPCWSTR partialName, BOOL isExtensible)
    {
        Recycler* recycler = scriptContext->GetRecycler();

        // Create the projection for the sub namespace
        NamespaceProjection *namespaceProjection = RecyclerNew(recycler, NamespaceProjection, this, isExtensible);
        namespaceProjection->Initialize(fullName, partialName, parentObject);

        // Make a copy of final namespace
        m_namespaces->Add(namespaceProjection->GetFullName(), namespaceProjection);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_NAMESPACE_OBJECT(namespaceProjection->GetJSInstance(), fullName));

        return namespaceProjection;
    }

    void ProjectionContext::DeleteSubNamespace(_In_z_ LPCWSTR fullName)
    {
        if (m_namespaces != nullptr)
        {
            TRACE_METADATA(_u("ProjectionContext::DeleteSubNamespace(%s) invoked - m_namespaces->Count() = %d\n"), fullName, m_namespaces->Count());

            m_namespaces->Remove(fullName);

            TRACE_METADATA(_u("ProjectionContext::DeleteSubNamespace(%s) completed - m_namespaces->Count() = %d\n"), fullName, m_namespaces->Count());
        }
        else
        {
            TRACE_METADATA(_u("ProjectionContext::DeleteSubNamespace(%s) no-op - m_namespaces == nullptr\n"), fullName);
        }
    }

    HRESULT ProjectionContext::SetProjectionHost(IActiveScriptProjectionHost * host, BOOL isConfigurable, DWORD targetVersion, IDelegateWrapper* delegateWrapper)
    {
        host->AddRef();
        m_pProjectionHost = host;
        m_isConfigurable = isConfigurable;
        TRACE_METADATA(_u("ProjectionContext::SetProjectionHost(): set m_isConfigurable = %d\n"), m_isConfigurable);
        m_delegateWrapper = delegateWrapper;
        if (m_delegateWrapper != nullptr)
        {
            m_delegateWrapper->AddRef();
        }

        // optional; we just don't do telemetry logging if the host doesn't support this interface.
        HRESULT hrIgnore;
        hrIgnore = m_pProjectionHost->QueryInterface(__uuidof(IActiveScriptProjectionTelemetryHost), (void**)&m_projectionTelemetryHost);
#if DBG_DUMP
        if (FAILED(hrIgnore) && (hrIgnore != E_NOINTERFACE))
        {
            if (Js::Configuration::Global.flags.TraceProjection <= TraceLevel_Info)
            {
                Output::Print(_u("Failed to get IActiveScriptProjectionTelemetryHost from host\n"));
                Output::Flush();
            }
        }
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::TargetWinRTVersionFlag))
        {
            switch(Js::Configuration::Global.flags.TargetWinRTVersion)
            {
            case 0:
                // Version 0
                targetVersion = 0;
                break;
            case 2:
                // Version NTDDI_MOCK_WIN8SP1
                targetVersion = 0x06020100;
                break;
            case 3:
                // Version NTDDI_MOCK_WIN9 || NTDDI_WINBLUE
                targetVersion = 0x06030000;
                break;
            case 4:
                // Version NTDDI_MAX
                targetVersion = 0xFFFFFFFE;
                break;
            default:
                // Version NTDDI_WIN8
                targetVersion = 0x06020000;
                break;
            }
        }
#endif

        this->scriptContext->SetProjectionTargetVersion(targetVersion);

        TRACE_METADATA(_u("Setting TargetWinRTVersion as 0x%08X\n"), targetVersion);

        return S_OK;
    }

    HRESULT ProjectionContext::ResetDelegateWrapper(IDelegateWrapper* newDelegateWrapper)
    {
        if (m_delegateWrapper != nullptr)
        {
            m_delegateWrapper->Release();
        }

        m_delegateWrapper = newDelegateWrapper;

        if (m_delegateWrapper != nullptr)
        {
            m_delegateWrapper->AddRef();
        }

        return S_OK;
    }

    DWORD ProjectionContext::GetTargetVersion() const 
    { 
        Assert(scriptContext->GetConfig()->GetProjectionConfig());

        return scriptContext->GetConfig()->GetProjectionConfig()->GetTargetVersion();
    }

    HRESULT ProjectionContext::ReserveNamespace(LPCWSTR name, BOOL isExtensible)
    {
        HRESULT hr = NOERROR;
        IfNullReturnError(name, E_POINTER);

        // check - if design mode - make all namespaces extensible! (even Microsoft.PlayReady)
        if (AreProjectionPrototypesConfigurable())
        {
            // override
            isExtensible = TRUE;
        }
        
        TRACE_METADATA(_u("ProjectionContext::ReserveNamespace(): %s: isExtensible = %d %s\n"), name, isExtensible, AreProjectionPrototypesConfigurable() == TRUE ? _u("(DESIGN MODE)") : _u(""));

        // TODO - make sure we don't begin or end with a _u('.')
        if (m_namespaces->ContainsKey(name))
        {
            return E_INVALIDARG;
        }

        Var parentNamespaceObject = scriptContext->GetGlobalObject();

        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false) 
        {
            // Parse the name to ensure that all parent namespaces have been created
            // TODO: use StringBuilder.
            // Space holder to store parent namespaces and eventually the full name of this namespace;
            AutoHeapString finalNamespace;
            finalNamespace.CreateNew(wcslen(name)+1);
            wcscpy_s(finalNamespace.Get(), finalNamespace.GetLength(), name);

            LPWSTR nextValue = NULL;
            LPWSTR currentNamespace = wcstok_s(finalNamespace.Get(), _u("."), &nextValue);

            while(NULL != currentNamespace)
            {
                // See if the projection has already been created for this namespace;
                NamespaceProjection* parentNamespaceProjection = NULL;
                if (!m_namespaces->TryGetValue(finalNamespace.Get(), &parentNamespaceProjection))
                {
                    // We couldn't find the match, so create the projection for this namespace
                    parentNamespaceProjection = CreateSubNamespace(parentNamespaceObject, finalNamespace.Get(), currentNamespace, isExtensible);

                    // TODO - what do we do if we fail to create a NamespaceProjection
                }

                // The newly created parent namespace is now the current parent
                parentNamespaceObject = parentNamespaceProjection->GetJSInstance();

                currentNamespace = wcstok_s(NULL, _u("."), &nextValue);

                // Append a "." to our name if there is more to add to our namespace
                if (currentNamespace != nullptr)
                {
                    // Not root space
                    Assert(currentNamespace - finalNamespace.Get() > 0);
                    *(currentNamespace - 1) = _u('.');
                }
            }
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
        return hr;
    }

    HRESULT ProjectionContext::ResolveTypeName(MetadataStringId typeId, LPCWSTR typeName, Metadata::TypeDefProperties ** typeDef)
    {
        Assert(typeId != MetadataStringIdNil);
        Assert(typeName == nullptr || IdOfString(typeName) == typeId);        
        Assert(threadContext->IsScriptActive());
        
        if (typeDefs == nullptr)
        {
            typeDefs = Anew(projectionAllocator, TYPEDEFPROPERTIESMAP, projectionAllocator);
        }

        // Look in our name->typeDef cache
        if (typeDefs->TryGetValue(typeId, typeDef))
        {
            // If there's an entry in the cache but it's null, that means the type doesn't exist.
            if (*typeDef == nullptr)
            {
                return RO_E_METADATA_NAME_NOT_FOUND;
            }

            return S_OK;
        }
        
        LPCWSTR typeDefName = typeName ? typeName : StringOfId(typeId);

        HRESULT hr = S_OK;
        Metadata::Assembly *assembly = nullptr;
        CComPtr<IUnknown> info = nullptr;
        //
        // Before we happily run off and look for our metadata using the file system, attempt to look through
        // the current assemblies.
        //
        bool found = false;
        DWORD typeDefToken = mdTokenNil;

        if (metadata)
        {         
            int mapCount = metadata->Count();

            for (int i = 0; i < mapCount && !found; i++)
            {
                assembly = metadata->GetValueAt(i);

                typeDefToken = assembly->TryGetTypeByName(typeDefName);
                if (typeDefToken != mdTokenNil)
                {
                    found = true;
                    break;
                }
                
            }
        }
        
        if (!found)
        {
            BOOL isVersioned;
            JS_ETW(EventWriteJSCRIPT_PROJECTION_GETTYPEMETADATAINFORMATION_START(typeDefName));
            hr = GetTypeMetaDataInformation(typeDefName, &info, &typeDefToken, &isVersioned);
            JS_ETW(EventWriteJSCRIPT_PROJECTION_GETTYPEMETADATAINFORMATION_STOP(typeDefName));

            // If GetTypeMetaDataInformation told us it's neither a type nor a namespace,
            // cache that info by adding a cache entry with value == null.
            if (hr == RO_E_METADATA_NAME_NOT_FOUND)
            {
                typeDefs->Add(typeId, nullptr);
                return hr;
            }

            // Any other failure (including RO_E_METADATA_NAME_IS_NAMESPACE), return now and don't cache.
            IfFailedReturn(hr);

            CComPtr<IMetaDataImport2> import;
            hr = info->QueryInterface(IID_IMetaDataImport2, (void**)&import);
            IfFailedReturn(hr);
            hr = CreateMetadataAssembly(import, &assembly, (isVersioned == TRUE));
            IfFailedReturn(hr);
        }

        
        *typeDef = const_cast<Metadata::TypeDefProperties *>(assembly->GetTypeDefProperties(typeDefToken));
        typeDefs->Add(typeId, *typeDef);
        
        return S_OK;
    }

    // Info:        Get the projection string id for a given string.
    // Parameter:   sz - the string
    // Return:      The id of the string
    MetadataStringId ProjectionContext::IdOfString(LPCWSTR sz) 
    {
        return Projection::IdOfString(scriptContext, sz);
    }

    // Info:        Get the string for a given string id.
    // Parameter:   id - the id
    // Return:      The string
    LPCWSTR ProjectionContext::StringOfId(MetadataStringId id)
    {
        return Projection::StringOfId(scriptContext, id);
    }

    HRESULT ProjectionContext::TypeInstanceCreated(__in IUnknown *unknown)
    {
        HRESULT hr = E_FAIL;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = m_pProjectionHost->TypeInstanceCreated(unknown);
        }
        END_LEAVE_SCRIPT(scriptContext);

        return hr;
    }

    HRESULT ProjectionContext::CreateTypeFactoryInstance(LPCWSTR typeName, IID factoryID, IUnknown** instance)
    {
        HRESULT hr = E_FAIL;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = m_pProjectionHost->CreateTypeFactoryInstance(typeName, factoryID, instance);
        }
        END_LEAVE_SCRIPT(scriptContext);

        return hr;
    }

    HRESULT ProjectionContext::GetTypeMetaDataInformation(LPCWSTR typeName, IUnknown** metaDataInformation, DWORD* typeDefToken, BOOL* isVersioned)
    {
        HRESULT hr = E_FAIL;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = m_pProjectionHost->GetTypeMetaDataInformation(typeName, metaDataInformation, typeDefToken, isVersioned);

            TRACE_METADATA(_u("GetTypeMetadataInformation(\"%s\"), result: %d\n"), typeName, *typeDefToken);            
        }
        END_LEAVE_SCRIPT(scriptContext);

        return hr;
    }

    HRESULT ProjectionContext::GetNamespaceChildren(LPCWSTR fullNamespace, DWORD* metaDataImportCount, IUnknown*** metaDataImport, DWORD* childrenNamespacesCount, LPWSTR** childrenNamespaces, BOOL** metaDataImportIsVersioned)
    {
        Assert(!threadContext->IsScriptActive());
        return m_pProjectionHost->GetNamespaceChildren(fullNamespace, metaDataImportCount, metaDataImport, childrenNamespacesCount, childrenNamespaces, metaDataImportIsVersioned);
    }

    HRESULT ProjectionContext::MarkForClose()
    {
        HRESULT hr = S_OK;
        if (!threadContext->IsInScript())
        {
            hr = Close();
            IfFailedReturn(hr);
        }
        else
        {
            if (writer != nullptr)
            {
                writer->MarkForClose();
            }
            threadContext->AddToPendingProjectionContextCloseList(this);
        }

        return hr;
    }

    HRESULT ProjectionContext::Close()
    {
        HRESULT hr = S_OK;

        // Notify the ProjectionWriter to close
        if (writer)
        {
            hr = writer->Close();
            IfFailedReturn(hr);

            writer = nullptr;
        }

#if DBG_DUMP
        threadContext->DumpProjectionContextMemoryStats(_u("Stats after ProjectionContextClose"));
#endif

        Assert(!threadContext->IsScriptActive());
        hr = m_pProjectionHost->Close();
        HeapDelete(this);
        return hr;
    }

    Js::JavascriptWinRTFunction * 
    ProjectionContext::CreateWinRTFunction(Js::JavascriptMethod entryPoint, PropertyId nameId, Var signature, bool fConstructor)
    {
        Recycler * recycler = this->scriptContext->GetRecycler();
        auto functionInfo = RecyclerNew(recycler, Js::WinRTFunctionInfo, entryPoint);
        Js::JavascriptWinRTFunction *function = nullptr;
        Js::DynamicType * type = this->scriptContext->GetLibrary()->CreateDeferredPrototypeFunctionType(entryPoint);
        if (fConstructor)
        {
            function = RecyclerNewEnumClass(recycler, Js::JavascriptLibrary::EnumFunctionClass, Js::JavascriptWinRTConstructorFunction, type, functionInfo, signature);
        }
        else
        {
            function = RecyclerNewEnumClass(recycler, Js::JavascriptLibrary::EnumFunctionClass, Js::JavascriptWinRTFunction, type, functionInfo, signature);
        }

        function->SetFunctionNameId(Js::TaggedInt::ToVarUnchecked(nameId));
        return function;
    }

    void ProjectionContext::IncrementSQMCount(DWORD dataID, DWORD count)
    {
        // TODO: I don't want to start a new SQM model just for one method yet. We should have
        // a separate SQM class if we need to support different provider.
        if (nullptr != m_projectionTelemetryHost)
        {
            HRESULT hr;
            hr = m_projectionTelemetryHost->TelemetryIncrement(dataID, count);
            Assert(SUCCEEDED(hr));
        }
    }
}

#ifdef ENABLE_PROJECTION
// *** Implementation of ScriptEngine's IActiveScriptProjection Methods ***
STDMETHODIMP ScriptEngine::SetProjectionHost(IActiveScriptProjectionHost * host, BOOL isConfigurable, DWORD targetVersion, IDelegateWrapper* delegateWrapper)
{
    IfNullReturnError(host, E_POINTER);
    
    HRESULT hr = EnsureProjection();
    IfFailedReturn(hr);

    Assert(projectionContext != nullptr);
    return projectionContext->SetProjectionHost(host, isConfigurable, targetVersion, delegateWrapper);
}

STDMETHODIMP ScriptEngine::ReserveNamespace(LPCWSTR name, BOOL isExtensible)
{
    HRESULT hr = EnsureProjection();
    IfFailedReturn(hr);

    Assert(projectionContext != nullptr);
    return projectionContext->ReserveNamespace(name, isExtensible);
}

// *** Implementation of ScriptEngine's IPrivateScriptProjection Methods ***
STDMETHODIMP ScriptEngine::ResetDelegateWrapper(IDelegateWrapper* newDelegateWrapper)
{
    HRESULT hr = EnsureProjection();
    IfFailedReturn(hr);

    Assert(projectionContext != nullptr);
    return projectionContext->ResetDelegateWrapper(newDelegateWrapper);
}
#endif
