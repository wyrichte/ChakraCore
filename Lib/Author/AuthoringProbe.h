//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class AuthoringProbe : public Js::Probe
    {
    private:
        bool m_triggered;
        bool m_ignoreNextCleanup;
        bool m_awaitingAsync;
        bool m_asyncBreakTriggered;
        bool m_uninstallProbeOnCleanup;
        bool m_checkingExceptions;
        Js::RegSlot m_slot;
        Js::ParseableFunctionInfo *m_body;
        int m_offset;
        Js::Var m_value;
        int m_exceptionCount;
        Js::FunctionBody *m_lastExceptionBody;
        bool m_loopGuardsEnabled;
        bool m_enableLoopGuardsOnAsyncBreak;
        bool m_keepLoopGuardOnUninstall; // This enforces to not disable the loop guard on uninstall.
        bool m_isFinalPass;
        bool m_isAbortCalled;
        IAuthorCompletionDiagnosticCallback *m_diagCallback;

    public:
        AuthoringProbe(Js::RegSlot slot, Js::ParseableFunctionInfo *body, bool enableLoopGuardsOnAsyncBreak = true, int breakpointOffset = -1, IAuthorCompletionDiagnosticCallback *diagCallback = nullptr, bool isFinalPass = false);
        ~AuthoringProbe();

        bool Triggered() { return m_triggered; }
        bool AsyncBreakTriggered() { return m_asyncBreakTriggered; }
        void Reset() { m_asyncBreakTriggered = false; m_triggered = false; m_awaitingAsync = false; }

        void SetKeepLoopGuardOnUninstall(bool set) { m_keepLoopGuardOnUninstall = set; }

        Js::Var Value() { return m_value; }

        virtual bool CanHalt(Js::InterpreterHaltState* pHaltState);
        virtual void DispatchHalt(Js::InterpreterHaltState* pHaltState);
        virtual void CleanupHalt();
        virtual bool Install(Js::ScriptContext* pScriptContext);
        virtual bool Uninstall(Js::ScriptContext* pScriptContext);
        virtual bool CanAllowBreakpoints() { return true; }

        void EnableAsyncBreak(Js::ScriptContext *scriptContext, bool isAbort);
        void ClearAsyncBreak(Js::ScriptContext *scriptContext);
    };
}
