//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "jdrecycler.h"
#include "RemoteRecyclerList.h"
#include "RecyclerRoots.h"

#include <hash_set>
#include <stack>
#include <queue>
#include "RecyclerObjectGraph.h"
#include "RemoteHeapBlockMap.h"
#include "RemoteObjectInfoBits.h"
#include "ProgressTracker.h"
#include "RemoteJavascriptLibrary.h"

ULONG64 GetStackTop()
{
    // Fix for x64?
    ULONG64 offset;
    GetExtension()->m_Registers2->GetStackOffset(&offset);
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
        currentEntry.stackBackTrace = (TPointerType)_transientPinnedObjectStackBackTrace;
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

                if (entry.value != 0)
                {
                    currentEntry.address = entry.key;
                    currentEntry.pinnedCount = entry.value;
                    currentEntry.stackBackTrace = 0;
                    fn(i, j++, current, currentEntry);
                }
                else
                {
                    Assert(this->_hasPendingUnpinnedObject);
                }
                current = (TPointerType)entry.next;
            }
            else
            {
                DebugHashEntry entry;
                remoteEntry.ReadBuffer(&entry, sizeof(DebugHashEntry));

                if (entry.value.refCount != 0)
                {
                    currentEntry.address = entry.key;
                    currentEntry.pinnedCount = entry.value.refCount;
                    currentEntry.stackBackTrace = entry.value.backTraceNode;
                    fn(i, j++, current, currentEntry);
                }
                else
                {
                    Assert(this->_hasPendingUnpinnedObject);
                }
                current = (TPointerType)entry.next;
            }
        }
    }

    free(pinnedObjectMapEntries);
}

void RootPointerReader::ScanRegisters(bool print)
{
    ULONG numRegisters;
    ExtCheckedPointer<IDebugRegisters2> pRegisters = GetExtension()->m_Registers2;

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

        if (GetExtension()->m_PtrSize == 4)
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

        ULONG64 value = GetExtension()->m_PtrSize == 4? debugValue.I32 : debugValue.I64;

        if (this->TryAdd(value, RootType::RootTypeRegister) && print)
        {
            if (print)
            {
                GetExtension()->Out("0x%p (Register %s)\n", value, buffer);
            }
        }
    }

    if (print)
    {
        GetExtension()->Out("Number of scanned registers: %d\n", scannedRegisters);
        GetExtension()->Out("Number of total registers: %d\n", numRegisters);
    }
}

void RootPointerReader::ScanStack(RemoteRecycler& recycler, ULONG64 stackTop, bool print, bool showScriptContext)
{
    ULONG64 stackBase = 0;
    if (recycler.GetExtRemoteTyped().HasField("stackBase") && recycler.GetExtRemoteTyped().Field("mainThreadHandle").GetPtr() != 0)
    {
        stackBase = (ULONG64)((GetExtension()->m_PtrSize == 4) ? recycler.GetExtRemoteTyped().Field("stackBase").GetUlong()
            : recycler.GetExtRemoteTyped().Field("stackBase").GetPtr());
    }
    else
    {        
        ExtRemoteTyped stackBaseField = ExtRemoteTypedUtil::GetTeb().Field("NtTib.StackBase");
        stackBase = (GetExtension()->m_PtrSize == 4) ? stackBaseField.GetUlong() : stackBaseField.GetUlong64();
    }

    size_t stackSizeInBytes = (size_t)(stackBase - stackTop);
    byte * stack = (byte *)malloc(stackSizeInBytes);
    if (!stack)
    {
        GetExtension()->ThrowOutOfMemory();
    }
    ULONG stackSizeInBytesLong = (ULONG)stackSizeInBytes;
    GetExtension()->Out("Scanning stack %d bytes starting from 0x%p to 0x%p\n", stackSizeInBytes, stackTop, stackBase);

    ExtRemoteData data(stackTop, stackSizeInBytesLong);
    data.ReadBuffer(stack, stackSizeInBytesLong);

    if (GetExtension()->m_PtrSize == 4)
    {
        ULONG32* stack32 = (ULONG32*)stack;
        for (uint i = 0; i < (uint)stackSizeInBytesLong / GetExtension()->m_PtrSize; i++)
        {
            ULONG64 address = (ULONG64)stack32[i];
            if (this->TryAdd(address, RootType::RootTypeStack))
            {
                if (print)
                {
                    GetExtension()->Out("0x%p", stack32[i]);
                    GetExtension()->Out(" (+0x%x)", i * GetExtension()->m_PtrSize);
                    GetExtension()->DumpPossibleSymbol(address, true, showScriptContext);
                    GetExtension()->Out("\n");
                }
            }
        }
    }
    else
    {
        ULONG64* stack64 = (ULONG64*)stack;
        for (uint i = 0; i < (uint)stackSizeInBytesLong / GetExtension()->m_PtrSize; i++)
        {
            ULONG64 address = stack64[i];
            if (this->TryAdd(address, RootType::RootTypeStack) && print)
            {
                if (print)
                {
                    GetExtension()->Out("0x%p", address);
                    GetExtension()->Out(" (+0x%x)", i * GetExtension()->m_PtrSize);
                    GetExtension()->DumpPossibleSymbol(address, true, showScriptContext);
                    GetExtension()->Out("\n");
                }
            }
        }
    }

    free(stack);
}

void RootPointerReader::ScanObject(ULONG64 object, ULONG64 bytes, RootType rootType)
{
    struct Context
    {
        ULONG64 start;
        ULONG64 current;
    } context;

    context.start = object;

    ULONG64 remainingBytes = bytes;
    ULONG64 curr = object;
    while (remainingBytes != 0)
    {
        ULONG readBytes = remainingBytes < 4096 ? (ULONG)remainingBytes : 4096;
        byte buffer[4096];
        ExtRemoteData data(curr, readBytes);
        data.ReadBuffer(buffer, readBytes);
        ULONG numPointers = readBytes / GetExtension()->m_PtrSize;
        byte * currBuffer = buffer;
        for (uint i = 0; i < numPointers; i++)
        {
            context.current = curr + (currBuffer - buffer);

            this->TryAdd(GetExtension()->m_PtrSize == 8 ? *(ULONG64 *)currBuffer : *(ULONG *)currBuffer, rootType, &context);
            currBuffer += GetExtension()->m_PtrSize;
        }

        remainingBytes -= readBytes;
        curr += (ULONG64)readBytes;
    }
}

void RootPointerReader::ScanArenaBigBlocks(ExtRemoteTyped blocks)
{
    while (blocks.GetPtr() != NULL)
    {
#if VERBOSE
        blocks.OutFullValue();
#endif

        ExtRemoteTyped block = blocks.Dereference();
        ULONG64 blockBytes = blocks.GetPtr() + GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sBigBlock))"));
        ExtRemoteTyped nBytesField = blocks.Field("currentByte");
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
    while (blocks.GetPtr() != NULL)
    {
        ULONG64 blockBytes = blocks.GetPtr() + GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sArenaMemoryBlock))"));
        ExtRemoteTyped nBytesField = blocks.Field("nbytes");
        size_t byteCount = (size_t)nBytesField.GetLong();
        ScanObject(blockBytes, byteCount, RootType::RootTypeArena);
        blocks = blocks.Field("next");
    }
}

void RootPointerReader::ScanArenaData(ULONG64 arenaDataPtr)
{
    ExtRemoteTyped arenaData(GetExtension()->FillModuleAndMemoryNS("(%s!%sArenaData*)@$extin"), arenaDataPtr);
    ScanArenaBigBlocks(arenaData.Field("bigBlocks"));
    ScanArenaBigBlocks(arenaData.Field("fullBlocks"));
    ScanArenaMemoryBlocks(arenaData.Field("mallocBlocks"));
}

void RootPointerReader::PrintArenaName(ULONG64 arena)
{
    RemoteThreadContext::ForEach([=](RemoteThreadContext threadContext)
    {
        return threadContext.ForEachScriptContext([=](RemoteScriptContext scriptContext)
        {
            return scriptContext.ForEachArenaAllocator([&](char const * name, JDRemoteTyped arenaAllocator)
            {
                if (arena == arenaAllocator.GetObjectPtr())
                {
                    GetExtension()->Out(" %s from ScriptContext 0x%p", name, scriptContext.GetPtr());
                    return true;
                }
                return false;
            });
        });
    });
}

void RootPointerReader::ScanArena(ULONG64 arena, bool verbose)
{
    if (verbose)
    {
        GetExtension()->Out("Scanning arena 0x%p", arena);
        PrintArenaName(arena);
        GetExtension()->Out("\n");
    }
    ULONG64 arenaData = arena + GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sAllocator))"));
    ScanArenaData(arenaData);
}

