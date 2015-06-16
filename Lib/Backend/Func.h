//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

struct CodeGenWorkItem;
class Lowerer;
class Inline;
class FlowGraph;

#if defined(_M_ARM32_OR_ARM64)
#include "UnwindInfoManager.h"
#endif

struct Cloner
{
    Cloner(Lowerer *lowerer, JitArenaAllocator *alloc) : 
        alloc(alloc), 
        symMap(NULL), 
        labelMap(NULL), 
        lowerer(lowerer),
        instrFirst(NULL), 
        instrLast(NULL), 
        fRetargetClonedBranch(FALSE)
    {
    }

    ~Cloner()
    {
        if (symMap)
        {
            Adelete(alloc, symMap);
        }
        if (labelMap)
        {
            Adelete(alloc, labelMap);
        }
    }

    void AddInstr(IR::Instr * instrOrig, IR::Instr * instrClone);
    void Finish();
    void RetargetClonedBranches();

    HashTable<StackSym*> *symMap;
    HashTable<IR::LabelInstr*> *labelMap;
    Lowerer * lowerer;
    IR::Instr * instrFirst;
    IR::Instr * instrLast;
    BOOL fRetargetClonedBranch;
    JitArenaAllocator *alloc;
    bool clonedInstrGetOrigArgSlotSym;
};

typedef JsUtil::Pair<uint32, IR::LabelInstr*> YieldOffsetResumeLabel;
typedef JsUtil::List<YieldOffsetResumeLabel, JitArenaAllocator> YieldOffsetResumeLabelList;

