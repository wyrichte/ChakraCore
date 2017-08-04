//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#include "JDBackendUtil.h"

void PrintFunctionBody(RemoteFunctionBody& functionBody)
{
    functionBody.PrintNameAndNumberWithRawLink();
    GetExtension()->Out(" ");
    functionBody.PrintByteCodeLink();
    GetExtension()->Out("\n");
    functionBody.PrintAuxPtrs();
    GetExtension()->Out("Source Url: ");
    functionBody.PrintSourceUrl();
    GetExtension()->Out("\n");
    functionBody.PrintSource();
    GetExtension()->Out("\n");

}

EXT_COMMAND(ffb,
    "Find FunctionBody",
    "{exact;b,o;exactMatch;do exact match}"
    "{;x;functionName;String of function name}")
{
    bool exactMatch = HasArg("exact");
    PCSTR arg = GetUnnamedArgStr(0);
    size_t length = strlen(arg);
    if (length == 0)
    {
        return;
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
                    PCWSTR name = functionBody.GetDisplayName(&buffer);
                    size_t nameLength = wcslen(name);
                    if (exactMatch)
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
                    else
                    {
                        PCWSTR match = wcschr(name, arg[0]);
                        bool matched = false;
                        while (match && nameLength - (match - name) >= length)
                        {
                            matched = true;
                            for (size_t i = 1; i < length; i++)
                            {
                                if (match[i] != arg[i])
                                {
                                    matched = false;
                                    break;
                                }
                            }
                            if (matched)
                            {
                                break;
                            }
                            match = wcschr(match + 1, arg[0]);
                        }
                        if (!matched)
                        {
                            return false;
                        }
                    }
                    functionBody.PrintNameAndNumberWithLink();
                    Out("\n");
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
        input = JDRemoteTyped::FromPtrWithVtable(input.GetLong());
        inputType = input.GetTypeName();
    }
    else if (strcmp(inputType, "int64") == 0)
    {
        input = JDRemoteTyped::FromPtrWithVtable(input.GetLong64());
        inputType = input.GetTypeName();
    }

    inputType = JDUtil::StripStructClass(inputType);
    if (strcmp(inputType, "Js::FunctionBody") == 0 || strcmp(inputType, "Js::FunctionBody *") == 0
        || strcmp(inputType, "Js::ParseableFunctionInfo") == 0
        || strcmp(inputType, "Js::ParseableFunctionInfo *") == 0)
    {
        functionBody = input;
    }
    else if (strcmp(inputType, "Js::InterpreterStackFrame") == 0 || strcmp(inputType, "Js::InterpreterStackFrame *") == 0)
    {
        functionBody = input.Field("m_functionBody");
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
        functionBody = input.Field("m_functionBody");
    }
    else
    {
        this->ThrowLastError("Unknown type for function body dump");
    }
    PrintFunctionBody(functionBody);
}
