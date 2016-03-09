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
    : heapBlockAddress(0), hasCachedTotalObjectCountAndSize(true), bucketObjectSize((ULONG)-1),
    totalObjectCount(0), totalObjectSize(0), size(0), type(-1), address(0), attributes(nullptr),
    hasCachedAllocatedObjectCountAndSize(true), allocatedObjectCount(0), allocatedObjectSize(0), debuggeeMemory(nullptr)
{

}

RemoteHeapBlock::RemoteHeapBlock(ULONG64 heapBlockAddress)
    : heapBlockAddress(heapBlockAddress), attributes(nullptr), debuggeeMemory(nullptr)
{
    JDRemoteTyped heapBlock = GetExtension()->recyclerCachedData.GetAsHeapBlock(heapBlockAddress);
    type = heapBlock.Field("heapBlockType").GetChar();
    Initialize();
}

RemoteHeapBlock::RemoteHeapBlock(ExtRemoteTyped& heapBlock)
    : heapBlockAddress(heapBlock.GetPtr()), attributes(nullptr), debuggeeMemory(nullptr)
{
    type = heapBlock.Field("heapBlockType").GetChar();
    Initialize();
}

RemoteHeapBlock::RemoteHeapBlock(RemoteHeapBlock const& other) :
    heapBlockAddress(other.heapBlockAddress), address(other.address), type(other.type), size(other.size),
    hasCachedTotalObjectCountAndSize(other.hasCachedTotalObjectCountAndSize), bucketObjectSize(other.bucketObjectSize),
    totalObjectCount(other.totalObjectCount), totalObjectSize(other.totalObjectSize), hasCachedAllocatedObjectCountAndSize(false),
    attributes(nullptr), debuggeeMemory(nullptr)
{

}
RemoteHeapBlock::~RemoteHeapBlock()
{
    GetExtension()->recyclerCachedData.RemoveCachedDebuggeeMemory(&debuggeeMemory);

    if (attributes)
    {
        delete[] attributes;
    }
}
void
RemoteHeapBlock::Initialize()
{
    JDRemoteTyped heapBlock = GetExtRemoteTyped();

    address = heapBlock.Field("address").GetPtr();
    if (IsLargeHeapBlock())
    {
        size = (g_Ext->m_PageSize * ExtRemoteTypedUtil::GetSizeT(heapBlock.Field("pageCount")));
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

    hasCachedAllocatedObjectCountAndSize = false;
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
    JDRemoteTyped heapBlock = GetExtRemoteTyped();
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

ULONG RemoteHeapBlock::GetFreeObjectCount()
{
    EnsureCachedAllocatedObjectCountAndSize();
    return GetTotalObjectCount() - GetAllocatedObjectCount();
}

ULONG64 RemoteHeapBlock::GetFreeObjectSize()
{
    EnsureCachedAllocatedObjectCountAndSize();
    return GetTotalObjectSize() - GetAllocatedObjectSize();
}

ULONG RemoteHeapBlock::GetAllocatedObjectCount()
{
    EnsureCachedAllocatedObjectCountAndSize();
    return allocatedObjectCount;
}

ULONG64 RemoteHeapBlock::GetAllocatedObjectSize()
{
    EnsureCachedAllocatedObjectCountAndSize();
    return allocatedObjectSize;
}

void RemoteHeapBlock::EnsureCachedTotalObjectCountAndSize()
{
    if (hasCachedTotalObjectCountAndSize)
    {
        return;
    }

    totalObjectCount = 0;
    totalObjectSize = 0;

    ForEachLargeObjectHeader([&](JDRemoteTyped& largeObjectHeader)
    {
        totalObjectCount++;
        totalObjectSize += ExtRemoteTypedUtil::GetSizeT(largeObjectHeader.Field("objectSize"));
        return false;
    });
}

ULONG RemoteHeapBlock::GetObjectIndex(ULONG64 objectAddress)
{
    Assert(!this->IsLargeHeapBlock());
    ULONG64 objectIndex = (objectAddress - GetAddress()) / this->GetBucketObjectSize();
    Assert(objectIndex < this->GetTotalObjectCount());
    return (ULONG)objectIndex;
}

ULONG64 RemoteHeapBlock::GetObjectAddressFromIndex(ULONG objectIndex)
{
    Assert(!this->IsLargeHeapBlock());
    return objectIndex * this->GetBucketObjectSize() + this->GetAddress();
}

void RemoteHeapBlock::EnsureCachedAllocatedObjectCountAndSize()
{
    if (hasCachedAllocatedObjectCountAndSize)
    {
        return;
    }

    if (IsLargeHeapBlock())
    {
        allocatedObjectCount = 0;
        allocatedObjectSize = 0;
        ForEachLargeObjectHeader([&](JDRemoteTyped& largeObjectHeader)
        {
            allocatedObjectCount++;
            allocatedObjectSize += ExtRemoteTypedUtil::GetSizeT(largeObjectHeader.Field("objectSize"));
            return false;
        });
        return;
    }

    allocatedObjectCount = totalObjectCount;
    allocatedObjectSize = totalObjectSize;
    freeObject = std::vector<bool>(totalObjectCount);
    if (this->IsFinalizableHeapBlock())
    {
        attributes = new UCHAR[totalObjectCount];
        ExtRemoteData objectInfo(this->GetHeapBlockAddress() - totalObjectCount, totalObjectCount);
        objectInfo.ReadBuffer(attributes, totalObjectCount);
    }

    JDRemoteTyped heapBlock = GetExtRemoteTyped();
    JDRemoteTyped head;
    bool isBumpAllocation = false;
    if (heapBlock.Field("isInAllocator").GetChar())
    {
        JDRemoteTyped heapBucket(GetExtension()->GetSmallHeapBucketTypeName(), heapBlock.Field("heapBucket").GetPtr(), false);
        bool found = ExtRemoteTypedUtil::LinkListForEach(
            heapBucket.Field("allocatorHead").GetPointerTo(), "next",
            [&](JDRemoteTyped& allocator)
        {
            if (allocator.Field("heapBlock").GetPtr() == this->GetHeapBlockAddress())
            {
                head = allocator.Field("freeObjectList");
                ULONG64 endAddress = allocator.Field("endAddress").GetPtr();
                if (endAddress != 0)
                {
                    isBumpAllocation = true;
                    ULONG64 freeSize = endAddress - head.GetPtr();
                    ULONG64 freeObjectCount = freeSize / this->GetBucketObjectSize();
                    allocatedObjectCount -= (ULONG)freeObjectCount;
                    allocatedObjectSize -= freeObjectCount * this->GetBucketObjectSize();
                    for (ULONG64 i = head.GetPtr(); i < this->GetAddress() + totalObjectSize; i++)
                    {
                        ULONG objectIndex = GetObjectIndex(i);
                        freeObject[objectIndex] = true;
                    }
                }
                return true;
            }
            return false;
        });

        Assert(found);
    }
    else
    {
        head = heapBlock.Field("freeObjectList");
    }

    if (!isBumpAllocation && head.GetPtr())
    {
        AutoDebuggeeMemory autoDebuggeeMemory(this, this->GetAddress(), (ULONG)this->GetSize());
        ULONG64 curr = head.GetPtr();
        while (curr != 0)
        {
            Assert(allocatedObjectCount != 0);
            Assert(allocatedObjectSize >= this->bucketObjectSize);
            allocatedObjectCount--;
            allocatedObjectSize -= this->bucketObjectSize;
            ULONG objectIndex = GetObjectIndex(curr);
            freeObject[objectIndex] = true;
            char * currBuffer = (char *)autoDebuggeeMemory + (curr - this->GetAddress());
            curr = (g_Ext->m_PtrSize == 8 ? *(ULONG64 *)currBuffer : *(ULONG *)currBuffer) &~1;
        }
    }
    hasCachedAllocatedObjectCountAndSize = true;
}

char * RemoteHeapBlock::GetDebuggeeMemory(ULONG64 address, ULONG size, bool * cached)
{
    Assert(address >= this->GetAddress());
    Assert(size <= this->GetSize());
    Assert(address - this->GetAddress() < this->GetSize());
    if (this->debuggeeMemory != nullptr || GetExtension()->recyclerCachedData.GetCachedDebuggeeMemory(this->GetAddress(), (ULONG)this->GetSize(), &this->debuggeeMemory))
    {
        *cached = true;
        return this->debuggeeMemory + address - this->GetAddress();
    }

    ExtRemoteData data(address, size);
    std::auto_ptr<char> newBuffer(new char[size]);
    data.ReadBuffer(newBuffer.get(), size);
    *cached = false;
    return newBuffer.release();
}

ULONG RemoteHeapBlock::GetLargeHeapBlockHeaderList(JDRemoteTyped& headerList)
{
    Assert(this->IsLargeHeapBlock());
    JDRemoteTyped heapBlock = GetExtRemoteTyped();
    headerList = JDRemoteTyped(GetExtension()->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), this->GetHeapBlockAddress() + heapBlock.GetTypeSize());
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
        JDRemoteTyped largeObjectHeader;
        if (interior)
        {
            bool found = this->ForEachLargeObjectHeader([&](JDRemoteTyped header)
            {
                if (header.GetPtr() + sizeOfObjectHeader < originalAddress)
                {
                    return false;
                }
                objectSize = ExtRemoteTypedUtil::GetSizeT(header.Field("objectSize"));
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
            largeObjectHeader = JDRemoteTyped(GetExtension()->FillModuleAndMemoryNS("%s!%sLargeObjectHeader"), headerAddress, false);
            if (headerAddress < this->GetAddress())
            {
                if (verbose)
                {
                    g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", originalAddress);
                }
                return false;
            }
            JDRemoteTyped headerList;
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
            objectSize = ExtRemoteTypedUtil::GetSizeT(largeObjectHeader.Field("objectSize"));;
        }

        UCHAR attributes;
        if (largeObjectHeader.HasField("attributesAndChecksum"))
        {
            attributes = (UCHAR)((largeObjectHeader.Field("attributesAndChecksum").GetUshort() ^ (USHORT)GetRecyclerCookie()) >> 8);
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

    if (originalAddress >= GetAddress() + GetTotalObjectSize())
    {
        if (verbose)
        {
            g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", originalAddress);
            g_Ext->Out("Pass end of valid objects 0x%p\n", originalAddress);
        }
        return false;
    }
    ULONG objectIndex = GetObjectIndex(originalAddress);
    ULONG64 objectAddress = GetObjectAddressFromIndex(objectIndex);
    if (!interior && objectAddress != originalAddress)
    {
        if (verbose)
        {
            g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", originalAddress);
            g_Ext->Out("Interior pointer to index: %d, 0x%p\n", objectIndex, objectAddress);
        }
        return false;
    }

    this->EnsureCachedAllocatedObjectCountAndSize();
    if (this->freeObject[objectIndex])
    {
        if (verbose)
        {
            g_Ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
            g_Ext->Out("Free object at index: %d, 0x%p\n", objectIndex, objectAddress);
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
        info.attributes = this->attributes[totalObjectCount - objectIndex - 1];
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