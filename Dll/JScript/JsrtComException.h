//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class JsrtComException : public Js::ExceptionBase
{
private:    
    JsrtComException(HRESULT hr) : hr(hr) { }

public:     
    void static ThrowIfFailed(HRESULT hr) 
    {
        if (FAILED(hr))
        {
            throw JsrtComException(hr);
        }
    }

    static JsErrorCode JsErrorFromHResult(HRESULT hr) 
    {
        switch (hr)
        {
        case S_OK:
            return JsNoError;
        case CTL_E_OUTOFSTACKSPACE:
        case CTL_E_OUTOFMEMORY:
        case E_OUTOFMEMORY:        
            return JsErrorOutOfMemory;
        case E_POINTER:
            return JsErrorNullArgument;
        case E_INVALIDARG:
            return JsErrorInvalidArgument;
        case E_ABORT:
            return JsErrorScriptTerminated;  
        case E_NOTIMPL:
            return JsErrorNotImplemented;
        case E_FAIL:
            return JsErrorFatal;
        default:
            Assert(FALSE);
            return JsErrorFatal;
        }
    }


    JsErrorCode GetJsErrorFromHResult()
    {
        return JsErrorFromHResult(hr);
    }

private:
    HRESULT hr;
};