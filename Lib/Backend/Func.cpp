//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "BackEnd.h"
#ifdef F_JSETW
#include <IERESP_mshtml.h>
#endif

bool
Func::IsLoopBody() const
{
    return this->m_workItem->Type() == JsLoopBodyWorkItemType;
}

bool
Func::IsLoopBodyInTry() const
{
    return IsLoopBody() && ((JsLoopBodyCodeGen*)this->m_workItem)->loopHeader->isInTry;
}

///----------------------------------------------------------------------------
///
/// Func::Codegen
///
///     Codegen this function.
///
///----------------------------------------------------------------------------
void
Func::Codegen()
{
    Assert(!IsJitInDebugMode() || !m_jnFunction->GetHasTry());

    Js::ScriptContext* scriptContext = this->GetScriptContext();

    {
        if(EventEnabledJSCRIPT_FUNCTION_JIT_START())
        {
            WCHAR displayNameBuffer[256];
            WCHAR* displayName = displayNameBuffer;
            size_t sizeInChars = this->m_workItem->GetDisplayName(displayName, 256);
            if(sizeInChars > 256)
            {
                displayName = new WCHAR[sizeInChars];
                this->m_workItem->GetDisplayName(displayName, 256);
            }
            JSETW(EventWriteJSCRIPT_FUNCTION_JIT_START(
                this->GetFunctionNumber(),
                displayName,
                this->GetScriptContext(),
                this->m_workItem->GetInterpretedCount(),
                (const unsigned int)this->m_jnFunction->LengthInBytes(),
                this->m_jnFunction->GetByteCodeCount(),
                this->m_jnFunction->GetByteCodeInLoopCount(),
                (int)this->m_workItem->GetJitMode()));

            if(displayName != displayNameBuffer)
            {
                delete[] displayName;
            }
        }
    }

#if DBG_DUMP
    if (Js::Configuration::Global.flags.TestTrace.IsEnabled(Js::BackEndPhase))
    {
        if (this->IsLoopBody())
        {
            Output::Print(L"---BeginBackEnd: function: %s, loop:%d---\r\n", this->GetJnFunction()->GetDisplayName(), 
                static_cast<JsLoopBodyCodeGen *>(this->m_workItem)->GetLoopNumber());            
        }
        else
        {
            Output::Print(L"---BeginBackEnd: function: %s---\r\n", this->GetJnFunction()->GetDisplayName());
        }
        Output::Flush();
    }
#endif

    wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
    LARGE_INTEGER start_time = { 0 };

    if(PHASE_TRACE(Js::BackEndPhase, GetJnFunction()))
    {
        QueryPerformanceCounter(&start_time);
        if (this->IsLoopBody())
        {
            Output::Print(
                L"BeginBackEnd - function: %s (%s, line %u), loop: %u, mode: %S",
                GetJnFunction()->GetDisplayName(),
                GetJnFunction()->GetDebugNumberSet(debugStringBuffer),
                GetJnFunction()->GetLineNumber(),
                static_cast<JsLoopBodyCodeGen *>(this->m_workItem)->GetLoopNumber(),
                ExecutionModeName(m_workItem->GetJitMode()));
            if (this->m_jnFunction->GetIsAsmjsMode())
            {
                Output::Print(L" (Asmjs)\n");
            }
            else
            {
                Output::Print(L"\n");
            }
        }
        else
        {
            Output::Print(
                L"BeginBackEnd - function: %s (%s, line %u), mode: %S",
                GetJnFunction()->GetDisplayName(),
                GetJnFunction()->GetDebugNumberSet(debugStringBuffer),
                GetJnFunction()->GetLineNumber(),
                ExecutionModeName(m_workItem->GetJitMode()));

            if (this->m_jnFunction->GetIsAsmjsMode())
            {
                Output::Print(L" (Asmjs)\n");
            }
            else
            {
                Output::Print(L"\n");
            }
        }
        Output::Flush();
    }
#ifdef FIELD_ACCESS_STATS
    if (PHASE_TRACE(Js::ObjTypeSpecPhase, this->GetJnFunction()) || PHASE_TRACE(Js::EquivObjTypeSpecPhase, this->GetJnFunction()))
    {
        if (this->m_jitTimeData->inlineCacheStats)
        {
            auto stats = this->m_jitTimeData->inlineCacheStats;
            Output::Print(L"ObjTypeSpec: jit-ing function %s (#%s): inline cache stats:\n", this->GetJnFunction()->GetDisplayName(), this->GetJnFunction()->GetDebugNumberSet(debugStringBuffer));
            Output::Print(L"    overall: total %u, no profile info %u\n", stats->totalInlineCacheCount, stats->noInfoInlineCacheCount);
            Output::Print(L"    mono: total %u, empty %u, cloned %u\n", 
                stats->monoInlineCacheCount, stats->emptyMonoInlineCacheCount, stats->clonedMonoInlineCacheCount);
            Output::Print(L"    poly: total %u (high %u, low %u), null %u, empty %u, ignored %u, disabled %u, equivalent %u, non-equivalent %u, cloned %u\n",
                stats->polyInlineCacheCount, stats->highUtilPolyInlineCacheCount, stats->lowUtilPolyInlineCacheCount, 
                stats->nullPolyInlineCacheCount, stats->emptyPolyInlineCacheCount, stats->ignoredPolyInlineCacheCount, stats->disabledPolyInlineCacheCount,
                stats->equivPolyInlineCacheCount, stats->nonEquivPolyInlineCacheCount, stats->clonedPolyInlineCacheCount);
        }
        else
        {
            Output::Print(L"EquivObjTypeSpec: function %s (%s): inline cache stats unavailable\n", this->GetJnFunction()->GetDisplayName(), this->GetJnFunction()->GetDebugNumberSet(debugStringBuffer));
        }
        Output::Flush();
    }
#endif

    BEGIN_CODEGEN_PHASE(this, Js::BackEndPhase);
    {
        // IRBuilder

        BEGIN_CODEGEN_PHASE(this, Js::IRBuilderPhase);

        if (m_jnFunction->GetIsAsmjsMode())
        {
            IRBuilderAsmJs asmIrBuilder(this);
            asmIrBuilder.Build();
        }
        else
        {
            IRBuilder irBuilder(this);
            irBuilder.Build();
        }

        END_CODEGEN_PHASE(this, Js::IRBuilderPhase);

#ifdef IR_VIEWER
        IRtoJSObjectBuilder::DumpIRtoGlobalObject(this, Js::IRBuilderPhase);
#endif /* IR_VIEWER */


        BEGIN_CODEGEN_PHASE(this, Js::InlinePhase);

        InliningHeuristics heuristics(this->GetJnFunction());
        Inline inliner(this, heuristics);
        inliner.Optimize();

        END_CODEGEN_PHASE(this, Js::InlinePhase);

        if (scriptContext->IsClosed())
        {
            // Should not be jitting something in the foreground when the script context is actually closed
            Assert(IsBackgroundJIT() || !scriptContext->IsActuallyClosed());

            throw Js::OperationAbortedException();
        }

        // FlowGraph

        { //scope for FG arena

            NoRecoverMemoryJitArenaAllocator fgAlloc(L"BE-FlowGraph", m_alloc->GetPageAllocator(), Js::Throw::OutOfMemory);

            BEGIN_CODEGEN_PHASE(this, Js::FGBuildPhase);

            this->m_fg = FlowGraph::New(this, &fgAlloc);
            this->m_fg->Build();

            END_CODEGEN_PHASE(this, Js::FGBuildPhase);

            // Global Optimization and Type Specialization
            BEGIN_CODEGEN_PHASE(this, Js::GlobOptPhase);

            GlobOpt globOpt(this);
            globOpt.Optimize();

            END_CODEGEN_PHASE(this, Js::GlobOptPhase);

            // Delete flowGraph now
            this->m_fg->Destroy();
            this->m_fg = NULL;
        }

#ifdef IR_VIEWER
        IRtoJSObjectBuilder::DumpIRtoGlobalObject(this, Js::GlobOptPhase);
#endif /* IR_VIEWER */

        ThrowIfScriptClosed();

        // Lowering
        Lowerer lowerer(this);
        BEGIN_CODEGEN_PHASE(this, Js::LowererPhase);        
        lowerer.Lower();
        END_CODEGEN_PHASE(this, Js::LowererPhase);

#ifdef IR_VIEWER
        IRtoJSObjectBuilder::DumpIRtoGlobalObject(this, Js::LowererPhase);
#endif /* IR_VIEWER */

        // Encode constants

        Security security(this);

        BEGIN_CODEGEN_PHASE(this, Js::EncodeConstantsPhase)              
        security.EncodeLargeConstants();
        END_CODEGEN_PHASE(this, Js::EncodeConstantsPhase);

        if (this->GetScriptContext()->GetThreadContext()->DoInterruptProbe(this->GetJnFunction()))
        {
            BEGIN_CODEGEN_PHASE(this, Js::InterruptProbePhase)
            lowerer.DoInterruptProbes();
            END_CODEGEN_PHASE(this, Js::InterruptProbePhase)
        }

        // Register Allocation

        BEGIN_CODEGEN_PHASE(this, Js::RegAllocPhase);

        LinearScan linearScan(this);
        linearScan.RegAlloc();

        END_CODEGEN_PHASE(this, Js::RegAllocPhase);

#ifdef IR_VIEWER
        IRtoJSObjectBuilder::DumpIRtoGlobalObject(this, Js::RegAllocPhase);
#endif /* IR_VIEWER */

        ThrowIfScriptClosed();
        
        // Peephole optimizations

        BEGIN_CODEGEN_PHASE(this, Js::PeepsPhase);

        Peeps peeps(this);
        peeps.PeepFunc();

        END_CODEGEN_PHASE(this, Js::PeepsPhase);

        // Layout

        BEGIN_CODEGEN_PHASE(this, Js::LayoutPhase);

        SimpleLayout layout(this);
        layout.Layout();

        END_CODEGEN_PHASE(this, Js::LayoutPhase);

        if (this->HasTry() && this->hasBailoutInEHRegion)
        {
            BEGIN_CODEGEN_PHASE(this, Js::EHBailoutPatchUpPhase);
            lowerer.EHBailoutPatchUp();
            END_CODEGEN_PHASE(this, Js::EHBailoutPatchUpPhase);
        }

        // Insert NOPs (moving this before prolog/epilog for AMD64 and possibly ARM).
        BEGIN_CODEGEN_PHASE(this, Js::InsertNOPsPhase);
        security.InsertNOPs();
        END_CODEGEN_PHASE(this, Js::InsertNOPsPhase);

        // Prolog/Epilog
        BEGIN_CODEGEN_PHASE(this, Js::PrologEpilogPhase);
        if (m_jnFunction->GetIsAsmjsMode())
        {
            lowerer.LowerPrologEpilogAsmJs();
        }
        else
        {
            lowerer.LowerPrologEpilog();
        }
        END_CODEGEN_PHASE(this, Js::PrologEpilogPhase);

        BEGIN_CODEGEN_PHASE(this, Js::FinalLowerPhase);
        lowerer.FinalLower();
        END_CODEGEN_PHASE(this, Js::FinalLowerPhase);

        // Encoder
        BEGIN_CODEGEN_PHASE(this, Js::EncoderPhase);

        Encoder encoder(this);
        encoder.Encode();

        END_CODEGEN_PHASE_NO_DUMP(this, Js::EncoderPhase);

#ifdef IR_VIEWER
        IRtoJSObjectBuilder::DumpIRtoGlobalObject(this, Js::EncoderPhase);
#endif /* IR_VIEWER */

    }
    END_CODEGEN_PHASE(this, Js::BackEndPhase);

#if DBG_DUMP
    if (Js::Configuration::Global.flags.TestTrace.IsEnabled(Js::BackEndPhase))
    {
        Output::Print(L"---EndBackEnd---\r\n");
        Output::Flush();
    }
#endif

    if (PHASE_TRACE(Js::BackEndPhase, GetJnFunction()))
    {
        LARGE_INTEGER freq;
        LARGE_INTEGER end_time;
        QueryPerformanceCounter(&end_time);
        QueryPerformanceFrequency(&freq);
        if (this->IsLoopBody())
        {
            Output::Print(
                L"EndBackEnd - function: %s (%s, line %u), loop: %u, mode: %S, time:%8.6f mSec",
                GetJnFunction()->GetDisplayName(),
                GetJnFunction()->GetDebugNumberSet(debugStringBuffer),
                GetJnFunction()->GetLineNumber(),
                static_cast<JsLoopBodyCodeGen *>(this->m_workItem)->GetLoopNumber(),
                ExecutionModeName(m_workItem->GetJitMode()),
                (((double)((end_time.QuadPart - start_time.QuadPart)* (double)1000.0 / (double)freq.QuadPart))) / (1));

            if (this->m_jnFunction->GetIsAsmjsMode())
            {
                Output::Print(L" (Asmjs)\n");
            }
            else
            {
                Output::Print(L"\n");
            }
        }
        else
        {
            Output::Print(
                L"EndBackEnd - function: %s (%s, line %u), mode: %S time:%8.6f mSec",
                GetJnFunction()->GetDisplayName(),
                GetJnFunction()->GetDebugNumberSet(debugStringBuffer),
                GetJnFunction()->GetLineNumber(),
                ExecutionModeName(m_workItem->GetJitMode()),
                (((double)((end_time.QuadPart - start_time.QuadPart)* (double)1000.0 / (double)freq.QuadPart))) / (1));

            if (this->m_jnFunction->GetIsAsmjsMode())
            {
                Output::Print(L" (Asmjs)\n");
            }
            else
            {
                Output::Print(L"\n");
            }
        }
        Output::Flush();
    }

    {

        if(EventEnabledJSCRIPT_FUNCTION_JIT_STOP())
        {
            WCHAR displayNameBuffer[256];
            WCHAR* displayName = displayNameBuffer;
            size_t sizeInChars = this->m_workItem->GetDisplayName(displayName, 256);
            if(sizeInChars > 256)
            {
                displayName = new WCHAR[sizeInChars];
                this->m_workItem->GetDisplayName(displayName, 256);
            }
            void* entryPoint;
            ptrdiff_t codeSize;
            this->m_workItem->GetEntryPointAddress(&entryPoint, &codeSize);
            JSETW(EventWriteJSCRIPT_FUNCTION_JIT_STOP(
                this->GetFunctionNumber(),
                displayName,
                scriptContext,
                this->m_workItem->GetInterpretedCount(),
                entryPoint,
                codeSize));

            if(displayName != displayNameBuffer)
            {
                delete[] displayName;
            }
        }
    }

#if DBG_DUMP
    if (Js::Configuration::Global.flags.IsEnabled(Js::AsmDumpModeFlag))
    {
        FILE * oldFile = 0;
        FILE * asmFile = scriptContext->GetNativeCodeGenerator()->asmFile;
        if (asmFile)
        {
            oldFile = Output::SetFile(asmFile);
        }

        this->Dump(IRDumpFlags_AsmDumpMode);

        Output::Flush();

        if (asmFile)
        {
            FILE *openedFile = Output::SetFile(oldFile);
            Assert(openedFile == asmFile);
        }
    }
#endif
}


