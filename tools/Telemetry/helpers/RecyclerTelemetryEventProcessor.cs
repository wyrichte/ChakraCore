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
            public Int64 passCount;
            public Int64[] passElapsedTimeMicros;
            public string[] passStartTimes;
            public string[] lastScriptExecutionTimes;
            public bool[] isInScript;
            public bool[] isScriptActive;
            public Int64[] UIThreadBlockedMicros;
            public Int64[] UIThreadBlockedNameCRCs;
            public Int64[] AllocatorByteSizeEntries;
            public Int64[] AllocatorByteSizeEntryNameCRCs;
            public Int64[] GCStartProcessingElapsedMicros;
            public Int64[] GCEndProcessingElapsedMicros;
            public Int64[] GCBucketStatsProcessingElapsedMicros;
            public Int64[] HeapInfoUsedBytes;
            public Int64[] HeapInfoTotalBytes;
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
            output_schema.Add(new ColumnInfo("passCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("passElapsedTimeMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("passStartTimes", typeof(string[])));
            output_schema.Add(new ColumnInfo("lastScriptExecutionTimes", typeof(string[])));
            output_schema.Add(new ColumnInfo("isInScript", typeof(bool[])));
            output_schema.Add(new ColumnInfo("isScriptActive", typeof(bool[])));
            output_schema.Add(new ColumnInfo("UIThreadBlockedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("UIThreadBlockedNameCRCs", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("AllocatorByteSizeEntries", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("AllocatorByteSizeEntryNameCRCs", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("GCStartProcessingElapsedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("GCEndProcessingElapsedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("GCBucketStatsProcessingElapsedMicros", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("HeapInfoUsedBytes", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("HeapInfoTotalBytes", typeof(Int64[])));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("ThreadPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LeafPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("LargeBlockPageAllocator_maxDeltaMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_numDecommitCalls", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_numPagesDecommitted", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_numFreePageCount", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("WithBarrierPageAllocator_maxDeltaMicros", ColumnDataType.Long));

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
                output_row["passCount"].Set(parsed.passCount);
                output_row["passElapsedTimeMicros"].Set(parsed.passElapsedTimeMicros);
                output_row["passStartTimes"].Set(parsed.passStartTimes);
                output_row["lastScriptExecutionTimes"].Set(parsed.lastScriptExecutionTimes);
                output_row["isInScript"].Set(parsed.isInScript);
                output_row["isScriptActive"].Set(parsed.isScriptActive);
                output_row["UIThreadBlockedMicros"].Set(parsed.UIThreadBlockedMicros);
                output_row["UIThreadBlockedNameCRCs"].Set(parsed.UIThreadBlockedNameCRCs);
                output_row["AllocatorByteSizeEntries"].Set(parsed.AllocatorByteSizeEntries);
                output_row["AllocatorByteSizeEntryNameCRCs"].Set(parsed.AllocatorByteSizeEntryNameCRCs);
                output_row["GCStartProcessingElapsedMicros"].Set(parsed.GCStartProcessingElapsedMicros);
                output_row["GCEndProcessingElapsedMicros"].Set(parsed.GCEndProcessingElapsedMicros);
                output_row["GCBucketStatsProcessingElapsedMicros"].Set(parsed.GCBucketStatsProcessingElapsedMicros);
                output_row["HeapInfoUsedBytes"].Set(parsed.HeapInfoUsedBytes);
                output_row["HeapInfoTotalBytes"].Set(parsed.HeapInfoTotalBytes);
                output_row["ThreadPageAllocator_numDecommitCalls"].Set(parsed.ThreadPageAllocator_numDecommitCalls);
                output_row["ThreadPageAllocator_numPagesDecommitted"].Set(parsed.ThreadPageAllocator_numPagesDecommitted);
                output_row["ThreadPageAllocator_numFreePageCount"].Set(parsed.ThreadPageAllocator_numFreePageCount);
                output_row["ThreadPageAllocator_maxDeltaMicros"].Set(parsed.ThreadPageAllocator_maxDeltaMicros);
                output_row["LeafPageAllocator_numDecommitCalls"].Set(parsed.LeafPageAllocator_numDecommitCalls);
                output_row["LeafPageAllocator_numPagesDecommitted"].Set(parsed.LeafPageAllocator_numPagesDecommitted);
                output_row["LeafPageAllocator_numFreePageCount"].Set(parsed.LeafPageAllocator_numFreePageCount);
                output_row["LeafPageAllocator_maxDeltaMicros"].Set(parsed.LeafPageAllocator_maxDeltaMicros);
                output_row["LargeBlockPageAllocator_numDecommitCalls"].Set(parsed.LargeBlockPageAllocator_numDecommitCalls);
                output_row["LargeBlockPageAllocator_numPagesDecommitted"].Set(parsed.LargeBlockPageAllocator_numPagesDecommitted);
                output_row["LargeBlockPageAllocator_numFreePageCount"].Set(parsed.LargeBlockPageAllocator_numFreePageCount);
                output_row["LargeBlockPageAllocator_maxDeltaMicros"].Set(parsed.LargeBlockPageAllocator_maxDeltaMicros);
                output_row["WithBarrierPageAllocator_numDecommitCalls"].Set(parsed.WithBarrierPageAllocator_numDecommitCalls);
                output_row["WithBarrierPageAllocator_numPagesDecommitted"].Set(parsed.WithBarrierPageAllocator_numPagesDecommitted);
                output_row["WithBarrierPageAllocator_numFreePageCount"].Set(parsed.WithBarrierPageAllocator_numFreePageCount);
                output_row["WithBarrierPageAllocator_maxDeltaMicros"].Set(parsed.WithBarrierPageAllocator_maxDeltaMicros);

                yield return output_row;
            }
        }

    }
}
