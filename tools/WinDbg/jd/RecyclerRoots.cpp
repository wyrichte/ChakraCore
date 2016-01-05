#include "stdafx.h"
#include "jdrecycler.h"
#include "RemoteRecyclerList.h"
#include "RecyclerRoots.h"

#include <hash_set>
#include <stack>
#include <queue>

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

    if (_transientPinnedObject != 0 )
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

RecyclerObjectGraph::RecyclerObjectGraph(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler, bool verbose) :
    _ext(extension),
    _verbose(verbose),
    _heapBlockHelper(extension, recycler),
    _recycler(recycler)
{
}

RecyclerObjectGraph::~RecyclerObjectGraph()
{
}

// Dumps the object graph for manipulation in python
void RecyclerObjectGraph::DumpForPython(const char* outfile)
{
    _objectGraph.ExportToPython(outfile);
}

// Dumps the object graph for manipulation in js
void RecyclerObjectGraph::DumpForJs(const char* outfile)
{
    _objectGraph.ExportToJs(outfile);
}

void RecyclerObjectGraph::PushMark(ULONG64 object, ULONG64 prev)
{
    if (object != 0)
    {
        MarkStackEntry entry;
        entry.first = object;
        entry.second = prev;
        _markStack.push(entry);
    }
}

void RecyclerObjectGraph::Construct(Addresses& roots)
{
    roots.Map([&] (ULONG64 root)
    {
        PushMark(root, 0);
    });

    int iters = 0;

    while (_markStack.size() != 0)
    {
        const MarkStackEntry& object = _markStack.top();
        _markStack.pop();

        ULONG64 objectAddress = object.first;
        ULONG64 prevAddress = object.second;

        MarkObject(objectAddress, prevAddress);

        iters++;
        if (iters % 0x10000 == 0)
        {
            iters = 0;
            _ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rTraversing object graph, unvisited object count: %d", _markStack.size(), _objectGraph.Count());
        }
    }
}

#if ENABLE_MARK_OBJ
void RecyclerObjectGraph::FindPathTo(RootPointers& roots, ULONG64 address, ULONG64 root)
{
    if (_objectGraph.GetNode(address) != nullptr)
    {
        std::vector<ULONG64> shortestPath;
        int pathsFound = 0;

        if (root)
        {
            shortestPath = _objectGraph.FindPath(root, address);
        }
        else
        {
            roots.Map([&] (ULONG64 rootAddress)
            {
                _ext->Out("Finding path from 0x%P\n", rootAddress);
                std::vector<ULONG64> path = _objectGraph.FindPath(rootAddress, address);

                if (shortestPath.size() == 0 || shortestPath.size() > path.size())
                {
                    shortestPath = path;
                    if (shortestPath.size() != 0) {
                        _ext->Out("Shortest path so far is from 0x%P\n", rootAddress);
                        pathsFound++;
                    }
                }
            });
        }

        _ext->Out("Shortest path has %d nodes\n", shortestPath.size());
        for (auto it = shortestPath.begin(); it != shortestPath.end(); it++)
        {
            _ext->Out("0x%P\n", (*it));
        }
    }
    else
    {
        _ext->Out("Object not found\n");
    }
}
#endif

