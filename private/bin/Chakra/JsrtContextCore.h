//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once

#include "JsrtRuntime.h"

class JsrtContextCore sealed : public JsrtContext
{
public:
    static JsrtContextCore *New(JsrtRuntime * runtime);
    virtual void Dispose(bool isShutdown) override;

    void OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo);
private:
    DEFINE_VTABLE_CTOR(JsrtContextCore, JsrtContext);
    JsrtContextCore(JsrtRuntime * runtime);
    Js::ScriptContext* EnsureScriptContext();
};
