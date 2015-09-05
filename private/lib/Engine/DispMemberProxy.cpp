/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"


///----------------------------------------------------------------------------
///
/// DispMemberProxy
/// Jn - IDispatch integration
///
///
///----------------------------------------------------------------------------

Js::Var DispMemberProxy::DefaultInvoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    RUNTIME_ARGUMENTS(args, callInfo);
    AssertMsg(args.Info.Count > 0, "bad this argument in Invoke");

    DispMemberProxy* _this = (DispMemberProxy*)function;
    Js::ScriptContext *pScriptContext = _this->GetScriptSite()->GetScriptSiteContext();
    // If the scriptsite is closed, the scriptcontext is reset to NULL. we need to detect this and reject the call
    // We have similar check in other HostDispatch calls already.
    if (NULL == pScriptContext)
    {
        _this->HandleDispatchError(E_ACCESSDENIED, NULL);
    }

    return _this->InvokeByDispId(args, _this->m_id);
}

Js::Var DispMemberProxy::ProfileInvoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    RUNTIME_ARGUMENTS(args, callInfo);
    AssertMsg(args.Info.Count > 0, "bad this argument in Invoke");

    DispMemberProxy* _this = (DispMemberProxy*)function;
    Js::ScriptContext *pScriptContext = _this->GetScriptSite()->GetScriptSiteContext();
    Assert(pScriptContext != NULL);

    pScriptContext->OnDispatchFunctionEnter(_this->m_pwszName);

    Js::Var returnValue = NULL;
    __try
    {
        returnValue = _this->InvokeByDispId(args, _this->m_id);
    }
    __finally
    {
        pScriptContext->OnDispatchFunctionExit(_this->m_pwszName);
    }
    return returnValue;
}

Js::RecyclableObject * DispMemberProxy::CloneToScriptContext(Js::ScriptContext* requestContext)
{    
    Recycler* recycler = requestContext->GetRecycler();
    
    DispMemberProxy* proxy = RecyclerNewFinalized(
        recycler,
        DispMemberProxy,        
        refCountedHostVariant,
        this->m_id,
        requestContext->GetLibrary()->GetDispMemberProxyType(),
        this->m_pwszName);
    return proxy;
}