///----------------------------------------------------------------------------
/// Func::StackAllocate
///     Allocate stack space of given size.
///----------------------------------------------------------------------------
int32 
Func::StackAllocate(int size)
{
    Assert(this->IsTopFunc());

    int32 offset;

#ifdef MD_GROW_LOCALS_AREA_UP
    // Locals have positive offsets and are allocated from bottom to top.
    m_localStackHeight = Math::Align(m_localStackHeight, min(size, MachStackAlignment));

    offset = m_localStackHeight;
    m_localStackHeight += size;
#else
    // Locals have negative offsets and are allocated from top to bottom.    
    m_localStackHeight += size;
    m_localStackHeight = Math::Align(m_localStackHeight, min(size, MachStackAlignment));

    offset = -m_localStackHeight;
#endif

    return offset;
}

///----------------------------------------------------------------------------
///
/// Func::StackAllocate
///
///     Allocate stack space for this symbol.
///
///----------------------------------------------------------------------------

int32               
Func::StackAllocate(StackSym *stackSym, int size)
{
    Assert(size > 0);
    if (stackSym->IsArgSlotSym() || stackSym->IsParamSlotSym() || stackSym->IsAllocated())
    {        
        return stackSym->m_offset;
    }
#ifdef  DBG
    if (stackSym->m_type != TyMisc)
    {
        int stackSymSize = max(stackSym->GetSymSize(), MachRegInt);
        Assert(stackSymSize == size);
    }
#endif
    Assert(stackSym->m_offset == 0);
    stackSym->m_allocated = true;
    stackSym->m_offset = StackAllocate(size);
   
    return stackSym->m_offset;
}

