//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "jdrecycler.h"
#include "RemoteRecyclerList.h"
#include "RemoteRecycler.h"
#include "RecyclerRoots.h"
#include "RemoteHeapBlockMap.h"
#include "ExtRemotetypedUtil.h"
#include "RemoteObjectInfoBits.h"
#include "RemoteBitVector.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

#pragma warning(disable: 4481)  // allow use of abstract and override keywords

PCSTR EXT_CLASS_BASE::GetPageAllocatorType()
{
    static char* pageAllocatorTypeName = nullptr;
    if (pageAllocatorTypeName == nullptr) {
        pageAllocatorTypeName = "Memory::PageAllocatorBase<Memory::VirtualAllocWrapper>";
        if (!CheckTypeName(pageAllocatorTypeName))
        {
            pageAllocatorTypeName = "PageAllocator";
        }
    }
    return pageAllocatorTypeName;
}

PCSTR EXT_CLASS_BASE::GetSegmentType()
{
    static char* segmentTypeName = nullptr;
    if (segmentTypeName == nullptr) {
        segmentTypeName = "Memory::SegmentBase<Memory::VirtualAllocWrapper>";
        if (!CheckTypeName(segmentTypeName))
        {
            segmentTypeName = "Segment";
        }
    }
    return segmentTypeName;
}
PCSTR EXT_CLASS_BASE::GetPageSegmentType()
{
    static char* pageSegmentTypeName = nullptr;
    if (pageSegmentTypeName == nullptr) {
        pageSegmentTypeName = "Memory::PageSegmentBase<Memory::VirtualAllocWrapper>";
        if (!CheckTypeName(pageSegmentTypeName))
        {
            pageSegmentTypeName = "PageSegment";
        }
    }
    return pageSegmentTypeName;
}

bool EXT_CLASS_BASE::CheckTypeName(PCSTR typeName, ULONG* typeId /*= nullptr*/)
{
    char buf[MAX_SYM_NAME];
    ULONG id = 0;
    sprintf_s(buf, "%s!%s", typeName, GetModuleName());
    HRESULT hr = this->m_Symbols2->GetSymbolTypeId(buf, &id, nullptr);
    if (typeId != nullptr)
    {
        *typeId = id;
    }
    return hr == S_OK;
}


JD_PRIVATE_COMMAND(markmap,
    "Dump the mark map",
    "{;s;filename;Filename to output to}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    PCSTR filename = GetUnnamedArgStr(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    ExtRemoteTyped recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }

    if (!recycler.HasField("markMap"))
    {
        Out(_u("Recycler doesn't have mark map field. Please rebuild jscript9 with RECYCLER_MARK_TRACK enabled.\n"));
        return;
    }

    FILE* f = fopen(filename, "w+");
    if (f != nullptr)
    {
        Out("Recycler is 0x%p\n", recycler.GetPtr());
        ExtRemoteTyped map = recycler.Field("markMap");
        uint bucketCount = map.Field("bucketCount").GetUlong();
        ExtRemoteTyped buckets = map.Field("buckets");
        ExtRemoteTyped entries = map.Field("entries");

        int numEntries = 0;

        fprintf(f, "import networkx as nx\n");
        fprintf(f, "G = nx.DiGraph()\n");

        for (uint i = 0; i < bucketCount; i++)
        {
            if (buckets.ArrayElement(i).GetLong() != -1)
            {
                for (int currentIndex = buckets.ArrayElement(i).GetLong(); currentIndex != -1; currentIndex = entries.ArrayElement(currentIndex).Field("next").GetLong())
                {
                    numEntries++;
                    //if (numEntries > 10) break;

                    ExtRemoteTyped data = entries.ArrayElement(currentIndex);
                    ULONG64 key = 0;
                    ULONG64 value = 0;

                    //Out("0x%p\n", data.m_Offset);
                    //Out("Key: 0x%p\n", data.Field("key").m_Offset);

                    if (m_PtrSize == 4)
                    {
                        key = data.Field("key").GetUlongPtr();
                        value = data.Field("value").GetUlongPtr();
                    }
                    else
                    {
                        key = data.Field("key").GetUlong64();
                        value = data.Field("value").GetUlong64();
                    }

                    fprintf(f, "# Item %d\n", currentIndex);
                    fprintf(f, "G.add_edge(");
                    fprintf(f, "'0x%I64X'", value);
                    fprintf(f, ", ");
                    fprintf(f, "'0x%I64X'", key);
                    fprintf(f, ")\n");

                }
            }
        }

        fclose(f);

        Out("%d entries written to '%s'\n", numEntries, filename);
    }
    else
    {
        Out("Could not open '%s'\n", filename);
    }
}

void EXT_CLASS_BASE::DumpBlock(ExtRemoteTyped block, LPCSTR desc, LPCSTR sizeField, int index)
{
    ULONG64 sizeOfBigBlock = this->EvalExprU64(this->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sBigBlock))"));
    // ULONG sizeOfBigBlockPtr = (ULONG) this->EvalExprU64("@@c++(sizeof(BigBlock*))");
    ULONG64 ptrBlock = block.GetPtr();
    ULONG64 length = block.Field(sizeField).GetUlong64();

    Out("%s %d: ", desc, index);
    if (PreferDML())
    {
        Dml("<link cmd=\"db 0x%p l0x%x\">0x%p</link>", ptrBlock + sizeOfBigBlock, length, ptrBlock); // Hack- since ArenaMemoryData and BigBlock have the same size
    }
    else
    {
        Out("0x%p /*\"db 0x%p l0x%x\" to display*/", ptrBlock, ptrBlock + sizeOfBigBlock, length); // Hack- since ArenaMemoryData and BigBlock have the same size
    }
    Out(" (");

    Out("0x%x", length);
    Out(" bytes)\n");
}


struct HeapObject
{
    ULONG64 heapBlockType;
    ULONG64 address;
    ushort index;
    ULONG64 heapBlock;
    ULONG64 objectInfoAddress;
    unsigned char objectInfoBits;
    ULONG64 objectSize;
    ULONG64 vtable;
    ushort addressBitIndex;
    ULONG64 freeBitWord;
    ULONG64 markBitWord;
    bool isFreeSet;
    bool isMarkSet;
};

class ObjectInfoHelper
{
public:
    ObjectInfoHelper(RemoteRecycler recycler)
        : recycler(recycler)
    {
    }

    void DumpObjectInfoBits(unsigned char objectInfoBits);
    void DumpSmallHeapBlockObject(RemoteHeapBlock heapBlock, ULONG64 address, bool verbose);
    void DumpLargeHeapBlockObject(RemoteHeapBlock heapBlock, ULONG64 address, bool verbose);
    void DumpHeapObject(const HeapObject& heapObject, bool verbose);
    void DumpHeapBlockLink(ULONG64 heapBlockType, ULONG64 heapBlock);

private:
    // TODO (doilij) refactor the recycler field out of this class. Persisting an ExtRemoteTyped causes problems.
    RemoteRecycler recycler;
};

void ObjectInfoHelper::DumpObjectInfoBits(unsigned char info)
{
    GetExtension()->Out(_u("Info: 0x%x ("), info);

    if (info & RemoteObjectInfoBits::FinalizeBit) GetExtension()->Out(" Finalize ");
    if (info & RemoteObjectInfoBits::PendingDisposeBit) GetExtension()->Out(" PendingDispose ");
    if (info & RemoteObjectInfoBits::LeafBit) GetExtension()->Out(" Leaf ");
    if (info & RemoteObjectInfoBits::TrackBit) GetExtension()->Out(" Track ");
    if (info & RemoteObjectInfoBits::ImplicitRootBit) GetExtension()->Out(" ImplticitRoot ");
    if (info & RemoteObjectInfoBits::NewTrackBit) GetExtension()->Out(" NewTrack ");
    if (info & RemoteObjectInfoBits::MemoryProfilerOldObjectBit) GetExtension()->Out(" MemoryProfilerOldObject ");
    if (info & RemoteObjectInfoBits::EnumClass_1_Bit) GetExtension()->Out(" EnumClass_1_Bit ");

    GetExtension()->Out(_u(")"));
}

void ObjectInfoHelper::DumpLargeHeapBlockObject(RemoteHeapBlock heapBlock, ULONG64 objectAddress, bool verbose)
{
    ULONG64 heapBlockAddress = heapBlock.GetHeapBlockAddress();
    ULONG64 blockAddress = heapBlock.GetAddress();

    ULONG64 sizeOfHeapBlock = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeHeapBlock))"));
    ULONG64 sizeOfObjectHeader = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));

    ULONG64 headerAddress = objectAddress - sizeOfObjectHeader;

    if (headerAddress < blockAddress)
    {
        GetExtension()->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
        DumpHeapBlockLink(GetExtension()->enum_LargeBlockType(), heapBlockAddress);
        return;
    }

    ExtRemoteTyped largeObjectHeader(GetExtension()->FillModuleAndMemoryNS("%s!%sLargeObjectHeader"), headerAddress, false);

    HeapObject heapObject;
    heapObject.index = (ushort)largeObjectHeader.Field("objectIndex").m_Typed.Data; // Why does calling UShort not work?

    ULONG largeObjectHeaderPtrSize = GetExtension()->m_PtrSize;

    ULONG64 headerArrayAddress = heapBlockAddress + sizeOfHeapBlock + (heapObject.index * largeObjectHeaderPtrSize);
    ExtRemoteData  headerData(headerArrayAddress, largeObjectHeaderPtrSize);

    if (headerData.GetPtr() != headerAddress)
    {
        GetExtension()->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
        DumpHeapBlockLink(GetExtension()->enum_LargeBlockType(), heapBlockAddress);
        GetExtension()->Out("Header address: 0x%p, Header in index %d is 0x%p\n", headerAddress, heapObject.index, headerData.GetPtr());
        return;
    }

    heapObject.address = objectAddress;
    heapObject.addressBitIndex = heapObject.index;
    heapObject.heapBlock = heapBlockAddress;
    heapObject.heapBlockType = GetExtension()->enum_LargeBlockType();
    heapObject.objectInfoAddress = headerAddress + sizeof(uint) + sizeof(uint) + largeObjectHeaderPtrSize;
    if (largeObjectHeader.HasField("attributesAndChecksum"))
    {
        heapObject.objectInfoBits = (UCHAR)((largeObjectHeader.Field("attributesAndChecksum").GetUshort() ^ (USHORT)recycler.GetCookie()) >> 8);
    }
    else
    {
        heapObject.objectInfoBits = largeObjectHeader.Field("attributes").GetUchar();
    }
    heapObject.objectSize = ExtRemoteTypedUtil::GetSizeT(largeObjectHeader.Field("objectSize"));

    ULONG64 objectCount = ExtRemoteTypedUtil::GetSizeT(heapBlock.GetExtRemoteTyped().Field("objectCount"));

    ExtRemoteTyped freeBitWord;
    heapObject.isFreeSet = (headerAddress >= blockAddress && heapObject.index < ExtRemoteTypedUtil::GetSizeT(heapBlock.GetExtRemoteTyped().Field("allocCount")) && headerData.m_Data == NULL);
    heapObject.freeBitWord = NULL;

    if (heapBlock.GetExtRemoteTyped().HasField("markCount"))
    {
        // Before CL #1362395
        ExtRemoteTyped markBitWord;
        ULONG64 markBitVectorAddress = (heapBlockAddress + sizeOfHeapBlock + largeObjectHeaderPtrSize * objectCount);
        RemoteBitVector markBitVector = RemoteBitVector::FromBVFixedPointer(markBitVectorAddress);
        heapObject.isMarkSet = markBitVector.Test(heapObject.addressBitIndex, &heapObject.markBitWord);
    }
    else
    {
        // TODO: CL# 1362395 moved to heap block map based marking
    }


    ExtRemoteData heapObjectData(heapObject.address, GetExtension()->m_PtrSize);
    heapObject.vtable = heapObjectData.GetPtr();

    DumpHeapObject(heapObject, verbose);
}