void RootPointerReader::ScanImplicitRoots(bool print)
{
    ULONG64 implicitRootCount;
    ULONG64 implicitRootSize;

    RemoteHeapBlockMap hbm = _recycler.GetHeapBlockMap();

    hbm.ForEachHeapBlock([&](RemoteHeapBlock& remoteHeapBlock)
    {
        if (remoteHeapBlock.IsLargeHeapBlock())
        {
            ULONG64 sizeOfObjectHeader = GetExtension()->recyclerCachedData.GetSizeOfLargeObjectHeader();
            return remoteHeapBlock.ForEachLargeObjectHeader([&](JDRemoteTyped& h)
            {
                ExtRemoteTyped header = h.GetExtRemoteTyped();
                byte attribute;

                if (header.HasField("attributesAndChecksum"))
                {
                    attribute = (UCHAR)(header.Field("attributesAndChecksum").GetUshort() ^ _recycler.GetCookie() >> 8);
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

                if (attribute & RemoteObjectInfoBits::ImplicitRootBit)
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
                if ((attributes[objectCount - i - 1] & RemoteObjectInfoBits::ImplicitRootBit)
                    && (attributes[objectCount - i - 1] & RemoteObjectInfoBits::PendingDisposeBit) == 0)
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

bool IsUsingDebugPinRecord(bool verbose)
{
    ULONG typeId = 0;
    HRESULT hr = GetExtension()->m_Symbols->GetTypeId(0, GetExtension()->FillModuleAndMemoryNS("%s!%sRecycler::PinRecord"), &typeId);

    // On retail builds, the pinRecord type doesn't even exist, it gets optimized away to int
    // On debug builds, the type might exist but it may or may not have extra information in it
    if (SUCCEEDED(hr) && typeId != 0)
    {
        ULONG64 sizeofPinRecord = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sRecycler::PinRecord))"));
        bool is = (sizeofPinRecord != sizeof(uint));
        if (verbose)
        {
            GetExtension()->Out("Using pin record with stack traces: %s\n", (is ? "true" : "false"));
        }

        return is;
    }

    return false;
}

template <typename Fn>
void MapPinnedObjects(RemoteRecycler recycler, const Fn& callback, bool verbose)
{
    bool isUsingDebugPinnedRecord = IsUsingDebugPinRecord(verbose);
    // GetExtension()->Out(_u("Possible symbol for %p: "), heapObject.vtable); GetExtension()->m_Symbols3->OutputSymbolByOffset(DEBUG_OUTCTL_AMBIENT, DEBUG_OUTSYM_ALLOW_DISPLACEMENT, heapObject.vtable); GetExtension()->Out("\n");
    if (GetExtension()->m_PtrSize == 8)
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

bool EXT_CLASS_BASE::DumpPossibleSymbol(RecyclerObjectGraph::GraphImplNodeType* node, bool makeLink, bool showScriptContext)
{
    if (!node->HasVtable() && node->GetTypeNameOrField() && !node->IsPropagated())
    {
        this->Out(" %s%s", node->IsPropagated() ? "[RefBy] " : "", node->GetTypeNameOrField());
        return true;
    }
    return DumpPossibleSymbol(node->Key(), makeLink, showScriptContext);    
}


bool EXT_CLASS_BASE::DumpPossibleSymbol(ULONG64 address, bool makeLink, bool showScriptContext)
{
    char const * typeName;
    JDRemoteTyped object = JDRemoteTyped::FromPtrWithVtable(address, &typeName);
    
    if (typeName == nullptr)
    {
        return false;
    }
    
    this->Out(" = ");
    if (makeLink)
    {
        std::string encodedTypeName = JDUtil::EncodeDml(typeName);
        if (PreferDML())
        {
            this->Dml("<link cmd=\"?? (%s *)0x%p\">(??)</link> ", encodedTypeName.c_str(), address);
        }
        else
        {
            this->Out("(\"?? (%s *)0x%p\" to display) ", encodedTypeName.c_str(), address);
        }

        if (object.HasField("type") && object.Field("type").HasField("typeId"))
        {
            if (PreferDML())
            {
                this->Dml("<link cmd=\"!jd.var 0x%p\">(!jd.var)</link> ", address);
            }
            else
            {
                this->Out("(\"!jd.var 0x%p\" to display) ", address);
            }
            if (showScriptContext)
            {
                this->Out("SC:0x%p ", object.Field("type").Field("javascriptLibrary").Field("scriptContext").GetPtr());
            }
        }
        else
        {
            this->Out(showScriptContext? (this->m_PtrSize == 8 ? "%32s" : "%24s") : "%10s", "");
        }
    }
    this->Out("%s", typeName);

    RemoteRecyclableObject(object).DumpPossibleExternalSymbol(typeName, makeLink, showScriptContext);    

    return true;
}

void DumpPinnedObject(int i, int j, ULONG64 entryPointer, const PinnedObjectEntry& entry, bool showEntries, bool showScriptContext)
{
    if (showEntries)
    {
        GetExtension()->Out("Index: (0x%x, %d), Entry: 0x%p, Address: ", i, j, entryPointer);
    }
    GetExtension()->Out("0x%p, ", entry.address);
    GetExtension()->Out("Ref:%d", entry.pinnedCount);

    if (entry.stackBackTrace != 0)
    {
        if (GetExtension()->PreferDML())
        {
            GetExtension()->Dml(" <link cmd=\"?? (StackBackTraceNode *)0x%p\">(stack)</link>", entry.stackBackTrace);
        }
        else
        {
            GetExtension()->Out(" StackBackTraceNode: 0x%p", entry.stackBackTrace);
        }
    }
    // There appears to be a bug in GetExtension()->Out where it doesn't deal with %d properly when
    // mixed with other types

    GetExtension()->DumpPossibleSymbol(entry.address, true, showScriptContext);

    GetExtension()->Out("\n");
}

JD_PRIVATE_COMMAND(showpinned,
    "Show pinned object list",
    "{a;b,o;all;all thread context}"
    "{;e,o,d=0;recycler;Recycler address}"
    "{e;b,o;Show Entry}"
    "{sc;b,o;Show script context}"
)
{
    ULONG64 arg = GetUnnamedArgU64(0);
    const bool allThreadContext = HasArg("a");
    const bool showScriptContext = HasArg("sc");
    const bool showEntries = HasArg("e");

    auto dumpPinnedObjectFromRecycler = [showScriptContext, showEntries](RemoteRecycler recycler)
    {
        uint count = 0;
        MapPinnedObjects(recycler, [&count, showScriptContext, showEntries](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
        {
            DumpPinnedObject(i, j, entryPointer, entry, showEntries, showScriptContext);
            count++;
        }, true);

        GetExtension()->Out("Count is %d\n", count);
    };

    if (allThreadContext)
    {
        RemoteThreadContext::ForEach([dumpPinnedObjectFromRecycler](RemoteThreadContext threadContext)
        {
            GetExtension()->Out("===========================================================\n");
            GetExtension()->Out("ThreadContext %p\n", threadContext.GetPtr());
            dumpPinnedObjectFromRecycler(threadContext.GetRecycler());
            return false;
        });
    }
    else
    {
        RemoteRecycler recycler = GetRecycler(arg);
        dumpPinnedObjectFromRecycler(recycler);
    }

}

template <class Fn>
void SearchRange(ULONG64 startAddress, uint numBytes, ULONG64 targetValue, Fn fn)
{
    char* pageContents = (char*)malloc(numBytes);
    AutoFree guard((void*)pageContents);
    ExtRemoteData remotePage(startAddress, numBytes);
    size_t ptrSize = GetExtension()->m_PtrSize;

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
            GetExtension()->Out("Start address: 0x%p, Value: 0x%p, Current: %p, Offset: %d\n", startAddress, value, current, offset);
#endif
            fn(addr);
        }

        current += ptrSize;
    }
}

RemoteRecycler EXT_CLASS_BASE::GetRecycler(ULONG64 recyclerPtr, RemoteThreadContext * threadContext)
{
    if (recyclerPtr != 0)
    {
        RemoteRecycler recycler = recyclerPtr;
        if (threadContext)
        {
            *threadContext = recycler.GetThreadContext();
        }
        return recycler;
    }

    RemoteThreadContext remoteThreadContext = RemoteThreadContext::GetCurrentThreadContext();
    if (threadContext)
    {
        *threadContext = remoteThreadContext;
    }
    return remoteThreadContext.GetRecycler();
}

JD_PRIVATE_COMMAND(findref,
    "Find objects referencing given object",
    "{;e,r;address;Address whose referrers to find}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 referencedObject = GetUnnamedArgU64(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    Addresses * rootPointers = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    RemoteHeapBlockMap hbm = recycler.GetHeapBlockMap();
    struct FindRefData
    {
        ULONG64 address;
        ULONG64 offset;
        bool isRoot;
    };
    std::vector<FindRefData> results;
    hbm.ForEachHeapBlock([rootPointers, referencedObject, &results](RemoteHeapBlock heapBlock)
    {
        if (heapBlock.IsLargeHeapBlock())
        {
            ULONG64 sizeOfObjectHeader = GetExtension()->recyclerCachedData.GetSizeOfLargeObjectHeader();
            heapBlock.ForEachLargeObjectHeader([referencedObject, sizeOfObjectHeader, rootPointers, &results](JDRemoteTyped header)
            {
                ULONG64 objectSize = header.Field("objectSize").GetSizeT();

                ULONG64 startAddress = header.GetPtr() + sizeOfObjectHeader;

                SearchRange(startAddress, (uint)objectSize, referencedObject, [&](ULONG64 addr) {
                    FindRefData result;
                    result.address = startAddress;
                    result.offset = addr - startAddress;
                    result.isRoot = rootPointers->Contains(startAddress);
                    results.push_back(result);
                });
                return false;
            });
        }
        else
        {
            ULONG64 startAddress = heapBlock.GetAddress();
            ULONG objectSize = heapBlock.GetBucketObjectSize();

            SearchRange(startAddress, (uint)heapBlock.GetTotalObjectSize(), referencedObject, [&](ULONG64 addr)
            {
                ULONG64 offset = (addr - startAddress);
                ULONG objectIndex = (ULONG)(offset / objectSize);
                ULONG64 objectAddress = (objectSize * objectIndex) + startAddress;
                offset = (addr - objectAddress);

                FindRefData result;
                result.address = objectAddress;
                result.offset = offset;
                result.isRoot = rootPointers->Contains(objectAddress);
                results.push_back(result);
            });
        }
        return false;
    });

    Out("Referring objects:\n");

    std::for_each(results.begin(), results.end(), [&](decltype(results[0])& result) {
        if (PreferDML())
        {
            this->Dml("\t<link cmd=\"!findref 0x%p%s\">+</link> ",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg));
            this->Dml("<link cmd=\"!oi 0x%p%s\">0x%p</link>",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg), result.address);
        }
        else
        {
            this->Out("\t+ /*\"!findref 0x%p%s\" to expand*/ ",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg));
            this->Out("0x%p /*\"!oi 0x%p%s\" to display*/",
                result.address, recyclerArg == 0 ? "" : FillModuleV(" 0x%p", recyclerArg), result.address);
        }
        this->Out("+0x%02x", result.offset);
        this->Out(" %s\n", (result.isRoot ? "(root)" : ""));
    });

    if (results.size() == 0)
    {
        Out("\tNo referencing objects\n");
    }
}

class AutoSwitchThread
{
public:
    AutoSwitchThread(RemoteThreadContext * threadContext) :
        prevThreadId(0), newThreadId(0)
    {
        if (threadContext)
        {
            HRESULT hr = g_Ext->m_System->GetCurrentThreadId(&prevThreadId);
            if (FAILED(hr))
            {
                g_Ext->ThrowStatus(hr, "Unable to get current thread ID");
            }
            newThreadId = prevThreadId;
            if (!threadContext->TryGetDebuggerThreadId(&newThreadId))
            {
                RemoteThreadContext currentThreadContext;
                if (!RemoteThreadContext::TryGetCurrentThreadContext(currentThreadContext)
                    || currentThreadContext.GetPtr() != threadContext->GetPtr())
                {
                    g_Ext->ThrowStatus(hr, "Unable to get debugger thread ID.  Switch to the thread where the recycler is manually");
                }
            }
            if (prevThreadId != newThreadId)
            {
                hr = g_Ext->m_System->SetCurrentThreadId(newThreadId);
                if (FAILED(hr))
                {
                    g_Ext->ThrowStatus(hr, "Unable to switch to recycler's thread ID %u", newThreadId);
                }
            }
        }
    }
    ~AutoSwitchThread()
    {
        if (prevThreadId != newThreadId)
        {
            HRESULT hr = g_Ext->m_System->SetCurrentThreadId(prevThreadId);
            if (FAILED(hr))
            {
                g_Ext->ThrowStatus(hr, "Unable to switch to original thread ID %u", prevThreadId);
            }
        }
    }

private:
    ULONG prevThreadId;
    ULONG newThreadId;
};

JD_PRIVATE_COMMAND(showroots,
    "Show the recycler roots",
    "{;e,o,d=0;recycler;Recycler address}"
    "{e;b,o;Show pinned entry}"
    "{sc;b,o;Show script context}")
{
    ULONG64 recyclerArg = GetUnnamedArgU64(0);
    const bool showScriptContext = HasArg("sc");
    const bool showEntries = HasArg("e");

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RootPointerReader rootPointerManager(recycler);

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

    if (recycler.EnableScanImplicitRoots())
    {
        rootPointerManager.ScanImplicitRoots();
    }

    if (threadContext.GetPtr() != NULL && threadContext.GetExtRemoteTyped().HasField("externalWeakReferenceCacheList"))
    {
        ExtRemoteTyped externalWeakReferenceCache = threadContext.GetExtRemoteTyped().Field("externalWeakReferenceCacheList");
        ULONG64 externalWeakReferenceList = externalWeakReferenceCache.GetPointerTo().GetPtr();
        if (externalWeakReferenceList != NULL)
        {
            Out("External Weak Reference caches\n");
            DumpList<true>(externalWeakReferenceList, "ExternalWeakReferenceCache *");
        }
    }

    ULONG64 externalRootMarker = recycler.GetExternalRootMarker();
    if (externalRootMarker != NULL)
    {
        Out("External root marker installed (Address: 0x%p), some roots might be missed\n", externalRootMarker);
    }

    Out("\nPinned objects\n");
    uint count = 0;
    MapPinnedObjects(recycler, [&count, &rootPointerManager, showEntries, showScriptContext](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
    {
        DumpPinnedObject(i, j, entryPointer, entry, showEntries, showScriptContext);
        count++;
        rootPointerManager.TryAdd((ULONG64)entry.address, RootType::RootTypePinned);
    }, true);

    Out("Count is %d\n", count);

    Out("\nGuest arenas\n");
    ULONG64 guestArenaList = recycler.GetExtRemoteTyped().Field("guestArenaList").GetPointerTo().GetPtr();
    DumpList<false>(guestArenaList, FillModuleAndMemoryNS("%s!%sRecycler::GuestArenaAllocator"));

    Out("\nExternal guest arenas\n");

    ExtRemoteTyped egal = recycler.GetExtRemoteTyped().Field("externalGuestArenaList");
    ULONG64 externalGuestArenaList = egal.GetPointerTo().GetPtr();
    DumpList<false>(externalGuestArenaList, "ArenaData *");

    Out("\nStack\n");
    {
        AutoSwitchThread autoSwitchThread(&threadContext);
        rootPointerManager.ScanRegisters();
        rootPointerManager.ScanStack(recycler, GetStackTop(), true, showScriptContext);
    }

    PCSTR typeName = recycler.GetExtRemoteTyped().Field("heapBlockMap").GetTypeName();
    Out("Heap block map type is %s\n", typeName);
}

Addresses * ComputeRoots(RemoteRecycler recycler, RemoteThreadContext* threadContext)
{
    const bool dump = false;
    RootPointerReader rootPointerManager(recycler);

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

    if (recycler.EnableScanImplicitRoots())
    {
        rootPointerManager.ScanImplicitRoots(dump);
    }

    //
    // Find external weak referenced roots
    //

    if (threadContext && threadContext->GetPtr() != 0 && threadContext->GetExtRemoteTyped().HasField("externalWeakReferenceCacheList"))
    {
        ExtRemoteTyped externalWeakReferenceCache = threadContext->GetExtRemoteTyped().Field("externalWeakReferenceCacheList");
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

    MapPinnedObjects(recycler, [&rootPointerManager](int i, int j, ULONG64 entryPointer, PinnedObjectEntry entry)
    {
        rootPointerManager.TryAdd(entry.address, RootType::RootTypePinned);
    }, dump);

    //
    // Scan guest arena (if it's not pending delete)
    //

    ExtRemoteTyped recyclerExtRemoteTyped = recycler.GetExtRemoteTyped();
    ULONG64 recyclerAddress = recycler.GetPtr();
    ULONG64 guestArenaList = recyclerAddress + recyclerExtRemoteTyped.GetFieldOffset("guestArenaList");

    RemoteListIterator<false> guestArenaIterator("Recycler::GuestArenaAllocator", guestArenaList);
    while (guestArenaIterator.Next())
    {
        ULONG64 data = guestArenaIterator.GetDataPtr();
        ExtRemoteTyped guestArena(GetExtension()->FillModuleAndMemoryNS("(%s!%sRecycler::GuestArenaAllocator*)@$extin"), data);

        ExtRemoteTyped isPendingDelete = guestArena.Dereference().Field("pendingDelete");
        if (!isPendingDelete.GetBoolean())
        {
            rootPointerManager.ScanArena(data, dump);
        }
    }

    //
    // Scan external guest arena
    //

    ExtRemoteTyped egal = recyclerExtRemoteTyped.Field("externalGuestArenaList");
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

    {
        AutoSwitchThread autoSwitchThread(threadContext);
        rootPointerManager.ScanRegisters(dump);
        rootPointerManager.ScanStack(recycler, GetStackTop(), dump);


    }

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

void DumpPointerPropertiesSeparatorLine()
{
    GetExtension()->Out("----------------------------+-------------------------------------\n");
}

void DumpPointerPropertiesPredecessorsHeader()
{
    GetExtension()->Out("----------------------------+-Predecessors------------------------\n");
}

void DumpPointerPropertiesDescendantsHeader()
{
    GetExtension()->Out("----------------------------+-Descendants-------------------------\n");
}

void DumpPointerPropertiesHorizontalSpacer()
{
    GetExtension()->Out("                            | ");
}

void DumpPointerPropertiesSpacerLine()
{
    GetExtension()->Out("                            |\n");
}

void DumpPointerPropertiesHeader()
{
    GetExtension()->Out("\n");
    GetExtension()->Out("              P             | Pinned Root\n");
    GetExtension()->Out("               S            | Stack Root\n");
    GetExtension()->Out("                R           | Register Root\n");
    GetExtension()->Out("                 A          | Arena Root\n");
    GetExtension()->Out("                  I         | Implicit Root\n");
    GetExtension()->Out("                    ^       | Click to execute `!jd.predecessors` on this node\n");
    GetExtension()->Out("                      v     | Click to execute `!jd.successors` on this node\n");
    GetExtension()->Out("                        >   | Click to execute `!jd.traceroots` on this node\n");
    GetExtension()->Out("  Pred   Succ Flags       * | Original input pointer\n");
    DumpPointerPropertiesSeparatorLine();
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

void DumpPointerProperties(RecyclerObjectGraph &objectGraph, ULONG64 pointerArg, ULONG64 address, int currentLevel, bool showLib)
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
    GetExtension()->Out("%6d ", pred);
    GetExtension()->Out("%6d ", desc);

    const uint bufferLength = 6; // space for 5 flags plus NULL
    char buffer[bufferLength];
    FormatPointerFlags(buffer, bufferLength, node);
    GetExtension()->Out("%s ", buffer);

    if (GetExtension()->PreferDML())
    {
        GetExtension()->Dml("<link cmd=\"!jd.predecessors -limit 0 0x%p\">^</link> ", address);
        GetExtension()->Dml("<link cmd=\"!jd.successors -limit 0 0x%p\">v</link> ", address);
    }
    else
    {
        GetExtension()->Out("^ /*\"!jd.predecessors -limit 0 0x%p\"*/", address);
        GetExtension()->Out("v /*\"!jd.successors -limit 0 0x%p\"*/", address);
    }

    bool isInput = (pointerArg == address); // display * or > with link as appropriate
    if (GetExtension()->PreferDML())
    {
        if (isInput)
        {
            GetExtension()->Out("  "); // spacer for >
            GetExtension()->Dml("<link cmd=\"!jd.traceroots 0x%p\">*</link>", address);
        }
        else
        {
            GetExtension()->Dml("<link cmd=\"!jd.traceroots 0x%p\">&gt;</link>", address);
            GetExtension()->Out("  "); // spacer for *
        }
    }
    else
    {
        if (isInput)
        {
            GetExtension()->Out("  "); // spacer for >
            GetExtension()->Out("* /*\"!jd.traceroots 0x%p\"*/", address);
        }
        else
        {
            GetExtension()->Out("> /*\"!jd.traceroots 0x%p\"*/", address);
            GetExtension()->Out("  "); // spacer for *
        }
    }

    GetExtension()->Out(" | ");

    // print the address and symbol
    GetExtension()->Out("0x%p ", address);
    GetExtension()->Out("(level %c%-3d)", (currentLevel < 0 ? '-' : ' '), abs(currentLevel));
    if (showLib)
    {
        GetExtension()->Out(" ");
        RemoteJavascriptLibrary(node->GetAssociatedJavascriptLibrary()).PrintLink();
    }
    GetExtension()->DumpPossibleSymbol(node);
    GetExtension()->Out("\n");
}

template <bool predecessorsMode = false, bool links = true>
void DumpPredSucc(Node currentNode, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 limitArg, bool showOnlyRoots, bool showLib)
{
    // Display the target pointer and one level of its descendants
    DumpPointerProperties(objectGraph, pointerArg, pointerArg, 0, showLib);

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
    auto mapper = [&objectGraph, pointerArg, &count, limitArg, level, command, showOnlyRoots, &countFunction, currentNode, showLib](Node next)
    {
        RootType rootType = next->GetRootType();
        if (showOnlyRoots && !RootTypeUtils::IsAnyRootType(rootType))
        {
            return false;
        }

        ULONG64 address = next->Key();
        DumpPointerProperties(objectGraph, pointerArg, address, level, showLib);

        // Display all children if the value is 0, otherwise display up to count children.
        if (limitArg != 0 && (++count) >= limitArg)
        {
            DumpPointerPropertiesHorizontalSpacer();
            uint nodeCount = (currentNode->*countFunction)();
            GetExtension()->Out("Limit Reached. %d more not displayed.", nodeCount - count);
            if (GetExtension()->PreferDML())
            {
                GetExtension()->Dml(" <link cmd=\"!jd.%s /limit 0 0x%p\">(Display all %s.)</link>\n", command, pointerArg, command);
            }
            else
            {
                GetExtension()->Out(" (\"!jd.%s /limit 0 0x%p\" to display all %s.)\n", command, pointerArg, command);
            }
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
        GetExtension()->Out("\n");
        if (GetExtension()->PreferDML())
        {
            if (showOnlyRoots)
            {
                GetExtension()->Dml("<link cmd=\"!jd.%s /limit 0 0x%p\">(Display all %s.)</link>\n", command, pointerArg, command);
            }
            else
            {
                GetExtension()->Dml("<link cmd=\"!jd.%s /r /limit 0 0x%p\">(Display only roots.)</link>\n", command, pointerArg);
            }
        }
        else
        {
            if (showOnlyRoots)
            {
                GetExtension()->Out("(\"!jd.%s /limit 0 0x%p\" to display all %s.)\n", command, pointerArg, command);
            }
            else
            {
                GetExtension()->Out("(\"!jd.%s /r /limit 0 0x%p\" to display only roots.)\n", command, pointerArg);
            }
        }
    }
}

template <bool predecessorsMode>
void EXT_CLASS_BASE::PredSuccImpl()
{
    auto predSuccFn = &DumpSuccessors<true>;
    if (predecessorsMode)
    {
        predSuccFn = &DumpPredecessors<true>;
    }

    const ULONG64 pointerArg = GetUnnamedArgU64(0);
    const ULONG64 recyclerArg = GetUnnamedArgU64(1);
    const ULONG64 limitArg = GetArgU64("limit");
    const bool showOnlyRoots = HasArg("r");
    const bool showLib = HasArg("lib");

    if (pointerArg == NULL)
    {
        Out("Please specify a non-null object pointer.\n");
        return;
    }

    //
    // Perform necessary setup.
    //

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext));
    Node node = objectGraph.FindNode(pointerArg);

    //
    // Actually dump the pointers.
    //

    DumpPointerPropertiesHeader();
    predSuccFn(node, objectGraph, pointerArg, limitArg, showOnlyRoots, showLib);
}

template <bool links = true>
void DumpPredecessors(Node node, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 limitArg, bool showOnlyRoots, bool showLib)
{
    DumpPredSucc<true, links>(node, objectGraph, pointerArg, limitArg, showOnlyRoots, showLib);
}

template <bool links = true>
void DumpSuccessors(Node node, RecyclerObjectGraph &objectGraph, ULONG64 pointerArg,
    ULONG64 limitArg, bool showOnlyRoots, bool showLib)
{
    DumpPredSucc<false, links>(node, objectGraph, pointerArg, limitArg, showOnlyRoots, showLib);
}

JD_PRIVATE_COMMAND(predecessors,
    "Given a pointer in the graph, show all of its ancestors.",
    "{;ed,o,d=0;pointer;Address to trace}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{r;b,o;onlyRoots;Only show ancestors which are also roots}"
    "{limit;edn=(10),o,d=10;limit;Number of nodes to list}"
    "{lib;b,o;showlib;Show library}")
{
    PredSuccImpl<true>();
}

JD_PRIVATE_COMMAND(successors,
    "Given a pointer in the graph, show all of its descendants.",
    "{;ed,o,d=0;pointer;Address to trace}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{r;b,o;onlyRoots;Only show descendants which are also roots}"
    "{limit;edn=(10),o,d=10;limit;Number of nodes to list}"
    "{lib;b,o;showlib;Show library}")
{
    PredSuccImpl<false>();
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
    "{r;b,o;root;Show all roots that can reach the object}"
    "{pred;b,o;showPredecessors;Show up to limit predecessors in the output}"
    "{vt;b,o;vtable;Vtable Only}"
    "{lib;b,o;showlib;Show library}")
{
    const ULONG64 pointerArg = GetUnnamedArgU64(0);
    const ULONG64 recyclerArg = GetUnnamedArgU64(1);
    const bool infer = !HasArg("vt");

    TraceRoot traceRoot;
    traceRoot.ParseArg();

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

    RecyclerObjectGraph::TypeInfoFlags flags = RecyclerObjectGraph::TypeInfoFlags::None;
    if (infer)
    {
        flags = (RecyclerObjectGraph::TypeInfoFlags)(flags | RecyclerObjectGraph::TypeInfoFlags::Infer);
    }

    //
    // Perform necessary setup.
    //

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext, flags));

    traceRoot.Trace(objectGraph, recyclerArg, pointerArg);
}

void TraceRoot::ParseArg()
{
    numRootsArg = GetExtension()->GetArgU64("roots");
    limitArg = GetExtension()->GetArgU64("limit");
    transientRoots = GetExtension()->HasArg("t");
    allShortestPath = GetExtension()->HasArg("a");
    allRoots = GetExtension()->HasArg("r") || allShortestPath;
    showPath = allShortestPath || !allRoots;
    showSuccessors = true;
    showPredecessors = GetExtension()->HasArg("pred");
    showLib = GetExtension()->HasArg("lib");
    showHeader = true;
}

void TraceRoot::Trace(RecyclerObjectGraph& objectGraph, const ULONG64 recyclerArg, const ULONG64 pointerArg)
{
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
    // If transient roots is being ignore, still keep track of them in case there are no non-transient roots
    NodeQueue transientRootQueue;

    //
    // Pass 0: Seed the traversal.
    //

    Node node = objectGraph.FindNode(pointerArg);

    if (node == nullptr)
    {
        g_Ext->Err("ERROR: %p not a GC pointer\n", pointerArg);
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

        // If this is an allowed root, record it as a root pointer.
        // Make sure the pointer is not the one we started with because
        // it is not useful to consider the node we started at as a root of the traversal.
        if (RootTypeUtils::IsAnyRootType(rootType)
            && pointerArg != currentNode->Key())
        {
            bool allowedRoot = (transientRoots || RootTypeUtils::IsNonTransientRootType(rootType));
            if (allowedRoot)
            {
                currentRootHitCount++;
                rootQueue.push(current);
                if (!allRoots)
                {
                    // we found the root corresponding to the shortest path (by the properties of BFS), so stop traversing
                    break;
                }
            }
            else
            {
                transientRootQueue.push(current);
            }
        }

        // Ascend upwards towards the roots
        currentNode->MapAllPredecessors([this, currentData, &traversalMap, &nodeQueue, currentRootHitCount](Node parent)
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

    g_Ext->Out("\n");

    if (showHeader)
    {
        if (transientRoots)
        {
            if (GetExtension()->PreferDML())
            {
                g_Ext->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Ignore transient recycler roots for traversal root limit.)</link>\n",
                    numRootsArg, pointerArg, recyclerArg);
            }
            else
            {
                g_Ext->Out("(\"!jd.traceroots /roots %d 0x%p 0x%p\" to ignore transient recycler roots for traversal root limit.)\n",
                    numRootsArg, pointerArg, recyclerArg);
            }
        }
        else
        {
            if (GetExtension()->PreferDML())
            {
                g_Ext->Dml("<link cmd=\"!jd.traceroots /t /roots %d 0x%p 0x%p\">(Use transient recycler roots for traversal root limit.)</link>\n",
                    numRootsArg, pointerArg, recyclerArg);
            }
            else
            {
                g_Ext->Out("(\"!jd.traceroots /t /roots %d 0x%p 0x%p\" to use transient recycler roots for traversal root limit.)\n",
                    numRootsArg, pointerArg, recyclerArg);
            }
        }

        if (numRootsArg == 0)
        {
            g_Ext->Out("Traversing as far as possible.\n");
            if (GetExtension()->PreferDML())
            {
                g_Ext->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Traverse through just one recycler root.)</link>\n",
                    1, pointerArg, recyclerArg);
            }
            else
            {
                g_Ext->Out("(\"!jd.traceroots /roots %d 0x%p 0x%p\" to traverse through just one recycler root.)\n",
                    1, pointerArg, recyclerArg);
            }
        }
        else
        {
            if (GetExtension()->PreferDML())
            {
                g_Ext->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Traverse as far as possible.)</link>\n",
                    0, pointerArg, recyclerArg);
                g_Ext->Dml("<link cmd=\"!jd.traceroots /roots %d 0x%p 0x%p\">(Traverse through %d recycler roots.)</link>\n",
                    numRootsArg + 1, pointerArg, recyclerArg, numRootsArg + 1);
            }
            else
            {
                g_Ext->Out("(\"!jd.traceroots /roots %d 0x%p 0x%p\" to traverse as far as possible.)\n",
                    0, pointerArg, recyclerArg);
                g_Ext->Out("(\"!jd.traceroots /roots %d 0x%p 0x%p\" to traverse through %d recycler roots.)\n",
                    numRootsArg + 1, pointerArg, recyclerArg, numRootsArg + 1);
            }
        }

        DumpPointerPropertiesHeader();
    }

    // Print itself if no other roots found and itself is a root
    if (rootQueue.empty() && transientRootQueue.empty())
    {
        RootType rootType = node->GetRootType();
        if (transientRootQueue.empty() && RootTypeUtils::IsAnyRootType(rootType))
        {
            if (RootTypeUtils::IsNonTransientRootType(rootType))
            {
                rootQueue.push(std::make_pair(node, data));
            }
            else
            {
                transientRootQueue.push(std::make_pair(node, data));
            }
        }
    }

    if (rootQueue.empty() && !transientRootQueue.empty())
    {
        g_Ext->Out("                            | NOTE: No non-transient root found. Showing transient roots\n");

        if (allRoots)
        {
            rootQueue = transientRootQueue;
        }
        else
        {
            rootQueue.push(transientRootQueue.front());
        }
    }

    uint rootCount = 0;
    while (!rootQueue.empty())
    {
        auto root = rootQueue.front();
        rootQueue.pop();

        NodeQueueNode rootNode = root.first;
        ULONG64 address = rootNode->Key();
        TraversalData *pTraversalData = traversalMap.Get(address);
        while (pTraversalData != nullptr)
        {
            DumpPointerProperties(objectGraph, pointerArg, pTraversalData->address, pTraversalData->level, showLib);
            if (!showPath)
            {
                rootCount++;
                break;
            }
            pTraversalData = pTraversalData->child;
        }

        if (showPath)
        {
            DumpPointerPropertiesSeparatorLine();
        }
    }

    if (rootCount != 0)
    {
        g_Ext->Out("Roots found: %u\n", rootCount);
    }

    // Only display the descendants section if node actually has descendants.
    if (showSuccessors && node->GetSuccessorCount() > 0)
    {
        DumpPointerPropertiesSpacerLine();
        DumpPointerPropertiesDescendantsHeader();
        DumpSuccessors<false>(node, objectGraph, pointerArg, limitArg, false, showLib);
    }

    // Display an option to show predecessors if there are any.
    if (showPredecessors && node->GetPredecessorCount() > 0)
    {
        DumpPointerPropertiesSpacerLine();
        DumpPointerPropertiesPredecessorsHeader();
        DumpPredecessors<false>(node, objectGraph, pointerArg, limitArg, false, showLib);
    }

    // Delete all of the TraversalData pointers in traversalMap so we don't leak.
    traversalMap.MapAll([](TraversalMapKey key, TraversalMapValue value)
    {
        delete value; // `value` is the TraversalData pointer
    });
}

