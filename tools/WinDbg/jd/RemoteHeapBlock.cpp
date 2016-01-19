//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once
#include "stdafx.h"
#include "RemoteHeapBlock.h"

// TODO: Get this from the PDB
#define LeafBit 0x20

bool HeapObjectInfo::IsLeaf()
{
    return (attributes & LeafBit) != 0;
}

RemoteHeapBlock RemoteHeapBlock::NullHeapBlock;

RemoteHeapBlock::RemoteHeapBlock()
    : heapBlockAddress(0), hasCachedTotalObjectCountAndSize(true), bucketObjectSize((ULONG)-1), totalObjectCount(0), totalObjectSize(0), size(0), type(-1), address(0)
{

}

RemoteHeapBlock::RemoteHeapBlock(ULONG64 heapBlockAddress)
    : heapBlockAddress(heapBlockAddress)
{
    ExtRemoteTyped heapBlock = GetExtension()->recyclerCachedData.GetAsHeapBlock(heapBlockAddress);
    type = heapBlock.Field("heapBlockType").GetChar();
    Initialize();
}

RemoteHeapBlock::RemoteHeapBlock(ExtRemoteTyped& heapBlock)
    : heapBlockAddress(heapBlock.GetPtr())
{ 
    type = heapBlock.Field("heapBlockType").GetChar();
    Initialize(); 
}

void
RemoteHeapBlock::Initialize()
{
    ExtRemoteTyped heapBlock = GetExtRemoteTyped();

    address = heapBlock.Field("address").GetPtr();
    if (IsLargeHeapBlock())
    {
        size = (g_Ext->m_PageSize * EXT_CLASS_BASE::GetSizeT(heapBlock.Field("pageCount")));
        hasCachedTotalObjectCountAndSize = false;
        bucketObjectSize = (ULONG)-1;     // No bucket object size
    }
    else
    {
        hasCachedTotalObjectCountAndSize = true;
        // Small/Medium block can have multple pages in Threshold        
        totalObjectCount = heapBlock.Field("objectCount").GetUshort();
        bucketObjectSize = heapBlock.Field("objectSize").GetUshort();
        totalObjectSize = bucketObjectSize * totalObjectCount;
        size = JDUtil::Align<ULONG64>(totalObjectSize, g_Ext->m_PageSize);
    }
}

ExtRemoteTyped 
RemoteHeapBlock::GetExtRemoteTyped()
{
    if (IsLargeHeapBlock())
    {
        return GetExtension()->recyclerCachedData.GetAsLargeHeapBlock(heapBlockAddress);
    }
    return GetExtension()->recyclerCachedData.GetAsSmallHeapBlock(heapBlockAddress);
}

char
RemoteHeapBlock::GetType()
{
    return type;
}

bool
RemoteHeapBlock::IsSmallLeafHeapBlock()
{
    return type == GetExtension()->enum_SmallLeafBlockType();
}

bool
RemoteHeapBlock::IsMediumLeafHeapBlock()
{
    return type == GetExtension()->enum_MediumLeafBlockType();
}

bool
RemoteHeapBlock::IsSmallFinalizableHeapBlock()
{
    return type == GetExtension()->enum_SmallFinalizableBlockType();
}

bool
RemoteHeapBlock::IsMediumFinalizableHeapBlock()
{
    return type == GetExtension()->enum_MediumFinalizableBlockType();
}

bool
RemoteHeapBlock::IsSmallFinalizableWithBarrierHeapBlock()
{
    return type == GetExtension()->enum_SmallFinalizableBlockWithBarrierType();
}

bool
RemoteHeapBlock::IsMediumFinalizableWithBarrierHeapBlock()
{
    return type == GetExtension()->enum_MediumFinalizableBlockWithBarrierType();
}

bool
RemoteHeapBlock::IsLeafHeapBlock()
{
    return IsSmallLeafHeapBlock() || IsMediumLeafHeapBlock();
}

