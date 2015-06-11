//-----------------------------------------------------------------------
// <copyright file="ProgramTests.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry.Test
{
    using System.Threading.Tasks;
    using ChakraUnitTestTelemetry.Fakes;
    using Microsoft.QualityTools.Testing.Fakes;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Represents unit tests for testing the <see cref="Program"/> class.
    /// </summary>
    [TestClass]
    public class ProgramTests
    {
        /// <summary>
        /// Tests that UploadSnapLogsInformationToTelemetryWebService is called (only once) when specifying
        /// the -snap command line parameter.  Also checks that the return value is success.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithSnapParameter_UploadSnapLogsInformationToTelemetryWebServiceIsCalledAndResultIsSuccess()
        {
            using (ShimsContext.Create())
            {
                bool wasCalled = false;

                // Shim out UploadSnapLogsInformationToTelemetryWebService so we can tell if it was called.
                ShimUnitTestLogsWorker.UploadSnapLogsInformationToTelemetryWebServiceAsync =
                    () =>
                    {
                        Assert.IsFalse(wasCalled, "UploadSnapLogsInformationToTelemetryWebService was called more than once.");
                        return Task.Run(() => wasCalled = true);
                    };

                // Call main.
                string[] commandLineArgs = new string[] { "-snap" };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(wasCalled, "UploadSnapLogsInformationToTelemetryWebServiceAsync was never called.");
                Assert.AreEqual(result, ErrorCode.Success, "Invalid return code.");
            }
        }

        /// <summary>
        /// Tests that UploadUserLogsInformationToTelemetryWebService is called (only once) when specifying
        /// the -user command line parameter.  Also checks that the return value is success.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithUserParameter_UploadUserLogsInformationToTelemetryWebServiceIsCalledAndResultIsSuccess()
        {
            using (ShimsContext.Create())
            {
                bool wasCalled = false;

                // Shim out UploadSnapLogsInformationToTelemetryWebService so we can tell if it was called.
                ShimUnitTestLogsWorker.UploadUserLogsInformationToTelemetryWebServiceAsync =
                    () =>
                    {
                        Assert.IsFalse(wasCalled, "UploadUserLogsInformationToTelemetryWebServiceAsync was called more than once.");
                        return Task.Run(() => wasCalled = true);
                    };

                // Call main.
                string[] commandLineArgs = new string[] { "-user" };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(wasCalled, "UploadUserLogsInformationToTelemetryWebServiceAsync was never called.");
                Assert.AreEqual(result, ErrorCode.Success, "Invalid return code.");
            }
        }

        /// <summary>
        /// Tests that UploadSnapLogsInformationToTelemetryWebService and UploadUserLogsInformationToTelemetryWebService
        /// are each called (only once) when specifying the -snap and -user command line parameters.
        /// Also checks that the return value is success.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithSnapAndUserParameter_BothUploadApisAreCalledAndResultIsSuccess()
        {
            using (ShimsContext.Create())
            {
                bool snapWasCalled = false;

                // Shim out UploadSnapLogsInformationToTelemetryWebService so we can tell if it was called.
                ShimUnitTestLogsWorker.UploadSnapLogsInformationToTelemetryWebServiceAsync =
                    () =>
                    {
                        Assert.IsFalse(snapWasCalled, "UploadSnapLogsInformationToTelemetryWebService was called more than once.");
                        return Task.Run(() => snapWasCalled = true);
                    };

                bool userWasCalled = false;

                // Shim out UploadSnapLogsInformationToTelemetryWebService so we can tell if it was called.
                ShimUnitTestLogsWorker.UploadUserLogsInformationToTelemetryWebServiceAsync =
                    () =>
                    {
                        Assert.IsFalse(userWasCalled, "UploadUserLogsInformationToTelemetryWebServiceAsync was called more than once.");
                        return Task.Run(() => userWasCalled = true);
                    };

                // Call main.
                string[] commandLineArgs = new string[] { "-snap", "-user" };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(snapWasCalled, "UploadSnapLogsInformationToTelemetryWebServiceAsync was never called.");
                Assert.IsTrue(userWasCalled, "UploadUserLogsInformationToTelemetryWebServiceAsync was never called.");
                Assert.AreEqual(result, ErrorCode.Success, "Invalid return code.");
            }
        }

        /// <summary>
        /// Tests that WriteHelpStringToConsole is called (only once) when specifying
        /// the /? command line parameter.  Also checks that the return value is success.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithSlashQuestionParameter_WriteHelpStringToConsoleIsCalledAndResultIsSuccess()
        {
            using (ShimsContext.Create())
            {
                bool wasCalled = false;

                // Shim out WriteHelpStringToConsole so we can tell if it was called.
                ShimProgram.WriteHelpStringToConsole =
                    () =>
                    {
                        Assert.IsFalse(wasCalled, "WriteHelpStringToConsole was called more than once.");
                        wasCalled = true;
                    };

                // Call main.
                string[] commandLineArgs = new string[] { "/?" };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(wasCalled, "WriteHelpStringToConsole was never called");
                Assert.AreEqual(result, ErrorCode.Success, "Invalid return code.");
            }
        }

        /// <summary>
        /// Tests that WriteHelpStringToConsole is called (only once) when specifying
        /// the -? command line parameter.  Also checks that the return value is success.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithDashQuestionParameter_WriteHelpStringToConsoleIsCalledAndResultIsSuccess()
        {
            using (ShimsContext.Create())
            {
                bool wasCalled = false;

                // Shim out WriteHelpStringToConsole so we can tell if it was called.
                ShimProgram.WriteHelpStringToConsole =
                    () =>
                    {
                        Assert.IsFalse(wasCalled, "WriteHelpStringToConsole was called more than once.");
                        wasCalled = true;
                    };

                // Call main.
                string[] commandLineArgs = new string[] { "-?" };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(wasCalled, "WriteHelpStringToConsole was never called");
                Assert.AreEqual(result, ErrorCode.Success, "Invalid return code.");
            }
        }

        /// <summary>
        /// Tests that WriteHelpStringToConsole is called (only once) when specifying
        /// invalid command line parameters.  Also checks that the return value indicates invalid arguments.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithInvalidParameters_WriteHelpStringToConsoleIsCalledAndResultIsInvalidArguments()
        {
            using (ShimsContext.Create())
            {
                bool wasCalled = false;

                // Shim out WriteHelpStringToConsole so we can tell if it was called.
                ShimProgram.WriteHelpStringToConsole =
                    () =>
                    {
                        Assert.IsFalse(wasCalled, "WriteHelpStringToConsole was called more than once.");
                        wasCalled = true;
                    };

                // Call main.
                string[] commandLineArgs = new string[] { "-badparameter" };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(wasCalled, "WriteHelpStringToConsole was never called");
                Assert.AreEqual(result, ErrorCode.InvalidArguments, "Invalid return code.");
            }
        }

        /// <summary>
        /// Tests that WriteHelpStringToConsole is called (only once) when specifying
        /// no command line parameters.  Also checks that the return value indicates invalid arguments.
        /// </summary>
        [TestMethod]
        public void Program_Main_WhenCalledWithNoParameters_WriteHelpStringToConsoleIsCalledAndResultIsInvalidArguments()
        {
            using (ShimsContext.Create())
            {
                bool wasCalled = false;

                // Shim out WriteHelpStringToConsole so we can tell if it was called.
                ShimProgram.WriteHelpStringToConsole =
                    () =>
                    {
                        Assert.IsFalse(wasCalled, "WriteHelpStringToConsole was called more than once.");
                        wasCalled = true;
                    };

                // Call main.
                string[] commandLineArgs = new string[] { };
                int result = Program.Main(commandLineArgs);

                Assert.IsTrue(wasCalled, "WriteHelpStringToConsole was never called");
                Assert.AreEqual(result, ErrorCode.InvalidArguments, "Invalid return code.");
            }
        }
    }
}
