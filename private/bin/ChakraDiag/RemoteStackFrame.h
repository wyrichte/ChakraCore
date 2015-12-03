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
         public CComObjectRoot,
         public IJsDebugFrame
    {
        DECLARE_NOT_AGGREGATABLE(RemoteStackFrame)
        BEGIN_COM_MAP(RemoteStackFrame)
            COM_INTERFACE_ENTRY(IJsDebugFrame)
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
        CComPtr<InspectionContext> m_context; // Optional, used by locals inspection
        AutoPtr<RemoteInterpreterStackFrame> m_interpreterFrame;
        
        // Valid only within lifetime of this RemoteStackFrame.
        // Alternative solution could be to use RefCounted<RemoteDiagFrame>, encapsulate RemoteStackFrame in it 
        // and have all inspection use RemoteDiagFrame. Seems to be a bit of overkill.
        AutoPtr<RemoteDiagFrame> m_diagFrame;

    public:
        RemoteStackFrame();
        ~RemoteStackFrame();

        ULONG GetRow();
        ULONG GetColumn();
        wchar_t* GetUri() const;
        wchar_t* GetFunctionName() const;
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

        void SetInspectionContext(InspectionContext* context) { m_context = context; }
        InspectionContext* GetInspectionContext() const { return m_context; }
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

         // Returns an instance of RemoteDiagFrame that is valid only within lifetime of this RemoteStackFrame instance,
         // that's why is has "temp" in the name.
         // The instance is owned by RemoteStackFrame, so do not call destructor outside.
         RemoteDiagFrame* GetTempDiagFrame();

         /* === IJsDebugFrame  ===== */
         virtual STDMETHODIMP GetStackRange(
            /* [out] */ __RPC__out UINT64 *pStart,
            /* [out] */ __RPC__out UINT64 *pEnd);

        virtual STDMETHODIMP GetName(
            /* [out] */ __RPC__deref_out_opt BSTR *pbstrName);

        virtual STDMETHODIMP GetDocumentPositionWithId(
            /* [out] */ __RPC__out UINT64 *pDocumentId,
            /* [out] */ __RPC__out DWORD *pCharacterOffset,
            /* [out] */ __RPC__out DWORD *pStatementCharCount);

        virtual STDMETHODIMP GetDocumentPositionWithName(
            /* [out] */ __RPC__deref_out_opt BSTR *pDocumentName,
            /* [out] */ __RPC__out DWORD *pLine,
            /* [out] */ __RPC__out DWORD *pColumn);

        virtual STDMETHODIMP GetDebugProperty(
            /* [out] */ __RPC__deref_out_opt IJsDebugProperty **ppDebugProperty);

        virtual STDMETHODIMP GetReturnAddress(
            /* [out] */ __RPC__out UINT64 *pReturnAddress);

        virtual STDMETHODIMP Evaluate(
            /* [in] */ __RPC__in LPCOLESTR pExpressionText,
            /* [out] */ __RPC__deref_out_opt IJsDebugProperty **ppDebugProperty,
            /* [out] */ __RPC__deref_out_opt BSTR *pError);

    private:
        void SetByteCodeOffset(int byteCodeOffset);
        void SetIsInlineFrame(bool isInlineFrame);
        void SetInterpreterFrame(RemoteInterpreterStackFrame* interpreterFrame);
        void SetLoopBodyNumber(uint loopNumber);

        bool IsAmd64FakeEHFrame() const
        {
#ifdef _M_AMD64
            return !GetRemoteFunctionBody(); // This is the only frame type that doesn't have a functionBody.
#endif
            return false;
        }

        void GetRowAndColumn(ULONG* pRow, ULONG* pColumn);
        void GetStatementStartAndEndOffset(ULONG* startOffset, ULONG* endOffset);
        static wchar_t* WcsDup(_In_z_ const wchar_t* src, size_t maxCharacterCount);
        RemoteDiagFrame* CreateDiagFrame();

    private:
        // Implementation of RemoteDiagFrame, for GetTempDiagFrame, private to this class to avoid mis-use.
        class RemoteDiagInterpreterFrame : public RemoteDiagFrame
        {
        private:
            RemoteStackFrame* m_actualFrame;    // Note: this class is always lives as part of RemoteStackFrame, thus no reference counting is needed.

        public:
            RemoteDiagInterpreterFrame(RemoteStackFrame* actualFrame);

            virtual ScriptContext* GetScriptContext() override;
            virtual FrameDisplay* GetFrameDisplay(RegSlot frameDisplayRegister) override;
            virtual Js::Var GetInnerScope(RegSlot scopeLocation) override;
            virtual Js::Var GetRootObject() override;
            virtual Js::Var GetArgumentsObject() override;
            virtual Js::Var GetReg(RegSlot reg) override;
            virtual Js::Var* GetInParams() override;
            virtual int GetInParamCount() override;
            virtual RemoteFunctionBody* GetRemoteFunctionBody() override;
            virtual ScriptFunction* GetFunction() override;
            virtual bool IsInterpreterFrame() override;
            virtual UINT16 GetFlags();
        };

        class RemoteDiagNativeFrame : public RemoteDiagFrame
        {
        private:
            RemoteStackFrame* m_actualFrame;    // Note: this class is always lives as part of RemoteStackFrame, thus no reference counting is needed.
            int32 m_localVarSlotsOffset;

        public:
            RemoteStackFrame::RemoteDiagNativeFrame(RemoteStackFrame* actualFrame);

            virtual ScriptContext* GetScriptContext() override;
            virtual FrameDisplay* GetFrameDisplay(RegSlot frameDisplayRegister) override;
            virtual Js::Var GetInnerScope(RegSlot scopeLocation) override;
            virtual Js::Var GetRootObject() override;
            virtual Js::Var GetArgumentsObject() override;
            virtual Js::Var GetReg(RegSlot reg) override;
            virtual Js::Var* GetInParams() override;
            virtual int GetInParamCount() override;
            virtual RemoteFunctionBody* GetRemoteFunctionBody() override;
            virtual ScriptFunction* GetFunction() override;
            virtual bool IsInterpreterFrame() override;
            virtual UINT16 GetFlags();

        private:
            Js::Var* GetNonTempSlotOffsetLocation(RegSlot slotId);
        };
        // End of implementation of RemoteDiagFrame.

    };
} // namespace JsDiag.
