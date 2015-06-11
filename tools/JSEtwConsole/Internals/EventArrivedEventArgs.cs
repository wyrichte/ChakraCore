// <copyright file="EventArrivedEventArgs.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <summary>
// Contains the context for an event being captured
// based on http://code.msdn.microsoft.com/EventTraceWatcher
// </summary>

namespace JsEtwConsole.Internals
{
    using System;
    using Diagnostics = System.Diagnostics;
    using InteropTypes = Interop.Types;

    /// <summary>
    /// Represents the context for an event being captured
    /// </summary>
    public sealed class EventArrivedEventArgs : EventArgs
    {
        #region Lifetime Methods
        /// <summary>
        /// Initializes a new instance of the <see cref="EventArrivedEventArgs"/> class.
        /// </summary>
        /// <param name="exception">exception to be encapsulated upon</param>
        internal EventArrivedEventArgs(Exception exception)
            : this(Guid.Empty, 0, 0, Diagnostics.TraceEventType.Critical, DateTime.Now, 0, 0, string.Empty, new PropertyBag())
        {
            this.EventException = exception;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="EventArrivedEventArgs"/> class.
        /// </summary>
        /// <param name="providerId">provider guid</param>
        /// <param name="keyword">provider keyword</param>
        /// <param name="eventId">event identifier</param>
        /// <param name="level">event level (info, ...)</param>
        /// <param name="date">date when the event was generated</param>
        /// <param name="processId">process id generating the event</param>
        /// <param name="threadId">thread id generating the event</param>
        /// <param name="eventName">event name</param>
        /// <param name="properties">property bag</param>
        internal EventArrivedEventArgs(
            Guid providerId,
            ulong keyword,
            ushort eventId,
            Diagnostics.TraceEventType level,
            DateTime date,
            uint processId,
            uint threadId,
            string eventName,
            PropertyBag properties)
        {
            this.ProviderId = providerId;
            this.Keyword = keyword;
            this.EventId = eventId;
            this.Level = level;
            this.Date = date;
            this.ProcessId = processId;
            this.ThreadId = threadId;
            this.EventName = eventName;
            this.Properties = properties;
        }
        #endregion // Lifetime Methods

        #region Properties

        #region Public Properties
        /// <summary>
        /// Gets the exception encapsulated in this event (if any)
        /// </summary>
        public Exception EventException
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the event level
        /// </summary>
        public Diagnostics.TraceEventType Level
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the process id generating the event
        /// </summary>
        public uint ProcessId
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the thread id generating the event
        /// </summary>
        public uint ThreadId
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the date time the event was generated on
        /// </summary>
        public DateTime Date
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the event name
        /// </summary>
        public string EventName
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the property bag associated with this event
        /// </summary>
        public PropertyBag Properties
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the event id
        /// </summary>
        public ushort EventId
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the keyword
        /// </summary>
        public ulong Keyword
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the trace guid
        /// </summary>
        public Guid ProviderId
        {
            get;
            private set;
        }
        #endregion // Public Properties

        #endregion // Properties
    }
}