class Func
{
public:
    Func(JitArenaAllocator *alloc, CodeGenWorkItem* workItem, const Js::FunctionCodeGenRuntimeData *const runtimeData,
        Js::PolymorphicInlineCacheInfo * const polymorphicInlineCacheInfo, CodeGenAllocators *const codeGenAllocators, 
        CodeGenNumberAllocator * numberAllocator, Js::ReadOnlyDynamicProfileInfo *const profileInfo,
        Js::ScriptContextProfiler *const codeGenProfiler, const bool isBackgroundJIT, Func * parentFunc = null,
        uint postCallByteCodeOffset = Js::Constants::NoByteCodeOffset,
        Js::RegSlot returnValueRegSlot = Js::Constants::NoRegister, const bool isInlinedConstructor = false, 
        Js::ProfileId callSiteIdInParentFunc = UINT16_MAX, bool isGetterSetter = false):
      m_alloc(alloc), 
      m_workItem(workItem),
      m_jitTimeData(workItem->IsInMemoryWorkItem() ? ((InMemoryCodeGenWorkItem *)workItem)->RecyclableData()->JitTimeData() : nullptr),
      m_runtimeData(runtimeData),
      m_polymorphicInlineCacheInfo(polymorphicInlineCacheInfo),
      m_codeGenAllocators(codeGenAllocators),
      pinnedTypeRefs(null),
      singleTypeGuards(null),
      equivalentTypeGuards(null),
      propertyGuardsByPropertyId(null),
      ctorCachesByPropertyId(null),
      callSiteToArgumentsOffsetFixupMap(null),
      indexedPropertyGuardCount(0),
      propertiesWrittenTo(null),
      lazyBailoutProperties(alloc),
      anyPropertyMayBeWrittenTo(false),
#ifdef PROFILE_EXEC
      m_codeGenProfiler(codeGenProfiler),
#endif
      m_isBackgroundJIT(isBackgroundJIT),
      m_cloner(NULL),
      m_cloneMap(NULL),
      m_loopParamSym(NULL),
      m_funcObjSym(NULL),
      m_localClosureSym(NULL),
      m_localFrameDisplaySym(NULL),
      m_bailoutReturnValueSym(null),
      m_hasBailedOutSym(null),
      m_inlineeFrameStartSym(null),
      m_regsUsed(0),
      m_fg(null), 
      m_labelCount(0),
      m_argSlotsForFunctionsCalled(0),
      m_isLeaf(false),    
      m_hasCalls(false),
      m_hasInlineArgsOpt(false),
      m_canDoInlineArgsOpt(true),
      m_doFastPaths(false),
      hasBailout(false),
      hasBailoutInEHRegion(false),
      hasInstrNumber(false),
      maintainByteCodeOffset(true),
      frameSize(0),
      parentFunc(parentFunc),
      argObjSyms(null),
      m_nonTempLocalVars(null),
      hasAnyStackNestedFunc(false),
      hasMarkTempObjects(false),
      postCallByteCodeOffset(postCallByteCodeOffset),
      maxInlineeArgOutCount(0),
      returnValueRegSlot(returnValueRegSlot),
      firstActualStackOffset(-1),
      m_localVarSlotsOffset(Js::Constants::InvalidOffset),
      m_hasLocalVarChangedOffset(Js::Constants::InvalidOffset),
      actualCount((Js::ArgSlot)-1),
      tryCatchNestingLevel(0),
      m_localStackHeight(0),
      tempSymDouble(NULL),
      hasInlinee(false),
      thisOrParentInlinerHasArguments(false),
      hasStackArgs(false),
      hasArgumentObject(false),
      hasUnoptimizedArgumentsAcccess(false),
      hasApplyTargetInlining(false),
      hasImplicitCalls(false),
      hasTempObjectProducingInstr(false),
      isInlinedConstructor(isInlinedConstructor),
      numberAllocator(numberAllocator),
      profileInfo(profileInfo),
      loopCount(0),
      callSiteIdInParentFunc(callSiteIdInParentFunc),
      isGetterSetter(isGetterSetter),
      frameInfo(nullptr),
      isTJLoopBody(false),
      isFlowGraphValid(false),
#if DBG
      m_callSiteCount(0),
#endif
      stackNestedFunc(false),
      stackClosure(false)

#if defined(_M_ARM32_OR_ARM64)
      ,m_ArgumentsOffset(0)
      ,m_epilogLabel(nullptr)
#endif
#ifdef _M_X64
      , m_prologEncoder(alloc)
#endif
#if DBG
      , hasCalledSetDoFastPaths(false)
      , allowRemoveBailOutArgInstr(false)
      , isPostLower(false)
      , isPostRegAlloc(false)
      , isPostPeeps(false)
      , isPostLayout(false)
      , isPostFinalLower(false)
#endif
#if DBG
      , vtableMap(null)
#endif
      , m_yieldOffsetResumeLabelList(nullptr)
      , constantAddressRegOpnd(alloc)
      , lastConstantAddressRegLoadInstr(nullptr)    
    {
        Assert(this->IsInlined() == !!runtimeData);
        
        m_jnFunction = m_workItem->GetFunctionBody();
        bool doStackNestedFunc = m_jnFunction->DoStackNestedFunc();
        bool doStackClosure = m_jnFunction->DoStackClosure() && !PHASE_OFF(Js::FrameDisplayFastPathPhase, this);
        Assert(!doStackClosure || doStackNestedFunc);
        this->stackClosure = doStackClosure && this->IsTopFunc();
        if (m_workItem->Type() == JsFunctionType || m_workItem->Type() == JsFunctionSerializedType)
        {
            if (m_jnFunction->GetDoBackendArgumentsOptimization() && !m_jnFunction->GetHasTry())
            {
                // doBackendArgumentsOptimization bit is set when there is no eval inside a function 
                // as determined by the bytecode generator.
                SetHasStackArgs(true);
            }            
            if (doStackNestedFunc && m_jnFunction->GetNestedCount() != 0)
            {
                Assert(!(this->IsJitInDebugMode() && !m_jnFunction->GetUtf8SourceInfo()->GetIsLibraryCode()));
                stackNestedFunc = true;
                this->GetTopFunc()->hasAnyStackNestedFunc = true;
            }
        }
        else
        {
            Assert(m_workItem->Type() == JsLoopBodyWorkItemType);
        }

        if(m_jnFunction->GetHasOrParentHasArguments() || parentFunc && parentFunc->thisOrParentInlinerHasArguments)
        {
            thisOrParentInlinerHasArguments = true;
        }

        if (parentFunc == null)
        {
            inlineDepth = 0;
            m_symTable = JitAnew(alloc, SymTable);
            m_symTable->Init(this);
            Assert(Js::Constants::NoByteCodeOffset == postCallByteCodeOffset);
            Assert(Js::Constants::NoRegister == returnValueRegSlot);

#if defined(_M_IX86) ||  defined(_M_X64)
            if (HasArgumentSlot())
            {
                // Pre-allocate the single argument slot we'll reserve for the arguments object.
                // For ARM, the argument slot is not part of the local but part of the register saves
                m_localStackHeight = MachArgsSlotOffset;
            }
#endif
        }
        else
        {
            inlineDepth = parentFunc->inlineDepth + 1;
            Assert(Js::Constants::NoByteCodeOffset != postCallByteCodeOffset);
        }

        this->constructorCacheCount = 0;
        this->constructorCaches = AnewArrayZ(this->m_alloc, Js::JitTimeConstructorCache*, this->m_jnFunction->GetProfiledCallSiteCount());

#if DBG_DUMP        
        m_codeSize = -1;
#endif

#if defined(_M_X64)
        m_spillSize        = -1;
        m_argsSize         = -1;
        m_savedRegSize     = -1;
#endif

        if (this->IsJitInDebugMode())
        {
            m_nonTempLocalVars = Anew(this->m_alloc, BVSparse<JitArenaAllocator>, this->m_alloc);
        }

        if (this->m_jnFunction->IsGenerator())
        {
            m_yieldOffsetResumeLabelList = YieldOffsetResumeLabelList::New(this->m_alloc);
        }

        canHoistConstantAddressLoad = !PHASE_OFF(Js::HoistConstAddrPhase, this);
    }

public:
    ArenaAllocator *GetCodeGenAllocator() const { return &this->m_codeGenAllocators->allocator; }
    CodeGenAllocators * const GetCodeGenAllocators()
    { 
        // If we're serializing, anything generated in dynamic memory won't get encoded into the PE
        Assert(m_workItem->IsInMemoryWorkItem());
        return this->GetTopFunc()->m_codeGenAllocators; 
    }
    NativeCodeData::Allocator *GetNativeCodeDataAllocator() 
    { 
        // If we're serializing, anything generated in dynamic memory won't get encoded into the PE
        Assert(m_workItem->IsInMemoryWorkItem());
        return &this->GetTopFunc()->nativeCodeDataAllocator; 
    }
    NativeCodeData::Allocator *GetTransferDataAllocator() 
    { 
        // If we're serializing, anything generated in dynamic memory won't get encoded into the PE
        Assert(m_workItem->IsInMemoryWorkItem());
        return &this->GetTopFunc()->transferDataAllocator; 
    }
    CodeGenNumberAllocator * GetNumberAllocator()
    {
        // If we're serializing, anything generated in dynamic memory won't get encoded into the PE
        Assert(m_workItem->IsInMemoryWorkItem());
        return this->numberAllocator; 
    }
    EmitBufferManager<CriticalSection> *GetEmitBufferManager() const 
    { 
        // If we're serializing, anything generated in dynamic memory won't get encoded into the PE
        Assert(m_workItem->IsInMemoryWorkItem());
        return &this->m_codeGenAllocators->emitBufferManager; 
    }

