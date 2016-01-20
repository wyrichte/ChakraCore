//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "jdrecycler.h"
#include "RemoteRecyclerList.h"
#include "RecyclerRoots.h"

#include <hash_set>
#include <stack>
#include <queue>
#include "RecyclerObjectGraph.h"
#include "RemoteHeapBlockMap.h"

#ifdef JD_PRIVATE

ULONG64 GetStackTop(EXT_CLASS_BASE* ext)
{
    // Fix for x64?
    ULONG64 offset;
    ext->m_Registers2->GetStackOffset(&offset);
    return offset;
}


template <typename TPointerType>
template <class Fn>
void PinnedObjectMap<TPointerType>::Map(Fn fn)
{
    struct DebugPinRecord
    {
        TPointerType backTraceNode;
        uint  refCount;
    };

    struct HashEntry {
        TPointerType key;
        uint value;
        TPointerType next;
    };

    struct DebugHashEntry {
        TPointerType key;
        DebugPinRecord value;
        TPointerType next;
    };

    PinnedObjectEntry currentEntry;

    if (_transientPinnedObject != 0)
    {
        currentEntry.address = (TPointerType)_transientPinnedObject;
        currentEntry.pinnedCount = 1;
        fn(-1, 0, NULL, currentEntry);
    }

    ExtRemoteData tableEntries(_pinnedObjectEntries, g_Ext->m_PtrSize * _pinnedObjectTableSize);

    TPointerType* pinnedObjectMapEntries = (TPointerType*)malloc(g_Ext->m_PtrSize * _pinnedObjectTableSize);

    if (!pinnedObjectMapEntries) g_Ext->ThrowOutOfMemory();
    tableEntries.ReadBuffer(pinnedObjectMapEntries, g_Ext->m_PtrSize * _pinnedObjectTableSize);

    for (uint i = 0; i < _pinnedObjectTableSize; i++)
    {
        // Print out index of entry in the list of chained entries
        uint j = 0;
        for (TPointerType current = pinnedObjectMapEntries[i]; current != 0;)
        {
            ExtRemoteData remoteEntry((TPointerType)current, sizeof(HashEntry));

            if (!_pinRecordsWithStacks)
            {
                HashEntry entry;
                remoteEntry.ReadBuffer(&entry, sizeof(HashEntry));

                currentEntry.address = entry.key;
                currentEntry.pinnedCount = entry.value;
                fn(i, j++, current, currentEntry);
                current = (TPointerType)entry.next;
            }
            else
            {
                DebugHashEntry entry;
                remoteEntry.ReadBuffer(&entry, sizeof(DebugHashEntry));

                currentEntry.address = entry.key;
                currentEntry.pinnedCount = entry.value.refCount;
                fn(i, j++, current, currentEntry);
                current = (TPointerType)entry.next;
            }
        }
    }

    free(pinnedObjectMapEntries);
}

void RootPointerReader::ScanRegisters(EXT_CLASS_BASE* ext, bool print)
{
    ULONG numRegisters;
    ExtCheckedPointer<IDebugRegisters2> pRegisters = ext->m_Registers2;

    pRegisters->GetNumberRegisters(&numRegisters);

    if (print)
    {
        ext->Out("Number of registers: %d\n", numRegisters);
    }
    for (ULONG i = 0; i < numRegisters; i++)
    {
        char buffer[32];
        DEBUG_REGISTER_DESCRIPTION registerDescription;
        ULONG nameSize = 0;
        pRegisters->GetDescription(i, buffer, 32, &nameSize, &registerDescription);

        DEBUG_VALUE debugValue;
        pRegisters->GetValue(i, &debugValue);

        ULONG64 value = debugValue.I64;
        if (ext->m_PtrSize == 4)
        {
            value = debugValue.I32;
        }

        if (this->TryAdd(value) && print)
        {
            ext->Out("0x%p (Register %s)\n", value, buffer);
        }
    }
}

