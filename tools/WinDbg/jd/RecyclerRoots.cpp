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

    if (_transientPinnedObject != NULL)
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
        for (TPointerType current = pinnedObjectMapEntries[i]; current != NULL;)
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

    ULONG scannedRegisters = 0;

    for (ULONG i = 0; i < numRegisters; i++)
    {
        char buffer[32];
        DEBUG_REGISTER_DESCRIPTION registerDescription;
        ULONG nameSize = 0;
        pRegisters->GetDescription(i, buffer, 32, &nameSize, &registerDescription);

        if ((registerDescription.Flags & DEBUG_REGISTER_SUB_REGISTER))
        {
            // Don't care about subregister
            continue;
        }

        if (ext->m_PtrSize == 4)
        {
            if (registerDescription.Type != DEBUG_VALUE_INT32)
            {
                continue;
            }
        }
        else
        {
            if (registerDescription.Type != DEBUG_VALUE_INT64)
            {
                continue;
            }
        }

        // Don't scan debug registers
        if (strncmp(buffer, "dr", 2) == 0)
        {
            continue;
        }

        scannedRegisters++;
        DEBUG_VALUE debugValue;
        pRegisters->GetValue(i, &debugValue);

        ULONG64 value = ext->m_PtrSize == 4? debugValue.I32 : debugValue.I64;

        if (this->TryAdd(value, RootType::RootTypeRegister) && print)
        {
            ext->Out("0x%p (Register %s)\n", value, buffer);
        }
    }

    if (print)
    {
        ext->Out("Number of scanned registers: %d\n", scannedRegisters);
        ext->Out("Number of total registers: %d\n", numRegisters);
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
            if (this->TryAdd((ULONG64)stack32[i], RootType::RootTypeStack) && print)
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
            ULONG64 address = (ULONG64)stack[i];
            if (this->TryAdd(address, RootType::RootTypeStack) && print)
            {
                ext->Out("0x%p", address);
                ext->Out(" (+0x%x)", i * ext->m_PtrSize);
                ext->DumpPossibleSymbol(address);
                ext->Out("\n");
            }
        }
    }

    free(stack);
}

void RootPointerReader::ScanObject(ULONG64 object, ULONG64 bytes, RootType rootType)
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
            this->TryAdd(ext->m_PtrSize == 8 ? *(ULONG64 *)currBuffer : *(ULONG *)currBuffer, rootType);
            currBuffer += ext->m_PtrSize;
        }

        remainingBytes -= readBytes;
        curr += (ULONG64)readBytes;
    }
}

void RootPointerReader::ScanArenaBigBlocks(ExtRemoteTyped blocks)
{
    EXT_CLASS_BASE* ext = GetExtension();

    while (blocks.GetPtr() != NULL)
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
            ScanObject(blockBytes, byteCount, RootType::RootTypeArena);
        }
        blocks = block.Field("nextBigBlock");
    }
}

void RootPointerReader::ScanArenaMemoryBlocks(ExtRemoteTyped blocks)
{
    EXT_CLASS_BASE* ext = GetExtension();
    while (blocks.GetPtr() != NULL)
    {
        ULONG64 blockBytes = blocks.GetPtr() + ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sArenaMemoryBlock))"));
        ExtRemoteTyped nBytesField = blocks.Field("nbytes");
        size_t byteCount = (size_t)nBytesField.GetLong();
        ScanObject(blockBytes, byteCount, RootType::RootTypeArena);
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
    ExtRemoteTyped heapBlockMap = _recycler.Field("heapBlockMap");

    hbm.ForEachHeapBlock(heapBlockMap, [&](RemoteHeapBlock& remoteHeapBlock)
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
                    this->Add(header.GetPtr() + sizeOfObjectHeader, RootType::RootTypeImplicit);
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
                    this->Add(heapBlockAddress + objectSize * i, RootType::RootTypeImplicit);
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

    // On retail builds, the pinRecord type doesn't even exist, it gets optimized away to int
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
    // ext->Out(_u("Possible symbol for %p: "), heapObject.vtable); ext->m_Symbols3->OutputSymbolByOffset(DEBUG_OUTCTL_AMBIENT, DEBUG_OUTSYM_ALLOW_DISPLACEMENT, heapObject.vtable); ext->Out("\n");
    if (ext->m_PtrSize == 8)
    {
        PinnedObjectMap<ULONG64> pinnedObjects(recycler, isUsingDebugPinnedRecord);
        pinnedObjects.Map(callback);
    }
    else
    {
        PinnedObjectMap<ULONG32> pinnedObjects(recycler, isUsingDebugPinnedRecord);
        pinnedObjects.Map(callback);
    }
}