void OnArenaRootScanned(ULONG64 root, RootType rootType, void* context)
{
    struct Context
    {
        ULONG64 start;
        ULONG64 current;
    };

    Context* ctx = (Context*)context;
    GetExtension()->Out("  0x%p: 0x%p => 0x%p", ctx->start, ctx->current, root);
    GetExtension()->DumpPossibleSymbol(root, true);
    GetExtension()->Out("\n");
}

JD_PRIVATE_COMMAND(arenaroots, "Dump all roots from arenas.", 
    "{;ed,o,d=0;recycler;Recycler address}")
{
    const ULONG64 recyclerArg = GetUnnamedArgU64(0);

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RootPointerReader rootPointerManager(recycler, OnArenaRootScanned);
    ExtRemoteTyped recyclerExtRemoteTyped = recycler.GetExtRemoteTyped();
    ULONG64 recyclerAddress = recyclerExtRemoteTyped.m_Data; // TODO: recycler needs to be a pointer to make this work
    ULONG64 guestArenaList = recyclerAddress + recyclerExtRemoteTyped.GetFieldOffset("guestArenaList");

    RemoteListIterator<false> guestArenaIterator("Recycler::GuestArenaAllocator", guestArenaList);
    while (guestArenaIterator.Next())
    {
        ULONG64 data = guestArenaIterator.GetDataPtr();
        ExtRemoteTyped guestArena(this->FillModuleAndMemoryNS("(%s!%sRecycler::GuestArenaAllocator*)@$extin"), data);

        ExtRemoteTyped isPendingDelete = guestArena.Dereference().Field("pendingDelete");
        if (!isPendingDelete.GetBoolean())
        {
            rootPointerManager.ScanArena(data, true);
        }
    }

    //
    // Scan external guest arena
    //

    ExtRemoteTyped egal = recyclerExtRemoteTyped.Field("externalGuestArenaList");
    ULONG64 externalGuestArenaList = egal.GetPointerTo().GetPtr();
    RemoteListIterator<false> externalGuestArenaIterator("ArenaData *", externalGuestArenaList);
    while (externalGuestArenaIterator.Next())
    {
        ULONG64 dataPtr = externalGuestArenaIterator.GetDataPtr();
        Out("Scanning 0x%p\n", dataPtr);
        rootPointerManager.ScanArenaData(GetPointerAtAddress(dataPtr));
    }
}

