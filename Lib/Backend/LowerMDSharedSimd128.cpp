//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "BackEnd.h"

#define GET_SIMDOPCODE(irOpcode) m_simd128OpCodesMap[(uint32)(irOpcode - Js::OpCode::Simd128_Start)]
                                 

#define SET_SIMDOPCODE(irOpcode, mdOpcode) \
    Assert((uint32)m_simd128OpCodesMap[(uint32)(Js::OpCode::irOpcode - Js::OpCode::Simd128_Start)] == 0); \
    Assert(Js::OpCode::mdOpcode > Js::OpCode::MDStart); \
    m_simd128OpCodesMap[(uint32)(Js::OpCode::irOpcode - Js::OpCode::Simd128_Start)] = Js::OpCode::mdOpcode;

IR::Instr* LowererMD::Simd128Instruction(IR::Instr *instr)
{
    // Curently only handles type-specialized/asm.js opcodes
    if (Simd128TryLowerMappedInstruction(instr))
    {
        return instr->m_prev;
    }
    return Simd128LowerUnMappedInstruction(instr);
}

bool LowererMD::Simd128TryLowerMappedInstruction(IR::Instr *instr)
{
    bool legalize = true;
    Js::OpCode opcode = GET_SIMDOPCODE(instr->m_opcode);

    if ((uint32)opcode == 0)
        return false;

    Assert(instr->GetDst() && instr->GetDst()->IsRegOpnd() && instr->GetDst()->GetType() == TySimd128 || instr->GetDst()->GetType() == TyInt32);
    Assert(instr->GetSrc1() && instr->GetSrc1()->IsRegOpnd() && instr->GetSrc1()->GetType() == TySimd128);
    Assert(!instr->GetSrc2() || (instr->GetSrc2() &&
        ((instr->GetSrc2()->IsRegOpnd() && instr->GetSrc2()->GetType() == TySimd128) || (instr->GetSrc2()->IsIntConstOpnd() && instr->GetSrc2()->GetType() == TyInt8))));

    switch (instr->m_opcode)
    {
    case Js::OpCode::Simd128_Abs_F4:
        Assert(opcode == Js::OpCode::ANDPS);
        instr->SetSrc2(IR::MemRefOpnd::New((void*)&X86_ABS_MASK_F4, TySimd128, m_func));
        break;
    case Js::OpCode::Simd128_Abs_D2:
        Assert(opcode == Js::OpCode::ANDPD);
        instr->SetSrc2(IR::MemRefOpnd::New((void*)&X86_ABS_MASK_D2, TySimd128, m_func));
        break;
    case Js::OpCode::Simd128_Neg_F4:
        Assert(opcode == Js::OpCode::XORPS);
        instr->SetSrc2(IR::MemRefOpnd::New((void*)&X86_NEG_MASK_F4, TySimd128, m_func));
        break;
    case Js::OpCode::Simd128_Neg_D2:
        Assert(opcode == Js::OpCode::XORPS);
        instr->SetSrc2(IR::MemRefOpnd::New((void*)&X86_NEG_MASK_D2, TySimd128, m_func));
        break;
    case Js::OpCode::Simd128_Not_F4:
    case Js::OpCode::Simd128_Not_I4:
        Assert(opcode == Js::OpCode::XORPS);
        instr->SetSrc2(IR::MemRefOpnd::New((void*)&X86_ALL_NEG_ONES, TySimd128, m_func));
        break;
    case Js::OpCode::Simd128_Gt_F4:
    case Js::OpCode::Simd128_Gt_D2:
    case Js::OpCode::Simd128_GtEq_F4:
    case Js::OpCode::Simd128_GtEq_D2:
    case Js::OpCode::Simd128_Lt_I4:
    {
        Assert(opcode == Js::OpCode::CMPLTPS || opcode == Js::OpCode::CMPLTPD || opcode == Js::OpCode::CMPLEPS || opcode == Js::OpCode::CMPLEPD || opcode == Js::OpCode::PCMPGTD);
        // swap operands
        auto *src1 = instr->UnlinkSrc1();
        auto *src2 = instr->UnlinkSrc2();
        instr->SetSrc1(src2);
        instr->SetSrc2(src1);
        break;
    }
    case Js::OpCode::Simd128_LdSignMask_F4:
    case Js::OpCode::Simd128_LdSignMask_I4:
    case Js::OpCode::Simd128_LdSignMask_D2:
        legalize = false;
        break;
        
    }
    instr->m_opcode = opcode;
    if (legalize)
    {
        //MakeDstEquSrc1(instr);
        Legalize(instr);
    }
    
    return true;
}

