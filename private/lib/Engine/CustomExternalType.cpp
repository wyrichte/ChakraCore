//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

#include "Types\DeferredTypeHandler.h"

#ifdef ENABLE_JS_ETW
#include <IERESP_mshtml.h>
#include "microsoft-scripting-jscript9.internalevents.h"
#endif

namespace Js
{
    DEFINE_RECYCLER_TRACKER_PERF_COUNTER(CustomExternalObject);

    CustomExternalType * CustomExternalType::New(CustomExternalType * type)
    {
        uint8 extraSlotsCount = type->GetExtraSlotsCount();
        if (extraSlotsCount != 0)
        {
            size_t extraSlotsSize = sizeof(void *) * extraSlotsCount;
            CustomExternalType * newType = RecyclerNewPlus(type->GetLibrary()->GetRecycler(), extraSlotsSize, CustomExternalType, type);
            js_memcpy_s((void *)(newType + 1), extraSlotsSize, (void *)(type + 1), extraSlotsSize);
            return newType;
        }
        return RecyclerNew(type->GetLibrary()->GetRecycler(), CustomExternalType, type);
    }

    CustomExternalType * CustomExternalType::New(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, ExternalMethod entryPoint, 
        DynamicTypeHandler * typeHandler, bool isLocked, bool isShared, ITypeOperations * operations, PropertyId nameId, const JavascriptTypeId* inheritedTypeIds, 
        UINT inheritedTypeIdsCount, uint8 extraSlotsCount)
    {
        if (extraSlotsCount)
        {
            return RecyclerNewPlus(scriptContext->GetRecycler(), sizeof(void *) * extraSlotsCount, CustomExternalType, scriptContext, typeId, prototype, entryPoint,
                typeHandler, isLocked, isShared, operations, nameId, inheritedTypeIds, inheritedTypeIdsCount, extraSlotsCount);
        }
        return RecyclerNew(scriptContext->GetRecycler(), CustomExternalType, scriptContext, typeId, prototype, entryPoint,
            typeHandler, isLocked, isShared, operations, nameId, inheritedTypeIds, inheritedTypeIdsCount, extraSlotsCount);
    }

    // while this interface is defined with HRESULT return value, we don't really use the hr as trident will
    // throw if there is something wrong. Add explicit statement to ignore those return hr;
#define IGNORE_HR(hr) hr; 
    HRESULT CustomExternalType::Initialize()
    {
        if (NULL == this->operations)
        {
            return E_INVALIDARG;
        }
        if (FAILED(this->GetTypeOperations()->GetFinalizer(&finalizer)))
        {
            finalizer = nullptr;
        }
        IUnknown* defaultOperators = nullptr;
        if (NOERROR == this->operations->QueryInterface(
            IID_IDefaultScriptOperations, (void**)&defaultOperators))
        {
            // Note, we don't need to release as we don't addref for defaultscriptoperators. it's effectively static instance.
            isSimpleWrapper = true;

        }
        HRESULT hr = this->GetTypeOperations()->GetOperationUsage(&this->usage);
        IGNORE_HR(hr)
        if (GetOperationUsage().useWhenPropertyNotPresentInPrototypeChain & OperationFlagsForNamespaceOrdering_allGetPropertyOperations)
        {
            this->flags |= TypeFlagMask_SkipsPrototype;
        }
        return hr;
    }

    bool CustomExternalType::DeferredInitializer(DynamicObject * instance, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        CustomExternalObject * object = (CustomExternalObject *)instance;
        HRESULT hr = E_FAIL;

        ScriptContext * scriptContext = object->GetScriptContext();
        InitializeMethod initializer;
        int initSlotCapacity = 0;
        BOOL hasAccessors = FALSE;

        if (!object->VerifyObjectAlive())
        {
            // VerifyObjectAlive will throw, but in some cases return FALSE instead. We need to still throw even in that case.
            // This check looks for host disabled objects during navigation as well. In those cases we should definitely not
            // continue with initialization since the host will like induce Abandonment.
            Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
        }
        ThreadContext * threadContext = scriptContext->GetThreadContext();

        if (object->GetCustomExternalType()->IsSimpleWrapper())
        {
            hr = (DefaultScriptOperations::s_DefaultScriptOperations).GetInitializer(&initializer, &initSlotCapacity, &hasAccessors);
            Assert(SUCCEEDED(hr));
            typeHandler->Convert(instance, mode, initSlotCapacity, hasAccessors);
        }
        else if (threadContext->IsDisableImplicitCall())
        {
            // Expanding defer initialize type can have any side effect
            threadContext->AddImplicitCallFlags(ImplicitCall_External);
            return false;
        }
        else
        {
            BEGIN_LEAVE_SCRIPT_INTERNAL(scriptContext)
            {
                ASYNC_HOST_OPERATION_START(threadContext);

                hr = object->GetCustomExternalType()->GetTypeOperations()->GetInitializer(&initializer, &initSlotCapacity, &hasAccessors);

                ASYNC_HOST_OPERATION_END(threadContext);
            }
            END_LEAVE_SCRIPT_INTERNAL(scriptContext);

            if (SUCCEEDED(hr) && initializer != NULL)
            {
                typeHandler->Convert(instance, mode, initSlotCapacity, hasAccessors);

                BEGIN_LEAVE_SCRIPT_INTERNAL(scriptContext)
                {
                    ASYNC_HOST_OPERATION_START(threadContext);

                    hr = initializer(instance);

                    ASYNC_HOST_OPERATION_END(threadContext);
                }
                END_LEAVE_SCRIPT_INTERNAL(scriptContext);
            }
            else
            {
                hr = E_FAIL;
                // why are we here?!?
                Assert(false);
                typeHandler->Convert(instance, mode, initSlotCapacity, hasAccessors);
            }
        }

        if (FAILED(hr))
        {
            Js::JavascriptError::MapAndThrowError(scriptContext, hr);
        }

        return true;
    }

    CustomExternalObject::CustomExternalObject(CustomExternalType * type
#if DBG
                    , UINT byteCount
#endif
        ) :
    ExternalObject(type
#if DBG
                    , byteCount
#endif
        )
    {
        this->finalizer = this->GetCustomExternalType()->GetFinalizer();
    }