void
Func::SetArgOffset(StackSym *stackSym, int32 offset)
{
    AssertMsg(offset >= 0, "Why is the offset, negative?");
    stackSym->m_offset = offset;
    stackSym->m_allocated = true;
}

///
/// Ensures that local var slots are created, if the function has locals.
///     Allocate stack space for locals used for debugging 
///     (for local non-temp vars we write-through memory so that locals inspection can make use of that.).
//      On stack, after local slots we allocate space for metadata (in particular, whether any the locals was changed in debugger).
///
void 
Func::EnsureLocalVarSlots()
{
    Assert(IsJitInDebugMode());

    if (!this->HasLocalVarSlotCreated())
    {
        Assert(this->m_jnFunction != NULL);
        uint32 localSlotCount = this->m_jnFunction->GetNonTempLocalVarCount();
        if (localSlotCount && m_localVarSlotsOffset == Js::Constants::InvalidOffset)
        {
            // Allocate the slots.
            int32 size = localSlotCount * GetDiagLocalSlotSize();
            m_localVarSlotsOffset = StackAllocate(size);
            m_hasLocalVarChangedOffset = StackAllocate(max(1, MachStackAlignment)); // Can't alloc less than StackAlignment bytes.

            // TODO : Add metadata to know what kind of value it contains.

            Assert(this->m_workItem->Type() == JsFunctionType);

            // Store in the entry point info, so that it will later be used when we do the variable inspection.
            Js::FunctionEntryPointInfo * entryPointInfo = static_cast<Js::FunctionEntryPointInfo*>(this->m_workItem->AsInMemoryWorkItem()->GetEntryPoint());
            Assert(entryPointInfo != NULL);

            entryPointInfo->localVarSlotsOffset = AdjustOffsetValue(m_localVarSlotsOffset);
            entryPointInfo->localVarChangedOffset = AdjustOffsetValue(m_hasLocalVarChangedOffset);
        }
    }
}

