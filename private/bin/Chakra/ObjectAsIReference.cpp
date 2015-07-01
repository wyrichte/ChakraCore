//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterable in JavaScript
// *******************************************************


#include "stdafx.h"

namespace Projection
{
    Define_InspectableImpl_VTable(g_IReferenceVtable,
        CUnknownImpl_VTableEntry(ObjectAsIReference, get_ValueIReference));

    Define_InspectableImpl_VTable(g_IReferenceArrayVtable,
        CUnknownImpl_VTableEntry(ObjectAsIReference, get_ValueIReferenceArray));

    ObjectAsIReference::ObjectAsIReference(ProjectionContext *projectionContext, PFN_VTABLE_ENTRY *pvtbl)
        : CUnknownImpl(projectionContext, pvtbl
#if DBG_DUMP
            , pvtbl == g_IReferenceVtable ? referenceWrapper : referenceArrayWrapper
#endif
        ), 
        m_pPropertyValue(nullptr), 
        finalizableTypedArrayContents(nullptr)
    {
        getValueId = IdOfString(projectionContext->GetScriptContext(), L"getValue");
    }


    ObjectAsIReference::~ObjectAsIReference()
    {
        // If not initialised dont do anything
        if (finalizableTypedArrayContents == nullptr)
        {
            return;
        }

        if (m_pPropertyValue)
        {
            delete m_pPropertyValue;
        }
        JS_ETW(EventWriteJSCRIPT_RECYCLER_FREE_WINRT_PROPERTYVALUE_OBJECT(this));
    }

    HRESULT ObjectAsIReference::Initialize(
            __in bool isArray,
            __in size_t typeStorageSize,
            __in_bcount(typeStorageSize) byte *typeStorage,
            __in RtCONCRETETYPE elementType)
    {
        Assert(elementType != nullptr);

        UINT32 numberOfElements = typeStorageSize/ elementType->storageSize;

        // Get IID of IReference<elementType>
        ArenaAllocator *a = projectionContext->ProjectionAllocator();
        IID instantiatedIID;
        ImmutableList<RtTYPE> *genericInstantiations = ImmutableList<RtTYPE>::Empty();
        genericInstantiations = genericInstantiations->Prepend(elementType, a);
        LPCWSTR parentTypeName = (isArray) ? L"Windows.Foundation.IReferenceArray`1" :  L"Windows.Foundation.IReference`1";
        auto parentTypeNameId = IdOfString(projectionContext->GetScriptContext(), parentTypeName);

        HRESULT hr = projectionContext->GetProjectionBuilder()->GetInstantiatedIID(
            parentTypeNameId, 
            genericInstantiations, 
            &HeapAllocator::Instance,
            &instantiatedIID);
        IfFailedReturn(hr);

        Js::PropertyId fullObjectReferenceNameId = GetGenericInstantiationNameFromParentName(parentTypeNameId, genericInstantiations, a, projectionContext->GetStringConverter());
        hr = __super::Initialize(instantiatedIID, StringOfId(projectionContext->GetScriptContext(), fullObjectReferenceNameId));
        IfFailedReturn(hr);

        this->isArray = isArray;

        // Read it in TypeStorage instead
        byte *newStorage = new byte[typeStorageSize];
        get_Value(typeStorageSize, newStorage, elementType, numberOfElements, typeStorageSize, typeStorage);

        Recycler *recycler = projectionContext->GetScriptContext()->GetRecycler();

#if DBG_DUMP
        finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, numberOfElements, newStorage, releaseBufferUsingDeleteArray, (ProjectionMemoryInformation*) projectionContext->GetThreadContext()->GetProjectionContextMemoryInformation());
#else
        finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, numberOfElements, newStorage, releaseBufferUsingDeleteArray);
#endif

        finalizableTypedArrayContents->Initialize();

        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_PROPERTYVALUE_OBJECT(this, this->GetFullTypeName(), isArray));

        AddRef();
        return hr;
    }

    HRESULT ObjectAsIReference::CreateIReference(
            __in ProjectionContext *projectionContext, 
            __in size_t typeStorageSize,
            __in_bcount(typeStorageSize) byte *typeStorage,
            __in RtCONCRETETYPE elementType,
            __out ObjectAsIReference **newObjectAsIReference)
    {
        IfNullReturnError(newObjectAsIReference, E_INVALIDARG);
        *newObjectAsIReference = nullptr;
        Assert(projectionContext != nullptr);
        Assert(typeStorage != nullptr);
        Assert(elementType != nullptr);

        ObjectAsIReference *pIReference = new ObjectAsIReference(projectionContext, g_IReferenceVtable);
        IfNullReturnError(pIReference, E_OUTOFMEMORY);

        HRESULT hr = pIReference->Initialize(false, typeStorageSize, typeStorage, elementType);
        if (FAILED(hr))
        {
            delete pIReference;
            return hr;
        }

        *newObjectAsIReference = pIReference;
        return hr;
    }

    HRESULT ObjectAsIReference::CreateIReferenceArray(
            __in ProjectionContext *projectionContext, 
            __in size_t typeStorageSize,
            __in_bcount(typeStorageSize) byte *typeStorage,
            __in RtCONCRETETYPE elementType,
            __out ObjectAsIReference **newObjectAsIReference)
    {
        IfNullReturnError(newObjectAsIReference, E_INVALIDARG);
        *newObjectAsIReference = nullptr;
        Assert(projectionContext != nullptr);
        Assert(typeStorage != nullptr);
        Assert(elementType != nullptr);

        ObjectAsIReference *pIReference = new ObjectAsIReference(projectionContext, g_IReferenceArrayVtable);
        IfNullReturnError(pIReference, E_OUTOFMEMORY);

        HRESULT hr = pIReference->Initialize(true, typeStorageSize, typeStorage, elementType);
        if (FAILED(hr))
        {
            delete pIReference;
            return hr;
        }

        *newObjectAsIReference = pIReference;
        return hr;
    }

    HRESULT ObjectAsIReference::GetPropertyValue(void **ppv)
    {
        if (m_pPropertyValue == nullptr)
        {
            HRESULT hr = ObjectAsIPropertyValue::Create(projectionContext, this, &m_pPropertyValue);
            IfFailedReturn(hr);
        }
        else
        {
            m_pPropertyValue->AddRef();
        }

        *ppv = m_pPropertyValue->GetUnknown();
        return S_OK;
    }

    CUnknownMethodNoError_Prolog(ObjectAsIReference, QueryInterface, REFIID riid, void **ppv)
    {
        HRESULT hr = __super::QueryInterface(riid, ppv);

        if (hr == E_NOINTERFACE && IsEqualGUID(riid, Windows::Foundation::IID_IPropertyValue))
        {
            hr = GetPropertyValue(ppv);
        }

        return hr;
    }
    CUnknownMethodNoError_Epilog()
        
    CUnknownMethodNoError_Prolog(ObjectAsIReference, GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        return GetTwoIids(Windows::Foundation::IID_IPropertyValue, iidCount, iids);
    }
    CUnknownMethodNoError_Epilog()

