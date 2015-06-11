//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

extern "C" PVOID _ReturnAddress(VOID);
#pragma intrinsic(_ReturnAddress)
class BailOutRecord;


extern "C" void __cdecl _alloca_probe_16();
namespace Js
{
    
    enum InterpreterStackFrameFlags : UINT16
    {
        InterpreterStackFrameFlags_None = 0,
        InterpreterStackFrameFlags_WithinTryBlock = 1,
        InterpreterStackFrameFlags_WithinCatchBlock = 2,
        InterpreterStackFrameFlags_WithinFinallyBlock = 4,
        InterpreterStackFrameFlags_FromBailOut = 8,
        InterpreterStackFrameFlags_ProcessingBailOutFromEHCode = 0x10,
        InterpreterStackFrameFlags_All = 0xFFFF,
    };
    struct InterpreterStackFrame   /* Stack allocated, no virtuals */
    {
        PREVENT_COPY(InterpreterStackFrame)

        friend class BailOutRecord;
        friend class JavascriptGeneratorFunction;
        friend class JavascriptGenerator;

        class Setup
        {
        public:
            Setup(ScriptFunction * function, Arguments& args);
            Setup(ScriptFunction * function, Var * inParams, int inSlotsCount);
            size_t GetAllocationVarCount() const { return varAllocCount; }

            InterpreterStackFrame * AllocateAndInitialize(bool doProfile, bool * releaseAlloc);

#if DBG
            InterpreterStackFrame * InitializeAllocation(__in_ecount(varAllocCount) Var * allocation, bool initParams, bool profileParams, Var loopHeaderArray, DWORD_PTR stackAddr, Var invalidStackVar);
#else
            InterpreterStackFrame * InitializeAllocation(__in_ecount(varAllocCount) Var * allocation, bool initParams, bool profileParams, Var loopHeaderArray, DWORD_PTR stackAddr);
#endif
            uint GetLocalCount() const { return localCount; }

        private:
            template <class Fn>
            void InitializeParams(InterpreterStackFrame * newInstance, Fn callback, Var **pprestDest);
            template <class Fn>
            void InitializeParamsAndUndef(InterpreterStackFrame * newInstance, Fn callback, Var **pprestDest);
            void InitializeRestParam(InterpreterStackFrame * newInstance, Var *dest);
            void SetupInternal();
            int inSlotsCount;
            Var * inParams;
            uint localCount;
            uint varAllocCount;
            ScriptFunction * const function;
            FunctionBody * const executeFunction;
            void** inlineCaches;
            uint inlineCacheCount;
        };
    private:
        ByteCodeReader m_reader;        // Reader for current function
        int m_inSlotsCount;             // Count of actual incoming parameters to this function
        Var* m_inParams;               // Range of 'in' parameters        
        Var* m_outParams;              // Range of 'out' parameters (offset in m_localSlots)
        Var* m_outIntParams;              // Range of 'out' parameters (offset in m_localSlots)
        Var* m_outSp;                  // Stack pointer for next outparam
        Var* m_outIntSp;                  // Stack pointer for next outparam
        Var  m_arguments;              // Dedicated location for this frame's arguments object
        StackScriptFunction * stackNestedFunctions;
        FrameDisplay * localFrameDisplay;
        Var * localScopeSlots;
        ScriptContext* scriptContext;
        ScriptFunction * function;
        FunctionBody * m_functionBody;
        void** inlineCaches;
        uint inlineCacheCount;
        void * returnAddress;
        void * addressOfReturnAddress;  // Tag this frame with stack position, used by (remote) stack walker to test partially initialized interpreter stack frame.
        InterpreterStackFrame *previousInterpreterFrame;
        uint currentLoopNum;
        Var  loopHeaderArray;          // Keeps alive any JITted loop bodies while the function is being interpreted
        uint currentLoopCounter;       // This keeps tracks of loopiteration, how many times the current loop is executed. Its hit only in cases where jitloopbodies are not hit
                                       // such as loops inside try\catch.

        // 'stack address' of the frame, used for recursion detection during stepping.
        // For frames created via interpreter path, we use 'this', for frames created by bailout we use stack addr of actual jitted frame
        // the interpreter frame is created for.
        DWORD_PTR m_stackAddress;

        ImplicitCallFlags * savedLoopImplicitCallFlags;
        UINT16 m_flags;                // based on InterpreterStackFrameFlags

        bool switchProfileMode;
        bool isAutoProfiling;
        uint32 switchProfileModeOnLoopEndNumber;
        int16 nestedTryDepth;
        int16 nestedCatchDepth;
        int16 nestedFinallyDepth;
        uint retOffset;

        void (InterpreterStackFrame::*opLoopBodyStart)(uint32 loopNumber, LayoutSize layoutSize, bool isFirstIteration);
        void (InterpreterStackFrame::*opProfiledLoopBodyStart)(uint32 loopNumber, LayoutSize layoutSize, bool isFirstIteration);
#if DBG || DBG_DUMP
        void * DEBUG_currentByteOffset;
#endif

        // Asm.js stack pointer
        int* m_localIntSlots;
        double* m_localDoubleSlots;
        float* m_localFloatSlots;

#ifdef SIMD_JS_ENABLED
         _SIMDValue* m_localSimdSlots;
        
#endif
        EHBailoutData * ehBailoutData;

        //////////////////////////////////////////////////////////////////////////

        // 16-byte aligned
        __declspec(align(16)) Var m_localSlots[0];           // Range of locals and temporaries

