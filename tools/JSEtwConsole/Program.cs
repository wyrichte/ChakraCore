// <copyright file="Program.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.Globalization;
    using System.IO;
    using System.Text;
    using System.Threading;
    using System.Xml;
    using InteropTypes = Internals.Interop.Types;
    using InteropMethods = Internals.Interop.NativeMethods;

    /// <summary>
    /// Represents JsEtwConsole program implementation
    /// </summary>
    public static class Program
    {
        #region Constants
        /// <summary>
        /// Switch for file output
        /// </summary>
        private const string SwitchFileOutput = "-file";

        /// <summary>
        /// Switch for auto exit when file exists
        /// </summary>
        private const string SwitchAutoExitFilename = "-exitwhenexists";

        /// <summary>
        /// Switch for name of event to set after startup / shutdown
        /// </summary>
        private const string SwitchEventName = "-eventname";

        /// <summary>
        /// Switch for enabling info level at startup
        /// </summary>
        private const string SwitchInfoLevel = "-info";

        /// <summary>
        /// Switch for enabling info and debug levels at startup
        /// </summary>
        private const string SwitchDebugLevel = "-debug";

        /// <summary>
        /// Switch for disabling output to the console.
        /// </summary>
        private const string SwitchNoConsoleOutput = "-noconsole";

        /// <summary>
        /// Switch for disabling cleanup of all running ETW sessions.
        /// </summary>
        private const string SwitchNoETWCleanupAtStartup = "-noetwcleanup";

        /// <summary>
        /// Switch for disabling verbose file output.
        /// </summary>
        private const string SwitchNonVerboseFileOutput = "-filenonverbose";

        /// <summary>
        /// Switch for defining a startup keyword mask
        /// </summary>
        private const string SwitchKeywordMask = "-keywordmask";

        /// <summary>
        /// Switch for defining a process id to filter events
        /// </summary>
        private const string SwitchProcessId = "-processid";

        /// <summary>
        /// Flushing ETW logic timeout (1 minute)
        /// </summary>
        private static readonly TimeSpan FlushETWTimeout = new TimeSpan(0, 1, 0);

        /// <summary>
        /// Number of ETW messages to trace out to cause a ETW buffer flush
        /// </summary>
        private static readonly int FlushETWNumberMessages = 32;
        
        /// <summary>
        /// Internal start etw message tag (JsEtwConsole internal)
        /// </summary>
        private const string InternalMessageStartTag = "<JsEtwConsole>:";

        /// <summary>
        /// Internal end etw message tag (JsEtwConsole internal)
        /// </summary>
        private const string InternalMessageEndTag = ":</JsEtwConsole>";
        #endregion // Constants

        #region Fields
        /// <summary>
        /// Filename (txt) output file (if any)
        /// </summary>
        private static string outputFileName = null;

        /// <summary>
        /// Filename (as semaphore file) dictating when to exit the application (on exists)
        /// </summary>
        private static string autoExitFileName = null;

        /// <summary>
        /// Name of the event to set when program startup / shutdown is complete (if any)
        /// </summary>
        private static string eventName = null;

        /// <summary>
        /// The event handle used to signal caller on startup / shutdown
        /// </summary>
        private static IntPtr eventHandle = IntPtr.Zero;

        /// <summary>
        /// The process id used to filter events (if any)
        /// </summary>
        private static uint processId = 0;

        /// <summary>
        /// Indicates whether the console output is enabled
        /// </summary>
        private static bool outputToConsole = true;

        /// <summary>
        /// Indicates whether to cleanup all already running ETW sessions on startup
        /// </summary>
        private static bool noCleanup = false;

        /// <summary>
        /// Indicates whether to enable verbose file output
        /// </summary>
        private static bool enableFileVerboseOutput = true;

        /// <summary>
        /// Synchronization object to the outputFileStream
        /// </summary>
        private static object outputFileStreamLock = new object();

        /// <summary>
        /// Output stream written to file
        /// </summary>
        private static StreamWriter outputFileStream = null;

        /// <summary>
        /// Indicates whether the debug level is to be outputted.
        /// </summary>
        private static volatile bool outputDebugLevel = true;

        /// <summary>
        /// Indicates whether the info level is to be outputted.
        /// </summary>
        private static volatile bool outputInfoLevel = true;

        /// <summary>
        /// All guids currently tracked, with their friendly name and 'to be outputted' flag
        /// </summary>
        private static SortedDictionary<Guid, KeyValuePair<string, bool>> guidsTracked;

        /// <summary>
        /// Keyword mask for each tracked guid.
        /// </summary>
        private static SortedDictionary<Guid, ulong> keywordMasks;

        /// <summary>
        /// Indicates whether the application is exiting
        /// </summary>
        private static volatile bool isExiting = false;

        /// <summary>
        /// Indicates whether the application is running
        /// </summary>
        private static volatile bool isRunning = false;

        /// <summary>
        /// Auto-process termination semaphore file watcher
        /// </summary>
        private static FileSystemWatcher autoExitFileSystemWatcher = null;

        /// <summary>
        /// Synchronization object for flushing ETW logic
        /// </summary>
        private static object tagFlushLogMessageSyncLock = new object();

        /// <summary>
        /// Tag (expected) message for flushing ETW logic
        /// </summary>
        private static volatile string tagFlushLogMessage = string.Empty;

        /// <summary>
        /// Flag indicating to discard any pending (to be traced out) log message up to the sync point
        /// </summary>
        private static bool tagFlushLogMessageDiscardPreFlush = false;

        /// <summary>
        /// Timeout (UTC) before declaring flushing ETW logic timeout
        /// </summary>
        private static DateTime tagFlushLogMessageEndTimePollingUtc = DateTime.MinValue;

        /// <summary>
        /// Current process ID (used for flush)
        /// </summary>
        private static readonly int currentProcessId = System.Diagnostics.Process.GetCurrentProcess().Id;
        #endregion // Fields

        #region Properties

        /// <summary>
        /// returns whether the fileverboseoutput is enabled.
        /// </summary>
        public static bool EnableFileVerboseOutput
        {
            get
            {
                return Program.enableFileVerboseOutput;
            }
        }

        #endregion // Properties

        #region Methods

        #region Public Methods
        /// <summary>
        /// Starts the trace with fileOutput for all GUIDs (and disable the console output)
        /// </summary>
        /// <param name="outputFileName">output file name</param>
        /// <param name="isInfo">outputs info level trace messages (if isDebug not enabled, otherwise set to true)</param>
        /// <param name="isDebug">outputs debug level trace messages</param>
        /// <param name="cleanupPreviousETWSessions">indicates whether to clean all previous outstanding ETW sessions on entry</param>
        /// <returns>
        /// EtwListener instance on success, otherwise exception thrown
        /// </returns>
        public static EtwListener StartTrace(string outputFileName, bool isInfo, bool isDebug, bool cleanupPreviousETWSessions)
        {
            bool outputToConsole = false; // disabled by default for library 'call'

            List<string> arguments = new List<string>();

            arguments.Add(Program.SwitchFileOutput);
            arguments.Add(outputFileName);
            if (isDebug)
            {
                arguments.Add(Program.SwitchDebugLevel);
            }

            if (isInfo)
            {
                arguments.Add(Program.SwitchInfoLevel);
            }

            if (cleanupPreviousETWSessions == false)
            {
                arguments.Add(Program.SwitchNoETWCleanupAtStartup);
            }

            arguments.Add(EtwProviders.AllGuidKey);

            return Program.StartTrace(outputToConsole, arguments.ToArray());
        }

        /// <summary>
        /// Stop a given trace instance
        /// </summary>
        /// <param name="listener">trace listener instance to be stopped</param>
        /// <param name="discardDumpETWFile">indicates whether to discard the output dump file (if any)</param>
        public static void StopTrace(EtwListener listener, bool discardDumpETWFile)
        {
            // Flush ETW channel!
            Program.FlushETW(discardDumpETWFile);

            // Dispose
            if (listener != null)
            {
                listener.Stop();
                listener.Dispose();
                listener = null;
            }

            Program.isRunning = false;

            if (Program.outputFileStream != null)
            {
                // append a line for non-empty output file
                Program.outputFileStream.WriteLine("Log completed");
                Program.outputFileStream.Flush();
                Program.outputFileStream.Close();
                Program.outputFileStream.Dispose();
                Program.outputFileStream = null;

                if (discardDumpETWFile)
                {
                    FileHelper.TryDeleteFile(Program.outputFileName);
                }
            }

            if (Program.eventHandle != IntPtr.Zero)
            {
                // Set the event to indicate tracing is complete.
                Internals.Interop.NativeMethods.SetEvent(Program.eventHandle);
            }
        }

        #endregion // Public Methods

        #region Private Methods
        /// <summary>
        /// Starts the ETW trace
        /// </summary>
        /// <param name="outputToConsole">indicates whether to output to console</param>
        /// <param name="args">command line parameters</param>
        /// <returns>EtwListener instance on success, otherwise exception thrown</returns>
        private static EtwListener StartTrace(bool outputToConsole, string[] args)
        {
            Program.ResetStaticVariables(outputToConsole);

            EtwListener retValue = null;

            Program.ParseArguments(args);

            InteropMethods.SetConsoleCtrlHandler(new InteropTypes.HandlerRoutine(Program.ConsoleCtrlCheck), true);

            if (Program.noCleanup == false)
            {
                EtwListener.RemoveAllOldSessions();
            }

            // start the file monitoring for automatic exit - if specified
            if (String.IsNullOrWhiteSpace(Program.autoExitFileName) == false)
            {
                if (File.Exists(Program.autoExitFileName))
                {
                    ConsoleHelper.WriteError("FILE {0} already exists -- terminating!", Program.autoExitFileName);
                    throw new System.NotSupportedException();
                }

                string folderName = Path.GetFullPath(Path.Combine(".", Path.GetDirectoryName(Program.autoExitFileName)));
                string fileName = Path.GetFileName(Program.autoExitFileName);
                Program.autoExitFileSystemWatcher = new FileSystemWatcher(folderName, fileName);
                Program.autoExitFileSystemWatcher.Created += fileSystemWatcher_Created;
                Program.autoExitFileSystemWatcher.EnableRaisingEvents = true;
                ConsoleHelper.WriteDebug("Registered file monitor on {0} for auto-exit", Program.autoExitFileName);
            }

            if (String.IsNullOrEmpty(Program.outputFileName) == false)
            {
                ConsoleHelper.WriteInformation("OUTPUT FILE: {0}", outputFileName);
                File.WriteAllText(Program.outputFileName, String.Empty, Encoding.UTF8);
                Program.outputFileStream = File.AppendText(Program.outputFileName);
                Program.outputFileStream.AutoFlush = false;
            }

            retValue = EtwListener.Create(Program.guidsTracked.Keys, new EventHandler<Internals.EventArrivedEventArgs>(Watcher_EventReceived));

            if (Program.outputFileStream != null)
            {
                // append a line for non-empty output file
                Program.outputFileStream.WriteLine("Log started");
                Program.outputFileStream.Flush();
            }

            retValue.Start();

            Program.isRunning = true;

            if (Program.eventHandle != IntPtr.Zero)
            {
                // Set the event to indicate program startup is complete.
                Internals.Interop.NativeMethods.SetEvent(Program.eventHandle);
            }

            return retValue;
        }

        /// <summary>
        /// Event triggering auto-exit on presence of the given semaphore file
        /// </summary>
        /// <param name="sender">object originating the event</param>
        /// <param name="e">arguments to the event</param>
        static void fileSystemWatcher_Created(object sender, FileSystemEventArgs e)
        {
            ConsoleHelper.WriteWarning("Detected presence of file {0} - exiting!", Program.autoExitFileName);
            Program.isExiting = true;
        }

        /// <summary>
        /// Parse all program arguments
        /// </summary>
        /// <param name="args">command line arguments (if any)</param>
        private static void ParseArguments(string[] args)
        {
            List<string> filteredArguments = new List<string>();
            bool isFileOutputParameter = false;
            bool isFileAutoExitParameter = false;
            bool isKeywordMaskParameter = false;
            bool isEventNameParameter = false;
            bool isProcessIdParameter = false;
            string keywordMaskArg = "0x" + ulong.MaxValue.ToString("X8", CultureInfo.InvariantCulture);

            foreach (string arg in args)
            {
                if (isEventNameParameter)
                {
                    Program.eventName = arg;
                    isEventNameParameter = false;

                    Program.eventHandle = Internals.Interop.NativeMethods.OpenEvent((uint)InteropTypes.SyncObjectAccess.EVENT_ALL_ACCESS, false, Program.eventName);
                }
                else if (string.Compare(arg, Program.SwitchEventName, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    if (isEventNameParameter)
                    {
                        throw new NotSupportedException(Program.SwitchEventName);
                    }

                    isEventNameParameter = true;
                }
                else if (isFileOutputParameter)
                {
                    Program.outputFileName = arg;
                    isFileOutputParameter = false;
                }
                else if (String.Compare(arg, Program.SwitchFileOutput, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    if (isFileOutputParameter)
                    {
                        throw new NotSupportedException(Program.SwitchFileOutput);
                    }

                    isFileOutputParameter = true;
                }
                else if (isFileAutoExitParameter)
                {
                    Program.autoExitFileName = arg;
                    isFileAutoExitParameter = false;
                }
                else if (String.Compare(arg, Program.SwitchAutoExitFilename, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    if (isFileAutoExitParameter)
                    {
                        throw new NotSupportedException(Program.SwitchAutoExitFilename);
                    }

                    isFileAutoExitParameter = true;
                }
                else if (isKeywordMaskParameter)
                {
                    // Delay processing the keyword mask until we have gotten the list of provider guids
                    keywordMaskArg = arg;
                    isKeywordMaskParameter = false;
                }
                else if (String.Compare(arg, Program.SwitchKeywordMask, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    if (isKeywordMaskParameter)
                    {
                        throw new NotSupportedException(Program.SwitchKeywordMask);
                    }

                    isKeywordMaskParameter = true;
                }
                else if (String.Compare(arg, Program.SwitchNoConsoleOutput, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    Program.outputToConsole = false;
                }
                else if (String.Compare(arg, Program.SwitchNoETWCleanupAtStartup, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    Program.noCleanup = true;
                }
                else if (String.Compare(arg, Program.SwitchNonVerboseFileOutput, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    Program.enableFileVerboseOutput = false;
                }
                else if (String.Compare(arg, Program.SwitchInfoLevel, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    Program.outputInfoLevel = true;
                }
                else if (String.Compare(arg, Program.SwitchDebugLevel, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    Program.outputDebugLevel = Program.outputInfoLevel = true;
                }
                else if (isProcessIdParameter)
                {
                    Program.processId = uint.Parse(arg, NumberStyles.AllowTrailingWhite | NumberStyles.AllowLeadingWhite, CultureInfo.InvariantCulture);
                    isProcessIdParameter = false;
                }
                else if (String.Compare(arg, Program.SwitchProcessId, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    if (isProcessIdParameter)
                    {
                        throw new NotSupportedException(Program.SwitchProcessId);
                    }

                    isProcessIdParameter = true;
                }
                else
                {
                    filteredArguments.Add(arg);
                }
            }

            if (filteredArguments.Count == 0)
            {
                // add JScript9 as default
                filteredArguments.Add(EtwProviders.JScript9GuidKey);
            }

            // Only one guid / well-known provider alias list (or single)
            bool isValidUsage = filteredArguments.Count == 1;

            if (isValidUsage)
            {
                Program.guidsTracked = new SortedDictionary<Guid, KeyValuePair<string, bool>>();

                isValidUsage = isValidUsage && Program.SetTrackedGuids(filteredArguments[0]);

                Program.keywordMasks = new SortedDictionary<Guid, ulong>();

                // Now that we know which guids we are tracking, set the keyword mask
                isValidUsage = isValidUsage && Program.SetKeywordMask(keywordMaskArg);
            }

            if (isValidUsage == false)
            {
                Console.WriteLine("Usage: JsEtwConsole (option(s)) [List of GUIDs or aliases separated by ;]");
                Console.WriteLine();
                Console.WriteLine("  List of aliases:");
                foreach (string alias in EtwProviders.KnownAliases.Keys)
                {
                    Console.WriteLine("    {0}", alias);
                }

                Console.WriteLine();
                Console.WriteLine("Options:");
                Console.WriteLine();
                Console.WriteLine("OUTPUT FILE:");
                Console.WriteLine(" -file [f]             : output all etw events collected to the output file 'f'");
                Console.WriteLine(" -fileNonVerbose       : enables non-verbose file output, for discarding variant fields (like timestamps) in file output");
                Console.WriteLine();
                Console.WriteLine("STARTUP BEHAVIOR:");
                Console.WriteLine(" -info                 : enables information level output by default on startup");
                Console.WriteLine(" -debug                : enables debug level output by default on startup");
                Console.WriteLine(" -noConsole            : do not enable console output on startup");
                Console.WriteLine(" -noEtwCleanup         : do not clean all registered ETW sessions on startup");
                Console.WriteLine(" -keywordMask [0x..]   : set the keyword mask for all providers to 0x.. on startup");
                Console.WriteLine("                         also supports setting the keyword mask on a per-provider level");
                Console.WriteLine("                         ex: -keywordMask jscript9:0x8000;asynccausality:0x202");
                Console.WriteLine("                             -keywordMask 19a4c69a-28eb-4d4b-8d94-5f19055a1b5c:0x8000");
                Console.WriteLine(" -eventName [name]     : named event will be set after startup / shutdown is complete");
                Console.WriteLine(" -processId [pid]      : only listen for events from this process id");
                Console.WriteLine();
                Console.WriteLine("EXIT BEHAVIOR:");
                Console.WriteLine(" -exitWhenExists [f]   : exit the program automatically on presence of file 'f'");
                Console.WriteLine();
                Console.WriteLine("EXAMPLE USAGES:");
                Console.WriteLine(" JsEtwConsole -keywordMask jscript9:0x8000;19a4c69a-28eb-4d4b-8d94-5f19055a1b5c:0x202 -fileNonVerbose -file etw.log jscript9;19a4c69a-28eb-4d4b-8d94-5f19055a1b5c");
                Console.WriteLine(" JsEtwConsole -keywordMask 0x2 all");
                Console.WriteLine(" JsEtwConsole -processId 8466 trident");
                Console.WriteLine();

                throw new ArgumentOutOfRangeException("args");
            }

            ConsoleHelper.WriteInformation(String.Empty);
        }

        /// <summary>
        /// Reset the state of all static variables
        /// </summary>
        private static void ResetStaticVariables(bool outputToConsole)
        {
            Program.outputDebugLevel = Program.outputInfoLevel = false;
            Program.outputFileName = null;
            Program.isExiting = Program.isRunning = false;
            Program.guidsTracked = null;
            Program.outputToConsole = outputToConsole;
            Program.noCleanup = false;
        }

        /// <summary>
        /// Stop a given trace instance
        /// </summary>
        /// <param name="listener">trace listener instance to be stopped</param>
        private static void StopTrace(EtwListener listener)
        {
            Program.StopTrace(listener, false);
        }

        /// <summary>
        /// Entry point
        /// </summary>
        /// <param name="args">cmdline arguments</param>
        /// <returns>0 on success</returns>
        private static int Main(string[] args)
        {
            ConsoleHelper.Write(ConsoleColor.Yellow, false, "JsEtwConsole");
            ConsoleHelper.Write(ConsoleColor.White, true, " - real time JScript9 ETW listener");
            ConsoleHelper.Write(ConsoleColor.White, true, "--");

            if (PrincipalHelper.IsRunningAsAdmin() == false)
            {
                ConsoleHelper.WriteError("Must be running as administrator");
                return -1;
            }

            EtwListener listener = null;

            try
            {
                try
                {
                    listener = Program.StartTrace(Program.outputToConsole, args);
                }
                catch (ArgumentOutOfRangeException)
                {
                    // when parameters are incorrect
                    return -1;
                }

                // Listen events until user press <Enter> 
                ConsoleHelper.WriteWarning("CLEAR SCREEN                     with C");
                ConsoleHelper.WriteWarning("ENABLE/DISABLE     info messages with I (current = {0})", Program.outputInfoLevel);
                ConsoleHelper.WriteWarning("ENABLE/DISABLE    debug messages with D (current = {0})", Program.outputDebugLevel);
                ConsoleHelper.WriteWarning("ENABLE/DISABLE console output    with O (current = {0})", Program.outputToConsole);
                ConsoleHelper.WriteWarning("ENABLE/DISABLE individual traces with S");
                ConsoleHelper.WriteWarning("CHANGE keyword mask              with M");
                ConsoleHelper.WriteWarning("(Generate a test debug ETW event with T)");
                ConsoleHelper.WriteWarning("Press <Enter> to exit");

                while (Program.isExiting == false)
                {
                    ConsoleKeyInfo key;

                    if (Console.KeyAvailable)
                    {
                        key = Console.ReadKey(true);
                    }
                    else
                    {
                        Thread.Sleep(100);
                        continue;
                    }

                    switch (key.Key)
                    {
                        case ConsoleKey.T:
                            JScript9Etw.Logger.Debug("This is a test!");
                            break;
                             
                        case ConsoleKey.Enter:
                            Program.isExiting = true;
                            break;

                        case ConsoleKey.M:
                            if (Program.IsTrackingJScript9())
                            {
                                Program.ListAllJScript9Keywords();
                            }

                            OutputKeywordMask();
                            ConsoleHelper.WriteWarning("-- new value?");

                            string newValueMaskAsString = Console.ReadLine();

                            Program.SetKeywordMask(newValueMaskAsString);
                            break;

                        case ConsoleKey.C:
                            Console.Clear();
                            break;

                        case ConsoleKey.D:
                            Program.outputDebugLevel = !Program.outputDebugLevel;
                            ConsoleHelper.WriteWarning("DEBUG output is {0}", Program.outputDebugLevel);
                            break;

                        case ConsoleKey.I:
                            if (Program.outputInfoLevel)
                            {
                                Program.outputDebugLevel = Program.outputInfoLevel = false;
                            }
                            else
                            {
                                // keep the debug at false
                                Program.outputInfoLevel = true;
                            }

                            ConsoleHelper.WriteWarning("INFO output is {0}", Program.outputInfoLevel);
                            break;

                        case ConsoleKey.S:
                            bool isBackSelected = false;
                            ConsoleHelper.WriteWarning("(GUID SELECTION MENU)-----------------------------");
                            while (isBackSelected == false)
                            {
                                ConsoleHelper.WriteWarning("Guid(s) being tracked:");
                                int index = 1;
                                foreach (KeyValuePair<Guid, KeyValuePair<string, bool>> guidWithnameAndOutput in Program.guidsTracked)
                                {
                                    ConsoleHelper.WriteWarning(" {0}: {1} is {2}", index, guidWithnameAndOutput.Value.Key, guidWithnameAndOutput.Value.Value);
                                    index++;
                                }

                                ConsoleHelper.WriteWarning(" 0: back");
                                char keyTyped = Console.ReadKey(false).KeyChar;

                                int indexSelected;
                                if (int.TryParse(keyTyped.ToString(CultureInfo.InvariantCulture), out indexSelected))
                                {
                                    if (indexSelected == 0)
                                    {
                                        ConsoleHelper.WriteWarning("(MAIN MENU)-----------------------------");
                                        isBackSelected = true;
                                    }
                                    else
                                    {
                                        if ((indexSelected - 1) >= Program.guidsTracked.Count ||
                                            indexSelected < 0)
                                        {
                                            ConsoleHelper.WriteError("invalid {0} index", indexSelected);
                                        }
                                        else
                                        {
                                            Guid[] allGuids = new Guid[Program.guidsTracked.Count];
                                            Program.guidsTracked.Keys.CopyTo(allGuids, 0);
                                            Guid guidSelected = allGuids[indexSelected - 1];
                                            KeyValuePair<string, bool> nameAndOutput = Program.guidsTracked[guidSelected];
                                            nameAndOutput = new KeyValuePair<string, bool>(nameAndOutput.Key, !nameAndOutput.Value);
                                            Program.guidsTracked[guidSelected] = nameAndOutput;
                                            ConsoleHelper.WriteWarning("{0} output is {1}", nameAndOutput.Key, nameAndOutput.Value);
                                        }
                                    }
                                }
                                else
                                {
                                    ConsoleHelper.WriteError("invalid {0} as index", keyTyped.ToString(CultureInfo.InvariantCulture));
                                }
                            }

                            break;

                        case ConsoleKey.O:
                            Program.outputToConsole = !Program.outputToConsole;
                            ConsoleHelper.WriteWarning("Output to console is {0}", Program.outputToConsole);
                            break;
                    }
                }

                ConsoleHelper.WriteWarning("<Enter> pressed - exiting...");
            }
            catch (Exception ex)
            {
                ConsoleHelper.WriteError("Caught exception: {0}", ex.ToString());
            }
            finally
            {
                if (Program.autoExitFileSystemWatcher != null)
                {
                    Program.autoExitFileSystemWatcher.EnableRaisingEvents = false;
                    Program.autoExitFileSystemWatcher.Dispose();
                    Program.autoExitFileSystemWatcher = null;
                }

                if (listener != null)
                {
                    Program.StopTrace(listener);

                    listener.Dispose();
                    listener = null;
                }
            }

            return 0;
        }

        /// <summary>
        /// Write out the keyword mask to the console
        /// </summary>
        private static void OutputKeywordMask()
        {
            if (Program.keywordMasks.Count == 1)
            {
                ulong mask = Program.keywordMasks.Values.GetEnumerator().Current;
                ConsoleHelper.WriteInformation("Keyword mask is 0x{0}", mask.ToString("X8", CultureInfo.InvariantCulture));
            }
            else
            {
                foreach (var maskPair in Program.keywordMasks)
                {
                    string friendlyName = EtwProviders.GetKnownAlias(maskPair.Key);
                    ConsoleHelper.WriteInformation("Keyword mask for {0}{1} is 0x{2}", maskPair.Key, String.IsNullOrEmpty(friendlyName) ? String.Empty : " (" + friendlyName + ")", maskPair.Value.ToString("X8", CultureInfo.InvariantCulture));
                }
            }
        }

        /// <summary>
        /// Parse a single hex value from a string.
        /// </summary>
        /// <param name="value">string from which to parse a hexadecimal value</param>
        /// <returns>0 if value is null or empty, a valid ulong otherwise</returns>
        private static ulong ParseSingleHexadecimalValue(string value)
        {
            if (String.IsNullOrEmpty(value))
            {
                return 0;
            }

            if (value.ToLower(CultureInfo.InvariantCulture).StartsWith("0x", StringComparison.OrdinalIgnoreCase))
            {
                return ulong.Parse(value.Substring(2), System.Globalization.NumberStyles.AllowHexSpecifier, CultureInfo.InvariantCulture);
            }
            else
            {
                return (ulong)long.Parse(value, CultureInfo.InvariantCulture);
            }
        }

        /// <summary>
        /// Set the keyword mask (possibly per-provider) as specified
        /// </summary>
        /// <param name="newValueMaskAsString">
        /// Either a single value to use as a global keyword mask or a list of Guid or alias / keyword mask (for per-provider keyword masks).
        /// Guid / keyword mask pairs needs to be delimited by a ';' character.
        /// Each Guid / keyword mask pair should be internally delmited by a ':' character.
        /// The following are valid:
        ///     -keywordmask jscript9:0x8000;19a4c69a-28eb-4d4b-8d94-5f19055a1b5c:0x202
        ///     -keywordmask 0x400
        ///     -keywordmask all:0x0;jscript9:0x8000
        /// </param>
        /// <returns>true if successfully set the keyword mask</returns>
        private static bool SetKeywordMask(string newValueMaskAsString)
        {
            // Keyword mask should be specified as either a series of provider friendly-name or guid / keyword values as such:
            //     -keywordmask jscript9:0x8000;19a4c69a-28eb-4d4b-8d94-5f19055a1b5c:0x202
            // or a single hexadecimal mask value as such:
            //     -keywordmask 0x202
            // if we only specify a single mask value, it will be applied to all providers.
            if (!string.IsNullOrEmpty(newValueMaskAsString))
            {
                // Clear any old keyword masks
                Program.keywordMasks.Clear();
                string[] maskPairs = newValueMaskAsString.Split(';');

                foreach (string maskPair in maskPairs)
                {
                    string[] mask = maskPair.Split(':');

                    Guid[] guids;
                    ulong maskValue;

                    if (mask.Length == 2)
                    {
                        // maskPair is formatted as [friendlyname/guid:keywordmask]
                        string friendlyName = mask[0];
                        maskValue = Program.ParseSingleHexadecimalValue(mask[1]);

                        // See if the first argument is a well-known provider name
                        if (EtwProviders.KnownAliases.TryGetValue(friendlyName, out guids) == false)
                        {
                            // friendlyName is not a known provider - try to parse a guid from the string
                            Guid tempGuid;
                            if (!Guid.TryParse(friendlyName, out tempGuid))
                            {
                                ConsoleHelper.WriteError("Failed to parse friendly name or guid from {0}", friendlyName);
                                Console.WriteLine();
                                return false;
                            }

                            guids = new Guid[] { tempGuid };
                        }
                    }
                    else if (mask.Length == 1)
                    {
                        // mask is simply formatted as a single hex value
                        maskValue = Program.ParseSingleHexadecimalValue(mask[0]);

                        // Apply the global mask to all guids
                        if (EtwProviders.KnownAliases.TryGetValue(EtwProviders.AllGuidKey, out guids) == false)
                        {
                            ConsoleHelper.WriteError("Failed to set keyword mask for all known guids");
                            Console.WriteLine();
                            return false;
                        }
                    }
                    else
                    {
                        // some other mask format is provided
                        ConsoleHelper.WriteError("Cannot parse friendly name / guid : keyword mask from {0}", maskPair);
                        Console.WriteLine();
                        return false;
                    }

                    // We now have a list of the guids to which we should apply this mask
                    foreach (Guid guid in guids)
                    {
                        Program.keywordMasks[guid] = maskValue;
                    }
                }
            }

            // If we are tracking guids which did not have keyword masks specified, we should default them
            foreach (var pair in Program.guidsTracked)
            {
                ulong value;
                if (Program.keywordMasks.TryGetValue(pair.Key, out value) == false)
                {
                    Program.keywordMasks[pair.Key] = ulong.MaxValue;
                }
            }

            Program.OutputKeywordMask();

            return true;
        }

        /// <summary>
        /// Print the list of tracked Guids out to the console
        /// </summary>
        private static void OutputTrackedGuids()
        {
            ConsoleHelper.WriteInformation("Listening to guids:");

            foreach(var pair in Program.guidsTracked)
            {
                ConsoleHelper.WriteInformation("  {0}{1}", pair.Key, String.IsNullOrEmpty(pair.Value.Key) ? String.Empty : " (" + pair.Value.Key + ")");
            }
        }

        /// <summary>
        /// Set the list of tracked Guids as specified
        /// </summary>
        /// <param name="trackedGuidList">A string including a list of guids or well-known provider aliases to track. Each should be delimited with a ';' char.</param>
        /// <returns>true if successfully set the list of tracked Guids</returns>
        private static bool SetTrackedGuids(string trackedGuidList)
        {
            if (!String.IsNullOrEmpty(trackedGuidList))
            {
                // Clear any existing tracked Guids
                Program.guidsTracked.Clear();
                List<Guid> guids = new List<Guid>();
                string[] trackedGuidStrings = trackedGuidList.Split(';');

                foreach (string guidOrAlias in trackedGuidStrings)
                {
                    Guid[] tempGuids;

                    // Try using guidOrAlias as a well-known provider alias
                    if (EtwProviders.KnownAliases.TryGetValue(guidOrAlias, out tempGuids) == false)
                    {
                        Guid parsedGuid;

                        // Try to parse guidOrAlias as a Guid
                        if (Guid.TryParse(guidOrAlias, out parsedGuid) == false)
                        {
                            ConsoleHelper.WriteError("Invalid Guid {0}", guidOrAlias);
                            Console.WriteLine();
                            return false;
                        }
                        else
                        {
                            guids.Add(parsedGuid);
                        }
                    }
                    else
                    {
                        guids.AddRange(tempGuids);
                    }
                }

                foreach (Guid guid in guids)
                {
                    // Try getting friendly name
                    string friendlyName = EtwProviders.GetKnownAlias(guid);

                    // Add a mapping for this Guid to the list of tracked Guids
                    Program.guidsTracked.Add(guid, new KeyValuePair<string, bool>(friendlyName, true));
                }
            }

            Program.OutputTrackedGuids();

            return true;
        }

        /// <summary>
        /// List all keywords for JScript9 on the Console 
        /// </summary>
        private static void ListAllJScript9Keywords()
        {
            string xpath = string.Format(CultureInfo.InvariantCulture, "//e:provider[translate(@guid, 'ABCDEF', 'abcdef')='{0}']/e:keywords/e:keyword", EtwProviders.TraceGuidJScript9Provider.ToString("B", CultureInfo.InvariantCulture));

            foreach (XmlNode xnoKeyword in EtwManifestCache.ManifestXml.SelectNodes(xpath, EtwManifestCache.ManifestNamespaces))
            {
                XmlElement xelKeyword = xnoKeyword as XmlElement;
                Contract.Assert(xelKeyword != null);
                ConsoleHelper.WriteInformation("{0} {1}", xelKeyword.GetAttribute("name").PadRight(40), xelKeyword.GetAttribute("mask"));
            }
        }

        /// <summary>
        /// Indicates whether JScript9 GUID is currently tracked
        /// </summary>
        /// <returns>true if listening to JScript9 events, false otherwise</returns>
        private static bool IsTrackingJScript9()
        {
            return Program.IsTrackingGuid(EtwProviders.TraceGuidJScript9Provider);
        }

        /// <summary>
        /// Indicates whether Guid g is currently being tracked
        /// </summary>
        /// <param name="g">Guid to check</param>
        /// <returns>true if tracking events from provider with Guid g</returns>
        private static bool IsTrackingGuid(Guid g)
        {
            bool isTracked;
            KeyValuePair<string, bool> match;

            if (Program.guidsTracked.TryGetValue(g, out match) == false)
            {
                isTracked = false;
            }
            else
            {
                isTracked = match.Value;
            }

            return isTracked;
        }

        /// <summary>
        /// Trap all console exit (process killed, ctrlc, ...) and wait for proper cleanup
        /// </summary>
        /// <param name="ctrlType">type of exit event</param>
        /// <returns>true always to indicate success</returns>
        private static bool ConsoleCtrlCheck(InteropTypes.CtrlTypes ctrlType)
        {
            Program.isExiting = true;

            ConsoleHelper.WriteInformation("Caught {0} event - exiting", ctrlType.ToString());

            while (Program.isRunning == true)
            {
                Thread.Sleep(10);
            }

            return true;
        }

        /// <summary>
        /// Flush ETW via the flush (not outputted) channel
        /// </summary>
        /// <param name="discardDumpETWFile">indicates whether to discard the output dump file (if any)</param>
        private static void FlushETW(bool discardDumpETWFile)
        {
            Console.WriteLine(">>> Flush in progress <<<");

            string tagFlushLogMessage = string.Format(CultureInfo.InvariantCulture, 
                "{2}----------------------- {0}::{1} -----------------------{3}", 
                DateTime.UtcNow.ToString("o", CultureInfo.InvariantCulture), Thread.CurrentThread.ManagedThreadId, 
                InternalMessageStartTag, InternalMessageEndTag);

            DateTime startTime = DateTime.UtcNow;
            DateTime endTimePolling = startTime.Add(Program.FlushETWTimeout);

            lock (Program.tagFlushLogMessageSyncLock)
            {
                Program.tagFlushLogMessage = tagFlushLogMessage;
                Program.tagFlushLogMessageDiscardPreFlush = discardDumpETWFile;

                if (discardDumpETWFile == true)
                {
                    // async wait - gate the logging logic with timeout logic
                    Program.tagFlushLogMessageEndTimePollingUtc = endTimePolling;
                }
                else
                {
                    Program.tagFlushLogMessageEndTimePollingUtc = DateTime.MaxValue;
                }

                // do it N times, to cause a ETW buffer flush
                for (int i = 0; i < Program.FlushETWNumberMessages; i++)
                {
                    JScript9Etw.Logger.Debug(tagFlushLogMessage);
                }
            }

            // full wait in case we do want to flush the etw file
            if (discardDumpETWFile == false)
            {
                // sync wait
                while (DateTime.UtcNow < endTimePolling)
                {
                    lock (Program.tagFlushLogMessageSyncLock)
                    {
                        if (string.IsNullOrEmpty(Program.tagFlushLogMessage))
                        {
                            Console.WriteLine(">>> Flush completed <<<");
                            // done!
                            break;
                        }
                    }

                    // wait a while
                    Thread.Sleep(100);
                }

                if (DateTime.UtcNow >= endTimePolling)
                {
                    // timeout
                    Console.WriteLine(">>> Flush completed (timed-out) <<<");
                    lock (Program.tagFlushLogMessageSyncLock)
                    {
                        Program.tagFlushLogMessage = null;
                        Program.tagFlushLogMessageDiscardPreFlush = false;
                        Program.tagFlushLogMessageEndTimePollingUtc = DateTime.MinValue;
                    }
                }
            }
        }
        #endregion // Private Methods

        #endregion // Methods

        #region Event Handlers
        /// <summary>
        /// Triggered when a new event received
        /// </summary>
        /// <param name="sender">instance triggering</param>
        /// <param name="e">context as parameter</param>
        private static void Watcher_EventReceived(object sender, Internals.EventArrivedEventArgs e)
        {
            if (e.EventException != null)
            {
                // Handle the exception 
                ConsoleHelper.WriteError(e.EventException.ToString());
                Environment.Exit(-1);
            }

            KeyValuePair<string, bool> nameAndTrace;
            ulong keywordMask;

            // NOTE: e.Keyword == 0 for all .NET generated events
            if (Program.guidsTracked.TryGetValue(e.ProviderId, out nameAndTrace) &&
                nameAndTrace.Value &&
                ((Program.processId == 0) || (e.ProcessId == Program.processId) || (e.ProcessId == Program.currentProcessId)) &&
                Program.keywordMasks.TryGetValue(e.ProviderId, out keywordMask) &&
                ((e.Keyword == 0) || (keywordMask & e.Keyword) != 0)) 
            {
                object val;
                string valAsString = string.Empty;

                if (e.Properties.TryGetValue(Internals.TraceEventInfoWrapper.DefaultPropertyEventDataName, out val))
                {
                    valAsString = val as string;
                }
                else
                {
                    // more generic
                    StringBuilder sb = new StringBuilder();
                    string eventName = e.EventName.Trim();
                    sb.Append(eventName);
                    sb.Append(" ");
                    sb.Append(e.Properties.ToString());

                    valAsString = sb.ToString();
                }

                // prepend the value as string with the symbol name, if known
                string symbolName = EtwManifestCache.GetSymbolName(e.ProviderId, e.EventId);
                if (string.IsNullOrWhiteSpace(symbolName) == false)
                {
                    valAsString = symbolName + valAsString;
                }

                // handle flushing logic
                string message = Program.tagFlushLogMessage;
                if (string.IsNullOrEmpty(message) == false)
                {
                    if (e.ProviderId == JScript9Etw.TraceGuid)
                    {
                        if (string.Compare(message, valAsString, StringComparison.Ordinal) == 0)
                        {
                            // potentially the flush to ETW is in progress
                            lock (Program.tagFlushLogMessageSyncLock)
                            {
                                if (string.IsNullOrEmpty(Program.tagFlushLogMessage) == false)
                                {
                                    // flush to ETW to be completed!
                                    Program.tagFlushLogMessage = null;
                                    Program.tagFlushLogMessageDiscardPreFlush = false;
                                }
                            }
                        }
                    }

                    // flush ETW logic with discard trace 'til timeout ...
                    if (DateTime.UtcNow >= Program.tagFlushLogMessageEndTimePollingUtc)
                    {
                        // timeout
                        lock (Program.tagFlushLogMessageSyncLock)
                        {
                            if (DateTime.UtcNow >= Program.tagFlushLogMessageEndTimePollingUtc)
                            {
                                Program.tagFlushLogMessage = null;
                                Program.tagFlushLogMessageDiscardPreFlush = false;
                                Program.tagFlushLogMessageEndTimePollingUtc = DateTime.MinValue;
                            }
                        }
                    }
                }

                // do never trace for 'internal' messages
                bool doTrace = (e.ProviderId != JScript9Etw.TraceGuid) ||
                    (valAsString == null) ||
                    !(valAsString.StartsWith(Program.InternalMessageStartTag, StringComparison.Ordinal) &&
                      valAsString.EndsWith(Program.InternalMessageEndTag, StringComparison.Ordinal));

                if (doTrace)
                {
                    // write ETW (to file, console)
                    Program.WriteETWMessage(
                        e.Date,
                        e.ProcessId,
                        e.ThreadId,
                        e.Level,
                        e.Keyword,
                        nameAndTrace.Key,
                        valAsString);
                }
            }
        }

        /// <summary>
        /// Write an ETW message to the console output
        /// </summary>
        /// <param name="dateTime">date time of the event</param>
        /// <param name="processId">process id generating the event</param>
        /// <param name="threadId">thread id generating the event</param>
        /// <param name="traceEventType">event level</param>
        /// <param name="keyword">event keyword</param>
        /// <param name="traceProviderName">friendly name of the trace provider</param>
        /// <param name="eventMessage">event message</param>
        private static void WriteETWMessage(
            DateTime dateTime,
            uint processId,
            uint threadId,
            System.Diagnostics.TraceEventType traceEventType,
            ulong keyword,
            string traceProviderName,
            string eventMessage)
        {
            // create key
            ulong key = (processId << 32) | threadId;

            string message = String.Join(
                String.Empty,
                new string[] { traceProviderName.PadRight(7), processId.ToString("X4", CultureInfo.InvariantCulture), "#", threadId.ToString("X4", CultureInfo.InvariantCulture), "@", keyword.ToString("X4", CultureInfo.InvariantCulture), " ", dateTime.ToCachedRoundTripLocalString(), " ", eventMessage });

            if (Program.outputFileStream != null)
            {
                string traceEventTypeString = String.Empty;
                switch (traceEventType)
                {
                    case System.Diagnostics.TraceEventType.Critical:
                    case System.Diagnostics.TraceEventType.Error:
                        traceEventTypeString = "E    ";
                        break;

                    case System.Diagnostics.TraceEventType.Warning:
                        traceEventTypeString = " W   ";
                        break;

                    case System.Diagnostics.TraceEventType.Information:
                        traceEventTypeString = "  I  ";
                        break;

                    case System.Diagnostics.TraceEventType.Verbose:
                        traceEventTypeString = "   V ";
                        break;

                    default:
                        traceEventTypeString = "   - ";
                        break;
                }

                lock (Program.outputFileStreamLock)
                {
                    if (Program.isRunning)
                    {
                        // the message is either verbose or not based on the enableFileVerboseOutput flag
                        Program.outputFileStream.WriteLine(traceEventTypeString + (Program.enableFileVerboseOutput ? message : eventMessage));
                    }
                }
            }

            if (Program.outputToConsole == false)
            {
                return;
            }

            if (Program.outputDebugLevel == false && traceEventType == System.Diagnostics.TraceEventType.Verbose)
            {
                // ignored
                return;
            }

            if (Program.outputInfoLevel == false && traceEventType == System.Diagnostics.TraceEventType.Information)
            {
                // ignored
                return;
            }

            switch (traceEventType)
            {
                case System.Diagnostics.TraceEventType.Critical:
                case System.Diagnostics.TraceEventType.Error:
                    ConsoleHelper.WriteError(message);
                    break;

                case System.Diagnostics.TraceEventType.Warning:
                    ConsoleHelper.WriteWarning(message);
                    break;

                case System.Diagnostics.TraceEventType.Information:
                    ConsoleHelper.WriteInformation(message);
                    break;

                case System.Diagnostics.TraceEventType.Verbose:
                    ConsoleHelper.WriteDebug(message);
                    break;

                default:
                    ConsoleHelper.WriteOther(message);
                    break;
            }
        }
        #endregion // Event Handlers
    }
}
