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
    static bool LinkListForEach(JDRemoteTyped head, PCSTR field, Fn fn)
    {
        ExtRemoteTyped headExtRemoteTyped = head.GetExtRemoteTyped();
        ExtRemoteTyped deferencedType = headExtRemoteTyped.Dereference();
        JDTypeInfo typeInfo = JDTypeInfo::FromExtRemoteTyped(deferencedType);
        ULONG64 current = GetAsPointer(headExtRemoteTyped);
        ULONG64 headPtr = current;
        ULONG offset = headExtRemoteTyped.GetFieldOffset(field);
        while (current != 0)
        {
            JDRemoteTyped object(typeInfo, current, true);
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
