using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chakra.Utils
{
    public class MiscUtils
    {
        /**
         *  Convert a long to string that corresponds to the host type.
         *  See HostType enum in core/lib/Common/Core/ConfigFlagsTable.h.
         */
        public static string MapHostingInterfaceToString(long? v)
        {
            string rtrn = "Unknown";
            if (v.HasValue)
            {
                switch (v.Value)
                {
                    case 0:
                        rtrn = "Default";
                        break;
                    case 1:
                        rtrn = "Browser";
                        break;
                    case 2:
                        rtrn = "Application";
                        break;
                    case 3:
                        rtrn = "Webview";
                        break;
                    default:
                        break;
                }
            }
            return rtrn;
        }

        public static Guid TryParseGuid(string guidString, bool generateGuidIfParseFails = false)
        {
            Guid guid = Guid.Empty;
            if (!Guid.TryParse(guidString, out guid) && generateGuidIfParseFails)
            {
                guid = Guid.NewGuid();
            }
            return guid;
        }
    }
}