        static const int LocalsThreshold = 32 * 1024; // Number of locals vars we'll allocate on the frame.
                                                      // If there are more, we'll use an arena.

        typedef void(InterpreterStackFrame::*ArrFunc)(uint32, RegSlot);

        static const ArrFunc StArrFunc[8];
        static const ArrFunc LdArrFunc[8];

        //This class must have an empty ctor (otherwise it will break the code in InterpreterStackFrame::InterpreterThunk
        inline InterpreterStackFrame() { }

        void ProcessTryFinally(const byte* ip, Js::JumpOffset jumpOffset, Js::RegSlot regException = Js::Constants::NoRegister, Js::RegSlot regOffset = Js::Constants::NoRegister, bool hasYield = false);
    public:
        inline void OP_SetOutAsmDb(RegSlot outRegisterID, double val);
        inline void OP_SetOutAsmInt(RegSlot outRegisterID, int val);
        inline void OP_I_SetOutAsmInt(RegSlot outRegisterID, int val);
        inline void OP_I_SetOutAsmDb(RegSlot outRegisterID, double val);
        inline void OP_I_SetOutAsmFlt(RegSlot outRegisterID, float val);
#ifdef SIMD_JS_ENABLED
        inline void OP_I_SetOutAsmSimd(RegSlot outRegisterID, AsmJsSIMDValue val);
#endif

        inline void SetOut(ArgSlot outRegisterID, Var bValue);
        inline void SetOut(ArgSlot_OneByte outRegisterID, Var bValue);
        inline void PushOut(Var aValue);
        inline void PopOut(ArgSlot argCount);

        void ValidateRegValue(Var value, bool allowStackVar = false, bool allowStackVarOnDisabledStackNestedFunc = true) const;
        void ValidateSetRegValue(Var value, bool allowStackVar = false, bool allowStackVarOnDisabledStackNestedFunc = true) const;
        template <typename RegSlotType> Var GetReg(RegSlotType localRegisterID) const;
        template <typename RegSlotType> void SetReg(RegSlotType localRegisterID, Var bValue);
        template <typename RegSlotType> Var GetRegAllowStackVar(RegSlotType localRegisterID) const;
        template <typename RegSlotType> void SetRegAllowStackVar(RegSlotType localRegisterID, Var bValue);
        template <typename RegSlotType> int GetRegRawInt( RegSlotType localRegisterID ) const;
        template <typename RegSlotType> void SetRegRawInt( RegSlotType localRegisterID, int bValue );
        template <typename RegSlotType> double GetRegRawDouble(RegSlotType localRegisterID) const;
        template <typename RegSlotType> float GetRegRawFloat(RegSlotType localRegisterID) const;
        template <typename RegSlotType> void SetRegRawDouble(RegSlotType localRegisterID, double bValue);
        template <typename RegSlotType> void SetRegRawFloat(RegSlotType localRegisterID, float bValue);
        template <typename T> T GetRegRaw( RegSlot localRegisterID ) const;
        template <typename T> void SetRegRaw( RegSlot localRegisterID, T bValue );


#ifdef SIMD_JS_ENABLED
        //template<>  void SetRegRaw<AsmJsSIMDValue>(RegSlot localRegisterID, AsmJsSIMDValue bValue);
        template <typename RegSlotType> AsmJsSIMDValue GetRegRawSimd(RegSlotType localRegisterID) const;
        template <typename RegSlotType> void           SetRegRawSimd(RegSlotType localRegisterID, AsmJsSIMDValue bValue);
        static DWORD GetAsmSimdValOffSet(AsmJsCallStackLayout* stack);
#endif

        template <typename RegSlotType>
        inline Var GetRegAllowStackVarEnableOnly(RegSlotType localRegisterID) const;
        template <typename RegSlotType>
        inline void SetRegAllowStackVarEnableOnly(RegSlotType localRegisterID, Var bValue);

        inline Var GetNonVarReg(RegSlot localRegisterID) const;
        inline void SetNonVarReg(RegSlot localRegisterID, void * bValue);
        inline ScriptContext* GetScriptContext() const { return scriptContext; }
        Var GetRootObject() const;
        inline ScriptFunction* GetJavascriptFunction() const { return function; }
        inline FunctionBody * GetFunctionBody() const { return m_functionBody; }
        inline ByteCodeReader* GetReader() { return &m_reader;}
        inline uint GetCurrentLoopNum() const { return currentLoopNum; }
        inline InterpreterStackFrame* GetPreviousFrame() const {return previousInterpreterFrame;}
        inline void SetPreviousFrame(InterpreterStackFrame *interpreterFrame) {previousInterpreterFrame = interpreterFrame;}
        inline Var GetArgumentsObject() const { return m_arguments; }
        inline void SetArgumentsObject(Var args) { m_arguments = args; }
        inline UINT16 GetFlags() const { return m_flags; }
        inline void OrFlags(UINT16 addTo) { m_flags |= addTo; }
        bool IsInCatchOrFinallyBlock();
        static bool IsDelayDynamicInterpreterThunk(void* entryPoint);

        inline Var CreateHeapArguments(ScriptContext* scriptContext);

        bool IsCurrentLoopNativeAddr(void * codeAddr) const;
        void * GetReturnAddress() { return returnAddress; }

        static size_t GetOffsetOfLocals() { return offsetof(InterpreterStackFrame, m_localSlots); }
        static size_t GetOffsetOfArguments() { return offsetof(InterpreterStackFrame, m_arguments); }
        static size_t GetOffsetOfInParams() { return offsetof(InterpreterStackFrame, m_inParams); }
        static size_t GetOffsetOfInSlotsCount() { return offsetof(InterpreterStackFrame, m_inSlotsCount); }
        void PrintStack(const int* const intSrc, const float* const fltSrc, const double* const dblSrc, int intConstCount, int floatConstCount, int doubleConstCount, const wchar_t* state);

