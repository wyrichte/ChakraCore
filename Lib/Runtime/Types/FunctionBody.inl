// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Js {

    // same as MachDouble, used in the Func.h
#define DIAGLOCALSLOTSIZE       8

    inline uint
    ParseableFunctionInfo::GetSourceIndex() const
    {
        return this->m_sourceIndex;
    }

    inline LPCUTF8
    ParseableFunctionInfo::GetSource(const wchar_t* reason) const
    {
        return this->m_utf8SourceInfo->GetSource(reason == nullptr ? L"ParseableFunctionInfo::GetSource" : reason) + this->StartOffset();
    }

    inline LPCUTF8
    ParseableFunctionInfo::GetStartOfDocument(const wchar_t* reason) const
    {
        return this->m_utf8SourceInfo->GetSource(reason == nullptr ? L"ParseableFunctionInfo::GetStartOfDocument" : reason);
    }

    inline charcount_t
    ParseableFunctionInfo::LengthInChars() const
    {
        return this->m_cchLength;
    }

    inline bool
    ParseableFunctionInfo::IsEval() const
    {
        return this->m_isEval;
    }

    inline bool
    ParseableFunctionInfo::IsDynamicFunction() const
    {
        return this->m_isDynamicFunction;
    }

    inline bool
    ParseableFunctionInfo::IsDynamicScript() const
    {
        return this->GetSourceContextInfo()->IsDynamic();
    }

    inline size_t
    ParseableFunctionInfo::LengthInBytes() const
    {
        return this->m_cbLength;
    }

    inline charcount_t
    ParseableFunctionInfo::StartInDocument() const
    {
        return this->m_cchStartOffset;
    }

    // TODO: Add ParseableFunctionInfo::StartOffset
    inline size_t
    ParseableFunctionInfo::StartOffset() const
    {
        return this->m_cbStartOffset;
    }

    // Given an offset into the source buffer, determine if the end of this SourceInfo
    // lies after the given offset.
    inline bool
    FunctionBody::EndsAfter(size_t offset) const
    {
        return offset < this->StartOffset() + this->LengthInBytes();
    }

    inline void
    FunctionBody::RecordStatementMap(StatementMap* pStatementMap)
    {
        Assert(!this->m_sourceInfo.pSpanSequence);
        Recycler* recycler = this->m_scriptContext->GetRecycler();
        StatementMapList * statementMaps = this->GetStatementMaps();
        if (!statementMaps)
        {
            statementMaps = RecyclerNew(recycler, StatementMapList, recycler);
            this->pStatementMaps = statementMaps;
        }

        statementMaps->Add(pStatementMap);
    }

    inline void
    FunctionBody::RecordStatementMap(SmallSpanSequenceIter &iter, StatementData * data)
    {
        Assert(!this->GetStatementMaps());

        if (!this->m_sourceInfo.pSpanSequence)
        {
            this->m_sourceInfo.pSpanSequence = HeapNew(SmallSpanSequence);
        }

        this->m_sourceInfo.pSpanSequence->RecordARange(iter, data);
    }

    inline void
    FunctionBody::RecordStatementAdjustment(uint offset, StatementAdjustmentType adjType)
    {
        this->EnsureAuxStatementData();

        Recycler* recycler = this->m_scriptContext->GetRecycler();
        if (this->GetStatementAdjustmentRecords() == NULL)
        {
            m_sourceInfo.m_auxStatementData->m_statementAdjustmentRecords = RecyclerNew(recycler, StatementAdjustmentRecordList, recycler);
        }

        StatementAdjustmentRecord record(adjType, offset);
        this->GetStatementAdjustmentRecords()->Add(record); // Will copy stack value and put the copy into the container.
    }

    inline BOOL
    FunctionBody::GetBranchOffsetWithin(uint start, uint end, __out StatementAdjustmentRecord* record)
    {
        Assert(start < end);

        if (!this->GetStatementAdjustmentRecords())
        {
            // No Offset
            return FALSE;
        }

        int count = this->GetStatementAdjustmentRecords()->Count();
        for (int i = 0; i < count; i++)
        {
            StatementAdjustmentRecord item = this->GetStatementAdjustmentRecords()->Item(i);
            if (item.GetByteCodeOffset() > start && item.GetByteCodeOffset() < end)
            {
                *record = item;
                return TRUE;
            }
        }

        // No offset found in the range.
        return FALSE;
    }

    inline ScriptContext* EntryPointInfo::GetScriptContext()
    {
        Assert(!IsCleanedUp());
        return this->library->GetScriptContext();
    }

#if DBG_DUMP | defined(VTUNE_PROFILING)
    inline void
    EntryPointInfo::RecordNativeMap(uint32 nativeOffset, uint32 statementIndex)
    {
        int count = nativeOffsetMaps.Count();
        if (count)
        {
            NativeOffsetMap* previous = &nativeOffsetMaps.Item(count-1);
            // Check if the range is still not finished.
            if (previous->nativeOffsetSpan.begin == previous->nativeOffsetSpan.end)
            {
                if (previous->statementIndex == statementIndex)
                {
                    // If the statement index is the same, we can continue with the previous range
                    return;
                }

                // If the range is empty, replace the previous range.
                if ((uint32)previous->nativeOffsetSpan.begin == nativeOffset)
                {
                    if (statementIndex == Js::Constants::NoStatementIndex)
                    {
                        nativeOffsetMaps.RemoveAtEnd();
                    }
                    else
                    {
                        previous->statementIndex = statementIndex;
                    }
                    return;
                }

                // Close the previous range
                previous->nativeOffsetSpan.end = nativeOffset;
            }
        }

        if (statementIndex == Js::Constants::NoStatementIndex)
        {
            // We do not explicitly record the offsets that do not map to user code.
            return;
        }

        NativeOffsetMap map;
        map.statementIndex = statementIndex;
        map.nativeOffsetSpan.begin = nativeOffset;
        map.nativeOffsetSpan.end = nativeOffset;

        nativeOffsetMaps.Add(map);
    }