bool EXT_CLASS_BASE::DumpPossibleSymbol(ULONG64 address, bool makeLink)
{
    char const * typeName;
    JDRemoteTyped object = GetExtension()->CastWithVtable(address, &typeName);
    
    if (typeName == nullptr)
    {
        return false;
    }

    if (makeLink)
    {
        this->Out(" = %s ", typeName);
        this->Dml("<link cmd=\"?? (%s *)0x%p\">(??)</link> ", typeName, address);

        // TODO (doilij) don't include link unless object has type.typeId
        //this->CastWithVtable(address, "Js::RecyclableObject");

        this->Dml("<link cmd=\"!jd.var 0x%p\">(!jd.var)</link> ", address);
    }
    else
    {
        this->Out(" = %s", typeName);
    }
#undef SYMBOL_GUESS

    if (strstr(typeName, "ArrayObjectInstance") != 0 ||
        strstr(typeName, "Js::CustomExternalObject") != 0)
    {
        ULONG64 offsetOfExternalObject = 0x18;

#ifdef _M_AMD64
        if (this->m_PtrSize == 8)
        {
            offsetOfExternalObject = 0x30;
        }
#endif
        ULONG64 externalObject = address + offsetOfExternalObject;
        ULONG64 domObject = GetPointerAtAddress(externalObject);
        if (domObject != NULL)
        {
            ULONG64 domVtable = GetPointerAtAddress(domObject);
            std::string symbol = GetSymbolForOffset(this, domVtable);

            if (!symbol.empty())
            {
                this->Out("(maybe DOM item %s)", symbol.c_str());
            }
            else
            {
                this->Out("(0x%p)", externalObject);
            }
        }
    }
    else if (strstr(typeName, "JavascriptDispatch") != 0)
    {
        ExtRemoteTyped scriptObject = object.Field("scriptObject");
        ULONG64 scriptObjectPointer = scriptObject.GetPtr();
        // scriptObject can be null if the ScriptEngine has been closed, so check for this scenario.
        if (scriptObjectPointer)
        {
            this->Out("[ScriptObject");
            if (!DumpPossibleSymbol(scriptObjectPointer, makeLink))
            {
                this->Out(" = 0x%p", scriptObjectPointer);
            }
            this->Out("]");
        }
    }

    return true;
}

void DumpPinnedObject(EXT_CLASS_BASE* ext, int i, int j, ULONG64 entryPointer, const PinnedObjectEntry& entry)
{
    ext->Out("Index: (0x%x, %d), Entry: 0x%p", i, j, entryPointer);
    ext->Out(", Key: 0x%p ", entry.address);
    ext->Out("(Ref count: %d)", entry.pinnedCount);
    // There appears to be a bug in ext->Out where it doesn't deal with %d properly when
    // mixed with other types

    ext->DumpPossibleSymbol(entry.address);

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
    "{;e,r;address;Address whose referrers to find}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;

    if (recyclerArg != NULL)
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

    if (recyclerArg != NULL)
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
    ULONG64 recyclerArg = GetUnnamedArgU64(0);
    ExtRemoteTyped recycler;
    ExtRemoteTyped threadContext;

    if (recyclerArg != NULL)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerArg);
        threadContext = CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();
        recycler = threadContext.Field("recycler");
    }

    RootPointerReader rootPointerManager(this, recycler);

    // TODO (doilij) refactor this with ComputeRoots below

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
        if (externalWeakReferenceList != NULL)
        {
            Out("External Weak Reference caches\n");
            DumpList<true>(this, externalWeakReferenceList, "ExternalWeakReferenceCache *");
        }
    }

    ExtRemoteTyped externalRootMarker = recycler.Field("externalRootMarker");
    if (externalRootMarker.GetPtr() != NULL)
    {
        Out("External root marker installed (Address: 0x%p), some roots might be missed\n", externalRootMarker.GetUlongPtr());
    }

    Out("\nPinned objects\n");
    uint count = 0;
    MapPinnedObjects(this, recycler, [&count, &rootPointerManager, this](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
    {
        DumpPinnedObject(this, i, j, entryPointer, entry);
        count++;
        rootPointerManager.TryAdd((ULONG64)entry.address, RootType::RootTypePinned);
    }, true);

    Out("Count is %d\n", count);

    Out("\nGuest arenas\n");
    ULONG64 guestArenaList = recycler.Field("guestArenaList").GetPointerTo().GetPtr();
    DumpList<false>(this, guestArenaList, FillModuleAndMemoryNS("%s!%sRecycler::GuestArenaAllocator"));

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
     * - Scan guest arena (if it's not pending delete)
     * - Scan external guest arena
     * - Scan stack
     */

    // TODO (doilij) cache the computation of this set of roots so the info can be easily reused

    //
    // Find Implicit roots
    //

    if (recycler.HasField("enableScanImplicitRoots") && recycler.Field("enableScanImplicitRoots").GetStdBool())
    {
        rootPointerManager.ScanImplicitRoots(dump);
    }

    //
    // Find external weak referenced roots
    //

    if (threadContext && threadContext->HasField("externalWeakReferenceCacheList"))
    {
        ExtRemoteTyped externalWeakReferenceCache = threadContext->Field("externalWeakReferenceCacheList");
        ULONG64 externalWeakReferenceList = externalWeakReferenceCache.GetPointerTo().GetPtr();
        if (externalWeakReferenceList != NULL)
        {
            // TODO: Implement external weak ref support
            // TODO: Fix RecyclerCachedData::GetRootPointers once this is implemented when thread context is not passed in
        }
    }

    //
    // Scan external roots
    //

    // (Nothing to do here; a warning is emitted elsewhere and no other action is taken.)

    //
    // Scan pinned objects
    //

    MapPinnedObjects(ext, recycler, [&rootPointerManager](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
    {
        rootPointerManager.TryAdd(entry.address, RootType::RootTypePinned);
    }, dump);

    //
    // Scan guest arena (if it's not pending delete)
    //

    ULONG64 recyclerAddress = recycler.m_Data; // TODO: recycler needs to be a pointer to make this work
    ULONG64 guestArenaList = recyclerAddress + recycler.GetFieldOffset("guestArenaList");

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

    //
    // Scan external guest arena
    //

    ExtRemoteTyped egal = recycler.Field("externalGuestArenaList");
    ULONG64 externalGuestArenaList = egal.GetPointerTo().GetPtr();
    RemoteListIterator<false> externalGuestArenaIterator("ArenaData *", externalGuestArenaList);
    while (externalGuestArenaIterator.Next())
    {
        ULONG64 dataPtr = externalGuestArenaIterator.GetDataPtr();
        rootPointerManager.ScanArenaData(GetPointerAtAddress(dataPtr));
    }

    //
    // Scan stack
    //

    rootPointerManager.ScanRegisters(ext, dump);
    rootPointerManager.ScanStack(ext, recycler, dump);

    return rootPointerManager.DetachAddresses();
}

#if ENABLE_MARK_OBJ
JD_PRIVATE_COMMAND(markobj,
    "Dumps a path to an object from the root (INCOMPLETE)",
    "{;e,r;address;Address to which the path should be traced}"
    "{;e,o,d=0;from;Root to trace from}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 root = GetUnnamedArgU64(1);
    ULONG64 recyclerArg = GetUnnamedArgU64(2);
    ExtRemoteTyped recycler;
    ExtRemoteTyped threadContext = GetCurrentThreadContext();

    if (recyclerArg != NULL)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerArg);
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
        RecyclerObjectGraph &objectGraph = *(this->GetOrCreateRecyclerObjectGraph(recycler, &threadContext));

        Out("Object graph constructed- finding path to 0x%p\n", address);
        objectGraph.FindPathTo(rootPointerManager, address, root);
    }
}
#endif

