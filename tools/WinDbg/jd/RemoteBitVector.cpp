//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "RemoteBitVector.h"

RemoteBitVector::RemoteBitVector(ExtRemoteTyped bv) : bv(bv)
{
}

ULONG64 RemoteBitVector::GetBVFixedAllocSize(ULONG64 length)
{
    ULONG64 bvFixedSize = g_Ext->EvalExprU64(GetExtension()->FillModule("@@c++(sizeof(%s!BVFixed))"));
    ULONG64 bvUnitSize = g_Ext->EvalExprU64(GetExtension()->FillModule("@@c++(sizeof(%s!BVUnit))"));
    ULONG64 bvUnitShiftValue = g_Ext->EvalExprU64(GetExtension()->FillModule("@@c++(BVUnit::ShiftValue)"));
    return bvFixedSize + bvUnitSize * (((length - 1) >> bvUnitShiftValue) + 1);
}

RemoteBitVector
RemoteBitVector::FromBVFixedPointer(ULONG64 address)
{
    return ExtRemoteTyped(GetExtension()->FillModule("(%s!BVFixed*) @$extin"), address);
}

bool
RemoteBitVector::Test(ULONG64 index, ULONG64 * bvUnitAddress)
{
    ExtRemoteTyped data = bv.Field("data");
    ULONG64 bvUnitSize = g_Ext->EvalExprU64(GetExtension()->FillModule("@@c++(sizeof(%s!BVUnit))"));
    ULONG64 bitPerUnit = bvUnitSize * 8;
    ExtRemoteTyped bvUnit = data[index / bitPerUnit];
    ULONG64 offset = index % bitPerUnit;

    ULONG64 word = ExtRemoteTypedUtil::GetSizeT(bvUnit.Field("word"));
    if (bvUnitAddress)
    {
        *bvUnitAddress = bvUnit.GetPointerTo().GetPtr();
    }
    return (word & ((ULONG64)1 << offset)) != 0;
}