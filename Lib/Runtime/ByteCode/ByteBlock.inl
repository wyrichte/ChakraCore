//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline uint ByteBlock::GetLength() const
    {
        return m_contentSize;
    }

    inline const byte* ByteBlock::GetBuffer() const
    {
        return m_content;
    }

    inline byte* ByteBlock::GetBuffer()
    {
        return m_content;
    }

    inline const byte ByteBlock::operator[](uint itemIndex) const
    {
        AssertMsg(itemIndex < m_contentSize, "Ensure valid offset");

        return m_content[itemIndex];
    }

    inline byte& ByteBlock::operator[] (uint itemIndex)
    {
        AssertMsg(itemIndex < m_contentSize, "Ensure valid offset");

        return m_content[itemIndex];
    }

} // namespace Js
