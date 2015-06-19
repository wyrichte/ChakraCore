//----------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once 
#include "mshtml.h"

//
// JavascriptPixelArrayDispatch implements a few simple COM APIs to represent a CanvasPixelArray
// for native code callers, in addition to the inherited IDispatch.  
//

class JavascriptPixelArrayDispatch sealed : 
    public JavascriptDispatch, 
    public ICanvasPixelArray,
    public ICanvasPixelArrayData
{
public:
    static JavascriptPixelArrayDispatch * Create(Js::DynamicObject* pgcScriptObject);

    /****************************************
    IUnknown methods
    ****************************************/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj) override;
    ULONG STDMETHODCALLTYPE AddRef(void) override 
        { return JavascriptDispatch::AddRef(); }
    ULONG STDMETHODCALLTYPE Release(void) override 
        { return JavascriptDispatch::Release(); }
    
    /****************************************
    ICanvasPixelArray methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE get_length(__out ULONG* plLength);

    /****************************************
    ICanvasPixelArrayData methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE GetBufferPointer(
        __deref_out_bcount(*pBufferLength) BYTE **ppBuffer, 
        __out ULONG *pBufferLength
        );

    /****************************************
    IDispatch methods
    ****************************************/
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) override
        { return JavascriptDispatch::GetTypeInfoCount(pctinfo); }
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) override
        { return JavascriptDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, __in_ecount(cpsz) LPOLESTR *prgpsz, UINT cpsz, LCID lcid, __in_ecount(cpsz) DISPID *prgid) override
        { return JavascriptDispatch::GetIDsOfNames(riid, prgpsz, cpsz, lcid, prgid); }
    HRESULT STDMETHODCALLTYPE Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, 
        VARIANT *pvarRes, EXCEPINFO *pexcepinfo, UINT *puArgErr) override
        { return JavascriptDispatch::Invoke(id, riid, lcid, wFlags, pdispparams, pvarRes, pexcepinfo, puArgErr); }
        
public:
    JavascriptPixelArrayDispatch(
        __in Js::DynamicObject* pgcScriptObject, 
        __in ScriptSite *scriptSite) : 
        JavascriptDispatch(pgcScriptObject, scriptSite)
    {
    }
};