void ObjectInfoHelper::DumpHeapBlockLink(ULONG64 heapBlockType, ULONG64 heapBlock)
{
    if (heapBlockType == GetExtension()->enum_LargeBlockType())
    {
        PCSTR heapblockSymbol = GetExtension()->FillModuleAndMemoryNS("%s!%sLargeHeapBlock");
        if (GetExtension()->PreferDML())
        {
            GetExtension()->Dml("Large Heap Block: <link cmd=\"dt %s 0x%p\">0x%p</link>\n", heapblockSymbol, heapBlock, heapBlock);
        }
        else
        {
            GetExtension()->Out("Large Heap Block: 0x%p /*\"dt %s 0x%p\" to display*/\n", heapBlock, heapblockSymbol, heapBlock);
        }
    }
    else
    {
        PCSTR heapBlockTypeString = "unknown Heap Block";

        if (heapBlockType == GetExtension()->enum_SmallNormalBlockType())
            heapBlockTypeString = "Small Normal Heap Block";
        else if (heapBlockType == GetExtension()->enum_SmallLeafBlockType())
            heapBlockTypeString = "Small Leaf Block";
        else if (heapBlockType == GetExtension()->enum_SmallFinalizableBlockType())
            heapBlockTypeString = "Small Finalizable Block";
        else if (heapBlockType == GetExtension()->enum_SmallNormalBlockWithBarrierType())
            heapBlockTypeString = "Small Normal Block (with SWB)";
        else if (heapBlockType == GetExtension()->enum_SmallFinalizableBlockWithBarrierType())
            heapBlockTypeString = "Small Finalizable Block (with SWB)";
        else if (heapBlockType == GetExtension()->enum_MediumNormalBlockType())
            heapBlockTypeString = "Medium Normal Heap Block";
        else if (heapBlockType == GetExtension()->enum_MediumLeafBlockType())
            heapBlockTypeString = "Medium Leaf Block";
        else if (heapBlockType == GetExtension()->enum_MediumFinalizableBlockType())
            heapBlockTypeString = "Medium Finalizable Block";
        else if (heapBlockType == GetExtension()->enum_MediumNormalBlockWithBarrierType())
            heapBlockTypeString = "Medium Normal Block (with SWB)";
        else if (heapBlockType == GetExtension()->enum_MediumFinalizableBlockWithBarrierType())
            heapBlockTypeString = "Medium Finalizable Block (with SWB)";
        else
            Assert(false);

        if (recycler.GetPtr())
        {
            if (GetExtension()->PreferDML())
            {
                GetExtension()->Dml("%s: <link cmd=\"!showblockinfo 0x%p 0x%p\">0x%p</link>\n", heapBlockTypeString, heapBlock, recycler.GetPtr(), heapBlock);
            }
            else
            {
                GetExtension()->Out("%s: 0x%p /*\"!showblockinfo 0x%p 0x%p\" to display*/\n", heapBlockTypeString, heapBlock, heapBlock, recycler.GetPtr());
            }
        }
        else
        {
            if (GetExtension()->PreferDML())
            {
                GetExtension()->Dml("%s: <link cmd=\"!showblockinfo 0x%p\">0x%p</link>\n", heapBlockTypeString, heapBlock, heapBlock);
            }
            else
            {
                GetExtension()->Out("%s: 0x%p /*\"!showblockinfo 0x%p\" to display*/\n", heapBlockTypeString, heapBlock, heapBlock);
            }
        }
    }
}

void ObjectInfoHelper::DumpHeapObject(const HeapObject& heapObject, bool verbose)
{
    // DumpHeapBlockLink(heapObject.heapBlockType, heapObject.heapBlock);

    GetExtension()->Out(_u("Object: "));
    std::string className = GetExtension()->GetTypeNameFromVTable(heapObject.vtable);

    if (!className.empty())
    {
        if (GetExtension()->PreferDML())
        {
            GetExtension()->Dml("<link cmd=\"dt %s 0x%p\">0x%p</link>", className.c_str(), heapObject.address, heapObject.address);
        }
        else
        {
            GetExtension()->Out("0x%p /*\"dt %s 0x%p\" to display*/", heapObject.address, className.c_str(), heapObject.address);
        }
    }
    else
    {
        GetExtension()->Out(_u("0x%p "), heapObject.address);
    }

    GetExtension()->Out(_u(" (Symbol @ 0x%p: "), heapObject.vtable);
    GetExtension()->m_Symbols3->OutputSymbolByOffset(DEBUG_OUTCTL_AMBIENT, DEBUG_OUTSYM_ALLOW_DISPLACEMENT, heapObject.vtable);
    GetExtension()->Out(")");

    GetExtension()->Out("\n");
    GetExtension()->Out(_u("Object size: 0x%x\n"), heapObject.objectSize);

    DumpObjectInfoBits(heapObject.objectInfoBits);
    GetExtension()->Out(" @0x%p\n", heapObject.objectInfoAddress);

    GetExtension()->Out("Object index: %d\n", heapObject.index);

    if (heapObject.heapBlockType == GetExtension()->enum_SmallLeafBlockType()
        || heapObject.heapBlockType == GetExtension()->enum_SmallNormalBlockType()
#ifdef RECYCLER_WRITE_BARRIER
        || heapObject.heapBlockType == GetExtension()->enum_SmallNormalBlockWithBarrierType()
#endif
        )
    {
        GetExtension()->Out(_u("Address bit index: %d\n"), heapObject.addressBitIndex);
    }

    if (verbose)
    {
        GetExtension()->Out("FreeBit: %d", heapObject.isFreeSet);
        if (!GetExtension()->IsMinidumpDebugging())
        {
            GetExtension()->Out(" (");
            if (GetExtension()->PreferDML())
            {
                GetExtension()->Dml("<link cmd=\"ba w1 0x%p\">Set Breakpoint</link>", heapObject.freeBitWord);
            }
            else
            {
                GetExtension()->Out("Set Breakpoint: \"ba w1 0x%p\"", heapObject.freeBitWord);
            }
            GetExtension()->Out(")");
        }
        GetExtension()->Out("\n");

        GetExtension()->Out("MarkBit: %d", heapObject.isMarkSet);
        if (!GetExtension()->IsMinidumpDebugging())
        {
            GetExtension()->Out(" (");
            if (GetExtension()->PreferDML())
            {
                GetExtension()->Dml("<link cmd=\"ba w1 0x%p\">Set Breakpoint</link>", heapObject.markBitWord);
            }
            else
            {
                GetExtension()->Out("Set Breakpoint: \"ba w1 0x%p\"", heapObject.markBitWord);
            }
            GetExtension()->Out(")");
        }
        GetExtension()->Out("\n");
    }
}

void ObjectInfoHelper::DumpSmallHeapBlockObject(RemoteHeapBlock heapBlock, ULONG64 objectAddress, bool verbose)
{
    ULONG64 heapBlockAddress = heapBlock.GetHeapBlockAddress();

    HeapObject heapObject;
    heapObject.heapBlock = heapBlockAddress;
    heapObject.heapBlockType = heapBlock.GetType();
    heapObject.objectSize = heapBlock.GetBucketObjectSize();

    uint objectCount = heapBlock.GetTotalObjectCount();
    ULONG64 startAddress = heapBlock.GetAddress();
    
    heapObject.index = (ushort)((objectAddress - startAddress) / heapObject.objectSize);
    heapObject.address = startAddress + heapObject.index * heapObject.objectSize;
    if (heapObject.index < objectCount && heapObject.address == objectAddress)
    {
        ExtRemoteData heapObjectData(heapObject.address, GetExtension()->m_PtrSize);
        heapObject.vtable = heapObjectData.GetPtr();

        heapObject.addressBitIndex = heapBlock.GetAddressBitIndex(heapObject.address);
        heapObject.isFreeSet = heapBlock.GetFreeBits().Test(heapObject.addressBitIndex, &heapObject.freeBitWord);
        heapObject.isMarkSet = heapBlock.GetMarkBits().Test(heapObject.addressBitIndex, &heapObject.markBitWord);
        heapObject.objectInfoAddress = heapBlockAddress - heapObject.index - 1;

        ExtRemoteData objectInfo(heapObject.objectInfoAddress, 1);
        heapObject.objectInfoBits = objectInfo.GetChar();
        DumpHeapObject(heapObject, verbose);
    }
    else
    {
        GetExtension()->Out("Pointer is not valid in this heap block\n");
    }
}


