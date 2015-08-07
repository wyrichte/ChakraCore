//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Projection
{
    ITypeOperations *GetDefaultTypeOperations()
    {
        return (ITypeOperations *)&Js::DefaultScriptOperations::s_DefaultScriptOperations;
    }

    OperationUsage ProjectionTypeOperations::projectionTypeOperationUsage =
    {
       (OperationFlags)(((int)OperationFlag_all & (~(int)OperationFlag_hasInstance))),
       OperationFlag_none,
       OperationFlagsForNamespaceOrdering_none
    };

    ProjectionTypeOperations::ProjectionTypeOperations() : refCount(1)
    {
    }

    // *** Overrides of DefaultScriptOperations
    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)  
    {
        if (riid == _uuidof(ITypeOperations))
        {
            *ppvObject = (ITypeOperations*)this;
        }
        else if (riid == IID_IUnknown)
        {
            *ppvObject = (IUnknown*)this;
        }
        else if (riid == IID_IProjectionTypeOperations)
        {
            *ppvObject = (IUnknown*)this;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return NOERROR;
    }

    ULONG STDMETHODCALLTYPE ProjectionTypeOperations::AddRef( void) 
    {
        ULONG newRef  = InterlockedIncrement(&refCount);
        return newRef;
    }

    ULONG STDMETHODCALLTYPE ProjectionTypeOperations::Release() 
    {
        ULONG newRef = InterlockedDecrement(&refCount);
        return newRef;
    }    

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetOperationUsage(
        /* [out] */ OperationUsage *flags) 
    {
        Assert(flags != nullptr);
        *flags = projectionTypeOperationUsage; 

#if DBG
        OperationUsage defaultUsage;
        HRESULT hr = Projection::GetDefaultTypeOperations()->GetOperationUsage(&defaultUsage);
        Assert(SUCCEEDED(hr));
        Assert((OperationFlags)((int)defaultUsage.useAlways & (~(int)(OperationFlag_hasInstance))) == flags->useAlways);
        Assert(defaultUsage.useWhenPropertyNotPresent == flags->useWhenPropertyNotPresent);
        Assert(defaultUsage.useWhenPropertyNotPresentInPrototypeChain == flags->useWhenPropertyNotPresentInPrototypeChain);
#endif

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::HasOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->HasOwnProperty(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetOwnProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        return Projection::GetDefaultTypeOperations()->GetOwnProperty(scriptDirect, instance, propertyId, value, propertyPresent);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetPropertyReference(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent) 
    {
        return Projection::GetDefaultTypeOperations()->GetPropertyReference(scriptDirect, instance, propertyId, value, propertyPresent);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->SetProperty(scriptDirect, instance, propertyId, value, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::DeleteProperty(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->DeleteProperty(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var *value,
        /* [out] */ BOOL *itemPresent) 
    {
        return Projection::GetDefaultTypeOperations()->GetOwnItem(scriptDirect, instance, index, value, itemPresent);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [in] */ Var value,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->SetItem(scriptDirect, instance, index, value, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::DeleteItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->DeleteItem(scriptDirect, instance, index, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetEnumerator(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ BOOL enumNonEnumerable,
        /* [in] */ BOOL enumSymbols,
        /* [out] */ IVarEnumerator **enumerator) 
    {
        return Projection::GetDefaultTypeOperations()->GetEnumerator(scriptDirect, instance, enumNonEnumerable, enumSymbols, enumerator);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::IsEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->IsEnumerable(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::IsWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->IsWritable(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::IsConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->IsConfigurable(scriptDirect, instance, propertyId, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetEnumerable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        return Projection::GetDefaultTypeOperations()->SetEnumerable(scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetWritable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        return Projection::GetDefaultTypeOperations()->SetWritable(scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetConfigurable(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value) 
    {
        return Projection::GetDefaultTypeOperations()->SetConfigurable(scriptDirect, instance, propertyId, value);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* getter,
        /* [out] */ Var* setter,
        /* [out] */ BOOL* result)
    {
        return Projection::GetDefaultTypeOperations()->GetAccessors(scriptDirect, instance, propertyId, getter, setter, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetAccessors(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var getter,
        /* [in] */ Var setter) 
    {
        return Projection::GetDefaultTypeOperations()->SetAccessors(scriptDirect, instance, propertyId, getter, setter);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetSetter( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags)
    {
        return Projection::GetDefaultTypeOperations()->GetSetter(scriptDirect, instance, propertyId, setter, flags);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::Equals( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->Equals(scriptDirect, instance, other, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::StrictEquals( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [out] */ BOOL *result) 
    {
        return Projection::GetDefaultTypeOperations()->StrictEquals(scriptDirect, instance, other, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::QueryObjectInterface(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ REFIID riid,
        /* [out] */ void **ppvObj) 
    {
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetInitializer(
        /* [out] */ InitializeMethod * initializer,
        /* [out] */ int * initSlotCapacity,
        /* [out] */ BOOL * hasAccessors) 
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetFinalizer(
        /* [out] */ FinalizeMethod * finalizer) 
    {
        return E_NOTIMPL; 
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::HasInstance(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var constructor,
        /* [in] */ Var instance, 
        /* [out] */ BOOL* result) 
    {
        Assert(false);
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetNamespaceParent(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [out] */ Var* namespaceParent) 
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::CrossDomainCheck(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [out] */ BOOL* result)
    {
        return Projection::GetDefaultTypeOperations()->CrossDomainCheck(scriptDirect, instance, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetHeapObjectInfo(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ ProfilerHeapObjectInfoFlags flags,
        /* [out] */ HostProfilerHeapObject** result,
        /* [out] */ HeapObjectInfoReturnResult* returnResult)
    {
        AssertMsg(returnResult, "The return result cannot be null.");
        *returnResult = HeapObjectInfoReturnResult_NoResult;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::HasOwnItem(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance, 
        /* [in] */ Var index,
        /* [out] */ BOOL* result)
    {
        return Projection::GetDefaultTypeOperations()->HasOwnItem(scriptDirect, instance, index, result);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::GetItemSetter(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index, 
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags)
    {
        return Projection::GetDefaultTypeOperations()->GetItemSetter(scriptDirect, instance, index, setter, flags);
    }

    HRESULT STDMETHODCALLTYPE ProjectionTypeOperations::SetPropertyWithAttributes(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ PropertyAttributes attributes,
            /* [in] */ SideEffects effects, 
            /* [out]*/ BOOL* result)
    {
        return Projection::GetDefaultTypeOperations()->SetPropertyWithAttributes(scriptDirect, instance, propertyId, value, attributes, effects, result);
    }

    bool IsWinRTType(Js::CustomExternalObject *ceo)
    {
        Js::ExternalType * externalType = (Js::ExternalType *)ceo->GetType();
        CComPtr<ProjectionTypeOperations> projectionTypeOperations = nullptr;
        return (SUCCEEDED(externalType->GetTypeOperations()->QueryInterface(IID_IProjectionTypeOperations, (void**)&projectionTypeOperations)));
    }

    bool IsWinRTConstructorFunction(Js::RecyclableObject *ro)
    {
        Js::TypeId typeId = ro->GetTypeId();
        if (typeId == Js::TypeIds_Function)
        {
            auto function = Js::JavascriptFunction::FromVar(ro);
            if (function->IsWinRTFunction())
            {
                auto winrtFunction = Js::JavascriptWinRTFunction::FromVar(ro);
                if (winrtFunction->IsConstructorFunction())
                {
                    return true;
                }
            }
        }

        return false;
    }
}