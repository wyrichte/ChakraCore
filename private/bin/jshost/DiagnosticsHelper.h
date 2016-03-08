/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include "edgescriptdirect.h"

class Debugger;

class WScriptDispatchCallbackMessage : public MessageBase
{
private:
    CComPtr<IDispatch> m_function; // Keeps a ref of the JavascriptDispatch in this message

public:
    WScriptDispatchCallbackMessage(unsigned int time, IDispatch* function) : MessageBase(time)
    {
        m_function = function;
    }

    HRESULT CallJavascriptFunction(bool force = false);
    typedef IDispatch* CustomArgType;

    template <class Func>
    static WScriptDispatchCallbackMessage* Create(IDispatch* function, const Func& func, unsigned int time = 0)
    {
        return new CustomMessage<Func, WScriptDispatchCallbackMessage>(time, function, func);
    }
};

// A common debugger and profiler helper class.
class DiagnosticsHelper 
{
private:
    DiagnosticsHelper();
    HRESULT EnableHtmlDebugging();
    HRESULT InitializeDebugManager();
    bool IsHtmlHost() const;

public:
    HRESULT HtmlDynamicAttach(DWORD cmdId);
    HRESULT HtmlDynamicDetach();

    void ReleaseDiagnosticsHelper(bool forceNull);
    HRESULT CreateDocumentHelper(__in IDebugDocumentHelper ** debugDocumentHelper);
    HRESULT InitializeDebugging(bool canSetBreakpoints = false);
    HRESULT AttachToDebugger();
    void SetHtmlDocument(IHTMLDocument2 *htmlDocument);
    bool IsHostInDebugMode() const;
    static DiagnosticsHelper* GetDiagnosticsHelper();
    static void DisposeHelper(bool forceNull = false);

    Debugger                        *m_debugger; // The test debugger.
    HINSTANCE                       m_hInstPdm;
    CComPtr<IProcessDebugManager>   m_processDebugManager;
    CComPtr<IDebugApplication>      m_debugApplication;
    CComPtr<IHTMLDocument2>         m_htmlDocument;
    DWORD                           m_debugAppCookie;

    bool                            m_shouldPerformSourceRundown;

    // Editing states
    bool AddEditRangeAndContent(const char16* editLabel, IDebugDocumentText* debugDocumentText, ULONG startOffset, ULONG length, PCWSTR editContent, ULONG newLength);
    bool GetEditRangeAndContent(const char16* editLabel, IDebugDocumentText** ppDebugDocumentText, ULONG* startOffset, ULONG* length, PCWSTR* editContent, ULONG* newLength);

    struct EditRangeAndContent
    {
    public:
        EditRangeAndContent(IDebugDocumentText* debugDocumentText, ULONG m_startOffset, ULONG m_length, const char16* editContent, ULONG newLength);

        CComPtr<IDebugDocumentText> debugDocumentText;
        ULONG m_startOffset;
        ULONG m_length;
        std::wstring m_content;
    };

    typedef std::map<std::wstring, EditRangeAndContent> EditMapType;
    EditMapType m_editRangeAndContents;
    
    static DiagnosticsHelper *      s_diagnosticsHelper;
};