#endif

    inline void
    FunctionBody::CopySourceInfo(ParseableFunctionInfo* originalFunctionInfo)
    {
        this->m_sourceIndex = originalFunctionInfo->GetSourceIndex();
        this->m_cchStartOffset = originalFunctionInfo->StartInDocument();
        this->m_cchLength = originalFunctionInfo->LengthInChars();
        this->m_lineNumber = originalFunctionInfo->GetRelativeLineNumber();
        this->m_columnNumber = originalFunctionInfo->GetRelativeColumnNumber();
        this->m_isEval = originalFunctionInfo->IsEval();
        this->m_isDynamicFunction = originalFunctionInfo->IsDynamicFunction();
        this->m_cbStartOffset = originalFunctionInfo->StartOffset();
        this->m_cbLength = originalFunctionInfo->LengthInBytes();

        this->FinishSourceInfo();
    }

    // When sourceInfo is complete, register this functionBody to utf8SourceInfo. This ensures we never
    // put incomplete functionBody into utf8SourceInfo map. (Previously we do it in FunctionBody constructor.
    // If an error occurs thereafter before SetSourceInfo, e.g. OOM, we'll have an incomplete functionBody
    // in utf8SourceInfo map whose source range is unknown and can't be reparsed.)
    inline void FunctionBody::FinishSourceInfo()
    {
        m_utf8SourceInfo->SetFunctionBody(this);
    }

    inline RegSlot FunctionBody::GetFrameDisplayRegister() const
    {
        return this->m_sourceInfo.frameDisplayRegister;
    }

    inline void FunctionBody::SetFrameDisplayRegister(RegSlot frameDisplayRegister)
    {
        this->m_sourceInfo.frameDisplayRegister = frameDisplayRegister;
    }

    inline RegSlot FunctionBody::GetObjectRegister() const
    {
        return this->m_sourceInfo.objectRegister;
    }

    inline void FunctionBody::SetObjectRegister(RegSlot objectRegister)
    {
        this->m_sourceInfo.objectRegister = objectRegister;
    }

    inline ScopeObjectChain *FunctionBody::GetScopeObjectChain() const
    {
        return this->m_sourceInfo.pScopeObjectChain;
    }

    inline void FunctionBody::SetScopeObjectChain(ScopeObjectChain *pScopeObjectChain)
    {
        this->m_sourceInfo.pScopeObjectChain = pScopeObjectChain;
    }

    inline ByteBlock *FunctionBody::GetProbeBackingBlock()
    {
        return this->m_sourceInfo.m_probeBackingBlock;
    }

    inline void FunctionBody::SetProbeBackingBlock(ByteBlock* probeBackingBlock)
    {
        this->m_sourceInfo.m_probeBackingBlock = probeBackingBlock;
    }

    inline
        FunctionBody * FunctionBody::NewFromRecycler(ScriptContext * scriptContext, const wchar_t * displayName, uint displayNameLength, uint nestedCount,
        Utf8SourceInfo* sourceInfo, uint uScriptId, Js::LocalFunctionId functionId, Js::PropertyRecordList* boundPropertyRecords, Attributes attributes
#ifdef PERF_COUNTERS
            , bool isDeserializedFunction
#endif
            )
    {
            return FunctionBody::NewFromRecycler(scriptContext, displayName, displayNameLength, nestedCount, sourceInfo,
            scriptContext->GetThreadContext()->NewFunctionNumber(), uScriptId, functionId, boundPropertyRecords, attributes
#ifdef PERF_COUNTERS
            , isDeserializedFunction
#endif
            );
    }

    inline
        FunctionBody * FunctionBody::NewFromRecycler(ScriptContext * scriptContext, const wchar_t * displayName, uint displayNameLength, uint nestedCount,
        Utf8SourceInfo* sourceInfo, uint uFunctionNumber, uint uScriptId, Js::LocalFunctionId  functionId, Js::PropertyRecordList* boundPropertyRecords, Attributes attributes
#ifdef PERF_COUNTERS
            , bool isDeserializedFunction
#endif
            )
    {
#ifdef PERF_COUNTERS
            return RecyclerNewWithBarrierFinalizedPlus(scriptContext->GetRecycler(), nestedCount * sizeof(FunctionBody*), FunctionBody, scriptContext, displayName, displayNameLength, nestedCount, sourceInfo, uFunctionNumber, uScriptId, functionId, boundPropertyRecords, attributes, isDeserializedFunction);
#else
            return RecyclerNewWithBarrierFinalizedPlus(scriptContext->GetRecycler(), nestedCount * sizeof(FunctionBody*), FunctionBody, scriptContext, displayName, displayNameLength, nestedCount, sourceInfo, uFunctionNumber, uScriptId, functionId, boundPropertyRecords, attributes);
#endif
    }

    inline
        FunctionBody::FunctionBody(ScriptContext* scriptContext, const wchar_t* displayName, uint displayNameLength, uint nestedCount,
    Utf8SourceInfo* utf8SourceInfo, uint uFunctionNumber, uint uScriptId,
    Js::LocalFunctionId  functionId, Js::PropertyRecordList* boundPropertyRecords, Attributes attributes
#ifdef PERF_COUNTERS
    , bool isDeserializedFunction
#endif
    ) :
    ParseableFunctionInfo(scriptContext->CurrentThunk, nestedCount, sizeof(FunctionBody), functionId, utf8SourceInfo, scriptContext, uFunctionNumber, displayName, displayNameLength, attributes, boundPropertyRecords),
    m_uScriptId(uScriptId),
    m_varCount(0),
    m_outParamMaxDepth(0),
    m_firstTmpReg(Constants::NoRegister),
    loopCount(0),
    cleanedUp(false),
    sourceInfoCleanedUp(false),
    profiledLdElemCount(0),
    profiledStElemCount(0),
    profiledCallSiteCount(0),
    profiledArrayCallSiteCount(0),
    profiledDivOrRemCount(0),
    profiledSwitchCount(0),
    profiledReturnTypeCount(0),
    profiledSlotCount(0),
    m_isFuncRegistered(false),
    m_isFuncRegisteredToDiag(false),
    m_hasBailoutInstrInJittedCode(false),
    m_depth(0),
    inlineDepth(0),
    m_pendingLoopHeaderRelease(false),
    inlineCacheCount(0),
    rootObjectLoadInlineCacheStart(0),
    rootObjectStoreInlineCacheStart(0),
    isInstInlineCacheCount(0),
    objLiteralCount(0),
    literalRegexCount(0),
    m_byteCodeCount(0),
    m_byteCodeWithoutLDACount(0),
    m_argUsedForBranch(0),
    m_byteCodeInLoopCount(0),
    m_envDepth((uint16)-1),
    flags(Flags_HasNoExplicitReturnValue),
    m_hasFinally(false),
    dynamicProfileInfo(null),
    polymorphicCallSiteInfoHead(null),
    savedInlinerVersion(0),
    savedImplicitCallsFlags(ImplicitCall_HasNoInfo),
    savedPolymorphicCacheState(0),
    functionBailOutRecord(null),
    hasExecutionDynamicProfileInfo(false),
    m_hasAllNonLocalReferenced(false),
    m_hasSetIsObject(false),
    m_hasFunExprNameReference(false),
    m_CallsEval(false),
    m_ChildCallsEval(false),
    m_hasReferenceableBuiltInArguments(false),
    m_firstFunctionObject(true),
    m_inlineCachesOnFunctionObject(false),
    m_hasDoneAllNonLocalReferenced(false),
    m_hasFunctionCompiledSent(false),
    byteCodeCache(nullptr),
    stackNestedFuncParent(null),
    stackClosureRegister(Constants::NoRegister),
    m_tag(TRUE),
    m_nativeEntryPointUsed(FALSE),
    debuggerScopeIndex(0),
    bailOnMisingProfileCount(0),
    bailOnMisingProfileRejitCount(0),
    auxBlock(NULL),
    auxContextBlock(NULL),
    byteCodeBlock(NULL),
    entryPoints(NULL),
    loopHeaderArray(NULL),
    m_constTable(NULL),
    literalRegexes(NULL),
    asmJsFunctionInfo(NULL),
    asmJsModuleInfo(NULL),
    m_codeGenRuntimeData(NULL),
    m_codeGenGetSetRuntimeData(NULL),
    pStatementMaps(null),
    inlineCaches(NULL),
    polymorphicInlineCachesHead(NULL),
    cacheIdToPropertyIdMap(null),
    referencedPropertyIdMap(null),
    propertyIdsForScopeSlotArray(NULL),
    propertyIdOnRegSlotsContainer(NULL),
    executionMode(ExecutionMode::Interpreter),
    interpreterLimit(0),
    autoProfilingInterpreter0Limit(0),
    profilingInterpreter0Limit(0),
    autoProfilingInterpreter1Limit(0),
    simpleJitLimit(0),
    profilingInterpreter1Limit(0),
    fullJitThreshold(0),
    fullJitRequeueThreshold(0),
    committedProfiledIterations(0),
    simpleJitEntryPointInfo(null),
    wasCalledFromLoop(false),
    hasNestedLoop(false),
    recentlyBailedOutOfJittedLoopBody(false),
    serializationIndex(-1),
    m_isAsmJsScheduledForFullJIT(false),
    m_isAsmJsFunction(false),
    m_asmJsTotalLoopCount(0),

        //
        // Even if the function does not require any locals, we must always have "R0" to propagate
        // a return value.  By enabling this here, we avoid unnecessary conditionals during execution.
        //
        m_constCount(1)
