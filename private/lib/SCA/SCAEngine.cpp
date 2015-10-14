//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "SCAPch.h"
#include "Debug\ProbeContainer.h"
#include "Debug\DebugContext.h"

#include "hostdispatch.h"

namespace Js
{
    void ScriptContextHolder::ThrowIfFailed(HRESULT hr) const
    {
        if (FAILED(hr))
        {
            if (GetScriptContext()->IsInDebugMode())
            {
                JavascriptExceptionObject *exception = nullptr;
                try
                {
                    GetScriptContext()->GetDebugContext()->GetProbeContainer()->SetThrowIsInternal(true);
                    HostDispatch::HandleDispatchError(GetScriptContext(), hr, nullptr);
                }
                catch (JavascriptExceptionObject *exception_)
                {
                    Assert(exception_);
                    GetScriptContext()->GetDebugContext()->GetProbeContainer()->SetThrowIsInternal(false);
                    exception = exception_;
                }

                if (exception)
                {
                    // Rethrow the exception again.
                    throw exception;
                }
            }
            else
            {
                HostDispatch::HandleDispatchError(GetScriptContext(), hr, nullptr);
            }
        }
    }
};