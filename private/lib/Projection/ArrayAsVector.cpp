//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IVector in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    Define_InspectableImpl_VTable(g_VectorVtable,
        CUnknownImpl_VTableEntry(ArrayAsVector, GetAt),
        CUnknownImpl_VTableEntry(ArrayAsVector, get_Size),
        CUnknownImpl_VTableEntry(ArrayAsVector, GetView),
        CUnknownImpl_VarArgT_VTableEntry(ArrayAsVector, IndexOf),
        CUnknownImpl_VarArgT_VTableEntry(ArrayAsVector, SetAt),
        CUnknownImpl_VarArgT_VTableEntry(ArrayAsVector, InsertAt),
        CUnknownImpl_VTableEntry(ArrayAsVector, RemoveAt),
        CUnknownImpl_VarArgT_VTableEntry(ArrayAsVector, Append),
        CUnknownImpl_VTableEntry(ArrayAsVector, RemoveAtEnd),
        CUnknownImpl_VTableEntry(ArrayAsVector, Clear),
        CUnknownImpl_VTableEntry(ArrayAsVector, GetMany),
        CUnknownImpl_VTableEntry(ArrayAsVector, ReplaceAll));

    Define_InspectableImpl_VTable(g_VectorViewVtable,
        CUnknownImpl_VTableEntry(ArrayAsVector, GetAt),
        CUnknownImpl_VTableEntry(ArrayAsVector, get_Size),
        CUnknownImpl_VarArgT_VTableEntry(ArrayAsVector, IndexOf),
        CUnknownImpl_VTableEntry(ArrayAsVector, GetMany));

    ArrayAsVector::ArrayAsVector(ProjectionContext *projectionContext, bool fReadOnly)
        : CUnknownImpl(projectionContext, fReadOnly ? g_VectorViewVtable : g_VectorVtable
#if DBG_DUMP
            , fReadOnly ? vectorViewWrapper : vectorWrapper
#endif        
        ), 
        m_fReadOnly(fReadOnly),
        m_pIterable(NULL),
        m_pVectorView(NULL),
        m_iidIterable(GUID_NULL),
        m_pUnderlyingArray(NULL), 
        vector(NULL),
        iterable(NULL)
    {
        indexOfId = projectionContext->indexOfId;
        setAtId = projectionContext->setAtId;
        insertAtId = projectionContext->insertAtId;
        appendId = projectionContext->appendId;
        replaceAllId = projectionContext->replaceAllId;

        // Make sure DLL is not unloaded until this object is destroyed. When the DLL is unloaded
        // we will clean up the ThreadContext which will empty a lot of memory which this object 
        // might try to use causing potential memory corruption or access violations.
        DLLAddRef();
    }

    ArrayAsVector::~ArrayAsVector()
    {
        // If not initialised dont do anything
        if (m_pUnderlyingArray == NULL)
        {
            return;
        }

        if (m_pVectorView)
        {
            m_pVectorView->Release();
        }

        if (m_pIterable)
        {
            delete m_pIterable;
        }
        JS_ETW(EventWriteJSCRIPT_RECYCLER_FREE_WINRT_COLLECTIONS_OBJECT(this));

        DLLRelease();
    }

    HRESULT ArrayAsVector::Initialize(
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR vector)
    {
        Assert(vector == NULL || (!m_fReadOnly && vector->iid->piid == ProjectionModel::IID_IVector1) || (m_fReadOnly && vector->iid->piid == ProjectionModel::IID_IVectorView1));
        HRESULT hr = __super::Initialize(vector->iid->instantiated, StringOfId(projectionContext->GetScriptContext(), vector->typeId));
        IfFailedReturn(hr);

        Js::JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(pUnderlyingArray);

        this->vector = vector;
        this->elementType = ProjectionModel::ConcreteType::From(vector->genericParameters->First());
        this->sizeOnStackOfElement = this->elementType->sizeOnStack;
        this->m_pUnderlyingArray = pUnderlyingArray;
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_COLLECTIONS_OBJECT(this, this->GetFullTypeName(), m_pUnderlyingArray));

        AddRef();

        return hr;
    }

    HRESULT ArrayAsVector::Create(
            __in ProjectionContext *projectionContext, 
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR vector,
            __in bool fReadOnly,
            __out ArrayAsVector **newArrayAsVector)
    {
        Assert(newArrayAsVector != NULL);
        Assert(projectionContext != NULL);
        Assert(pUnderlyingArray != NULL);
        Assert(vector != NULL);

        ArrayAsVector *pVector = new ArrayAsVector(projectionContext, fReadOnly);
        IfNullReturnError(pVector, E_OUTOFMEMORY);

        HRESULT hr = pVector->Initialize(pUnderlyingArray, vector);
        if (FAILED(hr))
        {
            delete pVector;
            return hr;
        }

        *newArrayAsVector = pVector;
        return hr;
    }

    IID ArrayAsVector::GetIterableGuid()
    {
        if (iterable == NULL)
        {
            iterable = ProjectionModel::FindRequiredMatchingInterfaceByPiid(vector, ProjectionModel::IID_IIterable1);
            m_iidIterable = iterable->iid->instantiated;
        }
        return m_iidIterable;
    }

    HRESULT ArrayAsVector::GetIterable(void **ppv)
    {
        if (m_pIterable == NULL)
        {
            if (vector && iterable == NULL)
            {
                iterable = ProjectionModel::FindRequiredMatchingInterfaceByPiid(vector, ProjectionModel::IID_IIterable1);
            }
            HRESULT hr = ArrayAsIterable::Create(projectionContext, m_pUnderlyingArray, iterable, &m_pIterable, m_iid, this);
            IfFailedReturn(hr);
        }
        else
        {
            m_pIterable->AddRef();
        }

        *ppv = m_pIterable->GetUnknown();
        return S_OK;
    }

    CUnknownMethodNoError_Prolog(ArrayAsVector, QueryInterface, REFIID riid, void **ppv)
    {
        HRESULT hr = __super::QueryInterface(riid, ppv);

        if (hr == E_NOINTERFACE && IsEqualGUID(riid, GetIterableGuid()))
        {
            hr = GetIterable(ppv);
        }

        return hr;
    }
    CUnknownMethodNoError_Epilog()

    CUnknownMethodNoError_Prolog(ArrayAsVector, GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        IfNullReturnError(iidCount, E_POINTER);
        *iidCount = 0;
        if (iids != nullptr)
        {
            ZeroMemory(iids, *iidCount * sizeof(*iids));
        }

        return GetTwoIids(GetIterableGuid(), iidCount, iids);
    }
    CUnknownMethodNoError_Epilog()

