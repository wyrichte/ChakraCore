//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

// global single instance of the dump reader
static CComPtr<JsDiag::ScriptDumpReader> g_scriptDumpReader;

//
// JS WER dump stream consumer implementation
//

//
// If JavaScriptDataStream dump stream is present, dbgeng will pass the stream to initialize
// this stack provider prior to reconstruct stack for each thread.
//
HRESULT WINAPI BeginThreadStackReconstruction(
    _In_ ULONG streamType,
    _In_ PVOID miniDumpStreamBuffer,
    _In_ ULONG bufferSize)
{
    if (streamType != MINIDUMP_STREAM_TYPE::JavaScriptDataStream
        || miniDumpStreamBuffer == nullptr
        || bufferSize == 0)
    {
        return E_INVALIDARG;
    }

    if (g_scriptDumpReader)
    {
        return E_UNEXPECTED;
    }

    return JsDiag::JsDebugApiWrapper([=]
    {
        JsDiag::CreateComObject(miniDumpStreamBuffer, bufferSize, &g_scriptDumpReader);
        return S_OK;
    });
}

//
// Queries dump stream provider per-thread. Stack frames and symbolic data are returned.
//
HRESULT WINAPI ReconstructStack(
    _In_ ULONG systemThreadId,
    _In_ PDEBUG_STACK_FRAME_EX /* pNativeFrames */,
    _In_ ULONG /* cNativeFrames */,
    _Out_ PSTACK_SYM_FRAME_INFO *ppStackSymFrames,
    _Out_ ULONG *pStackSymFramesFilled)
{
    if (g_scriptDumpReader == nullptr)
    {
        return E_UNEXPECTED;
    }

    if (ppStackSymFrames == nullptr || pStackSymFramesFilled == nullptr)
    {
        return E_POINTER;
    }

    return JsDiag::JsDebugApiWrapper([=]
    {
        g_scriptDumpReader->GetStack(systemThreadId, ppStackSymFrames, pStackSymFramesFilled);
        return S_OK;
    });
}

//
// After ReconstructStack is called and used dbgeng will call the stack provider to free memory
//
HRESULT WINAPI FreeStackSymFrames(
    _In_ PSTACK_SYM_FRAME_INFO pStackSymFrames)
{
    if (pStackSymFrames != nullptr)
    {
        delete[] pStackSymFrames;
    }
    return S_OK;
}

//
// Dbgeng is done with thread stack reconstruction.  Dump stack provider may clean up state.
//
HRESULT WINAPI EndThreadStackReconstruction()
{
    g_scriptDumpReader.Release();
    return S_OK;
}

//
// Private test api. Retrieve an existing ThreadId from JS dump stream in order to call ReconstructStack.
//
HRESULT WINAPI PrivateGetStackThreadId(
    _In_ ULONG index,
    _Out_ ULONG *pSystemThreadId)
{
    if (g_scriptDumpReader == nullptr)
    {
        return E_UNEXPECTED;
    }

    if (pSystemThreadId == nullptr)
    {
        return E_POINTER;
    }
    *pSystemThreadId = 0;

    return JsDiag::JsDebugApiWrapper([=]
    {
        bool exists = g_scriptDumpReader->GetStackThreadId(index, pSystemThreadId);
        return exists ? S_OK : S_FALSE;
    });
}

namespace JsDiag
{
    void ScriptDumpReader::Init(_In_ PVOID miniDumpStreamBuffer, _In_ ULONG bufferSize)
    {
        MemoryReadStream stream(miniDumpStreamBuffer, bufferSize);

        AutoPtr<WerMessage> message = new(oomthrow) WerMessage();
        Serializer::Deserialize(&stream, nullptr, message);

        // Verify magic cookie
        message->ValidateMagicCookie();

        // Build a map for fast lookup.
        for (ULONG i = 0; i < message->StackCount; i++)
        {
            const WerStack* werStack = &message->Stacks[i];

#ifdef DBG
            const WerStack* unused;
            Assert(!m_stacks.Lookup(werStack->ThreadId, unused));
#endif
            m_stacks[werStack->ThreadId] = werStack;
        }

        // Everything done, transfer ownership
        m_message = message.Detach();
    }

    void ScriptDumpReader::GetStack(_In_ ULONG systemThreadId, _Out_ PSTACK_SYM_FRAME_INFO *ppStackSymFrames, _Out_ ULONG *pStackSymFramesFilled) const
    {
        const WerStack* werStack;
        if (m_stacks.Lookup(systemThreadId, werStack))
        {
            ToStackInfo(werStack, ppStackSymFrames, pStackSymFramesFilled);
        }
        else
        {
            *ppStackSymFrames = nullptr;
            *pStackSymFramesFilled = 0;
        }
    }