JD_PRIVATE_COMMAND(oi,
    "Display object's recycler state",
    "{v;b,o;verbose;dump verbose information}"
    "{;e,r;address;Address of object to dump}{;e,o,d=0;recycler;Recycler address}")
{
    const bool verbose = HasArg("v");
    ULONG64 objectAddress = GetUnnamedArgU64(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    ObjectInfoHelper ObjectInfoHelper(recycler.GetExtRemoteTyped());
    RemoteHeapBlock * remoteHeapBlock = recycler.GetHeapBlockMap().FindHeapBlock(objectAddress);
    if (remoteHeapBlock != NULL)
    {
        ULONG64 heapBlockType = remoteHeapBlock->GetType();

        ObjectInfoHelper.DumpHeapBlockLink(heapBlockType, remoteHeapBlock->GetHeapBlockAddress());
        
        if (heapBlockType == this->enum_LargeBlockType())
        {
            ObjectInfoHelper.DumpLargeHeapBlockObject(*remoteHeapBlock, objectAddress, verbose);
        }
        else
        {
            ObjectInfoHelper.DumpSmallHeapBlockObject(*remoteHeapBlock, objectAddress, verbose);
        }

        if (verbose)
        {
            Addresses * rootPointers = this->recyclerCachedData.GetRootPointers(recycler, &threadContext, GetStackTop());
            if (rootPointers->Contains(objectAddress))
            {
                Out("Is Root: true\n");
            }
            else
            {
                Out("Is Root: false\n");
            }
        }
    }
    else
    {
        Out("Object not found (no heap block)\n");
    }
}

void
EXT_CLASS_BASE::DisplayLargeHeapBlockInfo(ExtRemoteTyped& largeHeapBlock)
{
    largeHeapBlock.OutFullValue();

    // TODO
}

void
EXT_CLASS_BASE::DisplaySmallHeapBlockInfo(ExtRemoteTyped& smallHeapBlock, RemoteRecycler recycler)
{
    smallHeapBlock.OutFullValue();

    ExtRemoteTyped heapBlockType = smallHeapBlock.Field("heapBlockType");
    auto type = heapBlockType.GetChar();

    if (type == this->enum_SmallFinalizableBlockType())
    {
        Out("Small finalizable block\n");
    }
    else if (type == this->enum_SmallLeafBlockType())
    {
        Out("Small leaf block\n");
    }
    else if (type == this->enum_SmallNormalBlockType())
    {
        Out("Small normal block\n");
    }
#ifdef RECYCLER_WRITE_BARRIER
    else if (type == this->enum_SmallNormalBlockWithBarrierType())
    {
        Out("Small normal block (with SWB)\n");
    }
    else if (type == this->enum_SmallFinalizableBlockWithBarrierType())
    {
        Out("Small finalizable block (with SWB)\n");
    }
#endif
    else if (type == this->enum_MediumFinalizableBlockType())
    {
        Out("Medium finalizable block\n");
    }
    else if (type == this->enum_MediumLeafBlockType())
    {
        Out("Medium leaf block\n");
    }
    else if (type == this->enum_MediumNormalBlockType())
    {
        Out("Medium normal block\n");
    }
#ifdef RECYCLER_WRITE_BARRIER
    else if (type == this->enum_MediumNormalBlockWithBarrierType())
    {
        Out("Medium normal block (with SWB)\n");
    }
    else if (type == this->enum_MediumFinalizableBlockWithBarrierType())
    {
        Out("Medium finalizable block (with SWB)\n");
    }
#endif
    else
    {
        Out("Unexpected heapblock type: 0x%x\n", type);
        return;
    }

    ushort objectSize = smallHeapBlock.Field("objectSize").GetUshort();
    uint bucketIndex = (objectSize >> recycler.GetObjectAllocationShift()) - 1;
    uint objectGranularity = recycler.GetObjectGranularity();
    uint objectBitDelta = objectSize / objectGranularity;

    Out("Object size: %u\n", objectSize);
    Out("Object allocation shift: %u\n", recycler.GetObjectAllocationShift());
    Out("Bucket index: %u\n", bucketIndex);
    Out("Object bit delta: %u\n", objectBitDelta);

    ExtRemoteTyped validPointersMap(FillModuleAndMemoryNS(
        strstr(heapBlockType.GetSimpleValue(), "Medium") == nullptr ?
        "%s!%sHeapInfo::mediumAllocValidPointersMap" : "%s!%sHeapInfo::smallAllocValidPointersMap"
        ));

    ExtRemoteTyped invalidBitBuffersPtr;
    ExtRemoteTyped invalidBitBuffers;
    try
    {
        invalidBitBuffersPtr = validPointersMap.Field("invalidBitsBuffers");
        invalidBitBuffers = invalidBitBuffersPtr.Dereference();
    }
    catch (ExtRemoteException&)
    {
        invalidBitBuffers = validPointersMap.Field("invalidBitsData");
    }
    ExtRemoteTyped bitVector = invalidBitBuffers.ArrayElement(bucketIndex);
    ExtRemoteTyped dataPtr;
    uint wordCount = 2;
    if (bitVector.HasField("data"))
    {
        dataPtr = bitVector.Field("data");
        if (m_PtrSize == 4)
        {
            wordCount = 8;
        }
    }
    else
    {
        dataPtr = bitVector;
        wordCount = dataPtr.GetTypeSize() / dataPtr.ArrayElement(0).GetTypeSize();
    }

    Out("Invalid bit vector: ");
    for (uint i = 0; i < wordCount; i++)
    {
        ExtRemoteTyped word = dataPtr.ArrayElement(i).Field("word");

        if (m_PtrSize == 4)
        {
            Out("%p ", word.GetUlong());
        }
        else
        {
            Out("%p ", word.GetUlong64());
        }
    }

    Out("\n");
}

// GC Debugger commands
JD_PRIVATE_COMMAND(recycler,
    "Dumps the given recycler or the recycler in the current thread context",
    "{;e,o,d=0;recycler;Recycler address}{webjd;b;;Output in WebJD format}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    RemoteRecycler recycler = GetRecycler(arg);
    BOOL webJd = HasArg("webjd");

    if (webJd)
    {
        Out("%p", recycler.GetPtr());
    }
    else
    {
        recycler.GetExtRemoteTyped().OutFullValue();
    }
}

JD_PRIVATE_COMMAND(showblockinfo,
    "Show heap block information",
    "{;e,r;address;Address of the heap block}{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    RemoteRecycler recycler = GetRecycler(arg);

    if (address != NULL)
    {
        JDRemoteTyped heapBlock = JDRemoteTyped::FromPtrWithVtable(address);
        ExtRemoteTyped heapBlockType = heapBlock.Field("heapBlockType");
        auto type = heapBlockType.GetChar();

        if (type == this->enum_LargeBlockType())
        {
            DisplayLargeHeapBlockInfo(heapBlock);
        }
        else
        {
            DisplaySmallHeapBlockInfo(heapBlock, recycler);
        }
    }
}

JD_PRIVATE_COMMAND(findblock,
    "Find recycler heap block",
    "{;e,r;address;Address to find heap block for}{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    RemoteRecycler recycler = GetRecycler(arg);
    
    RemoteHeapBlockMap hbm = recycler.GetHeapBlockMap();
    RemoteHeapBlock *  remoteHeapBlock = hbm.FindHeapBlock(address);

    if (remoteHeapBlock != NULL)
    {
        JDRemoteTyped(remoteHeapBlock->GetExtRemoteTyped()).CastWithVtable().OutFullValue();
    }
    else
    {
        Out("Could not find heap block corresponding to address\n");
    }
}



void EXT_CLASS_BASE::DisplaySegmentList(PCSTR strListName, ExtRemoteTyped segmentList, PageAllocatorStats& stats, CommandOutputType outputType, bool pageSegment)
{
    ULONG64 segmentListAddress = segmentList.GetPointerTo().GetPtr();
    PCSTR segmentType = (pageSegment ? GetPageSegmentType() : GetSegmentType());

    char qualifiedSegmentType[512];
    sprintf_s(qualifiedSegmentType, "%s%s", GetMemoryNS(), segmentType);
    RemoteListIterator<false> pageSegmentListIterator(qualifiedSegmentType, segmentListAddress);

    ULONG64 totalSize = 0;
    ULONG64 count = 0;

    if (outputType == CommandOutputType::NormalOutputType ||
        outputType == CommandOutputType::VerboseOutputType)
    {
        Out("%s\n", strListName);
    }

    while (pageSegmentListIterator.Next())
    {
        count++;
        ExtRemoteTyped segment = pageSegmentListIterator.Data();
        ULONG64 addressOfSegment = pageSegmentListIterator.GetDataPtr();

        ULONG64 address = segment.Field("address").GetPtr();
        ULONG64 segmentSize = ExtRemoteTypedUtil::GetSizeT(segment.Field("segmentPageCount")) * g_Ext->m_PageSize;
        totalSize += segmentSize;

        if (outputType == CommandOutputType::NormalOutputType ||
            outputType == CommandOutputType::VerboseOutputType)
        {
            PCSTR fullyQualifiedSegmentType = FillModuleV("%s!%s", GetModuleName(), qualifiedSegmentType);
            PCSTR segmentStr = pageSegment ? "PageSegment" : "Segment";
            if (PreferDML())
            {
                Dml("<link cmd=\"?? (%s*) 0x%p\">%s</link>: ",
                    fullyQualifiedSegmentType, addressOfSegment, segmentStr);
            }
            else
            {
                Out("%s /*\"?? (%s*) 0x%p\" to display*/: ",
                    segmentStr, fullyQualifiedSegmentType, addressOfSegment);
            }
            Out("(%p - %p)\n", address, address + segmentSize);
        }
    }

    if (outputType == CommandOutputType::SummaryOutputType)
    {
        Out("%s: %11I64u %11I64u\n", strListName, count, totalSize);
    }
    stats.count += count;
    stats.totalByteCount += totalSize;
}

void EXT_CLASS_BASE::DisplayPageAllocatorInfo(JDRemoteTyped pageAllocator, CommandOutputType outputType)
{
    pageAllocator = pageAllocator.CastWithVtable();
    Out("Page Allocator: 0x%x\n", pageAllocator.m_Offset);

    if (outputType == CommandOutputType::SummaryOutputType)
    {
        Out("-----------------------------------------\n");
        Out("Type                     Count       Size\n");
        Out("-----------------------------------------\n");
    }

    PageAllocatorStats stats = { 0 };
    DisplaySegmentList("Segments        ", pageAllocator.Field("segments"), stats, outputType);
    DisplaySegmentList("FullSegments    ", pageAllocator.Field("fullSegments"), stats, outputType);
    DisplaySegmentList("EmptySegments   ", pageAllocator.Field("emptySegments"), stats, outputType);
    DisplaySegmentList("DecommitSegments", pageAllocator.Field("decommitSegments"), stats, outputType);
    DisplaySegmentList("LargeSegments   ", pageAllocator.Field("largeSegments"), stats, outputType);
    if (outputType == CommandOutputType::SummaryOutputType)
    {
        Out("----------------------------------------\n");
        Out("Total           : %11I64u %11I64u\n", stats.count, stats.totalByteCount);
    }

    if (pageAllocator.HasField("zeroPageQueue"))
    {
        ExtRemoteTyped zeroPageQueue = pageAllocator.Field("zeroPageQueue");
        int count = 0;
        int pageCount = 0;
        ULONG64 freePageList = zeroPageQueue.Field("freePageList").GetPointerTo().GetPtr();
        ULONG64 alignment = 0;

        if (m_PtrSize == 8)
        {
            alignment = 8;
        }

        if (freePageList != 0)
        {
            RemoteListIterator<true> iter(this->FillModuleV("%s::FreePageEntry", this->GetPageAllocatorType()), freePageList + alignment);

            while (iter.Next())
            {
                // Operating on the data ptr directly because the symbol for PageAllocator::FreePageEntry
                // includes slist header which is not we want (we want just the data, not the list metadata)
                ULONG64 offsetOfPageCount = iter.GetDataPtr() + this->m_PtrSize;
                ExtRemoteData pageCountData(offsetOfPageCount, sizeof(uint));

                ulong pageCountValue = pageCountData.GetUlong();

                pageCount += pageCountValue;
                count++;
            }
        }

        Out("\nFree page list: %d entries (%d bytes)\n", count, (pageCount * g_Ext->m_PageSize));

        count = 0;
        pageCount = 0;
        ULONG64 pendingZeroPageList = zeroPageQueue.Field("pendingZeroPageList").GetPointerTo().GetPtr();

        if (pendingZeroPageList != 0)
        {
            RemoteListIterator<true> iter(this->FillModuleV("%s::FreePageEntry", this->GetPageAllocatorType()), pendingZeroPageList + alignment);

            while (iter.Next())
            {
                ULONG64 offsetOfPageCount = iter.GetDataPtr() + this->m_PtrSize;
                ExtRemoteData pageCountData(offsetOfPageCount, sizeof(uint));

                ulong pageCountValue = pageCountData.GetUlong();

                pageCount += pageCountValue;
                count++;
            }
        }
        Out("Pending Zero page list: %d entries (%d bytes)\n", count, (pageCount * g_Ext->m_PageSize));
    }
}

JD_PRIVATE_COMMAND(pagealloc,
    "Display information about a page allocator",
    "{;e,o,d=0;alloc;Page allocator address}"
    "{recycler;b;recycler;Display the recycler's page allocator}"
    "{webjd;b;;Output in WebJD format}"
    "{summary;b;;Display just a summary}"
    "{verbose;b;;Display verbose output}")
{
    if (!HasUnnamedArg(0) && !HasArg("recycler"))
    {
        Out("Usage: !pagealloc [page allocator address] | !pagealloc -recycler [recycler address]\n");
        return;
    }

    ExtRemoteTyped pageAllocator;

    char pageAllocatorFormatString[128];

    sprintf_s(pageAllocatorFormatString, "(%%s!%%s%s*)@$extin", this->GetPageAllocatorType());

    CommandOutputType outputType = NormalOutputType;

    if (HasArg("summary"))
    {
        outputType = SummaryOutputType;
    }
    else if (HasArg("webjd"))
    {
        outputType = WebJdOutputType;
    }
    else if (HasArg("verbose"))
    {
        outputType = VerboseOutputType;
    }

    if (HasArg("recycler"))
    {
        ExtRemoteTyped recycler;
        if (HasUnnamedArg(0) && GetUnnamedArgU64(0) != 0)
        {
            Out("Recycler is 0x%p\n", GetUnnamedArgU64(0));
            recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), GetUnnamedArgU64(0));
        }
        else
        {
            recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
        }

        char* allocators[] = {
            "recyclerPageAllocator",
            "recyclerWithBarrierPageAllocator",
            "recyclerLargeBlockPageAllocator"
        };

        for (int i = 0; i < _countof(allocators); i++)
        {
            Out("\n\nType: %s\n", allocators[i]);
            DisplayPageAllocatorInfo(recycler.Field(allocators[i]), outputType);
        }
    }
    else
    {
        Out("Allocator is 0x%p\n", GetUnnamedArgU64(0));

        pageAllocator = ExtRemoteTyped(FillModuleAndMemoryNS(pageAllocatorFormatString), GetUnnamedArgU64(0));
        DisplayPageAllocatorInfo(pageAllocator, outputType);
    }
}