void RecyclerObjectGraph::MarkObject(ULONG64 address, ULONG64 prev)
{
    if (_heapBlockHelper.IsAlignedAddress(address))
    {
        ULONG64 heapBlockAddress = _heapBlockHelper.FindHeapBlock(address, _recycler, false);
        if (heapBlockAddress != 0)
        {
            if (prev)
            {
                _objectGraph.AddEdge(prev, address);
            }

            GraphNode<ULONG64, RecyclerGraphNodeAux>* node = _objectGraph.GetNode(address);

            if (!node->aux.isScanned)
            {
                ExtRemoteTyped heapBlock(_ext->FillModuleAndMemoryNS("%s!%sHeapBlock"), heapBlockAddress, false);
                auto type = _heapBlockHelper.GetHeapBlockType(heapBlock);
                ULONG64 objectSize = 0;

                if (type == _ext->enum_LargeBlockType())
                {
                    ExtRemoteTyped largeBlock(_ext->FillModuleAndMemoryNS("%s!%sLargeHeapBlock"), heapBlockAddress, false);
                    objectSize = GetLargeObjectSize(largeBlock, address);
                }
                else if (type != _ext->enum_SmallLeafBlockType())
                {
                    ExtRemoteTyped smallBlock(_ext->GetSmallHeapBlockTypeName(), heapBlockAddress, false);

                    // Hack- make this a friend of SmallHeapBlock
                    if (_heapBlockHelper.GetSmallHeapBlockObjectIndex(smallBlock, address) != 0xffff)
                    {
                        objectSize = (ULONG64)smallBlock.Field("objectSize").GetUshort();
                    }
                }

                if (objectSize != 0)
                {
                    ScanBytes(address, objectSize);
                }

                if (type != _ext->enum_SmallLeafBlockType())
                {
                    node->aux.objectSize = (uint)objectSize;
                }
                else
                {
                    ExtRemoteTyped smallBlock(_ext->GetSmallHeapBlockTypeName(), heapBlockAddress, false);

                    node->aux.objectSize = (ULONG64)smallBlock.Field("objectSize").GetUshort();
                }
                node->aux.isScanned = true;
            }
        }
    }
}

ULONG64 RecyclerObjectGraph::GetLargeObjectSize(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress)
{
    ULONG64 heapBlock = heapBlockObject.GetPointerTo().GetPtr();
    ULONG64 blockAddress = heapBlockObject.Field("address").GetPtr();

    ULONG64 sizeOfHeapBlock = _ext->EvalExprU64(_ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeHeapBlock))"));
    ULONG64 sizeOfObjectHeader = _ext->EvalExprU64(_ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));

    ULONG64 headerAddress = objectAddress - sizeOfObjectHeader;

    if (headerAddress < blockAddress)
    {
        if (_verbose)
            _ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
        return 0;
    }

    ExtRemoteTyped largeObjectHeader(_ext->FillModuleAndMemoryNS("%s!%sLargeObjectHeader"), headerAddress, false);

    HeapObject heapObject;
    ULONG64 objectCount = EXT_CLASS_BASE::GetSizeT(heapBlockObject.Field("objectCount"));
    heapObject.index = (ushort) largeObjectHeader.Field("objectIndex").m_Typed.Data; // Why does calling UShort not work?

    if (heapObject.index > objectCount)
    {
        return 0;
    }

    ULONG largeObjectHeaderPtrSize = _ext->m_PtrSize;
    ULONG64 headerArrayAddress = heapBlock + sizeOfHeapBlock + (heapObject.index * largeObjectHeaderPtrSize);
    ExtRemoteData  headerData(headerArrayAddress, largeObjectHeaderPtrSize);

    if (headerData.GetPtr() != headerAddress)
    {
        if (_verbose)
        {
            _ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
            _ext->Out("Header address: 0x%p, Header in index %d is 0x%p\n", headerAddress, heapObject.index, headerData.GetPtr());
        }

        return 0;
    }

    return EXT_CLASS_BASE::GetSizeT(largeObjectHeader.Field("objectSize"));
}

void RecyclerObjectGraph::ScanBytes(ULONG64 address, ULONG64 size)
{
    char* object = (char*)malloc((size_t)size);
    if (!object)
    {
        _ext->ThrowOutOfMemory();
    }

    ExtRemoteData data(address, (ULONG) size);

    data.ReadBuffer(object, (ULONG) size);
    char* end = object + size;
    char* current = object;
    ulong ptrSize = this->_ext->m_PtrSize;

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

        ULONG64 objectAlignmentMask = this->_heapBlockHelper.GetObjectAlignmentMask();
        if ((0 == (((size_t)address) & objectAlignmentMask)) && (value != address))
        {
            PushMark(value, address);
        }

        current += ptrSize;
    }

    free(object);
}