        static uint32 GetStartLocationOffset() { return offsetof(InterpreterStackFrame, m_reader) + ByteCodeReader::GetStartLocationOffset(); }
        static uint32 GetCurrentLocationOffset() { return offsetof(InterpreterStackFrame, m_reader) + ByteCodeReader::GetCurrentLocationOffset(); }

        static bool IsBrLong(OpCode op, const byte * ip)
        {
#ifdef BYTECODE_BRANCH_ISLAND
            return (op == OpCode::ExtendedOpcodePrefix) && ((OpCode)(ByteCodeReader::PeekByteOp(ip) + (OpCode::ExtendedOpcodePrefix << 8)) == OpCode::BrLong);
#else
            return false;
#endif
        }

        DWORD_PTR GetStackAddress() const;
        void* GetAddressOfReturnAddress() const;   
        
#if _M_IX86
        static int GetRetType(JavascriptFunction* func);
        static int GetAsmJsArgSize(AsmJsCallStackLayout * stack);
        static int GetDynamicRetType(AsmJsCallStackLayout * stack);
        static DWORD GetAsmIntDbValOffSet(AsmJsCallStackLayout * stack);
        __declspec(noinline)   static int  AsmJsInterpreter(AsmJsCallStackLayout * stack);
#elif _M_X64
        template <typename T>
        static T AsmJsInterpreter(AsmJsCallStackLayout* layout);
        static void * GetAsmJsInterpreterEntryPoint(AsmJsCallStackLayout* stack);
        template <typename T>
        static T GetAsmJsRetVal(InterpreterStackFrame* instance);

        static Var AsmJsDelayDynamicInterpreterThunk(RecyclableObject* function, CallInfo callInfo, ...);
#ifdef SIMD_JS_ENABLED
        static __m128 AsmJsInterpreterSimdJs(AsmJsCallStackLayout* func);
#endif
#endif

#ifdef ASMJS_PLAT
        static void InterpreterAsmThunk(AsmJsCallStackLayout* layout);
#endif

#if DYNAMIC_INTERPRETER_THUNK
        static Var DelayDynamicInterpreterThunk(RecyclableObject* function, CallInfo callInfo, ...);
        __declspec(noinline) static Var InterpreterThunk(JavascriptCallStackLayout* layout);  
        static Var InterpreterHelper(ScriptFunction* function, ArgumentReader args, void* returnAddress, void* addressOfReturnAddress, const bool isAsmJs = false);
#else
        __declspec(noinline) static Var InterpreterThunk(RecyclableObject* function, CallInfo callInfo, ...);     
        static Var InterpreterHelper(RecyclableObject* function, ArgumentReader args,void* returnAddress,void* addressOfReturnAddress, const bool isAsmJs = false);
#endif
    private:
#if DYNAMIC_INTERPRETER_THUNK
        static JavascriptMethod EnsureDynamicInterpreterThunk(Js::ScriptFunction * function);
#endif
        template<typename T>
        T ReadByteOp( const byte *& ip
#if DBG_DUMP
                           , bool isExtended = false
#endif 
                           );

        inline void* __cdecl operator new(size_t byteSize, void* previousAllocation) throw();
        inline void __cdecl operator delete(void* allocationToFree, void* previousAllocation) throw();


        __declspec(noinline) Var ProcessThunk();
        __declspec(noinline) Var DebugProcessThunk();
        __declspec(noinline) Var LanguageServiceProcessThunk();

        void AlignMemoryForAsmJs();

        Var Process();
        Var ProcessAsmJsModule();
        Var ProcessLinkFailedAsmJsModule();
        Var ProcessAsmJs();
        Var ProcessProfiled();
        Var ProcessUnprofiled();

        Var ProcessWithDebugging();
        Var DebugProcess();
        Var ProcessForLanguageService(RegSlot& target, ArgSlot& popCount);
        Var LanguageServiceProcess();

        // This will be called for reseting outs when resume from break on error happened
        void ResetOut();

        Var OP_ArgIn0();
        template <class T> inline void OP_ArgOut_A(const unaligned T* playout);
        template <class T> void OP_ProfiledArgOut_A(const unaligned T * playout);
#if DBG
        template <class T> inline void OP_ArgOut_ANonVar(const unaligned T* playout);
#endif
        inline BOOL OP_BrFalse_A(Var aValue, ScriptContext* scriptContext);
        inline BOOL OP_BrTrue_A(Var aValue, ScriptContext* scriptContext);
        inline BOOL OP_BrNotNull_A(Var aValue);
        inline BOOL OP_BrOnHasProperty(Var argInstance, uint propertyIdIndex, ScriptContext* scriptContext);
        inline BOOL OP_BrOnNoProperty(Var argInstance, uint propertyIdIndex, ScriptContext* scriptContext);

        RecyclableObject * OP_CallGetFunc(Var target);

