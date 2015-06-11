// <copyright file="EtwLog.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <date>2013-06-26</date>

namespace JsEtwConsole
{
    using System;
    using System.Diagnostics;
    using System.Diagnostics.Eventing;
    using System.Globalization;
    using System.Text;
    using System.Text.RegularExpressions;

    /// <summary>
    /// Represents an ETW logger that logs messages to the Event Tracing for Windows system.
    /// </summary>
    public class EtwLog : IDisposable
    {
        #region Constants
        /// <summary>
        /// Maximum ETW message length (2k of data)
        /// </summary>
        private const int MaximumETWMessageLength = 2048;

        /// <summary>
        /// Delimiter used to separate fields in ETW output
        /// </summary>
        private const string Delimiter = "::";

        /// <summary>
        /// The maximum number of format arguments allowed in a message.
        /// </summary>
        private const int FormatArgumentMaxCount = 9;

        /// <summary>
        /// Default output fields for all ETW event traces
        /// </summary>
        private const TraceOptions TraceOutputOptions =
            TraceOptions.DateTime | TraceOptions.Timestamp | TraceOptions.ProcessId | TraceOptions.ThreadId;

        /// <summary>
        /// Application name
        /// </summary>
        private const string ApplicationName = "JScript9 Etw Logger";
        #endregion // Constants

        #region Fields
        /// <summary>
        /// Indicates whether the instance is already disposed
        /// </summary>
        private bool disposed = false;

        /// <summary>
        /// Gets or sets the TraceProvider used for tracing ETW events
        /// </summary>
        private EventProviderTraceListener traceProviderETW;
        #endregion // Fields

        #region Lifetime Methods
        /// <summary>
        /// Initializes a new instance of the <see cref="EtwLog"/> class.
        /// </summary>
        /// <param name="traceGuid">traceGuid to be used</param>
        public EtwLog(Guid traceGuid)
        {
            this.InitializeETWTraceSource(traceGuid);
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="EtwLog"/> class.
        /// </summary>
        ~EtwLog()
        {
            this.Dispose(false);
        }
        #endregion // Lifetime Methods

        #region Properties

        #region Public Properties
        /// <summary>
        /// Gets or Sets the Trace Guid used
        /// </summary>
        public Guid TraceGuid
        {
            get;

            private set;
        }
        #endregion // Public Properties

        #endregion // Properties

        #region Methods

        #region Public Methods
        /// <summary>
        /// Logs the specified error message to the ETW system.
        /// </summary>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public void Error(string message, params object[] args)
        {
            this.Log(TraceEventType.Error, message, args);
        }

        /// <summary>
        /// Logs the specified warning message to the ETW system.
        /// </summary>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public void Warning(string message, params object[] args)
        {
            this.Log(TraceEventType.Warning, message, args);
        }

        /// <summary>
        /// Logs the specified information message to the ETW system.
        /// </summary>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public void Information(string message, params object[] args)
        {
            this.Log(TraceEventType.Information, message, args);
        }

        /// <summary>
        /// Logs the specified debug message to the ETW system.
        /// </summary>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public void Debug(string message, params object[] args)
        {
            this.Log(TraceEventType.Verbose, message, args);
        }

        /// <summary>
        /// Logs the specified information message to the ETW system for potentially long messages.
        /// </summary>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public void InformationLongMessage(string message, params object[] args)
        {
            this.LogLongMessage(TraceEventType.Information, message, args);
        }

        /// <summary>
        /// Logs the specified debug message to the ETW system for potentially long messages.
        /// </summary>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public void DebugLongMessage(string message, params object[] args)
        {
            this.LogLongMessage(TraceEventType.Verbose, message, args);
        }

        /// <summary>
        /// Implement IDisposable
        /// </summary>
        public void Dispose()
        {
            if (this.disposed == false)
            {
                this.Dispose(true);
                GC.SuppressFinalize(this);
            }
        }
        #endregion // Public Methods

        #region Protected Methods
        /// <summary>
        /// To be overridden by sub-classes.
        /// Sub-class override methods should call Base.Dispose(disposing) first
        /// and set Disposed = true when done
        /// </summary>
        /// <param name="disposing">indicates if this is called from user code or finalizer</param>
        protected virtual void Dispose(bool disposing)
        {
            if (this.disposed == false)
            {
                // If disposing equals true, dispose all managed
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    this.traceProviderETW.Dispose();
                }

                // Call the appropriate methods to clean up
                // unmanaged resources here.

                // Note disposing has been done.
                this.disposed = true;
            }
        }
        #endregion // Protected Methods

        #region Private Methods
        /// <summary>
        /// Initialize the ETW trace source with the given application name
        /// </summary>
        /// <param name="traceGuid">guid to be used for this ETW trace</param>
        private void InitializeETWTraceSource(Guid traceGuid)
        {
            // Create a new ETW trace provider
            this.traceProviderETW =
                new EventProviderTraceListener(traceGuid.ToString(), EtwLog.ApplicationName, EtwLog.Delimiter);

            this.traceProviderETW.TraceOutputOptions = EtwLog.TraceOutputOptions;
        }

        /// <summary>
        /// Logs the specified message with the specified type to the ETW system.
        /// </summary>
        /// <param name="type">The type of message to store.</param>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        private void Log(TraceEventType type, string message, params object[] args)
        {
            TraceEventCache tec = new TraceEventCache();

            this.traceProviderETW.TraceEvent(tec, EtwLog.ApplicationName, type, 0, message, args);
        }

        /// <summary>
        /// Logs the specified message with the specified type to the ETW system for potentially long messages.
        /// </summary>
        /// <param name="type">The type of message to store.</param>
        /// <param name="message">The message format to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        private void LogLongMessage(TraceEventType type, string message, params object[] args)
        {
            string etwMessage = string.Format(CultureInfo.InvariantCulture, message, args);
            int etwMessageLength = etwMessage.Length;
            int numberChucks = etwMessageLength / EtwLog.MaximumETWMessageLength;
            if (etwMessageLength == 0)
            {
                this.Log(type, message, args);
            }
            else
            {
                if (etwMessageLength % EtwLog.MaximumETWMessageLength == 0)
                {
                    numberChucks++;
                }

                if (numberChucks > 1)
                {
                    for (int chunkIndex = 0; chunkIndex < numberChucks; chunkIndex++)
                    {
                        string messageChunk = (chunkIndex + 1 == numberChucks) ? etwMessage : etwMessage.Substring(0, EtwLog.MaximumETWMessageLength);
                        this.Log(type, "## CHUNK {0} / {1} ##{2}{3}", chunkIndex + 1, numberChucks, Environment.NewLine, messageChunk);
                        etwMessage = etwMessage.Substring(messageChunk.Length);
                    }
                }
                else
                {
                    this.Log(type, "{0}", etwMessage);
                }
            }
        }
        #endregion // Private Methods

        #endregion // Methods
    }
}