void RootPointerReader::ScanStack(EXT_CLASS_BASE* ext, ExtRemoteTyped& recycler, bool print)
{
    ULONG64 stackBase = 0;
    if (recycler.HasField("stackBase"))
    {
        stackBase = (ULONG64)((ext->m_PtrSize == 4) ? recycler.Field("stackBase").GetUlong()
            : recycler.Field("stackBase").GetPtr());
    }
    else
    {
        ExtRemoteTyped tib("@$TEB->NtTib");
        stackBase = (ULONG64)((ext->m_PtrSize == 4) ? tib.Field("StackBase").GetUlong()
            : tib.Field("StackBase").GetPtr());
    }


    ULONG64 stackTop = GetStackTop(ext);

    // memprotectheap recycler->stackBase is not set
    ExtRemoteTyped tib("@$TEB->NtTib");
    stackBase = tib.Field("StackBase").GetPtr();
    stackTop = tib.Field("StackLimit").GetPtr();

    size_t stackSizeInBytes = (size_t)(stackBase - stackTop);
    void** stack = (void**)malloc(stackSizeInBytes);
    if (!stack)
    {
        ext->ThrowOutOfMemory();
    }
    ULONG stackSizeInBytesLong = (ULONG)stackSizeInBytes;

#ifdef _M_X64
    if (ext->m_PtrSize == 4)
    {
        ULONG32* stack32 = (ULONG32*)stack;
        ExtRemoteData data(stackTop, stackSizeInBytesLong);
        data.ReadBuffer(stack32, stackSizeInBytesLong);

        if (print)
        {
            ext->Out("Stack top: 0x%p, stack start: 0x%p\n", stackTop, stackBase);
        }

        for (uint i = 0; i < (uint)stackSizeInBytesLong / ext->m_PtrSize; i++)
        {
            if (this->TryAdd((ULONG64)stack32[i]) && print)
            {
                ext->Out("0x%p", stack32[i]);
                ext->Out(" (+0x%x)\n", i * ext->m_PtrSize);
            }
        }

    }
    else
#endif
    {
        ExtRemoteData data(stackTop, stackSizeInBytesLong);
        data.ReadBuffer(stack, stackSizeInBytesLong);

        if (print)
        {
            ext->Out("Stack top: 0x%p, stack start: 0x%p\n", stackTop, stackBase);
        }

        for (uint i = 0; i < (uint)stackSizeInBytesLong / ext->m_PtrSize; i++)
        {
            if (this->TryAdd((ULONG64)stack[i]) && print)
            {
                ext->Out("0x%p", stack[i]);
                ext->Out(" (+0x%x)\n", i * ext->m_PtrSize);
            }
        }
    }


    free(stack);
}

void RootPointerReader::ScanObject(ULONG64 object, ULONG64 bytes)
{
    EXT_CLASS_BASE* ext = GetExtension();
    ULONG64 remainingBytes = bytes;
    ULONG64 curr = object;
    while (remainingBytes != 0)
    {
        ULONG readBytes = remainingBytes < 4096 ? (ULONG)remainingBytes : 4096;
        byte buffer[4096];
        ExtRemoteData data(curr, readBytes);
        data.ReadBuffer(buffer, readBytes);
        ULONG numPointers = readBytes / ext->m_PtrSize;
        byte * currBuffer = buffer;
        for (uint i = 0; i < numPointers; i++)
        {
            this->TryAdd(ext->m_PtrSize == 8? *(ULONG64 *)currBuffer : *(ULONG *)currBuffer);
            currBuffer += ext->m_PtrSize;
        }

        remainingBytes -= readBytes;
        curr += (ULONG64)readBytes;
    }
}

void RootPointerReader::ScanArenaBigBlocks(ExtRemoteTyped blocks)
{
    EXT_CLASS_BASE* ext = GetExtension();

    while (blocks.GetPtr() != 0)
    {
#if VERBOSE
        blocks.OutFullValue();
#endif

        ExtRemoteTyped block = blocks.Dereference();
        ULONG64 blockBytes = blocks.GetPtr() + ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sBigBlock))"));
        ExtRemoteTyped nBytesField = blocks.Field("nbytes");
        ULONG64 byteCount = ExtRemoteTypedUtil::GetSizeT(nBytesField);
        if (byteCount != 0)
        {
            ScanObject(blockBytes, byteCount);
        }
        blocks = block.Field("nextBigBlock");
    }

}

void RootPointerReader::ScanArenaMemoryBlocks(ExtRemoteTyped blocks)
{
    EXT_CLASS_BASE* ext = GetExtension();

    while (blocks.GetPtr() != 0)
    {
        ULONG64 blockBytes = blocks.GetPtr() + ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sArenaMemoryBlock))"));
        ExtRemoteTyped nBytesField = blocks.Field("nbytes");
        size_t byteCount = (size_t)nBytesField.GetLong();
        ScanObject(blockBytes, byteCount);
        blocks = blocks.Field("next");
    }
}

void RootPointerReader::ScanArenaData(ULONG64 arenaDataPtr)
{
    EXT_CLASS_BASE* ext = GetExtension();
    ExtRemoteTyped arenaData(ext->FillModuleAndMemoryNS("(%s!%sArenaData*)@$extin"), arenaDataPtr);
    ScanArenaBigBlocks(arenaData.Field("bigBlocks"));
    ScanArenaBigBlocks(arenaData.Field("fullBlocks"));
    ScanArenaMemoryBlocks(arenaData.Field("mallocBlocks"));
}

void RootPointerReader::ScanArena(ULONG64 arena, bool verbose)
{
    EXT_CLASS_BASE* ext = GetExtension();
    if (verbose)
        ext->Out("Scanning arena 0x%p\n", arena);
    arena += ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sAllocator))"));
    ScanArenaData(arena);
}

