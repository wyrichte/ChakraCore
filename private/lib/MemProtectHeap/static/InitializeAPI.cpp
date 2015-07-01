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
    JS_ETW(EtwTraceCore::Register());
}

void
__stdcall MemProtectHeapProcessDetach()
{
    JS_ETW(EtwTraceCore::UnRegister());
}