void Func::SetFirstArgOffset(IR::Instr* inlineeStart)
{
    Assert(inlineeStart->m_func == this);
    Assert(!IsTopFunc());
    int32 lastOffset;

    IR::Instr* arg = inlineeStart->GetNextArg();
    const auto lastArgOutStackSym = arg->GetDst()->AsSymOpnd()->m_sym->AsStackSym();
    lastOffset = lastArgOutStackSym->m_offset;
    Assert(lastArgOutStackSym->m_isSingleDef);
    const auto secondLastArgOutOpnd = lastArgOutStackSym->m_instrDef->GetSrc2();
    if (secondLastArgOutOpnd->IsSymOpnd())
    {
        const auto secondLastOffset = secondLastArgOutOpnd->AsSymOpnd()->m_sym->AsStackSym()->m_offset;
        if (secondLastOffset > lastOffset)
        {
            lastOffset = secondLastOffset;
        }
    }
    lastOffset += MachPtr;
    int32 firstActualStackOffset = lastOffset - ((this->actualCount + Js::Constants::InlineeMetaArgCount) * MachPtr);
    Assert((this->firstActualStackOffset == -1) || (this->firstActualStackOffset == firstActualStackOffset));
    this->firstActualStackOffset = firstActualStackOffset;
}

int32 
Func::GetLocalVarSlotOffset(int32 slotId)
{
    this->EnsureLocalVarSlots();
    Assert(m_localVarSlotsOffset != Js::Constants::InvalidOffset);

    int32 slotOffset = slotId * GetDiagLocalSlotSize();

    return m_localVarSlotsOffset + slotOffset;
}

void Func::OnAddSym(Sym* sym)
{
    Assert(sym);
    if (this->IsJitInDebugMode() && this->IsNonTempLocalVar(sym->m_id))
    {
        Assert(m_nonTempLocalVars);
        m_nonTempLocalVars->Set(sym->m_id);
    }
}

/// 
/// Returns offset of the flag (1 byte) whether any local was changed (in debugger).
/// If the function does not have any locals, returns -1.
///
int32 
Func::GetHasLocalVarChangedOffset()
{
    this->EnsureLocalVarSlots();
    return m_hasLocalVarChangedOffset;
}

bool
Func::IsJitInDebugMode()
{
    return 
        Js::Configuration::Global.EnableJitInDebugMode() && 
        this->m_workItem->IsInMemoryWorkItem() &&
        this->m_workItem->AsInMemoryWorkItem()->IsJitInDebugMode();
}

bool
Func::IsNonTempLocalVar(uint32 slotIndex)
{
    Assert(this->m_jnFunction != NULL);
    return this->m_jnFunction->IsNonTempLocalVar(slotIndex);
}

int32
Func::AdjustOffsetValue(int32 offset)
{
#ifdef MD_GROW_LOCALS_AREA_UP
        return -(offset + BailOutInfo::StackSymBias);
#else
        // Stack offset are negative, includes the PUSH EBP and return address
        return offset - (2 * MachPtr);
#endif
}

#ifdef MD_GROW_LOCALS_AREA_UP
// Note: this is called during jit-compile when we finalize bail out record.
void
Func::AjustLocalVarSlotOffset()
{
    if (m_jnFunction->GetNonTempLocalVarCount())
    {
        // Turn positive SP-relative base locals offset into negative frame-pointer-relative offset
        // This is changing value for restoring the locals when read due to locals inspection.

        int localsOffset = m_localVarSlotsOffset - (m_localStackHeight + m_ArgumentsOffset);
        int valueChangeOffset = m_hasLocalVarChangedOffset - (m_localStackHeight + m_ArgumentsOffset);

        Assert(this->m_workItem->IsInMemoryWorkItem());
        Js::FunctionEntryPointInfo * entryPointInfo = static_cast<Js::FunctionEntryPointInfo*>(this->m_workItem->AsInMemoryWorkItem()->GetEntryPoint());
        Assert(entryPointInfo != NULL);

        entryPointInfo->localVarSlotsOffset = localsOffset;
        entryPointInfo->localVarChangedOffset = valueChangeOffset;
    }
}
#endif

bool
Func::DoGlobOptsForGeneratorFunc()
{
    // Disable GlobOpt optimizations for generators initially. Will visit and enable each one by one.
    return !m_jnFunction->IsGenerator();
}


void
Func::SetDoFastPaths()
{
    // Make sure we only call this once!
    Assert(!this->hasCalledSetDoFastPaths);
    
    bool isLeaf = this->m_isLeaf && !PHASE_OFF(Js::LeafFastPathPhase, this);
    bool doFastPaths = false;

    if(!PHASE_OFF(Js::FastPathPhase, this) && (!IsSimpleJit() || Js::FunctionBody::IsNewSimpleJit()))
    {
        if (isLeaf || this->GetScriptContext()->GetThreadContext()->GetSourceSize() < (size_t)CONFIG_FLAG(FastPathCap) || CONFIG_FLAG(ForceFastPath))
        {
            doFastPaths = true;
        } 
    }

    this->m_doFastPaths = doFastPaths;
#ifdef DBG
    this->hasCalledSetDoFastPaths = true;
#endif
}

#ifdef _M_ARM

RegNum
Func::GetLocalsPointer() const
{
#ifdef DBG
    if (Js::Configuration::Global.flags.IsEnabled(Js::ForceLocalsPtrFlag))
    {
        return ALT_LOCALS_PTR;
    }
#endif

    if (this->m_jnFunction->GetHasTry())
    {
        return ALT_LOCALS_PTR;
    }

    return RegSP;
}

