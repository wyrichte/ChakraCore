//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"

bool 
RemoteUtf8SourceInfo::TryGetUtf8SourceInfoFromPointer(ULONG64 pointer, RemoteUtf8SourceInfo& remoteUtf8SourceInfo)
{
    return RemoteThreadContext::ForEach([pointer, &remoteUtf8SourceInfo](RemoteThreadContext threadContext)
    {
        return threadContext.ForEachScriptContext([pointer, &remoteUtf8SourceInfo](RemoteScriptContext scriptContext)
        {
            return scriptContext.ForEachUtf8SourceInfo([pointer, &remoteUtf8SourceInfo](ULONG i, RemoteUtf8SourceInfo utf8SourceInfo)
            {
                if (utf8SourceInfo.GetExtRemoteTyped().GetPtr() == pointer)
                {
                    remoteUtf8SourceInfo = utf8SourceInfo;
                    return true;
                }
                return false;
            });
        });
    });
}

JDRemoteTyped RemoteUtf8SourceInfo::GetSourceContextInfo()
{
    return utf8SourceInfo.GetExtRemoteTyped().Field("m_srcInfo").Field("sourceContextInfo");
}

void RemoteUtf8SourceInfo::PrintSourceUrl()
{
    JDRemoteTyped sourceContextInfo = GetSourceContextInfo();
    bool isDynamic;
    if (sourceContextInfo.HasField("isHostDynamicDocument"))
    {
        isDynamic = sourceContextInfo.Field("isHostDynamicDocument").GetStdBool();
    }
    else
    {
        // //depot/rs2_release_svc_sec/onecoreuap/inetcore/jscriptlegacy/Lib/Runtime/Language/SourceContextInfo.h#1:
        // bool IsDynamic() const { return dwHostSourceContext == Js::Constants::NoHostSourceContext; }
        isDynamic = sourceContextInfo.Field("dwHostSourceContext").GetLong() == -1;
    }
    if (isDynamic)
    {
        GetExtension()->Out("[dynamic script #%d]", sourceContextInfo.Field("hash").GetUlong());
    }
    else
    {
        ULONG64 urlPtr = sourceContextInfo.Field("url").GetPtr();
        if (urlPtr != 0)
        {
            GetExtension()->Out("%mu", urlPtr);
        }
    }
}

bool RemoteUtf8SourceInfo::GetSourceBuffer(ExtBuffer<CHAR>& sourceBuffer, ULONG64 startOffset, ULONG length)
{

    ULONG64 buffer = 0;

    if (length == (ULONG)-1)
    {
        length = utf8SourceInfo.Field("m_cchLength").GetUlong() - (ULONG)startOffset;
    }

    try
    {
        if (utf8SourceInfo.HasField("debugModeSource"))
        {
            buffer = utf8SourceInfo.Field("debugModeSource").GetPtr();
        }

        if (buffer == 0 && utf8SourceInfo.HasField("m_utf8Source"))
        {
            buffer = utf8SourceInfo.Field("m_utf8Source").GetPtr();
        }

        if (buffer == 0 && utf8SourceInfo.HasField("m_pTridentBuffer"))
        {
            buffer = utf8SourceInfo.Field("m_pTridentBuffer").GetPtr();
        }

        if (buffer == 0)
        {
            GetExtension()->Out("Unable to find source buffer (startOffset = %llu, length= %u)", startOffset, length);
            return false;
        }
    }
    catch (ExtException& ex)
    {
        GetExtension()->Out("Exception getting source buffer: %s (startOffset = %llu, length= %u)", ex.GetMessageW(), startOffset, length);
        return false;
    }

    try
    {
        ExtRemoteData source(buffer + startOffset, length);
        
        sourceBuffer.Require(length + 1);
        source.ReadBuffer(sourceBuffer.GetBuffer(), length);
        sourceBuffer.GetBuffer()[length] = 0;        
    }
    catch (ExtException& ex)
    {
        GetExtension()->Out("Exception reading source buffer: %s (startOffset = %llu, length= %u)", ex.GetMessageW(), startOffset, length);
        return false;
    }
    return true;
}
void RemoteUtf8SourceInfo::PrintSource(ULONG64 startOffset, ULONG length)
{    
    ExtBuffer<CHAR> sourceBuffer;
    if (GetSourceBuffer(sourceBuffer, startOffset, length))
    {
        GetExtension()->Out(sourceBuffer.GetBuffer());
    }
}

void RemoteUtf8SourceInfo::SaveSource(char const * filename, ULONG64 startOffset, ULONG length)
{
    ExtBuffer<CHAR> sourceBuffer;
    if (GetSourceBuffer(sourceBuffer, startOffset, length))
    {
        FILE * f = fopen(filename, "wb");
        if (f == nullptr)
        {
            g_Ext->Err("Could not open '%s'\n", filename);
            return;
        }
        fprintf(f, sourceBuffer.GetBuffer());
        fclose(f);
        g_Ext->Out("Finished saving to '%s'\n", filename);
    }
}

ULONG RemoteUtf8SourceInfo::GetSourceContextId()
{
    return GetSourceContextInfo().Field("sourceContextId").GetUlong();
}

RemoteScriptContext RemoteUtf8SourceInfo::GetScriptContext()
{
    return utf8SourceInfo.Field("m_scriptContext");
}

JDRemoteTyped RemoteUtf8SourceInfo::GetLineOffsetCache()
{
    if (!GetExtension()->IsJScript9())
    {
        return utf8SourceInfo.Field("m_lineOffsetCache");
    }
    return JDRemoteTyped::NullPtr();
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetDeferredFunctionsDictionary()
{
    if (!GetExtension()->IsJScript9())
    {
        return utf8SourceInfo.Field("m_deferredFunctionsDictionary");
    }
    return JDRemoteTyped::NullPtr();
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetFunctionBodyDictionary()
{
    return utf8SourceInfo.Field("functionBodyDictionary");
}

ExtRemoteTyped RemoteUtf8SourceInfo::GetExtRemoteTyped()
{
    return utf8SourceInfo.GetExtRemoteTyped();
}

bool RemoteUtf8SourceInfo::HasBoundedPropertyRecordHashSet()
{
    return utf8SourceInfo.HasField("boundedPropertyRecordHashSet");
}

RemoteBaseDictionary RemoteUtf8SourceInfo::GetBoundedPropertyRecordHashSet()
{
    return utf8SourceInfo.Field("boundedPropertyRecordHashSet");
}
