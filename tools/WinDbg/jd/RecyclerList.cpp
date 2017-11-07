#include "stdafx.h"

#include "RemoteRecyclerList.h"

#ifdef JD_PRIVATE
template <bool isSlist>
void DumpList(ULONG64 address, PCSTR type)
{
    RemoteListIterator<isSlist> iterator(type, address);

    uint count = 0;
    while (iterator.Next())
    {
        ULONG64 data = iterator.GetDataPtr();
        if (GetExtension()->PreferDML())
        {
            GetExtension()->Dml("<link cmd=\"?? (%s*) 0x%p\">0x%p</link>\n", type, data, data);
        }
        else
        {
            GetExtension()->Out("0x%p /*\"?? (%s*) 0x%p\" to display*/\n", data, type, data);
        }
        count++;
    }

    GetExtension()->Out("Count is %d\n", count);
}

JD_PRIVATE_COMMAND(slist,
    "Dumps an slist",
    "{;e,r;address;Address of the slist}{;x;r;type;Type of object in the SList}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    PCSTR type = GetUnnamedArgStr(1);

    DumpList<true>(arg, type);
}

JD_PRIVATE_COMMAND(dlist,
    "Dumps an dlist",
    "{;e,r;address;Address of the dlist}{;x;r;type;Type of object in the SList}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    PCSTR type = GetUnnamedArgStr(1);

    DumpList<false>(arg, type);
}

#endif