#endif

void Func::InitStackClosureSyms()
{
    Assert(this->DoStackFrameDisplay());
    Assert(this->m_localClosureSym == NULL);

    // Allocate stack space for closure pointers. Do this only if we're jitting for stack closures, and
    // tell bailout that these are not byte code symbols so that we don't try to encode them in the bailout record,
    // as they don't have normal lifetimes.
    Js::RegSlot regSlot = this->GetJnFunction()->GetStackScopeSlotsReg();
    this->m_localClosureSym = StackSym::FindOrCreate(static_cast<SymID>(regSlot), (Js::RegSlot)-1, this);
    regSlot = this->GetJnFunction()->GetStackFrameDisplayReg();
    this->m_localFrameDisplaySym = StackSym::FindOrCreate(static_cast<SymID>(regSlot), (Js::RegSlot)-1, this); 
}

bool Func::CanAllocInPreReservedHeapPageSegment ()
{
#ifdef _CONTROL_FLOW_GUARD 
    return PHASE_FORCE1(Js::PreReservedHeapAllocPhase) || (!PHASE_OFF1(Js::PreReservedHeapAllocPhase) && IsInMemory() &&
        !IsJitInDebugMode() && !GetScriptContext()->IsInDebugMode() && GetScriptContext()->GetThreadContext()->IsCFGEnabled()
#if _M_IX86
        && m_workItem->GetJitMode() == ExecutionMode::FullJit && GetCodeGenAllocators()->canCreatePreReservedSegment);
#elif _M_X64
        && true);
#else
        && false); //Not yet implemented for architectures other than x86 and amd64.
#endif  //_M_ARCH
#else 
    return false;
#endif//_CONTROL_FLOW_GUARD
}

///----------------------------------------------------------------------------
///
/// Func::GetInstrCount
///
///     Returns the number of instrs.
///     Note: It counts all instrs for now, including labels, etc.
///
///----------------------------------------------------------------------------

uint32
Func::GetInstrCount()
{
    uint instrCount = 0;

    FOREACH_INSTR_IN_FUNC(instr, this)
    {
        instrCount++;
    }NEXT_INSTR_IN_FUNC;

    return instrCount;
}

///----------------------------------------------------------------------------
///
/// Func::NumberInstrs
///
///     Number each instruction in order of appearance in the function.
///
///----------------------------------------------------------------------------

void
Func::NumberInstrs()
{
#if DBG_DUMP
    Assert(this->IsTopFunc());
    Assert(!this->hasInstrNumber);
    this->hasInstrNumber = true;
#endif
    uint instrCount = 1;

    FOREACH_INSTR_IN_FUNC(instr, this)
    {
        instr->SetNumber(instrCount++); 
    }
    NEXT_INSTR_IN_FUNC;
}


///----------------------------------------------------------------------------
///
/// Func::BeginPhase
///
/// Takes care of the profiler
///
///----------------------------------------------------------------------------


void
Func::BeginPhase(Js::Phase tag)
{
#ifdef PROFILE_EXEC
    AssertMsg((this->m_codeGenProfiler != null) == Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag),
        "Profiler tag is supplied but the profiler pointer is NULL");
    if (this->m_codeGenProfiler)
    {        
        this->m_codeGenProfiler->ProfileBegin(tag);
    }
#endif
}

///----------------------------------------------------------------------------
///
/// Func::EndPhase
///
/// Takes care of the profiler and dumper
///
///----------------------------------------------------------------------------

void

Func::EndProfiler(Js::Phase tag)
{
#ifdef PROFILE_EXEC
    AssertMsg((this->m_codeGenProfiler != null) == Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag),
        "Profiler tag is supplied but the profiler pointer is NULL");
    if (this->m_codeGenProfiler)
    {
        this->m_codeGenProfiler->ProfileEnd(tag);
    }
#endif
}
void
Func::EndPhase(Js::Phase tag, bool dump)
{
    this->EndProfiler(tag);
#if DBG_DUMP
    if(dump && (PHASE_DUMP(tag, this) 
        || PHASE_DUMP(Js::BackEndPhase, this)))
    {
        Output::Print(L"-----------------------------------------------------------------------------\n");

        if (m_workItem->Type() == JsLoopBodyWorkItemType)
        {            
            Output::Print(L"************   IR after %s (%S) Loop %d ************\n", Js::PhaseNames[tag], ExecutionModeName(m_workItem->GetJitMode()), ((JsLoopBodyCodeGen*)m_workItem)->GetLoopNumber());
        }
        else
        {
            Output::Print(L"************   IR after %s (%S)  ************\n", Js::PhaseNames[tag], ExecutionModeName(m_workItem->GetJitMode()));
        }
        this->Dump(Js::Configuration::Global.flags.AsmDiff? IRDumpFlags_AsmDumpMode : IRDumpFlags_None);
    }
#endif


#if DBG
    if (tag == Js::LowererPhase)
    {
        Assert(!this->isPostLower);
        this->isPostLower = true;
    }
    else if (tag == Js::RegAllocPhase)
    {
        Assert(!this->isPostRegAlloc);
        this->isPostRegAlloc = true;
    }
    else if (tag == Js::PeepsPhase)
    {
        Assert(this->isPostLower && !this->isPostLayout);
        this->isPostPeeps = true;
    }
    else if (tag == Js::LayoutPhase)
    {
        Assert(this->isPostPeeps && !this->isPostLayout);
        this->isPostLayout = true;
    }
    else if (tag == Js::FinalLowerPhase)
    {
        Assert(this->isPostLayout && !this->isPostFinalLower);
        this->isPostFinalLower = true;
    }
    if (this->isPostLower)
    {
#ifndef _M_ARM    // Need to verify ARM is clean.
        DbCheckPostLower dbCheck(this);

        dbCheck.Check();
#endif
    }
#endif

}

Func const * 
Func::GetTopFunc() const
{
    Func const * func = this;
    while (!func->IsTopFunc())
    {
        func = func->parentFunc;
    }
    return func;
} 
Func * 
Func::GetTopFunc() 
{
    Func * func = this;
    while (!func->IsTopFunc())
    {
        func = func->parentFunc;
    }
    return func;
}

