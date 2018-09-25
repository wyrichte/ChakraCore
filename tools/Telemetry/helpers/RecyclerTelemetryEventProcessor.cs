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
    public class RecyclerTelemetryEventProcessor : Processor
    {

        /**
         *  This is a marker type that represents the deserialized JSON from the data column
         */
        class DeserializedJson
        {
#pragma warning disable CS0649
            public string recyclerID;
            public Int64 recyclerLifeSpanMicros;
            public Int64 microsSinceLastTransmit;
            public bool isConcurrentEnabled;
            public UInt32 recyclerConfigFlags;
            public Int64 passCount;
            public Int64[] passElapsedTimesMicros;
            public string[] passStartTimes;
            public string[] lastScriptExecutionTimes;
            public bool[] isInScript;
            public bool[] isScriptActive;
            public Int64[] UIThreadBlockedMicros;
            public Int64[] UIThreadBlockedCpuUserTimeMicros;
            public Int64[] UIThreadBlockedCpuKernelTimeMicros;
            public UInt64[] UIThreadBlockedNameCRCs;
            public Int64[] AllocatorByteSizeEntries;
            public UInt64[] AllocatorByteSizeEntryNameCRCs;
            public Int64[] GCStartProcessingElapsedMicros;
            public Int64[] GCEndProcessingElapsedMicros;
            public Int64[] GCBucketStatsProcessingElapsedMicros;
            public Int64[] HeapInfoUsedBytes;
            public Int64[] HeapInfoTotalBytes;
            public Int64[] PinnedObjectCount;
            public Int64[] ClosedContextCount;

            public Int64[] startPassCollectionState;
            public Int64[] endPassCollectionState;
            public Int64[] collectionStartReason;
            public Int64[] collectionFinishReason;
            public Int64[] collectionStartFlags;

            // new
            public Int64[] ThreadPageAllocator_DecommitStats;
            public Int64[] LeafPageAllocator_DecommitStats;
            public Int64[] LargeBlockPageAllocator_DecommitStats;
            public Int64[] WithBarrierPageAllocator_DecommitStats;

            // old
            public Int64 ThreadPageAllocator_numDecommitCalls;
            public Int64 ThreadPageAllocator_numPagesDecommitted;
            public Int64 ThreadPageAllocator_numFreePageCount;
            public Int64 ThreadPageAllocator_maxDeltaMicros;
            public Int64 LeafPageAllocator_numDecommitCalls;
            public Int64 LeafPageAllocator_numPagesDecommitted;
            public Int64 LeafPageAllocator_numFreePageCount;
            public Int64 LeafPageAllocator_maxDeltaMicros;
            public Int64 LargeBlockPageAllocator_numDecommitCalls;
            public Int64 LargeBlockPageAllocator_numPagesDecommitted;
            public Int64 LargeBlockPageAllocator_numFreePageCount;
            public Int64 LargeBlockPageAllocator_maxDeltaMicros;
            public Int64 WithBarrierPageAllocator_numDecommitCalls;
            public Int64 WithBarrierPageAllocator_numPagesDecommitted;
            public Int64 WithBarrierPageAllocator_numFreePageCount;
            public Int64 WithBarrierPageAllocator_maxDeltaMicros;
#pragma warning restore CS0649
        }

        string[] keepColumns = {
                "time",
                "binaryVersion",
                "binaryFlavor",
                "binaryArch",
                "chakraBuildCommit",
                "runType",
                "discriminator1",
                "discriminator2",
                "chakraInstanceID",
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

            output_schema.Add(new ColumnInfo("recyclerID", ColumnDataType.Guid));
            output_schema.Add(new ColumnInfo("transmitEventID", ColumnDataType.Guid));
            output_schema.Add(new ColumnInfo("recyclerLifeSpanMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("microsSinceLastTransmit", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("isConcurrentEnabled", ColumnDataType.Boolean));
            output_schema.Add(new ColumnInfo("isMemProtectMode", ColumnDataType.BooleanQ));
            output_schema.Add(new ColumnInfo("isStressMode", ColumnDataType.BooleanQ));
            output_schema.Add(new ColumnInfo("recyclerConfigFlags", ColumnDataType.UInt));
            output_schema.Add(new ColumnInfo("passCount", ColumnDataType.UInt));
            output_schema.Add(new ColumnInfo("passElapsedTimesMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("passStartTimes", typeof(string[])));
            output_schema.Add(new ColumnInfo("lastScriptExecutionTimes", typeof(string[])));
            output_schema.Add(new ColumnInfo("isInScript", typeof(bool[])));
            output_schema.Add(new ColumnInfo("isScriptActive", typeof(bool[])));
            output_schema.Add(new ColumnInfo("UIThreadBlockedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("UIThreadBlockedCpuUserTimeMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("UIThreadBlockedCpuKernelTimeMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("UIThreadBlockedNameCRCs", typeof(UInt64[])));
            output_schema.Add(new ColumnInfo("AllocatorByteSizeEntries", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("AllocatorByteSizeEntryNameCRCs", typeof(UInt64[])));
            output_schema.Add(new ColumnInfo("GCStartProcessingElapsedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("GCEndProcessingElapsedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("GCBucketStatsProcessingElapsedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("HeapInfoUsedBytes", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("HeapInfoTotalBytes", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("PinnedObjectCount", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("ClosedContextCount", typeof(Int64[])));

            output_schema.Add(new ColumnInfo("startPassCollectionState", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("endPassCollectionState", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("collectionStartReason", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("collectionFinishReason", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("collectionStartFlags", typeof(Int64[])));

            output_schema.Add(new ColumnInfo("ThreadPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));


            output_schema.Add(new ColumnInfo("LeafPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));

            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));

            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime", ColumnDataType.Long));

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
                DeserializedJson parsed = Newtonsoft.Json.JsonConvert.DeserializeObject<DeserializedJson>(input_row["data"].String);

                for (int i = 0; i < keepColumns.Length; i++)
                {
                    string cname = keepColumns[i];
                    input_row[cname].CopyTo(output_row[cname]);
                }

                output_row["recyclerID"].Set(MiscUtils.TryParseGuid(parsed.recyclerID));
                output_row["transmitEventID"].Set(Guid.NewGuid());
                output_row["recyclerLifeSpanMicros"].Set(parsed.recyclerLifeSpanMicros);
                output_row["microsSinceLastTransmit"].Set(parsed.microsSinceLastTransmit);
                output_row["recyclerConfigFlags"].Set(parsed.recyclerConfigFlags);

                // account for older versions that didn't have recyclerConfigFlags
                if (parsed.recyclerConfigFlags > 0)
                {
                    RecyclerFlagsTableSummary flags = (RecyclerFlagsTableSummary)parsed.recyclerConfigFlags;
                    bool isConcurrentEnabled = (flags & RecyclerFlagsTableSummary.IsConcurrentEnabled) == RecyclerFlagsTableSummary.IsConcurrentEnabled;
                    bool isMemProtectMode = (flags & RecyclerFlagsTableSummary.IsMemProtectMode) == RecyclerFlagsTableSummary.IsMemProtectMode;
                    bool isStressMode = (flags & RecyclerFlagsTableSummary.RecyclerStress & RecyclerFlagsTableSummary.RecyclerPartialStress & RecyclerFlagsTableSummary.RecyclerBackgroundStress & RecyclerFlagsTableSummary.RecyclerConcurrentStress & RecyclerFlagsTableSummary.RecyclerConcurrentRepeatStress) > 0;
                    output_row["isConcurrentEnabled"].Set(isConcurrentEnabled);
                    output_row["isMemProtectMode"].Set(isMemProtectMode);
                    output_row["isStressMode"].Set(isStressMode);
                }
                else
                {
                    output_row["isConcurrentEnabled"].Set(parsed.isConcurrentEnabled);
                    output_row["isMemProtectMode"].Set((bool?)null);
                    output_row["isStressMode"].Set((bool?)null);
                }

                output_row["passCount"].Set(parsed.passCount);
                output_row["passElapsedTimesMicros"].Set(parsed.passElapsedTimesMicros);
                output_row["passStartTimes"].Set(parsed.passStartTimes);
                output_row["lastScriptExecutionTimes"].Set(parsed.lastScriptExecutionTimes);
                output_row["isInScript"].Set(parsed.isInScript);
                output_row["isScriptActive"].Set(parsed.isScriptActive);
                output_row["UIThreadBlockedMicros"].Set(parsed.UIThreadBlockedMicros);
                output_row["UIThreadBlockedCpuUserTimeMicros"].Set(parsed.UIThreadBlockedCpuUserTimeMicros);
                output_row["UIThreadBlockedCpuKernelTimeMicros"].Set(parsed.UIThreadBlockedCpuUserTimeMicros);
                output_row["UIThreadBlockedNameCRCs"].Set(parsed.UIThreadBlockedNameCRCs);
                output_row["AllocatorByteSizeEntries"].Set(parsed.AllocatorByteSizeEntries);
                output_row["AllocatorByteSizeEntryNameCRCs"].Set(parsed.AllocatorByteSizeEntryNameCRCs);
                output_row["GCStartProcessingElapsedMicros"].Set(parsed.GCStartProcessingElapsedMicros);
                output_row["GCEndProcessingElapsedMicros"].Set(parsed.GCEndProcessingElapsedMicros);
                output_row["GCBucketStatsProcessingElapsedMicros"].Set(parsed.GCBucketStatsProcessingElapsedMicros);
                output_row["HeapInfoUsedBytes"].Set(parsed.HeapInfoUsedBytes);
                output_row["HeapInfoTotalBytes"].Set(parsed.HeapInfoTotalBytes);
                output_row["PinnedObjectCount"].Set(parsed.PinnedObjectCount);
                output_row["ClosedContextCount"].Set(parsed.ClosedContextCount);

                output_row["startPassCollectionState"].Set(parsed.startPassCollectionState);
                output_row["endPassCollectionState"].Set(parsed.endPassCollectionState);
                output_row["collectionStartReason"].Set(parsed.collectionStartReason);
                output_row["collectionFinishReason"].Set(parsed.collectionFinishReason);
                output_row["collectionStartFlags"].Set(parsed.collectionStartFlags);

                if (parsed.ThreadPageAllocator_DecommitStats != null && parsed.ThreadPageAllocator_DecommitStats.Length > 0)
                {
                    output_row["ThreadPageAllocator_numDecommitCalls"].Set(parsed.ThreadPageAllocator_DecommitStats[0]);
                    output_row["ThreadPageAllocator_numPagesDecommitted"].Set(parsed.ThreadPageAllocator_DecommitStats[1]);
                    output_row["ThreadPageAllocator_numFreePageCount"].Set(parsed.ThreadPageAllocator_DecommitStats[2]);
                    output_row["ThreadPageAllocator_maxDeltaMicros"].Set(parsed.ThreadPageAllocator_DecommitStats[3]);
                    output_row["ThreadPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.ThreadPageAllocator_DecommitStats[4]);
                    output_row["ThreadPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.ThreadPageAllocator_DecommitStats[5]);
                    output_row["ThreadPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.ThreadPageAllocator_DecommitStats[6]);

                    output_row["LeafPageAllocator_numDecommitCalls"].Set(parsed.LeafPageAllocator_DecommitStats[0]);
                    output_row["LeafPageAllocator_numPagesDecommitted"].Set(parsed.LeafPageAllocator_DecommitStats[1]);
                    output_row["LeafPageAllocator_numFreePageCount"].Set(parsed.LeafPageAllocator_DecommitStats[2]);
                    output_row["LeafPageAllocator_maxDeltaMicros"].Set(parsed.LeafPageAllocator_DecommitStats[3]);
                    output_row["LeafPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.LeafPageAllocator_DecommitStats[4]);
                    output_row["LeafPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.LeafPageAllocator_DecommitStats[5]);
                    output_row["LeafPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.LeafPageAllocator_DecommitStats[6]);

                    output_row["LargeBlockPageAllocator_numDecommitCalls"].Set(parsed.LargeBlockPageAllocator_DecommitStats[0]);
                    output_row["LargeBlockPageAllocator_numPagesDecommitted"].Set(parsed.LargeBlockPageAllocator_DecommitStats[1]);
                    output_row["LargeBlockPageAllocator_numFreePageCount"].Set(parsed.LargeBlockPageAllocator_DecommitStats[2]);
                    output_row["LargeBlockPageAllocator_maxDeltaMicros"].Set(parsed.LargeBlockPageAllocator_DecommitStats[3]);
                    output_row["LargeBlockPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.LargeBlockPageAllocator_DecommitStats[4]);
                    output_row["LargeBlockPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.LargeBlockPageAllocator_DecommitStats[5]);
                    output_row["LargeBlockPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.LargeBlockPageAllocator_DecommitStats[6]);

                    output_row["WithBarrierPageAllocator_numDecommitCalls"].Set(parsed.WithBarrierPageAllocator_DecommitStats[0]);
                    output_row["WithBarrierPageAllocator_numPagesDecommitted"].Set(parsed.WithBarrierPageAllocator_DecommitStats[1]);
                    output_row["WithBarrierPageAllocator_numFreePageCount"].Set(parsed.WithBarrierPageAllocator_DecommitStats[2]);
                    output_row["WithBarrierPageAllocator_maxDeltaMicros"].Set(parsed.WithBarrierPageAllocator_DecommitStats[3]);
                    output_row["WithBarrierPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.WithBarrierPageAllocator_DecommitStats[4]);
                    output_row["WithBarrierPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.WithBarrierPageAllocator_DecommitStats[5]);
                    output_row["WithBarrierPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(parsed.WithBarrierPageAllocator_DecommitStats[6]);

                }
                else
                {
                    output_row["ThreadPageAllocator_numDecommitCalls"].Set(parsed.ThreadPageAllocator_numDecommitCalls);
                    output_row["ThreadPageAllocator_numPagesDecommitted"].Set(parsed.ThreadPageAllocator_numPagesDecommitted);
                    output_row["ThreadPageAllocator_numFreePageCount"].Set(parsed.ThreadPageAllocator_numFreePageCount);
                    output_row["ThreadPageAllocator_maxDeltaMicros"].Set(parsed.ThreadPageAllocator_maxDeltaMicros);
                    output_row["ThreadPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(-1);// these values didn't exist in old payloads
                    output_row["ThreadPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["ThreadPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(-1);

                    output_row["LeafPageAllocator_numDecommitCalls"].Set(parsed.LeafPageAllocator_numDecommitCalls);
                    output_row["LeafPageAllocator_numPagesDecommitted"].Set(parsed.LeafPageAllocator_numPagesDecommitted);
                    output_row["LeafPageAllocator_numFreePageCount"].Set(parsed.LeafPageAllocator_numFreePageCount);
                    output_row["LeafPageAllocator_maxDeltaMicros"].Set(parsed.LeafPageAllocator_maxDeltaMicros);
                    output_row["LeafPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["LeafPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["LeafPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(-1);

                    output_row["LargeBlockPageAllocator_numDecommitCalls"].Set(parsed.LargeBlockPageAllocator_numDecommitCalls);
                    output_row["LargeBlockPageAllocator_numPagesDecommitted"].Set(parsed.LargeBlockPageAllocator_numPagesDecommitted);
                    output_row["LargeBlockPageAllocator_numFreePageCount"].Set(parsed.LargeBlockPageAllocator_numFreePageCount);
                    output_row["LargeBlockPageAllocator_maxDeltaMicros"].Set(parsed.LargeBlockPageAllocator_maxDeltaMicros);
                    output_row["LargeBlockPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["LargeBlockPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["LargeBlockPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(-1);

                    output_row["WithBarrierPageAllocator_numDecommitCalls"].Set(parsed.WithBarrierPageAllocator_numDecommitCalls);
                    output_row["WithBarrierPageAllocator_numPagesDecommitted"].Set(parsed.WithBarrierPageAllocator_numPagesDecommitted);
                    output_row["WithBarrierPageAllocator_numFreePageCount"].Set(parsed.WithBarrierPageAllocator_numFreePageCount);
                    output_row["WithBarrierPageAllocator_maxDeltaMicros"].Set(parsed.WithBarrierPageAllocator_maxDeltaMicros);
                    output_row["WithBarrierPageAllocator_lastEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["WithBarrierPageAllocator_maxEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                    output_row["WithBarrierPageAllocator_totalEnterLeaveIdleDecommitCSWaitTime"].Set(-1);
                }

                yield return output_row;
            }
        }

        [Flags]
        enum RecyclerFlagsTableSummary : UInt32
        {
            None = 0x0000,
            IsMemProtectMode = 0x0001,
            IsConcurrentEnabled = 0x0002,
            EnableScanInteriorPointers = 0x0004,
            EnableScanImplicitRoots = 0x0008,
            DisableCollectOnAllocationHeuristics = 0x0016,
            RecyclerStress = 0x0032,
            RecyclerBackgroundStress = 0x0064,
            RecyclerConcurrentStress = 0x0128,
            RecyclerConcurrentRepeatStress = 0x0256,
            RecyclerPartialStress = 0x0512,
        };

    }
}
