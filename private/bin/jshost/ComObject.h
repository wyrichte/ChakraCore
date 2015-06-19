/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

//
// Helper class for implementing simple COM object.
//
template<class T>
class ComObject: public T
{
private:
    ULONG m_refCount;

    ComObject()
        : m_refCount(1)
    {
    }

    ~ComObject()
    {
    }

public:
    STDMETHODIMP_(ULONG) AddRef()
    {
        Assert(m_refCount > 0);
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        Assert(m_refCount > 0);
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0)
        {
            FinalDestruct();
            delete this;
        }
        return ref;
    }

    static HRESULT CreateInstance(T** ppInstance)
    {
        ComObject* pInstance = new ComObject();
        IfNullReturnError(pInstance, E_OUTOFMEMORY);
        *ppInstance = pInstance;
        return S_OK;
    }
};

//
// Simple COM object root with an empty FinalDestruct.
//
class ComObjectRoot
{
public:
    HRESULT FinalDestruct()
    {
        return S_OK;
    }
};
