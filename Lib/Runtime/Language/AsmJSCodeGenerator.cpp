//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#include "StdAfx.h"
#include "CodeGenAllocators.h"

namespace Js
{
    AsmJsCodeGenerator::AsmJsCodeGenerator( ScriptContext* scriptContext ) :
        mScriptContext( scriptContext )
        ,mPageAllocator(scriptContext->GetThreadContext()->GetPageAllocator())
    {
        //use the same foreground allocator as NativeCodeGen
        mForegroundAllocators = GetForegroundAllocator(scriptContext->GetNativeCodeGenerator(),mPageAllocator);
        mEncoder.SetPageAllocator( mPageAllocator );
        mEncoder.SetCodeGenAllocator( mForegroundAllocators );
    }

    void AsmJsCodeGenerator::CodeGen( FunctionBody* functionBody )
    {
        AsmJsFunctionInfo* asmInfo = functionBody->GetAsmJsFunctionInfo();
        Assert( asmInfo );
        
        void* address = mEncoder.Encode( functionBody );
        if( address )
        {
            FunctionEntryPointInfo* funcEntrypointInfo = (FunctionEntryPointInfo*)functionBody->GetDefaultEntryPointInfo();
            EntryPointInfo* entrypointInfo = (EntryPointInfo*)funcEntrypointInfo; 
            Assert(entrypointInfo->GetIsAsmJSFunction());
            //set entrypointinfo address and nativeAddress with TJ address
            entrypointInfo->address = address;
            entrypointInfo->SetNativeAddress((void*)address);
#if ENABLE_DEBUG_CONFIG_OPTIONS
            funcEntrypointInfo->SetIsTJMode(true);
#endif
        }
    }

}