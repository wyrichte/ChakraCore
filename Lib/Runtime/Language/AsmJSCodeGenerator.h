//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once

namespace Js
{
    class ScriptContext;
    class AsmJsCodeGenerator
    {
        ScriptContext* mScriptContext;
        CodeGenAllocators* mForegroundAllocators;
        PageAllocator * mPageAllocator;
        AsmJsEncoder    mEncoder;
    public:
        AsmJsCodeGenerator( ScriptContext* scriptContext );
        void CodeGen( FunctionBody* functionBody );

    };
}