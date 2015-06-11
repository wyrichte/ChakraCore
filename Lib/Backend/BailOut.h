//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once


class BailOutInfo
{
public:

#ifdef _M_IX86
    typedef struct
    {
        IR::Instr * instr;
        uint argCount;
        uint argRestoreAdjustCount;
    } StartCallInfo;
#else
    typedef uint StartCallInfo;
#endif

    BailOutInfo(uint32 bailOutOffset, Func* bailOutFunc) :
        bailOutOffset(bailOutOffset), bailOutFunc(bailOutFunc),
        byteCodeUpwardExposedUsed(null), polymorphicCacheIndex((uint)-1), startCallCount(0), startCallInfo(null), bailOutInstr(null),
        totalOutParamCount(0), argOutSyms(null), bailOutRecord(null), wasCloned(false), isInvertedBranch(false), sharedBailOutKind(true), outParamInlinedArgSlot(null), 
        liveVarSyms(null), liveLosslessInt32Syms(null), liveFloat64Syms(null), branchConditionOpnd(null),
        stackLiteralBailOutInfoCount(0), stackLiteralBailOutInfo(null)
    {      
        Assert(bailOutOffset != Js::Constants::NoByteCodeOffset);
#ifdef _M_IX86
        outParamFrameAdjustArgSlot = null;
#endif
#if DBG
        wasCopied = false;
#endif
        this->capturedValues.argObjSyms = null;
        this->usedCapturedValues.argObjSyms = null;
    }
    void Clear(JitArenaAllocator * allocator);

    void FinalizeBailOutRecord(Func * func);
#ifdef MD_GROW_LOCALS_AREA_UP  
    void FinalizeOffsets(__in_ecount(count) int * offsets, uint count, Func *func, BVSparse<JitArenaAllocator> *bvInlinedArgSlot);
#endif

    void RecordStartCallInfo(uint i, uint argRestoreAdjustCount, IR::Instr *instr);
    uint GetStartCallOutParamCount(uint i) const;
#ifdef _M_IX86
    bool NeedsStartCallAdjust(uint i, const IR::Instr * instr) const;
    void UnlinkStartCall(const IR::Instr * instr);
#endif

    static bool IsBailOutOnImplicitCalls(IR::BailOutKind kind)
    {
        const IR::BailOutKind kindMinusBits = kind & ~IR::BailOutKindBits;
        return kindMinusBits == IR::BailOutOnImplicitCalls ||
            kindMinusBits == IR::BailOutOnImplicitCallsPreOp;
    }

#if DBG
    static bool IsBailOutHelper(IR::JnHelperMethod helper);
#endif
    bool wasCloned;     
    bool isInvertedBranch;
    bool sharedBailOutKind;

#if DBG
    bool wasCopied;
#endif
    uint32 bailOutOffset;
    BailOutRecord * bailOutRecord;
    CapturedValues capturedValues;                                      // Values we know about after forward pass
    CapturedValues usedCapturedValues;                                  // Values that need to be restored in the bail out    
    BVSparse<JitArenaAllocator> * byteCodeUpwardExposedUsed;               // Non-constant stack syms that needs tob e restored in the bail out    
    uint polymorphicCacheIndex;
    uint startCallCount;
    uint totalOutParamCount;   
    Func ** startCallFunc;
    
    StartCallInfo * startCallInfo;
    StackSym ** argOutSyms;

    struct StackLiteralBailOutInfo
    {
        StackSym * stackSym;
        uint initFldCount;  
    };
    uint stackLiteralBailOutInfoCount;
    StackLiteralBailOutInfo * stackLiteralBailOutInfo;

    BVSparse<JitArenaAllocator> * liveVarSyms;
    BVSparse<JitArenaAllocator> * liveLosslessInt32Syms;                   // These are only the live int32 syms that fully represent the var-equivalent sym's value (see GlobOpt::FillBailOutInfo)
    BVSparse<JitArenaAllocator> * liveFloat64Syms;

    int * outParamOffsets;

    BVSparse<JitArenaAllocator> * outParamInlinedArgSlot;
#ifdef _M_IX86
    BVSparse<JitArenaAllocator> * outParamFrameAdjustArgSlot;     
    BVFixed * inlinedStartCall;
#endif

#ifdef MD_GROW_LOCALS_AREA_UP
    // Use a bias to guarantee that all sym offsets are non-zero.
    static const int32 StackSymBias = MachStackAlignment;
#endif

