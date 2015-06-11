//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#include "stdafx.h"

namespace JsDiag
{
    class RemoteInlineFrameWalker
    {
        // Nested types    
        enum InlineFrameIndex
        {
            IFI_This = 0,
            IFI_SecondScriptArg = 1
        };
    
        // Fields
    private:
        IVirtualReader* m_reader;
        InlinedFrameLayout** m_frames;
        int m_frameCount;
        int m_currentIndex;

    private:
        RemoteInlineFrameWalker(IVirtualReader* reader, int frameCount, __in_ecount(frameCount) InlinedFrameLayout** frames, Js::JavascriptFunction* parent);

    public:
        static RemoteInlineFrameWalker* FromPhysicalFrame(IVirtualReader* reader, InternalStackFrame* currentFrame, Js::ScriptFunction * parent);
        bool Next();
        void** GetCurrentArgvAddress();
        Js::JavascriptFunction* GetCurrentFunction();
        Js::CallInfo GetCurrentCallInfo();
        uint32 GetCurrentInlineeOffset();   // TODO: add comment to say what this means.
        uint32 GetBottomMostInlineeOffset();

    private:
        InlinedFrameLayout* GetCurrentFrame();
        InlinedFrameLayout* GetFrameAtIndex(int index);
    }; // class RemoteInlineFrameWalker.
} // namespace JsDiag.