    Js::ScriptContextProfiler *GetCodeGenProfiler() const
    {
#ifdef PROFILE_EXEC
        return m_codeGenProfiler;
#else
        return null;
#endif
    }

    void InitStackClosureSyms();

    bool HasAnyStackNestedFunc() const { return this->hasAnyStackNestedFunc; }
    bool DoStackNestedFunc() const { return this->stackNestedFunc; }
    bool DoStackFrameDisplay() const { return this->stackClosure; }
    bool DoStackScopeSlots() const { return this->stackClosure; }
    bool IsBackgroundJIT() const { return this->m_isBackgroundJIT; }
    bool HasArgumentSlot() const { return this->GetInParamsCount() != 0 && !this->IsLoopBody(); }
    bool IsInMemory() const { return this->m_workItem->IsInMemoryWorkItem(); }
    bool IsLoopBody() const;
    bool IsLoopBodyInTry() const;
    bool CanAllocInPreReservedHeapPageSegment();
    void SetDoFastPaths();
    bool DoFastPaths() const { Assert(this->hasCalledSetDoFastPaths); return this->m_doFastPaths; }

    bool DoLoopFastPaths() const
    {
        return
            (!IsSimpleJit() || Js::FunctionBody::IsNewSimpleJit()) &&
            !PHASE_OFF(Js::FastPathPhase, this) &&
            !PHASE_OFF(Js::LoopFastPathPhase, this);
    }

    bool DoGlobOpt() const
    {
        return
            !PHASE_OFF(Js::GlobOptPhase, this->GetJnFunction()) &&
            IsInMemory() &&
            !IsSimpleJit() &&
            (!GetTopFunc()->HasTry() || GetTopFunc()->CanOptimizeTryCatch());
    }

    bool DoInline() const
    {
        return DoGlobOpt() && !GetTopFunc()->HasTry();
    }

    bool DoOptimizeTryCatch() const
    {
        Assert(IsTopFunc());
        return DoGlobOpt();
    }

    bool CanOptimizeTryCatch() const
    {
        return !this->HasFinally() && !this->IsLoopBody() && !PHASE_OFF(Js::OptimizeTryCatchPhase, this);
    }

    bool DoSimpleJitDynamicProfile() const { return IsSimpleJit() && GetTopFunc()->GetJnFunction()->DoSimpleJitDynamicProfile(); }
    bool IsSimpleJit() const { return m_workItem->GetJitMode() == ExecutionMode::SimpleJit; }

    void BuildIR();
    void Codegen();

    void ThrowIfScriptClosed();

    int32 StackAllocate(int size);
    int32 StackAllocate(StackSym *stackSym, int size);
    void SetArgOffset(StackSym *stackSym, int32 offset);

    int32 GetLocalVarSlotOffset(int32 slotId);
    int32 GetHasLocalVarChangedOffset();
    bool IsJitInDebugMode();
    bool IsNonTempLocalVar(uint32 slotIndex);
    int32 AdjustOffsetValue(int32 offset);
    void OnAddSym(Sym* sym);

#ifdef MD_GROW_LOCALS_AREA_UP
    void AjustLocalVarSlotOffset();
#endif

    bool DoGlobOptsForGeneratorFunc();

    static inline uint32 GetDiagLocalSlotSize()
    {
        // For the debug purpose we will have fixed stack slot size
        // We will allcated the 8 bytes for each variable.
        return MachDouble;
    }

#ifdef DBG
    // The pattern used to pre-fill locals for CHK builds. 
    // When we restore bailout values we check for this pattern, this is how we assert for non-initialized variabled/garbage.

static const uint32 c_debugFillPattern4 = 0xcececece;
static const unsigned __int64 c_debugFillPattern8 = 0xcececececececece;
    
#if defined(_M_IX86) || defined (_M_ARM)
    static const uint32 c_debugFillPattern = c_debugFillPattern4;
#elif defined(_M_X64) || defined(_M_ARM64)
    static const unsigned __int64 c_debugFillPattern = c_debugFillPattern8;
#else
#error unsuported platform
#endif

#endif