IR::Instr* LowererMD::Simd128LowerUnMappedInstruction(IR::Instr *instr)
{
    AssertMsg(GET_SIMDOPCODE(instr->m_opcode) == 0, "Simd128 opcode is mapped to single instruction");
    switch (instr->m_opcode)
    {
    case Js::OpCode::Simd128_LdC:
        return Simd128LoadConst(instr);
        
    case Js::OpCode::Simd128_FloatsToF4:
    case Js::OpCode::Simd128_IntsToI4:
    case Js::OpCode::Simd128_DoublesToD2:
        return Simd128LowerConstructor(instr);

    case Js::OpCode::Simd128_LdLane_F4:
    case Js::OpCode::Simd128_LdLane_I4:
    case Js::OpCode::Simd128_LdLane_D2:
        return Simd128LowerLdLane(instr);

    case Js::OpCode::Simd128_Splat_F4:
    case Js::OpCode::Simd128_Splat_I4:
    case Js::OpCode::Simd128_Splat_D2:
        return Simd128LowerSplat(instr);

    case Js::OpCode::Simd128_Rcp_F4:
    case Js::OpCode::Simd128_Rcp_D2:
        return Simd128LowerRcp(instr);

    case Js::OpCode::Simd128_Sqrt_F4:
    case Js::OpCode::Simd128_Sqrt_D2:
        return Simd128LowerSqrt(instr);

    case Js::OpCode::Simd128_RcpSqrt_F4:
    case Js::OpCode::Simd128_RcpSqrt_D2:
        return Simd128LowerRcpSqrt(instr);

    case Js::OpCode::Simd128_Select_F4:
    case Js::OpCode::Simd128_Select_I4:
    case Js::OpCode::Simd128_Select_D2:
        return Simd128LowerSelect(instr);

    case Js::OpCode::Simd128_WithX_F4:
    case Js::OpCode::Simd128_WithY_F4:
    case Js::OpCode::Simd128_WithZ_F4:
    case Js::OpCode::Simd128_WithW_F4:
    case Js::OpCode::Simd128_WithX_I4:
    case Js::OpCode::Simd128_WithY_I4:
    case Js::OpCode::Simd128_WithZ_I4:
    case Js::OpCode::Simd128_WithW_I4:
    case Js::OpCode::Simd128_WithX_D2:
    case Js::OpCode::Simd128_WithY_D2:
        return Simd128LowerStLane(instr);

    case Js::OpCode::Simd128_Neg_I4:
        return Simd128LowerNegI4(instr);
    
    case Js::OpCode::Simd128_Mul_I4:
        return Simd128LowerMulI4(instr);
    default:
        AssertMsg(UNREACHED, "Unsupported Simd128 instruction");
    }
    return NULL;
}

IR::Instr* LowererMD::Simd128LoadConst(IR::Instr* instr)
{
    Assert(instr->m_opcode == Js::OpCode::Simd128_LdC);
    if (instr->GetDst()->GetType() == TySimd128)
    {
        Assert(instr->GetSrc1()->GetType() == TySimd128);
        Assert(instr->GetSrc1()->IsSimd128ConstOpnd());
        Assert(instr->GetSrc2() == NULL);

        AsmJsSIMDValue value = instr->GetSrc1()->AsSimd128ConstOpnd()->m_value;
        
        // ToDo: Optimize loading zero. MOVUPS_ZERO peeped into XORPS
        // ToDo: Avoid replicating constants. Map constants to memory locations and re-use them.

        // MOVUPS dst, [const]
        AsmJsSIMDValue *pValue = NativeCodeDataNew(instr->m_func->GetNativeCodeDataAllocator(), AsmJsSIMDValue);
        pValue->SetValue(value);
        IR::Opnd * opnd = IR::MemRefOpnd::New((void *)pValue, TySimd128, instr->m_func);
        instr->ReplaceSrc1(opnd);
        instr->m_opcode = LowererMDArch::GetAssignOp(TySimd128);
        Legalize(instr);
        return instr->m_prev;
    }
    else
    {
        AssertMsg(UNREACHED, "Non-typespecialized form of Simd128_LdC is unsupported");
    }
    return NULL;
}