void RootPointerReader::ScanImplicitRoots(bool print)
{
    ULONG64 implicitRootCount;
    ULONG64 implicitRootSize;

    RemoteHeapBlockMap hbm(_recycler.Field("heapBlockMap"));

    hbm.ForEachHeapBlock([&](RemoteHeapBlock& remoteHeapBlock)
    {
        if (remoteHeapBlock.IsLargeHeapBlock())
        {
            ULONG64 sizeOfObjectHeader = g_Ext->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));
            return remoteHeapBlock.ForEachLargeObjectHeader([&](ExtRemoteTyped& header)
            {
                byte attribute;

                if (header.HasField("attributesAndChecksum"))
                {
                    attribute = (UCHAR)(header.Field("attributesAndChecksum").GetUshort() ^ _recycler.Field("Cookie").GetUlong() >> 8);
                }
                else if (header.HasField("attributes"))
                {
                    attribute = header.Field("attributes").GetUchar();
                }
                else
                {
                    g_Ext->Err("Can't find either attributes or attributesAndChecksum on LargeHeapBlock");
                    return true;
                }

                if (attribute & ObjectInfoBits::ImplicitRootBit)
                {
                    this->Add(header.GetPtr() + sizeOfObjectHeader);
                    if (print)
                    {
                        implicitRootCount++;
                        implicitRootSize += ExtRemoteTypedUtil::GetSizeT(header.Field("objectSize"));
                    }
                }
                return false;
            });
        }
        else
        {
            ULONG64 heapBlock = remoteHeapBlock.GetHeapBlockAddress();
            ULONG objectCount = remoteHeapBlock.GetTotalObjectCount();
            ULONG64 attributeStartAddress = heapBlock - objectCount;

            ExtRemoteData remoteAttributes(attributeStartAddress, objectCount);
            std::vector<byte> attributes(objectCount);
            remoteAttributes.ReadBuffer(&attributes[0], objectCount);

            ULONG64 heapBlockAddress = remoteHeapBlock.GetAddress();
            ULONG64 objectSize = remoteHeapBlock.GetBucketObjectSize();
            for (ULONG i = 0; i < objectCount; i++)
            {
                if ((attributes[objectCount - i - 1] & ObjectInfoBits::ImplicitRootBit)
                    && (attributes[objectCount - i - 1] & ObjectInfoBits::PendingDisposeBit) == 0)
                {
                    this->Add(heapBlockAddress + objectSize * i);
                    if (print)
                    {
                        implicitRootCount++;
                        implicitRootSize += objectSize;
                    }
                }
            }
        }
        return false;
    });

    if (print)
    {
        g_Ext->Out("\nImplicit Root: Count=%I64d Size=%I64d\n", implicitRootCount, implicitRootSize);
    }
}

bool IsUsingDebugPinRecord(EXT_CLASS_BASE* ext, bool verbose)
{
    ULONG typeId = 0;
    HRESULT hr = ext->m_Symbols->GetTypeId(0, ext->FillModuleAndMemoryNS("%s!%sRecycler::PinRecord"), &typeId);

    // On retail builds, the pinRecord type doesnt even exist, it gets optimized away to int
    // On debug builds, the type might exist but it may or may not have extra information in it
    if (SUCCEEDED(hr) && typeId != 0)
    {
        ULONG64 sizeofPinRecord = ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sRecycler::PinRecord))"));
        bool is = (sizeofPinRecord != sizeof(uint));
        if (verbose)
        {
            ext->Out("Using pin record with stack traces: %s\n", (is ? "true" : "false"));
        }

        return is;
    }

    return false;
}