    uint32 GetInstrCount();
    inline Js::ScriptContext* GetScriptContext() const { return m_workItem->GetScriptContext(); }
    void NumberInstrs();
    bool IsTopFunc() const { return this->parentFunc == null; }        
    Func const * GetTopFunc() const;
    Func * GetTopFunc();

    void SetFirstArgOffset(IR::Instr* inlineeStart);
    
    uint GetFunctionNumber() const
    {
        Assert(this->IsTopFunc());
        return this->m_workItem->GetFunctionNumber();     
    }
    uint GetLocalFunctionId() const
    {
        return this->m_workItem->GetFunctionBody()->GetLocalFunctionId();
    }
    uint GetSourceContextId() const
    {
        return this->m_workItem->GetFunctionBody()->GetSourceContextId();
    }
    BOOL HasTry() const
    {
        Assert(this->IsTopFunc());
        Assert(this->m_jnFunction);     // For now we always has a function body
        return this->m_jnFunction->GetHasTry();
    }
    bool HasFinally() const
    {
        Assert(this->IsTopFunc());
        Assert(this->m_jnFunction);     // For now we always has a function body
        return this->m_jnFunction->GetHasFinally();
    }
    Js::ArgSlot GetInParamsCount() const
    {
        Assert(this->IsTopFunc());
        Assert(this->m_jnFunction);     // For now we always has a function body
        return this->m_jnFunction->GetInParamsCount();
    }
    bool IsGlobalFunc() const
    {
        Assert(this->IsTopFunc());
        Assert(this->m_jnFunction);     // For now we always has a function body
        return this->m_jnFunction->GetIsGlobalFunc();
    }

    RecyclerWeakReference<Js::FunctionBody> *GetWeakFuncRef() const;
    Js::FunctionBody * GetJnFunction() const { return m_jnFunction; }   
   
    StackSym *EnsureLoopParamSym();

    StackSym *GetFuncObjSym() const { return m_funcObjSym; }
    void SetFuncObjSym(StackSym *sym) { m_funcObjSym = sym; }

    StackSym *GetJavascriptLibrarySym() const { return m_javascriptLibrarySym; }
    void SetJavascriptLibrarySym(StackSym *sym) { m_javascriptLibrarySym = sym; }

    StackSym *GetScriptContextSym() const { return m_scriptContextSym; }
    void SetScriptContextSym(StackSym *sym) { m_scriptContextSym = sym; }

    StackSym *GetFunctionBodySym() const { return m_functionBodySym; }
    void SetFunctionBodySym(StackSym *sym) { m_functionBodySym = sym; }

    StackSym *GetLocalClosureSym() const { return m_localClosureSym; }
    void SetLocalClosureSym(StackSym *sym) { m_localClosureSym = sym; }

    StackSym *GetLocalFrameDisplaySym() const { return m_localFrameDisplaySym; }
    void SetLocalFrameDisplaySym(StackSym *sym) { m_localFrameDisplaySym = sym; }

    uint8 *GetCallsCountAddress() const;

    void EnsurePinnedTypeRefs();
    void PinTypeRef(void* typeRef);

    void EnsureSingleTypeGuards();
    Js::JitTypePropertyGuard* GetOrCreateSingleTypeGuard(Js::Type* type);

    void  EnsureEquivalentTypeGuards();
    Js::JitEquivalentTypeGuard * CreateEquivalentTypeGuard(Js::Type* type, uint32 objTypeSpecFldId);

    void EnsurePropertyGuardsByPropertyId();
    void EnsureCtorCachesByPropertyId();

    void LinkGuardToPropertyId(Js::PropertyId propertyId, Js::JitIndexedPropertyGuard* guard);
    void LinkCtorCacheToPropertyId(Js::PropertyId propertyId, Js::JitTimeConstructorCache* cache);

    Js::JitTimeConstructorCache* GetConstructorCache(const Js::ProfileId profiledCallSiteId);
    void SetConstructorCache(const Js::ProfileId profiledCallSiteId, Js::JitTimeConstructorCache* constructorCache);

    void EnsurePropertiesWrittenTo();

    void EnsureCallSiteToArgumentsOffsetFixupMap();

#ifdef _M_X64
    void SetSpillSize(IntConstType spillSize)
    {
        m_spillSize = spillSize;
    }

    IntConstType GetSpillSize()
    {
        return m_spillSize;
    }

    void SetArgsSize(IntConstType argsSize)
    {
        m_argsSize = argsSize;
    }

    IntConstType GetArgsSize()
    {
        return m_argsSize;
    }

    void SetSavedRegSize(IntConstType savedRegSize)
    {
        m_savedRegSize = savedRegSize;
    }

    IntConstType GetSavedRegSize()
    {
        return m_savedRegSize;
    }
#endif

    bool IsInlinee() const
    {
        Assert(m_inlineeFrameStartSym ? (m_inlineeFrameStartSym->m_offset != -1) : true);
        return m_inlineeFrameStartSym != null;
    }

