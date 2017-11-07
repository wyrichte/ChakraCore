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

    JDRemoteTyped GetFunctionObjectTypeList()
    {
        return this->GetAuxWrappedField("functionObjectTypeList", nullptr /* vtable cast */, "m_functionObjectTypeList");
    }

    // Get the auxiliary pointer on the function body by the field name
    JDRemoteTyped GetAuxPtrsField(const char* fieldName, char* castType);
    // Print all instantiated auxiliary pointers
    void PrintAuxPtrs();

protected:
    JDRemoteTyped GetAuxWrappedField(char* fieldName, char* castType, char* oldFieldName = nullptr);

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
        return this->GetAuxWrappedField("m_scopeInfo", "Js::ScopeInfo");
    }
    
    JDRemoteTyped GetBoundPropertyRecords()
    {
        return JDUtil::GetWrappedField(*this, "m_boundPropertyRecords");
    }

    JDRemoteTyped GetDisplayName()
    {
        return JDUtil::GetWrappedField(*this, "m_displayName");
    }

    PWCHAR GetDisplayName(ExtBuffer<WCHAR> * buffer)
    {
        return JDUtil::GetWrappedField(*this, "m_displayName").Dereference().GetString(buffer);
    }
};

class RemoteFunctionBody : public RemoteParseableFunctionInfo
{
public:
    RemoteFunctionBody() {}
    RemoteFunctionBody(ULONG64 pBody) : RemoteParseableFunctionInfo(pBody) {}
    RemoteFunctionBody(ExtRemoteTyped const& functionBody) : RemoteParseableFunctionInfo(functionBody) {}

    JDRemoteTyped GetByteCodeBlock()
    {
        return this->GetWrappedFieldRecyclerData("byteCodeBlock");
    }
    JDRemoteTyped GetAuxBlock()
    {
        return this->GetAuxWrappedFieldRecyclerData("auxBlock", "Js::ByteBlock");
    }
    JDRemoteTyped GetAuxContextBlock()
    {
        return this->GetAuxWrappedFieldRecyclerData("auxContextBlock", "Js::ByteBlock");
    }
    JDRemoteTyped GetLoopHeaderArray()
    {
        return this->GetAuxWrappedFieldRecyclerData("loopHeaderArray", "Js::LoopHeader");
    }
    JDRemoteTyped GetProbeBackingStore()
    {
        return GetSourceInfo().Field("m_probeBackingBlock");
    }
    JDRemoteTyped GetCacheIdToPropertyIdMap()
    {
        return this->GetWrappedFieldRecyclerData("cacheIdToPropertyIdMap");
    }
    JDRemoteTyped GetReferencedPropertyIdMap();

    uint GetConstCount()
    {
        return this->GetCounterField("m_constCount");
    }
    ushort GetParamCount()
    {
        return this->Field("m_inParamCount").GetUshort();
    }
    uint GetLoopCount()
    {
        return this->GetCounterField("loopCount");
    }
    bool HasImplicitArgIns()
    {
        return this->Field("m_hasImplicitArgIns").GetStdBool();
    }
    JDRemoteTyped GetConstTable()
    {
        return this->GetWrappedFieldRecyclerData("m_constTable");
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
    ULONG GetSourceContextId()
    {
        return GetSourceContextInfo().Field("sourceContextId").GetUlong();
    }

    ULONG GetLocalFunctionId()
    {
        if (HasField("functionId"))
        {
            return JDUtil::GetWrappedField(*this, "functionId").GetUlong();
        }
        else
        {
            return JDUtil::GetWrappedField(*this, "functionInfo").Field("functionId").GetUlong();
        }
    }

    ULONG GetFunctionNumber()
    {
        return JDUtil::GetWrappedField(*this, "m_functionNumber").GetUlong();
    }

    JDRemoteTyped GetStatementMaps()
    {
        return this->GetAuxWrappedFieldRecyclerData("StatementMaps", nullptr, "pStatementMaps");
    }

    JDRemoteTyped GetEntryPoints()
    {
        return this->GetWrappedFieldRecyclerData("entryPoints");
    }

    ULONG GetInlineCacheCount()
    {
        return this->GetCounterField("inlineCacheCount", true);
    }

    USHORT GetProfiledCallSiteCount()
    {
        return JDUtil::GetWrappedField(*this, "profiledCallSiteCount").GetUshort();
    }

    JDRemoteTyped GetCodeGenRuntiemData()
    {
        return this->GetAuxWrappedFieldRecyclerData("m_codeGenRuntimeData", "Js::FunctionCodeGenRuntimeData *");
    }
    JDRemoteTyped GetCodeGenGetSetRuntimeData()
    {
        return this->GetAuxWrappedFieldRecyclerData("m_codeGenGetSetRuntimeData", "Js::FunctionCodeGenRuntimeData *");
    }

    JDRemoteTyped GetInlineCaches()
    {
        return this->GetWrappedFieldRecyclerData("inlineCaches");
    }

    JDRemoteTyped GetPolymorphicInlineCaches()
    {
        return this->GetFieldRecyclerData("polymorphicInlineCaches");
    }

    JDRemoteTyped GetPolymorphicInlineCachesHead()
    {
        return this->GetAuxWrappedFieldRecyclerData("m_polymorphicInlineCachesHead", "Js::PolymorphicInlineCache");
    }

    JDRemoteTyped GetPropertyIdsForScopeSlotArray()
    {
        return this->GetAuxWrappedFieldRecyclerData("propertyIdsForScopeSlotArray", "Js::PropertyId");
    }

    JDRemoteTyped GetLiteralRegexes()
    {
        return this->GetAuxWrappedFieldRecyclerData("literalRegexes", "UnifiedRegex::RegexPattern *");
    }

    JDRemoteTyped GetFieldRecyclerData(char * fieldName);
    JDRemoteTyped GetWrappedFieldRecyclerData(char* fieldName);
    JDRemoteTyped GetAuxWrappedFieldRecyclerData(char* fieldName, char* castType, char* oldFieldName = nullptr);

    uint32 GetCounterField(const char* oldName, bool wasWrapped = false);

    void PrintNameAndNumber();
    void PrintNameAndNumberWithLink();
    void PrintNameAndNumberWithRawLink();
    void PrintByteCodeLink();
    void PrintSourceUrl();
    void PrintSource();
private:
    JDRemoteTyped GetUtf8SourceInfo();
    JDRemoteTyped GetSourceContextInfo();
};


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