JD_PRIVATE_COMMAND(savegraph,
    "Saves the current recycler object graph into a file (js, python, csv, etc.)",
    "{;s;filename;Filename to output to}"
    "{;ed,o,d=0;recycler;Recycler address}"
    "{;s,o,d=js;filetype;Save file type <js|python|csv>}"
    "{vt;b,o;vtable;Vtable Only}")
{
    PCSTR filename = GetUnnamedArgStr(0);
    ULONG64 recyclerArg = GetUnnamedArgU64(1);
    PCSTR strFiletype = GetUnnamedArgStr(2);
    bool infer = !HasArg("vt");

    enum OutputFileType
    {
        CSV,
        JavaScript,
        Python
    };

    OutputFileType filetype = CSV;
    if (_stricmp(strFiletype, "python") == 0)
    {
        filetype = Python;
    }
    else if (_stricmp(strFiletype, "js") == 0)
    {
        filetype = JavaScript;
    }

    if (filetype != CSV)
    {
        // type information is needed only for CSV files
        infer = false;
    }

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext,
        infer? RecyclerObjectGraph::TypeInfoFlags::Infer : RecyclerObjectGraph::TypeInfoFlags::None));
    
    if (filetype == CSV)
    {
        Out("Saving object graph to '%s.nodes.csv' and '%s.edges.csv\n'", filename, filename);
        objectGraph.DumpForCsv(filename);
    }
    else
    {
        Out("Saving object graph to '%s'\n", filename);
        if (filetype == JavaScript)
        {
            objectGraph.DumpForJs(filename);
        }
        else if (filetype == Python)
        {
            objectGraph.DumpForPython(filename);
        }
        else
        {
            Err("Unknown file type '%s'\n", filetype);
        }
    }
}

