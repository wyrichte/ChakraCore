//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    class MutationBreakpoint;

    typedef enum eStopType
    {
        STOP_BREAKPOINT,
        STOP_INLINEBREAKPOINT,
        STOP_STEPCOMPLETE,
        STOP_EXCEPTIONTHROW,
        STOP_ASYNCBREAK,
        STOP_MUTATIONBREAKPOINT
    } StopType;

    struct ReturnedValue
    {
        ReturnedValue() {}
        ReturnedValue(Js::Var _returnedValue, Js::JavascriptFunction * _calledFunction, bool _isValueOfReturnStatement)
            : returnedValue(_returnedValue), calledFunction(_calledFunction), isValueOfReturnStatement(_isValueOfReturnStatement)
        {
            if (isValueOfReturnStatement)
            {
                Assert(returnedValue == nullptr);
                Assert(calledFunction == nullptr);
            }
        }

        Js::Var returnedValue;
        Js::JavascriptFunction *calledFunction;
        bool isValueOfReturnStatement;
    };

    typedef JsUtil::List<ReturnedValue*> ReturnedValueList;

    class DiagStackFrame;
    typedef JsUtil::Stack<DiagStackFrame*> DiagStack;
    typedef WeakArenaReference<DiagStack> WeakDiagStack;

    struct InterpreterHaltState
    {
        StopType stopType;
        const FunctionBody* executingFunction;
        DiagStackFrame* topFrame;
        DiagStack* framePointers;
        ReferencedArenaAdapter* referencedDiagnosticArena;
        JavascriptExceptionObject* exceptionObject;
        StringBuilder<ArenaAllocator>* stringBuilder;
        MutationBreakpoint* activeMutationBP;

        InterpreterHaltState(StopType _stopType, const FunctionBody* _executingFunction, MutationBreakpoint* _activeMutationBP = nullptr);

        FunctionBody* GetFunction();
        int GetCurrentOffset();
        void SetCurrentOffset(int offset);
        bool IsValid() const;
    };

    class StepController;

    struct HaltCallback
    {
        virtual bool CanHalt(InterpreterHaltState* pHaltState) = 0;
        virtual void DispatchHalt(InterpreterHaltState* pHaltState) = 0;
        virtual void CleanupHalt() = 0;
        virtual bool IsInClosedState() { return false; }

        // Mentions the policy if the hitting a breakpoint is allowed (based on the fact whether we are at callback from the breakpoint)
        virtual bool CanAllowBreakpoints() { return false; }
    };

    struct Probe : HaltCallback
    {
        virtual bool Install(Js::ScriptContext* pScriptContext) = 0;
        virtual bool Uninstall(Js::ScriptContext* pScriptContext) = 0;
    };

    typedef enum eStepType : BYTE
    {
        STEP_NONE,
        STEP_IN         = 0x01,
        STEP_OVER       = 0x02,
        STEP_OUT        = 0x04,
        STEP_DOCUMENT   = 0x08,

        // On entry of an jitted function, need to bailout to handle stepping if in STEP_IN mode,
        // or STEP_OVER (e.g. STEP_OVER at the end of this function, and it is called again by a
        // library caller).
        STEP_BAILOUT    = STEP_IN | STEP_OVER,
    } StepType;

    class StepController
    {
        friend class ProbeManager;
        friend class ProbeContainer;

        StepType stepType;
        int byteOffset;
        RecyclerRootPtr<FunctionBody> body;
        FunctionBody::StatementMap* statementMap;

        int frameCountWhenSet;
        int returnedValueRecordingDepth;

        DWORD_PTR frameAddrWhenSet;
        uint scriptIdWhenSet;

        bool stepCompleteOnInlineBreakpoint;
        ScriptContext *pActivatedContext;

        ReturnedValueList *returnedValueList;

    public:

        StepController();
        ~StepController()
        {
            this->Deactivate();
        }

        bool IsActive();
        void Activate(StepType stepType, InterpreterHaltState* haltState);
        void Deactivate(InterpreterHaltState* haltState = nullptr);
        bool IsStepComplete_AllowingFalsePositives(InterpreterStackFrame * stackFrame);
        bool IsStepComplete(InterpreterHaltState* haltState, HaltCallback *haltCallback, OpCode originalOpcode);
        bool ContinueFromInlineBreakpoint();

        ScriptContext* GetActivatedContext() const
        {
            return this->pActivatedContext;
        }

        const StepType* GetAddressOfStepType() const
        {
            return &stepType;
        }

        void* GetAddressOfScriptIdWhenSet() const
        {
            return (void*)&scriptIdWhenSet;
        }

        void* GetAddressOfFrameAddress() const
        {
            return (void*)&frameAddrWhenSet;
        }

        void SetFrameAddr(DWORD_PTR value)
        {
            this->frameAddrWhenSet = value;
        }

        void AddToReturnedValueContainer(Js::Var returnValue, Js::JavascriptFunction * function, bool isValueOfReturnStatement);
        void AddReturnToReturnedValueContainer();
        void StartRecordingCall();
        void EndRecordingCall(Js::Var returnValue, Js::JavascriptFunction * function);

        ReturnedValueList* GetReturnedValueList() const { return this->returnedValueList; }
        void ResetReturnedValueList();
        void HandleResumeAction(Js::InterpreterHaltState* haltState, BREAKRESUMEACTION resumeAction);

    private:
        uint GetScriptId(_In_ FunctionBody* body);
    };

    // This is separate from the step controller because it is the only case where activation
    // happens while the script is running.

    class AsyncBreakController
    {
    private:
        HaltCallback* haltCallback;

     public:

        AsyncBreakController();
        void Activate(HaltCallback* haltCallback);
        void Deactivate();
        bool IsBreak();
        bool IsAtStoppingLocation(InterpreterHaltState* haltState);
        void DispatchAndReset(InterpreterHaltState* haltState);
    };

    typedef JsUtil::List<Probe*, ArenaAllocator> ProbeList;

    // This class contains the stepper and contains the haltcallback for probing.
    // The object if this class is maintained by threadcontext
    class ProbeManager
    {
        friend class RecyclableObjectDisplay;
        friend class RecyclableArrayWalker;
        template <typename TData> friend class RecyclableCollectionObjectWalker;
        template <typename TData> friend class RecyclableCollectionObjectDisplay;
        friend class RecyclableKeyValueDisplay;
        friend class ProbeContainer;

    private:
        InterpreterHaltState* pCurrentInterpreterLocation; // NULL if not Halted at a Probe
        
        DWORD_PTR secondaryCurrentSourceContext; // for resolving ambiguity among generated files, eg. eval, anonymous, etc.               

        // a unique number, which will be used to sync all probecontainer when on break
        ulong debugSessionNumber;

#if DBG
        void * dispatchHaltFrameAddress;
#endif
    public:
        bool languageServiceEnabled;

        StepController stepController;
        AsyncBreakController asyncBreakController;

        ThreadContext* pThreadContext;

        bool isAtDispatchHalt;

        // Holds the property id of $newValue$ property for object mutation breakpoint
        PropertyId mutationNewValuePid;
        // Holds the property id of $propertyName$ property for object mutation breakpoint
        PropertyId mutationPropertyNamePid;
        // Holds the property id of $mutationType$ property for object mutation breakpoint
        PropertyId mutationTypePid;

    public:
        ProbeManager(ThreadContext* _pThreadContext);
        ~ProbeManager();

        ReferencedArenaAdapter* GetDiagnosticArena();

        //HACK to pass document containment information to the functionBodies
        // I need to determine a better method, but this will work for now.

        DWORD_PTR AllocateSecondaryHostSourceContext();        

        void SetCurrentInterpreterLocation(InterpreterHaltState* pHaltState);
        void UnsetCurrentInterpreterLocation();

        ulong GetDebugSessionNumber() const { return debugSessionNumber; }

        MutationBreakpoint* GetActiveMutationBreakpoint() const;

#if DBG
    void SetDispatchHaltFrameAddress(void * returnAddress) { this->dispatchHaltFrameAddress = returnAddress; }
    void ValidateDebugAPICall();
#endif
    };
} // namespace Js.