#ifdef IR_VIEWER
        ,m_isIRDumpEnabled(false)
        ,m_irDumpBaseObject(NULL)
#endif /* IR_VIEWER */
        ,m_isFromNativeCodeModule(false)
#ifdef BODLOG
        ,callCount(0)
#endif
        , interpretedCount(0)
        , loopInterpreterLimit(CONFIG_FLAG(LoopInterpretCount))
        , hasHotLoop(false)
        , m_isPartialDeserializedFunction(false)
#ifdef PERF_COUNTERS
        , m_isDeserializedFunction(isDeserializedFunction)
#endif
#if DBG
        , m_DEBUG_executionCount(0)
        , m_nativeEntryPointIsInterpreterThunk(false)
        , m_canDoStackNestedFunc(false)
        , m_inlineCacheTypes(null)
        , m_iProfileSession(-1)
        , initializedExecutionModeAndLimits(false)
#endif
#if ENABLE_DEBUG_CONFIG_OPTIONS
        , regAllocLoadCount(0)
        , regAllocStoreCount(0)
        , callCountStats(0)
#endif
    {
        this->SetDefaultFunctionEntryPointInfo((FunctionEntryPointInfo*) this->GetDefaultEntryPointInfo(), DefaultEntryThunk);
        this->m_hasBeenParsed = true;

#ifdef PERF_COUNTERS
        if (isDeserializedFunction)
        {
            PERF_COUNTER_INC(Code, DeserializedFunctionBody);
        }
#endif
        Assert(!utf8SourceInfo || m_uScriptId == utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId);

        // Sync entryPoints changes to etw rundown lock
        CriticalSection* syncObj = scriptContext->GetThreadContext()->GetEtwRundownCriticalSection();
        this->entryPoints = RecyclerNew(this->m_scriptContext->GetRecycler(), FunctionEntryPointList, this->m_scriptContext->GetRecycler(), syncObj);

        this->AddEntryPointToEntryPointList(this->GetDefaultFunctionEntryPointInfo());

        Assert(this->GetDefaultEntryPointInfo()->address != NULL);

        InitDisableInlineApply();
        InitDisableInlineSpread();
    }

    inline void FunctionBody::SetDefaultFunctionEntryPointInfo(FunctionEntryPointInfo* entryPointInfo, const JavascriptMethod originalEntryPoint)
    {
        Assert(entryPointInfo);

        // Need to set twice since ProxyEntryPointInfo cast points to an interior pointer
        this->m_defaultEntryPointInfo = (ProxyEntryPointInfo*) entryPointInfo;
        this->defaultFunctionEntryPointInfo = entryPointInfo;
        SetOriginalEntryPoint(originalEntryPoint);
    }

    inline ByteBlock*
    FunctionBody::GetAuxiliaryData()
    {
        return this->auxBlock;
    }

    inline ByteBlock*
    FunctionBody::GetAuxiliaryContextData()
    {
        return this->auxContextBlock;
    }

    inline ByteBlock*
    FunctionBody::GetByteCode()
    {
        return this->byteCodeBlock;
    }

    // Returns original bytecode without probes (such as BPs).
    inline ByteBlock*
    FunctionBody::GetOriginalByteCode()
    {
        if (m_sourceInfo.m_probeBackingBlock)
        {
            return m_sourceInfo.m_probeBackingBlock;
        }
        else
        {
            return this->GetByteCode();
        }
    }

    inline const ByteCodeCache *
    FunctionBody::GetByteCodeCache() const
    {
        return byteCodeCache;
    }

    inline const int
    FunctionBody::GetSerializationIndex() const
    {
        return serializationIndex;
    }

    //
    // Algorithm to retrieve a function body's external display name. Template supports both
    // local FunctionBody and ScriptDAC (debugging) scenarios.
    //
    template <class T>
    static const wchar_t* ParseableFunctionInfo::GetExternalDisplayName(const T* funcBody)
    {
        Assert(funcBody != nullptr);
        Assert(funcBody->GetDisplayName() != nullptr);

        return funcBody->GetDisplayName();
    }

    inline const wchar_t* ParseableFunctionInfo::GetExternalDisplayName() const
    {
        return GetExternalDisplayName(this);
    }

    inline RegSlot
    FunctionBody::MapRegSlot(
        RegSlot reg)
    {
        if (this->RegIsConst(reg))
        {
            reg = CONSTREG_TO_REGSLOT(reg);
            Assert(reg < this->GetConstantCount());
        }
        else
        {
            reg += this->GetConstantCount();
        }

        return reg;
    }

    inline bool
    FunctionBody::RegIsConst(RegSlot reg)
    {
        return reg > REGSLOT_TO_CONSTREG(this->GetConstantCount());
    }

    inline RegSlot
    FunctionBody::GetLocalsCount()
    {
        return m_constCount + m_varCount;
    }

    inline RegSlot
    FunctionBody::GetConstantCount()
    {
        return m_constCount;
    }

    inline RegSlot
    FunctionBody::GetVarCount()
    {
        return m_varCount;
    }

    // Returns the number of non-temp local vars.
    inline uint32
    FunctionBody::GetNonTempLocalVarCount()
    {
        Assert(this->GetEndNonTempLocalIndex() >= this->GetFirstNonTempLocalIndex());
        return this->GetEndNonTempLocalIndex() - this->GetFirstNonTempLocalIndex();
    }

    inline uint32
    FunctionBody::GetFirstNonTempLocalIndex()
    {
        // First loval var starts when the const vars end.
        return m_constCount;
    }

    inline uint32
    FunctionBody::GetEndNonTempLocalIndex()
    {
        // It will give the index on which current non temp locals ends, which is a first temp reg.
        return m_firstTmpReg != Constants::NoRegister ? m_firstTmpReg : GetLocalsCount();
    }

    inline bool
    FunctionBody::IsNonTempLocalVar(uint32 varIndex)
    {
        return GetFirstNonTempLocalIndex() <= varIndex && varIndex < GetEndNonTempLocalIndex();
    }

    inline bool
    FunctionBody::GetSlotOffset(RegSlot slotId, __out int32 * slotOffset, bool allowTemp)
    {
        if (IsNonTempLocalVar(slotId) || allowTemp)
        {
            *slotOffset = (slotId - GetFirstNonTempLocalIndex()) * DIAGLOCALSLOTSIZE;
            return true;
        }
        return false;
    }

    inline void
    FunctionBody::SetConstantCount(
        RegSlot cNewConstants)                     // New register count
    {
        CheckNotExecuting();
        AssertMsg(m_constCount <= cNewConstants, "Cannot shrink register usage");

        m_constCount = cNewConstants;
    }

    inline void
    FunctionBody::SetVarCount(
        RegSlot cNewVars)                     // New register count
    {
        CheckNotExecuting();
        AssertMsg(m_varCount <= cNewVars, "Cannot shrink register usage");

        m_varCount = cNewVars;
    }

    inline RegSlot
    FunctionBody::GetYieldRegister()
    {
        return GetEndNonTempLocalIndex() - 1;
    }

    inline RegSlot
    FunctionBody::GetFirstTmpReg()
    {
        AssertMsg(m_firstTmpReg != Constants::NoRegister, "First temp hasn't been set yet");
        return m_firstTmpReg;
    }

    inline void
    FunctionBody::SetFirstTmpReg(
        RegSlot firstTmpReg)
    {
        CheckNotExecuting();
        AssertMsg(m_firstTmpReg == Constants::NoRegister, "Should not be resetting the first temp");

        m_firstTmpReg = firstTmpReg;
    }

    inline RegSlot
    FunctionBody::GetTempCount()
    {
        return GetLocalsCount() - m_firstTmpReg;
    }

    inline void
    FunctionBody::SetOutParamDepth(
        RegSlot cOutParamsDepth)
    {
        CheckNotExecuting();
        m_outParamMaxDepth = cOutParamsDepth;
    }


    inline RegSlot
    FunctionBody::GetOutParamsDepth()
    {
        return m_outParamMaxDepth;
    }

    inline ModuleID
    FunctionBody::GetModuleID() const
    {
        return this->GetHostSrcInfo()->moduleID;
    }

    ///----------------------------------------------------------------------------
    ///
    /// FunctionBody::BeginExecution
    ///
    /// BeginExecution() is called by InterpreterStackFrame when a function begins execution.
    /// - Once started execution, the function may not be modified, as it would
    ///   change the stack-frame layout:
    /// - This is a debug-only check because of the runtime cost.  At release time,
    ///   a stack-walk will be performed by GC to determine which functions are
    ///   executing.
    ///
    ///----------------------------------------------------------------------------

    inline void
    FunctionBody::BeginExecution()
    {
#if DBG
        m_DEBUG_executionCount++;
#endif
        // Don't allow loop headers to be released while the function is executing
        ::InterlockedIncrement(this->m_depth.AddressOf());
    }


    ///----------------------------------------------------------------------------
    ///
    /// FunctionBody::CheckEmpty
    ///
    /// CheckEmpty() validates that the given instance has not been given an
    /// implementation yet.
    ///
    ///----------------------------------------------------------------------------

    inline void
    FunctionBody::CheckEmpty()
    {
        AssertMsg((this->byteCodeBlock == null) && (this->auxBlock == null) && (this->auxContextBlock == null), "Function body may only be set once");
    }


    ///----------------------------------------------------------------------------
    ///
    /// FunctionBody::CheckNotExecuting
    ///
    /// CheckNotExecuting() checks that function is not currently executing when it
    /// is being modified.  See BeginExecution() for details.
    ///
    ///----------------------------------------------------------------------------

    inline void
    FunctionBody::CheckNotExecuting()
    {
        AssertMsg(m_DEBUG_executionCount == 0, "Function cannot be executing when modified");
    }

    ///----------------------------------------------------------------------------
    ///
    /// FunctionBody::EndExecution
    ///
    /// EndExecution() is called by InterpreterStackFrame when a function ends execution.
    /// See BeginExecution() for details.
    ///
    ///----------------------------------------------------------------------------

    inline void
    FunctionBody::EndExecution()
    {
#if DBG
        AssertMsg(m_DEBUG_executionCount > 0, "Must have a previous execution to end");

        m_DEBUG_executionCount--;
#endif
        uint depth = ::InterlockedDecrement(this->m_depth.AddressOf());

        // If loop headers were determined to be no longer needed
        // during the execution of the function, we release them now
        if (depth == 0 && this->m_pendingLoopHeaderRelease)
        {
            this->m_pendingLoopHeaderRelease = false;
            ReleaseLoopHeaders();
        }
    }

    inline void FunctionBody::AddEntryPointToEntryPointList(FunctionEntryPointInfo* entryPointInfo)
    {
        ThreadContext::AutoDisableExpiration disableExpiration(this->m_scriptContext->GetThreadContext());

        Recycler* recycler = this->m_scriptContext->GetRecycler();
        entryPointInfo->entryPointIndex = this->entryPoints->Add(recycler->CreateWeakReferenceHandle(entryPointInfo));
    }


    inline BOOL FunctionBody::IsInterpreterThunk() const
    {
        bool isInterpreterThunk = this->originalEntryPoint == DefaultEntryThunk;
#if DYNAMIC_INTERPRETER_THUNK
        isInterpreterThunk = isInterpreterThunk || IsDynamicInterpreterThunk();
#endif
        return isInterpreterThunk;
    }

    inline BOOL FunctionBody::IsDynamicInterpreterThunk() const
    {
        return this->GetScriptContext()->IsDynamicInterpreterThunk(this->originalEntryPoint);
    }

    inline FunctionEntryPointInfo * FunctionBody::TryGetEntryPointInfo(int index) const
    {
        // If we've already freed the recyclable data, we're shutting down the script context so skip clean up
        if (this->entryPoints == null) return 0;

        Assert(index < this->entryPoints->Count());
        FunctionEntryPointInfo* entryPoint = this->entryPoints->Item(index)->Get();

        return entryPoint;
    }

    inline FunctionEntryPointInfo * FunctionBody::GetEntryPointInfo(int index) const
    {
        FunctionEntryPointInfo* entryPoint = TryGetEntryPointInfo(index);
        Assert(entryPoint);

        return entryPoint;
    }

    inline uint32 FunctionBody::GetFrameHeight(FunctionEntryPointInfo* entryPointInfo) const
    {
        return entryPointInfo->frameHeight;
    }

    inline void FunctionBody::SetFrameHeight(FunctionEntryPointInfo* entryPointInfo, uint32 frameHeight)
    {
        entryPointInfo->frameHeight = frameHeight;
    }

    inline void
    FunctionBody::SetNativeThrowSpanSequence(SmallSpanSequence *seq, uint loopNum, LoopEntryPointInfo* entryPoint)
    {
        Assert(loopNum != LoopHeader::NoLoop);
        LoopHeader *loopHeader = this->GetLoopHeader(loopNum);
        Assert(loopHeader);
        Assert(entryPoint->loopHeader == loopHeader);

        entryPoint->SetNativeThrowSpanSequence(seq);
    }

    inline void
    FunctionBody::RecordNativeThrowMap(SmallSpanSequenceIter& iter, uint32 nativeOffset, uint32 statementIndex, EntryPointInfo* entryPoint, uint loopNum)
    {
        SmallSpanSequence *pSpanSequence;

        pSpanSequence = entryPoint->GetNativeThrowSpanSequence();

        if (!pSpanSequence)
        {
            if (statementIndex == -1)
            {
                return; // No need to initialize native throw map for non-user code
            }

            pSpanSequence = HeapNew(SmallSpanSequence);
            if (loopNum == LoopHeader::NoLoop)
            {
                ((FunctionEntryPointInfo*) entryPoint)->SetNativeThrowSpanSequence(pSpanSequence);
            }
            else
            {
                this->SetNativeThrowSpanSequence(pSpanSequence, loopNum, (LoopEntryPointInfo*) entryPoint);
            }
        }
        else if (iter.accumulatedSourceBegin == static_cast<int>(statementIndex))
        {
            return; // Compress adjacent spans which share the same statementIndex
        }

        StatementData data;
        data.sourceBegin = static_cast<int>(statementIndex); // sourceBegin represents statementIndex here
        data.bytecodeBegin = static_cast<int>(nativeOffset); // bytecodeBegin represents nativeOffset here

        pSpanSequence->RecordARange(iter, &data);
    }

    inline bool
    ParseableFunctionInfo::IsTrackedPropertyId(PropertyId pid)
    {
        Assert(this->m_boundPropertyRecords != null);

        PropertyRecordList* trackedProperties = this->m_boundPropertyRecords;
        const PropertyRecord* prop = NULL;
        if (trackedProperties->TryGetValue(pid, &prop))
        {
            Assert(prop != NULL);

            return true;
        }

        return this->m_scriptContext->IsTrackedPropertyId(pid);
    }

    inline PropertyId
    ParseableFunctionInfo::GetOrAddPropertyIdTracked(JsUtil::CharacterBuffer<WCHAR> const& propName)
    {
        Assert(this->m_boundPropertyRecords != null);

        const Js::PropertyRecord* propRecord = NULL;

        this->m_scriptContext->GetOrAddPropertyRecord(propName, &propRecord);

        PropertyId pid = propRecord->GetPropertyId();
        this->m_boundPropertyRecords->Item(pid, propRecord);

        return pid;
    }

    inline void
    FunctionBody::RecordNativeBaseAddress(BYTE* baseAddress, ptrdiff_t size, NativeCodeData * data, NativeCodeData * transferData,
        CodeGenNumberChunk * numberChunks, EntryPointInfo* entryPoint, uint loopNum)
    {
        entryPoint->SetCodeGenRecorded(baseAddress, size, data, transferData, numberChunks);
    }

    inline int
    FunctionBody::GetNextDebuggerScopeIndex()
    {
        return this->debuggerScopeIndex++;
    }

    inline
    SmallSpanSequence::SmallSpanSequence()
        : pStatementBuffer(NULL),
        pActualOffsetList(NULL),
        baseValue(0)
    {
    }

    inline
    BOOL SmallSpanSequence::RecordARange(SmallSpanSequenceIter &iter, StatementData * data)
    {
        Assert(data);

        if (!this->pStatementBuffer)
        {
            this->pStatementBuffer = JsUtil::GrowingUint32HeapArray::Create(4);
            baseValue = data->sourceBegin;
            Reset(iter);
        }

        SmallSpan span(0);

        span.sourceBegin = GetDiff(data->sourceBegin, iter.accumulatedSourceBegin);
        span.bytecodeBegin = GetDiff(data->bytecodeBegin, iter.accumulatedBytecodeBegin);

        this->pStatementBuffer->Add((uint32)span);

        // Update iterator for the next set

        iter.accumulatedSourceBegin = data->sourceBegin;
        iter.accumulatedBytecodeBegin = data->bytecodeBegin;

        return TRUE;
    }

    // FunctionProxy methods
    inline
    ScriptContext*
    FunctionProxy::GetScriptContext() const
    {
        return m_scriptContext;
    }

    inline
    void FunctionProxy::Copy(FunctionProxy* other)
    {
        Assert(other);

        other->SetIsTopLevel(this->m_isTopLevel);

        if (this->IsPublicLibraryCode())
        {
            other->SetIsPublicLibraryCode();
        }
    }

    inline
    void ParseableFunctionInfo::Copy(FunctionBody* other)
    {
#define CopyDeferParseField(field) other->field = this->field;
        CopyDeferParseField(m_isDeclaration);
        CopyDeferParseField(m_isAccessor);
        CopyDeferParseField(m_isStrictMode);
        CopyDeferParseField(m_isGlobalFunc);
        CopyDeferParseField(m_doBackendArgumentsOptimization);
        CopyDeferParseField(m_isEval);
        CopyDeferParseField(m_isDynamicFunction);
        CopyDeferParseField(m_hasImplicitArgIns);
        CopyDeferParseField(m_dontInline);
        CopyDeferParseField(m_inParamCount);
        CopyDeferParseField(m_grfscr);
        CopyDeferParseField(m_scopeInfo);
        CopyDeferParseField(m_utf8SourceHasBeenSet);
#if DBG
        CopyDeferParseField(deferredParseNextFunctionId);
        CopyDeferParseField(scopeObjectSize);
#endif
        CopyDeferParseField(scopeSlotArraySize);
        CopyDeferParseField(cachedSourceString);
        CopyDeferParseField(deferredStubs);
        CopyDeferParseField(m_isAsmjsMode);
#undef CopyDeferParseField

        other->CopySourceInfo(this);
    }

    inline
    void FunctionProxy::SetReferenceInParentFunction(FunctionProxyPtrPtr reference)
    {
        if (reference)
        {
            // Tag the reference so that the child function doesn't
            // keep the parent alive. If the parent funtion is going away,
            // it'll clear it's childrens references
            this->m_referenceInParentFunction = reference;
        }
        else
        {
            this->m_referenceInParentFunction = null;
        }
    }

    inline
    void FunctionProxy::UpdateReferenceInParentFunction(FunctionProxy* newFunctionInfo)
    {
        if (this->m_referenceInParentFunction)
        {
#ifdef RECYCLER_WRITE_BARRIER
            if (newFunctionInfo == nullptr)
            {
                (*m_referenceInParentFunction).NoWriteBarrierSet(nullptr);
                return;
            }
#endif

            (*m_referenceInParentFunction) = newFunctionInfo;
        }
    }

    // DeferDeserializeFunctionInfo methods

    inline
        DeferDeserializeFunctionInfo::DeferDeserializeFunctionInfo(int nestedCount, LocalFunctionId functionId, ByteCodeCache* byteCodeCache, const byte* serializedFunction, Utf8SourceInfo* sourceInfo, ScriptContext* scriptContext, uint functionNumber, const wchar_t* displayName, uint displayNameLength, NativeModule *nativeModule, Attributes attributes) :
        FunctionProxy(DefaultDeferredDeserializeThunk, (Attributes)(attributes | DeferredDeserialize), nestedCount, sizeof(DeferDeserializeFunctionInfo), functionId, scriptContext, sourceInfo, functionNumber),
        m_cache(byteCodeCache),
        m_functionBytes(serializedFunction),
        m_displayName(null),
        m_displayNameLength(0),
        m_nativeModule(nativeModule)
    {
        this->m_defaultEntryPointInfo = RecyclerNew(scriptContext->GetRecycler(), ProxyEntryPointInfo, DefaultDeferredDeserializeThunk);
        PERF_COUNTER_INC(Code, DeferDeserializeFunctionProxy);

        SetDisplayName(displayName, displayNameLength, FunctionProxy::SetDisplayNameFlagsDontCopy);
    }

    inline
        DeferDeserializeFunctionInfo* DeferDeserializeFunctionInfo::New(ScriptContext* scriptContext, int nestedCount, LocalFunctionId functionId, ByteCodeCache* byteCodeCache, const byte* serializedFunction, Utf8SourceInfo* sourceInfo, const wchar_t* displayName, uint displayNameLength, NativeModule *nativeModule, Attributes attributes)
    {
        return RecyclerNewFinalized(scriptContext->GetRecycler(),
            DeferDeserializeFunctionInfo,
            nestedCount,
            functionId,
            byteCodeCache,
            serializedFunction,
            sourceInfo,
            scriptContext,
            scriptContext->GetThreadContext()->NewFunctionNumber(),
            displayName,
            displayNameLength,
            nativeModule,
            attributes);
    }

    inline const wchar_t*
    DeferDeserializeFunctionInfo::GetDisplayName() const
    {
        return this->m_displayName;
    }

    // ParseableFunctionInfo methods
    inline
    ParseableFunctionInfo::ParseableFunctionInfo(JavascriptMethod entryPoint, int nestedCount, int derivedSize,
        LocalFunctionId functionId, Utf8SourceInfo* sourceInfo, ScriptContext* scriptContext, uint functionNumber,
        const wchar_t* displayName, uint displayNameLength, Attributes attributes, Js::PropertyRecordList* propertyRecords) :
      FunctionProxy(entryPoint, attributes, nestedCount, derivedSize, functionId, scriptContext, sourceInfo, functionNumber),
      m_dynamicInterpreterThunk(null),
      m_hasBeenParsed(false),
      m_isGlobalFunc(false),
      m_isDeclaration(false),
      m_isNamedFunctionExpression(false),
      m_isNameIdentifierRef (true),
      m_isStaticNameFunction(false),
      m_doBackendArgumentsOptimization(true),
      m_isStrictMode(false),
      m_isAsmjsMode(false),
      m_dontInline(false),
      m_hasImplicitArgIns(true),
      m_grfscr(0),
      m_inParamCount(0),
      m_reportedInParamCount(0),
      m_sourceIndex(Js::Constants::InvalidSourceIndex),
      m_utf8SourceHasBeenSet(false),
      m_cchLength(0),
      m_cbLength(0),
      m_cchStartOffset(0),
      m_cbStartOffset(0),
      m_lineNumber(0),
      m_columnNumber(0),
      m_isEval(false),
      m_isDynamicFunction(false),
      m_scopeInfo(null),
      m_displayName(null),
      m_displayNameLength(0),
      deferredStubs(null),
      scopeSlotArraySize(0),
      cachedSourceString(null),
      m_boundPropertyRecords(propertyRecords),
      m_reparsed(false),
