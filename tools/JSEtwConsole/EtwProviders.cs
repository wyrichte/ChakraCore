// <copyright file="EtwProviders.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    /// <summary>
    /// Represents a class tracking all known ETW providers
    /// </summary>
    public static class EtwProviders
    {
        /// <summary>
        /// Keyword for tracking all known Guids
        /// </summary>
        public const string AllGuidKey = "all";

        /// <summary>
        /// Keyword for tracking JScript9 Guid
        /// </summary>
        public const string JScript9GuidKey = "jscript9";

        /// <summary>
        /// Keyword for tracking async causality Guid
        /// </summary>
        public const string AsyncCausalityGuidKey = "asynccausality";

        /// <summary>
        /// Keyword for tracking immersive shell Guid
        /// </summary>
        public const string ImmersiveShellGuidKey = "immersiveshell";

        /// <summary>
        /// Keyword for tracking WinINet Guid
        /// </summary>
        public const string WinINetGuidKey = "wininet";

        /// <summary>
        /// Keyword for tracking IEFrame Guid
        /// </summary>
        public const string IEFrameGuidKey = "ieframe";

        /// <summary>
        /// Keyword for tracking trident Guid
        /// </summary>
        public const string TridentGuidKey = "trident";

        /// <summary>
        /// JScript9 well-known trace guid (Microsoft-Scripting-JScript9 provider)
        /// </summary>
        public static readonly Guid TraceGuidJScript9Provider = new Guid("{57277741-3638-4A4B-BDBA-0AC6E45DA56C}");

        /// <summary>
        /// Async causality well-known trace guid (Asynchronous-Causality-Provider provider)
        /// </summary>
        public static readonly Guid TraceGuidAsyncCausalityProvider = new Guid("{19a4c69a-28eb-4d4b-8d94-5f19055a1b5c}");

        /// <summary>
        /// Immersive shell well-known trace guid (Microsoft-Windows-Immersive-Shell provider)
        /// </summary>
        public static readonly Guid TraceGuidImmersiveShellProvider = new Guid("{315A8872-923E-4EA2-9889-33CD4754BF64}");

        /// <summary>
        /// WinINet well-known trace guid (Microsoft-Windows-WinINet provider)
        /// </summary>
        public static readonly Guid TraceGuidWinINetProvider = new Guid("{43d1a55c-76d6-4f7e-995c-64c711e5cafe}");

        /// <summary>
        /// IEFrame well-known trace guid (Microsoft-IEFRAME provider)
        /// </summary>
        public static readonly Guid TraceGuidIEFrameProvider = new Guid("{5c8bb950-959e-4309-8908-67961a1205d5}");

        /// <summary>
        /// Trident well-known trace guid (Microsoft-IE)
        /// </summary>
        public static readonly Guid TraceGuidTridentProvider = new Guid("{9e3b3947-ca5d-4614-91a2-7b624e0e7244}");

        /// <summary>
        /// Dictionary mapping 'aliases' and known guids
        /// </summary>
        /// <remarks>the padding for output is 7 chars only - thus the friendly alias should be 7 chars or less.</remarks>
        internal static Dictionary<string, Guid[]> KnownAliases =
            new Dictionary<string, Guid[]>(StringComparer.OrdinalIgnoreCase)
                {
                    { EtwProviders.JScript9GuidKey, new Guid[] { EtwProviders.TraceGuidJScript9Provider } },
                    { EtwProviders.AsyncCausalityGuidKey, new Guid[] { EtwProviders.TraceGuidAsyncCausalityProvider } },
                    { EtwProviders.ImmersiveShellGuidKey, new Guid[] { EtwProviders.TraceGuidImmersiveShellProvider } },
                    { EtwProviders.WinINetGuidKey, new Guid[] { EtwProviders.TraceGuidWinINetProvider } },
                    { EtwProviders.IEFrameGuidKey, new Guid[] { EtwProviders.TraceGuidIEFrameProvider} },
                    { EtwProviders.TridentGuidKey, new Guid[] { EtwProviders.TraceGuidTridentProvider } },
                    { EtwProviders.AllGuidKey, new Guid[] 
                        { 
                            EtwProviders.TraceGuidJScript9Provider,
                            EtwProviders.TraceGuidAsyncCausalityProvider,
                            EtwProviders.TraceGuidImmersiveShellProvider,
                            EtwProviders.TraceGuidWinINetProvider,
                            EtwProviders.TraceGuidIEFrameProvider,
                            EtwProviders.TraceGuidTridentProvider
                        } 
                    },
                };

        /// <summary>
        /// Reverse lookup a well-known provider alias from it's associated guid
        /// </summary>
        /// <param name="guid">Guid of the well-known alias to find</param>
        /// <returns>Alias of well-known provider or String.Empty if it isn't found</returns>
        internal static string GetKnownAlias(Guid guid)
        {
            // Try getting friendly name
            foreach (KeyValuePair<string, Guid[]> knownAlias in EtwProviders.KnownAliases)
            {
                if (knownAlias.Value != null &&
                    knownAlias.Value.Length == 1 &&
                    knownAlias.Value[0].Equals(guid))
                {
                    return knownAlias.Key;
                }
            }

            return String.Empty;
        }
    }
}
