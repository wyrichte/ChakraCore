/*****************************************************************************/
/*                              EzeAutomation.js                             */
/*****************************************************************************/

/* Generic routines that are useful, but may be specific to the Eze automation
   (assumes VBL layout etc).


   Conventions:  This file is designed to be used with the 'runjs.wsf'
   wrapper.  This wrapper allows you to call an arbitrary jscript function
   from the command line.   This is very nice because it allows you to
   run just pieces of the functionality in this file.  To make this work
   really well however, we need some conventions:

    1) Break your routines into useful independent sub-pieces.  Then have
       a routine that runs the pieces.  This gives users that capability
       of running just the pieces they need.

    2) DO put a nice comment right before the method explaining its args.
       This comment is put into the help for 'runjs' so you get user
       documentation for free.

    3) DO think about defaults.  It makes your code much easier to use
       from the command line.  If you follow the pattern used in the
       functions below, the runjs /? will lift the defaults from the
       source automatically.

    4) DO put a logMsg like the one in 'robocopy' in the beginning of the
       routine.  This allows users to see the pieces that they can
       independently run and and cut and paste to runjs.

    5) DON'T set environment variables in the source.  Instead use the
       runSetEnv command to set environment variables for a particular
       command.  This way the necessary environment is explicit.

    6) DON'T build up state in once piece that is used by another.  This
       makes it impossible to run the pieces independently.

    7) DON'T wire in absolute paths.  It is fine to have defaults, but
       generally people need control over where input and output goes.

    8) Any routines that are helpers should begin with an _, so show they
       are not intended for calling directly from the command line.

*/

/* AUTHOR: Jennifer Hamilton
   DATE: 04/25/11 */

/******************************************************************************/
var EzeAutomationModuleDefined = 1;              // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");

var LogEzeAutomation = logNewFacility("EzeAutomation");

if (ScriptDir == undefined)
        var ScriptDir  = WshFSO.GetParentFolderName(WshFSO.GetParentFolderName(WScript.ScriptFullName));

if (WshShell == undefined)
        var WshShell = WScript.CreateObject("WScript.Shell");

if (Env == undefined)
        var Env = WshShell.Environment("PROCESS");

// RUN scoping (if any)
var runProjectionTests = toBool(getEnvVar("_runProjTests", true));
var runRLTests = toBool(getEnvVar("_runRLTests", true));
var runJSRTTests = toBool(getEnvVar("_runJSRTTests", true));
var verboseFlag = toBool(getEnvVar("_verbose", false));

// This should match _Variants in inetcore\jscript\unittest\RunAllRLTests.cmd
var unittest_variants = [
    "interpreted",
    "dynapogo",
    // User supplied extra variants (from the command line).
    "forcedeferparse",
    "nodeferparse",
    "forceundodefer",
    "bytecodeserialized",
    "forceserialized"
];

var projection_unittest_variants = ["interpreted", "native"]; // This should match _Variants in inetcore\jscript\ProjectionTests\Tests\runalltests.cmd

/* Schedule a local nightly run to sync and build the enlistment.

    time:   Optionally specifies the start time to run the task. The time format
            is HH:mm (24 hour time). For example, 01:00 for 1AM.
            Default to a time between 0 ~ 4 AM.

    task_name: Optionally specifies a task name. Default to daily_[branch_name].
            Do not use space.
*/
function syncAndBuildNightly(time, task_name) {
    if (!time) {
        var m = Math.random() * 60 * 4;
        var h = Math.floor(m / 60);
        var m = Math.floor(m % 60);
        time = ("0" + h) + ":" + (m < 10 ? "0" + m : m);
    }
    task_name = task_name || ("daily_" + Env("_BuildBranch"));

    var runjs = srcBaseFromScript() + "\\inetcore\\jscript\\tools\\runjs.bat";
    var task_cmd = "\"'" + runjs + "' doRun syncAndPreCheckinTestsFull\"";

    function runTaskQuery() {
        var run = runCmd("SCHTASKS /Query /V /FO LIST /TN " + task_name);
        if (run.exitCode == 0) {
            var lines = run.output.split("\n");
            for (var i = 0; i < lines.length; i++) {
                var line = lines[i];
                if (line.match(/(TaskName)|(Task To Run)|(Schedule Type)|(Start Time)/)) {
                    WScript.Echo(line);
                }
            }
        } else {
            WScript.Echo(run.output);
        }
        return run;
    }

    if (!isElevated()) {
        WScript.Echo("Requires executing with elevated privilieges (projection unit tests need running elevated)");
        var shell = new ActiveXObject("Shell.Application");
        shell.ShellExecute(runjs, "syncAndBuildNightly " + time + " " + task_name, "", "runas", 1);

        WScript.Sleep(3000); // wait for the elevated cmd to complete
        return runTaskQuery().exitCode;
    }

    var run = runCmdAndDump("SCHTASKS /Create /F /RL HIGHEST /SC DAILY /ST " + time + " /TN " + task_name + " /TR " + task_cmd);
    if (run.exitCode == 0) {
        var run = runTaskQuery();
    }
    return run.exitCode;
}

/****************************************************************************/
/* this trival helper is designed to cause 'runjs doRun syncAndBuildAllFull' to go off
   every day at a particular time (timeToGoOff eg 23:00 is 11PM).  Basically
   all it does is write a 'syncAndBuildAllFull.cmd' in your startup folder so that 'runAt'
   starts up every time you log in.   (do runAt /? for details on runAt).
   Note that if you log off, runAt is killed and the job is not run.

   Parameters
      timeToGoOff   The time of day to start the script.  By default is
                    a random time between midnight and 1AM
*/
function installSyncAndBuildAllFull(timeToGoOff)
{
    if (!isElevated())
    {
        throw new Error(1, "installSyncAndBuildAllFull: Requires executing with elevated privilieges");
    }

    return installRunjsCommand(timeToGoOff, "doRun syncAndBuildAllFull");
}

