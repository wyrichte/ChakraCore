//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------
#include "StdAfx.h"

namespace JsDiag
{
    IVirtualReader* JsDebugProcess::GetReader() const { return m_debugClient.ptr->GetReader(); }

    STDMETHODIMP JsDebugProcess::CreateStackWalker(
        /* [in] */ DWORD threadId,
        /* [out] */ __RPC__deref_out_opt IJsDebugStackWalker **ppStackWalker)
    {
        return JsDebugApiWrapper([=] 
        {
            CreateComObject<JsDebugStackWalker>(this, threadId, ppStackWalker);
            return S_OK;
        });
    }

    STDMETHODIMP JsDebugProcess::CreateBreakPoint( 
        /* [in] */ UINT64 documentId,
        /* [in] */ DWORD characterOffset,
        /* [in] */ DWORD characterCount,
        /* [in] */ BOOL isEnabled,
        /* [out] */ _Out_ IJsDebugBreakPoint **ppDebugBreakPoint)
    {
        return JsDebugApiWrapper([=] 
        {
            JsDebugBreakPoint* breakPoint;
            CreateComObject(this, documentId, characterOffset, characterCount, isEnabled != 0, &breakPoint);
            *ppDebugBreakPoint = breakPoint;
            return S_OK;
        });
    }

    void JsDebugProcess::Init(UINT64 baseAddress, IJsDebugDataTarget* debugDataTarget, IStackProviderDataTarget* testDataTarget, DWORD processId, bool validateDebugMode)
    {
        m_validateDebugMode = validateDebugMode;
        m_debugDataTarget = debugDataTarget;
        if (testDataTarget != nullptr)
        {
            m_diagProvider = new(oomthrow) DbgEngDiagProvider(testDataTarget);
        }
        else
        {
            m_diagProvider = new(oomthrow) VSDiagProvider(debugDataTarget, baseAddress);
        }
        m_debugClient = new(oomthrow) DebugClient(m_diagProvider);

        m_processId = processId;
        m_remoteAllocator = new(oomthrow) RemoteAllocator(m_debugClient->GetReader());

        if(validateDebugMode)
        {
            RemoteConfiguration::SetHybridDebugging(m_debugClient->GetReader(),  m_debugClient->GetGlobalPointer<const Configuration>(Globals_Configuration));
        }
        // Sets up a named event to let the target process know that it's being debugged.
    }

    STDMETHODIMP JsDebugProcess::PerformAsyncBreak(
          /*in*/ DWORD threadId)
    {
        return JsDebugApiWrapper([=] 
        {
            ThreadContextTLSEntry* threadContextTlsEntry = m_debugClient->GetThreadContextTlsEntry(threadId);
            if (threadContextTlsEntry == nullptr)
            {
                DiagException::Throw(E_JsDEBUG_UNKNOWN_THREAD);
            }
            auto reader = m_debugClient->GetReader();
            RemoteThreadContextTLSEntry tlsEntry(reader, threadContextTlsEntry);
            RemoteThreadContext remoteThreadContext(reader, tlsEntry.GetThreadContext());
            RemoteDebugManager debugManager(reader, remoteThreadContext.GetDebugManager());
            ScriptContext* scriptContext = remoteThreadContext.GetScriptContextList();
            HaltCallback* callback = nullptr;
            while (scriptContext != nullptr)
            {
                RemoteScriptContext remoteScriptContext(reader, scriptContext);
                if(remoteScriptContext->isClosed)
                {
                    scriptContext = remoteScriptContext->next;
                    continue;
                }
                RemoteProbeContainer probeContainer(reader, remoteScriptContext.GetProbeContainer());
                callback = remoteScriptContext->scriptEngineHaltCallback;
                Assert(callback != nullptr);
                probeContainer.WriteField(offsetof(ProbeContainer, pAsyncHaltCallback), callback);
                scriptContext = remoteScriptContext->next;
            }
            Assert(callback != nullptr);
            
            AsyncBreakController* controller = debugManager.GetAsyncBreakController();
            RemoteAsyncBreakController remoteAsyncBreakContoller(reader, controller);
            remoteAsyncBreakContoller.WriteField(offsetof(AsyncBreakController, haltCallback), callback);

            if (DIAG_CONFIG_FLAG(EnableJitInDiagMode) && DIAG_CONFIG_FLAG(EnableJitInHybridDebugging))
            {
                // For JIT mode simulate threadContext->GetDebuggingFlags->SetForceInterpreter(true).
                RemoteDebuggingFlags debuggingFlags(reader, debugManager.GetDebuggingFlags());
                debuggingFlags.SetForceInterpreter(true);
                // TODO: HybridJit: how do we test for this? Web workers?
            }

            return S_OK;
        });
    }

    STDMETHODIMP JsDebugProcess::GetExternalStepAddress(
        /* [out] */ __RPC__out UINT64 * pCodeAddress)
    {
        return JsDebugApiWrapper([=]
        {
            *pCodeAddress = reinterpret_cast<UINT64>(
                m_debugClient->GetGlobalPointer<void>(Globals::Globals_ExternalStepAddress));
            return S_OK;
        });
    }

    /* internal API */
    STDMETHODIMP JsDebugProcess::InspectVar(VOID* var, IJsDebugProperty** ppDebugProperty)
    {
        return JsDebugApiWrapper([=]
        {
            CComPtr<InspectionContext> inspectionContext;
            CreateComObject(this, &inspectionContext);
            CString name;
            name.Format(L"0x%p", var);
            inspectionContext->CreateDebugProperty(PROPERTY_INFO(name, var), /*parent*/nullptr, ppDebugProperty);
            return S_OK;
        });
    }
}
