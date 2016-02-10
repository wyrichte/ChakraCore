//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

#include "JDUtil.h"
class RemoteFunctionBody : public JDRemoteTyped
{
public:
    RemoteFunctionBody() {}
    RemoteFunctionBody(ULONG64 pBody) : JDRemoteTyped("(Js::FunctionBody*)@$extin", pBody) {}
    RemoteFunctionBody(ExtRemoteTyped const& functionBody) : JDRemoteTyped(functionBody) {};
    JDRemoteTyped GetByteCodeBlock()
    {
        return JDUtil::GetWrappedField(*this, "byteCodeBlock");
    }
    JDRemoteTyped GetAuxBlock()
    {
        return JDUtil::GetWrappedField(*this, "auxBlock");
    }
    JDRemoteTyped GetAuxContextBlock()
    {
        return JDUtil::GetWrappedField(*this, "auxContextBlock");
    }
    JDRemoteTyped GetProbeBackingStore()
    {
        return GetSourceInfo().Field("m_probeBackingBlock");
    }
    JDRemoteTyped GetCacheIdToPropertyIdMap()
    {
        return JDUtil::GetWrappedField(*this, "cacheIdToPropertyIdMap");
    }
    uint GetConstCount()
    {
        return this->Field("m_constCount").GetUlong();
    }
    ushort GetParamCount()
    {
        return this->Field("m_inParamCount").GetUshort();
    }
    bool HasImplicitArgIns()
    {
        return this->Field("m_hasImplicitArgIns").GetStdBool();
    }
    JDRemoteTyped GetConstTable()
    {
        return JDUtil::GetWrappedField(*this, "m_constTable");
    }
    JDRemoteTyped GetSourceInfo()
    {
        return this->Field("m_sourceInfo");
    }

    JDRemoteTyped GetScriptContext()
    {
        return JDUtil::GetWrappedField(*this, "m_scriptContext");
    }
    JDRemoteTyped GetThreadContext()
    {
        return GetScriptContext().Field("threadContext");
    }
    PWCHAR GetDisplayName(ExtBuffer<WCHAR> * buffer)
    {
        return JDUtil::GetWrappedField(*this, "m_displayName").Dereference().GetString(buffer);
    }

    ULONG GetSourceContextId()
    {
        return GetSourceContextInfo().Field("sourceContextId").GetUlong();
    }

    ULONG GetLocalFunctionId()
    {
        return JDUtil::GetWrappedField(*this, "functionId").GetUlong();
    }

    ULONG GetFunctionNumber()
    {
        return JDUtil::GetWrappedField(*this, "m_functionNumber").GetUlong();
    }

    JDRemoteTyped GetStatementMaps()
    {
        return JDUtil::GetWrappedField(*this, "pStatementMaps");
    }

    JDRemoteTyped GetEntryPoints()
    {
        return JDUtil::GetWrappedField(*this, "entryPoints");
    }

    ULONG GetInlineCacheCount()
    {
        return JDUtil::GetWrappedField(*this, "inlineCacheCount").GetUlong();
    }

    USHORT GetProfiledCallSiteCount()
    {
        return JDUtil::GetWrappedField(*this, "profiledCallSiteCount").GetUshort();        
    }
    void PrintNameAndNumber(EXT_CLASS_BASE * ext);
    void PrintNameAndNumberWithLink(EXT_CLASS_BASE * ext);
    void PrintNameAndNumberWithRawLink(EXT_CLASS_BASE * ext);
    void PrintByteCodeLink(EXT_CLASS_BASE *ext);
    void PrintSourceUrl(EXT_CLASS_BASE *ext);
    void PrintSource(EXT_CLASS_BASE *ext);
private:
    JDRemoteTyped GetUtf8SourceInfo();
    JDRemoteTyped GetSourceContextInfo();
};


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
