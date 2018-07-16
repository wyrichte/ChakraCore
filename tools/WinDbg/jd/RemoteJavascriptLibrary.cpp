//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteJavascriptLibrary.h"

RemoteJavascriptLibrary::RemoteJavascriptLibrary(ULONG64 address) :
    javascriptLibrary(GetExtension()->FillModule("(%s!Js::JavascriptLibrary *)@$extin"), address)
{
    
}

RemoteScriptContext RemoteJavascriptLibrary::GetScriptContext()
{
    if (IsInternal())
    {
        return RemoteScriptContext();
    }
    return javascriptLibrary.Field("scriptContext");
}

bool RemoteJavascriptLibrary::IsInternal()
{
    ULONG64 library = javascriptLibrary.GetPtr();
    return library == 0 || library == RemoteJavascriptLibrary::GlobalLibrary;
}

bool
RemoteJavascriptLibrary::IsClosed()
{
    if (IsInternal())
    {
        return false;
    }

    return this->GetScriptContext().IsClosed();
}

void
RemoteJavascriptLibrary::PrintStateAndLink(bool showNull)
{    
    if (!IsInternal())
    {
        this->GetScriptContext().PrintState();
    }
    else
    {
        g_Ext->Out("  ");
    }
    g_Ext->Out(" ");
    PrintLink(showNull);
}

void
RemoteJavascriptLibrary::PrintLink(bool showNull)
{
    ULONG64 library = javascriptLibrary.GetPtr();
    if (showNull || library != 0)
    {
        if (!this->IsInternal() && GetExtension()->PreferDML())
        {
            GetExtension()->Dml("<link cmd=\"!jd.url -a 0x%p\">0x%p</link>", library, library);
        }
        else
        {
            GetExtension()->Out("0x%p", library);
        }
    }
    else
    {
        GetExtension()->Out(GetExtension()->m_PtrSize == 4 ? "%10s" : "%18s", "");
    }
}