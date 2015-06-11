// <copyright file="RunnerHelper.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <summary>
// Contains helper class for running processes
// </summary>

namespace JsEtwConsole
{
    using System;
    using System.Diagnostics;
    using System.Text;
    using System.Threading;

    /// <summary>
    /// Represents helper class for running processes
    /// </summary>
    public static class RunnerHelper
    {
        #region Methods

        #region Public Methods
        /// <summary>
        /// Run process
        /// </summary>
        /// <param name="executableFolderName">start folder</param>
        /// <param name="executableFileName">executable name to start</param>
        /// <param name="arguments">optional arguments</param>
        /// <param name="executionTimeout">optional execution timeout</param>
        /// <param name="consoleOut">output gathered post execution on stdout</param>
        /// <param name="errorOut">output gathered post execution on stderr</param>
        /// <returns>errorlevel if successful, null otherwise</returns>
        /// <exception cref="System.ArgumentNullException">thrown when either executableFolderName or executableFileName is null or empty</exception>
        public static int? RunProcess(
            string executableFolderName, 
            string executableFileName,
            string arguments, 
            TimeSpan? executionTimeout, 
            out string consoleOut, 
            out string errorOut)
        {
            int? errorCode = null;
            consoleOut = errorOut = String.Empty;
            StringBuilder outConsole = new StringBuilder();
            StringBuilder outError = new StringBuilder();

            if (String.IsNullOrEmpty(executableFolderName))
            {
                throw new ArgumentNullException("executableFolderName");
            }

            if (String.IsNullOrEmpty(executableFileName))
            {
                throw new ArgumentNullException("executableFileName");
            }

            Process process = null;

            try
            {
                process = new Process();

                ProcessStartInfo startInfo = new ProcessStartInfo(executableFileName, arguments);
                startInfo.CreateNoWindow = true;
                startInfo.WorkingDirectory = executableFolderName;
                startInfo.RedirectStandardError = true;
                startInfo.RedirectStandardOutput = true;
                startInfo.UseShellExecute = false;

                process.StartInfo = startInfo;

                process.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e)
                {
                    if (e != null && e.Data != null)
                    {
                        outConsole.AppendLine(e.Data);
                    }
                };

                process.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs e)
                {
                    if (e != null && e.Data != null)
                    {
                        outError.AppendLine(e.Data);
                    }
                };

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                bool exited = true;

                if (executionTimeout.HasValue)
                {
                    exited = process.WaitForExit((int)executionTimeout.Value.TotalMilliseconds);
                }
                else
                {
                    process.WaitForExit();
                }

                // trace output if failure
                if (exited == false)
                {
                    // kill
                    process.Kill();
                }
                else
                {
                    errorCode = process.ExitCode;
                }

                consoleOut = outConsole.ToString();
                errorOut = outError.ToString();
            }
            finally
            {
                if (process != null)
                {
                    process.Dispose();
                    process = null;
                }
            }

            return errorCode;
        }
        #endregion // Public Methods

        #region Private Methods
        #endregion // Private Methods

        #endregion // Methods

        #region Event Handlers
        #endregion // Event Handlers
    }
}