#if 0
FunctionBailOutRecord * 
Func::EnsureFunctionBailOutRecord()
{
    Js::FunctionBody * topFuncBody = this->GetTopFunc()->m_jnFunction;
    if (topFuncBody->HasFunctionBailOutRecord())
    {
        return topFuncBody->GetFunctionBailOutRecord();
    }
    FunctionBailOutRecord * record = JitAnew(this->GetScriptContext()->GetNativeCodeGenerator()->GetAllocator(),
        FunctionBailOutRecord);
    topFuncBody->SetFunctionBailOutRecord(record);
    return record;
}
#endif

StackSym *
Func::EnsureLoopParamSym()
{
    if (this->m_loopParamSym == NULL)
    {
        if (this->m_localClosureSym)
        {
            // In jitted loop body with stack closure, we need to access the loop param sym in the body of the function,
            // so we need a non-temp. Just re-use the stack closure sym, which won't be used in the function body.
            return this->m_localClosureSym;
        }
        this->m_loopParamSym = StackSym::New(TyMachPtr, this);
    }
    return this->m_loopParamSym;
}

void
Func::UpdateMaxInlineeArgOutCount(uint inlineeArgOutCount)
{
    if (maxInlineeArgOutCount < inlineeArgOutCount)
    {
        maxInlineeArgOutCount = inlineeArgOutCount;
    }
}

void
Func::BeginClone(Lowerer * lowerer, JitArenaAllocator *alloc)
{
    Assert(this->IsTopFunc());
    AssertMsg(m_cloner == NULL, "Starting new clone while one is in progress");
    m_cloner = JitAnew(alloc, Cloner, lowerer, alloc);
    if (m_cloneMap == NULL)
    {
         m_cloneMap = JitAnew(alloc, InstrMap, alloc, 7);
    }
}

void
Func::EndClone()
{
    Assert(this->IsTopFunc());
    if (m_cloner)
    {
        m_cloner->Finish();
        JitAdelete(m_cloner->alloc, m_cloner);
        m_cloner = NULL;
    }
}

IR::SymOpnd *
Func::GetInlineeOpndAtOffset(int32 offset)
{
    Assert(IsInlinee());

    StackSym *stackSym = CreateInlineeStackSym();
    this->SetArgOffset(stackSym, stackSym->m_offset + offset);
    Assert(stackSym->m_offset >= 0);

    return IR::SymOpnd::New(stackSym, 0, TyMachReg, this);
}

StackSym *
Func::CreateInlineeStackSym()
{
    // Make sure this is an inlinee and that GlobOpt has initialized the offset
    // in the inlinee's frame.
    Assert(IsInlinee());
    Assert(m_inlineeFrameStartSym->m_offset != -1);

    StackSym *stackSym = m_symTable->GetArgSlotSym((Js::ArgSlot)-1);
    stackSym->m_isInlinedArgSlot = true;
    stackSym->m_offset = m_inlineeFrameStartSym->m_offset;
    stackSym->m_allocated = true;

    return stackSym;
}

uint8 *
Func::GetCallsCountAddress() const
{
    Assert(this->m_workItem->Type() == JsFunctionType);

    JsFunctionCodeGen * functionCodeGen = static_cast<JsFunctionCodeGen *>(this->m_workItem);
    
    return functionCodeGen->GetFunctionBody()->GetCallsCountAddress(functionCodeGen->GetEntryPoint());
}

RecyclerWeakReference<Js::FunctionBody> *
Func::GetWeakFuncRef() const
{
    if (this->m_jitTimeData == null)
    {
        return null;
    }

    return this->m_jitTimeData->GetWeakFuncRef();
}

Js::InlineCache *
Func::GetRuntimeInlineCache(const uint index) const
{
    if(this->m_runtimeData)
    {
        const auto inlineCache = this->m_runtimeData->ClonedInlineCaches()->GetInlineCache(this->m_jnFunction, index);
        if(inlineCache)
        {
            return inlineCache;
        }
    }

    return this->m_jnFunction->GetInlineCache(index);
}

Js::PolymorphicInlineCache *
Func::GetRuntimePolymorphicInlineCache(const uint index) const
{
    if (this->m_polymorphicInlineCacheInfo)
    {
        return this->m_polymorphicInlineCacheInfo->GetPolymorphicInlineCaches()->GetInlineCache(this->m_jnFunction, index);
    }
    return null;
}

byte 
Func::GetPolyCacheUtilToInitialize(const uint index) const
{
    return this->GetRuntimePolymorphicInlineCache(index) ? this->GetPolyCacheUtil(index) : PolymorphicInlineCacheUtilizationMinValue;
}

byte
Func::GetPolyCacheUtil(const uint index) const
{
    return this->m_polymorphicInlineCacheInfo->GetUtilArray()->GetUtil(this->m_jnFunction, index);
}

Js::ObjTypeSpecFldInfo*
Func::GetObjTypeSpecFldInfo(const uint index) const
{
    if (this->m_jitTimeData == null)
    {
        return null;
    }
    Assert(this->m_jitTimeData->GetObjTypeSpecFldInfoArray());

    return this->m_jitTimeData->GetObjTypeSpecFldInfoArray()->GetInfo(this->m_jnFunction, index);
}

Js::ObjTypeSpecFldInfo*
Func::GetGlobalObjTypeSpecFldInfo(uint propertyInfoId) const
{
    Assert(this->m_jitTimeData != null);
    return this->m_jitTimeData->GetGlobalObjTypeSpecFldInfo(propertyInfoId);
}

void
Func::SetGlobalObjTypeSpecFldInfo(uint propertyInfoId, Js::ObjTypeSpecFldInfo* info)
{
    Assert(this->m_jitTimeData != null);
    this->m_jitTimeData->SetGlobalObjTypeSpecFldInfo(propertyInfoId, info);
}

void
Func::EnsurePinnedTypeRefs()
{
    if (this->pinnedTypeRefs == null)
    {
        this->pinnedTypeRefs = JitAnew(this->m_alloc, TypeRefSet, this->m_alloc);
    }
}

