//-----------------------------------------------------------------------
// <copyright file="IUnitTestTelemetryService.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraTelemetryWebService
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.Linq;
    using System.Runtime.Serialization;
    using System.ServiceModel;
    using System.ServiceModel.Web;
    using System.Text;

    /// <summary>
    /// Represents the interface contract for the unit test telemetry service.
    /// </summary>
    [ServiceContract]
    [ContractClass(typeof(UnitTestTelemetryServiceContract))]
    public interface IUnitTestTelemetryService
    {
        /// <summary>
        /// Records a unit test failure that occurred.
        /// </summary>
        /// <param name="failureInformation">The failure information.</param>
        [OperationContract]
        void RecordUnitTestFailure(UnitTestFailureInformation failureInformation);
    }
}
