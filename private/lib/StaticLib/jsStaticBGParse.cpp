//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    // See jsStaticBGParseExportWrapper.cpp for the definition of this function
    // HRESULT BGParse::QueueBackgroundParse(LPCSTR pszSrc, size_t cbLength, LPCWSTR fullPath, DWORD* dwBgParseCookie)

    // ExecuteBackgroundParse is a static function that executes the results of a background parse. The cookie passed in
    // should correspond to what was returned from a previous call to QueueBackgroundParse.
    // Note: This function must be called from UI (or, script-executing) thread from the provided activeScript parameter
    HRESULT BGParse::ExecuteBackgroundParse(DWORD dwBgParseCookie, IActiveScriptDirect* activeScriptDirect, DWORD_PTR dwSourceContext, DWORD dwFlags, EXCEPINFO* pexcepinfo)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->ExecuteBackgroundParse(dwBgParseCookie, dwSourceContext, dwFlags, pexcepinfo);
    }
}