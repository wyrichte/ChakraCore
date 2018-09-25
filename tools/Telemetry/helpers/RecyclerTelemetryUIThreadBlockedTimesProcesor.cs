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
    public class RecyclerTelemetryUIThreadBlockedTimesProcesor : Processor
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
                "chakraInstanceID",
                "recyclerID",
                "transmitEventID",
                "recyclerLifeSpanMicros",
                "passCount"
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

            output_schema.Add(new ColumnInfo("propName", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("UIThreadBlockedMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("UIThreadBlockedCpuUserTimeMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("UIThreadBlockedCpuKernelTimeMicros", ColumnDataType.Long));
            output_schema.Add(new ColumnInfo("passNumber", ColumnDataType.UInt));

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

                Int64[] UIThreadBlockedMicros = input_row["UIThreadBlockedMicros"].Value as Int64[];
                Int64[] UIThreadBlockedCpuUserTimeMicros = input_row["UIThreadBlockedCpuUserTimeMicros"].Value as Int64[];
                Int64[] UIThreadBlockedCpuKernelTimeMicros = input_row["UIThreadBlockedCpuKernelTimeMicros"].Value as Int64[];
                UInt64[] UIThreadBlockedNameCRCs = input_row["UIThreadBlockedNameCRCs"].Value as UInt64[];

                // ensure we support builds before CPU times were present
                if (UIThreadBlockedCpuUserTimeMicros == null) { UIThreadBlockedCpuUserTimeMicros = new long[UIThreadBlockedMicros.Length]; }
                if (UIThreadBlockedCpuKernelTimeMicros == null) { UIThreadBlockedCpuKernelTimeMicros = new long[UIThreadBlockedMicros.Length]; }
                
                uint currentPass = passCount;
                for (int pass = 0; pass < passCount; pass++)
                {
                    output_row["passNumber"].Set(currentPass--);

                    // yield a new row for each "ui-thread-blocked" entry
                    for (int i = 0; i<UIThreadBlockedNameCRCs.Length; i++)
                    {
                        int idx = (pass * UIThreadBlockedNameCRCs.Length) + i;
                        if (UIThreadBlockedMicros[idx] > 0 || UIThreadBlockedCpuUserTimeMicros[idx] > 0 || UIThreadBlockedCpuKernelTimeMicros[idx] > 0)
                        {
                            string propName = DeCRC.GetStringForCRC(UIThreadBlockedNameCRCs[i]);
                            output_row["propName"].Set(propName);
                            output_row["UIThreadBlockedMicros"].Set(UIThreadBlockedMicros[idx]);
                            output_row["UIThreadBlockedCpuUserTimeMicros"].Set(UIThreadBlockedCpuUserTimeMicros[idx]);
                            output_row["UIThreadBlockedCpuKernelTimeMicros"].Set(UIThreadBlockedCpuKernelTimeMicros[idx]);
                            yield return output_row;
                        }
                    }
                }
            }
        }
    }
}
