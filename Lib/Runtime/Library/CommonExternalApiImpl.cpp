//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Exception.h"


void JsUtil::ExternalApi::RecoverUnusedMemory()
{
    Js::Exception::RecoverUnusedMemory();
}

bool JsUtil::ExternalApi::RaiseOnIntOverflow()
{
    ::Math::DefaultOverflowPolicy();

    return true;
}


bool JsUtil::ExternalApi::RaiseOutOfMemoryIfScriptActive()
{
    return Js::Exception::RaiseIfScriptActive(null, Js::Exception::ExceptionKind_OutOfMemory);
}

bool JsUtil::ExternalApi::RaiseStackOverflowIfScriptActive(Js::ScriptContext * scriptContext, PVOID returnAddress)
{
    return Js::Exception::RaiseIfScriptActive(scriptContext, Js::Exception::ExceptionKind_StackOverflow, returnAddress);
}

ThreadContextId JsUtil::ExternalApi::GetCurrentThreadContextId()
{
    return ThreadContextTLSEntry::GetCurrentThreadContextId();
}

#if DBG || defined(EXCEPTION_CHECK)
BOOL JsUtil::ExternalApi::IsScriptActiveOnCurrentThreadContext()
{
    return ThreadContext::GetContextForCurrentThread() != null &&
        ThreadContext::GetContextForCurrentThread()->IsScriptActive();
}
#endif

