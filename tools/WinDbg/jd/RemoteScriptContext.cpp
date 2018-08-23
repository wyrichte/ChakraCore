//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"

RemoteScriptContext::RemoteScriptContext()
{
}

RemoteScriptContext::RemoteScriptContext(ULONG64 scriptContext) :
    scriptContext(GetExtension()->FillModule("(%s!Js::ScriptContext *)@$extin"), scriptContext)
{

}

RemoteScriptContext::RemoteScriptContext(JDRemoteTyped const& scriptContext) : scriptContext(scriptContext)
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
    return JDRemoteTyped::NullPtr();
}

JDRemoteTyped RemoteScriptContext::GetUrl()
{
    if (scriptContext.HasField("url"))
    {
        return scriptContext.Field("url");
    }

    ExtRemoteTyped omWindowProxy;
    ExtRemoteTyped globalObject = scriptContext.Field("globalObject").GetExtRemoteTyped();
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

    return JDRemoteTyped::NullPtr();
}

bool RemoteScriptContext::HasDebugContextField()
{
    return scriptContext.HasField("debugContext");
}

JDRemoteTyped RemoteScriptContext::GetDebugContext()
{
    return scriptContext.Field("debugContext");
}

JDRemoteTyped RemoteScriptContext::GetDebuggerMode()
{
    if (this->HasDebugContextField())
    {
        return this->GetDebugContext().Field("debuggerMode");
    }
    return scriptContext.Field("debuggerMode");
}


void RemoteScriptContext::PrintReferencedPids()
{
    scriptContext.GetExtRemoteTyped().OutTypeName();
    g_Ext->Out(" ");
    scriptContext.GetExtRemoteTyped().OutSimpleValue();
    g_Ext->Out("\n");

    bool isReferencedPropertyRecords = !scriptContext.HasField("referencedPropertyIds");
    ExtRemoteTyped referencedPidDictionary = isReferencedPropertyRecords ?
        scriptContext.Field("javascriptLibrary").Field("referencedPropertyRecords").GetExtRemoteTyped() :
        scriptContext.Field("referencedPropertyIds").GetExtRemoteTyped();
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

bool RemoteScriptContext::IsPrimaryEngine(bool * hasError)
{
    if (hasError)
    {
        *hasError = false;
    }
    try
    {
        bool fPrimaryEngine;
        JDRemoteTyped fNonPrimaryEngine = this->GetHostScriptContext().Field("scriptSite").Field("scriptEngine").Field("fNonPrimaryEngine");
        if (strcmp(fNonPrimaryEngine.GetTypeName(), "int") == 0)
        {
            fPrimaryEngine = strcmp(fNonPrimaryEngine.GetSimpleValue(), "0n0") == 0;
        }
        else
        {
            fPrimaryEngine = fNonPrimaryEngine.GetStdBool() == false;
        }
        return fPrimaryEngine;
    }
    catch(...)
    {
        *hasError = true;
    }
    return false;
}

void RemoteScriptContext::PrintState()
{
    if (this->IsScriptContextActuallyClosed())
    {
        g_Ext->Out(" C");
    }
    else if (this->IsClosed())
    {
        g_Ext->Out(" M");
    }
    else
    {
        char const * debuggerMode = this->GetDebuggerMode().GetEnumString();

        if (strcmp(debuggerMode, "SourceRundown") == 0)
        {
            g_Ext->Out("S");
        }
        else if (strcmp(debuggerMode, "Debugging") == 0)
        {
            g_Ext->Out("D");
        }
        else
        {
            g_Ext->Out(" ");
        }
        JDRemoteTyped hostScriptContext = this->GetHostScriptContext();
        if (hostScriptContext.GetPtr() && hostScriptContext.HasField("scriptSite"))
        {
            bool hasError;
            bool fPrimaryEngine = this->IsPrimaryEngine(&hasError);
            g_Ext->Out(hasError ? "E" : fPrimaryEngine ? "P" : "N");
        }
        else
        {
            g_Ext->Out(" ");
        }
    }
}

bool RemoteScriptContext::TryGetScriptContextFromPointer(ULONG64 pointer, RemoteScriptContext& remoteScriptContext)
{   
    return RemoteThreadContext::ForEach([&](RemoteThreadContext threadContext)
    {
        return threadContext.ForEachScriptContext([&](RemoteScriptContext scriptContext)
        {
            if (scriptContext.GetPtr() == pointer || scriptContext.GetJavascriptLibrary().GetPtr() == pointer)
            {
                remoteScriptContext = scriptContext;
                return true;
            }
            return false;
        });
    });
}