#pragma region("Heap Block Map Walker")

JD_PRIVATE_COMMAND(hbm,
    "Display information about the heap block map",
    "{recycler;e,o,d=0;recycler;Recycler address}"
    "{webjd;b;;Output in WebJD format}"
    "{match;x,o;;Command to filter against}"
    "{exec;x,o;;Command to execute}")
{
    ULONG64 recyclerAddress = GetArgU64("recycler");
    RemoteRecycler recycler = GetRecycler(recyclerAddress);

    PCSTR execStr = nullptr;
    bool filter = false;
    if (HasArg("match"))
    {
        execStr = GetArgStr("match");
        filter = true;
    }
    else if (HasArg("exec"))
    {
        execStr = GetArgStr("exec");
    }
    int largeCount = 0;
    int smallCount = 0;
    ULONG64 largeBlockBytes = 0;
    ULONG64 smallBlockBytes = 0;
    AutoCppExpressionSyntax cppSyntax(m_Control5);

    RemoteHeapBlockMap hbm = recycler.GetHeapBlockMap();

    hbm.ForEachHeapBlock([&](RemoteHeapBlock& remoteHeapBlock)
    {
        if (remoteHeapBlock.IsLargeHeapBlock())
        {
            largeBlockBytes += remoteHeapBlock.GetSize();
            largeCount++;
        }
        else
        {
            smallBlockBytes += remoteHeapBlock.GetSize();
            smallCount++;
        }

        if (execStr != nullptr)
        {
            CHAR    buffer[20] = "0x";
            std::string cmd = execStr;

            _ui64toa_s(remoteHeapBlock.GetHeapBlockAddress(), buffer + 2, _countof(buffer) - 2, 16);
            ReplacePlaceHolders("%1", buffer, cmd);

            DEBUG_VALUE value = { 0 };
            ULONG rem;

            this->ThrowInterrupt();
            if (filter)
            {
                if (SUCCEEDED(this->m_Control->Evaluate(cmd.c_str(), DEBUG_VALUE_INT64, &value, &rem) != 0) &&
                    value.I64 != 0)
                {
                    this->Out("Matched: 0x%x\n", remoteHeapBlock.GetHeapBlockAddress());
                }
            }
            else
            {
                this->m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS, cmd.c_str(), 0);
            }
        }

        return false;
    });

    if (!HasArg("exec") && !HasArg("match"))
    {
        this->Out("----------------------------\n");
        this->Out("Type     Count       Bytes\n");
        this->Out("----------------------------\n");
        this->Out("Large: %7u %11I64u\n", largeCount, largeBlockBytes);
        this->Out("Small: %7u %11I64u\n", smallCount, smallBlockBytes);
        this->Out("----------------------------\n");
        this->Out("Total: %7u %11I64u\n", smallCount + largeCount, largeBlockBytes + smallBlockBytes);
    }
}

JD_PRIVATE_COMMAND(swb,
    "Find the software write barrier bit for an address",
    "{;e,r;address;Address of object to look up}")
{
    ULONG64 objectAddress = GetUnnamedArgU64(0);

    if (this->m_PtrSize == 4) {
        Nullable<bool> usesSoftwareWriteBarrier;

        DetectFeatureBySymbol(usesSoftwareWriteBarrier, FillModuleAndMemoryNS("%s!%sRecyclerWriteBarrierManager::cardTable"));

        if (usesSoftwareWriteBarrier == false) {
            Out("Target process is not using Software Write Barrier (or symbols are incorrect)\n");
            return;
        }

        ExtRemoteTyped cardTable = ExtRemoteTyped(FillModuleAndMemoryNS("%s!%sRecyclerWriteBarrierManager::cardTable"));
        ULONG64 bytesPerCardOffset = 0;

        if (FAILED(this->m_Symbols->GetOffsetByName(FillModuleAndMemoryNS("%s!%sRecyclerWriteBarrierManager::s_BytesPerCard"), &bytesPerCardOffset)))
        {
            Out("Error resolving RecyclerWriteBarrierManager::s_BytesPerCard\n");
            return;
        }

        uint bytesPerCard = 0;
        ULONG bytesRead = 0;
        HRESULT hr = S_FALSE;
        if (FAILED(hr = this->m_Data->ReadVirtual(bytesPerCardOffset, (void*)&bytesPerCard, sizeof(bytesPerCard), &bytesRead)))
        {
            Out("Failed to read from 0x%p, HR = 0x%x\n", bytesPerCardOffset, hr);
            return;
        }

        uint cardIndex = ((uint)objectAddress) / bytesPerCard;
        BYTE value = cardTable.ArrayElement(cardIndex).GetChar();

        Out("Card Index: %d\nDirty: %d\n", cardIndex, value);
    }
    else
    {
        Out("Write barriers not implemented for 64 bit yet\n");
    }
}

struct RecyclerBucketStats
{
    ULONG64 count;
    ULONG64 emptyCount;
    ULONG64 finalizeBlockCount;
    ULONG64 objectCount;
    ULONG64 finalizeCount;
    ULONG64 objectByteCount;
    ULONG64 totalByteCount;

    void Merge(RecyclerBucketStats const& current)
    {
        count += current.count;
        emptyCount += current.emptyCount;
        finalizeCount += current.finalizeCount;
        objectCount += current.objectCount;
        objectByteCount += current.objectByteCount;
        totalByteCount += current.totalByteCount;
    }

    void Out()
    {
        GetExtension()->Out("%5I64u %7I64u %7I64u %11I64u %11I64u %11I64u   %6.2f%%",
            count, objectCount, finalizeCount,
            objectByteCount, totalByteCount - objectByteCount, totalByteCount,
            100.0 * (static_cast<double>(objectByteCount) / totalByteCount));
    }
};

typedef std::pair<char, ULONG> HeapBlockTypeSizePair;
typedef std::pair<HeapBlockTypeSizePair, RecyclerBucketStats> RecyclerBucketInfo;
int __cdecl RecyclerBucketInfoComparer(void const * a, void const * b)
{
    auto infoA = (RecyclerBucketInfo const*)a;
    auto infoB = (RecyclerBucketInfo const*)b;
    if (infoA->first.second == infoB->first.second)
    {
        return infoA->first.first - infoB->first.first;
    }
    return infoA->first.second < infoB->first.second ? -1 : 1;

}

namespace stdext
{
    // for stdext::hash_map
    template<> size_t hash_value(const HeapBlockTypeSizePair& _Keyval)
    {
        // hash _Keyval to size_t value one-to-one
        return ((size_t)(_Keyval.first << 8 | _Keyval.second) ^ _HASH_SEED);
    }
}

