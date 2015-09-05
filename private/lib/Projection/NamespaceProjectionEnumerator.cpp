//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corportion.  ll rights reserved.
//----------------------------------------------------------------------------

#include "ProjectionPch.h"

namespace Projection
{
    NamespaceProjectionEnumerator::NamespaceProjectionEnumerator(NamespaceProjection *pNamespaceProjection, Var instance, IVarEnumerator *pDefaultPropertyEnumerator)
        : refCount(0), firstMove(true), m_pNamespaceProjection(pNamespaceProjection, pNamespaceProjection->GetProjectionContext()->GetScriptContext()->GetRecycler()), 
        m_pDefaultPropertyEnumerator(pDefaultPropertyEnumerator)
    {        
        this->m_namespaceChildren = m_pNamespaceProjection->GetDirectChildren();        
        m_instance.Root(instance, m_pNamespaceProjection.GetRecycler());
        
        m_pDefaultPropertyEnumerator->AddRef();
    }

    NamespaceProjectionEnumerator::~NamespaceProjectionEnumerator()
    {
        m_pDefaultPropertyEnumerator->Release();
        m_instance.Unroot(m_pNamespaceProjection.GetRecycler());        
    }

    STDMETHODIMP NamespaceProjectionEnumerator::QueryInterface(REFIID riid, void **ppv)
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

    STDMETHODIMP_(ULONG) NamespaceProjectionEnumerator::AddRef(void) 
    { 
        return InterlockedIncrement(&refCount); 
    }

    STDMETHODIMP_(ULONG) NamespaceProjectionEnumerator::Release(void)
    {
        LONG res = InterlockedDecrement(&refCount);

        if (res == 0)
        {
            HeapDelete(this);
        }

        return res;
    }

    STDMETHODIMP NamespaceProjectionEnumerator::MoveNext(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes)
    {
        IfNullReturnError(itemsAvailable, E_POINTER);
        *itemsAvailable = FALSE;

        if (attributes != nullptr)
        {
            *(Js::PropertyAttributes*)attributes = PropertyEnumerable;
        }

        if (m_namespaceChildren)
        {
            if (!firstMove)
            {
                m_namespaceChildren = m_namespaceChildren->GetTail();
            }
            else
            {
                firstMove = FALSE;
            }

            *itemsAvailable = (m_namespaceChildren != nullptr);
        }
        
        if (!m_namespaceChildren && m_pNamespaceProjection->GetIsExtensible())
        {
            HRESULT hr;
            Var currentName;
            LPCWSTR szName;
            // Skip any properties that are in the list of namespace children. They have already been enumerated.
            do {
                hr = m_pDefaultPropertyEnumerator->MoveNext(itemsAvailable, attributes);
                IfFailedReturn(hr);
                if (*itemsAvailable)
                {
                    hr = m_pDefaultPropertyEnumerator->GetCurrentName(&currentName);
                    IfFailedReturn(hr);
                    szName = Js::JavascriptString::FromVar(currentName)->GetSz();
                }
            } while (*itemsAvailable && (m_pNamespaceProjection->GetDirectChildren()->ContainsWhere([&](LPCWSTR name) {
                return (wcscmp(name, szName) == 0);
            })));

            return hr;
        }
        
        return S_OK;
    }

    STDMETHODIMP NamespaceProjectionEnumerator::GetCurrentName(__out Var* item) 
    {
        IfNullReturnError(item, E_POINTER);
        *item = nullptr;

        if (m_namespaceChildren)
        {
            // return the first child
            Js::ScriptContext *pScriptContext = m_pNamespaceProjection->GetProjectionContext()->GetScriptContext();
            LPCWSTR child = m_namespaceChildren->First();
            HRESULT hr = S_OK;
            BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(pScriptContext)
            {
                *item = Js::JavascriptString::NewCopySz(child, pScriptContext);
            }
            END_JS_RUNTIME_CALL(pScriptContext)
            return hr;
        }
        else if (m_pNamespaceProjection->GetIsExtensible())
        {
            return m_pDefaultPropertyEnumerator->GetCurrentName(item);
        }

        return S_OK;
    }

};