void ScanRegisters(EXT_CLASS_BASE* ext, RootPointers& rootPointerManager, bool print = true)
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

        if (rootPointerManager.TryAdd(value) && print)
        {
            ext->Out("0x%p (Register %s)\n", value, buffer);
        }
    }
}

void ScanStack(EXT_CLASS_BASE* ext, ExtRemoteTyped& recycler, RootPointers& rootPointerManager, bool print = true)
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

    size_t stackSizeInBytes = (size_t) (stackBase - stackTop);
    void** stack = (void**) malloc(stackSizeInBytes);
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
            if (rootPointerManager.TryAdd((ULONG64)stack32[i]) && print)
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
            if (rootPointerManager.TryAdd((ULONG64)stack[i]) && print)
            {
                ext->Out("0x%p", stack[i]);
                ext->Out(" (+0x%x)\n", i * ext->m_PtrSize);
            }
        }
    }


    free(stack);
}

void ScanObject(ULONG64 object, size_t bytes, RootPointers& rootPointerManager)
{
    EXT_CLASS_BASE* ext = GetExtension();

#if _M_X64
    bool targetIs32Bit = (g_Ext->m_PtrSize == 4);

    if (targetIs32Bit)
    {
        Assert(bytes % sizeof(ULONG32) == 0);

        ULONG32* objectBytes = (ULONG32*)malloc(bytes);
        if (!objectBytes)
        {
            ext->ThrowOutOfMemory();
        }

        ExtRemoteData data(object, (ULONG)bytes);
        data.ReadBuffer(objectBytes, (ULONG)bytes);
        for (uint i = 0; i < bytes / ext->m_PtrSize; i++)
        {
            rootPointerManager.TryAdd((ULONG64)objectBytes[i]);
        }

        free(objectBytes);
    }
    else
#endif
    {
        void** objectBytes = (void**)malloc(bytes);
        if (!objectBytes)
        {
            ext->ThrowOutOfMemory();
        }

        ExtRemoteData data(object, (ULONG)bytes);
        data.ReadBuffer(objectBytes, (ULONG)bytes);
        for (uint i = 0; i < bytes / ext->m_PtrSize; i++)
        {
            rootPointerManager.TryAdd((ULONG64)objectBytes[i]);
        }

        free(objectBytes);
    }

}

void ScanArenaBigBlocks(ExtRemoteTyped blocks, RootPointers& rootPointerManager)
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
        size_t byteCount = (size_t)EXT_CLASS_BASE::GetSizeT(nBytesField);
        if (byteCount != 0)
        {
            ScanObject(blockBytes, byteCount, rootPointerManager);
        }
        blocks = block.Field("nextBigBlock");
    }

}

void ScanArenaMemoryBlocks(ExtRemoteTyped blocks, RootPointers& rootPointerManager)
{
    EXT_CLASS_BASE* ext = GetExtension();

    while (blocks.GetPtr() != 0)
    {
        ULONG64 blockBytes = blocks.GetPtr() + ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sArenaMemoryBlock))"));
        ExtRemoteTyped nBytesField = blocks.Field("nbytes");
        size_t byteCount = (size_t)nBytesField.GetLong();
        ScanObject(blockBytes, byteCount, rootPointerManager);
        blocks = blocks.Field("next");
    }
}

void ScanArenaData(ULONG64 arenaDataPtr, RootPointers& rootPointerManager)
{
    EXT_CLASS_BASE* ext = GetExtension();
    ExtRemoteTyped arenaData(ext->FillModuleAndMemoryNS("(%s!%sArenaData*)@$extin"), arenaDataPtr);
    ScanArenaBigBlocks(arenaData.Field("bigBlocks"), rootPointerManager);
    ScanArenaBigBlocks(arenaData.Field("fullBlocks"), rootPointerManager);
    ScanArenaMemoryBlocks(arenaData.Field("mallocBlocks"), rootPointerManager);
}

