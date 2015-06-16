//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once

#include "JsrtRuntime.h"

class JsrtContext sealed : public FinalizableObject
{
public:
    static JsrtContext *New(JsrtRuntime * runtime);

    Js::ScriptContext* GetScriptContext() const { return scriptContext; }
    JsrtRuntime * GetRuntime() const { return this->runtime; }

    static bool Initialize();
    static void Uninitialize();
    static JsrtContext * GetCurrent();
    static bool TrySetCurrent(JsrtContext * context);
    static bool Is(void * ref);

    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override;
    virtual void Mark(Recycler * recycler) override;

protected:

    JsrtContext(JsrtRuntime * runtime);
    DEFINE_VTABLE_CTOR_NOBASE(JsrtContext);

private:
    Js::ScriptContext* EnsureScriptContext();

    void InitSite(JsrtRuntime *runtime);

    static DWORD s_tlsSlot;
    Js::ScriptContext* scriptContext;
    JsrtRuntime * runtime;
    JsrtContext * previous;
    JsrtContext * next;
};


