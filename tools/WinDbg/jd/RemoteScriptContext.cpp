//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

RemoteScriptContext::RemoteScriptContext()
{
}

RemoteScriptContext::RemoteScriptContext(ExtRemoteTyped const& scriptContext) : scriptContext(scriptContext)
{
}

RemoteThreadContext
RemoteScriptContext::GetThreadContext()
{
    return scriptContext.Field("threadContext");
}

ExtRemoteTyped RemoteScriptContext::GetHostScriptContext()
{
    return scriptContext.Field("hostScriptContext");
}

void RemoteScriptContext::PrintReferencedPids()
{
    scriptContext.OutTypeName();
    g_Ext->Out(" ");
    scriptContext.OutSimpleValue();
    g_Ext->Out("\n");

    bool isReferencedPropertyRecords = !scriptContext.HasField("referencedPropertyIds");
    ExtRemoteTyped referencedPidDictionary = isReferencedPropertyRecords ?
        scriptContext.Field("javascriptLibrary.referencedPropertyRecords") :
        scriptContext.Field("referencedPropertyIds");
    ExtRemoteTyped referencedPidDictionaryCount = referencedPidDictionary.Field("count");
    ExtRemoteTyped referencedPidDictionaryEntries = referencedPidDictionary.Field("entries");
    long pidCount = referencedPidDictionaryCount.GetLong();

    g_Ext->Out("Number of pids on scriptContext: %d\n", pidCount);

    EXT_CLASS_BASE::PropertyNameReader propertyNameReader(GetExtension(), this->GetThreadContext());
    for (int i = 0; i < pidCount; i++)
    {
        ExtRemoteTyped entry = referencedPidDictionaryEntries.ArrayElement(i);
        long pid = entry.Field(isReferencedPropertyRecords ? "value.pid" : "value").GetLong();
        ExtRemoteTyped propertyName("(char16 *)@$extin", propertyNameReader.GetNameByPropertyId(pid));
        g_Ext->Out(_u("Pid: %d "), pid);
        propertyName.OutSimpleValue();
        g_Ext->Out(_u("\n"));
    }
}


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------