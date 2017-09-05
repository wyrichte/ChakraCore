#include "StdAfx.h"

// The Codex library requires this assertion.
void CodexAssert(bool condition)
{
    Assert(condition);
}

void CodexAssertOrFailFast(bool condition)
{
    Assert(condition);
    if (!condition)
    {
        TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
    }
}