    // The actual bailout instr, this is normally the instr that has the bailout info.
    // 1) If we haven't generated bailout (which happens in lowerer) for this bailout info, this is either of:
    // - the instr that has bailout info.
    // - in case of shared bailout this will be the BailTarget instr (corresponds to the call to SaveReesgtersAndBailOut, 
    //   while other instrs sharing bailout info will just have checks and JMP to BailTarget). 
    // 2) After we generated bailout, this becomes label instr. In case of shared bailout other instrs JMP to this label.
    IR::Instr * bailOutInstr;

#if ENABLE_DEBUG_CONFIG_OPTIONS
    Js::OpCode bailOutOpcode;
#endif
    Func * bailOutFunc;
    IR::Opnd * branchConditionOpnd;

    template<class Fn>
    void IterateArgSyms(Fn callback)
    {
        uint argOutIndex = 0;
        for(uint i = 0; i < this->startCallCount; i++)
        {
            uint outParamCount = this->GetStartCallOutParamCount(i);
            for(uint j = 0; j < outParamCount; j++)
            {
                StackSym* sym = this->argOutSyms[argOutIndex];
                if(sym)
                {
                    callback(argOutIndex, sym);
                }
                argOutIndex++;
            }
        }
    }
};

class BailOutRecord
{
public:
    BailOutRecord(uint32 bailOutOffset, uint bailOutCacheIndex, IR::BailOutKind kind, Func *bailOutFunc);
    static Js::Var BailOut(BailOutRecord const * bailOutRecord);
    static Js::Var BailOutFromFunction(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord, void * returnAddress, void * argoutRestoreAddress);
    static uint32 BailOutFromLoopBody(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord);

    static Js::Var BailOutInlined(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord, void * returnAddress);
    static uint32 BailOutFromLoopBodyInlined(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord, void * returnAddress);

    static size_t GetOffsetOfRegisterSaveSpace() { return offsetof(BailOutRecord, registerSaveSpace); }
    static size_t GetOffsetOfPolymorphicCacheIndex() { return offsetof(BailOutRecord, polymorphicCacheIndex); }
    static size_t GetOffsetOfBailOutKind() { return offsetof(BailOutRecord, bailOutKind); }

    static bool IsArgumentsObject(uint32 offset);
    static uint32 GetArgumentsObjectOffset();
    static const uint BailOutRegisterSaveSlotCount = LinearScanMD::RegisterSaveSlotCount;

public:
    template <size_t N>
    void FillNativeRegToByteCodeRegMap(uint (&nativeRegToByteCodeRegMap)[N]);

    void IsOffsetNativeIntOrFloat(uint offsetIndex, bool isLocal, int argOutSlotStart, bool * pIsFloat64, bool * pIsInt32) const;

    template <typename Fn>
    void MapStartCallParamCounts(Fn fn);

    void SetBailOutKind(IR::BailOutKind bailOutKind) { this->bailOutKind = bailOutKind; }
    uint32 GetBailOutOffset() { return bailOutOffset; }
#if ENABLE_DEBUG_CONFIG_OPTIONS
    Js::OpCode GetBailOutOpCode() { return bailOutOpcode; }
#endif
    template <typename Fn>
    void MapArgOutOffsets(Fn fn);

protected:     
    struct BailOutReturnValue
    {
        Js::Var returnValue;
        Js::RegSlot returnValueRegSlot;
    };
    static Js::Var BailOutCommonNoCodeGen(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord,
        uint32 bailOutOffset, void * returnAddress, IR::BailOutKind bailOutKind, Js::Var * registerSaves = null,
        BailOutReturnValue * returnValue = null, void * argoutRestoreAddress = nullptr);
    static Js::Var BailOutCommon(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord,
        uint32 bailOutOffset, void * returnAddress, IR::BailOutKind bailOutKind, BailOutReturnValue * returnValue = null, void * argoutRestoreAddress = nullptr);
    static Js::Var BailOutInlinedCommon(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord,
        uint32 bailOutOffset, void * returnAddress, IR::BailOutKind bailOutKind);
    static uint32 BailOutFromLoopBodyCommon(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord,
        uint32 bailOutOffset, IR::BailOutKind bailOutKind);
    static uint32 BailOutFromLoopBodyInlinedCommon(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord,
        uint32 bailOutOffset, void * returnAddress, IR::BailOutKind bailOutKind);

