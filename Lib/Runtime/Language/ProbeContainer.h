//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    class MutationBreakpoint;

    // This class contains the probes and list of function bodies.
    // The object of this class is maintained by Script context.
    class ProbeContainer
    {
        friend class RecyclableObjectDisplay;
        friend class RecyclableArrayWalker;

    private:
        ProbeList* diagProbeList;
        ProbeList* pendingProbeList;

        ScriptContext* pScriptContext;
        ProbeManager *pProbeManager;

        // Stack for a current scriptcontext
        DiagStack* framePointers;

        HaltCallback* haltCallbackProbe;
        IRemoteDebugApplication110 *pDebugApp110;

        // Refer to the callback which is responsible for making async break
        HaltCallback* pAsyncHaltCallback;

        Var jsExceptionObject;

        // Used for synchronizing with ProbeMananger
        ulong debugSessionNumber;

        uint32  tmpRegCount; // Mentions the temp register count for the current statement (this will be used to determine if SetNextStatement can be applied)

        // Used when SetNextStatement is applied.
        int bytecodeOffset;
        bool IsNextStatementChanged;

        // Used when the throw is internal and engine does not want to be broken at exception.
        bool isThrowInternal;

        // This variabled will be set true when we don't want to check for debug script engine being initialized.
        bool forceBypassDebugEngine;

        bool isPrimaryBrokenToDebuggerContext;

        JsUtil::List<DWORD_PTR, ArenaAllocator> *registeredFuncContextList;
        JsUtil::List<const Js::PropertyRecord*> *pinnedPropertyRecords;

        template <bool maySkipStack> void UpdateFramePointers(bool fMatchWithCurrentScriptContext);
        bool InitializeLocation(InterpreterHaltState* pHaltState, bool fMatchWithCurrentScriptContext = true);
        void DestroyLocation();

        bool ProbeContainer::GetNextUserStatementOffsetHelper(
            Js::FunctionBody* functionBody, int currentOffset, FunctionBody::StatementAdjustmentType adjType, int* nextStatementOffset);

        BOOL IsScriptDebuggerOptionsEnabled(SCRIPT_DEBUGGER_OPTIONS flag);

        void InitMutationBreakpointListIfNeeded();
        void ClearMutationBreakpoints();
        static bool FetchTmpRegCount(Js::FunctionBody * functionBody, Js::ByteCodeReader * reader, int atOffset, uint32 *pTmpRegCount, Js::OpCode *pOp);
    public:

        bool isForcedToEnterScriptStart;

        ProbeContainer();
        ~ProbeContainer();

        void StartRecordingCall();
        void EndRecordingCall(Js::Var returnValue, Js::JavascriptFunction * function);
        ReturnedValueList* GetReturnedValueList() const { return this->pProbeManager->stepController.GetReturnedValueList(); }
        void ResetReturnedValueList() { this->pProbeManager->stepController.ResetReturnedValueList(); }

        void Initialize(ScriptContext* pScriptContext);
        void Close();

        WeakDiagStack* GetFramePointers();

        // A break engine responsible for breaking at iniline statement and error statement.
        void InitializeInlineBreakEngine(HaltCallback* pProbe);
        void InitializeForScriptOption(IRemoteDebugApplication110 *pDebugApp);

        void UninstallInlineBreakpointProbe(HaltCallback* pProbe);

        void AddProbe(Probe* pProbe);
        void RemoveProbe(Probe* pProbe);

        void RemoveAllProbes();

        // When on breakpoint hit
        void DispatchProbeHandlers(InterpreterHaltState* pHaltState);

        // When on step in, step out and step over
        void DispatchStepHandler(InterpreterHaltState* pHaltState, OpCode* pOriginalOpcode);

        // When on break-all
        void DispatchAsyncBreak(InterpreterHaltState* pHaltState);

        // When executing 'debugger' statement
        void DispatchInlineBreakpoint(InterpreterHaltState* pHaltState);

        // When encountered and exception
        bool DispatchExceptionBreakpoint(InterpreterHaltState* pHaltState);

        // When on mutation breakpoint hit
        void DispatchMutationBreakpoint(InterpreterHaltState* pHaltState);

        void UpdateStep(bool fDuringSetupDebugApp = false);
        void DeactivateStep();

        bool GetNextUserStatementOffsetForSetNext(Js::FunctionBody* functionBody, int currentOffset, int* nextStatementOffset);
        bool GetNextUserStatementOffsetForAdvance(Js::FunctionBody* functionBody, ByteCodeReader* reader, int currentOffset, int* nextStatementOffset);
        bool AdvanceToNextUserStatement(Js::FunctionBody* functionBody, ByteCodeReader* reader);

        void SetNextStatementAt(int bytecodeOffset);
        bool IsSetNextStatementCalled() const { return IsNextStatementChanged; }
        int GetByteCodeOffset() const { Assert(IsNextStatementChanged); return bytecodeOffset; }

        void AsyncActivate(HaltCallback* haltCallback);
        void AsyncDeactivate();

        void PrepDiagForEnterScript();

        void RegisterContextToDiag(DWORD_PTR context, ArenaAllocator *alloc);
        bool IsContextRegistered(DWORD_PTR context);
        FunctionBody * GetGlobalFunc(ScriptContext* scriptContext, DWORD_PTR secondaryHostSourceContext);

        Var GetExceptionObject() { return jsExceptionObject; }

        bool HasAllowedForException(__in JavascriptExceptionObject* exceptionObject);

        void SetThrowIsInternal(bool set) { isThrowInternal = set; }

        BOOL IsNonUserCodeSupportEnabled();
        BOOL IsLibraryStackFrameSupportEnabled();

        void SetCurrentTmpRegCount(uint32 set) { tmpRegCount = set; }
        uint32 GetCurrentTmpRegCount() const { return tmpRegCount; }
        void PinPropertyRecord(const Js::PropertyRecord *propertyRecord);

        bool IsPrimaryBrokenToDebuggerContext() const { return isPrimaryBrokenToDebuggerContext; }
        void SetIsPrimaryBrokenToDebuggerContext(bool set) { isPrimaryBrokenToDebuggerContext = set; }

        ProbeManager *GetProbeManager() const { return this->pProbeManager; }

        typedef JsUtil::List<RecyclerWeakReference<Js::MutationBreakpoint>*, Recycler, false, Js::WeakRefFreeListedRemovePolicy> MutationBreakpointList;
        RecyclerRootPtr<MutationBreakpointList> mutationBreakpointList;
        bool HasMutationBreakpoints();
        void InsertMutationBreakpoint(MutationBreakpoint *mutationBreakpoint);
        void RemoveMutationBreakpointListIfNeeded();       
        static bool IsTmpRegCountIncreased(Js::FunctionBody* functionBody, ByteCodeReader* reader, int currentOffset, int nextStmOffset, bool restoreOffset);
    };
} // namespace Js.
