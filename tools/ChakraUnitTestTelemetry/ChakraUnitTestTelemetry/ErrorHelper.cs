//-----------------------------------------------------------------------
// <copyright file="ErrorHelper.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.IO;
    using System.Text;
    using System.Threading;

    /// <summary>
    /// Represents helper methods for dealing with error situations.
    /// </summary>
    public static class ErrorHelper
    {
        /// <summary>
        /// Dumps the exception to a file path along with information about the current
        /// environment that the machine occurred on.
        /// </summary>
        /// <param name="exception">The exception to dump to file.</param>
        /// <param name="filePath">The file path to dump to.</param>
        public static void DumpExceptionAndEnvironmentInformationToFile(Exception exception, string filePath)
        {
            Contract.Requires(exception != null);
            Contract.Requires(!string.IsNullOrWhiteSpace(filePath));

            const string FileContentsFormat =
@"ChakraUnitTestTelemetry Unhandled Exception Information

User                    : {0}\{1}
Computer Name           : {2}
Operating System        : {3}
Exception               :

{4}

Environment Variables   :

{5}

Trace Logs for All Threads:

{6}";

            string fileContents = string.Format(
                FileContentsFormat,
                Environment.UserDomainName,
                Environment.UserName,
                Environment.MachineName,
                Environment.OSVersion,
                exception,
                GetEnvironmentVariablesAsString(),
                Logger.GetTraceLogs());

            File.WriteAllText(filePath, fileContents);

            Contract.Assert(File.Exists(filePath));
        }

        /// <summary>
        /// Dumps the trace logs for each thread to a separate file per thread.
        /// </summary>
        /// <param name="filePath">The file path of the exception log that was dumped (will be modified per thread file).</param>
        public static void DumpThreadTraceLogsToFiles(string filePath)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(filePath));

            foreach (KeyValuePair<int, string> threadTracePair in Logger.GetPerThreadTraceLogs())
            {
                int threadId = threadTracePair.Key;
                string traceContents = threadTracePair.Value;

                string newFilePath = Path.Combine(
                    Path.GetDirectoryName(filePath),
                    Path.GetFileNameWithoutExtension(filePath) + string.Format("_thread_{0:00000000}", threadId) + Path.GetExtension(filePath));

                File.WriteAllText(newFilePath, traceContents);
            }
        }

        /// <summary>
        /// Gets the environment variables as a string list (newline delimited).
        /// </summary>
        /// <returns>The list of environment variables with their values, each on a new line.</returns>
        private static string GetEnvironmentVariablesAsString()
        {
            Contract.Ensures(Contract.Result<string>() != null);

            StringBuilder builder = new StringBuilder();
            foreach (DictionaryEntry entry in Environment.GetEnvironmentVariables())
            {
                builder.AppendLine(string.Format("{0}: {1}", entry.Key, entry.Value));
            }

            return builder.ToString();
        }
    }
}
