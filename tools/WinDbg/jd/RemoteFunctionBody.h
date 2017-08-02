//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

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
    JDRemoteTyped GetAuxPtrsField(char const* fieldName, char const* castType);
    // Print all instantiated auxiliary pointers
    void PrintAuxPtrs();

protected:
    JDRemoteTyped GetAuxWrappedField(char const* fieldName, char const* castType, char const* oldFieldName = nullptr);

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

    JDRemoteTyped GetDeferredPrototypeType()
    {
        return JDUtil::GetWrappedField(*this, "deferredPrototypeType");
    }

    JDRemoteTyped GetScopeInfo()
    {
        return this->GetAuxWrappedField("m_scopeInfo", "Js::ScopeInfo");
    }
    
    RemoteBaseDictionary GetBoundPropertyRecords()
    {
        return JDUtil::GetWrappedField(*this, "m_boundPropertyRecords");
    }

    JDRemoteTyped GetDisplayName()
    {
        return JDUtil::GetWrappedField(*this, "m_displayName");
    }

    PWCHAR GetDisplayName(ExtBuffer<WCHAR> * buffer)
    {
        return GetDisplayName().Dereference().GetString(buffer);
    }

    JDRemoteTyped GetNestedArray()
    {
        if (HasField("nestedArray"))
        {
            // Added in d76f76e9f72a4a7b99a62f64c3803d5e80aa4f9b fop RS2
            return JDUtil::GetWrappedField(*this, "nestedArray");
        }
        return JDRemoteTyped("(void *)0");
    }

    JDRemoteTyped GetPropertyIdsForScopeSlotArray()
    {
        return this->GetAuxWrappedFieldRecyclerData("propertyIdsForScopeSlotArray", "Js::PropertyId");
    }
    JDRemoteTyped GetDeferredStubs();
protected:
    JDRemoteTyped GetAuxWrappedFieldRecyclerData(char const * fieldName, char const* castType, char const * oldFieldName = nullptr);
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
    JDRemoteTyped GetInlineCacheTypes()
    {
        // DBG only field
        if (this->HasField("m_inlineCacheTypes"))
        {
            return JDUtil::GetWrappedField(*this, "m_inlineCacheTypes");
        }
        return JDRemoteTyped("(void *)0");
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
        JDRemoteTyped remoteTyped = this->GetAuxWrappedFieldRecyclerData("m_polymorphicInlineCachesHead", "Js::PolymorphicInlineCache");
        if (remoteTyped.HasField("next"))
        {
            return remoteTyped;
        }
        // After commit ebb59986122be91d451cefb72d9b6ba8320d2d69 in RS3
        return remoteTyped.Cast("Js::FunctionBodyPolymorphicInlineCache");
    }

    JDRemoteTyped GetLiteralRegexes()
    {
        return this->GetAuxWrappedFieldRecyclerData("literalRegexes", "UnifiedRegex::RegexPattern *");
    }

    JDRemoteTyped GetObjectLiteralTypes();

    JDRemoteTyped GetPropertyIdOnRegSlotsContainer()
    {
        return this->GetAuxWrappedFieldRecyclerData("propertyIdOnRegSlotsContainer", "Js::PropertyIdOnRegSlotsContainer");
    }

    void PrintNameAndNumber();
    void PrintNameAndNumberWithLink();
    void PrintNameAndNumberWithRawLink();
    void PrintByteCodeLink();
    void PrintSourceUrl();
    void PrintSource();
private:
    JDRemoteTyped GetFieldRecyclerData(char const * fieldName);
    JDRemoteTyped GetWrappedFieldRecyclerData(char const * fieldName);    

    uint32 GetCounterField(const char* oldName, bool wasWrapped = false);

    JDRemoteTyped GetUtf8SourceInfo();
    JDRemoteTyped GetSourceContextInfo();
};
