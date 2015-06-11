//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    struct RemoteStackFrameEnumerator;

    //
    // Abstracts the layer of external API needed for our diagonstics.
    //
    class DiagProvider
    {
    public:
        virtual ~DiagProvider() {}

        virtual RemoteStackFrameEnumerator* CreateStackFrameEnumerator(DWORD threadId, void* advanceToAddr) = 0;
        virtual void GetThreadContext(DWORD threadId, CONTEXT* context) = 0;
        virtual bool TryGetTargetModuleBaseAddr(PCWSTR simpleModuleName, ULONG64* baseAddr) = 0;
        virtual HRESULT GetTlsValue(_In_ DWORD threadId, _In_ UINT32 tlsIndex, _Out_ UINT64 *pValue) = 0;
        virtual IVirtualReader* GetReader() = 0;

    protected:
        RemoteStackFrameEnumerator* CreateFrameChainBasedEnumerator(DWORD threadId, void* advanceToAddr, IVirtualReader* reader);
        static void GetFrameAndInstructionOffset(CONTEXT* context, void** pFrameOffset, void** stackOffset, void** pInstructionOffset);
    };

    //
    // dbgeng-like Diagnostics provider
    //
    class DbgEngDiagProvider : public DiagProvider
    {
    private:
        CComPtr<IStackProviderDataTarget> m_dataTarget;   // We don't own the data target.
        AutoPtr<VirtualReader> m_reader;        // We own the reader.

    public:
        DbgEngDiagProvider(IStackProviderDataTarget* dataTarget);

        virtual RemoteStackFrameEnumerator* CreateStackFrameEnumerator(DWORD threadId, void* advanceToAddr) override;
        virtual void GetThreadContext(DWORD threadId, CONTEXT* context) override;
        virtual bool TryGetTargetModuleBaseAddr(PCWSTR simpleModuleName, ULONG64* baseAddr) override;
        virtual HRESULT GetTlsValue(_In_ DWORD threadId, _In_ UINT32 tlsIndex, _Out_ UINT64 *pValue) override;
        virtual IVirtualReader* GetReader() override;
    };

    //
    // Diagnostics provider based on services provided by VS.
    // Can be used inside VS only.
    //
    class VSDiagProvider : public DiagProvider
    {
        CComPtr<IJsDebugDataTarget> m_dataTarget;   // We don't own the data target.
        AutoPtr<JsDebugVirtualReader> m_reader;     // We own the reader.
        UINT64 m_js9BaseAddr;

    public:
        VSDiagProvider(IJsDebugDataTarget* dataTarget, UINT64 baseAddress);

        virtual RemoteStackFrameEnumerator* CreateStackFrameEnumerator(DWORD threadId, void* advanceToAddr) override;
        virtual void GetThreadContext(DWORD threadId, CONTEXT* context) override;
        virtual bool TryGetTargetModuleBaseAddr(PCWSTR simpleModuleName, ULONG64* baseAddr) override;
        virtual HRESULT GetTlsValue(_In_ DWORD threadId, _In_ UINT32 tlsIndex, _Out_ UINT64 *pValue) override;
        virtual IVirtualReader* GetReader() override;
    };
}