bool
RemoteHeapBlock::IsFinalizableHeapBlock()
{
    return IsSmallFinalizableHeapBlock() || IsMediumFinalizableHeapBlock() || IsSmallFinalizableWithBarrierHeapBlock() || IsMediumFinalizableWithBarrierHeapBlock();
}

bool
RemoteHeapBlock::IsLargeHeapBlock()
{
    return type == GetExtension()->enum_LargeBlockType();
}

ULONG64 RemoteHeapBlock::GetSize()
{
    return size;
}

ULONG RemoteHeapBlock::GetFinalizeCount()
{
    ExtRemoteTyped heapBlock = GetExtRemoteTyped();
    if (IsLargeHeapBlock())
    {
        return heapBlock.Field("finalizeCount").GetUlong();
    }
    if (heapBlock.HasField("finalizeCount"))
    {
        return heapBlock.Field("finalizeCount").GetUshort();
    }
    return 0;
}

ULONG RemoteHeapBlock::GetTotalObjectCount()
{
    EnsureCachedTotalObjectCountAndSize();
    return totalObjectCount;
}


ULONG64 RemoteHeapBlock::GetTotalObjectSize()
{
    EnsureCachedTotalObjectCountAndSize();
    return totalObjectSize;
}

void RemoteHeapBlock::EnsureCachedTotalObjectCountAndSize()
{
    if (hasCachedTotalObjectCountAndSize)
    {
        return;
    }

    totalObjectCount = 0;
    totalObjectSize = 0;

    ForEachLargeObjectHeader([&](ExtRemoteTyped& largeObjectHeader)
    {
        totalObjectCount++;
        totalObjectSize += EXT_CLASS_BASE::GetSizeT(largeObjectHeader.Field("objectSize"));
        return false;
    });
}

ULONG RemoteHeapBlock::GetLargeHeapBlockHeaderList(ExtRemoteTyped& headerList)
{
    Assert(this->IsLargeHeapBlock());
    ExtRemoteTyped heapBlock = GetExtRemoteTyped();
    headerList = ExtRemoteTyped(GetExtension()->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), this->GetHeapBlockAddress() + heapBlock.GetTypeSize());
    return heapBlock.Field("allocCount").GetUlong();
}

ULONG RemoteHeapBlock::GetRecyclerCookie()
{
    if (IsLargeHeapBlock())
    {
        return GetExtRemoteTyped().Field("heapInfo").Field("recycler").Field("Cookie").GetUlong();
    }
    return GetExtRemoteTyped().Field("bucket").Field("heapInfo").Field("recycler").Field("Cookie").GetUlong();
}