void ScanArena(ULONG64 arena, RootPointers& rootPointerManager, bool verbose)
{
    EXT_CLASS_BASE* ext = GetExtension();
    if (verbose)
        ext->Out("Scanning arena 0x%p\n", arena);
    arena += ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sAllocator))"));
    ScanArenaData(arena, rootPointerManager);
}

class ScanImplicitRoot : public HeapBlockMapWalker
{
public:
    ScanImplicitRoot(EXT_CLASS_BASE * ext, ExtRemoteTyped recycler, RootPointers& rootPointers, bool print = true) :
        HeapBlockMapWalker(ext, recycler), rootPointers(rootPointers), implicitRootCount(0), implicitRootSize(0), print(print)
    {};
    ~ScanImplicitRoot()
    {
        if (print)
        {
            ext->Out("\nImplicit Root: Count=%I64d Size=%I64d\n", implicitRootCount, implicitRootSize);
        }
    }
    virtual bool ProcessHeapBlock(size_t l1Id, size_t l2Id, ULONG64 blockAddress, ExtRemoteTyped block)
    {
        ULONG64 heapBlock = block.GetPtr();
        ushort objectCount = block.Field("objectCount").GetUshort();
        ULONG64 attributeStartAddress = heapBlock - objectCount;

        ExtRemoteData remoteAttributes(attributeStartAddress, objectCount);
        std::vector<byte> attributes(objectCount);
        remoteAttributes.ReadBuffer(&attributes[0], objectCount);

        ULONG64 heapBlockAddress = block.Field("address").GetPtr();
        ushort objectSize = block.Field("objectSize").GetUshort();
        for (ushort i = 0; i < objectCount; i++)
        {
            if ((attributes[objectCount - i - 1] & ObjectInfoBits::ImplicitRootBit)
                && (attributes[objectCount - i - 1] & ObjectInfoBits::PendingDisposeBit) == 0)
            {
                rootPointers.Add(heapBlockAddress + objectSize * i);
                if (print)
                {
                    implicitRootCount++;
                    implicitRootSize += objectSize;
                }
            }
        }
        return false;
    }
    virtual bool ProcessLargeHeapBlock(size_t l1Id, size_t l2Id, ULONG64 blockAddress, ExtRemoteTyped heapBlock)
    {
        unsigned int allocCount = heapBlock.Field("allocCount").GetUlong();
        ExtRemoteTyped headerList =
            ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), heapBlock.GetPtr() + heapBlock.Dereference().GetTypeSize());
        ULONG64 sizeOfObjectHeader = ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));
        for (unsigned int i = 0; i < allocCount; i++)
        {
            ExtRemoteTyped header = headerList.ArrayElement(i);
            if (header.GetPtr() == 0)
            {
                continue;
            }
            byte attribute;

            if (header.HasField("attributesAndChecksum"))
            {
                attribute = (UCHAR)(header.Field("attributesAndChecksum").GetUshort() ^ recycler.Field("Cookie").GetUlong() >> 8);
            }
            else if (header.HasField("attributes"))
            {
                attribute = header.Field("attributes").GetUchar();
            }
            else
            {
                ext->Err("Can't find either attributes or attributesAndChecksum on LargeHeapBlock");
                return false;
            }

            if (attribute & ObjectInfoBits::ImplicitRootBit)
            {
                rootPointers.Add(header.GetPtr() + sizeOfObjectHeader);
                if (print)
                {
                    implicitRootCount++;
                    implicitRootSize += EXT_CLASS_BASE::GetSizeT(header.Field("objectSize"));
                }
            }
        }
        return false;
    }

