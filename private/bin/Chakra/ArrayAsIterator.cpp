//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterator in JavaScript
// *******************************************************


#include "stdafx.h"

namespace Projection
{
    Define_InspectableImpl_VTable(g_IteratorVtable,
        CUnknownImpl_VTableEntry(ArrayAsIterator, get_Current),
        CUnknownImpl_VTableEntry(ArrayAsIterator, get_HasCurrent),
        CUnknownImpl_VTableEntry(ArrayAsIterator, MoveNext),
        CUnknownImpl_VTableEntry(ArrayAsIterator, GetMany));

    ArrayAsIterator::ArrayAsIterator(ProjectionContext *projectionContext)
        : CUnknownImpl(projectionContext, g_IteratorVtable
#if DBG_DUMP
            , iteratorWrapper
#endif
        ), 
        m_uCurrentIndex(0),
        m_pUnderlyingArray(NULL), 
        iterator(NULL)
    {
    }

    // Info:        Destruct
    ArrayAsIterator::~ArrayAsIterator()
    {
        JSETW(EventWriteJSCRIPT_RECYCLER_FREE_WINRT_COLLECTIONS_OBJECT(this));
    }

    HRESULT ArrayAsIterator::Initialize(
        __in Js::JavascriptArray *pUnderlyingArray, 
        __in RtRUNTIMEINTERFACECONSTRUCTOR iterator)
    {
        Assert(iterator == NULL || iterator->iid->piid == ProjectionModel::IID_IIterator1);

        HRESULT hr = __super::Initialize(iterator->iid->instantiated, StringOfId(projectionContext->GetScriptContext(), iterator->typeId));
        IfFailedReturn(hr);

        this->iterator = iterator;
        this->elementType = ProjectionModel::ConcreteType::From(iterator->genericParameters->First());
        this->m_pUnderlyingArray = pUnderlyingArray;
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_COLLECTIONS_OBJECT(this, this->GetFullTypeName(), m_pUnderlyingArray));

        AddRef();

        return hr;
    }

    HRESULT ArrayAsIterator::Create(
            __in ProjectionContext *projectionContext, 
            __in Js::JavascriptArray *pUnderlyingArray,
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterator,
            __out ArrayAsIterator **newArrayAsIterator)
    {
        Assert(projectionContext != NULL);
        Assert(pUnderlyingArray != NULL);
        Assert(iterator != NULL);

        IfNullReturnError(newArrayAsIterator, E_POINTER);
        *newArrayAsIterator = nullptr;

        ArrayAsIterator *pIterator = new ArrayAsIterator(projectionContext);
        IfNullReturnError(pIterator, E_OUTOFMEMORY);

        HRESULT hr = pIterator->Initialize(pUnderlyingArray, iterator);
        if (FAILED(hr))
        {
            delete pIterator;
            return hr;
        }

        *newArrayAsIterator = pIterator;
        return hr;
    }

#pragma warning(push)
#pragma warning(disable:28285)
#pragma warning(disable:28202)
    CUnknownMethodImpl_Prolog(ArrayAsIterator, get_Current, (current), __out_bcount(elementType->storageSize) byte * current)
#pragma warning(pop)
    {
        IfNullReturnError(current, E_POINTER);

        hr = ArrayAsCollection::GetAt(projectionContext, elementType, m_pUnderlyingArray, m_uCurrentIndex, current);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ArrayAsIterator, get_HasCurrent, (hasCurrent), __RPC__out boolean *hasCurrent)
    {
        IfNullReturnError(hasCurrent, E_POINTER);

        uint length = ArrayAsCollection::GetLength(m_pUnderlyingArray);
        *hasCurrent = (m_uCurrentIndex < length);
        hr = S_OK;
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ArrayAsIterator, MoveNext, (hasCurrent), __RPC__out boolean *hasCurrent)
    {
        IfNullReturnError(hasCurrent, E_POINTER);
        *hasCurrent = FALSE;

        uint length = ArrayAsCollection::GetLength(m_pUnderlyingArray);
        if (m_uCurrentIndex < length)
        {
            m_uCurrentIndex++;
            *hasCurrent = (m_uCurrentIndex < length);
            hr = S_OK;
        }
        else
        {
            hr = E_BOUNDS;
        }
    }
    CUnknownMethodImpl_Epilog()

#pragma warning(push)
#pragma warning(disable:28202)
#pragma warning(disable:28285)
    CUnknownMethodImpl_Prolog(ArrayAsIterator, GetMany, (capacity, items, actual), __in unsigned int capacity, __out_bcount(elementType->storageSize * capacity) byte *items, __RPC__out unsigned int *actual)
#pragma warning(pop)
    {
        IfNullReturnError(items, E_POINTER);
        *items = 0;
        IfNullReturnError(actual, E_POINTER);

        uint length = ArrayAsCollection::GetLength(m_pUnderlyingArray);
        if (m_uCurrentIndex < length)
        {
            hr = ArrayAsCollection::GetMany(projectionContext, elementType, m_pUnderlyingArray, m_uCurrentIndex, capacity, items, actual);
            if (SUCCEEDED(hr))
            {
                m_uCurrentIndex = m_uCurrentIndex + *actual;
            }
        }
        else
        {
            *actual = 0;
            hr = E_BOUNDS;
        }
    }
    CUnknownMethodImpl_Epilog()

    void ArrayAsIterator::MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect)
    {
        Assert(m_pUnderlyingArray != NULL);

        recycler->TryMarkNonInterior(m_pUnderlyingArray);
    }

    USHORT ArrayAsIterator::GetWinrtTypeFlags()
    {
        return PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE;
    }

    UINT ArrayAsIterator::GetHeapObjectRelationshipInfoSize()
    {
        return ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize();
    }

    void ArrayAsIterator::FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo)
    {
        this->GetHeapEnum()->ActiveScriptProfilerHeapEnum::FillHeapObjectInternalUnnamedJSVarProperty(optionalInfo, m_pUnderlyingArray);
    }
}
