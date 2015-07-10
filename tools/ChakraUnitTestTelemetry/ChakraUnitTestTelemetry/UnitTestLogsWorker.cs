//-----------------------------------------------------------------------
// <copyright file="UnitTestLogsWorker.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using ChakraUnitTestTelemetry.UnitTestTelemetryServiceReference;

    /// <summary>
    /// Represents the main worker that performs unit test log operations
    /// for the application.
    /// </summary>
    public static class UnitTestLogsWorker
    {
        /// <summary>
        /// The time marker that is placed at the start of SNAP log runs.  Used
        /// to determine the base time for when UTs are started.
        /// </summary>
        private const string StartTimeMarker = "Logging started at ";

        /// <summary>
        /// The variant line marker to search for in UT logs when
        /// switching between UT variants (interpreted, dynapogo, etc.).
        /// </summary>
        private const string VariantLineMarker = "############# Starting ";

        /// <summary>
        /// The failure line marker to search for in UT logs when
        /// identifying log failures.
        /// </summary>
        private const string FailureLineStartMarker = "ERROR: Test failed to run correctly: ";

        /// <summary>
        /// The failure line end marker to search for when grabbing the
        /// entire error contents.
        /// </summary>
        private const string FailureLineEndMarker = "ERROR: end of bad output file  ============";

        /// <summary>
        /// The machine name line marker to search for in UT logs when
        /// identifying where the tests were run.
        /// </summary>
        private const string MachineNameMarker = " run On Machine ";

        /// <summary>
        /// The machine name line marker to search for in UT logs when
        /// identifying if the run timed out.
        /// </summary>
        private const string TimedOutMarker = "TIMED OUT";

        /// <summary>
        /// The build architecture and type marker to search for in UT logs
        /// when identifying what build was used for a test run.
        /// </summary>
        private const string BuildArchitectureAndTypeMarker = "Setting up Razzle ";

        /// <summary>
        /// The regular expression used to pull out information around which host
        /// was executed and what command line parameters were specified in a failed
        /// test run.
        /// </summary>
        private static readonly Regex TestRunRegex = new Regex(
            @"(?<=\>\s+)(\w+)(?:\.\w+)?"    // Host name (ex. "jdtest.exe" or "jdtest").
          + @"\s+(.*\w)"                    // Command line parameters (ex. "-forceserialized -debuglaunch").
          + @"\s+(.+\.\w+)",                // Local file path to the unit test (ex. "S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js").
            RegexOptions.Compiled);

        /// <summary>
        /// Processes the snap logs on the share that is stored in
        /// <see cref="Settings.Default.SnapLogsRootFolder" /> and uploads
        /// information about the logs to the Chakra Telemetry Web Service for evaluation.
        /// The branches that will have their logs included is specified by the
        /// <see cref="Settings.Default.SnapBranchesToScan" /> setting.
        /// </summary>
        /// <returns>The asynchronous task being performed.</returns>
        public static async Task UploadSnapLogsInformationToTelemetryWebServiceAsync()
        {
            Contract.Ensures(Contract.Result<Task>() != null);

            Logger.Info("Starting gathering/uploading of snap logs to the Chakra telemetry service.");

            // Start searching for logs on the snap share.
            string snapFolder = Settings.Default.SnapLogsRootFolder;
            string[] branchesToSearch = Settings.Default.SnapBranchesToScan.Split(';');

            // Asynchronously upload log information from each SNAP branch folder.
            Task[] branchUploadTasks =
                (from branch in branchesToSearch
                select UploadSnapLogsInformationForBranchAsync(branch)).ToArray();

            // Wait for all tasks to proceed before continuing.
            await Task.WhenAll(branchUploadTasks);

            Logger.Info("Finished gathering/uploading of snap logs to the Chakra telemetry service.");
        }

        /// <summary>
        /// Processes the snap logs on the share that is stored in
        /// <see cref="Settings.Default.LocalLogsCopyToFolder" /> and uploads
        /// information about the logs to the Chakra Telemetry Web Service for evaluation.
        /// </summary>
        /// <returns>The asynchronous task being performed.</returns>
        /// <exception cref="System.NotImplementedException">Not implemented yet - coming soon.</exception>
        public static async Task UploadUserLogsInformationToTelemetryWebServiceAsync()
        {
            Contract.Ensures(Contract.Result<Task>() != null);

            Logger.Info("Starting gathering/uploading of user logs to the Chakra telemetry service.");

            // Start searching for logs on the user share.
            string logsFolder = Settings.Default.UserLogsCopyToFolder;

            // Grab the list of users to upload for.
            string[] userFolders = await Task.Run(() => Directory.GetDirectories(logsFolder));

            // Asynchronously upload log information from each user log folder.
            Task[] userUploadTasks =
                (from userFolder in userFolders
                 select UploadUserFolderLogsToTelemetryWebServiceAsync(userFolder)).ToArray();

            // Wait for all tasks to proceed before continuing.
            await Task.WhenAll(userUploadTasks);

            Logger.Info("Finished gathering/uploading of user logs to the Chakra telemetry service.");
        }

        /// <summary>
        /// Uploads the user folder logs to the telemetry web service.
        /// </summary>
        /// <param name="userFolder">The user folder to upload the logs for.</param>
        /// <returns>The asynchronous task being performed.</returns>
        private static async Task UploadUserFolderLogsToTelemetryWebServiceAsync(string userFolder)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(userFolder));
            Contract.Ensures(Contract.Result<Task>() != null);

            Logger.Verbose("Started scanning for user logs at '{0}'.", userFolder);
            string[] unitTestLogFiles = await Task.Run<string[]>(
                () => Directory.GetFiles(
                    userFolder,
                    "rl.full.log",
                    SearchOption.AllDirectories));
            Logger.Verbose("Finished scanning for user logs at '{0}'.", userFolder);

            Logger.Verbose("Processing '{0}' log files for the '{1}' user folder.", unitTestLogFiles.Length, userFolder);

            // Kick off async requests to grab each file's failure information (if failures are detected in the logs).
            var parseTasks =
                (from logFile in unitTestLogFiles
                 select ParseUserLogInformationAsync(logFile)).ToArray();

            // Wait for all parse requests to complete.
            await Task.WhenAll(parseTasks);

            Logger.Verbose("Finished processing '{0}' log files for the '{1}' user folder.", unitTestLogFiles.Length, userFolder);

            Logger.Verbose("Uploading '{0}' log files for the '{1}' user folder to the telemetry service.", unitTestLogFiles.Length, userFolder);

            // Upload the failure information to the telemetry web service.
            await UploadFailureInformationListToTelemetryWebServiceAsync(parseTasks);

            Logger.Verbose("Finished uploading '{0}' log files for the '{1}' user folder to the telemetry service.", unitTestLogFiles.Length, userFolder);
        }

        /// <summary>
        /// Uploads the failure information list to the telemetry web service asynchronously.
        /// </summary>
        /// <param name="parseTasks">The parse tasks that contain the failure information to upload.</param>
        /// <returns>The asynchronous task being performed.</returns>
        private static async Task UploadFailureInformationListToTelemetryWebServiceAsync(Task<IEnumerable<UnitTestFailureInformation>>[] parseTasks)
        {
            var uploadTasks =
                (from parseTask in parseTasks
                 let failureInformationList = parseTask.Result
                 from failureInformation in failureInformationList
                 select UploadFailureInformationToTelemetryWebServiceAsync(failureInformation)).ToArray();

            await Task.WhenAll(uploadTasks);
        }

        /// <summary>
        /// Parses the user log information from the specified log file.
        /// </summary>
        /// <param name="logFilePath">The log file path to parse out failures from.</param>
        /// <returns>The asynchronous task being performed.</returns>
        private static async Task<IEnumerable<UnitTestFailureInformation>> ParseUserLogInformationAsync(string logFilePath)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(logFilePath));
            Contract.Ensures(Contract.Result<Task<IEnumerable<UnitTestFailureInformation>>>() != null);

            Logger.Verbose("Parsing the user log information from '{0}''.", logFilePath);

            // Read out all the file lines.
            string[] unitTestLogLines = await Task<string[]>.Run(() => File.ReadAllLines(logFilePath));

            string variant;
            DateTime endTime;
            string buildArchitecture;
            string buildType;
            string branchName;
            string machineName;
            string userName;

            ParseRunInformationFromUserLogFilePath(
                logFilePath,
                out variant,
                out endTime,
                out buildArchitecture,
                out buildType,
                out branchName,
                out machineName,
                out userName);

            List<UnitTestFailureInformation> failureInformationList = new List<UnitTestFailureInformation>();
            for (int i = 0; i < unitTestLogLines.Length; ++i)
            {
                string currentLine = unitTestLogLines[i];

                if (currentLine.Contains(FailureLineStartMarker))
                {
                    UnitTestFailureInformation failureInformation = ReadFailureFromLines(
                        i,
                        unitTestLogLines,
                        null /*startTime*/);

                    // Fill in global information.
                    failureInformation.BranchName = branchName;
                    failureInformation.Variant = variant;
                    failureInformation.MachineName = machineName;
                    failureInformation.BuildArchitecture = buildArchitecture;
                    failureInformation.BuildType = buildType;

                    // Local UT run logs don't track timestamps so just use the
                    // times when the runalltests completed running.
                    failureInformation.StartTime = endTime;
                    failureInformation.EndTime = endTime;

                    failureInformation.LogFilePath = logFilePath;
                    failureInformation.WasRunInSnap = false;

                    // TODO: For now set the OS to unknown.  We'll gather this information in future
                    // check-ins.
                    failureInformation.OperatingSystemVersion = "Unknown";

                    failureInformation.UserName = userName;
                    failureInformation.SnapJobId = -1;
                    failureInformation.SnapCheckinId = "-1";

                    failureInformation.Duration = failureInformation.EndTime - failureInformation.StartTime;

                    // Track this failure.
                    failureInformationList.Add(failureInformation);
                }
            }

            Logger.Verbose("Finished parsing the user log information from '{0}'.", logFilePath);
            return failureInformationList;
        }

        /// <summary>
        /// Parses the run information from user log file path.
        /// Ex: \\bpt-scratch\UserFiles\cmorse\Tools\ChakraUnitTestTelemetry\UserLogs\cmorse\cmorse2\fbl_ie_script_dev\x86chk\2013.08.02_16.13.41\interpreted
        /// </summary>
        /// <param name="logFilePath">The log file path to parse the information from.</param>
        /// <param name="variant">The variant of the run (interpreted, dynapogo, etc.).</param>
        /// <param name="endTime">The ending time when all UTs finished running.</param>
        /// <param name="buildArchitecture">The build architecture that was run (x86, amd64, etc.).</param>
        /// <param name="buildType">Type of the build that was run (chk, fre, etc.).</param>
        /// <param name="branchName">The branch that the tests were run on (fbl_ie_script_dev, etc.).</param>
        /// <param name="machineName">Name of the machine that the user ran the tests on.</param>
        /// <param name="userName">Name of the user who ran the tests (alias).</param>
        private static void ParseRunInformationFromUserLogFilePath(
            string logFilePath,
            out string variant,
            out DateTime endTime,
            out string buildArchitecture,
            out string buildType,
            out string branchName,
            out string machineName,
            out string userName)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(logFilePath));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out variant)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out buildArchitecture)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out buildType)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out branchName)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out machineName)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out userName)));

            string filePath = logFilePath;

            // Remove the file name portion.
            filePath = Path.GetDirectoryName(filePath);

            // Read the variant.
            variant = Path.GetFileName(filePath);
            filePath = Path.GetDirectoryName(filePath);

            // Read the end time.
            string dateString = Path.GetFileName(filePath);
            filePath = Path.GetDirectoryName(filePath);
            endTime = DateTime.ParseExact(
                dateString,
                Constants.DateTimeCustomFileFolderFormat,
                CultureInfo.InvariantCulture.DateTimeFormat);

            // Read the architecture and type of the build.
            string buildArchitectureAndType = Path.GetFileName(filePath);
            buildArchitecture = buildArchitectureAndType.Substring(0, buildArchitectureAndType.Length - 3);
            buildType = buildArchitectureAndType.Substring(buildArchitectureAndType.Length - 3, 3);
            filePath = Path.GetDirectoryName(filePath);

            // Read the branch.
            branchName = Path.GetFileName(filePath);
            filePath = Path.GetDirectoryName(filePath);

            // Read the machine name.
            machineName = Path.GetFileName(filePath);
            filePath = Path.GetDirectoryName(filePath);

            // Read the user name.
            userName = Path.GetFileName(filePath);
            filePath = Path.GetDirectoryName(filePath);
        }

        /// <summary>
        /// Uploads the logs' information for the specified branch name to the Chakra Telemetry Web Service.
        /// </summary>
        /// <param name="branchName">The name of the branch to scan for logs.</param>
        /// <returns>The asynchronous task being performed.</returns>
        private static async Task UploadSnapLogsInformationForBranchAsync(string branchName)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(branchName));
            Contract.Ensures(Contract.Result<Task>() != null);

            Logger.Verbose("Scanning for snap logs for the '{0}' branch.", branchName);

            // Get the list of all files in the branch folder.
            string branchFolder = Path.Combine(Settings.Default.SnapLogsRootFolder, branchName);
            string[] unitTestLogFiles = await Task.Run<string[]>(
                () => Directory.GetFiles(
                    branchFolder,
                    "RunJScriptUnitTests*",
                    SearchOption.AllDirectories));

            Logger.Verbose("Processing '{0}' log files for the '{1}' branch.", unitTestLogFiles.Length, branchName);

            // Kick off async requests to grab each file's failure information (if failures are detected in the logs).
            var parseTasks =
                (from logFile in unitTestLogFiles
                select ParseSnapLogInformationAsync(branchName, logFile)).ToArray();

            // Wait for all parse requests to complete.
            await Task.WhenAll(parseTasks);

            Logger.Verbose("Finished processing '{0}' log files for the '{1}' branch.", unitTestLogFiles.Length, branchName);
            Logger.Verbose("Uploading '{0}' log files for the '{1}' branch to the telemetry service.", unitTestLogFiles.Length, branchName);

            // Upload the failure information to the telemetry web service.
            await UploadFailureInformationListToTelemetryWebServiceAsync(parseTasks);

            Logger.Verbose("Finished uploading '{0}' log files for the '{1}' branch to the telemetry service.", unitTestLogFiles.Length, branchName);
        }

        /// <summary>
        /// Uploads the failure information to the Chakra telemetry web service.
        /// </summary>
        /// <param name="failureInformation">The failure information to upload.</param>
        /// <returns>The asynchronous task being performed.</returns>
        private static async Task UploadFailureInformationToTelemetryWebServiceAsync(UnitTestFailureInformation failureInformation)
        {
            Contract.Requires(failureInformation != null);
            Contract.Requires(Settings.Default.MaxUploadAttemptCount > 0);
            Contract.Ensures(Contract.Result<Task>() != null);

            Logger.Verbose("Started uploading failure information to the telemetry service.");

            // Retry sending a specified number of times before giving up.
            int currentAttemptCount = -1;
            while (++currentAttemptCount < Settings.Default.MaxUploadAttemptCount)
            {
                try
                {
                    UnitTestTelemetryServiceClient serviceClient = new UnitTestTelemetryServiceClient();
                    await serviceClient.RecordUnitTestFailureAsync(failureInformation);
                    break;
                }
                catch (CommunicationException ex)
                {
                    Logger.Warning(
                        "A communication exception occurred when attempting to upload to the server on upload attempt '{0}'.\n\nException:\n\n{1}",
                        currentAttemptCount,
                        ex);
                }
                catch (TimeoutException ex)
                {
                    Logger.Warning(
                        "Communicating with the server timed out on upload attempt '{0}'.\n\nException:\n\n{1}",
                        currentAttemptCount,
                        ex);
                }
            }

            if (currentAttemptCount == Settings.Default.MaxUploadAttemptCount)
            {
                // If we have a certain number of failures we proceed without sending the message
                // but still log the issue.  Don't throw as other failures may get a chance to upload
                // if there was a temporary outage (throwing will kill the process).
                Logger.Error(
                    "Failed to upload failure information to the server after '{0}' attempts.  The server may be down or have a server side bug.",
                    currentAttemptCount);
            }

            Logger.Verbose("Finished uploading failure information to the telemetry service.");
        }

        /// <summary>
        /// Parses the snap log information and returns a list of the failure information.
        /// </summary>
        /// <param name="branchName">The name of the branch that the tests were run against.</param>
        /// <param name="logFilePath">The log file path to parse and upload.</param>
        /// <returns>
        /// The list of failures upon completion.  The list can be 0-length if no failures are found.
        /// </returns>
        private static async Task<IEnumerable<UnitTestFailureInformation>> ParseSnapLogInformationAsync(string branchName, string logFilePath)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(branchName));
            Contract.Requires(!string.IsNullOrWhiteSpace(logFilePath));
            Contract.Ensures(Contract.Result<Task<IEnumerable<UnitTestFailureInformation>>>() != null);

            Logger.Verbose("Parsing the snap log information from branch '{0}' at '{1}'.", branchName, logFilePath);

            // Read out all the file lines.
            string[] unitTestLogLines = await Task<string[]>.Run(() => File.ReadAllLines(logFilePath));

            // Current running environment information.
            string machineName = null;
            string currentVariant = null;
            DateTime? startTime = null;
            string buildArchitecture = null;
            string buildType = null;

            string userName;
            int snapJobId;
            string snapCheckinId;
            ParseUserNameAndSnapIdsFromFilePath(logFilePath, out userName, out snapJobId, out snapCheckinId);

            List<UnitTestFailureInformation> failureInformationList = new List<UnitTestFailureInformation>();
            for (int i = 0; i < unitTestLogLines.Length; ++i)
            {
                string currentLine = unitTestLogLines[i];

                if (startTime == null && currentLine.Contains(StartTimeMarker))
                {
                    startTime = ReadStartTimeFromLine(currentLine);
                }
                else if (machineName == null && currentLine.Contains(MachineNameMarker))
                {
                    machineName = ReadMachineNameFromLine(currentLine);
                }
                else if (currentLine.Contains(VariantLineMarker))
                {
                    currentVariant = ReadVariantFromLine(currentLine);
                }
                else if (buildArchitecture == null
                      && buildType == null
                      && currentLine.Contains(BuildArchitectureAndTypeMarker))
                {
                    ReadBuildArchitectureAndTypeFromLine(currentLine, out buildArchitecture, out buildType);
                }
                else if (currentLine.Contains(FailureLineStartMarker))
                {
                    Contract.Assert(
                        !string.IsNullOrWhiteSpace(currentVariant),
                        "The variant was not encountered before failures in the log file.");
                    Contract.Assert(
                        !string.IsNullOrWhiteSpace(machineName),
                        "The machine name was not encountered before failures in the log file.");
                    Contract.Assert(
                        !string.IsNullOrWhiteSpace(buildArchitecture),
                        "The build architecture was not encountered before failures in the log file.");
                    Contract.Assert(
                        !string.IsNullOrWhiteSpace(buildType),
                        "The build type was not encountered before failures in the log file.");
                    Contract.Assert(
                        startTime != null,
                        "The start time was not encountered before failures in the log file.");

                    Contract.Assert(startTime != null);
                    UnitTestFailureInformation failureInformation = ReadFailureFromLines(i, unitTestLogLines, startTime.Value);

                    // Fill in global information.
                    failureInformation.BranchName = branchName;
                    failureInformation.Variant = currentVariant;
                    failureInformation.MachineName = machineName;
                    failureInformation.BuildArchitecture = buildArchitecture;
                    failureInformation.BuildType = buildType;

                    failureInformation.LogFilePath = logFilePath;
                    failureInformation.WasRunInSnap = true;

                    // TODO: For now set the OS to unknown.  We'll gather this information in future
                    // check-ins.
                    failureInformation.OperatingSystemVersion = "Unknown";

                    failureInformation.UserName = userName;
                    failureInformation.SnapJobId = snapJobId;
                    failureInformation.SnapCheckinId = snapCheckinId;

                    failureInformation.Duration = failureInformation.EndTime - failureInformation.StartTime;

                    // Track this failure.
                    failureInformationList.Add(failureInformation);
                }
            }

            Logger.Verbose("Finished parsing the snap log information from branch '{0}' at '{1}'.", branchName, logFilePath);
            return failureInformationList;
        }

        /// <summary>
        /// Reads the build architecture and type from the passed in log line
        /// (ex. "Setting up Razzle amd64 fre").
        /// </summary>
        /// <param name="line">The line to read the build architecture and type from.</param>
        /// <param name="buildArchitecture">The returned build architecture (x86, amd64, etc.).</param>
        /// <param name="buildType">The returned type of the build (chk, fre, etc.).</param>
        private static void ReadBuildArchitectureAndTypeFromLine(string line, out string buildArchitecture, out string buildType)
        {
            ValidateLineIsValidAndContainsMarker(line, BuildArchitectureAndTypeMarker);
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out buildArchitecture)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out buildType)));

            Logger.Verbose("Reading the build architecture and type from log line '{0}'.", line);

            int buildArchitectureStartIndex = line.IndexOf(BuildArchitectureAndTypeMarker) + BuildArchitectureAndTypeMarker.Length;
            int buildArchitectureEndIndex = line.IndexOf(' ', buildArchitectureStartIndex);
            buildArchitecture = line.Substring(buildArchitectureStartIndex, buildArchitectureEndIndex - buildArchitectureStartIndex);
            int buildTypeStartIndex = buildArchitectureEndIndex + 1;
            buildType = line.Substring(buildTypeStartIndex);

            Logger.Verbose(
                "Finished reading the build architecture of '{0}' and type of '{1}' from log line '{2}'.",
                buildArchitecture,
                buildType,
                line);
        }

        /// <summary>
        /// Parses the user name and snap job/check-in ids from the log file path.
        /// </summary>
        /// <param name="logFilePath">The log file path to parse the user name and ID from
        /// (ex. \\iesnap\queue\FBL_IE_SCRIPT_DEV\90209.t-doilij\Job.112721\RunJScriptUnitTests.IESNAP-BLD38.amd64.fre.log).</param>
        /// <param name="userName">The name of the user who ran the test (ex. t-doilij).</param>
        /// <param name="snapJobId">The SNAP job id of the run (ex. 112721).</param>
        /// <param name="snapCheckinId">The SNAP check-in id assigned to the job (ex. 90209 or m113084).</param>
        private static void ParseUserNameAndSnapIdsFromFilePath(
            string logFilePath,
            out string userName,
            out int snapJobId,
            out string snapCheckinId)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(logFilePath));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out userName)));
            Contract.Ensures(Contract.ValueAtReturn<int>(out snapJobId) > 0);
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn<string>(out snapCheckinId)));

            Logger.Verbose("Reading the user name and snap IDs from log file path '{0}'.", logFilePath);

            string snapDirectoryPath = Path.GetDirectoryName(logFilePath);
            string snapDirectoryName = Path.GetFileName(snapDirectoryPath);

            // Read the job ID from the folder name (ex. Job.112721).
            string[] jobIdPair = snapDirectoryName.Split('.');
            Contract.Assert(jobIdPair.Length == 2);

            snapJobId = int.Parse(jobIdPair[1]);

            // Move up to the next folder and read the username and queue ID.
            snapDirectoryPath = Path.GetDirectoryName(snapDirectoryPath);
            snapDirectoryName = Path.GetFileName(snapDirectoryPath);

            string[] userNameAndCheckinIdPair = snapDirectoryName.Split('.');
            Contract.Assert(userNameAndCheckinIdPair.Length == 2);

            snapCheckinId = userNameAndCheckinIdPair[0];
            userName = userNameAndCheckinIdPair[1];

            Logger.Verbose(
                "Finished reading the user name of '{0}', snap job ID of '{1}', and snap check-in ID of '{2}' from log file path '{3}'.",
                userName,
                snapJobId,
                snapCheckinId,
                logFilePath);
        }

        /// <summary>
        /// Reads the machine name from the passed in line
        /// (ex. "... Task id 9145376 run On Machine IESNAP-BLD38 ...").
        /// </summary>
        /// <param name="line">The line to read the machine name from.</param>
        /// <returns>The name of the machine that is running the tests.</returns>
        private static string ReadMachineNameFromLine(string line)
        {
            ValidateLineIsValidAndContainsMarker(line, MachineNameMarker);
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.Result<string>()));

            Logger.Verbose("Reading the machine name from log line '{0}'.", line);

            int startIndex = line.IndexOf(MachineNameMarker) + MachineNameMarker.Length;
            string machineName = line.Substring(startIndex, line.IndexOf(' ', startIndex) - startIndex);

            Logger.Verbose("Finished Reading the machine name '{0}' from log line '{1}'.", machineName, line);
            return machineName;
        }

        /// <summary>
        /// Reads the start time from the passed in line
        /// (ex. "00.00: Logging started at Fri Aug 16 00:23:04 PDT 2013").
        /// </summary>
        /// <param name="line">The line to read the start line from.</param>
        /// <returns>The starting time when the UT logs were run.</returns>
        private static DateTime ReadStartTimeFromLine(string line)
        {
            ValidateLineIsValidAndContainsMarker(line, StartTimeMarker);

            Logger.Verbose("Reading the start time from log line '{0}'.", line);

            int dateStartIndex = line.IndexOf(StartTimeMarker) + StartTimeMarker.Length;
            string dateString = line.Substring(dateStartIndex).Replace("PDT ", string.Empty);

            // Example date format from log: Thu Aug 22 20:31:04 PDT 2013.
            DateTime startTime = DateTime.ParseExact(
                dateString,
                "ddd MMM d H:mm:ss yyyy",
                CultureInfo.InvariantCulture);

            Logger.Verbose("Finished reading the start time of '{0}' from log line '{1}'.", startTime, line);
            return startTime;
        }

        /// <summary>
        /// Reads the failure information from the lines surrounding the passed in index.
        /// </summary>
        /// <param name="currentLineIndex">The current line index where the failure was detected.</param>
        /// <param name="unitTestLogLines">The unit test log lines for the file.</param>
        /// <param name="startTime">
        /// The timestamp of when the unit tests started running.  Can pass <c>null</c> to avoid collecting
        /// timestamps for the start/end (used when reading user logs which don't have the timestamps
        /// available).
        /// </param>
        /// <returns>
        /// The parsed failure information from the log lines.
        /// </returns>
        private static UnitTestFailureInformation ReadFailureFromLines(
            int currentLineIndex,
            string[] unitTestLogLines,
            DateTime? startTime)
        {
            Contract.Requires(unitTestLogLines != null);
            Contract.Requires(currentLineIndex > 0 && currentLineIndex < unitTestLogLines.Length);
            Contract.Requires(unitTestLogLines[currentLineIndex].Contains(FailureLineStartMarker));
            Contract.Ensures(Contract.Result<UnitTestFailureInformation>() != null);
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.Result<UnitTestFailureInformation>().ErrorMessage));
            Contract.Ensures(startTime == null || Contract.Result<UnitTestFailureInformation>().StartTime >= startTime);
            Contract.Ensures(startTime == null || Contract.Result<UnitTestFailureInformation>().StartTime <= Contract.Result<UnitTestFailureInformation>().EndTime);

            Logger.Verbose("Reading a failure from a line block at line '{0}'.", currentLineIndex);

            UnitTestFailureInformation failureInformation = new UnitTestFailureInformation();

            if (startTime != null)
            {
                // The previous line will hold the time when the test was started (Running 'jshost...').
                failureInformation.StartTime = ReadTimeStampFromLine(unitTestLogLines[currentLineIndex - 1], (DateTime)startTime);
            }

            // The next line will hold information about the host used and parameters passed (jdtest.exe -dbgbaseline:interleavedStack...).
            string fullCommandLine = string.Empty;
            string hostType = string.Empty;
            string commandLineParameters = string.Empty;
            string unitTestFilePath = string.Empty;

            try
            {
                // If the log aborted or timed out, the next line may not be there in the form we expect.
                // As a result, we need to check for any exceptions that occur during parsing.
                ReadRunInformationFromLine(
                    unitTestLogLines[currentLineIndex + 1],
                    out fullCommandLine,
                    out hostType,
                    out commandLineParameters,
                    out unitTestFilePath);
            }
            catch
            {
                // Exception occurred, so we may have hit a case of timeout or abortion of the run.
                if (!WasJobTimedOutOrAborted(unitTestLogLines, currentLineIndex))
                {
                    // No timeout occurred so we probably have a bug.
                    throw;
                }
            }

            // Read the entire contents of the failure.
            StringBuilder errorStringBuilder = new StringBuilder();
            for (int i = currentLineIndex - 1; i < unitTestLogLines.Length; ++i)
            {
                string currentLine = unitTestLogLines[i];
                errorStringBuilder.AppendLine(currentLine);

                if (currentLine.Contains(FailureLineEndMarker))
                {
                    if (startTime != null)
                    {
                        // Found the end of the error block so read the ending timestamp
                        // before exiting.
                        failureInformation.EndTime = ReadTimeStampFromLine(currentLine, (DateTime)startTime);
                    }

                    break;
                }
                else if (currentLine.Contains(TimedOutMarker))
                {
                    // If the job timed out in the middle of writing the failure, just stop here.
                    failureInformation.EndTime = failureInformation.StartTime;
                    break;
                }
            }

            failureInformation.FullCommandLine = fullCommandLine;
            failureInformation.HostType = hostType;
            failureInformation.CommandLineParameters = commandLineParameters;
            failureInformation.UnitTestFileName = unitTestFilePath != string.Empty ? Path.GetFileName(unitTestFilePath) : unitTestFilePath;
            failureInformation.ErrorMessage = errorStringBuilder.ToString();

            Logger.Verbose("Finished reading a failure from a line block at line '{0}'.", currentLineIndex);
            return failureInformation;
        }

        /// <summary>
        /// Scans to confirm whether or not the log indicates that the job was timed out
        /// or aborted (searches for "TIMED OUT").
        /// </summary>
        /// <param name="unitTestLogLines">The unit test log lines.</param>
        /// <param name="lineIndex">Index of the line to start scanning from.</param>
        /// <returns><c>true</c> if a timeout marker was found in the log; <c>false</c> otherwise.</returns>
        private static bool WasJobTimedOutOrAborted(string[] unitTestLogLines, int lineIndex)
        {
            Contract.Requires(unitTestLogLines != null);
            Contract.Requires(lineIndex >= 0 && lineIndex <= unitTestLogLines.Length);

            for (int i = lineIndex; i < unitTestLogLines.Length; ++i)
            {
                if (unitTestLogLines[i].Contains(TimedOutMarker))
                {
                    return true;
                }

                // TODO: Add support for aborted logs.
            }

            return false;
        }

        /// <summary>
        /// Reads the run information from the passed in line
        /// (ex. &gt; ... jshost.exe -q ... C:\file ...).
        /// </summary>
        /// <param name="line">The line to read the run information from.</param>
        /// <param name="fullCommandLine">The full command line statement used to run the test (host + parameters + file).</param>
        /// <param name="hostType">The type of the host that was used for the run (jshost, jdtest, etc.).</param>
        /// <param name="commandLineParameters">The command line parameters passed to the test.</param>
        /// <param name="unitTestFilePath">The path to the unit test file that was run on the local machine.</param>
        private static void ReadRunInformationFromLine(
            string line,
            out string fullCommandLine,
            out string hostType,
            out string commandLineParameters,
            out string unitTestFilePath)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(line));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn(out fullCommandLine)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn(out hostType)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn(out commandLineParameters)));
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.ValueAtReturn(out unitTestFilePath)));

            Logger.Verbose("Reading run information from line '{0}'.", line);

            Match match = TestRunRegex.Match(line);
            Contract.Assert(match.Success);
            Contract.Assert(match.Captures.Count == 1);
            Contract.Assert(match.Groups.Count == 4);

            // The first group is for the entire matched string.
            fullCommandLine = match.Groups[0].Value;
            hostType = match.Groups[1].Value;
            commandLineParameters = match.Groups[2].Value;
            unitTestFilePath = match.Groups[3].Value;

            Logger.Verbose("Finished reading run information from line '{0}'.", line);
        }

        /// <summary>
        /// Reads the timestamp from passed in line (expected starting line format is 00.00:).
        /// </summary>
        /// <param name="line">The line to read the timestamp from.</param>
        /// <param name="startTime">The time when the UTs started running.</param>
        /// <returns>The timestamp when the log line was logged.</returns>
        private static DateTime ReadTimeStampFromLine(string line, DateTime startTime)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(line));
            Contract.Ensures(Contract.Result<DateTime>() >= startTime);

            Logger.Verbose("Reading time stamp from line '{0}' with start time '{1}'.", line, startTime);

            string timeStampString = line.Substring(0, line.IndexOf(':'));
            string[] minutesThenSecondsPair = timeStampString.Split('.');
            Contract.Assert(minutesThenSecondsPair.Length == 2);

            TimeSpan elapsedTimeSinceStart = new TimeSpan(
                0,                                      // Hours
                int.Parse(minutesThenSecondsPair[0]),   // Minutes
                int.Parse(minutesThenSecondsPair[1]));  // Seconds

            DateTime timeStamp = startTime + elapsedTimeSinceStart;

            Logger.Verbose(
                "Finished reading time stamp of '{0}' from line '{1}' with start time '{2}'.",
                timeStamp,
                line,
                startTime);

            return timeStamp;
        }

        /// <summary>
        /// Reads the current variant from the line passed in
        /// (ex. "############# Starting interpreted variant #############").
        /// </summary>
        /// <param name="line">The line to read the variant from.</param>
        /// <returns>The variant stored in the line.</returns>
        private static string ReadVariantFromLine(string line)
        {
            ValidateLineIsValidAndContainsMarker(line, VariantLineMarker);
            Contract.Ensures(!string.IsNullOrWhiteSpace(Contract.Result<string>()));

            Logger.Verbose("Reading variant from line '{0}'.", line);

            int startIndex = line.IndexOf(VariantLineMarker) + VariantLineMarker.Length;
            string variant = line.Substring(startIndex, line.IndexOf(' ', startIndex) - startIndex);

            Logger.Verbose("Finished reading variant '{0}' from line '{1}'.", variant, line);
            return variant;
        }

        /// <summary>
        /// Validates that the passed in line is not null or whitespace and
        /// that the expected marker is present in the line.
        /// </summary>
        /// <param name="line">The line to validate.</param>
        /// <param name="expectedMarker">The expected marker to search for in the line.</param>
        [ContractAbbreviator]
        private static void ValidateLineIsValidAndContainsMarker(string line, string expectedMarker)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(line));
            Contract.Requires(line.Contains(expectedMarker));
        }
    }
}
