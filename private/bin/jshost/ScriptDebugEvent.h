//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#include "activdbg100.h"
#include "activdbg.h"
#define SCRIPT_DEBUGGER_EXCEPTION_CODE ((DWORD)0x80020000L)

struct TARGET_STRING
{
    UINT64 Address;
    DWORD Length;
};


enum ScriptDebugEventType {
    ET_Invalid,
    ET_Breakpoint,
    ET_InsertDocumentText
};


struct ScriptDebugEvent {
    ScriptDebugEventType m_type;
    UINT64 m_applicationId;
    DWORD m_scriptThreadId;
    SCRIPT_DEBUGGER_OPTIONS m_scriptDebuggerOptions;
    BOOL                    m_scriptDebuggerOptionsValue;

    union {
        struct BREAKPOINT
        {
            BREAKREASON reason;
            BREAKRESUMEACTION breakResumeAction;
            ERRORRESUMEACTION errorResumeAction;
            TARGET_STRING ExceptionSource;      // EXCEPINFO bstrSource
            TARGET_STRING ExceptionDescription; // EXCEPINFO bstrDescription
            SCODE scode;
            SCRIPT_ERROR_DEBUG_EXCEPTION_THROWN_KIND ExceptionKind;
        } Breakpoint;

        struct INSERT_DOCUMENT_TEXT {
            UINT64 DocumentId;
            TARGET_STRING Text;
            TARGET_STRING DocumentUrl;
        } InsertDocumentText;
    };
};