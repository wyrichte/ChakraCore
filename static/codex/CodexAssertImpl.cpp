#include "stdafx.h"

void
__stdcall CodexAssert(bool condition)
{
    ASSERT(condition);
}

void
__stdcall CodexAssertOrFailFast(bool condition)
{
    ASSERT(condition);
    RaiseFailFastException(NULL, NULL, 0);
}