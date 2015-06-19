//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corportion.  ll rights reserved.
//----------------------------------------------------------------------------

#include "Stdafx.h"

namespace Projection
{
    VectorArrayEnumerator::VectorArrayEnumerator(SpecialProjection * specialization, Var instance, IVarEnumerator *pDefaultPropertyEnumerator)
        : refCount(0), m_iIndex(-1), specialization(specialization), m_pDefaultPropertyEnumerator(pDefaultPropertyEnumerator),
            m_instance(instance, specialization->projectionContext->GetScriptContext()->GetRecycler())
    {        
        m_pDefaultPropertyEnumerator->AddRef();
    }

    VectorArrayEnumerator::~VectorArrayEnumerator()
    {     
        m_pDefaultPropertyEnumerator->Release();
    }

    STDMETHODIMP VectorArrayEnumerator::QueryInterface(REFIID riid, void **ppv)
    {
        if (ppv == NULL)
        {
            return E_POINTER;
        }
        if (IsEqualGUID(riid, __uuidof(IVarEnumerator))) 
        {
            *ppv = static_cast<IVarEnumerator*>(this);
        }
        else if (IsEqualGUID(riid, IID_IUnknown)) 
        {
            *ppv = static_cast<IVarEnumerator*>(this);
        }
        else 
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        
        return S_OK;
    }

    STDMETHODIMP_(ULONG) VectorArrayEnumerator::AddRef(void) 
    { 
        return InterlockedIncrement(&refCount); 
    }

    STDMETHODIMP_(ULONG) VectorArrayEnumerator::Release(void)
    {
        LONG res = InterlockedDecrement(&refCount);

        if (res == 0)
        {
            HeapDelete(this);
        }

        return res;
    }

    STDMETHODIMP VectorArrayEnumerator::MoveNext(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes)
    {
        IfNullReturnError(itemsAvailable, E_POINTER);
        *itemsAvailable = FALSE;

        int length = -1;
        
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext) 
        {
            length = static_cast<int>(VectorArray::GetLength(specialization, m_instance, specialization->projectionContext->GetScriptContext()));
        }
        END_JS_RUNTIME_CALL(scriptContext);

        Assert(length != -1);
        if (m_iIndex < length - 1)
        {
            m_iIndex++;
            *itemsAvailable = TRUE;

            if (attributes != nullptr)
            {
                *(Js::PropertyAttributes*)attributes = PropertyEnumerable;
            }

            return S_OK;
        }
        else
        {
            m_iIndex = length;
            return m_pDefaultPropertyEnumerator->MoveNext(itemsAvailable, attributes);
        }
    }

    STDMETHODIMP VectorArrayEnumerator::GetCurrentName(__out Var* item) 
    {
        IfNullReturnError(item, E_POINTER);
        *item = nullptr;

        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext) 
        {
            int length = static_cast<int>(VectorArray::GetLength(specialization, m_instance, scriptContext));

            if (m_iIndex < length)
            {
                // return the index as a string
                Var aIndex = Js::JavascriptNumber::ToVar(m_iIndex, scriptContext);
                *item = Js::JavascriptConversion::ToString(aIndex, scriptContext);
            }
        }
        END_JS_RUNTIME_CALL(scriptContext);

        if (*item == nullptr)
        {
            return m_pDefaultPropertyEnumerator->GetCurrentName(item);
        }

        return hr;
    }
};
