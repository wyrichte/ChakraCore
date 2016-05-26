//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "winternl.h"

namespace JsDiag
{
///////////////////////////////////////////////////////////////////////////////////////////////////
// DiagProvider.
    RemoteStackFrameEnumerator* DiagProvider::CreateFrameChainBasedEnumerator(DWORD threadId, void* advanceToFrameAddr, IVirtualReader* reader)
    {
        void *frameAddr, *stackAddr, *ip;
        if (advanceToFrameAddr == nullptr)
        {
            // Get info about current frame so that we have EBP to start from and don't miss top frame in case it's the JS code.
            CONTEXT context;
            this->GetThreadContext(threadId, &context);
            this->GetFrameAndInstructionOffset(&context, &frameAddr, &stackAddr, &ip);
        }
        else
        {
            // Initialize the enumerator as started from specified frame.
            frameAddr = advanceToFrameAddr;
            stackAddr = nullptr;   // stackAddr is only used for 1st jitted frame, and when we skip frames 1st frame would be something in jscript9/non-jit frame.
            ip = nullptr;
        }

        return new(oomthrow) FrameChainBasedStackFrameEnumerator(frameAddr, stackAddr, ip, reader);
    }

    // Get offsets from platform-specific CONTEXT.
    // This is reliable only when we are inside script, which means that top-most frame belongs to JS.
    void DiagProvider::GetFrameAndInstructionOffset(CONTEXT* context, void** pFrameOffset, void** stackOffset, void** pInstructionOffset)
    {
        Assert(context);
#if defined(_M_IX86)
        *pFrameOffset = (void*)context->Ebp;
        *stackOffset = (void*)context->Esp;
        *pInstructionOffset = (void*)context->Eip;
#elif defined(_M_X64)
        AssertMsg(FALSE, "This is not supposed to be used on AMD64 (EBP frames are not applicable).");
        *pFrameOffset = (void*)context->Rbp;
        *stackOffset = (void*)context->Rsp;
        *pInstructionOffset = (void*)context->Rip;
#elif defined(_M_ARM)
        *pFrameOffset = (void*)context->R11;
        *stackOffset = (void*)context->Sp;
        *pInstructionOffset = (void*)context->Pc;
#endif
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
// DbgEngDiagProvider.
    DbgEngDiagProvider::DbgEngDiagProvider(IStackProviderDataTarget* dataTarget) : m_dataTarget(dataTarget)
    {
        m_reader = new(oomthrow) VirtualReader(dataTarget);
    }

    RemoteStackFrameEnumerator* DbgEngDiagProvider::CreateStackFrameEnumerator(DWORD threadId, void* advanceToAddr)
    {
#ifdef _M_X64
        ulong cacheSize = 16;
#if defined(DBG) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
        char16 envBuf[16];
        DWORD storedCharCount = GetEnvironmentVariableW(_u("FrameCacheSize"), envBuf, sizeof(envBuf) / sizeof(char16));
        if (storedCharCount > 0)
        {
            cacheSize = _wtoi(envBuf);
        }
#endif
        RemoteStackFrameEnumerator* enu = new(oomthrow) DbgEngStackFrameEnumerator(m_dataTarget, cacheSize, threadId);
        if (advanceToAddr)
        {
            enu->AdvanceToFrame(advanceToAddr);
        }
        return enu;
#else !_M_X64

        return this->CreateFrameChainBasedEnumerator(threadId, advanceToAddr, m_reader);

#endif _M_X64
    }

    void DbgEngDiagProvider::GetThreadContext(DWORD threadId, CONTEXT* context)
    {
        SecureZeroMemory(context, sizeof(CONTEXT));
        HRESULT hr = m_dataTarget->GetThreadContext(threadId, CONTEXT_ALL, sizeof(CONTEXT), (PBYTE)context);
        CheckHR(hr, DiagErrorCode::DIAGPROVIDER_GETTHREADCONTEXT);
    }

    //
    // Obtains base addr of the specified module *** in the target process ***.
    //
    bool DbgEngDiagProvider::TryGetTargetModuleBaseAddr(PCWSTR simpleModuleName, ULONG64* baseAddr)
    {
        HRESULT hr = m_dataTarget->GetModuleByModuleName(simpleModuleName, baseAddr);
        return SUCCEEDED(hr);
    }

    HRESULT DbgEngDiagProvider::GetTlsValue(_In_ DWORD threadId, _In_ UINT32 tlsIndex, _Out_ UINT64 *pValue)
    {
        HRESULT hr = S_OK;

        ULONG64 tebAddr;
        {
            ULONG tebSize;
            hr = m_dataTarget->GetThreadTeb(threadId, &tebAddr, &tebSize);
            CheckHR(hr, DiagErrorCode::DIAGPROVIDER_GETTHREADTEB);
        }

        size_t tlsSlotsOffset;  // Offset off start TEB of the tlsSlots array we need to use.
        if (tlsIndex < 64)
        {
            tlsSlotsOffset = offsetof(_TEB, TlsSlots);
        }
        else
        {
            tlsSlotsOffset = offsetof(_TEB, TlsExpansionSlots);
            tlsIndex -= 64;
        }

        PVOID value;
        ULONG bytesRead;
        hr = m_dataTarget->ReadVirtual(tebAddr + tlsSlotsOffset + tlsIndex * sizeof(PVOID), (PBYTE)&value, sizeof(value), &bytesRead);
        CheckHR(hr, DiagErrorCode::DIAGPROVIDER_READ_VIRTUAL);
        if (bytesRead == sizeof(value))
        {
            *pValue = (UINT64)value;
        }
        else
        {
            hr = E_FAIL;
        }

        return hr;
    }

    IVirtualReader* DbgEngDiagProvider::GetReader()
    {
        return m_reader;
    }
}
