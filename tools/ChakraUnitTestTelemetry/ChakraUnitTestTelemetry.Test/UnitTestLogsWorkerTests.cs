//-----------------------------------------------------------------------
// <copyright file="UnitTestLogsWorkerTests.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry.Test
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.IO.Fakes;
    using System.Linq;
    using System.Threading.Tasks;
    using ChakraUnitTestTelemetry.UnitTestTelemetryServiceReference;
    using Microsoft.QualityTools.Testing.Fakes;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Represents unit tests for testing the <see cref="UnitTestLogsWorker"/> class.
    /// </summary>
    [TestClass]
    public class UnitTestLogsWorkerTests
    {
        /// <summary>
        /// Tests that the machine name is correctly read out of a log line.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadMachineNameFromLine_WhenProvidedLogLine_ReturnsCorrectMachineName()
        {
            string logLine = "08/22/2013 20:30:54:566 [Daemon] v 2.1.5.19 ********Task id 9341515 run On Machine IESNAP-BLD55 ******";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));
            string result = unitTestLogsWorker.InvokeStatic("ReadMachineNameFromLine", logLine) as string;

            Assert.AreEqual("IESNAP-BLD55", result, "The machine name was not returned correctly.");
        }

        /// <summary>
        /// Tests that the start time is correctly read out of a log line.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadStartTimeFromLine_WhenProvidedLogLine_ReturnsCorrectDateAndTime()
        {
            string logLine = "00.00: Logging started at Thu Aug 22 20:31:04 PDT 2013";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));
            DateTime result = (DateTime)unitTestLogsWorker.InvokeStatic("ReadStartTimeFromLine", logLine);

            DateTime expectedResult = new DateTime(
                2013,
                8,
                22,
                20,
                31,
                4,
                DateTimeKind.Local);

            Assert.AreEqual(expectedResult, result, "The start date and time was not returned correctly.");
        }

        /// <summary>
        /// Tests that failure information is correctly read out of a snap log block.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadFailureFromLines_WhenProvidedSnapLogBlock_ReturnsCorrectFailureInformation()
        {
            string logBlock =
@"00.09:     2>Running 'jdtest.exe -q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js >testout26 2>&1'
00.09:     2>ERROR: Test failed to run correctly: diffs from baseline (S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.baseline):
00.09:     2>    jdtest.exe -q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js >testout26 2>&1
00.09:     2>ERROR: name of output file: S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\testout26; size: 439; creation: Thu Aug 22 20:31:08 2013, last access: Thu Aug 22 20:31:08 2013, now: Thu Aug 22 20:31:09 2013
00.09:     2>ERROR: bad output file follows ============
00.09:     2>ASSERTION (inetcore\jscript\lib\runtime\library\engineinterfaceobject.cpp, line 454) Failed to deserialize Intl.js bytecode - very probably the bytecode needs to be rebuilt.
00.09:     2> Failure: (false)
00.09:     2>Unable to write minidump (0x8007001E)
00.09:     2>Unable to write minidump (0x8007001E)
00.11:     2>FATAL ERROR: jshost.exe failed due to exception code 40010004
00.11:     2>ASSERTION (inetcore\jscript\dll\jscriptdiag\dllfunc.h, line 26) GetLockCount() == 0 Com object leaked?
00.11:     2>ERROR: end of bad output file  ============
";

            string[] logLines = logBlock.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));
            UnitTestFailureInformation result = (UnitTestFailureInformation)unitTestLogsWorker.InvokeStatic(
                "ReadFailureFromLines",
                1 /*currentLineIndex*/,
                logLines,
                DateTime.MinValue /*startTime*/);

            DateTime expectedStartTime = DateTime.MinValue + new TimeSpan(0, 0, 9);
            DateTime expectedEndTime = DateTime.MinValue + new TimeSpan(0, 0, 11);

            Assert.AreEqual(
                @"jdtest.exe -q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js",
                result.FullCommandLine,
                "The full command line was not returned correctly.");
            Assert.AreEqual("jdtest", result.HostType, "The host type was not returned correctly.");
            Assert.AreEqual(
                @"-q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid",
                result.CommandLineParameters,
                "The command line parameters were not returned correctly.");
            Assert.AreEqual("locals.js", result.UnitTestFileName, "The test file name was not returned correctly.");
            Assert.AreEqual(logBlock, result.ErrorMessage, "The error block was not returned correctly.");
            Assert.AreEqual(expectedStartTime, result.StartTime, "The start time was not returned correctly.");
            Assert.AreEqual(expectedEndTime, result.EndTime, "The start time was not returned correctly.");
        }

        /// <summary>
        /// Tests that failure information is correctly read out of a user log block.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadFailureFromLines_WhenProvidedUserLogBlock_ReturnsCorrectFailureInformation()
        {
            string logBlock =
@"1>Running 'jshost.exe -bvt   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest86  -jdtest:""-q -c \"".load ut_jsdiag; g;!echo ---1st bp---;!ldsym;!jsstack;!verifyOM;g;    !echo;!echo ---2nd bp---;!ldsym;!jsstack;g\"""" -Off:Inline -Off:JITLoopBody -dynamicprofilecache:profile.dpl.test1.js.1 E:\Code\inetcore\jscript\unittest\jd\test1.js  >testout18 2>&1'
1>ERROR: Test failed to run correctly: diffs from baseline (E:\Code\inetcore\jscript\unittest\jd\test1.baseline):
1>    jshost.exe -bvt   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest86  -jdtest:""-q -c \"".load ut_jsdiag; g;!echo ---1st bp---;!ldsym;!jsstack;!verifyOM;g;    !echo;!echo ---2nd bp---;!ldsym;!jsstack;g\"""" -Off:Inline -Off:JITLoopBody -dynamicprofilecache:profile.dpl.test1.js.1 E:\Code\inetcore\jscript\unittest\jd\test1.js  >testout18 2>&1
1>ERROR: name of output file: E:\Code\inetcore\jscript\unittest\jd\testout18; size: 542; creation: Fri Sep 20 13:06:22 2013, last access: Fri Sep 20 13:06:22 2013, now: Fri Sep 20 13:06:22 2013
1>ERROR: bad output file follows ============
1>---1st bp---
1>F     HRESULT failed: 0x80004005(E_FAIL)=pDAC->LoadScriptSymbols(pSinkDebugSite)
1>      ut_jsdiag.cpp(186): in function Extension::ldsym
1>
1>Printing stacktrace..
1>blah (test1.js:10,21)
1>bar (test1.js:20,9)
1>foo (test1.js:5,6)
1>Global code (test1.js:25,1)
1>
1>---2nd bp---
1>F     HRESULT failed: 0x80004005(E_FAIL)=pDAC->LoadScriptSymbols(pSinkDebugSite)
1>      ut_jsdiag.cpp(186): in function Extension::ldsym
1>
1>Printing stacktrace..
1>blah (test1.js:15,21)
1>bar (test1.js:20,9)
1>foo (test1.js:5,6)
1>Global code (test1.js:25,1)
1>ERROR: end of bad output file  ============
";

            string[] logLines = logBlock.Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));
            UnitTestFailureInformation result = (UnitTestFailureInformation)unitTestLogsWorker.InvokeStatic(
                "ReadFailureFromLines",
                1 /*currentLineIndex*/,
                logLines,
                null /*startTime*/);

            Assert.AreEqual(
                @"jshost.exe -bvt   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest86  -jdtest:""-q -c \"".load ut_jsdiag; g;!echo ---1st bp---;!ldsym;!jsstack;!verifyOM;g;    !echo;!echo ---2nd bp---;!ldsym;!jsstack;g\"""" -Off:Inline -Off:JITLoopBody -dynamicprofilecache:profile.dpl.test1.js.1 E:\Code\inetcore\jscript\unittest\jd\test1.js",
                result.FullCommandLine,
                "The full command line was not returned correctly.");
            Assert.AreEqual("jshost", result.HostType, "The host type was not returned correctly.");
            Assert.AreEqual(
                @"-bvt   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest86  -jdtest:""-q -c \"".load ut_jsdiag; g;!echo ---1st bp---;!ldsym;!jsstack;!verifyOM;g;    !echo;!echo ---2nd bp---;!ldsym;!jsstack;g\"""" -Off:Inline -Off:JITLoopBody -dynamicprofilecache:profile.dpl.test1.js.1",
                result.CommandLineParameters,
                "The command line parameters were not returned correctly.");
            Assert.AreEqual("test1.js", result.UnitTestFileName, "The test file name was not returned correctly.");
            Assert.AreEqual(logBlock, result.ErrorMessage, "The error block was not returned correctly.");
        }

        /// <summary>
        /// Tests that the output parameters are correctly set when parsing the host run log line.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadRunInformationFromLine_WhenProvidedLogLine_ReturnsCorrectOutParameters()
        {
            string logLine = @"00.09:     2>    jdtest.exe -q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js >testout26 2>&1";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            Type[] parameterTypes = new Type[]
            {
                typeof(string),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
            };

            object[] parameters = new object[]
            {
                logLine,
                null, /*out fullCommandLine*/
                null, /*out hostType*/
                null, /*out commandLineParameters*/
                null, /*out unitTestFilePath*/
            };

            Contract.Assert(parameterTypes.Length == parameters.Length);

            unitTestLogsWorker.InvokeStatic(
                "ReadRunInformationFromLine",
                parameterTypes,
                parameters);

            Assert.AreEqual(
                @"jdtest.exe -q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js",
                parameters[1],
                "The full command line returned is incorrect.");
            Assert.AreEqual("jdtest", parameters[2], "The host type returned is incorrect.");
            Assert.AreEqual(
                @"-q -c "".load jd;.load ut_jsdiag;!jd.utmode; g;!bp 0;!bp 100;!bp 200; !stack;!frame; g;!asyncBreak;!stack;!frame; g;!stack;!frame; q"" jshost   -DumpOnCrash -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest94  -DebugLaunch:hybrid",
                parameters[3],
                "The command line parameters returned are invalid.");
            Assert.AreEqual(@"S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\locals.js", parameters[4], "The test file path is incorrect.");
        }

        /// <summary>
        /// Tests that the output parameters are correctly set when parsing the host run log line and the host doesn't
        /// have a file extension.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadRunInformationFromLine_WhenProvidedLogLineWithNoHostExtension_ReturnsCorrectOutParameters()
        {
            string logLine = @"13>    jdtest -q -c "".load ut_jsdiag; g;!jsstack;!echo;!testdump; g;!jsstack;!echo;!testdump; q"" jshost   -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest104  -maxinterpretcount:1 -force:inline -force:jitloopbody S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\dump.js >testout36 2>&1";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            object[] parameters = new object[]
            {
                logLine,
                null, /*out fullCommandLine*/
                null, /*out hostType*/
                null, /*out commandLineParameters*/
                null, /*out unitTestFilePath*/
            };

            Type[] parameterTypes = new Type[]
            {
                typeof(string),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
            };

            Contract.Assert(parameters.Length == parameterTypes.Length);

            unitTestLogsWorker.InvokeStatic("ReadRunInformationFromLine", parameterTypes, parameters);

            Assert.AreEqual(
                @"jdtest -q -c "".load ut_jsdiag; g;!jsstack;!echo;!testdump; g;!jsstack;!echo;!testdump; q"" jshost   -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest104  -maxinterpretcount:1 -force:inline -force:jitloopbody S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\dump.js",
                parameters[1],
                "The host type returned is incorrect.");
            Assert.AreEqual("jdtest", parameters[2], "The host type returned is incorrect.");
            Assert.AreEqual(
                @"-q -c "".load ut_jsdiag; g;!jsstack;!echo;!testdump; g;!jsstack;!echo;!testdump; q"" jshost   -maxinterpretcount:1 -bgjit- -dynamicprofilecache:profile.dpl.UnnamedTest104  -maxinterpretcount:1 -force:inline -force:jitloopbody",
                parameters[3],
                "The command line parameters returned are invalid.");
            Assert.AreEqual(@"S:\FBL_IE_SCRIPT_DEV\inetcore\jscript\unittest\jd\dump.js", parameters[4], "The test file path is incorrect.");
        }

        /// <summary>
        /// Tests that the timestamp on a log line is correctly read out.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadTimeStampFromLine_WhenProvidedLogLine_ReturnsCorrectTimeStamp()
        {
            string logLine = @"00.11:     2>ERROR: end of bad output file  ============";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            DateTime startTime = new DateTime(2013, 8, 26);

            DateTime result = (DateTime)unitTestLogsWorker.InvokeStatic(
                "ReadTimeStampFromLine",
                logLine,
                startTime);

            // Expected time is 11 seconds from the start.
            DateTime expectedTime = startTime + new TimeSpan(0, 0, 11);

            Assert.AreEqual(expectedTime, result, "The returned time is incorrect.");
        }

        /// <summary>
        /// Tests that a variant is correctly read from the log (interpreted, for example).
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadVariantFromLine_WhenProvidedLogLine_ReturnsCorrectVariant()
        {
            string logLine = @"############# Starting interpreted variant #############";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            string result = unitTestLogsWorker.InvokeStatic("ReadVariantFromLine", logLine) as string;

            Assert.AreEqual("interpreted", result, "The returned variant is incorrect.");
        }

        /// <summary>
        /// Tests that the build architecture and type is correctly read from the log (x86 and fre, for example).
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ReadBuildArchitectureAndTypeFromLine_WhenProvidedLogLine_ReturnsCorrectArchitectureAndType()
        {
            string logLine = " Setting up Razzle amd64 fre";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            object[] parameters = new object[]
            {
                logLine,
                null, /*out buildArchitecture*/
                null, /*out buildType*/
            };

            Type[] parameterTypes = new Type[]
            {
                typeof(string),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
            };

            Contract.Assert(parameters.Length == parameterTypes.Length);

            unitTestLogsWorker.InvokeStatic("ReadBuildArchitectureAndTypeFromLine", parameterTypes, parameters);

            Assert.AreEqual("amd64", parameters[1], "The returned build architecture is incorrect.");
            Assert.AreEqual("fre", parameters[2], "The returned build architecture is incorrect.");
        }

        /// <summary>
        /// Tests that a parsing a snap log produces the correct number of failure items.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseSnapLogInformationAsync_WhenProvidedLogFile_ReturnsCorrectNumberOfParsedFailures()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            using (ShimsContext.Create())
            {
                // Create a shim around File.ReadAllLines so that we can return our test file contents.
                ShimFile.ReadAllLinesString =
                    (s) =>
                    {
                        return ResourceBin.SNAP_Failure_FBL_IE_SCRIPT_DEV.Split(
                            new char[] { '\r', '\n' },
                            StringSplitOptions.RemoveEmptyEntries);
                    };

                Task<IEnumerable<UnitTestFailureInformation>> results = unitTestLogsWorker.InvokeStatic(
                    "ParseSnapLogInformationAsync",
                    "FBL_IE_SCRIPT_DEV",
                    @"\\iesnap\queue\FBL_IE_SCRIPT_DEV\90209.t-doilij\Job.112721\RunJScriptUnitTests.IESNAP-BLD38.amd64.fre.log")
                    as Task<IEnumerable<UnitTestFailureInformation>>;

                results.Wait();

                Assert.AreEqual(240, results.Result.Count(), "The returned number of failures is incorrect.");
            }
        }

        /// <summary>
        /// Tests that a parsing a timed out snap log produces the correct number of failure items.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseSnapLogInformationAsync_WhenProvidedTimedOutLogFile_ReturnsCorrectNumberOfParsedFailures()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            using (ShimsContext.Create())
            {
                // Create a shim around File.ReadAllLines so that we can return our test file contents.
                ShimFile.ReadAllLinesString =
                    (s) =>
                    {
                        return ResourceBin.RunJScriptUnitTests_IESNAP_BLD38_amd64_fre.Split(
                            new char[] { '\r', '\n' },
                            StringSplitOptions.RemoveEmptyEntries);
                    };

                Task<IEnumerable<UnitTestFailureInformation>> results = unitTestLogsWorker.InvokeStatic(
                    "ParseSnapLogInformationAsync",
                    "FBL_IE_SCRIPT_DEV",
                    @"\\iesnap\queue\FBL_IE_SCRIPT_DEV\90209.t-doilij\Job.112721\RunJScriptUnitTests.IESNAP-BLD38.amd64.fre.log")
                    as Task<IEnumerable<UnitTestFailureInformation>>;

                results.Wait();

                Assert.AreEqual(697, results.Result.Count(), "The returned number of failures is incorrect.");
            }
        }

        /// <summary>
        /// Tests that a parsing a snap log produces failure information that is properly filled in.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseSnapLogInformationAsync_WhenProvidedLogFile_AllFailureInformationIsFilledIn()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            using (ShimsContext.Create())
            {
                // Create a shim around File.ReadAllLines so that we can return our test file contents.
                ShimFile.ReadAllLinesString =
                    (s) =>
                    {
                        return ResourceBin.SNAP_Failure_FBL_IE_SCRIPT_DEV.Split(
                            new char[] { '\r', '\n' },
                            StringSplitOptions.RemoveEmptyEntries);
                    };

                Task<IEnumerable<UnitTestFailureInformation>> results = unitTestLogsWorker.InvokeStatic(
                    "ParseSnapLogInformationAsync",
                    "FBL_IE_SCRIPT_DEV",
                    @"\\iesnap\queue\FBL_IE_SCRIPT_DEV\90209.t-doilij\Job.112721\RunJScriptUnitTests.IESNAP-BLD38.amd64.fre.log")
                    as Task<IEnumerable<UnitTestFailureInformation>>;

                results.Wait();

                results.Result.All(
                    (f) =>
                    {
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.BranchName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.BuildArchitecture));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.BuildType));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.Variant));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.CommandLineParameters));
                        Assert.IsTrue(f.Duration >= TimeSpan.Zero);
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.ErrorMessage));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.FullCommandLine));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.HostType));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.LogFilePath));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.MachineName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.OperatingSystemVersion));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.UnitTestFileName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.UserName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.SnapCheckinId));
                        Assert.IsTrue(f.SnapJobId > 0);
                        Assert.IsTrue(f.WasRunInSnap);
                        return true;
                    });
            }
        }

        /// <summary>
        /// Tests that a parsing a user log produces failure information that is properly filled in.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseUserLogInformationAsync_WhenProvidedLogFile_AllFailureInformationIsFilledIn()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            using (ShimsContext.Create())
            {
                // Create a shim around File.ReadAllLines so that we can return our test file contents.
                ShimFile.ReadAllLinesString =
                    (s) =>
                    {
                        return ResourceBin.USER_rl_full.Split(
                            new char[] { '\r', '\n' },
                            StringSplitOptions.RemoveEmptyEntries);
                    };

                Task<IEnumerable<UnitTestFailureInformation>> results = unitTestLogsWorker.InvokeStatic(
                    "ParseUserLogInformationAsync",
                    @"\\bpt-scratch\UserFiles\cmorse\Tools\ChakraUnitTestTelemetry\UserLogs\cmorse\cmorse2\fbl_ie_script_dev\x86chk\2013.08.02_16.13.41\interpreted\rl.full.log")
                    as Task<IEnumerable<UnitTestFailureInformation>>;

                results.Wait();

                results.Result.All(
                    (f) =>
                    {
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.BranchName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.BuildArchitecture));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.BuildType));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.Variant));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.CommandLineParameters));
                        Assert.IsTrue(f.Duration >= TimeSpan.Zero);
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.ErrorMessage));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.FullCommandLine));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.HostType));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.LogFilePath));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.MachineName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.OperatingSystemVersion));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.UnitTestFileName));
                        Assert.IsTrue(!string.IsNullOrWhiteSpace(f.UserName));
                        Assert.AreEqual("-1", f.SnapCheckinId);
                        Assert.AreEqual(-1, f.SnapJobId);
                        Assert.IsFalse(f.WasRunInSnap);
                        return true;
                    });
            }
        }

        /// <summary>
        /// Tests that a parsing a user log produces the correct number of failure items.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseUserLogInformationAsync_WhenProvidedLogFile_ReturnsCorrectNumberOfParsedFailures()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            using (ShimsContext.Create())
            {
                // Create a shim around File.ReadAllLines so that we can return our test file contents.
                ShimFile.ReadAllLinesString =
                    (s) =>
                    {
                        return ResourceBin.USER_rl_full.Split(
                            new char[] { '\r', '\n' },
                            StringSplitOptions.RemoveEmptyEntries);
                    };

                Task<IEnumerable<UnitTestFailureInformation>> results = unitTestLogsWorker.InvokeStatic(
                    "ParseUserLogInformationAsync",
                    @"\\bpt-scratch\UserFiles\cmorse\Tools\ChakraUnitTestTelemetry\UserLogs\cmorse\cmorse2\fbl_ie_script_dev\x86chk\2013.08.02_16.13.41\interpreted\rl.full.log")
                    as Task<IEnumerable<UnitTestFailureInformation>>;

                results.Wait();

                Assert.AreEqual(2, results.Result.Count(), "The returned number of failures is incorrect.");
            }
        }

        /// <summary>
        /// Tests that the output parameters are correctly set when parsing the user name, SNAP job ID,
        /// and SNAP check-in ID.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseUserNameAndSnapIdsFromFilePath_WhenProvidedSnapFilePath_ReturnsCorrectOutParameters()
        {
            string logFilePath = @"\\iesnap\queue\FBL_IE_SCRIPT_DEV\90209.t-doilij\Job.112721\RunJScriptUnitTests.IESNAP-BLD38.amd64.fre.log";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            object[] parameters = new object[]
            {
                logFilePath,
                null, /*out userName*/
                null, /*out snapJobId*/
                null, /*out snapCheckinId*/
            };

            Type[] parameterTypes = new Type[]
            {
                typeof(string),
                typeof(string).MakeByRefType(),
                typeof(int).MakeByRefType(),
                typeof(string).MakeByRefType(),
            };

            Contract.Assert(parameters.Length == parameterTypes.Length);

            unitTestLogsWorker.InvokeStatic("ParseUserNameAndSnapIdsFromFilePath", parameterTypes, parameters);

            Assert.AreEqual("t-doilij", parameters[1], "The user name is not correct.");
            Assert.AreEqual(112721, parameters[2], "The SNAP job ID is not correct.");
            Assert.AreEqual("90209", parameters[3], "The SNAP check-in ID is not correct.");
        }

        /// <summary>
        /// Tests that calling ParseRunInformationFromUserLogFilePath() returns the correctly
        /// parsed out parameters.
        /// </summary>
        [TestMethod]
        [Owner("cmorse")]
        public void UnitTestLogsWorker_ParseRunInformationFromUserLogFilePath_WhenProvidedFilePath_ReturnsCorrectOutParameters()
        {
            string logFilePath = @"\\bpt-scratch\UserFiles\cmorse\Tools\ChakraUnitTestTelemetry\UserLogs\cmorse\cmorse2\fbl_ie_script_dev\x86chk\2013.08.02_16.13.41\interpreted\rl.full.log";

            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            object[] parameters = new object[]
            {
                logFilePath,
                null, /*out variant*/
                null, /*out endDate*/
                null, /*out buildArchitecture*/
                null, /*out buildType*/
                null, /*out branchName*/
                null, /*out machineName*/
                null, /*out userName*/
            };

            Type[] parameterTypes = new Type[]
            {
                typeof(string),
                typeof(string).MakeByRefType(),
                typeof(DateTime).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
                typeof(string).MakeByRefType(),
            };

            Contract.Assert(parameters.Length == parameterTypes.Length);

            unitTestLogsWorker.InvokeStatic("ParseRunInformationFromUserLogFilePath", parameterTypes, parameters);

            DateTime expectedEndDate = new DateTime(2013, 8, 2, 16, 13, 41);

            Assert.AreEqual("interpreted", parameters[1], "The variant is not correct.");
            Assert.AreEqual(expectedEndDate, parameters[2], "The end date is not correct.");
            Assert.AreEqual("x86", parameters[3], "The build architecture is not correct.");
            Assert.AreEqual("chk", parameters[4], "The build type is not correct.");
            Assert.AreEqual("fbl_ie_script_dev", parameters[5], "The branch name is not correct.");
            Assert.AreEqual("cmorse2", parameters[6], "The machine name is not correct.");
            Assert.AreEqual("cmorse", parameters[7], "The user name is not correct.");
        }

        /// <summary>
        /// Tests that uploading to the telemetry service works.
        /// </summary>
        /// <remarks>
        /// Comment out the ignore attribute when you want to test connectivity with the
        /// service.  Also, set includeExceptionDetailInFaults to true in the web.config
        /// on the server to view exception details.
        /// </remarks>
        [TestMethod]
        [Owner("cmorse")]
        [Ignore]
        public void UnitTestLogsWorker_UploadFailureInformationToTelemetryWebServiceAsync_WhenProvidedFailureInformation_SuccessfullyUploads()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            UnitTestFailureInformation failureInformation = this.GetTestUnitTestFailureInformation();

            Task result = unitTestLogsWorker.InvokeStatic(
                "UploadFailureInformationToTelemetryWebServiceAsync",
                failureInformation) as Task;

            try
            {
                result.Wait();
            }
            catch
            {
                // Can put a breakpoint here to inspect AggregateException details.
                throw;
            }
        }

        /// <summary>
        /// Tests that uploading the same failure information to the telemetry service
        /// works and just fails silently.  This is to ensure that duplicate record additions
        /// don't throw a unique constraint error back to clients (idempotent).
        /// </summary>
        /// <remarks>
        /// Comment out the ignore attribute when you want to test against the
        /// service.  Also, set includeExceptionDetailInFaults to true in the web.config
        /// on the server to view exception details.
        /// </remarks>
        [TestMethod]
        [Owner("cmorse")]
        [Ignore]
        public void UnitTestLogsWorker_UploadFailureInformationToTelemetryWebServiceAsync_WhenProvidedDuplicateFailureInformation_ServiceFailsSilently()
        {
            PrivateType unitTestLogsWorker = new PrivateType(typeof(UnitTestLogsWorker));

            UnitTestFailureInformation failureInformation = this.GetTestUnitTestFailureInformation();

            for (int i = 0; i < 2; ++i)
            {
                Task result = unitTestLogsWorker.InvokeStatic(
                    "UploadFailureInformationToTelemetryWebServiceAsync",
                    failureInformation) as Task;

                try
                {
                    result.Wait();
                }
                catch
                {
                    // Can put a breakpoint here to inspect AggregateException details.
                    throw;
                }
            }
        }

        /// <summary>
        /// Gets the test unit test failure information.
        /// </summary>
        /// <returns>Test failure information that can be uploaded to the service.</returns>
        private UnitTestFailureInformation GetTestUnitTestFailureInformation()
        {
            return new UnitTestFailureInformation
            {
                BranchName = "TEST_BRANCH",
                BuildArchitecture = "x86",
                BuildType = "chk",
                Variant = "TestVariant",
                CommandLineParameters = "-testParameter1 -testParameter2",
                Duration = TimeSpan.Zero,
                EndTime = DateTime.MinValue,
                ErrorMessage = "Test Error Message",
                FullCommandLine = @"test.exe -testParameter1 -testParameter2 C:\TestFile.js",
                HostType = "test",
                LogFilePath = @"C:\TestFile.js",
                MachineName = Environment.MachineName,
                OperatingSystemVersion = Environment.OSVersion.ToString(),
                StartTime = DateTime.Now,
                UserName = Environment.UserName,
                WasRunInSnap = true,
                SnapCheckinId = "1",
                SnapJobId = 1,
                UnitTestFileName = "TestFile.js",
            };
        }
    }
}
