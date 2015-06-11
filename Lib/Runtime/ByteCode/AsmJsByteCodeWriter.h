//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    struct AsmJsByteCodeWriter : public ByteCodeWriter
    {
    private:
        using ByteCodeWriter::MarkLabel;
    public:
        void InitData        ( ArenaAllocator* alloc, long initCodeBufferSize );
        void EmptyAsm        ( OpCodeAsmJs op );
        void Conv            ( OpCodeAsmJs op, RegSlot R0, RegSlot R1 );
        void AsmInt1Const1   ( OpCodeAsmJs op, RegSlot R0, int C1 );
        void AsmDouble1Addr1 ( OpCodeAsmJs op, RegSlot R0, const double* A1 );
        void AsmDouble1Const2( OpCodeAsmJs op, RegSlot R0, double C1, double C2 );
        void AsmDouble2Const1( OpCodeAsmJs op, RegSlot R0, RegSlot R1, double C2 );
        void AsmReg1         ( OpCodeAsmJs op, RegSlot R0 );
        void AsmReg2         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1 );
        void AsmReg3         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2 );
        void AsmReg4         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3 );
        void AsmReg5         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4 );
        void AsmReg6         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5 );
        void AsmReg7         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6 );
        void AsmReg2IntConst1( OpCodeAsmJs op, RegSlot R0, RegSlot R1, int C2);
        void AsmBr           ( ByteCodeLabel labelID );
        void AsmBrReg1       ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1 );
        void AsmBrReg2       ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, RegSlot R2 );
        void AsmStartCall    ( OpCodeAsmJs op, ArgSlot ArgCount, bool isPatching = false);
        void AsmCall         ( OpCodeAsmJs op, RegSlot returnValueRegister, RegSlot functionRegister, ArgSlot givenArgCount, AsmJsRetType retType );
        void AsmSlot         ( OpCodeAsmJs op, RegSlot value, RegSlot instance, int32 slotId );
        void AsmTypedArr     ( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, ArrayBufferView::ViewType viewType);

        void MarkAsmJsLabel(ByteCodeLabel labelID);
        void AsmJsUnsigned1(OpCodeAsmJs op, uint C1);
        uint EnterLoop(Js::ByteCodeLabel loopEntrance);
        void ExitLoop(uint loopId);

    private:
        template <typename SizePolicy> bool TryWriteAsmReg1      ( OpCodeAsmJs op, RegSlot R0 );
        template <typename SizePolicy> bool TryWriteAsmReg2      ( OpCodeAsmJs op, RegSlot R0, RegSlot R1 );
        template <typename SizePolicy> bool TryWriteAsmReg3      ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2 );
        template <typename SizePolicy> bool TryWriteAsmReg4      ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3 );
        template <typename SizePolicy> bool TryWriteAsmReg5      ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4 );
        template <typename SizePolicy> bool TryWriteAsmReg6      ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5);
        template <typename SizePolicy> bool TryWriteAsmReg7      ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6);
        template <typename SizePolicy> bool TryWriteAsmReg2IntConst1(OpCodeAsmJs op, RegSlot R0, RegSlot R1, int C2);
        template <typename SizePolicy> bool TryWriteInt1Const1   ( OpCodeAsmJs op, RegSlot R0, int C1 );
        template <typename SizePolicy> bool TryWriteDouble1Addr1 ( OpCodeAsmJs op, RegSlot R0, const double* A1 );
        template <typename SizePolicy> bool TryWriteInt1Const2   ( OpCodeAsmJs op, RegSlot R0, int C1, int C2 );
        template <typename SizePolicy> bool TryWriteDouble1Const2( OpCodeAsmJs op, RegSlot R0, double C1, double C2 );
        template <typename SizePolicy> bool TryWriteInt2Const1   ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, int C2 );
        template <typename SizePolicy> bool TryWriteDouble2Const1( OpCodeAsmJs op, RegSlot R0, RegSlot R1, double C2 );
        template <typename SizePolicy> bool TryWriteAsmBrReg1    ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1 );
        template <typename SizePolicy> bool TryWriteAsmBrReg2    ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, RegSlot R2 );
        template <typename SizePolicy> bool TryWriteAsmCall      ( OpCodeAsmJs op, RegSlot returnValueRegister, RegSlot functionRegister, ArgSlot givenArgCount, AsmJsRetType retType );
        template <typename SizePolicy> bool TryWriteAsmSlot      ( OpCodeAsmJs op, RegSlot value, RegSlot instance, int32 slotId );
        template <typename SizePolicy> bool TryWriteAsmTypedArr  ( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, ArrayBufferView::ViewType viewType);
        template <typename SizePolicy> bool TryWriteAsmJsUnsigned1(OpCodeAsmJs op, uint C1);
        void AddJumpOffset(Js::OpCodeAsmJs op, ByteCodeLabel labelId, size_t fieldByteOffset);
    };
}

