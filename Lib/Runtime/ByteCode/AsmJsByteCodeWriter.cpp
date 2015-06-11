//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js {

    template <>
    __inline size_t ByteCodeWriter::Data::EncodeT<SmallLayout>(OpCodeAsmJs op, ByteCodeWriter* writer, bool isPatching)
    {
        Assert(op < Js::OpCodeAsmJs::ByteCodeLast);

        size_t offset;
        if (op <= Js::OpCode::MaxByteSizedOpcodes)
        {
            byte byteop = (byte)op;
            offset = Write(&byteop, sizeof(byte));
        }
        else
        {
            byte byteop = (byte)Js::OpCodeAsmJs::ExtendedOpcodePrefix;
            offset = Write(&byteop, sizeof(byte));
            byteop = (byte)op;
            Write(&byteop, sizeof(byte));
        }
        if (!isPatching)
        {
            writer->IncreaseByteCodeCount();
        }
        return offset;
    }

    template <LayoutSize layoutSize>
    __inline size_t ByteCodeWriter::Data::EncodeT(OpCodeAsmJs op, ByteCodeWriter* writer, bool isPatching)
    {
        Assert(op < Js::OpCodeAsmJs::ByteCodeLast);

        CompileAssert(layoutSize != SmallLayout);
        const byte exop = (byte)((op <= Js::OpCodeAsmJs::MaxByteSizedOpcodes)?
            (layoutSize == LargeLayout? Js::OpCodeAsmJs::LargeLayoutPrefix : Js::OpCodeAsmJs::MediumLayoutPrefix) :
            (layoutSize == LargeLayout? Js::OpCodeAsmJs::ExtendedLargeLayoutPrefix : Js::OpCodeAsmJs::ExtendedMediumLayoutPrefix));

        size_t offset = Write(&exop, sizeof(byte));
        byte byteop = (byte)op;
        Write(&byteop, sizeof(byte));

        if (!isPatching)
        {
            writer->IncreaseByteCodeCount();
        }
        return offset;
    }

    template <LayoutSize layoutSize>
    __inline size_t ByteCodeWriter::Data::EncodeT(OpCodeAsmJs op, const void* rawData, int byteSize, ByteCodeWriter* writer, bool isPatching)
    {
        AssertMsg((rawData != null) && (byteSize < 100), "Ensure valid data for opcode");

        size_t offset = EncodeT<layoutSize>(op, writer, isPatching);
        Write(rawData, byteSize);
        return offset;
    }

    
    void AsmJsByteCodeWriter::InitData( ArenaAllocator* alloc, long initCodeBufferSize )
    {
        ByteCodeWriter::InitData( alloc, initCodeBufferSize );
#ifdef BYTECODE_BRANCH_ISLAND
        useBranchIsland = false;
#endif
    }


    #define MULTISIZE_LAYOUT_WRITE(layout, ...) \
    if (!TryWrite##layout<SmallLayoutSizePolicy>(__VA_ARGS__) && !TryWrite##layout<MediumLayoutSizePolicy>(__VA_ARGS__)) \
    { \
        bool success = TryWrite##layout<LargeLayoutSizePolicy>(__VA_ARGS__); \
        Assert(success); \
    }

    //////////////////////////////////////////////////////////////////////////
    /// Asm.js Specific functions

    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmJsUnsigned1(OpCodeAsmJs op, uint C1)
    {
        OpLayoutT_AsmUnsigned1<SizePolicy> layout;
        if (SizePolicy::Assign(layout.C1, C1))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }

    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg1( OpCodeAsmJs op, RegSlot R0 )
    {
        OpLayoutT_AsmReg1<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg2( OpCodeAsmJs op, RegSlot R0, RegSlot R1 )
    {
        OpLayoutT_AsmReg2<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg3( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2 )
    {
        OpLayoutT_AsmReg3<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1) && SizePolicy::Assign(layout.R2, R2))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg4( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3 )
    {
        OpLayoutT_AsmReg4<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1) && SizePolicy::Assign(layout.R2, R2) && SizePolicy::Assign(layout.R3, R3))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg5(OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4)
    {
        OpLayoutT_AsmReg5<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1) && SizePolicy::Assign(layout.R2, R2) && SizePolicy::Assign(layout.R3, R3) && SizePolicy::Assign(layout.R4, R4))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg6(OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5)
    {
        OpLayoutT_AsmReg6<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1) && SizePolicy::Assign(layout.R2, R2) && SizePolicy::Assign(layout.R3, R3) && SizePolicy::Assign(layout.R4, R4) && SizePolicy::Assign(layout.R5, R5))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg7(OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6)
    {
        OpLayoutT_AsmReg7<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1) && SizePolicy::Assign(layout.R2, R2) && SizePolicy::Assign(layout.R3, R3) && SizePolicy::Assign(layout.R4, R4) && SizePolicy::Assign(layout.R5, R5) && SizePolicy::Assign(layout.R6, R6))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteAsmReg2IntConst1(OpCodeAsmJs op, RegSlot R0, RegSlot R1, int C2)
    {
        OpLayoutT_AsmReg2IntConst1<SizePolicy> layout;
        if (SizePolicy::Assign(layout.R0, R0) && SizePolicy::Assign(layout.R1, R1) && SizePolicy::Assign(layout.C2, C2))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    template <typename SizePolicy>
    bool AsmJsByteCodeWriter::TryWriteInt1Const1( OpCodeAsmJs op, RegSlot R0,  int C1 )
    {
        OpLayoutT_Int1Const1<SizePolicy> layout;
        if( SizePolicy::Assign( layout.I0, R0 ) && SizePolicy::Assign( layout.C1, C1 ) )
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>( op, &layout, sizeof( layout ), this );
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteDouble1Addr1( OpCodeAsmJs op, RegSlot R0, const double* A1 )
    {
        OpLayoutT_Double1Addr1<SizePolicy> layout;
        if( SizePolicy::Assign( layout.D0, R0 ) && SizePolicy::Assign( layout.A1, A1 ) )
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>( op, &layout, sizeof( layout ), this );
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteInt1Const2( OpCodeAsmJs op, RegSlot R0, int C1, int C2 )
    {
        OpLayoutT_Int1Const2<SizePolicy> layout;
        if( SizePolicy::Assign( layout.I0, R0 ) && SizePolicy::Assign( layout.C1, C1 ) && SizePolicy::Assign( layout.C2, C2 ) )
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>( op, &layout, sizeof( layout ), this );
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteDouble1Const2( OpCodeAsmJs op, RegSlot R0, double C1, double C2 )
    {
        OpLayoutT_Double1Const2<SizePolicy> layout;
        if( SizePolicy::Assign( layout.D0, R0 ) && SizePolicy::Assign( layout.C1, C1 ) && SizePolicy::Assign( layout.C2, C2 ) )
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>( op, &layout, sizeof( layout ), this );
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteInt2Const1( OpCodeAsmJs op, RegSlot R0, RegSlot R1, int C2 )
    {
        OpLayoutT_Int2Const1<SizePolicy> layout;
        if( SizePolicy::Assign( layout.I0, R0 ) && SizePolicy::Assign( layout.I1, R1 ) && SizePolicy::Assign( layout.C2, C2 ) )
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>( op, &layout, sizeof( layout ), this );
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteDouble2Const1( OpCodeAsmJs op, RegSlot R0, RegSlot R1, double C2 )
    {
        OpLayoutT_Double2Const1<SizePolicy> layout;
        if( SizePolicy::Assign( layout.D0, R0 ) && SizePolicy::Assign( layout.D1, R1 ) && SizePolicy::Assign( layout.C2, C2 ) )
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>( op, &layout, sizeof( layout ), this );
            return true;
        }
        return false;
    }


    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteAsmBrReg1( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1 )
    {
        OpLayoutT_BrInt1<SizePolicy> layout;
        if (SizePolicy::Assign(layout.I1, R1))
        {
            size_t const offsetOfRelativeJumpOffsetFromEnd = sizeof(OpLayoutT_BrInt1<SizePolicy>) - offsetof(OpLayoutT_BrInt1<SizePolicy>, RelativeJumpOffset);
            layout.RelativeJumpOffset = offsetOfRelativeJumpOffsetFromEnd;
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            AddJumpOffset(op, labelID, offsetOfRelativeJumpOffsetFromEnd);
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteAsmBrReg2( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, RegSlot R2 )
    {
        OpLayoutT_BrInt2<SizePolicy> layout;
        if (SizePolicy::Assign(layout.I1, R1) && SizePolicy::Assign(layout.I2, R2))
        {
            size_t const offsetOfRelativeJumpOffsetFromEnd = sizeof(OpLayoutT_BrInt2<SizePolicy>) - offsetof(OpLayoutT_BrInt2<SizePolicy>, RelativeJumpOffset);
            layout.RelativeJumpOffset = offsetOfRelativeJumpOffsetFromEnd;
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            AddJumpOffset(op, labelID, offsetOfRelativeJumpOffsetFromEnd);
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteAsmCall( OpCodeAsmJs op, RegSlot returnValueRegister, RegSlot functionRegister, ArgSlot givenArgCount, AsmJsRetType retType )
    {
        OpLayoutT_AsmCall<SizePolicy> layout;
        if (SizePolicy::Assign(layout.Return, returnValueRegister) && SizePolicy::Assign(layout.Function, functionRegister)
            && SizePolicy::Assign(layout.ArgCount, givenArgCount) && SizePolicy::Assign<int8>(layout.ReturnType, (int8)retType.which()))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteAsmSlot( OpCodeAsmJs op, RegSlot value, RegSlot instance, int32 slotId )
    {
        OpLayoutT_ElementSlot<SizePolicy> layout;
        if (SizePolicy::Assign(layout.Value, value) && SizePolicy::Assign(layout.Instance, instance)
            && SizePolicy::Assign(layout.SlotIndex, slotId))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }

    template <typename SizePolicy> 
    bool AsmJsByteCodeWriter::TryWriteAsmTypedArr( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, ArrayBufferView::ViewType viewType )
    {
        OpLayoutT_AsmTypedArr<SizePolicy> layout;
        if (SizePolicy::Assign(layout.Value, value) && SizePolicy::Assign<int8>(layout.ViewType, (int8)viewType)
            && SizePolicy::Assign(layout.SlotIndex, slotIndex))
        {
            m_byteCodeData.EncodeT<SizePolicy::LayoutEnum>(op, &layout, sizeof(layout), this);
            return true;
        }
        return false;
    }
    


    void AsmJsByteCodeWriter::EmptyAsm(OpCodeAsmJs op)
    {
        m_byteCodeData.Encode(op, this);
    }

    void AsmJsByteCodeWriter::Conv( OpCodeAsmJs op, RegSlot R0, RegSlot R1 )
    {
        MULTISIZE_LAYOUT_WRITE( AsmReg2, op, R0, R1 );
    }

    void AsmJsByteCodeWriter::AsmInt1Const1( OpCodeAsmJs op, RegSlot R0, int C1 )
    {
        MULTISIZE_LAYOUT_WRITE( Int1Const1, op, R0, C1 );
    }

    void AsmJsByteCodeWriter::AsmDouble1Addr1( OpCodeAsmJs op, RegSlot R0, const double* A1 )
    {
        MULTISIZE_LAYOUT_WRITE( Double1Addr1, op, R0, A1 );
    }

    void AsmJsByteCodeWriter::AsmDouble1Const2( OpCodeAsmJs op, RegSlot R0, double C1, double C2 )
    {
        MULTISIZE_LAYOUT_WRITE( Double1Const2, op, R0, C1, C2 );
    }

    void AsmJsByteCodeWriter::AsmDouble2Const1( OpCodeAsmJs op, RegSlot R0, RegSlot R1, double C2 )
    {
        MULTISIZE_LAYOUT_WRITE( Double2Const1, op, R0, R1, C2 );
    }

    void AsmJsByteCodeWriter::AsmReg1( OpCodeAsmJs op, RegSlot R0 )
    {
        MULTISIZE_LAYOUT_WRITE( AsmReg1, op, R0 );
    }

    void AsmJsByteCodeWriter::AsmReg2( OpCodeAsmJs op, RegSlot R0, RegSlot R1 )
    {
        MULTISIZE_LAYOUT_WRITE( AsmReg2, op, R0, R1 );
    }

    void AsmJsByteCodeWriter::AsmReg3( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2 )
    {
        MULTISIZE_LAYOUT_WRITE( AsmReg3, op, R0, R1, R2 );
    }

    void AsmJsByteCodeWriter::AsmReg4( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3 )
    {
        MULTISIZE_LAYOUT_WRITE( AsmReg4, op, R0, R1, R2, R3 );
    }
    
    void AsmJsByteCodeWriter::AsmReg5( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4 )
    {
        MULTISIZE_LAYOUT_WRITE(AsmReg5, op, R0, R1, R2, R3, R4);
    }

    void AsmJsByteCodeWriter::AsmReg6(OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5)
    {
        MULTISIZE_LAYOUT_WRITE(AsmReg6, op, R0, R1, R2, R3, R4, R5);
    }

    void AsmJsByteCodeWriter::AsmReg7(OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6)
    {
        MULTISIZE_LAYOUT_WRITE(AsmReg7, op, R0, R1, R2, R3, R4, R5, R6);
    }

    void AsmJsByteCodeWriter::AsmReg2IntConst1(OpCodeAsmJs op, RegSlot R0, RegSlot R1, int C2)
    {
        MULTISIZE_LAYOUT_WRITE(AsmReg2IntConst1, op, R0, R1, C2);
    }

    void AsmJsByteCodeWriter::AsmBr( ByteCodeLabel labelID )
    {
        CheckOpen();
        CheckLabel( labelID );

        const OpCodeAsmJs op = OpCodeAsmJs::AsmBr;
        size_t const offsetOfRelativeJumpOffsetFromEnd = sizeof(OpLayoutAsmBr) - offsetof(OpLayoutAsmBr, RelativeJumpOffset);
        OpLayoutAsmBr data;
        data.RelativeJumpOffset = offsetOfRelativeJumpOffsetFromEnd;

        m_byteCodeData.Encode(op, &data, sizeof(data), this);
        AddJumpOffset(op, labelID, offsetOfRelativeJumpOffsetFromEnd);
    }


    void AsmJsByteCodeWriter::AsmBrReg1( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1 )
    {
        CheckOpen();
        CheckLabel( labelID );

        MULTISIZE_LAYOUT_WRITE( AsmBrReg1, op, labelID, R1 );
    }

    void AsmJsByteCodeWriter::AsmBrReg2( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, RegSlot R2 )
    {
        CheckOpen();
        CheckLabel( labelID );

        MULTISIZE_LAYOUT_WRITE( AsmBrReg2, op, labelID, R1, R2 );
    }

    void AsmJsByteCodeWriter::AsmStartCall( OpCodeAsmJs op, ArgSlot ArgCount, bool isPatching)
    {
        CheckOpen();

        OpLayoutStartCall data;
        data.ArgCount = ArgCount;
        m_byteCodeData.Encode( op, &data, sizeof( data ), this, isPatching);
    }

    void AsmJsByteCodeWriter::AsmCall( OpCodeAsmJs op, RegSlot returnValueRegister, RegSlot functionRegister, ArgSlot givenArgCount, AsmJsRetType retType )
    {
        MULTISIZE_LAYOUT_WRITE( AsmCall, op, returnValueRegister, functionRegister, givenArgCount, retType );
    }

    void AsmJsByteCodeWriter::AsmTypedArr( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, ArrayBufferView::ViewType viewType)
    { 
        MULTISIZE_LAYOUT_WRITE( AsmTypedArr, op, value, slotIndex, viewType );
    }

    void AsmJsByteCodeWriter::AsmSlot( OpCodeAsmJs op, RegSlot value, RegSlot instance, int32 slotId )
    {
        MULTISIZE_LAYOUT_WRITE( AsmSlot, op, value, instance, slotId );
    }

    uint AsmJsByteCodeWriter::EnterLoop(Js::ByteCodeLabel loopEntrance)
    {
        uint loopId = m_functionWrite->IncrLoopCount();
        Assert((uint)m_loopHeaders->Count() == loopId);
		
        m_loopHeaders->Add(LoopHeaderData(m_byteCodeData.GetCurrentOffset(), 0, m_loopNest > 0));
        m_loopNest++;
        Js::OpCodeAsmJs loopBodyOpcode = Js::OpCodeAsmJs::AsmJsLoopBodyStart;
        this->MarkAsmJsLabel(loopEntrance);
        this->AsmJsUnsigned1(loopBodyOpcode, loopId);
        
        return loopId;
    }

    void AsmJsByteCodeWriter::AsmJsUnsigned1(OpCodeAsmJs op, uint c1)
    {
        MULTISIZE_LAYOUT_WRITE(AsmJsUnsigned1, op, c1);
    }
    void AsmJsByteCodeWriter::ExitLoop(uint loopId)
    {
        Assert(m_loopNest > 0);
        m_loopNest--;
        m_loopHeaders->Item(loopId).endOffset = m_byteCodeData.GetCurrentOffset();
    }

    void AsmJsByteCodeWriter::AddJumpOffset( Js::OpCodeAsmJs op, ByteCodeLabel labelId, size_t fieldByteOffset )
    {
        AssertMsg(fieldByteOffset < 100, "Ensure valid field offset");
        CheckOpen();
        CheckLabel(labelId);

        size_t jumpByteOffset = m_byteCodeData.GetCurrentOffset() - fieldByteOffset;

        //
        // Branch targets are created in two passes:
        // - In the instruction stream, write "labelID" into "OpLayoutBrC.Offset".  Record this
        //   location in "m_jumpOffsets" to be patched later.
        // - When the byte-code is closed, update all "OpLayoutBrC.Offset"'s with their actual
        //   destinations.
        //

        JumpInfo jumpInfo = { labelId, jumpByteOffset };
        m_jumpOffsets->Add(jumpInfo);
    }

    void AsmJsByteCodeWriter::MarkAsmJsLabel( ByteCodeLabel labelID )
    {
        MarkLabel( labelID );
        EmptyAsm( OpCodeAsmJs::Label );
    }

} // namespace Js
