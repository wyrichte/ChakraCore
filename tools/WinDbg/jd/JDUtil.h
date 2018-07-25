//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
class JDUtil
{
public:

    static JDRemoteTyped GetWrappedField(JDRemoteTyped obj, PCSTR field)
    {
        JDRemoteTyped value = obj.Field(field);
        char const * simpleTypeName = value.GetSimpleTypeName();
        if (strncmp(simpleTypeName, "WriteBarrierPtr<", _countof("WriteBarrierPtr<") - 1) == 0
          || strncmp(simpleTypeName, "Memory::WriteBarrierPtr<", _countof("Memory::WriteBarrierPtr<") - 1) == 0)
        {
            return value.Field("ptr");
        }
        if (strncmp(simpleTypeName, "NoWriteBarrierPtr<", _countof("NoWriteBarrierPtr<") - 1) == 0
          || strncmp(simpleTypeName, "Memory::NoWriteBarrierPtr<", _countof("Memory::NoWriteBarrierPtr<") - 1) == 0
          || strncmp(simpleTypeName, "NoWriteBarrierField<", _countof("NoWriteBarrierField<") - 1) == 0
          || strncmp(simpleTypeName, "Memory::NoWriteBarrierField<", _countof("Memory::NoWriteBarrierField<") - 1) == 0)
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