        template <class T> const byte * OP_Br(const unaligned T * playout);
        void OP_AsmStartCall(const unaligned OpLayoutStartCall * playout);        
        void OP_StartCall( const unaligned OpLayoutStartCall * playout );
        void OP_StartCall(uint outParamCount);
        template <class T> void OP_CallCommon(const unaligned T *playout, RecyclableObject * aFunc, unsigned flags, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        void OP_CallAsmInternal( RecyclableObject * function);
        template <class T> void OP_I_AsmCall(const unaligned T* playout) { OP_CallAsmInternal((ScriptFunction*)OP_CallGetFunc(GetRegAllowStackVar(playout->Function))); }

        template <class T> void OP_CallCommonI(const unaligned T *playout, RecyclableObject * aFunc, unsigned flags);
        template <class T> void OP_ProfileCallCommon(const unaligned T *playout, RecyclableObject * aFunc, unsigned flags, ProfileId profileId, InlineCacheIndex inlineCacheIndex = Js::Constants::NoInlineCacheIndex, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        template <class T> void OP_ProfileReturnTypeCallCommon(const unaligned T *playout, RecyclableObject * aFunc, unsigned flags, ProfileId profileId, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        template <class T> void OP_CallPutCommon(const unaligned T *playout, RecyclableObject * aFunc);
        template <class T> void OP_CallPutCommonI(const unaligned T *playout, RecyclableObject * aFunc);

        template <class T> void OP_AsmCall(const unaligned T* playout);

        template <class T> void OP_CallI(const unaligned T* playout, unsigned flags) { OP_CallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags); }
        template <class T> void OP_CallIExtended(const unaligned T* playout, unsigned flags) { OP_CallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, (playout->Options & CallIExtended_SpreadArgs) ? m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody()) : nullptr); }
        template <class T> void OP_CallIPut(const unaligned T* playout) { OP_CallPutCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function))); }

        template <class T> void OP_ProfiledCallI(const unaligned OpLayoutDynamicProfile<T>* playout, unsigned flags) { OP_ProfileCallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, playout->profileId); }
        template <class T> void OP_ProfiledCallIExtended(const unaligned OpLayoutDynamicProfile<T>* playout, unsigned flags) { OP_ProfileCallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, playout->profileId, Js::Constants::NoInlineCacheIndex, (playout->Options & CallIExtended_SpreadArgs) ? m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody()) : nullptr); }
        template <class T> void OP_ProfiledCallIWithICIndex(const unaligned OpLayoutDynamicProfile<T>* playout, unsigned flags) { OP_ProfileCallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, playout->profileId, playout->inlineCacheIndex); }
        template <class T> void OP_ProfiledCallIExtendedWithICIndex(const unaligned OpLayoutDynamicProfile<T>* playout, unsigned flags) { OP_ProfileCallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, playout->profileId, playout->inlineCacheIndex, (playout->Options & CallIExtended_SpreadArgs) ? m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody()) : nullptr); }
        template <class T> void OP_ProfiledCallIPut(const unaligned T* playout) { OP_CallPutCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function))); }

        template <class T> void OP_ProfiledReturnTypeCallI(const unaligned OpLayoutDynamicProfile<T>* playout, unsigned flags) { OP_ProfileReturnTypeCallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, playout->profileId); }
        template <class T> void OP_ProfiledReturnTypeCallIExtended(const unaligned OpLayoutDynamicProfile<T>* playout, unsigned flags) { OP_ProfileReturnTypeCallCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function)), flags, playout->profileId, (playout->Options & CallIExtended_SpreadArgs) ? m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody()) : nullptr); }
        template <class T> void OP_ProfiledReturnTypeCallIPut(const unaligned T* playout) { OP_CallPutCommon(playout, OP_CallGetFunc(GetRegAllowStackVar(playout->Function))); }

        // Patching Fastpath Operations
        template <class T> void OP_GetRootProperty(unaligned T* playout);
        template <class T> void OP_GetRootPropertyForTypeOf(unaligned T* playout);
        template <class T> void OP_GetRootProperty_NoFastPath(unaligned T* playout);
        template <class T, bool Root, bool Method, bool CallApplyTarget> void ProfiledGetProperty(unaligned T* playout, const Var instance);
        template <class T> void OP_ProfiledGetRootProperty(unaligned T* playout);
        template <class T> void OP_ProfiledGetRootPropertyForTypeOf(unaligned T* playout);
        template <class T> void OP_GetProperty(unaligned T* playout);
        template <class T> void OP_GetSuperProperty(unaligned T* playout);
        template <class T> void OP_GetPropertyForTypeOf(unaligned T* playout);
        template <class T> void OP_GetProperty_NoFastPath(unaligned T* playout);
        template <class T> void OP_ProfiledGetProperty(unaligned T* playout);
        template <class T> void OP_ProfiledGetSuperProperty(unaligned T* playout);
        template <class T> void OP_ProfiledGetPropertyForTypeOf(unaligned T* playout);
        template <class T> void OP_ProfiledGetPropertyCallApplyTarget(unaligned T* playout);
        template <class T> void OP_GetRootMethodProperty(unaligned T* playout);
        template <class T> void OP_GetRootMethodProperty_NoFastPath(unaligned T* playout);
        template <class T> void OP_ProfiledGetRootMethodProperty(unaligned T* playout);
        template <class T> void OP_GetMethodProperty(unaligned T* playout);
        template <class T> void OP_GetMethodProperty_NoFastPath(unaligned T* playout);
        template <class T> void OP_ProfiledGetMethodProperty(unaligned T* playout);
        template <typename T> void OP_GetPropertyScoped(const unaligned OpLayoutT_ElementCP<T>* playout);
        template <typename T> void OP_GetPropertyForTypeOfScoped(const unaligned OpLayoutT_ElementCP<T>* playout);
        template <typename T> void OP_GetPropertyScoped_NoFastPath(const unaligned OpLayoutT_ElementCP<T>* playout);
        template <class T> void OP_GetMethodPropertyScoped(unaligned T* playout);
        template <class T> void OP_GetMethodPropertyScoped_NoFastPath(unaligned T* playout);

        template <class T> void UpdateFldInfoFlagsForGetSetInlineCandidate(unaligned T* playout, FldInfoFlags& fldInfoFlags, CacheType cacheType,
                                                DynamicProfileInfo * dynamicProfileInfo, uint inlineCacheIndex, RecyclableObject * obj);

        template <class T> void UpdateFldInfoFlagsForCallApplyInlineCandidate(unaligned T* playout, FldInfoFlags& fldInfoFlags, CacheType cacheType,
                                                DynamicProfileInfo * dynamicProfileInfo, uint inlineCacheIndex, RecyclableObject * obj);
        
        template <class T> void OP_SetProperty(unaligned T* playout);
        template <class T> void OP_ProfiledSetProperty(unaligned T* playout);
        template <class T> void OP_SetRootProperty(unaligned T* playout);
        template <class T> void OP_ProfiledSetRootProperty(unaligned T* playout);
        template <class T> void OP_SetPropertyStrict(unaligned T* playout);
        template <class T> void OP_ProfiledSetPropertyStrict(unaligned T* playout);
        template <class T> void OP_SetRootPropertyStrict(unaligned T* playout);
        template <class T> void OP_ProfiledSetRootPropertyStrict(unaligned T* playout);
        template <class T> void OP_SetPropertyScoped(unaligned T* playout, PropertyOperationFlags flags = PropertyOperation_None);
        template <class T> void OP_SetPropertyScoped_NoFastPath(unaligned T* playout, PropertyOperationFlags flags);
        template <class T> void OP_SetPropertyScopedStrict(unaligned T* playout);

        template <class T> void DoSetProperty(unaligned T* playout, Var instance, PropertyOperationFlags flags);
        template <class T> void DoSetProperty_NoFastPath(unaligned T* playout, Var instance, PropertyOperationFlags flags);
        template <class T, bool Root> void ProfiledSetProperty(unaligned T* playout, Var instance, PropertyOperationFlags flags);

        template <class T> void OP_InitProperty(unaligned T* playout);
        template <class T> void OP_InitRootProperty(unaligned T* playout);
        template <class T> void OP_InitUndeclLetProperty(unaligned T* playout);
        void OP_InitUndeclRootLetProperty(uint propertyIdIndex);
        template <class T> void OP_InitUndeclConstProperty(unaligned T* playout);
        void OP_InitUndeclRootConstProperty(uint propertyIdIndex);
        template <class T> void OP_ProfiledInitProperty(unaligned T* playout);
        template <class T> void OP_ProfiledInitRootProperty(unaligned T* playout);
        template <class T> void OP_ProfiledInitUndeclProperty(unaligned T* playout);

        template <class T> void DoInitProperty(unaligned T* playout, Var instance);
        template <class T> void DoInitProperty_NoFastPath(unaligned T* playout, Var instance);
        template <class T> void ProfiledInitProperty(unaligned T* playout, Var instance);

        template <class T> bool TrySetPropertyLocalFastPath(unaligned T* playout, PropertyId pid, Var instance, InlineCache*& inlineCache, PropertyOperationFlags flags = PropertyOperation_None);

        template <bool doProfile> Var ProfiledDivide(Var aLeft, Var aRight, ScriptContext* scriptContext, ProfileId profileId);
        template <bool doProfile> Var ProfileModulus(Var aLeft, Var aRight, ScriptContext* scriptContext, ProfileId profileId);
        template <bool doProfile> Var ProfiledSwitch(Var exp, ProfileId profileId);

        // Non-patching Fastpath operations
        template <typename T> void OP_GetElementI(const unaligned T* playout);
        template <typename T> void OP_ProfiledGetElementI(const unaligned OpLayoutDynamicProfile<T>* playout);

        template <typename T> void OP_SetElementI(const unaligned T* playout, PropertyOperationFlags flags = PropertyOperation_None);
        template <typename T> void OP_ProfiledSetElementI(const unaligned OpLayoutDynamicProfile<T>* playout, PropertyOperationFlags flags = PropertyOperation_None);
        template <typename T> void OP_SetElementIStrict(const unaligned T* playout);
        template <typename T> void OP_ProfiledSetElementIStrict(const unaligned OpLayoutDynamicProfile<T>* playout);

        template<class T> void OP_LdLen(const unaligned T *const playout);
        template<class T> void OP_ProfiledLdLen(const unaligned OpLayoutDynamicProfile<T> *const playout);

        Var OP_ProfiledLdThis(Var thisVar, int moduleID, ScriptContext* scriptContext);
        Var OP_ProfiledStrictLdThis(Var thisVar, ScriptContext* scriptContext);

        template <class T> void OP_SetArrayItemI_CI4(const unaligned T* playout);
        template <class T> void OP_SetArrayItemC_CI4(const unaligned T* playout);
        template <class T> void OP_SetArraySegmentItem_CI4(const unaligned T* playout);
        template <class T> void SetArrayLiteralItem(JavascriptArray *arr, uint32 index, T value);
        void OP_SetArraySegmentVars(const unaligned OpLayoutAuxiliary * playout);

        template <class T> void OP_NewScArray(const unaligned T * playout);
        template <bool Profiled, class T> void ProfiledNewScArray(const unaligned OpLayoutDynamicProfile<T> * playout);
        template <class T> void OP_ProfiledNewScArray(const unaligned OpLayoutDynamicProfile<T> * playout) { ProfiledNewScArray<true, T>(playout); }
        template <class T> void OP_ProfiledNewScArray_NoProfile(const unaligned OpLayoutDynamicProfile<T> * playout)  { ProfiledNewScArray<false, T>(playout); }
        void OP_NewScIntArray(const unaligned OpLayoutAuxiliary * playout);
        void OP_NewScFltArray(const unaligned OpLayoutAuxiliary * playout);
        void OP_ProfiledNewScIntArray(const unaligned OpLayoutDynamicProfile<OpLayoutAuxiliary> * playout);
        void OP_ProfiledNewScFltArray(const unaligned OpLayoutDynamicProfile<OpLayoutAuxiliary> * playout);

        template <class T> void OP_LdArrayHeadSegment(const unaligned T* playout);

        template <class T> inline void OP_LdFunctionExpression(const unaligned T* playout);
        template <class T> inline void OP_StFunctionExpression(const unaligned T* playout);
        inline Var OP_Ld_A(Var aValue);
        void OP_ChkUndecl(Var aValue);
        void OP_EnsureNoRootProperty(uint propertyIdIndex);
        void OP_EnsureNoRootRedeclProperty(uint propertyIdIndex);
        void OP_ScopedEnsureNoRedeclProperty(Var aValue, uint propertyIdIndex, Var aValue2);
        Var OP_InitUndecl();
        void OP_InitUndeclSlot(Var aValue, int32 slot);
        template <class T> inline void OP_InitLetFld(const unaligned T * playout);
        template <class T> inline void OP_InitRootLetFld(const unaligned T * playout);
        template <class T> inline void OP_InitConstFld(const unaligned T * playout);
        template <class T> inline void OP_InitRootConstFld(const unaligned T * playout);
        template <class T> inline void DoInitLetFld(const unaligned T * playout, Var instance, PropertyOperationFlags flags = PropertyOperation_None);
        template <class T> inline void DoInitConstFld(const unaligned T * playout, Var instance, PropertyOperationFlags flags = PropertyOperation_None);
        inline Var OP_LdEnv();
        template<typename T> uint32 LogSizeOf();
        template <typename T2> inline void OP_LdArr(  uint32 index, RegSlot value  );
        template <class T> inline void OP_LdArrFunc(const unaligned T* playout);
        template <class T> inline void OP_ReturnDb(const unaligned T* playout);
        template<typename T> T GetArrayViewOverflowVal();
        template <typename T2> inline void OP_StArr( uint32 index, RegSlot value );
        template <class T, typename T2> inline void OP_StSlotPrimitive(const unaligned T* playout);
        template <class T, typename T2> inline void OP_LdSlotPrimitive( const unaligned T* playout );
        template <class T> inline void OP_LdArrGeneric   ( const unaligned T* playout );
        template <class T> inline void OP_LdArrConstIndex( const unaligned T* playout );
        template <class T> inline void OP_StArrGeneric   ( const unaligned T* playout );
        template <class T> inline void OP_StArrConstIndex( const unaligned T* playout );
        template <class T> inline Var OP_LdSlot(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_ProfiledLdSlot(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_LdSlotChkUndecl(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_ProfiledLdSlotChkUndecl(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_LdObjSlot(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_ProfiledLdObjSlot(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_LdObjSlotChkUndecl(Var instance, const unaligned T* playout);
        template <class T> inline Var OP_ProfiledLdObjSlotChkUndecl(Var instance, const unaligned T* playout);
        inline void OP_StSlot(Var instance, int32 slotIndex, Var value);
        inline void OP_StSlotChkUndecl(Var instance, int32 slotIndex, Var value);
        inline void OP_StObjSlot(Var instance, int32 slotIndex, Var value);
        inline void OP_StObjSlotChkUndecl(Var instance, int32 slotIndex, Var value);
        inline void* OP_LdArgCnt();
        inline Var OP_LdHeapArguments(Var frameObj, Var argsArray, ScriptContext* scriptContext);
        inline Var OP_LdLetHeapArguments(Var frameObj, Var argsArray, ScriptContext* scriptContext);
        inline Var OP_LdHeapArgsCached(Var frameObj, ScriptContext* scriptContext);
        inline Var OP_LdLetHeapArgsCached(Var frameObj, ScriptContext* scriptContext);
        inline Var OP_LdStackArgPtr();
        inline Var OP_LdArgumentsFromFrame();
        Var OP_NewScObjectSimple();
        void OP_NewScObjectLiteral(const unaligned OpLayoutAuxiliary * playout);
        void OP_NewScObjectLiteral_LS(const unaligned OpLayoutAuxiliary * playout, RegSlot& target);
        void OP_LdPropIds(const unaligned OpLayoutAuxiliary * playout);
        template <bool Profile, bool JITLoopBody> void LoopBodyStart(uint32 loopNumber, LayoutSize layoutSize, bool isFirstIteration);
        LoopHeader const * DoLoopBodyStart(uint32 loopNumber, LayoutSize layoutSize, const bool doProfileLoopCheck, bool isFirstIteration);
        template <bool Profile, bool JITLoopBody> void ProfiledLoopBodyStart(uint32 loopNumber, LayoutSize layoutSize, bool isFirstIteration);
        void OP_RecordImplicitCall(uint loopNumber);
        template <class T, bool Profiled, bool ICIndex> void OP_NewScObject_Impl(const unaligned T* playout, InlineCacheIndex inlineCacheIndex = Js::Constants::NoInlineCacheIndex, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        template <class T, bool Profiled> void OP_NewScObjArray_Impl(const unaligned T* playout, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        template <class T> void OP_NewScObject(const unaligned T* playout) { OP_NewScObject_Impl<T, false, false>(playout); }
        template <class T> void OP_NewScObjectSpread(const unaligned T* playout) { OP_NewScObject_Impl<T, false, false>(playout, Js::Constants::NoInlineCacheIndex, m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody())); }
        template <class T> void OP_NewScObjArray(const unaligned T* playout) { OP_NewScObjArray_Impl<T, false>(playout); }
        template <class T> void OP_NewScObjArraySpread(const unaligned T* playout) { OP_NewScObjArray_Impl<T, false>(playout, m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody())); }
        template <class T> void OP_ProfiledNewScObject(const unaligned OpLayoutDynamicProfile<T>* playout) { OP_NewScObject_Impl<T, true, false>(playout); }
        template <class T> void OP_ProfiledNewScObjectSpread(const unaligned OpLayoutDynamicProfile<T>* playout) { OP_NewScObject_Impl<T, true, false>(playout, Js::Constants::NoInlineCacheIndex, m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody())); }
        template <class T> void OP_ProfiledNewScObjectWithICIndex(const unaligned OpLayoutDynamicProfile<T>* playout) { OP_NewScObject_Impl<T, true, true>(playout, playout->inlineCacheIndex); }
        template <class T> void OP_ProfiledNewScObjArray(const unaligned OpLayoutDynamicProfile2<T>* playout) { OP_NewScObjArray_Impl<T, true>(playout); }
        template <class T> void OP_ProfiledNewScObjArray_NoProfile(const unaligned OpLayoutDynamicProfile2<T>* playout) { OP_NewScObjArray_Impl<T, false>(playout); }
        template <class T> void OP_ProfiledNewScObjArraySpread(const unaligned OpLayoutDynamicProfile2<T>* playout) { OP_NewScObjArray_Impl<T, true>(playout, m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody())); }
        template <class T> void OP_ProfiledNewScObjArraySpread_NoProfile(const unaligned OpLayoutDynamicProfile2<T>* playout) { OP_NewScObjArray_Impl<T, true>(playout, m_reader.ReadAuxArray<uint32>(playout->SpreadAuxOffset, this->GetFunctionBody())); }
        Var NewScObject_Helper(Var target, ArgSlot ArgCount, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        Var ProfiledNewScObject_Helper(Var target, ArgSlot ArgCount, ProfileId profileId, InlineCacheIndex inlineCacheIndex, const Js::AuxArray<uint32> *spreadIndices = nullptr);
        template <class T, bool Profiled, bool ICIndex> Var OP_NewScObjectNoArg_Impl(const unaligned T *playout, InlineCacheIndex inlineCacheIndex = Js::Constants::NoInlineCacheIndex);
        template<bool LanguageService> void OP_NewScObject_A_Impl(const unaligned OpLayoutAuxiliary * playout, RegSlot *target = null);
        void OP_NewScObject_A(const unaligned OpLayoutAuxiliary * playout) { return OP_NewScObject_A_Impl<false>(playout); }
        void OP_NewScObject_A_LS(const unaligned OpLayoutAuxiliary * playout, RegSlot& target) { return OP_NewScObject_A_Impl<true>(playout, &target); }
        void OP_InitCachedScope(const unaligned OpLayoutReg2Aux * playout);
        void OP_InitLetCachedScope(const unaligned OpLayoutReg2Aux * playout);
        void OP_InitCachedFuncs(const unaligned OpLayoutReg2Aux * playout);
        Var OP_GetCachedFunc(Var instance, int32 index);
        void OP_CommitScope(const unaligned OpLayoutAuxiliary * playout);
        void OP_CommitScopeHelper(const unaligned OpLayoutAuxiliary *playout, const PropertyIdArray *propIds);
        void OP_TryCatch(const unaligned OpLayoutBr* playout);
        void ProcessCatch();
        int ProcessFinally();
        void ProcessTryCatchBailout(EHBailoutData * innermostEHBailoutData, uint32 tryNestingDepth);
        void OP_TryFinally(const unaligned OpLayoutBr* playout);
        void OP_TryFinallyWithYield(const byte* ip, Js::JumpOffset jumpOffset, Js::RegSlot regException, Js::RegSlot regOffset);
        void OP_ResumeCatch();
        void OP_ResumeFinally(const byte* ip, Js::JumpOffset jumpOffset, RegSlot exceptionRegSlot, RegSlot offsetRegSlot);
        inline Var OP_ResumeYield(Var yieldDataVar, RegSlot yieldStarIterator = Js::Constants::NoRegister);
        template <typename T> void OP_IsInst(const unaligned T * playout);
        template <class T> void OP_InitClass(const unaligned OpLayoutT_Class<T> * playout);
        inline Var OP_LdSuper(ScriptContext * scriptContext);
        inline Var OP_ScopedLdSuper(ScriptContext * scriptContext);
        template <typename T> void OP_LdElementUndefined(const unaligned OpLayoutT_ElementU<T>* playout);
        template <typename T> void OP_LdElementUndefinedScoped(const unaligned OpLayoutT_ElementU<T>* playout);
        void OP_SpreadArrayLiteral(const unaligned OpLayoutReg2Aux * playout);
        template <LayoutSize layoutSize,bool profiled> const byte * OP_ProfiledLoopStart(const byte *ip);
        template <LayoutSize layoutSize,bool profiled> const byte * OP_ProfiledLoopEnd(const byte *ip);
        template <LayoutSize layoutSize,bool profiled> const byte * OP_ProfiledLoopBodyStart(const byte *ip);
        template <typename T> void OP_ApplyArgs(const unaligned OpLayoutT_Reg5<T> * playout);
        template <class T> void OP_EmitTmpRegCount(const unaligned OpLayoutT_Reg1<T> * ip);

        template<bool strict> FrameDisplay * OP_NewStackFrameDisplay(void *argHead, void *argEnv);
        template<bool strict> FrameDisplay * OP_NewStackFrameDisplayNoParent(void *argHead);
        FrameDisplay * OP_LdLocalFrameDisplay();
        Var * OP_NewStackScopeSlots();
        Var * OP_LdLocalScopeSlots();
        template <class T> void OP_NewStackScFunc(const unaligned T * playout);
        template <class T> void OP_DeleteFld(const unaligned T * playout);
        template <class T> void OP_DeleteRootFld(const unaligned T * playout);
        template <class T> void OP_DeleteFldStrict(const unaligned T * playout);
        template <class T> void OP_DeleteRootFldStrict(const unaligned T * playout);
        template <typename T> void OP_ScopedDeleteFld(const unaligned OpLayoutT_ElementC<T> * playout);
        template <typename T> void OP_ScopedDeleteFldStrict(const unaligned OpLayoutT_ElementC<T> * playout);
        template <class T> void OP_ScopedLdInst(const unaligned T * playout);
        template <typename T> void OP_ScopedInitFunc(const unaligned OpLayoutT_ElementC<T> * playout);
        template <class T> void OP_ClearAttributes(const unaligned T * playout);
        template <class T> void OP_BindEvt(const unaligned T * playout);
        template <class T> void OP_InitGetFld(const unaligned T * playout);
        template <class T> void OP_InitSetFld(const unaligned T * playout);
        template <class T> void OP_InitSetElemI(const unaligned T * playout);
        template <class T> void OP_InitGetElemI(const unaligned T * playout);
        template <class T> void OP_InitComputedProperty(const unaligned T * playout);
        template <class T> void OP_InitProto(const unaligned T * playout);

        inline void DisableLanguageServiceExceptionSkipping();
        inline bool LanguageServiceExceptionSkippingEnabled();
        uint CallLoopBody(JavascriptMethod address);
        uint CallAsmJsLoopBody(JavascriptMethod address);
        void DoInterruptProbe();
        void CheckIfLoopIsHot(uint profiledLoopCounter);
        bool CheckAndResetImplicitCall(DisableImplicitFlags prevDisableImplicitFlags,ImplicitCallFlags savedImplicitCallFlags);
        class PushPopFrameHelper
        {
        public:
            PushPopFrameHelper(InterpreterStackFrame *interpreterFrame, void *returnAddress, void *addressOfReturnAddress)
                : m_threadContext(interpreterFrame->GetScriptContext()->GetThreadContext()), m_interpreterFrame(interpreterFrame), m_isHiddenFrame(false)
            {
                interpreterFrame->returnAddress = returnAddress; // Ensure these are set before pushing to interpreter frame list
                interpreterFrame->addressOfReturnAddress = addressOfReturnAddress;
                if (interpreterFrame->GetFunctionBody()->GetIsAsmJsFunction())
                {
                    m_isHiddenFrame = true;
                }
                else
                {
                    m_threadContext->PushInterpreterFrame(interpreterFrame);
                }
            }
            ~ PushPopFrameHelper()
            {
                if (!m_isHiddenFrame)
                {
                    Js::InterpreterStackFrame *interpreterFrame = m_threadContext->PopInterpreterFrame();
                    AssertMsg(interpreterFrame == m_interpreterFrame,
                        "Interpreter frame chain corrupted?");
                }
            }
        private:
            ThreadContext *m_threadContext;
            InterpreterStackFrame *m_interpreterFrame;
            bool m_isHiddenFrame;
        };

        inline InlineCache* GetInlineCache(uint cacheIndex)
        {
            Assert(this->inlineCaches != null);
            Assert(cacheIndex < this->inlineCacheCount);

            return reinterpret_cast<InlineCache *>(this->inlineCaches[cacheIndex]);
        }

        inline IsInstInlineCache* GetIsInstInlineCache(uint cacheIndex)
        {
            return m_functionBody->GetIsInstInlineCache(cacheIndex);
        }

        inline PropertyId GetPropertyIdFromCacheId(uint cacheIndex)
        {
            return m_functionBody->GetPropertyIdFromCacheId(cacheIndex);
        }

        void InitializeStackFunctions(StackScriptFunction * scriptFunctions);
        StackScriptFunction * GetStackNestedFunction(uint index);
        void SetExecutingStackFunction(ScriptFunction * scriptFunction);
        friend class StackScriptFunction;

        FrameDisplay *GetLocalFrameDisplay() const;// { return this->localFrameDisplay; }
        void SetLocalFrameDisplay(FrameDisplay *frameDisplay);// { this->localFrameDisplay = frameDisplay; }
        Var *GetLocalScopeSlots() const;// { return this->localScopeSlots; }
        void SetLocalScopeSlots(Var *slotArray);// { this->localScopeSlots = slotArray; }
        void TrySetRetOffset();
    };

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    // Used to track how many interpreter stack frames we have on stack.
    class InterpreterThunkStackCountTracker
    {
    public:
        InterpreterThunkStackCountTracker()  { ++s_count; }
        ~InterpreterThunkStackCountTracker() { --s_count; }
        static int GetCount() { return s_count; }
    private:
        __declspec(thread) static int s_count;
    };
#endif

} // namespace Js
