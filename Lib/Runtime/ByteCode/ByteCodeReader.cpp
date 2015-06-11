//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    void ByteCodeReader::Create(FunctionBody * functionRead, uint startOffset /* = 0 */)
    {
        Assert(functionRead);
        ByteCodeReader::Create(functionRead, startOffset, /* useOriginalByteCode = */ false);
    }

    void ByteCodeReader::Create(FunctionBody* functionRead, uint startOffset, bool useOriginalByteCode)
    {
        AssertMsg(functionRead != null, "Must provide valid function to execute");

        ByteBlock * byteCodeBlock = useOriginalByteCode ? 
            functionRead->GetOriginalByteCode() : 
            functionRead->GetByteCode();
            
        AssertMsg(byteCodeBlock != null, "Must have valid byte-code to read");

        m_startLocation = byteCodeBlock->GetBuffer();
        m_currentLocation = m_startLocation + startOffset;

#if DBG
        m_endLocation = m_startLocation + byteCodeBlock->GetLength();
        Assert(m_currentLocation <= m_endLocation);
#endif
    }

    template <typename T>
    AuxArray<T> const * 
    ByteCodeReader::ReadAuxArray(uint offset, FunctionBody * functionBody)
    {
        Js::AuxArray<T> const * auxArray = (Js::AuxArray<T> const *)(functionBody->GetAuxiliaryData()->GetBuffer() + offset);
        Assert(offset + auxArray->GetDataSize() <= functionBody->GetAuxiliaryData()->GetLength());
        return auxArray;    
    }

    // explict instantiations
    template AuxArray<Var> const * ByteCodeReader::ReadAuxArray<Var>(uint offset, FunctionBody * functionBody);
    template AuxArray<int32> const * ByteCodeReader::ReadAuxArray<int32>(uint offset, FunctionBody * functionBody);
    template AuxArray<uint32> const * ByteCodeReader::ReadAuxArray<uint32>(uint offset, FunctionBody * functionBody);
    template AuxArray<double> const * ByteCodeReader::ReadAuxArray<double>(uint offset, FunctionBody * functionBody);
    template AuxArray<FuncInfoEntry> const * ByteCodeReader::ReadAuxArray<FuncInfoEntry>(uint offset, FunctionBody * functionBody);

    const Js::PropertyIdArray * 
    ByteCodeReader::ReadPropertyIdArray(uint offset, FunctionBody * functionBody, uint extraSlots)
    {        
        Js::PropertyIdArray const * propIds = (Js::PropertyIdArray const *)(functionBody->GetAuxiliaryData()->GetBuffer() + offset);
        Assert(offset + propIds->GetDataSize(extraSlots) <= functionBody->GetAuxiliaryData()->GetLength());
        return propIds;        
    }

    uint32 
    VarArrayVarCount::GetDataSize() const
    {
        return sizeof(VarArrayVarCount) + sizeof(Var) * TaggedInt::ToInt32(count); 
    }

    void 
    VarArrayVarCount::SetCount(uint count) 
    { 
        this->count = Js::TaggedInt::ToVarUnchecked(count); 
    }

    const Js::VarArrayVarCount * 
    ByteCodeReader::ReadVarArrayVarCount(uint offset, FunctionBody * functionBody)
    {        
        Js::VarArrayVarCount const * varArray = (Js::VarArrayVarCount const *)(functionBody->GetAuxiliaryContextData()->GetBuffer() + offset);
        Assert(offset + varArray->GetDataSize() <= functionBody->GetAuxiliaryContextData()->GetLength());
        return varArray;        
    }   

#if DBG_DUMP
    byte ByteCodeReader::GetRawByte(int i) 
    { 
        return m_startLocation[i]; 
    }
#endif
} // namespace Js
