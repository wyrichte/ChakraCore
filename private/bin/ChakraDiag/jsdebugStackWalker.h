
//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
     class ATL_NO_VTABLE JsDebugStackWalker :
        public CComObjectRoot,
        public IJsDebugStackWalker
    {
    private:
        CComPtr<JsDebugProcess> m_debugProcess;
        AutoPtr<RemoteStackWalker> m_stackWalker;
        CComPtr<InspectionContext> m_inspectionContext;
        CComPtr<RemoteStackFrame> m_nextFrame;
        void* m_previousEffectiveFrameBase;

        DECLARE_NOT_AGGREGATABLE(JsDebugStackWalker)
        BEGIN_COM_MAP(JsDebugStackWalker)
           COM_INTERFACE_ENTRY(IJsDebugStackWalker)
        END_COM_MAP()

        JsDebugStackWalker();
        ~JsDebugStackWalker();

    public:
        void Init(JsDebugProcess* process, DWORD threadId);

        virtual STDMETHODIMP GetNext(
            /* [out] */ IJsDebugFrame **ppFrame);
    };

}
