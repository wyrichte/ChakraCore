//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#include "FieldInfoCache.h"
class JDUtil
{
public:

    static JDRemoteTyped GetWrappedField(JDRemoteTyped obj, PCSTR field)
    {
        JDRemoteTyped value = obj.Field(field);
        if (strncmp(value.GetTypeName(), "class WriteBarrierPtr<", _countof("class WriteBarrierPtr<") - 1) == 0
          || strncmp(value.GetTypeName(), "class Memory::WriteBarrierPtr<", _countof("class Memory::WriteBarrierPtr<") - 1) == 0)
        {
            return value.Field("ptr");
        }
        if (strncmp(value.GetTypeName(), "class NoWriteBarrierPtr<", _countof("class NoWriteBarrierPtr<") - 1) == 0
          || strncmp(value.GetTypeName(), "class Memory::NoWriteBarrierPtr<", _countof("class Memory::NoWriteBarrierPtr<") - 1) == 0
          || strncmp(value.GetTypeName(), "class NoWriteBarrierField<", _countof("class NoWriteBarrierField<") - 1) == 0
          || strncmp(value.GetTypeName(), "class Memory::NoWriteBarrierField<", _countof("class Memory::NoWriteBarrierField<") - 1) == 0)
        {
            return value.Field("value");
        }
        return value;
    }

    static PCSTR StripStructClass(PCSTR name)
    {
        if (strncmp(name, "struct ", 7) == 0)
        {
            return name + 7;
        }
        else if (strncmp(name, "class ", 6) == 0)
        {
            return name + 6;
        }
        return name;
    }
    static bool IsPointerType(PCSTR name)
    {
        return (strchr(name, '*') != 0);
    }
    static PCSTR StripModuleName(PCSTR name)
    {
        PCSTR n = strchr(name, '!');
        if (n == nullptr)
        {
            return name;
        }
        return n + 1;
    }

    static std::string EncodeDml(PCSTR name)
    {
        std::string s(name);
        ReplaceString(s, "<", "&lt;");
        ReplaceString(s, ">", "&gt;");
        return s;
    }

    static void ReplaceString(std::string& s, PCSTR before, PCSTR after)
    {                
        size_t beforeLen = strlen(before);
        size_t afterLen = strlen(after);
        size_t pos = s.find(before, 0);
        while (pos != std::string::npos)
        {
            s.replace(pos, beforeLen, after);
            pos += afterLen;
            pos = s.find(before, pos);
        }        
    }

    template <typename T>
    static T Align(T value, T alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }
};

#define STR_START_WITH(str, prefix) (strncmp(str, prefix, _countof(prefix) - 1) == 0)
#define ENUM_EQUAL(e, n) STR_START_WITH(e, #n)