// Get value from ENV VAR - setting it to a known value if the env var is not defined
function getEnvVar(varName, defaultValue)
{
    if (!varName) {
        throw new Error(1, "getEnvVar(): varName is empty");
    }

    var retValue = Env(varName);
    if (!retValue) {
        retValue = defaultValue;
    }

    return retValue;
}

// Normalize a given val to true/false as boolean value
function toBool(val)
{
    var valAsString = val.toString().toLowerCase();

    if (valAsString == "true" || val == "yes" || val == "1") {
        return true;
    } else {
        return false;
    }
}

function getBldType(bldType)
{
    if (bldType == undefined) {
        bldType = Env("_BuildType");
        if (!bldType)
            bldType = "chk";
    }
    return bldType;
}

function getBldArch(bldArch)
{
    if (bldArch == undefined) {
        bldArch = Env("_BuildArch");
        if (!bldArch)
            bldArch = "x86";
    }
    return bldArch;
}

function getRunDir(runRelDir)
{
    return (runRelDir == undefined) ? WshShell.CurrentDirectory : srcBaseFromScript() + "\\" + runRelDir;
}

function razzleCommand(bldType, bldArch, runRelDir, commandToRun, timeOut)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    var runDir = getRunDir(runRelDir);

    var run = runCmdToLog("pushd " + runDir + " & call " +
        srcBaseFromScript() + "\\tools\\razzle.cmd " + bldArch + " " + bldType +
                  " " + " Exec " + commandToRun,
        runSetTimeout(timeOut, runSetNoThrow()));
    return run.exitCode;
}

function runConsoleUnitTestCommand(bldType, bldArch, timeOut, args, inRazzle, inDRT, unitTestRelDir, unitTestCommand, logFile)
{
    var srcBase = srcBaseFromScript();
    var unitTestDir = srcBase + unitTestRelDir;
    var unitTestCommand = srcBase + unitTestCommand;

    args = args || "";
    if (IsWinBlueOrLater()) // FUTURE: split -winTH?
    {
        args += " -winBlue";
    }
    else if (IsWin8())
    {
        args += " -win8";
    }
    else if (IsWin7())
    {
        args += " -win7";
    }
    
    if (inDRT)
    {
        args += " -DRT";
    }

    if (inDRT)
    {
        args += " -DRT";
    }

    if(logFile)
    {
        args += " -logFile " + logFile;
    }

    args += " -platform " + bldArch + " -buildType " + bldType

    var runCommand = "pushd " + unitTestDir + " & call ";
    if (!inRazzle && !inDRT)
    {
        runCommand += srcBase + "\\tools\\razzle.cmd " + bldArch + " " + bldType + " Exec ";
    }
    runCommand += unitTestCommand + " " + args;
    return runCmdToLog(runCommand, runSetTimeout(timeOut, runSetNoThrow()));
}

function runProjectionUnitTests(bldType, bldArch, timeOut, args, inRazzle, inDRT)
{
    // projection tests to run either on Win8||WinBlue or -snap enabled machines
    var canRunProjectionTests = IsWin8OrLater();
    var isSnapRun = args && args.match(/-snap/i);

    WScript.Echo("");
    WScript.Echo("ProjectionTest flags:");
    WScript.Echo("\tisSnapRun: " + isSnapRun);
    WScript.Echo("\tcanRunProjectionTests: " + canRunProjectionTests);
    WScript.Echo("\trunProjectionTests: " + runProjectionTests);
    WScript.Echo("");

    var summaryProjection = "";
    if (isSnapRun)
    {
        summaryProjection += "\n<---- Projection Unit Test Summary ---->\n\n";
        summaryProjection += "File: ProjectionTests\\Tests bypassed since SNAP run\n";
        summaryProjection += "Summary: jscript\\ProjectionTests\\Tests\\interpreted had 0 tests; 0 failures\n";
        summaryProjection += "Summary: jscript\\ProjectionTests\\Tests\\native had 0 tests; 0 failures\n";
        summaryProjection += "\n<---- End Projection Unit Test Summary ---->\n";

        return summaryProjection;
    }

    if (!canRunProjectionTests)
    {
        summaryProjection += "\n<---- Projection Unit Test Summary ---->\n\n";
        summaryProjection += "The projection unit tests do require Windows 8 or above\n";
        summaryProjection += "Summary: ProjectionTests ALL TEST failures\n";
        summaryProjection += "\n<---- End Projection Unit Test Summary ---->\n";

        return summaryProjection;
    }

    if (!runProjectionTests)
    {
        summaryProjection += "\n<---- Projection Unit Test Summary ---->\n\n";
        summaryProjection += "ProjectionTests are disabled as per _runProjTests env variable.\n";
        summaryProjection += "Summary: jscript\\ProjectionTests\\Tests\\interpreted had 0 tests; 0 failures\n";
        summaryProjection += "Summary: jscript\\ProjectionTests\\Tests\\native had 0 tests; 0 failures\n";
        summaryProjection += "\n<---- End Projection Unit Test Summary ---->\n";

        return summaryProjection;
    }

    // Do not pass the incoming args flags - we do not want the -snap flag to get into the below script
    runConsoleUnitTestCommand(bldType, bldArch, timeOut, "", inRazzle, inDRT, "\\inetcore\\jscript\\tools", "\\inetcore\\jscript\\tools\\RunAllProjectionTests.cmd");
    summaryProjection = _printProjectionUnitTestSummaryToString();
    return summaryProjection;
}

