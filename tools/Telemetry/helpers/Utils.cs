using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Chakra.Utils
{
    public class Filters
    {
        public static string ExtractAppName(string appVer)
        {
            string imageName = appVer == null ? "Unknown" : appVer.Substring(appVer.LastIndexOf("!") + 1).ToLower();

            int dotIndex = imageName.LastIndexOf(".");

            if (dotIndex == -1)
            {
                return imageName;
            }
            else
            {
                // Remove trailing characters after 3-character extension. See: Bug 1668107:UTC: AppId value contains garbage characters at the end of the strin ( https://microsoft.visualstudio.com/DefaultCollection/OS/_workitems/edit/1668107 )
                return (imageName.Length > dotIndex + 4) ? imageName.Substring(0, dotIndex + 4) : imageName;
            }
        }

        public static bool IsBrowser(string appVer)
        {
            string appName = ExtractAppName(appVer).ToLower();
            return (appName == "iexplore.exe" || appName.StartsWith("spartan"));
        }

#region ExtractDomain

        /// <summary>The "domain" value reported by telemetry often is a complete filesystem path or is prefixed with "www.", this function cleans them up.</summary>
        public static string ExtractDomain(String purportedDomain)
        {
            String hostName;

            if( DetectSimple( purportedDomain ) ) hostName = purportedDomain;
            else
            {
                if( DetectInvalid( purportedDomain, out hostName ) )
                {
                    return hostName; // `hostName` will actually be something like "(File URI)" than an actual hostname.
                }

                Boolean isInvalid = false;
                hostName = ExtractHostName( purportedDomain, ref isInvalid );
                if( isInvalid ) return hostName;
            }

            // Detect "localhost", "www."-prefixes and single-label domains, and trim to 253.

            if( hostName.Equals("localhost", StringComparison.OrdinalIgnoreCase ) ) return hostName;
            
            if     ( hostName.IndexOf('.') == -1 ) return "(Intranet URI)";
            else if( hostName.StartsWith("www.") ) hostName = hostName.Substring(4);
            
            if( hostName.Length > 253 ) return hostName.Substring( 0, 253 );

            return hostName;
        }

        private static readonly Char[] _simpleForbidden = new Char[] { '/', '\\', ':', '(' };

        /// <summary>If the specified input string represents a domain name with no extra (e.g. no URI scheme, no path, no port, etc).</summary>
        private static Boolean DetectSimple(String input)
        {
            return input.IndexOfAny( _simpleForbidden ) == -1;
        }

        private static Boolean DetectInvalid(String input, out String result)
        {
            // Preserve original input if ExtractDomain is called on its own output (e.g. two scripts, both using ExtractDomain, called from one other).
            switch (input)
            {
                case "(Unknown Host)":
                case "(Data URI)":
                case "(Intranet URI)":
                case "(File URI)":
                case "(IP Address)":
                case "(ms-appx-web URI)":
                case "(about: URI)":
                    result = input;
                    return true;
            }
            // Known bad URI schemes and prefixes
            if (input.StartsWith("data:")) { result = "(Data URI)"; return true; }
            if (input.StartsWith("about:")) { result = "(about: URI)"; return true; }
            if (input.StartsWith("file:") ||
                input.StartsWith(@"\") ||
                input.StartsWith(@"/") ||
                (input.Length >= 2 && input[1] == ':')) { result = "(File URI)"; return true; }
            if (input.StartsWith("ms-appx-web") ||
                input.StartsWith("ms-appx")) { result = "(ms-appx-web URI)"; return true; }
//            string ipblock = @"(([1-9]?[0-9])|(1[0-9][0-9])|(2[0-4][0-9])|(25[0-5]))";
//            string ipjoins = @"\.";
//            string ipmatch = @"^" + ipblock + ipjoins + ipblock + ipjoins + ipblock + ipjoins + ipblock + @"$";
//            if (Regex.IsMatch(input, ipmatch)) { result = "(IP Address)"; return true; }

            result = null;
            return false;
        }

        private static String ExtractHostName(String purportedDomain, ref Boolean isInvalid)
        {
            try {
                Uri uri;
                if( Uri.TryCreate( purportedDomain, UriKind.RelativeOrAbsolute, out uri ) )
                {
                    switch( uri.HostNameType )
                    {
                        case UriHostNameType.Dns:
                            return uri.Host;
                        case UriHostNameType.IPv4:
                        case UriHostNameType.IPv6:
                            isInvalid = true;
                            return "(IP Address)"; // not worth differentiating IPv4 vs IPv6.
                        case UriHostNameType.Basic:
                        case UriHostNameType.Unknown:
                        default:
                            isInvalid = true;
                            return "(Unknown Host)";
                    }
                }
            }
            catch(UriFormatException) // despite the name TryCreate, it can still raise exceptions.
            {
            }
            catch(InvalidOperationException) // thrown by `uri`'s property accessors.
            {
            }

            // Fallback parsing:

            Int32 httpLength = -1;
               
            // All http:// URIs: extract the hostname between 2nd '/' and 3rd '/' or 2nd ':'
            if( purportedDomain.StartsWith( "http", StringComparison.OrdinalIgnoreCase ) )
            {
                if     ( purportedDomain.StartsWith("http://")  ) httpLength = 7;
                else if( purportedDomain.StartsWith("https://") ) httpLength = 8;

                if( httpLength > -1 )
                {
                    Int32 firstSlashAfterScheme = purportedDomain.IndexOf( '/', httpLength );
                    Int32 firstColonAfterScheme = purportedDomain.IndexOf( ':', httpLength );
                    if( firstSlashAfterScheme > -1 || firstColonAfterScheme > -1 )
                    {
                        Int32 hostNameEndIdx = firstColonAfterScheme == -1 ? firstSlashAfterScheme : firstColonAfterScheme;

                        return purportedDomain.Substring( httpLength, hostNameEndIdx - httpLength );
                    }
                    // case: http:// URI without any path info
                    return purportedDomain.Substring( httpLength );
                }
            }

            // Remove scheme, if any, but not if it's a port.
            if( purportedDomain.IndexOf(':') > -1 )
            {
                
            }

            Int32 slashIdx = purportedDomain.IndexOf('/'); // case: domain with trailing path info
            if( slashIdx > -1 )
            {
                return purportedDomain.Substring( 0, slashIdx );
            }



            return purportedDomain;
        }

#endregion
        
#region GetIso8601Week

        private static readonly Calendar _calendar = new GregorianCalendar( GregorianCalendarTypes.USEnglish );

        public static String GetIso8601Week(DateTime date)
        {
            DayOfWeek day;
            Int32 weekNumber = GetIso8601WeekNumber( date, out day );
            Int32 year       = _calendar.GetYear( date );

            if     ( weekNumber == 53 && _calendar.GetMonth(date) < 12 ) year--;
            else if( weekNumber ==  1 && _calendar.GetMonth(date) >  1 ) year++;

            return String.Format( CultureInfo.InvariantCulture, "{0}W{1:00}", year, weekNumber );
        }

        /// <remarks>Copied from http://blogs.msdn.com/b/shawnste/archive/2006/01/24/iso-8601-week-of-year-format-in-microsoft-net.aspx</remarks>
        private static Int32 GetIso8601WeekNumber(DateTime date, out DayOfWeek day)
        {
            // If its Monday, Tuesday or Wednesday, then it will be the same week number as whatever Thursday, Friday or Saturday are, and .NET's GetWeekOfYear will compute those correctly.
            day = _calendar.GetDayOfWeek( date );
            if (day >= DayOfWeek.Monday && day <= DayOfWeek.Wednesday)
            {
                date = date.AddDays(3);
            }
            
            // Return the week of our adjusted day
            return _calendar.GetWeekOfYear( date, CalendarWeekRule.FirstFourDayWeek, DayOfWeek.Monday );
        }

        public static DateTime GetMondayOfLastCompleteWeek(DateTime now)
        {
            now = now.Date;
            Int32 daysSinceLastSunday = (Int32)_calendar.GetDayOfWeek( now );
            if( daysSinceLastSunday == 0 ) daysSinceLastSunday = 7;
            DateTime lastSunday = _calendar.AddDays( now, -daysSinceLastSunday );
            DateTime lastMonday = _calendar.AddDays( lastSunday, -6 );
            return lastMonday;
        }

#endregion

#region Watson

        /// <summary>Watson streams have a field `osBuildFlightId` which contains a </summary>
        /// <param name="osBuildFlightId"></param>
        /// <returns></returns>
        public static String ExtractFlightId(String osBuildFlightId)
        {
            if( String.IsNullOrEmpty( osBuildFlightId ) ) return osBuildFlightId;

            // Hexadecimal GUIDs are 32 characters.
            // ...but usually have 4 dashes between digit groups, giving 36 characters.
            // With braces they're 38 digits.

            Int32 brace0Idx = osBuildFlightId.IndexOf('{');
            Int32 brace1Idx = osBuildFlightId.LastIndexOf('}');

            Boolean isValid = false;
            Guid guid;
            String cleanGuid = null;

            switch( osBuildFlightId.Length )
            {
                case 32: // "00000000000000000000000000000000"
                    isValid = Guid.TryParseExact( osBuildFlightId, "N", out guid );
                    if( isValid ) cleanGuid = guid.ToString("D");
                    break;
                case 36: // "00000000-0000-0000-0000-000000000000"
                    isValid = Guid.TryParseExact( osBuildFlightId, "D", out guid );
                    if( isValid ) cleanGuid = osBuildFlightId;
                    break;
                case 38: // "{00000000-0000-0000-0000-000000000000}"
                    if( brace0Idx == 0 && brace1Idx == 37 )
                    {
                        isValid = Guid.TryParseExact( osBuildFlightId, "B", out guid );
                        if( isValid ) cleanGuid = osBuildFlightId.Substring( brace0Idx + 1, 36 );
                    }
                    else isValid = false;
                    break;
                default: // "{00000000-0000-0000-0000-000000000000}.000"
                    if( brace0Idx == 0 && brace1Idx == 37 )
                    {
                        String guidStr = osBuildFlightId.Substring( brace0Idx + 1, 36 );
                        isValid = Guid.TryParseExact( guidStr, "D", out guid );
                        if( isValid ) cleanGuid = guidStr;
                    }
                    else isValid = false;
                    break;
            }

            if( isValid ) return cleanGuid;

            // Indicate error in return value:
            return "ERROR";
        }

        /// <summary>Given a date and time, snaps the time value to the last 6-hour day portion (e.g. 13:30 goes to 12:00 and 21:00 goes to 18:00).</summary>
        public static DateTime LastTimeQuarter(DateTime dateTime)
        {
            // I would use modulo arithmetic, but `if` is easier:
            Int32 retHour;
            {
                TimeSpan time = dateTime.TimeOfDay;
                if( time.Hours < 0 ) throw new ArgumentOutOfRangeException("dateTime", dateTime, "Time component must be positive.");
                if      ( time.Hours <  6 ) retHour = 0;
                else if ( time.Hours < 12 ) retHour = 6;
                else if ( time.Hours < 18 ) retHour = 12;
                else                        retHour = 18;
            }

            DateTime ret = new DateTime( dateTime.Year, dateTime.Month, dateTime.Day, retHour, 0, 0 );
            return ret;
        }

#endregion
        
        public static int IsEmptyMachineConfig(Guid? machineConfig)
        {
            return (machineConfig == null || machineConfig == Guid.Empty) ? 1 : 0;
        }

        public static int IsEmptyDomain(string domain)
        {
            return String.IsNullOrEmpty(domain) ? 1 : 0;
        }

        public static int IsEmptyActivityId(string activityId)
        {
            return String.IsNullOrEmpty(activityId) || activityId == Guid.Empty.ToString() ? 1 : 0;
        }

        public static int IsEmptyDeviceId(string deviceId)
        {
            return IsEmptyActivityId(deviceId);
        }

        public static int IsEmptyStat(string stat)
        {
            return String.IsNullOrEmpty(stat) ? 1 : 0;
        }
    }

    public class SorterUtils
    {
        public static double Score(double l5, double w5t10, double w10t20, double w20t50, double w50t100, double w100t300, double g300)
        {
            double total = l5 + w5t10 + w10t20 + w20t50 + w50t100 + w100t300 + g300;

            if (total == 0)
                return 0;
            else
                return l5 / total + 10 * w5t10 / total + 20 * w10t20 / total + 50 * w20t50 / total + 100 * w50t100 / total + 300 * w100t300 / total + 1000 * g300 / total;
        }

        public static double? Combine(double? x, double? y)
        {
            if (x == null)
                return y;
            else if (y == null)
                return x;
            else
                return x + y;
        }

        public static Regex speedRegex = new Regex(@"([\d.]+)\s*GHz", RegexOptions.IgnoreCase);

        public static string ExtractGHz(string processor)
        {
            if (processor == null)
                return "";

            Match m = speedRegex.Match(processor);
            if (m != null)
            {
                return m.Groups[1].Value;
            }
            else
            {
                return "";
            }
        }




        class MobileMatcher
        {
            public MobileMatcher(Regex pattern, int groupNumber)
            {
                Pattern = pattern;
                GroupNumber = groupNumber;
            }

            public Regex Pattern;
            public int GroupNumber;
        }
        
        // If any of these regexes return a captured group in the indicated position, the processor is considered to be a mobile processor.
        static List<MobileMatcher> MobileRegexes = new List<MobileMatcher> {
            // Intel(R) Core(TM) i5-2450M CPU @ 2.50GHz
            // Intel(R) Core(TM) i3-4130 CPU @ 3.40GHz  [FALSE - not Mobile]
            // Intel(R) Core(TM) i7-3740QM CPU @ 2.70GHz
            new MobileMatcher (new Regex(@"Intel.*Core.*(i5|i7|i3)\s*\-\s*\d+(M|U|Y|Q|QM|MQ|QH|HQ|MX)"), 2),

            // Intel(R) Core(TM)2 Quad CPU    Q8400  @ 2.66GHz
            // Intel(R) Core(TM) i7 CPU       Q 820  @ 1.73GHz
            // Intel(R) Core(TM) Solo CPU     U1400  @ 1.20GHz  
            // Intel(R) Core(TM) i7 CPU       Q 820  @ 1.73GHz
            new MobileMatcher(new Regex(@"Intel.*Core.*\s+(M|U|Y|Q|QM|MQ|QH|HQ|MX)\s*\d+\s+\@"), 1),

            // Intel(R) Atom(TM) CPU N455   @ 1.66GHz
            new MobileMatcher(new Regex(@"Intel.*(Atom)"), 1),

            // Snapdragon S4 Processor MSM8960
            new MobileMatcher(new Regex(@"Snapdragon"), 0)

        };


        public static bool IsMobile(string processor)
        {
            if (processor == null)
            {
                return false;
            }
            foreach (MobileMatcher matcher in MobileRegexes)
            {
                Match m = matcher.Pattern.Match(processor);
                if (m != null && m.Success && m.Groups.Count >= matcher.GroupNumber && m.Groups[matcher.GroupNumber] != null)
                {
                    return true;
                }
            }

            return false;
        }
    }

}
