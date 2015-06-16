//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    AuthoringProbe::AuthoringProbe(Js::RegSlot slot, Js::ParseableFunctionInfo *body, bool enableLoopGuardsOnAsyncBreak, int breakpointOffset, IAuthorCompletionDiagnosticCallback *diagCallback, bool isFinalPass)
            : m_triggered(false), 
            m_slot(slot), 
            m_body(body), 
            m_awaitingAsync(false), 
            m_asyncBreakTriggered(false), 
            m_value(nullptr),
            m_checkingExceptions(true),
            m_lastExceptionBody(nullptr),
            m_exceptionCount(0),
            m_loopGuardsEnabled(false),
            m_enableLoopGuardsOnAsyncBreak(enableLoopGuardsOnAsyncBreak),
            m_keepLoopGuardOnUninstall(false),
            m_isFinalPass(isFinalPass),
            m_diagCallback(diagCallback),
            m_isAbortCalled(false)
     {
        if (m_diagCallback)
        {
            m_diagCallback->AddRef();
        }

        if(breakpointOffset == -1)
        {
            // No breakpoint specified
            // Construct a probe to be used to execute complete methods with no breakpoints installed
            m_offset = 0;
            m_ignoreNextCleanup = true;
            m_uninstallProbeOnCleanup = false;
        }
        else
        {
            // Construct a probe to run until installed breakpoints
            m_offset = breakpointOffset;
            m_ignoreNextCleanup = false;
            m_uninstallProbeOnCleanup = true;
        }

        m_loopGuardsEnabled = LoopGuardsHelpers::LoopGuardsEnabled(m_body->GetScriptContext());
    }

    AuthoringProbe::~AuthoringProbe()
    {
        ReleasePtr(m_diagCallback);
    }

    bool AuthoringProbe::CanHalt(Js::InterpreterHaltState* pHaltState)
    {
        return !m_triggered && pHaltState->stopType == Js::STOP_BREAKPOINT;
    }
        
    void AuthoringProbe::DispatchHalt(Js::InterpreterHaltState* pHaltState)
    {
        if (!m_triggered)
        {
            switch (pHaltState->stopType) 
            {
            case Js::STOP_BREAKPOINT:
                {
                    m_awaitingAsync = false;

                    if (pHaltState->GetCurrentOffset() == m_offset && pHaltState->GetFunction() == m_body->GetFunctionBody())
                    {
                        // This is the probe set at the cursor, record the result of the
                        // expression.
                        m_triggered = true;
                        m_value = pHaltState->framePointers->Peek(0)->AsInterpreterFrame()->GetReg(m_slot);

                        if (m_diagCallback)
                        {
                            // Determine whether to invoke the diagnostic callback
                            Js::ScriptContext * scriptContext = m_body->GetScriptContext();
                            Js::TempArenaAllocatorObject *tempArena = nullptr; 
                            
                            if (// Anytime the value at the breakpoint is type null or undefined and is not a tracking value, invoke the callback
                                (m_value && JsHelpers::IsNullOrUndefined(m_value) && !JsHelpers::IsTrackingKeyValue(scriptContext, m_value)) ||
                                // If the value is null or is an exception object, invoke the callback if it is the final execution pass, as these
                                // conditions will trigger additional passes.
                                (m_isFinalPass && (!m_value || JsHelpers::IsExceptionObject(
                                    (tempArena = scriptContext->GetTemporaryAllocator(L"completionsCallbackAlloc"))->GetAllocator(),
                                    scriptContext, m_value
                                    )
                                )))
                            {
                                m_diagCallback->Invoke();
                            }

                            if (tempArena)
                            {
                                scriptContext->ReleaseTemporaryAllocator(tempArena);
                            }
                        }
                    }
                    else
                    {
                        OUTPUT_TRACE(Js::JSLSPhase, L"AuthoringProbe::DispatchHalt Stopped due to AsyncBreak, m_enableLoopGuardsOnAsyncBreak : %ls, m_loopGuardsEnabled : %ls\n", m_enableLoopGuardsOnAsyncBreak ? L"yes" : L"no", m_loopGuardsEnabled ? L"yes" : L"no");
                        // async stop caused by Hurry().
                        m_asyncBreakTriggered = true;
                        if (!m_isAbortCalled && m_enableLoopGuardsOnAsyncBreak && !m_loopGuardsEnabled)
                        {
                            // This is the first time hurry is called, so enable loop guards and continue
                            Assert(!LoopGuardsHelpers::LoopGuardsEnabled(m_body->GetScriptContext()));
                            m_loopGuardsEnabled = true;
                            LoopGuardsHelpers::EnableLoopGuards(m_body->GetScriptContext());
                            Assert(LoopGuardsHelpers::LoopGuardsEnabled(m_body->GetScriptContext()));

                            // Disable next cleanup to avoid removing the original breakpoint
                            m_ignoreNextCleanup = true;
                            break;
                        }

                    }

                    // In either case stop execution.
                    throw ExecutionStop();
                }
                break;
            case Js::STOP_EXCEPTIONTHROW:
                {
                    // Skip all exceptions
                    if(pHaltState->exceptionObject)
                    {
                        // We shouldn't have debug-dispatched the ForceCatch execution exception - so we shouldn't be here.
                        Assert(!pHaltState->exceptionObject->IsForceCatchException());

                        pHaltState->exceptionObject->SetDebuggerSkip(true);
                        m_ignoreNextCleanup = true;

#ifdef DEBUG                        
                        auto errorObject = pHaltState->exceptionObject->GetThrownObject(null);
                        if(Js::JavascriptError::Is(errorObject))
                        {
                            auto err = Js::JavascriptError::FromVar(errorObject);
                            LPCWSTR msg = nullptr;
                            Js::JavascriptError::GetRuntimeError(err, &msg);
                            if(msg)
                            {
                                ::OutputDebugStringW(msg);
                                ::OutputDebugStringW(L"\n");
                            }
                        }
#endif

                        OUTPUT_TRACE(Js::JSLSPhase, L"AuthoringProbe::DispatchHalt Stopped due to Exception Break, m_checkingExceptions : %ls\n", m_checkingExceptions ? L"yes" : L"no");

                        if (m_checkingExceptions)
                        {
                            // Determine if we should lower the loop guard
                            auto exceptionBody = pHaltState->GetFunction();
                            if (exceptionBody == m_lastExceptionBody)
                            {
                                // If we get more than one exception in the same function start counting them.
                                auto count = ++m_exceptionCount;
                                if (count > LOOP_EXCEPTION_LIMIT)
                                {
                                    // If we get more then LOOP_EXCEPTION_LIMIT exceptions in the same function, lower 
                                    // the loop guards because we are probably repeating a loop that generates exceptions.
                                    auto scriptContext = pHaltState->GetFunction()->GetScriptContext();
                                    auto global = scriptContext->GetGlobalObject();
                                    JsHelpers::SetField(global, LOOP_GUARD_LIMIT, 1, scriptContext);
                                    OUTPUT_TRACE(Js::JSLSPhase, L"AuthoringProbe::DispatchHalt setting %ls to 1\n", LOOP_GUARD_LIMIT);
                                    m_checkingExceptions = false;
                                }
                            }
                            else
                                m_lastExceptionBody = exceptionBody;
                        }
                        
                    }
                    if (m_awaitingAsync)
                    {
                        EnableAsyncBreak(pHaltState->framePointers->Peek(0)->GetScriptContext(), /* isAbort = */false);
                    }
                }
                break;
            }
        }
        else if (pHaltState->stopType == Js::STOP_EXCEPTIONTHROW)
        {
            // we have to ingnore the next cleanup, otherwise we will try to clear the same breakpoint again.
            m_ignoreNextCleanup = true;
        }
    }

    void AuthoringProbe::CleanupHalt() 
    {
        if (!m_ignoreNextCleanup && m_uninstallProbeOnCleanup)
            m_body->GetFunctionBody()->UninstallProbe(m_offset);
        m_ignoreNextCleanup = false;
    }

    bool AuthoringProbe::Install(Js::ScriptContext* pScriptContext)  { return true; }
    bool AuthoringProbe::Uninstall(Js::ScriptContext* pScriptContext) 
    { 
        if (m_loopGuardsEnabled && !m_keepLoopGuardOnUninstall)
        {
            // LoopGuards were enabled during the course of execution, disable them
            LoopGuardsHelpers::DisableLoopGuards(m_body->GetScriptContext());
            Assert(!LoopGuardsHelpers::LoopGuardsEnabled(m_body->GetScriptContext()));
        }
        return true; 
    }

    void AuthoringProbe::EnableAsyncBreak(Js::ScriptContext *scriptContext, bool isAbort) 
    {
        this->m_awaitingAsync = true;
        this->m_isAbortCalled = this->m_isAbortCalled || isAbort;
        scriptContext->diagProbesContainer.AsyncActivate(this);
    }

    void AuthoringProbe::ClearAsyncBreak(Js::ScriptContext *scriptContext)
    {
        scriptContext->diagProbesContainer.AsyncDeactivate();
    }

}