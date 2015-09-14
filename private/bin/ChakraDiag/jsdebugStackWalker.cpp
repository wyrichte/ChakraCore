//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace JsDiag
{
    JsDebugStackWalker::JsDebugStackWalker():
        m_previousEffectiveFrameBase(nullptr)
    {
    }

    JsDebugStackWalker::~JsDebugStackWalker()
    {
    }

    void JsDebugStackWalker::Init(JsDebugProcess* process, DWORD threadId)
    {
        m_debugProcess = process;
        CreateComObject(process, &m_inspectionContext);

        m_stackWalker = new(oomthrow) RemoteStackWalker(m_debugProcess->GetDebugClient(), threadId, /*threadContextAddr*/nullptr, /*walkInternalFrame*/false, m_debugProcess->DoDebugModeValidation());
    }

    //
    // Here we get the next frame and cache it for the scenario when the frame
    // is the bottom most runtime frame with a native->script transition, we will
    // get the correct value for 'start' & returnaddress pointing to the the entry frame 
    // which is typically ScriptEngine::CallRootFunction.
    // 
    STDMETHODIMP JsDebugStackWalker::GetNext(
        /* [out] */ IJsDebugFrame **ppFrame)
    {
        return JsDebugApiWrapper([=]
        {
            RemoteStackFrame* remoteFrame = nullptr;

            if(!m_nextFrame.p)
            {
                if(m_stackWalker->WalkToNextJavascriptFrame())
                {
                    m_stackWalker->GetCurrentJavascriptFrame(&m_nextFrame);
                    m_nextFrame->SetEnd(max(m_previousEffectiveFrameBase, m_stackWalker->GetCurrentScriptExitFrameBase()));
                    m_previousEffectiveFrameBase = m_nextFrame->GetEffectiveFrameBase();
                }
            }
            
            if(m_nextFrame.p)
            {
                remoteFrame = m_nextFrame.Detach();
                if(m_stackWalker->WalkToNextJavascriptFrame())
                {
                    m_stackWalker->GetCurrentJavascriptFrame(&m_nextFrame);
                    m_nextFrame->SetEnd(max(m_previousEffectiveFrameBase, m_stackWalker->GetCurrentScriptExitFrameBase()));
                    m_previousEffectiveFrameBase = m_nextFrame->GetEffectiveFrameBase();
                }
            }

            if(remoteFrame)
            {
                remoteFrame->SetInspectionContext(m_inspectionContext);
                // Set 'start' only after getting the next frame
                if(remoteFrame->GetEffectiveFrameBase() > m_stackWalker->GetCurrentScriptEntryFrameBase())
                {
                    remoteFrame->SetStart(remoteFrame->GetEffectiveFrameBase());
                }
                else
                {
                    remoteFrame->SetStart(m_stackWalker->GetCurrentScriptEntryFrameBase());
                    remoteFrame->SetReturnAddress(m_stackWalker->GetCurrentScriptEntryReturnAddress());
                }
                *ppFrame = remoteFrame;
                return S_OK;
            }
            Assert(m_nextFrame == nullptr);
            m_previousEffectiveFrameBase = nullptr;
            return E_JsDEBUG_OUTSIDE_OF_VM;
        });
    }

}
