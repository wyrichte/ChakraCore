//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

#include "JDUtil.h"
#include <functional>
class RemoteFunctionProxy : public JDRemoteTyped
{
public:
    RemoteFunctionProxy() {}
    RemoteFunctionProxy(ULONG64 pBody);
    RemoteFunctionProxy(const char* subType, ULONG64 pBody) : JDRemoteTyped(subType, pBody) {}
    RemoteFunctionProxy(ExtRemoteTyped const& functionProxy) : JDRemoteTyped(functionProxy) {}

    // Get the auxiliary pointer on the function body by the field name
    JDRemoteTyped GetAuxPtrsField(const char* fieldName, char* castType = nullptr);
    // Print all instantiated auxiliary pointers
    void PrintAuxPtrs(EXT_CLASS_BASE *ext);
private:
    template<typename Fn>
    void WalkAuxPtrs(Fn fn);

};

class RemoteParseableFunctionInfo : public RemoteFunctionProxy
{
public:
    RemoteParseableFunctionInfo() {}
    RemoteParseableFunctionInfo(ULONG64 pBody) : RemoteFunctionProxy(pBody) {}
    RemoteParseableFunctionInfo(ExtRemoteTyped const& functionProxy) : RemoteFunctionProxy(functionProxy) {}

    JDRemoteTyped GetScopeInfo()
    {
        return this->GetWrappedField("m_scopeInfo");
    }
    
    JDRemoteTyped GetWrappedField(char* fieldName);
};

class RemoteFunctionBody : public RemoteParseableFunctionInfo
{
public:
    RemoteFunctionBody() {}
    RemoteFunctionBody(ULONG64 pBody) : RemoteParseableFunctionInfo(pBody) {}
    RemoteFunctionBody(ExtRemoteTyped const& functionBody) : RemoteParseableFunctionInfo(functionBody) {}

    JDRemoteTyped GetByteCodeBlock()
    {
        return JDUtil::GetWrappedField(*this, "byteCodeBlock");
    }
    JDRemoteTyped GetAuxBlock()
    {
        return this->GetWrappedField("auxBlock", "Js::ByteBlock");
    }
    JDRemoteTyped GetAuxContextBlock()
    {
        return this->GetWrappedField("auxContextBlock");
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
        return this->GetCounterField("m_constCount");
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
        return this->GetWrappedField("StatementMaps", nullptr, "pStatementMaps");
    }

    JDRemoteTyped GetEntryPoints()
    {
        return JDUtil::GetWrappedField(*this, "entryPoints");
    }

    ULONG GetInlineCacheCount()
    {
        return this->GetCounterField("inlineCacheCount", true);
    }

    USHORT GetProfiledCallSiteCount()
    {
        return JDUtil::GetWrappedField(*this, "profiledCallSiteCount").GetUshort();        
    }

    JDRemoteTyped GetWrappedField(char* fieldName, char* castType = nullptr, char* oldFieldName = nullptr);
    uint32 GetCounterField(const char* oldName, bool wasWrapped = false);

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
