//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "BackEnd.h"

NativeCodeGenerator * 
NewNativeCodeGenerator(Js::ScriptContext * scriptContext)
{
    return HeapNew(NativeCodeGenerator, scriptContext);
}

void 
DeleteNativeCodeGenerator(NativeCodeGenerator * nativeCodeGen)
{
    HeapDelete(nativeCodeGen);
}
     
void 
CloseNativeCodeGenerator(NativeCodeGenerator * nativeCodeGen)
{
    nativeCodeGen->Close();
}

bool
IsClosedNativeCodeGenerator(NativeCodeGenerator * nativeCodeGen)
{
    return nativeCodeGen->IsClosed();
}

void SetProfileModeNativeCodeGen(NativeCodeGenerator *pNativeCodeGen, BOOL fSet)
{
    pNativeCodeGen->SetProfileMode(fSet);
}

void UpdateNativeCodeGeneratorForDebugMode(NativeCodeGenerator* nativeCodeGen)
{
    nativeCodeGen->UpdateQueueForDebugMode();
}

CriticalSection *GetNativeCodeGenCriticalSection(NativeCodeGenerator *pNativeCodeGen)
{
    return pNativeCodeGen->Processor()->GetCriticalSection();
}
     
///----------------------------------------------------------------------------
///
/// GenerateFunction
///
///     This is the main entry point for the runtime to call the native code 
///     generator for js funciton.
///
///----------------------------------------------------------------------------

void 
GenerateFunction(NativeCodeGenerator * nativeCodeGen, Js::FunctionBody * fn, Js::ScriptFunction * function) 
{
    nativeCodeGen->GenerateFunction(fn, function);
}
CodeGenAllocators* GetForegroundAllocator(NativeCodeGenerator * nativeCodeGen, PageAllocator* pageallocator)
{
    return nativeCodeGen->GetCodeGenAllocator(pageallocator);
}
#ifdef ENABLE_PREJIT
void 
GenerateAllFunctions(NativeCodeGenerator * nativeCodeGen, Js::FunctionBody *fn) 
{
    nativeCodeGen->GenerateAllFunctions(fn);
}
#endif

void
GenerateAllFunctionsForSerialization(NativeCodeGenerator  * nativeCodeGen, Js::FunctionBody * fn, BYTE *sourceCode, DWORD dwSourceCodeSize, BYTE *byteCode, DWORD dwByteCodeSize, DWORD dwFunctionTableLength, BYTE * functionTable, BYTE ** nativeCode, DWORD * pdwNativeCodeSize)
{
    nativeCodeGen->GenerateAllFunctionsForSerialization(fn, sourceCode, dwSourceCodeSize, byteCode, dwByteCodeSize, dwFunctionTableLength, functionTable, nativeCode, pdwNativeCodeSize);
}

bool
DeserializeFunction(NativeCodeGenerator  * nativeCodeGen, Js::FunctionBody *function, Js::NativeModule *nativeModule)
{
    return nativeCodeGen->DeserializeFunction(function, nativeModule);
}

#ifdef IR_VIEWER
Js::Var
RejitIRViewerFunction(NativeCodeGenerator *nativeCodeGen, Js::FunctionBody *fn, Js::ScriptContext *scriptContext)
{
    return nativeCodeGen->RejitIRViewerFunction(fn, scriptContext);
}
#endif

void 
GenerateLoopBody(NativeCodeGenerator *nativeCodeGen, Js::FunctionBody *fn, Js::LoopHeader * loopHeader, Js::EntryPointInfo* info, uint localCount, Js::Var localSlots[])
{
    nativeCodeGen->GenerateLoopBody(fn, loopHeader, info, localCount, localSlots);
}

void
NativeCodeGenEnterScriptStart(NativeCodeGenerator * nativeCodeGen)
{
    if (nativeCodeGen)
    {
        nativeCodeGen->EnterScriptStart();
    }
}

BOOL IsIntermediateCodeGenThunk(Js::JavascriptMethod codeAddress)
{
    return NativeCodeGenerator::IsThunk(codeAddress);
}

BOOL IsAsmJsCodeGenThunk(Js::JavascriptMethod codeAddress)
{
    return NativeCodeGenerator::IsAsmJsCodeGenThunk(codeAddress);
}

CheckCodeGenFunction GetCheckCodeGenFunction(Js::JavascriptMethod codeAddress)
{
    return NativeCodeGenerator::GetCheckCodeGenFunction(codeAddress);
}

uint GetBailOutRegisterSaveSlotCount()
{
    // REVIEW: not all registers are used, we are allocation more space then necessary.
    return LinearScanMD::GetRegisterSaveSlotCount();
}

uint 
GetBailOutReserveSlotCount()
{
    return 1; //For arugments id
}


#if DBG
void CheckIsExecutable(Js::RecyclableObject * function, Js::JavascriptMethod entrypoint)
{
    Js::ScriptContext * scriptContext = function->GetScriptContext();
    // it's easy to call the default entrypoint from RecyclableObject.
    AssertMsg((Js::JavascriptFunction::Is(function) && Js::JavascriptFunction::FromVar(function)->IsExternalFunction())
        || Js::CrossSite::IsThunk(entrypoint) || !scriptContext->IsActuallyClosed() || 
        (scriptContext->GetThreadContext()->IsScriptActive() && !Js::JavascriptConversion::IsCallable(function)),
        "Can't call function when the script context is closed");

    if (scriptContext->GetThreadContext()->IsScriptActive())
    {
        return;
    }
    if (function->IsExternal())
    {
        return;
    }
    if (Js::JavascriptOperators::GetTypeId(function) == Js::TypeIds_HostDispatch)
    {
        AssertMsg(false, "Has to go thru CallRootFunction to start calling javascript function");
    }
    else if (Js::JavascriptFunction::Is(function))
    {
        if (((Js::JavascriptFunction*)function)->IsExternalFunction())
        {
            return;
        }
        else if (((Js::JavascriptFunction*)function)->IsWinRTFunction())
        {
            return;
        }
        else
        {
            AssertMsg(false, "Has to go thru CallRootFunction to start calling javascript function");
        }
    }
    else
    {
        AssertMsg(false, "Has to go thru CallRootFunction to start calling javascript function");
    }
}
#endif

#ifdef PROFILE_EXEC
void
CreateProfilerNativeCodeGen(NativeCodeGenerator * nativeCodeGen, Js::ScriptContextProfiler * profiler)
{
    nativeCodeGen->CreateProfiler(profiler);
}

void
ProfilePrintNativeCodeGen(NativeCodeGenerator * nativeCodeGen)
{
    nativeCodeGen->ProfilePrint();
}

void
SetProfilerFromNativeCodeGen(NativeCodeGenerator * toNativeCodeGen, NativeCodeGenerator * fromNativeCodeGen)
{
    toNativeCodeGen->SetProfilerFromNativeCodeGen(fromNativeCodeGen);
}
#endif

void DeleteNativeCodeData(NativeCodeData * data)
{
    delete data;
}