//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

RemoteRecyclableObject::RemoteRecyclableObject() : typeName(nullptr)
{}

RemoteRecyclableObject::RemoteRecyclableObject(ULONG64 address)
{    
    object = JDRemoteTyped::FromPtrWithVtable(address, &typeName);
    if (typeName == nullptr)
    {
        g_Ext->ThrowLastError("Pointer doesn't have a valid vtable");
    }
}

RemoteRecyclableObject::RemoteRecyclableObject(ExtRemoteTyped const& o) : object(o), typeName(nullptr)
{}

ULONG64
RemoteRecyclableObject::GetPtr()
{
    return object.GetPtr();
}

char const *
RemoteRecyclableObject::GetTypeName()
{
    if (typeName == nullptr)
    {
        object = JDRemoteTyped::FromPtrWithVtable(GetPtr(), &typeName);
        if (typeName == nullptr)
        {
            g_Ext->ThrowLastError("Pointer doesn't have a valid vtable");
        }
    }
    return typeName;
}

JDRemoteTyped
RemoteRecyclableObject::GetType()
{
    return object.Field("type");
}

RemoteScriptContext
RemoteRecyclableObject::GetScriptContext()
{
    JDRemoteTyped type = GetType();
    if (type.HasField("javascriptLibrary"))
    {
        return type.Field("javascriptLibrary").Field("scriptContext");
    }
    return type.Field("globalObject").Field("scriptContext");
}

char const * 
RemoteRecyclableObject::GetTypeIdEnumString()
{
    return JDUtil::GetEnumString(GetType().Field("typeId"));
}

bool
RemoteRecyclableObject::IsUndefined()
{
    return strcmp(GetTypeIdEnumString(), "TypeIds_Undefined") == 0;
}

bool
RemoteRecyclableObject::IsNull()
{
    return strcmp(GetTypeIdEnumString(), "TypeIds_Null") == 0;
}

bool
RemoteRecyclableObject::IsJavascriptFunction()
{
    return strcmp(GetTypeIdEnumString(), "TypeIds_Function") == 0;
}

RemoteJavascriptFunction
RemoteRecyclableObject::AsJavascriptFunction()
{
    if (!IsJavascriptFunction())
    {
        return RemoteJavascriptFunction();
    }
    return GetPtr();
}

void
RemoteRecyclableObject::PrintSimpleVarValue()
{
    char const * typeName = GetTypeName();  
    if (GetExtension()->PreferDML())
    {
        std::string encodedTypeName = JDUtil::EncodeDml(typeName);
        g_Ext->Dml("%s * <link cmd=\"!jd.var 0x%p\">0x%p</link> (%s)", JDUtil::StripModuleName(encodedTypeName.c_str()), this->GetPtr(),
            this->GetPtr(), this->GetTypeIdEnumString());
    }
    else
    {
        g_Ext->Out("%s * 0x%p (%s) /*\"!jd.var 0x%p\" to display*/", JDUtil::StripModuleName(typeName), 
            this->GetPtr(), this->GetTypeIdEnumString(), this->GetPtr());
    }
}

void
RemoteRecyclableObject::PrintLink(char const * link)
{
    RemoteVar(this->GetPtr()).PrintLink(link);
}

void
RemoteRecyclableObject::PrintPrimitiveValue(char const * value, int depth)
{
    if (depth == 0)
    {
        g_Ext->Out("%s (0x%p)", value, this->GetPtr());
    }
    else
    {
        PrintLink(value);
    }
}

