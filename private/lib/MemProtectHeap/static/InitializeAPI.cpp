#include "stdafx.h"

class EtwTrace
{
public:
    static void Register();
    static void UnRegister();
};

void
__stdcall MemProtectHeapProcessAttach()
{
    EtwTraceCore::Register();
}

void
__stdcall MemProtectHeapProcessDetach()
{
    EtwTraceCore::UnRegister();
}
