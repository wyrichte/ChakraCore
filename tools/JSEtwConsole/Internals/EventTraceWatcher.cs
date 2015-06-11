// <copyright file="EventTraceWatcher.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <summary>
// Contains the watcher class monitoring the ETW logging system
// based on http://code.msdn.microsoft.com/EventTraceWatcher
// </summary>

namespace JsEtwConsole.Internals
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Xml;
    using InteropTypes = Interop.Types;

    /// <summary>
    /// Represents the watcher class monitoring the ETW logging system
    /// </summary>
    public class EventTraceWatcher : IDisposable
    {
        #region Fields
        /// <summary>
        /// Logger name
        /// </summary>
        private readonly string loggerName;

        /// <summary>
        /// Current state of the logger (enabled vs. disabled)
        /// </summary>
        private volatile bool enabled;

        /// <summary>
        /// Event trace log file
        /// </summary>
        private InteropTypes.EventTraceLogfile logFile;

        /// <summary>
        /// Lock primitive
        /// </summary>
        private ReaderWriterLockSlim multipleReaderSingleWriterLock = new ReaderWriterLockSlim(LockRecursionPolicy.NoRecursion);

        /// <summary>
        /// Unique ID for the trace
        /// </summary>
        private ulong traceHandle;

        /// <summary>
        /// Async result interface
        /// </summary>
        private IAsyncResult asyncResult;

        /// <summary>
        /// Processing back-end callback
        /// </summary>
        private ProcessTraceDelegate processEventsDelegate;

        /// <summary>
        /// Indicates whether the instance is already disposed
        /// </summary>
        private bool disposed = false;
        #endregion // Fields

        #region Lifetime Methods
        /// <summary>
        /// Initializes a new instance of the <see cref="EventTraceWatcher"/> class.
        /// </summary>
        /// <param name="loggerName">logger name</param>
        /// <param name="customManifestXml">custom manifest (MAN) as XML string, if any</param>
        public EventTraceWatcher(string loggerName, string customManifestXml)
        {
            this.loggerName = loggerName;
            this.CustomManifestXml = customManifestXml;
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="EventTraceWatcher"/> class.
        /// </summary>
        ~EventTraceWatcher()
        {
            this.Dispose(false);
        }
        #endregion // Lifetime Methods

        #region Properties

        #region Types
        /// <summary>
        /// Delegate for background event monitoring
        /// </summary>
        /// <param name="traceHandle">handle of the trace to be monitored</param>
        private delegate void ProcessTraceDelegate(ulong traceHandle);

        /// <summary>
        /// Event triggered when a new ETW event is captured
        /// </summary>
        public event EventHandler<EventArrivedEventArgs> EventArrived;
        #endregion // Types

        #region Public Properties
        /// <summary>
        /// Gets or sets a value indicating whether the logger is enabled or not
        /// </summary>
        public bool Enabled
        {
            get
            {
                return this.enabled;
            }

            set
            {
                this.multipleReaderSingleWriterLock.EnterReadLock();
                if (this.enabled == value)
                {
                    this.multipleReaderSingleWriterLock.ExitReadLock();
                    return;
                }

                this.multipleReaderSingleWriterLock.ExitReadLock();

                this.multipleReaderSingleWriterLock.EnterWriteLock();
                try
                {
                    if (value)
                    {
                        this.StartTracing();
                    }
                    else
                    {
                        this.StopTracing();
                    }

                    this.enabled = value;
                }
                finally
                {
                    this.multipleReaderSingleWriterLock.ExitWriteLock();
                }
            }
        }

        /// <summary>
        /// XML to be used as custom manifest
        /// </summary>
        public string CustomManifestXml
        {
            get;
            private set;
        }

        /// <summary>
        /// Filepath to the custom manifest
        /// </summary>
        public string CustomManifestFile
        {
            get;
            private set;
        }
        #endregion // Public Properties

        #endregion // Properties

        #region Methods
        #region Private Methods
        /// <summary>
        /// Create a proper EventArrivedEventArgs instance based on a captured EventRecord
        /// </summary>
        /// <param name="eventRecord">captured EventRecord</param>
        /// <returns>matching EventArrivedEventArgs instance</returns>
        private EventArrivedEventArgs CreateEventArgsFromEventRecord(InteropTypes.EventRecord eventRecord)
        {
            Guid providerId = eventRecord.EventHeader.ProviderId;
            Internals.TraceEventInfoWrapper traceEventInfo = null;
            TraceEventType level = (TraceEventType)eventRecord.EventHeader.EventDescriptor.Level;
            ulong keyword = eventRecord.EventHeader.EventDescriptor.Keyword;
            ushort eventId = eventRecord.EventHeader.EventDescriptor.Id;
            DateTime date = DateTime.FromFileTime(eventRecord.EventHeader.TimeStamp);
            EventArrivedEventArgs args = null;

            try
            {
                traceEventInfo = new Internals.TraceEventInfoWrapper(eventRecord);

                // Get the properties using the current event information (schema).
                PropertyBag properties = traceEventInfo.GetProperties(eventRecord);

                args = new EventArrivedEventArgs(
                    providerId,
                    keyword,
                    eventId,
                    level,
                    date,
                    eventRecord.EventHeader.ProcessId,
                    eventRecord.EventHeader.ThreadId,
                    traceEventInfo.EventName,
                    properties);
            }
            finally
            {
                if (traceEventInfo != null)
                {
                    traceEventInfo.Dispose();
                }
            }

            return args;
        }

        /// <summary>
        /// Background processing method to route etw events to event processing method
        /// </summary>
        /// <param name="traceHandle">trace handle to listen to</param>
        private void ProcessTraceInBackground(ulong traceHandle)
        {
            try
            {
                Exception asyncException = null;

                try
                {
                    ulong[] array = { traceHandle };
                    int error = Interop.NativeMethods.ProcessTrace(array, 1, IntPtr.Zero, IntPtr.Zero);
                    if (error != 0)
                    {
                        throw new Win32Exception(error, "ProcessTrace");
                    }
                }
                catch (Exception exception)
                {
                    asyncException = exception;
                }

                // Send exception to subscribers.
                if (asyncException != null && this.EventArrived != null)
                {
                    this.EventArrived(this, new EventArrivedEventArgs(asyncException));
                }
            }
            catch (Exception exception)
            {
                System.Diagnostics.Debugger.Log(1, "JsEtwConsole", String.Format(CultureInfo.InvariantCulture, "ProcessTraceInBackground() - caught exception {0}", exception.ToString()));
            }
        }

        /// <summary>
        /// Start ETW tracing listener
        /// </summary>
        private void StartTracing()
        {
            // custom manifest file load
            if (this.CustomManifestXml != null)
            {
                this.CustomManifestFile = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName() + ".man");
                File.WriteAllText(this.CustomManifestFile, this.CustomManifestXml);

                // load it
                int hr = Interop.NativeMethods.TdhLoadManifest(this.CustomManifestFile);
                if (hr != 0)
                {
                    throw new Win32Exception(hr, "TdhLoadManifest");
                }
            }

            const uint RealTime = 0x00000100;
            const uint EventRecord = 0x10000000;

            this.logFile = new InteropTypes.EventTraceLogfile();
            this.logFile.LoggerName = this.loggerName;
            this.logFile.EventRecordCallback = this.EventRecordCallback;

            this.logFile.ProcessTraceMode = EventRecord | RealTime;
            this.traceHandle = Interop.NativeMethods.OpenTrace(ref this.logFile);

            int error = Marshal.GetLastWin32Error();
            if (error != 0)
            {
                throw new Win32Exception(error, "OpenTrace");
            }

            this.processEventsDelegate = new ProcessTraceDelegate(this.ProcessTraceInBackground);
            this.asyncResult = this.processEventsDelegate.BeginInvoke(this.traceHandle, null, this.processEventsDelegate);
        }

        /// <summary>
        /// Stop ETW tracing listener
        /// </summary>
        private void StopTracing()
        {
            int errorCode = Interop.NativeMethods.CloseTrace(this.traceHandle);
            ConsoleHelper.WriteDebug("StopTracing(): CloseTrace returned 0x{0}", errorCode.ToString("X8", CultureInfo.InvariantCulture));

            this.traceHandle = 0;
            this.processEventsDelegate.EndInvoke(this.asyncResult);

            if (string.IsNullOrEmpty(this.CustomManifestFile) == false &&
                File.Exists(this.CustomManifestFile))
            {
                errorCode = Interop.NativeMethods.TdhUnloadManifest(this.CustomManifestFile);
                ConsoleHelper.WriteDebug("StopTracing(): TdhUnloadManifest returned 0x{0}", errorCode.ToString("X8", CultureInfo.InvariantCulture));

                FileHelper.TryDeleteFile(this.CustomManifestFile);
                this.CustomManifestFile = null;
            }
        }
        #endregion // Private Methods

        #region IDisposable Members
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
                    this.Enabled = false;
                    this.multipleReaderSingleWriterLock.Dispose();
                }

                // Call the appropriate methods to clean up
                // unmanaged resources here.

                // Note disposing has been done.
                this.disposed = true;
            }
        }
        #endregion // IDisposable Members
        #endregion // Methods

        #region Event Handlers
        /// <summary>
        /// Event logic triggered when a new ETW event is captured
        /// </summary>
        /// <param name="eventRecord">EventRecord as context</param>
        private void EventRecordCallback([In] ref InteropTypes.EventRecord eventRecord)
        {
            try
            {
                if (this.EventArrived != null)
                {
                    EventArrivedEventArgs e = this.CreateEventArgsFromEventRecord(eventRecord);
                    this.EventArrived(this, e);
                }
            }
            catch (Exception exception)
            {
                string message = String.Format(CultureInfo.InvariantCulture, "EventRecordCallback() - caught exception {0}", exception.ToString());
                System.Diagnostics.Debugger.Log(1, "JsEtwConsole", message);
                Console.WriteLine(message);
            }
        }
        #endregion // Event Handlers
    }
}