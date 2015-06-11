//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description: Breakpoint Diagnostic Probe

#include "StdAfx.h"

BreakpointProbe::BreakpointProbe(CScriptBody* _pScript, Js::StatementLocation &statement)
    : pScript(_pScript),
    pBody(statement.function),
    characterOffset(statement.statement.begin),
    byteOffset(statement.bytecodeSpan.begin)
{
}

bool BreakpointProbe::Install(Js::ScriptContext* pScriptContext)
{
    Assert(this->pBody);
    return pBody->InstallProbe(byteOffset);
}

bool BreakpointProbe::Uninstall(Js::ScriptContext* pScriptContext)
{
    Assert(this->pBody);

    if (this->pBody)
    {
        Assert(this->pScript);
        this->pScript->RemoveBreakpointProbe(this);

        return pBody->UninstallProbe(byteOffset);
    }

    // Already been uninstalled...
    return true;
}

bool BreakpointProbe::CanHalt(Js::InterpreterHaltState* pHaltState)
{
    Assert(this->pBody);

    Js::FunctionBody* pCurrentFuncBody = pHaltState->GetFunction();
    int offset = pHaltState->GetCurrentOffset();

    if (pBody == pCurrentFuncBody && byteOffset == offset)
    {
        return true;
    }
    return false;
}

void BreakpointProbe::DispatchHalt(Js::InterpreterHaltState* pHaltState)
{
    Assert(false);
}

void BreakpointProbe::CleanupHalt()
{
    Assert(this->pBody);

    // Nothing to clean here
}

bool BreakpointProbe::Matches(Js::FunctionBody* _pBody, int _characterOffset)
{
    Assert(this->pBody);
    return _pBody == pBody && _characterOffset == characterOffset;
}

