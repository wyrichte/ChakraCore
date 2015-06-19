//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    class ATL_NO_VTABLE JsDebugProcess :
        public CComObjectRoot,
        public IJsDebugProcessPrivate
    {
    private:
        AutoPtr<DiagProvider> m_diagProvider;
        AutoPtr<DebugClient> m_debugClient;
        DWORD m_processId;
        CComPtr<IJsDebugDataTarget> m_debugDataTarget;
        AutoPtr<RemoteAllocator> m_remoteAllocator;
        bool m_validateDebugMode;   // signifies if the API is expected to assert that the runtime is in debug mode
    public:
        void Init(UINT64 baseAddress, IJsDebugDataTarget* debugDataTarget, IStackProviderDataTarget* testDataTarget, DWORD processId, bool validateDebugMode);

        DECLARE_NOT_AGGREGATABLE(JsDebugProcess)  
        BEGIN_COM_MAP(JsDebugProcess)
            COM_INTERFACE_ENTRY(IJsDebugProcess)
            COM_INTERFACE_ENTRY(IJsDebugProcessPrivate)
        END_COM_MAP()

        DebugClient* GetDebugClient() const { return m_debugClient; }
        DWORD GetProcessId() { return m_processId; }
        IVirtualReader* GetReader() const;
        IJsDebugDataTarget* GetDebugDataTarget() const { return m_debugDataTarget; }
        RemoteAllocator* GetRemoteAllocator() const { return m_remoteAllocator; }
        bool DoDebugModeValidation() const { return m_validateDebugMode; }

        // ** IJsDebugProcess ** /
        virtual STDMETHODIMP CreateStackWalker( 
            /* [in] */ DWORD ThreadId,
            /* [out] */ __RPC__deref_out_opt IJsDebugStackWalker **ppStackWalker);

        virtual STDMETHODIMP CreateBreakPoint( 
            /* [in] */ UINT64 documentId,
            /* [in] */ DWORD characterOffset,
            /* [in] */ DWORD characterCount,
            /* [in] */ BOOL isEnabled,
            /* [out] */ _Out_ IJsDebugBreakPoint **ppDebugbreakPoint);

        virtual STDMETHODIMP PerformAsyncBreak(
            /*in*/ DWORD threadId);

        virtual STDMETHODIMP GetExternalStepAddress(
            /* [out] */ __RPC__out UINT64 * pCodeAddress);

        /* IJsDebugProcessPrivate */
        virtual STDMETHODIMP InspectVar(
            _In_ VOID* var,
            _Out_ IJsDebugProperty** ppDebugProperty);
    };
}