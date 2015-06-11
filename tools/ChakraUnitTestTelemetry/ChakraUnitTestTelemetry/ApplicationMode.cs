//-----------------------------------------------------------------------
// <copyright file="ApplicationModes.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    /// <summary>
    /// Represents the various application modes that the
    /// application can run in (different modes for
    /// collecting log information).
    /// </summary>
    [Flags]
    public enum ApplicationModes
    {
        /// <summary>
        /// No application mode has been set yet.
        /// </summary>
        None = 0x0,

        /// <summary>
        /// The application is set to show the help documentation to the user.
        /// </summary>
        ShowHelp = 0x1,

        /// <summary>
        /// The application is set to scan for snap unit test logs on the snap logs share.
        /// </summary>
        ScanningForSnapShareLogs = 0x2,

        /// <summary>
        /// The application is set to scan for logs copied to a share from users' machines.
        /// </summary>
        ScanningForUserShareLogs = 0x4
    }
}