function runJSRTCheckinTests(bldType, bldArch, timeOut, args, inRazzle, inDRT)
{
    WScript.Echo("");
    WScript.Echo("JSRT Test flags:");
    WScript.Echo("\trunJSRTTests: " + runJSRTTests);
    WScript.Echo("");

    var summaryJSRT = "";
    var jsrtLogFile = srcBaseFromScript() + "\\inetcore\\jscript\\unittest\\jsrt\\te." + bldArch + bldType + ".log";
    if (FSOFileExists(jsrtLogFile)) WshFSO.DeleteFile(jsrtLogFile, true);

    if (!runJSRTTests)
    {
        summaryJSRT = "JSRTTests are disabled as per _runJSRTTests env variable\n";

        return summaryJSRT;
    }

    var run = runConsoleUnitTestCommand(bldType, bldArch, timeOut, args, inRazzle, inDRT, "\\inetcore\\jscript\\tools", "\\inetcore\\jscript\\tools\\RunAllJsrtTests.cmd");
    FSOWriteToFile(run.output, jsrtLogFile);

    summaryJSRT = _printJSRTTestSummaryToString(bldType, bldArch, run.output);
    return summaryJSRT;
}

function runRLUnitTests(bldType, bldArch, inRazzle, inDRT, args, timeOut, testDir)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    inRazzle = inRazzle && (inRazzle === 'true' || inRazzle === true) ? true : false;
    inDRT = inDRT && (inDRT === 'true' || inDRT === true) ? true : false;
    if (timeOut == undefined) timeOut = MINUTE * 30;
    if (args == undefined) args = "";

    var summary = "";
    if (!runRLTests)
    {
        summary = "RLTests are disabled as per _runRLTests env variable\n";
        return summary;
    }

    runConsoleUnitTestCommand(bldType, bldArch, timeOut, args, inRazzle, inDRT, "\\inetcore\\jscript\\" + testDir, "\\inetcore\\jscript\\unittest\\RunAllRLTests.cmd");

    var summary = _printUnitTestSummaryToString(bldType, bldArch, testDir);
    return summary;
}

function runConsoleUnitTests(bldType, bldArch, timeOut, args, inRazzle, inDRT)
{
    var isSnapRun = args && args.match(/-snap/i);
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    if (timeOut == undefined) timeOut = MINUTE * 30;
    inRazzle = inRazzle && (inRazzle === 'true' || inRazzle === true) ? true : false;
    inDRT = inDRT && (inDRT === 'true' || inDRT === true) ? true : false;
    if (args == undefined) args = "";

    WScript.Echo("");
    WScript.Echo("Running console unit tests via runjs.bat:");
    WScript.Echo("\tisSnapRun = " + isSnapRun);
    WScript.Echo("\tbldType = " + bldType);
    WScript.Echo("\tbldArch = " + bldArch);
    WScript.Echo("\ttimeOut = " + timeOut);
    WScript.Echo("\tinRazzle = " + inRazzle);
    WScript.Echo("\tinDRT = " + inDRT);
    WScript.Echo("\targs = " + args);
    WScript.Echo("");

    // Setup Windows.Globalization.dll as all tests now require it.
    // Additionally, each test will make a call of its own to setup Windows.Globalization.dll, and 
    // although this may seem redundant, this allows each test script to be run independently.
    setupWindowsGlobalization();

    var summaryProjection = runProjectionUnitTests(bldType, bldArch, timeOut, args, inRazzle, inDRT);
    var summaryRLunit = runRLUnitTests(bldType, bldArch, inRazzle, inDRT, args, timeOut, "unittest");
    var summaryRLcore = runRLUnitTests(bldType, bldArch, inRazzle, inDRT, args, timeOut, "core\\test");
    var summaryJSRT = runJSRTCheckinTests(bldType, bldArch, timeOut, args, inRazzle, inDRT);

    WScript.Echo(summaryProjection);
    WScript.Echo(summaryRLunit);
    WScript.Echo(summaryRLcore);
    WScript.Echo(summaryJSRT);

    var variantCount = _getUnitTestVariantRanCount(bldType, bldArch);
    var unittestRunPattern = new RegExp("(Unit Test Summary ----.*?\\n)((.|\\n)*?; 0 failures){" + variantCount + "}");
    var projectiontestRunPattern = new RegExp("(Unit Test Summary ----.*?\\n)((.|\\n)*?; 0 failures){" + projection_unittest_variants.length + "}");

    var passed = (!runRLTests || (summaryRLunit.match(unittestRunPattern) && summaryRLcore.match(unittestRunPattern))) &&
        (!runProjectionTests || summaryProjection.match(projectiontestRunPattern)) &&
        // Check for jsrt test failures
        (!runJSRTTests || summaryJSRT.match(/Failed=0/g) && summaryJSRT.match(/Blocked=0/g));

    if (passed) {
        if (!runRLTests || !runProjectionTests || !runJSRTTests) {
            WScript.Echo("\n\n========================================================================================\n");
            WScript.Echo("Warning - all tests weren't run.\n");
            WScript.Echo("========================================================================================\n\n\n");
        }
        return 0;
    }

    WScript.Echo("\n\n========================================================================================\n");
    WScript.Echo("Unit test summary output didn't match expected output. See runConsoleUnitTests function.\n");
    WScript.Echo("========================================================================================\n\n\n");
    return -1;
}

// Gets a number that is padded with a single leading zero if the number is
// less than 10.
function _getZeroPaddedNumber(number)
{
    return number < 10 ? "0" + number : number;
}