// Lower float32x4 constructor
IR::Instr* LowererMD::Simd128LowerConstructor(IR::Instr *instr)
{
   
    IR::Opnd* dst  = NULL;
    IR::Opnd* src1 = NULL;
    IR::Opnd* src2 = NULL;
    IR::Opnd* src3 = NULL;
    IR::Opnd* src4 = NULL;
    

    if (instr->m_opcode == Js::OpCode::Simd128_FloatsToF4 || instr->m_opcode == Js::OpCode::Simd128_IntsToI4)
    {
        // use MOVSS for both int32x4 and float32x4. MOVD zeroes upper bits.
        Js::OpCode movOpcode = Js::OpCode::MOVSS;
        Js::OpCode shiftOpcode = Js::OpCode::PSLLDQ;
        SList<IR::Opnd*> *args = Simd128GetExtendedArgs(instr);

        // The number of src opnds should be exact. If opnds are missing, they should be filled in by globopt during type-spec.
        Assert(args->Count() == 5);

        dst = args->Pop();
        src1 = args->Pop();
        src2 = args->Pop();
        src3 = args->Pop();
        src4 = args->Pop();

        if (instr->m_opcode == Js::OpCode::Simd128_FloatsToF4)
        {
            // src's have to be regs. No float const prop should've happened.
            Assert(src1->IsRegOpnd() && src1->GetType() == TyFloat32);
            Assert(src2->IsRegOpnd() && src2->GetType() == TyFloat32);
            Assert(src3->IsRegOpnd() && src3->GetType() == TyFloat32);
            Assert(src4->IsRegOpnd() && src4->GetType() == TyFloat32);

            // MOVSS dst, src4
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, src4, m_func));
            // PSLLDQ dst, dst, 4
            instr->InsertBefore(IR::Instr::New(shiftOpcode, dst, dst, IR::IntConstOpnd::New(4, TyInt8, m_func, true), m_func));

            // MOVSS dst, src3
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, src3, m_func));
            // PSLLDQ dst, 4
            instr->InsertBefore(IR::Instr::New(shiftOpcode, dst, dst, IR::IntConstOpnd::New(4, TyInt8, m_func, true), m_func));

            // MOVSS dst, src2
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, src2, m_func));
            // PSLLDQ dst, 4
            instr->InsertBefore(IR::Instr::New(shiftOpcode, dst, dst, IR::IntConstOpnd::New(4, TyInt8, m_func, true), m_func));

            // MOVSS dst, src1
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, src1, m_func));
        }
        else
        {
            //Simd128_IntsToI4

            // b-namost: better way to implement this on SSE2? Using MOVD directly zeroes upper bits.
            IR::RegOpnd *temp = IR::RegOpnd::New(TyFloat32, m_func);
            
            // src's might have been constant prop'ed. Enregister them if so.
            src4 = EnregisterIntConst(instr, src4);
            src3 = EnregisterIntConst(instr, src3);
            src2 = EnregisterIntConst(instr, src2);
            src1 = EnregisterIntConst(instr, src1);
            
            Assert(src1->GetType() == TyInt32 && src1->IsRegOpnd());
            Assert(src2->GetType() == TyInt32 && src2->IsRegOpnd());
            Assert(src3->GetType() == TyInt32 && src3->IsRegOpnd());
            Assert(src4->GetType() == TyInt32 && src4->IsRegOpnd());

            // MOVD t(TyFloat32), src4(TyInt32)
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVD, temp, src4, m_func));
            
            // MOVSS dst, t
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, temp, m_func));
            // PSLLDQ dst, dst, 4
            instr->InsertBefore(IR::Instr::New(shiftOpcode, dst, dst, IR::IntConstOpnd::New(TySize[TyInt32], TyInt8, m_func, true), m_func));

            // MOVD t(TyFloat32), sr34(TyInt32)
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVD, temp, src3, m_func));
            // MOVSS dst, t
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, temp, m_func));
            // PSLLDQ dst, dst, 4
            instr->InsertBefore(IR::Instr::New(shiftOpcode, dst, dst, IR::IntConstOpnd::New(TySize[TyInt32], TyInt8, m_func, true), m_func));

            // MOVD t(TyFloat32), src2(TyInt32)
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVD, temp, src2, m_func));
            // MOVSS dst, t
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, temp, m_func));
            // PSLLDQ dst, dst, 4
            instr->InsertBefore(IR::Instr::New(shiftOpcode, dst, dst, IR::IntConstOpnd::New(TySize[TyInt32], TyInt8, m_func, true), m_func));

            // MOVD t(TyFloat32), src1(TyInt32)
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVD, temp, src1, m_func));
            // MOVSS dst, t
            instr->InsertBefore(IR::Instr::New(movOpcode, dst, temp, m_func));
        }
    }
    else
    {
        Assert(instr->m_opcode == Js::OpCode::Simd128_DoublesToD2);
        dst = instr->GetDst();
        src1 = instr->GetSrc1();
        src2 = instr->GetSrc2();

        Assert(src1->IsRegOpnd() && src1->GetType() == TyFloat64);
        Assert(src2->IsRegOpnd() && src2->GetType() == TyFloat64);
        // MOVSD dst, src2
        instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSD, dst, src2, m_func));
        // PSLLDQ dst, dst, 8
        instr->InsertBefore(IR::Instr::New(Js::OpCode::PSLLDQ, dst, dst, IR::IntConstOpnd::New(TySize[TyFloat64], TyInt8, m_func, true), m_func));
        // MOVSD dst, src1
        instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSD, dst, src1, m_func));
    }

    Assert(dst->IsRegOpnd() && dst->GetType() == TySimd128);
    IR::Instr* prevInstr;
    prevInstr = instr->m_prev;
    instr->Remove();
    return prevInstr;
}

