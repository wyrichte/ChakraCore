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
            if (GetScriptContext()->IsScriptContextInDebugMode())
            {
                try
                {
                    GetScriptContext()->GetDebugContext()->GetProbeContainer()->SetThrowIsInternal(true);
                    HostDispatch::HandleDispatchError(GetScriptContext(), hr, nullptr);
                }
                catch (const Js::JavascriptException&)
                {
                    GetScriptContext()->GetDebugContext()->GetProbeContainer()->SetThrowIsInternal(false);

                    // Rethrow the exception again.
                    throw;
                }
            }
            else
            {
                HostDispatch::HandleDispatchError(GetScriptContext(), hr, nullptr);
            }
        }
    }
};