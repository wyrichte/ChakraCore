//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

#include "JDUtil.h"
class RemoteFunctionBody : public ExtRemoteTyped
{
public:
    RemoteFunctionBody() {}
    RemoteFunctionBody(ULONG64 pBody) : ExtRemoteTyped("(Js::FunctionBody*)@$extin", pBody) {}
    RemoteFunctionBody(ExtRemoteTyped const& functionBody) : ExtRemoteTyped(functionBody) {};
    ExtRemoteTyped GetByteCodeBlock() 
    {
        return JDUtil::GetWrappedField(*this, "byteCodeBlock");
    }
    ExtRemoteTyped GetProbeBackingStore()
    {
        return GetSourceInfo().Field("m_probeBackingBlock");
    }
    ExtRemoteTyped GetCacheIdToPropertyIdMap()
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
    ExtRemoteTyped GetConstTable()
    {
        return JDUtil::GetWrappedField(*this, "m_constTable");
    }
    ExtRemoteTyped GetSourceInfo()
    {
        return this->Field("m_sourceInfo");
    }

    ExtRemoteTyped GetScriptContext()
    {
        return JDUtil::GetWrappedField(*this, "m_scriptContext");
    }
    ExtRemoteTyped GetThreadContext()
    {
        return GetScriptContext().Field("threadContext");
    }
};


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