#pragma warning(push)
#pragma warning(disable:28285)
#pragma warning(disable:28202)
    CUnknownMethodImpl_Prolog(ArrayAsVector, GetAt, (index, returnValue), __in unsigned index, __out_bcount(elementType->storageSize) byte *returnValue)
    {
        IfNullReturnError(returnValue, E_POINTER);

        hr = ArrayAsCollection::GetAt(projectionContext, elementType, m_pUnderlyingArray, index, returnValue);
    }
    CUnknownMethodImpl_Epilog()
#pragma warning(pop)

    CUnknownMethodImpl_Prolog(ArrayAsVector, get_Size, (returnValue), __out unsigned *returnValue)
    {
        IfNullReturnError(returnValue, E_POINTER);

        *returnValue = (unsigned)ArrayAsCollection::GetLength(m_pUnderlyingArray);
        hr = S_OK;
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ArrayAsVector, GetView, (returnValue), __deref_out_opt IUnknown **returnValue)
    {
        IfNullReturnError(returnValue, E_POINTER);
        *returnValue = nullptr;

        hr = SupportsWrite();
        if (SUCCEEDED(hr))
        {
            // Create the Vector View if it doesnt exist
            if (m_pVectorView == NULL)
            {
                RtEXPR expr = nullptr;
                hr = projectionContext->GetExpr(MetadataStringIdNil, IdOfString(projectionContext->GetScriptContext(), _u("Windows.Foundation.Collections.IVectorView`1")), _u("Windows.Foundation.Collections.IVectorView`1"), vector->genericParameters, &expr);

                if (SUCCEEDED(hr))
                {
                    RtRUNTIMEINTERFACECONSTRUCTOR vectorView = ProjectionModel::RuntimeInterfaceConstructor::From(expr);
                    hr = ArrayAsVector::Create(projectionContext, m_pUnderlyingArray, vectorView, true, &m_pVectorView);
                }
            }

            if (SUCCEEDED(hr))
            {
                *returnValue = m_pVectorView->GetUnknown();
                m_pVectorView->AddRef();
            }
        }
    }
    CUnknownMethodImpl_Epilog()


