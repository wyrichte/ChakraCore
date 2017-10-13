//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// TODO: Copied from heapblock.h. Get this from the PDB instead.

// ObjectInfoBits is unsigned short, but only the lower byte is stored as the object attribute
// The upper bits are used to pass other information about allocation (e.g. NoDisposeBit)
//
enum RemoteObjectInfoBits : unsigned char
{
    // Bits that are actually stored in ObjectInfo

    NoBit = 0x00,    // assume an allocation is not leaf unless LeafBit is specified.
    FinalizeBit = 0x80,    // Indicates that the object has a finalizer
    PendingDisposeBit = 0x40,    // Indicates that the object is pending dispose
    LeafBit = 0x20,    // Indicates that the object is a leaf-object (objects without this bit need to be scanned)
    TrackBit = 0x10,    // Indicates that the object is a TrackableObject, or, in the case of RecyclerVisitedHostHeap objects, that the object is traced
    ImplicitRootBit = 0x08,
    NewTrackBit = 0x04,    // Tracked object is newly allocated and hasn't been process by concurrent GC
    MemoryProfilerOldObjectBit = 0x02,
    EnumClass_1_Bit = 0x01,    // This can be extended to add more enumerable classes (if we still have bits left)
};