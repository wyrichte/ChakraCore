//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace JsDiag
{
    RemoteInlineFrameWalker::RemoteInlineFrameWalker(
        IVirtualReader* reader, int frameCount, __in_ecount(frameCount) InlinedFrameLayout** frames, Js::JavascriptFunction* parent) : 
        m_reader(reader), m_currentIndex(-1), m_frameCount(frameCount), m_frames(frames)
    {
    }

    //static 
    RemoteInlineFrameWalker* RemoteInlineFrameWalker::FromPhysicalFrame(
        IVirtualReader* reader, InternalStackFrame* physicalFrame, Js::ScriptFunction *parent)
    {
        Assert(physicalFrame);
        Assert(parent);

        RemoteScriptFunction function(reader, parent);
        FunctionBody* functionBodyAddr = function.GetFunction();
        Assert(functionBodyAddr);
        RemoteFunctionBody functionBody(reader, functionBodyAddr);

        FunctionEntryPointInfo* entryPointInfoAddr = functionBody.GetEntryPointFromNativeAddress(
            (DWORD_PTR)physicalFrame->InstructionPointer);
        AssertMsg(entryPointInfoAddr != NULL, "Inlined frame should resolve to the right parent address");

        if (function.GetFrameHeight(entryPointInfoAddr))   // When we inline, frameHeight != 0.
        {
            RemoteFunctionEntryPointInfo entryPointInfo(reader, entryPointInfoAddr);
            void* entryAddr = entryPointInfo->nativeAddress;

            InlinedFrameLayout* lastFrame = RemoteInlinedFrameLayout::FromPhysicalFrame(reader, physicalFrame, entryAddr, parent, entryPointInfoAddr);
            if (lastFrame)
            {
                int32 frameCount = 0;
                InlinedFrameLayout* frameIterator = lastFrame;
                while (frameIterator)
                {
                    frameCount++;
                    frameIterator = RemoteInlinedFrameLayout(reader, frameIterator).Next();
                }

                InlinedFrameLayout** frames = new(oomthrow) InlinedFrameLayout*[frameCount];
                frameIterator = lastFrame;
                for (int i = frameCount - 1; i >= 0; i--)
                {
                    Assert(frameIterator);
                    frames[i] = frameIterator;
                    frameIterator = RemoteInlinedFrameLayout(reader, frameIterator).Next();
                }

                return new(oomthrow) RemoteInlineFrameWalker(reader, frameCount, frames, parent);
            }
        }

        return NULL;
    }

    bool RemoteInlineFrameWalker::Next()
    {
        m_currentIndex++;
        return this->GetCurrentFrame() != NULL;
    }

    void** RemoteInlineFrameWalker::GetCurrentArgvAddress()
    {
        InlinedFrameLayout* currentFrame = this->GetCurrentFrame();
        Assert(currentFrame);
        void* argv = 
            reinterpret_cast<BYTE*>(currentFrame) + // start
            sizeof(InlinedFrameLayout);             // now we point to the start of argv, i.e. "this" arg.
        return reinterpret_cast<void**>(argv);
    }

    Js::JavascriptFunction* RemoteInlineFrameWalker::GetCurrentFunction()
    {
        InlinedFrameLayout* currentFrame = this->GetCurrentFrame();
        Assert(currentFrame);
        return RemoteInlinedFrameLayout(m_reader, currentFrame)->function;
    }

    Js::CallInfo RemoteInlineFrameWalker::GetCurrentCallInfo()
    {
        RemoteInlinedFrameLayout inlinedFrameLayout(m_reader, this->GetCurrentFrame());
        Js::CallInfo callInfo(inlinedFrameLayout->callInfo.Count & 0xFFFF);
        callInfo.Flags = Js::CallFlags_None;
        return callInfo;
    }

    uint32 RemoteInlineFrameWalker::GetCurrentInlineeOffset()
    {
        return m_currentIndex > 0 ?
            RemoteInlinedFrameLayout(m_reader, GetFrameAtIndex(m_currentIndex - 1))->callInfo.InlineeStartOffset :
            0;
    }

    uint32 RemoteInlineFrameWalker::GetBottomMostInlineeOffset()
    {
        Assert(m_frameCount > 0);
        return RemoteInlinedFrameLayout(m_reader, GetFrameAtIndex(m_frameCount - 1))->callInfo.InlineeStartOffset;
    }

    InlinedFrameLayout* RemoteInlineFrameWalker::GetCurrentFrame()
    {
        return this->GetFrameAtIndex(m_currentIndex);
    }

    InlinedFrameLayout* RemoteInlineFrameWalker::GetFrameAtIndex(int index)
    {
        // Note: this can be called to check whether a frame is available.
        return index >= 0 && index < m_frameCount ? 
            m_frames[index] :
            NULL;
    }

} // namespace JsDiag.
