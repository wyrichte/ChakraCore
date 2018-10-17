//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#include "JDBackendUtil.h"

void PrintFunctionBody(RemoteParseableFunctionInfo& function)
{
    function.PrintNameAndNumberWithRawLink();
    GetExtension()->Out(" ");
    if (function.IsFunctionBody())
    {
        RemoteFunctionBody(function).PrintByteCodeLink();
    }
    GetExtension()->Out("\n");
    function.PrintAuxPtrs();
    GetExtension()->Out("Source Url: ");
    function.PrintSourceUrl();
    GetExtension()->Out("\n");
    function.PrintSource();
    GetExtension()->Out("\n");

}

EXT_COMMAND(ffb,
    "Find FunctionBody",
    "{exact;b,o;exactMatch;do exact match}"
    "{functionId;s,o;functionId;Function Id to filter to}"
    "{sourceLocation;s,o;sourceId;Source Location to filter to}"
    "{;x,o;functionName;String of function name (last arg)}")
{
    bool exactMatch = HasArg("exact");
    bool compareByName = true;
    PCSTR functionId = HasArg("functionId") ? GetArgStr("functionId") : nullptr;
    PCSTR sourceLocation = HasArg("sourceLocation") ? GetArgStr("sourceLocation") : nullptr;

    ULONG64 functionIdFilter = 0;
    ULONG64 sourceIdFilter = 0;
    ULONG64 sourceLineFilter = 0;
    ULONG64 sourceColumnFilter = 0;

    PCSTR arg = nullptr;
    size_t length = 0; 
    bool showAll = (functionId == nullptr && sourceLocation == nullptr && GetNumUnnamedArgs() == 0);

    if (GetNumUnnamedArgs() > 0)
    {
        arg = GetUnnamedArgStr(0);
        length = strlen(arg);
    }

    if (functionId != nullptr || sourceLocation != nullptr)
    {
        exactMatch = true;
        compareByName = false;

        if (functionId != nullptr)
        {
            int tokens = sscanf(functionId, "%I64u.%I64u", &sourceIdFilter, &functionIdFilter);
            if (tokens != 2)
            {
                Out("Invalid function id- expected <source id>.<function id>\n");
                return;
            }
        }

        if (sourceLocation != nullptr)
        {
            int tokens = sscanf(sourceLocation, "%I64u.%I64u", &sourceLineFilter, &sourceColumnFilter);
            if (tokens != 2)
            {
                Out("Invalid source location- expected <source row>.<source column>\n");
                return;
            }
        }
    }

    RemoteThreadContext::ForEach([=](RemoteThreadContext threadContext)
    {
        return threadContext.ForEachScriptContext([=](RemoteScriptContext scriptContext)
        {
            return scriptContext.ForEachUtf8SourceInfo([=](ULONG, RemoteUtf8SourceInfo utf8SourceInfo)
            {
                return utf8SourceInfo.GetFunctionBodyDictionary().ForEachValue([=](RemoteFunctionBody functionBody)
                {
                    ExtBuffer<WCHAR> buffer;
                    bool foundMatch = false;

                    PCWSTR name = functionBody.GetDisplayName(&buffer);
                    size_t nameLength = wcslen(name);
                    if (exactMatch)
                    {
                        if (functionId != nullptr || sourceLocation != nullptr)
                        {
                            bool functionIdMatch = (functionBody.GetSourceContextId() == sourceIdFilter &&
                                                    functionBody.GetLocalFunctionId() == functionIdFilter);

                            bool sourceLocMatch = (functionBody.GetLineNumber() == sourceLineFilter &&
                                                   functionBody.GetColumnNumber() == sourceColumnFilter);

                            if (functionId && sourceLocation)
                            {
                                foundMatch = (functionIdMatch && sourceLocMatch);
                            }
                            else
                            {
                                foundMatch = (functionIdMatch || sourceLocMatch);
                            }
                        }

                        if (compareByName)
                        {
                            if (nameLength != length)
                            {
                                return false;
                            }

                            for (size_t i = 0; i < length; i++)
                            {
                                if (arg[i] != name[i])
                                {
                                    return false;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (arg != nullptr)
                        {
                            PCWSTR functionNameMatch = wcschr(name, arg[0]);
                            bool matched = false;
                            while (functionNameMatch && nameLength - (functionNameMatch - name) >= length)
                            {
                                matched = true;
                                for (size_t i = 1; i < length; i++)
                                {
                                    if (functionNameMatch[i] != arg[i])
                                    {
                                        matched = false;
                                        break;
                                    }
                                }
                                if (matched)
                                {
                                    break;
                                }
                                functionNameMatch = wcschr(functionNameMatch + 1, arg[0]);
                            }
                            if (!functionNameMatch)
                            {
                                return false;
                            }

                            foundMatch = true;
                        }
                    }

                    if (foundMatch || showAll)
                    {
                        functionBody.PrintNameAndNumberWithLink();
                        Out("\n");
                    }

                    return false;
                });
            });
        });
    });
}

EXT_COMMAND(fb,
    "Dump FunctionBody",
    "{;x;functionBody;expression or address of function body}")
{
    PCSTR arg = GetUnnamedArgStr(0);
    ExtRemoteTyped input = ExtRemoteTyped(arg);
    RemoteFunctionBody functionBody;
    PCSTR inputType = input.GetTypeName();
    // Infer from vtable if it is just an address
    if (strcmp(inputType, "int") == 0)
    {
        input = JDRemoteTyped::FromPtrWithVtable(input.GetLong()).GetExtRemoteTyped();
        inputType = input.GetTypeName();
    }
    else if (strcmp(inputType, "int64") == 0)
    {
        input = JDRemoteTyped::FromPtrWithVtable(input.GetLong64()).GetExtRemoteTyped();
        inputType = input.GetTypeName();
    }

    inputType = JDUtil::StripStructClass(inputType);
    if (strcmp(inputType, "Js::FunctionBody") == 0 || strcmp(inputType, "Js::FunctionBody *") == 0
        || strcmp(inputType, "Js::ParseableFunctionInfo") == 0
        || strcmp(inputType, "Js::ParseableFunctionInfo *") == 0)
    {
        functionBody = JDRemoteTyped(input);
    }
    else if (strcmp(inputType, "Js::InterpreterStackFrame") == 0 || strcmp(inputType, "Js::InterpreterStackFrame *") == 0)
    {
        functionBody = JDRemoteTyped(input.Field("m_functionBody"));
    }
    else if (strcmp(inputType, "Js::JavascriptFunction") == 0 || strcmp(inputType, "Js::JavascriptFunction *") == 0
        || strcmp(inputType, "Js::ScriptFunction") == 0 || strcmp(inputType, "Js::ScriptFunction *") == 0)
    {
        RemoteFunctionInfo functionInfo(input.Field("functionInfo"));
        if (functionInfo.HasBody())
        {
            functionBody = functionInfo.GetFunctionBody();
        }
        else
        {
            this->ThrowLastError("Function Info not a function body");
        }
    }
    else if (strcmp(inputType, "Js::RecyclableObject") == 0 || strcmp(inputType, "Js::RecyclableObject *") == 0)
    {
        RemoteRecyclableObject recyclableObject(input);
        if (recyclableObject.IsJavascriptFunction())
        {
            RemoteFunctionInfo functionInfo(recyclableObject.AsJavascriptFunction().GetFunctionInfo());
            if (functionInfo.HasBody())
            {
                functionBody = functionInfo.GetFunctionBody();
            }
            else
            {
                this->ThrowLastError("Recyclable object not a script function");
            }
        }
        else
        {
            this->ThrowLastError("Recyclable object not a script function");
        }
    }
    else if (strcmp(inputType, "Func") == 0 || strcmp(inputType, "Func *") == 0)
    {
        functionBody = JDBackendUtil::GetFunctionBodyFromFunc(input);
    }
    else if (strcmp(inputType, "IRBuilder") == 0 || strcmp(inputType, "IRBuilder *") == 0)
    {
        functionBody = JDRemoteTyped(input.Field("m_functionBody"));
    }
    else
    {
        this->ThrowLastError("Unknown type for function body dump");
    }
    PrintFunctionBody(functionBody);
}
