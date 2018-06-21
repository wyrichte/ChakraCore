
#include "TelemetryPch.h"
#include "RecyclerTelemetryTransmitter.h"
#include "TelemetryMacros.h"
#include "memory/HeapInfo.h"
#include "memory/BucketStatsReporter.h"

namespace Js
{
    const uint8 WAIT_FOR_CONCURRENT_SOURCE_COUNT = RecyclerWaitReason::Other + 1;
#ifdef RECYCLER_WRITE_BARRIER_ALLOC_SEPARATE_PAGE
    const uint8 NUM_RECYCLER_ALLOCATORS = 4;
#else
    const uint8 NUM_RECYCLER_ALLOCATORS = 3;
#endif

    uint32 waitForConcurrentCRCs[WAIT_FOR_CONCURRENT_SOURCE_COUNT] = {
#define P(n) CalculateCRC32(STRINGIZE(GC_UI_THREAD_BLOCKED_ ## n)),
#include "Memory/RecyclerWaitReasonInc.h"
#undef P
    };

    enum RecyclerSizeEntries :uint8
    {
#define RECYCLER_SIZE_NO_SUBFIELD(size_entry) size_entry,
#define RECYCLER_SIZE_SUBFIELD(entry, subfield) entry ## _ ## subfield,
#include "RecyclerSizeEntries.h"
#undef RECYCLER_SIZE_NO_SUBFIELD
#undef RECYCLER_SIZE_SUBFIELD
        Count
    };

    uint32 sizeEntriesCRCs[RecyclerSizeEntries::Count] =
    {
#define RECYCLER_SIZE_NO_SUBFIELD(size_entry) CalculateCRC32(STRINGIZE(size_entry)),
#define RECYCLER_SIZE_SUBFIELD(entry, subfield) CalculateCRC32(STRINGIZE(entry ## _ ## subfield)),
#include "RecyclerSizeEntries.h"
#undef RECYCLER_SIZE_NO_SUBFIELD
#undef RECYCLER_SIZE_SUBFIELD
    };

    /**
     * return true if telemetry provider is enabled
     */
    bool IsTelemetryProviderEnabled()
    {
        return g_TraceLoggingClient->IsProviderEnabled();
    }

    /**
     *  Transmit an gc telemetry error message. Used when we want to report that something went wrong.
     */
    bool TransmitRecyclerTelemetryError(const RecyclerTelemetryInfo& info, const char * msg)
    {
        bool sent = false;
        if (!g_TraceLoggingClient->IsThrottled())
        {
            TraceLogChakra("GCTelemetryError",
                TraceLoggingGuid(info.GetRecyclerID(), "recyclerID"),
                TraceLoggingString(msg, "errorMessage")
            );
            sent = true;
        }
        return sent;
    }