    static Js::Var BailOutHelper(Js::JavascriptCallStackLayout * layout, Js::ScriptFunction ** functionRef, Js::Arguments& args, const bool boxArgs,
        BailOutRecord const * bailOutRecord, uint32 bailOutOffset, void * returnAddress, IR::BailOutKind bailOutKind, Js::Var * registerSaves, BailOutReturnValue * returnValue, Js::Var* pArgumentsObject,
        void * argoutRestoreAddress = nullptr);
    static void BailOutInlinedHelper(Js::JavascriptCallStackLayout * layout, BailOutRecord const *& bailOutRecord,
        uint32 bailOutOffset, void * returnAddress, IR::BailOutKind bailOutKind, Js::Var * registerSaves, BailOutReturnValue * returnValue, Js::ScriptFunction ** innerMostInlinee);
    static uint32 BailOutFromLoopBodyHelper(Js::JavascriptCallStackLayout * layout, BailOutRecord const * bailOutRecord,
        uint32 bailOutOffset, IR::BailOutKind bailOutKind, Js::Var * registerSaves = null, BailOutReturnValue * returnValue = null);

    static void UpdatePolymorphicFieldAccess(Js::JavascriptFunction *  function, BailOutRecord const * bailOutRecord);

    static void ScheduleFunctionCodeGen(Js::ScriptFunction * function, Js::ScriptFunction * innerMostInlinee, BailOutRecord const * bailOutRecord, IR::BailOutKind bailOutKind, void * returnAddress);
    static void ScheduleLoopBodyCodeGen(Js::ScriptFunction * function, Js::ScriptFunction * innerMostInlinee, BailOutRecord const * bailOutRecord, IR::BailOutKind bailOutKind);

    void RestoreValues(IR::BailOutKind bailOutKind, Js::JavascriptCallStackLayout * layout, Js::InterpreterStackFrame * newInstance, Js::ScriptContext * scriptContext,
        bool fromLoopBody, Js::Var * registerSaves, BailOutReturnValue * returnValue, Js::Var* pArgumentsObject, void* returnAddress = null, bool useStartCall = true, void * argoutRestoreAddress = nullptr) const;
    void RestoreValues(IR::BailOutKind bailOutKind, Js::JavascriptCallStackLayout * layout, uint count, __in_ecount(count) int * offsets, int argOutSlotId,
        __out_ecount(count)Js::Var * values, Js::ScriptContext * scriptContext, bool fromLoopBody, Js::Var * registerSaves, Js::InterpreterStackFrame * newInstance, Js::Var* pArgumentsObject,
        void * argoutRestoreAddress = nullptr) const;
    void RestoreInlineFrame(InlinedFrameLayout *inlinedFrame, Js::JavascriptCallStackLayout * layout, Js::Var * registerSaves);

    void AdjustOffsetsForDiagMode(Js::JavascriptCallStackLayout * layout, Js::ScriptFunction * function) const;

    Js::Var EnsureArguments(Js::InterpreterStackFrame * newInstance, Js::JavascriptCallStackLayout * layout, Js::ScriptContext* scriptContext, Js::Var* pArgumentsObject) const;

    Js::JavascriptCallStackLayout *GetStackLayout() const;

    // The offset to 'registerSaveSpace' is hard-coded in LinearScanMD::SaveAllRegisters, so let this be the first member variable
    Js::Var *const registerSaveSpace;

    BailOutRecord * parent;
    Js::Var * constants;
    BVFixed * losslessInt32Syms;
    BVFixed * float64Syms;

    // Index of start of section of argOuts for current record/current func, used with argOutFloat64Syms and 
    // argOutLosslessInt32Syms when restoring values, as BailOutInfo uses one argOuts array for all funcs. 
    // Similar to outParamOffsets which points to current func section for the offsets.
    // TODO: make this simpler.
    uint argOutSymStart;

    BVFixed * argOutFloat64Syms;        // Used for float-type-specialized ArgOut symbols. Index = [0 .. BailOutInfo::totalOutParamCount].
    BVFixed * argOutLosslessInt32Syms;  // Used for int-type-specialized ArgOut symbols (which are native int and for bailout we need tagged ints).
    int * localOffsets;

    Js::RegSlot minLocalSyms;
    Js::RegSlot localOffsetsCount;
    uint32 const bailOutOffset;

