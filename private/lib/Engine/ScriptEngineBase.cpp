//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "share.h"
#pragma hdrstop
#include <fcntl.h>
#ifdef ENABLE_JS_ETW
#include <IERESP_mshtml.h>
#endif
#include "edgejsStatic.h"

#include "Library\JavascriptRegularExpression.h"
#include "Library\JavascriptPromise.h"
#include "Library\JavascriptWeakMap.h"

#include "RegexCommon.h"
#include "Library\RegexHelper.h"

#include "Types\DeferredTypeHandler.h"
#include "Types\PathTypeHandler.h"

// SCA support
#include "SCAEngine.h"
#include "StreamHelper.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "SCASerialization.h"
#include "SCADeserialization.h"

using namespace PlatformAgnostic;

ScriptEngineBase::ScriptEngineBase() :
    scriptContext(nullptr),
    threadContext(nullptr),
    scriptSiteHolder(nullptr),
    m_refCount(0),
    wasScriptDirectEnabled(false),
    wasBinaryVerified(true)
{
}

ScriptEngineBase::~ScriptEngineBase()
{
}

// *** IUnknown Methods ***
STDMETHODIMP ScriptEngineBase::QueryInterface(
    /* [in]  */ REFIID riid,
    /* [out] */ void **ppvObj)
{
    QI_IMPL(__uuidof(IActiveScriptDirect), IActiveScriptDirect);
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ScriptEngineBase::AddRef(void)
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) ScriptEngineBase::Release(void)
{
    long lw = InterlockedDecrement(&m_refCount);
    if (0 == lw)
    {
        delete this;
    }
    return lw;
}



HRESULT STDMETHODCALLTYPE ScriptEngineBase::VerifyBinaryConsistency(__in void* dataContext)
{
    JsStaticAPI::BinaryVerificationData * binaryVerificationData = (JsStaticAPI::BinaryVerificationData*)dataContext;
    if (
        binaryVerificationData->majorVersion != SCRIPT_ENGINE_MAJOR_VERSION ||
        binaryVerificationData->minorVersion != SCRIPT_ENGINE_MINOR_VERSION ||
        binaryVerificationData->scriptEngineBaseSize != sizeof(ScriptEngineBase) ||
        binaryVerificationData->scriptEngineBaseOffset != (DWORD)(static_cast<ScriptEngine*>((IActiveScriptDirect*)0x0)) ||
        binaryVerificationData->scriptContextBaseSize != sizeof(Js::ScriptContextBase) ||
        binaryVerificationData->scriptContextBaseOffset != (DWORD)((Js::ScriptContext*)0x0)->GetScriptContextBase() ||
        binaryVerificationData->javascriptLibraryBaseSize != sizeof(Js::JavascriptLibraryBase) ||
        binaryVerificationData->javascriptLibraryBaseOffset != (DWORD)((Js::JavascriptLibrary*)0x0)->GetLibraryBase() ||
        binaryVerificationData->customExternalObjectSize != sizeof(Js::CustomExternalObject) ||
        binaryVerificationData->typeOffset != (DWORD)((Js::RecyclableObject*)(0x0))->GetOffsetOfType() ||
        binaryVerificationData->typeIdOffset != (DWORD)((Js::Type*)(0x0))->GetTypeIdFieldOffset() ||
        binaryVerificationData->taggedIntSize != sizeof(Js::TaggedInt) ||
        binaryVerificationData->typeIdLimit != TypeIds_Limit ||
        binaryVerificationData->javascriptNumberSize != sizeof(Js::JavascriptNumber) ||
        binaryVerificationData->numberUtilitiesBaseSize != sizeof(Js::NumberUtilitiesBase) ||
        binaryVerificationData->numberUtilitiesBaseOffset != (DWORD)((Js::NumberUtilities*)0x0)->GetNumberUtilitiesBase())
    {
        wasBinaryVerified = FALSE;
        JS_ETW(EventWriteJSCRIPT_HOSTING_BINARYINCONSISTENCY(
            sizeof(ScriptEngineBase), binaryVerificationData->scriptEngineBaseSize,
            sizeof(Js::ScriptContextBase), binaryVerificationData->scriptContextBaseSize,
            sizeof(Js::JavascriptLibraryBase), binaryVerificationData->javascriptLibraryBaseSize,
            sizeof(Js::CustomExternalObject), binaryVerificationData->customExternalObjectSize,
            (DWORD)(static_cast<ScriptEngine*>((IActiveScriptDirect*)0x0)), binaryVerificationData->scriptEngineBaseOffset,
            (DWORD)((Js::ScriptContext*)0x0)->GetScriptContextBase(), binaryVerificationData->scriptContextBaseOffset,
            (DWORD)((Js::JavascriptLibrary*)0x0)->GetLibraryBase(), binaryVerificationData->javascriptLibraryBaseOffset,
            (DWORD)((Js::RecyclableObject*)(0x0))->GetOffsetOfType(), binaryVerificationData->typeOffset,
            (DWORD)((Js::Type*)(0x0))->GetTypeIdFieldOffset(), binaryVerificationData->typeIdOffset,
            sizeof(Js::TaggedInt), binaryVerificationData->taggedIntSize,
            sizeof(Js::JavascriptNumber), binaryVerificationData->javascriptNumberSize,
            TypeIds_Limit, binaryVerificationData->typeIdLimit,
            sizeof(Js::NumberUtilitiesBase), binaryVerificationData->numberUtilitiesBaseSize,
            (DWORD)((Js::NumberUtilities*)0x0)->GetNumberUtilitiesBase(), binaryVerificationData->numberUtilitiesBaseOffset
            ));
        Binary_Inconsistency_fatal_error();
        AssertMsg(FALSE, "should not come here");
        return E_FAIL;
    }
    return NOERROR;
}