template <typename Fn>
void MapPinnedObjects(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, const Fn& callback, bool verbose)
{
    bool isUsingDebugPinnedRecord = IsUsingDebugPinRecord(ext, verbose);
    // ext->Out(L"Possible symbol for %p: ", heapObject.vtable); ext->m_Symbols3->OutputSymbolByOffset(DEBUG_OUTCTL_AMBIENT, DEBUG_OUTSYM_ALLOW_DISPLACEMENT, heapObject.vtable); ext->Out("\n");
    if (ext->m_PtrSize == 8)
    {
        PinnedObjectMap<ULONG64> pinnedObjects(recycler, isUsingDebugPinnedRecord);
        pinnedObjects.Map([&callback](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
        {
            callback(i, j, entryPointer, entry);
        });
    }
    else
    {
        PinnedObjectMap<ULONG32> pinnedObjects(recycler, isUsingDebugPinnedRecord);
        pinnedObjects.Map([&callback](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
        {
            callback(i, j, entryPointer, entry);
        });
    }
}

void DumpPinnedObject(EXT_CLASS_BASE* ext, int i, int j, ULONG64 entryPointer, const PinnedObjectEntry& entry)
{
    ext->Out("Index: (0x%x, %d), Entry: 0x%p", i, j, entryPointer);
    ext->Out(", Key: 0x%p ", entry.address);
    ext->Out("(Ref count: %d)", entry.pinnedCount);
    // Appears to be a bug in ext->Out where it doesn't deal with %d properly when
    // mixed with other types

    ULONG64 vtable = GetPointerAtAddress(entry.address);
    std::string symbol = GetSymbolForOffset(ext, vtable);

    if (!symbol.empty())
    {
        ext->Out(", Possible symbol for offset: %s", symbol.c_str());

        if (symbol.rfind("ArrayObjectInstance") != std::string::npos ||
            symbol.rfind("Js::CustomExternalObject") != std::string::npos)
        {
            ULONG64 offsetOfExternalObject = 0x18;

#ifdef _M_AMD64
            if (ext->m_PtrSize == 8)
            {
                offsetOfExternalObject = 0x30;
            }
#endif
            ULONG64 externalObject = (ULONG64)entry.address + offsetOfExternalObject;
            ULONG64 domObject = GetPointerAtAddress(externalObject);
            if (domObject != 0)
            {
                ULONG64 domVtable = GetPointerAtAddress(domObject);
                std::string symbol = GetSymbolForOffset(ext, domVtable);

                if (!symbol.empty())
                {
                    ext->Out("(maybe DOM item %s)", symbol.c_str());
                }
                else
                {
                    ext->Out("(0x%p)", externalObject);
                }
            }
        }
        else if (symbol.rfind("JavascriptDispatch") != std::string::npos)
        {
            ULONG64 offsetOfDispatch = 0x14;

#ifdef _M_AMD64
            if (ext->m_PtrSize == 8)
            {
                offsetOfDispatch = 0x28;
            }
#endif
            ULONG64 externalObject = (ULONG64)entry.address + offsetOfDispatch;
            ULONG64 dispatchObject = GetPointerAtAddress(externalObject);
            if (dispatchObject)
            {
                ULONG64 dispatchVTable = GetPointerAtAddress(dispatchObject);
                std::string symbol = GetSymbolForOffset(ext, dispatchVTable);

                if (!symbol.empty())
                {
                    ext->Out("(maybe script Object is %s)", symbol.c_str());
                }
                else
                {
                    ext->Out("(0x%p)", externalObject);
                }
            }


        }
    }

    ext->Out("\n");
}

JD_PRIVATE_COMMAND(showpinned,
    "Show pinned object list",
    "{a;b,o;all;all thread context}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    const bool allThreadContext = HasArg("a");

    auto dumpPinnedObjectFromRecycler = [this](RemoteRecycler recycler)
    {
        uint count = 0;
        MapPinnedObjects(this, recycler.GetExtRemoteTyped(), [&count, this](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
        {
            DumpPinnedObject(this, i, j, entryPointer, entry);
            count++;
        }, true);

        Out("Count is %d\n", count);
    };

    if (allThreadContext)
    {
        RemoteThreadContext::ForEach([this, dumpPinnedObjectFromRecycler](RemoteThreadContext threadContext)
        {
            Out("===========================================================\n");
            Out("ThreadContext %p\n", threadContext.GetExtRemoteTyped().GetPtr());
            dumpPinnedObjectFromRecycler(threadContext.GetRecycler());
            return false;
        });
    }
    else
    {
        ExtRemoteTyped recycler = GetRecycler(arg);
        dumpPinnedObjectFromRecycler(recycler);
    }

}

template <class Fn>
void SearchRange(ULONG64 startAddress, uint numBytes, ExtExtension* ext, ULONG64 targetValue, Fn fn)
{
    char* pageContents = (char*)malloc(numBytes);
    AutoFree guard((void*)pageContents);
    ExtRemoteData remotePage(startAddress, numBytes);
    size_t ptrSize = ext->m_PtrSize;

    try
    {
        remotePage.ReadBuffer(pageContents, numBytes);
    }
    catch (ExtRemoteException&)
    {
        return;
    }

    char* start = pageContents;
    char* end = start + numBytes;
    char* current = start;

    while (current < end)
    {
        ULONG64 value;

        if (ptrSize == 8)
        {
            value = *((ULONG64*)current);
        }
        else
        {
            value = *((ULONG32*)current);
        }

        if (value == targetValue)
        {
            ULONG64 offset = (ULONG64)(current - start);
            ULONG64 addr = startAddress + offset;
#if VERBOSE
            ext->Out("Start address: 0x%p, Value: 0x%p, Current: %p, Offset: %d\n", startAddress, value, current, offset);
#endif
            fn(addr);
        }

        current += ptrSize;
    }
}

bool
RecyclerFindReference::ProcessHeapBlock(ExtRemoteTyped block, bool isAllocator, ExtRemoteTyped freeObjectList, bool isBumpAllocator)
{
    ULONG64 startAddress = block.Field("address").GetPtr();
    USHORT objectSize = block.Field("objectSize").GetUshort();
    USHORT objectCount = block.Field("objectCount").GetUshort();

    Addresses * rootPointers = this->rootPointerManager;
    SearchRange(startAddress, objectSize*objectCount, this->ext, this->referencedObject, [&](ULONG64 addr)
    {
        bool isRoot = rootPointers->Contains(addr);
        ULONG64 offset = (addr - startAddress);
        USHORT objectIndex = (USHORT)(offset / objectSize);
        ULONG64 objectAddress = (objectSize * objectIndex) + startAddress;
        offset = (addr - objectAddress);

        FindRefData result;
        result.address = objectAddress;
        result.offset = offset;
        result.isRoot = isRoot;
        result.isLarge = false;
        results.push_back(result);
    });

    return false;
}

bool
RecyclerFindReference::ProcessLargeHeapBlock(ExtRemoteTyped block)
{
    Addresses * rootPointers = this->rootPointerManager;
    unsigned int allocCount = (uint)ExtRemoteTypedUtil::GetSizeT(block.Field("allocCount"));
    ExtRemoteTyped headerList =
        ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), block.GetPtr() + block.Dereference().GetTypeSize());

    ULONG64 sizeOfObjectHeader = ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));

    for (unsigned int i = 0; i < allocCount; i++)
    {
        ExtRemoteTyped header = headerList.ArrayElement(i);
        if (header.GetPtr() == 0)
        {
            continue;
        }
        ULONG64 objectSize = ExtRemoteTypedUtil::GetSizeT(header.Field("objectSize"));

        ULONG64 startAddress = header.GetPtr() + sizeOfObjectHeader;

        SearchRange(startAddress, (uint)objectSize, ext, this->referencedObject, [&](ULONG64 addr) {
            bool isRoot = rootPointers->Contains(addr);

            FindRefData result;
            result.address = addr;
            result.offset = 0;
            result.isRoot = isRoot;
            result.isLarge = true;
            results.push_back(result);
        });
    }
    return false;
}