    _Success_(return) bool ScriptDumpReader::GetStackThreadId(_In_ ULONG index, _Out_ ULONG *pSystemThreadId) const
    {
        if (index < m_message->StackCount)
        {
            const WerStack* werStack = &m_message->Stacks[index];
            *pSystemThreadId = werStack->ThreadId;
            return true;
        }

        return false;
    }

    //
    // Convert and return JS stack frame info, to be used by debugger for stack stitching.
    // 
    // General JS frames include FrameOffset, ReturnOffset, InstructionOffset, function name, uri, row/col.
    // Inlined JS frames have 0x82xx in InlineFrameContext, where xx is inline frame index starting from 0.
    // Full details: http://windowsblue/docs/home/Windows%20Blue%20Feature%20Docs/Engineering%20Systems%20and%20Compatibility%20(ESC)/Engineer%20Desktop%20(END)/DAT/JavaScript%20Stack%20Debugger%20Consumption.docx
    //
    // Sample JS stack info:
    //  Thread: 0x3774
    //  ChildEBP RetAddr  Inst
    //  0097e930 100e1eae 021801f2 --foo (d:\fbl_ie\script_dev\inetcore\jscript\tmp.js:7,9)
    //  0097e930 100e1eae 021801f2 --bar (d:\fbl_ie\script_dev\inetcore\jscript\tmp.js:12,5)
    //  0097e930 100e1eae 021801f2 f (d:\fbl_ie\script_dev\inetcore\jscript\tmp.js:16,5)
    //  0097eb88 100e1eae 00b00fe9 Global code (d:\fbl_ie\script_dev\inetcore\jscript\tmp.js:21,5)
    //
    // Native frames:
    //  ChildEBP RetAddr
    //  0097e8d8 021801f2 KERNELBASE!DebugBreak+0x2
    //  0097e930 100e1eae 0x21801f2
    //  0097e970 0ff91e89 jscript9test!Js::JavascriptFunction::CallFunction<1>+0xbe
    //  ...
    //  0097ea30 0ff9150e jscript9test!Js::InterpreterStackFrame::Process+0xe8d
    //  0097eb7c 00b00fe9 jscript9test!Js::InterpreterStackFrame::InterpreterThunk<1>+0x46e
    //  0097eb88 100e1eae 0xb00fe9
    //  0097ebc0 100e28ec jscript9test!Js::JavascriptFunction::CallFunction<1>+0xbe
    //
    void ScriptDumpReader::ToStackInfo(const WerStack* werStack, _Out_ PSTACK_SYM_FRAME_INFO *ppStackSymFrames, _Out_ ULONG *pStackSymFramesFilled)
    {
        ULONG frameCount = werStack->FrameCount;

        AutoArrayPtr<STACK_SYM_FRAME_INFO> info = new(oomthrow) STACK_SYM_FRAME_INFO[frameCount];
        SecureZeroMemory(info, sizeof(STACK_SYM_FRAME_INFO) * frameCount);

        BYTE inlineIndex = 0;
        for (ULONG i = 0; i < frameCount; i++)
        {
            const WerFrame& frame = werStack->Frames[i];

            info[i].StackFrameEx.InstructionOffset = frame.InstructionPointer;
            info[i].StackFrameEx.ReturnOffset = frame.ReturnAddress;
            info[i].StackFrameEx.FrameOffset = frame.FrameBase;
            info[i].StackFrameEx.StackOffset = frame.StackPointer;

            if (frame.IsInlineFrame)
            {
                if (inlineIndex >= 0xFF)
                {
                    Assert(false);
                    DiagException::Throw(E_FAIL, DiagErrorCode::TOO_MANY_INLINE_FRAMES); // We don't expect these many inline frames. We can't encode the index.
                }

                info[i].StackFrameEx.InlineFrameContext = MakeInlineFrameContext(inlineIndex++);
            }
            else
            {
                if (inlineIndex != 0)
                {
                    inlineIndex = 0; // End inline frames
                }

                info[i].StackFrameEx.InlineFrameContext = MakePhysicalFrameContext();
            }

            info[i].SrcInfo.ImagePath = frame.Uri;
            info[i].SrcInfo.Function = frame.FunctionName;
            info[i].SrcInfo.Row = frame.Row;
            info[i].SrcInfo.Column = frame.Col;
        }

        // Done!
        *ppStackSymFrames = info.Detach();
        *pStackSymFramesFilled = frameCount;
    }
}
