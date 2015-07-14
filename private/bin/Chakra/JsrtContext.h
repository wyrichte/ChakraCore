//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#include "JsrtRuntime.h"

class JsrtContext : public FinalizableObject
{
public:
    static JsrtContext *New(JsrtRuntime * runtime);

    Js::ScriptContext * GetScriptContext() const { return this->scriptContext; }
    JsrtRuntime * GetRuntime() const { return this->runtime; }

    static bool Initialize();
    static void Uninitialize();
    static JsrtContext * GetCurrent();
    static bool TrySetCurrent(JsrtContext * context);
    static bool Is(void * ref);

    virtual void Finalize(bool isShutdown) override sealed;    
    virtual void Mark(Recycler * recycler) override sealed;

    void OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo);
protected:
    DEFINE_VTABLE_CTOR_NOBASE(JsrtContext);
    JsrtContext(JsrtRuntime * runtime);
    void Link();
    void Unlink();
    void SetScriptContext(Js::ScriptContext * scriptContext);
private:
    static DWORD s_tlsSlot;
    Js::ScriptContext * scriptContext;
    JsrtRuntime * runtime;
    JsrtContext * previous;
    JsrtContext * next;
};