IR::Instr* LowererMD::Simd128LowerLdLane(IR::Instr *instr)
{
    IR::Opnd* dst, *src1, *src2;
    Js::OpCode movOpcode = Js::OpCode::MOVSS;
    uint laneSize = 0, laneIndex = 0;

    dst = instr->GetDst();
    src1 = instr->GetSrc1();
    src2 = instr->GetSrc2();
    
    Assert(dst && dst->IsRegOpnd() && (dst->GetType() == TyFloat32 || dst->GetType() == TyInt32 || dst->GetType() == TyFloat64));
    Assert(src1 && src1->IsRegOpnd() && src1->GetType() == TySimd128);
    Assert(src2 && src2->IsIntConstOpnd());

    laneIndex = (uint)src2->AsIntConstOpnd()->m_value;

    switch (instr->m_opcode)
    {
    case Js::OpCode::Simd128_LdLane_F4:
        laneSize = 4;
        movOpcode = Js::OpCode::MOVSS;
        Assert(laneIndex < 4);
        break;
    case Js::OpCode::Simd128_LdLane_I4:
        laneSize = 4;
        movOpcode = Js::OpCode::MOVD;
        Assert(laneIndex < 4);
        break;
    case Js::OpCode::Simd128_LdLane_D2:
        laneSize = 8;
        movOpcode = Js::OpCode::MOVSD;
        Assert(laneIndex < 2);
        break;
    default:
        Assert(UNREACHED);
    }
    
    IR::Opnd* tmp = src1;
    if (laneIndex != 0)
    {
        // tmp = PSRLDQ src1, shamt
        tmp = IR::RegOpnd::New(TySimd128, m_func);
        IR::Instr *shiftInstr = IR::Instr::New(Js::OpCode::PSRLDQ, tmp, src1, IR::IntConstOpnd::New(laneSize * laneIndex, TyInt8, m_func, true), m_func);
        instr->InsertBefore(shiftInstr);
        //MakeDstEquSrc1(shiftInstr);
        Legalize(shiftInstr);
    }
    // MOVSS/MOVSD/MOVD dst, tmp
    instr->InsertBefore(IR::Instr::New(movOpcode, dst, tmp, m_func));
    IR::Instr* prevInstr = instr->m_prev;
    instr->Remove();

    return prevInstr;
}

IR::Instr* LowererMD::Simd128LowerStLane(IR::Instr *instr)
{
    IR::Opnd* dst, *src1, *laneValue;
    uint32 laneIndex = 0;
    int width = 0; // in bytes
    dst = instr->GetDst();
    src1 = instr->GetSrc1();
    laneValue = instr->GetSrc2();

    Assert(dst && dst->IsRegOpnd() && dst->GetType() == TySimd128);
    Assert(src1 && src1->IsRegOpnd() && src1->GetType() == TySimd128);
    
    // int32 operands can be const prop'ed. Enregister them.
    laneValue = laneValue->GetType() == TyInt32 ? EnregisterIntConst(instr, instr->GetSrc2()) : instr->GetSrc2();

    Assert(laneValue && laneValue->IsRegOpnd());
    
    switch (instr->m_opcode)
    {
    case Js::OpCode::Simd128_WithX_F4:
    case Js::OpCode::Simd128_WithX_I4:
        laneIndex = 0;
        width = TySize[TyInt32];
        break;
    case Js::OpCode::Simd128_WithX_D2:
        laneIndex = 0;
        width = TySize[TyFloat64];
        break;
    case Js::OpCode::Simd128_WithY_F4:
    case Js::OpCode::Simd128_WithY_I4:
        laneIndex = 1;
        width = TySize[TyInt32];
        break;
    case Js::OpCode::Simd128_WithY_D2:
        laneIndex = 1;
        width = TySize[TyFloat64];
        break;

    case Js::OpCode::Simd128_WithZ_F4:
    case Js::OpCode::Simd128_WithZ_I4:
        laneIndex = 2;
        width = TySize[TyInt32];
        break;
    case Js::OpCode::Simd128_WithW_F4:
    case Js::OpCode::Simd128_WithW_I4:
        laneIndex = 3;
        width = TySize[TyInt32];
        break;
    default:
        Assert(UNREACHED);
    }

    // MOVAPS dst, src1
    instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVAPS, dst, src1, m_func));
    
    if (width == TySize[TyFloat64])
    {
        Assert(laneValue->GetType() == TyFloat64);
        if (laneIndex == 0)
        {
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSD, dst, laneValue, m_func));
        }
        else
        {
            Assert(laneIndex == 1);
            IR::RegOpnd *tmp = IR::RegOpnd::New(TySimd128, m_func);
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVHLPS, tmp, dst, m_func));
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSD, tmp, laneValue, m_func));
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVLHPS, dst, tmp, m_func));
        }
    }
    else
    {
        Assert(width == TySize[TyInt32] && (laneValue->GetType() == TyInt32 || laneValue->GetType() == TyFloat32));
        if (laneValue->GetType() == TyInt32)
        {
            // value is in int register, move to xmm
            IR::RegOpnd *tempReg = IR::RegOpnd::New(TyFloat32, m_func);
            // MOVD 
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVD, tempReg, laneValue, m_func));
            laneValue = tempReg;
        }
        Assert(laneValue->GetType() == TyFloat32);
        if (laneIndex == 0)
        {
            // MOVSS for both TyFloat32 and TyInt32. MOVD zeroes upper bits.
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSS, dst, laneValue, m_func));
        }
        else if (laneIndex == 2)
        {
            IR::RegOpnd *tmp = IR::RegOpnd::New(TySimd128, m_func);
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVHLPS, tmp, dst, m_func));
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSS, tmp, laneValue, m_func));
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVLHPS, dst, tmp, m_func));
        }
        else 
        {
            Assert(laneIndex == 1 || laneIndex == 3);
            
            uint8 shufMask = 0xE4; // 11 10 01 00
            shufMask |= laneIndex; // 11 10 01 id
            shufMask &= ~(0x03 << (laneIndex << 1)); // set 2 bits corresponding to lane index to 00

            // SHUFPS dst, dst, shufMask
            instr->InsertBefore(IR::Instr::New(Js::OpCode::SHUFPS, dst, dst, IR::IntConstOpnd::New(shufMask, TyInt8, m_func, true), m_func));

            // MOVSS dst, value
            instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVSS, dst, laneValue, m_func));

            // SHUFPS dst, dst, shufMask
            instr->InsertBefore(IR::Instr::New(Js::OpCode::SHUFPS, dst, dst, IR::IntConstOpnd::New(shufMask, TyInt8, m_func, true), m_func));
        }
    }
    
    IR::Instr* prevInstr = instr->m_prev;
    instr->Remove();
    return prevInstr;
}