void
Func::PinTypeRef(void* typeRef)
{
    EnsurePinnedTypeRefs();
    this->pinnedTypeRefs->AddNew(typeRef);
}

void 
Func::EnsureSingleTypeGuards()
{
    if (this->singleTypeGuards == null)
    {
        this->singleTypeGuards = JitAnew(this->m_alloc, TypePropertyGuardDictionary, this->m_alloc);
    }
}

Js::JitTypePropertyGuard* 
Func::GetOrCreateSingleTypeGuard(Js::Type* type)
{
    EnsureSingleTypeGuards();

    Js::JitTypePropertyGuard* guard;
    if (!this->singleTypeGuards->TryGetValue(type, &guard))
    {
        // Property guards are allocated by NativeCodeData::Allocator so that their lifetime extends as long as the EntryPointInfo is alive.
        guard = NativeCodeDataNew(GetNativeCodeDataAllocator(), Js::JitTypePropertyGuard, type, this->indexedPropertyGuardCount++);
        this->singleTypeGuards->Add(type, guard);
    }
    else
    {
        Assert(guard->GetType() == type);
    }

    return guard;
}

void 
Func::EnsureEquivalentTypeGuards()
{
    if (this->equivalentTypeGuards == null)
    {
        this->equivalentTypeGuards = JitAnew(this->m_alloc, EquivalentTypeGuardList, this->m_alloc);
    }
}

Js::JitEquivalentTypeGuard* 
Func::CreateEquivalentTypeGuard(Js::Type* type, uint32 objTypeSpecFldId)
{
    EnsureEquivalentTypeGuards();

    Js::JitEquivalentTypeGuard* guard = NativeCodeDataNew(GetNativeCodeDataAllocator(), Js::JitEquivalentTypeGuard, type, this->indexedPropertyGuardCount++, objTypeSpecFldId);
    // If we want to hard code the address of the cache, we will need to go back to allocating it from the native code data allocator.
    // We would then need to maintain consistency (double write) to both the recycler allocated cache and the one on the heap.
    Js::EquivalentTypeCache* cache = NativeCodeDataNewZ(GetTransferDataAllocator(), Js::EquivalentTypeCache);
    guard->SetCache(cache);
    // Give the cache a back-pointer to the guard so that the guard can be cleared at runtime if necessary.
    cache->SetGuard(guard);
    this->equivalentTypeGuards->Prepend(guard);

    return guard;
}

void
Func::EnsurePropertyGuardsByPropertyId()
{
    if (this->propertyGuardsByPropertyId == null)
    {
        this->propertyGuardsByPropertyId = JitAnew(this->m_alloc, PropertyGuardByPropertyIdMap, this->m_alloc);
    }
}

void
Func::EnsureCtorCachesByPropertyId()
{
    if (this->ctorCachesByPropertyId == null)
    {
        this->ctorCachesByPropertyId = JitAnew(this->m_alloc, CtorCachesByPropertyIdMap, this->m_alloc);
    }
}

void 
Func::LinkGuardToPropertyId(Js::PropertyId propertyId, Js::JitIndexedPropertyGuard* guard)
{
    Assert(guard != null);
    Assert(guard->GetValue() != null);

    Assert(this->propertyGuardsByPropertyId != null);

    IndexedPropertyGuardSet* set;
    if (!this->propertyGuardsByPropertyId->TryGetValue(propertyId, &set))
    {
        set = JitAnew(this->m_alloc, IndexedPropertyGuardSet, this->m_alloc);
        this->propertyGuardsByPropertyId->Add(propertyId, set);
    }

    set->Item(guard);
}

void 
Func::LinkCtorCacheToPropertyId(Js::PropertyId propertyId, Js::JitTimeConstructorCache* cache)
{
    Assert(cache != null);
    Assert(this->ctorCachesByPropertyId != null);

    CtorCacheSet* set;
    if (!this->ctorCachesByPropertyId->TryGetValue(propertyId, &set))
    {
        set = JitAnew(this->m_alloc, CtorCacheSet, this->m_alloc);
        this->ctorCachesByPropertyId->Add(propertyId, set);
    }

    set->Item(cache->runtimeCache);
}

Js::JitTimeConstructorCache* Func::GetConstructorCache(const Js::ProfileId profiledCallSiteId)
{
    Assert(GetJnFunction() != null);
    Assert(profiledCallSiteId < GetJnFunction()->GetProfiledCallSiteCount());
    Assert(this->constructorCaches != null);
    return this->constructorCaches[profiledCallSiteId];
}

void Func::SetConstructorCache(const Js::ProfileId profiledCallSiteId, Js::JitTimeConstructorCache* constructorCache)
{
    const auto functionBody = this->GetJnFunction();
    Assert(functionBody != null);
    Assert(profiledCallSiteId < functionBody->GetProfiledCallSiteCount());
    Assert(constructorCache != null);
    Assert(this->constructorCaches != null);
    Assert(this->constructorCaches[profiledCallSiteId] == null);
    this->constructorCacheCount++;
    this->constructorCaches[profiledCallSiteId] = constructorCache;
}

void Func::EnsurePropertiesWrittenTo()
{
    if (this->propertiesWrittenTo == null)
    {
        this->propertiesWrittenTo = JitAnew(this->m_alloc, PropertyIdSet, this->m_alloc);
    }
}

void Func::EnsureCallSiteToArgumentsOffsetFixupMap()
{
    if (this->callSiteToArgumentsOffsetFixupMap == null)
    {
        this->callSiteToArgumentsOffsetFixupMap = JitAnew(this->m_alloc, CallSiteToArgumentsOffsetFixupMap, this->m_alloc);
    }
}

void
Cloner::AddInstr(IR::Instr * instrOrig, IR::Instr * instrClone)
{
    if (!this->instrFirst)
    {
        this->instrFirst = instrClone;
    }
    this->instrLast = instrClone;
}

void
Cloner::Finish()
{
    this->RetargetClonedBranches();
    if (this->lowerer)
    {
        lowerer->LowerRange(this->instrFirst, this->instrLast, false, false);
    }
}