JD_PRIVATE_COMMAND(findref,
    "Find objects referencing given object",
    "{;e,r;address;Address whose referrers to find}{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;

    if (recyclerArg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerArg);
        threadContext = CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        RemoteThreadContext remoteThreadContext = RemoteThreadContext::GetCurrentThreadContext();
        threadContext = remoteThreadContext.GetExtRemoteTyped();
        recycler = remoteThreadContext.GetRecycler().GetExtRemoteTyped();
    }

    Addresses * rootPointers = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    RecyclerFindReference findRef(this, address, rootPointers, recycler);
    Out("Referring objects:\n");

    findRef.Run();
    std::for_each(findRef.results.begin(), findRef.results.end(), [&](decltype(findRef.results[0])& result) {
        if (result.isLarge)
        {
            this->Dml("\t<link cmd=\"!oi 0x%p%s\">0x%p</link> (Object) %s\n",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg), result.address, (result.isRoot ? "(root)" : ""));
        }
        else
        {
            this->Dml("\t<link cmd = \"!findref 0x%p%s\">+</link> ",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg));
            this->Dml("<link cmd=\"!oi 0x%p%s\">0x%p</link>",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg), result.address);
            this->Dml("+0x%02x", result.offset);
            this->Dml(" %s\n", (result.isRoot ? "(root)" : ""));
        }
    });

    if (findRef.FoundNoReferences())
    {
        Out("\tNo referencing objects\n");
    }

    if (findRef.SkippedSomeAddresses())
    {
        Out("\tWARNING: Some memory was inaccessible and not scanned so some references might be missed\n");
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
    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;

    if (recyclerArg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerArg);
        threadContext = CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        RemoteThreadContext remoteThreadContext = RemoteThreadContext::GetCurrentThreadContext();
        threadContext = remoteThreadContext.GetExtRemoteTyped();
        recycler = remoteThreadContext.GetRecycler().GetExtRemoteTyped();
    }

    HeapBlockHelper heapBlockHelper(this, recycler);
    RemoteHeapBlock * remoteHeapBlock = heapBlockHelper.FindHeapBlock(objectAddress, recycler);
    if (remoteHeapBlock != NULL)
    {        
        ULONG64 heapBlockType = remoteHeapBlock->GetType();

        heapBlockHelper.DumpHeapBlockLink(heapBlockType, remoteHeapBlock->GetHeapBlockAddress());

        if (heapBlockType >= this->enum_BlockTypeCount())
        {
            Out("Object returned Invalid Heap Block\n");
            return;
        }

        ExtRemoteTyped heapBlock = remoteHeapBlock->GetExtRemoteTyped();
        if (heapBlockType == this->enum_LargeBlockType())
        {
            heapBlockHelper.DumpLargeHeapBlockObject(heapBlock, objectAddress, verbose);
        }
        else
        {
            heapBlockHelper.DumpSmallHeapBlockObject(heapBlock, objectAddress, verbose);
        }

        if (verbose)
        {
            Addresses * rootPointers = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
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

JD_PRIVATE_COMMAND(showroots,
    "Show the recycler roots",
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped recycler;
    ExtRemoteTyped threadContext;

    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
        threadContext = CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();
        recycler = threadContext.Field("recycler");
    }

    RootPointerReader rootPointerManager(this, recycler);

    /*
     * FindRoots algorithm
     * - Find Implicit roots
     * - Find external weak referenced roots
     * - Scan external roots
     * - Scan pinned objects
     * - Scan guest arena (if its not pending delete)
     * - Scan external guest arena
     * - Scan stack
     */

    if (recycler.HasField("enableScanImplicitRoots") && recycler.Field("enableScanImplicitRoots").GetStdBool())
    {
        rootPointerManager.ScanImplicitRoots();
    }

    if (threadContext.GetPtr() != NULL && threadContext.HasField("externalWeakReferenceCacheList"))
    {
        ExtRemoteTyped externalWeakReferenceCache = threadContext.Field("externalWeakReferenceCacheList");
        ULONG64 externalWeakReferenceList = externalWeakReferenceCache.GetPointerTo().GetPtr();
        if (externalWeakReferenceList != 0)
        {
            Out("External Weak Reference caches\n");
            DumpList<true>(this, externalWeakReferenceList, "ExternalWeakReferenceCache *");
        }
    }

    ExtRemoteTyped externalRootMarker = recycler.Field("externalRootMarker");

    if (externalRootMarker.GetPtr() != 0)
    {
        Out("External root marker installed (Address: 0x%p), some roots might be missed\n", externalRootMarker.GetUlongPtr());
    }

    Out("\nPinned objects\n");
    uint count = 0;
    MapPinnedObjects(this, recycler, [&count, &rootPointerManager, this](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
    {
        DumpPinnedObject(this, i, j, entryPointer, entry);
        count++;
        rootPointerManager.TryAdd((ULONG64)entry.address);
    }, true);

    Out("Count is %d\n", count);

    Out("\nGuest arenas\n");
    ULONG64 guestArenaList = recycler.Field("guestArenaList").GetPointerTo().GetPtr();
    DumpList<false>(this, guestArenaList, "Recycler::GuestArenaAllocator");

    Out("\nExternal guest arenas\n");

    ExtRemoteTyped egal = recycler.Field("externalGuestArenaList");
    ULONG64 externalGuestArenaList = egal.GetPointerTo().GetPtr();
    DumpList<false>(this, externalGuestArenaList, "ArenaData *");

    Out("\nStack\n");
    rootPointerManager.ScanRegisters(this);
    rootPointerManager.ScanStack(this, recycler);

    PCSTR typeName = recycler.Field("heapBlockMap").GetTypeName();
    Out("Heap block map type is %s\n", typeName);
}

Addresses * ComputeRoots(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, ExtRemoteTyped* threadContext, bool dump)
{
    RootPointerReader rootPointerManager(ext, recycler);

    /*
     * FindRoots algorithm
     * - Find Implicit roots
     * - Find external weak referenced roots
     * - Scan external roots
     * - Scan pinned objects
     * - Scan guest arena (if its not pending delete)
     * - Scan external guest arena
     * - Scan stack
     */

    if (recycler.HasField("enableScanImplicitRoots") && recycler.Field("enableScanImplicitRoots").GetStdBool())
    {
        rootPointerManager.ScanImplicitRoots(dump);
    }

    if (threadContext && threadContext->HasField("externalWeakReferenceCacheList"))
    {
        ExtRemoteTyped externalWeakReferenceCache = threadContext->Field("externalWeakReferenceCacheList");
        ULONG64 externalWeakReferenceList = externalWeakReferenceCache.GetPointerTo().GetPtr();
        if (externalWeakReferenceList != 0)
        {
            // TODO: Implement external weak ref support
            // TODO: Fix RecyclerCachedData::GetRootPointers once this is implemented when thread context is not passed in
        }
    }

    MapPinnedObjects(ext, recycler, [&rootPointerManager](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
    {
        rootPointerManager.TryAdd(entry.address);
    }, dump);

    ULONG64 recyclerAddress = recycler.m_Data; // TODO: recycler needs to be a pointer to make this work
    ULONG64 guestArenaList = recyclerAddress + recycler.GetFieldOffset("guestArenaList");

    // Need to scan guest arena
    RemoteListIterator<false> guestArenaIterator("Recycler::GuestArenaAllocator", guestArenaList);

    while (guestArenaIterator.Next())
    {
        ULONG64 data = guestArenaIterator.GetDataPtr();
        ExtRemoteTyped guestArena(ext->FillModuleAndMemoryNS("(%s!%sRecycler::GuestArenaAllocator*)@$extin"), data);

        ExtRemoteTyped isPendingDelete = guestArena.Dereference().Field("pendingDelete");
        if (!isPendingDelete.GetBoolean())
        {
            rootPointerManager.ScanArena(data, dump);
        }
    }

    ExtRemoteTyped egal = recycler.Field("externalGuestArenaList");
    ULONG64 externalGuestArenaList = egal.GetPointerTo().GetPtr();

    // Need to scan external guest arena
    RemoteListIterator<false> externalGuestArenaIterator("ArenaData *", externalGuestArenaList);

    while (externalGuestArenaIterator.Next())
    {
        ULONG64 dataPtr = externalGuestArenaIterator.GetDataPtr();

        rootPointerManager.ScanArenaData(GetPointerAtAddress(dataPtr));
    }

    rootPointerManager.ScanRegisters(ext, dump);
    rootPointerManager.ScanStack(ext, recycler, dump);

    return rootPointerManager.DetachAddresses();
}

#if ENABLE_MARK_OBJ
JD_PRIVATE_COMMAND(markobj,
    "Dumps a path to an object from the root (INCOMPLETE)",
    "{;e,r;address;Address to which the path should be traced}{;e,o,d=0;from;Root to trace from}{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 root = GetUnnamedArgU64(1);
    ULONG64 arg = GetUnnamedArgU64(2);
    ExtRemoteTyped recycler;
    ExtRemoteTyped threadContext = GetCurrentThreadContext();

    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = threadContext.Field("recycler");
    }

    Addresses * rootPointerManager = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    Out("\nNumber of root GC pointers found: %d\n\n", rootPointerManager.Count());

    bool found = false;
    rootPointerManager.Map([this, &found, &address](ULONG64 root) {
        if (address == root)
        {
            this->Out("0x%p is a root\n", address);
            found = true;
        }
    });

    if (!found)
    {
        Out("Not a root- building object graph\n");
        RecyclerObjectGraph objectGraph(this, recycler);

        objectGraph.Construct(rootPointerManager);

        Out("Object graph constructed- finding path to 0x%p\n", address);
        objectGraph.FindPathTo(rootPointerManager, address, root);
    }
}
#endif

JD_PRIVATE_COMMAND(savegraph,
    "Save's current recycler object graph into a python file",
    "{;s;filename;Filename to output to}"
    "{;e,o,d=0;recycler;Recycler address}"
    "{;s,o,d=js;filetype;Save file type<js|python>, default is js}")
{
    PCSTR filename = GetUnnamedArgStr(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    PCSTR filetype = GetUnnamedArgStr(2);

    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
        threadContext = CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        RemoteThreadContext remotetTreadContext = RemoteThreadContext::GetCurrentThreadContext();
        threadContext = remotetTreadContext.GetExtRemoteTyped();
        recycler = remotetTreadContext.GetRecycler().GetExtRemoteTyped();
    }

    Addresses * rootPointerManager = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    Out("\nNumber of root GC pointers found: %d\n\n", rootPointerManager->Count());

    RecyclerObjectGraph objectGraph(this, recycler);

    objectGraph.Construct(*rootPointerManager);

    Out("Saving object graph to %s\n", filename);
    if (_stricmp(filetype, "js") == 0)
    {
        objectGraph.DumpForJs(filename);
    }
    else if (_stricmp(filetype, "python") == 0)
    {
        objectGraph.DumpForPython(filename);
    }
    else
    {
        Out("Unknown file type %s\n", filetype);
    }
}


struct ObjectAllocStats
{
    uint count;
    uint size;
    uint unknownCount;
    uint unknownSize;
    bool hasVtable;
};

int __cdecl ObjectAllocCountComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return ptrB->second.count - ptrA->second.count;
}

int __cdecl ObjectAllocSizeComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return ptrB->second.size - ptrA->second.size;
}

int __cdecl ObjectAllocUnknownCountComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return ptrB->second.unknownCount - ptrA->second.unknownCount;
}

int __cdecl ObjectAllocUnknownSizeComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return ptrB->second.unknownSize - ptrA->second.unknownSize;
}

int __cdecl ObjectAllocNameComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return strcmp(ptrA->first, ptrB->first);
}

JD_PRIVATE_COMMAND(jsobjectstats,
    "Dump a table of object types and statistics",
    "{;e,o,d=0;recycler;Recycler address}"
    "{v;b,o;verbose;Display verbose tracing}"
    "{t;b,o;trident;Display trident symbols}"
    "{sc;b,o;sortByCount;Sort by count instead of bytes}"
    "{sn;b,o;sortByName;Sort by name instead of bytes}"
    "{su;b,o;sortByUnknown;Sort by unknown}"
    "{top;en=(10),o,d=-1;count;Number of entries to display}"
    "{vt;b,o;vtable;Vtable Only}"
    "{u;b,o;grouped;Show unknown count}"
    "{k;b,o;known;Known object only}"
    )
{
    const bool trident = HasArg("t");
    const bool verbose = HasArg("v");
    const bool infer = !HasArg("vt");
    const bool showUnknown = HasArg("u");
    const bool knownOnly = HasArg("k");
    const ULONG64 limit = GetArgU64("top");

    if (HasArg("sc") && HasArg("sn"))
    {
        throw ExtException(E_FAIL, "Can't specify both -sc and -sn");
    }
    if (HasArg("su") && HasArg("sn"))
    {
        throw ExtException(E_FAIL, "Can't specify both -su and -sn");
    }
    auto sortComparer = HasArg("sn")? ObjectAllocNameComparer :
        HasArg("su")?
        (HasArg("sc") ? ObjectAllocUnknownCountComparer : ObjectAllocUnknownSizeComparer) :
        (HasArg("sc") ? ObjectAllocCountComparer :  ObjectAllocSizeComparer);


    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped recycler = GetRecycler(arg);
    ExtRemoteTyped threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();

    Addresses * rootPointerManager = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    if (verbose)
    {
        Out("\nNumber of root GC pointers found: %d\n\n", rootPointerManager->Count());
    }

    RecyclerObjectGraph objectGraph(this, recycler);

    objectGraph.Construct(*rootPointerManager);    
    objectGraph.EnsureTypeInfo(infer, trident, verbose);

    stdext::hash_map<char const *, ObjectAllocStats> objectCounts;
    int numNodes = 0;
    int totalSize = 0;

    auto addStats = [&](RecyclerObjectGraph::GraphImplNodeType * node)
    {        
        char const * typeName = infer ? node->aux.typeNameOrField : node->aux.typeName;
        auto i = objectCounts.find(typeName);
        if (i != objectCounts.end())
        {
            ObjectAllocStats& stats = (*i).second;
            stats.count++;
            stats.size += node->aux.objectSize;
            stats.unknownCount += node->aux.isPropagated;
            stats.unknownSize += node->aux.isPropagated ? node->aux.objectSize : 0;
        }
        else
        {
            ObjectAllocStats stats;
            stats.count = 1;
            stats.size = node->aux.objectSize;
            stats.unknownCount = node->aux.isPropagated;
            stats.unknownSize = node->aux.isPropagated ? node->aux.objectSize : 0;
            stats.hasVtable = node->aux.hasVtable;
            objectCounts[typeName] = stats;
        }
    };

    objectGraph.MapAllNodes([&](ULONG64 objectAddress, RecyclerObjectGraph::GraphImplNodeType* node)
    {
        numNodes++;
        totalSize += (node->aux.objectSize);
        addStats(node);
    });

    Out("\r");
    if (showUnknown)
    {
        Out(" Count?      Bytes? %%Count %%Bytes | ");
    }
    Out("  Count       Bytes %%Count %%Bytes Symbol                \n");
    uint knownObjectCount = 0;
    uint knownObjectSize = 0;
    uint vtableCount = 0;
    std::auto_ptr<std::pair<char const *, ObjectAllocStats>> sortedArray(new std::pair<char const *, ObjectAllocStats>[objectCounts.size()]);
    int c = 0;
    for (auto i = objectCounts.begin(); i != objectCounts.end(); i++)
    {
        sortedArray.get()[c++] = (*i);
        ObjectAllocStats& stats = (*i).second;
        knownObjectCount += stats.count - stats.unknownCount;
        knownObjectSize += stats.size - stats.unknownSize;
        vtableCount += stats.hasVtable;
    }
 
    qsort(sortedArray.get(), c, sizeof(std::pair<char const *, ObjectAllocStats>), sortComparer);


    Out("----------------------------------------------------------------------------\n");

    for (int i = 0; i < c; i++)
    {
        char const * typeName = sortedArray.get()[i].first;
        ObjectAllocStats& stats = sortedArray.get()[i].second;
        uint currCount = stats.count;
        uint currSize = stats.size;
        if (knownOnly)
        {
            currCount -= stats.unknownCount;
            currSize -= stats.unknownSize;
        }
        if (showUnknown)
        {
            Out("%7u %11u %5.1f%% %5.1f%% | ", stats.unknownCount, stats.unknownSize, 
                (float)stats.unknownCount / (float)numNodes * 100, (float)stats.unknownSize / (float)totalSize * 100);
        }
        Out("%7u %11u %5.1f%% %5.1f%% %s%s\n", currCount, currSize, (float)currCount / (float)numNodes * 100, (float)currSize / (float)totalSize * 100,
            stats.hasVtable ? "" : "[Field] ", typeName);        

        if (i > limit)
        {
            Out("<%d limit reached>\n", limit);
            break;
        }
    }
    Out("----------------------------------------------------------------------------\n");

    uint unknownTotalCount = numNodes - knownObjectCount;
    uint unknownTotalSize = totalSize - knownObjectSize;
    Out("%7u %11u %5.1f%% %5.1f%%", unknownTotalCount, unknownTotalSize,
        (float)unknownTotalCount / (float)numNodes * 100, (float)unknownTotalSize / (float)totalSize * 100);
    Out(showUnknown ? " | " : " Unknown object summary\n");

    Out("%7u %11u %5.1f%% %5.1f%% Known object summary\n", knownObjectCount, knownObjectSize,
        (float)knownObjectCount / (float)numNodes * 100, (float)knownObjectSize / (float)totalSize * 100);

    if (showUnknown)
    {
        Out("                                  | ");
    }
    Out("%7u %11u               Total object summary\n", numNodes, totalSize);
    Out("Found %d (%d vtable, %d field)\n", objectCounts.size(), vtableCount, objectCounts.size() - vtableCount);
}

#endif
