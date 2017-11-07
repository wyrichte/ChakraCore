//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

void InitEnums(_In_ const char* enumType, std::map<std::string, uint8>& nameValMap, std::vector<std::string>& names)
{
    if (nameValMap.empty())
    {
        char buf[MAX_PATH];
        for (uint8 i = 0; i < 255; i++)
        {
            buf[0] = '\0'; // guarantee that the buffer will be null-terminated in case sprintf_s fails.
            sprintf_s(buf, "@@c++((%s!%s)%d)", GetExtension()->FillModule("%s"), enumType, i);

            auto enumName = JDRemoteTyped(buf).GetSimpleValue();
            if (strstr(enumName, "No matching enumerant"))
            {
                break;
            }

            *strchr(enumName, ' ') = '\0';
            nameValMap[enumName] = i;
            names.push_back(enumName);
        }
    }
}

static std::map<std::string, uint8> auxPtrsEnum;
static std::vector<std::string> vecAuxPtrsEnum;
void EnsureAuxPtrsEnums()
{
    InitEnums("Js::FunctionProxy::AuxPointerType", auxPtrsEnum, vecAuxPtrsEnum);
}
template<typename Fn>
void RemoteFunctionProxy::WalkAuxPtrs(Fn fn)
{
    if (this->HasField("auxPtrs"))
    {
        JDRemoteTyped auxPtrs = this->Field("auxPtrs").Field("ptr");
        if (auxPtrs.GetPtr() != 0)
        {
            EnsureAuxPtrsEnums();
            uint8 count = auxPtrs.Field("count").GetUchar();
            uint8 maxCount16 = g_Ext->IsCurMachine64() ? 1 : 3;
            uint8 maxCount32 = g_Ext->IsCurMachine64() ? 3 : 6;
            if (count == maxCount16)
            {
                auto auxPtr16 = GetExtension()->m_AuxPtrsFix16.Cast(auxPtrs.GetPtr());
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
                auto auxPtr32 = GetExtension()->m_AuxPtrsFix32.Cast(auxPtrs.GetPtr());
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

RemoteFunctionProxy::RemoteFunctionProxy(ULONG64 pBody) : 
    JDRemoteTyped(JDRemoteTyped::FromPtrWithVtable(pBody))
{}
 
JDRemoteTyped RemoteFunctionProxy::GetAuxPtrsField(const char* fieldName, char* castType)
{
    JDRemoteTyped ret = Eval("@@c++((void*)0)");

    EnsureAuxPtrsEnums();
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
        ret = ret.Cast(castType);
    }
    else if (ret.GetPtr() != 0)
    {
        ret = ret.CastWithVtable();
    }

    return ret;
}

void RemoteFunctionProxy::PrintAuxPtrs()
{
    WalkAuxPtrs([](uint8 type, const char* name, ExtRemoteTyped auxPtr) ->bool
    {
        GetExtension()->Out("\t%s:\t\t\t0x%I64X\n", name, auxPtr.GetPtr());
        return false;
    });
}

JDRemoteTyped RemoteFunctionProxy::GetAuxWrappedField(char* fieldName, char* castType, char* oldFieldName)
{
    if (this->HasField("auxPtrs"))
    {
        return GetAuxPtrsField(fieldName, castType);
    }

    return JDUtil::GetWrappedField(*this, oldFieldName ? oldFieldName : fieldName);
}

JDRemoteTyped RemoteFunctionBody::GetReferencedPropertyIdMap()
{
    if (!GetExtension()->IsJScript9())
    {
        return this->GetAuxWrappedField("referencedPropertyIdMap", "Js::PropertyId");
    }
    return JDRemoteTyped("(void *)0");
}

JDRemoteTyped RemoteFunctionBody::GetFieldRecyclerData(char * fieldName)
{
    if (GetExtension()->IsJScript9())
    {
        return this->Field("recyclerData").Field(fieldName);
    }
    return this->Field(fieldName);
}
JDRemoteTyped RemoteFunctionBody::GetWrappedFieldRecyclerData(char * fieldName)
{
    if (GetExtension()->IsJScript9())
    {
        return this->Field("recyclerData").Field(fieldName);
    }
    return JDUtil::GetWrappedField(*this, fieldName);
}

JDRemoteTyped RemoteFunctionBody::GetAuxWrappedFieldRecyclerData(char* fieldName, char* castType, char* oldFieldName)
{
    if (GetExtension()->IsJScript9())
    {
        return this->Field("recyclerData").Field(oldFieldName != nullptr ? oldFieldName : fieldName);
    }
    return this->GetAuxWrappedField(fieldName, castType, oldFieldName);
}

static std::map<std::string, uint8> counterEnum;
static std::vector<std::string> vecCounterEnum;
static std::map<std::string, std::string> counterFieldNameMap; // counter field old name and new enum name map
void EnsureCountersEnums()
{
    // note, with this way it can't parse duplicated enum names. 
    // TODO: change to parse 'dt' command result when we need to use the duplicated enum names
    InitEnums("Js::FunctionBody::CounterFields", counterEnum, vecCounterEnum);
    if (counterFieldNameMap.empty()) 
    {
        counterFieldNameMap["m_constCount"] = "ConstantCount";
        counterFieldNameMap["inlineCacheCount"] = "InlineCacheCount";
        counterFieldNameMap["loopCount"] = "LoopCount";
    }
}

uint32 RemoteFunctionBody::GetCounterField(const char* oldName, bool wasWrapped)
{
    if (this->HasField("counters"))
    {
        EnsureCountersEnums();
        if (counterFieldNameMap.find(oldName) != counterFieldNameMap.end()) 
        {
            uint8 fieldEnum = counterEnum[counterFieldNameMap[oldName]];
            auto counter = this->Field("counters");
            auto fieldSize = counter.Field("fieldSize").GetUchar();
            auto fields = JDUtil::GetWrappedField(counter, "fields");
            if (fieldSize == 1) 
            {
                return fields.Field("u8Fields").ArrayElement(fieldEnum).GetUchar();
            }
            else if (fieldSize == 2)
            {
                return fields.Field("u16Fields").ArrayElement(fieldEnum).GetUshort();
            }
            else if (fieldSize == 4)
            {
                return fields.Field("u32Fields").ArrayElement(fieldEnum).GetUlong();
            }
            else 
            {
                g_Ext->ThrowStatus(E_FAIL, "Function body counter structure corrupted, fieldSize is: %d", fieldSize);
            }
        }
        else 
        {
            g_Ext->ThrowStatus(E_FAIL, "JD need to update to map %s to new field enum on FunctionBody", oldName);
        }
    }

    if(wasWrapped)
    {
        return JDUtil::GetWrappedField(*this, oldName).GetUlong();
    }
    else 
    {
        return this->Field(oldName).GetUlong();
    }
}


void
RemoteFunctionBody::PrintNameAndNumber()
{
    ExtBuffer<WCHAR> displayNameBuffer;
    GetExtension()->Out(_u("%s (#%d.%d, #%d)"), GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber());
}

void
RemoteFunctionBody::PrintNameAndNumberWithLink()
{
    ExtBuffer<WCHAR> displayNameBuffer;
    if (GetExtension()->PreferDML())
    {
        GetExtension()->Dml(_u("<link cmd=\"!jd.fb (%s *)0x%p\">%s</link> (#%d.%d, #%d)"), GetExtension()->FillModule("%s!Js::FunctionBody"), this->GetPtr(), GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber());
    }
    else
    {
        GetExtension()->Out(_u("%s (#%d.%d, #%d) /*\"!jd.fb (%s *)0x%p\" to display*/"), GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber(), GetExtension()->FillModule("%s!Js::FunctionBody"), this->GetPtr());
    }
}

void
RemoteFunctionBody::PrintNameAndNumberWithRawLink()
{
    ExtBuffer<WCHAR> displayNameBuffer;
    if (GetExtension()->PreferDML())
    {
        GetExtension()->Dml(_u("%s (#%d.%d, #%d) @ <link cmd=\"dt %s 0x%p\">0x%p</link>"), GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber(), GetExtension()->FillModule("%s!Js::FunctionBody"), this->GetPtr(), this->GetPtr());
    }
    else
    {
        GetExtension()->Out(_u("%s (#%d.%d, #%d) @ 0x%p /*\"dt %s 0x%p\" to display*/"), GetDisplayName(&displayNameBuffer), GetSourceContextId(), GetLocalFunctionId(), GetFunctionNumber(), GetExtension()->FillModule("%s!Js::FunctionBody"), this->GetPtr(), this->GetPtr());
    }
}

void
RemoteFunctionBody::PrintByteCodeLink()
{
    if (GetExtension()->PreferDML())
    {
        GetExtension()->Dml("<link cmd=\"!jd.bc (%s *)0x%p\">Byte Code</link>", GetExtension()->FillModule("%s!Js::FunctionBody"), this->GetPtr());
    }
    else
    {
        GetExtension()->Out("Byte Code /*\"!jd.bc (%s *)0x%p\" to display*/", GetExtension()->FillModule("%s!Js::FunctionBody"), this->GetPtr());
    }
}

void
RemoteFunctionBody::PrintSourceUrl()
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
        GetExtension()->Out("[dynamic script #%d]", sourceContextInfo.Field("hash").GetPtr());
    }
    else
    {
        GetExtension()->Out("%mu", sourceContextInfo.Field("url").GetPtr());
    }
}

void
RemoteFunctionBody::PrintSource()
{
    
    JDRemoteTyped utf8SourceInfo = GetUtf8SourceInfo();
    ULONG64 buffer = utf8SourceInfo.Field("debugModeSource").GetPtr();
    if (buffer == 0)
    {
        buffer = utf8SourceInfo.Field("m_pTridentBuffer").GetPtr();
        if (buffer == 0)
        {
            GetExtension()->Out("Unable to find source buffer");
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
    GetExtension()->Out("%s", sourceBuffer.GetBuffer());
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