struct ObjectAllocStats
{
    ULONG64 count;
    ULONG64 size;
    ULONG64 unknownCount;
    ULONG64 unknownSize;
    bool hasVtable;
};

typedef stdext::hash_map<char const *, ObjectAllocStats> ObjectCountsMap;
struct ObjectCountData
{
    ObjectCountData(bool infer, ULONG64 javascriptLibrary = (ULONG64)-1)
        : numNodes(0), totalSize(0), infer(infer), javascriptLibrary(javascriptLibrary)
    {}

    void addNode(RecyclerObjectGraph::GraphImplNodeType * node)
    {
        numNodes++;
        totalSize += (node->GetObjectSize());

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

    ObjectCountsMap objectCounts;
    ULONG64 numNodes;
    ULONG64 totalSize;
    ULONG64 javascriptLibrary;
    bool infer;
};

int __cdecl ObjectAllocCountComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return (int)(ptrB->second.count - ptrA->second.count);
}

int __cdecl ObjectAllocSizeComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return (int)(ptrB->second.size - ptrA->second.size);
}

int __cdecl ObjectAllocUnknownCountComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return (int)(ptrB->second.unknownCount - ptrA->second.unknownCount);
}

int __cdecl ObjectAllocUnknownSizeComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return (int)(ptrB->second.unknownSize - ptrA->second.unknownSize);
}

