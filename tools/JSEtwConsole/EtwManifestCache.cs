// <copyright file="EtwManifestCache.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System.Collections.Generic;
    using System.Globalization;
    using System.Text;
    using System.Xml;

    /// <summary>
    /// Represents a manifest cache
    /// </summary>
    public static class EtwManifestCache
    {
        /// <summary>
        /// All cached event details by their eventId
        /// </summary>
        private static Dictionary<ushort, EtwEventDetails> eventsById = new Dictionary<ushort, EtwEventDetails>();

        /// <summary>
        /// Initializes static members of the <see cref="EtwManifestCache"/> class.
        /// </summary>
        static EtwManifestCache()
        {
            EtwManifestCache.ManifestXml = new XmlDocument();
            EtwManifestCache.ManifestXml.LoadXml(ResourceBin.Microsoft_Scripting_JScript9);
            EtwManifestCache.ManifestNamespaces = new XmlNamespaceManager(EtwManifestCache.ManifestXml.NameTable);
            EtwManifestCache.ManifestNamespaces.AddNamespace("e", "http://schemas.microsoft.com/win/2004/08/events");
        }

        /// <summary>
        /// Gets the manifest XML document
        /// </summary>
        public static XmlDocument ManifestXml
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the namespace manager for the manifest XML
        /// </summary>
        public static XmlNamespaceManager ManifestNamespaces
        {
            get;
            private set;
        }

        /// <summary>
        /// Get etw event details based on event id
        /// </summary>
        /// <param name="eventId">given id</param>
        /// <returns>matching event details</returns>
        public static EtwEventDetails GetEtwEventDetails(ushort eventId)
        {
            EtwEventDetails retValue = null;

            if (eventsById.TryGetValue(eventId, out retValue) == false)
            {
                // lazy load
                string xpath = string.Format(CultureInfo.InvariantCulture, "//e:event[@value='{0}']", eventId);
                XmlElement xelEvent = EtwManifestCache.ManifestXml.SelectSingleNode(xpath, EtwManifestCache.ManifestNamespaces) as XmlElement;

                if (xelEvent == null)
                {
                    throw new System.ArgumentOutOfRangeException("eventId = " + eventId);
                }

                retValue = new EtwEventDetails()
                {
                    Symbol = xelEvent.GetAttribute("symbol"),
                };

                eventsById.Add(eventId, retValue);
            }

            return retValue;
        }

        /// <summary>
        /// Get the symbol name of a given event, if known, from the JScript9 manifest
        /// </summary>
        /// <param name="providerId">originating ETW provider ID</param>
        /// <param name="eventId">originating ETW event ID</param>
        /// <returns>matching user friendly string if known, string.Empty otherwise</returns>
        internal static string GetSymbolName(System.Guid providerId, ushort eventId)
        {
            StringBuilder sb = new StringBuilder();

            if (eventId != 0)
            {
                // only for JScript9 and eventId != 0
                // NOTE: .NET events are with eventId == 0
                if (providerId == EtwProviders.TraceGuidJScript9Provider)
                {
                    EtwEventDetails eventDetails = EtwManifestCache.GetEtwEventDetails(eventId);

                    sb.Append(eventDetails.Symbol);
                    sb.Append(' ');
                }

                sb.Append("(#");
                sb.Append(eventId);
                sb.Append("): ");
            }

            return sb.ToString();
        }
    }
}