function runJsrtUnitTests(binaryRoot)
{
    if (binaryRoot == undefined)
    {
        binaryRoot = Env("_NTTREE") + "\\jscript";
    }
    var args = arguments;

    var scriptsSourceFolder = Env("sdxroot") + "\\onecoreuap\\inetcore\\jscript\\unittest\\jsrt\\scripts";
    var scriptsDestFolder = binaryRoot + "\\jsrt\\unittest";
    
     // Making the current folder writable so that FSOCopyFolder will not fail.
    FSOMakeWriteable(scriptsDestFolder);
    WScript.Echo("copying scripts from : " + scriptsSourceFolder + " to : " + scriptsDestFolder);
    FSOCopyFolder(scriptsSourceFolder, scriptsDestFolder, true);

    var testPath = binaryRoot + "\\jsrt\\unittest\\";
    WScript.Echo("testPath: " + testPath + "\n");
    var testList = testPath + "UnitTest.JsRT.API.dll ";
    testList += testPath + "UnitTest.JsRT.ComProjection.dll ";
    testList += testPath + "UnitTest.JsRT.Engine.dll ";
    testList += testPath + "UnitTest.JsRT.MemoryPolicy.dll ";
    testList += testPath + "UnitTest.JsRT.RentalThreading.dll ";
    testList += testPath + "UnitTest.JsRT.ThreadService.dll ";
    var runManagedTests = true;

    var logFile = undefined;

    if (args)
    {
        for (var i=0; i < args.length; i++)
        {
            if (args[i] == "-snap") runManagedTests = false;

            if (!logFile) logFile = parseLogFileArg(args[i]);
        }
    }
    if (runManagedTests)
    {
        if (! IsWin8OrLater()) setupJsrtUnitTests();
        testList += testPath +  "UnitTest.JsRT.Managed.API.dll ";
        testList += testPath + "UnitTest.JsRT.WinRT.dll ";
    }

    var logArg = "";
    if (logFile) {
        logArg = "\/enableWttLogging \/logFile:\"" + logFile + "\"";
    }

    debugger;

    var runCommand = "te " + testList + " \/select:\"not(@Disabled=\'Yes\')\" \/unicodeoutput:false \/parallel:" + 1 /*Env("NUMBER_OF_PROCESSORS")*/ + " \/logOutput:lowest " + logArg;
    var result = runCmdToLog(runCommand,
        runSetEnv("_NT_SYMBOL_PATH", runGetEnv("_NT_SYMBOL_PATH") + ";" + testPath,
        runSetTimeout(60 * 5 /* 5minutes */, runSetNoThrow())));

    // te.exe sets errorlevel to the number of failed tests
    return result.exitCode;
}

function runMSTestUnitTests(bldType, bldArch)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    var srcBase = srcBaseFromScript();
    var unitTestDir = srcBase + "\\inetcore\\jscript\\unittest\\DirectAuthor\\VSTests";
    var unitTestCommand = unitTestDir + "\\runtests.cmd";

    var run = runCmdToLog("pushd " + unitTestDir + " & call " +
        srcBase + "\\tools\\razzle.cmd " + bldArch + " " + bldType +
                  " " + " Exec " + unitTestCommand,
        runSetTimeout(60 * 20 /* 20 minutes */, runSetNoThrow())
        );

    var testRunPattern = /Test Run Succeeded/;
    if (run.output.match(testRunPattern))
    {
        return 0;
    }
    WScript.Echo("\n\n========================================================================================\n");
    WScript.Echo("MSTest Unit test summary output didn't match expected output. See runMSTestUnitTests function.\n");
    WScript.Echo("========================================================================================\n\n\n");
    return -1;
}

function OACRLogFileName(bldType, bldArch, runRelDir)
{
    return srcBaseFromScript() + "\\" + runRelDir + "." + bldArch + bldType + ".report.xml";
}

/*  Check all OACR queues for errors and compare with TVS gate (current enlistment flavor).
        bldType, bldArch -- currently not supported. This function checks all queues (if any).
*/
function checkOACR(bldType, bldArch, runRelDir, timeOut)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    if (timeOut == undefined) timeOut = MINUTE * 30;
    var runDir = getRunDir(runRelDir);

    var logFile = OACRLogFileName(bldType, bldArch, runRelDir);
    if (FSOFileExists(logFile))
    {
        WScript.Echo("Deleting " + logFile);
        FSODeleteFile(logFile);
    }

    var checkinTestRun = runCmdAndDump("call oacr check all", runSetTimeout(timeOut, runSetNoThrow()));
    if (checkinTestRun.exitCode == 0 && !checkinTestRun.output.match(/error\(s\)/))
    {
        return 0; // No OACR errors found
    }

    if (checkOACRGate() == 0) {
        return 0; // errors already in TVS baseline
    }

    runCmd("oacr list all " + logFile, runSetNoThrow());
    runCmd("oacr export all " + logFile, runSetNoThrow());
    WScript.Echo("\n\nTo view all oacr failures:\n");
    WScript.Echo("    oacr view " + logFile);

    return -1;
}

/*  Compares OACR results to TVS gate (current enlistment flavor). This is included in "checkOACR".
    You can run this separately if you have OACR checked differently (e.g. by the OACR daemon).
*/
function checkOACRGate() {
    var filterRun = runCmdAndDump("perl -S " + srcBaseFromScript() + "\\tools\\filteroacr.pl", runSetNoThrow());
    if (filterRun.exitCode == 0 && !filterRun.output.match(/Failed:/)) {
        return 0;
    }

    WScript.Echo("\n\n========================================================================================\n");
    WScript.Echo("*** OACR Quality Gate Verification failed!!! This may be RI blocker. ***\n\n");
    WScript.Echo("To view failure details:\n");
    WScript.Echo("    oacr view " + Env("_NTTREE") + "\\build_logs\\analysis\\tvs-failures.xml");
    return -1;
}

function showOACR(logFile)
{
    var bldType = getBldType(bldType);
    var bldArch = getBldArch(bldArch);
    if (runRelDir == undefined)
    {
        var runRelDir = "inetcore";
    }
    var runDir = getRunDir(runRelDir);

    if (logFile == undefined)
    {
        var logFile = OACRLogFileName(bldType, bldArch, runRelDir);
    }
    razzleCommand(bldType, bldArch, runRelDir, "call oacr view " + logFile);
}

