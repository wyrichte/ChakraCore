//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#define ReturnDefaultTypeOperationsMethodResult(methodName, scriptDirect, instance, ...)      return __super::methodName(scriptDirect,instance, __VA_ARGS__);
#define GetDefaultTypeOperationsMethodResult(methodName, scriptDirect, instance, ...)         HRESULT hr = __super::methodName(scriptDirect,instance, __VA_ARGS__);
#define ReturnHrIfNotEBOUNDS() \
    if (hr != E_BOUNDS) \
    { \
        return hr; \
    }

namespace Projection
{
    InspectableObjectTypeOperations::InspectableObjectTypeOperations(
        ProjectionContext *projectionContext, 
        SpecialProjection * specialization)
        : ProjectionTypeOperations(), specialization(specialization)
    {
#if DBG
        scriptContext = projectionContext->GetScriptContext();
#endif
    }

    HRESULT InspectableObjectTypeOperations::Create(
        __in ProjectionContext *projectionContext, 
        __in SpecialProjection * specialization,
        __out InspectableObjectTypeOperations** newInstance)
    {
        IfNullReturnError(newInstance, E_INVALIDARG);
        *newInstance = nullptr;

        Recycler* recycler = projectionContext->GetScriptContext()->GetRecycler();
        InspectableObjectTypeOperations* pInspectableObjectTypeOperations = RecyclerNew(recycler, InspectableObjectTypeOperations, projectionContext, specialization);
        *newInstance = pInspectableObjectTypeOperations;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::HasOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        // REVIEW: deferinit here? override?
        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, result);
            ReturnHrIfNotEBOUNDS();
            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::HasOwnProperty(specialization, instance, propertyId, result);
        }

        ReturnDefaultTypeOperationsMethodResult(HasOwnProperty, scriptDirect, instance, propertyId, result);
    }


    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_INVALIDARG);
        *propertyPresent = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for
            HRESULT hr = VectorArray::GetOwnProperty(specialization, instance, propertyId, value, propertyPresent);
            ReturnHrIfNotEBOUNDS();

            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::GetOwnProperty(specialization, instance, propertyId, value, propertyPresent);
        }
        ReturnDefaultTypeOperationsMethodResult(GetOwnProperty, scriptDirect, instance, propertyId, value, propertyPresent);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetPropertyReference(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(propertyPresent, E_INVALIDARG);
        *propertyPresent = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for
            HRESULT hr = VectorArray::GetOwnProperty(specialization, instance, propertyId, value, propertyPresent);
            ReturnHrIfNotEBOUNDS();

            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::GetOwnProperty(specialization, instance, propertyId, value, propertyPresent);
        }
        ReturnDefaultTypeOperationsMethodResult(GetPropertyReference, scriptDirect, instance, propertyId, value, propertyPresent);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for
            HRESULT hr = VectorArray::SetProperty(specialization, instance, propertyId, value, result);
            ReturnHrIfNotEBOUNDS();

            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::SetProperty(specialization, instance, propertyId, value, result);
        }

        ReturnDefaultTypeOperationsMethodResult(SetProperty, scriptDirect, instance, propertyId, value, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::DeleteProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::DeleteProperty(specialization, instance, propertyId, result);
        }

#if DBG
        uint32 index = 0;
        Assert (!scriptContext->IsNumericPropertyId(propertyId, &index));
