// <copyright file="Etw.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
    using System.Threading;

    public class EtwListener : IDisposable
    {
        #region Constants
        /// <summary>
        /// Name of the logman executable
        /// </summary>
        private const string LogManExecutableName = "logman.exe";

        /// <summary>
        /// Prefix for all sessions
        /// </summary>
        private const string LogManETWSessionPrefix = "JsEtwConsole-";
        #endregion // Constants

        #region Fields
        /// <summary>
        /// Indicates whether the instance is already disposed
        /// </summary>
        private bool disposed = false;

        /// <summary>
        /// Logman session name
        /// </summary>
        private string logManName;

        /// <summary>
        /// ETW Provider guids monitored
        /// </summary>
        private Guid[] guids;

        /// <summary>
        /// Internal EventTraceWatcher listening to ETW events
        /// </summary>
        private Internals.EventTraceWatcher watcher;
        #endregion // Fields

        #region Lifetime Methods
        /// <summary>
        /// Prevents the creation of a new instance of the <see cref="PropertyBag"/> class.
        /// </summary>
        private EtwListener() 
        { 
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="EtwListener"/> class.
        /// </summary>
        /// <param name="logManName">Logman session name</param>
        /// <param name="guids">ETW provider guids monitored</param>
        /// <param name="onEventReceived">Event to be triggered on event received</param>
        private EtwListener(string logManName, Guid[] guids, EventHandler<Internals.EventArrivedEventArgs> onEventReceived)
        {
            if (string.IsNullOrWhiteSpace(logManName))
            {
                throw new System.ArgumentNullException("logManName");
            }

            if (guids == null)
            {
                throw new System.ArgumentNullException("guids");
            }

            if (guids.Length == 0)
            {
                throw new System.ArgumentOutOfRangeException("guids");
            }

            this.logManName = logManName;
            this.guids = guids;

            EtwListener.CreateLogman(logManName, guids);

            watcher = new Internals.EventTraceWatcher(logManName, ResourceBin.Microsoft_Scripting_JScript9);
            watcher.EventArrived += onEventReceived;
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="EtwListener"/> class.
        /// </summary>
        ~EtwListener()
        {
            this.Dispose(false);
        }
        #endregion // Lifetime Methods

        #region Methods
        /// <summary>
        /// Starts listening to ETW events
        /// </summary>
        public void Start()
        {
            this.watcher.Enabled = true;
        }

        /// <summary>
        /// Stops listening to ETW events
        /// </summary>
        public void Stop()
        {
            this.watcher.Enabled = false;
        }

        /// <summary>
        /// Creates an ETW listener
        /// </summary>
        /// <param name="doCleanup"></param>
        /// <param name="guids"></param>
        /// <param name="onEventReceived"></param>
        /// <returns></returns>
        public static EtwListener Create(IEnumerable<Guid> guids, EventHandler<Internals.EventArrivedEventArgs> onEventReceived)
        {
            EtwListener retValue = null;
            EtwListener watcher = null;

            try
            {
                string logManName =
                    String.Format(
                        CultureInfo.InvariantCulture, 
                        "{0}{1}",
                        EtwListener.LogManETWSessionPrefix,
                        DateTime.Now.ToString("yyyyMMdd-HHmmss", CultureInfo.InvariantCulture));

                List<Guid> guidsAsList = new List<Guid>(guids);
                watcher = new EtwListener(logManName, guidsAsList.ToArray(), onEventReceived);

                // swap
                retValue = watcher;
                watcher = null;
            }
            finally
            {
                if (watcher != null)
                {
                    watcher.Dispose();
                    watcher = null;
                }
            }

            return retValue;
        }

        /// <summary>
        /// Create a new user-defined Data Collector set
        /// </summary>
        /// <param name="logManName">name of the new data collector set</param>
        /// <param name="guids">guids to be registered</param>
        private static void CreateLogman(string logManName, Guid[] guids)
        {
            string folderName = Path.Combine(Environment.ExpandEnvironmentVariables("%WINDIR%"), "System32");
            string consoleOut, errorOut;

            RunnerHelper.RunProcess(
                folderName,
                EtwListener.LogManExecutableName,
                String.Format(CultureInfo.InvariantCulture, "create trace {0}", logManName),
                null,
                out consoleOut,
                out errorOut);

            // enable real time
            RunnerHelper.RunProcess(
                folderName,
                EtwListener.LogManExecutableName,
                String.Format(CultureInfo.InvariantCulture, "update trace {0} -rt", logManName),
                null,
                out consoleOut,
                out errorOut);

            // add guid(s)
            foreach (Guid guid in guids)
            {
                RunnerHelper.RunProcess(
                    folderName,
                    EtwListener.LogManExecutableName,
                    String.Format(CultureInfo.InvariantCulture, "update trace {0} -p {1}", logManName, guid.ToString("B", CultureInfo.InvariantCulture)),
                    null,
                    out consoleOut,
                    out errorOut);
            }

            RunnerHelper.RunProcess(
                folderName,
                EtwListener.LogManExecutableName,
                String.Format(CultureInfo.InvariantCulture, "start {0}", logManName),
                null,
                out consoleOut,
                out errorOut);
        }

        /// <summary>
        /// Remove/unregister all previous sessions of JsEtwConsole
        /// </summary>
        public static void RemoveAllOldSessions()
        {
            string folderName = Path.Combine(Environment.ExpandEnvironmentVariables("%WINDIR%"), "System32");
            string consoleOut, errorOut;

            RunnerHelper.RunProcess(
                folderName,
                EtwListener.LogManExecutableName,
                "query",
                null,
                out consoleOut,
                out errorOut);

            if (String.IsNullOrEmpty(consoleOut) == false)
            {
                // split by rows
                string[] consoleOutRows = consoleOut.Split(Environment.NewLine.ToCharArray(), StringSplitOptions.RemoveEmptyEntries);

                foreach (string consoleOutRow in consoleOutRows)
                {
                    string consoleOutRowString = consoleOutRow.Trim();

                    if (consoleOutRowString.StartsWith(EtwListener.LogManETWSessionPrefix, StringComparison.OrdinalIgnoreCase))
                    {
                        string logManSessionName = consoleOutRowString.Substring(0, consoleOutRowString.IndexOf(' '));

                        EtwListener.RemoveLogman(logManSessionName);
                    }
                }
            }
        }

        /// <summary>
        /// Remove/unregister a given user-defined data collector set
        /// </summary>
        /// <param name="logManName">name of the data collector set to be removed</param>
        private static void RemoveLogman(string logManName)
        {
            string folderName = Path.Combine(Environment.ExpandEnvironmentVariables("%WINDIR%"), "System32");
            string consoleOut, errorOut;

            ConsoleHelper.WriteInformation("Cleaning ETW session {0}", logManName);

            RunnerHelper.RunProcess(
                folderName,
                "logman.exe",
                String.Format(CultureInfo.InvariantCulture, "stop {0}", logManName),
                null,
                out consoleOut,
                out errorOut);

            Thread.Sleep(new TimeSpan(0, 0, 2));

            RunnerHelper.RunProcess(
                folderName,
                "logman.exe",
                String.Format(CultureInfo.InvariantCulture, "delete {0}", logManName),
                null,
                out consoleOut,
                out errorOut);

            ConsoleHelper.WriteInformation("  Cleaned ETW session {0}", logManName);
        }

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
                    // Dispose managed resources.
                    // rollback (if needed) and dispose transaction (if needed)
                    if (this.watcher != null)
                    {
                        this.watcher.Dispose();
                        this.watcher = null;
                    }

                    // clean logman session
                    EtwListener.RemoveLogman(this.logManName);
                }

                // Call the appropriate methods to clean up
                // unmanaged resources here.

                // Note disposing has been done.
                this.disposed = true;
            }
        }
        #endregion // IDisposable Members
        #endregion // Methods
    }
}