HRESULT STDMETHODCALLTYPE ScriptEngineBase::SetHostObject(
    __in Var hostObject,
    __in Var secureHostObject)
{
    HRESULT hr = NOERROR;
    IFFAILRET(VerifyOnEntry());

    if (Js::TaggedNumber::Is(hostObject) || hostObject == nullptr || secureHostObject == nullptr || Js::TaggedNumber::Is(secureHostObject))
    {
        return E_INVALIDARG;
    }
    else
    {
        Assert(wasBinaryVerified);

        bool fSetupFastDOM = !scriptContext->IsFastDOMEnabled();

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
        if (SUCCEEDED(hr) && DBG_DUMP && (Js::JavascriptOperators::GetTypeId(secureHostObject) != Js::TypeIds_Undefined)  && (
#ifdef PROFILE_EXEC
        Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag)
#endif
#ifdef PROFILE_MEM
        || MemoryProfiler::IsEnabled() || MemoryProfiler::IsTraceEnabled()
#endif
        ) )
        {
            GetScriptSiteHolder()->SetupWindowHost(Js::RecyclableObject::FromVar(secureHostObject));
        }
#endif
        hr = scriptContext->GetGlobalObject()->SetDirectHostObject(Js::RecyclableObject::FromVar(hostObject), Js::RecyclableObject::FromVar(secureHostObject));

        if (SUCCEEDED(hr) && fSetupFastDOM)
        {
            GetScriptSiteHolder()->SetupFastDOM(static_cast<IActiveScriptDirect*>(this));
            scriptContext->SetDirectHostTypeId(Js::JavascriptOperators::GetTypeId(hostObject));
            wasScriptDirectEnabled = TRUE;
        }
#if DBG_DUMP
        GetScriptSiteHolder()->CaptureSetHostObjectTrace();

#endif
        return hr;
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetHostObject(
    __out Var* hostObject)
{
    IfNullReturnError(hostObject, E_INVALIDARG);
    *hostObject = nullptr;

    HRESULT hr = NOERROR;
    IFFAILRET(VerifyOnEntry());

    *hostObject = scriptContext->GetGlobalObject()->GetDirectHostObject();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ReserveGlobalProperty(
    __in PropertyId propertyId)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetOrAddPropertyId(
    /* [in] */ __RPC__in LPCWSTR name,
    /* [out] */ __RPC__out PropertyId *id)
{
    IfNullReturnError(id, E_INVALIDARG);
    *id = Js::Constants::NoProperty;

    IfNullReturnError(name, E_INVALIDARG);
    HRESULT hr = NO_ERROR;
    IFFAILRET(VerifyOnEntry(TRUE));

    if (threadContext->GetRecycler()->IsHeapEnumInProgress())
    {
        Js::PropertyRecord const * propertyRecord;
        threadContext->FindPropertyRecord(name, Js::JavascriptString::GetBufferLength(name), &propertyRecord);
        if (propertyRecord != nullptr)
        {
            *id = propertyRecord->GetPropertyId();
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        return hr;
    }
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *id = scriptContext->GetOrAddPropertyIdTracked(name, Js::JavascriptString::GetBufferLength(name));
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr)
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetPropertyName(
    /* [in] */ __RPC__in PropertyId id,
    /* [out] */ __RPC__out LPCWSTR *nameRef)
{
    IfNullReturnError(nameRef, E_INVALIDARG);

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        *nameRef = nullptr;
        return hr;
    }

    Js::PropertyRecord const * nameStr = scriptContext->GetPropertyName(id);
    if (nameStr != nullptr)
    {
        *nameRef = nameStr->GetBuffer();
        return S_OK;
    }
    else
    {
        *nameRef = nullptr;
        return E_FAIL;
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::Parse(
    __in LPWSTR scriptText,
    __out Var *scriptFunc)
{
    IfNullReturnError(scriptFunc, E_INVALIDARG);
    *scriptFunc = nullptr;

    ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(this);
    HRESULT result = scriptEngine->ParseInternal(scriptText, scriptFunc, nullptr);
    if (result == NO_ERROR)
    {
        Assert(Js::ScriptFunction::Is(*scriptFunc));
        Js::ScriptFunction::FromVar(*scriptFunc)->SetIsActiveScript(true);
    }
    return result;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::Execute(
    __in Var instance,
    __in CallInfo callInfo,
    /* [annotation][in] */
    __in_xcount(callInfo.Count)  Var *arguments,
    __in IServiceProvider* serviceProvider,
    __out_opt Var *varResult)
{
    AssertMsg((callInfo.Flags & CallFlags_CallPut) == 0, "This is NOT allowed.");
    if (varResult != nullptr)
    {
        *varResult = nullptr;
    }

    HRESULT hr = NO_ERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    IFFAILRET(VerifyOnEntry());
    if (!Js::JavascriptConversion::IsCallable(instance))
    {
        return E_INVALIDARG;
    }

    Js::ScriptContext* localScriptContext = scriptContext;
    Js::Arguments jsArguments(0, nullptr);
    jsArguments.Info.Count = callInfo.Count;
    jsArguments.Info.Flags = (Js::CallFlags)callInfo.Flags;
    jsArguments.Values = (Js::Var*)arguments;
    Js::RecyclableObject* jsObj = Js::RecyclableObject::FromVar(instance);
    Js::Var returnVar = localScriptContext->GetLibrary()->GetUndefined();

    AutoCallerPointer callerPointer(GetScriptSiteHolder(), serviceProvider);

    hr = GetScriptSiteHolder()->Execute(jsObj, &jsArguments, serviceProvider, &returnVar);
    // The caller needs to marshal the result back to caller context. we don't now who is the caller here.
    if (varResult)
    {
        *varResult = returnVar;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetGlobalObject(
    __out Var *globalObject)
{
    IfNullReturnError(globalObject, E_INVALIDARG);

    HRESULT hr = NOERROR;

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        *globalObject = nullptr;
        return hr;
    }
    *globalObject = scriptContext->GetGlobalObject();
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetDefaultTypeOperations(
    __out ITypeOperations **operations)
{
    IfNullReturnError(operations, E_INVALIDARG);
    *operations = nullptr;

    HRESULT hr = NOERROR;

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    return Js::DefaultScriptOperations::s_DefaultScriptOperations.QueryInterface(_uuidof(ITypeOperations), (void**)operations);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetJavascriptOperations(
    __out IJavascriptOperations **operations)
{
    IfNullReturnError(operations, E_INVALIDARG);
    *operations = nullptr;

    ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(this);
    return scriptEngine->GetJavascriptOperationsInternal(operations);
}


HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetTypeIdForType(
    __in HTYPE type,
    __out JavascriptTypeId* typeIdRef)
{
    HRESULT hr = S_OK;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    if (type == nullptr || typeIdRef == nullptr)
    {
        hr = E_INVALIDARG;
        return hr;
    }

    *typeIdRef = (JavascriptTypeId)((Js::ExternalType*)type)->GetTypeId();

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetTypeIdForVar(
    __in Var instance,
    __out JavascriptTypeId* typeIdRef)
{
    IfNullReturnError(typeIdRef, E_INVALIDARG);
    *typeIdRef = 0;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    *typeIdRef = (JavascriptTypeId)Js::JavascriptOperators::GetTypeId(instance);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ReserveStaticTypeIds(
    __in int first,
    __in int last)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    if (scriptContext->ReserveStaticTypeIds(first, last))
    {
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ReserveTypeIds(
    __in int count,
    __out JavascriptTypeId* firstTypeId)
{
    IfNullReturnError(firstTypeId, E_INVALIDARG);
    *firstTypeId = 0;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    *firstTypeId = (JavascriptTypeId)scriptContext->ReserveTypeIds(count);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::RegisterWellKnownTypeId(
    __in WellKnownType wellKnownType,
    __in JavascriptTypeId typeId)
{
    HRESULT hr = NOERROR;
    IFFAILRET(VerifyOnEntry());

    if (wellKnownType > WellKnownType_Last || typeId == TypeId_Unspecified)
    {
        return E_INVALIDARG;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        scriptContext->SetWellKnownHostTypeId((WellKnownHostType)wellKnownType, (Js::TypeId)typeId);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateObject(
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *instance = scriptContext->GetLibrary()->CreateObject();
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*instance));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *instance = Js::JavascriptProxy::AutoProxyWrapper(*instance);
        }
#endif
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateConstructor(
    __in Var prototype,
    __in ScriptMethod entryPoint,
    __in PropertyId nameId,
    __in BOOL bindReference,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    if (nameId != 0 && !scriptContext->IsTrackedPropertyId(nameId))
    {
        return E_INVALIDARG;
    }
    if (prototype != nullptr && !Js::RecyclableObject::Is(prototype))
    {
        return E_INVALIDARG;
    }
    if (prototype != nullptr)
    {
        prototype = Js::CrossSite::MarshalVar(scriptContext, prototype);
    }
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *instance = library->CreateExternalConstructor((Js::ExternalMethod)entryPoint, nameId, prototype == nullptr ? nullptr : Js::RecyclableObject::FromVar(prototype));

        if ( bindReference )
        {
            scriptContext->BindReference(*instance);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateDeferredConstructor(
    __in_opt ScriptMethod entryPoint,
    __in PropertyId nameId,
    __in InitializeMethod method,
    __in unsigned short deferredTypeSlots,
    __in BOOL hasAccessors,
    __in BOOL bindReference,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    IfNullReturnError(method, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    if (nameId != 0 && !scriptContext->IsTrackedPropertyId(nameId))
    {
        return E_INVALIDARG;
    }
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *instance = library->CreateExternalConstructor((Js::ExternalMethod)entryPoint, nameId, method, deferredTypeSlots, (hasAccessors != FALSE));

        if ( bindReference )
        {
            scriptContext->BindReference(*instance);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

    return hr;
}

HRESULT ScriptEngineBase::GetOPrototypeInformationForTypeCreation(
    __in Var &varPrototype,
    __in PropertyId nameId,
    __out Js::RecyclableObject** objPrototype)
{
    HRESULT hr = S_OK;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    Assert(objPrototype != nullptr);
    Js::ScriptContext* scriptContext = GetScriptContext();
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();
    if (nameId != 0 && !scriptContext->IsTrackedPropertyId(nameId))
    {
        return E_INVALIDARG;
    }

    if (varPrototype == nullptr)
    {
        *objPrototype = library->GetObjectPrototype();
    }
    else if (Js::RecyclableObject::Is(varPrototype))
    {
        varPrototype = Js::CrossSite::MarshalVar(scriptContext, varPrototype);
        *objPrototype = Js::RecyclableObject::FromVar(varPrototype);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT ScriptEngineBase::CreateTypeFromPrototype(
    __in TypeId inTypeId,
    __in Js::RecyclableObject* objPrototype,
    __in ScriptMethod entryPoint,
    __in ITypeOperations* operations,
    __in BOOL fDeferred,
    __in PropertyId nameId,
    __in BOOL bindReference,
    __out HTYPE* typeRef)
{
    return CreateTypeFromPrototypeInternal(inTypeId, nullptr, 0, objPrototype, entryPoint, operations, fDeferred, nameId, bindReference, 0, typeRef);
}

HRESULT ScriptEngineBase::CreateTypeFromPrototypeInternal(
    __in TypeId inTypeId,
    __in __RPC__in_ecount_full(inheritedTypeIdsCount) const JavascriptTypeId* inheritedTypeIds,
    __in UINT inheritedTypeIdsCount,
    __in Js::RecyclableObject* objPrototype,
    __in ScriptMethod entryPoint,
    __in ITypeOperations* operations,
    __in BOOL fDeferred,
    __in PropertyId nameId,
    __in BOOL bindReference,
    __in UINT extraSlotCount,
    __out HTYPE* typeRef)
{
    HRESULT hr = S_OK;
    Js::TypeId typeId = (Js::TypeId)inTypeId;
    Js::ScriptContext* localScriptContext = scriptContext;
    if (typeId == TypeId_Unspecified)
    {
        typeId = localScriptContext->CreateTypeId();
    }

    Recycler* recycler = localScriptContext->GetRecycler();

    // Keep the type on the stack to keep it alive until we bind it
    Js::Type * type;
    if (operations != nullptr)
    {
        if (extraSlotCount > UCHAR_MAX)
        {
            // Too many slots
            return E_INVALIDARG;
        }
        Js::DynamicTypeHandler* customExternalTypeHandler;
        if ( fDeferred )
        {
            customExternalTypeHandler = Js::DeferredTypeHandler<Js::CustomExternalType::DeferredInitializer>::GetDefaultInstance();
        }
        else
        {
            customExternalTypeHandler = Js::SimplePathTypeHandler::New(localScriptContext, localScriptContext->GetLibrary()->GetRootPath(), 0, 0, 0, true, true);
        }

        Js::CustomExternalType * customExternalType =
            Js::CustomExternalType::New(
            localScriptContext, (Js::TypeId)typeId,
                objPrototype, (Js::ExternalMethod)entryPoint,
                customExternalTypeHandler, true, true, operations, nameId, inheritedTypeIds, inheritedTypeIdsCount, (uint8)extraSlotCount);
        IfFailedReturn(customExternalType->Initialize());
        type = customExternalType;
    }
    else
    {
        if (extraSlotCount != 0)
        {
            // Only type with ITypeOperation can have extra slots
            return E_INVALIDARG;
        }
        Js::ExternalType * externalType = RecyclerNew(recycler, Js::ExternalType,
            localScriptContext, (Js::TypeId)typeId, objPrototype, (Js::ExternalMethod)entryPoint,
            Js::SimplePathTypeHandler::New(localScriptContext, localScriptContext->GetLibrary()->GetRootPath(), 0, 0, 0, true, true), true, true, nameId);
        type = externalType;
    }

    if (bindReference)
    {
        // Why the host close the site halfway during typed object creation?
        Assert(scriptContext != nullptr);
        // Make the lifetime the same as the script context
        if (scriptContext != nullptr)
        {
            scriptContext->BindReference(type);
        }
    }

    // See if the host has registered typeId as a well known type.
    if (localScriptContext->IsWellKnownHostType<WellKnownHostType_HTMLAllCollection>(typeId))
    {
        type->SetIsFalsy(true);
    }

    *typeRef = type;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateType(
    __in JavascriptTypeId typeId,
    __in __RPC__in_ecount_full(inheritedTypeIdsCount) const JavascriptTypeId* inheritedTypeIds,
    __in UINT inheritedTypeIdsCount,
    __in Var varPrototype,
    __in ScriptMethod entryPoint,
    __in ITypeOperations* operations,
    __in BOOL fDeferred,
    __in PropertyId nameId,
    __in BOOL bindReference,
    __out HTYPE* typeRef)
{
    return ScriptEngineBase::CreateTypeWithExtraSlots(typeId, inheritedTypeIds, inheritedTypeIdsCount, varPrototype, entryPoint, operations, fDeferred, nameId, bindReference, 0, typeRef);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateTypeWithExtraSlots(
    __in JavascriptTypeId typeId,
    __in __RPC__in_ecount_full(inheritedTypeIdsCount) const JavascriptTypeId* inheritedTypeIds,
    __in UINT inheritedTypeIdsCount,
    __in Var varPrototype,
    __in ScriptMethod entryPoint,
    __in ITypeOperations* operations,
    __in BOOL fDeferred,
    __in PropertyId nameId,
    __in BOOL bindReference,
    __in UINT extraSlotsCount,
    __out HTYPE* typeRef)
{
    IfNullReturnError(typeRef, E_INVALIDARG);
    *typeRef = nullptr;

    Js::RecyclableObject* objPrototype;
    HRESULT hr;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    hr = GetOPrototypeInformationForTypeCreation(varPrototype, nameId, &objPrototype);
    IfFailedReturn(hr);

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        hr = CreateTypeFromPrototypeInternal((TypeId)typeId, inheritedTypeIds, inheritedTypeIdsCount, objPrototype, entryPoint, operations, fDeferred, nameId, bindReference, extraSlotsCount, typeRef);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT ScriptEngineBase::CreateTypeFromScript(
    __in TypeId typeId,
    __in Var varPrototype,
    __in ScriptMethod entryPoint,
    __in ITypeOperations* operations,
    __in BOOL fDeferred,
    __in PropertyId nameId,
    __in BOOL bindReference,
    __out HTYPE* typeRef)
{
    IfNullReturnError(typeRef, E_INVALIDARG);
    *typeRef = nullptr;

    Js::RecyclableObject* objPrototype;
    HRESULT hr = GetOPrototypeInformationForTypeCreation(varPrototype, nameId, &objPrototype);
    IfFailedReturn(hr);

    return CreateTypeFromPrototype(typeId, objPrototype, entryPoint, operations, fDeferred, nameId, bindReference, typeRef);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateTypedObjectFromScript(
    __in HTYPE type,
    __in int byteCount,
    __in BOOL bindReference,
    __out Var* instance)
{
    Assert(GetScriptSiteHolder() != nullptr);
    Assert(SUCCEEDED(ValidateBaseThread()));

    Recycler* recycler = scriptContext->GetRecycler();

    // TODO: verify this type is one we allocated
    Assert(type != nullptr);

    Js::ExternalType * externalType = (Js::ExternalType *)type;

    Js::DynamicObject* object;
    *instance = nullptr;
    if (externalType->GetTypeOperations() != nullptr)
    {
        Js::CustomExternalType * customExternalType = (Js::CustomExternalType *)externalType;

        if (customExternalType->GetFinalizer() != nullptr)
        {
            if (byteCount == 0)
            {
                object = RecyclerNewFinalized(recycler, Js::CustomExternalObject, customExternalType);
            }
            else
            {

#if DBG
                object = RecyclerNewFinalizedPlus(recycler, byteCount, Js::CustomExternalObject, customExternalType, byteCount);
#else
                object = RecyclerNewFinalizedPlus(recycler, byteCount, Js::CustomExternalObject, customExternalType);
#endif
            }
        }
        else
        {
            if (byteCount == 0)
            {
                object = RecyclerNew(recycler, Js::CustomExternalObject, customExternalType);
            }
            else
            {
#if DBG
                object = RecyclerNewPlus(recycler, byteCount, Js::CustomExternalObject, customExternalType, byteCount);
#else
                object = RecyclerNewPlus(recycler, byteCount, Js::CustomExternalObject, customExternalType);
#endif
            }
        }
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_RECYCLER_ALLOCATE_DOM_OBJECT())
        {
            Assert(!IsWinRTType(Js::CustomExternalObject::FromVar(object)));
            Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(object);
            EventWriteJSCRIPT_RECYCLER_ALLOCATE_DOM_OBJECT(object, typeId);
        }
#endif
    }
    else
    {
        if (byteCount == 0)
        {
            object = RecyclerNew(recycler, Js::ExternalObject, externalType);
        }
        else
        {
#if DBG
            object = RecyclerNewPlus(recycler, byteCount, Js::ExternalObject, externalType, byteCount);
#else
            object = RecyclerNewPlus(recycler, byteCount, Js::ExternalObject, externalType);
#endif
        }
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(object));
        // We can't easily wrap a proxy going to trident as they have memory layout dependency. however we can create
        // a Var from user code which will forward the call to CEO correctly.
    }

    // we can have the scriptcontext closed during object creation.
    if (bindReference)
    {
        // Why the host close the site halfway during typed object creation?
        Assert(scriptContext != nullptr);
        // Make the lifetime the same as the script context
        if (scriptContext != nullptr)
        {
            scriptContext->BindReference(object);
        }
    }
    *instance = object;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateTypedObject(
    __in HTYPE type,
    __in int byteCount,
    __in BOOL bindReference,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    // TODO: verify this type is one we allocated
    if (type == nullptr)
    {
        hr = E_INVALIDARG;
        return hr;
    }

    *instance = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        hr = CreateTypedObjectFromScript(type, byteCount, bindReference, instance);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

// TODO:
//  Create an external object wrapper?
//  type specialization (simpler array)
//  allow different prototype? (more complex array)
HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateArrayObject(
    __in UINT length,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *instance = scriptContext->GetLibrary()->CreateArray(length);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_ARRAY(*instance));
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IsArrayObject(
    __in Var instance,
    __out BOOL* isArray)
{
    IfNullReturnError(isArray, E_INVALIDARG);
    *isArray = FALSE;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    *isArray = Js::DynamicObject::IsAnyArray(instance);
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IsPrimitiveType(
    __in Var instance,
    __out BOOL* result)
{
    IfNullReturnError(result, E_INVALIDARG);
    *result = FALSE;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(instance);
    switch (typeId)
    {
    case Js::TypeIds_Undefined:
    case Js::TypeIds_Null:
    case Js::TypeIds_Boolean:
    case Js::TypeIds_Integer:
    case Js::TypeIds_Number:
    case Js::TypeIds_Int64Number:
    case Js::TypeIds_UInt64Number:
    case Js::TypeIds_String:
    case Js::TypeIds_Symbol:
        *result = TRUE;
        break;

    default:
        *result = FALSE;
        break;
    }

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToDouble(
    __in Var instance,
    __out double* d)
{
    Assert(S_OK == ValidateBaseThread());

    if ( GetScriptSiteHolder() != nullptr )
    {
        return GetScriptSiteHolder()->ExternalToNumber(instance, d);
    }
    else
    {
        return E_ACCESSDENIED;
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::DoubleToVar(
    __in double d,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalDoubleToVar(d, instance);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToDate(
    __in Var instance,
    __out double* d)
{
    IfNullReturnError(d, E_INVALIDARG);
    *d = 0;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalToDate(instance, d);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::DateToVar(
    __in double d,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalDateToVar(d, instance);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::SYSTEMTIMEToVar(
            __in SYSTEMTIME *pst,
            __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    IfNullReturnError(pst, E_INVALIDARG);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalSYSTEMTIMEToVar(pst, instance);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToSYSTEMTIME(
            __in Var instance,
            __out SYSTEMTIME* result)
{
    IfNullReturnError(result, E_INVALIDARG);
    SecureZeroMemory(result, sizeof(SYSTEMTIME));

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);

    hr = VerifyOnEntry();
    if (FAILED(hr))
    {

        return hr;
    }

    if(!Js::JavascriptDate::Is(instance))
    {
        return E_INVALIDARG;
    }

    AUTO_NO_EXCEPTION_REGION;
    Js::JavascriptDate* date = Js::JavascriptDate::FromVar(instance);
    DateTime::YMD ymdDate;
    Js::DateImplementation::GetYmdFromTv(date->GetTime(), &ymdDate);
    ymdDate.ToSystemTime(result);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToInt(
    __in Var instance,
    __out int* i)
{
    IfNullReturnError(i, E_INVALIDARG);
    *i = 0;

    Assert(S_OK == ValidateBaseThread());

    if ( GetScriptSiteHolder() != nullptr )
    {
        return GetScriptSiteHolder()->ExternalToInt32(instance, i);
    }
    else
    {
        return E_ACCESSDENIED;
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToInt64(
    __in Var instance,
    __out __int64 * i)
{
    IfNullReturnError(i, E_INVALIDARG);
    *i = 0;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalToInt64(instance, i);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToUInt64(
    __in Var instance,
    __out unsigned __int64 * i)
{
    IfNullReturnError(i, E_INVALIDARG);
    *i = 0;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalToUInt64(instance, i);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToBOOL(
    __in Var instance,
    __out BOOL* b)
{
    IfNullReturnError(b, E_INVALIDARG);
    *b = FALSE;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalToBoolean(instance, b);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToString(
    __in Var instance,
    __out BSTR* str)
{
    IfNullReturnError(str, E_INVALIDARG);
    *str = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    Js::JavascriptString * result = nullptr;
    hr = GetScriptSiteHolder()->ExternalToString(instance, &result);
    if (FAILED(hr))
    {
        return hr;
    }

    // ExternalToString will flatten the string for us
    Assert(result->UnsafeGetBuffer());
    *str = SysAllocStringLen(result->UnsafeGetBuffer(), (uint32)result->GetLength());
    hr = *str != nullptr ? S_OK : E_OUTOFMEMORY;
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToRawString(
    __in Var instance,
    __out const WCHAR** str,
    __out unsigned int* length)
{
    IfNullReturnError(str, E_INVALIDARG);
    *str = nullptr;
    IfNullReturnError(length, E_INVALIDARG);
    *length = 0;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    Js::JavascriptString * result = nullptr;
    hr = GetScriptSiteHolder()->ExternalToString(instance, &result);
    if (FAILED(hr))
    {
        return hr;
    }

    // ExternalToString will flatten the string for us
    Assert(result->UnsafeGetBuffer());
    *str = result->UnsafeGetBuffer();
    *length = result->GetLength();
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::BOOLToVar(
    __in BOOL b,
    __out Var* instance)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalBOOLToVar(b, instance);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IntToVar(
    __in int i,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    Assert(S_OK == ValidateBaseThread());

    if ( GetScriptSiteHolder() != nullptr )
    {
        return GetScriptSiteHolder()->ExternalInt32ToVar(i, instance);
    }
    else
    {
        return E_ACCESSDENIED;
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::Int64ToVar(
    __in __int64 i,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    return GetScriptSiteHolder()->ExternalInt64ToVar(i, instance);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::UInt64ToVar(
    __in unsigned __int64 i,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {

        return hr;
    }
    return GetScriptSiteHolder()->ExternalUInt64ToVar(i, instance);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::StringToVar(
    __in_ecount(length) const WCHAR* str,
    __in int length,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    if (length < 0) { return E_INVALIDARG; }
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    *instance = nullptr;
    hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if ( str )
        {
            *instance = Js::JavascriptString::NewCopyBuffer(str, (size_t)length, this->scriptContext);
        }
        else
        {
            *instance = scriptContext->GetLibrary()->GetNullString();
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ExtensionToVar(
    __in void* buffer,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    IfNullReturnError(buffer, E_INVALIDARG);

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {

        return hr;
    }
    *instance = (Var)((char*)buffer - sizeof(Js::ExternalObject));
    if (Js::ExternalObject::Is(*instance) || Js::CustomExternalObject::Is(instance))
    {
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToExtension(
    __in Var instance,
    __deref_out void** buffer,
    __out JavascriptTypeId* typeIdRef)
{
    IfNullReturnError(buffer, E_INVALIDARG);
    *buffer = nullptr;
    IfNullReturnError(typeIdRef, E_INVALIDARG);
    *typeIdRef = 0;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!Js::ExternalObject::Is(instance))
    {
        return E_INVALIDARG;
    }

    *buffer = (void*)(((char*)instance) + sizeof(Js::CustomExternalObject));
    *typeIdRef = (JavascriptTypeId)Js::JavascriptOperators::GetTypeId(instance);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::DispExToVar(
    __in IDispatchEx* pdispex,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

#if DBG
    IUnknown* result = nullptr;
    Assert(FAILED(pdispex->QueryInterface(__uuidof(IInspectable), (void**)&result)));
    if (result)
    {
        result->Release();
    }
#endif
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        hr = DispatchHelper::MarshalIDispatchToJsVar(GetScriptSiteHolder()->GetScriptSiteContext(), pdispex,  instance);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToDispEx(
    __in Var instance,
    __out IDispatchEx** pdispex)
{
    IfNullReturnError(pdispex, E_INVALIDARG);
    *pdispex = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    if (Js::TaggedNumber::Is(instance))
    {
        return E_INVALIDARG;
    }

    if (Js::JavascriptOperators::GetTypeId(instance) == Js::TypeIds_HostDispatch)
    {
        IDispatch* pDispatch = static_cast<HostDispatch*>(instance)->GetDispatchNoRef();
        if (nullptr == pDispatch)
        {
            return E_NOINTERFACE;
        }
        return pDispatch->QueryInterface(__uuidof(IDispatchEx), (void**)pdispex);
    }
    Js::RecyclableObject* jsInstance = Js::RecyclableObject::FromVar(instance);

    JavascriptDispatch* jsDispatch = nullptr;
    // Collection will be disabled in VarDispEx because it could be called from projection reentrance as ASTA allows
    // QI/AddRef/Release to come back. During stub creation Trident might call VarToDispEx to return our wrapper
    // back to COM. we don't want GC to mess up the projection state (we might be called during weakref resolution time)
    Recycler* recycler = jsInstance->GetType()->GetScriptContext()->GetRecycler();
    Assert(!recycler->IsCollectionDisabled());
    Assert(!(recycler->IsInThreadFindRootsState() || recycler->IsInRefCountTrackingForProjection()));

    // We can not treat IInspectables as wrappable in an IDispatch pointer, so we should fail for WinRT objects.
    // IJavascriptOperations::QueryObjectInterface should be used to obtain a native pointer for these objects.
    IUnknown* result = nullptr;
    if (SUCCEEDED(jsInstance->QueryObjectInterface(__uuidof(IInspectable), (void**)&result)))
    {
        result->Release();
        return E_INVALIDARG;
    }
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (recycler->IsInThreadFindRootsState())
        {
            Recycler::AutoAllowAllocationDuringReentrance autoAllowAllocationDuringReentrance(recycler);
            jsDispatch = JavascriptDispatch::Create<false>((Js::DynamicObject*)jsInstance);
        }
        else
        {
            jsDispatch = JavascriptDispatch::Create<false>((Js::DynamicObject*)jsInstance);
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    if (SUCCEEDED(hr))
    {
        hr = jsDispatch->QueryInterface(__uuidof(IDispatchEx),(void**)pdispex);
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE ScriptEngineBase::InspectableUnknownToVar(
    __in IUnknown* unknown,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(this);
    return scriptEngine->InspectableUnknownToVarInternal(unknown, instance);
}


HRESULT STDMETHODCALLTYPE ScriptEngineBase::BuildDOMDirectFunction(
    __in Var signature,
    __in ScriptMethod entryPoint,
    __in PropertyId nameId,
    __in JavascriptTypeId prototypeTypeId,
    __in UINT64 flags,
    __out Var* jsFunction)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(jsFunction, E_INVALIDARG);
    *jsFunction = nullptr;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    if (nameId != 0 && !scriptContext->IsTrackedPropertyId(nameId))
    {
        return E_INVALIDARG;
    }
    *jsFunction = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *jsFunction = library->CreateExternalFunction((Js::ExternalMethod)entryPoint, nameId, signature, prototypeTypeId, flags);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetTypedObjectSlotAccessor(
    __in JavascriptTypeId typeId,
    __in PropertyId nameId,
    __in unsigned int slotIndex,
    __out_opt Var* getter,
    __out_opt Var* setter)
{
    return GetObjectSlotAccessor(typeId, nameId, slotIndex, nullptr, nullptr, getter, setter);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetObjectSlotAccessor(
    __in JavascriptTypeId typeId,
    __in PropertyId nameId,
    __in unsigned int slotIndex,
    __in ScriptMethod getterFallBackEntryPoint,
    __in ScriptMethod setterFallBackEntryPoint,
    __out_opt Var* getter,
    __out_opt Var* setter)
{
    if (getter != nullptr)
    {
        *getter = nullptr;
    }
    if (setter != nullptr)
    {
        *setter = nullptr;
    }

    HRESULT hr = NOERROR;
    if (slotIndex >= ActiveScriptExternalLibrary::DOM_BUILTIN_MAX_OBJECT_SLOT_COUNT)
    {
        return E_INVALIDARG;
    }
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (getter)
        {
            *getter = GetScriptSiteHolder()->GetDefaultSlotGetter(/* isObject */ true, typeId, nameId, slotIndex, getterFallBackEntryPoint);
        }
        if (setter)
        {
            *setter = GetScriptSiteHolder()->GetDefaultSlotSetter(/* isObject */ true, typeId, nameId, slotIndex, setterFallBackEntryPoint);
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetTypeSlotAccessor(
    __in JavascriptTypeId typeId,
    __in PropertyId nameId,
    __in unsigned int slotIndex,
    __in ScriptMethod getterFallBackEntryPoint,
    __in ScriptMethod setterFallBackEntryPoint,
    __out_opt Var* getter,
    __out_opt Var* setter)
{
    if (getter != nullptr)
    {
        *getter = nullptr;
    }
    if (setter != nullptr)
    {
        *setter = nullptr;
    }

    HRESULT hr = NOERROR;
    if (slotIndex >= ActiveScriptExternalLibrary::DOM_BUILTIN_MAX_TYPE_SLOT_COUNT)
    {
        return E_INVALIDARG;
    }
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (getter)
        {
            *getter = GetScriptSiteHolder()->GetDefaultSlotGetter(/* isObject */ false, typeId, nameId, slotIndex, getterFallBackEntryPoint);
        }
        if (setter)
        {
            *setter = GetScriptSiteHolder()->GetDefaultSlotSetter(/* isObject */ false, typeId, nameId, slotIndex, setterFallBackEntryPoint);
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateErrorObject(
    __in JsErrorType errorType,
    __in HRESULT hCode,
    __in LPCWSTR message,
    __out Var* errorObject)
{
    IfNullReturnError(errorObject, E_INVALIDARG);
    IfNullReturnError(message, E_INVALIDARG);
    *errorObject = nullptr;

    ErrorTypeEnum errorTypeInternal;
    HRESULT hr = ErrorTypeHelper::MapToInternal(errorType, errorTypeInternal);
    IfFailedReturn(hr);

    // If oure script site has closed, we can't create a new error, so throw a preallocated exception instead
    if (this->GetScriptSiteHolder() == nullptr)
    {
        *errorObject = threadContext->GetPendingOOMErrorObject();
        return S_OK;
    }

    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        Js::JavascriptError *pError = scriptContext->GetLibrary()->CreateExternalError(errorTypeInternal);

        size_t length = wcslen(message);
        char16* allocatedString = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), char16, length + 1);
        wmemcpy_s(allocatedString, length + 1, message, length);
        allocatedString[length] = _u('\0');

        Js::JavascriptError::SetErrorMessageProperties(pError, hCode, allocatedString, scriptContext);
        *errorObject = pError;
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(pError));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *errorObject = Js::JavascriptProxy::AutoProxyWrapper(*errorObject);
        }
#endif

    }
    END_TRANSLATE_OOM_TO_HRESULT_AND_EXCEPTION_OBJECT(hr, scriptContext, errorObject);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ReinitializeObject(__in Var instance, __in HTYPE type, __in BOOL keepProperties)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    if (!Js::ExternalObject::Is(instance) || (type == nullptr))
    {
        return E_INVALIDARG;
    }

    Js::ExternalType* externalType = (Js::ExternalType*)type;

    Js::ExternalObject* externalObject = Js::ExternalObject::FromVar(instance);
    if (!externalObject->IsExternal())
    {
        return E_INVALIDARG;
    }
    threadContext->ClearInlineCaches();
    threadContext->InvalidateAllProtoTypePropertyCaches();
    hr = externalObject->Reinitialize(externalType, keepProperties);
#if DBG
    if (externalObject == externalType->GetLibrary()->GetGlobalObject()->GetSecureDirectHostObject())
    {
        GetScriptSiteHolder()->CaptureReinitHostObjectTrace();
    }
#endif
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::UnrootScriptEngine()
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    Js::GlobalObject* globalObject = scriptContext->GetGlobalObject();
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    if (globalObject->GetDirectHostObject() == nullptr)
    {
        return E_UNEXPECTED;
    }

    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        threadContext->ClearInlineCaches();
        threadContext->InvalidateAllProtoTypePropertyCaches();

        Js::DynamicTypeHandler * typeHandler = globalObject->GetDynamicType()->GetTypeHandler();
        Js::RecyclableObject * undef = library->GetUndefined();
        uint slotCapacity = typeHandler->GetSlotCapacity();
        for (uint16 i = 0; i < slotCapacity; i++)
        {
            Js::PropertyId propertyId = typeHandler->GetPropertyId(scriptContext, (Js::PropertyIndex)i);
            threadContext->InvalidatePropertyGuards(propertyId);
            // Global object's type handler will continue to report the field as fixed if it was before,
            // but any existing code that had the old value hard-coded will be invalidated due to the line
            // above.  Any new requests for fixed fields will see undefined.
            globalObject->SetSlot(SetSlotArguments(Js::Constants::NoProperty, i, undef));
        }

        DebugOnly(Js::DynamicType * oldType = globalObject->GetDynamicType());
        globalObject->SetPrototype(library->GetNull());
        // Set prototype should have change the type if it is locked.
        Assert(!oldType->GetIsLocked() || globalObject->GetType() != oldType);
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ReleaseAndRethrowException(__in HRESULT hresultToThrow)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    if (hresultToThrow == SCRIPT_E_RECORDED || hresultToThrow == SCRIPT_E_PROPAGATE)
    {
        // release the current engine before rethrowing, because the caller cannot release
        // the current interface pointer.
        // SCRIPT_E_RECORDED indicates there is recorded exception in current thread;
        // SCRIPT_E_PROPAGATE should be called in dispatchmethod method when the error code
        // has been propagated back to the caller thread, and fastDOM needs to rethrow it
        // before returning to jscript.
        AssertMsg(scriptContext->HasRecordedException(), "no recorded exception to rethrow");
        AssertMsg(m_refCount > 1, "refcount too low in rethrow");
        Js::ScriptContext* localScriptContext = scriptContext;
        Release();

        // For SCRIPT_E_PROPAGATE, enter script, since RethrowExceptionObject, as it needs to fill exception info from HR,
        // will eventually call into JavascriptError::GetRuntimeError, which needs to be called inside script.
        // For SCRIPT_E_RECORDED we don't have to do that, but it's cleaner to do for both cases,
        // and it's OK as both call same path.
        BEGIN_ENTER_SCRIPT(localScriptContext, /*doCleanup*/ false, /*isCallRoot*/ false, /*hasCaller*/ true)
        {
            localScriptContext->RethrowRecordedException(nullptr);
        }
        END_ENTER_SCRIPT;
    }
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ChangeTypeFromVar(
    __in Var instance,
    __in VARTYPE vtNew,
    __out VARIANT *outVariant)
{
    IfNullReturnError(outVariant, E_INVALIDARG);
    ZeroMemory(outVariant, sizeof(outVariant));

    HRESULT hr = NOERROR;
    VARIANT srcVariant;
    Var varValue = nullptr;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    VariantInit(outVariant);
    Js::JavascriptHint hint = (vtNew == VT_BSTR) ? Js::JavascriptHint::HintString : Js::JavascriptHint::HintNumber;

    if (!Js::DynamicObject::Is(instance))
    {
        // let MarhshalJsVarToVariant handles the type checking.
        varValue = instance;
    }
    else
    {
        Js::DynamicObject* varSource = Js::DynamicObject::FromVar(instance);
        hr = GetScriptSiteHolder()->ExternalToPrimitive(varSource, hint, &varValue);
    }

    if (SUCCEEDED(hr))
    {
        hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varValue, &srcVariant, scriptContext);
    }

    if (SUCCEEDED(hr))
    {
        if (vtNew == VT_VARIANT)
        {
            // Caller asked for a VARIANT, and we have one, so let's just make and hand out a copy.
            // Note that result won't be VT_VARIANT, which would always have to be byref, which in turn would
            // hevily complicate code on both sides. This creates cleaner and simple contract.
            hr = VariantCopy(outVariant, &srcVariant);
        }
        else
        {
            ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(this);
            hr = scriptEngine->ChangeType(outVariant, &srcVariant, 0x409, vtNew);
        }
        VariantClear(&srcVariant);
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ChangeTypeToVar(
    __in VARIANT *inVariant,
    __out Var *instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(inVariant, instance, scriptContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetScriptType(
    __in Var instance,
    __out ScriptType *scriptType)
{
    IfNullReturnError(scriptType, E_INVALIDARG);
    *scriptType = ScriptType_Undefined;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    switch (Js::JavascriptOperators::GetTypeId(instance))
    {
        // review: is this correct?
    case Js::TypeIds_Undefined:
        *scriptType = ScriptType_Undefined;
        break;

    case Js::TypeIds_Null:
        *scriptType = ScriptType_Null;
        break;

    case Js::TypeIds_Integer:
        *scriptType = ScriptType_Int;
        break;


    case Js::TypeIds_Number:
    case Js::TypeIds_NumberObject:
        *scriptType = ScriptType_Number;
        break;

    case Js::TypeIds_Int64Number:
        *scriptType = ScriptType_Int64;
        break;

    case Js::TypeIds_UInt64Number:
        *scriptType = ScriptType_UInt64;
        break;

    case Js::TypeIds_Boolean:
    case Js::TypeIds_BooleanObject:
        *scriptType = ScriptType_Bool;
        break;

    case Js::TypeIds_Date:
    case Js::TypeIds_WinRTDate:
        *scriptType = ScriptType_Date;
        break;

    case Js::TypeIds_Symbol:
    case Js::TypeIds_SymbolObject:
        *scriptType = ScriptType_Symbol;
        break;

    case Js::TypeIds_WithScopeObject:
        AssertMsg(false, "WithScopeObjects should not be exposed");
        break;

    case Js::TypeIds_Error:
    case Js::TypeIds_Object:
    case Js::TypeIds_Array:
    case Js::TypeIds_NativeIntArray:
    case Js::TypeIds_CopyOnAccessNativeIntArray:
    case Js::TypeIds_NativeFloatArray:
    case Js::TypeIds_ES5Array:
    case Js::TypeIds_Function:
    case Js::TypeIds_GlobalObject:
    case Js::TypeIds_ModuleRoot:
    case Js::TypeIds_Map:
    case Js::TypeIds_Set:
    case Js::TypeIds_WeakMap:
    case Js::TypeIds_WeakSet:
    case Js::TypeIds_ArrayIterator:
    case Js::TypeIds_MapIterator:
    case Js::TypeIds_SetIterator:
    case Js::TypeIds_StringIterator:
    case Js::TypeIds_Generator:
    case Js::TypeIds_Promise:
    case Js::TypeIds_Arguments:
    case Js::TypeIds_ActivationObject:
    case Js::TypeIds_ArrayBuffer:
    case Js::TypeIds_Int8Array:
    case Js::TypeIds_Uint8Array:
    case Js::TypeIds_Uint8ClampedArray:
    case Js::TypeIds_Int16Array:
    case Js::TypeIds_Uint16Array:
    case Js::TypeIds_Int32Array:
    case Js::TypeIds_Uint32Array:
    case Js::TypeIds_Float32Array:
    case Js::TypeIds_Float64Array:
    case Js::TypeIds_Int64Array:
    case Js::TypeIds_Uint64Array:
    case Js::TypeIds_BoolArray:
    case Js::TypeIds_CharArray:
        *scriptType = ScriptType_Object;
        break;
    case Js::TypeIds_HostDispatch:
        // For hostdispatch wrapping normal VARIANT just for round tripping,
        // the type shouldn't be object. We should just fail it here.
        if ((static_cast<HostDispatch*>(instance))->GetDispatchNoRef())
        {
            *scriptType = ScriptType_Object;
        }
        else
        {
            return E_FAIL;
        }
        break;

    case Js::TypeIds_RegEx:
        *scriptType = ScriptType_RegularExpression;

    case Js::TypeIds_String:
    case Js::TypeIds_StringObject:
        *scriptType = ScriptType_String;
        break;

    case Js::TypeIds_Enumerator:
    case Js::TypeIds_VariantDate:
         return E_FAIL; //CJScript9Holder::ChangeTypeFromVar will take care of the conversion

    case Js::TypeIds_Proxy:
    {
        Js::JavascriptProxy* proxy = Js::JavascriptProxy::FromVar(instance);
        return GetScriptType(proxy->GetTarget(), scriptType);
    }

    default:
        {
            return E_FAIL;
        }
    }

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetServiceProvider(
        IServiceProvider** serviceProvider)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        *serviceProvider = nullptr;
        return hr;
    }
    return this->CreateServiceProvider(false, serviceProvider);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetServiceProviderOfCaller(
        IServiceProvider** serviceProvider)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        *serviceProvider = nullptr;
        return hr;
    }
    return this->CreateServiceProvider(true, serviceProvider);
}

HRESULT ScriptEngineBase::CreateServiceProvider(bool asCaller, IServiceProvider** serviceProvider)
{
    HRESULT hr;
    Assert(GetScriptSiteHolder() != nullptr);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        *serviceProvider = nullptr;
        return hr;
    }
    DispatchExCaller * dispatchExCaller = nullptr;
    hr = GetScriptSiteHolder()->GetDispatchExCaller(&dispatchExCaller);
    if (SUCCEEDED(hr) && !asCaller)
    {
        DispatchExCaller* newCaller = HeapNewNoThrow(DispatchExCaller, dispatchExCaller);
        if (nullptr == newCaller)
        {
            hr = E_OUTOFMEMORY;
        }
        GetScriptSiteHolder()->ReleaseDispatchExCaller(dispatchExCaller);
        dispatchExCaller = newCaller;
    }

    if (SUCCEEDED(hr))
    {
        hr = dispatchExCaller->QueryInterface(__uuidof(IServiceProvider), (void**)serviceProvider);
        dispatchExCaller->Release();
    }
    return hr;

}

HRESULT ScriptEngineBase::ReleaseServiceProviderOfCaller(IServiceProvider * serviceProvider)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    DispatchExCaller * dispatchExCaller = (DispatchExCaller *)serviceProvider;
    dispatchExCaller->GetScriptSite()->ReleaseDispatchExCaller(dispatchExCaller);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetNull(
    __out Var *nullValue)
{
    IfNullReturnError(nullValue, E_INVALIDARG);
    *nullValue = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    *nullValue = GetScriptContext()->GetLibrary()->GetNull();
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetUndefined(
    __out Var *undefinedValue)
{
    IfNullReturnError(undefinedValue, E_INVALIDARG);
    *undefinedValue = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    *undefinedValue = GetScriptContext()->GetLibrary()->GetUndefined();
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreatePixelArray(
    __in UINT length,
    __out Var *instance)
{
    return CreateTypedArray(Uint8ClampedArray, nullptr, length, instance);
}

// Sets *ppBuffer to point to the start of the array in RGBA format.
// *ppBuffer will be set to nullptr if the pixel array was allocated with zero bytes.

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetPixelArrayBuffer(
    __in Var instance,
    __deref_out_bcount(*pBufferLength) BYTE **ppBuffer,
    __out UINT *pBufferLength)
{
    return GetTypedArrayBuffer(instance, ppBuffer, pBufferLength, nullptr, nullptr);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateArrayBuffer(
    __in UINT length,
    __out Var *instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();
    *instance = library->GetNull();

    // ArrayBuffer constructor may throw runtime exceptions.
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *instance = library->CreateArrayBuffer(length);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*instance));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *instance = Js::JavascriptProxy::AutoProxyWrapper(*instance);
        }
#endif
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateArrayBufferFromBuffer(
    __in BYTE * buffer,
    __in UINT length,
    __out Var *instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();
    *instance = library->GetNull();

    // ArrayBuffer constructor may throw runtime exceptions.
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *instance = library->CreateProjectionArraybuffer(buffer, length);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*instance));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *instance = Js::JavascriptProxy::AutoProxyWrapper(*instance);
        }
#endif
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateTypedArray(
    __in TypedArrayType typedArrayType,
    __in_opt Var baseVar,
    __in UINT length,
    __out Var* instance)
{
    IfNullReturnError(instance, E_INVALIDARG);
    *instance = nullptr;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    if (length != 0 && baseVar != nullptr)
    {
        return E_INVALIDARG;
    }

    Js::JavascriptLibrary* javascriptLibrary = scriptContext->GetLibrary();
    *instance = javascriptLibrary->GetNull();

    Js::JavascriptFunction* constructorFunc = nullptr;

    Js::Var values[2] = {   javascriptLibrary->GetUndefined(),   baseVar};

    if (baseVar == nullptr)
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            values[1] = Js::JavascriptNumber::ToVar(length, scriptContext);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr)
    }
    Js::CallInfo info(Js::CallFlags_New, 2);

    Js::Arguments args(info, values);
    switch (typedArrayType)
    {
    case Int8Array:
        constructorFunc = javascriptLibrary->GetInt8ArrayConstructor();
        break;
    case Uint8Array:
        constructorFunc = javascriptLibrary->GetUint8ArrayConstructor();
        break;
    case Uint8ClampedArray:
        constructorFunc = javascriptLibrary->GetUint8ClampedArrayConstructor();
        break;
    case Int16Array:
        constructorFunc = javascriptLibrary->GetInt16ArrayConstructor();
        break;
    case Uint16Array:
        constructorFunc = javascriptLibrary->GetUint16ArrayConstructor();
        break;
    case Int32Array:
        constructorFunc = javascriptLibrary->GetInt32ArrayConstructor();
        break;
    case Uint32Array:
        constructorFunc = javascriptLibrary->GetUint32ArrayConstructor();
        break;
    case Float32Array:
        constructorFunc = javascriptLibrary->GetFloat32ArrayConstructor();
        break;
    case Float64Array:
        constructorFunc = javascriptLibrary->GetFloat64ArrayConstructor();
        break;
    default:
        AssertMsg(0, "invalid typed array type");
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        hr = ScriptSite::CallRootFunction(constructorFunc, args, nullptr, instance);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*instance));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            *instance = Js::JavascriptProxy::AutoProxyWrapper(*instance);
        }
#endif
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreatePromise(
    __deref_out Var* promise,
    __deref_out Var* resolveFunc,
    __deref_out Var* rejectFunc)
{
    IfNullReturnError(promise, E_INVALIDARG);
    IfNullReturnError(resolveFunc, E_INVALIDARG);
    IfNullReturnError(rejectFunc, E_INVALIDARG);

    *promise = nullptr;
    *resolveFunc = nullptr;
    *rejectFunc = nullptr;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    Js::JavascriptLibrary* javascriptLibrary = scriptContext->GetLibrary();

    *promise = javascriptLibrary->GetNull();
    *resolveFunc = javascriptLibrary->GetNull();
    *rejectFunc = javascriptLibrary->GetNull();

    if (!scriptContext->GetConfig()->IsES6PromiseEnabled())
    {
        return E_NOTIMPL;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        IGNORE_STACKWALK_EXCEPTION(scriptContext);
        Js::JavascriptPromise* p = javascriptLibrary->CreatePromise();
        Js::JavascriptPromiseResolveOrRejectFunction* resolve;
        Js::JavascriptPromiseResolveOrRejectFunction* reject;

        Js::JavascriptPromise::InitializePromise(p, &resolve, &reject, scriptContext);

        *promise = p;
        *resolveFunc = resolve;
        *rejectFunc = reject;
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    return hr;
}
HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsureArrayPrototypeForEachFunction(__out Var* func)
{
    return EnsureFunctionValidation(func, [] (Js::ScriptContext *scriptContext) -> Var {
        return scriptContext->GetLibrary()->EnsureArrayPrototypeForEachFunction();
    });
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsureArrayPrototypeKeysFunction(__out Var* func)
{
    return EnsureFunctionValidation(func, [](Js::ScriptContext *scriptContext) -> Var {
        return scriptContext->GetLibrary()->EnsureArrayPrototypeKeysFunction();
    });
}
HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsureArrayPrototypeEntriesFunction(__out Var* func)
{
    return EnsureFunctionValidation(func, [](Js::ScriptContext *scriptContext) -> Var {
        return scriptContext->GetLibrary()->EnsureArrayPrototypeEntriesFunction();
    });
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsureArrayPrototypeValuesFunction(__out Var* func)
{
    return EnsureFunctionValidation(func, [](Js::ScriptContext *scriptContext) -> Var {
        return scriptContext->GetLibrary()->EnsureArrayPrototypeValuesFunction();
    });
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsurePromiseResolveFunction(
    __out Var* resolveFunc)
{
    IfNullReturnError(resolveFunc, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *resolveFunc = scriptContext->GetLibrary()->EnsurePromiseResolveFunction();
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsurePromiseThenFunction(
    __out Var* thenFunc)
{
    IfNullReturnError(thenFunc, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *thenFunc = scriptContext->GetLibrary()->EnsurePromiseThenFunction();
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

        return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsureJSONStringifyFunction(
    __out Var* jsonStringifyFunc)
{
    IfNullReturnError(jsonStringifyFunc, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *jsonStringifyFunc = scriptContext->GetLibrary()->EnsureJSONStringifyFunction();
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

        return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::EnsureObjectFreezeFunction(
    __out Var* objFreezeFunc)
{
    IfNullReturnError(objFreezeFunc, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *objFreezeFunc = scriptContext->GetLibrary()->EnsureObjectFreezeFunction();
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateWeakMap(__out Var *mapInstance)
{
    IfNullReturnError(mapInstance, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (SUCCEEDED(hr))
    {
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            *mapInstance = scriptContext->GetLibrary()->CreateWeakMap();
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::WeakMapHas(Var mapInstance, Var key, __out bool *has)
{
    IfNullReturnError(mapInstance, E_INVALIDARG);
    IfNullReturnError(key, E_INVALIDARG);
    IfNullReturnError(has, E_INVALIDARG);
    *has = false;
    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }
    
    if (!Js::JavascriptWeakMap::Is(mapInstance) || Js::JavascriptWeakMap::FromVar(mapInstance)->GetScriptContext() != scriptContext)
    {
        return E_INVALIDARG;
    }

    if (Js::JavascriptOperators::IsObject(key) && Js::JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
    {
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            Js::JavascriptWeakMap *map = Js::JavascriptWeakMap::FromVar(mapInstance);
            Var key1 = Js::CrossSite::MarshalVar(scriptContext, key);
            *has = map->Has(Js::DynamicObject::FromVar(key1));
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::WeakMapSet(Var mapInstance, Var key, Var value)
{
    IfNullReturnError(mapInstance, E_INVALIDARG);
    IfNullReturnError(key, E_INVALIDARG);
    IfNullReturnError(value, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    if (!Js::JavascriptWeakMap::Is(mapInstance)
        || Js::JavascriptWeakMap::FromVar(mapInstance)->GetScriptContext() != scriptContext
        || !Js::JavascriptOperators::IsObject(key)
        || Js::JavascriptOperators::GetTypeId(key) == TypeIds_HostDispatch)
    {
        return E_INVALIDARG;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        Js::JavascriptWeakMap *map = Js::JavascriptWeakMap::FromVar(mapInstance);
        Var key1 = Js::CrossSite::MarshalVar(scriptContext, key);
        map->Set(Js::DynamicObject::FromVar(key1), Js::CrossSite::MarshalVar(scriptContext, value));
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::WeakMapGet(Var mapInstance, Var key, __out Var *value, __out bool *found)
{
    IfNullReturnError(mapInstance, E_INVALIDARG);
    IfNullReturnError(key, E_INVALIDARG);
    IfNullReturnError(value, E_INVALIDARG);
    IfNullReturnError(found, E_INVALIDARG);
    *value = nullptr;
    *found = false;

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    if (!Js::JavascriptWeakMap::Is(mapInstance) || Js::JavascriptWeakMap::FromVar(mapInstance)->GetScriptContext() != scriptContext)
    {
        return E_INVALIDARG;
    }

    if (Js::JavascriptOperators::IsObject(key) && Js::JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
    {
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            Js::JavascriptWeakMap *map = Js::JavascriptWeakMap::FromVar(mapInstance);
            Var value1 = nullptr;
            *found = map->Get(Js::DynamicObject::FromVar(key), &value1);
            if (*found)
            {
                *value = Js::CrossSite::MarshalVar(scriptContext, value1);
            }
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::WeakMapDelete(Var mapInstance, Var key, __out bool *result)
{
    IfNullReturnError(mapInstance, E_INVALIDARG);
    IfNullReturnError(key, E_INVALIDARG);
    IfNullReturnError(result, E_INVALIDARG);
    *result = false;
    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    if (!Js::JavascriptWeakMap::Is(mapInstance) || Js::JavascriptWeakMap::FromVar(mapInstance)->GetScriptContext() != scriptContext)
    {
        return E_INVALIDARG;
    }

    if (Js::JavascriptOperators::IsObject(key) && Js::JavascriptOperators::GetTypeId(key) != TypeIds_HostDispatch)
    {
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
        {
            Js::JavascriptWeakMap *map = Js::JavascriptWeakMap::FromVar(mapInstance);
            Var key1 = Js::CrossSite::MarshalVar(scriptContext, key);
            *result = map->Delete(Js::DynamicObject::FromVar(key1));
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateIteratorCreatorFunction(
    JavascriptTypeId typeId,
    Js::JavascriptMethod entryPoint,
    uint byteCount,
    Var prototypeForIterator,
    InitIteratorFunction initFunction,
    NextFunction nextFunction,
    __out Var* func)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(func, E_INVALIDARG);
    IfNullReturnError(prototypeForIterator, E_INVALIDARG);
    IfNullReturnError(initFunction, E_INVALIDARG);
    IfNullReturnError(nextFunction, E_INVALIDARG);
    *func = nullptr;
    if (byteCount < sizeof(void*))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = VerifyOnEntry();
        if (SUCCEEDED(hr))
        {
            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                *func = Js::ExternalIteratorCreatorFunction::CreateFunction(scriptContext->GetLibrary(), typeId,
                entryPoint, byteCount, prototypeForIterator, initFunction, nextFunction);
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateIteratorEntriesFunction(JavascriptTypeId typeId,
    uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction, __out Var* func)
{
    return CreateIteratorCreatorFunction(typeId,
        Js::ExternalIteratorCreatorFunction::EntryExternalEntries,
        byteCount,
        prototypeForIterator,
        initFunction,
        nextFunction,
        func);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateIteratorKeysFunction(JavascriptTypeId typeId,
    uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction, __out Var* func)
{
    return CreateIteratorCreatorFunction(typeId,
        Js::ExternalIteratorCreatorFunction::EntryExternalKeys,
        byteCount,
        prototypeForIterator,
        initFunction,
        nextFunction,
        func);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateIteratorValuesFunction(JavascriptTypeId typeId,
    uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction, __out Var* func)
{
    return CreateIteratorCreatorFunction(typeId,
        Js::ExternalIteratorCreatorFunction::EntryExternalValues,
        byteCount,
        prototypeForIterator,
        initFunction,
        nextFunction,
        func);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateIteratorNextFunction(JavascriptTypeId typeId, __out Var* func)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(func, E_INVALIDARG);

    *func = nullptr;
    hr = VerifyOnEntry();
    if (SUCCEEDED(hr))
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            *func = Js::CustomExternalIterator::CreateNextFunction(scriptContext->GetLibrary(), typeId);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }

    return hr;
}

#include "Library\JSONParser.h"

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ParseJson(
    __in LPCWSTR str,
    __in UINT length,
    __deref_out Var* var)
{
    IfNullReturnError(str, E_INVALIDARG);
    IfNullReturnError(var, E_INVALIDARG);

    HRESULT hr = S_OK;
    hr = VerifyOnEntry();

    if (FAILED(hr))
    {
        return hr;
    }

    *var = scriptContext->GetLibrary()->GetNull();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        // alignment required because of the union in JSONParser::m_token
        __declspec (align(8)) JSON::JSONParser parser(scriptContext, nullptr);
        *var = parser.Parse(str, length);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    return hr;
}

void GetArrayTypeAndElementSize(__in Var instance, __out_opt TypedArrayType* typedArrayType, __out_opt INT* elementSize, __out_opt Js::ArrayBuffer **buffer)
{
    if (!typedArrayType && !elementSize && !buffer)
    {
        return;
    }

    TypedArrayType arrayType;
    INT tmpElementSize;
    Js::ArrayBuffer *tmpBuffer = nullptr;
    bool isTypedArray = true;
    Js::DataView* dView = nullptr;
    switch (Js::JavascriptOperators::GetTypeId(instance))
    {
    case Js::TypeIds_Int16Array:
        arrayType = Int16Array;
        tmpElementSize = sizeof(int16);
        break;
    case Js::TypeIds_Int8Array:
        arrayType = Int8Array;
        tmpElementSize = sizeof(int8);
        break;
    case Js::TypeIds_Uint8Array:
        arrayType = Uint8Array;
        tmpElementSize = sizeof(uint8);
        break;
    case Js::TypeIds_Uint8ClampedArray:
        arrayType = Uint8ClampedArray;
        tmpElementSize = sizeof(uint8);
        break;
    case Js::TypeIds_Uint16Array:
        arrayType = Uint16Array;
        tmpElementSize = sizeof(uint16);
        break;
    case Js::TypeIds_Int32Array:
        arrayType = Int32Array;
        tmpElementSize = sizeof(int32);
        break;
    case Js::TypeIds_Uint32Array:
        arrayType = Uint32Array;
        tmpElementSize = sizeof(uint32);
        break;
    case Js::TypeIds_Float32Array:
        arrayType = Float32Array;
        tmpElementSize = sizeof(float);
        break;
    case Js::TypeIds_Float64Array:
        arrayType = Float64Array;
        tmpElementSize = sizeof(double);
        break;
    case Js::TypeIds_ArrayBuffer:
        arrayType = (TypedArrayType)-1;
        tmpElementSize = -1;
        tmpBuffer = Js::ArrayBuffer::FromVar(instance);
        isTypedArray = false;
        break;
    case Js::TypeIds_DataView:
        arrayType = (TypedArrayType)-1;
        tmpElementSize = -1;
        dView = Js::DataView::FromVar(instance);
        tmpBuffer = dView->GetArrayBuffer()->GetAsArrayBuffer();
        isTypedArray = false;
        break;
    default:
        AssertMsg(FALSE, "invalid typed array type");
        arrayType = (TypedArrayType)-1;
        tmpElementSize = 0;
        isTypedArray = false;
        break;
    }
    if (typedArrayType)
    {
        *typedArrayType = arrayType;
    }
    if (elementSize)
    {
        *elementSize = tmpElementSize;
    }
    if (buffer)
    {
        *buffer = isTypedArray ? Js::TypedArrayBase::FromVar(instance)->GetArrayBuffer()->GetAsArrayBuffer() : tmpBuffer;
    }
}
HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetTypedArrayBuffer(
    __in Var instance,
    __deref_out_bcount(*pBufferLength) BYTE **ppBuffer,
    __out UINT *pBufferLength,
    __out_opt TypedArrayType* typedArrayType,
    __out_opt INT* elementSize)
{
    IfNullReturnError(ppBuffer, E_INVALIDARG);
    *ppBuffer = nullptr;
    IfNullReturnError(pBufferLength, E_INVALIDARG);
    *pBufferLength = 0;
    if (typedArrayType != nullptr)
    {
        *typedArrayType = Int8Array;
    }
    if (elementSize != nullptr)
    {
        *elementSize = 0;
    }

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
#if ENABLE_DEBUG_CONFIG_OPTIONS
    // unwrap the autoproxy'ed proxy to make the call go through.
    if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag) && Js::JavascriptProxy::Is(instance))
    {
        instance = Js::JavascriptProxy::FromVar(instance)->GetTarget();
    }
#endif
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        Js::ArrayBuffer* arrayBuffer = nullptr;
        uint32 offset = 0;
        uint32 length = 0;
        hr = Js::TypedArrayBase::GetBuffer(instance, &arrayBuffer, &offset, &length);
        if (SUCCEEDED(hr))
        {
            *ppBuffer = arrayBuffer->GetBuffer() + offset;
            *pBufferLength = length;

            GetArrayTypeAndElementSize(instance, typedArrayType, elementSize, nullptr);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::DetachTypedArrayBuffer(
    __in Var instance,
    __deref_out_bcount(*pBufferLength) BYTE** ppBuffer,
    __out UINT* pBufferLength,
    __out TypedArrayBufferAllocationType * pAllocationType,
    __out_opt TypedArrayType* typedArrayType,
    __out_opt INT* elementSize)
{
    *ppBuffer = nullptr;
    *pBufferLength = 0;
    *pAllocationType = TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_Heap;
    if (typedArrayType != nullptr)
    {
        *typedArrayType = Int8Array;
    }
    if (elementSize != nullptr)
    {
        *elementSize = 0;
    }

    HRESULT hr = S_OK;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

#if ENABLE_DEBUG_CONFIG_OPTIONS
    // unwrap the autoproxy'ed proxy to make the call go through.
    if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag) && Js::JavascriptProxy::Is(instance))
    {
        instance = Js::JavascriptProxy::FromVar(instance)->GetTarget();
    }
#endif
    Js::ArrayBuffer* arrayBufferInstance = nullptr;
    GetArrayTypeAndElementSize(instance, typedArrayType, elementSize, &arrayBufferInstance);

    if (arrayBufferInstance->IsDetached())
    {
        return E_FAIL;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        AutoDiscardPTR<Js::ArrayBufferDetachedStateBase> state(arrayBufferInstance->DetachAndGetState());

        switch (state->allocationType)
        {
        case Js::ArrayBufferAllocationType::Heap:
            *pAllocationType = TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_Heap;
            break;
        case Js::ArrayBufferAllocationType::MemAlloc:
            *pAllocationType = TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_MemAlloc;
            break;
        case Js::ArrayBufferAllocationType::CoTask:
            *pAllocationType = TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_CoTask;
            break;
        default:
            AssertMsg(false, "Unknown allocationType of ArrayBufferDetachedStateBase ");
        }

        *ppBuffer = state->buffer;
        *pBufferLength = state->bufferLength;
        state->MarkAsClaimed();
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr)

    return hr;
}


HRESULT STDMETHODCALLTYPE ScriptEngineBase::FreeDetachedTypedArrayBuffer(__in BYTE * pBuffer,
    __in UINT bufferLength,
    __in TypedArrayBufferAllocationType allocationType)
{
    switch (allocationType)
    {
    case TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_Heap:
        free(pBuffer);
        break;
    case TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_MemAlloc:
        VirtualFree((LPVOID)pBuffer, 0, MEM_RELEASE);
        break;
    case TypedArrayBufferAllocationType::TypedArrayBufferAllocationType_CoTask:
        CoTaskMemFree(pBuffer);
        break;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateRegex(
    __in LPCWSTR pattern,
    __in UINT patternLength,
    __in RegexFlags flags,
    __deref_out Var *regex)
{
    IfNullReturnError(regex, E_INVALIDARG);
    *regex = nullptr;

    HRESULT hr = S_OK;
    IfNullReturnError(pattern, E_INVALIDARG);
    if(patternLength > MaxCharCount)
    {
        return E_INVALIDARG;
    }

    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        IGNORE_STACKWALK_EXCEPTION(scriptContext);
        *regex =
            Js::JavascriptRegExp::CreateRegEx(
            pattern,
            patternLength,
            static_cast<UnifiedRegex::RegexFlags>(flags),
            scriptContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::RegexTest(
    __in Var regex,
    __in LPCWSTR input,
    __in UINT inputLength,
    __in BOOL mustMatchEntireInput,
    __out BOOL *matched)
{
    IfNullReturnError(matched, E_INVALIDARG);
    *matched = FALSE;

    HRESULT hr = S_OK;
    IfNullReturnError(regex, E_INVALIDARG);
    IfNullReturnError(input, E_INVALIDARG);
    if(inputLength > MaxCharCount)
    {
        return E_INVALIDARG;
    }
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }
    if(!Js::JavascriptRegExp::Is(regex))
    {
        return E_INVALIDARG;
    }

    // Errors in regex execution are not expected but we need to safeguard anyway for OOM, stack overflow, etc.
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    BEGIN_JS_RUNTIME_CALLROOT_EX(scriptContext, false)
    {
        IGNORE_STACKWALK_EXCEPTION(scriptContext);

        *matched =
            mustMatchEntireInput ?
            Js::RegexHelper::RegexTest_NonScript<true>(scriptContext, Js::JavascriptRegExp::FromVar(regex), input, inputLength) :
            Js::RegexHelper::RegexTest_NonScript<false>(scriptContext, Js::JavascriptRegExp::FromVar(regex), input, inputLength);
    }
    END_JS_RUNTIME_CALL(scriptContext)
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

    return hr;
}

HRESULT ScriptEngineBase::HandleSCAException(
    Js::JavascriptExceptionObject* exceptionObject, Js::ScriptContext* scriptContext, IServiceProvider* pspCaller)
{
    HRESULT hr = NOERROR;
    GET_RUNTIME_ERROR(hr, exceptionObject);

    // Host will handle these private SCA error codes.
    if (hr == E_SCA_UNSUPPORTED
        || hr == E_SCA_NEWVERSION
        || hr == E_SCA_DATACORRUPT
        || hr == E_SCA_TRANSFERABLE_NEUTERED
        || hr == E_SCA_TRANSFERABLE_UNSUPPORTED)
    {
        return hr;
    }

    return ScriptSite::HandleJavascriptException(exceptionObject, scriptContext, pspCaller);
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::Serialize(
    __in ISCAContext* context,
    __in Var instance,
    __in_xcount(cTransferableVars) Var* transferableVars,
    __in UINT cTransferableVars,
    __in IStream* pOutSteam,
    __in IServiceProvider* serviceProvider)
{
    HRESULT hr = S_OK;

    IfNullReturnError(context, E_INVALIDARG);
    IfNullReturnError(pOutSteam, E_INVALIDARG);

    // Effective transferableVars to be passed to SCA
    Var* effectiveTransferableVars = nullptr;
    UINT cEffectiveTransferableVars = 0;

    if (cTransferableVars > 0)
    {
        IfNullReturnError(transferableVars, E_INVALIDARG);

        SCAContextType contextType;
        IfFailedReturn(context->GetContext(&contextType));

        switch (contextType)
        {
            case SCAContext_SameThread:
            case SCAContext_CrossThread:
                effectiveTransferableVars = transferableVars;
                cEffectiveTransferableVars = cTransferableVars;
                break;

            case SCAContext_CrossProcess:
                break;  // no transfer, but will neuter

            default:  // otherwise ignore the list (no transfer, no neuter)
                Assert(contextType == SCAContext_Persist);
                transferableVars = nullptr;
                cTransferableVars = 0;
                break;
        }
    }

    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    // Cache the script context locally in case the script engine gets
    // pulled out underneath us
    Js::ScriptContext* scriptContext = this->scriptContext;

    CComPtr<Js::TransferablesHolder> transferableHolder;

    for (uint i = 0; i < cTransferableVars; i++)
    {
        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(transferableVars[i]);
        if(typeId != TypeIds_ArrayBuffer && !Js::CustomExternalObject::Is(transferableVars[i]))
        {
            AssertMsg(false, "These should have been filtered out by the host.");
            return E_SCA_TRANSFERABLE_UNSUPPORTED;
        }

        if (Js::JavascriptOperators::IsObjectDetached(transferableVars[i]))
        {
            return E_SCA_TRANSFERABLE_NEUTERED;
        }
    }

    transferableHolder = HeapNewNoThrow(Js::TransferablesHolder, cTransferableVars);
    if (transferableHolder == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    
    Js::JavascriptExceptionObject *caughtExceptionObject = nullptr;
    AutoCallerPointer callerPointer(GetScriptSiteHolder(), serviceProvider);


    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        BEGIN_ENTER_SCRIPT(scriptContext, true, /*isCallRoot*/ false, /*hasCaller*/serviceProvider != nullptr)
        try
        {
            Js::StreamWriter writer(scriptContext, pOutSteam);
            Js::SCASerializationEngine::Serialize(context, instance, &writer,
                effectiveTransferableVars, cEffectiveTransferableVars, transferableHolder->GetSharedContentsList()); // Use effective transferableVars
            writer.Flush(); // Flush bufferred content to output stream

            // Always detach all supplied transferable vars
            if (transferableHolder)
            {
                transferableHolder->DetachAll(transferableVars);
            }

            if (cEffectiveTransferableVars > 0 // Transferred
                || transferableHolder->GetSharedContentsList()->Count()>0)  // Shared
            {
                hr = context->SetDependentObject(transferableHolder);
                if (FAILED(hr))
                {
                    goto LReturn;
                }
            }
            else  // We did not transfer any vars, manually cleanup
            {
                transferableHolder.Release();
            }
        }
        catch (const Js::JavascriptException& err)
        {
            caughtExceptionObject = err.GetAndClear();
        }
        END_ENTER_SCRIPT

        if (caughtExceptionObject != nullptr)
        {
            caughtExceptionObject = caughtExceptionObject->CloneIfStaticExceptionObject(scriptContext);
        }
    }
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr)
    CATCH_UNHANDLED_EXCEPTION(hr)

    if (caughtExceptionObject != nullptr)
    {
        hr = HandleSCAException(caughtExceptionObject, scriptContext, serviceProvider);
    }

LReturn:

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::Deserialize(
    __in ISCAContext* context,
    __in IStream* pInSteam,
    __in IServiceProvider* serviceProvider,
    __out Var* pValue)
{
    IfNullReturnError(context, E_INVALIDARG);
    IfNullReturnError(pInSteam, E_INVALIDARG);
    IfNullReturnError(pValue, E_POINTER);
    *pValue = nullptr;

    HRESULT hr = S_OK;

    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        *pValue = nullptr;
        return hr;
    }

    // Cache the script context locally in case the script engine gets
    // pulled out underneath us
    Js::ScriptContext* scriptContext = this->scriptContext;

    CComPtr<IUnknown> dependentObject(nullptr);
    CComPtr<Js::TransferablesHolder> transferableHolder = nullptr;

    hr = context->GetDependentObject(&dependentObject);

    if (FAILED(hr))
    {
        return hr;
    }

    if ((IUnknown *)dependentObject != nullptr)
    {
        hr = dependentObject->QueryInterface(Js::CLSID_TransferablesHolder, reinterpret_cast<void **>(&transferableHolder));
        if (FAILED(hr))
        {
            AssertMsg(false, "This shouldn't be failing.");
            return hr;
        }
    }

    AutoCallerPointer callerPointer(GetScriptSiteHolder(), serviceProvider);
    Js::JavascriptExceptionObject *caughtExceptionObject = nullptr;
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        BEGIN_ENTER_SCRIPT(scriptContext, true, /*isCallRoot*/ false, /*hasCaller*/serviceProvider != nullptr)
        try
        {
            AutoLeaveScriptPtr<ISCAHost> pSCAHost(scriptContext);
            ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(this);

            scriptEngine->GetScriptSite(__uuidof(ISCAHost), (void**)&pSCAHost); // OK not to support ISCAHost

            Js::StreamReader reader(scriptContext, pInSteam);
            *pValue = Js::SCADeserializationEngine::Deserialize(pSCAHost, context, &reader, transferableHolder);
        }
        catch (const Js::JavascriptException& err)
        {
            caughtExceptionObject = err.GetAndClear();
        }
        END_ENTER_SCRIPT

        if (caughtExceptionObject != nullptr)
        {
            caughtExceptionObject = caughtExceptionObject->CloneIfStaticExceptionObject(scriptContext);
        }
    }
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr)
    CATCH_UNHANDLED_EXCEPTION(hr)

    if (caughtExceptionObject != nullptr)
    {
        hr = HandleSCAException(caughtExceptionObject, scriptContext, serviceProvider);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::Discard(
    __in ISCAContext* context,
    __in IStream* pInSteam,
    __in IServiceProvider* serviceProvider)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IsVarFunctionObject(
    __in Var instance,
    __out BOOL *result)
{
    IfNullReturnError(result, E_INVALIDARG);
    *result = FALSE;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        return hr;
    }

    *result = (Js::JavascriptOperators::GetTypeId(instance) == Js::TypeIds_Function);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IsFunctionObject(
    __in IDispatch* pDisp,
    __out BOOL *result)
{
    IfNullReturnError(result, E_INVALIDARG);
    *result = FALSE;

    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    IJavascriptDispatchLocalProxy *pProxy = nullptr;
    hr = pDisp->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void**)&pProxy);
    if (S_OK == hr)
    {
        JavascriptDispatch* javascriptDispatch = static_cast<JavascriptDispatch*>(pProxy);
        Js::DynamicObject *obj = javascriptDispatch->GetObject();
        if (obj == nullptr)
        {
            hr = E_ACCESSDENIED;
        }
        else
        {
            Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(obj);
            *result = (typeId == Js::TypeIds_Function);
        }
        pProxy->Release();
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IsHostDispatch(
    __in Var instance,
    __out BOOL *result)
{
    IfNullReturnError(result, E_INVALIDARG);
    *result = FALSE;

    HRESULT hr = NOERROR;
    IfNullReturnError(instance, E_INVALIDARG);

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    *result = (Js::JavascriptOperators::GetTypeId(instance) == Js::TypeIds_HostDispatch);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::IsObjectCallable(Var obj, BOOL* isCallable)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(obj, E_INVALIDARG);
    IfNullReturnError(isCallable, E_INVALIDARG);

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    *isCallable = Js::JavascriptConversion::IsCallable(obj);

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::InvalidateHostObjects()
{
    HRESULT hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    scriptContext->InvalidateHostObjects();

    return hr;
}


HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetCurrentSourceInfo(
    __out LPCWSTR* url,
    __out ULONG* line,
    __out ULONG* column)
{
    if (!CONFIG_FLAG(CurrentSourceInfo)) // Reject if feature disabled
    {
        return E_NOTIMPL;
    }

    IfNullReturnError(url, E_POINTER);
    *url = nullptr;
    IfNullReturnError(line, E_POINTER);
    *line = 0;
    IfNullReturnError(column, E_POINTER);
    *column = 0;

    HRESULT hr = NOERROR;


    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    Js::JavascriptFunction* func;
    uint32 byteCodeOffset;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (Js::JavascriptStackWalker::GetCaller(&func, &byteCodeOffset, scriptContext))
        {
            Js::FunctionBody* body = func->GetFunctionBody();
            if (!body->GetSourceContextInfo()->IsDynamic()
                && !body->GetUtf8SourceInfo()->GetIsLibraryCode())
            {
                *url = body->GetSourceName();

                LONG characterOffset;
                body->GetLineCharOffset(byteCodeOffset, line, &characterOffset);
                *column = characterOffset;
            }
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr)

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ClearRecordedException()
{
    HRESULT hr;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    scriptContext->GetThreadContext()->SetRecordedException(nullptr);
    return NOERROR;
}


/// operationID : An ID (or cookie), which will be emitted in the stack trace payload.
///             Chakra will not validate the uniqueness of this ID
/// maxFrameCount : This param limits the number of frames to walked. (To get full stack trace pass JSCRIPT_FULL_STACKTRACE).
HRESULT STDMETHODCALLTYPE ScriptEngineBase::EmitStackTraceEvent(__in UINT64 operationID, __in USHORT maxFrameCount)
{
    HRESULT hr = NOERROR;
#ifdef ENABLE_JS_ETW
    if (IS_JS_ETW(EventEnabledJSCRIPT_STACKTRACE()) || PHASE_TRACE1(Js::StackFramesEventPhase))
    {
        if (!this->scriptContext->IsRunningScript())
        {
            // Not on script execution, early bailout.
            return S_OK;
        }

        hr = VerifyOnEntry(TRUE);
        if (SUCCEEDED(hr))
        {
            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                this->scriptContext->EmitStackTraceEvent(operationID, maxFrameCount, false /*emitV2AsyncStackEvent*/);
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr)
        }
    }
#endif
    return hr;
}

/// Convert a Var to a native array with given type. The algorithm is like:
/// ulong length = ToLength(arrayObject); for (i = 0; i < length; i++) {arraybuffer[i] = ToGivenType(arrayObject[i], valueType); }
/// arrayObject         : The object to be treated as an array. It needs to have length property.
/// valueType           : The target native value type each element is to be converted to.
/// contentBuffer       : The content buffer holding all the converted objects. It's allocated using CoTaskMemAlloc. caller needs to free it.
/// length              : The length of the input object.
/// elementSize         : The size of each element. This is for debug/verification only. AllocationSize(contentBuffer)
//  This method is used by edgehtml's VarToSequence to convert a Var to an native array, mostly in WebGL case to improve performance by reducing
//  the enter/leave script cost. We can move the code to be general place if there are future usage of this.
HRESULT STDMETHODCALLTYPE ScriptEngineBase::VarToNativeArray(Var arrayObject,
    JsNativeValueType valueType,
    __deref_out_bcount_opt(*length*(*elementSize)) byte** contentBuffer,
    UINT* length,
    UINT* elementSize)
{
    HRESULT hr = NOERROR;
    *contentBuffer = nullptr;
    *length = 0;

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    if (Js::StaticType::Is(Js::JavascriptOperators::GetTypeId(arrayObject)))
    {
        return E_INVALIDARG;
    }

    // Cache the script context locally in case the script engine gets
    // pulled out underneath us
    Js::ScriptContext* scriptContext = this->scriptContext;
    byte* buffer = nullptr;
    UINT length32 = 0;
    auto ElementSize = [&](JsNativeValueType valueType) -> int{
        switch (valueType)
        {
        case JsInt8Type:
        case JsUint8Type:
            return sizeof(byte);
        case JsInt16Type:
        case JsUint16Type:
            return sizeof(short);
        case JsInt32Type:
        case JsUint32Type:
        case JsFloatType:
            return sizeof(int32);
        case JsInt64Type:
        case JsUint64Type:
        case JsDoubleType:
            return sizeof(int64);
        case JsNativeStringType:
            return sizeof(JsNativeString);
        default:
            Assert(FALSE);
            return 0xffffffff;
        };
    };

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        Js::Var varLength = Js::JavascriptOperators::OP_GetLength(arrayObject, scriptContext);
        int64 length64 = Js::JavascriptConversion::ToLength(varLength, scriptContext);
        // we don't accept > 2G buffers.
        if (length64 >= INT_MAX/ElementSize(valueType) || length64 < 0)
        {
            hr = E_INVALIDARG;
        }
        length32 = (UINT)length64;
        if (SUCCEEDED(hr))
        {
            *elementSize = ElementSize(valueType);
            buffer = (byte*)CoTaskMemAlloc(length32* (*elementSize));
            if (buffer == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            Js::JavascriptOperators::VarToNativeArray(arrayObject, valueType, length32, *elementSize, buffer, scriptContext);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    if (SUCCEEDED(hr))
    {
        *contentBuffer = buffer;
        *length = length32;
    }
    else
    {
        if (buffer != nullptr)
        {
            CoTaskMemFree(buffer);
        }
        *elementSize = 0;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ThrowException(_In_ Var exceptionObject)
{
    AssertMsg(threadContext->HasPreviousHostScriptContext(), "This method should always be executed from within a script callout and thus have a context stack");

    // We may be disconnected from the scriptSiteHolder already so get the last script context from the stack and use it.
    HostScriptContext* hostThrowScriptContext = threadContext->GetPreviousHostScriptContext();
    Js::ScriptContext* throwScriptContext = hostThrowScriptContext->GetScriptContext();

    HRESULT hr = NOERROR;

    // We need to enter script now as the throw code path can possibly call a getter defined on Error.stackTraceLimit.
    // Also assume hasCaller, as there must be caller in the stack to catch this exception.
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_OOM_TO_HRESULT(throwScriptContext, /*doCleanup*/false, /*hasCaller*/true)
    {
        if (exceptionObject == (Var)threadContext->GetPendingOOMErrorObject())
        {
            // This is actually the OOM exception object masquerading as a Var. CreateErrorObject passed it
            // to the DOM because it was unable to create an error object.
            Js::JavascriptExceptionOperators::ThrowExceptionObject((Js::JavascriptExceptionObject*)exceptionObject, throwScriptContext);
        }
        else
        {
            Js::JavascriptExceptionOperators::Throw(exceptionObject, throwScriptContext);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_OOM_TO_HRESULT(hr);

    // We should always be able to Throw and should never get here. We can't annotate this properly across the IASD boundary though so we have code backing the condition.
    VERIFYHRESULTBEFORERETURN(hr, throwScriptContext);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::InitializeModuleRecord(
    /* [in] */ __RPC__deref_in_opt ModuleRecord referencingModule,
    /* [size_is][in] */ __RPC__in_ecount_full(specifierLength) LPCWSTR normalizedSpecifier,
    /* [in] */ UINT specifierLength,
    /* [out] */ __RPC__deref_out_opt ModuleRecord *moduleRecord)
{
    HRESULT hr = NOERROR;

    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    Js::SourceTextModuleRecord* referencingModuleRecord = Js::SourceTextModuleRecord::FromHost(referencingModule);
    Js::SourceTextModuleRecord* childModuleRecord = nullptr;

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        Js::ScriptContext *scriptContext = GetScriptContext();
        childModuleRecord = Js::SourceTextModuleRecord::Create(scriptContext);

        if (normalizedSpecifier != nullptr)
        {
            childModuleRecord->SetSpecifier(Js::JavascriptString::NewCopyBuffer(normalizedSpecifier, specifierLength, scriptContext));
        }
        else
        {
            childModuleRecord->SetSpecifier(scriptContext->GetLibrary()->GetNullString());
        }

        if (referencingModule == nullptr)
        {
            childModuleRecord->SetIsRootModule();
        }
        else
        {
            Assert(normalizedSpecifier != nullptr);
            if (normalizedSpecifier != nullptr)
            {
                childModuleRecord->SetParent(referencingModuleRecord, normalizedSpecifier);
            }
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    if (SUCCEEDED(hr))
    {
        *moduleRecord = childModuleRecord;
    }
    return hr;
}

// Theoretically this method can be called from different thread. We'll need to move out the
// moduledeclarationInitialization (for GC allocation) part. ModuleEvaluation should be out by default, called from host.
HRESULT STDMETHODCALLTYPE ScriptEngineBase::ParseModuleSource(
    /* [in] */ __RPC__in ModuleRecord requestModule,
    /* [in] */ __RPC__in_opt IUnknown *punkContext,
    /* [in] */ __RPC__in void *sourceContext,
    /* [size_is][in] */ __RPC__in_ecount_full(sourceLength) byte *sourceText,
    /* [in] */ unsigned long sourceLength,
    /* [in] */ ParseModuleSourceFlags sourceFlag,
    /* [in] */ unsigned long startingLine,
    /* [in] */ unsigned long startingColumn,
    /* [in] */ unsigned long startingOffset,
    /* [out] */ __RPC__deref_out_opt Var *exceptionVar)
{
    HRESULT hr = NOERROR;
    // TODO: allow parallel parsing? we need to pick the right allocator in ModuleRecord,
    // and remove thread check (VerifyOnEntry call)
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    if (scriptSiteHolder == nullptr)
    {
        return E_ACCESSDENIED;
    }
    if (requestModule == nullptr || exceptionVar == nullptr || sourceFlag > ParseModuleSourceFlags_DataMax)
    {
        return E_INVALIDARG;
    }
    if (sourceFlag == ParseModuleSourceFlags_DataIsIntermediateCode)
    {
        // TODO: investigate the possibility of using bytecode cache.
        return E_INVALIDARG;
    }

    SmartFPUControl smartFpuControl;
    if (smartFpuControl.HasErr())
    {
        return smartFpuControl.GetErr();
    }

    *exceptionVar = nullptr;

    Js::SourceTextModuleRecord* moduleRecord = Js::SourceTextModuleRecord::FromHost(requestModule);
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(this);
        SourceContextInfo* sourceContextInfo = scriptEngine->GetSourceContextInfo((ULONG_PTR)sourceContext, (uint)sourceLength, FALSE, nullptr, nullptr);
        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ startingLine,
            /* ulColumnHost        */ startingColumn,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ sourceLength,
            /* ulCharOffset        */ startingOffset,
            /* mod                 */ 0,
            /* grfsi               */ 0
        };
        hr = moduleRecord->ParseSource(sourceText, sourceLength, &si, exceptionVar, sourceFlag == ParseModuleSourceFlags_DataIsUTF8 ? true : false);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::ModuleEvaluation(
    /* [in] */ __RPC__deref_in_opt ModuleRecord requestModule,
    /* [out] */ __RPC__deref_out_opt Var *varResult)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    Js::SourceTextModuleRecord* moduleRecord = Js::SourceTextModuleRecord::FromHost(requestModule);
    if (moduleRecord->GetScriptContext() != GetScriptContext())
    {
        Assert(false);
        return E_INVALIDARG;
    }
    SmartFPUControl smartFpuControl;
    if (smartFpuControl.HasErr())
    {
        return smartFpuControl.GetErr();
    }

    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
    {
        BEGIN_ENTER_SCRIPT(scriptContext, true /*cleanup*/, true /*isRoot*/, false /*hasCaller*/)
        {
            *varResult = moduleRecord->ModuleEvaluation();
        }
        END_ENTER_SCRIPT
    }
    TRANSLATE_EXCEPTION_TO_HRESULT_ENTRY(const Js::JavascriptException& err)
    {
        *varResult = scriptContext->GetLibrary()->GetUndefined();

        Js::JavascriptExceptionObject * exceptionObject = err.GetAndClear();
        if (exceptionObject != nullptr)
        {
            exceptionObject = exceptionObject->CloneIfStaticExceptionObject(scriptContext);
        }
        hr = GetScriptSiteHolder()->HandleJavascriptException(exceptionObject, scriptContext);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::CreateScriptErrorFromVar(Var errorObject, IActiveScriptError** scriptError)
{
    HRESULT hr = NOERROR;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }

    Js::JavascriptExceptionObject* exceptionObject = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        exceptionObject = RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptExceptionObject, errorObject, scriptContext, nullptr);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    if (FAILED(hr))
    {
        return hr;
    }

    ActiveScriptError* activeScriptError = nullptr;
    if (SUCCEEDED(ActiveScriptError::CreateRuntimeError(exceptionObject, &hr, nullptr, scriptContext, &activeScriptError)))
    {
        hr = activeScriptError->QueryInterface(__uuidof(IActiveScriptError), (void**)scriptError);
        activeScriptError->Release();
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::SetModuleHostInfo(
    /* [in] */ __RPC__deref_in_opt ModuleRecord requestModule,
    /* [in] */ ModuleHostInfoKind moduleHostState,
    /* [in] */ __RPC__in void *hostInfo)
{
    HRESULT hr;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    Js::SourceTextModuleRecord* moduleRecord = Js::SourceTextModuleRecord::FromHost(requestModule);
    switch (moduleHostState)
    {
    case ModuleHostInfo_HostDefined:
        moduleRecord->SetHostDefined(hostInfo);
        hr = S_OK;
        break;
    case ModuleHostInfo_Exception:
        // This is after the script engine ask the host for child module, and it failed to download the module
        hr = moduleRecord->OnHostException(hostInfo);
        Assert(false);
        break;
    default:
        Assert(false);
        hr = E_INVALIDARG;
        break;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngineBase::GetModuleHostInfo(
    /* [in] */ __RPC__deref_in_opt ModuleRecord requestModule,
    /* [in] */ ModuleHostInfoKind moduleHostState,
    /* [out] */ __RPC__deref_out_opt void **hostInfo)
{
    HRESULT hr;
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        return hr;
    }
    Js::SourceTextModuleRecord* moduleRecord = Js::SourceTextModuleRecord::FromHost(requestModule);
    switch (moduleHostState)
    {
    case ModuleHostInfo_HostDefined:
        *hostInfo = moduleRecord->GetHostDefined();
        hr = S_OK;
        break;
    case ModuleHostInfo_Exception:
        Assert(false);
        break;
    default:
        Assert(false);
        hr = E_INVALIDARG;
        break;
    }
    return hr;
}

HRESULT ScriptEngineBase::VerifyOnEntry(BOOL allowedInHeapEnum)
{
    HRESULT hr = NOERROR;
    if (scriptSiteHolder == nullptr)
    {
        return E_ACCESSDENIED;
    }
    IFFAILRET(ValidateBaseThread());

    if (!threadContext->IsStackAvailableNoThrow())
    {
        return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
    }
    if (threadContext->GetRecycler()->IsHeapEnumInProgress() && !allowedInHeapEnum)
    {
        Assert(FALSE);
        return E_UNEXPECTED;
    }
    Assert(!scriptSiteHolder->GetScriptSiteContext()->IsClosed());
    return NOERROR;
}

inline HRESULT ScriptEngineBase::ValidateBaseThread(void)
{
    if (this->threadContext != ThreadContext::GetContextForCurrentThread())
    {
        AssertMsg(FALSE, "Not Base Thread");
        return E_UNEXPECTED;
    }
    else
        return NOERROR;
}
