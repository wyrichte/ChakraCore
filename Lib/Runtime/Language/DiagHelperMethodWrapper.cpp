//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    AutoRegisterIgnoreExceptionWrapper::AutoRegisterIgnoreExceptionWrapper(ThreadContext* threadContext) :
        m_threadContext(threadContext)
    { 
        AssertMsg(!IsRegistered(threadContext), "BuiltInWrapper is already registered.");
        m_threadContext->GetDebuggingFlags()->SetIsBuiltInWrapperPresent(true); 
    }

    AutoRegisterIgnoreExceptionWrapper::~AutoRegisterIgnoreExceptionWrapper() 
    { 
        m_threadContext->GetDebuggingFlags()->SetIsBuiltInWrapperPresent(false); 
    }

    // static
    bool AutoRegisterIgnoreExceptionWrapper::IsRegistered(ThreadContext* threadContext)
    {
        return threadContext->GetDebuggingFlags()->IsBuiltInWrapperPresent();
    }

    // These are wrappers for helpers that can throw non-OOM / non-SO exceptions.
    // Under debugger, if "continue after exception" is on, we catch the exception and bail out to next statement.
    
    // IMPORTANT note: 
    // - we are taking advantage of stack alignment, that's why we can say all args have size not greater than sizeof(Var),
    //   for args that have less size, stack will be aligned, and next arg will start from alignment position, 
    //   while we can take the value of current arg at current position and ignore remaining bytes used for alignment.
    // - all these wrappers expect that arguments are not float/double 
    //   (double takes 8 bytes != stack alignment on x86 and ARM, double and float use different registers (VFP) rather than Var on ARM).

    typedef Var (__stdcall *OrigHelperMethod0)();
    typedef Var (__stdcall *OrigHelperMethod1)(Var arg1);
    typedef Var (__stdcall *OrigHelperMethod2)(Var arg1, Var arg2);
    typedef Var (__stdcall *OrigHelperMethod3)(Var arg1, Var arg2, Var arg3);
    typedef Var (__stdcall *OrigHelperMethod4)(Var arg1, Var arg2, Var arg3, Var arg4);
    typedef Var (__stdcall *OrigHelperMethod5)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5);
    typedef Var (__stdcall *OrigHelperMethod6)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6);
    typedef Var (__stdcall *OrigHelperMethod7)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7);
    typedef Var (__stdcall *OrigHelperMethod8)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8);
    typedef Var (__stdcall *OrigHelperMethod9)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9);
    typedef Var (__stdcall *OrigHelperMethod10)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10);
    typedef Var (__stdcall *OrigHelperMethod11)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11);
    typedef Var (__stdcall *OrigHelperMethod12)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12);
    typedef Var (__stdcall *OrigHelperMethod13)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13);
    typedef Var (__stdcall *OrigHelperMethod14)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14);
    typedef Var (__stdcall *OrigHelperMethod15)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14, Var arg15);
    typedef Var (__stdcall *OrigHelperMethod16)(Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14, Var arg15, Var arg16);

    template<typename Fn>
    Var HelperMethodWrapper(ScriptContext* scriptContext, Fn fn)
    {
        if (AutoRegisterIgnoreExceptionWrapper::IsRegistered(scriptContext->GetThreadContext()))
        {
            return fn();
        }
        else
        {
            AutoRegisterIgnoreExceptionWrapper autoWrapper(scriptContext->GetThreadContext());
            return HelperOrLibraryMethodWrapper<false>(scriptContext, fn);
        }
    }

    Var HelperMethodWrapper0(ScriptContext* scriptContext, void* origHelperAddr)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod0)origHelperAddr)();
        });
    }

    Var HelperMethodWrapper1(ScriptContext* scriptContext, void* origHelperAddr, Var arg1)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod1)origHelperAddr)(arg1);
        });
    }

    Var HelperMethodWrapper2(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod2)origHelperAddr)(arg1, arg2);
        });
    }

    Var HelperMethodWrapper3(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod3)origHelperAddr)(arg1, arg2, arg3);
        });
    }

    Var HelperMethodWrapper4(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod4)origHelperAddr)(arg1, arg2, arg3, arg4);
        });
    }

    Var HelperMethodWrapper5(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod5)origHelperAddr)(arg1, arg2, arg3, arg4, arg5);
        });
    }

    Var HelperMethodWrapper6(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod6)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6);
        });
    }

    Var HelperMethodWrapper7(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod7)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        });
    }

    Var HelperMethodWrapper8(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod8)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        });
    }

    Var HelperMethodWrapper9(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod9)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        });
    }

    Var HelperMethodWrapper10(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod10)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
        });
    }

    Var HelperMethodWrapper11(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod11)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
        });
    }

    Var HelperMethodWrapper12(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod12)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12);
        });
    }

    Var HelperMethodWrapper13(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod13)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
        });
    }

    Var HelperMethodWrapper14(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod14)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14);
        });
    }

    Var HelperMethodWrapper15(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14, Var arg15)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod15)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15);
        });
    }

    Var HelperMethodWrapper16(ScriptContext* scriptContext, void* origHelperAddr, Var arg1, Var arg2, Var arg3, Var arg4, Var arg5, Var arg6, Var arg7, Var arg8, Var arg9, Var arg10, Var arg11, Var arg12, Var arg13, Var arg14, Var arg15, Var arg16)
    {
        Assert(origHelperAddr);
        return HelperMethodWrapper(scriptContext, [=] {
            return ((OrigHelperMethod16)origHelperAddr)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16);
        });
    }

} // namespace Js
