//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corportion.  ll rights reserved.
//----------------------------------------------------------------------------

#include "Stdafx.h"

namespace Projection
{
    MapWithStringKeyEnumerator::MapWithStringKeyEnumerator(SpecialProjection * specialization, Var instance)
        : 
        refCount(0), 
        specialization(specialization), 
        keys(&HeapAllocator::Instance),
        m_instance(instance, specialization->projectionContext->GetScriptContext()->GetRecycler()),
        winrtStringLib(specialization->projectionContext->GetThreadContext()->GetWinRTStringLibrary())
    { }

    MapWithStringKeyEnumerator::~MapWithStringKeyEnumerator()
    {
        // Delete any keys remaining if at all
        while(!keys.Empty())
        {
            HSTRING currentKey = keys.Pop();
            winrtStringLib->WindowsDeleteString(currentKey);
        }
    }

    // Gets the list of keys currently in map. Its a no-throw function to make sure we can get the HRESULT and cleanup memory in case of failures
    HRESULT MapWithStringKeyEnumerator::InitializeKeys(Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, IInspectable *> *> *iterator)
    {
        boolean hasCurrent;
        HRESULT hr = iterator->get_HasCurrent(&hasCurrent);
        IfFailedReturn(hr);

        while (hasCurrent)
        {
            CComPtr<Windows::Foundation::Collections::IKeyValuePair<HSTRING, IInspectable *>> keyValuePair;
            hr = iterator->get_Current(&keyValuePair);
            IfFailedReturn(hr);

            HSTRING hString;
            hr = keyValuePair->get_Key(&hString);
            IfFailedReturn(hr);

            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                keys.Prepend(hString);
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr)
            IfFailedReturn(hr);

            hr = iterator->MoveNext(&hasCurrent);
            IfFailedReturn(hr);
        }

        keys.Reverse();

        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            keys.Prepend(nullptr);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr)
        
        return hr;
    }

    STDMETHODIMP MapWithStringKeyEnumerator::QueryInterface(REFIID riid, void **ppv)
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

    STDMETHODIMP_(ULONG) MapWithStringKeyEnumerator::AddRef(void) 
    { 
        return InterlockedIncrement(&refCount); 
    }

    STDMETHODIMP_(ULONG) MapWithStringKeyEnumerator::Release(void)
    {
        LONG res = InterlockedDecrement(&refCount);

        if (res == 0)
        {
            HeapDelete(this);
        }

        return res;
    }

    STDMETHODIMP MapWithStringKeyEnumerator::MoveNext(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes)
    {
        IfNullReturnError(itemsAvailable, E_POINTER);
        *itemsAvailable = FALSE;

        if (attributes != nullptr)
        {
            *(Js::PropertyAttributes*)attributes = PropertyEnumerable;
        }

        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));

        BOOL isPresent;
        do
        {
            // Delete the existing key before moving to next
            HSTRING currentKey = keys.Pop();
            winrtStringLib->WindowsDeleteString(currentKey);

            if (keys.Empty())
            {
                return S_OK;
            }

            // Check if the current key is still available in the instance
            HRESULT hr = MapWithStringKey::HasOwnKey(specialization, m_instance, keys.Head(), &isPresent); 
            IfFailedReturn(hr);

        } while(!isPresent);

        Assert(!keys.Empty());
        *itemsAvailable = TRUE;
        return S_OK;
    }

    STDMETHODIMP MapWithStringKeyEnumerator::GetCurrentName(__out Var* item) 
    {
        IfNullReturnError(item, E_POINTER);
        *item = nullptr;

        Assert(MapWithStringKey::IsMapOrMapViewWithStringKey(specialization));
        Js::ScriptContext *scriptContext = specialization->projectionContext->GetScriptContext();
        HRESULT hr = S_OK;
        BEGIN_JS_RUNTIME_CALL_EX_PROJECTION_TYPE_OP(scriptContext)
        {
            *item = MapWithStringKey::GetVarFromHSTRING(keys.Head(), specialization);
        }
        END_JS_RUNTIME_CALL(scriptContext);
        return hr;
    }
};