/****************************************************************************/
/* Copies jscript9 test binaries to mshtmlhost.exe.local for use with mshtml tests.
*/
function refreshMshtmlHost()
{
    refreshTestExeBinaries("mshtmlhost.exe");
}

function refreshTestExeBinaries(testExeName, targetDir, sourceBinaryName, targetFileName)
{
    if (sourceBinaryName === undefined) {
        sourceBinaryName = "chakratest.dll";
    }
    if (targetFileName === undefined) {
        targetFileName = "chakra.dll";
    }
    if (targetDir == undefined) {
        targetDir = Env("_NTTREE") + "\\jscript";
    }

    var testBinDir = targetDir + "\\" + testExeName;
    if (testExeName.match(/.*\.exe$/)) testBinDir += ".local";

    fromFile = targetDir + "\\" + sourceBinaryName;
    toFile = testBinDir + "\\" + targetFileName;
    refreshOneFile(testBinDir, toFile, fromFile);
}

function refreshOneFile(dstDir, toFile, fromFile)
{
    var fromFileDate = new Date(FSOGetDateLastModified(fromFile));
    if (isBinaryUpToDate(toFile, fromFileDate))
    {
        return;
    }

    WScript.Echo("Copying from " + fromFile + " to " + toFile);
    FSOCreatePath(dstDir);
    FSOCopyFile(fromFile, toFile, true);
}

function isBinaryUpToDate(binary, currentBinaryDate)
{
    if (!FSOFileExists(binary))
    {
        return false;
    }
    var binaryDate = new Date(FSOGetDateLastModified(binary));
    binaryDate.setSeconds(0); // don't get seconds from dir on file, and they won't match otherwise
    if (binaryDate.valueOf() < currentBinaryDate.valueOf())
    {
        return false;
    }
    WScript.Echo(binary + " is up to date: " + binaryDate);
    return true;
}

function FindLatestBuildFromReleaseShare()
{
    var winBuildDir = "\\\\winbuilds\\release\\fbl_ie_dev1\\";
    var latestFolder = FSOGetLatestSubFolder(winBuildDir);
    WScript.Echo( latestFolder);
    return winBuildDir+  latestFolder + "\\" + Env("build.arch") + Env("build.type") + "\\" + "bin";
}

function refreshBuiltMshtml()
{
    var binDir = Env("_NTTREE");
    var testBinDir = binDir + "\\jscript\\mshtmlhost.exe.local";

    var fromFile = binDir + "\\edgehtml.dll";
    if (!FSOFileExists(fromFile))
    {
        var mshtmlRemoteBinDir = "\\\\iefs\\iefiles\\tools\\jscript\\Win8MshtmlForWin7\\win8";
        var testBinDir = Env("_NTTREE") + "\\jscript\\mshtmlhost.exe.local";
        var currentBinaryDate = new Date("11/12/2012 12:08 AM");
        currentBinaryDate.setSeconds(0); // don't get seconds from dir on file, and they won't match otherwise, so make sure is 0
        WScript.Echo("getting edgehtml from " + mshtmlRemoteBinDir + " to " + testBinDir);
        try
        {
            syncNetworkFolder(testBinDir, mshtmlRemoteBinDir, "edgehtml.dll", currentBinaryDate);
        }
        catch (ex) {
            WScript.Echo("Failed to sync the network folder due to error: " + ex.message);
            throw ex;
        }
    }

    var toFile = testBinDir + "\\edgehtml.dll";
    refreshOneFile(testBinDir, toFile, fromFile);
}

/****************************************************************************/
/* Copies binaries from network folder (if available) to local folder if
 * representative file doesn't exist in local folder.
*/
function syncNetworkFolder(localFolder, remoteFolder, representativeFile, currentBinaryDate)
{
    var localDll = localFolder + "\\" + representativeFile;
    var remoteDll = remoteFolder + "\\" + representativeFile;
    if (isBinaryUpToDate(localDll, currentBinaryDate))
    {
        return;
    }
    WScript.Echo(localDll + " needs updating. Checking for " + remoteDll);
    if (!FSOFileExists(remoteDll))
    {
        throw new Error("Can't access " + remoteDll);
    }

    var remoteBinaryDate = new Date(FSOGetDateLastModified(remoteDll));
    remoteBinaryDate.setSeconds(0); // don't get seconds from dir on file, and they won't match otherwise
    if (currentBinaryDate.valueOf() != remoteBinaryDate.valueOf())
    {
        throw new Error("Current binary date " + currentBinaryDate + " doesn\'t match date " + remoteBinaryDate + " of " + remoteDll);
    }
    if (FSOFolderExists(localFolder))
    {
        WScript.Echo("Deleting " + localFolder);
        FSODeleteFolder(localFolder, true);
    }
    WScript.Echo("Creating " + localFolder);
    FSOCreatePath(localFolder);
    WScript.Echo("Copying from " + remoteFolder + " to " + localFolder);
    robocopy(remoteFolder, localFolder, undefined, undefined, false, false);

    if (!FSOFileExists(localDll))
    {
        throw new Error("Can't access " + localDll + " after network copy" );
    }
}

