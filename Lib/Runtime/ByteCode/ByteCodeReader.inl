//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    
    template<typename LayoutType>
    inline const unaligned LayoutType * ByteCodeReader::GetLayout()
    {
        size_t layoutSize = sizeof(LayoutType);
        
        AssertMsg((layoutSize > 0) && (layoutSize < 100), "Ensure valid layout size");

        const byte * layoutData = m_currentLocation;
        m_currentLocation += layoutSize;

        Assert(m_currentLocation <= m_endLocation);

        return reinterpret_cast<const unaligned LayoutType *>(layoutData);
    }

    template<typename LayoutType>
    inline const unaligned LayoutType * ByteCodeReader::GetLayout(const byte*& ip)
    {
        size_t layoutSize = sizeof(LayoutType);
        
        AssertMsg((layoutSize > 0) && (layoutSize < 100), "Ensure valid layout size");

        const byte * layoutData = ip;
        ip += layoutSize;
        m_currentLocation = ip;

        Assert(m_currentLocation <= m_endLocation);

        return reinterpret_cast<const unaligned LayoutType *>(layoutData);
    }
    
    template<>
    inline const unaligned OpLayoutEmpty * ByteCodeReader::GetLayout<OpLayoutEmpty>()
    {
        return null;
    }

    template<>
    inline const unaligned OpLayoutEmpty * ByteCodeReader::GetLayout<OpLayoutEmpty>(const byte*& ip)
    {
        m_currentLocation = ip;        
        return null;
    }

    inline OpCode ByteCodeReader::ReadOp(const byte *&ip, LayoutSize& layoutSize) const
    {
        // Return current location and advance past data.
        
        Assert(ip < m_endLocation);
        OpCode op = (OpCode)*ip++;
        
        if (!OpCodeUtil::IsPrefixOpcode(op))
        {
            layoutSize = SmallLayout;
            return op;
        }
                               
        return ReadPrefixedOp(ip, layoutSize, op);
    }

    inline OpCode ByteCodeReader::ReadPrefixedOp(const byte *&ip, LayoutSize& layoutSize, OpCode prefix) const
    {
        Assert(ip < m_endLocation);
        OpCode op = (OpCode)*ip++;       
        switch (prefix)
        {
        case Js::OpCode::MediumLayoutPrefix:
            layoutSize = MediumLayout;
            return op;
        case Js::OpCode::LargeLayoutPrefix:        
            layoutSize = LargeLayout;
            return op;
        case Js::OpCode::ExtendedOpcodePrefix:
            layoutSize = SmallLayout;
            break;
        case Js::OpCode::ExtendedMediumLayoutPrefix:
            layoutSize = MediumLayout;
            break;
        default:
            Assert(prefix == Js::OpCode::ExtendedLargeLayoutPrefix);
            layoutSize = LargeLayout;
        };
        return (OpCode)(op + (Js::OpCode::ExtendedOpcodePrefix << 8));
    }

    inline OpCode ByteCodeReader::ReadOp(LayoutSize& layoutSize)
    {
        return ReadOp(m_currentLocation, layoutSize);       
    }

    inline OpCode ByteCodeReader::ReadPrefixedOp(LayoutSize& layoutSize, OpCode prefix)
    {
        Assert(OpCodeUtil::IsPrefixOpcode(prefix));
        return ReadPrefixedOp(m_currentLocation, layoutSize, prefix);
    }
    inline OpCode ByteCodeReader::PeekOp(LayoutSize& layoutSize) const
    {
        const byte * ip = m_currentLocation;
        return ReadOp(ip, layoutSize);        
    }
   
    inline OpCode ByteCodeReader::PeekOp(const byte * ip, LayoutSize& layoutSize)
    {
        return ReadOp(ip, layoutSize);
    }

    inline OpCode ByteCodeReader::ReadByteOp(const byte*& ip)
    {
        return (OpCode)*ip++;
    }

    inline OpCode ByteCodeReader::PeekByteOp(const byte * ip)
    {
        return (OpCode)*ip;        
    }

    inline const byte* ByteCodeReader::GetIP()
    {
        return m_currentLocation;
    }

    inline void ByteCodeReader::SetIP(const byte *const ip)
    {
        Assert(ip >= m_startLocation);
        Assert(ip < m_endLocation);

        m_currentLocation = ip;
    }

    // Define reading functions
#define LAYOUT_TYPE(layout) \
    inline const unaligned OpLayout##layout * ByteCodeReader::layout() \
    { \
        return GetLayout<OpLayout##layout>(); \
    } \
    inline const unaligned OpLayout##layout * ByteCodeReader::layout(const byte*& ip) \
    { \
        return GetLayout<OpLayout##layout>(ip); \
    }
#include "LayoutTypes.h"
// Define reading functions
#define LAYOUT_TYPE(layout) \
    inline const unaligned OpLayout##layout * ByteCodeReader::layout() \
    { \
        return GetLayout<OpLayout##layout>(); \
    } \
    inline const unaligned OpLayout##layout * ByteCodeReader::layout(const byte*& ip) \
    { \
        return GetLayout<OpLayout##layout>(ip); \
    }
#define EXCLUDE_DUP_LAYOUT
#include "LayoutTypesAsmJs.h"

    inline uint ByteCodeReader::GetCurrentOffset() const
    {
        Assert(m_currentLocation >= m_startLocation);
        Assert(m_currentLocation - m_startLocation <= UINT_MAX);
        return (uint) (m_currentLocation - m_startLocation);
    }
    
    inline const byte * ByteCodeReader::SetCurrentOffset(int byteOffset)
    {
        const byte * ip = m_startLocation + byteOffset;
        Assert(ip < m_endLocation);
        m_currentLocation = ip;        
        return ip;
    }

    inline const byte * ByteCodeReader::SetCurrentRelativeOffset(const byte * ip, int byteOffset)
    {
        Assert(ip < m_endLocation);
        const byte * targetip = ip + byteOffset;
        Assert(targetip < m_endLocation);
        m_currentLocation = targetip;        
        return targetip;
    }
} // namespace Js