void DumpPointerPropertiesSeparatorLine(EXT_CLASS_BASE* ext)
{
    ext->Out("----------------------------+-------------------------------------\n");
}

void DumpPointerPropertiesPredecessorsHeader(EXT_CLASS_BASE* ext)
{
    ext->Out("----------------------------+-Predecessors------------------------\n");
}

void DumpPointerPropertiesDescendantsHeader(EXT_CLASS_BASE* ext)
{
    ext->Out("----------------------------+-Descendants-------------------------\n");
}

void DumpPointerPropertiesHorizontalSpacer(EXT_CLASS_BASE* ext)
{
    ext->Out("                            | ");
}

void DumpPointerPropertiesSpacerLine(EXT_CLASS_BASE* ext)
{
    ext->Out("                            |\n");
}

void DumpPointerPropertiesHeader(EXT_CLASS_BASE* ext)
{
    ext->Out("\n");
    ext->Out("              P             | Pinned Root\n");
    ext->Out("               S            | Stack Root\n");
    ext->Out("                R           | Register Root\n");
    ext->Out("                 A          | Arena Root\n");
    ext->Out("                  I         | Implicit Root\n");
    ext->Out("                    ^       | Click to execute `!jd.predecessors` on this node\n");
    ext->Out("                      v     | Click to execute `!jd.predecessors` on this node\n");
    ext->Out("                        >   | Click to execute `!jd.traceroots` on this node\n");
    ext->Out("  Pred   Succ Flags       * | Original input pointer\n");
    DumpPointerPropertiesSeparatorLine(ext);
}

typedef RecyclerObjectGraph::GraphImplNodeType NodeType;
typedef RecyclerObjectGraph::GraphImplNodeType *Node;

// Format the following root type flags into a buffer for display:
// P     - Pinned   root
//  S    - Stack    root
//   R   - Register root
//    A  - Arena    root
//     I - Implicit root
template <>
void FormatPointerFlags(char *buffer, uint bufferLength, Node node)
{
    RootType rootType = node->GetRootType();
    bool isPinned     = RootTypeUtils::IsType(rootType, RootType::RootTypePinned);      // P
    bool isStack      = RootTypeUtils::IsType(rootType, RootType::RootTypeStack);       //  S
    bool isRegister   = RootTypeUtils::IsType(rootType, RootType::RootTypeRegister);    //   R
    bool isArena      = RootTypeUtils::IsType(rootType, RootType::RootTypeArena);       //    A
    bool isImplicit   = RootTypeUtils::IsType(rootType, RootType::RootTypeImplicit);    //     I

    Assert(bufferLength > 5);
    if (bufferLength <= 5)
    {
        if (bufferLength > 0)
        {
            buffer[0] = NULL; // terminate the string immediately to prevent buffer overrun issues
        }

        throw exception("FormatPointerFlags: buffer is not long enough to format flags correctly.");
    }

    // manually construct because we're building one character at a time and this is faster than printf format string parsing.
    buffer[0] = isPinned    ? 'P' : ' ';
    buffer[1] = isStack     ? 'S' : ' ';
    buffer[2] = isRegister  ? 'R' : ' ';
    buffer[3] = isArena     ? 'A' : ' ';
    buffer[4] = isImplicit  ? 'I' : ' ';
    buffer[5] = NULL; // we know that this index is valid because of earlier checks, NULL terminate the string here

    // write redundant NULL terminator at the end of the buffer as calculated by bufferLength, as a sanity check
    buffer[bufferLength - 1] = NULL;
}

