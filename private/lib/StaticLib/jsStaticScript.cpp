//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    HRESULT Script::Execute(IActiveScriptDirect* activeScriptDirect,
        ScriptContents* contents,
        ScriptExecuteMetadata* metadata,
        VARIANT*  pvarResult,
        EXCEPINFO* pexcepinfo)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->ExecuteScript(contents, metadata, pvarResult, pexcepinfo);
    }

    HRESULT Script::GenerateByteCodeBuffer(
        IActiveScriptDirect* activeScriptDirect,
        DWORD dwSourceCodeLength,
        BYTE *utf8Code,
        IUnknown *punkContext,
        DWORD_PTR dwSourceContext,
        EXCEPINFO *pexcepinfo,
        DWORD     dwFlags,
        BYTE **byteCode,
        DWORD *pdwByteCodeSize)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->GenerateByteCodeBufferCommon(dwSourceCodeLength, utf8Code, punkContext, dwSourceContext, pexcepinfo, dwFlags, byteCode, pdwByteCodeSize);
    }

    HRESULT Script::ExecuteByteCodeBuffer(
        IActiveScriptDirect* activeScriptDirect,
        DWORD dwByteCodeSize,
        BYTE *byteCode,
        IActiveScriptByteCodeSource *pbyteCodeSource,
        IUnknown *punkContext,
        DWORD_PTR dwSourceContext,
        DWORD     dwFlags,
        EXCEPINFO *pexcepinfo,
        VARIANT * pvarResult)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->ExecuteByteCodeBufferCommon(dwByteCodeSize, byteCode, pbyteCodeSource, punkContext, dwSourceContext, dwFlags, pexcepinfo, pvarResult);
    }
}