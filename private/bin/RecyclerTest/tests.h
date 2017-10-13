/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

typedef void (*PFTestFunc)(Recycler*, ArenaAllocator*, TestContext *ctx);

struct TestCase
{
    PFTestFunc func;
    char *name;
};

extern TestCase Tests[];

void TestMarkSweepDynamic(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestMarkSweepRandom(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestMarkSweep(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestLargeSimple(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestPerformance(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestRandom(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestRandomOperationString(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx);
void TestGraded(Recycler* recycler,ArenaAllocator* alloc, TestContext *ctx);
void TestList(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestBasic(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestBasic2(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestCycles(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestList(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestCustomHeap(Recycler* recycler, ArenaAllocator* alloc, TestContext* ctx);
void TestPointerSwaps(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestDoublyLinkedList(Recycler* recycler,ArenaAllocator* alloc, TestContext *ctx);
void TestWeakReferences(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestWeakReferenceHashTable(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx);
void TestNor(Recycler*, ArenaAllocator* alloc, TestContext *ctx);
void TestMem(Recycler*, ArenaAllocator* alloc, TestContext *ctx);
#ifdef RECYCLER_PAGE_HEAP
void TestPageHeapAlloc(Recycler*, ArenaAllocator* alloc, TestContext *ctx);
#endif
void TestRecyclerVisitedObjects(Recycler*, ArenaAllocator* alloc, TestContext *ctx);