    void SetInlineeFrameStartSym(StackSym *sym)
    {
        Assert(m_inlineeFrameStartSym == null);
        m_inlineeFrameStartSym = sym;
    }

    IR::SymOpnd *GetInlineeArgCountSlotOpnd()
    {
        return GetInlineeOpndAtOffset(Js::Constants::InlineeMetaArgIndex_Argc * MachPtr);
    }

    IR::SymOpnd *GetNextInlineeFrameArgCountSlotOpnd()
    {
        Assert(!this->m_hasInlineArgsOpt);
        return GetInlineeOpndAtOffset((Js::Constants::InlineeMetaArgCount + actualCount) * MachPtr);
    }

    IR::SymOpnd *GetInlineeFunctionObjectSlotOpnd()
    {
        Assert(!this->m_hasInlineArgsOpt);
        return GetInlineeOpndAtOffset(Js::Constants::InlineeMetaArgIndex_FunctionObject * MachPtr);
    }

    IR::SymOpnd *GetInlineeArgumentsObjectSlotOpnd()
    {
        return GetInlineeOpndAtOffset(Js::Constants::InlineeMetaArgIndex_ArgumentsObject * MachPtr);
    }

    IR::SymOpnd *GetInlineeArgvSlotOpnd()
    {
        Assert(!this->m_hasInlineArgsOpt);
        return GetInlineeOpndAtOffset(Js::Constants::InlineeMetaArgIndex_Argv * MachPtr);
    }

    bool IsInlined() const
    {
        return this->parentFunc != NULL;
    }

    bool IsInlinedConstructor() const
    {
        return this->isInlinedConstructor;
    }
    bool IsTJLoopBody()const {
        return this->isTJLoopBody;
    }

    Js::ObjTypeSpecFldInfo* GetObjTypeSpecFldInfo(const uint index) const;
    Js::ObjTypeSpecFldInfo* GetGlobalObjTypeSpecFldInfo(uint propertyInfoId) const;
    void SetGlobalObjTypeSpecFldInfo(uint propertyInfoId, Js::ObjTypeSpecFldInfo* info);

    // Gets an inline cache pointer to use in jitted code. Cached data may not be stable while jitting. Does not return null.
    Js::InlineCache *GetRuntimeInlineCache(const uint index) const;
    Js::PolymorphicInlineCache * GetRuntimePolymorphicInlineCache(const uint index) const;
    byte GetPolyCacheUtil(const uint index) const;
    byte GetPolyCacheUtilToInitialize(const uint index) const;

#if defined(_M_ARM32_OR_ARM64)
    RegNum GetLocalsPointer() const;
#endif

#if DBG_DUMP
    void                Dump(IRDumpFlags flags);
    void                Dump();
    void                DumpHeader();
#endif

#if DBG_DUMP || defined(ENABLE_IR_VIEWER)
    LPCSTR GetVtableName(INT_PTR address);
#endif
#if DBG_DUMP | defined(VTUNE_PROFILING)
    bool DoRecordNativeMap() const;
#endif
public:
    JitArenaAllocator *    m_alloc;
    CodeGenWorkItem*    m_workItem;
    const Js::FunctionCodeGenJitTimeData *const m_jitTimeData;
    const Js::FunctionCodeGenRuntimeData *const m_runtimeData;
    Js::PolymorphicInlineCacheInfo *const m_polymorphicInlineCacheInfo;

    // This indicates how many constructor caches we inserted into the constructorCaches array, not the total size of the array.
    uint constructorCacheCount;

    // This array maps callsite ids to constructor caches. The size corresponds to the number of callsites in the function.
    Js::JitTimeConstructorCache** constructorCaches;

    typedef JsUtil::BaseHashSet<void*, JitArenaAllocator, PowerOf2SizePolicy> TypeRefSet;
    TypeRefSet* pinnedTypeRefs;

    typedef JsUtil::BaseDictionary<Js::Type*, Js::JitTypePropertyGuard*, JitArenaAllocator, PowerOf2SizePolicy> TypePropertyGuardDictionary;
    TypePropertyGuardDictionary* singleTypeGuards;

    typedef SListCounted<Js::JitEquivalentTypeGuard*> EquivalentTypeGuardList;
    EquivalentTypeGuardList* equivalentTypeGuards;

    typedef JsUtil::BaseHashSet<Js::JitIndexedPropertyGuard*, JitArenaAllocator, PowerOf2SizePolicy> IndexedPropertyGuardSet;
    typedef JsUtil::BaseDictionary<Js::PropertyId, IndexedPropertyGuardSet*, JitArenaAllocator, PowerOf2SizePolicy> PropertyGuardByPropertyIdMap;
    PropertyGuardByPropertyIdMap* propertyGuardsByPropertyId;

    typedef JsUtil::BaseHashSet<Js::ConstructorCache*, JitArenaAllocator, PowerOf2SizePolicy> CtorCacheSet;
    typedef JsUtil::BaseDictionary<Js::PropertyId, CtorCacheSet*, JitArenaAllocator, PowerOf2SizePolicy> CtorCachesByPropertyIdMap;
    CtorCachesByPropertyIdMap* ctorCachesByPropertyId;

