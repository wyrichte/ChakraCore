//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include <ProjectionPch.h>

#ifdef PROJECTION_METADATA_TRACE
#define TRACE_METADATA(...) { Trace(__VA_ARGS__); }
#else
#define TRACE_METADATA(...)
#endif

namespace Projection
{
    //****************************************************************************
    // Implementation for NamespaceProjection
    //****************************************************************************
    NamespaceProjection::NamespaceProjection(ProjectionContext* projectionContext, BOOL isExtensible) :
        ProjectionTypeOperations(),
        m_pszFullName(NULL),
        m_isExtensible(isExtensible),
        m_directChildren(nullptr),
        m_alreadyFullyProjectedPropertiesIfConfigurable(ImmutableList<LPCWSTR>::Empty()),
        projectionContext(projectionContext)
    {
    }

    // TODO: can we defer load this?
    void NamespaceProjection::Initialize(LPCWSTR fullName, LPCWSTR partialName, Var parentObject, bool fEnumerable)
    {
        // Create the JS Object representing this namespace
        Js::ScriptContext* scriptContext = projectionContext->GetScriptContext();
        m_pszFullName = StringOfId(scriptContext, IdOfString(scriptContext, fullName));
        PropertyId propertyId = IdOfString(scriptContext, partialName);

        ScriptEngine *scriptEngine = projectionContext->GetScriptEngine();
        IfNullMapAndThrowHr(scriptContext, scriptEngine, E_ACCESSDENIED);

        HRESULT hr = scriptEngine->CreateTypeFromScript((::TypeId)scriptContext->CreateTypeId(), NULL, NULL, this, false, propertyId, true, &m_type);
        IfFailedMapAndThrowHr(scriptContext, hr);

        // Now, create an instance
        Recycler* recycler = scriptContext->GetRecycler();
        m_instance = RecyclerNew(recycler, Js::CustomExternalObject, (Js::CustomExternalType *)m_type);
        
        BOOL wasSet = false;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            PropertyAttributes propertyAttributes = fEnumerable ? PropertyAttributes_Enumerable : PropertyAttributes_None;
            if (projectionContext->AreProjectionPrototypesConfigurable())
            {
                propertyAttributes = PropertyAttributes_Default;
            }

            hr = Projection::GetDefaultTypeOperations()->SetPropertyWithAttributes(scriptContext->GetActiveScriptDirect(), parentObject, propertyId, m_instance, propertyAttributes, SideEffects_None, &wasSet);
        }
        END_LEAVE_SCRIPT(scriptContext)
        IfFailedMapAndThrowHr(scriptContext, hr);
    }

    NamespaceProjection::~NamespaceProjection()
    {
    }
    
    HRESULT STDMETHODCALLTYPE NamespaceProjection::HasOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        HRESULT hr = NOERROR;

        IfNullReturnError(instance, E_INVALIDARG);
        if (Js::TaggedInt::Is(instance))
        {
            *result = false;
            return NO_ERROR;
        }

        hr = EnsureHasProperty(scriptDirect, instance, propertyId);

        if (SUCCEEDED(hr))
        {
            hr = __super::HasOwnProperty(scriptDirect, instance, propertyId, result);
        }
        return hr;
    }


    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetPropertyReference(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        HRESULT hr = NOERROR;
        hr = EnsureHasProperty(scriptDirect, instance, propertyId);
        if (SUCCEEDED(hr))
        {
            hr = __super::GetPropertyReference(scriptDirect, instance, propertyId, value, propertyPresent);
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::DeleteProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        HRESULT hr = NOERROR;
        *result = FALSE;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::DeleteProperty(scriptDirect, instance, propertyId, result);
            }

            if (SUCCEEDED(hr))
            {
                // also delete the subNamespace from the list of children namespaces
                // note - this is done for consistency of internal state. if design mode is NOT enabled, the
                // next EnsureHasProperty() will re-project the namespace. This behavior (always allow delete)
                // is necessary to allow extensibility scenario.
                Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

                BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
                {
                    Js::PropertyRecord const * nameStr = scriptContext->GetPropertyName(propertyId);
                    IfNullReturnError(nameStr, E_FAIL);
                    LPCWSTR propertyName = nameStr->GetBuffer();

                    AutoHeapString heapString;
                    heapString.CreateNew(wcslen(m_pszFullName) + wcslen(propertyName) + 2); // +2, 1 for the '.', 1 for '\0'
                    swprintf_s(heapString.Get(), heapString.GetLength(), _u("%s%c%s"), m_pszFullName, _u('.'), propertyName);

                    projectionContext->DeleteSubNamespace(heapString.Get());

                    if (projectionContext->AreProjectionPrototypesConfigurable())
                    {
                        // record the property name as projected
                        RecordAlreadyFullyProjectedProperty(propertyName);
                    }
                }
                END_JS_RUNTIME_CALL(scriptContext);
            }
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var *value,
        /* [out] */ BOOL *itemPresent) 
    {
        *itemPresent = FALSE;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        *result = FALSE;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::DeleteItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result) 
    {
        *result = FALSE;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetEnumerator(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ BOOL enumNonEnumerable,
        /* [in] */ BOOL enumSymbols,
        /* [out] */ IVarEnumerator **enumerator) 
    {
        HRESULT hr = S_OK;

        if (m_directChildren == nullptr)
        {
            hr = GetDirectNamespaceChildren();
            IfFailedReturn(hr);
        }

        CComPtr<IVarEnumerator> pDefaultOperatationEnumerator;
        hr = __super::GetEnumerator(scriptDirect, instance, enumNonEnumerable, enumSymbols, &pDefaultOperatationEnumerator);
        IfFailedReturn(hr);

        // Create the NamespaceProjectionEnumerator
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(projectionContext->GetScriptContext())
        {  
            NamespaceProjectionEnumerator *pNamespaceProjectionEnumerator = HeapNew(NamespaceProjectionEnumerator, this, instance, pDefaultOperatationEnumerator); 
            hr = pNamespaceProjectionEnumerator->QueryInterface(__uuidof(IVarEnumerator), (void**)enumerator);            
            Assert(SUCCEEDED(hr));
        }
        END_JS_RUNTIME_CALL(projectionContext->GetScriptContext());
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::IsEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        HRESULT hr = NO_ERROR;
        hr = EnsureHasProperty(scriptDirect, instance, propertyId);
        if (SUCCEEDED(hr))
        {
            hr = __super::IsEnumerable(scriptDirect, instance, propertyId, result); 
        } 
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::IsWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        HRESULT hr = NO_ERROR;
        hr = EnsureHasProperty(scriptDirect, instance, propertyId);
        if (SUCCEEDED(hr))
        {
            hr = __super::IsWritable(scriptDirect, instance, propertyId, result);
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::IsConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        HRESULT hr = NO_ERROR;
        hr = EnsureHasProperty(scriptDirect, instance, propertyId);
        if (SUCCEEDED(hr))
        {
            hr = __super::IsConfigurable(scriptDirect, instance, propertyId, result);
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        HRESULT hr = NO_ERROR;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::SetEnumerable(scriptDirect, instance, propertyId, value); 
            } 
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        HRESULT hr = NO_ERROR;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::SetWritable(scriptDirect, instance, propertyId, value); 
            } 
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        HRESULT hr = NO_ERROR;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::SetConfigurable(scriptDirect, instance, propertyId, value); 
            } 
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* getter,
        /* [out] */ Var* setter,
        /* [out] */ BOOL* result)
    {
        HRESULT hr = NO_ERROR;
        *result = FALSE;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::GetAccessors(scriptDirect, instance, propertyId, getter, setter, result); 
            } 
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var getter,
        /* [in] */ Var setter) 
    {
        HRESULT hr = NO_ERROR;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::SetAccessors(scriptDirect, instance, propertyId, getter, setter); 
            } 
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetSetter( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        HRESULT hr = NO_ERROR;
        *flags = DescriptorFlags_None;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::GetSetter(scriptDirect, instance, propertyId, setter, flags); 
            } 
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::HasOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ Var index,
        /* [out] */ BOOL* result)
    {
        *result = FALSE;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetItemSetter(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index, 
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags)
    {
        *flags = DescriptorFlags_None;
        return NOERROR;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetPropertyWithAttributes(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ PropertyAttributes attributes,
            /* [in] */ SideEffects effects, 
            /* [out]*/ BOOL* result)
    {
        HRESULT hr = NO_ERROR;
        *result = FALSE;

        if (m_isExtensible)
        {
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::SetPropertyWithAttributes(scriptDirect, instance, propertyId, value, attributes, effects, result); 
            }
        } 
        return hr;
    }

    HRESULT STDMETHODCALLTYPE NamespaceProjection::GetOwnProperty(
                /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
                Var instance, PropertyId propertyId, Var *value, BOOL *propertyPresent)
    {
        HRESULT hr = NO_ERROR;
        *propertyPresent = FALSE;

        // Make sure that if this property is on the namespace, we have it.
        hr = EnsureHasProperty(scriptDirect, instance, propertyId);
        if (SUCCEEDED(hr))
        {
            hr = __super::GetOwnProperty(scriptDirect, instance, propertyId, value, propertyPresent);
        }
        return hr;
    }


    HRESULT STDMETHODCALLTYPE NamespaceProjection::SetProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        Var instance, PropertyId propertyId, Var value, BOOL *result)
    {
        HRESULT hr = NO_ERROR;
        *result = FALSE;
        
        if (m_isExtensible)
        {
            // Make sure that if this property is on the namespace, we have it.
            hr = EnsureHasProperty(scriptDirect, instance, propertyId);
            if (SUCCEEDED(hr))
            {
                hr = __super::SetProperty(scriptDirect, instance, propertyId, value, result);
            }
        }
        return hr;
    }

    // *** ABI Projector Helper Methods
    HTYPE NamespaceProjection::GetJSType()
    {
        return m_type;
    }

    Var NamespaceProjection::GetJSInstance()
    {
        return m_instance;
    }

    LPCWSTR NamespaceProjection::GetFullName()
    {
        return m_pszFullName;
    }

    BOOL NamespaceProjection::GetIsExtensible()
    {
        return m_isExtensible;
    }

    ImmutableList<LPCWSTR> * NamespaceProjection::GetDirectChildren()
    {
        return m_directChildren;
    }

    // *** Private Helper Methods

    HRESULT NamespaceProjection::EnsureHasProperty(IActiveScriptDirect* scriptDirect, Var instance, PropertyId propertyId)
    {
        HRESULT hr = S_OK;
        BOOL propertyPresent = FALSE;

        hr = __super::HasOwnProperty(scriptDirect, instance, propertyId, &propertyPresent);

        if (SUCCEEDED(hr))
        {
            if (!propertyPresent)
            {
                Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
                Js::PropertyRecord const * nameStr = scriptContext->GetPropertyName(propertyId);
                IfNullReturnError(nameStr, E_FAIL);
                LPCWSTR propertyName = nameStr->GetBuffer();

                Var constructorInstance = nullptr;
                BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
                {
                    AutoHeapString heapString;
                    heapString.CreateNew(wcslen(m_pszFullName) + wcslen(propertyName) + 2); // +2, 1 for the '.', 1 for '\0'
                    swprintf_s(heapString.Get(), heapString.GetLength(), _u("%s%c%s"), m_pszFullName, _u('.'), propertyName);

                    RtEXPR expr = nullptr;
                    hr = projectionContext->GetExpr(MetadataStringIdNil, IdOfString(scriptContext, heapString.Get()), heapString.Get(), nullptr, &expr);

                    if (RO_E_METADATA_NAME_NOT_FOUND == hr)
                    {
                        // This is not a namespace or type, do not project
                        hr = S_OK;
                    }
                    else
                    {
                        // Project it only if it hasn't already been deleted
                        bool doProject = true;

                        // This is a namespace/property/..., generate it only the 1st time 'til deleted (for design mode only)!
                        if (projectionContext->AreProjectionPrototypesConfigurable())
                        {
                            // only project the first time
                            doProject = !IsAlreadyFullyProjectedProperty(propertyName);
                        }

                        TRACE_METADATA(_u("EnsureHasProperty(%s (%s#%d)) - doProject = %d\n"), heapString.Get(), propertyName, propertyId, doProject);

                        if (doProject)
                        {
                            if (projectionContext->AreProjectionPrototypesConfigurable())
                            {
                                // record the property name as projected
                                RecordAlreadyFullyProjectedProperty(propertyName);
                            }

                            if (RO_E_METADATA_NAME_IS_NAMESPACE == hr)
                            {
                                projectionContext->CreateSubNamespace(instance, heapString.Get(), propertyName, m_isExtensible);
                            }
                            else if (SUCCEEDED(hr) && (NULL != expr))
                            {
                                ProjectionWriter *writer = projectionContext->GetProjectionWriter();
                                constructorInstance = writer->WriteExpr(expr);
                            }
                            else
                            {
                                // An unexpected error has occured in obtaining the metadata information for this type. Throw an error.
                                AutoHeapString errorString;
                                errorString.CreateNew(heapString.GetLength() + /*" (0x)"*/ 5 + /*HRESULT length*/ 8);
                                swprintf_s(errorString.Get(), errorString.GetLength(), _u("%s (0x%X)"), heapString.Get(), hr);
                                Js::JavascriptError::ThrowError(scriptContext, JSERR_UnexpectedMetadataFailure, errorString.Get());
                            }
                        }

                        hr = S_OK;
                    }
                }
                END_JS_RUNTIME_CALL(scriptContext);
                
                if (constructorInstance != nullptr)
                {
                    Assert(SUCCEEDED(hr));
                    BOOL wasSet;

                    if (this->projectionContext->AreProjectionPrototypesConfigurable())
                    {
                        // if design/unit test mode - allow CTORs to be enumerable/configurable/writable
                        hr = __super::SetPropertyWithAttributes(scriptDirect, instance, propertyId, constructorInstance, PropertyAttributes_Default, SideEffects_Any, &wasSet);
                        Assert(SUCCEEDED(hr));
                        Assert(wasSet);
                    }
                    else
                    {
                        // otherwise - allow CTORs to be only enumerable
                        hr = __super::SetPropertyWithAttributes(scriptDirect, instance, propertyId, constructorInstance, PropertyAttributes_Enumerable, SideEffects_Any, &wasSet);
                        Assert(SUCCEEDED(hr));
                        Assert(wasSet);
                    }
                }
            }
        }

        return hr;
    }

    HRESULT NamespaceProjection::AddDirectChildTypesFromMetadata(IUnknown* metadataImport, BOOL isVersioned)
    {
        IfNullReturnError(metadataImport, E_POINTER);

        // Get the Assembly for this IMetaDataImport
        Metadata::Assembly *assembly = nullptr;
        CComPtr<IMetaDataImport2> import;
        HRESULT hr = metadataImport->QueryInterface(IID_IMetaDataImport2, (void**)&import);
        IfFailedReturn(hr);
        hr = projectionContext->CreateMetadataAssembly(import, &assembly, (isVersioned == TRUE));
        IfFailedReturn(hr);

        LPCWSTR nsName = m_pszFullName;
        size_t nsLength = wcslen(m_pszFullName);

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        // returns true if the type is a public, WinRT child of this namespace (not necessarily a direct child)
        auto isNamepaceChild = [&](const Metadata::TypeDefProperties* typeDef)->bool
        {
            LPCWSTR typeName = Projection::StringOfId(scriptContext, typeDef->id);
            // Filter for public, WinRT types that start with the full namespace name
            return ((wcslen(typeName) > (nsLength + 1)) &&
                (wcsncmp(nsName, typeName, nsLength) == 0));
        };

        auto isRuntimeClassOrEnum = [&](const Metadata::TypeDefProperties* typeDef)->bool
        {
            if (typeDef->assembly.IsEnumTypeRef(typeDef->extends))
            {
                // Return true if Enum
                return true;
            }
            if (typeDef->assembly.IsValueTypeRef(typeDef->extends) || typeDef->assembly.IsDelegateTypeRef(typeDef->extends))
            {
                // Return false if struct or delegate
                return false;
            }

            // Otherwise return true if type is a class (and also enforce the AllowForWeb attribute if it is a class)
            return typeDef->IsClass() &&
                   (!projectionContext->EnforceAllowForWeb() || typeDef->assembly.IsAttributePresent(typeDef->td, _u("Windows.Foundation.Metadata.AllowForWebAttribute")));
        };

        // Get all type definitions contained in the metadata file
        ImmutableList<const Metadata::TypeDefProperties*> *typeDefs = assembly->TypeDefinitions();
        typeDefs->WhereInPlace(isNamepaceChild)->Iterate([&](const Metadata::TypeDefProperties* typeDef) 
        {
            // get type name after target namespace and dot
            LPCWSTR childName = &(Projection::StringOfId(scriptContext, typeDef->id)[nsLength + 1]);

            auto projectionAllocator = ProjectionAllocator();
            auto nextSegment = ProjectionModel::GetToDot(childName, [projectionAllocator](int strLength)->LPWSTR {
                return AnewArray(projectionAllocator, char16, strLength);
            });
            
            if (!nextSegment.HasValue())
            {
                // If the type is a direct child type, is a public WinRT type, is not WebHostHidden, is within the target version, and is either a runtimeclass or an enum,
                // add the child name to the list of direct children
                if (typeDef->IsWindowsRuntime() &&
                    (!typeDef->assembly.IsAttributePresent(typeDef->td, _u("Windows.Foundation.Metadata.WebHostHiddenAttribute")) || projectionContext->IgnoreWebHidden()) &&
                    (projectionContext->GetProjectionBuilder()->IsWithinTargetVersion(typeDef->td, typeDef->assembly))&&
                    isRuntimeClassOrEnum(typeDef))
                {
                    Assert(childName != nullptr);
                    m_directChildren = m_directChildren->Prepend(childName, ProjectionAllocator());
                }
            }
            else
            {
                // Add the direct subnamespace only if it has not already been added
                LPCWSTR childNamespaceName = nextSegment.GetValue();
                if (!m_directChildren->ContainsWhere([&](LPCWSTR name) {
                    return (wcscmp(name, childNamespaceName) == 0);
                }))
                {
                    m_directChildren = m_directChildren->Prepend(childNamespaceName, ProjectionAllocator());
                }
            }
        });

        return S_OK;
    }

    void NamespaceProjection::ReleaseAll(__in DWORD unkCount, __in_ecount(unkCount) IUnknown ** toRelease)
    {
        for (DWORD i=0; i<unkCount; i++)
        {
            if (toRelease[i])
            {
                toRelease[i]->Release();
            }
        }
    }

    HRESULT NamespaceProjection::GetDirectNamespaceChildren()
    {
        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
        {
            DWORD metadataImportCount = 0;
            IUnknown ** metadataImports = nullptr;
            DWORD childNamespacesCount = 0;
            LPWSTR * childNamespaces = nullptr;
            BOOL * metadataImportIsVersioned = nullptr;

            hr = projectionContext->GetNamespaceChildren(m_pszFullName, &metadataImportCount, &metadataImports, &childNamespacesCount, &childNamespaces, &metadataImportIsVersioned);
            IfFailedReturn(hr);

            for (DWORD i=0; i<childNamespacesCount; i++)
            {
                LPWSTR childNamespace = childNamespaces[i];
                size_t currentNameLength = wcslen(childNamespace) + 1;
                LPWSTR childNamespaceCopy = AnewArray(ProjectionAllocator(), char16, currentNameLength);
                wcscpy_s(childNamespaceCopy, currentNameLength, childNamespace);
                m_directChildren = m_directChildren->Prepend(childNamespaceCopy, ProjectionAllocator());
                CoTaskMemFree(childNamespace);
                childNamespaces[i] = nullptr;
            }
            CoTaskMemFree(childNamespaces);
            childNamespaces = nullptr;

            for (DWORD i=0; i<metadataImportCount; i++)
            {
                hr = AddDirectChildTypesFromMetadata(metadataImports[i], metadataImportIsVersioned[i]);
                if (FAILED(hr))
                {
                    ReleaseAll(metadataImportCount-i, &metadataImports[i]);
                    CoTaskMemFree(metadataImports);
                    metadataImports = nullptr;
                    CoTaskMemFree(metadataImportIsVersioned);
                    metadataImportIsVersioned = nullptr;
                    return hr;
                }
                metadataImports[i]->Release();
            }
            m_directChildren = m_directChildren->SortCurrentList(&CompareRefConstNames::Instance);

            CoTaskMemFree(metadataImports);
            metadataImports = nullptr;
            CoTaskMemFree(metadataImportIsVersioned);
            metadataImportIsVersioned = nullptr;
        }
        END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

        return hr;
    }

    void NamespaceProjection::RecordAlreadyFullyProjectedProperties(ImmutableList<LPCWSTR>* names)
    {
        if (projectionContext->AreProjectionPrototypesConfigurable())
        {
            if (names != nullptr)
            {
                names->Iterate([&](LPCWSTR name) {
                    RecordAlreadyFullyProjectedProperty(name);
                });
            }
        }
    }

    void NamespaceProjection::RecordAlreadyFullyProjectedProperty(LPCWSTR name)
    {
        if (projectionContext->AreProjectionPrototypesConfigurable())
        {
            if (IsAlreadyFullyProjectedProperty(name) == FALSE)
            {
                this->m_alreadyFullyProjectedPropertiesIfConfigurable = this->m_alreadyFullyProjectedPropertiesIfConfigurable->Prepend(name, ProjectionAllocator());
            }
        }
    }

    BOOL NamespaceProjection::IsAlreadyFullyProjectedProperty(LPCWSTR name)
    {
        BOOL retValue = FALSE;

        if (projectionContext->AreProjectionPrototypesConfigurable())
        {
            retValue = m_alreadyFullyProjectedPropertiesIfConfigurable->ContainsWhere([&](LPCWSTR projectedPropertyName) {
                return (wcscmp(name, projectedPropertyName) == 0);
            });
        }

        return retValue;
    }

    BOOL NamespaceProjection::Is(Var instance)
    {
        Js::CustomExternalObject* externalObject = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(instance);
        if (!externalObject)
        {
            return FALSE;
        }
        
        Js::ExternalType * externalType = (Js::ExternalType *)externalObject->GetType();
        ProjectionTypeOperations* projectionTypeOperations = NULL;
        if (SUCCEEDED(externalType->GetTypeOperations()->QueryInterface(IID_IProjectionTypeOperations, (void**)&projectionTypeOperations)))
        {
            ProjectionType projectionType = projectionTypeOperations->GetProjectionType();
            projectionTypeOperations->Release();
            return projectionType == NamespaceProjectionType;
        }

        return FALSE;
    }
}