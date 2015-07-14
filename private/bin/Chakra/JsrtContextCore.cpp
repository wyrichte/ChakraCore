//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <stdafx.h>
#include "jsrtprivate.h"
#include "jsrtcontextcore.h"

JsrtContext *JsrtContext::New(JsrtRuntime * runtime)
{
    return JsrtContextCore::New(runtime);
}

/* static */
bool JsrtContext::Is(void * ref)
{
    return VirtualTableInfo<JsrtContextCore>::HasVirtualTable(ref);
}

void JsrtContext::OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo)
{
    ((JsrtContextCore *)this)->OnScriptLoad(scriptFunction, utf8SourceInfo);
}

JsrtContextCore::JsrtContextCore(JsrtRuntime * runtime) :
    JsrtContext(runtime)
{       
    Link();
}

/* static */
JsrtContextCore *JsrtContextCore::New(JsrtRuntime * runtime)
{        
    auto result = RecyclerNewFinalizedLeaf(runtime->GetThreadContext()->EnsureRecycler(), JsrtContextCore, runtime);

    result->EnsureScriptContext();

    return result;
}

void JsrtContextCore::Dispose(bool isShutdown)
{
    if (nullptr != this->GetScriptContext())
    {
        Js::ScriptContext::Delete(this->GetScriptContext());
        this->SetScriptContext(nullptr);

        Unlink();        
    }
}

Js::ScriptContext* JsrtContextCore::EnsureScriptContext()
{
    Assert(this->GetScriptContext() == nullptr);
    
    ThreadContext* localThreadContext = this->GetRuntime()->GetThreadContext();

    AutoPtr<Js::ScriptContext> newScriptContext(Js::ScriptContext::New(localThreadContext));

    newScriptContext->Initialize();

    this->SetScriptContext(newScriptContext.Detach());

    Js::JavascriptLibrary *library = this->GetScriptContext()->GetLibrary();
    Assert(library != nullptr);

    library->GetEvalFunctionObject()->SetEntryPoint(&Js::GlobalObject::EntryEval);
    library->GetFunctionConstructor()->SetEntryPoint(&Js::JavascriptFunction::NewInstance);

    return this->GetScriptContext();
}

void JsrtContextCore::OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo)
{
    // Do nothing
}
