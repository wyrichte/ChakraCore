//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

void
RemoteFunctionBody::PrintNameAndNumber(EXT_CLASS_BASE * ext)
{
    ExtBuffer<WCHAR> displayNameBuffer;
    ext->Out(L"%s (#%d.%d, #%d)", GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber());
}

void
RemoteFunctionBody::PrintNameAndNumberWithLink(EXT_CLASS_BASE * ext)
{
    ExtBuffer<WCHAR> displayNameBuffer;
    ext->Dml(L"<link cmd=\"!jd.fb (Js::FunctionBody *)0x%p\">%s</link> (#%d.%d, #%d)", this->GetPtr(), GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber());
}

void
RemoteFunctionBody::PrintNameAndNumberWithRawLink(EXT_CLASS_BASE * ext)
{
    ExtBuffer<WCHAR> displayNameBuffer;
    ext->Dml(L"%s (#%d.%d, #%d) @ <link cmd=\"dt Js::FunctionBody 0x%p\">0x%p</link>", GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber(), this->GetPtr(), this->GetPtr());
}

void
RemoteFunctionBody::PrintByteCodeLink(EXT_CLASS_BASE * ext)
{    
    ext->Dml("<link cmd=\"!jd.bc (Js::FunctionBody *)0x%p\">Byte Code</link>", this->GetPtr());
}

void
RemoteFunctionBody::PrintSourceUrl(EXT_CLASS_BASE *ext)
{
    JDRemoteTyped sourceContextInfo = GetSourceContextInfo();
    if (sourceContextInfo.Field("isHostDynamicDocument").GetStdBool())
    {
        ext->Out("[dynamic script #%d]", sourceContextInfo.Field("hash"));
    }
    else
    {
        ext->Out("%mu", sourceContextInfo.Field("url").GetPtr());
    }
}

void
RemoteFunctionBody::PrintSource(EXT_CLASS_BASE * ext)
{
    
    JDRemoteTyped utf8SourceInfo = GetUtf8SourceInfo();
    ULONG64 buffer = utf8SourceInfo.Field("debugModeSource").GetPtr();
    if (buffer == 0)
    {
        buffer = utf8SourceInfo.Field("m_pTridentBuffer").GetPtr();
        if (buffer == 0)
        {
            ext->Out("Unable to find source buffer");
            return;
        }
    }
    ULONG64 startOffset = ExtRemoteTypedUtil::GetSizeT(JDUtil::GetWrappedField(*this, "m_cbStartOffset"));
    ULONG length = (ULONG)ExtRemoteTypedUtil::GetSizeT(JDUtil::GetWrappedField(*this, "m_cbLength"));
    ExtRemoteData source(buffer + startOffset, length);
    ExtBuffer<CHAR> sourceBuffer;
    sourceBuffer.Require(length + 1);    
    source.ReadBuffer(sourceBuffer.GetBuffer(), length);
    sourceBuffer.GetBuffer()[length] = 0;
    ext->Out("%s", sourceBuffer.GetBuffer());
}

JDRemoteTyped
RemoteFunctionBody::GetUtf8SourceInfo()
{
    return JDUtil::GetWrappedField(*this, "m_utf8SourceInfo");
}

JDRemoteTyped
RemoteFunctionBody::GetSourceContextInfo()
{
    return GetUtf8SourceInfo().Field("m_srcInfo.sourceContextInfo");
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------