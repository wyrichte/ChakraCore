//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace JsDiag
{
     class ATL_NO_VTABLE JsDebugBreakPoint :
        public CComObjectRoot,
        public IJsDebugBreakPoint
    {
    public:
        void Init(JsDebugProcess* process, UINT64 documentId, DWORD characterOffset, DWORD characterCount, bool isEnabled);

        DECLARE_NOT_AGGREGATABLE(JsDebugBreakPoint)  
        BEGIN_COM_MAP(JsDebugBreakPoint)
           COM_INTERFACE_ENTRY(IJsDebugBreakPoint)
        END_COM_MAP()
        
        STDMETHODIMP Delete();
        
        STDMETHODIMP GetDocumentPosition( 
            /* [out] */ UINT64 *pDocumentId,
            /* [out] */ DWORD *pCharacterOffset,
            /* [out] */ DWORD *pStatementCharCount);

        STDMETHODIMP IsEnabled( 
            /* [out] */ BOOL *pIsEnabled);
        
        STDMETHODIMP Enable();
        
        STDMETHODIMP Disable();

    private:
        void BindBreakPoint(const RemoteUtf8SourceInfo& utf8SourceInfo, DWORD characterOffset, DWORD characterCount);
        void SetByteCodeBreakPoint(DWORD bytecodeOffset);
        JsDebugProcess* m_process;
        FunctionBody* m_functionBody;
        UINT64 m_documentId;
        DWORD m_byteCodeOffset;
        DWORD m_characterOffset;
        DWORD m_characterCount;
        bool m_isEnabled;
    };
}