void DumpPointerProperties(EXT_CLASS_BASE* ext, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 address = NULL, int currentLevel = 0)
{
    if (address == NULL)
    {
        address = pointerArg;
    }

    //
    // Print the flags and spacing before the rest of the info
    //

    auto node = objectGraph.FindNode(address);
    size_t pred = node->GetPredecessorCount();
    size_t desc = node->GetSuccessorCount();
    ext->Out("%6d ", pred);
    ext->Out("%6d ", desc);

    const uint bufferLength = 6; // space for 5 flags plus NULL
    char buffer[bufferLength];
    FormatPointerFlags(buffer, bufferLength, node);
    ext->Out("%s ", buffer);

    ext->Dml("<link cmd=\"!jd.predecessors -limit 0 0x%p\">^</link> ", address);
    ext->Dml("<link cmd=\"!jd.successors -limit 0 0x%p\">v</link> ", address);

    bool isInput = (pointerArg == address); // display * or > with link as appropriate
    if (isInput)
    {
        ext->Out("  "); // spacer for >
        ext->Dml("<link cmd=\"!jd.traceroots 0x%p\">*</link>", address);
    }
    else
    {
        ext->Dml("<link cmd=\"!jd.traceroots 0x%p\">&gt;</link>", address);
        ext->Out("  "); // spacer for *
    }

    ext->Out(" | ");

    // print the address and symbol
    ext->Out("0x%p ", address);
    ext->Out("(level %c%-8d)", (currentLevel < 0 ? '-' : ' '), abs(currentLevel));
    ext->DumpPossibleSymbol(address);
    ext->Out("\n");
}

template <bool predecessorsMode = false, bool links = true>
void DumpPredSucc(EXT_CLASS_BASE* ext, Node currentNode, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 limitArg, bool showOnlyRoots)
{
    // Display the target pointer and one level of its descendants
    DumpPointerProperties(ext, objectGraph, pointerArg, pointerArg, 0);

    int level = 1;
    char *command = "successors";
    auto countFunction = &NodeType::GetSuccessorCount;
    if (predecessorsMode)
    {
        level = -1;
        command = "predecessors";
        countFunction = &NodeType::GetPredecessorCount;
    }

    uint count = 0;
    auto mapper = [ext, &objectGraph, pointerArg, &count, limitArg, level, command, showOnlyRoots, &countFunction, currentNode](Node next)
    {
        RootType rootType = next->GetRootType();
        if (showOnlyRoots && !RootTypeUtils::IsAnyRootType(rootType))
        {
            return false;
        }

        ULONG64 address = next->Key();
        DumpPointerProperties(ext, objectGraph, pointerArg, address, level);

        // Display all children if the value is 0, otherwise display up to count children.
        if (limitArg != 0 && (++count) >= limitArg)
        {
            DumpPointerPropertiesHorizontalSpacer(ext);
            uint nodeCount = (currentNode->*countFunction)();
            ext->Out("Limit Reached. %d more not displayed.", nodeCount - count);
            ext->Dml(" <link cmd=\"!jd.%s /limit 0 0x%p\">(Display all %s.)</link>\n", command, pointerArg, command);
            return true;
        }

        return false;
    };

    if (predecessorsMode)
    {
        currentNode->MapPredecessors(mapper);
    }
    else
    {
        currentNode->MapSuccessors(mapper);
    }

    if (limitArg == 0 && links)
    {
        ext->Out("\n");
        if (showOnlyRoots)
        {
            ext->Dml("<link cmd=\"!jd.%s /limit 0 0x%p\">(Display all %s.)</link>\n", command, pointerArg, command);
        }
        else
        {
            ext->Dml("<link cmd=\"!jd.%s /r /limit 0 0x%p\">(Display only roots.)</link>\n", command, pointerArg);
        }
    }
}

template <bool predecessorsMode = false>
void PredSuccImpl(EXT_CLASS_BASE *ext)
{
    auto predSuccFn = &DumpSuccessors<true>;
    if (predecessorsMode)
    {
        predSuccFn = &DumpPredecessors<true>;
    }

    const ULONG64 pointerArg = ext->GetUnnamedArgU64(0);
    const ULONG64 recyclerArg = ext->GetUnnamedArgU64(1);
    const ULONG64 limitArg = ext->GetArgU64("limit");
    const bool showOnlyRoots = ext->HasArg("r");

    if (pointerArg == NULL)
    {
        ext->Out("Please specify a non-null object pointer.\n");
        return;
    }

    //
    // Perform necessary setup.
    //

    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;
    if (recyclerArg != NULL)
    {
        ext->Out("Manually provided Recycler pointer: 0x%p\n", recyclerArg);
        recycler = ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerArg);
        threadContext = ext->CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        RemoteThreadContext remoteThreadContext = RemoteThreadContext::GetCurrentThreadContext();
        threadContext = remoteThreadContext.GetExtRemoteTyped();
        recycler = remoteThreadContext.GetRecycler().GetExtRemoteTyped();
    }

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext));
    Node node = objectGraph.FindNode(pointerArg);

    //
    // Actually dump the pointers.
    //

    DumpPointerPropertiesHeader(ext);
    predSuccFn(ext, node, objectGraph, pointerArg, limitArg, showOnlyRoots);
}

template <bool links = true>
void DumpPredecessors(EXT_CLASS_BASE* ext, Node node, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 limitArg, bool showOnlyRoots)
{
    DumpPredSucc<true, links>(ext, node, objectGraph, pointerArg, limitArg, showOnlyRoots);
}

