//-----------------------------------------------------------------------
// <copyright file="ErrorCode.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    /// <summary>
    /// Represents error codes returned by the application.
    /// </summary>
    public static class ErrorCode
    {
        /// <summary>
        /// The application completed processing successfully.
        /// </summary>
        public const int Success = 0;

        /// <summary>
        /// An unhandled exception occurred.
        /// </summary>
        public const int UnhandledExceptionOccurred = -1;

        /// <summary>
        /// The user supplied invalid arguments to the application.
        /// </summary>
        public const int InvalidArguments = -2;
    }
}
