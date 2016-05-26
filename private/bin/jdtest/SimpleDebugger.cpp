//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#include <initguid.h>
#include "guids.h"

SimpleDebugger::SimpleDebugger()
    : m_quiet(false)
{
}

SimpleDebugger::~SimpleDebugger()
{
}

HRESULT SimpleDebugger::Initialize(const string& initialCommand)
{
    HRESULT hr = S_OK;

    try
    {
        m_initialCommand = initialCommand;
    }
    catch(const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

void SimpleDebugger::Create(const string& initialCommand, _Out_ SimpleDebugger** dbg)
{
    UT_USE_DUMMY_TEST_GROUP();

    UT_COM_SUCCEEDED(CreateComObject(dbg));
    UT_COM_SUCCEEDED((*dbg)->Initialize(initialCommand));
}

void SimpleDebugger::DebugLaunch(_In_ LPTSTR pCmdLine)
{
    UT_COM_SUCCEEDED(DebugCreate(IID_PPV_ARGS(&m_client)));

    UT_COM_SUCCEEDED(m_client->SetEventCallbacksT(this));
    UT_COM_SUCCEEDED(m_client->CreateProcessT(0, pCmdLine, DEBUG_ONLY_THIS_PROCESS));
    UT_COM_SUCCEEDED(m_client.QueryInterface(&m_control));
    UT_COM_SUCCEEDED(m_client.QueryInterface(&m_debugDataSpaces));
    RunDebugLoop();
    // Since we created the process, make sure we kill the process too.
    UT_COM_SUCCEEDED(m_client->TerminateProcesses());
    m_client->EndSession(DEBUG_END_PASSIVE); // Cleanup session (and extensions)
}

void SimpleDebugger::RunDebugLoop()
{
    HRESULT hr = S_OK;

    // Enable initial breakpoint
    {
        DEBUG_SPECIFIC_FILTER_PARAMETERS initbp = {DEBUG_FILTER_BREAK, DEBUG_FILTER_GO_NOT_HANDLED};
        UT_COM_SUCCEEDED(m_control->SetSpecificFilterParameters(DEBUG_FILTER_INITIAL_BREAKPOINT, 1, &initbp));
    }

    bool initialBreak = true;

    UT_COM_SUCCEEDED(m_client->SetOutputCallbacksT(this));
    UT_COM_SUCCEEDED(m_client->SetInputCallbacks(this));

    while (true)
    {
        hr = m_control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if (hr != S_OK)
        {
            break;
        }

        _TCHAR cmd[MAX_PATH];
        cmd[0] = _T('\0');

        if (initialBreak)
        {
            if(m_verbose)
            {
                ULONG mask;
                UT_COM_SUCCEEDED(m_client->GetOutputMask(&mask));
                UT_COM_SUCCEEDED(m_client->SetOutputMask(mask | DEBUG_OUTPUT_VERBOSE));
            }

            initialBreak = false;
            if (!m_initialCommand.empty())
            {
                _tcscpy_s(cmd, m_initialCommand.c_str());
            }
        }

        ULONG status = DEBUG_STATUS_BREAK;
        while (status == DEBUG_STATUS_BREAK)
        {
            if (cmd[0] == _T('\0'))
            {
                m_control->OutputPromptT(DEBUG_OUTCTL_THIS_CLIENT | DEBUG_OUTCTL_NOT_LOGGED, _T(" "));

                ULONG size;
                m_control->InputT(cmd, _countof(cmd), &size);
            }

            UT_COM_SUCCEEDED(m_control->ExecuteT(DEBUG_OUTCTL_ALL_CLIENTS, cmd, DEBUG_EXECUTE_DEFAULT));
            cmd[0] = _T('\0');

            if (m_control->GetExecutionStatus(&status) != S_OK)
            {
                status = DEBUG_STATUS_NO_DEBUGGEE;
            }
        }
    }

    m_client->FlushCallbacks();
    UT_COM_SUCCEEDED(m_client->SetInputCallbacks(NULL));
    UT_COM_SUCCEEDED(m_client->SetOutputCallbacksT(NULL));
    UT_COM_SUCCEEDED(m_client->SetEventCallbacksT(NULL));
}

STDMETHODIMP SimpleDebugger::GetInterestMask(_Out_ PULONG Mask)
{
    *Mask = 0x0;
    return S_OK;
}


STDMETHODIMP SimpleDebugger::Exception (
        _In_ PEXCEPTION_RECORD64 Exception,
        _In_ ULONG FirstChance
        ) 
{
    return DEBUG_STATUS_GO_NOT_HANDLED;
}

STDMETHODIMP SimpleDebugger::Breakpoint (
        _In_ PDEBUG_BREAKPOINT2 Bp
        )
{
    return S_OK;
}

STDMETHODIMP SimpleDebugger::LoadModule(
        _In_ ULONG64 ImageFileHandle,
        _In_ ULONG64 BaseOffset,
        _In_ ULONG ModuleSize,
        _In_opt_ PCWSTR ModuleName,
        _In_opt_ PCWSTR ImageName,
        _In_ ULONG CheckSum,
        _In_ ULONG TimeDateStamp
        )
{
    return NOERROR;
}

STDMETHODIMP SimpleDebugger::StartInput(_In_ ULONG BufferSize)
{
    UNREFERENCED_PARAMETER(BufferSize);

    _TCHAR cmd[MAX_PATH];
    size_t size;
    _cgetts_s(cmd, _countof(cmd), &size);

    m_control->ReturnInputT(cmd);
    return S_OK;
}

STDMETHODIMP SimpleDebugger::EndInput()
{
    return S_OK;
}

STDMETHODIMP SimpleDebugger::Output(_In_ ULONG Mask, _In_ PCTSTR Text)
{
    UNREFERENCED_PARAMETER(Mask);

    // In "quiet" mode, only output text prefixed with $ut$
    const int PREFIXLEN = 4;
    if (m_quiet)
    {
        if (_tcsncmp(Text, _T("$ut$"), PREFIXLEN) == 0)
        {
            Text += PREFIXLEN;
        }
        else
        {
            return S_OK;
        }
    }

    _tprintf_s(_T("%s"), Text);
    return S_OK;
}

void SimpleDebugger::Out(_In_ bool unitTest, _In_ PCTSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if(unitTest)
    {
        std::wstring s = std::wstring(_u("$ut$")) + fmt;
        fmt = s.c_str();
    }

    m_control->OutputVaListWide(DEBUG_OUTPUT_NORMAL, fmt, args);

    va_end(args);
}

USE_DbgEngDataTarget(); // Use DbgEngDataTarget implementation