IR::Instr* LowererMD::Simd128LowerSplat(IR::Instr *instr)
{
    Js::OpCode shufOpCode = Js::OpCode::SHUFPS, movOpCode = Js::OpCode::MOVSS;
    IR::Opnd *dst, *src1;
    dst = instr->GetDst();
    src1 = instr->GetSrc1();

    Assert(dst && dst->IsRegOpnd() && dst->GetType() == TySimd128);
    Assert(src1 && src1->IsRegOpnd() && (src1->GetType() == TyFloat32 || src1->GetType() == TyInt32 || src1->GetType() == TyFloat64));
    Assert(!instr->GetSrc2());

    switch (instr->m_opcode)
    {
    case Js::OpCode::Simd128_Splat_F4:
        shufOpCode = Js::OpCode::SHUFPS;
        movOpCode = Js::OpCode::MOVSS;
        break;
    case Js::OpCode::Simd128_Splat_I4:
        shufOpCode = Js::OpCode::PSHUFD;
        movOpCode = Js::OpCode::MOVD;
        break;
    case Js::OpCode::Simd128_Splat_D2:
        shufOpCode = Js::OpCode::SHUFPD;
        movOpCode = Js::OpCode::MOVSD;
        break;
    default:
        Assert(UNREACHED);
    }

    instr->InsertBefore(IR::Instr::New(movOpCode, dst, src1, m_func));
    instr->InsertBefore(IR::Instr::New(shufOpCode, dst, dst, IR::IntConstOpnd::New(0, TyInt8, m_func, true), m_func));

    IR::Instr* prevInstr = instr->m_prev;
    instr->Remove();

    return prevInstr;
    
}


IR::Instr* LowererMD::Simd128LowerRcp(IR::Instr *instr, bool removeInstr)
{
    Js::OpCode opcode = Js::OpCode::DIVPS;
    void* x86_allones_mask = NULL;
    IR::Opnd *dst, *src1;
    dst = instr->GetDst();
    src1 = instr->GetSrc1();

    
    Assert(dst && dst->IsRegOpnd());
    Assert(src1 && src1->IsRegOpnd());
    Assert(instr->GetSrc2() == NULL);
    if (instr->m_opcode == Js::OpCode::Simd128_Rcp_F4 || instr->m_opcode == Js::OpCode::Simd128_RcpSqrt_F4)
    {
        opcode = Js::OpCode::DIVPS;
        x86_allones_mask = (void*)(&X86_ALL_ONES_F4);
    }
    else
    {
        Assert(instr->m_opcode == Js::OpCode::Simd128_Rcp_D2 || instr->m_opcode == Js::OpCode::Simd128_RcpSqrt_D2);
        opcode = Js::OpCode::DIVPD;
        x86_allones_mask = (void*)(&X86_ALL_ONES_D2);
    }
    IR::RegOpnd* tmp = IR::RegOpnd::New(TySimd128, m_func);
    IR::Instr* movInstr = IR::Instr::New(Js::OpCode::MOVAPS, tmp, IR::MemRefOpnd::New(x86_allones_mask, TySimd128, m_func), m_func);
    instr->InsertBefore(movInstr);
    Legalize(movInstr);

    instr->InsertBefore(IR::Instr::New(opcode, tmp, tmp, src1, m_func));
    instr->InsertBefore(IR::Instr::New(Js::OpCode::MOVAPS, dst, tmp, m_func));
    if (removeInstr)
    {
        IR::Instr* prevInstr = instr->m_prev;
        instr->Remove();
        return prevInstr;
    }
    return instr;
}

IR::Instr* LowererMD::Simd128LowerSqrt(IR::Instr *instr)
{
    Js::OpCode opcode = Js::OpCode::SQRTPS;
    
    IR::Opnd *dst, *src1;
    dst = instr->GetDst();
    src1 = instr->GetSrc1();

    Assert(dst && dst->IsRegOpnd());
    Assert(src1 && src1->IsRegOpnd());
    Assert(instr->GetSrc2() == NULL);
    if (instr->m_opcode == Js::OpCode::Simd128_Sqrt_F4)
    {
        opcode = Js::OpCode::SQRTPS;
    }
    else
    {
        Assert(instr->m_opcode == Js::OpCode::Simd128_Sqrt_D2);
        opcode = Js::OpCode::SQRTPD;
    }

    instr->InsertBefore(IR::Instr::New(opcode, dst, src1, m_func));

    IR::Instr* prevInstr = instr->m_prev;
    instr->Remove();
    return prevInstr;
}