void
RemoteRecyclableObject::Print(bool printSlotIndex, int depth)
{
    const char * typeName = this->GetTypeName();
    const char * typeIdStr = GetTypeIdEnumString();
    if (depth == 0)
    {
        if (GetExtension()->PreferDML())
        {
            std::string encodedClassName = JDUtil::EncodeDml(typeName);
            g_Ext->Dml("%s * <link cmd=\"dt %s 0x%p\">0x%p</link> (%s)", JDUtil::StripModuleName(encodedClassName.c_str()),
                encodedClassName.c_str(), this->GetPtr(), this->GetPtr(), typeIdStr);
        }
        else
        {
            g_Ext->Out("%s * 0x%p (%s) /*\"dt %s 0x%p\" to display*/", JDUtil::StripModuleName(typeName),
                this->GetPtr(), typeIdStr, typeName, this->GetPtr());
        }
        this->DumpPossibleExternalSymbol(typeName);
        g_Ext->Out("\n");
    }

    if (strcmp(typeIdStr, "TypeIds_Undefined") == 0)
    {
        PrintPrimitiveValue("<undefined>", depth);
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_Null") == 0)
    {
        PrintPrimitiveValue("<null>", depth);    
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_Boolean") == 0)
    {
        PrintPrimitiveValue(object.Field("value").GetW32Bool() ? "<true>" : "<false>", depth);
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_Number") == 0)
    {
        object.Field("m_value").OutFullValue();
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_String") == 0)
    {
        if (GetExtension()->PreferDML())
        {
            g_Ext->Dml("<link cmd=\"!jd.var 0x%p\">\"%mu\"</link>", this->GetPtr(), object.Field("m_pszValue").GetPtr());
        }
        else
        {
            g_Ext->Out("\"%mu\" (%p)", object.Field("m_pszValue").GetPtr(), this->GetPtr());
        }
        return; // done
    }
    else if (depth == 0)
    {
        if (strcmp(typeIdStr, "TypeIds_StringObject") == 0)
        {
            RemoteRecyclableObject value = object.Field("value");
            value.PrintSimpleVarValue();
            g_Ext->Out("\n");
        }
        else if (strcmp(typeIdStr, "TypeIds_Function") == 0)
        {
            this->AsJavascriptFunction().Print();
        }
        else if (strcmp(typeIdStr, "TypeIds_Array") == 0)
        {
            object.Field("head").OutFullValue();
        }
    }
    else
    {
        this->PrintSimpleVarValue();
    }

    if (depth == 0)
    {
        RemoteRecyclableObject prototype = GetType().Field("prototype");
        g_Ext->Out("\n[prototype] ");
        prototype.PrintSimpleVarValue();
        g_Ext->Out("\n");
        RemoteDynamicObject(*this).PrintProperties(printSlotIndex, depth + 1);
    }
}


bool RemoteRecyclableObject::DumpPossibleExternalSymbol(char const * typeName, bool makeLink, bool showScriptContext)
{
    if (strstr(typeName, "ArrayObjectInstance") != 0 ||
        strstr(typeName, "Js::CustomExternalObject") != 0)
    {
        ULONG64 offsetOfExternalObject = 0x18;

        if (g_Ext->m_PtrSize == 8)
        {
            offsetOfExternalObject = 0x30;
        }

        ULONG64 externalObject = object.GetPtr() + offsetOfExternalObject;
        ULONG64 domObject = GetPointerAtAddress(externalObject);
        if (domObject != NULL)
        {            
            try
            {
                // Untag the last two bits.
                ULONG64 untaggedDOMObject = domObject & ~0x3;
                ULONG64 domVtable = GetPointerAtAddress(untaggedDOMObject);
                std::string symbol = GetSymbolForOffset(domVtable);

                if (!symbol.empty())
                {
                    g_Ext->Out(" (DOM item %s)", symbol.c_str());
                }
                else
                {
                    g_Ext->Out(" (0x%p)", externalObject);
                }
            }
            catch (ExtException ex)
            {
                g_Ext->Err(" (fail to deref 0x%p, Error: %s)", domObject, ex.GetMessageW());
            }
        }
        return true;
    }
    if (strstr(typeName, "JavascriptDispatch") != 0)
    {
        ExtRemoteTyped scriptObject = object.Field("scriptObject");
        ULONG64 scriptObjectPointer = scriptObject.GetPtr();
        // scriptObject can be null if the ScriptEngine has been closed, so check for this scenario.
        if (scriptObjectPointer)
        {
            g_Ext->Out(" [ScriptObject");
            if (!GetExtension()->DumpPossibleSymbol(scriptObjectPointer, makeLink, showScriptContext))
            {
                g_Ext->Out(" = 0x%p", scriptObjectPointer);
            }
            g_Ext->Out("]");
        }
        return true;
    }
    return false;
}