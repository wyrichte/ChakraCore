//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    // Set in ctor/clear in dtor the GetDebuggingFlags()->SetIsBuiltInWrapperPresent.
    struct AutoRegisterIgnoreExceptionWrapper
    {
        AutoRegisterIgnoreExceptionWrapper(ThreadContext* threadContext);
        ~AutoRegisterIgnoreExceptionWrapper();
        static bool IsRegistered(ThreadContext* threadContext);
    private:
        ThreadContext* m_threadContext;
    };

    // Note: we don't need to take special for orig helpers that return void, 
    // as when we call we assume that EAX is trashed, so returning Var is fine.
    Var HelperMethodWrapper0(ScriptContext* scriptContext, void* origHelperAddr);
    Var HelperMethodWrapper1(ScriptContext* scriptContext, void* origHelperAddr, Var arg1);
    Var HelperMethodWrapper2(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2);
    Var HelperMethodWrapper3(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3);
    Var HelperMethodWrapper4(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4);
    Var HelperMethodWrapper5(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5);
    Var HelperMethodWrapper6(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6);
    Var HelperMethodWrapper7(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7);
    Var HelperMethodWrapper8(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8);
    Var HelperMethodWrapper9(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9);
    Var HelperMethodWrapper10(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10);
    Var HelperMethodWrapper11(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11);
    Var HelperMethodWrapper12(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12);
    Var HelperMethodWrapper13(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13);
    Var HelperMethodWrapper14(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14);
    Var HelperMethodWrapper15(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14, Var arg15);
    Var HelperMethodWrapper16(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14, Var arg15, Var arg16);

    template <bool doCheckParentInterpreterFrame, typename Fn>
    Var HelperOrLibraryMethodWrapper(ScriptContext* scriptContext, Fn fn);
}
