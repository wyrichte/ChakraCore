//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Used by jscript9diag.dll to hook into jscript9.dll.
    //
    class DiagHook : public Js::ComObjectBase<IDiagHook, DiagHook>
    {
        friend class Js::ComObjectBase<IDiagHook, DiagHook>;
    private:
        DiagHook();

    public:
        // *** IDiagHook ***
        virtual STDMETHODIMP GetGlobals(
            /* [size_is][out] */ __RPC__out_ecount_full(globalsCount) void **globals,
            /* [in] */ ULONG globalsCount);
        virtual STDMETHODIMP GetVTables(
            /* [size_is][out] */ __RPC__out_ecount_full(bufferSize) void **vtables,
            /* [in] */ ULONG bufferSize);
        virtual STDMETHODIMP GetErrorString( 
            /* [in] */ HRESULT errorCode,
            /* [out] */ __RPC__deref_out_opt BSTR *bsResource);
    };
}