int __cdecl ObjectAllocNameComparer(const void * a, const void * b)
{
    auto ptrA = (std::pair<char const *, ObjectAllocStats> *)a;
    auto ptrB = (std::pair<char const *, ObjectAllocStats> *)b;
    return strcmp(ptrA->first, ptrB->first);
}

int __cdecl LibraryAllocCountComparer(const void * a, const void *b)
{
    auto ptrA = (std::pair<ULONG64, ObjectCountData *> *)a;
    auto ptrB = (std::pair<ULONG64, ObjectCountData *> *)b;
    return (int)(ptrB->second->numNodes - ptrA->second->numNodes);
}

int __cdecl LibraryAllocSizeComparer(const void * a, const void *b)
{
    auto ptrA = (std::pair<ULONG64, ObjectCountData *> *)a;
    auto ptrB = (std::pair<ULONG64, ObjectCountData *> *)b;
    return (int)(ptrB->second->totalSize - ptrA->second->totalSize);
}

void PrintNodeLink(bool filterUnknown, ULONG64 library, char const * type)
{
    if (GetExtension()->PreferDML())
    {
        if (library != (ULONG64)-1)
        {
            g_Ext->Dml("<link cmd=\"!jd.jsobjectnodes -fl %p %s%s%s\">(nodes)</link>", library, filterUnknown ? "-fu " : "", type? "-ft ": "", type? type : "");
        }
        else
        {
            g_Ext->Dml("<link cmd=\"!jd.jsobjectnodes %s%s%s\">(nodes)</link>", filterUnknown ? "-fu " : "", type ? "-ft " : "", type ? type : "");
        }
    }
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
    "{g;b,o;group;Group unknown objects}"
    "{lib;b,o;library;Infer and display per library}"
    "{fd;edn=(10),o,d=-1;filterDepth;Filter to depth}"
    "{fl;ed,o;filterLib;Filter to library}"
    "{fs;edn=(10),o;filterSize;Filter to object size}"
    "{fsmin;edn=(10),o;filterSizeMin;Filter to minimum object size}"
    "{fsmax;edn=(10),o;filterSizeMax;Filter to maximum object size}"
    "{summary;b,o;summary;Summary only}"
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
    const bool groupUnknown = HasArg("g");
    const bool hasFilterLib = HasArg("fl");
    const bool hasFilterSize = HasArg("fs");
    const bool hasFilterSizeMin = HasArg("fsmin");
    const bool hasFilterSizeMax = HasArg("fsmax");
    const bool perLibrary = hasFilterLib || HasArg("lib");
    const bool summary = HasArg("summary");

    if (hasFilterSize && (hasFilterSizeMin || hasFilterSizeMax))
    {
        throw ExtException(E_FAIL, "Can't specify both -fs and -fsmin/-fsmax");
    }
    const ULONG64 libraryFilter = hasFilterLib ? GetArgU64("fl") : (ULONG64)-1;
    const ULONG64 sizeFilter = hasFilterSize ? GetArgU64("fs") : (ULONG64)-1;
    const ULONG64 sizeFilterMin = hasFilterSizeMin ? GetArgU64("fsmin") : (ULONG64)0;
    const ULONG64 sizeFilterMax = hasFilterSizeMax ? GetArgU64("fsmax") : (ULONG64)-1;
    const ULONG64 depthFilter = GetArgU64("fd");

    if (sortByCount && sortByName)
    {
        throw ExtException(E_FAIL, "Can't specify both -sc and -sn");
    }

    if (sortByUnknown && sortByName)
    {
        throw ExtException(E_FAIL, "Can't specify both -su and -sn");
    }

    // Note: (sortByCount && sortByUnknown) is allowed -- see below

    auto objectSortComparer = sortByName ? ObjectAllocNameComparer :
        sortByUnknown ?
        (sortByCount ? ObjectAllocUnknownCountComparer : ObjectAllocUnknownSizeComparer) :
        (sortByCount ? ObjectAllocCountComparer : ObjectAllocSizeComparer);

    auto librarySortComparer = sortByCount ? LibraryAllocCountComparer : LibraryAllocSizeComparer;

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);    

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
    ObjectCountData allObjectCounts(infer);
    class PerLibraryObjectCountData : public stdext::hash_map<ULONG64, ObjectCountData *>
    {
    public:
        ~PerLibraryObjectCountData()
        {
            for (auto i = this->begin(); i != this->end(); i++)
            {
                delete i->second;
            }
        }
    } perLibraryObjectCountData;

    ULONG64 actualTotalSize = 0;
    ULONG64 actualTotalCount = 0;

    objectGraph.MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        actualTotalCount++;
        actualTotalSize += node->GetObjectSize();

        if (hasFilterSize)
        {
            if (node->GetObjectSize() != sizeFilter)
            {
                return;
            }
        }

        if (node->GetObjectSize() < sizeFilterMin || node->GetObjectSize() > sizeFilterMax)
        {
            return;
        }

        if (depthFilter != (ULONG64)-1 && node->GetDepth() == depthFilter)
        {
            return;
        }

        ObjectCountData * objectCountData;
        if (perLibrary)
        {
            ULONG64 javascriptLibrary = node->GetAssociatedJavascriptLibrary();
            auto i = perLibraryObjectCountData.find(javascriptLibrary);
            if (i != perLibraryObjectCountData.end())
            {
                objectCountData = i->second;
            }
            else
            {
                objectCountData = new ObjectCountData(infer, javascriptLibrary);
                perLibraryObjectCountData[javascriptLibrary] = objectCountData;
            }
        }
        else
        {
            objectCountData = &allObjectCounts;
        }

        objectCountData->addNode(node);
    });

    Out("\r");

    auto displayObjectCounts = [&](ObjectCountData * data)
    {
        ObjectCountsMap& objectCounts = data->objectCounts;
        if (showUnknown)
        {
            Out(" Count?      Bytes? %%Count %%Bytes        | ");
        }

        if (this->PreferDML())
        {
            Out("  Count       Bytes %%Count %%Bytes         Symbol                \n");
        }
        else
        {
            Out("  Count       Bytes %%Count %%Bytes Symbol                \n");
        }

        ULONG64 knownObjectCount = 0;
        ULONG64 knownObjectSize = 0;
        uint vtableCount = 0;
        std::auto_ptr<std::pair<char const *, ObjectAllocStats>> sortedArray(new std::pair<char const *, ObjectAllocStats>[objectCounts.size()]);
        int c = 0;
        for (auto i = objectCounts.begin(); i != objectCounts.end(); i++)
        {
            ObjectAllocStats& stats = (*i).second;
            knownObjectCount += stats.count - stats.unknownCount;
            knownObjectSize += stats.size - stats.unknownSize;
            vtableCount += stats.hasVtable;

            if (!groupUnknown)
            {
                stats.count -= stats.unknownCount;
                stats.size -= stats.unknownSize;
            }

            sortedArray.get()[c++] = (*i);
        }

        qsort(sortedArray.get(), c, sizeof(std::pair<char const *, ObjectAllocStats>), objectSortComparer);

        Out("----------------------------------------------------------------------------\n");

        ULONG64 numNodes = data->numNodes;
        ULONG64 totalSize = data->totalSize;
        if (!summary)
        {
            for (int i = 0; i < c; i++)
            {
                char const * typeName = sortedArray.get()[i].first;
                ObjectAllocStats& stats = sortedArray.get()[i].second;
                ULONG64 currCount = stats.count;
                ULONG64 currSize = stats.size;

                std::string encodedTypeName = JDUtil::EncodeDml(typeName);

                if (showUnknown)
                {
                    Out("%7I64u %11I64u %5.1f%% %5.1f%% ", stats.unknownCount, stats.unknownSize,
                        (double)stats.unknownCount / (double)numNodes * 100, (double)stats.unknownSize / (double)totalSize * 100);

                    PrintNodeLink(true, data->javascriptLibrary, encodedTypeName.c_str());

                    Out(" | ");
                }
                else if (currCount == 0 && currSize == 0)
                {
                    // Some entries only have unknown count, if we don't show unknown, and it is 0, don't show the entry.
                    continue;
                }

                Out("%7I64u %11I64u %5.1f%% %5.1f%% ", currCount, currSize, (double)currCount / (double)numNodes * 100,
                    (double)currSize / (double)totalSize * 100);
                PrintNodeLink(false, data->javascriptLibrary, encodedTypeName.c_str());               
                Out(" %s%s\n", stats.hasVtable ? (groupUnknown ? "[Group] " : "") : "[Field] ", typeName);

                if (i > limit)
                {
                    Out("<%d limit reached>\n", limit);
                    break;
                }
            }

            Out("----------------------------------------------------------------------------\n");
        }
        const ULONG64 unknownTotalCount = numNodes - knownObjectCount;
        const ULONG64 unknownTotalSize = totalSize - knownObjectSize;
        Out("%7I64u %11I64u %5.1f%% %5.1f%%", unknownTotalCount, unknownTotalSize,
            (double)unknownTotalCount / (double)numNodes * 100, (double)unknownTotalSize / (double)totalSize * 100);
        Out(showUnknown ? "         | " : " Unknown object summary\n");

        Out("%7I64u %11I64u %5.1f%% %5.1f%% Known object summary\n", knownObjectCount, knownObjectSize,
            (double)knownObjectCount / (double)numNodes * 100, (double)knownObjectSize / (double)totalSize * 100);

        if (showUnknown)
        {
            Out("                                          | ");
        }

        Out("%7I64u %11I64u %5.1f%% %5.1f%% Total object summary\n", numNodes, totalSize, (double)numNodes / (double)actualTotalCount * 100, (double)totalSize / (double)actualTotalSize * 100);
        Out("Found %d (%d vtable, %d field)\n", objectCounts.size(), vtableCount, objectCounts.size() - vtableCount);
    };

    if (perLibrary)
    {
        std::auto_ptr<std::pair<ULONG64, ObjectCountData *>> sortedLibraryArray(new std::pair<ULONG64, ObjectCountData *>[perLibraryObjectCountData.size()]);
        uint numLibrary = 0;
        for (auto i = perLibraryObjectCountData.begin(); i != perLibraryObjectCountData.end(); i++)
        {
            sortedLibraryArray.get()[numLibrary++] = *i;
        }

        qsort(sortedLibraryArray.get(), numLibrary, sizeof(std::pair<ULONG64, ObjectCountData *>), librarySortComparer);

        if (summary)
        {
            char const * headerFormat = (this->m_PtrSize == 4)? "%-10s %-10s %7s %11s %-6s %-6s         %s\n" : "%-18s %-18s %7s %11s %-6s %-6s         %s\n";
            Out(headerFormat, "Library", "ScrContext", "Count", "Bytes", "%Count", "%Bytes", "URL");
            Out("============================================================================================\n");
        }

        for (uint i = 0; i < numLibrary; i++)
        {
            ULONG64 javascriptLibrary = sortedLibraryArray.get()[i].first;
            if (libraryFilter != (ULONG64)-1 && javascriptLibrary != libraryFilter)
            {
                continue;
            }

            ULONG64 scriptContextPtr = 0;
            ULONG64 url = 0;
            
            if (javascriptLibrary != 0)
            {
                JDRemoteTyped scriptContext = JDRemoteTyped::FromPtrWithVtable(javascriptLibrary).Field("scriptContext");
                scriptContextPtr = scriptContext.GetPtr();
                if (scriptContext.HasField("url"))
                {
                    url = scriptContext.Field("url").GetPtr();
                }
            }

            ObjectCountData * data = sortedLibraryArray.get()[i].second;
            if (summary)
            {
                if (PreferDML())
                {
                    // TODO: Only sort byte is propagated. Do the other flags
                    Dml("<link cmd=\"!jd.jsobjectstats -fl 0x%p %s%s%s\">0x%p</link>", javascriptLibrary, 
                        sortByCount? "-sc " : "", sortByName? "-sn " : "", sortByUnknown? "-su " : "", javascriptLibrary);
                }
                else
                {
                    Out("0x%p", javascriptLibrary);
                }
                Out(" 0x%p %7I64u %11I64u %5.1f%% %5.1f%% ", scriptContextPtr, data->numNodes, data->totalSize, 
                    (double)data->numNodes / (double)actualTotalCount * 100, (double)data->totalSize / (double)actualTotalSize * 100);

                PrintNodeLink(false, javascriptLibrary, nullptr);
                if (url != 0)
                {
                    Out(" %mu", url);
                }
                Out("\n");
            }
            else
            {
                Out("============================================================================================\n");
                if (javascriptLibrary != 0)
                {
                    Out("Associated Library %p, ScriptContext %p", javascriptLibrary, scriptContextPtr);
                    if (url != 0)
                    {
                        Out(", URL %mu", url);
                    }
                    Out("\n");
                }
                else
                {
                    Out("No Associated Library\n");
                }

                displayObjectCounts(data);
            }    
        }

        if (summary)
        {            
            Out("--------------------------------------------------------------------------------------------\n");
            char const * formatStr = this->m_PtrSize == 4 ? "%-10s %-10s %7I64u %11I64u                       Total\n" : "%-18s %-18s %7I64u %11I64u                       Total\n";
            Out(formatStr, "", "", actualTotalCount, actualTotalSize);
        }
    }
    else
    {
        displayObjectCounts(&allObjectCounts);
    }
}