    typedef JsUtil::BaseDictionary<Js::ProfileId, int32, JitArenaAllocator, PrimeSizePolicy> CallSiteToArgumentsOffsetFixupMap;
    CallSiteToArgumentsOffsetFixupMap* callSiteToArgumentsOffsetFixupMap;
    int indexedPropertyGuardCount;

    typedef JsUtil::BaseHashSet<Js::PropertyId, JitArenaAllocator> PropertyIdSet;
    PropertyIdSet* propertiesWrittenTo;
    PropertyIdSet lazyBailoutProperties;
    bool anyPropertyMayBeWrittenTo;

    IR::Instr *         m_headInstr;
    IR::Instr *         m_exitInstr;
    IR::Instr *         m_tailInstr;
#ifdef _M_X64
    IntConstType        m_spillSize;
    IntConstType        m_argsSize;
    IntConstType        m_savedRegSize;
    PrologEncoder       m_prologEncoder;
#endif         
        
    SymTable *          m_symTable;
    StackSym *          m_loopParamSym;
    StackSym *          m_funcObjSym;
    StackSym *          m_javascriptLibrarySym;
    StackSym *          m_scriptContextSym;
    StackSym *          m_functionBodySym;
    StackSym *          m_localClosureSym;
    StackSym *          m_localFrameDisplaySym;
    StackSym *          m_bailoutReturnValueSym;
    StackSym *          m_hasBailedOutSym;

    int32               m_localStackHeight;
    uint                frameSize;
    uint32              inlineDepth;
    uint32              postCallByteCodeOffset;  
    Js::RegSlot         returnValueRegSlot;
    Js::ArgSlot         actualCount;
    int32               firstActualStackOffset;
    uint32              tryCatchNestingLevel;
#if defined(_M_ARM32_OR_ARM64)
    //Offset to arguments from sp + m_localStackHeight;
    //For non leaf functions this is (callee saved register count + LR + R11) * MachRegInt
    //For leaf functions this is (saved registers) * MachRegInt
    int32               m_ArgumentsOffset; 
    UnwindInfoManager   m_unwindInfo;
    IR::LabelInstr *    m_epilogLabel;
#endif

    // Keep track of the maximum number of args on the stack.
    uint32              m_argSlotsForFunctionsCalled;    
#if DBG
    uint32              m_callSiteCount;
#endif
    FlowGraph *         m_fg;
    unsigned int        m_labelCount;    
    BitVector           m_regsUsed;
    StackSym *          tempSymDouble;
    uint32              loopCount;
    Js::ProfileId       callSiteIdInParentFunc;
    bool                m_isLeaf: 1;  // This is set in the IRBuilder and might be innacurate after inlining
    bool                m_hasCalls: 1; // This is more accurate compared to m_isLeaf
    bool                m_hasInlineArgsOpt : 1;
    bool                m_doFastPaths : 1;
    bool                hasBailout: 1;
    bool                hasBailoutInEHRegion : 1;
    bool                hasStackArgs: 1;
    bool                hasArgumentObject : 1;
    bool                hasUnoptimizedArgumentsAcccess : 1; // True if there is any arguments access beyond the simple case of this.apply pattern
    bool                m_canDoInlineArgsOpt : 1;
    bool                hasApplyTargetInlining:1;
    bool                isGetterSetter : 1;
    const bool          isInlinedConstructor: 1;
    bool                hasImplicitCalls: 1;
    bool                hasTempObjectProducingInstr:1; //Atleast one instruction which can produce temp object
    bool                isTJLoopBody : 1;
    bool                isFlowGraphValid : 1;
#if DBG
    bool                hasCalledSetDoFastPaths:1;
    bool                isPostLower:1;
    bool                isPostRegAlloc:1;
    bool                isPostPeeps:1;
    bool                isPostLayout:1;
    bool                isPostFinalLower:1;
#endif

    void                BeginPhase(Js::Phase tag);
    void                EndPhase(Js::Phase tag, bool dump = true);    
    void                EndProfiler(Js::Phase tag);

    void                BeginClone(Lowerer *lowerer, JitArenaAllocator *alloc);
    void                EndClone();
    Cloner *            GetCloner() const { return GetTopFunc()->m_cloner; }
    InstrMap *          GetCloneMap() const { return GetTopFunc()->m_cloneMap; }
    void                ClearCloneMap() { Assert(this->IsTopFunc()); this->m_cloneMap = null; }

