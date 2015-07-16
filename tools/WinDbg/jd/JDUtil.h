//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class JDUtil
{
public:
    
    static char * GetEnumString(ExtRemoteTyped value)
    {
        // Trim out the enum numeric value
        char * valueStr = value.GetSimpleValue();
        char * endValueStr = strchr(valueStr, '(');
        if (endValueStr != null)
        {
            *(endValueStr - 1) = null;
        }
        return valueStr;
    }

    static ExtRemoteTyped GetWrappedField(ExtRemoteTyped obj, PCSTR field)        
    {
        ExtRemoteTyped value = obj.Field(field);
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
};

#define ENUM_EQUAL(e, n) (strncmp(e, #n, _countof(#n) - 1) == 0)