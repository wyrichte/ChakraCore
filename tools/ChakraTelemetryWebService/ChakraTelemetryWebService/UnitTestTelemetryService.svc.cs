//-----------------------------------------------------------------------
// <copyright file="UnitTestTelemetryService.svc.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraTelemetryWebService
{
    using System;
    using System.Diagnostics.Contracts;
    using System.ServiceModel;

    /// <summary>
    /// Represents the web service used for recording information about Chakra unit test failures.
    /// </summary>
    public class UnitTestTelemetryService : IUnitTestTelemetryService
    {
        /// <summary>
        /// Records a unit test failure that occurred.
        /// </summary>
        /// <param name="failureInformation">The failure information.</param>
        public void RecordUnitTestFailure(UnitTestFailureInformation failureInformation)
        {
            ChakraTelemetryDatabaseEntities entities = new ChakraTelemetryDatabaseEntities();
            int result = entities.RecordUnitTestFailure(
                failureInformation.UserName,
                failureInformation.WasRunInSnap,
                failureInformation.StartTime,
                failureInformation.EndTime,
                failureInformation.Duration,
                failureInformation.FullCommandLine,
                failureInformation.Variant,
                failureInformation.CommandLineParameters,
                failureInformation.UnitTestFileName,
                failureInformation.HostType,
                failureInformation.ErrorMessage,
                failureInformation.MachineName,
                failureInformation.OperatingSystemVersion,
                failureInformation.LogFilePath,
                failureInformation.SnapJobId,
                failureInformation.SnapCheckinId,
                failureInformation.BuildArchitecture,
                failureInformation.BuildType,
                failureInformation.BranchName);

            Contract.Assert(result == 0);
        }
    }
}
