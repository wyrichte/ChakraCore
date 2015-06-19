//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corportion.  ll rights reserved.
//----------------------------------------------------------------------------

#include "Stdafx.h"

namespace Projection
{
    ArrayProjectionEnumerator::ArrayProjectionEnumerator(ArrayProjection *pArrayProjection, Var instance, IVarEnumerator *pDefaultPropertyEnumerator)
        : refCount(0), m_iIndex(-1), m_pArrayProjection(pArrayProjection, pArrayProjection->GetScriptContext()->GetRecycler()),
        m_pDefaultPropertyEnumerator(pDefaultPropertyEnumerator)
    {
        m_instance.Root(instance, m_pArrayProjection.GetRecycler());        
        m_pDefaultPropertyEnumerator->AddRef();
    }

    ArrayProjectionEnumerator::~ArrayProjectionEnumerator()
    {
        m_pDefaultPropertyEnumerator->Release();
        m_instance.Unroot(m_pArrayProjection.GetRecycler());            
    }

    STDMETHODIMP ArrayProjectionEnumerator::QueryInterface(REFIID riid, void **ppv)
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

    STDMETHODIMP_(ULONG) ArrayProjectionEnumerator::AddRef(void) 
    { 
        return InterlockedIncrement(&refCount); 
    }

    STDMETHODIMP_(ULONG) ArrayProjectionEnumerator::Release(void)
    {
        LONG res = InterlockedDecrement(&refCount);

        if (res == 0)
        {
            HeapDelete(this);
        }

        return res;
    }

    STDMETHODIMP ArrayProjectionEnumerator::MoveNext(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes)
    {
        IfNullReturnError(itemsAvailable, E_POINTER);
        *itemsAvailable = FALSE;

        int length = static_cast<int>(m_pArrayProjection->GetLength(m_instance));

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

    STDMETHODIMP ArrayProjectionEnumerator::GetCurrentName(__out Var* item) 
    {
        IfNullReturnError(item, E_POINTER);
        *item = nullptr;

        int length = static_cast<int>(m_pArrayProjection->GetLength(m_instance));

        if (m_iIndex < length)
        {
            // return the index as a string
            Js::ScriptContext *pScriptContext = m_pArrayProjection->GetScriptContext();
            HRESULT hr = S_OK;
            BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(pScriptContext)
            {
                Var aIndex = Js::JavascriptNumber::ToVar(m_iIndex, pScriptContext);
                *item = Js::JavascriptConversion::ToString(aIndex, pScriptContext);
            }
            END_JS_RUNTIME_CALL(pScriptContext);
            return hr;
        }
        else
        {
            return m_pDefaultPropertyEnumerator->GetCurrentName(item);
        }
    }
};
