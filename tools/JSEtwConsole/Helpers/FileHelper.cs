// <copyright file="FileHelper.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;
    using System.IO;
    using System.Threading;

    /// <summary>
    /// Represents helper logic to interact with the file system.
    /// </summary>
    public static class FileHelper
    {
        #region Constants
        /// <summary>
        /// Delay between delete retries
        /// </summary>
        private static readonly TimeSpan DelayBetweenRetries = new TimeSpan(0, 0, 0, 1, 0);

        /// <summary>
        /// Number of retries
        /// </summary>
        private const int NumberRetries = 30;
        #endregion // Constants

        #region Methods

        #region Public Methods
        /// <summary>
        /// Try moving a file if present (with retries)
        /// </summary>
        /// <param name="sourceFileFullName">file to be moved</param>
        /// <param name="destinationFileName">file to be moved to</param>
        /// <returns>Whether the operation was successful.</returns>
        public static bool TryMoveFile(string sourceFileFullName, string destinationFileName)
        {
            if (String.IsNullOrEmpty(sourceFileFullName) == false && String.IsNullOrEmpty(destinationFileName) == false && File.Exists(sourceFileFullName))
            {
                int retryNumber = 0;
                while (retryNumber < FileHelper.NumberRetries && String.IsNullOrEmpty(sourceFileFullName) == false && File.Exists(sourceFileFullName))
                {
                    try
                    {
                        File.Move(sourceFileFullName, destinationFileName);
                        return true;
                    }
                    catch (System.UnauthorizedAccessException)
                    {
                        // shallow
                    }
                    catch (IOException)
                    {
                        // shallow
                    }

                    Thread.Sleep(FileHelper.DelayBetweenRetries);
                    retryNumber++;
                }
            }

            return false;
        }

        /// <summary>
        /// Try deleting a file if present (with retries)
        /// </summary>
        /// <param name="fileFullName">file to be deleted</param>
        /// <returns>Whether the operation was successful.</returns>
        public static bool TryDeleteFile(string fileFullName)
        {
            if (String.IsNullOrEmpty(fileFullName) == false && File.Exists(fileFullName))
            {
                int retryNumber = 0;
                while (retryNumber < FileHelper.NumberRetries && String.IsNullOrEmpty(fileFullName) == false && File.Exists(fileFullName))
                {
                    try
                    {
                        File.SetAttributes(fileFullName, FileAttributes.Normal);
                        File.Delete(fileFullName);
                        return true;
                    }
                    catch (System.UnauthorizedAccessException)
                    {
                        // shallow
                    }
                    catch (IOException)
                    {
                        // shallow
                    }

                    Thread.Sleep(FileHelper.DelayBetweenRetries);
                    retryNumber++;
                }
            }

            return false;
        }

        /// <summary>
        /// Try deleting a folder if present
        /// </summary>
        /// <param name="folderFullName">folder to be deleted</param>
        /// <returns>Whether the operation was successful.</returns>
        public static bool TryDeleteFolder(string folderFullName)
        {
            if (String.IsNullOrEmpty(folderFullName) == false && Directory.Exists(folderFullName))
            {
                FileHelper.DeleteFolder(folderFullName, true);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Deleting a folder if present (with retry logic)
        /// </summary>
        /// <param name="fullName">folder to be deleted</param>
        /// <param name="ignoreFailure">whether a failure is to be ignored</param>
        public static void DeleteFolder(string fullName, bool ignoreFailure)
        {
            if (Directory.Exists(fullName))
            {
                int retryNumber = 0;
                while (retryNumber < FileHelper.NumberRetries && Directory.Exists(fullName))
                {
                    try
                    {
                        // try deleting the folder directly
                        try
                        {
                            Directory.Delete(fullName, true);
                            break;
                        }
                        catch (Exception)
                        {
                            // shallow
                        }

                        string[] files = Directory.GetFiles(fullName, "*.*", SearchOption.AllDirectories);
                        foreach (string file in files)
                        {
                            File.SetAttributes(file, FileAttributes.Normal);
                            File.Delete(file);
                        }

                        if (Directory.Exists(fullName))
                        {
                            Directory.Delete(fullName, true);
                        }

                        break;
                    }
                    catch (Exception)
                    {
                        if (ignoreFailure)
                        {
                            break;
                        }
                    }

                    Thread.Sleep(FileHelper.DelayBetweenRetries);
                    retryNumber++;
                }
            }
        }

        /// <summary>
        /// Tries the read all bytes.
        /// </summary>
        /// <param name="fileName">Name of the file.</param>
        /// <param name="bytes">The resulting bytes.</param>
        /// <returns>Whether the operation was successful.</returns>
        public static bool TryReadAllBytes(string fileName, out byte[] bytes)
        {
            var exists = File.Exists(fileName);
            try
            {
                bytes = exists ? File.ReadAllBytes(fileName) : null;
            }
            catch
            {
                bytes = null;
                return false;
            }

            return exists;
        }
        #endregion // Public Methods

        #endregion // Methods
    }
}
