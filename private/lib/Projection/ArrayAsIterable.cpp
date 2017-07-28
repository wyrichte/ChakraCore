//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterable in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    Define_InspectableImpl_VTable(g_IterableVtable,
        CUnknownImpl_VTableEntry(ArrayAsIterable,First));

    ArrayAsIterable::ArrayAsIterable(ProjectionContext *projectionContext)
        : CUnknownImpl(projectionContext, g_IterableVtable
#if DBG_DUMP
            , iterableWrapper
#endif
        ), 
        m_pUnderlyingArray(NULL), 
        m_iidVectorOrView(GUID_NULL),
        m_pVectorOrView(NULL),
        iterable(NULL),
        iterator(nullptr)
    {
        // Make sure DLL is not unloaded until this object is destroyed. When the DLL is unloaded
        // we will clean up the ThreadContext which will empty a lot of memory which this object 
        // might try to use causing potential memory corruption or access violations.
        DLLAddRef();
    }

    // Info:        Destruct
    ArrayAsIterable::~ArrayAsIterable()
    {
        JS_ETW(EventWriteJSCRIPT_RECYCLER_FREE_WINRT_COLLECTIONS_OBJECT(this));

        DLLRelease();
    }

    HRESULT ArrayAsIterable::Initialize(
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterable,
            __in_opt IID iidVectorOrView, 
            __in_opt ArrayAsVector *pVectorOrView)
    {
        Assert(iterable == NULL || iterable->iid->piid == ProjectionModel::IID_IIterable1);

        HRESULT hr = __super::Initialize(iterable->iid->instantiated, (pVectorOrView != nullptr) ? pVectorOrView->GetFullTypeName() : StringOfId(projectionContext->GetScriptContext(), iterable->typeId), pVectorOrView == NULL);
        IfFailedReturn(hr);

        Js::JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(pUnderlyingArray);

        this->m_pUnderlyingArray = pUnderlyingArray;
        this->m_pVectorOrView = pVectorOrView;
        this->m_iidVectorOrView = iidVectorOrView;
        this->iterable = iterable;
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_COLLECTIONS_OBJECT(this, this->GetFullTypeName(), m_pUnderlyingArray));

        AddRef();
        return hr;
    }

    HRESULT ArrayAsIterable::Create(
            __in ProjectionContext *projectionContext, 
            __in Js::JavascriptArray *pUnderlyingArray, 
            __in RtRUNTIMEINTERFACECONSTRUCTOR iterable,
            __out ArrayAsIterable **newArrayAsIterable,
            __in_opt IID iidVectorOrView, 
            __in_opt ArrayAsVector *pVectorOrView)
    {
        Assert(newArrayAsIterable != NULL);
        Assert(projectionContext != NULL);
        Assert(pUnderlyingArray != NULL);
        Assert(iterable != NULL);

        ArrayAsIterable *pIterable = new ArrayAsIterable(projectionContext);
        IfNullReturnError(pIterable, E_OUTOFMEMORY);

        HRESULT hr = pIterable->Initialize(pUnderlyingArray, iterable, iidVectorOrView, pVectorOrView);
        if (FAILED(hr))
        {
            delete pIterable;
            return hr;
        }

        *newArrayAsIterable = pIterable;
        return hr;
    }

    CUnknownMethodNoError_Prolog(ArrayAsIterable, QueryInterface, REFIID riid, void **ppv)
    {
        HRESULT hr = E_NOINTERFACE;

        // If we are vector view we support our own iid and rest of them go to Vector/View
        if (IsVectorOrView())
        {
            hr = OwnQueryInterface(riid, ppv);
            if (hr == E_NOINTERFACE)
            {
                hr = m_pVectorOrView->QueryInterface(riid, ppv);
            }
        }
        else
        {
            hr = __super::QueryInterface(riid, ppv);
        }

        return hr;
    }
    CUnknownMethodNoError_Epilog()

    CUnknownMethod_ULONGReturn_Prolog(ArrayAsIterable, AddRef)
    { 
        if (IsVectorOrView())
        {
            uRetVal = m_pVectorOrView->AddRef(); 
        }
        else
        {
            uRetVal = __super::AddRef();
        }
    }
    CUnknownMethod_ULONGReturn_Epilog()

    CUnknownMethod_ULONGReturn_Prolog(ArrayAsIterable, Release)
    {
        if (IsVectorOrView())
        {
            uRetVal = m_pVectorOrView->Release();
        }
        else
        {
            uRetVal = __super::Release();
        }
    }
    CUnknownMethod_ULONGReturn_Epilog()
        
    CUnknownMethodNoError_Prolog(ArrayAsIterable, GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        CHECK_POINTER(iidCount);
        *iidCount = 0;
        if (iids != nullptr)
        {
            *iids = nullptr;
        }

        if (IsVectorOrView())
        {
            return GetTwoIids(m_iidVectorOrView, iidCount, iids);
        }

        return __super::GetIids(iidCount, iids);
    }
    CUnknownMethodNoError_Epilog()

    CUnknownMethodImpl_Prolog(ArrayAsIterable, First, (first), __out IUnknown **first)
    {
        CHECK_POINTER(first);
        *first = nullptr;

        if (iterable != NULL && iterator == nullptr)
        {
            RtEXPR expr = nullptr;
            hr = projectionContext->GetExpr(MetadataStringIdNil, IdOfString(projectionContext->GetScriptContext(), _u("Windows.Foundation.Collections.IIterator`1")), _u("Windows.Foundation.Collections.IIterator`1"), iterable->genericParameters, &expr);

            if (SUCCEEDED(hr))
            {
                iterator = ProjectionModel::RuntimeInterfaceConstructor::From(expr);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Create the iterator
            ArrayAsIterator *pIIterator = NULL;
            hr = ArrayAsIterator::Create(projectionContext, m_pUnderlyingArray, iterator, &pIIterator);

            if (SUCCEEDED(hr))
            {
                // Place the IIterator ptr into the target buffer
                *first = pIIterator->GetUnknown();
            }
        }
    }
    CUnknownMethodImpl_Epilog()

    void ArrayAsIterable::MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect)
    {
        Assert(m_pUnderlyingArray != NULL);
        Assert (!IsVectorOrView());

        recycler->TryMarkNonInterior(m_pUnderlyingArray);
    }

    USHORT ArrayAsIterable::GetWinrtTypeFlags()
    {
        return PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE;
    }

    UINT ArrayAsIterable::GetHeapObjectRelationshipInfoSize()
    {
        Assert (!IsVectorOrView());
        return ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize();
    }

    void ArrayAsIterable::FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo)
    {
        Assert (!IsVectorOrView());
        this->GetHeapEnum()->FillHeapObjectInternalUnnamedJSVarProperty(optionalInfo, m_pUnderlyingArray);
    }
}
