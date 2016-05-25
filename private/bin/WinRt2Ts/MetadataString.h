//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "Metadata.h"

class MetadataString
{
public:
    MetadataString() :
        m_id(MetadataStringIdNil)
    {
    }

    MetadataString(MetadataStringId id) :
        m_id(id)
#if DBG
        , m_dbgString(s_stringConverter.StringOfId(id))
#endif
    {
    }

    MetadataString(LPCWSTR string) :
        m_string(string)
    {
    }

    LPCWSTR ToString() const
    {
        if (m_id != MetadataStringIdNil)
        {
            return s_stringConverter.StringOfId(m_id);
        }
        else
        {
            return m_string.c_str();
        }
    }

    MetadataStringId ToId() const
    {
        if (!m_string.empty())
        {
            return s_stringConverter.IdOfString(m_string.c_str());
        }
        else
        {
            return m_id;
        }
    }

    static Metadata::IStringConverter& s_stringConverter;

private:
    MetadataStringId m_id = MetadataStringIdNil;
    std::wstring m_string;
#if DBG
    std::wstring m_dbgString;
#endif
};