#if DBG
      m_wasEverAsmjsMode(false),
      scopeObjectSize(0),
#endif
      isByteCodeDebugMode(false)
    {
        // TODO: Remove this altogether for the parse/deserialize case
        // We should just create the entry point info in the FunctionBody constructor
        if ((attributes & Js::FunctionInfo::DeferredParse) == 0)
        {
            this->m_defaultEntryPointInfo = RecyclerNewFinalized(scriptContext->GetRecycler(),
                FunctionEntryPointInfo, this, entryPoint, scriptContext->GetThreadContext(), (void*) scriptContext->GetNativeCodeGenerator());
        }
        else
        {
            this->m_defaultEntryPointInfo = RecyclerNew(scriptContext->GetRecycler(), ProxyEntryPointInfo, entryPoint);
        }

        // TODO: Consider having a perf counter here
        SetDisplayName(displayName, displayNameLength);
        this->originalEntryPoint = DefaultEntryThunk;
    }

    inline
    ParseableFunctionInfo* ParseableFunctionInfo::New(ScriptContext* scriptContext, int nestedCount,
    LocalFunctionId functionId, Utf8SourceInfo* sourceInfo, const wchar_t* displayName, uint displayNameLength, Js::PropertyRecordList* propertyRecords, Attributes attributes)
    {
        Assert(scriptContext->DeferredParsingThunk == ProfileDeferredParsingThunk
            || scriptContext->DeferredParsingThunk == DefaultDeferredParsingThunk);
#ifdef PERF_COUNTERS
        PERF_COUNTER_INC(Code, DeferedFunction);
#endif
        uint newFunctionNumber = scriptContext->GetThreadContext()->NewFunctionNumber();
        if (!sourceInfo->GetSourceContextInfo()->IsDynamic())
        {
            PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"Function was defered from parsing - ID: %d; Display Name: %s; Utf8SourceInfo ID: %d; Source Length: %d; Source Url:%s\n", newFunctionNumber, displayName, sourceInfo->GetSourceInfoId(), sourceInfo->GetCchLength(), sourceInfo->GetSourceContextInfo()->url);
        }
        else
        {
            PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"Function was defered from parsing - ID: %d; Display Name: %s; Utf8SourceInfo ID: %d; Source Length: %d;\n", newFunctionNumber, displayName, sourceInfo->GetSourceInfoId(), sourceInfo->GetCchLength());
        }

        // When generating a new defer parse function, we always use a new function number
        return RecyclerNewWithBarrierFinalizedPlus(scriptContext->GetRecycler(),
            nestedCount * sizeof(FunctionBody*),
            ParseableFunctionInfo,
            scriptContext->DeferredParsingThunk,
            nestedCount,
            sizeof(ParseableFunctionInfo),
            functionId,
            sourceInfo,
            scriptContext,
            newFunctionNumber,
            displayName,
            displayNameLength,
            (Attributes)(attributes | DeferredParse),
            propertyRecords);
    }

    inline
    DWORD_PTR FunctionProxy::GetSecondaryHostSourceContext() const
    {
        return this->m_utf8SourceInfo->GetSecondaryHostSourceContext();
    }

    inline
    DWORD_PTR FunctionProxy::GetHostSourceContext() const
    {
        return this->GetSourceContextInfo()->dwHostSourceContext;
    }

    inline
    SourceContextInfo * FunctionProxy::GetSourceContextInfo() const
    {
        return this->GetHostSrcInfo()->sourceContextInfo;
    }

    inline
    SRCINFO const * FunctionProxy::GetHostSrcInfo() const
    {
        return m_utf8SourceInfo->GetSrcInfo();
    }

    //
    // Returns the start line for the script buffer (code buffer for the entire script tag) of this current function.
    // We subtract the lnMinHost because it is the number of lines we have added to augment scriplets passed through
    // ParseProcedureText to have a function name.
    //
    inline
    ULONG FunctionProxy::GetHostStartLine() const
    {
        return this->GetHostSrcInfo()->dlnHost - this->GetHostSrcInfo()->lnMinHost;
    }

    //
    // Returns the start column of the first line for the script buffer of this current function.
    //
    inline
    ULONG FunctionProxy::GetHostStartColumn() const
    {
        return this->GetHostSrcInfo()->ulColumnHost;
    }

    //
    // Returns line number in unmodified host buffer (i.e. without extra scriptlet code added by ParseProcedureText --
    // when e.g. we add extra code for event handlers, such as "function onclick(event)\n{\n").
    //
    inline
    ULONG FunctionProxy::GetLineNumberInHostBuffer(ULONG relativeLineNumber) const
    {
        ULONG lineNumber = relativeLineNumber;
        if (lineNumber >= this->GetHostSrcInfo()->lnMinHost)
        {
            lineNumber -= this->GetHostSrcInfo()->lnMinHost;
        }
        // Note that '<' is still a valid case -- that would be the case for onclick scriptlet function itself (lineNumber == 0).

        return lineNumber;
    }

    inline ULONG FunctionProxy::ComputeAbsoluteLineNumber(ULONG relativeLineNumber) const
    {
        // We add 1 because the line numbers start from 0.
        return this->GetHostSrcInfo()->dlnHost + GetLineNumberInHostBuffer(relativeLineNumber) + 1;
    }

    inline ULONG FunctionProxy::ComputeAbsoluteColumnNumber(ULONG relativeLineNumber, ULONG relativeColumnNumber) const
    {
        if (this->GetLineNumberInHostBuffer(relativeLineNumber) == 0)
        {
            // The host column matters only for the first line.
            return this->GetHostStartColumn() + relativeColumnNumber + 1;
        }

        // We add 1 because the column numbers start from 0.
        return relativeColumnNumber + 1;
    }

    //
    // Returns the line number of the function declaration in the source file.
    //
    inline ULONG
    ParseableFunctionInfo::GetLineNumber() const
    {
        return this->ComputeAbsoluteLineNumber(this->m_lineNumber);
        
    }

    //
    // Returns the column number of the function declaration in the source file.
    //
    inline ULONG
    ParseableFunctionInfo::GetColumnNumber() const
    {
        return ComputeAbsoluteColumnNumber(this->m_lineNumber, m_columnNumber);
    }

    template <class T>
    LPCWSTR ParseableFunctionInfo::GetSourceName(const T& sourceContextInfo) const
    {
        return GetSourceName<T>(sourceContextInfo, this->m_isEval, this->m_isDynamicFunction);
    }

    //
    // Algorithm to retrieve a function body's source name (url). Template supports both
    // local FunctionBody and ScriptDAC (debugging) scenarios.
    //
    template <class T>
    inline static LPCWSTR ParseableFunctionInfo::GetSourceName(const T& sourceContextInfo, bool m_isEval, bool m_isDynamicFunction)
    {
        if (sourceContextInfo->IsDynamic())
        {
            if (m_isEval)
            {
                return Constants::EvalCode;
            }
            else if (m_isDynamicFunction)
            {
                return Constants::FunctionCode;
            }
            else
            {
                return Constants::UnknownScriptCode;
            }
        }
        else
        {
            return sourceContextInfo->url;
        }
    }

    inline LPCWSTR ParseableFunctionInfo::GetSourceName() const
    {
        return GetSourceName(this->GetSourceContextInfo());
    }

    inline void
    ParseableFunctionInfo::SetGrfscr(ulong grfscr)
    {
        this->m_grfscr = grfscr;
    }

    inline ulong
    ParseableFunctionInfo::GetGrfscr() const
    {
        return this->m_grfscr;
    }


    inline
    ProxyEntryPointInfo* FunctionProxy::GetDefaultEntryPointInfo() const
    {
        return this->m_defaultEntryPointInfo;
    }

    inline
    FunctionEntryPointInfo* FunctionBody::GetDefaultFunctionEntryPointInfo() const
    {
        Assert(((ProxyEntryPointInfo*) this->defaultFunctionEntryPointInfo) == this->m_defaultEntryPointInfo);
        return this->defaultFunctionEntryPointInfo;
    }

    ///----------------------------------------------------------------------------
    ///
    /// ParseableFunctionInfo::GetInParamsCount
    ///
    /// GetInParamsCount() returns the number of "in parameters" that have
    /// currently been declared for this function:
    /// - If this is "RegSlot_VariableCount", the function takes a variable number
    ///   of parameters.
    ///
    /// TODO: Change to store type information about parameters- names, type,
    /// direction, etc.
    ///
    ///----------------------------------------------------------------------------

    inline ArgSlot
    ParseableFunctionInfo::GetInParamsCount() const
    {
        return m_inParamCount;
    }


    inline void
    ParseableFunctionInfo::SetInParamsCount(ArgSlot newInParamCount)
    {
        AssertMsg(m_inParamCount <= newInParamCount, "Cannot shrink register usage");

        m_inParamCount = newInParamCount;

        if (newInParamCount <= 1)
        {
            SetHasImplicitArgIns(false);
        }
    }

    inline ArgSlot
    ParseableFunctionInfo::GetReportedInParamsCount() const
    {
        return m_reportedInParamCount;
    }

    inline void
    ParseableFunctionInfo::SetReportedInParamsCount(ArgSlot newInParamCount)
    {
        AssertMsg(m_reportedInParamCount <= newInParamCount, "Cannot shrink register usage");

        m_reportedInParamCount = newInParamCount;
    }

    inline void
    ParseableFunctionInfo::ResetInParams()
    {
        m_inParamCount = 0;
        m_reportedInParamCount = 0;
    }

    inline const wchar_t*
    ParseableFunctionInfo::GetDisplayName() const
    {
        return this->m_displayName;
    }

} // namespace Js