private:
    RootPointers& rootPointers;
    ULONG64 implicitRootCount;
    ULONG64 implicitRootSize;
    bool print;
};

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
            ULONG64 externalObject = (ULONG64) entry.address + offsetOfExternalObject;
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
    unsigned int allocCount = (uint)EXT_CLASS_BASE::GetSizeT(block.Field("allocCount"));
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
        ULONG64 objectSize = EXT_CLASS_BASE::GetSizeT(header.Field("objectSize"));

        ULONG64 startAddress = header.GetPtr() + sizeOfObjectHeader;

        SearchRange(startAddress, (uint)objectSize, ext, this->referencedObject, [&](ULONG64 addr){
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
    std::for_each(findRef.results.begin(), findRef.results.end(), [&](decltype(findRef.results[0])& result){
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
    ULONG64 heapBlockAddress = heapBlockHelper.FindHeapBlock(objectAddress, recycler);
    if (heapBlockAddress != NULL)
    {
        ExtRemoteTyped heapBlock(FillModuleAndMemoryNS("(%s!%sHeapBlock*)@$extin"), heapBlockAddress);
        ULONG64 heapBlockType = heapBlockHelper.GetHeapBlockType(heapBlock);

        heapBlockHelper.DumpHeapBlockLink(heapBlockType, heapBlock.GetPtr());

        if (heapBlockType >= this->enum_BlockTypeCount())
        {
            Out("Object returned Invalid Heap Block\n");
            return;
        }

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
        Out("Object not found\n");
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

    RootPointers rootPointerManager(this, recycler);

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
        ScanImplicitRoot implicitRootScan(this, recycler, rootPointerManager);
        implicitRootScan.Run();
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
        rootPointerManager.TryAdd((ULONG64) entry.address);
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
    ScanRegisters(this, rootPointerManager);
    ScanStack(this, recycler, rootPointerManager);

    PCSTR typeName = recycler.Field("heapBlockMap").GetTypeName();
    Out("Heap block map type is %s\n", typeName);
}

RootPointers * ComputeRoots(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, ExtRemoteTyped* threadContext, bool dump)
{
    std::auto_ptr<RootPointers> rootPointerManager(new RootPointers(ext, recycler));

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
        ScanImplicitRoot implicitRootScan(ext, recycler, *rootPointerManager, dump);
        implicitRootScan.Run();
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
        rootPointerManager->TryAdd(entry.address);
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
            ScanArena(data, *rootPointerManager, dump);
        }
    }

    ExtRemoteTyped egal = recycler.Field("externalGuestArenaList");
    ULONG64 externalGuestArenaList = egal.GetPointerTo().GetPtr();

    // Need to scan external guest arena
    RemoteListIterator<false> externalGuestArenaIterator("ArenaData *", externalGuestArenaList);

    while (externalGuestArenaIterator.Next())
    {
        ULONG64 dataPtr = externalGuestArenaIterator.GetDataPtr();

        ScanArenaData(GetPointerAtAddress(dataPtr), *rootPointerManager);
    }

    ScanRegisters(ext, *rootPointerManager, dump);
    ScanStack(ext, recycler, *rootPointerManager, dump);

    return rootPointerManager.release();
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

JD_PRIVATE_COMMAND(jsobjectstats,
    "Dump a table of object types and statistics",
    "{;e,o,d=0;recycler;Recycler address}"
    "{v;b,o;verbose;Display verbose tracing}"
    "{t;b,o;trident;Display trident symbols}"
    )
{
    ULONG moduleIndex = 0;
    ULONG64 baseAddress = 0;
    ULONG64 endAddress = 0;
    bool verbose = HasArg("v");

    if (FAILED(this->m_Symbols3->GetModuleByModuleName(this->GetModuleName(), 0, &moduleIndex, &baseAddress)))
    {
        Out("Unable to get range for module '%s'. Is Chakra loaded?\n", this->GetModuleName());
        return;
    }

    IMAGEHLP_MODULEW64 moduleInfo;
    this->GetModuleImagehlpInfo(baseAddress, &moduleInfo);
    endAddress = baseAddress + moduleInfo.ImageSize;
    Out("Chakra Vtables are in the range %p to %p\n", baseAddress, endAddress);

    bool trident = HasArg("t");

    ULONG64 tridentBaseAddress = 0;
    ULONG64 tridentEndAddress = 0;
    if (trident)
    {
        if (FAILED(this->m_Symbols3->GetModuleByModuleName("mshtml", 0, &moduleIndex, &tridentBaseAddress)) &&
            FAILED(this->m_Symbols3->GetModuleByModuleName("edgehtml", 0, &moduleIndex, &tridentBaseAddress)))
        {
            Out("Unable to get range for module 'mshtml' or 'edgehtml. Is Trident loaded?\n");
            return;
        }
        this->GetModuleImagehlpInfo(tridentBaseAddress, &moduleInfo);
        tridentEndAddress = tridentBaseAddress + moduleInfo.ImageSize;
        Out("Trident Vtables are in the range %p to %p\n", tridentBaseAddress, tridentEndAddress);
    }

    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped recycler = GetRecycler(arg);
    ExtRemoteTyped threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();

    Addresses * rootPointerManager = this->recyclerCachedData.GetRootPointers(recycler, &threadContext);
    if (verbose)
        Out("\nNumber of root GC pointers found: %d\n\n", rootPointerManager->Count());

    RecyclerObjectGraph objectGraph(this, recycler);

    objectGraph.Construct(*rootPointerManager);
    Out("\rObject graph construction complete                                  \n");

    struct ObjectAllocStats
    {
        int count;
        int size;
        ULONG64 pointerTo;
    };

    enum SortKind
    {
        SortKindByCount,
        SortKindBySize
    };

    SortKind sortKind = SortKindByCount;
    stdext::hash_map<ULONG64, ObjectAllocStats> vtableCounts;
    int numNodes = 0;
    int totalSize = 0;

    SortedBuffer<ULONG64, int, 50> topNVTables;

    objectGraph.MapNodes([&](ULONG64& objectAddress, RecyclerObjectGraph::GraphImplNodeType* node)
    {
        numNodes++;
        totalSize += (node->aux.objectSize);

        ExtRemoteData data(objectAddress, this->m_PtrSize);
        data.Read();
        ULONG64 vtable = data.GetPtr();

        bool isVtable = vtable % 4 == 0 && vtable >= baseAddress && vtable < endAddress;

        if (!isVtable && trident)
        {
            // REVIEW: 0xc is the start object offset?
            ExtRemoteData tridentdata(objectAddress + 0xc, this->m_PtrSize);
            tridentdata.Read();
            vtable = tridentdata.GetPtr();
            isVtable = vtable % 4 == 0 && vtable >= tridentBaseAddress && vtable < tridentEndAddress;
        }

        if (isVtable)
        {
            ObjectAllocStats stats;
            stats.count = 0;
            stats.size = 0;
            stats.pointerTo = objectAddress;

            if (vtableCounts.find(vtable) != vtableCounts.end())
            {
                stats = vtableCounts[vtable];
            }

            stats.count++;
            stats.size += (node->aux.objectSize);

            vtableCounts[vtable] = stats;

            if (sortKind == SortKindByCount)
            {
                topNVTables.Add(vtable, stats.count);
            }
            else if (sortKind == SortKindBySize)
            {
                topNVTables.Add(vtable, stats.size);
            }
        }
    });

    if (sortKind == SortKindByCount)
    {
        Out("      Count\t       Size\tSymbol\n");
    }
    else if (sortKind == SortKindBySize)
    {
        Out("       Size\t      Count\t\tSymbol\n");
    }

    Out("----------------------------------------------------------------------------\n");
    topNVTables.Map([&](ULONG64 vtable, int count)
    {
        ObjectAllocStats stats = vtableCounts[vtable];
        if (sortKind == SortKindByCount)
        {
            Out("%11u\t%11u\t%y (0x%p)\n", stats.count, stats.size, vtable, stats.pointerTo);
        }
        else if (sortKind == SortKindBySize)
        {
            Out("%11u\t%11u\t%y (0x%p)\n", stats.size, stats.count, vtable, stats.pointerTo);
        }
    });
    Out("----------------------------------------------------------------------------\n");

    if (verbose)
    {
        Out("%d vtable's found\n", vtableCounts.size());
    }
    Out("Number of marked objects: %d\n", numNodes);
    Out("Total size of marked objects: %d\n", totalSize);

}

#endif
