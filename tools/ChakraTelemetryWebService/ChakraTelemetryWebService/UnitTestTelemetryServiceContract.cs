//-----------------------------------------------------------------------
// <copyright file="UnitTestTelemetryServiceContract.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraTelemetryWebService
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.Linq;
    using System.Web;

    /// <summary>
    /// Represents the code contract that all classes which inherit from
    /// <see cref="IUnitTestTelemetryService"/> must adhere to.
    /// </summary>
    [ContractClassFor(typeof(IUnitTestTelemetryService))]
    public abstract class UnitTestTelemetryServiceContract : IUnitTestTelemetryService
    {
        /// <summary>
        /// Records a unit test failure that occurred.
        /// </summary>
        /// <param name="failureInformation">The failure information.</param>
        void IUnitTestTelemetryService.RecordUnitTestFailure(UnitTestFailureInformation failureInformation)
        {
            Contract.Requires(failureInformation != null);
        }
    }
}