#pragma warning(push)
#pragma warning(disable:28285)
#pragma warning(disable:28202)
    CUnknownMethodImpl_Prolog(ObjectAsIReference, get_ValueIReference, (value), __out_bcount(finalizableTypedArrayContents->elementType->storageSize) byte *value)
    {
        IfNullReturnError(value, E_INVALIDARG);
        ZeroMemory(value, finalizableTypedArrayContents->elementType->storageSize * sizeof(byte));

        Assert(!isArray);
        hr = get_Value(finalizableTypedArrayContents->elementType->storageSize, value);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIReference, get_ValueIReferenceArray, (length, value), __out UINT32 *length, __out_bcount(sizeof(LPVOID)) byte *value)
    {
        IfNullReturnError(length, E_POINTER);
        *length = 0;
        IfNullReturnError(value, E_POINTER);
        *value = 0;
        Assert(isArray);
        byte *allocatedArray = (byte *)CoTaskMemAlloc(finalizableTypedArrayContents->elementType->storageSize * finalizableTypedArrayContents->numberOfElements);
        if (allocatedArray != nullptr)
        {
            hr = get_Value(finalizableTypedArrayContents->elementType->storageSize * finalizableTypedArrayContents->numberOfElements, allocatedArray);
            if (SUCCEEDED(hr))
            {
                *((LPVOID *)value) = allocatedArray;
                *length = finalizableTypedArrayContents->numberOfElements;
            }
            else
            {
                CoTaskMemFree(allocatedArray);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    CUnknownMethodImpl_Epilog()
#pragma warning(pop)

    HRESULT ObjectAsIReference::get_Value(__in size_t valueSize, __in_bcount(valueSize) byte *value, __in RtCONCRETETYPE elementType, __in uint32 numberOfElements, __in size_t srcValueSize, __in_bcount_opt(srcValueSize) byte *srcValue)
    {
        Assert(finalizableTypedArrayContents != nullptr || srcValue != nullptr);
        Assert(value != nullptr);

        if (srcValue == nullptr)
        {
            elementType = finalizableTypedArrayContents->elementType;
            numberOfElements = finalizableTypedArrayContents->numberOfElements;
            srcValue = finalizableTypedArrayContents->typedArrayBuffer;
        }

        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
        for (UINT32 index = 0; index < numberOfElements; index++)
        {
            byte *elementPointer = srcValue + (index * elementType->storageSize);
            Var elementVar = marshal.ReadOutType(nullptr, elementType, true, elementPointer, elementType->storageSize, getValueId);

            byte *destinationPointer = value + (index * elementType->storageSize);
            marshal.WriteInType(elementVar, elementType, destinationPointer, elementType->storageSize, true);
        }

        return S_OK;
    }
    
    void ObjectAsIReference::MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect)
    {
        if (finalizableTypedArrayContents != nullptr)
        {
            Assert(finalizableTypedArrayContents->typedArrayBuffer != nullptr);
            recycler->TryMarkNonInterior(finalizableTypedArrayContents);
        }
    }

    LPCWSTR ObjectAsIReference::GetFullElementTypeName() 
    { 
        return StringOfId(projectionContext->GetScriptContext(), finalizableTypedArrayContents->elementType->fullTypeNameId); 
    }

    USHORT ObjectAsIReference::GetWinrtTypeFlags()
    {
        return PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE;
    }

    UINT ObjectAsIReference::GetHeapObjectRelationshipInfoSize()
    {
        // If this is delegate array then only we have something to report
        if (isArray && ProjectionModel::DelegateType::Is(finalizableTypedArrayContents->elementType))
        {
            return finalizableTypedArrayContents->GetHeapObjectOptionalIndexPropertiesSize();
        }

        return 0;
    }

    void ObjectAsIReference::FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo)
    {
        Assert(isArray && finalizableTypedArrayContents != nullptr && ProjectionModel::DelegateType::Is(finalizableTypedArrayContents->elementType));
        finalizableTypedArrayContents->FillHeapObjectOptionalIndexProperties(this->GetHeapEnum(), optionalInfo, projectionContext->GetScriptContext());
    }
}