#endif

        ReturnDefaultTypeOperationsMethodResult(DeleteProperty, scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var *value,
        /* [out] */ BOOL *itemPresent) 
    {
        IfNullReturnError(value, E_INVALIDARG);
        *value = nullptr;
        IfNullReturnError(itemPresent, E_INVALIDARG);
        *itemPresent = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            return VectorArray::GetItem(specialization, instance, index, value, itemPresent);
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::GetOwnItem(specialization, instance, index, value, itemPresent);
        }
        ReturnDefaultTypeOperationsMethodResult(GetOwnItem, scriptDirect, instance, index, value, itemPresent);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            return VectorArray::SetItem(specialization, instance, index, value, result);
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::SetItem(specialization, instance, index, value, result);
        }
        ReturnDefaultTypeOperationsMethodResult(SetItem, scriptDirect, instance, index, value, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::DeleteItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            return VectorArray::DeleteItem(specialization, instance, index, result);
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::DeleteItem(specialization, instance, index, result);
        }
        ReturnDefaultTypeOperationsMethodResult(DeleteItem, scriptDirect, instance, index, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetEnumerator(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ BOOL enumNonEnumerable,
        /* [in] */ BOOL enumSymbols,
        /* [out] */ IVarEnumerator **enumerator) 
    {
        IfNullReturnError(enumerator, E_INVALIDARG);
        *enumerator = nullptr;

        if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::GetEnumerator(specialization, instance, enumerator);
        }

        // create two enumerators, based if we do return a single or one encapsulating the other
        CComPtr<IVarEnumerator> pEnumerator;
        CComPtr<IVarEnumerator> pOuterEnumerator;
        GetDefaultTypeOperationsMethodResult(GetEnumerator, scriptDirect, instance, enumNonEnumerable, enumSymbols, &pEnumerator);

        if (HasVectorOrVectorViewArrayLikeProjection() && SUCCEEDED(hr))
        {
            hr = VectorArray::GetEnumerator(specialization, instance, pEnumerator, &pOuterEnumerator);
        }

        // return the output pointer and transfer (release) ownership to caller
        if (pOuterEnumerator != nullptr)
        {
            *enumerator = pOuterEnumerator;
            pOuterEnumerator.Detach();
        }
        else
        {
            *enumerator = pEnumerator;
            pEnumerator.Detach();
        }

        return hr;
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::IsEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for - then it is enumerable
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, result);
            ReturnHrIfNotEBOUNDS();

            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            // if we have property we can enumerate it
            if (!MapWithStringKey::HasPrototypeProperty(specialization, propertyId))
            {
                *result = TRUE;
                return S_OK;
            }
        }
        ReturnDefaultTypeOperationsMethodResult(IsEnumerable, scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::IsWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for - then it is writable
            HRESULT hr = VectorArray::IsWritable(specialization, instance, propertyId, result);
            ReturnHrIfNotEBOUNDS();

            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            // If we have property we can write it
            if (!MapWithStringKey::HasPrototypeProperty(specialization, propertyId))
            {
                *result = TRUE;
                return S_OK;
            }
        }

        ReturnDefaultTypeOperationsMethodResult(IsWritable, scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::IsConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for - then it is configurable
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, result);
            if (hr != E_BOUNDS)
            {
                *result = FALSE;
                return hr;
            }
            
            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            // If we have property we can delete it
            if (!MapWithStringKey::HasPrototypeProperty(specialization, propertyId))
            {
                *result = TRUE;
                return S_OK;
            }
        }
        ReturnDefaultTypeOperationsMethodResult(IsConfigurable, scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // no change if its one of our own index
            BOOL fHasOwnProperty = FALSE;
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, &fHasOwnProperty);
            ReturnHrIfNotEBOUNDS();
            
            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            // If we have property dont do anything
            if (!MapWithStringKey::HasPrototypeProperty(specialization, propertyId))
            {
                return S_OK;
            }
        }
        ReturnDefaultTypeOperationsMethodResult(SetEnumerable, scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // no change if its one of our own index
            BOOL fHasOwnProperty = FALSE;
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, &fHasOwnProperty);
            ReturnHrIfNotEBOUNDS();
            
            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            // If we have property dont do anything
            if (!MapWithStringKey::HasPrototypeProperty(specialization, propertyId))
            {
                return S_OK;
            }
        }
        ReturnDefaultTypeOperationsMethodResult(SetWritable, scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // no change if its one of our own index
            BOOL fHasOwnProperty = FALSE;
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, &fHasOwnProperty);
            ReturnHrIfNotEBOUNDS();
            
            // otherwise search the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            // If we have property dont do anything
            if (!MapWithStringKey::HasPrototypeProperty(specialization, propertyId))
            {
                return S_OK;
            }
        }
        ReturnDefaultTypeOperationsMethodResult(SetConfigurable, scriptDirect, instance, propertyId, value);
    }


    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* getter,
        /* [out] */ Var* setter,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(getter, E_INVALIDARG);
        *getter = nullptr;
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            HRESULT hr = VectorArray::GetAccessors(specialization, propertyId, result);
            ReturnHrIfNotEBOUNDS();
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::GetAccessors(specialization, propertyId, result);
        }            
        // otherwise search the default property store
        ReturnDefaultTypeOperationsMethodResult(GetAccessors,scriptDirect, instance, propertyId, getter, setter, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var getter,
        /* [in] */ Var setter) 
    {
        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            HRESULT hr = VectorArray::SetAccessors(specialization, propertyId);
            ReturnHrIfNotEBOUNDS();
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::SetAccessors(specialization, propertyId);
        }            
            
        // otherwise search the default property store
        ReturnDefaultTypeOperationsMethodResult(SetAccessors, scriptDirect, instance, propertyId, getter, setter);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetSetter( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(flags, E_INVALIDARG);
        *flags = ::DescriptorFlags_None;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            BOOL hasOwnProperty;
            HRESULT hr = VectorArray::HasOwnProperty(specialization, instance, propertyId, &hasOwnProperty);
            if (hr != E_BOUNDS)
            {
                if (SUCCEEDED(hr))
                {
                    *flags = VectorArray::IsVector(specialization) ? DescriptorFlags_Writable : DescriptorFlags_Data;
                }
                return hr;
            }
        }
        else if (HasMapOrMapViewSpecialization())
        {
            BOOL hasOwnProperty;
            HRESULT hr = this->HasOwnProperty(scriptDirect, instance, propertyId, &hasOwnProperty);
            IfFailedReturn(hr);

            *flags = (hasOwnProperty == TRUE) ? DescriptorFlags_Writable : DescriptorFlags_None;
            return hr;
        }       
    
        ReturnDefaultTypeOperationsMethodResult(GetSetter, scriptDirect, instance, propertyId, setter, flags);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::Equals( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        ProjectionObjectInstance *instanceObjectInstance = GetProjectionObjectInstanceFromVarNoThrow(instance);
        ProjectionObjectInstance *instanceObjectOther = GetProjectionObjectInstanceFromVarNoThrow(other);

        if (instanceObjectInstance != NULL &&  instanceObjectOther != NULL)
        {
            *result = instanceObjectInstance->IsEqual(instanceObjectOther);
            return S_OK;
        }

        return __super::Equals(scriptDirect, instance, other, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::StrictEquals( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [out] */ BOOL *result) 
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        ProjectionObjectInstance *instanceObjectInstance = GetProjectionObjectInstanceFromVarNoThrow(instance);
        ProjectionObjectInstance *instanceObjectOther = GetProjectionObjectInstanceFromVarNoThrow(other);

        if (instanceObjectInstance != NULL &&  instanceObjectOther != NULL)
        {
            *result = instanceObjectInstance->IsEqual(instanceObjectOther);
            return S_OK;
        }

        return __super::StrictEquals(scriptDirect, instance, other, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::QueryObjectInterface(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ REFIID riid,
        /* [out] */ void **ppvObj) 
    {
        IfNullReturnError(ppvObj, E_INVALIDARG);
        *ppvObj = nullptr;

        ProjectionObjectInstance * projectionObject = GetProjectionObjectInstanceFromVarNoThrow(instance);
        if (projectionObject == nullptr)
        {
            return E_POINTER;
        }

        IUnknown *unknown = projectionObject->GetNativeABI();
        if (unknown == nullptr)
        {
            return E_POINTER;
        }
        
        return unknown->QueryInterface(riid, ppvObj);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetHeapObjectInfo(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ ProfilerHeapObjectInfoFlags flags,
        /* [out] */ HostProfilerHeapObject** result,
        /* [out] */ HeapObjectInfoReturnResult* returnResult)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = nullptr;
        IfNullReturnError(returnResult, E_INVALIDARG);
        *returnResult = HeapObjectInfoReturnResult_NoResult;

        Assert(ProjectionObjectInstance::Is(instance));
        
        // We dont need to fill in info about size, flags and objectId because that will be taken care of by common method
        if (flags == ProfilerHeapObjectInfoFull)
        {
            return ((ProjectionObjectInstance *)instance)->GetFullHeapObjectInfo(result, returnResult);
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::HasOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ Var index,
        /* [out] */ BOOL* result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            return VectorArray::HasItem(specialization, instance, index, result);
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::HasOwnItem(specialization, instance, index, result);
        }   

        ReturnDefaultTypeOperationsMethodResult(HasOwnItem, scriptDirect, instance, index, result);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::GetItemSetter(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index, 
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags)
    {
        IfNullReturnError(setter, E_INVALIDARG);
        *setter = nullptr;
        IfNullReturnError(flags, E_INVALIDARG);
        *flags = ::DescriptorFlags_None;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            BOOL hasOwnProperty;
            HRESULT hr = this->HasOwnItem(scriptDirect, instance, index, &hasOwnProperty);
            IfFailedReturn(hr);

            *flags = VectorArray::IsVector(specialization) ? DescriptorFlags_Writable : DescriptorFlags_Data;
            return hr;
        }
        else if (HasMapOrMapViewSpecialization())
        {
            BOOL hasOwnProperty;
            HRESULT hr = this->HasOwnItem(scriptDirect, instance, index, &hasOwnProperty);
            IfFailedReturn(hr);

            *flags = (hasOwnProperty == TRUE) ? DescriptorFlags_Writable : DescriptorFlags_None;
            return hr;
        }       
    
        ReturnDefaultTypeOperationsMethodResult(GetItemSetter, scriptDirect, instance, index, setter, flags);
    }

    HRESULT STDMETHODCALLTYPE InspectableObjectTypeOperations::SetPropertyWithAttributes(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ PropertyAttributes attributes,
            /* [in] */ SideEffects effects, 
            /* [out]*/ BOOL* result)
    {
        IfNullReturnError(result, E_INVALIDARG);
        *result = FALSE;

        if (HasVectorOrVectorViewArrayLikeProjection())
        {
            // check if it is one of the index that we have values for
            HRESULT hr = VectorArray::SetProperty(specialization, instance, propertyId, value, result);
            ReturnHrIfNotEBOUNDS();

            // otherwise set using the default property store
        }
        else if (HasMapOrMapViewSpecialization())
        {
            return MapWithStringKey::SetProperty(specialization, instance, propertyId, value, result);
        }   

        ReturnDefaultTypeOperationsMethodResult(SetPropertyWithAttributes, scriptDirect, instance, propertyId, value, attributes, effects, result);    
    }
}
