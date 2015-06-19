/*******************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

//#include <cor.h>
//#include "DelayLoadLibrary.h"
//#include "JsHostScriptSite.h"
//#include "Wscript.h"
//#include "docobj.h"

class DelegateWrapper : public IDelegateWrapper
{
public:
    DelegateWrapper():refcount(1) {}
    ~DelegateWrapper() {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void) {ULONG count = InterlockedIncrement(&refcount); return count;}
    ULONG STDMETHODCALLTYPE Release(void) { 
        ULONG count = InterlockedDecrement(&refcount); 
        if (count == 0) 
            delete this;
        return count; }
    HRESULT STDMETHODCALLTYPE RegisterPriorityItem( 
        IUnknown *originalDelegate,
        REFIID originalDelegateInterfaceID,
        IUnknown **priorityDelegate);
private:
    UINT refcount;

};


class PriorityDelegate : public IUnknown
{
public:
    PriorityDelegate(IUnknown* originalDelegate, IUnknown* ftmProxy);
    ~PriorityDelegate();
    // HACKHACK: I'm creating my own vtbl here; 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef(void) override {ULONG count = InterlockedIncrement(&refcount); return count;}
    ULONG STDMETHODCALLTYPE Release(void)  override { 
        ULONG count = InterlockedDecrement(&refcount); 
        if (count == 0) 
            delete this;
        return count; }
    virtual HRESULT STDMETHODCALLTYPE Invoke();

private:
    void ReplaceVtable();
    // The order here is important as it's assumed in assembly code thunk. Please add new fields between ftmProxy & vtbl.
    IUnknown* forwardingDelegate;
    UINT refcount;
    // I'm not actually using ftmProxy, and depend on the current test infrastructure of CoMarshalInterfaceInStream. 
    // leave it here for now.
    IUnknown* ftmProxy;
    void* vtbl[4];
};