IR::Instr* LowererMD::Simd128LowerRcpSqrt(IR::Instr *instr)
{
    Js::OpCode opcode = Js::OpCode::SQRTPS;
    Simd128LowerRcp(instr, false);
    
    if (instr->m_opcode == Js::OpCode::Simd128_RcpSqrt_F4)
    {
        opcode = Js::OpCode::SQRTPS;
    }
    else
    {
        Assert(instr->m_opcode == Js::OpCode::Simd128_RcpSqrt_D2);
        opcode = Js::OpCode::SQRTPD;
    }
    instr->InsertBefore(IR::Instr::New(opcode, instr->GetDst(), instr->GetDst(), m_func));
    IR::Instr* prevInstr = instr->m_prev;
    instr->Remove();
    return prevInstr;
}

IR::Instr* LowererMD::Simd128LowerSelect(IR::Instr *instr)
{
    Assert(instr->m_opcode == Js::OpCode::Simd128_Select_F4 || instr->m_opcode == Js::OpCode::Simd128_Select_I4 || instr->m_opcode == Js::OpCode::Simd128_Select_D2);
    IR::Opnd* dst = NULL;
    IR::Opnd* src1 = NULL;
    IR::Opnd* src2 = NULL;
    IR::Opnd* src3 = NULL;
    SList<IR::Opnd*> *args = Simd128GetExtendedArgs(instr);
    // The number of src opnds should be exact. Missing opnds means type-error, and we should generate an exception throw instead (or globopt does).
    Assert(args->Count() == 4);
    
    dst = args->Pop();
    src1 = args->Pop(); // mask
    src2 = args->Pop(); // trueValue
    src3 = args->Pop(); // falseValue
    
    Assert(dst->IsRegOpnd() && dst->GetType() == TySimd128);
    Assert(src1->IsRegOpnd() && src1->GetType() == TySimd128);
    Assert(src2->IsRegOpnd() && src2->GetType() == TySimd128);
    Assert(src3->IsRegOpnd() && src3->GetType() == TySimd128);

    
    IR::RegOpnd *tmp = IR::RegOpnd::New(TySimd128, m_func);
    IR::Instr *pInstr = NULL;
    // ANDPS tmp1, mask, tvalue
    pInstr = IR::Instr::New(Js::OpCode::ANDPS, tmp, src1, src2, m_func);
    instr->InsertBefore(pInstr);
    //MakeDstEquSrc1(pInstr);
    Legalize(pInstr);
    // ANDPS dst, mask, fvalue
    pInstr = IR::Instr::New(Js::OpCode::ANDNPS, dst, src1, src3, m_func);
    instr->InsertBefore(pInstr);
    //MakeDstEquSrc1(pInstr);
    Legalize(pInstr);
    // ORPS dst, dst, tmp1
    pInstr = IR::Instr::New(Js::OpCode::ORPS, dst, dst, tmp, m_func);
    instr->InsertBefore(pInstr);

    pInstr = instr->m_prev;
    instr->Remove();
    return pInstr;
}

IR::Instr* LowererMD::Simd128LowerNegI4(IR::Instr *instr)
{
    Assert(instr->m_opcode == Js::OpCode::Simd128_Neg_I4);
    IR::Opnd* dst = instr->GetDst();
    IR::Opnd* src1 = instr->GetSrc1();
    
    Assert(dst->IsRegOpnd() && dst->GetType() == TySimd128);
    Assert(src1->IsRegOpnd() && src1->GetType() == TySimd128);
    Assert(instr->GetSrc2() == NULL);

    // MOVAPS dst, src1
    IR::Instr *pInstr = IR::Instr::New(Js::OpCode::MOVAPS, dst, src1, m_func);
    instr->InsertBefore(pInstr);

    // XORPS dst, dst, 0xfff...f
    pInstr = IR::Instr::New(Js::OpCode::XORPS, dst, dst, IR::MemRefOpnd::New((void*)&X86_ALL_NEG_ONES, TySimd128, m_func), m_func);
    instr->InsertBefore(pInstr);
    Legalize(pInstr);

    // PADDD dst, dst, {1,1,1,1}
    pInstr = IR::Instr::New(Js::OpCode::PADDD, dst, dst, IR::MemRefOpnd::New((void*)&X86_ALL_ONES_I4, TySimd128, m_func), m_func);
    instr->InsertBefore(pInstr);
    Legalize(pInstr);

    pInstr = instr->m_prev;
    instr->Remove();
    return pInstr;
}

