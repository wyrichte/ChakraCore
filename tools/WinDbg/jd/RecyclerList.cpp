#include "stdafx.h"

#include "RemoteRecyclerList.h"

#ifdef JD_PRIVATE
template <bool isSlist>
void DumpList(EXT_CLASS_BASE* ext, ULONG64 address, PCSTR type)
{
    RemoteListIterator<isSlist> iterator(type, address);

    uint count = 0;
    while (iterator.Next())
    {
        ULONG64 data = iterator.GetDataPtr();
        ext->Dml("<link cmd=\"?? (%s*) 0x%p\">0x%p</link>\n", type, data, data);
        count++;
    }

    ext->Out("Count is %d\n", count);
}

JD_PRIVATE_COMMAND(slist,
    "Dumps an slist",
    "{;e,r;address;Address of the slist}{;x;r;type;Type of object in the SList}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    PCSTR type = GetUnnamedArgStr(1);

    DumpList<true>(this, arg, type);
}

JD_PRIVATE_COMMAND(dlist,
    "Dumps an dlist",
    "{;e,r;address;Address of the dlist}{;x;r;type;Type of object in the SList}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    PCSTR type = GetUnnamedArgStr(1);

    DumpList<false>(this, arg, type);
}

#endif
