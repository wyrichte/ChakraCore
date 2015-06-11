//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js {

    class Exception
    {
        friend JsUtil::ExternalApi;

    private:

        enum {
            ExceptionKind_OutOfMemory,
            ExceptionKind_StackOverflow
        };

        static bool RaiseIfScriptActive(ScriptContext *scriptContext, unsigned kind, PVOID returnAddress = NULL);

        static void RecoverUnusedMemory();
    };

}
