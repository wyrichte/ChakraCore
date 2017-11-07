//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

// forward
class EXT_CLASS_BASE;
class ObjectPropertyListener;
class TypeHandlerPropertyNameReader;

class RemoteTypeHandler
{
protected:
    std::string m_name;

    // NOTE: Keeping ExtRemoteTyped instances around can easily cause extension to fail/AV at debugger
    // shutdown on x64. Try to avoid ~ExtRemoteTyped() being called at shutdown.
    ExtRemoteTyped* m_typeHandler;

public:
    RemoteTypeHandler(PCSTR name)
        : m_name(name)
    {
    }

    PCSTR GetName() const { return m_name.c_str(); }
    ExtRemoteTyped* GetTypeHandlerData() const { return m_typeHandler; }

    void Set(const std::string& module, ExtRemoteTyped& typeHandler);

    virtual void EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener) = 0;
};

class RemoteNullTypeHandler : public RemoteTypeHandler
{
public:
    RemoteNullTypeHandler()
        : RemoteTypeHandler("Js::NullTypeHandler")
    {
    }

    virtual void EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener) override;
};

class RemoteSimpleTypeHandler : public RemoteTypeHandler
{
public:
    RemoteSimpleTypeHandler()
        : RemoteTypeHandler("Js::SimpleTypeHandler")
    {
    }

    virtual void EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener) override;
};

class RemotePathTypeHandler : public RemoteTypeHandler
{
public:
    RemotePathTypeHandler(PCSTR name = "Js::PathTypeHandler")
        : RemoteTypeHandler(name)
    {
    }

    virtual void EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener) override;
};

class RemoteSimplePathTypeHandler : public RemotePathTypeHandler
{
public:
    RemoteSimplePathTypeHandler() :
        RemotePathTypeHandler("Js::SimplePathTypeHandler")
    {
    }
};

// Note that we don't care about the other template parameter "IsNotExtensibleSupported" here.
template <typename T>
class RemoteSimpleDictionaryTypeHandler : public RemoteTypeHandler
{
public:
    RemoteSimpleDictionaryTypeHandler(PCSTR name)
        : RemoteTypeHandler(name)
    {
    }

    virtual void EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener) override;
};

template <typename T>
class RemoteDictionaryTypeHandler : public RemoteTypeHandler
{
public:
    RemoteDictionaryTypeHandler(PCSTR name)
        : RemoteTypeHandler(name)
    {
    }

    virtual void EnumerateProperties(ExtRemoteTyped& obj, const ObjectPropertyListener& listener) override;
};

class ObjectPropertyListener
{
public:
    //
    // name: PropertyId or PropertyRecord* for later builds.
    // value: data property, or getter of accessor property
    // value1: setter of accessor property
    //
    virtual void Enumerate(ExtRemoteTyped& name, ULONG64 value, ULONG64 value1 = 0) const = 0;
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
