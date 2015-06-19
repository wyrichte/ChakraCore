//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description: Breakpoint Diagnostic Probe


#pragma once

class CScriptBody;

class BreakpointProbe : public Js::Probe
{
    int characterOffset;
    int byteOffset;
    CScriptBody* pScript;

    // Use raw non-pinning "FunctionBody*" here. We can NOT pin/unpin from debugger thread.
    // Fortunately, pBody is guaranteed alive. This BreakpointProbe is owned and kept alive by
    // CScriptBody. CScriptBody pins the root functionBody (on script engine thread). The
    // root functionBody references utf8SourceInfo, and utf8SourceInfo references this pBody.
    // As long as owner CScriptBody keeps this BreakpointProbe alive, this pBody is alive.
    Js::FunctionBody* pBody;

public:
    BreakpointProbe(CScriptBody* pScript, Js::StatementLocation &statement);

    virtual bool Install(Js::ScriptContext* pScriptContext);
    virtual bool Uninstall(Js::ScriptContext* pScriptContext);
    virtual bool CanHalt(Js::InterpreterHaltState* pHaltState);
    virtual void DispatchHalt(Js::InterpreterHaltState* pHaltState);
    virtual void CleanupHalt();

    bool Matches(Js::FunctionBody* _pBody, int characterPosition);
};

typedef JsUtil::List<BreakpointProbe*, ArenaAllocator> BreakpointProbeList;
