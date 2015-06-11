//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#include "stackprovdatatarget.h"

//
//TEMP: Delete when available from debugger sdk. Put in namespace to avoid build conflict with debugger sdk.
//
namespace JsDiag
{
    enum _JsDumpFlagValues
    {
        JS_PHYSICAL_STACK_FRAME_TYPE    = STACK_FRAME_TYPE_STACK | STACK_FRAME_TYPE_RA,
        JS_INLINE_STACK_FRAME_TYPE      = STACK_FRAME_TYPE_INLINE | STACK_FRAME_TYPE_RA,
    };

    // Make a physical JS stack frame context for InlineFrameContext
    inline DWORD MakePhysicalFrameContext()
    {
        INLINE_FRAME_CONTEXT FrameContext = {0};
        FrameContext.FrameType = (BYTE)JS_PHYSICAL_STACK_FRAME_TYPE;
        return FrameContext.ContextValue;
    }

    // Make an inline JS stack frame context for InlineFrameContext. InlineFrameId begins from 0.
    inline DWORD MakeInlineFrameContext(BYTE InlineFrameId)
    {
        INLINE_FRAME_CONTEXT FrameContext = {0};
        FrameContext.FrameType = (BYTE)JS_INLINE_STACK_FRAME_TYPE;
        FrameContext.FrameId = InlineFrameId;
        return FrameContext.ContextValue;
    }

    inline _Success_(return) bool TryGetInlineFrameId(DWORD InlineFrameContext, _Out_ BYTE* pFrameId)
    {
        INLINE_FRAME_CONTEXT FrameContext;
        FrameContext.ContextValue = InlineFrameContext;

        if (FrameContext.FrameType == JS_INLINE_STACK_FRAME_TYPE)
        {
            *pFrameId = FrameContext.FrameId;
            return true;
        }
        
        // We only create 2 types of FrameType
        Assert(FrameContext.FrameType == JS_PHYSICAL_STACK_FRAME_TYPE);
        Assert(FrameContext.FrameId == 0);
        return false;
    }

    // Private test api. Retrieve an existing ThreadId from JS dump stream in order to call ReconstructStack.
    typedef HRESULT WINAPI PrivateGetStackThreadIdFunc(
        _In_ ULONG index,
        _Out_ ULONG *pSystemThreadId);
};

//
// Private unit test interface that provides extra memory access support.
//
#undef  INTERFACE
#define INTERFACE IJsDiagTestDataTarget
DECLARE_INTERFACE_IID_(IJsDiagTestDataTarget, IStackProviderDataTarget, "45AF1BBF-5D3C-4FDC-AAF4-5D4633DC441E")
{
    STDMETHOD(AllocateVirtual)(THIS_
        _In_ ULONG64 address,
        _In_ SIZE_T size,
        _In_ DWORD allocationType,
        _In_ DWORD pageProtection,
        _Out_ ULONG64* pAllocatedAddress
        ) PURE;

    STDMETHOD(FreeVirtual)(THIS_
        _In_ ULONG64 address,
        _In_ SIZE_T size,
        _In_ DWORD freeType
        ) PURE;

    STDMETHOD(WriteVirtual)(THIS_
        _In_ ULONG64 address,
        _In_reads_bytes_(bufferSize) PVOID buffer,
        _In_ ULONG bufferSize,
        _Out_opt_ PULONG bytesWritten
        ) PURE;
};
