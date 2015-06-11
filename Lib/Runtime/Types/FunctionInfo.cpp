// Copyright (C) Microsoft. All rights reserved. 

#include "StdAfx.h"

namespace Js
{   
    FunctionInfo::FunctionInfo(JavascriptMethod entryPoint, Attributes attributes, LocalFunctionId functionId, FunctionBody* functionBodyImpl) 
        : originalEntryPoint(entryPoint), attributes(attributes), functionBodyImpl(functionBodyImpl), functionId(functionId)
    {
#if !DYNAMIC_INTERPRETER_THUNK
        Assert(entryPoint != null);
#endif
    }

    void FunctionInfo::VerifyOriginalEntryPoint() const
    {
        Assert(!this->HasBody() || this->IsDeferredParseFunction() || this->IsDeferredDeserializeFunction() || this->GetFunctionProxy()->HasValidEntryPoint());
    }

    JavascriptMethod
    FunctionInfo::GetOriginalEntryPoint() const
    {
        VerifyOriginalEntryPoint();
        return GetOriginalEntryPoint_Unchecked();
    }

    JavascriptMethod FunctionInfo::GetOriginalEntryPoint_Unchecked() const
    {
        return originalEntryPoint;
    }

    void FunctionInfo::SetOriginalEntryPoint(const JavascriptMethod originalEntryPoint)
    {
        Assert(originalEntryPoint);
        this->originalEntryPoint = originalEntryPoint;
    }

    FunctionBody *
    FunctionInfo::GetFunctionBody() const
    {
        Assert(functionBodyImpl == null || functionBodyImpl->IsFunctionBody());
        return (FunctionBody *)functionBodyImpl;
    }
}
