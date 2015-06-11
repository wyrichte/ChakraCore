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
          || strncmp(value.GetTypeName(), "class Memory::NoWriteBarrierPtr<", _countof("class Memory::NoWriteBarrierPtr<") - 1) == 0)
        {
            return value.Field("value");
        }
        return value;
    }   
};

#define ENUM_EQUAL(e, n) (strncmp(e, #n, _countof(#n) - 1) == 0)