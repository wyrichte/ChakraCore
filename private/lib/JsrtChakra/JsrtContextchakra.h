//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class JsrtDummyScriptSite;
class JsrtContextChakra sealed : public JsrtContext
{
public:
    static JsrtContextChakra *New(JsrtRuntime * runtime);

    ScriptEngine * GetScriptEngine() const { return this->scriptEngine; }
    JsErrorCode ReserveWinRTNamespace(_In_z_ const char16* nameSpace);
    JsErrorCode SetProjectionDelegateWrapper(_In_ IDelegateWrapper *delegateWrapper);
    HRESULT EnsureProjectionHost();
    bool SetDebugApplication(IDebugApplication *debugApplication);
    void ReleaseDebugApplication();

    void OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo, CompileScriptException* compileException);
    virtual void Dispose(bool isShutdown) override;
    virtual void Finalize(bool isShutdown) override {};
private:
    DEFINE_VTABLE_CTOR(JsrtContextChakra, JsrtContext);
    JsrtContextChakra(JsrtRuntime * runtime);
    void InitSite(JsrtRuntime *runtime);

    ScriptEngine * scriptEngine;
    JsrtDummyScriptSite * scriptSite;
    IDelegateWrapper* projectionDelegateWrapper;
    bool hasProjectionHost;
};