IR::Instr* LowererMD::Simd128LowerMulI4(IR::Instr *instr)
{
    Assert(instr->m_opcode == Js::OpCode::Simd128_Mul_I4);
    IR::Instr *pInstr;
    IR::Opnd* dst = instr->GetDst();
    IR::Opnd* src1 = instr->GetSrc1();
    IR::Opnd* src2 = instr->GetSrc2();
    IR::Opnd* temp1, *temp2, *temp3;
    Assert(dst->IsRegOpnd() && dst->GetType() == TySimd128);
    Assert(src1->IsRegOpnd() && src1->GetType() == TySimd128);
    Assert(src2->IsRegOpnd() && src2->GetType() == TySimd128);

    temp1 = IR::RegOpnd::New(TySimd128, m_func);
    temp2 = IR::RegOpnd::New(TySimd128, m_func);
    temp3 = IR::RegOpnd::New(TySimd128, m_func);
    
    // temp1 = PMULUDQ src1, src2
    pInstr = IR::Instr::New(Js::OpCode::PMULUDQ, temp1, src1, src2, m_func);
    instr->InsertBefore(pInstr);
    //MakeDstEquSrc1(pInstr);
    Legalize(pInstr);

    // temp2 = PSLRD src1, 0x4
    pInstr = IR::Instr::New(Js::OpCode::PSRLDQ, temp2, src1, IR::IntConstOpnd::New(TySize[TyInt32], TyInt8, m_func, true), m_func);
    instr->InsertBefore(pInstr);
    //MakeDstEquSrc1(pInstr);
    Legalize(pInstr);

    // temp3 = PSLRD src2, 0x4
    pInstr = IR::Instr::New(Js::OpCode::PSRLDQ, temp3, src2, IR::IntConstOpnd::New(TySize[TyInt32], TyInt8, m_func, true), m_func);
    instr->InsertBefore(pInstr);
    //MakeDstEquSrc1(pInstr);
    Legalize(pInstr);

    // temp2 = PMULUDQ temp2, temp3
    pInstr = IR::Instr::New(Js::OpCode::PMULUDQ, temp2, temp2, temp3, m_func);
    instr->InsertBefore(pInstr);
    Legalize(pInstr);

    //PSHUFD temp1, temp1, 0x8
    instr->InsertBefore(IR::Instr::New(Js::OpCode::PSHUFD, temp1, temp1, IR::IntConstOpnd::New( 8 /*b00001000*/, TyInt8, m_func, true), m_func));

    //PSHUFD temp2, temp2, 0x8
    instr->InsertBefore(IR::Instr::New(Js::OpCode::PSHUFD, temp2, temp2, IR::IntConstOpnd::New(8 /*b00001000*/, TyInt8, m_func, true), m_func));
    
    // PUNPCKLDQ dst, temp1, temp2
    pInstr = IR::Instr::New(Js::OpCode::PUNPCKLDQ, dst, temp1, temp2, m_func);
    instr->InsertBefore(pInstr);
    Legalize(pInstr);

    
    pInstr = instr->m_prev;
    instr->Remove();
    return pInstr;
}

// Builds args list <dst, src1, src2, src3 ..>
SList<IR::Opnd*> * LowererMD::Simd128GetExtendedArgs(IR::Instr *instr)
{
    SList<IR::Opnd*> * args = JitAnew(m_lowerer->m_alloc, SList<IR::Opnd*>, m_lowerer->m_alloc);
    IR::Instr *pInstr = instr;
    IR::Opnd *dst, *src1, *src2;

    dst = src1 = src2 = NULL;

    dst = pInstr->UnlinkDst();

    src1 = pInstr->UnlinkSrc1();
    Assert(src1->GetStackSym()->IsSingleDef());


    pInstr = src1->GetStackSym()->GetInstrDef();
    
    while (pInstr && pInstr->m_opcode == Js::OpCode::ExtendArg_A)
    {
        Assert(pInstr->GetSrc1());
        src1 = pInstr->GetSrc1()->Copy(this->m_func);
        if (src1->IsRegOpnd())
        {
            this->m_lowerer->addToLiveOnBackEdgeSyms->Set(src1->AsRegOpnd()->m_sym->m_id);
        }
        args->Push(src1);

        if (pInstr->GetSrc2())
        {
            src2 = pInstr->GetSrc2();
            Assert(src2->GetStackSym()->IsSingleDef());
            pInstr = src2->GetStackSym()->GetInstrDef();
        }
        else
        {
            pInstr = NULL;
        }

    }
    args->Push(dst);
    Assert(args->Count() > 3);
    return args;
}

IR::Opnd* LowererMD::EnregisterIntConst(IR::Instr* instr, IR::Opnd *constOpnd)
{
    if (constOpnd->IsRegOpnd())
    {
        // already a register
        return constOpnd;
    }
    Assert(constOpnd->GetType() == TyInt32);
    IR::RegOpnd *tempReg = IR::RegOpnd::New(TyInt32, m_func);
    
    // MOV tempReg, constOpnd
    instr->InsertBefore(IR::Instr::New(Js::OpCode::MOV, tempReg, constOpnd, m_func));
    return tempReg;
}

