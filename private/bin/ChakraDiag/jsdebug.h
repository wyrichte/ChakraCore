//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace JsDiag
{
    class ATL_NO_VTABLE JsDebug :
        public CComObjectRoot,
        public CComCoClass<JsDebug, &CLSID_ChakraDiag>,
        public IJsDebug2
    {
    public:
        // Provide registration for JsDebug
        DECLARE_REGISTRY_RESOURCE(IDR_JSDEBUG)

        DECLARE_NOT_AGGREGATABLE(JsDebug)
        BEGIN_COM_MAP(JsDebug)
            COM_INTERFACE_ENTRY(IJsDebug)
            COM_INTERFACE_ENTRY(IJsDebug2)
        END_COM_MAP()

    public:
        // *** IJsDebug ***
        virtual STDMETHODIMP OpenVirtualProcess(
            /* [in] */ DWORD processId,
            /* [in] */ UINT64 RuntimeJsBaseAddress,
            /* [in] */ __RPC__in_opt IJsDebugDataTarget *pDataTarget,
            /* [out] */ __RPC__deref_out_opt IJsDebugProcess **ppProcess);

        // *** IJsDebug2 ***
        virtual STDMETHODIMP OpenVirtualProcess(
            /* [in] */ __RPC__in_opt IUnknown *pTestDataTarget,
            /* [in] */ __RPC__in_opt bool debugMode,
            /* [in] */ DWORD processId,
            /* [in] */ UINT64 runtimeJsBaseAddress,
            /* [in] */ __RPC__in_opt IJsDebugDataTarget *pDataTarget,
            /* [out] */ __RPC__deref_out_opt IJsDebugProcess **ppProcess);
    };

    // Ensure JsDebug is hooked up with ATL's implementation of GetClassObject
    OBJECT_ENTRY_AUTO(CLSID_ChakraDiag, JsDebug);
}
