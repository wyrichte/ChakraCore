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
        }
    }

}
