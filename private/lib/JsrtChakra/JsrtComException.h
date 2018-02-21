//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include "JsrtExceptionBase.h"

class JsrtComException : public JsrtExceptionBase
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
        case 0x800A001C: // CTL_E_OUTOFSTACKSPACE
        case 0x800A0007: // CTL_E_OUTOFMEMORY
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


    JsErrorCode GetJsErrorCode()
    {
        return JsErrorFromHResult(hr);
    }

private:
    HRESULT hr;
};