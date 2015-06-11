//-----------------------------------------------------------------------
// <copyright file="Program.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.Contracts;
    using System.IO;
    using System.Reflection;
    using System.Threading;
    using System.Threading.Tasks;

    /// <summary>
    /// Represents the main application class.
    /// </summary>
    public class Program
    {
        /// <summary>
        /// The help string that is displayed when the user passes
        /// incorrect parameters from the command line (also -? or /?).
        /// </summary>
        private const string HelpStringFormat =
@"ChakraUnitTestTelemetry.exe Help

The tool processes unit test logs that have been copied
up to shares from individual user and snap runs and
sends the data to the Telemetry Web Service for later
querying.

Usage:
ChakraUnitTestTelemetry.exe -user|-snap [-log]

Parameters:

-user : Causes the tool to scan the snap logs at
        {0}
        for Javascript Unit Test logs.  The info
        from these scanned logs will be stored in
        the telemetry web service similar to local
        telemetry runs.  This is the default if
        no -user or -snap flag is supplied.

-snap : Causes the tool to scan the snap logs at
        {1}
        for Javascript Unit Test logs.  The info
        from these scanned logs will be stored in
        the telemetry web service similar to local
        telemetry runs.

-log  : Causes the tool to log all output to disk
        in a folder called 'Logs' next to the exe.

Return Codes:
 0:     Scanning completed successfully.
-1:     An unhandled exception occurred.
-2:     Invalid parameters were passed to the tool.
";

        /// <summary>
        /// The application modes that the application is current running in.
        /// </summary>
        private static ApplicationModes applicationModes = ApplicationModes.None;

        /// <summary>
        /// Whether or not the application should write logs to a logs folder where the
        /// exe was run.
        /// </summary>
        private static bool shouldWriteLogs = false;

        /// <summary>
        /// The main entry point of the application.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        /// <returns>0 if the scanning completed successfully; -1 otherwise.</returns>
        public static int Main(string[] args)
        {
            // Register for any unhandled exceptions so we can track them and kill the application.
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;

            applicationModes = ApplicationModes.None;

            int errorCode;
            if (!ParseCommandLineParameters(args, out errorCode))
            {
                WriteHelpStringToConsole();
                return errorCode;
            }

            Task.WaitAll(ProcessWorkAsync());

            if (shouldWriteLogs)
            {
                // Dump the logs to a file.
                DumpTraceLogsToFile();
            }

            // Everything processed successfully.
            return ErrorCode.Success;
        }

        /// <summary>
        /// Dumps the trace logs to a file.
        /// </summary>
        private static void DumpTraceLogsToFile()
        {
            string executingDirectory = Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]);
            string logsDirectory = Path.Combine(executingDirectory, "Logs");

            DirectoryHelper.CreateDirectoryIfItDoesntExist(logsDirectory);

            string logFileName =
                DateTime.Now.ToString(Constants.DateTimeCustomFileFolderFormat)
              + ".log";

            string logFilePath = Path.Combine(logsDirectory, logFileName);

            File.WriteAllText(logFilePath, Logger.GetTraceLogs());
        }

        /// <summary>
        /// Processes the main work of the application, which is to scan log
        /// folders (local or otherwise) and send the results to the Chakra
        /// telemetry service.
        /// </summary>
        /// <returns>The asynchronous task being performed.</returns>
        private static async Task ProcessWorkAsync()
        {
            List<Task> workList = new List<Task>();
            if (IsApplicationModeSet(applicationModes, ApplicationModes.ShowHelp))
            {
                WriteHelpStringToConsole();
                return;
            }

            if (IsApplicationModeSet(applicationModes, ApplicationModes.ScanningForUserShareLogs))
            {
                workList.Add(UnitTestLogsWorker.UploadUserLogsInformationToTelemetryWebServiceAsync());
            }

            if (IsApplicationModeSet(applicationModes, ApplicationModes.ScanningForSnapShareLogs))
            {
                workList.Add(UnitTestLogsWorker.UploadSnapLogsInformationToTelemetryWebServiceAsync());
            }

            await Task.WhenAll(workList);
        }

        /// <summary>
        /// Determines whether the specified application mode is set (using bitwise flag testing).
        /// </summary>
        /// <param name="applicationModes">The application mode(s) to test.</param>
        /// <param name="applicationModesFlags">The application modes flags to test with.</param>
        /// <returns>
        ///   <c>true</c> if the specified application mode is set; otherwise, <c>false</c>.
        /// </returns>
        private static bool IsApplicationModeSet(ApplicationModes applicationModes, ApplicationModes applicationModesFlags)
        {
            return (applicationModes & applicationModesFlags) == applicationModesFlags;
        }

        /// <summary>
        /// Parses the command line parameters.
        /// </summary>
        /// <param name="args">The command line parameters to parse.</param>
        /// <param name="errorCode">
        /// The error code returned from parsing.  Will return <see cref="ErrorCode.Success"/>
        /// on any valid parameter, otherwise will return <see cref="ErrorCode.InvalidArguments"/>.
        /// </param>
        /// <returns>
        ///   <c>true</c> if the command line parameters were successfully parsed, <c>false</c> otherwise.
        /// </returns>
        /// <exception cref="System.ArgumentNullException">Thrown when <paramref name="args" /> is <c>null</c>.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Thrown when <paramref name="args" />.Length is incorrect.</exception>
        private static bool ParseCommandLineParameters(string[] args, out int errorCode)
        {
            Contract.Requires(args != null);
            Contract.Ensures(
                Contract.ValueAtReturn(out errorCode) == ErrorCode.InvalidArguments
             || Contract.ValueAtReturn(out errorCode) == ErrorCode.Success);

            Logger.Verbose("Parsing command line parameters: {0}", string.Join(",", args));

            errorCode = ErrorCode.InvalidArguments;

            for (int i = 0; i < args.Length; ++i)
            {
                switch (args[i].ToLower())
                {
                    case "-snap":
                        applicationModes |= ApplicationModes.ScanningForSnapShareLogs;
                        break;
                    case "-user":
                        applicationModes |= ApplicationModes.ScanningForUserShareLogs;
                        break;
                    case "-log":
                        shouldWriteLogs = true;
                        break;
                    case "-?":
                    case "/?":
                        applicationModes |= ApplicationModes.ShowHelp;
                        break;
                    default:
                        return false;
                }
            }

            if (applicationModes == ApplicationModes.None)
            {
                applicationModes = ApplicationModes.ShowHelp;
                return false;
            }

            errorCode = ErrorCode.Success;
            return true;
        }

        /// <summary>
        /// Writes the help string to the console.
        /// </summary>
        private static void WriteHelpStringToConsole()
        {
            string message = string.Format(
                HelpStringFormat,
                Settings.Default.UserLogsCopyToFolder,
                Settings.Default.SnapLogsRootFolder);

            Console.WriteLine(message);
        }

        /// <summary>
        /// Handles the UnhandledException event of the CurrentDomain.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="UnhandledExceptionEventArgs" /> instance containing the event data.</param>
        private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            Exception exception = e.ExceptionObject as Exception;
            try
            {
                // Don't allow exceptions here (file share could be down, etc.).
                DumpUnhandledException(exception);

                // Output the exception.
                Console.WriteLine(exception);
                Console.Out.Flush();
            }
            catch
            {
                // Swallow.
            }

            // Terminate immediately so we don't pop up any post mortem debuggers.
            // This application should run non-intrusively on a developer machine.
            Environment.Exit(ErrorCode.UnhandledExceptionOccurred);
        }

        /// <summary>
        /// Dumps the unhandled exception to a file share for investigation (as specified in settings).
        /// </summary>
        /// <param name="exception">The exception to dump.</param>
        private static void DumpUnhandledException(Exception exception)
        {
            Contract.Requires(exception != null);

            // Use the current time and user name for stamping the folder and file name.
            string subFolderName = string.Format(
                "{0}_{1}",
                DateTime.Now.ToString(Constants.DateTimeCustomFileFolderFormat),
                Environment.UserName);

            string fileName = subFolderName + ".log";

            string folderPath = Path.Combine(Settings.Default.UnhandledExceptionFolder, subFolderName);
            DirectoryHelper.CreateDirectoryIfItDoesntExist(folderPath);

            string filePath = Path.Combine(folderPath, fileName);
            ErrorHelper.DumpExceptionAndEnvironmentInformationToFile(exception, filePath);
            ErrorHelper.DumpThreadTraceLogsToFiles(filePath);

            Contract.Assert(File.Exists(filePath));
        }
    }
}
