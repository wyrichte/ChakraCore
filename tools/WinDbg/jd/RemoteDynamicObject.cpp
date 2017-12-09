//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"


RemoteDynamicObject::RemoteDynamicObject(RemoteRecyclableObject const& o) : RemoteRecyclableObject(o)
{

}

JDRemoteTyped RemoteDynamicObject::GetDynamicType()
{
    return GetType().Cast("Js::DynamicType");
}

class ObjectPropertyDumper : public ObjectPropertyListener
{
private:
    bool printSlotIndex;
    int m_depth;
    TypeHandlerPropertyNameReader* m_propertyNameReader;

public:
    ObjectPropertyDumper(int depth, TypeHandlerPropertyNameReader* propertyNameReader, bool printSlotIndex)
        : m_depth(depth), m_propertyNameReader(propertyNameReader), printSlotIndex(printSlotIndex)
    {
    }

    virtual void Enumerate(ExtRemoteTyped& name, LONG slot, ULONG64 value, LONG slot1, ULONG64 value1) const override
    {
        ULONG64 pName;
        if (strcmp(JDUtil::StripStructClass(name.GetTypeName()), "Js::JavascriptString *") == 0)
        {
            pName = name.Field("m_pszValue").GetPtr();
        }
        else
        {
            pName = m_propertyNameReader->GetPropertyName(name);
        }
        RemoteDynamicObject::PrintProperty(printSlotIndex, pName, slot, value, slot1, value1, m_depth);
    }

    static void Enumerate(ExtRemoteTyped& obj, RemoteTypeHandler* typeHandler, TypeHandlerPropertyNameReader* reader, int depth, bool printSlotIndex)
    {
        ObjectPropertyDumper dumper(depth, reader, printSlotIndex);
        typeHandler->EnumerateProperties(obj, dumper);
    }
};

void RemoteDynamicObject::PrintProperties(bool printSlotIndex, int depth)
{
    g_Ext->Out("[properties] ");

    JDRemoteTyped type = this->GetDynamicType();
    JDRemoteTyped typeHandler = type.Field("typeHandler");

    RemoteTypeHandler* pRemoteTypeHandler = GetExtension()->GetTypeHandler(object, typeHandler);

    if (depth == 1)
    {
        PCSTR typeHandlerName = "Unknown handler type";
        if (pRemoteTypeHandler)
        {
            typeHandlerName = pRemoteTypeHandler->GetName();
        }
        else
        {
            typeHandler = typeHandler.CastWithVtable(&typeHandlerName);
            if (typeHandlerName != nullptr)
            {
                typeHandlerName = JDUtil::StripModuleName(typeHandlerName);
            }
        }
        g_Ext->Out("%s * ", pRemoteTypeHandler ? pRemoteTypeHandler->GetName() : typeHandlerName);
        typeHandler.OutSimpleValue();
        g_Ext->Out("\n");
    }

    if (pRemoteTypeHandler)
    {
        if (GetExtension()->GetUsingPropertyRecordInTypeHandlers())
        {
            TypeHandlerPropertyRecordNameReader reader;
            ObjectPropertyDumper::Enumerate(object, pRemoteTypeHandler, &reader, depth, printSlotIndex);
        }
        else
        {
            
            TypeHandlerPropertyIdNameReader reader(this->GetScriptContext().GetThreadContext());
            ObjectPropertyDumper::Enumerate(object, pRemoteTypeHandler, &reader, depth, printSlotIndex);
        }
    }
    else
    {
        g_Ext->Out("WARNING: unknown type Handler, unable to enumerate properties\n");
    }
}

void RemoteDynamicObject::PrintProperty(bool printSlotIndex, ULONG64 name, LONG slot, ULONG64 value, LONG slot1, ULONG64 value1, int depth)
{
    // indent
    for (int i = 0; i < depth; i++)
    {
        g_Ext->Out("   ");
    }

    if (printSlotIndex)
    {
        g_Ext->Out("[%d] ", slot);
    }
    g_Ext->Out("%-12mu : ", name);
    try
    {
        RemoteVar(value).Print(printSlotIndex, depth);
        g_Ext->Out("\n");
    }
    catch (ExtException)
    {
        g_Ext->Out("%p <ERROR: Not a valid Var>\n", value);
    }

    if (value1)
    {
        PrintProperty(printSlotIndex, name, slot1, value1, -1, 0, depth);
    }
}
