//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    HRESULT ReportError(Js::JavascriptExceptionObject * pError);
    HRESULT CallRootFunction(Js::JavascriptFunction *function, Js::Arguments args, Js::Var *result);
    HRESULT GetPropertyOf(Js::RecyclableObject *object, Js::PropertyId id, Js::ScriptContext *scriptContext, Js::Var *result);
}