//-----------------------------------------------------------------------
// <copyright file="UnitTestFailureInformation.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraTelemetryWebService
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.Serialization;
    using System.Web;

    /// <summary>
    /// Represents the data contract for unit test failure information.
    /// </summary>
    [DataContract]
    public class UnitTestFailureInformation : IExtensibleDataObject
    {
        /// <summary>
        /// The extension data object used for preserving future added
        /// items during round tripping with older clients.
        /// </summary>
        private ExtensionDataObject extensionDataObject;

        /// <summary>
        /// Gets or sets the name of the user who did the UT run.
        /// </summary>
        [DataMember]
        public string UserName
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets a value indicating whether or not this was a SNAP run.
        /// </summary>
        [DataMember]
        public bool WasRunInSnap
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the time when the test was started.
        /// </summary>
        [DataMember]
        public DateTime StartTime
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the time when the test finished.
        /// </summary>
        [DataMember]
        public DateTime EndTime
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the duration of the UT run.
        /// </summary>
        [DataMember]
        public TimeSpan Duration
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the full command line that was passed for the UT run.
        /// </summary>
        [DataMember]
        public string FullCommandLine
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the variant of test that failed (dynapogo, interpreted, etc.).
        /// </summary>
        [DataMember]
        public string Variant
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the flags that were used in the test run (passed to the test host).
        /// </summary>
        [DataMember]
        public string CommandLineParameters
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the name of the unit test file that the failure occurred in.
        /// </summary>
        [DataMember]
        public string UnitTestFileName
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the name of the test host that the failure occurred in.
        /// </summary>
        [DataMember]
        public string HostType
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the text of the error that was encountered when the test failed.
        /// </summary>
        [DataMember]
        public string ErrorMessage
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the machine name that the error occurred on.
        /// </summary>
        [DataMember]
        public string MachineName
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the version of the operating system that did the run.
        /// </summary>
        [DataMember]
        public string OperatingSystemVersion
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the absolute file path to where the log originated from.
        /// </summary>
        [DataMember]
        public string LogFilePath
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the job ID that was assigned to the UT run in SNAP.  If
        /// <paramref name="UnitTestFailureInformation.WasRunInSnap"/> is false,
        /// this value will be 0.
        /// </summary>
        [DataMember]
        public int SnapJobId
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the check-in ID that was assigned to the UT run in SNAP.  If
        /// <paramref name="UnitTestFailureInformation.WasRunInSnap"/> is false,
        /// this value will be 0.
        /// </summary>
        [DataMember]
        public string SnapCheckinId
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the build architecture of the binaries that the unit test was
        /// run against (x86, arm, etc.).
        /// </summary>
        [DataMember]
        public string BuildArchitecture
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the build type of the binaries that the unit test was run
        /// against (chk, fre, etc.).
        /// </summary>
        [DataMember]
        public string BuildType
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the name of the branch that the failure occurred in
        /// (FBL_IE_SCRIPT, etc.).
        /// </summary>
        [DataMember]
        public string BranchName
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the structure that contains extra data (for round-tripping support).
        /// </summary>
        /// <returns>An <see cref="T:System.Runtime.Serialization.ExtensionDataObject" /> that contains data that is not recognized as belonging to the data contract.</returns>
        public ExtensionDataObject ExtensionData
        {
            get
            {
                return this.extensionDataObject;
            }

            set
            {
                this.extensionDataObject = value;
            }
        }
    }
}