    struct StackLiteralBailOutRecord
    {
        Js::RegSlot regSlot;
        uint initFldCount;
    };
    uint stackLiteralBailOutRecordCount;
    StackLiteralBailOutRecord * stackLiteralBailOutRecord;

    uint * startCallOutParamCounts;
#ifdef _M_IX86
    uint * startCallArgRestoreAdjustCounts;
#endif
    int * outParamOffsets;    
    uint startCallCount;       

    // inlinee
    Js::RegSlot returnValueRegSlot;    
    int32       firstActualStackOffset;
    const bool isInlinedFunction;
    const bool isInlinedConstructor;

    const bool isLoopBody;

    mutable Js::Var branchValue;
    Js::RegSlot branchValueRegSlot;

    uint polymorphicCacheIndex;
    ushort bailOutCount;
    IR::BailOutKind bailOutKind;

    Js::EHBailoutData * ehBailoutData;
#if DBG
    Js::ArgSlot actualCount;
    uint constantCount;
    int inlineDepth;
    void DumpValues(uint count, int* offsets, int argOutSlotStart);
    void DumpValue(int offset, bool isFloat64);
#endif
    friend class LinearScan;
    friend class BailOutInfo;
    friend struct FuncBailOutData;
#if ENABLE_DEBUG_CONFIG_OPTIONS
public:
    Js::OpCode bailOutOpcode;
#if DBG
   void Dump();
#endif
#endif
};

class BranchBailOutRecord : public BailOutRecord
{
public:
    BranchBailOutRecord(uint32 trueBailOutOffset, uint32 falseBailOutOffset, Js::RegSlot resultByteCodeReg, IR::BailOutKind kind, Func *bailOutFunc);

    static Js::Var BailOut(BranchBailOutRecord const * bailOutRecord, BOOL cond);
    static Js::Var BailOutFromFunction(Js::JavascriptCallStackLayout * layout, BranchBailOutRecord const * bailOutRecord, BOOL cond, void * returnAddress, void * argoutRestoreAddress);
    static uint32 BailOutFromLoopBody(Js::JavascriptCallStackLayout * layout, BranchBailOutRecord const * bailOutRecord, BOOL cond);  

    static Js::Var BailOutInlined(Js::JavascriptCallStackLayout * layout, BranchBailOutRecord const * bailOutRecord, BOOL cond, void * returnAddress);
    static uint32 BailOutFromLoopBodyInlined(Js::JavascriptCallStackLayout * layout, BranchBailOutRecord const * bailOutRecord, BOOL cond, void * returnAddress);
private:
    uint falseBailOutOffset;
};

class FunctionBailOutRecord
{
public:
    FunctionBailOutRecord() : constantCount(0), constants(null) {}    

    uint constantCount;
    Js::Var * constants;  
};

template <size_t N>
inline void BailOutRecord::FillNativeRegToByteCodeRegMap(uint (&nativeRegToByteCodeRegMap)[N])
{
    static_assert(RegNumCount == N, "register map array size needs to match current architecture's RegisterSaveSlotCount");

    if (this->localOffsets)
    {
        for (uint i = 0; i < this->localOffsetsCount; i++)
        {
            int offset = this->localOffsets[i];

            if (offset > 0 && (uint)offset < GetBailOutRegisterSaveSlotCount())
            {
                nativeRegToByteCodeRegMap[LinearScanMD::GetRegisterFromSaveIndex(offset)] = i + this->minLocalSyms;
            }
        }
    }

    // Do not need to check outParamOffsets for any that map to native registers because they are always located on the stack
}

template <typename Fn>
inline void BailOutRecord::MapStartCallParamCounts(Fn fn)
{
    for (uint i = 0; i < this->startCallCount; i++)
    {
        fn(this->startCallOutParamCounts[i]);
    }
}

template <typename Fn>
inline void BailOutRecord::MapArgOutOffsets(Fn fn)
{
    uint outParamSlot = 0;
    uint argOutSlotOffset = 0;

    for (uint i = 0; i < this->startCallCount; i++)
    {
        uint startCallOutParamCount = this->startCallOutParamCounts[i];
        argOutSlotOffset += 1; // skip pointer to self which is pushed by OP_StartCall

        for (uint j = 0; j < startCallOutParamCount; j++, outParamSlot++, argOutSlotOffset++)
        {
            if (this->outParamOffsets[outParamSlot] != 0)
            {
                fn(argOutSlotOffset, this->outParamOffsets[outParamSlot]);
            }
        }
    }
}

