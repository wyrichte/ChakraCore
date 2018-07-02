using ScopeRuntime;
using System;
using System.Collections.Generic;
using System.Linq;

//
// cosmos nuget info
//   - https://microsoft.sharepoint.com/teams/Cosmos/Wiki/Cosmos%20NuGet%20Packages.aspx
//   - https://mscosmos.visualstudio.com/CosmosPackages/_packaging?feed=CosmosProd&_a=feed
//
//    - Add the cosmos nuget feed
//         nuget.exe sources Add -Name "CosmosProd" -Source "https://mscosmos.pkgs.visualstudio.com/_packaging/CosmosProd/nuget/v3/index.json"
//    -    Install-Package Microsoft.Cosmos.ScopeSDK -version 1.201803061.1
//
//
namespace Chakra.Utils
{
    public class RecyclerTelemetryGCPassProcessor : Processor
    {

        string[] keepColumns = {
                "time",
                "binaryVersion",
                "binaryFlavor",
                "binaryArch",
                "chakraBuildCommit",
                "runType",
                "discriminator1",
                "discriminator2",
                "recyclerID",
                "transmitEventID",
                "recyclerLifeSpanMicros",
                "data",
            };

        public override Schema Produces(string[] requested_columns, string[] args, Schema input_schema)
        {
            Schema output_schema = new Schema();

            foreach (ColumnInfo ci in input_schema.Columns)
            {
                if (keepColumns.Contains(ci.Name))
                {
                    ColumnInfo ci2 = ci.Clone();
                    // setting source indicates column is a "pass-through" column. 
                    // improves perf of cosmos' optimizers
                    ci2.Source = ci;
                    output_schema.Add(ci2);
                }
            }

            output_schema.Add(new ColumnInfo("passNumber", ColumnDataType.UInt));
            output_schema.Add(new ColumnInfo("passElapsedTimeMicros", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("passStartTimes", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("lastScriptExecutionTimes", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("isInScript", ColumnDataType.BooleanQ));
            output_schema.Add(new ColumnInfo("isScriptActive", ColumnDataType.BooleanQ));
            //output_schema.Add(new ColumnInfo("UIThreadBlockedMicros", ColumnDataType.LongQ));
            //output_schema.Add(new ColumnInfo("UIThreadBlockedNameCRCs", ColumnDataType.LongQ));
            //output_schema.Add(new ColumnInfo("AllocatorByteSizeEntries", ColumnDataType.LongQ));
            //output_schema.Add(new ColumnInfo("AllocatorByteSizeEntryNameCRCs", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("GCStartProcessingElapsedMicros", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("GCEndProcessingElapsedMicros", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("GCBucketStatsProcessingElapsedMicros", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("HeapInfoUsedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("HeapInfoTotalBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("PinnedObjectCount", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("ClosedContextCount", ColumnDataType.LongQ));

            output_schema.Add(new ColumnInfo("startPassCollectionState_val", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("endPassCollectionState_val", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("collectionStartReason_val", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("collectionFinishReason_val", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("collectionStartFlags_val", ColumnDataType.LongQ));

            output_schema.Add(new ColumnInfo("startPassCollectionState", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("endPassCollectionState", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("collectionStartReason", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("collectionFinishReason", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("collectionStartFlags", ColumnDataType.String));


            // values transmitted from via the AllocatorByteSizeEntries array.  We assume these will stay the same, so we'll 
            // add their values inline in the GC Pass Row, instead of as name/value pairs
            output_schema.Add(new ColumnInfo("processAllocaterUsedBytes_start", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("processAllocaterUsedBytes_end", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("processCommittedBytes_start", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("processCommittedBytes_end", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_start_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_start_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_start_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_start_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_end_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_end_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_end_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("threadPageAllocator_end_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_start_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_start_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_start_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_start_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_end_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_end_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_end_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLeafPageAllocator_end_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_start_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_start_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_start_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_start_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_end_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_end_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_end_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerLargeBlockPageAllocator_end_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_start_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_start_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_start_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_start_numberOfSegments", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_end_committedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_end_usedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_end_reservedBytes", ColumnDataType.LongQ));
            output_schema.Add(new ColumnInfo("recyclerWithBarrierPageAllocator_end_numberOfSegments", ColumnDataType.LongQ));

            return output_schema;
        }

        /**
         *  We send telemetry from Chakra that includes an array of string hashes, and an array counter values for each of those hashes.  
         *  We do this for a number of reasons, including size of data sent & to work around constaints in how many property fields we 
         *  can send through windows telemetry APIs. 
         *  
         *  This processor will "split" these arrays into a number of new rows with name/value pair data.  
         */
        public override IEnumerable<Row> Process(RowSet input_rowset, Row output_row, string[] args)
        {
            foreach (Row input_row in input_rowset.Rows)
            {
                for (int i = 0; i < keepColumns.Length; i++)
                {
                    string cname = keepColumns[i];
                    input_row[cname].CopyTo(output_row[cname]);
                }

                uint passCount = input_row["passCount"].UInt;
                Int64[] passElapsedTimeMicros = input_row["passElapsedTimesMicros"].Value as Int64[];
                string[] passStartTimes = input_row["passStartTimes"].Value as string[];
                string[] lastScriptExecutionTimes = input_row["lastScriptExecutionTimes"].Value as string[];
                bool[] isInScript = input_row["isInScript"].Value as bool[];
                bool[] isScriptActive = input_row["isScriptActive"].Value as bool[];
                //Int64[] UIThreadBlockedMicros = input_row["UIThreadBlockedMicros"].Value as Int64[];
                //Int64[] UIThreadBlockedNameCRCs = input_row["UIThreadBlockedNameCRCs"].Value as Int64[];
                Int64[] AllocatorByteSizeEntries = input_row["AllocatorByteSizeEntries"].Value as Int64[];
                UInt64[] AllocatorByteSizeEntryNameCRCs = input_row["AllocatorByteSizeEntryNameCRCs"].Value as UInt64[];
                Int64[] GCStartProcessingElapsedMicros = input_row["GCStartProcessingElapsedMicros"].Value as Int64[];
                Int64[] GCEndProcessingElapsedMicros = input_row["GCEndProcessingElapsedMicros"].Value as Int64[];
                Int64[] GCBucketStatsProcessingElapsedMicros = input_row["GCBucketStatsProcessingElapsedMicros"].Value as Int64[];
                Int64[] HeapInfoUsedBytes = input_row["HeapInfoUsedBytes"].Value as Int64[];
                Int64[] HeapInfoTotalBytes = input_row["HeapInfoTotalBytes"].Value as Int64[];
                Int64[] PinnedObjectCount = input_row["PinnedObjectCount"].Value as Int64[];
                Int64[] ClosedContextCount = input_row["ClosedContextCount"].Value as Int64[];

                Int64[] startPassCollectionState = input_row["startPassCollectionState"].Value as Int64[];
                Int64[] endPassCollectionState = input_row["endPassCollectionState"].Value as Int64[];
                Int64[] collectionStartReason = input_row["collectionStartReason"].Value as Int64[];
                Int64[] collectionFinishReason = input_row["collectionFinishReason"].Value as Int64[];
                Int64[] collectionStartFlags = input_row["collectionStartFlags"].Value as Int64[];

                uint currentPass = passCount;
                for (uint pass = 0; pass < passCount; pass++)
                {
                    // "GC Pass" data is transmitted last pass first, so account for that here
                    output_row["passNumber"].Set(currentPass--);

                    SafeSet("passElapsedTimeMicros", passElapsedTimeMicros, output_row, pass);
                    SafeSet("passStartTimes", passStartTimes, output_row, pass);
                    SafeSet("lastScriptExecutionTimes", lastScriptExecutionTimes, output_row, pass);
                    SafeSet("isInScript", isInScript, output_row, pass);
                    SafeSet("isScriptActive", isScriptActive, output_row, pass);
                    //SafeSet("UIThreadBlockedMicros",  UIThreadBlockedMicros                , output_row, pass); 
                    //SafeSet("UIThreadBlockedNameCRCs",  UIThreadBlockedNameCRCs              , output_row, pass); 
                    //SafeSet("AllocatorByteSizeEntries",  AllocatorByteSizeEntries             , output_row, pass); 
                    //SafeSet("AllocatorByteSizeEntryNameCRCs",  AllocatorByteSizeEntryNameCRCs       , output_row, pass); 
                    SafeSet("GCStartProcessingElapsedMicros", GCStartProcessingElapsedMicros, output_row, pass);
                    SafeSet("GCEndProcessingElapsedMicros", GCEndProcessingElapsedMicros, output_row, pass);
                    SafeSet("GCBucketStatsProcessingElapsedMicros", GCBucketStatsProcessingElapsedMicros, output_row, pass);
                    SafeSet("HeapInfoUsedBytes", HeapInfoUsedBytes, output_row, pass);
                    SafeSet("HeapInfoTotalBytes", HeapInfoTotalBytes, output_row, pass);
                    SafeSet("PinnedObjectCount", PinnedObjectCount, output_row, pass);
                    SafeSet("ClosedContextCount", ClosedContextCount, output_row, pass);

                    SafeSet("startPassCollectionState_val", startPassCollectionState, output_row, pass);
                    SafeSet("endPassCollectionState_val", endPassCollectionState, output_row, pass);
                    SafeSet("collectionStartReason_val", collectionStartReason, output_row, pass);
                    SafeSet("collectionFinishReason_val", collectionFinishReason, output_row, pass);
                    SafeSet("collectionStartFlags_val", collectionStartFlags, output_row, pass);

                    // add columns that conver integer values to the string enums.
                    SafeSet("startPassCollectionState", startPassCollectionState, output_row, pass, CollectionStateToString);
                    SafeSet("endPassCollectionState", endPassCollectionState, output_row, pass, CollectionStateToString);
                    SafeSet("collectionStartReason", collectionStartReason, output_row, pass, GCActivationTriggerToString);
                    SafeSet("collectionFinishReason", collectionFinishReason, output_row, pass,GCActivationTriggerToString);
                    SafeSet("collectionStartFlags", collectionStartFlags, output_row, pass, CollectionFlagsToString);

                    // we're transmitting these as name/value pairs (i.e., an array of "name CRCs" and a corresponding array of values. 
                    // however, we don't expect these to change frequently (thus invalidating schemas), so just put them inline
                    AddAllocatorByteSizeEntries(output_row, pass, AllocatorByteSizeEntries, AllocatorByteSizeEntryNameCRCs);

                    yield return output_row;
                }
            }
        }

        private void AddAllocatorByteSizeEntries(Row outputRow, uint currentPass, Int64[] allocatorByteSizeEntries, UInt64[] allocatorByteSizeEntryNameCRCs)
        {
            for (int i = 0; i < allocatorByteSizeEntryNameCRCs.Length; i++)
            {
                long idx = (currentPass * allocatorByteSizeEntryNameCRCs.Length) + i;
                string columnName = DeCRC.GetStringForCRC(allocatorByteSizeEntryNameCRCs[i]);
                outputRow[columnName].Set(allocatorByteSizeEntries[idx]);
            }
        }

        private void SafeSet<T>(string columnName, T[] values, Row outputRow, uint currentIndex)
        {
            if (values != null && currentIndex < values.Length)
            {
                outputRow[columnName].Set(values[currentIndex]);
            }
            else
            {
                outputRow[columnName].Set(default(T));
            }
        }

        private void SafeSet<T, TResult>(string columnName, T[] values, Row outputRow, uint currentIndex, Func<T, TResult> mapFunction)
        {
            if (values != null && currentIndex < values.Length)
            {
                outputRow[columnName].Set(mapFunction(values[currentIndex]));
            }
            else
            {
                outputRow[columnName].Set(default(TResult));
            }
        }

        // from $/chakra/core/lib/Common/Memory/CollectionFlags.h
        enum ETWEventGCActivationTrigger
        {
            ETWEvent_GC_Trigger_Unknown = 0,
            ETWEvent_GC_Trigger_IdleCollect = 1,
            ETWEvent_GC_Trigger_Partial_GC_AllocSize_Heuristic = 2,
            ETWEvent_GC_Trigger_TimeAndAllocSize_Heuristic = 3,
            ETWEvent_GC_Trigger_TimeAndAllocSizeIfScriptActive_Heuristic = 4,
            ETWEvent_GC_Trigger_TimeAndAllocSizeIfInScript_Heuristic = 5,
            ETWEvent_GC_Trigger_NoHeuristic = 6,
            ETWEvent_GC_Trigger_Status_Completed = 7,
            ETWEvent_GC_Trigger_Status_StartedConcurrent = 8,
            ETWEvent_GC_Trigger_Status_Failed = 9,
            ETWEvent_GC_Trigger_Status_FailedTimeout = 10
        };

        public static string GCActivationTriggerToString(Int64 val)
        {
            var trigger = (ETWEventGCActivationTrigger)val;
            return trigger.ToString();
        }

        // from $/chakra/core/lib/Common/Memory/CollectionState.h
        enum CollectionState
        {
            Collection_Mark = 0x00000001,
            Collection_Sweep = 0x00000002,
            Collection_Exit = 0x00000004,
            Collection_PreCollection = 0x00000008,

            // Mark related states
            Collection_ResetMarks = 0x00000010,
            Collection_FindRoots = 0x00000020,
            Collection_Rescan = 0x00000040,
            Collection_FinishMark = 0x00000080,

            // Sweep related states
            //#if ENABLE_CONCURRENT_GC
            Collection_ConcurrentSweepSetup = 0x00000100,
            //#endif
            Collection_TransferSwept = 0x00000200,

            // State attributes
            //#if ENABLE_PARTIAL_GC
            Collection_Partial = 0x00001000,
            //#endif
            //#if ENABLE_CONCURRENT_GC
            Collection_Concurrent = 0x00002000,
            Collection_ExecutingConcurrent = 0x00004000,
            Collection_FinishConcurrent = 0x00008000,

            Collection_ConcurrentMark = Collection_Concurrent | Collection_Mark,
            Collection_ConcurrentSweep = Collection_Concurrent | Collection_Sweep,
            //#endif
            Collection_Parallel = 0x00010000,

            Collection_PostCollectionCallback = 0x00020000,
            Collection_PostSweepRedeferralCallback = 0x00040000,
            Collection_WrapperCallback = 0x00080000,

            // EXTRAS - not in enum but in our telemetry data
            //  24593, 0x6011
            //     17, 0x11
            EXTRA_ResetMarks_Mark = Collection_ResetMarks | Collection_Mark,
            EXTRA_ExecutingConcurrent_Concurrent_ResetMarks_Mark = Collection_ExecutingConcurrent | Collection_Concurrent | Collection_ResetMarks | Collection_Mark,
        }

        public static string CollectionStateToString(Int64 val)
        {
            CollectionState state = (CollectionState)val;
            return state.ToString();
        }

        // from $/chakra/core/lib/Common/Memory/CollectionFlags.h
        enum CollectionFlags : Int64
        {
            CollectHeuristic_AllocSize = 0x00000001,
            CollectHeuristic_Time = 0x00000002,
            CollectHeuristic_TimeIfScriptActive = 0x00000004,
            CollectHeuristic_TimeIfInScript = 0x00000008,
            CollectHeuristic_Never = 0x00000080,
            CollectHeuristic_Mask = 0x000000FF,

            CollectOverride_FinishConcurrent = 0x00001000,
            CollectOverride_ExhaustiveCandidate = 0x00002000,
            CollectOverride_ForceInThread = 0x00004000,
            CollectOverride_AllowDispose = 0x00008000,
            CollectOverride_AllowReentrant = 0x00010000,
            CollectOverride_ForceFinish = 0x00020000,
            CollectOverride_Explicit = 0x00040000,
            CollectOverride_DisableIdleFinish = 0x00080000,
            CollectOverride_BackgroundFinishMark = 0x00100000,
            CollectOverride_FinishConcurrentTimeout = 0x00200000,
            CollectOverride_NoExhaustiveCollect = 0x00400000,
            CollectOverride_SkipStack = 0x01000000,
            CollectOverride_CheckScriptContextClose = 0x02000000,
            CollectMode_Partial = 0x08000000,
            CollectMode_Concurrent = 0x10000000,
            CollectMode_Exhaustive = 0x20000000,
            CollectMode_DecommitNow = 0x40000000,
            CollectMode_CacheCleanup = 0x80000000,

            CollectNowForceInThread = CollectOverride_ForceInThread,
            CollectNowForceInThreadExternal = CollectOverride_ForceInThread | CollectOverride_AllowDispose,
            CollectNowForceInThreadExternalNoStack = CollectOverride_ForceInThread | CollectOverride_AllowDispose | CollectOverride_SkipStack,
            CollectNowDefault = CollectOverride_FinishConcurrent,
            CollectNowDefaultLSCleanup = CollectOverride_FinishConcurrent | CollectOverride_AllowDispose,
            CollectNowDecommitNowExplicit = CollectNowDefault | CollectMode_DecommitNow | CollectMode_CacheCleanup | CollectOverride_Explicit | CollectOverride_AllowDispose,
            CollectNowConcurrent = CollectOverride_FinishConcurrent | CollectMode_Concurrent,
            CollectNowExhaustive = CollectOverride_FinishConcurrent | CollectMode_Exhaustive | CollectOverride_AllowDispose,
            CollectNowPartial = CollectOverride_FinishConcurrent | CollectMode_Partial,
            CollectNowConcurrentPartial = CollectMode_Concurrent | CollectNowPartial,

            CollectOnAllocation = CollectHeuristic_AllocSize | CollectHeuristic_Time | CollectMode_Concurrent | CollectMode_Partial | CollectOverride_FinishConcurrent | CollectOverride_AllowReentrant | CollectOverride_FinishConcurrentTimeout,
            CollectOnTypedArrayAllocation = CollectHeuristic_AllocSize | CollectHeuristic_Time | CollectMode_Concurrent | CollectMode_Partial | CollectOverride_FinishConcurrent | CollectOverride_AllowReentrant | CollectOverride_FinishConcurrentTimeout | CollectOverride_AllowDispose,
            CollectOnScriptIdle = CollectOverride_CheckScriptContextClose | CollectOverride_FinishConcurrent | CollectMode_Concurrent | CollectMode_CacheCleanup | CollectOverride_SkipStack,
            CollectOnScriptExit = CollectOverride_CheckScriptContextClose | CollectHeuristic_AllocSize | CollectOverride_FinishConcurrent | CollectMode_Concurrent | CollectMode_CacheCleanup,
            CollectExhaustiveCandidate = CollectHeuristic_Never | CollectOverride_ExhaustiveCandidate,
            CollectOnScriptCloseNonPrimary = CollectNowConcurrent | CollectOverride_ExhaustiveCandidate | CollectOverride_AllowDispose,
            CollectOnRecoverFromOutOfMemory = CollectOverride_ForceInThread | CollectMode_DecommitNow,
            CollectOnSuspendCleanup = CollectNowConcurrent | CollectMode_Exhaustive | CollectMode_DecommitNow | CollectOverride_DisableIdleFinish,

            FinishConcurrentOnIdle = CollectMode_Concurrent | CollectOverride_DisableIdleFinish,
            FinishConcurrentOnIdleAtRoot = CollectMode_Concurrent | CollectOverride_DisableIdleFinish | CollectOverride_SkipStack,
            FinishConcurrentDefault = CollectMode_Concurrent | CollectOverride_DisableIdleFinish | CollectOverride_BackgroundFinishMark,
            FinishConcurrentOnExitScript = FinishConcurrentDefault,
            FinishConcurrentOnEnterScript = FinishConcurrentDefault,
            FinishConcurrentOnAllocation = FinishConcurrentDefault,
            FinishDispose = CollectOverride_AllowDispose,
            FinishDisposeTimed = CollectOverride_AllowDispose | CollectHeuristic_TimeIfScriptActive,
            ForceFinishCollection = CollectOverride_ForceFinish | CollectOverride_ForceInThread,

            //# ifdef RECYCLER_STRESS
            CollectStress = CollectNowForceInThread,
            //#if ENABLE_PARTIAL_GC
            CollectPartialStress = CollectMode_Partial,
            //#endif
            //#if ENABLE_CONCURRENT_GC
            CollectBackgroundStress = CollectNowDefault,
            CollectConcurrentStress = CollectNowConcurrent,
            //#if ENABLE_PARTIAL_GC
            CollectConcurrentPartialStress = CollectConcurrentStress | CollectPartialStress,
            //#endif
            //#endif
            //#endif

            //#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
            CollectNowFinalGC = CollectNowExhaustive | CollectOverride_ForceInThread | CollectOverride_SkipStack | CollectOverride_Explicit | CollectOverride_AllowDispose,
            //#endif
            //# ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            CollectNowExhaustiveSkipStack = CollectNowExhaustive | CollectOverride_SkipStack, // Used by test
                                                                                              //#endif

        };

        public static string CollectionFlagsToString(Int64 val)
        {
            CollectionFlags flags = (CollectionFlags)val;
            return flags.ToString();
        }

    }

}