/****************************************************************************/
/* Copies OS dependent Windows.Globalization.dll to jshost.exe.local for use with Intl tests.
*/
function setupWindowsGlobalization() {
    try {
        var destinationPaths = [
            "\\jscript\\jshost.exe.local",
            "\\jscript\\JC.exe.local",
            "\\jscript\\jdtest.exe.local",
            "\\jscript\\testHost.exe.local",
            "\\Projection\\WinRT\\jdtest.exe.local",
            "\\Projection\\WinRT\\jshost.exe.local"
        ];

        for (var i = 0; i < destinationPaths.length; i++) {
            var path = destinationPaths[i];

            var dir = Env("_NTTREE") + path;
            var dll = dir + "\\Windows.Globalization.dll";

            if (FSOFileExists(dll)) {
                FSODeleteFile(dll);//Do a clean copy
            } else if (!FSOFolderExists(dir)) {
                FSOCreateFolder(dir);//Verify folder exists
            }

            if (IsWinBlueOrLater()) {
                WScript.Echo("Skip Windows.Globalization setup on winBlue or later for path: " + path);
                continue;
            }
            var currentArchitecture = Env("build.arch");
            if (currentArchitecture === "arm") {
                // ARM devices cannot access IEFS without credentials. The test deployment script takes care of this.
                throw new Error("ARM is not supported.");
            }

            var dllSharePath = "\\\\iefs\\IEFILES\\Tools\\jscript\\WindowsGlobalization";

            var dllPath = dllSharePath + "\\" + currentArchitecture + "\\Windows.Globalization.dll";
            if (!FSOFileExists(dllPath)) {
                throw new Error("Windows.Globalization.dll has not been found on the share path '" + dllPath + "'.");
            }

            FSOCopyFile(dllPath, dll, "FORCE");
        }

        WScript.Echo("Setup Windows.Globalization Succesfully!");
    } catch (ex) {
        WScript.Echo("Error setting up Windows.Globalization: " + ex.message);
        return -1;
    }
    return 0;
}

/****************************************************************************/
/* Copies binaries to mshtmlhost.exe.local for use with edgehtml tests.
*/
function setupMshtmlHost()
{
    try
    {
        var IEVersion = _getIEVersion();
        if (IEVersion == undefined || Number(IEVersion) < 9)
        {
            WScript.Echo("IE Version " + IEVersion + " is less than 9");
            return -1;
        }

        if (IsWin8OrLater()) {
            // Refresh chakratest.dll and enlistment built edgehtml.dll if exists
            refreshMshtmlHost();
            refreshBuiltMshtml();
            return 0;
        }

        // If the edgehtml tests are failing in SNAP after an FI, uncomment the line below to failfast
        // return -1;

        // To fix properly, update the date below to the date of the new edgehtml.dll (can paste result of dir) and submit with the breaking checkin. This will trigger a refresh of the binaries
        // to the local cache, but the refresh will fail because the date will not match, so the tests will not be run. Once the checkin goes through, place the matching binary at
        // \\iefs\iefiles\tools\jscript\Win8MshtmlForWin7. The tests will then successfully refresh the binaries to the local cache because the currentBinaryDate matches the binary on the share.
        var currentBinaryDate = new Date("10/18/2012 10:17 PM");
        currentBinaryDate.setSeconds(0); // don't get seconds from dir on file, and they won't match otherwise, so make sure is 0

        if (Env("X86") != "1")
        {
            WScript.Echo("Not X86");
            return -1;
        }
        if (Env("build.type").match(/fre/) && Env("FRETEST") != "1")
        {
            WScript.Echo("build.type is fre, but FRETEST not set");
            return -1;
        }

        var buildBranch = Env("_BuildBranch");
        if (!buildBranch.match(/^fbl_ie_script_dev$/i) && !buildBranch.match(/^fbl_ie_script$/i))
        {
            WScript.Echo("Branch is not fbl_ie_dev1 nor fbl_ie_script");
            return -1;
        }

        if (Number(IEVersion) == 10)
        {
            // treat IE10 on Win7 as Win8.
            refreshMshtmlHost();
            refreshBuiltMshtml();
            return 0;
        }

        // Get rid of any old copies of the edgehtml binaries before moved local cache to LOCALAPPDATA. Can remove this after a time
        var oldMshtmlLocalBinDir = srcBaseFromScript() + "\\inetcore\\jscript\\tools\\Win8MshtmlForWin7";
        if (FSOFolderExists(oldMshtmlLocalBinDir))
        {
            WScript.Echo("Deleting " + oldMshtmlLocalBinDir);
            FSODeleteFolder(oldMshtmlLocalBinDir, true);
        }

        var testBinDir = Env("_NTTREE") + "\\jscript\\mshtmlhost.exe.local";
        var testBinDll = testBinDir + "\\edgehtml.dll";
        if (isBinaryUpToDate(testBinDll, currentBinaryDate))
        {
            refreshMshtmlHost();
            return 0;
        }
        var mshtmlRemoteBinDir = "\\\\iefs\\iefiles\\tools\\jscript\\Win8MshtmlForWin7";
        var mshtmlLocalBinDir = Env("LOCALAPPDATA") + "\\jscript\\tools\\Win8MshtmlForWin7";
        syncNetworkFolder(mshtmlLocalBinDir, mshtmlRemoteBinDir, "edgehtml.dll", currentBinaryDate);

        WScript.Echo("Copying from " + mshtmlLocalBinDir + " to " + testBinDir);
        FSOCopyFolder(mshtmlLocalBinDir, testBinDir, true);
        if (!isBinaryUpToDate(testBinDll, currentBinaryDate))
        {
            return -1;
        }
        refreshMshtmlHost();
        runCmdToLog(mshtmlRemoteBinDir + "\\" + "setupwin7jscript9.cmd");
        return 0;
    }
    catch (e)
    {
        WScript.Echo("Could not setup Win8 mshtml binaries: " + e.message);
        return -1;
    }
}

function setupJdTest(sourceDir, targetDir)
{
    if (sourceDir == undefined) {
        sourceDir = Env("_NTTREE");
    }
    if (targetDir == undefined) {
        targetDir = Env("_NTTREE") + "\\jscript";
    }

    try
    {
        refreshTestExeBinaries("jdtest.exe", targetDir);
        refreshTestExeBinaries("jdtest.exe", targetDir, "chakradiagtest.dll", "chakradiag.dll");

        return 0;
    }
    catch (e)
    {
        WScript.Echo("Could not setup jdtest binaries: " + e.message);
        return -1;
    }
}