template <bool links = true>
void DumpSuccessors(EXT_CLASS_BASE* ext, Node node, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 limitArg, bool showOnlyRoots)
{
    DumpPredSucc<false, links>(ext, node, objectGraph, pointerArg, limitArg, showOnlyRoots);
}

JD_PRIVATE_COMMAND(predecessors,
    "Given a pointer in the graph, show all of its descendants.",
    "{;ed,o,d=0;pointer;Address to trace}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{r;b,o;onlyRoots;Only show predecessors which are also roots}"
    "{limit;edn=(10),o,d=10;limit;Number of nodes to list}")
{
    PredSuccImpl<true>(this);
}

JD_PRIVATE_COMMAND(successors,
    "Given a pointer in the graph, show all of its descendants.",
    "{;ed,o,d=0;pointer;Address to trace}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{r;b,o;onlyRoots;Only show descendants which are also roots}"
    "{limit;edn=(10),o,d=10;limit;Number of nodes to list}")
{
    PredSuccImpl<false>(this);
}

// TODO (doilij) revise documentation for !jd.traceroots

// !jd.traceroots algorithm:
//
// We will trace the RecyclerObjectGraph from the given address to the closest root, and provide information
// about every object we encounter along the way to allow an easier interface to analyze the contents of the
// object graph which does not require an external tool to analyze the output of !jd.savegraph to even get
// started with analysis. Additionally, this *may* provide more detailed or accurate analysis that cannot be
// conducted with another tool.
//
// By default we want to stop when we hit the first root, but I've added a parameter to tune the number of
// roots the traversal can hit before it must stop.
//
// Initially we perform some necessary setup by collecting information that will be needed to initialize the
// algorithm, declare some types to make the code more readable, and declare a bunch of state which will be
// needed for traversal.
//
// We've opted to do a non-recursive BFS algorithm by taking advantage of a queue and some local state to
// visit every node in the traversal in FIFO order. The state variables such as currentLevel are used to
// restore the state according to the node we're processing to make sure that we respect its frame of reference.
//
// We follow a 3-stage traversal algorithm.
//
// Pass 0: Seed the traversal.
//
// Initially we need to find a starting point. We get the root node of the graph and associate some initial traversal
// data, and add that to a hash map of traversal data. Then we traverse the parents of that node as an initial
// pass. If there are no parents, this is a no-op and the next step completes trivially after 0 iterations.
//
// Pass 1: Traverse upwards to roots.
//
// From this starting point we traverse up towards the roots by taking each node from the queue, traversing it's
// parents, and adding all of the parents with their new traversal data.
//
// Each node in the queue at any point in time is the deepest (where 'deep' means in the direction of the graph roots)
// node on some path from the target pointer to the graph roots.
//
// When we hit a graph root where we must stop traversing upwards, we add it to the rootQueue. The first node in the
// rootQueue will be the one for which there is the shortest path from root to destination pointer. This is because of
// the BFS traversal.
//
// Pass 2: Traverse downwards from the roots the specified number of levels.
//
// One at a time, we take a node from the rootQueue and add it to the nodeQueue and use a descent function to traverse
// from the root to the destination. We use the traversal data to confirm we are taking one step at a time in the right
// direction towards the target pointer as we traverse each node's children.
//
// Finally we print the results in a pretty table where we display information about each pointer to allow for easier
// analysis of the results using other JD and WinDbg commands.
//
JD_PRIVATE_COMMAND(traceroots,
    "Given a pointer in the graph, perform a BFS traversal to find the shortest path to a root.",
    "{;ed,o,d=0;pointer;Address to trace}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{roots;edn=(10),o,d=1;numroots;Stop after hitting this many roots along a traversal (0 for full traversal)}"
    "{limit;edn=(10),o,d=10;limit;Number of descendants or predecessors to list}"
    "{t;b,o;transientRoots;Use Transient Roots}"
    "{a;b,o;all;Shortest path to all roots}"
    "{pred;b,o;showPredecessors;Show up to limit predecessors in the output}")
{
    const ULONG64 pointerArg = GetUnnamedArgU64(0);
    const ULONG64 recyclerArg = GetUnnamedArgU64(1);
    const ULONG64 numRootsArg = GetArgU64("roots");
    const ULONG64 limitArg = GetArgU64("limit");
    const bool transientRoots = HasArg("t");
    const bool allShortestPath = HasArg("a");
    const bool showPredecessors = HasArg("pred");

    if (pointerArg == NULL)
    {
        this->Out("Please specify a non-null object pointer.\n"
            "Use output of !jd.savegraph to see all objects' addresses in the recycler graph.\n"
            "Try one of the following to find some interesting points in the graph:\n"
            "    !jd.showpinned\n"
            "    !jd.showroots\n"
            "    !jd.jsobjectnodes\n");
        return;
    }

    //
    // Perform necessary setup.
    //

    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;
    if (recyclerArg != NULL)
    {
        this->Out("Manually provided Recycler pointer: 0x%p\n", recyclerArg);
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerArg);
        threadContext = CastWithVtable(recycler.Field("collectionWrapper"));
    }
    else
    {
        RemoteThreadContext remoteThreadContext = RemoteThreadContext::GetCurrentThreadContext();
        threadContext = remoteThreadContext.GetExtRemoteTyped();
        recycler = remoteThreadContext.GetRecycler().GetExtRemoteTyped();
    }

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext));

    //
    // Data types and traversal state
    //

    struct TraversalData
    {
        ULONG64 address;
        ULONG64 rootHitCount;
        TraversalData * child;
        int level;
    };

    typedef HashMap<ULONG64, TraversalData *>                   TraversalMap;
    typedef TraversalMap::EntryType::first_type                 TraversalMapKey;
    typedef TraversalMap::EntryType::second_type                TraversalMapValue;

    typedef std::queue<std::pair<Node, TraversalData *>>        NodeQueue;
    typedef NodeQueue::container_type::value_type::first_type   NodeQueueNode;
    typedef NodeQueue::container_type::value_type::second_type  NodeQueueData;

    // Use this to store information about the traversal state at a given node
    TraversalMap traversalMap;

    // Used to traverse toward roots in BFS order
    NodeQueue nodeQueue;
    // Populate roots as we come to them, so we can start at each of these roots for the output phase
    NodeQueue rootQueue;

    //
    // Pass 0: Seed the traversal.
    //

    Node node = objectGraph.FindNode(pointerArg);

    if (node == nullptr)
    {
        this->Err("ERROR: %p not a GC pointer\n", pointerArg);
        return;
    }

    // Initially add the current node to the hash with level 0 and 0 roots (initial values)
    // Even if the first node is a root we don't care about that. We probably want to see one level past that if possible.
    TraversalData * data = new TraversalData{ node->Key(), 0, nullptr, 0 };
    traversalMap.Add(node->Key(), data);
    nodeQueue.push(std::make_pair(node, data));

    //
    // Pass 1: Traverse upwards to roots.
    //

    // Continue traversing until ONE of the following conditions occurs:
    // * nodeQueue is empty (nothing more to traverse)
    // * Both of the following:
    //   * numRootsArg != 0 (if it were 0, we should traverse all the way to the roots)
    //   * currentRootHitCount >= numRootsArg
    while (!nodeQueue.empty())
    {
        auto current = nodeQueue.front(); // retrieve
        nodeQueue.pop(); // remove from front
        Node currentNode = current.first;
        TraversalData *currentData = current.second;

        ULONG64 currentRootHitCount = currentData->rootHitCount; // the current count of roots encountered on the current traversal path

        RootType rootType = currentNode->GetRootType();
        Assert(currentNode->GetPredecessorCount() != 0 || RootTypeUtils::IsAnyRootType(rootType));
        bool allowedRoot = (transientRoots || RootTypeUtils::IsNonTransientRootType(rootType));

        // If this is an allowed root, record it as a root pointer.
        // Make sure the pointer is not the one we started with because
        // it is not useful to consider the node we started at as a root of the traversal.
        if (allowedRoot && RootTypeUtils::IsAnyRootType(rootType)
            && pointerArg != currentNode->Key())
        {
            currentRootHitCount++;
            rootQueue.push(current);
            if (!allShortestPath)
            {
                // we found the root corresponding to the shortest path (by the properties of BFS), so stop traversing
                break;
            }
        }

        // Ascend upwards towards the roots
        currentNode->MapAllPredecessors([this, currentData, &traversalMap, &nodeQueue, numRootsArg, currentRootHitCount](Node parent)
        {
            ULONG64 address = parent->Key();

            // Don't keep traversing from this node if the currentRootHitCount is too large
            if (numRootsArg != 0 && currentRootHitCount >= numRootsArg)
            {
                return;
            }

            int currentLevel = currentData->level - 1;
            if (traversalMap.Contains(address))
            {
#if defined(DBG)
                auto node = traversalMap.Get(address);
                Assert(node->level >= currentLevel);
#endif
                return;
            }

            TraversalData *pTraversalData = new TraversalData{ address, currentRootHitCount, currentData, currentLevel };

            traversalMap.Add(address, pTraversalData);

            // actually push the parent nodes
            nodeQueue.push(std::make_pair(parent, pTraversalData));
        });
    }

    //
    // DUMP OUTPUT
    //

    this->Out("\n");

    if (transientRoots)
    {
        this->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Ignore transient recycler roots for traversal root limit.)</link>\n",
            numRootsArg, pointerArg, recyclerArg);
    }
    else
    {
        this->Dml("<link cmd=\"!jd.traceroots /t /roots %d 0x%p 0x%p\">(Use transient recycler roots for traversal root limit.)</link>\n",
            numRootsArg, pointerArg, recyclerArg);
    }

    if (numRootsArg == 0)
    {
        this->Out("Traversing as far as possible.\n");
        this->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Traverse through just one recycler root.)</link>\n",
            1, pointerArg, recyclerArg);
    }
    else
    {
        this->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Traverse as far as possible.)</link>\n",
            0, pointerArg, recyclerArg);
        this->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Traverse through %d recycler roots.)</link>\n",
            numRootsArg + 1, pointerArg, recyclerArg, numRootsArg + 1);
    }

    DumpPointerPropertiesHeader(this);

    while (!rootQueue.empty())
    {
        auto root = rootQueue.front();
        rootQueue.pop();

        NodeQueueNode rootNode = root.first;
        ULONG64 address = rootNode->Key();
        TraversalData *pTraversalData = traversalMap.Get(address);
        while (pTraversalData != nullptr)
        {
            DumpPointerProperties(this, objectGraph, pointerArg, pTraversalData->address, pTraversalData->level);
            pTraversalData = pTraversalData->child;
        }

        DumpPointerPropertiesSeparatorLine(this);
    }

    // Only display the descendants section if node actually has descendants.
    if (node->GetSuccessorCount() > 0)
    {
        DumpPointerPropertiesSpacerLine(this);
        DumpPointerPropertiesDescendantsHeader(this);
        DumpSuccessors<false>(this, node, objectGraph, pointerArg, limitArg, false);
    }

    // Display an option to show predecessors if there are any.
    if (showPredecessors && node->GetPredecessorCount() > 0)
    {
        DumpPointerPropertiesSpacerLine(this);
        DumpPointerPropertiesPredecessorsHeader(this);
        DumpPredecessors<false>(this, node, objectGraph, pointerArg, limitArg, false);
    }

    // Delete all of the TraversalData pointers in traversalMap so we don't leak.
    traversalMap.MapAll([](TraversalMapKey key, TraversalMapValue value)
    {
        delete value; // `value` is the TraversalData pointer
    });
}

