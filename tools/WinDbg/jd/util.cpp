#include "stdafx.h"

#include <string>
#include <guiddef.h>

#ifdef JD_PRIVATE
EXT_CLASS_BASE* GetExtension()
{
    return (EXT_CLASS_BASE*)g_Ext.Get();
}

std::string GetSymbolForOffset(ULONG64 offset)
{
    std::string str;
    ExtBuffer<char> name;
    bool result;

    try 
    {
        result = GetExtension()->GetOffsetSymbol(offset, &name);
    } 
    catch (...) 
    {
        result = false;
    }

    if (result && name.GetEltsUsed()) 
    {
        str.assign(name.GetRawBuffer(), name.GetEltsUsed());
    }
    else 
    {
        str = "";
    }
    return str;
}

ULONG64 GetPointerAtAddress(ULONG64 offset)
{
    ExtRemoteData data(offset, g_Ext->m_PtrSize);
    ULONG64 pointer = 0;

#if _M_X64
    if (g_Ext->m_PtrSize == 4)
    {
        pointer = (ULONG64)data.GetUlong();
    }
    else
#endif
    {
        pointer = data.GetPtr();
    }

    return pointer;
}

// Adapted from WSH code
int GuidToString(GUID& guid, LPSTR strGuid, int cchStrSize) 
{
    const int GuidLength = (32 + 4 + 2 + 1);

    if (cchStrSize <= GuidLength) {
        // buffer is too small
        return 0;
    }

    ::sprintf_s(strGuid, cchStrSize, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                guid.Data1,
                guid.Data2,
                guid.Data3,
                guid.Data4[0], 
                guid.Data4[1],
                guid.Data4[2],
                guid.Data4[3],
                guid.Data4[4],
                guid.Data4[5],
                guid.Data4[6],
                guid.Data4[7] );

    return GuidLength;
}


void ReplacePlaceHolders(PCSTR holder, std::string value, std::string& cmd)
{
    if (NULL == holder)
    {
        return;
    }

    size_t size = strlen(holder);
    while (true)
    {
        std::string::size_type idx = cmd.find(holder);
        if (idx != std::string::npos)
        {
            cmd.replace(idx, size, value);
        }
        else
        {
            break;
        }
    }
}
#endif
