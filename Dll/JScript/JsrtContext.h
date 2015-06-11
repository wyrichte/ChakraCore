//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "JsrtRuntime.h"

class JsrtDummyScriptSite;
class JsrtActiveScriptDirectHost;

class JsrtContext sealed : public FinalizableObject
{
private:
    DEFINE_VTABLE_CTOR_NOBASE(JsrtContext);

public:
    static JsrtContext *New(JsrtRuntime * runtime);

    Js::ScriptContext * GetScriptContext() const { return this->scriptEngine->GetScriptContext(); }
    ScriptEngine * GetScriptEngine() const { return this->scriptEngine; }
    JsrtRuntime * GetRuntime() const { return this->runtime; }
    JsErrorCode ReserveWinRTNamespace(_In_z_ const wchar_t* nameSpace);
    JsErrorCode SetProjectionDelegateWrapper(_In_ IDelegateWrapper *delegateWrapper);

    static bool Initialize();
    static void Uninitialize();
    static JsrtContext * GetCurrent();
    static bool TrySetCurrent(JsrtContext * context);
    static bool Is(void * ref);

    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override;
    virtual void Mark(Recycler * recycler) override;

    bool SetDebugApplication(IDebugApplication *debugApplication);
    void ReleaseDebugApplication();

private:
    JsrtContext(JsrtRuntime * runtime);

    void InitSite(JsrtRuntime *runtime);

    static DWORD s_tlsSlot;
    ScriptEngine * scriptEngine;
    JsrtDummyScriptSite * scriptSite;
    JsrtRuntime * runtime;
    JsrtContext * previous;
    JsrtContext * next;
    IDelegateWrapper* projectionDelegateWrapper;
    bool hasProjectionHost;
};