JD_PRIVATE_COMMAND(savegraph,
    "Saves the current recycler object graph into a file (js, python, csv, etc.)",
    "{;s;filename;Filename to output to}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{;s,o,d=js;filetype;Save file type <js|python|csv|csvx>}")
{
    PCSTR filename = GetUnnamedArgStr(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    PCSTR filetype = GetUnnamedArgStr(2);

    ExtRemoteTyped threadContext;
    ExtRemoteTyped recycler;
    if (recyclerArg != NULL)
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

    Addresses * rootPointerManager = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    Out("\nNumber of root GC pointers found: %d\n\n", rootPointerManager->Count());

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext));

    Out("Saving object graph to %s\n", filename);
    if (_stricmp(filetype, "js") == 0)
    {
        objectGraph.DumpForJs(filename);
    }
    else if (_stricmp(filetype, "python") == 0)
    {
        objectGraph.DumpForPython(filename);
    }
    else if (_stricmp(filetype, "csv") == 0)
    {
        objectGraph.DumpForCsv(filename);
    }
    else if (_stricmp(filetype, "csvx") == 0)
    {
        objectGraph.DumpForCsvExtended(this, filename);
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
    "{;ed,o,d=0;recycler;Recycler address}"
    "{top;edn=(10),o,d=-1;count;Number of entries to display}"
    "{v;b,o;verbose;Display verbose tracing}"
    "{t;b,o;trident;Display trident symbols}"
    "{sc;b,o;sortByCount;Sort by count instead of bytes}"
    "{sn;b,o;sortByName;Sort by name instead of bytes}"
    "{su;b,o;sortByUnknown;Sort by unknown}"
    "{vt;b,o;vtable;Vtable Only}"
    "{u;b,o;grouped;Show unknown count}"
    "{k;b,o;known;Known objects only}"
    )
{
    const ULONG64 recyclerArg = GetUnnamedArgU64(0);
    const ULONG64 limit = GetArgU64("top");
    const bool verbose = HasArg("v");
    const bool trident = HasArg("t");
    const bool sortByCount = HasArg("sc");
    const bool sortByName = HasArg("sn");
    const bool sortByUnknown = HasArg("su");
    const bool infer = !HasArg("vt");
    const bool showUnknown = HasArg("u");
    const bool knownOnly = HasArg("k");

    if (sortByCount && sortByName)
    {
        throw ExtException(E_FAIL, "Can't specify both -sc and -sn");
    }

    if (sortByUnknown && sortByName)
    {
        throw ExtException(E_FAIL, "Can't specify both -su and -sn");
    }

    // Note: (sortByCount && sortByUnknown) is allowed -- see below

    auto sortComparer = sortByName ? ObjectAllocNameComparer :
        sortByUnknown ?
        (sortByCount ? ObjectAllocUnknownCountComparer : ObjectAllocUnknownSizeComparer) :
        (sortByCount ? ObjectAllocCountComparer : ObjectAllocSizeComparer);

    ExtRemoteTyped recycler = GetRecycler(recyclerArg);
    ExtRemoteTyped threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();

    if (verbose)
    {
        Addresses * rootPointerManager = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
        Out("\nNumber of root GC pointers found: %d\n\n", rootPointerManager->Count());
    }

    RecyclerObjectGraph::TypeInfoFlags flags = RecyclerObjectGraph::TypeInfoFlags::None;
    if (infer)
    {
        flags = (RecyclerObjectGraph::TypeInfoFlags)(flags | RecyclerObjectGraph::TypeInfoFlags::Infer);
    }
    if (trident)
    {
        flags = (RecyclerObjectGraph::TypeInfoFlags)(flags | RecyclerObjectGraph::TypeInfoFlags::Trident);
    }
    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext, flags));

    stdext::hash_map<char const *, ObjectAllocStats> objectCounts;
    int numNodes = 0;
    int totalSize = 0;

    auto addStats = [&](RecyclerObjectGraph::GraphImplNodeType * node)
    {
        char const * typeName = infer ? node->GetTypeNameOrField() : node->GetTypeName();
        auto i = objectCounts.find(typeName);
        if (i != objectCounts.end())
        {
            ObjectAllocStats& stats = (*i).second;
            stats.count++;
            stats.size += node->GetObjectSize();
            stats.unknownCount += node->IsPropagated();
            stats.unknownSize += node->IsPropagated() ? node->GetObjectSize() : 0;
        }
        else
        {
            ObjectAllocStats stats;
            stats.count = 1;
            stats.size = node->GetObjectSize();
            stats.unknownCount = node->IsPropagated();
            stats.unknownSize = node->IsPropagated() ? node->GetObjectSize() : 0;
            stats.hasVtable = node->HasVtable();
            objectCounts[typeName] = stats;
        }
    };

    objectGraph.MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        numNodes++;
        totalSize += (node->GetObjectSize());
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
        Out("%7u %11u %5.1f%% %5.1f%% %s%s\n", currCount, currSize, (float)currCount / (float)numNodes * 100,
            (float)currSize / (float)totalSize * 100,
            stats.hasVtable ? (knownOnly ? "" : "[Group] ") : "[Field] ", typeName);

        if (i > limit)
        {
            Out("<%d limit reached>\n", limit);
            break;
        }
    }

    Out("----------------------------------------------------------------------------\n");

    const uint unknownTotalCount = numNodes - knownObjectCount;
    const uint unknownTotalSize = totalSize - knownObjectSize;
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

