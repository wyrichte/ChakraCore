//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

static std::map<std::string, uint8> auxPtrsEnum;
static std::vector<std::string> vecAuxPtrsEnum;

void InitAuxPtrsEnums()
{
    if (auxPtrsEnum.empty())
    {
        char buf[MAX_PATH];
        for (uint8 i = 0; i < 255; i++)
        {
            sprintf_s(buf, "@@c++((%s!Js::FunctionProxy::AuxPointerType)%d)", GetExtension()->FillModule("%s"), i);
            auto enumName = JDRemoteTyped(buf).GetSimpleValue();
            if (strstr(enumName, "No matching enumerant"))
            {
                break;
            }
            *strchr(enumName, ' ') = '\0';
            auxPtrsEnum[enumName] = i;
            vecAuxPtrsEnum.push_back(enumName);
        }
    }
}

template<typename Fn>
void RemoteFunctionProxy::WalkAuxPtrs(Fn fn)
{
    if (this->HasField("auxPtrs"))
    {
        JDRemoteTyped auxPtrs = this->Field("auxPtrs").Field("ptr");
        g_Ext->Out("auxPtrs: 0x%I64X\n", auxPtrs.GetPtr());
        if (auxPtrs.GetPtr() != 0)
        {
            InitAuxPtrsEnums();
            char buf[MAX_PATH];
            uint8 count = auxPtrs.Field("count").GetUchar();
            uint8 maxCount16 = g_Ext->IsCurMachine64() ? 1 : 3;
            uint8 maxCount32 = g_Ext->IsCurMachine64() ? 3 : 6;
            if (count == maxCount16)
            {
                sprintf_s(buf, "@@c++((%s!Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,16,%u>*)0x%I64X)", GetExtension()->FillModule("%s"), maxCount16, auxPtrs.GetPtr());
                auto auxPtr16 = Eval(buf);
                for (uint i = 0; i < count; i++)
                {
                    uint8 type = auxPtr16.Field("type").ArrayElement(i).GetUchar();
                    if (type != 0xff)
                    {
                        if (fn(type, vecAuxPtrsEnum[type].c_str(), auxPtr16.Field("ptr").ArrayElement(i).Field("ptr")))
                        {
                            return;
                        }
                    }
                }
            }
            else if (count == maxCount32)
            {
                sprintf_s(buf, "@@c++((%s!Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,32,%u>*)0x%I64X)", GetExtension()->FillModule("%s"), maxCount32, auxPtrs.GetPtr());
                auto auxPtr32 = Eval(buf);
                for (uint i = 0; i < count; i++)
                {
                    uint8 type = auxPtr32.Field("type").ArrayElement(i).GetUchar();
                    if (type != 0xff)
                    {
                        if (fn(type, vecAuxPtrsEnum[type].c_str(), auxPtr32.Field("ptr").ArrayElement(i).Field("ptr")))
                        {
                            return;
                        }
                    }
                }
            }
            else if (count > maxCount32)
            {
                auto offsets = auxPtrs.Field("offsets");
                auto ptrs = auxPtrs.Field("ptrs");
                for (uint8 i = 0; i < vecAuxPtrsEnum.size(); i++)
                {
                    auto offset = offsets.ArrayElement(i).GetUchar();
                    if (offset != 0xff)
                    {
                        if (fn(i, vecAuxPtrsEnum[i].c_str(), ptrs.ArrayElement(offset).Field("ptr")))
                        {
                            return;
                        }
                    }
                }
            }
            else
            {
                g_Ext->Err("AuxPtrs structure is corrupted, ptr: 0x%I64X", auxPtrs.GetPtr());
            }
        }
    }
}
 
JDRemoteTyped RemoteFunctionProxy::GetAuxPtrsField(const char* fieldName, char* castType)
{
    JDRemoteTyped ret = Eval("@@c++((void*)0)");

    InitAuxPtrsEnums();
    if (strlen(fieldName) > 2 && (fieldName[0] == 'm' && fieldName[1] == '_')) // 'm_' has been removed in the field enum name in core
    {
        fieldName = fieldName + 2;
    }

    std::string newFieldName(fieldName);
    newFieldName[0] = (char)toupper(newFieldName[0]); // the field name changed to pascal case in core
    if (auxPtrsEnum.find(newFieldName) != auxPtrsEnum.end())
    {
        auto fieldEnum = auxPtrsEnum[newFieldName];
        WalkAuxPtrs([&](uint8 type, const char* name, ExtRemoteTyped auxPtr) ->bool
        {
            if (type == fieldEnum) 
            {
                ret = auxPtr;
                return true;
            }
            return false;
        });
    }

    if (castType)
    {
        char buf[MAX_PATH];
        sprintf_s(buf, "@@c++((%s!%s*)0x%I64X)", GetExtension()->FillModule("%s"), castType, ret.GetPtr());
        ret = Eval(buf);
    }

    return ret;
}

void RemoteFunctionProxy::PrintAuxPtrs(EXT_CLASS_BASE *ext)
{
    WalkAuxPtrs([](uint8 type, const char* name, ExtRemoteTyped auxPtr) ->bool
    {
        g_Ext->Out("\t%s:\t\t\t0x%I64X\n", name, auxPtr.GetPtr());
        return false;
    });
}

JDRemoteTyped RemoteParseableFunctionInfo::GetWrappedField(char* fieldName)
{
    if (this->HasField("auxPtrs"))
    {
        return GetAuxPtrsField(fieldName);
    }

    return JDUtil::GetWrappedField(*this, fieldName);
}

JDRemoteTyped RemoteFunctionBody::GetWrappedField(char* fieldName, char* castType, char* oldFieldName)
{
    if (this->HasField("auxPtrs"))
    {
        return GetAuxPtrsField(fieldName, castType);
    }

    return JDUtil::GetWrappedField(*this, oldFieldName ? oldFieldName : fieldName);
}

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
    return GetUtf8SourceInfo().Field("m_srcInfo").Field("sourceContextInfo");
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------