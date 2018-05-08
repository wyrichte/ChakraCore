//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// Note: JsStaticAPI::BGParse::QueueBackgroundParse is defined in this file separately from the rest of the class
// because the function can be invoked on any thread (and, thus, doesn't have a hard dependency on a ScriptEngine).
// This call currently wraps an export function because statically linking with this export function causes jshost
// to statically link to chakra at runtime. This causes the dll to be loaded at process launch (rather than at
// LoadLibrary) and changes the timing of various globals that prevents files from launching.
//
// When jshost's dependencies are cleaned up, this file can be merged with the rest of BGParse.

#include "StaticLibPch.h"

extern HRESULT JsQueueBackgroundParse(JsStaticAPI::ScriptContents* contents, DWORD* dwBgParseCookie);

namespace JsStaticAPI
{
    // QueueBackgroundParse is a static function that queues up parsing on a background thread. More details
    // at the declaration of BGParseManager.
    // Note:
    // - This function can be called from any thread
    // - This function can only take UTF8 source
    // - The host must not be in debug mode
    HRESULT BGParse::QueueBackgroundParse(ScriptContents* contents, DWORD* dwBgParseCookie)
    {
        return JsQueueBackgroundParse(contents, dwBgParseCookie);
    }
}