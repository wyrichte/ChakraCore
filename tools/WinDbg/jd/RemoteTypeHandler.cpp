//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

void RemoteTypeHandler::Set(const std::string& module, ExtRemoteTyped& typeHandler)
{
    // Cast typeHandler to concrete type.
    auto symbol = "(" + module + "!" + m_name + "*)@$extin";
    typeHandler.Set(symbol.c_str(), typeHandler.GetPtr());
    m_typeHandler = &typeHandler;
}

class RemoteObjectSlotReader
{
private:
    LONG m_inlineSlotCapacity;
    ExtRemoteTyped m_inlineSlots;
    ExtRemoteTyped m_auxSlots;

public:
    RemoteObjectSlotReader(RemoteTypeHandler* remoteTypeHandler, ExtRemoteTyped& obj)
    {
        ExtRemoteTyped& typeHandler = *remoteTypeHandler->GetTypeHandlerData();
        if (GetExtension()->GetUsingInlineSlots(typeHandler))
        {
            // inlineSlotCapacity was changed from int to uint16, try to support both value type
            ExtRemoteTyped inlineSlotCapacity = typeHandler.Field("inlineSlotCapacity");
            m_inlineSlotCapacity = static_cast<LONG>(inlineSlotCapacity.GetData(inlineSlotCapacity.GetTypeSize()));

            m_inlineSlots.Set("(void**)@$extin", obj.GetPtr() + typeHandler.Field("offsetOfInlineSlots").GetUshort());
            m_auxSlots = obj.Field("auxSlots");
        }
        else
        {
            m_inlineSlotCapacity = 0;
            m_auxSlots = obj.Field("slots");
        }
    }

    ULONG64 GetSlot(LONG i)
    {
        return i < m_inlineSlotCapacity ? m_inlineSlots[i].GetPtr() : m_auxSlots[i - m_inlineSlotCapacity].GetPtr();
    }
};

void RemoteNullTypeHandler::EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener)
{
    // No properties, do nothing
}

void RemoteSimpleTypeHandler::EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener)
{
    RemoteObjectSlotReader slotReader(this, obj);

    LONG propertyCount = m_typeHandler->Field("propertyCount").GetLong();
    for (LONG i = 0; i < propertyCount; i++)
    {
        auto descriptor = m_typeHandler->Field("descriptors")[i];
        auto attr = descriptor.Field("Attributes").GetUchar();
        if (!(attr & PropertyDeleted))
        {
            auto name = descriptor.Field("Id");
            ULONG64 value = slotReader.GetSlot(i);
            listener.Enumerate(name, value);
        }
    }
}

void RemotePathTypeHandler::EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener)
{
    RemoteObjectSlotReader slotReader(this, obj);

    ExtRemoteTyped typePath = m_typeHandler->Field("typePath");
    ExtRemoteTyped assignments = typePath.Field("assignments");

    LONG pathLength;
    if (m_typeHandler->HasField("pathLength"))
    {
        // Old style pathLength (before C_u(#1160359))
        pathLength = m_typeHandler->Field("pathLength").GetLong();
    }
    else if (m_typeHandler->GetFieldOffset("unusedBytes") > m_typeHandler->GetFieldOffset("inlineSlotCapacity"))
    {
        // Old style of unused byte without the tag bit (before C_u(#1329022))
        pathLength = m_typeHandler->Field("unusedBytes").GetUshort();
    }
    else
    {
        // the unused bytes are tagged
        pathLength = (m_typeHandler->Field("unusedBytes").GetUshort() >> 1);
    }

    for (LONG i = 0; i < pathLength; i++)
    {
        auto name = assignments[i];
        ULONG64 value = slotReader.GetSlot(i);
        listener.Enumerate(name, value);
    }
}

template <typename T>
void RemoteSimpleDictionaryTypeHandler<T>::EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener)
{
    RemoteObjectSlotReader slotReader(this, obj);

    auto propertyMap = m_typeHandler->Field("propertyMap");
    auto entries = propertyMap.Field("entries");

    LONG count = propertyMap.Field("count").GetLong() - propertyMap.Field("freeCount").GetLong();
    for (LONG i = 0; i < count; i++)
    {
        auto entry = entries[i];
        
        auto descriptor = entry.Field("value");
        auto attr = descriptor.Field("Attributes").GetUchar();
        if (!(attr & PropertyDeleted))
        {
            auto name = entry.Field("key");
            LONG slot = (LONG)(descriptor.Field("propertyIndex").GetData(sizeof(T)));
            ULONG64 value = slotReader.GetSlot(slot);
            listener.Enumerate(name, value);
        }
    }
}

// Specialize on USHORT and INT
template class RemoteSimpleDictionaryTypeHandler<USHORT>;
template class RemoteSimpleDictionaryTypeHandler<INT>;

template <typename T>
void RemoteDictionaryTypeHandler<T>::EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener)
{
    RemoteObjectSlotReader slotReader(this, obj);

    auto propertyMap = m_typeHandler->Field("propertyMap");
    auto entries = propertyMap.Field("entries");

    LONG count = propertyMap.Field("count").GetLong() - propertyMap.Field("freeCount").GetLong();
    for (LONG i = 0; i < count; i++)
    {
        auto entry = entries[i];
        
        auto descriptor = entry.Field("value");
        auto attr = descriptor.Field("Attributes").GetUchar();
        if (!(attr & PropertyDeleted))
        {
            auto name = entry.Field("key");

            T data = (T)(descriptor.Field("Data").GetData(sizeof(T)));
            if (data != (T)-1)
            {
                listener.Enumerate(name, slotReader.GetSlot(data));
            }
            else
            {
                T getter = (T)(descriptor.Field("Getter").GetData(sizeof(T)));
                T setter = (T)(descriptor.Field("Setter").GetData(sizeof(T)));
                listener.Enumerate(name, slotReader.GetSlot(getter), slotReader.GetSlot(setter));
            }
        }
    }
}

// Specialize on USHORT
template class RemoteDictionaryTypeHandler<USHORT>;

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