JD_PRIVATE_COMMAND(hbstats,
    "Count recycler objects",
    "{;e,o,d=0;recycler;Recycler address}"
    "{ft;s,o;filter by type;Filter the output to particular bucket type (norm|fin|leaf|normwb|finwb) or combination (wb|nwb)}"
    "{fs;edn=(10),o,d=0;filter by size;Filter the output to particular bucket size}"
    "{fc;s,o;filter by size class;Filter the output to particular size class (small|medium|large)}"
    "{noheader;b,o;no header;Do not display header}"
    )
{
    ULONG64 recyclerArg = GetUnnamedArgU64(0);
    enum FilterType
    {
        ShowSmallBlock = 0x1,
        ShowMediumBlock = 0x2,
        ShowLargeBlock = 0x4,

        ShowNormalBlock = 0x8,
        ShowLeafBlock = 0x10,
        ShowFinalizableBlock = 0x20,
        ShowNormalWriteBarrierBlock = 0x40,
        ShowFinalizableWriteBarrierBlock = 0x80,

        ShowAnyNonWriteBarrierBlock = ShowNormalBlock | ShowLeafBlock | ShowFinalizableBlock,
        ShowAnyWriteBarrierBlock = ShowNormalWriteBarrierBlock | ShowFinalizableWriteBarrierBlock,

        ShowAnySizeClass = ShowSmallBlock | ShowMediumBlock | ShowLargeBlock,
        ShowAnyType = ShowAnyNonWriteBarrierBlock | ShowAnyWriteBarrierBlock,

        ShowAnyBlock = ShowAnySizeClass | ShowAnyType
    } filterType = ShowAnyBlock;
    if (this->HasArg("ft"))
    {
        PCSTR arg = this->GetArgStr("ft");

        if (_stricmp("norm", arg) == 0)
        {
            filterType = (FilterType)(ShowNormalBlock | ShowAnySizeClass);
        }
        else if (_stricmp("fin", arg) == 0)
        {
            filterType = (FilterType)(ShowFinalizableBlock | ShowAnySizeClass);
        }
        else if (_stricmp("leaf", arg) == 0)
        {
            filterType = (FilterType)(ShowLeafBlock | ShowAnySizeClass);
        }
        else if (_stricmp("normwb", arg) == 0)
        {
            filterType = (FilterType)(ShowNormalWriteBarrierBlock | ShowAnySizeClass);
        }
        else if (_stricmp("finwb", arg) == 0)
        {
            filterType = (FilterType)(ShowFinalizableWriteBarrierBlock | ShowAnySizeClass);
        }
        else if (_stricmp("wb", arg) == 0)
        {
            filterType = (FilterType)(ShowAnyWriteBarrierBlock | ShowAnySizeClass);
        }
        else if (_stricmp("nwb", arg) == 0)
        {
            filterType = (FilterType)(ShowAnyNonWriteBarrierBlock | ShowAnySizeClass);
        }
        else
        {
            Out("Invalid argument for -ft: %s\n", arg);
            return;
        }
    }

    if (this->HasArg("fc"))
    {
        PCSTR arg = this->GetArgStr("fc");

        if (_stricmp("small", arg) == 0)
        {
            filterType = (FilterType)(ShowSmallBlock | (filterType & ShowAnyType));
        }
        else if (_stricmp("medium", arg) == 0)
        {
            filterType = (FilterType)(ShowMediumBlock | (filterType & ShowAnyType));
        }
        else if (_stricmp("large", arg) == 0)
        {
            filterType = (FilterType)(ShowLargeBlock | (filterType & ShowAnyType));
        }        
        else
        {
            Out("Invalid argument for -fc: %s\n", arg);
            return;
        }
    }

    ULONG64 filterSize = this->GetArgU64("fs");

    RemoteRecycler recycler = GetRecycler(recyclerArg);    
    RemoteHeapBlockMap hbm = recycler.GetHeapBlockMap();

    stdext::hash_map<HeapBlockTypeSizePair, RecyclerBucketStats> statsMap;
    RecyclerBucketStats totalStats = { 0 };

    hbm.ForEachHeapBlock([&](RemoteHeapBlock& remoteHeapBlock)
    {
        ULONG bucketObjectSize = remoteHeapBlock.GetBucketObjectSize();
        if (filterSize != 0 && filterSize != bucketObjectSize)
        {
            return false;
        }

        if (remoteHeapBlock.IsSmallNormalHeapBlock())
        {
            if ((filterType & (ShowSmallBlock | ShowNormalBlock)) != (ShowSmallBlock | ShowNormalBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsSmallLeafHeapBlock())
        {
            if ((filterType & (ShowSmallBlock | ShowLeafBlock)) != (ShowSmallBlock | ShowLeafBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsSmallFinalizableHeapBlock())
        {
            if ((filterType & (ShowSmallBlock | ShowFinalizableBlock)) != (ShowSmallBlock | ShowFinalizableBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsSmallNormalWithBarrierHeapBlock())
        {
            if ((filterType & (ShowSmallBlock | ShowNormalWriteBarrierBlock)) != (ShowSmallBlock | ShowNormalWriteBarrierBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsSmallFinalizableWithBarrierHeapBlock())
        {
            if ((filterType & (ShowSmallBlock | ShowFinalizableWriteBarrierBlock)) != (ShowSmallBlock | ShowFinalizableWriteBarrierBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsMediumNormalHeapBlock())
        {
            if ((filterType & (ShowMediumBlock | ShowNormalBlock)) != (ShowMediumBlock | ShowNormalBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsMediumLeafHeapBlock())
        {
            if ((filterType & (ShowMediumBlock | ShowLeafBlock)) != (ShowMediumBlock | ShowLeafBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsMediumFinalizableHeapBlock())
        {
            if ((filterType & (ShowMediumBlock | ShowFinalizableBlock)) != (ShowMediumBlock | ShowFinalizableBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsMediumNormalWithBarrierHeapBlock())
        {
            if ((filterType & (ShowMediumBlock | ShowNormalWriteBarrierBlock)) != (ShowMediumBlock | ShowNormalWriteBarrierBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsMediumFinalizableWithBarrierHeapBlock())
        {
            if ((filterType & (ShowMediumBlock | ShowFinalizableWriteBarrierBlock)) != (ShowMediumBlock | ShowFinalizableWriteBarrierBlock))
            {
                return false;
            }
        }
        else if (remoteHeapBlock.IsLargeHeapBlock())
        {
            if ((filterType & ShowLargeBlock) != ShowLargeBlock)
            {
                return false;
            }
        }
        else
        {
            Assert(false);
        }

        RecyclerBucketStats* stats = nullptr;
        auto key = HeapBlockTypeSizePair(remoteHeapBlock.GetType(), bucketObjectSize);
        auto i = statsMap.find(key);
        if (i == statsMap.end())
        {
            stats = &(statsMap[key] = { 0 });
        }
        else
        {
            stats = &(*i).second;
        }

        stats->count++;
        stats->totalByteCount += remoteHeapBlock.GetSize();
        stats->finalizeCount += remoteHeapBlock.GetFinalizeCount();
        stats->objectCount += remoteHeapBlock.GetAllocatedObjectCount();
        stats->objectByteCount += remoteHeapBlock.GetAllocatedObjectSize();
        return false;
    });

    std::auto_ptr<RecyclerBucketInfo> sortedArray(new RecyclerBucketInfo[statsMap.size()]);
    int c = 0;
    for (auto i = statsMap.begin(); i != statsMap.end(); i++)
    {
        sortedArray.get()[c++] = (*i);
    }
    qsort(sortedArray.get(), c, sizeof(RecyclerBucketInfo), RecyclerBucketInfoComparer);

    if (!this->HasArg("noheader"))
    {
        this->Out("Bucket stats Recycler: %p", recycler.GetPtr());
        if (recycler.IsPageHeapEnabled())
        {
            this->Out(" (Page Heap Enabled)");
        }
        this->Out("\n");
        this->Out("---------------------------------------------------------------------------------------\n");
        this->Out("                  #Blk   #Objs    #Fin      PgBytes   FreeBytes  TotalBytes UsedPercent\n");
        this->Out("---------------------------------------------------------------------------------------\n");
    }

    for (int i = 0; i < c; i++)
    {
        RecyclerBucketInfo& info = sortedArray.get()[i];

        if (info.second.count == 0)
        {
            continue;
        }

        totalStats.Merge(info.second);

        if (filterSize != 0 && filterSize != info.first.second)
        {
            continue;
        }

        char const * name = "";
        char const * format = "%-9s %4d : ";
        char type = info.first.first;
        if (type == this->enum_SmallNormalBlockType())
        {
            Assert((filterType & (ShowSmallBlock | ShowNormalBlock)) == (ShowSmallBlock | ShowNormalBlock));
            name = "Normal (S)";
        }
        else if (type == this->enum_SmallLeafBlockType())
        {
            Assert((filterType & (ShowSmallBlock | ShowLeafBlock)) == (ShowSmallBlock | ShowLeafBlock));
            name = "Leaf   (S)";
        }
        else if (type == this->enum_SmallFinalizableBlockType())
        {
            Assert((filterType & (ShowSmallBlock | ShowFinalizableBlock)) == (ShowSmallBlock | ShowFinalizableBlock));
            name = "Fin    (S)";
        }
        else if (type == this->enum_SmallNormalBlockWithBarrierType())
        {
            Assert((filterType & (ShowSmallBlock | ShowNormalWriteBarrierBlock)) == (ShowSmallBlock | ShowNormalWriteBarrierBlock));
            name = "NormWB (S)";
        }
        else if (type == this->enum_SmallFinalizableBlockWithBarrierType())
        {
            Assert((filterType & (ShowSmallBlock | ShowFinalizableWriteBarrierBlock)) == (ShowSmallBlock | ShowFinalizableWriteBarrierBlock));
            name = "FinWB  (S)";
        }
        else if (type == this->enum_MediumNormalBlockType())
        {
            Assert((filterType & (ShowMediumBlock | ShowNormalBlock)) == (ShowMediumBlock | ShowNormalBlock));
            name = "Normal (M)";
        }
        else if (type == this->enum_MediumLeafBlockType())
        {
            Assert((filterType & (ShowMediumBlock | ShowLeafBlock)) == (ShowMediumBlock | ShowLeafBlock));
            name = "Leaf   (M)";
        }
        else if (type == this->enum_MediumFinalizableBlockType())
        {
            Assert((filterType & (ShowMediumBlock | ShowFinalizableBlock)) == (ShowMediumBlock | ShowFinalizableBlock));
            name = "Fin    (M)";
        }
        else if (type == this->enum_MediumNormalBlockWithBarrierType())
        {
            Assert((filterType & (ShowMediumBlock | ShowNormalWriteBarrierBlock)) == (ShowMediumBlock | ShowNormalWriteBarrierBlock));
            name = "NormWB (M)";
        }
        else if (type == this->enum_MediumFinalizableBlockWithBarrierType())
        {
            Assert((filterType & (ShowMediumBlock | ShowFinalizableWriteBarrierBlock)) == (ShowMediumBlock | ShowFinalizableWriteBarrierBlock));
            name = "FinWB  (M)";
        }
        else if (type == this->enum_LargeBlockType())
        {
            Assert((filterType & ShowLargeBlock) == ShowLargeBlock);
            name = "Large     ";
            format = "%-9s      : ";
        }
        else
        {
            Assert(false);
        }

        this->Out(format, name, info.first.second);
        info.second.Out();
        this->Out("\n");
    }

    if (c > 1)
    {
        this->Out("-----------------------------------------------------------------------------------------\n");
        this->Out("Total           : ");
        totalStats.Out();
        this->Out("\n");

        this->Out("\n");
    }
}

void DisplayArenaAllocatorDataHeader()
{
    ExtOut("-------------------------------------------------------------------------------------\n");
    ExtOut("Allocator     #Block       Total        Used      Unused    Overhead OverHead%% Unused%%\n");
    ExtOut("-------------------------------------------------------------------------------------\n");
}

struct ArenaAllocatorData
{
    ULONG64 blockSize;
    ULONG64 blockUsedSize;
    ULONG64 overheadSize;
    uint blockCount;
};

void AccumulateArenaAllocatorData(ExtRemoteTyped arenaAllocator, ArenaAllocatorData& data)
{
    ULONG64 sizeofBigBlockHeader = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sBigBlock))"));
    ULONG64 sizeofArenaMemoryBlockHeader = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sArenaMemoryBlock))"));
    auto arenaMemoryBlockFn = [&](ExtRemoteTyped mallocBlock)
    {
        data.overheadSize += sizeofArenaMemoryBlockHeader;
        data.blockCount++;
        ULONG64 nbytes = ExtRemoteTypedUtil::GetSizeT(mallocBlock.Field("nbytes"));
        data.blockSize += nbytes;
        data.blockUsedSize += nbytes;
        return false;
    };
    auto bigBlockFn = [&](ExtRemoteTyped bigBlock)
    {
        data.overheadSize += sizeofBigBlockHeader;
        data.blockCount++;
        data.blockSize += ExtRemoteTypedUtil::GetSizeT(bigBlock.Field("nbytes"));
        data.blockUsedSize += ExtRemoteTypedUtil::GetSizeT(bigBlock.Field("currentByte"));
        return false;
    };
    ExtRemoteTyped firstBigBlock = arenaAllocator.Field("bigBlocks");
    if (firstBigBlock.GetPtr())
    {
        data.blockUsedSize +=
            arenaAllocator.Field("cacheBlockCurrent").GetPtr() - firstBigBlock.GetPtr() - sizeofBigBlockHeader
            - ExtRemoteTypedUtil::GetSizeT(firstBigBlock.Field("currentByte"));
    }
    ExtRemoteTypedUtil::LinkListForEach(firstBigBlock, "nextBigBlock", bigBlockFn);
    ExtRemoteTypedUtil::LinkListForEach(arenaAllocator.Field("fullBlocks"), "nextBigBlock", bigBlockFn);
    ExtRemoteTypedUtil::LinkListForEach(arenaAllocator.Field("mallocBlocks"), "next", arenaMemoryBlockFn);
}

void DisplayArenaAllocatorData(char const * name, ExtRemoteTyped arenaAllocator, bool showZeroEntries)
{
    ArenaAllocatorData data = { 0 };
    AccumulateArenaAllocatorData(arenaAllocator, data);

    if (showZeroEntries || data.blockCount != 0)
    {
        ULONG64 unused = data.blockSize - data.blockUsedSize;
        ULONG64 totalSize = data.blockSize + data.overheadSize;
        ExtOut("%-15s: %3u %11I64u %11I64u %11I64u %11I64u %6.2f%% %6.2f%%\n", name, data.blockCount, totalSize, data.blockUsedSize, unused, data.overheadSize,
            100.0 * (data.overheadSize / (double)data.blockSize), 100.0 * (unused / (double)data.blockSize));
    }
}

void DisplayArenaAllocatorPtrData(char const * name, ExtRemoteTyped arenaAllocator, bool showZeroEntries)
{
    if (arenaAllocator.GetPtr() == 0) { return; }
    DisplayArenaAllocatorData(name, arenaAllocator, showZeroEntries);
}

JD_PRIVATE_COMMAND(memstats,
    "All memory stats",
    "{all;b,o;;Display all information}"
    "{a;b,o;;Display arena allocator information}"
    "{p;b,o;;Display page allocator information}"
    "{t;e,o,d=0;Display information for thread context}"
    "{z;b,o;;Display zero entries}")
{
    ULONG64 threadContextAddress = GetArgU64("t");
    bool showZeroEntries = this->HasArg("z");
    bool showAll = this->HasArg("all");
    bool showArenaAllocator = showAll || this->HasArg("a");
    bool showPageAllocator = showAll || this->HasArg("p") || (threadContextAddress && !showArenaAllocator);
    bool showThreadSummary = (!showArenaAllocator && !showPageAllocator);
    ULONG numThreads = 0;
    ULONG64 totalReservedBytes = 0;
    ULONG64 totalCommittedBytes = 0;
    ULONG64 totalUsedBytes = 0;
    ULONG64 totalUnusedBytes = 0;
    if (showThreadSummary || !threadContextAddress)
    {
        ExtRemoteTyped remoteTotalUsedBytes(this->FillModule("%s!totalUsedBytes"));
        this->Out("Page Allocator Total Used Bytes: %u\n", ExtRemoteTypedUtil::GetSizeT(remoteTotalUsedBytes));
    }
    if (showThreadSummary)
    {
        RemotePageAllocator::DisplayDataHeader("Thread Context");
    }
    RemoteThreadContext::ForEach([=, &numThreads, &totalReservedBytes, &totalCommittedBytes, &totalUsedBytes, &totalUnusedBytes](RemoteThreadContext threadContext)
    {
        numThreads++;

        ULONG64 threadContextPtr = threadContext.GetPtr();
        if (threadContextAddress && threadContextAddress != threadContextPtr)
        {
            return false;  // continue iterating
        }

        if (showPageAllocator || showArenaAllocator)
        {
            ulong threadContextThreadId = 0;
            if (threadContext.TryGetDebuggerThreadId(&threadContextThreadId))
            {
                if (PreferDML())
                {
                    Dml("Thread context: %p <link cmd=\"~%us\">(switch to thread)</link>\n", threadContextPtr, threadContextThreadId);
                }
                else
                {
                    Out("Thread context: %p (switch to thread: \"~%us\")\n", threadContextPtr, threadContextThreadId);
                }
            }
            else
            {
                Out("Thread context: %p\n", threadContextPtr);
            }
        }
        if (showPageAllocator)
        {
            Out("Page Allocators:\n");
            RemotePageAllocator::DisplayDataHeader("Allocator");
        }

        ULONG64 reservedBytes = 0;
        ULONG64 committedBytes = 0;
        ULONG64 usedBytes = 0;
        ULONG64 unusedBytes = 0;
        threadContext.ForEachPageAllocator([=, &reservedBytes, &committedBytes, &usedBytes, &unusedBytes](PCSTR name, RemotePageAllocator pageAllocator)
        {
            reservedBytes += pageAllocator.GetReservedBytes();
            committedBytes += pageAllocator.GetCommittedBytes();
            usedBytes += pageAllocator.GetUsedBytes();
            unusedBytes += pageAllocator.GetUnusedBytes();

            if (showPageAllocator)
            {
                pageAllocator.DisplayData(name, showZeroEntries);
            }
            return false;
        });

        totalReservedBytes += reservedBytes;
        totalCommittedBytes += committedBytes;
        totalUsedBytes += usedBytes;
        totalUnusedBytes += unusedBytes;

        if (showPageAllocator && !showThreadSummary)
        {
            RemotePageAllocator::DisplayDataLine();
        }
        if (showPageAllocator || showThreadSummary)
        {
            if (PreferDML())
            {
                Dml("<link cmd=\"!jd.memstats -t %p\">%016p</link>", threadContextPtr, threadContextPtr);
            }
            else
            {
                Out("/*\"!jd.memstats -t %p\" to display*/\n", threadContextPtr);
                Out("%016p", threadContextPtr);
            }
            RemotePageAllocator::DisplayData(16, usedBytes, reservedBytes, committedBytes, unusedBytes);
        }

        if (showArenaAllocator)
        {
            ExtRemoteTyped threadContextExtRemoteTyped = threadContext.GetExtRemoteTyped();
            this->Out("Arena Allocators:\n");
            DisplayArenaAllocatorDataHeader();
            DisplayArenaAllocatorData("TC", threadContextExtRemoteTyped.Field("threadAlloc"), showZeroEntries);
            DisplayArenaAllocatorData("TC-InlineCache", threadContextExtRemoteTyped.Field("inlineCacheThreadInfoAllocator"), showZeroEntries);
            if (threadContextExtRemoteTyped.HasField("isInstInlineCacheThreadInfoAllocator"))
            {
                // IE11 don't have this arena allocator
                DisplayArenaAllocatorData("TC-IsInstIC", threadContextExtRemoteTyped.Field("isInstInlineCacheThreadInfoAllocator"), showZeroEntries);
            }
            DisplayArenaAllocatorData("TC-ProtoChain", threadContextExtRemoteTyped.Field("prototypeChainEnsuredToHaveOnlyWritableDataPropertiesAllocator"), showZeroEntries);

            threadContext.ForEachScriptContext([showZeroEntries](RemoteScriptContext scriptContext)
            {
                scriptContext.ForEachArenaAllocator([showZeroEntries](char const * name, ExtRemoteTyped arenaAllocator)
                {
                    DisplayArenaAllocatorData(name, arenaAllocator, showZeroEntries);
                });               
                return false;
            });
        }

        return false; // Don't stop iterating
    });

    if (!showArenaAllocator && numThreads > 1)
    {
        RemotePageAllocator::DisplayDataLine();
        this->Out("Total");
        RemotePageAllocator::DisplayData(_countof("Total") - 1, totalUsedBytes, totalReservedBytes, totalCommittedBytes, totalUnusedBytes);
    }
}

typedef struct _OBJECTINFO
{
    enum
    {
        freed,
        rooted,
        unrooted
    } state = freed;
    ULONG64 heapEntry = 0;
    ULONG64 userPtr = 0;
    ULONG64 userSize = 0;
    UCHAR attributes = 0;
    JDRemoteTyped heapBlock;
    bool succeeded = false;
    std::string message;
    char const * typeName;
} OBJECTINFO;

OBJECTINFO GetObjectInfo(ULONG64 address, RemoteRecycler recycler)
{
    OBJECTINFO info;
    JDRemoteTyped& heapBlock = info.heapBlock;
    RemoteHeapBlock * remoteHeapBlock = recycler.GetHeapBlockMap().FindHeapBlock(address);
    if (remoteHeapBlock == nullptr)
    {
        info.message = "Could not find heap block corresponding to this address";
        return info;
    }
    heapBlock = remoteHeapBlock->GetExtRemoteTyped();

    char const *& typeName = info.typeName;
    heapBlock = heapBlock.CastWithVtable(&typeName);

    ExtRemoteTyped heapBlockType = heapBlock.Field("heapBlockType");
    auto typeEnumName = heapBlockType.GetSimpleValue();
    if (strstr(typeEnumName, "Large") == typeEnumName)
    {
        if (strstr(typeName, "LargeHeapBlock") == nullptr)
        {
            info.message = "not a valid large block.";
            return info;
        }

        ULONG64 headerListAddress = heapBlock.GetPtr() + heapBlock.GetTypeSize();
        ULONG objectCount = heapBlock.Field("objectCount").GetUlong();
        ExtRemoteTyped headerList(GetExtension()->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader**)@$extin"), headerListAddress);

        bool foundInfo = false;
        ExtRemoteTyped header;
        for (ULONG i = 0; i < objectCount; i++)
        {
            header = headerList.ArrayElement(i);
            info.heapEntry = header.GetPtr();
            info.userPtr = info.heapEntry + header.GetTypeSize();
            info.userSize = GetExtension()->GetNumberValue<ULONG64>(header.Field("objectSize"));

            if (address >= info.userPtr  && address < info.userPtr + info.userSize)
            {
                ushort attributesAndChecksum = header.Field("attributesAndChecksum").GetUshort();
                info.attributes = (UCHAR)((attributesAndChecksum ^ (USHORT)recycler.GetCookie()) >> 8);
                foundInfo = true;
                break;
            }
        }

        if (!foundInfo)
        {
            info.message = "fatal error: target address not found on the large heap block object header list.";
            return info;
        }
    }
    else
    {
        auto blockAddress = heapBlock.Field("address").GetPtr();
        auto objectSize = GetExtension()->GetNumberValue<ULONG64>(heapBlock.Field("objectSize"));
        info.userPtr = address - ((address - blockAddress) % objectSize);
        ULONG64 index = (address - blockAddress) / objectSize;
        Assert(index < USHRT_MAX);

        if (strstr(typeEnumName, "Small") == typeEnumName)
        {
            if (strstr(typeName, "Small") == nullptr)
            {
                info.message = "not a valid small block.";
                return info;
            }
        }
        else if (strstr(typeEnumName, "Medium") == typeEnumName)
        {
            if (strstr(typeName, "Medium") == nullptr)
            {
                info.message = "not a valid Medium block.";
                return info;
            }
        }
        else
        {
            info.message = GetExtension()->FillModuleV("Can't handle HeapBlockType: %s", typeEnumName);
            return info;
        }

        info.attributes = ExtRemoteData(heapBlock.GetPtr() - index - 1, sizeof(info.attributes)).GetUchar();
        info.userSize = objectSize;
        info.heapEntry = blockAddress + index * info.userSize;
    }

    if ((info.attributes & RemoteObjectInfoBits::PendingDisposeBit))
    {
        info.state = info.freed;
    }
    else
    {
        if ((info.attributes & RemoteObjectInfoBits::ImplicitRootBit))
        {
            info.state = info.rooted;
        }
        else
        {
            info.state = info.unrooted;
        }
    }
    info.succeeded = true;
    return info;
};

void ShowStack(ExtRemoteTyped heapBlock, PCSTR stackType)
{
    // stackType is "Alloc" or "Free"
    std::string stackFieldName = GetExtension()->FillModuleV("pageHeap%sStack", stackType);
    if (!heapBlock.HasField(stackFieldName.c_str()))
    {
        GetExtension()->Out("Page heap %s stack trace is not supported\n", stackType);
        return;
    }
    ExtRemoteTyped stackField = heapBlock.Field(stackFieldName.c_str());
    std::string HeapBlockType = GetExtension()->FillModuleAndMemoryNS("%s!%sHeapBlock");
    char buffer[1024];
    if (stackField.GetPtr() != NULL)
    {
        sprintf_s(buffer, "%s", GetExtension()->FillModuleV("dps @@c++(((%s*)(0x%llx))->%s->stackBackTrace) L@@c++(((%s*)(0x%llx))->%s->framesCount)",
            HeapBlockType.c_str(), heapBlock.GetPtr(), stackFieldName.c_str(), HeapBlockType.c_str(), heapBlock.GetPtr(), stackFieldName.c_str()));
        if (GetExtension()->PreferDML())
        {
            GetExtension()->Dml("\t<b>%s</b> <link cmd=\"%s\">stack</link>:\n", stackType, buffer);
        }
        else
        {
            GetExtension()->Out("\t%s stack /*\"%s\" to display*/:\n", stackType, buffer);
        }

        ExtBuffer<char> symbol;
        ULONG64 displacement;
        auto stack = stackField.Field("stackBackTrace");
        for (ULONG i = 0; i < stackField.Field("framesCount").GetUlong(); i++)
        {
            GetExtension()->GetOffsetSymbol(stack.ArrayElement(i).GetPtr(), &symbol, &displacement);
            GetExtension()->Out("\t  %016I64x %s+0X%x\n", stack.ArrayElement(i).GetPtr(), symbol.GetBuffer(), displacement);
        }
    }
}

MPH_COMMAND(mpheap,
    "Memory protect heap",
    "{v;b,o;s;Verbose output}"
    "{s;b,o;s;Summary}"
    "{srch;b,o;srch;Search}{b;e8;o;DWORD}{w;e16;o;DWORD}{d;e32;o;DWORD}{q;e64;o;QWORD}"
    "{p;b,o;p;Specifies that page heap information is being requested}""{a;e64,o;a;Address}"
    "{isref;e64,o;isref;Find reference}"
    )
{
    MphCmdsWrapper::AutoMPHCmd autoMphCmd((MphCmdsWrapper*)this);
    bool verbose = HasArg("v");
    char buffer[1024];

    char* gpHeapHandle;
    if (HasType(tridentModule, "MemoryProtection::g_pHeapHandle")) 
    {
        gpHeapHandle = "MemoryProtection::g_pHeapHandle";
    }
    else if(HasType(tridentModule, "g_pHeapHandle"))
    {
        gpHeapHandle = "g_pHeapHandle";
    }
    else
    {
        this->Err("Can't find either %s!MemoryProtection::g_pHeapHandle or %s!:g_pHeapHandle", tridentModule, tridentModule);
        return;
    }

    ExtRemoteTyped g_pHeapHandle(FillModuleV("%s!%s", tridentModule, gpHeapHandle));
    if (verbose)
    {
        this->Out("%s!%s: %x\n", tridentModule, gpHeapHandle, g_pHeapHandle.GetPtr());
    }

    ExtRemoteTyped heapInstance(FillModuleV("(%s!MemProtectHeap*)(%s!%s)", memGCModule, tridentModule, gpHeapHandle));
    if (verbose)
    {
        if (PreferDML())
        {
            this->Dml("<link cmd=\"?? %s\">g_pHeapHandle</link>: %x\n", FillModuleV("(%s!MemProtectHeap*)(%s!%s)", memGCModule, tridentModule), heapInstance.GetPtr());
        }
        else
        {
            this->Out("g_pHeapHandle /*\"?? %s\" to display*/: %x\n", FillModuleV("(%s!MemProtectHeap*)(%s!%s)", memGCModule, tridentModule), heapInstance.GetPtr());
        }
        this->Out("threadContextTlsIndex: %x\n", heapInstance.Field("threadContextTlsIndex").GetUlong(), gpHeapHandle);
    }

    // list of thread contexts:
    if (strcmp(this->GetRawArgStr(), "") == 0)
    {
        ExtRemoteTyped next = heapInstance.Field("threadList").Field("head");
        if (next.GetUlongPtr() == NULL)
        {
            this->Out("threadList is not initialized yet.\n");
        }
        else
        {
            this->Out("threadId  memProtectHeap     Recycler\n");
            this->Out("-------------------------------------\n");
            do
            {
                auto threadId = next.Field("threadId").GetLong();
                auto memProtectHeapPtr = next.Field("memProtectHeap").GetPtr();
                auto recyclerPtr = next.Field("memProtectHeap").Field("recycler").GetPointerTo().GetPtr();
                this->Out("%8x     %11I64x  %11I64x\n", threadId, memProtectHeapPtr, recyclerPtr);
                next = next.Field("next");
            } while (next.GetPtr() != 0);
        }
        return;
    }

    // TODO: get heap instance from input parameter and check existance
    RemoteRecycler remoteRecycler(heapInstance.Field("recycler").GetPointerTo());
    ExtRemoteTyped recycler = remoteRecycler.GetExtRemoteTyped();
        
    // Summary
    if (HasArg("s"))
    {
        bool showZeroEntries = true;
        RemotePageAllocator::DisplayDataHeader("Allocator");
        remoteRecycler.ForEachPageAllocator("Thread", [showZeroEntries](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.DisplayData(name, showZeroEntries);
            return false;
        });
        return;
    }

    // Search
    if (HasArg("srch"))
    {
        byte len = 0;
        char type;
        ULONG64 pattern = 0;
        if (this->HasArg("b"))
        {
            len = 1;
            pattern = this->GetArgU64("b");
            type = 'b';
        }
        else if (this->HasArg("w"))
        {
            len = 2;
            pattern = this->GetArgU64("w");
            type = 'w';
        }
        else if (this->HasArg("d"))
        {
            len = 4;
            pattern = this->GetArgU64("d");
            type = 'd';
        }
        else if (this->HasArg("q"))
        {
            len = 8;
            pattern = this->GetArgU64("q");
            type = 'q';
        }
        else
        {
            this->Out("Unknown search pattern.\n");
        }

        std::map<ULONG64, ULONG> validAddresses;
        remoteRecycler.ForEachPageAllocator("Thread", [&](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.ForEachSegment([&](PCSTR segName, ExtRemoteTyped pageSegment)->bool {
                this->ThrowInterrupt();

                ULONG64 address = pageSegment.Field("address").GetPtr();
                ULONG pageCount = (ULONG)pageSegment.Field("segmentPageCount").GetPtr();
                ULONG leadingGuardPageCount = pageSegment.Field("leadingGuardPageCount").GetUlong();
                ULONG trailingGuardPageCount = pageSegment.Field("trailingGuardPageCount").GetUlong();
                ULONG validPageCount = pageCount - leadingGuardPageCount - trailingGuardPageCount;
                ULONG64 validAddress = address + leadingGuardPageCount * m_PageSize;

                if (validAddresses.find(validAddress) != validAddresses.end())
                {
                    this->Out("duplicate segment address found\n");
                    return false;
                }

                PCSTR searchcmd = FillModuleV(this->m_PtrSize == 8 ? "s -%c 0x%016I64x L?0x%x 0x%016I64x" : "s -%c 0x%08I64x L?0x%x 0x%08I64x",
                    type, validAddress, validPageCount * m_PageSize, pattern);
                if (verbose)
                {
                    if (PreferDML())
                    {
                        this->Dml("\nSegment: %p+<link cmd=\"%s\">%x</link>:\n", validAddress, searchcmd, validPageCount * m_PageSize);
                    }
                    else
                    {
                        this->Out("\nSegment: %p+%x /*\"%s\" to search*/:\n", validAddress, validPageCount * m_PageSize, searchcmd);
                    }
                }
                this->Execute(searchcmd);
                return false;
            });
            return false;
        });
        return;
    }

    // inspect address(-isref and -p -a)
    // TODO: combine -isref and -p -a
    ULONG64 address = 0;
    // -isref
    bool isref = HasArg("isref");
    if (isref)
    {
        address = GetArgU64("isref");
    }

    // -p -a
    if (isref || HasArg("p"))
    {
        if (HasArg("a"))
        {
            address = GetArgU64("a");
        }
        else if (!isref)
        {
            this->Out("Please specify address.\n");
            return;
        }

        if (verbose)
        {
            if (this->m_PtrSize == 8)
            {
                //!list -t chakra!HeapBlockMap64::Node.next -x ".if(poi(@$extret)==(%x>>0n32)){?? ((chakra!HeapBlockMap64::Node*)@$extret)->map}" @@c++(((chakra!MemProtectHeap*)(edgehtml!g_pHeapHandle))->recycler.heapBlockMap.list)
                sprintf_s(buffer, "!list -t %s!%sHeapBlockMap64::Node.next -x "
                    "\".if(poi(@$extret)==(0x%I64x>>0n32)){ ?? @@c++((((%s!%sHeapBlockMap64::Node*)@$extret)->map.map[(0x%I64x&0xffffffff)>>0x14]->map[(0x%I64x&0x000FF000)>>0xc]))}\""
                    " @@c++(((%s!MemProtectHeap*)(%s!%s))->recycler.heapBlockMap.list)",
                    memGCModule, this->GetMemoryNS(), address, memGCModule, this->GetMemoryNS(), address, address, memGCModule, tridentModule, gpHeapHandle);
            }
            else
            {
                //?? ((mshtml!MemProtectHeap*)(mshtml!g_pHeapHandle))->recycler.heapBlockMap.map[0x051e8130>>0x14]->map[(0x051e8130&0x000FF000)>>0xc]
                sprintf_s(buffer, "?? @@c++(((%s!MemProtectHeap*)(%s!%s))->recycler.heapBlockMap.map[0x%llx>>0x14]->map[(0x%llx&0x000FF000)>>0xc])",
                    memGCModule, tridentModule, gpHeapHandle, address, address);
            }
            this->Out("Command to show block: %s\n", buffer);
        }

        OBJECTINFO info = GetObjectInfo(address, recycler);
        if (!info.succeeded)
        {
            this->Err(info.message.c_str());
            this->Err("\n");
            return;
        }

        JDRemoteTyped heapBlock = info.heapBlock.CastWithVtable();
        std::string typeName = info.typeName;
        bool pageHeapOn = false;
        if (heapBlock.HasField("pageHeapMode"))
        {
            pageHeapOn = heapBlock.Field("pageHeapMode").GetLong() != 0;
        }

        if (m_PtrSize == 8)
        {
            sprintf_s(buffer, "dx (%s!Memory::HeapBlock*)0x%I64x", memGCModule, heapBlock.GetPtr());
        }
        else
        {
            sprintf_s(buffer, "dx (%s!Memory::HeapBlock*)0x%llx", memGCModule, heapBlock.GetPtr());
        }

        this->Out("\taddress %p found in\n", address);
        if (PreferDML())
        {
            this->Dml("\tHeapBlock @ <link cmd=\"%s\">%p</link>", buffer, heapBlock.GetPtr());
        }
        else
        {
            this->Out("\tHeapBlock @ %p /*\"%s\" to display*/", heapBlock.GetPtr(), buffer);
        }
        this->Out(" (page heap is %senabled)\n", !pageHeapOn ? "not " : "");

        const char* states[3] = { "freed", "rooted", "unrooted" };
        this->Out("\t   HEAP_ENTRY Attributes     UserPtr UserSize - State\n");
        this->Out("\t  %11I64x       %4x %11I64x     %4I64x   (%s)\n", info.heapEntry, info.attributes, info.userPtr, info.userSize, states[info.state]);

        if (isref)
        {
            if (info.state == info.freed)
            {
                return;
            }

            Addresses * rootPointers = this->recyclerCachedData.GetRootPointers(recycler, nullptr, GetStackTop());
            rootPointers->Map([&recycler, &verbose, &address](ULONG64 rootAddress)
            {
                GetExtension()->ThrowInterrupt();
                auto info = GetObjectInfo(rootAddress, recycler);
                char buffer[1024];
                sprintf_s(buffer, GetExtension()->m_PtrSize == 8 ? "0x%016I64x" : "0x%08I64x", rootAddress);
                if (verbose)
                {
                    if (GetExtension()->PreferDML())
                    {
                        GetExtension()->Dml("\tRoot:  <link cmd=\"!mpheap -p -a %s\">%s</link>",
                            buffer, buffer);
                    }
                    else
                    {
                        GetExtension()->Out("\tRoot:  %s /*\"!mpheap -p -a %s\" to display*/",
                            buffer, buffer);
                    }
                    if (info.succeeded)
                    {
                        if (GetExtension()->PreferDML())
                        {
                            GetExtension()->Dml("  +<link cmd=\"dp %s L0x%x\">0x%x</link>\n",
                                buffer, info.userSize / GetExtension()->m_PtrSize, info.userSize);
                        }
                        else
                        {
                            GetExtension()->Out("  +0x%x /*\"dp %s L0x%x\" to display*/\n",
                                info.userSize, buffer, info.userSize / GetExtension()->m_PtrSize);
                        }
                    }
                    else
                    {
                        GetExtension()->Out((info.message + "\n").c_str());
                    }
                }

                if (info.succeeded)
                {
                    ExtRemoteTyped rootMem("(void**)@$extin", rootAddress);
                    for (size_t i = 0; i < info.userSize / GetExtension()->m_PtrSize; i++)
                    {
                        auto data = rootMem.ArrayElement(i).GetPtr();
                        if ((ULONG64)data == address)
                        {
                            GetExtension()->Out("\t      Ref: %s +0x%x\n",
                                buffer, i*GetExtension()->m_PtrSize);
                        }
                    }
                }
            });

            return;
        }

        // page heap
        if (pageHeapOn)
        {
            ShowStack(heapBlock, "Alloc");
            ShowStack(heapBlock, "Free");
        }

        return;
    }
}

void MphCmdsWrapper::InitializeForMPH()
{
    GetExtension()->inMPHCmd = true;

    enum IEMode
    {
        legacy = 0,
        edge = 1
    };

    bool verbose = GetExtension()->HasArg("v");

    char* tridentModules[] = { "mshtml", "edgehtml" };
    char* memGCModules[] = { "mshtml", "chakra" };
    char buffer[1024];

    IEMode mode = legacy;
    sprintf_s(buffer, "%s!MemProtectHeap", memGCModules[mode]); // should not be inlined
    ULONG typeIdMemProtectHeap;
    if (GetExtension()->m_Symbols2->GetSymbolTypeId(buffer, &typeIdMemProtectHeap, nullptr) == S_OK)
    {
        if (verbose)
        {
            GetExtension()->Out("Found MemProtectHeap in mshtml(legacy mode).\n");
        }
    }
    else
    {
        mode = edge;
        sprintf_s(buffer, "%s!MemProtectHeap", memGCModules[mode]); // should not be inlined
        if (GetExtension()->m_Symbols2->GetSymbolTypeId(buffer, &typeIdMemProtectHeap, nullptr) == S_OK)
        {
            if (verbose)
            {
                GetExtension()->Out("Found MemProtectHeap in chakra(edge mode).\n");
            }
        }
        else
        {
            GetExtension()->Err("Cannot find MemProtectHeap.\n");
            return;
        }
    }

    tridentModule = tridentModules[mode];
    memGCModule = memGCModules[mode];

}

JD_PRIVATE_COMMAND(findpage,
    "Find page for an address",
    "{;e,r;;Address to find}")
{

    ULONG64 targetAddress = this->GetUnnamedArgU64(0);


    RemoteThreadContext::ForEach([&](RemoteThreadContext threadContext)
    {
        this->Out("Thread context: %p\n", threadContext.GetPtr());
        threadContext.ForEachPageAllocator([&](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.ForEachSegment([&](PCSTR segName, ExtRemoteTyped pageSegment)->bool {
                ULONG64 address = pageSegment.Field("address").GetUlongPtr();
                ULONG pageCount = pageSegment.Field("segmentPageCount").GetUlong();
                if (address <= targetAddress && pageCount * g_Ext->m_PageSize + address > targetAddress)
                {
                    this->Out("Page Allocator: %p, %s\n", pageAllocator.GetExtRemoteTyped().GetPointerTo().GetPtr(), name);
                    pageAllocator.GetExtRemoteTyped().OutFullValue();
                    this->Out("PageSegment:%p, %s\n", pageSegment.GetPointerTo().GetPtr(), segName);
                    pageSegment.OutFullValue();
                }
                return false;
            });
            return false;
        });


        return false; // Don't stop iterating
    });
}
#endif

#pragma endregion()

// ---- End jd private commands implementation ----------------------------------------------------
//JD_PRIVATE
// ------------------------------------------------------------------------------------------------