struct SortNodeByKey
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->Key() < right->Key();
    }
};

template <typename SecondarySort>
struct SortNodeByDepthT
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetDepth() > right->GetDepth() ||
            (left->GetDepth() == right->GetDepth() && SecondarySort()(left, right));
    }
};

template <typename SecondarySort>
struct SortNodeBySuccessorT
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetSuccessorCount() > right->GetSuccessorCount() ||
            (left->GetSuccessorCount() == right->GetSuccessorCount() && SecondarySort()(left, right));
    }
};

template <typename SecondarySort>
struct SortNodeByPredecessorT
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetPredecessorCount() > right->GetPredecessorCount() ||
            (left->GetPredecessorCount() == right->GetPredecessorCount() && SecondarySort()(left, right));
    }
};

template <typename SecondarySort>
struct SortNodeByObjectSizeT
{
    bool operator()(RecyclerObjectGraph::GraphImplNodeType* left, RecyclerObjectGraph::GraphImplNodeType* right) const
    {
        return left->GetObjectSize() > right->GetObjectSize() ||
            (left->GetObjectSize() == right->GetObjectSize() && SecondarySort()(left, right));
    }
};

typedef SortNodeByObjectSizeT<SortNodeBySuccessorT<SortNodeByKey>> SortNodeByObjectSizeAndSuccessor;
typedef SortNodeBySuccessorT<SortNodeByObjectSizeT<SortNodeByKey>> SortNodeBySuccessorAndObjectSize;
typedef SortNodeByPredecessorT<SortNodeByObjectSizeT<SortNodeByKey>> SortNodeByPredecessorAndObjectSize;
typedef SortNodeByDepthT<SortNodeByObjectSizeT<SortNodeByKey>> SortNodeByDepthAndObjectSize;

