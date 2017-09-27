//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Stack walker for remote process.
    // Main difference from JavascriptStackWalker is that we have to DAC/can't call into runtime directly,
    // so that for every field access we read target process memory.
    //
    class RemoteStackWalker
    {
        // Private types.
        enum InternalFrameType
        {
            IFT_None,
            IFT_LoopBody,
            IFT_EHFrame
        };

        // Kind of state machine for internal frame.
        // Note that we could implement internal frame tracking simpler by just using m_internalFrame field,
        // we'll just have more checks for InterpreterStateFrame::IsMyInternalFrame
        // Anyhow it seems to be make sense to have logic close to JavascriptStackWalker.
        class InternalFrameTracker
        {
            enum InternalFrameState
            {
                IFS_NoFrame,
                IFS_FrameDetected,
                IFS_FrameConsumed
            };

            InternalFrameState m_state;
            void* m_frameAddress;
            InternalFrameType m_frameType;
        public:
            InternalFrameTracker();
            void Reset();
            void SetFrame(void* frameAddr, InternalFrameType frameType);
            void* GetFrameAddress();
            InternalFrameType GetFrameType();
            bool IsFrameConsumed();
            void SetIsFrameConsumed();
        private:
            void SetState(InternalFrameState state);
#ifdef DBG
            bool IsValidStateTransition(InternalFrameState newState);
#endif DBG
        }; // InternalFrameTracker.

        // Fields of RemoteStackWalker.
        DebugClient* m_debugClient;
        AutoPtr<RemoteStackFrameEnumerator> m_frameEnumerator;
        IVirtualReader* m_reader;
        ULONG m_threadId;
        uint m_currentJavascriptFrameIndex;

        AutoPtr<RemoteScriptEntryExitRecord> m_scriptEntryExitRecord;
        RefCounted<RemoteScriptContext> m_scriptContext;

        // Corresponds to InterpreterStackFarme::previousInterpreterFrame, which is interpreter frame below that we haven't walked to yet.
        AutoPtr<RemoteInterpreterStackFrame> m_prevInterpreterFrame;
        
        // If current frame is an interpreter frame, this is the corresponding InterpreterStackFrame, otherwise NULL.
        AutoPtr<RemoteInterpreterStackFrame> m_interpreterFrame;

        AutoPtr<InternalStackFrame> m_currentFrame;
        InternalFrameTracker m_lastInternalFrame;
        AutoPtr<RemoteInlineFrameWalker> m_inlineWalker;

        void* m_scriptExitFrameAddr;  // The frame base of the physical runtime frame above current frame, or if it's topmost frame, current stack pointer.
        void* m_scriptEntryFrameBase; // The stack pointer of the bottom most physical runtime frame.
        void* m_scriptEntryReturnAddress; // The return address of the script entry frame
        bool m_isJavascriptFrame;

        bool m_checkedFirstInterpreterFrame;    // Used for extra validation for first interpreter frame

        CAtlMap<const FunctionBody*, RefCounted<RemoteFunctionBody>> m_functionBodyMap;    // map to cache RemoteFunctionBody
        CAtlMap<const ScriptContext*, RefCounted<RemoteScriptContext>> m_scriptContextMap; // map to cache RemoteScriptContext

    public:
        RemoteStackWalker(DebugClient* debugClient, ULONG threadId, ThreadContext* threadContextAddr);
        ~RemoteStackWalker();

        bool WalkToNextJavascriptFrame();
        void GetCurrentJavascriptFrame(RemoteStackFrame** frame);
        void* GetCurrentScriptExitFrameBase();
        void* GetCurrentScriptEntryFrameBase();
        void* GetCurrentScriptEntryReturnAddress();
        Js::JavascriptFunction* GetCurrentFunction(bool includeInlineFrames = true);
        Js::CallInfo GetCurrentCallInfo(bool includeInlineFrames = true);

    private:
        bool WalkOneFrame();
        bool IsJavascriptFrame();
        void* AdvanceToFrame(const void* frameAddress);
        bool CheckJavascriptFrame();
        void UpdateFrame();
        bool IsInterpreterFrame(void* instructionPointer);
        bool IsNativeFrame(void* instructionPointer);
        uint32 GetByteCodeOffset(uint* loopNumber);
        void GetCodeAddrAndLoopNumberFromCurrentFrame(DWORD_PTR* codeAddr, uint* loopNumber);

        void** GetCurrentArgvAddress(bool includeInlineFrames = true);

        void SetScriptContext(const Js::ScriptContext* newScriptContextAddr);
        void SetScriptEntryExitRecord(const Js::ScriptEntryExitRecord* record);

        void GetRefCountedRemoteFunctionBody(const FunctionBody* addr, _Out_ RefCountedObject<RemoteFunctionBody>** ppRefCountedRemoteFunctionBody);
        void GetRefCountedRemoteScriptContext(const ScriptContext* addr, _Out_ RefCountedObject<RemoteScriptContext>** ppRefCountedRemoteScriptContext);

#if defined(DBG) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
        void DumpFrame();
#endif
    }; // class RemoteStackWalker.
} // namespace JsDiag.