    /**
     * Transmit telemetry data associated with given RecyclerTelemetryInfo.
     * returns true if telemetry was sent, false if not sent.
     */
    bool TransmitRecyclerTelemetry(RecyclerTelemetryInfo& info)
    {
        bool sent = false;
        AssertMsg(info.GetLastPassStats() == nullptr ? info.GetPassCount() == 0 : info.GetPassCount() > 0, "unexpected info.passCount value");
        if (!g_TraceLoggingClient->IsThrottled() && info.GetPassCount() > 0 && info.GetLastPassStats() != nullptr)
        {
            Js::Tick start = Js::Tick::Now();

            //
            // We don't want to send telemetry events too frequently, so we pool
            // the structs populated with each GC pass, and then send a variable
            // number periodically. However, win32 telemetry macros require that
            // we have a fixed set of fields. To accomdate this, we pack the 
            // fields from a variable number of RecyclerTelemetryGCPassStats 
            // structs into a fixed set of arrays, one array per struct field.
            // These will need to be unpacked on the backend to get per-pass
            // stats
            //
            const uint16 passCount = info.GetPassCount();
            const uint16 uiThreadBlockedTimesSize = passCount * (WAIT_FOR_CONCURRENT_SOURCE_COUNT);
            const uint16 sizesArrayLength = passCount * RecyclerSizeEntries::Count;

            int64*     passElapsedTimesMicros          = HeapNewNoThrowArrayZ(int64, passCount);
            LPFILETIME lastScriptExecutionTimes        = HeapNewNoThrowArrayZ(FILETIME, passCount);
            LPFILETIME passStartTimes                  = HeapNewNoThrowArrayZ(FILETIME, passCount);
            bool*      isInScriptArray                 = HeapNewNoThrowArrayZ(bool, passCount);
            bool*      isScriptActiveArray             = HeapNewNoThrowArrayZ(bool, passCount);
            int64*     heapInfoUsedBytesArray          = HeapNewNoThrowArrayZ(int64, passCount);
            int64*     heapInfoTotalBytes              = HeapNewNoThrowArrayZ(int64, passCount);
            int64*     startProcessingTimeMicros       = HeapNewNoThrowArrayZ(int64, passCount);
            int64*     endProcessingTimeMicros         = HeapNewNoThrowArrayZ(int64, passCount);
            int64*     bucketStatsProcessingTimeMicros = HeapNewNoThrowArrayZ(int64, passCount);
            uint32*    startPassCollectionState        = HeapNewNoThrowArrayZ(uint32, passCount);
            uint32*    endPassCollectionState          = HeapNewNoThrowArrayZ(uint32, passCount);
            uint32*    collectionStartReason           = HeapNewNoThrowArrayZ(uint32, passCount);
            uint32*    collectionFinishReason          = HeapNewNoThrowArrayZ(uint32, passCount);
            uint32*    collectionStartFlags            = HeapNewNoThrowArrayZ(uint32, passCount);
            int64*     uiThreadBlockedTimes            = HeapNewNoThrowArrayZ(int64, uiThreadBlockedTimesSize);
            int64*     sizesArray                      = HeapNewNoThrowArrayZ(int64, sizesArrayLength);
            uint*      pinnedObjectCountArray          = HeapNewNoThrowArrayZ(uint, passCount);
            uint*      closedContextCountArray         = HeapNewNoThrowArrayZ(uint, passCount);



            //
            // This method is invoked via recycler. If we fail to allocate memory,
            // we'll just skip sending telemetry & try again next time.
            //
            if (passElapsedTimesMicros          &&
                lastScriptExecutionTimes        &&
                passStartTimes                  &&
                isInScriptArray                 &&
                isScriptActiveArray             &&
                heapInfoUsedBytesArray          &&
                heapInfoTotalBytes              &&
                startProcessingTimeMicros       &&
                endProcessingTimeMicros         &&
                bucketStatsProcessingTimeMicros &&
                startPassCollectionState        &&
                endPassCollectionState          &&
                collectionStartReason           &&
                collectionFinishReason          &&
                collectionStartFlags            &&
                uiThreadBlockedTimes            &&
                sizesArray                      &&
                pinnedObjectCountArray          &&
                closedContextCountArray)
            {
                // Walk the list of stats for each GC pass, and we pack all the values
                // from similiar fields into a single array for transmission.
                GCPassStatsList::Iterator  it = info.GetGCPassStatsIterator();
                size_t currCount = 0;
                while (it.Next())
                {
                    RecyclerTelemetryGCPassStats& curr = it.Data();

                    passElapsedTimesMicros[currCount] = (curr.passEndTimeTick - curr.passStartTimeTick).ToMicroseconds();
                    passStartTimes[currCount] = curr.passStartTimeFileTime;
                    lastScriptExecutionTimes[currCount] = curr.lastScriptExecutionEndTime;
                    isInScriptArray[currCount] = curr.isInScript;
                    isScriptActiveArray[currCount] = curr.isScriptActive;
                    pinnedObjectCountArray[currCount] = curr.pinnedObjectCount;
                    closedContextCountArray[currCount] = curr.closedContextCount;

                    heapInfoUsedBytesArray[currCount] = curr.bucketStats.objectByteCount;
                    heapInfoTotalBytes[currCount] = curr.bucketStats.totalByteCount;

                    startProcessingTimeMicros[currCount] = curr.startPassProcessingElapsedTime.ToMicroseconds();
                    endProcessingTimeMicros[currCount] = curr.endPassProcessingElapsedTime.ToMicroseconds();
                    bucketStatsProcessingTimeMicros[currCount] = curr.computeBucketStatsElapsedTime.ToMicroseconds();

                    startPassCollectionState[currCount] = curr.startPassCollectionState;
                    endPassCollectionState[currCount] = curr.endPassCollectionState;
                    collectionStartReason[currCount] = curr.collectionStartReason;
                    collectionFinishReason[currCount] = curr.collectionFinishReason;
                    collectionStartFlags[currCount] = curr.collectionStartFlags;

                    // pack the array of times where UI thread was blocked into a single array.  Will need 
                    // to unapck as part of backend processing
                    for (size_t j = 0; j < WAIT_FOR_CONCURRENT_SOURCE_COUNT; j++)
                    {
                        size_t index = (currCount * WAIT_FOR_CONCURRENT_SOURCE_COUNT) + j;
                        uiThreadBlockedTimes[index] = curr.uiThreadBlockedTimes[j].ToMicroseconds();
                    }

                    // Add in recycler size data.  Offset is the offset for this GC pass into sizesArray
                    size_t offset = currCount * RecyclerSizeEntries::Count;
#if DBG
                    // sanity-check we're writing into correct places in array
                    for (size_t i = offset; i < (offset + RecyclerSizeEntries::Count); i++)
                    {
                        AssertMsg(sizesArray[i] == 0, "something's wrong with writing values into sizesArray");
                    }
#endif

                    // pre-processor magic to get data from structs into sizesArray
#define RECYCLER_SIZE_NO_SUBFIELD(size_entry) sizesArray[offset + RecyclerSizeEntries::size_entry] = curr.size_entry;
#define RECYCLER_SIZE_SUBFIELD(entry, subfield) sizesArray[offset + RecyclerSizeEntries::entry ## _ ## subfield] = curr.entry ## . ## subfield;
#include "RecyclerSizeEntries.h"
#undef RECYCLER_SIZE_NO_SUBFIELD
#undef RECYCLER_SIZE_SUBFIELD

                    currCount++;
                };

                AssertMsg(currCount == passCount, "mismatch between i and info.passCount");

                Js::Tick now = Js::Tick::Now();
                int64 recyclerLifeSpanMicros = (now - info.GetRecyclerStartTime()).ToMicroseconds();
                int64 microsSinceListTransmit = info.GetLastTransmitTime().ToMicroseconds() == 0 ? -1 : (now - info.GetLastTransmitTime()).ToMicroseconds();

                int64 withBarrier_numDecommitCalls    = 0;
                int64 withBarrier_numPagesDecommitted = 0;
                int64 withBarrier_numFreePageCount    = 0;
                int64 withBarrier_maxDelta            = 0;
                int64 withBarrier_lastEnterLeaveIdleDecommitCSWaitTime = 0;
                int64 withBarrier_maxEnterLeaveIdleDecommitCSWaitTime = 0;
                int64 withBarrier_totalEnterLeaveIdleDecommitCSWaitTime = 0;

#ifdef RECYCLER_WRITE_BARRIER_ALLOC_SEPARATE_PAGE
                // TraceLog* APIs are macros, and we can't have a #define inside the call to a macro
                withBarrier_numDecommitCalls = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->numDecommitCalls;
                withBarrier_numPagesDecommitted = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->numPagesDecommitted;
                withBarrier_numFreePageCount = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->numFreePageCount;
                withBarrier_maxDelta = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->maxDeltaBetweenDecommitRegionLeaveAndDecommit.ToMicroseconds();
                withBarrier_lastEnterLeaveIdleDecommitCSWaitTime = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->lastEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds();
                withBarrier_maxEnterLeaveIdleDecommitCSWaitTime = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->maxEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds();
                withBarrier_totalEnterLeaveIdleDecommitCSWaitTime = info.GetRecyclerWithBarrierPageAllocator_decommitStats()->totalEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds();
#endif

                TraceLogChakra("GCTelemetry_0",
                    TraceLoggingGuid(info.GetRecyclerID(), "recyclerID"),
                    TraceLoggingInt64(recyclerLifeSpanMicros, "recyclerLifeSpanMicros"),
                    TraceLoggingInt64(microsSinceListTransmit, "microsSinceLastTransmit"),
                    TraceLoggingBool(info.GetIsConcurrentEnabled(), "isConcurrentEnabled"),
                    TraceLoggingUInt32(passCount, "passCount"),

                    TraceLoggingInt64Array(passElapsedTimesMicros, passCount, "passElapsedTimesMicros"),
                    TraceLoggingFileTimeArray(passStartTimes, passCount, "passStartTimes"),
                    TraceLoggingFileTimeArray(lastScriptExecutionTimes, passCount, "lastScriptExecutionTimes"),

                    TraceLoggingBoolArray(isInScriptArray, passCount, "isInScript"),
                    TraceLoggingBoolArray(isScriptActiveArray, passCount, "isScriptActive"),

                    TraceLoggingInt64Array(uiThreadBlockedTimes, uiThreadBlockedTimesSize, "UIThreadBlockedMicros"),
                    TraceLoggingUInt32Array(waitForConcurrentCRCs, WAIT_FOR_CONCURRENT_SOURCE_COUNT, "UIThreadBlockedNameCRCs"),

                    TraceLoggingInt64Array(sizesArray, sizesArrayLength, "AllocatorByteSizeEntries"),
                    TraceLoggingUInt32Array(sizeEntriesCRCs, RecyclerSizeEntries::Count, "AllocatorByteSizeEntryNameCRCs"),

                    TraceLoggingInt64Array(startProcessingTimeMicros, passCount, "GCStartProcessingElapsedMicros"),
                    TraceLoggingInt64Array(endProcessingTimeMicros, passCount, "GCEndProcessingElapsedMicros"),
                    TraceLoggingInt64Array(bucketStatsProcessingTimeMicros, passCount, "GCBucketStatsProcessingElapsedMicros"),

                    TraceLoggingInt64Array(heapInfoUsedBytesArray, passCount, "HeapInfoUsedBytes"),
                    TraceLoggingInt64Array(heapInfoTotalBytes, passCount, "HeapInfoTotalBytes"),

                    TraceLoggingUInt32Array(startPassCollectionState, passCount, "startPassCollectionState"),
                    TraceLoggingUInt32Array(endPassCollectionState, passCount, "endPassCollectionState"),
                    TraceLoggingUInt32Array(collectionStartReason, passCount, "collectionStartReason"),
                    TraceLoggingUInt32Array(collectionFinishReason, passCount, "collectionFinishReason"),
                    TraceLoggingUInt32Array(collectionStartFlags, passCount, "collectionStartFlags"),

                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->numDecommitCalls, "ThreadPageAllocator_numDecommitCalls"),
                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->numPagesDecommitted, "ThreadPageAllocator_numPagesDecommitted"),
                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->numFreePageCount, "ThreadPageAllocator_numFreePageCount"),
                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->maxDeltaBetweenDecommitRegionLeaveAndDecommit.ToMicroseconds(), "ThreadPageAllocator_maxDeltaMicros"),
                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->lastEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "ThreadPageAllocator_lastEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->maxEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "ThreadPageAllocator_maxEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(info.GetThreadPageAllocator_decommitStats()->totalEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "ThreadPageAllocator_totalEnterLeaveIdleDecommitCSWaitTimeMicros"),

                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->numDecommitCalls,    "LeafPageAllocator_numDecommitCalls"),
                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->numPagesDecommitted, "LeafPageAllocator_numPagesDecommitted"),
                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->numFreePageCount, "LeafPageAllocator_numFreePageCount"),
                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->maxDeltaBetweenDecommitRegionLeaveAndDecommit.ToMicroseconds(), "LeafPageAllocator_maxDeltaMicros"),
                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->lastEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "LeafPageAllocator_lastEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->maxEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "LeafPageAllocator_maxEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(info.GetRecyclerLeafPageAllocator_decommitStats()->totalEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "LeafPageAllocator_totalEnterLeaveIdleDecommitCSWaitTimeMicros"),

                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->numDecommitCalls, "LargeBlockPageAllocator_numDecommitCalls"),
                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->numPagesDecommitted, "LargeBlockPageAllocator_numPagesDecommitted"),
                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->numFreePageCount, "LargeBlockPageAllocator_numFreePageCount"),
                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->maxDeltaBetweenDecommitRegionLeaveAndDecommit.ToMicroseconds(), "LargeBlockPageAllocator_maxDeltaMicros"),
                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->lastEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "LargeBlockPageAllocator_lastEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->maxEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "LargeBlockPageAllocator_maxEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(info.GetRecyclerLargeBlockPageAllocator_decommitStats()->totalEnterLeaveIdleDecommitCSWaitTime.ToMicroseconds(), "LargeBlockPageAllocator_totalEnterLeaveIdleDecommitCSWaitTimeMicros"),

                    TraceLoggingInt64(withBarrier_numDecommitCalls, "WithBarrierPageAllocator_numDecommitCalls"),
                    TraceLoggingInt64(withBarrier_numPagesDecommitted, "WithBarrierPageAllocator_numPagesDecommitted"),
                    TraceLoggingInt64(withBarrier_numFreePageCount, "WithBarrierPageAllocator_numFreePageCount"),
                    TraceLoggingInt64(withBarrier_maxDelta, "WithBarrierPageAllocator_maxDeltaMicros"),

                    TraceLoggingUInt32Array(pinnedObjectCountArray, passCount, "PinnedObjectCount"),
                    TraceLoggingUInt32Array(closedContextCountArray, passCount, "ClosedContextCount"),
                
                    TraceLoggingInt64(withBarrier_lastEnterLeaveIdleDecommitCSWaitTime, "WithBarrierPageAllocator_lastEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(withBarrier_maxEnterLeaveIdleDecommitCSWaitTime, "WithBarrierPageAllocator_maxEnterLeaveIdleDecommitCSWaitTimeMicros"),
                    TraceLoggingInt64(withBarrier_totalEnterLeaveIdleDecommitCSWaitTime, "WithBarrierPageAllocator_totalEnterLeaveIdleDecommitCSWaitTimeMicros")
                );

                sent = true;
            }

            //
            // free allcoations if made
            //
            if (passElapsedTimesMicros)          { HeapDeleteArray(passCount, passElapsedTimesMicros); }
            if (lastScriptExecutionTimes)        { HeapDeleteArray(passCount, lastScriptExecutionTimes); }
            if (passStartTimes)                  { HeapDeleteArray(passCount, passStartTimes); }
            if (isInScriptArray)                 { HeapDeleteArray(passCount, isInScriptArray); }
            if (isScriptActiveArray)             { HeapDeleteArray(passCount, isScriptActiveArray); }
            if (heapInfoUsedBytesArray)          { HeapDeleteArray(passCount, heapInfoUsedBytesArray); }
            if (heapInfoTotalBytes)              { HeapDeleteArray(passCount, heapInfoTotalBytes); }
            if (startProcessingTimeMicros)       { HeapDeleteArray(passCount, startProcessingTimeMicros); }
            if (endProcessingTimeMicros)         { HeapDeleteArray(passCount, endProcessingTimeMicros); }
            if (bucketStatsProcessingTimeMicros) { HeapDeleteArray(passCount, bucketStatsProcessingTimeMicros); }
            if (startPassCollectionState)        { HeapDeleteArray(passCount, startPassCollectionState); }
            if (endPassCollectionState)          { HeapDeleteArray(passCount, endPassCollectionState);}
            if (collectionStartReason)           { HeapDeleteArray(passCount, collectionStartReason);}
            if (collectionFinishReason)          { HeapDeleteArray(passCount, collectionFinishReason);}
            if (collectionStartFlags)            { HeapDeleteArray(passCount, collectionStartFlags);}

            if (pinnedObjectCountArray)          { HeapDeleteArray(passCount, pinnedObjectCountArray); }
            if (closedContextCountArray)         { HeapDeleteArray(passCount, closedContextCountArray); }

            if (uiThreadBlockedTimes)            { HeapDeleteArray(uiThreadBlockedTimesSize, uiThreadBlockedTimes); }
            if (sizesArray)                      { HeapDeleteArray(sizesArrayLength,         sizesArray); }
        }

        return sent;

    }
}
