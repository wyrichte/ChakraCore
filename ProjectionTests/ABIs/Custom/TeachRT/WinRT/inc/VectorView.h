//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.
#include <wrl\implements.h>
#include "AsyncServers.h"
#include <WindowsStringp.h>
#pragma once

namespace REX {
namespace Reference {
namespace Async {

class CAsyncVectorView : 
    public Microsoft::WRL::RuntimeClass<
        Windows::Foundation::Collections::IVectorView< HSTRING >,
        Windows::Foundation::Collections::IIterable< HSTRING > >
{
public:
    CAsyncVectorView(PCWSTR *strings, int length) :
        _ppszStrings(strings),
        _length(length)
    {
    };
    virtual ~CAsyncVectorView(){};
    
    // IVectorView::GetAt
    STDMETHODIMP GetAt(__in unsigned int index, __deref_out HSTRING*)
    {
        return E_NOTIMPL;
    }
    
    // IVectorView::IndexOf   
    STDMETHODIMP IndexOf(__in HSTRING value, __deref_out unsigned int* index, __deref_out boolean *returnValue)
    {
        return E_NOTIMPL;
    }
    
    // IVectorView_impl::get_Size
    STDMETHODIMP get_Size(unsigned int* size)
    {
        *size = _length;
        return S_OK;
    }

    // IIterable::First
    STDMETHODIMP First(Windows::Foundation::Collections::IIterator<HSTRING> **returnValue)
    {
        Microsoft::WRL::ComPtr<Windows::Foundation::Collections::IIterator<HSTRING>> pIterator = 
            Microsoft::WRL::Make<CAsyncVectorIterator>(this);

        if(pIterator == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        pIterator.CopyTo(returnValue);
        return S_OK;
    }

    class CAsyncVectorIterator : 
        public Microsoft::WRL::RuntimeClass< 
            Windows::Foundation::Collections::IIterator< HSTRING > >
    {
    public:
        CAsyncVectorIterator(CAsyncVectorView* outer) :
            _pOuter(outer),
            _index(0)
        {
        }
        virtual ~CAsyncVectorIterator(){};
        
        // IIterator::Current
        STDMETHODIMP get_Current(__deref_out HSTRING *returnValue)
        {
            Windows::Internal::String string;
            string.Initialize(_pOuter->_ppszStrings[_index]);
            string.Detach(returnValue);
            return S_OK;
        }
        
        // IIterator::HasCurrent
        STDMETHODIMP get_HasCurrent(__deref_out boolean *returnValue)
        {
            *returnValue = (_index < _pOuter->_length);
            return S_OK;
        }
        
        // IIterator::MoveNext
        STDMETHODIMP MoveNext(__deref_out boolean *returnValue)
        {
            _index++;
            *returnValue = (_index < _pOuter->_length);
            return S_OK;
        }
        
    private:
        CAsyncVectorView* _pOuter;
        int               _index;
    };
    
private:
    PCWSTR *_ppszStrings;
    int     _length;
};

}
}
}
