//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once

//TODO remove this redundant header (mshtml.h has this interface)
//Razzle builds are picking the right mshtml.h but VS picks up the wrong headers and dependency.
MIDL_INTERFACE("3051049b-98b5-11cf-bb82-00aa00bdce0b")
IHTMLDOMConstructor : public IDispatch
{
public:
    virtual /* [hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get_constructor( 
        /* [out][retval] */ __RPC__deref_out_opt IDispatch **p) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE LookupGetter( 
        /* [in] */ __RPC__in BSTR propname,
        /* [out][retval] */ __RPC__out VARIANT *ppDispHandler) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE LookupSetter( 
        /* [in] */ __RPC__in BSTR propname,
        /* [out][retval] */ __RPC__out VARIANT *ppDispHandler) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE DefineGetter( 
        /* [in] */ __RPC__in BSTR propname,
        /* [in] */ __RPC__in VARIANT *pdispHandler) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE DefineSetter( 
        /* [in] */ __RPC__in BSTR propname,
        /* [in] */ __RPC__in VARIANT *pdispHandler) = 0;
    
};