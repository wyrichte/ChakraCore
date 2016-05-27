//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class ExtRemoteTypedUtil
{
public:
    static ULONG64 GetAsPointer(ExtRemoteTyped object);
    static ULONG64 Count(ExtRemoteTyped head, PCSTR field);
    static ULONG64 TaggedCount(ExtRemoteTyped head, PCSTR field);
    static ULONG64 GetSizeT(ExtRemoteTyped data);
    static ExtRemoteTyped GetTeb();

    template <bool tagged = false, typename Fn>
    static bool LinkListForEach(ExtRemoteTyped head, PCSTR field, Fn fn)
    {
        ExtRemoteTyped deferencedType = head.Dereference();
        ULONG64 current = GetAsPointer(head);
        ULONG64 headPtr = current;
        ULONG offset = head.GetFieldOffset(field);
        while (current != 0)
        {
            JDRemoteTyped object(deferencedType.m_Typed.ModBase, deferencedType.m_Typed.TypeId, current, true);
            if (fn(object))
            {
                return true;
            }
            ExtRemoteData data(current + offset, g_Ext->m_PtrSize);
            current = data.GetPtr() & ~(ULONG64)tagged;
            if (current == headPtr)   // circular
            {
                break;
            }
        }
        return false;
    }

    template <typename Fn>
    static bool LinkListForEachTagged(ExtRemoteTyped head, PCSTR field, Fn fn)
    {
        return LinkListForEach<true>(head, field, fn);
    }
};
