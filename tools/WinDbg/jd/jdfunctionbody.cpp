//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE
#include "JDBackendUtil.h"

void PrintFunctionBody(RemoteFunctionBody& functionBody)
{
    functionBody.PrintNameAndNumberWithRawLink();
    GetExtension()->Out(" ");
    functionBody.PrintByteCodeLink();
    GetExtension()->Out("\nSource Url: ");
    functionBody.PrintSourceUrl();
    GetExtension()->Out("\n");
    functionBody.PrintSource();
    GetExtension()->Out("\n");
    functionBody.PrintAuxPtrs();
}

EXT_COMMAND(fb,
    "Dump FunctionBody",
    "{;x;functionBody;express or address of function body}")
{
    PCSTR arg = GetUnnamedArgStr(0);
    ExtRemoteTyped input = ExtRemoteTyped(arg);
    RemoteFunctionBody functionBody;
    PCSTR inputType = input.GetTypeName();
    if (strcmp(inputType, "int") == 0 || strcmp(inputType, "int64") == 0)
    {
        // Just an address
        functionBody = ExtRemoteTyped(this->FillModule("(%s!Js::FunctionBody *)@$extin"), input.GetLong64());
    }
    else
    {
        inputType = JDUtil::StripStructClass(inputType);        
        if (strcmp(inputType, "Js::FunctionBody") == 0 || strcmp(inputType, "Js::FunctionBody *") == 0)
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
    }
    PrintFunctionBody(functionBody);
}
#endif