bool RemoteHeapBlock::GetRecyclerHeapObjectInfo(ULONG64 originalAddress, HeapObjectInfo& info, bool interior, bool verbose)
{
    Assert(originalAddress >= this->GetAddress() && originalAddress < this->GetAddress() + this->GetSize());
    if (this->IsLargeHeapBlock())
    {
        ULONG64 objectAddress;
        ULONG64 objectSize;
        ULONG64 sizeOfObjectHeader = g_Ext->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));
        ExtRemoteTyped largeObjectHeader;
        if (interior)
        {
            bool found = this->ForEachLargeObjectHeader([&](ExtRemoteTyped header)
            {
                if (header.GetPtr() + sizeOfObjectHeader < originalAddress)
                {
                    return false;
                }
                objectSize = EXT_CLASS_BASE::GetSizeT(header.Field("objectSize"));
                if (header.GetPtr() + sizeOfObjectHeader + objectSize >= originalAddress)
                {
                    return false;
                }
                largeObjectHeader = header;
                objectAddress = header.GetPtr() + sizeOfObjectHeader;
                return true;
            });

            if (!found)
            {
                if (verbose)
                {
                    g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", originalAddress);
                }
                return false;
            }
        }
        else
        {
            ULONG64 headerAddress = originalAddress - sizeOfObjectHeader;
            largeObjectHeader = ExtRemoteTyped(GetExtension()->FillModuleAndMemoryNS("%s!%sLargeObjectHeader"), headerAddress, false);
            if (headerAddress < this->GetAddress())
            {
                if (verbose)
                {
                    g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", originalAddress);
                }
                return false;
            }
            ExtRemoteTyped headerList;
            ULONG allocCount = this->GetLargeHeapBlockHeaderList(headerList);

            ULONG objectIndex = largeObjectHeader.Field("objectIndex").GetUlong();

            if (objectIndex > allocCount)
            {
                return false;
            }

            if (headerList.ArrayElement(objectIndex).GetPtr() != headerAddress)
            {
                if (verbose)
                {
                    g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", originalAddress);
                    g_Ext->Out("Header address: 0x%p, Header in index %d is 0x%p\n", headerAddress, objectIndex, headerAddress);
                }

                return false;
            }

            objectAddress = originalAddress;
            objectSize = EXT_CLASS_BASE::GetSizeT(largeObjectHeader.Field("objectSize"));;
        }
        
        UCHAR attributes;
        if (largeObjectHeader.HasField("attributesAndChecksum"))
        {
            attributes = (UCHAR)(largeObjectHeader.Field("attributesAndChecksum").GetUshort() ^ GetRecyclerCookie());
        }
        else if (largeObjectHeader.HasField("attributes"))
        {
            attributes = largeObjectHeader.Field("attributes").GetUchar();
        }
        else
        {
            g_Ext->Err("Can't find either attributes or attributesAndChecksum on LargeHeapBlockHeader");
            return false;
        }

        info.originalAddress = originalAddress;
        info.objectAddress = objectAddress;
        // TODO: This might potentially truncate, but very unlikely to happen.
        // Just output an message for now
        if (objectSize > ULONG_MAX)
        {
            g_Ext->Out("Object >= 4G size found at 0x%p, size %I64u trucated\n", originalAddress);
        }
        info.objectSize = (ULONG)objectSize;
        info.attributes = attributes;
        return true;
    }

    ULONG64 objectIndex = (originalAddress - this->GetAddress()) / this->GetBucketObjectSize();
    ULONG64 objectAddress = objectIndex * this->GetBucketObjectSize() + this->GetAddress();
    if (!interior && objectAddress != originalAddress)
    {
        if (verbose)
        {
            g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
            g_Ext->Out("Interior pointer to index: %d, 0x%p\n", objectIndex, objectAddress);
        }
        return false;
    }

    info.originalAddress = originalAddress;
    info.objectAddress = objectAddress;
    info.objectSize = this->GetBucketObjectSize();
    if (this->IsLeafHeapBlock())
    {
        info.attributes = LeafBit;
    }
    else if (!this->IsFinalizableHeapBlock())
    {
        info.attributes = 0;
    }
    else 
    {
        ULONG64 objectInfoAddress = this->GetHeapBlockAddress() - objectIndex - 1;
        ExtRemoteData objectInfo(objectInfoAddress, 1);
        info.attributes = objectInfo.GetUchar();
    }
    return true;
}

void RemoteHeapBlock::VerboseOut()
{
    ExtRemoteTyped heapBlock = GetExtRemoteTyped();
    g_Ext->Out("0x%p, InAllocator: %d, Size: 0x%x, Type: %d",
        this->GetHeapBlockAddress(),
        this->GetBucketObjectSize(),
        this->GetType(),
        heapBlock.Field("isInAllocator").GetChar()
        );
    // isNewHeapBlock is gone for TH and above, so we need to check if it exists
    if (heapBlock.HasField("isNewHeapBlock"))
    {
        g_Ext->Out(" New: %d", heapBlock.Field("isNewHeapBlock").GetChar());
    }
    g_Ext->Out("\n");
}