/****************************************************************************/
/* Copies jscript binary to testhost.exe.local.
*/
function setupTesthost(sourceDir, targetDir)
{
    if (sourceDir == undefined) {
        sourceDir = Env("_NTTREE");
    }
    if (targetDir == undefined) {
        targetDir = Env("_NTTREE") + "\\jscript";
    }

    // Only setup testhost.exe.local on Win8.
    if (!IsWin8OrLater())
    {
        return 0;
    }

    try
    {
        refreshTestExeBinaries("testhost.exe", targetDir);

        var target = targetDir + "\\testhost.exe.local";
        copyOneFile(sourceDir, target, "jscript.dll", "FORCE");
    }
    catch (e)
    {
        // If failed to copy the dll, just note a warning in the log instead of logging a failure.
        WScript.Echo("Warning: Could not copy jscript.dll from enlistment. It may not be built. See setupTesthost(): " + e.message);
    }

    return 0;
}

/****************************************************************************/
/* Copies jscript binary to testhost.exe.local.
*/
function setupJsrtUnitTests()
{
    // Only setup jsrt test env on Win7 (the managed tests explicitly load chakra.dll.)
    if (IsWin8OrLater())
    {
        return 0;
    }

    try
    {
        refreshTestExeBinaries("jsrt\\unittest");
    }
    catch (e)
    {
        WScript.Echo("Could not setup jsrt unit test binaries: " + e.message);
        return -1;
    }

    return 0;
}

function copyOneFile(source, target, filename, force) {
    var sourceFile = source + "\\" + filename;

    if (!FSOFileExists(sourceFile)) {
        throw new Error(1, "File not found (" + sourceFile + ").");
    }

    WScript.Echo("Copying " + filename + " from " + source + " to " + target + " (force = \"" + force + "\")...");
    FSOCopyFile(sourceFile, target, force);
}

/****************************************************************************/
/* Copies pdm, pdmproxy and msdbg2 binaries to the target folder (default: _NTTREE\jscript)
*/

function setupJsHostTest(sourceDir, targetDir)
{
    if (sourceDir == undefined) {
        sourceDir = Env("sdxroot") + "\\inetcore\\devtoolbar\\jstools\\setup\\" + Env("build.arch");
    }
    if (targetDir == undefined) {
        targetDir = Env("_NTTREE") + "\\jscript";
    }

    try
    {
        copyOneFile(sourceDir, targetDir, "msdbg2.dll", "FORCE");
        copyOneFile(sourceDir, targetDir, "pdm.dll", "FORCE");
        copyOneFile(sourceDir, targetDir, "pdmproxy100.dll", "FORCE");

        // Needed by new -html tests. Please ensure jscript9test.dll is already present in targetDir.
        refreshTestExeBinaries("jshost.exe", targetDir);

        return 0;
    }
    catch (e)
    {
        WScript.Echo("Could not setup jshost binaries: " + e.message);
        return -1;
    }
}

/****************************************************************************/
/* Copy tests to Win8 machine
*/
function installTestsForSnap(targetDir, sourceJscriptDir, sourceBinDir)
{
    WScript.Echo("\n\n[installTestsForSnap]: sourceJscriptDir: " + sourceJscriptDir + ", sourceBinDir: " + sourceBinDir  + ", targetDir: " + targetDir);
    try {
        _copyBinariesToWin8(sourceBinDir, sourceBinDir, targetDir, /*copyMshtml*/true);
    }
    catch (e)
    {
        WScript.Echo("installWin8Tests failed: " + e.message);
        return 1;
    }
    return 0;
}

function _copyTestFilesToWin8(source, dest)
{
    WScript.Echo("[_copyTestFilesToWin8]: Installing test files from " + source + " to " + dest);
    var logFile = FSOGetTempPath("installWin8Tests") + ".log";
    logFile = undefined;
    var robocopyOptions = "/XF *.asm *.dll *.exe *.obj *.pdb";
    robocopy(source + "\\UnitTest", dest + "\\UnitTest", robocopyOptions, logFile, /*doSync*/ false);
    robocopy(source + "\\Tools", dest + "\\Tools", robocopyOptions, logFile, /*doSync*/ false, /*recurse*/ false);
    robocopy(source + "\\Tools\\ScriptLib", dest + "\\Tools\\ScriptLib", robocopyOptions, logFile, /*doSync*/ false, /*recurse*/ false);
}

function _copyBinariesToWin8(nttreeBinDir, jscriptBinDir, dest, copyMshtml)
{
    var binDest = dest + "\\Bins";
    WScript.Echo("[_copyBinariesToWin8]: Installing nttree binaries from " + nttreeBinDir + ", jscript binaries from " +  jscriptBinDir + " to " + binDest);
    var logFile = FSOGetTempPath("installWin8Binaries") + ".log";
    FSOCreatePath(binDest);
    WScript.Echo("    Copying " + jscriptBinDir + "\\rl.exe to " + binDest);
    FSOCopyFile(jscriptBinDir + "\\rl.exe", binDest, "FORCE");
    WScript.Echo("    Copying " + jscriptBinDir + "\\jshost.exe to " + binDest);
    FSOCopyFile(jscriptBinDir + "\\jshost.exe", binDest, "FORCE");
    WScript.Echo("    Copying " + jscriptBinDir + "\\mshtmlhost.exe to " + binDest);
    FSOCopyFile(jscriptBinDir + "\\mshtmlhost.exe", binDest, "FORCE");

    var jshostDest = binDest + "\\jshost.exe.local";
    FSOCreatePath(jshostDest);

    WScript.Echo("    Copying " + nttreeBinDir + "\\chakra.dll to " + jshostDest);
    FSOCopyFile(nttreeBinDir + "\\chakra.dll", jshostDest, "FORCE");

    var mshtmlHostDest = binDest + "\\mshtmlhost.exe.local";
    FSOCreatePath(mshtmlHostDest);
    FSOCopyFile(jshostDest + "\\chakra.dll", mshtmlHostDest, "FORCE");
    if (copyMshtml)
    {
        WScript.Echo("    Copying " + nttreeBinDir + "\\edgehtml.dll to " + mshtmlHostDest);
        FSOCopyFile(nttreeBinDir + "\\edgehtml.dll", mshtmlHostDest, "FORCE");
    }
}