void LowererMD::Simd128InitOpcodeMap()
{
    m_simd128OpCodesMap = JitAnewArrayZ(m_lowerer->m_alloc, Js::OpCode, (uint32)(Js::OpCode::Simd128_End - Js::OpCode::Simd128_Start));
    SET_SIMDOPCODE(Simd128_FromFloat64x2_I4     , CVTTPD2DQ);
    SET_SIMDOPCODE(Simd128_FromFloat64x2Bits_I4 , MOVAPS);
    SET_SIMDOPCODE(Simd128_FromFloat32x4_I4     , CVTTPS2DQ);
    SET_SIMDOPCODE(Simd128_FromFloat32x4Bits_I4 , MOVAPS);
    SET_SIMDOPCODE(Simd128_Add_I4               , PADDD);
    SET_SIMDOPCODE(Simd128_Sub_I4               , PSUBD);
    SET_SIMDOPCODE(Simd128_Lt_I4                , PCMPGTD);
    SET_SIMDOPCODE(Simd128_Gt_I4                , PCMPGTD);
    SET_SIMDOPCODE(Simd128_Eq_I4                , PCMPEQD);
    SET_SIMDOPCODE(Simd128_And_I4               , PAND);
    SET_SIMDOPCODE(Simd128_Or_I4                , POR);
    SET_SIMDOPCODE(Simd128_Xor_I4               , XORPS);
    SET_SIMDOPCODE(Simd128_Not_I4               , XORPS);
    SET_SIMDOPCODE(Simd128_LdSignMask_I4        , MOVMSKPS);
    
    SET_SIMDOPCODE(Simd128_FromFloat64x2_F4      , CVTPD2PS);
    SET_SIMDOPCODE(Simd128_FromFloat64x2Bits_F4  , MOVAPS);
    SET_SIMDOPCODE(Simd128_FromInt32x4_F4        , CVTDQ2PS);
    SET_SIMDOPCODE(Simd128_FromInt32x4Bits_F4    , MOVAPS);
    SET_SIMDOPCODE(Simd128_Abs_F4                , ANDPS);
    SET_SIMDOPCODE(Simd128_Neg_F4                , XORPS);
    SET_SIMDOPCODE(Simd128_Add_F4                , ADDPS);
    SET_SIMDOPCODE(Simd128_Sub_F4                , SUBPS);
    SET_SIMDOPCODE(Simd128_Mul_F4                , MULPS);
    SET_SIMDOPCODE(Simd128_Div_F4                , DIVPS);
    SET_SIMDOPCODE(Simd128_Min_F4                , MINPS);
    SET_SIMDOPCODE(Simd128_Max_F4                , MAXPS);
    SET_SIMDOPCODE(Simd128_Sqrt_F4               , SQRTPS);
    SET_SIMDOPCODE(Simd128_Lt_F4                 , CMPLTPS); // CMPLTPS
    SET_SIMDOPCODE(Simd128_LtEq_F4               , CMPLEPS); // CMPLEPS
    SET_SIMDOPCODE(Simd128_Eq_F4                 , CMPEQPS); // CMPEQPS
    SET_SIMDOPCODE(Simd128_Neq_F4                , CMPNEQPS); // CMPNEQPS
    SET_SIMDOPCODE(Simd128_Gt_F4                 , CMPLTPS); // CMPLTPS (swap srcs)
    SET_SIMDOPCODE(Simd128_GtEq_F4               , CMPLEPS); // CMPLEPS (swap srcs)
    SET_SIMDOPCODE(Simd128_And_F4                , ANDPS);
    SET_SIMDOPCODE(Simd128_Or_F4                 , ORPS);
    SET_SIMDOPCODE(Simd128_Xor_F4                , XORPS );
    SET_SIMDOPCODE(Simd128_Not_F4                , XORPS );
    SET_SIMDOPCODE(Simd128_LdSignMask_F4         , MOVMSKPS );

    SET_SIMDOPCODE(Simd128_FromFloat32x4_D2     , CVTPS2PD);
    SET_SIMDOPCODE(Simd128_FromFloat32x4Bits_D2 , MOVAPS);
    SET_SIMDOPCODE(Simd128_FromInt32x4_D2       , CVTDQ2PD);
    SET_SIMDOPCODE(Simd128_FromInt32x4Bits_D2   , MOVAPS);
    SET_SIMDOPCODE(Simd128_Neg_D2               , XORPS);
    SET_SIMDOPCODE(Simd128_Add_D2               , ADDPD);
    SET_SIMDOPCODE(Simd128_Abs_D2               , ANDPD);
    SET_SIMDOPCODE(Simd128_Sub_D2               , SUBPD);
    SET_SIMDOPCODE(Simd128_Mul_D2               , MULPD);
    SET_SIMDOPCODE(Simd128_Div_D2               , DIVPD);
    SET_SIMDOPCODE(Simd128_Min_D2               , MINPD);
    SET_SIMDOPCODE(Simd128_Max_D2               , MAXPD);
    SET_SIMDOPCODE(Simd128_Sqrt_D2              , SQRTPD);
    SET_SIMDOPCODE(Simd128_Lt_D2                , CMPLTPD); // CMPLTPD
    SET_SIMDOPCODE(Simd128_LtEq_D2              , CMPLEPD); // CMPLEPD
    SET_SIMDOPCODE(Simd128_Eq_D2                , CMPEQPD); // CMPEQPD
    SET_SIMDOPCODE(Simd128_Neq_D2               , CMPNEQPD); // CMPNEQPD
    SET_SIMDOPCODE(Simd128_Gt_D2                , CMPLTPD); // CMPLTPD (swap srcs)
    SET_SIMDOPCODE(Simd128_GtEq_D2              , CMPLEPD); // CMPLEPD (swap srcs)
    SET_SIMDOPCODE(Simd128_LdSignMask_D2        , MOVMSKPD);
}


#undef SIMD_SETOPCODE
#undef SIMD_GETOPCODE