#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'register' modified by inline assembly code
#pragma warning(disable:4189) // local variable is initialized but not referenced
#pragma warning(disable:26000)
#pragma warning(disable:28931)
    // Parameters: __in_opt T value, __out unsigned *index, __out boolean *found
    CUnknownMethodImpl_ArgT_Prolog(ArrayAsVector, IndexOf, 3, (DWORD)(sizeOnStackOfElement + sizeof(uint *) + sizeof (boolean*)), Var varValue;, E_ACCESSDENIED)
    {
        ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
        DefineCallingConventionLocals();

        // GetNextParameterLocation_ is only for ARM: T is never going to out/Array, so using elementType->sizeOnStack is safe.
        GetNextParameterLocation_(elementType->sizeOnStack, CallingConventionHelper::IsFloatingPoint(elementType), 
            CallingConventionHelper::Is64BitAlignRequired(elementType), parameterLocation);
        byte* varValuelocation = GetNextInParameterAddressFromParamType(elementType);
        varValue = marshal.ReadOutType(nullptr, elementType, false, varValuelocation, elementType->sizeOnStack, indexOfId);
        UpdateParameterRead(elementType->sizeOnStack);

        GetNextParameterLocation_(sizeof(int*), false, false, parameterLocation);
        uint  *index = (uint *)GetNextOutParameterAddress(false, false);
        UpdateParameterRead(sizeof(uint *));

        GetNextParameterLocation_(sizeof(int*), false, false, parameterLocation);
        boolean *found = (boolean *)(GetNextOutParameterAddress(false, false));
        UpdateParameterRead(sizeof(boolean *));
        
        if (index != nullptr && found != nullptr)
        {
        hr = ArrayAsCollection::IndexOf(projectionContext, m_pUnderlyingArray, varValue, index, found);
        }
        else
        {
            hr = E_POINTER;
        }
    }
    CUnknownMethodImpl_ArgT_Epilog()

    // Parameters : __in unsigned index, __in_opt T value
    CUnknownMethodImpl_ArgT_Prolog(ArrayAsVector, SetAt, 2, sizeof(uint) + sizeOnStackOfElement, Var varValue;, E_ACCESSDENIED)
    {
        hr = SupportsWrite();
        
        if (SUCCEEDED(hr))
        {
            DefineCallingConventionLocals();
            GetNextParameterLocation_(sizeof(uint*), false, false, parameterLocation);
            uint index = *(uint *)(GetNextInParameterAddress(false, false));
            UpdateParameterRead(sizeof(uint));

            ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
            GetNextParameterLocation_(elementType->sizeOnStack, CallingConventionHelper::IsFloatingPoint(elementType), 
                CallingConventionHelper::Is64BitAlignRequired(elementType), parameterLocation);
            byte* valueLocation = GetNextInParameterAddressFromParamType(elementType);
            varValue = marshal.ReadOutType(nullptr, elementType, false, valueLocation, elementType->sizeOnStack, setAtId);
            UpdateParameterRead(elementType->sizeOnStack); 
        
            // Do the core work 
            hr = ArrayAsCollection::SetAt(projectionContext, m_pUnderlyingArray, index, varValue);
        }
    }
    CUnknownMethodImpl_ArgT_Epilog()

    // Parameters : __in unsigned index, __in_opt T value
    CUnknownMethodImpl_ArgT_Prolog(ArrayAsVector, InsertAt, 2, sizeof(uint) + sizeOnStackOfElement, Var varValue;, E_ACCESSDENIED)
    {
        hr = SupportsWrite();
        
        if (SUCCEEDED(hr))
        {
            DefineCallingConventionLocals();
            GetNextParameterLocation_(sizeof(uint*), false, false, parameterLocation);
            uint index = *(uint *)(GetNextInParameterAddress(false, false));
            UpdateParameterRead(sizeof(uint));

            ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
            GetNextParameterLocation_(elementType->sizeOnStack, CallingConventionHelper::IsFloatingPoint(elementType), 
                CallingConventionHelper::Is64BitAlignRequired(elementType), parameterLocation);
            byte* valueLocation = GetNextInParameterAddressFromParamType(elementType);
            varValue = marshal.ReadOutType(nullptr, elementType, false, valueLocation, elementType->sizeOnStack, insertAtId);
            UpdateParameterRead(elementType->sizeOnStack); 
        
            // Do the core work 
            hr = ArrayAsCollection::InsertAt(projectionContext, m_pUnderlyingArray, index, varValue);
        }
    }
    CUnknownMethodImpl_ArgT_Epilog()