    bool                HasByteCodeOffset() const { return !this->GetTopFunc()->hasInstrNumber; }
    bool                DoMaintainByteCodeOffset() const { return this->HasByteCodeOffset() && this->GetTopFunc()->maintainByteCodeOffset; }
    void                StopMaintainByteCodeOffset() { this->GetTopFunc()->maintainByteCodeOffset = false; }
    Func *              GetParentFunc() const { return parentFunc; }
    uint                GetMaxInlineeArgOutCount() const { return maxInlineeArgOutCount; }
    void                UpdateMaxInlineeArgOutCount(uint inlineeArgOutCount);
#if DBG_DUMP    
    ptrdiff_t           m_codeSize;
#endif 
    bool                GetHasCalls() const { return this->m_hasCalls; }
    void                SetHasCalls() { this->m_hasCalls = true; }
    void                SetHasCallsOnSelfAndParents() 
    {
                        Func *curFunc = this;
                        while (curFunc)
                        {
                            curFunc->SetHasCalls();
                            curFunc = curFunc->GetParentFunc();
                        }
    }
    void                SetHasInstrNumber(bool has) { this->GetTopFunc()->hasInstrNumber = has; }
    bool                HasInstrNumber() const { return this->GetTopFunc()->hasInstrNumber; }
    bool                HasInlinee() const { Assert(this->IsTopFunc()); return this->hasInlinee; }
    void                SetHasInlinee() { Assert(this->IsTopFunc()); this->hasInlinee = true; }

    bool                GetThisOrParentInlinerHasArguments() const { return thisOrParentInlinerHasArguments; }

    bool                GetHasStackArgs() const { return this->hasStackArgs;}
    void                SetHasStackArgs(bool has) { this->hasStackArgs = has;}

    bool                GetHasArgumentObject() const { return this->hasArgumentObject;}
    void                SetHasArgumentObject() { this->hasArgumentObject = true;}

    bool                GetHasUnoptimizedArgumentsAcccess() const { return this->hasUnoptimizedArgumentsAcccess; }
    void                SetHasUnoptimizedArgumentsAccess(bool args) 
    { 
                        // Once set to 'true' make sure this does not become false
                        if (!this->hasUnoptimizedArgumentsAcccess)
                        {
                            this->hasUnoptimizedArgumentsAcccess = args;
                        }

                        if (args)
                        {
                            Func *curFunc = this->GetParentFunc();
                            while (curFunc)
                            {
                                curFunc->hasUnoptimizedArgumentsAcccess = args;
                                curFunc = curFunc->GetParentFunc();
                            }
                        }
    }

    void               DisableCanDoInlineArgOpt()
    {
                        Func* curFunc = this;
                        while (curFunc)
                        {
                            curFunc->m_canDoInlineArgsOpt = false;
                            curFunc->m_hasInlineArgsOpt = false;
                            curFunc = curFunc->GetParentFunc();
                        }
    }

    bool                GetHasApplyTargetInlining() const { return this->hasApplyTargetInlining;}
    void                SetHasApplyTargetInlining() { this->hasApplyTargetInlining = true;}

    bool                GetHasMarkTempObjects() const { return this->hasMarkTempObjects; }
    void                SetHasMarkTempObjects() { this->hasMarkTempObjects = true; }

    bool                GetHasImplicitCalls() const { return this->hasImplicitCalls;}
    void                SetHasImplicitCalls(bool has) { this->hasImplicitCalls = has;}
    void                SetHasImplicitCallsOnSelfAndParents() 
                        { 
                            this->SetHasImplicitCalls(true);
                            Func *curFunc = this->GetParentFunc();
                            while (curFunc && !curFunc->IsTopFunc())
                            {
                                curFunc->SetHasImplicitCalls(true);
                                curFunc = curFunc->GetParentFunc();
                            }
                        }

    bool                GetHasTempObjectProducingInstr() const { return this->hasTempObjectProducingInstr; }
    void                SetHasTempObjectProducingInstr(bool has) { this->hasTempObjectProducingInstr = has; }

    Js::ReadOnlyDynamicProfileInfo * GetProfileInfo() const { return this->profileInfo; }
    bool                HasProfileInfo() { return this->profileInfo->HasProfileInfo(); }    
    bool                HasArrayInfo() 
    { 
        const auto top = this->GetTopFunc();
        return this->HasProfileInfo() && this->GetWeakFuncRef() && !(top->HasTry() && !top->DoOptimizeTryCatch()) &&
            top->DoGlobOpt() && !PHASE_OFF(Js::LoopFastPathPhase, top);
    }

    static Js::BuiltinFunction GetBuiltInIndex(IR::Opnd* opnd)
    {
        Assert(opnd);
        Js::BuiltinFunction index;
        if (opnd->IsRegOpnd())
        {
            index = opnd->AsRegOpnd()->m_sym->m_builtInIndex;
        }
        else if (opnd->IsSymOpnd())
        {
            PropertySym *propertySym = opnd->AsSymOpnd()->m_sym->AsPropertySym();
            index = Js::JavascriptLibrary::GetBuiltinFunctionForPropId(propertySym->m_propertyId);
        }
        else
        {
            index = Js::BuiltinFunction::None;
        }
        return index;
    }

    static bool IsBuiltInInlinedInLowerer(IR::Opnd* opnd)
    {
        Assert(opnd);
        Js::BuiltinFunction index = Func::GetBuiltInIndex(opnd);
        switch (index)
        {
        case Js::BuiltinFunction::String_CharAt:
        case Js::BuiltinFunction::String_CharCodeAt:
        case Js::BuiltinFunction::String_CodePointAt:
        case Js::BuiltinFunction::Math_Abs:
        case Js::BuiltinFunction::Array_Push:
        case Js::BuiltinFunction::String_Replace:
            return true;

        default:
            return false;
        }
    }