    bool CustomExternalObject::Is(Var instance)
    {
        if (!ExternalObject::Is(instance))
        {
            return false;
        }
        return ExternalObject::FromVar(instance)->IsCustomExternalObject();
    }

    CustomExternalObject* CustomExternalObject::FromVar(Var instance)
    {
        Assert(Is(instance));
        CustomExternalObject* obj = static_cast<CustomExternalObject*>(instance);
        return obj;
    }

    void CustomExternalObject::Finalize(bool isShutdown)
    {
        if (!isShutdown && finalizer)
        {
            GetFinalizer()(this, FinalizeMode_Finalize);
        }
    }

    void CustomExternalObject::Dispose(bool isShutdown)
    {
        if (finalizer)
        {
            if (!isShutdown)
            {
                GetFinalizer()(this, FinalizeMode_Dispose);
            }
            else
            {
                LEAK_REPORT_PRINT(_u("CustomExternalObject %p: Finalize not called on shutdown\n"), this);
            }
        }
    }


    PropertyQueryFlags CustomExternalObject::HasPropertyQuery(PropertyId propertyId)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return PropertyQueryFlags::Property_NotFound;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::HasPropertyQuery(propertyId);
        }
        // we don't throw in hasProperty, but possibly throw in GetProperty
        // this is consistent with HostDispatch code path as well.
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_hasOwnProperty)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return PropertyQueryFlags::Property_Found;
            }
            else
            {
                notPresent = true;
            }
        }

        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_hasOwnProperty))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // The SecureHostObject is useAlways, but it also wraps the GlobalObject so we never have to fall back to it.
            // Also as of IE 9, script collection ordering no longer matters, the Chakra engine is always first, so the
            // global optimization can always apply and no fallback to this objects ITypeOperations is needed.
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                return JavascriptConversion::BooleanToPropertyQueryFlags(globalObject->HasOwnProperty(propertyId));
            }
            else
            {
                // Reject implicit call
                ThreadContext* threadContext = scriptContext->GetThreadContext();
                if (threadContext->IsDisableImplicitCall())
                {
                    threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                    return PropertyQueryFlags::Property_Found;
                }

                HRESULT hr;
                BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), propertyId, CustomExternalObject_HasOwnProperty)
                {
                    hr = this->GetTypeOperations()->HasOwnProperty(scriptContext->GetActiveScriptDirect(), this, propertyId, &result);
                }
                END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), propertyId, CustomExternalObject_HasOwnProperty);
                IGNORE_HR(hr)
            }
            return JavascriptConversion::BooleanToPropertyQueryFlags(result);
        }
        return ExternalObject::HasPropertyQuery(propertyId);
    }

    PropertyQueryFlags CustomExternalObject::GetPropertyQuery(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL notPresent=false;
        BOOL dontCachePrototype = false;
        if (!this->VerifyObjectAlive())
        {
            return PropertyQueryFlags::Property_NotFound;
        }
        Assert(GetScriptContext()->GetThreadContext()->IsActivePropertyId(propertyId));

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetPropertyQuery(originalInstance, propertyId, value, info, requestContext);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_getOwnProperty)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::GetPropertyQuery(originalInstance, propertyId, value, info, requestContext);
            }
            else
            {
                notPresent = true;
            }
        }

        if (this->GetOperationUsage().useWhenPropertyNotPresentInPrototypeChain & OperationFlagsForNamespaceOrdering_allGetPropertyOperations)
        {
            if (JavascriptOperators::HasProperty(this->GetPrototype(), propertyId))
            {
                BOOL result;
                if (info)
                {
                    // The recursive call to GetProperty will populate the cache as appropriate.
                    // Capture with a local PropertyValueInfo so that the caller doesn't try to populate again with
                    // inconsistent info.
                    PropertyValueInfo localInfo = *info;
                    result = JavascriptOperators::GetProperty(originalInstance, this->GetPrototype(), propertyId, value, requestContext, &localInfo);
                }
                else
                {
                    result = JavascriptOperators::GetProperty(originalInstance, this->GetPrototype(), propertyId, value, requestContext, nullptr);
                }

                if (result)
                {
                    *value = CrossSite::MarshalVar(requestContext, *value);
                }
                return JavascriptConversion::BooleanToPropertyQueryFlags(result);
            }
            else
            {
                // TODO: If we can't find the property from the prototype chain here, we don't really go back and continue the prototype chain in
                // JavascriptOperators::GetProperty/GetPropertyReference. We should pass the flag up.
                dontCachePrototype = true;
                notPresent = true;
            }
            if (!this->VerifyObjectAlive())
            {
                return PropertyQueryFlags::Property_NotFound;
            }
        }
        else
        {
            dontCachePrototype = true;
        }

        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_getOwnProperty))
        {
            PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

            GlobalObject* globalObject = GetLibrary()->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->GetProperty(originalInstance, propertyId, value, info, requestContext))
                {
                    *value = CrossSite::MarshalVar(requestContext, *value);
                    return PropertyQueryFlags::Property_Found;
                }
                else
                {
                    return PropertyQueryFlags::Property_NotFound;
                }
            }

            ScriptContext * scriptContext = this->GetScriptContext();
            ThreadContext * threadContext = scriptContext->GetThreadContext();

            // Reject implicit call
            if (threadContext->IsDisableImplicitCall())
            {
                *value = requestContext->GetLibrary()->GetNull();
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return PropertyQueryFlags::Property_Found;
            }

            BOOL result = false;
            bool fXDomainMarshal = false;
            HRESULT hr = NOERROR;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetOwnProperty)
            {
                hr = this->GetTypeOperations()->GetOwnProperty(requestContext->GetActiveScriptDirect(), this, propertyId, value, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetOwnProperty);
            IGNORE_HR(hr);
            if (result)
                {
                    // The site might have been closed during the GetOwnProperty call.
                    if (!this->VerifyObjectAlive())
                    {
                        return PropertyQueryFlags::Property_NotFound;
                    }
                    if ((this->GetOperationUsage().useAlways & OperationFlag_crossDomainCheck) == OperationFlag_crossDomainCheck)
                    {
                        BOOL resultCrossDomainCheck;
                        BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetOwnProperty)
                        {
                            hr = this->GetTypeOperations()->CrossDomainCheck(requestContext->GetActiveScriptDirect(), this, &resultCrossDomainCheck);
                        }
                        END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetOwnProperty);
                        IGNORE_HR(hr)
                        if (resultCrossDomainCheck)
                        {
                            fXDomainMarshal = true;
                        }
                    }
                }
            if(result)
            {
                // This is defense in depth. In some weird scenario DOM could return S_OK without actually getting the return value.
                // It is basically access denied.
                if (*value == nullptr)
                {
                    Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
                }
                *value = CrossSite::MarshalVar(requestContext, *value, fXDomainMarshal);
            }
            else if (dontCachePrototype)
            {
                // if we need to go through ITypeOperation first to find property, it is possible for the DOM side
                // to add property without going through jscript side of code. We need to disable the prototype
                // lookup as well, because we cannot guarantee if we'll see the property in this object next round.
                // We might get property from current object instead of prototype object without any change in jscript code.
                PropertyValueInfo::DisablePrototypeCache(info, this); // We can't cache the property
            }
            return JavascriptConversion::BooleanToPropertyQueryFlags(result);
        }
        return ExternalObject::GetPropertyQuery(originalInstance, propertyId, value, info, requestContext);
    }

    PropertyQueryFlags CustomExternalObject::GetPropertyQuery(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return CustomExternalObject::GetPropertyQuery(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
    }

    PropertyQueryFlags CustomExternalObject::GetPropertyReferenceQuery(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info,
        ScriptContext* requestContext)
    {
        BOOL notPresent=false;
        BOOL dontCachePrototype = false;
        if (!this->VerifyObjectAlive())
        {
            return PropertyQueryFlags::Property_NotFound;
        }
        Assert(GetScriptContext()->GetThreadContext()->IsActivePropertyId(propertyId));

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetPropertyReferenceQuery(originalInstance, propertyId, value, info, requestContext);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_getPropertyReference)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::GetPropertyReferenceQuery(originalInstance, propertyId, value, info, requestContext);
            }
            else
            {
                notPresent = true;
            }
        }

        if (this->GetOperationUsage().useWhenPropertyNotPresentInPrototypeChain & OperationFlagsForNamespaceOrdering_allGetPropertyOperations)
        {
            if (JavascriptOperators::HasProperty(this->GetPrototype(), propertyId))
            {
                BOOL result;
                if (info)
                {
                    // The recursive call to GetPropertyReference will populate the cache as appropriate.
                    // Capture with a local PropertyValueInfo so that the caller doesn't try to populate again with
                    // inconsistent info.
                    PropertyValueInfo localInfo = *info;
                    result = JavascriptOperators::GetPropertyReference(originalInstance, this->GetPrototype(), propertyId, value, requestContext, &localInfo);
                }
                else
                {
                    result = JavascriptOperators::GetPropertyReference(originalInstance, this->GetPrototype(), propertyId, value, requestContext, nullptr);
                }
                if (result)
                {
                    *value = CrossSite::MarshalVar(requestContext, *value);
                }
                return JavascriptConversion::BooleanToPropertyQueryFlags(result);
            }
            else
            {
                dontCachePrototype = true;
                notPresent = true;
            }
            if (!this->VerifyObjectAlive())
            {
                return PropertyQueryFlags::Property_NotFound;
            }
        }
        else
        {
            dontCachePrototype = true;
        }

        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_getPropertyReference))
        {
            PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

            GlobalObject* globalObject = GetScriptContext()->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->GetPropertyReference(originalInstance, propertyId, value, info, requestContext))
                {
                    *value = CrossSite::MarshalVar(requestContext, *value);
                    return PropertyQueryFlags::Property_Found;
                }
                else
                {
                    return PropertyQueryFlags::Property_NotFound;
                }
            }

            ScriptContext * scriptContext = this->GetScriptContext();
            ThreadContext * threadContext = scriptContext->GetThreadContext();

            // Reject implicit call
            if (threadContext->IsDisableImplicitCall())
            {
                *value = requestContext->GetLibrary()->GetNull();
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return PropertyQueryFlags::Property_Found;
            }

            BOOL result = FALSE;
            bool fXDomainMarshal = false;
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetPropertyReference)
            {
                hr = this->GetTypeOperations()->GetPropertyReference(requestContext->GetActiveScriptDirect(), this, propertyId, value, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetPropertyReference);
            IGNORE_HR(hr);
            if (result)
            {
                if (!this->VerifyObjectAlive())
                {
                    return PropertyQueryFlags::Property_NotFound;
                }
                if ((this->GetOperationUsage().useAlways & OperationFlag_crossDomainCheck) == OperationFlag_crossDomainCheck)
                {
                    BOOL resultCrossDomainCheck;
                    BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetPropertyReference)
                    {
                        hr = this->GetTypeOperations()->CrossDomainCheck(requestContext->GetActiveScriptDirect(), this, &resultCrossDomainCheck);
                    }
                    END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetPropertyReference);
                    IGNORE_HR(hr)
                        if (resultCrossDomainCheck)
                        {
                            fXDomainMarshal = true;
                        }
                }
            }

            if(result)
            {
                // This is defense in depth. In some weird scenario DOM could return S_OK without actually getting the return value.
                // It is basically access denied.
                if (*value == nullptr)
                {
                    Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
                }
                *value = CrossSite::MarshalVar(requestContext, *value, fXDomainMarshal);
            }
            else if (dontCachePrototype)
            {
                // if we need to go through ITypeOperation first to find property, it is possible for the DOM side
                // to add property without going through jscript side of code. We need to disable the prototype
                // lookup as well, because we cannot guarantee if we'll see the property in this object next round.
                // We might get property from current object instead of prototype object without any change in jscript code.
                PropertyValueInfo::DisablePrototypeCache(info, this); // We can't cache the property
            }

            return JavascriptConversion::BooleanToPropertyQueryFlags(result);
        }
        return ExternalObject::GetPropertyReferenceQuery(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL CustomExternalObject::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        Assert(GetScriptContext()->GetThreadContext()->IsActivePropertyId(propertyId));

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetProperty(propertyId, value, flags, info);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setProperty)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::SetProperty(propertyId, value, flags, info);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setProperty))
        {
            PropertyValueInfo::SetNoCache(info, this); // We can't cache the property

            ScriptContext * scriptContext = this->GetScriptContext();
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            BOOL result = FALSE;
            value = Js::CrossSite::MarshalVar(scriptContext, value);

            if (this == globalObject->GetSecureDirectHostObject() )
            {
                return globalObject->SetProperty(propertyId, value, flags, info);
            }

            ThreadContext * threadContext = scriptContext->GetThreadContext();

            // Reject implicit call
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }

            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetProperty)
            {
                hr = this->GetTypeOperations()->SetProperty(scriptContext->GetActiveScriptDirect(), this, propertyId, value, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetProperty);
            IGNORE_HR(hr)
            return result;
        }
            return ExternalObject::SetProperty(propertyId, value, flags, info);
    }

    BOOL CustomExternalObject::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return CustomExternalObject::SetProperty(propertyRecord->GetPropertyId(), value, flags, info);
    }

    BOOL CustomExternalObject::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setPropertyWithAttributes)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setPropertyWithAttributes))
        {
            PropertyValueInfo::SetNoCache(info, this); // We can't cache the property
            ScriptContext * scriptContext = this->GetScriptContext();

            BOOL result = FALSE;
            value = Js::CrossSite::MarshalVar(scriptContext, value);

            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                return globalObject->SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
            }

            ThreadContext * threadContext = scriptContext->GetThreadContext();

            // Reject implicit call
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }

            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext,  Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetPropertyWithAttributes)
            {
                hr = this->GetTypeOperations()->SetPropertyWithAttributes(scriptContext->GetActiveScriptDirect(), this, propertyId, value, (::PropertyAttributes) attributes, (::SideEffects) possibleSideEffects, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetPropertyWithAttributes);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
    }

    BOOL CustomExternalObject::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }

        GlobalObject* globalObject = GetScriptContext()->GetGlobalObject();
        if (this == globalObject->GetSecureDirectHostObject())
        {
            return globalObject->InitProperty(propertyId, value, flags, info);
        }

        return SetProperty(propertyId, value, flags, info);
    }

    BOOL CustomExternalObject::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        Assert(GetScriptContext()->GetThreadContext()->IsActivePropertyId(propertyId));

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::DeleteProperty(propertyId, flags);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_deleteProperty)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::DeleteProperty(propertyId, flags);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_deleteProperty))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->DeleteProperty()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    return globalObject->DynamicObject::DeleteProperty(propertyId, flags);
                }
            }

            // Reject implicit call
            ThreadContext* threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }

            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_DeleteProperty)
            {
                hr = this->GetTypeOperations()->DeleteProperty(scriptContext->GetActiveScriptDirect(), this, propertyId, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_DeleteProperty);
            return result;
        }
        return ExternalObject::DeleteProperty(propertyId, flags);
    }

    BOOL CustomExternalObject::DeleteProperty(JavascriptString *propertyNameString, PropertyOperationFlags flags)
    {
        PropertyRecord const *propertyRecord = nullptr;
        if (JavascriptOperators::ShouldTryDeleteProperty(this, propertyNameString, &propertyRecord))
        {
            Assert(propertyRecord);
            return DeleteProperty(propertyRecord->GetPropertyId(), flags);
        }

        return TRUE;
    }

    PropertyQueryFlags CustomExternalObject::HasItemQuery(uint32 index)
    {
        if (!this->VerifyObjectAlive())
        {
            return PropertyQueryFlags::Property_NotFound;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::HasItemQuery(index);
        }

        BOOL notPresent=false;

        // we don't throw in HasItem, but possibly throw in GetItem
        // this is consistent with HostDispatch code path as well.
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_hasOwnItem)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasItemQuery(index)))
            {
                return PropertyQueryFlags::Property_Found;
            }
            else
            {
                notPresent = true;
            }
        }

        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_hasOwnItem))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // The SecureHostObject is useAlways, but it also wraps the GlobalObject so we never have to fall back to it.
            // Also as of IE 9, script collection ordering no longer matters, the Chakra engine is always first, so the
            // global optimization can always apply and no fallback to this objects ITypeOperations is needed.
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                return JavascriptConversion::BooleanToPropertyQueryFlags(globalObject->HasOwnItem(index));
            }
            else
            {
                // Reject implicit call
                ThreadContext * threadContext = scriptContext->GetThreadContext();
                if (threadContext->IsDisableImplicitCall())
                {
                    threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                    return PropertyQueryFlags::Property_Found;
                }

                Var varIndex = JavascriptNumber::ToVar(index, GetScriptContext());
                HRESULT hr;
                BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_HasOwnItem)
                {
                    hr = this->GetTypeOperations()->HasOwnItem(scriptContext->GetActiveScriptDirect(), this, varIndex, &result);
                }
                END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_HasOwnItem);
                IGNORE_HR(hr)
            }
            return JavascriptConversion::BooleanToPropertyQueryFlags(result);
        }
        return ExternalObject::HasItemQuery(index);
    }

    PropertyQueryFlags CustomExternalObject::GetItemQuery(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return PropertyQueryFlags::Property_NotFound;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetItemQuery(originalInstance, index, value, requestContext);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_getOwnItem)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::GetItemQuery(originalInstance, index, value, requestContext)))
            {
                return PropertyQueryFlags::Property_Found;
            }
            else
            {
                notPresent = true;
            }
        }

        if (this->GetOperationUsage().useWhenPropertyNotPresentInPrototypeChain & OperationFlagsForNamespaceOrdering_allGetPropertyOperations)
        {
            if (JavascriptOperators::HasItem(this->GetPrototype(), index))
            {
                BOOL result = JavascriptOperators::GetItem(this->GetPrototype(), index, value, requestContext);
                if (result)
                {
                    *value = CrossSite::MarshalVar(requestContext, *value);
                }
                return JavascriptConversion::BooleanToPropertyQueryFlags(result);
            }
            else
            {
                notPresent = true;
            }
            if (!this->VerifyObjectAlive())
            {
                return PropertyQueryFlags::Property_NotFound;
            }
        }

        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_getOwnItem))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                result = globalObject->GetItem(originalInstance, index, value, requestContext);
                if (result)
                {
                    *value = Js::CrossSite::MarshalVar(requestContext, *value);
                }
                return JavascriptConversion::BooleanToPropertyQueryFlags(result);
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                *value = requestContext->GetLibrary()->GetNull();
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return PropertyQueryFlags::Property_Found;
            }
            Var varIndex = JavascriptNumber::ToVar(index, GetScriptContext());
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_GetOwnItem)
            {
                hr = this->GetTypeOperations()->GetOwnItem(requestContext->GetActiveScriptDirect(), this, varIndex, value, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_GetOwnItem);
            IGNORE_HR(hr)
            if (result)
            {
                // This is defense in depth. In some weird scenario DOM could return S_OK without actually getting the return value.
                // It is basically access denied.
                if (*value == nullptr)
                {
                    Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
                }
                *value = Js::CrossSite::MarshalVar(requestContext, *value);
            }
            return JavascriptConversion::BooleanToPropertyQueryFlags(result);
        }
        return ExternalObject::GetItemQuery(originalInstance, index, value, requestContext);
    }

    PropertyQueryFlags CustomExternalObject::GetItemReferenceQuery(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive())
        {
            return PropertyQueryFlags::Property_NotFound;
        }
        return GetItemQuery(originalInstance, index, value, requestContext);
    }

    BOOL CustomExternalObject::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetItem(index, value, flags);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setItem)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasItemQuery(index)))
            {
                return ExternalObject::SetItem(index, value, flags);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setItem))
        {
            BOOL result = FALSE;
            value = Js::CrossSite::MarshalVar(GetScriptContext(), value);
            ScriptContext * scriptContext = this->GetScriptContext();
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                return globalObject->DynamicObject::SetItem(index, value, flags);
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            Var varIndex = JavascriptNumber::ToVar(index, GetScriptContext());
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_SetItem)
            {
                hr = this->GetTypeOperations()->SetItem(scriptContext->GetActiveScriptDirect(), this, varIndex, value, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_SetItem);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::SetItem(index, value, flags);
    }

    BOOL CustomExternalObject::GetEnumerator(JavascriptStaticEnumerator * enumerator, EnumeratorFlags flags, ScriptContext* requestContext, ForInCache * forInCache)
    {
        HRESULT hr = FALSE;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetEnumerator(enumerator, flags, requestContext);
        }
        if (this->GetOperationUsage().useAlways & OperationFlag_getEnumerator)
        {
            IVarEnumerator* varEnumerator = NULL;
            ScriptContext * scriptContext = this->GetScriptContext();
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return FALSE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), 0, CustomExternalObject_GetEnumerator)
            {
                hr = this->GetTypeOperations()->GetEnumerator(requestContext->GetActiveScriptDirect(), this, 
                    !!(flags & EnumeratorFlags::EnumNonEnumerable), !!(flags & EnumeratorFlags::EnumSymbols), &varEnumerator);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), 0, CustomExternalObject_GetEnumerator);
            IGNORE_HR(hr)
            // This is defense in depth. In some weird scenario DOM could return S_OK without actually getting the return value.
            // It is basically access denied.
            if (varEnumerator == nullptr)
            {
                Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
            }
            IVarEnumerator2 * varEnumerator2 = NULL;
            if (SUCCEEDED(varEnumerator->QueryInterface(__uuidof(IVarEnumerator2), (void **)&varEnumerator2)))
            {
                Js::JavascriptStaticEnumerator * externalEnumerator;
                hr = varEnumerator2->GetJavascriptEnumerator((Var *)&externalEnumerator);
                varEnumerator2->Release();
                if (SUCCEEDED(hr))
                {
                    varEnumerator->Release();
                    if (externalEnumerator == nullptr)
                    {
                        enumerator->Clear(flags, requestContext);
                    }
                    else
                    {
                        *enumerator = *externalEnumerator;
                    }

                    return TRUE;
                }
            }
            enumerator->Initialize(CreateEnumerator(requestContext, varEnumerator), nullptr, nullptr, flags, requestContext, nullptr);
            varEnumerator->Release();
            return TRUE;
        }
        return ExternalObject::GetEnumerator(enumerator, flags, requestContext, forInCache);
    }

    // just to allow try_finally in caller function
    CustomEnumerator* CustomExternalObject::CreateEnumerator(ScriptContext* scriptContext, IVarEnumerator* varEnumerator)
    {
        CustomEnumerator* customEnumerator = RecyclerNewFinalized(scriptContext->GetRecycler(), CustomEnumerator, scriptContext, varEnumerator, this);

        return customEnumerator;
    }

    BOOL CustomExternalObject::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::DeleteItem(index, flags);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_deleteItem)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasItemQuery(index)))
            {
                return ExternalObject::DeleteItem(index, flags);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_deleteItem))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                return globalObject->DynamicObject::DeleteItem(index, flags);
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            Var varIndex = JavascriptNumber::ToVar(index, GetScriptContext());
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_DeleteItem)
            {
                hr = this->GetTypeOperations()->DeleteItem(scriptContext->GetActiveScriptDirect(), this, varIndex, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_DeleteItem);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::DeleteItem(index, flags);
    }

    BOOL CustomExternalObject::IsEnumerable(PropertyId propertyId)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::IsEnumerable(propertyId);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_isEnumerable)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::IsEnumerable(propertyId);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_isEnumerable))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->IsEnumerable()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    return globalObject->DynamicObject::IsEnumerable(propertyId);
                }
            }

            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_IsEnumerable)
            {
                hr = this->GetTypeOperations()->IsEnumerable(scriptContext->GetActiveScriptDirect(), this, propertyId, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_IsEnumerable);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::IsEnumerable(propertyId);
    }

    BOOL CustomExternalObject::IsWritable(PropertyId propertyId)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::IsWritable(propertyId);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_isWritable)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::IsWritable(propertyId);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_isWritable))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->IsWritable()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    return globalObject->DynamicObject::IsWritable(propertyId);
                }
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_IsWritable)
            {
                hr = this->GetTypeOperations()->IsWritable(scriptContext->GetActiveScriptDirect(), this, propertyId, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_IsWritable);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::IsWritable(propertyId);
    }

    BOOL CustomExternalObject::IsConfigurable(PropertyId propertyId)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::IsConfigurable(propertyId);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_isConfigurable)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::IsConfigurable(propertyId);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_isConfigurable))
        {
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->IsConfigurable()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    return globalObject->DynamicObject::IsConfigurable(propertyId);
                }
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_IsConfigurable)
            {
                hr = this->GetTypeOperations()->IsConfigurable(scriptContext->GetActiveScriptDirect(), this, propertyId, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_IsConfigurable);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::IsConfigurable(propertyId);
    }

    BOOL CustomExternalObject::SetEnumerable(PropertyId propertyId, BOOL value)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetEnumerable(propertyId, value);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setEnumerable)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::SetEnumerable(propertyId, value);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setEnumerable))
        {
            HRESULT result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->SetEnumerable()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    return globalObject->DynamicObject::SetEnumerable(propertyId, value);
                }
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetEnumerable)
            {
                hr = result = this->GetTypeOperations()->SetEnumerable(scriptContext->GetActiveScriptDirect(), this, propertyId, value);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetEnumerable);
            IGNORE_HR(hr)
            return SUCCEEDED(result);
        }
        return ExternalObject::SetEnumerable(propertyId, value);
    }

    BOOL CustomExternalObject::SetWritable(PropertyId propertyId, BOOL value)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetWritable(propertyId, value);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setWritable)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::SetWritable(propertyId, value);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setWritable))
        {
            HRESULT result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->SetWritable()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    globalObject->DynamicObject::SetWritable(propertyId, value);
                }
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            HRESULT hr;
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetWritable)
            {
                hr = result = this->GetTypeOperations()->SetWritable(scriptContext->GetActiveScriptDirect(), this, propertyId, value);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetWritable);
            IGNORE_HR(hr)
            return SUCCEEDED(result);
        }
        return ExternalObject::SetWritable(propertyId, value);
    }

    BOOL CustomExternalObject::SetConfigurable(PropertyId propertyId, BOOL value)
    {
        BOOL notPresent=false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetConfigurable(propertyId, value);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setConfigurable)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::SetConfigurable(propertyId, value);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setConfigurable))
        {
            HRESULT result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->SetConfigurable()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    globalObject->DynamicObject::SetConfigurable(propertyId, value);
                }
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetConfigurable)
            {
                result = this->GetTypeOperations()->SetConfigurable(scriptContext->GetActiveScriptDirect(), this, propertyId, value);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetConfigurable);
            IGNORE_HR(result)
            return true;
        }
        return ExternalObject::SetConfigurable(propertyId, value);
    }

    // TODO: WARNING: slow perf as it calls GetConfigurable/Enumerable/Writable individually.
    BOOL CustomExternalObject::SetAttributes(PropertyId propertyId, PropertyAttributes attributes)
    {
        return
            this->SetConfigurable(propertyId, attributes & PropertyConfigurable) &&
            this->SetEnumerable(propertyId, attributes & PropertyEnumerable) &&
            this->SetWritable(propertyId, attributes & PropertyWritable);
        // Note that although technically it's not valid to set writable to accessor properties,
        // internally we keep writable as true for accessors, and take it out before returning
        // property descriptor to the user. In other words, (1) it's OK to do and (2) we need it
        // as this is low-level call.
    }

    BOOL CustomExternalObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        BOOL notPresent = false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::SetAccessors(propertyId, getter, setter, flags);
        }
        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_setAccessors)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::SetAccessors(propertyId, getter, setter, flags);
            }
            else
            {
                notPresent = true;
            }
        }
        if (notPresent || (this->GetOperationUsage().useAlways & OperationFlag_setAccessors))
        {
            HRESULT result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();
            if (getter)
            {
                getter = CrossSite::MarshalVar(GetScriptContext(), getter);
            }
            if (setter)
            {
                setter = CrossSite::MarshalVar(GetScriptContext(), setter);
            }

            // CONSIDER: We should be able to go directly to globalObject->SetAccessors()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                if (globalObject->DynamicObject::HasProperty(propertyId))
                {
                    return globalObject->DynamicObject::SetAccessors(propertyId, getter, setter, flags);
                }
            }

            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetAccessors)
            {
                result = this->GetTypeOperations()->SetAccessors(scriptContext->GetActiveScriptDirect(), this, propertyId, getter, setter);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_SetAccessors);
            IGNORE_HR(result)
            return true;
        }
        return ExternalObject::SetAccessors(propertyId, getter, setter, flags);
    }

    BOOL CustomExternalObject::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext)
    {
        BOOL notPresent = false;
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetAccessors(propertyId, getter, setter, requestContext);
        }

        if (this->GetOperationUsage().useWhenPropertyNotPresent & OperationFlag_getAccessors)
        {
            if (JavascriptConversion::PropertyQueryFlagsToBoolean(ExternalObject::HasPropertyQuery(propertyId)))
            {
                return ExternalObject::GetAccessors(propertyId, getter, setter, requestContext);
            }
            else
            {
                notPresent = true;
            }
        }

        if (this->GetOperationUsage().useWhenPropertyNotPresentInPrototypeChain & OperationFlagsForNamespaceOrdering_allGetPropertyOperations)
        {
            if (JavascriptOperators::HasProperty(this->GetPrototype(), propertyId))
            {
                BOOL result = JavascriptOperators::GetAccessors(this->GetPrototype(), propertyId, requestContext, getter, setter);
                if (result)
                {
                    *getter = CrossSite::MarshalVar(requestContext, *getter);
                    *setter = CrossSite::MarshalVar(requestContext, *setter);
                }
                return result;
            }
            else
            {
                notPresent = true;
            }
            if (!this->VerifyObjectAlive())
            {
                return FALSE;
            }
        }

        if (notPresent || this->GetOperationUsage().useAlways & OperationFlag_getAccessors)
        {
            HRESULT hr = E_FAIL;
            BOOL result = FALSE;
            ScriptContext * scriptContext = this->GetScriptContext();
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                *getter = requestContext->GetLibrary()->GetNull();
                *setter = requestContext->GetLibrary()->GetNull();
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(requestContext, Js::JavascriptOperators::GetTypeId(this), requestContext->GetThreadContext()->GetPropertyName(propertyId), CustomExternalObject_GetAccessors)
            {
                hr = this->GetTypeOperations()->GetAccessors(scriptContext->GetActiveScriptDirect(), this, propertyId, getter, setter, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(requestContext, Js::JavascriptOperators::GetTypeId(this), requestContext->GetThreadContext()->GetPropertyName(propertyId), CustomExternalObject_GetAccessors);
            IGNORE_HR(hr)
            if (result)
            {
                if (*getter)
                {
                    *getter = Js::CrossSite::MarshalVar(requestContext, *getter);
                }
                if (*setter)
                {
                    *setter = Js::CrossSite::MarshalVar(requestContext, *setter);
                }
                return TRUE;
            }
            return FALSE;
        }
        return ExternalObject::GetAccessors(propertyId, getter, setter, requestContext);
    }

    DescriptorFlags CustomExternalObject::GetSetter(PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (!this->VerifyObjectAlive())
        {
            return DescriptorFlags::None;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetSetter(propertyId, setterValue, info, requestContext);
        }
        DescriptorFlags flags = None;

        if (this->GetOperationUsage().useAlways & OperationFlag_getSetter)
        {
            HRESULT hr = E_FAIL;
            ScriptContext * scriptContext = this->GetScriptContext();

            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            // CONSIDER: We should be able to go directly to globalObject->GetSetter()
            if (this == globalObject->GetSecureDirectHostObject())
            {
                flags = globalObject->DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
                if ((flags & Accessor) == Accessor && *setterValue)
                {
                    *setterValue = CrossSite::MarshalVar(requestContext, *setterValue);
                }

                // we will fall back to type operator if we can't find the property in global object directly.
                if (flags != None)
                {
                    return flags;
                }
            }

            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                *setterValue = requestContext->GetLibrary()->GetNull();
                PropertyValueInfo::SetNoCache(info, this);
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return Accessor;
            }

            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetSetter)
            {
                hr = this->GetTypeOperations()->GetSetter(requestContext->GetActiveScriptDirect(), this, propertyId, setterValue, (::DescriptorFlags*)&flags);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), threadContext->GetPropertyName(propertyId), CustomExternalObject_GetSetter);
            IGNORE_HR(hr)

            // DictionaryTypeHandler only set the setterValue in Accessor scenario, so we don't need to worry about
            // the setterValue marshalling.
            // TODO: verify that we are not reading the setterValue in all different code path with Data flag set.
            if ((flags & Accessor) == Accessor && *setterValue)
            {
                // review: add inline cache if getsetter comes back to us. this shortcircuit the
                // ITypeOpertions
                DescriptorFlags tmpFlags;
                Var tempSetterValue;
                tmpFlags = ExternalObject::GetSetter(propertyId, &tempSetterValue, info, requestContext);
                if (tempSetterValue != *setterValue)
                {
                    PropertyValueInfo::SetNoCache(info, this);
                }
                else
                {
                    Assert(tmpFlags == flags);
                }

                *setterValue = CrossSite::MarshalVar(requestContext, *setterValue);
            }
        }
        else
        {
            flags = ExternalObject::GetSetter(propertyId, setterValue, info, requestContext);
        }

        return flags;
    }

    DescriptorFlags CustomExternalObject::GetSetter(JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return CustomExternalObject::GetSetter(propertyRecord->GetPropertyId(), setterValue, info, requestContext);
    }

    DescriptorFlags CustomExternalObject::GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext)
    {
        if (!this->VerifyObjectAlive())
        {
            return DescriptorFlags::None;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetItemSetter(index, setterValue, requestContext);
        }
        DescriptorFlags flags = None;

        if (this->GetOperationUsage().useAlways & OperationFlag_getItemSetter)
        {
            HRESULT hr = E_FAIL;
            ScriptContext * scriptContext = this->GetScriptContext();

            // CONSIDER: We should be able to go directly to globalObject->GetItemSetter()
            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                flags = globalObject->DynamicObject::GetItemSetter(index, setterValue, requestContext);
                if ((flags & Accessor) == Accessor && *setterValue)
                {
                    *setterValue = CrossSite::MarshalVar(requestContext, *setterValue);
                }

                // we will fall back to type operator if we can't find the property in global object directly.
                if (flags != None)
                {
                    return flags;
                }
            }

            // Reject implicit call
            ThreadContext * threadContext = requestContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                *setterValue = requestContext->GetLibrary()->GetNull();
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return Accessor;
            }

            Var varIndex = JavascriptNumber::ToVar(index, scriptContext);
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_GetItemSetter)
            {
                hr = this->GetTypeOperations()->GetItemSetter(requestContext->GetActiveScriptDirect(),
                    this, varIndex, setterValue, (::DescriptorFlags*)&flags);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), index, CustomExternalObject_GetItemSetter);
            IGNORE_HR(hr)
            if ((flags & Accessor) == Accessor && *setterValue)
            {
                *setterValue = CrossSite::MarshalVar(requestContext, *setterValue);
            }
        }
        else
        {
            flags = ExternalObject::GetItemSetter(index, setterValue, requestContext);
        }

        return flags;
    }

    HRESULT CustomExternalObject::QueryObjectInterface(REFIID riid, void **ppvObj)
    {
        HRESULT hr = E_FAIL;
        if (!this->IsObjectAlive())
        {
            return  E_NOINTERFACE;
        }
        ScriptContext * scriptContext = this->GetScriptContext();
        Assert(!scriptContext->GetThreadContext()->IsScriptActive());
        // TODO: with minimal call here adding etw might be too expensive.

        hr = this->GetTypeOperations()->QueryObjectInterface(scriptContext->GetActiveScriptDirect(), this, riid, ppvObj);
        if (FAILED(hr))
        {
            hr = ExternalObject::QueryObjectInterface(riid, ppvObj);
        }
        return hr;
    }

    // We should be able to get rid of Equals and StrictEquals post rs1.
    BOOL CustomExternalObject::Equals(__in Var other, __out BOOL* returnResult, ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive())
        {
            *returnResult = FALSE;
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::Equals(other, returnResult, requestContext);
        }
        if (this->GetOperationUsage().useAlways & OperationFlag_Equals)
        {
            BOOL result = FALSE;
            // Reject implicit call
            ThreadContext * threadContext = requestContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                *returnResult = FALSE;
                return TRUE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(requestContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(other), CustomExternalObject_Equals)
            {
                result = SUCCEEDED(this->GetTypeOperations()->Equals(requestContext->GetActiveScriptDirect(), this, other, returnResult));
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(requestContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(other), CustomExternalObject_Equals);
            return result;
        }
        return ExternalObject::Equals(other, returnResult, requestContext);
    }

    BOOL CustomExternalObject::StrictEquals(__in Var other, __out BOOL* returnResult, ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive())
        {
            *returnResult = FALSE;
            return FALSE;
        }
        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::StrictEquals(other, returnResult, requestContext);
        }
        if (this->GetOperationUsage().useAlways & OperationFlag_StrictEquals)
        {
            BOOL result = FALSE;
            // Reject implicit call
            ThreadContext * threadContext = requestContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                *returnResult = FALSE;
                return TRUE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(requestContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(other), CustomExternalObject_StrictEquals)
            {
                result = SUCCEEDED(this->GetTypeOperations()->StrictEquals(requestContext->GetActiveScriptDirect(), this, other, returnResult));
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(requestContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(other), CustomExternalObject_StrictEquals);
            return result;
        }
        return ExternalObject::StrictEquals(other, returnResult, requestContext);
    }

    DynamicType* CustomExternalObject::DuplicateType()
    {
        return CustomExternalType::New(this->GetCustomExternalType());
    }

    BOOL CustomExternalObject::HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        if (!this->VerifyObjectAlive())
        {
            return FALSE;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            Var funcPrototype = JavascriptOperators::GetProperty(this, PropertyIds::prototype, scriptContext);
            return JavascriptFunction::HasInstance(funcPrototype, instance, scriptContext, nullptr, nullptr);
        }
        if (this->GetOperationUsage().useAlways & OperationFlag_hasInstance)
        {
            HRESULT hr = E_FAIL;
            BOOL result = FALSE;

            GlobalObject* globalObject = scriptContext->GetGlobalObject();
            if (this == globalObject->GetSecureDirectHostObject())
            {
                return globalObject->HasInstance(instance, scriptContext);
            }
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return TRUE;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(instance), CustomExternalObject_HasInstance)
            {
                hr = this->GetTypeOperations()->HasInstance(scriptContext->GetActiveScriptDirect(), this, instance, &result);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(instance), CustomExternalObject_HasInstance);
            IGNORE_HR(hr)
            return result;
        }
        return ExternalObject::HasInstance(instance, scriptContext);
    }

    Var CustomExternalObject::GetNamespaceParent(Var instance)
    {
        if (!this->VerifyObjectAlive())
        {
            return nullptr;
        }

        if (this->GetCustomExternalType()->IsSimpleWrapper())
        {
            return ExternalObject::GetNamespaceParent(instance);
        }
        if (this->GetOperationUsage().useAlways & OperationFlag_getNamespaceParent)
        {
            Var namespaceParent = NULL;
            HRESULT hr = E_FAIL;
            ScriptContext * scriptContext = this->GetScriptContext();
            // Reject implicit call
            ThreadContext * threadContext = scriptContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return NULL;
            }
            BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(instance), CustomExternalObject_GetNamespaceParent)
            {
                hr = this->GetTypeOperations()->GetNamespaceParent(scriptContext->GetActiveScriptDirect(), instance, &namespaceParent);
            }
            END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, Js::JavascriptOperators::GetTypeId(this), Js::JavascriptOperators::GetTypeId(instance), CustomExternalObject_GetNamespaceParent);

            if (FAILED(hr) || NULL == namespaceParent)
            {
                return NULL;
            }
            namespaceParent = CrossSite::MarshalVar(scriptContext, namespaceParent);
            return namespaceParent;
        }
        return ExternalObject::GetNamespaceParent(instance);
    }

    RecyclableObject* CustomExternalObject::GetConfigurablePrototype(ScriptContext * requestContext)
    {
        if (!this->VerifyObjectAlive())
        {
            return nullptr;
        }
        if ((this->GetOperationUsage().useAlways & OperationFlag_crossDomainCheck) == OperationFlag_crossDomainCheck)
        {
            GlobalObject* globalObject = this->GetScriptContext()->GetGlobalObject();
            // Reject implicit call
            ThreadContext * threadContext = requestContext->GetThreadContext();
            if (threadContext->IsDisableImplicitCall())
            {
                threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
                return requestContext->GetLibrary()->GetNull();
            }
            if (this != globalObject->GetSecureDirectHostObject())
            {
                BOOL resultCrossDomainCheck;
                HRESULT hr;
                hr = this->GetTypeOperations()->CrossDomainCheck(globalObject->GetScriptContext()->GetActiveScriptDirect(),this, &resultCrossDomainCheck);
                IGNORE_HR(hr)
                if ( resultCrossDomainCheck )
                {
                    return requestContext->GetLibrary()->GetNull();
                }
            }
        }
        return ExternalObject::GetConfigurablePrototype(requestContext);
    }

    void CustomExternalObject::SetPrototype(RecyclableObject* newPrototype)
    {
        if (!this->VerifyObjectAlive())
        {
            return;
        }

        GlobalObject* globalObject = this->GetScriptContext()->GetGlobalObject();
        if (this == globalObject->GetSecureDirectHostObject())
        {
            return; // reject window
        }
        Assert(this != globalObject->GetDirectHostObject());

        ExternalObject::SetPrototype(newPrototype);
    }

    void CustomExternalObject::CacheJavascriptDispatch(JavascriptDispatch* javascriptDispatch)
    {
        // GC will keep the instance alive.
        this->cachedJavascriptDispatch = javascriptDispatch;
    }

}
