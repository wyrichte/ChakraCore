//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once


namespace Js
{
    class JavascriptExternalOperators
    {
    public:
        static BOOL GetProperty(DynamicObject* scriptObject, PropertyId id, Var* varMember, ScriptContext * scriptContext);
        static BOOL GetPropertyReference(DynamicObject* scriptObject, PropertyId id, Var* varMember, ScriptContext * scriptContext);
        static BOOL SetProperty(DynamicObject* scriptObject, PropertyId id, Var value, ScriptContext * scriptContext);
        static BOOL DeleteProperty(DynamicObject * scriptObject, PropertyId id, ScriptContext * scriptContext);
    };

    class JavascriptExternalConversion
    {
    public:
        static Var ToPrimitive(Js::DynamicObject * object, JavascriptHint hint, ScriptContext * scriptContext);
        static JavascriptString * ToString(Var instance, ScriptContext * scriptContext);
        static double ToNumber(Var instance, ScriptContext * scriptContext);
        static int ToInt32(Var instance, ScriptContext * scriptContext);
        static __int64 ToInt64(Var instance, ScriptContext* scriptContext);
        static unsigned __int64 ToUInt64(Var instance, ScriptContext* scriptContext);
        static BOOL ToBoolean(Var instance, ScriptContext * scriptContext);
    };

#if DBG
    // TODO:  we can have an autoptr for more checking if we need to verify entry state.
#define VERIFYHRESULTBEFORERETURN(hr, scriptContext)  \
    AssertMsg(!scriptContext->HasRecordedException() || scriptContext->GetThreadContext()->GetRecordedException()->IsPendingExceptionObject() ||  scriptContext->GetThreadContext()->HasUnhandledException() || (hr == SCRIPT_E_RECORDED || hr == SCRIPT_E_PROPAGATE), "lost recorded exception");
#else
#define VERIFYHRESULTBEFORERETURN(hr, threadContext)
#endif
};
