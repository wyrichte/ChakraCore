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

                long passCount = input_row["passCount"].Long;
                Int64[] passElapsedTimeMicros = input_row["passElapsedTimeMicros"].Value as Int64[];
                string[] passStartTimes = input_row["passStartTimes"].Value as string[];
                string[] lastScriptExecutionTimes = input_row["lastScriptExecutionTimes"].Value as string[];
                bool[] isInScript = input_row["isInScript"].Value as bool[];
                bool[] isScriptActive = input_row["isScriptActive"].Value as bool[];
                Int64[] UIThreadBlockedMicros = input_row["UIThreadBlockedMicros"].Value as Int64[];
                Int64[] UIThreadBlockedNameCRCs = input_row["UIThreadBlockedNameCRCs"].Value as Int64[];
                Int64[] AllocatorByteSizeEntries = input_row["AllocatorByteSizeEntries"].Value as Int64[];
                Int64[] AllocatorByteSizeEntryNameCRCs = input_row["AllocatorByteSizeEntryNameCRCs"].Value as Int64[];
                Int64[] GCStartProcessingElapsedMicros = input_row["GCStartProcessingElapsedMicros"].Value as Int64[];
                Int64[] GCEndProcessingElapsedMicros = input_row["GCEndProcessingElapsedMicros"].Value as Int64[];
                Int64[] GCBucketStatsProcessingElapsedMicros = input_row["GCBucketStatsProcessingElapsedMicros"].Value as Int64[];
                Int64[] HeapInfoUsedBytes = input_row["HeapInfoUsedBytes"].Value as Int64[];
                Int64[] HeapInfoTotalBytes = input_row["HeapInfoTotalBytes"].Value as Int64[];

                for (int pass = 0; pass < passCount; pass++)
                {
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

                    yield return output_row;
                }
            }
        }
        private void SafeSet<T>(string columnName, T[] values, Row outputRow, int currentIndex)
        {
            if (values != null && currentIndex < values.Length)
            {
                outputRow[columnName].Set(values[currentIndex]);
            }
        }
    }

}