function _destFromWin8Machine(win8Machine)
{
    if (win8Machine == undefined)
    {
        throw new Error(1, "Target machine missing")
    }
    return "\\\\" + win8Machine + "\\c$";
}

/****************************************************************************/
/* Copy tests files from local enlistment to Win8 machine
*/
function copyTestFilesToWin8(win8Machine)
{
    _copyTestFilesToWin8(srcBaseFromScript() + "\\inetcore\\jscript", _destFromWin8Machine(win8Machine) + "\\JsRoot\\JscriptTests");
}

/****************************************************************************/
/* Copy binaries to Win8 machine
*/
function copyBinariesToWin8(win8Machine, copyMshtml)
{
    _copyBinariesToWin8(Env("_NTTREE"), Env("_NTTREE") + "\\jscript", _destFromWin8Machine(win8Machine) + "\\JsRoot\\JscriptTests", copyMshtml);
}

/****************************************************************************/
/* Copy tests and binaries to Win8 machine at \\Machine\c$\JscriptTests
*/
function installTestsOnWin8(win8Machine, copyMshtml)
{
    WScript.Echo("\nInstalling UnitTests to " + _destFromWin8Machine(win8Machine));
    copyTestFilesToWin8(win8Machine);
    copyBinariesToWin8(win8Machine, copyMshtml);
    return 0;
}

function _getUnitTestVariantRanCount(bldType, bldArch)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    var srcBase = srcBaseFromScript();
    var unitTestLogsDir = srcBase + "\\inetcore\\jscript\\unittest\\logs\\" + bldArch + bldType;
    var variantCount = 0;

    for (var i in unittest_variants) {
        var logfile = unitTestLogsDir + "\\" + unittest_variants[i] + "\\rl.log";
        if (FSOFileExists(logfile)) {
            ++variantCount;
        }
    }

    return variantCount;
}

function _printUnitTestSummaryToString(bldType, bldArch, testDir)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    var summary = "\n<---- (" + testDir + ") Unit Test Summary ---->\n\n";

    var srcBase = srcBaseFromScript();
    var unitTestLogsDir = srcBase + "\\inetcore\\jscript\\" + testDir + "\\logs\\" + bldArch + bldType;

    for (i in unittest_variants)
    {
        var logfile = unitTestLogsDir + "\\" + unittest_variants[i] + "\\rl.log";

        if(FSOFileExists(logfile))
        {
            summary += logfile + ":\n";
            var result = FSOReadFromFile(logfile).match(/^Summary.*failures$/mg);
            if (result && result.length > 0)
            {
                summary += result[0];
            }
            summary += "\n\n";
        }
    }

    summary += "<---- End Unit Test Summary ---->\n";

    return summary;
}

function _printProjectionUnitTestSummaryToString()
{
    var summary = "";

    summary = "\n<---- Projection Unit Test Summary ---->\n\n";

    var srcBase = srcBaseFromScript();
    var unitTestLogsDir = srcBase + "\\inetcore\\jscript\\projectionTests\\Tests\\logs";

    for (i in projection_unittest_variants)
    {
        var logfile = unitTestLogsDir + "\\" + projection_unittest_variants[i] + "\\rl.log";

        summary += "File: " + logfile + "\n";

        if(FSOFileExists(logfile))
        {
            var result = FSOReadFromFile(logfile).match(/^Summary.*failures$/mg);
            if (result.length > 0)
            {
                summary += "  " + result[0];
            }
            summary += "\n\n";
        }
    }

    summary += "<---- End Projection Unit Test Summary ---->\n";

    return summary;
}

function _printJSRTTestSummaryToString(bldType, bldArch, runOutput)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    var summary = "\n<---- JSRT Test Summary ---->\n\n";
    var logfile = srcBaseFromScript() + "\\inetcore\\jscript\\unittest\\jsrt\\te." + bldArch + bldType + ".log";
    summary += logfile + ":\n";

    var result;
    if (runOutput) {
        result = runOutput.match(/Summary:.*$/mg);
    } else if (FSOFileExists(logfile)) {
        result = FSOReadFromFile(logfile).match(/Summary:.*$/mg).slice(1); /* skip the starting , */
    }

    if (result) {
        summary += result;
    } else {
        summary += "<< NO TESTS RAN >>";
    }

    summary += "\n\n";
    summary += "<---- End JSRT Test Summary ---->\n";

    return summary;
}

// string helper functions
if (typeof String.prototype.startsWith != 'function') {
  String.prototype.startsWith = function (str){
    return this.slice(0, str.length) == str;
  };
}

if (typeof String.prototype.endsWith != 'function') {
  String.prototype.endsWith = function (str){
    return this.slice(-str.length) == str;
  };
}

// parse the -logFile:"{file}" argument - to return {file}
function parseLogFileArg(arg) {
    var logFile = "";
    var prefix = "-logFile:";

    verboseLog("parseLogFileArg(): arg = " + arg);

    if (arg.startsWith(prefix)) {

        logFile = arg.slice(prefix.length);
        if(logFile.startsWith('"')) {
            if(logFile.endsWith('"') == false) {
                // error
                WScript.Echo("\n>>> INVALID LOG FILE " + logFile + " <<<\n");
                return "";
            }

            logFile = logFile.slice(1, logFile.length-2);
        }
    }

    verboseLog("parseLogFileArg(): returns " + logFile);
    return logFile;
}

// log under the _verbose env var
function verboseLog(arg)
{
    if( verboseFlag ) {
        WScript.Echo(arg);
    }
}
