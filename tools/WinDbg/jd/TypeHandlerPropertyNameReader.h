//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class TypeHandlerPropertyNameReader
{
public:
    virtual ULONG64 GetPropertyName(ExtRemoteTyped& key) = 0;
};

class TypeHandlerPropertyRecordNameReader : public TypeHandlerPropertyNameReader
{
public:
    ULONG64 GetPropertyName(ExtRemoteTyped& key) override
    {
        return key[1UL].GetPointerTo().GetPtr();
    }
};

class TypeHandlerPropertyIdNameReader : public TypeHandlerPropertyNameReader
{
private:
    EXT_CLASS_BASE::PropertyNameReader m_reader;

public:
    TypeHandlerPropertyIdNameReader(RemoteThreadContext threadContext)
        : m_reader(threadContext)
    {
    }

    ULONG64 GetPropertyName(ExtRemoteTyped& key) override
    {
        return m_reader.GetNameByPropertyId(key.GetUlong());
    }
};
