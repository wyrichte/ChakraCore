#include "stdafx.h"
#include "pinned.h"


void EnterPinnedScope(volatile void** var)
{
    UNREFERENCED_PARAMETER(var);    
    return;
}

void LeavePinnedScope()
{
    return;
}
