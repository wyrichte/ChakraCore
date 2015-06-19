#include "stdafx.h"
#include "combaseapi.h"

HRESULT STDMETHODCALLTYPE DelegateWrapper::RegisterPriorityItem( 
        IUnknown *originalDelegate,
        REFIID originalDelegateInterfaceID,
        IUnknown **priorityDelegate)
{
    HRESULT hr;
    IUnknown* ftmProxy = nullptr;
    *priorityDelegate = nullptr;
    hr = CoCreateFreeThreadedMarshaler(originalDelegate, &ftmProxy);
    if (SUCCEEDED(hr))
    {
        *priorityDelegate = new PriorityDelegate(originalDelegate, ftmProxy);
        printf("new priority delegate");
    }
    return hr;
};

HRESULT   STDMETHODCALLTYPE DelegateWrapper::QueryInterface(REFIID riid, void ** ppvObject)
{
    if (riid == IID_IUnknown )        
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    if (riid == __uuidof(IDelegateWrapper))
    {
        *ppvObject = static_cast<IDelegateWrapper*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

PriorityDelegate::PriorityDelegate(IUnknown* originalDelegate, IUnknown* inFtmProxy) :
    refcount(1), ftmProxy(inFtmProxy)
{
    this->forwardingDelegate = originalDelegate;
    this->forwardingDelegate->AddRef();
    ReplaceVtable();
}

// major hack. Don't do this in real code. 
void PriorityDelegate::ReplaceVtable()
{
    vtbl[0] = (*(void***)this)[0];
    vtbl[1] = (*(void***)this)[1];
    vtbl[2] = (*(void***)this)[2];
    vtbl[3] = (*(void***)this)[3];
    *((void***)this) = vtbl;
}

// fake a call, set the "this" pointer to real delegate, and jump to 
// delegate's invoke;return will be taken care of by the delegate as 
// it knowns the number of arguments to pop.
#ifdef _M_IX86
HRESULT __declspec(naked)   PriorityDelegate::Invoke()
{
    static const offset = offsetof(PriorityDelegate, forwardingDelegate);
//    void* vtblMethod = (*(void***)this->forwardingDelegate)[3];
    _asm
    {
        mov eax, DWORD PTR [esp + 4] // this pointer
        mov eax, [eax + 4]           // forwardDelegate
        mov edx, DWORD_PTR [eax]     // vtbl of forwardDelegate
        mov edx, [edx + 3 * 4]          // save the jmp invoke addr
        mov [esp + 4], eax
        jmp edx
    }
}
#else
HRESULT PriorityDelegate::Invoke()
{
    Assert(FALSE);
    return E_NOTIMPL;
}
#endif

PriorityDelegate::~PriorityDelegate() 
{
    this->forwardingDelegate->Release();
    this->ftmProxy->Release();
}

HRESULT STDMETHODCALLTYPE PriorityDelegate::QueryInterface(REFIID riid, void ** ppvObject)
{
    if (riid == IID_IUnknown )
    {
        *ppvObject = (IUnknown*)(this);
        AddRef();
        return S_OK;
    }
    return forwardingDelegate->QueryInterface(riid, ppvObject);
}