JD_PRIVATE_COMMAND(jsobjectnodes,
    "Dump a table of object nodes sorted by number of successors, number or predecessors, or size (default).",
    "{;e,o,d=0;recycler;Recycler address}"
    "{ti;b,o;typeInfo;Type info}"
    "{sd;b,o;depth;Sort by depth}"
    "{sp;b,o;predecssorCount;Sort by predecessor count}"
    "{ss;b,o;successorCount;Sort by successor count}"
    "{limit;edn=(10),o,d=10;nodes;Number of nodes to display}"
    "{skip;edn=(10),o,d=0;nodes;Number of nodes to skip}"
    "{lib;b,o;showlib;Show library}"
    "{fl;ed,o;filterLib;Filter to library}"
    "{fd;edn=(10),o,d=-1;filterDepth;Filter to depth}"
    "{fu;b,o;filterUnknownType;Filter to unknown}"
    "{fr;b,o;filterRoot;Filter to root}"
    "{ft;x,o;filterType;Filter to type (last arg)}")
{
    const ULONG64 recyclerArg = GetUnnamedArgU64(0);
    const ULONG64 limit = GetArgU64("limit");
    const ULONG64 skip = GetArgU64("skip");

    const bool sortByPred = HasArg("sp");
    const bool sortBySucc = HasArg("ss");
    const bool sortByDepth = HasArg("sd");
    const bool showLib = HasArg("lib");
    const bool hasFilterLib = HasArg("fl");
    const bool hasFilterType = HasArg("ft");
    const bool filterUnknownType = HasArg("fu");
    const bool typeInfo = showLib || hasFilterLib || hasFilterType || HasArg("ti");

    char const * typeFilter = hasFilterType ? GetArgStr("ft") : nullptr;
    const ULONG64 libraryFilter = hasFilterLib ? GetArgU64("fl") : (ULONG64)-1;
    const ULONG64 depthFilter = GetArgU64("fd");

    if (sortByPred + sortBySucc + sortByDepth > 1)
    {
        this->Err("ERROR: -sp/-ss/-sd can't be specified together\n");
        return;
    }

    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext,
        typeInfo ? RecyclerObjectGraph::TypeInfoFlags::Infer : RecyclerObjectGraph::TypeInfoFlags::None));

    this->Out("\n");
    this->Out("%22s ^     Run !jd.predecessors on this node\n", "");
    this->Out("%22s   v   Run !jd.successors on this node\n", "");
    this->Out("%22s     > Run !jd.traceroots on this node\n", "");
    if (showLib)
    {
        this->Out("%6s %6s %8s %4s %5s %-18s %-18s %s\n", "Pred", "Succ", "Size", "Dep","", "Address", "Library", "Type");
        this->Out("------ ------ -------- ---- ----- ------------------ ------------------ --------------\n");
    }
    else
    {
        this->Out("%6s %6s %8s %4s %5s %-18s %s\n", "Pred", "Succ", "Size", "Dep", "", "Address", "Type");
        this->Out("------ ------ -------- ---- ----- ------------------ --------------\n");
    }

    if (skip != 0)
    {
        this->Out("Skipping %I64d\n", skip);
    }

    auto include = [&](RecyclerObjectGraph::GraphImplNodeType* node)
    {
        if (filterUnknownType)
        {
            if (!node->IsPropagated())
            {
                return false;
            }
        }
        if (libraryFilter != (ULONG64)-1 && node->GetAssociatedJavascriptLibrary() != libraryFilter)
        {
            return false;
        }

        if (hasFilterType)
        {
            if (node->IsPropagated() && !filterUnknownType)
            {
                return false;
            }
            char const * typeNameOfField = node->GetTypeNameOrField();
            if (strcmp(typeNameOfField, typeFilter) != 0)
            {
                return false;
            }
        }

        if (depthFilter != (ULONG64)-1 && node->GetDepth() != depthFilter)
        {
            return false;
        }
        return true;
    };
    
    ULONG64 count = 0;
    auto output = [&](RecyclerObjectGraph::GraphImplNodeType* node)
    {       
        if (count >= skip)
        {
            this->Out("%6d %6d %8d %4d ", node->GetPredecessorCount(), node->GetSuccessorCount(), node->GetObjectSize(), node->GetDepth());

            if (PreferDML())
            {
                this->Dml("<link cmd=\"!jd.predecessors -limit 0 0x%p\">^</link> ", node->Key());
                this->Dml("<link cmd=\"!jd.successors -limit 0 0x%p\">v</link> ", node->Key());
                this->Dml("<link cmd=\"!jd.traceroots 0x%p\">&gt;</link> ", node->Key());
            }
            else
            {
                this->Out("^ /*\"!jd.predecessors -limit 0 0x%p\"*/ ", node->Key());
                this->Out("v /*\"!jd.successors -limit 0 0x%p\"*/ ", node->Key());
                this->Out("> /*\"!jd.traceroots 0x%p\"*/ ", node->Key());
            }

            this->Out("0x%p", node->Key());

            if (showLib)
            {
                GetExtension()->Out(" ");
                RemoteJavascriptLibrary(node->GetAssociatedJavascriptLibrary()).PrintLink();
            }

            DumpPossibleSymbol(node);

            this->Out("\n");
        }

        // limit == 0 means show all nodes
        if (limit != 0 && (++count) >= skip + limit)
        {
            this->Out("\nLimit of %I64d reached.", limit);

            std::string options = "";
            if (typeInfo) { options += " -ti"; }
            if (sortByPred) { options += " -sp"; }
            if (sortBySucc) { options += " -ss"; }
            if (sortByDepth) { options += " -sd"; }
            if (filterUnknownType) { options += " -fu"; }
            if (hasFilterLib)
            {
                char buffer[20];
                _i64toa_s(libraryFilter, buffer, _countof(buffer), 16);
                options += " -fl ";
                options += buffer;
            }
            if (hasFilterType) { options += " -ft " + JDUtil::EncodeDml(typeFilter); }
            if (depthFilter != (ULONG64)-1) { options += " -fd " + depthFilter; }
            if (PreferDML())
            {
                this->Dml(" <link cmd=\"!jd.jsobjectnodes -skip %I64d -limit %I64d%s\">(Display next %d)</link>", skip + limit, limit, options.c_str(), limit);
                this->Dml(" <link cmd=\"!jd.jsobjectnodes -limit 0%s\">(Display all)</link>", options.c_str());
            }
            else
            {
                this->Out(" (\"!jd.jsobjectnodes -skip %I64d -limit %I64d%s\" to display next %d)", skip + limit, limit, options.c_str(), limit);
                this->Out(" (\"!jd.jsobjectnodes -limit 0%s\" to display all)", options.c_str());
            }
            this->Out("\n");
            return true;
        }

        return false;
    };

    if (sortBySucc)
    {
        objectGraph.MapSorted<SortNodeBySuccessorAndObjectSize>(include, output);
    }
    else if (sortByPred)
    {
        objectGraph.MapSorted<SortNodeByPredecessorAndObjectSize>(include, output);
    }
    else if (sortByDepth)
    {
        objectGraph.MapSorted<SortNodeByDepthAndObjectSize>(include, output);
    }
    else
    {
        objectGraph.MapSorted<SortNodeByObjectSizeAndSuccessor>(include, output);
    }

    this->Out("--------------------------------------------------------------\n");
    this->Out("Total %d nodes, %d edges\n", objectGraph.GetNodeCount(), objectGraph.GetEdgeCount());
}

JD_PRIVATE_COMMAND(jsobjectdepths,
    "Dump a table of count of node of each depth",
    "{;e,o,d=0;recycler;Recycler address}"
    "{fl;ed,o;filterLib;Filter to library}"
    "{ft;x,o;filterType;Filter to type (last arg)}")
{
    ULONG64 recyclerArg = GetUnnamedArgU64(0);
    RemoteThreadContext threadContext;
    RemoteRecycler recycler = GetRecycler(recyclerArg, &threadContext);

    bool hasFilterLib = HasArg("fl");
    bool hasFilterType = HasArg("ft");
    bool typeInfo = hasFilterLib || hasFilterType;

    char const * typeFilter = hasFilterType ? GetArgStr("ft") : nullptr;
    ULONG64 libraryFilter = hasFilterLib ? GetArgU64("fl") : (ULONG64)-1;

    RecyclerObjectGraph &objectGraph = *(RecyclerObjectGraph::New(recycler, &threadContext,
        typeInfo ? RecyclerObjectGraph::TypeInfoFlags::Infer : RecyclerObjectGraph::TypeInfoFlags::None));

    ProgressTracker progress("Accumulating depth counts", 0x10000, objectGraph.GetNodeCount());
    std::map<uint, uint> depthCountMap;    
    objectGraph.MapAllNodes([&](RecyclerObjectGraph::GraphImplNodeType * node)
    {
        if (libraryFilter != (ULONG64)-1 && node->GetAssociatedJavascriptLibrary() != libraryFilter)
        {
            return;
        }

        if (hasFilterType)
        {
            if (node->IsPropagated())
            {
                return;
            }
            char const * typeNameOfField = node->GetTypeNameOrField();
            if (strcmp(typeNameOfField, typeFilter) != 0)
            {
                return;
            }
        }
        depthCountMap[node->GetDepth()]++;
        progress.Inc();
    });

    progress.Done("Accumulating depth counts completed");

    this->Out(" Dep    Count\n");
    this->Out("---- --------\n");
    size_t graphDepth = objectGraph.GetDepth();
    for (uint i = 0; i < graphDepth; i++)
    {
        this->Out("%4d %8d\n", i, depthCountMap[i]);
    }
}

