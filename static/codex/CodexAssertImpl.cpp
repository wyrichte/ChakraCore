#include "stdafx.h"

void
__stdcall CodexAssert(bool condition)
{
    ASSERT(condition);
}
