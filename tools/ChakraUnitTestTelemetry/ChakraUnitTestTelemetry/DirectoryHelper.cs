//-----------------------------------------------------------------------
// <copyright file="DirectoryHelper.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    using System;
    using System.Diagnostics.Contracts;
    using System.IO;

    /// <summary>
    /// Represents helper methods for working with directories.
    /// </summary>
    public static class DirectoryHelper
    {
        /// <summary>
        /// Copies the contents (files and folders) of the source directory to the destination directory.
        /// Will overwrite existing files if they are present.
        /// </summary>
        /// <param name="sourceDirectory">The source directory to copy from.</param>
        /// <param name="destinationDirectory">The destination directory to copy to.</param>
        /// <param name="shouldCopySubDirectories">If set to <c>true</c>, recursive sub directories will also be copied.</param>
        /// <exception cref="System.ArgumentNullException">Thrown if <paramref name="sourceDirectory"/> or <paramref name="destinationDirectory"/> is <c>null</c> or whitespace.</exception>
        public static void Copy(string sourceDirectory, string destinationDirectory, bool shouldCopySubDirectories = true)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(sourceDirectory));
            Contract.Requires(!string.IsNullOrWhiteSpace(destinationDirectory));

            // Copy the files at this level.
            string[] sourceFilePaths = Directory.GetFiles(sourceDirectory);
            foreach (string sourceFilePath in sourceFilePaths)
            {
                string destinationFileName = Path.GetFileName(sourceFilePath);
                string destinationFilePath = Path.Combine(destinationDirectory, destinationFileName);
                File.Copy(sourceFilePath, destinationFilePath, true /*overwrite*/);
            }

            // Copy the sub directories.
            string[] sourceSubDirectories = Directory.GetDirectories(sourceDirectory);

            Contract.Assert(File.Exists(destinationDirectory));
        }

        /// <summary>
        /// Creates the directory if it doesn't exist already.
        /// </summary>
        /// <param name="directoryPath">The directory path to create.</param>
        /// <exception cref="System.ArgumentNullException">Thrown if <paramref name="directoryPath"/> is <c>null</c> or whitespace.</exception>
        public static void CreateDirectoryIfItDoesntExist(string directoryPath)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(directoryPath));

            if (!Directory.Exists(directoryPath))
            {
                Directory.CreateDirectory(directoryPath);
            }

            Contract.Assert(Directory.Exists(directoryPath));
        }
    }
}