struct SortNodeBySuccessor
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetSuccessorCount() > right->GetSuccessorCount() ||
            (left->GetSuccessorCount() == right->GetSuccessorCount() && left->Key() < right->Key());
    }
};

struct SortNodeByPredecessor
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetPredecessorCount() > right->GetPredecessorCount() ||
            (left->GetPredecessorCount() == right->GetPredecessorCount() && left->Key() < right->Key());
    }
};

struct SortNodeByObjectSize
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetObjectSize() > right->GetObjectSize() ||
            (left->GetObjectSize() == right->GetObjectSize() && left->Key() < right->Key());
    }
};

JD_PRIVATE_COMMAND(jsobjectnodes,
    "Dump a table of object nodes sorted by number of successors, number or predecessors, or size.",
    "{;e,o,d=0;recycler;Recycler address}"
    "{ti;b,o;typeInfo;Type info}"
    "{sp;b,o;predecssorCount;Sort by predecessor count}"
    "{ss;b,o;objectSize;Sort by object size}"
    "{limit;edn=(10),o,d=10;nodes;Number of nodes to display}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped recycler = GetRecycler(arg);
    ULONG64 limit = GetArgU64("limit");
    bool typeInfo = HasArg("ti");
    bool sortByPred = HasArg("sp");
    bool sortBySize = HasArg("ss");

    if (sortByPred && sortBySize)
    {
        this->Err("ERROR: -sp and -ss can't be specified together\n");
        return;
    }

    ExtRemoteTyped threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext, RecyclerObjectGraph::TypeInfoFlags::Infer));

    this->Out("\n");
    this->Out("%22s ^     Run !jd.predecessors on this node\n", "");
    this->Out("%22s   v   Run !jd.successors on this node\n", "");
    this->Out("%22s     > Run !jd.traceroots on this node\n", "");
    this->Out("%6s %6s %8s %5s %-18s %s\n", "Pred", "Succ", "Size", "", "Address", "Type");
    this->Out("------ ------ -------- ----- ------------------ --------------\n");

    ULONG64 count = 0;
    auto output = [&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        this->Out("%6d %6d %8d ", node->GetPredecessorCount(), node->GetSuccessorCount(), node->GetObjectSize());

        this->Dml("<link cmd=\"!jd.predecessors -limit 0 0x%p\">^</link> ", node->Key());
        this->Dml("<link cmd=\"!jd.successors -limit 0 0x%p\">v</link> ", node->Key());
        this->Dml("<link cmd=\"!jd.traceroots 0x%p\">&gt;</link> ", node->Key());

        this->Out("0x%p", node->Key());

        if (typeInfo)
        {
            this->Out(" %s%s", node->Key(), node->IsPropagated() ? "[RefBy] " : "", node->GetTypeName());
        }
        else
        {
            DumpPossibleSymbol(node->Key());
        }

        this->Out("\n");

        // limit == 0 means show all nodes
        if (limit != 0 && (++count) >= limit)
        {
            this->Out("\nLimit of %d reached.", limit);
            this->Dml(" <link cmd=\"!jd.jsobjectnodes -limit %d %s%s%s\">(Display %d more.)</link>",
                limit * 2,
                typeInfo ? " -ti" : "", sortByPred ? " -sp" : "", sortBySize ? " -ss" : "",
                limit);
            this->Dml(" <link cmd=\"!jd.jsobjectnodes -limit %d %s%s%s\">(Display all.)</link>",
                0,
                typeInfo ? " -ti" : "", sortByPred ? " -sp" : "", sortBySize ? " -ss" : "");
            this->Out("\n");
            return true;
        }

        return false;
    };

    if (sortBySize)
    {
        objectGraph.MapSorted<SortNodeByObjectSize>(output);
    }
    else if (sortByPred)
    {
        objectGraph.MapSorted<SortNodeByPredecessor>(output);
    }
    else
    {
        objectGraph.MapSorted<SortNodeBySuccessor>(output);
    }

    this->Out("--------------------------------------------------------------\n");
    this->Out("Total %d nodes, %d edges\n", objectGraph.GetNodeCount(), objectGraph.GetEdgeCount());
}

#endif // JD_PRIVATE
