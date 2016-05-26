//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Represents remote stack frame.
    // Contains "disconnected" data rather than hooks to retrive names from remote process (which could be alrernarive impl).
    //
    class RemoteStackFrame :
         public CComObjectRoot
    {
        DECLARE_NOT_AGGREGATABLE(RemoteStackFrame)
        BEGIN_COM_MAP(RemoteStackFrame)
        END_COM_MAP()

        // Delay intialized class for row/column.
        class RowColumnInfo
        {
            ULONG m_row;
            ULONG m_column;
            bool m_isInitialized;
        public:
            RowColumnInfo();
            ULONG GetRow(RemoteStackFrame* frame);
            ULONG GetColumn(RemoteStackFrame* frame);
        };

        struct StatementStartEndOffsetInfo
        {
            ULONG m_startOffset;
            ULONG m_endOffset;
            bool m_isInitialized;
        public:
            StatementStartEndOffsetInfo();
            ULONG GetStartOffset(RemoteStackFrame* frame);
            ULONG GetEndOffset(RemoteStackFrame* frame);
        };

        friend class RemoteStackWalker;

    private:
        // Fields.
        void* m_effectiveFrameBase;         // Parent SP -- points right to the arguments (amd64) or EBP/r11 (x86/ARM).
        void* m_returnAddress;
        void* m_instructionPointer;
        void* m_frameBase;                // InternalStackFrame::FrameBase
        void* m_stackPointer;
        void* m_frameEnd;
        void* m_frameStart;
        void* m_frameStartReturnAddress;
        RefCounted<RemoteFunctionBody> m_functionBody;   // Multiple stack frames may refer to the same function. Use a shared RemoteFunctionBody.
        RefCounted<RemoteScriptContext> m_scriptContext; // Multiple stack frames may refer to the same script context. Use shared one.
        JavascriptFunction* m_functionAddr;              // Used only by Hybrid/not WER.
        void** m_argvAddr;                               // Used only by Hybrid/not WER.
        Js::CallInfo m_callInfo;                         // Used only by Hybrid/not WER.
        bool m_isInlineFrame;
        int m_byteCodeOffset;
        uint m_loopNumber;
        uint m_frameId;

        RowColumnInfo m_rowColumn;
        StatementStartEndOffsetInfo m_statementStartOffset;

        // locals inspection
        AutoPtr<RemoteInterpreterStackFrame> m_interpreterFrame;
        
        

    public:
        RemoteStackFrame();
        ~RemoteStackFrame();

        ULONG GetRow();
        ULONG GetColumn();
        char16* GetUri() const;
        char16* GetFunctionName() const;
        bool IsInlineFrame() const;
        void* GetEffectiveFrameBase() const;
        void* GetReturnAddress() const;
        void* GetInstructionPointer() const;
        void* GetFrameBase() const;
        void* GetStackPointer() const;

        void SetEnd(void* frameEnd);
        void SetStart(void* frameStart);
        void SetReturnAddress(void* returnAddress);
        bool IsTopJavascriptFrame() const { return m_frameId == 1; }
        void SetFrameId(uint frameId) { m_frameId = frameId; }

        RemoteFunctionBody* GetRemoteFunctionBody() const { return m_functionBody->Get(); }
        RemoteScriptContext* GetRemoteScriptContext() const { return m_scriptContext->Get(); }
        JavascriptFunction* GetFunction() const { return m_functionAddr; }
        int GetByteCodeOffset() const { return m_byteCodeOffset; }

        void** GetArgvAddr();
        Js::CallInfo GetCallInfo();
        bool IsInterpreterFrame() const { return m_interpreterFrame != NULL; }

    private:
        RemoteInterpreterStackFrame* GetRemoteInterpreterFrame() const { return m_interpreterFrame; }
    
    public:
         void Init(const RefCounted<RemoteFunctionBody>& functionBody, const RefCounted<RemoteScriptContext>& scriptContext, 
             JavascriptFunction* function, void** argvAddr, Js::CallInfo callInfo, const InternalStackFrame* frame);

       

    private:
        void SetByteCodeOffset(int byteCodeOffset);
        void SetIsInlineFrame(bool isInlineFrame);
        void SetInterpreterFrame(RemoteInterpreterStackFrame* interpreterFrame);
        void SetLoopBodyNumber(uint loopNumber);

        bool IsAmd64FakeEHFrame() const
        {
#ifdef _M_AMD64
            return !GetRemoteFunctionBody(); // This is the only frame type that doesn't have a functionBody.
#else
            return false;
#endif
        }

        void GetRowAndColumn(ULONG* pRow, ULONG* pColumn);
        void GetStatementStartAndEndOffset(ULONG* startOffset, ULONG* endOffset);
        static char16* WcsDup(_In_z_ const char16* src, size_t maxCharacterCount);
        

   

    };
} // namespace JsDiag.