#pragma warning(pop)

    CUnknownMethodImpl_Prolog(ArrayAsVector, RemoveAt, (index), __in unsigned index)
    {
        hr = SupportsWrite();
        
        if (SUCCEEDED(hr))
        {
            hr = ArrayAsCollection::RemoveAt(projectionContext, m_pUnderlyingArray, index);
        }
    }
    CUnknownMethodImpl_Epilog()

#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'register' modified by inline assembly code
#pragma warning(disable:4189) // local variable is initialized but not referenced
#pragma warning(disable:28931)
    // Parameters : __in_opt T value
    CUnknownMethodImpl_ArgT_Prolog(ArrayAsVector, Append, 1, sizeOnStackOfElement, Var varValue;, E_ACCESSDENIED) 
    {
        hr = SupportsWrite();
        
        if (SUCCEEDED(hr))
        {
            DefineCallingConventionLocals();
            ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
            GetNextParameterLocation_(elementType->sizeOnStack, CallingConventionHelper::IsFloatingPoint(elementType), 
                CallingConventionHelper::Is64BitAlignRequired(elementType), parameterLocation);
            byte* valueLocation = GetNextInParameterAddressFromParamType(elementType);
            varValue = marshal.ReadOutType(nullptr, elementType, false, valueLocation, elementType->sizeOnStack, appendId);
            UpdateParameterRead(elementType->sizeOnStack); 
        
            // Do the core work 
            hr = ArrayAsCollection::Append(projectionContext, m_pUnderlyingArray, varValue);
        }
    }
    CUnknownMethodImpl_ArgT_Epilog()
#pragma warning(pop)

    CUnknownMethodImpl_NoArgs_Prolog(ArrayAsVector, RemoveAtEnd)
    {
        hr = SupportsWrite();

        if (SUCCEEDED(hr))
        {
            hr = ArrayAsCollection::RemoveAtEnd(m_pUnderlyingArray);
        }
    }
    CUnknownMethodImpl_NoArgs_Epilog()
    
    CUnknownMethodImpl_NoArgs_Prolog(ArrayAsVector, Clear)
    {
        hr = SupportsWrite();
        if (SUCCEEDED(hr))
        {
            hr = ArrayAsCollection::Clear(m_pUnderlyingArray);
        }
    }
    CUnknownMethodImpl_NoArgs_Epilog()

#pragma warning(push)
#pragma warning(disable:28202)
#pragma warning(disable:28285)
    CUnknownMethodImpl_Prolog(ArrayAsVector, GetMany, (startIndex, capacity, items, actual), __in unsigned int startIndex, __in unsigned int capacity, __out_bcount(elementType->storageSize * capacity) byte *items, __RPC__out unsigned int *actual)
#pragma warning(pop)
    {
        IfNullReturnError(items, E_POINTER);
        IfNullReturnError(actual, E_POINTER);
        *actual = 0;

        hr = ArrayAsCollection::GetMany(projectionContext, elementType, m_pUnderlyingArray, startIndex, capacity, items, actual);
    }
    CUnknownMethodImpl_Epilog()

#pragma warning(push)
#pragma warning(disable:28202)
#pragma warning(disable:28285)
    CUnknownMethodImpl_Prolog(ArrayAsVector, ReplaceAll, (count, value), __in unsigned int count, __out_bcount(elementType->storageSize * count) byte *value)
#pragma warning(pop)
    {
        hr = SupportsWrite();
        if (SUCCEEDED(hr))
        {
            hr = ArrayAsCollection::ReplaceAll(projectionContext, elementType, m_pUnderlyingArray, count, value, replaceAllId);
        }
    }
    CUnknownMethodImpl_Epilog()

    void ArrayAsVector::MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect)
    {
        Assert(m_pUnderlyingArray != NULL);

        recycler->TryMarkNonInterior(m_pUnderlyingArray);
    }

    USHORT ArrayAsVector::GetWinrtTypeFlags()
    {
        return PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE;
    }

    UINT ArrayAsVector::GetHeapObjectRelationshipInfoSize()
    {
        return ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize();
    }

    void ArrayAsVector::FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo)
    {
        this->GetHeapEnum()->FillHeapObjectInternalUnnamedJSVarProperty(optionalInfo, m_pUnderlyingArray);
    }
}