    void AddYieldOffsetResumeLabel(uint32 offset, IR::LabelInstr* label)
    {
        m_yieldOffsetResumeLabelList->Add(YieldOffsetResumeLabel(offset, label));
    }

    template <typename Fn>
    void MapYieldOffsetResumeLabels(Fn fn)
    {
        m_yieldOffsetResumeLabelList->Map(fn);
    }

    template <typename Fn>
    bool MapUntilYieldOffsetResumeLabels(Fn fn)
    {
        return m_yieldOffsetResumeLabelList->MapUntil(fn);
    }

    void RemoveYieldOffsetResumeLabel(const YieldOffsetResumeLabel& yorl)
    {
        m_yieldOffsetResumeLabelList->Remove(yorl);
    }

    IR::Instr * GetFunctionEntryInsertionPoint();
    IR::IndirOpnd * GetConstantAddressIndirOpnd(void * address, IR::AddrOpndKind kind, IRType type, Js::OpCode loadOpCode);
    void MarkConstantAddressSyms(BVSparse<JitArenaAllocator> * bv);
    void DisableConstandAddressLoadHoist() { canHoistConstantAddressLoad = false; }

#if DBG   
    bool                allowRemoveBailOutArgInstr;
#endif

#if defined(_M_ARM32_OR_ARM64)
    int32               GetInlineeArgumentStackSize() 
    {
        int32 count = this->GetMaxInlineeArgOutCount();
        if (count)
        {
            return ((count + 1) * MachPtr); //+1 for the dedicated zero out argc slot
        }
        return 0;
    }
#endif

public: 
    BVSparse<JitArenaAllocator> *  argObjSyms;
    BVSparse<JitArenaAllocator> *  m_nonTempLocalVars;  // Only populated in debug mode as part of IRBuilder. Used in GlobOpt and BackwardPass.
    InlineeFrameInfo*              frameInfo;
private:
#ifdef PROFILE_EXEC
    Js::ScriptContextProfiler *const m_codeGenProfiler;
#endif
    Js::FunctionBody*   m_jnFunction;
    Func * const        parentFunc;  
    StackSym *          m_inlineeFrameStartSym;     
    uint                maxInlineeArgOutCount;
    const bool          m_isBackgroundJIT;
    bool                hasInstrNumber;  
    bool                maintainByteCodeOffset;
    bool                hasInlinee;
    bool                thisOrParentInlinerHasArguments;
    bool                useRuntimeStats;    
    bool                stackNestedFunc;
    bool                stackClosure;
    bool                hasAnyStackNestedFunc;
    bool                hasMarkTempObjects;
    Cloner *            m_cloner;
    InstrMap *          m_cloneMap;          
    Js::ReadOnlyDynamicProfileInfo *const profileInfo;
    NativeCodeData::Allocator       nativeCodeDataAllocator;
    NativeCodeData::Allocator       transferDataAllocator;
    CodeGenNumberAllocator *        numberAllocator;
    int32           m_localVarSlotsOffset;
    int32           m_hasLocalVarChangedOffset;    // Offset on stack of 1 byte which indicates if any local var has changed.
    CodeGenAllocators *const m_codeGenAllocators;
    YieldOffsetResumeLabelList * m_yieldOffsetResumeLabelList;
    
    StackSym *CreateInlineeStackSym();
    IR::SymOpnd *GetInlineeOpndAtOffset(int32 offset);
    bool HasLocalVarSlotCreated() const { return m_localVarSlotsOffset != Js::Constants::InvalidOffset; }
    void EnsureLocalVarSlots();
    
    SList<IR::RegOpnd *> constantAddressRegOpnd;
    IR::Instr * lastConstantAddressRegLoadInstr;
    bool canHoistConstantAddressLoad;
#if DBG
    VtableHashMap * vtableMap;
#endif
};

class AutoCodeGenPhase
{
public:
    AutoCodeGenPhase(Func * func, Js::Phase phase) : func(func), phase(phase), dump(false), isPhaseComplete(false)
    {
        func->BeginPhase(phase);
    }
    ~AutoCodeGenPhase()
    {
        if(this->isPhaseComplete)
        {
            func->EndPhase(phase, dump);
        }
        else
        {
            //End the profiler tag
            func->EndProfiler(phase);
        }
    }
    void EndPhase(Func * func, Js::Phase phase, bool dump, bool isPhaseComplete)
    {
        Assert(this->func == func);
        Assert(this->phase == phase);
        this->dump = dump && (PHASE_DUMP(Js::SimpleJitPhase, func->GetJnFunction()) || !func->IsSimpleJit());
        this->isPhaseComplete = isPhaseComplete;
    }
private:
    Func * func;
    Js::Phase phase;
    bool dump;
    bool isPhaseComplete;
};
#define BEGIN_CODEGEN_PHASE(func, phase) { AutoCodeGenPhase __autoCodeGen(func, phase); 
#define END_CODEGEN_PHASE(func, phase) __autoCodeGen.EndPhase(func, phase, true, true); }
#define END_CODEGEN_PHASE_NO_DUMP(func, phase) __autoCodeGen.EndPhase(func, phase, false, true); }
