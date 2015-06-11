//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class IRBuilderAsmJs
{
public:
    IRBuilderAsmJs(Func * func)
        : m_func(func), m_IsTJLoopBody(false)
    {
        func->m_workItem->InitializeReader(m_jnReader, m_statementReader);
        m_asmFuncInfo = m_func->GetJnFunction()->GetAsmJsFunctionInfo();
        m_entryPoint = (Js::FunctionEntryPointInfo*)(((InMemoryCodeGenWorkItem *)func->m_workItem)->GetEntryPoint());
        Assert(m_entryPoint);
        m_ModuleAddress = m_entryPoint->GetModuleAddress();
        Assert(m_ModuleAddress);
        if (m_entryPoint->IsLoopBody())
        {
            Js::LoopEntryPointInfo* loopEntryPointInfo = (Js::LoopEntryPointInfo*)m_entryPoint;
            if (loopEntryPointInfo->GetIsTJMode())
            {
                m_IsTJLoopBody = true;
                func->isTJLoopBody = true;
            }
        }
        m_ArrayBufferRef = (Js::ArrayBuffer**)((Js::Var *)m_ModuleAddress + Js::AsmJsModuleMemory::MemoryTableBeginOffset);
    }

    void Build();

private:
    void                    AddInstr(IR::Instr * instr, uint32 offset);
    bool                    IsLoopBody()const;
    uint                    GetLoopBodyExitInstrOffset() const;
    IR::SymOpnd *           BuildLoopBodySlotOpnd(SymID symId);
    IR::SymOpnd *           BuildAsmJsLoopBodySlotOpnd(SymID symId);
    void                    EnsureLoopBodyLoadSlot(SymID symId);
    void                    EnsureLoopBodyAsmJsLoadSlot(SymID symId);
    bool                    IsLoopBodyOuterOffset(uint offset) const;
    bool                    IsLoopBodyReturnIPInstr(IR::Instr * instr) const;
    IR::Opnd *              InsertLoopBodyReturnIPInstr(uint targetOffset, uint offset);
    IR::Instr *             CreateLoopBodyReturnIPInstr(uint targetOffset, uint offset);
    IR::RegOpnd *           BuildDstOpnd(Js::RegSlot dstRegSlot, IRType type);
    IR::RegOpnd *           BuildSrcOpnd(Js::RegSlot srcRegSlot, IRType type);
    SymID                   BuildSrcStackSymID(Js::RegSlot regSlot);
    IR::SymOpnd *           BuildFieldOpnd(Js::RegSlot reg, Js::PropertyId propertyId, Js::PropertyIdIndexType propertyIdIndex, PropertyKind propertyKind, IRType type);
    PropertySym *           BuildFieldSym(Js::RegSlot reg, Js::PropertyId propertyId, Js::PropertyIdIndexType propertyIdIndex, PropertyKind propertyKind);
    uint                    AddStatementBoundary(uint statementIndex, uint offset);
    BranchReloc *           AddBranchInstr(IR::BranchInstr *instr, uint32 offset, uint32 targetOffset);
    BranchReloc *           CreateRelocRecord(IR::BranchInstr * branchInstr, uint32 offset, uint32 targetOffset);
    void                    BuildHeapBufferReload(uint32 offset);
    void                    BuildConstantLoads();
    void                    BuildImplicitArgIns();
    void                    InsertLabels();
    IR::LabelInstr *        CreateLabel(IR::BranchInstr * branchInstr, uint& offset);
#if DBG
    BVFixed *               m_usedAsTemp;
#endif
    Js::RegSlot             GetRegSlotFromIntReg(Js::RegSlot srcIntReg);
    Js::RegSlot             GetRegSlotFromFloatReg(Js::RegSlot srcFloatReg);
    Js::RegSlot             GetRegSlotFromDoubleReg(Js::RegSlot srcDoubleReg);
    Js::RegSlot             GetRegSlotFromVarReg(Js::RegSlot srcVarReg);
#ifdef SIMD_JS_ENABLED
    Js::RegSlot             GetRegSlotFromSimd128Reg(Js::RegSlot srcSimd128Reg);
    IR::Instr *             AddExtendedArg(IR::RegOpnd *src1, IR::RegOpnd *src2, uint32 offset);
    BOOL                    RegIsSimd128Var(Js::RegSlot reg);
#endif
    SymID                   GetMappedTemp(Js::RegSlot reg);
    void                    SetMappedTemp(Js::RegSlot reg, SymID tempId);
    BOOL                    GetTempUsed(Js::RegSlot reg);
    void                    SetTempUsed(Js::RegSlot reg, BOOL used);
    BOOL                    RegIsTemp(Js::RegSlot reg);
    BOOL                    RegIsConstant(Js::RegSlot reg);
    BOOL                    RegIsVar(Js::RegSlot reg);
    BOOL                    RegIsIntVar(Js::RegSlot reg);
    BOOL                    RegIsFloatVar(Js::RegSlot reg);
    BOOL                    RegIsDoubleVar(Js::RegSlot reg);

#define LAYOUT_TYPE(layout) \
    void                    Build##layout(Js::OpCodeAsmJs newOpcode, uint32 offset);
#define LAYOUT_TYPE_WMS(layout) \
    template <typename SizePolicy> void Build##layout(Js::OpCodeAsmJs newOpcode, uint32 offset);
#include "ByteCode\LayoutTypesAsmJs.h"

    void                    BuildElementSlot(Js::OpCodeAsmJs newOpcode, uint32 offset, int32 slotIndex, Js::RegSlot value, Js::RegSlot instance);
    void                    BuildAsmUnsigned1(Js::OpCodeAsmJs newOpcode, uint value);
    void                    BuildAsmTypedArr(Js::OpCodeAsmJs newOpcode, uint32 offset, uint32 slotIndex, Js::RegSlot value, int8 viewType);
    void                    BuildAsmCall(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::ArgSlot argCount, Js::RegSlot ret, Js::RegSlot function, int8 returnType);
    void                    BuildAsmReg1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstReg);
    void                    BuildInt1Double1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstIntReg, Js::RegSlot srcDoubleReg);
    void                    BuildInt1Float1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstIntReg, Js::RegSlot srcFloatReg);
    void                    BuildDouble1Int1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstDoubleReg, Js::RegSlot srcIntReg);
    void                    BuildDouble1Float1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstDoubleReg, Js::RegSlot srcFloatReg);
    void                    BuildFloat1Reg1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstFloatReg, Js::RegSlot srcVarReg);
    void                    BuildDouble1Reg1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstDoubleReg, Js::RegSlot srcVarReg);
    void                    BuildInt1Reg1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstIntReg, Js::RegSlot srcVarReg);
    void                    BuildReg1Double1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstReg, Js::RegSlot srcDoubleReg);
    void                    BuildReg1Float1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstReg, Js::RegSlot srcFloatReg);
    void                    BuildReg1Int1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstReg, Js::RegSlot srcIntReg);
    void                    BuildInt1Const1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstInt, int constInt);
    void                    BuildDouble1Const1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstDoubleReg, double constDouble);
    void                    BuildFloat1Const1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstFloatReg, float constFloat);
    void                    BuildDouble1Addr1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstDoubleReg, const double * addr);
    void                    BuildFloat1Addr1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dstFloatReg, const float * addr);
    void                    BuildInt1Double2(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src1, Js::RegSlot src2);
    void                    BuildInt1Float2(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src1, Js::RegSlot src2);
    void                    BuildInt2(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src);
    void                    BuildInt3(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src1, Js::RegSlot src2);
    void                    BuildDouble2(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src);
    void                    BuildFloat2(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src);
    void                    BuildFloat3(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src1, Js::RegSlot src2);
    void                    BuildFloat1Double1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src);
    void                    BuildFloat1Int1(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src);
    void                    BuildDouble3(Js::OpCodeAsmJs newOpcode, uint32 offset, Js::RegSlot dst, Js::RegSlot src1, Js::RegSlot src2);
    void                    BuildBrInt1(Js::OpCodeAsmJs newOpcode, uint32 offset, int32 relativeOffset, Js::RegSlot src);
    void                    BuildBrInt2(Js::OpCodeAsmJs newOpcode, uint32 offset, int32 relativeOffset, Js::RegSlot src1, Js::RegSlot src2);  
    void                    GenerateLoopBodySlotAccesses(uint offset);
    void                    GenerateLoopBodyStSlots(SymID loopParamSymId, uint offset);
    IR::Instr*              GenerateStSlotForReturn(IR::RegOpnd* srcOpnd, IRType type);
    JitArenaAllocator *     m_tempAlloc;
    JitArenaAllocator *     m_funcAlloc;
    Func *                  m_func;
    IR::Instr *             m_lastInstr;
    IR::Instr **            m_offsetToInstruction;
    Js::ByteCodeReader      m_jnReader;
    Js::StatementReader     m_statementReader;
    SList<IR::Instr *> *    m_argStack;
    SList<IR::Instr *> *    m_tempList;
    SList<int32> *          m_argOffsetStack;
    SList<BranchReloc *> *  m_branchRelocList;
    Js::RegSlot             m_firstIntConst;
    Js::RegSlot             m_firstFloatConst;
    Js::RegSlot             m_firstDoubleConst;
    Js::RegSlot             m_firstVarConst;
    Js::RegSlot             m_firstIntVar;
    Js::RegSlot             m_firstFloatVar;
    Js::RegSlot             m_firstDoubleVar;
    Js::RegSlot             m_firstIntTemp;
    Js::RegSlot             m_firstFloatTemp;
    Js::RegSlot             m_firstDoubleTemp;
    Js::RegSlot             m_firstIRTemp;
#ifdef SIMD_JS_ENABLED
    Js::RegSlot             m_firstSimdConst;
    Js::RegSlot             m_firstSimdVar;
    Js::RegSlot             m_firstSimdTemp;
    Js::OpCode *            m_simdOpcodesMap;
#endif
    SymID *                 m_tempMap;
    BVFixed *               m_fbvTempUsed;
    uint32                  m_functionStartOffset;
    Js::AsmJsFunctionInfo * m_asmFuncInfo;
    Js::ArrayBuffer**       m_ArrayBufferRef;
    uintptr_t               m_ModuleAddress;
    Js::FunctionEntryPointInfo* m_entryPoint;
    StackSym *              m_loopBodyRetIPSym;
    BVFixed *               m_ldSlots;
    BVFixed *               m_stSlots;
    BOOL                    m_IsTJLoopBody;
#if DBG
    uint32                  m_offsetToInstructionCount;
#endif
};
