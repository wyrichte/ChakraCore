//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// This implements a simple debugger that allows actions at break points.
//
class ATL_NO_VTABLE SimpleDebugger:
    public CComObjectRoot,
    public DummyTestGroup,
    public IDebugInputCallbacks,
    public IDebugOutputCallbacksT,
    public IDebugEventCallbacksWide
{
private:
    CComPtr<IDebugClient5> m_client;
    CComPtr<IDebugControl4> m_control;
    CComPtr<IDebugDataSpaces4> m_debugDataSpaces;
    string m_initialCommand;
    bool m_quiet;
    bool m_verbose;

public:
    SimpleDebugger();
    ~SimpleDebugger();

    BEGIN_COM_MAP(SimpleDebugger)
        COM_INTERFACE_ENTRY(IDebugInputCallbacks)
        COM_INTERFACE_ENTRY(IDebugOutputCallbacksT)
        COM_INTERFACE_ENTRY(IDebugEventCallbacksWide)
    END_COM_MAP()

    // IDebugInputCallbacks
    STDMETHOD(StartInput)(_In_ ULONG BufferSize);
    STDMETHOD(EndInput)();

    // IDebugOutputCallbacks
    STDMETHOD(Output)(_In_ ULONG Mask, _In_ PCTSTR Text);
    void Out(_In_ bool unitTest, _In_ PCTSTR fmt, ...);

    HRESULT Initialize(const string& initialCommand);
    void SetQuiet(bool quiet) { m_quiet = quiet; }
    void SetVerbose(bool verbose) { m_verbose = verbose; }
    void DebugLaunch(_In_ LPTSTR pCmdLine);
    static void Create(const string& initialCommand, _Out_ SimpleDebugger** dbg);
    IDebugDataSpaces4* GetReader() { return m_debugDataSpaces; }
#pragma warning(disable:4100) //  unreferenced formal parameter
    // IDebugEventCallbacksWide.

    // The engine calls GetInterestMask once when
    // the event callbacks are set for a client.
    STDMETHOD(GetInterestMask)(
        _Out_ PULONG Mask
        ) ;

    // A breakpoint event is generated when
    // a breakpoint exception is received and
    // it can be mapped to an existing breakpoint.
    // The callback method is given a reference
    // to the breakpoint and should release it when
    // it is done with it.
    STDMETHOD(Breakpoint)(
        _In_ PDEBUG_BREAKPOINT2 Bp
        ) ;

    // Exceptions include breaks which cannot
    // be mapped to an existing breakpoint
    // instance.
    STDMETHOD(Exception)(
        _In_ PEXCEPTION_RECORD64 Exception,
        _In_ ULONG FirstChance
        ) ;

    // Any of these values can be zero if they
    // cannot be provided by the engine.
    // Currently the kernel does not return thread
    // or process change events.
    STDMETHOD(CreateThread)(
        _In_ ULONG64 Handle,
        _In_ ULONG64 DataOffset,
        _In_ ULONG64 StartOffset
        ) {  return E_NOTIMPL; } 
    _Analysis_noreturn_
    STDMETHOD(ExitThread)(
        _In_ ULONG ExitCode
        ) { return E_NOTIMPL; } 

    // Any of these values can be zero if they
    // cannot be provided by the engine.
    STDMETHOD(CreateProcess)(
        _In_ ULONG64 ImageFileHandle,
        _In_ ULONG64 Handle,
        _In_ ULONG64 BaseOffset,
        _In_ ULONG ModuleSize,
        _In_opt_ PCWSTR ModuleName,
        _In_opt_ PCWSTR ImageName,
        _In_ ULONG CheckSum,
        _In_ ULONG TimeDateStamp,
        _In_ ULONG64 InitialThreadHandle,
        _In_ ULONG64 ThreadDataOffset,
        _In_ ULONG64 StartOffset
        ) { return E_NOTIMPL; } 
    STDMETHOD(ExitProcess)(

        _In_ ULONG ExitCode
        ) { return E_NOTIMPL; } 

    // Any of these values may be zero.
    STDMETHOD(LoadModule)(

        _In_ ULONG64 ImageFileHandle,
        _In_ ULONG64 BaseOffset,
        _In_ ULONG ModuleSize,
        _In_opt_ PCWSTR ModuleName,
        _In_opt_ PCWSTR ImageName,
        _In_ ULONG CheckSum,
        _In_ ULONG TimeDateStamp
        );
    STDMETHOD(UnloadModule)(

        _In_opt_ PCWSTR ImageBaseName,
        _In_ ULONG64 BaseOffset
        ) { return E_NOTIMPL; } 

    STDMETHOD(SystemError)(

        _In_ ULONG Error,
        _In_ ULONG Level
        ) { return E_NOTIMPL; } 

    // Session status is synchronous like the other
    // wait callbacks but it is called as the state
    // of the session is changing rather than at
    // specific events so its return value does not
    // influence waiting.  Implementations should just
    // return DEBUG_STATUS_NO_CHANGE.
    // Also, because some of the status
    // notifications are very early or very
    // late in the session lifetime there may not be
    // current processes or threads when the notification
    // is generated.
    STDMETHOD(SessionStatus)(
        _In_ ULONG Status
        ) { return E_NOTIMPL; } 

    // The following callbacks are informational
    // callbacks notifying the provider about
    // changes in debug state.  The return value
    // of these callbacks is ignored.  Implementations
    // can not call back into the engine.

    // Debuggee state, such as registers or data spaces,
    // has changed.
    STDMETHOD(ChangeDebuggeeState)(
        _In_ ULONG Flags,
        _In_ ULONG64 Argument
        ) { return E_NOTIMPL; } 
    // Engine state has changed.
    STDMETHOD(ChangeEngineState)(
        _In_ ULONG Flags,
        _In_ ULONG64 Argument
        ) { return E_NOTIMPL; } 
    // Symbol state has changed.
    STDMETHOD(ChangeSymbolState)(
        _In_ ULONG Flags,
        _In_ ULONG64 Argument
        ) { return E_NOTIMPL; }

private:
    void RunDebugLoop();
};
