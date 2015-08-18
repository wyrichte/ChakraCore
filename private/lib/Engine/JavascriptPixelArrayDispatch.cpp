/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "StdAfx.h"
#include "JavascriptPixelArrayDispatch.h"

HRESULT JavascriptPixelArrayDispatch::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = JavascriptDispatch::QueryInterface(riid, ppvObj);

    if (FAILED(hr))
    {
        if (IID_ICanvasPixelArray == riid)
        {
            *ppvObj = (ICanvasPixelArray *)this;
            AddRef();
            hr = S_OK;
        }
        else if (IID_ICanvasPixelArrayData == riid)
        {
            *ppvObj = (ICanvasPixelArrayData *)this;
            AddRef();
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT JavascriptPixelArrayDispatch::get_length(__out ULONG* plLength)
{
    IfNullReturnError(plLength, E_INVALIDARG);
    *plLength = 0;

    Assert(scriptObject->GetTypeId() == Js::TypeIds_Uint8ClampedArray);

    // GetByteLength returns the array length in bytes (elements * element_size)
    *plLength = Js::Uint8ClampedArray::FromVar(scriptObject)->GetByteLength();

    return S_OK;
}

/*virtual*/
HRESULT JavascriptPixelArrayDispatch::GetBufferPointer(
    __deref_out_bcount(*pBufferLength) BYTE **ppBuffer, 
    __out ULONG *pBufferLength
    )
{
    UINT iBufferLength;
    HRESULT hr = S_OK;

    IfNullReturnError(ppBuffer, E_INVALIDARG);
    *ppBuffer = nullptr;
    IfNullReturnError(pBufferLength, E_INVALIDARG);
    *pBufferLength = 0;

    if (!VerifyCallingThread())
    {
        hr = E_UNEXPECTED;
        goto Error;
    } 

    if (scriptObject == NULL) 
    {
        AssertMsg(scriptSite->IsClosed(), "ScriptContext not closed but script object is NULL");
        hr = E_ACCESSDENIED;
        goto Error;
    }
    
    ScriptEngine* engine = scriptSite->GetScriptEngine();
    if (engine == NULL)
    {
        hr = E_ACCESSDENIED;
        goto Error;
    }

    hr = engine->GetPixelArrayBuffer(scriptObject, ppBuffer, &iBufferLength);
    if (FAILED(hr))
    {
        *pBufferLength = 0;
        goto Error;
    }

    *pBufferLength = iBufferLength;

Error:
    return hr;
}
