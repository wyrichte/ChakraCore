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

RemoteScriptContext::RemoteScriptContext(ULONG64 scriptContext) :
    scriptContext(GetExtension()->FillModule("(%s!Js::ScriptContext *)@$extin"), scriptContext)
{

}

RemoteScriptContext::RemoteScriptContext(ExtRemoteTyped const& scriptContext) : scriptContext(scriptContext)
{
}

ULONG64
RemoteScriptContext::GetPtr()
{
    return scriptContext.GetPtr();
}

RemoteThreadContext
RemoteScriptContext::GetThreadContext()
{
    return scriptContext.Field("threadContext");
}

JDRemoteTyped RemoteScriptContext::GetHostScriptContext()
{
    JDRemoteTyped hostScriptContext = scriptContext.Field("hostScriptContext");
    return hostScriptContext.CastWithVtable();
}

bool RemoteScriptContext::IsClosed()
{
    return scriptContext.Field("isClosed").GetStdBool();
}

bool RemoteScriptContext::IsScriptContextActuallyClosed()
{
    return scriptContext.Field("isScriptContextActuallyClosed").GetStdBool();
}

JDRemoteTyped RemoteScriptContext::GetSourceList()
{
    if (scriptContext.HasField("sourceList"))
    {
        return scriptContext.Field("sourceList").Field("ptr");
    }
    return JDRemoteTyped("(void *)0");
}

JDRemoteTyped RemoteScriptContext::GetUrl()
{
    if (scriptContext.HasField("url"))
    {
        return scriptContext.Field("url");
    }

    ExtRemoteTyped omWindowProxy;
    ExtRemoteTyped globalObject = scriptContext.Field("globalObject");
    ExtRemoteTyped directHostObject = globalObject.Field("directHostObject");

    try
    {
        if (directHostObject.GetPtr()) {
            // IE9 mode
            ExtRemoteTyped customExternalObject("(Js::CustomExternalObject*)@$extin", directHostObject.GetPtr());
            omWindowProxy = ExtRemoteTyped("(mshtml!COmWindowProxy**)@$extin",
                customExternalObject[1UL].GetPointerTo().GetPtr()).Dereference();
        }
        else {
            // IE8 mode
            ExtRemoteTyped hostObject("(HostObject*)@$extin", globalObject.Field("hostObject").GetPtr());

            ExtRemoteTyped tearoffThunk("(mshtml!TEAROFF_THUNK*)@$extin",
                hostObject.Field("hostDispatch").Field("refCountedHostVariant").Field("hostVariant").Field("varDispatch").Field("pdispVal").GetPtr());
            omWindowProxy = ExtRemoteTyped("(mshtml!COmWindowProxy*)@$extin",
                tearoffThunk.Field("pvObject1").GetPtr());
        }

        return omWindowProxy.Field("_pCWindow").Field("_pMarkup").Field("_pHtmCtx").Field("_pDwnInfo").Field("_cusUri").Field("m_LPWSTRProperty");
    }
    catch (...)
    {

    }

    return JDRemoteTyped("(void *)0");
}

void RemoteScriptContext::PrintReferencedPids()
{
    scriptContext.OutTypeName();
    g_Ext->Out(" ");
    scriptContext.OutSimpleValue();
    g_Ext->Out("\n");

    bool isReferencedPropertyRecords = !scriptContext.HasField("referencedPropertyIds");
    ExtRemoteTyped referencedPidDictionary = isReferencedPropertyRecords ?
        scriptContext.Field("javascriptLibrary").Field("referencedPropertyRecords") :
        scriptContext.Field("referencedPropertyIds");
    ExtRemoteTyped referencedPidDictionaryCount = referencedPidDictionary.Field("count");
    ExtRemoteTyped referencedPidDictionaryEntries = referencedPidDictionary.Field("entries");
    long pidCount = referencedPidDictionaryCount.GetLong();

    g_Ext->Out("Number of pids on scriptContext: %d\n", pidCount);

    EXT_CLASS_BASE::PropertyNameReader propertyNameReader(this->GetThreadContext());
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

bool RemoteScriptContext::TryGetScriptContextFromPointer(ULONG64 pointer, RemoteScriptContext& remoteScriptContext)
{   
    return RemoteThreadContext::ForEach([&](RemoteThreadContext threadContext)
    {
        return threadContext.ForEachScriptContext([&](RemoteScriptContext scriptContext)
        {
            if (scriptContext.GetPtr() == pointer)
            {
                remoteScriptContext = scriptContext;
                return true;
            }
            return false;
        });
    });
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------