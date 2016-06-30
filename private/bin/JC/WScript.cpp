/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "StdAfx.h"

///----------------------------------------------------------------------------
///
/// WScript::Echo
///
/// Echo() printes all of the specified arguments on a line, separated by
/// spaces:
/// - Arg:0 = "this"
/// - Arg:1-n = Values to compare
///
///----------------------------------------------------------------------------

Js::Var WScript::Echo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Js::RecyclableObject* dynamo = Js::RecyclableObject::FromVar(args[0]);
    Js::ScriptContext* scriptContext = dynamo->GetScriptContext();
    for (uint index = 1; index < args.Info.Count; index++)
    {
        if (index > 1)
        {
            printf(" ");
        }
        Write(args[index], scriptContext, stdout);
    }

    printf("\n");

    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var WScript::Quit(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Js::RecyclableObject* dynamo = Js::RecyclableObject::FromVar(args[0]);
    Js::ScriptContext* scriptContext = dynamo->GetScriptContext();
    int value = 0;
    if (args.Info.Count > 1)
    {
        value = (int)Js::JavascriptConversion::ToInteger(args[1], scriptContext);
    }
    exit(value);
}

Js::Var WScript::StdErrWriteLine(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Js::RecyclableObject* dynamo = Js::RecyclableObject::FromVar(args[0]);
    Js::ScriptContext* scriptContext = dynamo->GetScriptContext();
    for (uint index = 1; index < args.Info.Count; index++)
    {
        if (index > 1)
        {
            printf(" ");
        }
        Write(args[index], scriptContext, stderr);
    }
    printf("\n");
    return scriptContext->GetLibrary()->GetUndefined();
}

void WScript::Write(Js::Var aValue, Js::ScriptContext* scriptContext, FILE* file)
{
    switch (Js::JavascriptOperators::GetTypeId(aValue))
    {
    case Js::TypeIds_Integer:
    {
        //
        // Special case SmInts to avoid creating extra garbage.
        //

        int nValue = Js::TaggedInt::ToInt32(aValue);
        fprintf(file, "%d", nValue);
        break;
    }

    default:
    {
        Js::JavascriptString* pstDisplay = Js::JavascriptConversion::ToString(aValue, scriptContext);
        const char16 *pstDisplayStr = pstDisplay->GetString();
        charcount_t charsToGo = pstDisplay->GetLength();

        char16 buf[1024];
        while (charsToGo != 0)
        {
            charcount_t count = min(static_cast<charcount_t>(_countof(buf) - 1), charsToGo);
            wcsncpy_s(buf, pstDisplayStr, count); // ensures NUL terminated
            fputws(buf, file);
            pstDisplayStr += count;
            charsToGo -= count;
        };

        break;
    }
    }
}

Js::FunctionInfo WScript::EntryInfo::Echo(WScript::Echo, Js::FunctionInfo::DoNotProfile);
Js::FunctionInfo WScript::EntryInfo::Quit(WScript::Quit, Js::FunctionInfo::DoNotProfile);
Js::FunctionInfo WScript::EntryInfo::StdErrWriteLine(WScript::StdErrWriteLine, Js::FunctionInfo::DoNotProfile);

void WScript::Initialize(Js::ScriptContext* scriptContext)
{
    // add WScript object to the root.
    Js::DynamicObject* wscriptObject = scriptContext->GetLibrary()->CreateObject();
    Js::PropertyRecord const * propRecord;
    scriptContext->GetOrAddPropertyRecord(_u("WScript"), 7, &propRecord);
    Js::JavascriptOperators::InitProperty(scriptContext->GetGlobalObject(), propRecord->GetPropertyId(), wscriptObject);

    // Add Echo function to WScript
    Js::JavascriptFunction* function = scriptContext->GetLibrary()->CreateNonProfiledFunction(&WScript::EntryInfo::Echo);
    Js::JavascriptOperators::InitProperty(function, Js::PropertyIds::length, Js::TaggedInt::ToVarUnchecked(1));
    scriptContext->GetOrAddPropertyRecord(_u("Echo"), 4, &propRecord);
    Js::JavascriptOperators::InitProperty(wscriptObject, propRecord->GetPropertyId(), function);

    //Javascript output from Emscripten relies on print being on global object.
    //This makes easy to run the Asmjs code generated from C++ source code through emscripten without any hand modification.
    scriptContext->GetOrAddPropertyRecord(_u("print"), 5, &propRecord);
    Js::JavascriptOperators::InitProperty(scriptContext->GetGlobalObject(), propRecord->GetPropertyId(), function);

    // Add Quit function to WScript
    function = scriptContext->GetLibrary()->CreateNonProfiledFunction(&WScript::EntryInfo::Quit);
    scriptContext->GetOrAddPropertyRecord(_u("Quit"), 4, &propRecord);
    Js::JavascriptOperators::InitProperty(wscriptObject, propRecord->GetPropertyId(), function);

    // Add StdError.WriteLine to WScript
    Js::DynamicObject* stderrObject = scriptContext->GetLibrary()->CreateObject();
    scriptContext->GetOrAddPropertyRecord(_u("StdErr"), 6, &propRecord);
    Js::JavascriptOperators::InitProperty(wscriptObject, propRecord->GetPropertyId(), stderrObject);
    function = scriptContext->GetLibrary()->CreateNonProfiledFunction(&WScript::EntryInfo::StdErrWriteLine);
    scriptContext->GetOrAddPropertyRecord(_u("WriteLine"), 9, &propRecord);
    Js::JavascriptOperators::InitProperty(stderrObject, propRecord->GetPropertyId(), function);
}