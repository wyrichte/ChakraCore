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
    public class ScriptContextTelemetryProcessor : Processor
    {

        /**
         *  This is a marker type that represents the deserialized JSON from the data column
         */
        class DeserializedJson
        {
#pragma warning disable CS0649
            public UInt64[] BuiltInCountNameCRCs;
            public UInt64[] BuiltInCountValues;
            public UInt64[] LanguageFeaturesNameCRCs;
            public UInt64[] LanguageFeaturesValues;
            public UInt64[] RejitReasonCRCs;
            public UInt64[] RejitReasonCounts;
            public UInt64[] RejitReasonCountsCap;
            public UInt64[] BailoutReasonCRCs;
            public UInt64[] BailOutCounts;
            public UInt64[] BailOutCountsCap;
#pragma warning restore CS0649
        }

        string[] keepColumns = {
                "time",
                "markupIsTopLevel",
                "markupDomainHash",
                "binaryVersion",
                "binaryFlavor",
                "binaryArch",
                "discriminator1",
                "discriminator2",
                "activityID",
                "data",
                "domain"
            };

        public override Schema Produces(string[] requested_columns, string[] args, Schema input_schema)
        {
            Schema output_schema = new Schema();

            foreach (ColumnInfo ci in input_schema.Columns)
            {
                if (keepColumns.Contains(ci.Name))
                {
                    output_schema.Add(ci);
                }
            }

            output_schema.Add(new ColumnInfo("scriptContextLifeSpanMicros", ColumnDataType.UInt));
            output_schema.Add(new ColumnInfo("propType", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("propName", ColumnDataType.String));
            output_schema.Add(new ColumnInfo("propValue", ColumnDataType.UInt));

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

                DeserializedJson parsed = Newtonsoft.Json.JsonConvert.DeserializeObject<DeserializedJson>(input_row["data"].String);

                foreach (Row r in AddNameValuePair(parsed.BuiltInCountValues, parsed.BuiltInCountNameCRCs, "builtin", output_row))
                {
                    yield return r;
                }

                foreach (Row r in AddNameValuePair(parsed.LanguageFeaturesValues, parsed.LanguageFeaturesNameCRCs, "languageFeature", output_row))
                {
                    yield return r;
                }

                foreach (Row r in AddNameValuePair(parsed.RejitReasonCounts, parsed.RejitReasonCRCs, "rejitReason", output_row))
                {
                    yield return r;
                }

                foreach (Row r in AddNameValuePair(parsed.RejitReasonCountsCap, parsed.RejitReasonCRCs, "rejitReasonCap", output_row))
                {
                    yield return r;
                }

                foreach (Row r in AddNameValuePair(parsed.BailOutCounts, parsed.BailoutReasonCRCs, "bailoutReason", output_row))
                {
                    yield return r;
                }

                foreach (Row r in AddNameValuePair(parsed.BailOutCountsCap, parsed.BailoutReasonCRCs, "bailoutReasonCap", output_row))
                {
                    yield return r;
                }
            }
        }

        private IEnumerable<Row> AddNameValuePair(UInt64[] values, UInt64[] crcs, string type, Row outputRow)
        {
            for (int i = 0; i < values.Length; i++)
            {
                if (values[i] > 0)
                {
                    // add a new row
                    string propName = DeCRC.GetStringForCRC(crcs[i]);
                    outputRow["propName"].Set(propName);
                    outputRow["propValue"].Set(values[i]);
                    outputRow["propType"].Set(type);

                    // TODO:  update this when we have scriptContextLifeSpan data flowing in
                    outputRow["scriptContextLifeSpanMicros"].Set(0);
                    yield return outputRow;
                }
            }
        }
    }
}