void
Cloner::RetargetClonedBranches()
{
    if (!this->fRetargetClonedBranch)
    {
        return;
    }

    FOREACH_INSTR_IN_RANGE(instr, this->instrFirst, this->instrLast)
    {
        if (instr->IsBranchInstr())
        {
            instr->AsBranchInstr()->RetargetClonedBranch();
        }
    }
    NEXT_INSTR_IN_RANGE;
}

void Func::ThrowIfScriptClosed()
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    if(scriptContext->IsClosed())
    {
        // Should not be jitting something in the foreground when the script context is actually closed
        Assert(IsBackgroundJIT() || !scriptContext->IsActuallyClosed());

        throw Js::OperationAbortedException();
    }
}

IR::IndirOpnd * Func::GetConstantAddressIndirOpnd(void * address, IR::AddrOpndKind kind, IRType type, Js::OpCode loadOpCode)
{
    Assert(this->GetTopFunc() == this);    
    if (!canHoistConstantAddressLoad)
    {
        // We can't hoist constant address load after lower, as we can't mark the sym as
        // live on back edge
        return nullptr;
    }    
    int offset = 0;
    IR::RegOpnd ** foundRegOpnd = this->constantAddressRegOpnd.Find([address, &offset](IR::RegOpnd * regOpnd)
    {
        Assert(regOpnd->m_sym->IsSingleDef());
        void * curr = regOpnd->m_sym->m_instrDef->GetSrc1()->AsAddrOpnd()->m_address;
        ptrdiff_t diff = (intptr_t)address - (intptr_t)curr;
        if (!Math::FitsInDWord(diff))
        {
            return false;
        }

        offset = (int)diff;
        return true;
    });

    IR::RegOpnd * addressRegOpnd;
    if (foundRegOpnd != nullptr)
    {
        addressRegOpnd = *foundRegOpnd;              
    }
    else
    {
        Assert(offset == 0);
        addressRegOpnd = IR::RegOpnd::New(TyMachPtr, this);
        IR::Instr *const newInstr =
            IR::Instr::New(
            loadOpCode,
            addressRegOpnd,
            IR::AddrOpnd::New(address, kind, this, true),
            this);
        this->constantAddressRegOpnd.Prepend(addressRegOpnd);

        IR::Instr * insertBeforeInstr = this->lastConstantAddressRegLoadInstr;
        if (insertBeforeInstr == nullptr)
        {
            insertBeforeInstr = this->GetFunctionEntryInsertionPoint();
            this->lastConstantAddressRegLoadInstr = newInstr;
        }
        insertBeforeInstr->InsertBefore(newInstr);
    }
    IR::IndirOpnd * indirOpnd =  IR::IndirOpnd::New(addressRegOpnd, offset, type, this, true);    
#if DBG_DUMP
    indirOpnd->SetAddrKind(kind, address);
#endif
    return indirOpnd;
}

void Func::MarkConstantAddressSyms(BVSparse<JitArenaAllocator> * bv)
{
    Assert(this->GetTopFunc() == this);
    this->constantAddressRegOpnd.Iterate([bv](IR::RegOpnd * regOpnd)
    {
        bv->Set(regOpnd->m_sym->m_id);
    });
}


IR::Instr *
Func::GetFunctionEntryInsertionPoint()
{    
    Assert(this->GetTopFunc() == this);
    IR::Instr * insertInsert = this->lastConstantAddressRegLoadInstr;
    if (insertInsert != nullptr)
    {
        return insertInsert->m_next;
    }

    insertInsert = this->m_headInstr;

    // TODO: SCCLiveness can assume we are in the root region from FunctionEntry without a label
    // then we won't need this
    if (this->HasTry())
    {
        // Insert it inside the root region
        insertInsert = insertInsert->m_next;
        Assert(insertInsert->IsLabelInstr() && insertInsert->AsLabelInstr()->GetRegion()->GetType() == RegionTypeRoot);
    }

    return insertInsert->m_next;
}
#if DBG_DUMP

///----------------------------------------------------------------------------
///
/// Func::DumpHeader
///
///----------------------------------------------------------------------------

void
Func::DumpHeader()
{
    Output::Print(L"-----------------------------------------------------------------------------\n");
    this->m_jnFunction->DumpFullFunctionName();

    Output::SkipToColumn(50);
    Output::Print(L"Instr Count:%d", GetInstrCount());

    if(m_codeSize > 0)
    {
        Output::Print(L"\t\tSize:%d\n\n", m_codeSize);
    }
    else
    {
        Output::Print(L"\n\n");
    }
}

///----------------------------------------------------------------------------
///
/// Func::Dump
///
///----------------------------------------------------------------------------

void 
Func::Dump(IRDumpFlags flags)
{
    this->DumpHeader();

    FOREACH_INSTR_IN_FUNC(instr, this)
    {
        instr->DumpGlobOptInstrString();
        instr->Dump(flags);
    }NEXT_INSTR_IN_FUNC;

    Output::Flush();
}

void
Func::Dump()
{
    this->Dump(IRDumpFlags_None);
}

#endif

#if DBG_DUMP || defined(ENABLE_IR_VIEWER)
LPCSTR
Func::GetVtableName(INT_PTR address)
{
#if DBG
    if (vtableMap == null)
    {
        vtableMap = VirtualTableRegistry::CreateVtableHashMap(this->m_alloc);
    };
    LPCSTR name = vtableMap->Lookup(address);
    if (name)
    {
         if (strncmp(name, "class ", _countof("class ") - 1) == 0)
         {
             name += _countof("class ") - 1;
         }
    }
    return name;
#else
    return "";
#endif
}
#endif


#if DBG_DUMP | defined(VTUNE_PROFILING)
bool Func::DoRecordNativeMap() const
{
#if defined(VTUNE_PROFILING)
    if (EtwTrace::isJitProfilingActive)
    {
        return true;
    }
#endif
#if DBG_DUMP
    return PHASE_DUMP(Js::EncoderPhase, this) && Js::Configuration::Global.flags.Verbose;
#else 
    return false;
#endif
}
#endif