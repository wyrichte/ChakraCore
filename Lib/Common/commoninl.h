//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// INL files
#ifdef F_JSETW
// Recycler.inl needs this.
#include <IERESP_mshtml.h>
#include <Microsoft-Scripting-ChakraEvents.h>
#endif
#include "Common\Tick.inl"
#include "core\ConfigFlagsTable.inl"
#include "core\ProfileMemory.inl"
#include "common\Int32Math.inl"
#include "Common\NumberUtilities.inl"
#include "Common\vtinfo.inl"
#include "DataStructures\BigInt.inl"
#include "DataStructures\DoublyLinkedListElement.inl"
#include "DataStructures\DoublyLinkedList.inl"
#include "Memory\Recycler.inl"
#include "Memory\MarkContext.inl"
#include "Memory\HeapBucket.inl"
#include "Memory\LargeHeapBucket.inl"
#include "Memory\HeapBlock.inl"
#include "Memory\HeapBlockMap.inl"
#include "Common\Jobs.inl"
