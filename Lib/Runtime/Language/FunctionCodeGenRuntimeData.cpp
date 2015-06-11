// Copyright (C) Microsoft. All rights reserved. 

#include "StdAfx.h"

namespace Js
{
    FunctionCodeGenRuntimeData::FunctionCodeGenRuntimeData(FunctionBody *const functionBody)
        : functionBody(functionBody), inlinees(null),next(0)
    {
    }

    FunctionBody *FunctionCodeGenRuntimeData::GetFunctionBody() const
    {
        return functionBody;
    }

    const InlineCachePointerArray<InlineCache> *FunctionCodeGenRuntimeData::ClonedInlineCaches() const
    {
        return &clonedInlineCaches;
    }

    InlineCachePointerArray<InlineCache> *FunctionCodeGenRuntimeData::ClonedInlineCaches()
    {
        return &clonedInlineCaches;
    }

    const FunctionCodeGenRuntimeData *FunctionCodeGenRuntimeData::GetInlinee(const ProfileId profiledCallSiteId) const
    {
        Assert(profiledCallSiteId < functionBody->GetProfiledCallSiteCount());

        return inlinees ? inlinees[profiledCallSiteId] : null;
    }

    const FunctionCodeGenRuntimeData *FunctionCodeGenRuntimeData::GetInlineeForTargetInlinee(const ProfileId profiledCallSiteId, FunctionBody *inlineeFuncBody) const
    {
        Assert(profiledCallSiteId < functionBody->GetProfiledCallSiteCount());

        if (!inlinees)
        {
            return null;
        }
        FunctionCodeGenRuntimeData *runtimeData = inlinees[profiledCallSiteId];
        while(runtimeData && runtimeData->GetFunctionBody() != inlineeFuncBody)
        {
            runtimeData = runtimeData->next;
        }
        return runtimeData;
    }

    void FunctionCodeGenRuntimeData::SetupRecursiveInlineeChain(
        Recycler *const recycler,
        const ProfileId profiledCallSiteId,
        FunctionBody *const inlinee)
    {
        Assert(recycler);
        Assert(profiledCallSiteId < functionBody->GetProfiledCallSiteCount());
        Assert(inlinee == functionBody);
        if (!inlinees)
        {
            inlinees = RecyclerNewArrayZ(recycler, FunctionCodeGenRuntimeData *, functionBody->GetProfiledCallSiteCount());
        }
        inlinees[profiledCallSiteId] = this;
    }

    FunctionCodeGenRuntimeData *FunctionCodeGenRuntimeData::EnsureInlinee(
        Recycler *const recycler,
        const ProfileId profiledCallSiteId,
        FunctionBody *const inlinee)
    {
        Assert(recycler);
        Assert(profiledCallSiteId < functionBody->GetProfiledCallSiteCount());
        Assert(inlinee);

        if(!inlinees)
        {
            inlinees = RecyclerNewArrayZ(recycler, FunctionCodeGenRuntimeData *, functionBody->GetProfiledCallSiteCount());
        }
        const auto inlineeData = inlinees[profiledCallSiteId];

        if(!inlineeData)
        {
            return inlinees[profiledCallSiteId] = RecyclerNew(recycler, FunctionCodeGenRuntimeData, inlinee);
        }

        //Find the right code gen runtime data
        FunctionCodeGenRuntimeData *next = inlineeData;  
        while(next && next->functionBody != inlinee)
        {
            next = next->next;
        }

        if (next)
        {
            return next;
        }

        FunctionCodeGenRuntimeData *runtimeData = RecyclerNew(recycler, FunctionCodeGenRuntimeData, inlinee);
        runtimeData->next = inlineeData;
        return inlinees[profiledCallSiteId] = runtimeData;;
    }

    const FunctionCodeGenRuntimeData *FunctionCodeGenRuntimeData::GetLdFldInlinee(const InlineCacheIndex inlineCacheIndex) const
    {
        Assert(inlineCacheIndex < functionBody->GetInlineCacheCount());

        return ldFldInlinees ? ldFldInlinees[inlineCacheIndex] : null;
    }

    const FunctionCodeGenRuntimeData *FunctionCodeGenRuntimeData::GetRuntimeDataFromFunctionInfo(FunctionInfo *polyFunctionInfo) const
    {
        const FunctionCodeGenRuntimeData *next = this;
        while(next && next->functionBody != polyFunctionInfo)
        {
            next = next->next;
        }
        return next;
    }

    FunctionCodeGenRuntimeData *FunctionCodeGenRuntimeData::EnsureLdFldInlinee(
        Recycler *const recycler,
        const InlineCacheIndex inlineCacheIndex,
        FunctionBody *const inlinee)
    {
        Assert(recycler);
        Assert(inlineCacheIndex < functionBody->GetInlineCacheCount());
        Assert(inlinee);

        if(!ldFldInlinees)
        {
            ldFldInlinees = RecyclerNewArrayZ(recycler, FunctionCodeGenRuntimeData *, functionBody->GetInlineCacheCount());
        }
        const auto inlineeData = ldFldInlinees[inlineCacheIndex];
        if (!inlineeData)
        {
            return ldFldInlinees[inlineCacheIndex] = RecyclerNew(recycler, FunctionCodeGenRuntimeData, inlinee);
        }
        return inlineeData;
    }
}
