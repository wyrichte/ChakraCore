//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

class ScriptMemoryDumper
{
public:
    typedef struct HeapStats
    {
        uint32 pageCount;
        uint32 objectSize;
        uint32 freeObjectCount;
        uint32 activeObjectCount;
        uint32 activeObjectByteSize;
        uint32 totalByteCount;
        uint32 finalizeCount; // hostdispatch etc.
    };

    ScriptMemoryDumper(Js::ScriptContext* scriptContext);

    ~ScriptMemoryDumper()
    {
        // just to reduce the noise on the stack.
        dumpObject = null;
    }

    Js::Var Dump();

private:
    template <typename TBlockType>
    void DumpHeapBucket(uint index, HeapBucketT<TBlockType> * heapBucket);
    template <typename TBlockType, typename TBlockAttributes>
    void DumpHeapBucket(uint index, SmallNormalHeapBucketBase<TBlockType>* heapBucket);
    template <typename TBlockAttributes>
    void DumpHeapBucket(uint index, SmallFinalizableHeapBucketT<TBlockAttributes>* heapBucket);
    void DumpLargeBucket(LargeHeapBucket* bucket);

    void ResetCurrentStats();
    void MergeCurrentStats();
    void SaveCurrentAtIndex(uint32 index);
    void SaveCurrentAsLargeBlock();
    void SaveSummary();
    void FillObjectWithStats(Js::DynamicObject* dynamicObject, HeapStats stats);

    template <class TBlockAttributes>
    void DumpSmallHeapBlockList(SmallHeapBlockT<TBlockAttributes>* heapBlockHead);
    template <class TBlockAttributes>
    void DumpSmallHeapBlock(SmallHeapBlockT<TBlockAttributes>* heapBlock);

    void DumpLargeHeapBlockList(LargeHeapBlock* heapBlockHead);
    void DumpLargeHeapBlock(LargeHeapBlock* heapBlock);
    void Init();

    Js::ScriptContext* scriptContext;
    Js::DynamicObject* dumpObject;
    HeapStats current;
    HeapStats total;
    Js::PropertyId pageCountId;
    Js::PropertyId objectSizeId;
    Js::PropertyId freeObjectCountId;
    Js::PropertyId activeObjectCountId;
    Js::PropertyId activeObjectByteSizeId;
    Js::PropertyId totalByteCountId;
    Js::PropertyId finalizeCountId;
    Js::PropertyId weakReferenceCountId;
    Js::PropertyId largeObjectsId;
    Js::PropertyId summaryId;

};

#endif
