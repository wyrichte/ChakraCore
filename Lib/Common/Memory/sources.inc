TARGETNAME=JScript.Common.memory
TARGETTYPE=LIBRARY

INCLUDES= $(COREINCLUDES);$(OBJ_PATH)\..\..\..\..\..\manifests\inbox\$(O);$(OBJ_PATH)\..\..\..\..\manifests\$(O)

!if "$(_BUILDARCH)" == "AMD64"
INCLUDES=$(INCLUDES);amd64
!else
INCLUDES=$(INCLUDES);arm
!endif

SOURCES= \
    ..\Allocator.cpp \
    ..\HeapAllocator.cpp \
    ..\HeapAllocatorOperators.cpp \
    ..\VirtualAllocWrapper.cpp \
    ..\PageAllocator.cpp \
    ..\ArenaAllocator.cpp \
    ..\CustomHeap.cpp \
    ..\EtwMemoryTracking.cpp \
    ..\ForcedMemoryConstraints.cpp \
    ..\GCTelemetry.cpp \
    ..\HeapBlock.cpp \
    ..\IdleDecommitPageAllocator.cpp \
    ..\LargeHeapBlock.cpp \
    ..\LeakReport.cpp \
    ..\HeapBlockMap.cpp \
    ..\HeapBucket.cpp \
    ..\HeapInfo.cpp \
    ..\LargeHeapBucket.cpp \
    ..\MarkContext.cpp \
    ..\MemoryTracking.cpp \
    ..\MemoryLogger.cpp \
    ..\Recycler.cpp \
    ..\RecyclerHeuristic.cpp \
    ..\RecyclerObjectDumper.cpp \
    ..\RecyclerObjectGraphDumper.cpp \
    ..\RecyclerPageAllocator.cpp \
    ..\RecyclerSweep.cpp \
    ..\RecyclerWriteBarrierManager.cpp \
    ..\SmallHeapBlockAllocator.cpp \
    ..\SmallNormalHeapBlock.cpp \
    ..\SmallNormalHeapBucket.cpp \
    ..\SmallLeafHeapBlock.cpp \
    ..\SmallLeafHeapBucket.cpp \
    ..\SmallFinalizableHeapBlock.cpp \
    ..\SmallFinalizableHeapBucket.cpp \
    ..\StressTest.cpp \

AMD64_SOURCES= \
    ..\amd64\XDataAllocator.cpp \
    ..\amd64\amd64_SAVE_REGISTERS.asm \

ARM_SOURCEs= \
    ..\arm\arm_SAVE_REGISTERS.asm \
    ..\arm\XDataAllocator.cpp 

ARM64_SOURCES= \
    ..\arm64\arm64_SAVE_REGISTERS.asm \

PRECOMPILED_INCLUDE=..\..\stdafx.h
PRECOMPILED_SOURCEFILE=..\..\stdafx.cpp
PRECOMPILED_CXX=1
