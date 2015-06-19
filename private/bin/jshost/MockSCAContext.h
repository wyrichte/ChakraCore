/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

// Mock SCAContext
class MockSCAContext:
    public ComObjectRoot,
    public ISCAContext
{
private:
    SCAContextType m_context;
    CComPtr<IUnknown> m_target;
    CComPtr<IUnknown> m_dependentObject;

public:
    HRESULT Init(SCAContextType context, IUnknown* target);

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

    STDMETHODIMP GetContext(SCAContextType* pContext);
    STDMETHODIMP GetTarget(IUnknown** ppTarget);
    STDMETHODIMP SetDependentObject(__in IUnknown *obj);
    STDMETHODIMP GetDependentObject(__out IUnknown **obj);
};
