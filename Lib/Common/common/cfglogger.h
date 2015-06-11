//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#ifdef CONTROL_FLOW_GUARD_LOGGER
class CFGLogger
{
public:
    static void Enable();
    static void __fastcall GuardCheck(_In_ uintptr_t Target);
private:
    CFGLogger() {}
    ~CFGLogger();
    
    typedef void(__fastcall * PfnGuardCheckFunction)(_In_ uintptr_t Target);
    static PfnGuardCheckFunction oldGuardCheck;
    static bool inGuard;
    static CriticalSection cs;
    static JsUtil::BaseDictionary<uintptr_t, uint, NoCheckHeapAllocator> guardCheckRecord;
    static CFGLogger Instance;
};

#endif