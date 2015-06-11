/*********************************************************************************/
/*                               beagle.js                                       */
/*********************************************************************************/

/*********************************************************************************/
/* Utilities for interacting with a beagle board, assumes user has getBeagleIP.js*/
/* to return beagle board's IP address                                           */
/*********************************************************************************/

// AUTHOR: Grant Richins
// DATE: 3/15/2010
// Dependencies: clrtask.js
//               fso.js
//               log.js
//               run.js
//               util.js


/*****************************************************************************/
var beagleModuleDefined = 1; // Indicate that this module exists

if (!fsoModuleDefined)   throw new Error(1, "Need to include fso.js");
if (!logModuleDefined)   throw new Error(1, "Need to include log.js");
if (!runModuleDefined)   throw new Error(1, "Need to include run.js");
if (!utilModuleDefined)  throw new Error(1, "Need to include util.js");
if (!Env)                throw new Error(1, "Need to have Env define. This script expects it to be defined by run.js");

var LogBeagle = logNewFacility("beagle");
var _cachedBeagleIP = undefined;


/*********************************************************************************/

/* Find all the COMPLUS_* environment variables in the environment, and create a script to set them
 * that can be used on the beagle board. Thus, propagate the COMPLUS_* variables in the current
 * environment to the beagle board.
 *
 * Ideally, we would just iterate over all the COMPLUS_* variables.
 */

/* Hard-code a few variables that we won't propagate */
var aDontPropagateVars = new Array(
    "COMPLUS_DEFAULTVERSION",
    "COMPLUS_VERSION",
    "COMPLUS_LoadFromRemoteSources",
    "COMPLUS_SOProbeAssertOnOverrun"
);

function _getCOMPLUSVars() {
    var oExec = WshShell.Exec(nsGetCommandProcessor() + " set COMPLUS_");
    var sLine = "";
    var aLines = [];
    var s;

    while (!oExec.StdOut.AtEndOfStream) {
        s = oExec.StdOut.Read(1);
        if (s == "\r" || s == "\n")
        {
            if (sLine != "")
            {
                aLines.push(sLine);
                sLine = "";
            }
        }
        else
        {
            sLine += s;
        }
    }

    // pick up the last line, if there is no \r\n
    if (sLine != "")
    {
        aLines.push(sLine);
        sLine = "";
    }

    return aLines;
}

function _getEnvToPropagateToBeagle() {
    var result = "";
    var aComplusVars = _getCOMPLUSVars();
    // Filter out the ones we don't propagate, and convert the ones we
    // do into a series of "set" commands to be used in a script.
    for (var i = 0; i < aComplusVars.length; i++) {
        var line = aComplusVars[i];
        if (line.match(/^(\w+)=(.*)$/))
        {
            var envVar   = RegExp.$1;
            var envValue = RegExp.$2;

            /* Is it on the don't propagate list? */
            var skip = false;
            for (var j = 0; j < aDontPropagateVars.length; j++) {
                if (envVar == aDontPropagateVars[j]) {
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                result += "set " + envVar + "=" + envValue + "\r\n";
            }
        }
    }
    return result;
}

/*********************************************************************************/

function _getBeagleIP() {
    if (_cachedBeagleIP == undefined) {
        try {
            var scriptLibDir = WScript.ScriptFullName.match(/^(.*)\\/)[1] + "\\scriptlib";
            var fileName = "user\\" + Env("USERNAME") + "\\getBeagleIP.js";

            if (FSOFileExists(scriptLibDir + "\\" + fileName)) {
                _cachedBeagleIP = eval(includeJS(fileName));
            }
            else {
                _cachedBeagleIP = "";
            }
        } catch (e) {
            _cachedBeagleIP = "";
        }
    }
    return _cachedBeagleIP;
}


var _beaglePrepped;

/****************************************************************************/
/* Preps the beagle board after a reboot by starting the sftp_server
   and debug server (no-op if they are already started)

     Parameters
       targetIP  : The IP address of the beagle board (default: uses
                   getBeagleIP.js)
       debugPort : The port to start the debug server on
*/
function prepBeagleBoard(targetIP, debugPort) {

    if (targetIP == undefined)
        targetIP = _getBeagleIP();

    if (debugPort == undefined)
        debugPort = "4231"; // Compliments to Rudi

    if (_beaglePrepped == targetIP)
        return 0;

    logMsg(LogBeagle, LogInfo, "prepBeagleBoard(", targetIP, ", ", debugPort, ") {\n");

    var runOpts = runSetIdleTimeout(60); // 1 minute should be enough
    var checkStartedBatch = FSOGetTempPath("checkStarted-") + ".bat";
    FSOWriteToFile("tlist.exe\r\nexit\r\n", checkStartedBatch);
    var run = runCmd(ScriptDir + "\\x86\\simple_telnet.exe " + targetIP + " " + checkStartedBatch, runOpts);
    FSODeleteFile(checkStartedBatch);

    var startCmds = "";
    if (!run.output.match(/sftp_server/i)) {
        logMsg(LogBeagle, LogInfo10, "Starting sftp_server on ", targetIP, "\n");
        startCmds += "copy /y \\\\GRANTRI5\\beagle_sftp_server\\sftp_server.exe\r\n";
        startCmds += "start /I sftp_server.exe\r\n";
    }
    else {
        logMsg(LogBeagle, LogInfo10, "sftp_server already running on ", targetIP, "\n");
    }

    if (run.output.match(/winpeshl/i))
    {
        if (!run.output.match(/DBGSRV/i)) {
            logMsg(LogBeagle, LogInfo10, "Starting DbgSrv on ", targetIP, " on port ", debugPort, "\n");
            startCmds += "start /I X:\\debugger\\DBGSRV.EXE -t tcp:port=" + debugPort + "\r\n";
        }
        else {
            logMsg(LogBeagle, LogInfo10, "bgSrv already running on ", targetIP, " (with unknown port)\n");
        }
    }
    else
    {
        if (!run.output.match(/MWDBGSRV/i)) {
            logMsg(LogBeagle, LogInfo10, "Starting MWDbgSrv on ", targetIP, " on port ", debugPort, "\n");
            startCmds += "start /I MWDBGSRV.EXE -t tcp:port=" + debugPort + "\r\n";
        }
        else {
            logMsg(LogBeagle, LogInfo10, "MDbgSrv already running on ", targetIP, " (with unknown port)\n");
        }
    }

    if (startCmds != "") {
        startCmds += "exit\r\n";
        logMsg(LogBeagle, LogInfo1000, startCmds);
        var startBatch = FSOGetTempPath("start-") + ".bat";
        try {
            FSOWriteToFile(startCmds, startBatch);
            run = runCmdToLog(ScriptDir + "\\x86\\simple_telnet.exe " + targetIP + " " + startBatch, runOpts);
        } finally {
            FSODeleteFile(startBatch);
        }
        logMsg(LogBeagle, LogInfo1000, run.output);
    }

    logMsg(LogBeagle, LogInfo, " } prepBeagleBoard()\n");

    _beaglePrepped = targetIP;

    return 0;
}

/****************************************************************************/
/* Task to prep the Beagle Board


     Parameters
       targetIP    : The IP address of the beagle board
       debugPort   : The port to start the debug server on
       dependents  : Any dependents you want for this task (like a build)
*/

function _prepBeagleBoardTask(targetIP, debugPort, dependents) {

    if (targetIP == undefined)
        targetIP = "_";

    if (debugPort == undefined)
        debugPort = "_";

    var ret = taskNew("prepBeagleBoard", "runjs prepBeagleBoard " + targetIP + " " + debugPort, dependents, "local");

    ret.description = "Starts the debug server, and sftp_server on the beagle board.";
    return ret;
}

function _makeSendBeaglePaths(binDir, targetDir, file) {
    return " /send " + binDir + "\\" + file + " " + targetDir + "\\" + file;
}

/*****************************************************************************/
/* Helper to copy a file to the Beagle board. No logging.

     Parameters:
       sourceFile : The source file to copy
       targetDir  : The target directory to copy to (on the device)
       targetIP   : the IP address of the beagle board (default: uses getBeagleIP.js)
*/

function _beagleCopy(sourceFile, targetDir, targetIP) {

    if (sourceFile == undefined)
    {
        throw new Error(1, "_beagleCopy: Required argument sourceFile is missing");
    }
    if (targetDir == undefined)
    {
        targetDir = ""; // default is root directory: _makeSendBeaglePaths makes this happen
    }
    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    var sourceMatch = sourceFile.match(/^(.*\\)?(.*)/);
    if (sourceMatch == null)
    {
        return -2;
    }
    var sourceDir = sourceMatch[1];
    var sourceFileName = sourceMatch[2];
    if (sourceDir == null || sourceDir == "")
    {
        sourceDir = ".";
    }
    else
    {
        // strip off the trailing backslash since _makeSendBeaglePaths adds it back
        sourceDir = sourceDir.match(/^(.*)?\\/)[1];
    }

    var targetMatch = targetDir.match(/^(.*)\\$/); // is there a trailing backslash?
    if (targetMatch != null)
    {
        targetDir = targetMatch[1]; // strip trailing backslash
    }

    var fileList = _makeSendBeaglePaths(sourceDir, targetDir, sourceFileName);

    var runOpts = runSetNoThrow(runSetExpectedReturnCode(1, runOpts));
    var retried = false;
    while (true) {
        run = runCmdToLog( ScriptDir + "\\x86\\sftp.exe " + targetIP + fileList, runOpts);

        if (!retried && run.exitCode == -5 && run.output.match(/: 10061:/)) {
            // This is usually because the server isn't started:
            //     Failed to connect to server: 10061: No connection could be made
            //     because the target machine actively refused it.
            prepBeagleBoard(targetIP); // Try to start the sftp_server
            retried = true;
            continue;
        }
        break;
    }

    return run.exitCode;
}

/*****************************************************************************/
/* Copy a file to the Beagle board.

     Parameters:
       sourceFile : The source file to copy
       targetDir  : The target directory to copy to (on the device)
       targetIP   : the IP address of the beagle board (default: uses
                    getBeagleIP.js)
*/

function beagleCopy(sourceFile, targetDir, targetIP) {

    if (sourceFile == undefined)
    {
        throw new Error(1, "beagleCopy: Required argument sourceFile is missing");
    }
    if (targetDir == undefined)
    {
        targetDir = ""; // default is root directory: _makeSendBeaglePaths makes this happen
    }
    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    logMsg(LogBeagle, LogInfo, "beagleCopy(", sourceFile, ", ", targetDir, ", ", targetIP, ") {\n");

    var retval = _beagleCopy(sourceFile, targetDir, targetIP);

    if (retval < 0) {
        logMsg(LogBeagle, LogInfo, "} beagleCopy() failed, error = " + retval + "\n");
        return -1;
    }

    logMsg(LogBeagle, LogInfo, "} beagleCopy()\n");

    return 0;
}

/****************************************************************************/
/* Helper to construct the CoreCLR install path. Use a machine name in the
 * path to allow multiple machines to target a single Beagle Board without
 * worrying about locking. Currently assumes a single machine only has one
 * task targetting the Beagle Board.

  Parameters
    bldType     : The build type (chk, dbg, ret ...)

 * Returns the CoreCLR install path to use on the device.
*/

function _constructInstallPath(bldType)
{
    if (bldType == undefined)
    {
        bldType = "core" + Env("_BuildType");
    }

    var machineName = Env("COMPUTERNAME");
    if (!machineName)
    {
        machineName = "UNKNOWN";
    }

    var installDir = "\\CoreCLR." + machineName + "\\v2.0.ARM" + bldType;
    return installDir;
}

/*****************************************************************************/
/* Run a single test on the Beagle board. Assumes the runtime is already on the
   machine, probably via fxpBeagleSetup.

     Parameters:
       test       : The test file to run
       runtimeDir : The directory of the runtime
       targetDir  : The target directory to copy to (on the device)
       targetIP   : The IP address of the beagle board (default: uses
                    getBeagleIP.js)
*/

function beagleRun(test, runtimeDir, targetDir, targetIP) {

    if (test == undefined)
    {
        throw new Error(1, "beagleRun: Required argument test is missing");
    }
    if (runtimeDir == undefined)
    {
        runtimeDir = _constructInstallPath();
    }
    if (targetDir == undefined)
    {
        targetDir = "\\";
    }
    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    logMsg(LogBeagle, LogInfo, "beagleRun(", test, ", ", runtimeDir, ", ", targetDir, ", ", targetIP, ") {\n");

    var retval = _beagleCopy(test, targetDir, targetIP);
    if (retval < 0) {
        logMsg(LogBeagle, LogInfo, "} beagleRun() failed\n");
        return -1;
    }

    // Get the test file name without a path, as it will exist on the target
    var sourceMatch = test.match(/^(.*\\)?(.*)/);
    if (sourceMatch == null)
    {
        logMsg(LogBeagle, LogInfo, "} beagleRun() failed -- internal error splitting test path name\n");
        return -2;
    }
    var testFileName = sourceMatch[2];
    var runtimeDirMatch = runtimeDir.match(/^.*\\$/); // is there a trailing backslash?
    if (runtimeDirMatch == null)
    {
        runtimeDir = runtimeDir + "\\"; // if no trailing backslash, add one
    }

    var runOneCommands =
            "cd " + targetDir + "\r\n" +
            _getEnvToPropagateToBeagle() +
            runtimeDir + "fxprun.exe " + testFileName + "\r\n" +
            "echo ERRORLEVEL is %ERRORLEVEL%\r\n" +
            "exit\r\n";

    logMsg(LogBeagle, LogInfo, "running:\r\n" + runOneCommands);

    var runOneBatch = FSOGetTempPath("runOne-") + ".bat";
    FSOWriteToFile(runOneCommands, runOneBatch);
    var run = runCmdToLog( "simple_telnet " + targetIP + " " + runOneBatch);
    FSODeleteFile(runOneBatch);

    logMsg(LogBeagle, LogInfo, "} beagleRun()\n");

    return 0;
}

/*****************************************************************************/
/* Run a single test on the Beagle board under the debugger. Assumes the
   runtime is already on the machine, probably via fxpBeagleSetup.

     Parameters:
       test       : The test file to run
       runtimeDir : The directory of the runtime
       targetDir  : The target directory to copy to (on the device)
       debugCmd   : the debugger to use (including path)
       targetIP   : The IP address of the beagle board (default: uses
                    getBeagleIP.js)
       debugPort  : The debug server port to use
*/

function beagleDebug(test, runtimeDir, targetDir, debugCmd, targetIP, debugPort)
{
    if (test == undefined)
    {
        throw new Error(1, "beagleDebug: Required argument test is missing");
    }
    if (runtimeDir == undefined)
    {
        runtimeDir = _constructInstallPath();
    }
    if (targetDir == undefined)
    {
        targetDir = "\\";
    }
    if (debugCmd == undefined)
    {
        debugCmd = "windbg";
    }
    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }
    if (debugPort == undefined)
    {
        debugPort = "4231";
    }

    // Trim any leading or trailing whitespace
    targetIP = targetIP.replace(/(^\s*|\s*$)/g, "");

    logMsg(LogBeagle, LogInfo, "beagleDebug(", test, ", ", runtimeDir, ", ",
                                               targetDir, ", ", debugCmd, ", ",
                                               targetIP, ", ", debugPort, ") {\n");

    var retval = _beagleCopy(test, targetDir, targetIP);
    if (retval < 0) {
        logMsg(LogBeagle, LogInfo, "} beagleDebug() failed\n");
        return -1;
    }

    // Make sure targetDir ends with a trailing backslash
    var targetDirMatch = targetDir.match(/^.*\\$/); // is there a trailing backslash?
    if (targetDirMatch == null)
    {
        targetDir = targetDir + "\\"; // if no trailing backslash, add one
    }

    // Change the test filename to be device-relative
    var sourceMatch = test.match(/^(.*\\)?(.*)/);
    if (sourceMatch == null)
    {
        logMsg(LogBeagle, LogInfo, "} beagleDebug() failed -- internal error splitting test path name\n");
        return -2;
    }
    var testFileName = targetDir + sourceMatch[2];

    // Make sure runtimeDir ends with a trailing backslash
    var runtimeDirMatch = runtimeDir.match(/^.*\\$/); // is there a trailing backslash?
    if (runtimeDirMatch == null)
    {
        runtimeDir = runtimeDir + "\\"; // if no trailing backslash, add one
    }

    var cmd =
            debugCmd + " -premote tcp:server=" + targetIP + ",port=" + debugPort + " " +
            runtimeDir + "fxprun.exe " + testFileName;
    logMsg(LogBeagle, LogInfo, "running: " + cmd + "\n");


    WshShell.Run(cmd, 1, false);

    logMsg(LogBeagle, LogInfo, "} beagleDebug()\n");

    return 0;
}

/****************************************************************************/
/* install CoreCLR runtime from 'binDir' to 'targetDir' on beagle board at
   'targetIP'

     Parameters
       binDir    : The directory to get the runtime binaries from
       targetDir : The directory to install the runtime binaries to
       targetIP  : The IP address of the beagle board (default: uses
                   getBeagleIP.js)
*/
function fxpBeagleSetup(binDir, targetDir, targetIP) {

    if (binDir == undefined) {
        binDir = Env("_NTTREE");
        if (!Env("_NTTREE"))
            throw new Error(1, "fxpSetup: Required argument binDir for the fxpBeagleSetup command not present");
    }

    if (targetDir == undefined)
        targetDir = _constructInstallPath();

    if (targetIP == undefined)
        targetIP = _getBeagleIP();

    logMsg(LogBeagle, LogInfo, "fxpBeagleSetup(", binDir, ", ", targetDir, ", ", targetIP, ") {\n");

    var fileList = _makeSendBeaglePaths(binDir, targetDir, "fxprun.exe") +
                   _makeSendBeaglePaths(binDir, targetDir, "fxprun.exe.managed_manifest") +
                   _makeSendBeaglePaths(binDir, targetDir, "coreclr.dll") +
                   _makeSendBeaglePaths(binDir, targetDir, "mscorlib.dll") +
                   _makeSendBeaglePaths(binDir, targetDir, "mscorrc.dll") +
                   _makeSendBeaglePaths(binDir, targetDir, "coregen.exe") +
                   _makeSendBeaglePaths(binDir, targetDir, "sandboxhelper.dll");

    var runOpts = runSetIdleTimeout(60);
    var prepDirBatch = FSOGetTempPath("prepDir-") + ".bat";
    FSOWriteToFile("rd /s /q " + targetDir + "\r\nmd " + targetDir + "\r\nexit\r\n", prepDirBatch);
    var run = runCmdToLog(ScriptDir + "\\x86\\simple_telnet.exe " + targetIP + " " + prepDirBatch, runOpts);
    FSODeleteFile(prepDirBatch);

    var retried = false;
    while (true) {
        runOpts = runSetNoThrow(runSetExpectedReturnCode(7, runOpts));
        run = runCmdToLog( ScriptDir + "\\x86\\sftp.exe " + targetIP + fileList, runOpts);

        if (!retried && run.exitCode == -5 && run.output.match(/: 10061:/)) {
            // This is usually because the server isn't started:
            //     Failed to connect to server: 10061: No connection could be made
            //     because the target machine actively refused it.
            prepBeagleBoard(targetIP); // Try to start the sftp_server
            retried = true;
            continue;
        }
        break;
    }

    if (run.exitCode != 7) {
        logMsg(LogBeagle, LogInfo, "} fxpSetup() failed\n");
        return -1;
    }

    logMsg(LogBeagle, LogInfo, "} fxpSetup()\n");
    return 0;
}


/****************************************************************************/
/* Helper to construct the BVT path. Same idea as _constructInstallPath()
 * Returns the BVT path to use on the device.
*/

function _constructBvtPath()
{
    var machineName = Env("COMPUTERNAME");
    if (!machineName)
    {
        machineName = "UNKNOWN";
    }

    var bvtDir = "\\bvts." + machineName;
    return bvtDir;
}

/****************************************************************************/
/* Task to install the runtime using sftp to copy to Beagle Board

logs go to %outDir%\ARM<bldType>

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    binDir      : The directory to get the runtime binaries from
    dependents  : Any dependents you want for this task (like a build)
*/

function _fxpBeagleSetupTask(bldType, binDir, targetIP, dependents) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");

    if (binDir == undefined)
        binDir = "%outDir%\\ARM" + bldType + "\\bin";

    if (targetIP == undefined)
        targetIP = "_";

    if (dependents == undefined) {
        if (!clrTaskModuleDefined) throw new Error(1, "Need to include clrtask.js");
        dependents = [_razzleBuildTask(bldType, "ARM", "ndp\\clr"), _prepBeagleBoardTask()];
    }

    var taskName = "fxpBeagleSetup@ARM" + bldType;
    var targetDir = _constructInstallPath(bldType);

    var ret = taskNew(taskName,
        "runjs fxpBeagleSetup " + binDir + " "
                                + targetDir + " "
                                + targetIP,
        dependents, "local");

    ret.description = "The fxpBeagleSetup task is responsible for installing a given set of CLR binaries on the beagle board.";
    return ret;
}

/*****************************************************************************/
/* Prepare for a beagle BVT run. Specifically, create the bvt directory where
 * tests will be copied.

     Parameters:
       outDir   : Where to place the test results
       ddsDir   : The base of the ddSuites tree
       bvtDir   : Which bvt directory to use (on the device)
       targetIP : The IP address of the beagle board (default: uses getBeagleIP.js)
       bvtRoot  : The test bvt root (on the host)
*/

function _prepBeagleBVT(outDir, ddsDir, bvtDir, targetIP, bvtRoot)
{
    if (ddsDir == undefined) {
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "beagleBVT: Required argument ddsDir is missing");
    }

    if (outDir == undefined) {
        outDir = ddsDir + "\\src\\coreclr\\ARM\\test.BeagleBVT";
    }

    if (bvtDir == undefined)
    {
        bvtDir = _constructBvtPath();
    }

    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    if (bvtRoot == undefined)
    {
        bvtRoot = ddsDir + "\\src\\coreclr\\ARM";
    }

    logMsg(LogBeagle, LogInfo, "_prepBeagleBVT(", outDir, ", ", ddsDir, ", ", bvtDir, ", ", targetIP, ", ", bvtRoot, ") {\n");

    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    var runOpts = runSetExpectedReturnCode(0,
                  runSetNoThrow(
                  runSetLog(LogBeagle, LogInfo)));

    var run = runCmdToLog("call " + bvtRoot + "\\prepBVT.bat "
                                  + targetIP + " "
                                  + bvtDir + " "
                                  + outDir,
                          runOpts);

    logMsg(LogBeagle, LogInfo, "} _prepBeagleBVT\n");
}

/*****************************************************************************/
/* Runs the beagleBvts (a bucket of test that need to be run before checkin).
   This routine will run on the currently installed runtime, so you need
   to have the runtime set up before calling this routine.

     Parameters:
       outDir   : Where to place the test results
       ddsDir   : The base of the ddSuites tree
       runtime  : Which runtime directory to use (on the device)
       bvtDir   : Which bvt directory to use (on the device)
       targetIP : The IP address of the beagle board (default: uses getBeagleIP.js)
       bvtRoot  : The test bvt root (on the host)
       bvtTestList : The file name with the list of tests to run. The file lives
                     in bvtRoot if not qualified with a path. (default: TestList.txt)
*/

function beagleBVT(outDir, ddsDir, runtime, bvtDir, targetIP, bvtRoot, bvtTestList) {

    if (ddsDir == undefined) {
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "beagleBVT: Required argument ddsDir is missing");
    }
    if (outDir == undefined) {
        outDir = ddsDir + "\\src\\coreclr\\ARM\\test.BeagleBVT";
    }

    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    if (bvtRoot == undefined)
    {
        bvtRoot = ddsDir + "\\src\\coreclr\\ARM";
    }

    if (runtime == undefined)
    {
        runtime = _constructInstallPath();
    }

    if (bvtDir == undefined)
    {
        bvtDir = _constructBvtPath();
    }

    if (bvtTestList == undefined)
    {
        bvtTestList = "TestList.txt";
    }

    logMsg(LogBeagle, LogInfo, "beagleBVT(", outDir, ", ", ddsDir, ", ", runtime, ", ", bvtDir, ", ", targetIP, ", ", bvtRoot, ", ", bvtTestList, ") {\n");

    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    _prepBeagleBVT(outDir, ddsDir, bvtDir, targetIP, bvtRoot);

    var TestExe = "";
    var testMsg = "";
    var FailedTests = "";
    var PassCount = 0;
    var FailCount = 0;
    var TestNumber = 1;
    var TestCount = 1;
    var TestRun;
    var testPassFailMsg;

    var TestListFileName;
    var TestListDir;
    if (bvtTestList.match(/^(.*)\\([^\\]*)$/)) {
        TestListDir = RegExp.$1;
        TestListFileName = RegExp.$2;
    }
    else {
        TestListDir = bvtRoot;
        TestListFileName = bvtTestList;
    }
    bvtTestList = TestListDir + "\\" + TestListFileName;
    var FailedListFileName = outDir + "\\Failed_" + TestListFileName;
      
    var runOpts = runSetExpectedReturnCode(0,
                  runSetNoThrow());

    TestRun = runCmd("call " + bvtRoot + "\\devBVT.bat "
                             + targetIP + " "
                             + runtime + " "
                             + bvtDir + " "
                             + outDir,
                     runOpts);

    TestExe = "HelloWorld.exe";

    if (TestRun.exitCode != 0) {
        FailCount++;
        FailedTests += "\t" + TestExe + " (exit code: " + TestRun.exitCode + ")\n";
        logMsg(LogBeagle, LogInfo, "Test failed:\n" + TestRun.output);
        testPassFailMsg = " FAILED";
    }
    else {
        PassCount++;
        testPassFailMsg = "";
    }

    testMsg = TestNumber + "/" + TestCount + " (pass: " + PassCount + ", fail: " + FailCount + "): " + TestExe + testPassFailMsg + "\n";
    logMsg(LogBeagle, LogInfo, testMsg);

    // Only run the remaining tests if HelloWorld.exe passed

    if (TestRun.exitCode == 0) {

        runOpts = runSetExpectedReturnCode(100,
                  runSetNoThrow());

        var TestListFile = FSOOpenTextFile(bvtTestList, FSOForReading);
        var TestList = [];
        var FailedTestList = [];
        var TestLine;

        // Read in the test list
        while (!TestListFile.AtEndOfStream) {
            TestLine = TestListFile.ReadLine();
            if (!TestLine.match(/^#/) && !TestLine.match(/^ *$/))
            {
                // Skip empty lines and comment lines with a leading #
                TestList.push(TestLine);
                TestCount++;
            }
        }
        TestListFile.Close();

        // Now actually run the tests
        for (var i = 0; i < TestList.length; i++) {
            TestExe = TestList[i];

            TestNumber++;
            TestRun = runCmd("call " + bvtRoot + "\\runtest.bat "
                                     + targetIP + " "
                                     + TestExe + " "
                                     + runtime + " "
                                     + bvtDir + " "
                                     + outDir,
                             runOpts);

            if (TestRun.exitCode != 100) {
                FailCount++;
                FailedTests += "\t" + TestExe + " (exit code: " + TestRun.exitCode + ")\n";
                logMsg(LogBeagle, LogInfo, "Test failed:\n" + TestRun.output);
                testPassFailMsg = " FAILED";
                FailedTestList.push(TestExe);
            } else {
                PassCount++;
                testPassFailMsg = "";
            }

            testMsg = TestNumber + "/" + TestCount + " (pass: " + PassCount + ", fail: " + FailCount + "): " + TestExe + testPassFailMsg + "\n";
            logMsg(LogBeagle, LogInfo, testMsg);
        }
    }

    if (FailCount == 0) {
        logMsg(LogBeagle, LogInfo, "\n} beagleBVT all tests passed.\n");
        return 0;
    } else {
        logMsg(LogBeagle, LogInfo, "\n} beagleBVT tests failed.\nTest failures:\n", FailedTests);

        if (FailedTestList.length > 0) {
            var FailedTestListFile = FSOCreateTextFile(FailedListFileName, true, false);
            for (var i = 0; i < FailedTestList.length; i++) {
                FailedTestListFile.WriteLine(FailedTestList[i]);
            }
            FailedTestListFile.Close();
            logMsg(LogBeagle, LogInfo, "\nTo rerun just the failures (this will overwrite previous output):\n  runjs beagleBVT ", outDir,
                   " ", ddsDir, " ", runtime, " ", bvtDir, " ", targetIP, " ", bvtRoot, " ", FailedListFileName, "\n");
        }

        return -1;
    }
}

/****************************************************************************/
/* Task to run BVT tests on Beagle Board

  logs go to %outDir%\<bldArch><bldType>\test.BeagleBVT

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    binDir      : The directory to get the runtime binaries from
    targetIP    : The IP address of the beagle board (default: uses getBeagleIP.js)
    dependencies: Any dependents you want for this task (like a build)
*/

function _beagleBVTTask(bldType, binDir, targetIP, dependencies) {

    if (bldType == undefined)
    {
        bldType = "core" + Env("_BuildType");
        if (!Env("_BuildType"))
            throw Error(1, "Arg bldType not supplied");
    }

    if (dependencies == undefined)
    {
        dependencies = [_ngenBVTTask(bldType), _fxpBeagleSetupTask(bldType, binDir, targetIP), _prepBeagleBoardTask(), _razzleBuildTask(bldType, "arm")];
    }

    if (targetIP == undefined)
        targetIP = "_";

    var taskName = "test.beagleBVT@ARM" + bldType;
    var logDir = "%outDir%\\ARM" + bldType + "\\test.BeagleBVT";
    var extRootOverride = _constructInstallPath(bldType);
    var bvtDir = _constructBvtPath(bldType);

    var bvtTestList;
    if (bldType.match(/coredbg/i))
    {
        // Use a different test list for DBG runs, to make them faster
        bvtTestList = "DBGTestList.txt";
    }
    else
    {
        bvtTestList = "TestList.txt";
    }

    var ret = taskNew(taskName,
        "runjs beagleBVT " + logDir + " "
                      + "%srcBase%\\ddsuites "
                      + extRootOverride + " "
                      + bvtDir + " "
                      + targetIP + " "
                      + "_ "
                      + bvtTestList,
        dependencies, "local");

    ret.description = "Run the group of tests that developers are required to run before checkin on the beagle board." +
                      " This task assumes that the runtime has already been installed.";
    return ret;
}

/*****************************************************************************/
/* Runs the beagleEHBvts (a set of EH tests that check for asserts, but don't
   verify test output). This is similar to beagleBVT, but the test list format
   is slightly different: each line has relative path followed by space followed
   by test name. Each test is expected to depend on a "common.dll" in the same
   directory.

     Parameters:
       outDir   : Where to place the test results
       ddsDir   : The base of the ddSuites tree
       runtime  : Which runtime directory to use (on the device)
       bvtDir   : Which bvt directory to use (on the device)
       targetIP : The IP address of the beagle board (default: uses getBeagleIP.js)
       bvtRoot  : The test bvt root (on the host)
       bvtTestList : The file name with the list of tests to run. The file lives
                     in bvtRoot if not qualified with a path. (default: EHTestList.txt)
*/

function beagleEHBVT(outDir, ddsDir, runtime, bvtDir, targetIP, bvtRoot, bvtTestList) {

    if (ddsDir == undefined) {
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "beagleEHBVT: Required argument ddsDir is missing");
    }
    if (outDir == undefined) {
        outDir = ddsDir + "\\src\\coreclr\\ARM\\test.BeagleEHBVT";
    }

    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    if (bvtRoot == undefined)
    {
        bvtRoot = ddsDir + "\\src\\coreclr\\ARM";
    }

    if (runtime == undefined)
    {
        runtime = _constructInstallPath();
    }

    if (bvtDir == undefined)
    {
        bvtDir = _constructBvtPath();
    }

    if (bvtTestList == undefined)
    {
        bvtTestList = "EHTestList.txt";
    }

    logMsg(LogBeagle, LogInfo, "beagleEHBVT(", outDir, ", ", ddsDir, ", ", runtime, ", ", bvtDir, ", ", targetIP, ", ", bvtRoot, ", ", bvtTestList, ") {\n");

    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    _prepBeagleBVT(outDir, ddsDir, bvtDir, targetIP, bvtRoot);

    var testMsg = "";
    var FailedTests = "";
    var PassCount = 0;
    var FailCount = 0;
    var TestNumber = 0;
    var TestCount = 0;
    var TestRun;
    var testPassFailMsg;

    var TestListFileName;
    var TestListDir;
    if (bvtTestList.match(/^(.*)\\([^\\]*)$/)) {
        TestListDir = RegExp.$1;
        TestListFileName = RegExp.$2;
    }
    else {
        TestListDir = bvtRoot;
        TestListFileName = bvtTestList;
    }
    bvtTestList = TestListDir + "\\" + TestListFileName;
    var FailedListFileName = outDir + "\\Failed_" + TestListFileName;
      
    var runOpts = runSetExpectedReturnCode(100,
                  runSetNoThrow());

    var TestListFile = FSOOpenTextFile(bvtTestList, FSOForReading);
    var TestList = [];
    var FailedTestList = [];
    var TestLine;

    // Read in the test list
    while (!TestListFile.AtEndOfStream) {
        TestLine = TestListFile.ReadLine();
        if (!TestLine.match(/^#/) && !TestLine.match(/^ *$/))
        {
            // Skip empty lines and comment lines with a leading #
            TestList.push(TestLine);
            TestCount++;
        }
    }
    TestListFile.Close();

    // Now actually run the tests
    for (var i = 0; i < TestList.length; i++) {
        TestLine = TestList[i]; // in EHTestList.txt, we get the test directory followed by the test name, separated by a space

        var testArray = TestLine.split(" ");
        var TestPath = testArray[0];
        var TestName = testArray[1];

        var TestExe = TestPath + "\\" + TestName;

        TestNumber++;
        TestRun = runCmd("call " + bvtRoot + "\\runEHtest.bat "
                                 + targetIP + " "
                                 + TestPath + " "
                                 + TestName + " "
                                 + runtime + " "
                                 + bvtDir + " "
                                 + outDir,
                         runOpts);

        if (TestRun.exitCode != 100) {
            FailedTests += "\t" + TestExe + " (exit code: " + TestRun.exitCode + ")\n";
            FailCount++;
            logMsg(LogBeagle, LogInfo, "Test failed:\n" + TestRun.output);
            testPassFailMsg = " FAILED";
            FailedTestList.push(TestLine);
        } else {
            PassCount++;
            testPassFailMsg = "";
        }

        testMsg = TestNumber + "/" + TestCount + " (pass: " + PassCount + ", fail: " + FailCount + "): " + TestExe + testPassFailMsg + "\n";
        logMsg(LogBeagle, LogInfo, testMsg);
    }

    if (FailCount == 0) {
        logMsg(LogBeagle, LogInfo, "\n} beagleEHBVT all tests passed.\n");
        return 0;
    } else {
        logMsg(LogBeagle, LogInfo, "\n} beagleEHBVT tests failed.\nTest failures:\n", FailedTests);

        if (FailedTestList.length > 0) {
            var FailedTestListFile = FSOCreateTextFile(FailedListFileName, true, false);
            for (var i = 0; i < FailedTestList.length; i++) {
                FailedTestListFile.WriteLine(FailedTestList[i]);
            }
            FailedTestListFile.Close();
            logMsg(LogBeagle, LogInfo, "\nTo rerun just the failures (this will overwrite previous output):\n  runjs beagleEHBVT ", outDir,
                   " ", ddsDir, " ", runtime, " ", bvtDir, " ", targetIP, " ", bvtRoot, " ", FailedListFileName, "\n");
        }

        return -1;
    }
}

/****************************************************************************/
/* Task to run EH BVT tests on Beagle Board

  logs go to %outDir%\<bldArch><bldType>\test.BeagleEHBVT

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    binDir      : The directory to get the runtime binaries from
    targetIP    : The IP address of the beagle board (default: uses getBeagleIP.js)
    dependencies: Any dependents you want for this task (like a build)
*/

function _beagleEHBVTTask(bldType, binDir, targetIP, dependencies) {

    if (bldType == undefined)
    {
        bldType = "core" + Env("_BuildType");
        if (!Env("_BuildType"))
            throw Error(1, "Arg bldType not supplied");
    }

    if (dependencies == undefined)
    {
        dependencies = [_ngenBVTTask(bldType), _fxpBeagleSetupTask(bldType, binDir, targetIP), _prepBeagleBoardTask(), _razzleBuildTask(bldType, "arm")];
    }

    if (targetIP == undefined)
        targetIP = "_";

    var taskName = "test.beagleEHBVT@ARM" + bldType;
    var logDir = "%outDir%\\ARM" + bldType + "\\test.BeagleEHBVT";
    var extRootOverride = _constructInstallPath(bldType);
    var bvtDir = _constructBvtPath(bldType);

    var ret = taskNew(taskName,
        "runjs beagleEHBVT " + logDir + " "
                      + "%srcBase%\\ddsuites "
                      + extRootOverride + " "
                      + bvtDir + " "
                      + targetIP + " ",
        dependencies, "local");

    ret.description = "Run the EH group of tests on the beagle board." +
                      " This task assumes that the runtime has already been installed.";
    return ret;
}

/*****************************************************************************/
/* Runs ngen of mscorlib.

     Parameters:
       outDir   : Where to place the test results
       ddsDir   : The base of the ddSuites tree
       runtime  : Which runtime directory to use (on the device)
       targetIP : The IP address of the beagle board (default: uses
                  getBeagleIP.js)
       bvtRoot  : The test bvt root (on the host)
*/

function ngenBVT(outDir, ddsDir, runtime, targetIP, bvtRoot) {

    if (ddsDir == undefined) {
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "ngenBVT: Required argument ddsDir is missing");
    }
    if (outDir == undefined) {
        outDir = ddsDir + "\\src\\coreclr\\ARM\\test.ngenBVT";
    }

    if (targetIP == undefined)
    {
        targetIP = _getBeagleIP();
    }

    if (bvtRoot == undefined)
    {
        bvtRoot = ddsDir + "\\src\\coreclr\\ARM";
    }

    if (runtime == undefined)
    {
        runtime = _constructInstallPath();
    }

    logMsg(LogBeagle, LogInfo, "ngenBVT(", outDir, ", ", ddsDir, ", ", runtime, ", ", targetIP, ", ", bvtRoot, ") {\n");

    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    var AllTestsPassed = true;

    var runOpts = runSetExpectedReturnCode(0,
                  runSetNoThrow(
                  runSetLog(LogBeagle, LogInfo)));

    var run = runCmdToLog("call " + bvtRoot + "\\ngenBVT.bat "
                                            + targetIP + " "
                                            + runtime + " "
                                            + outDir,
                          runOpts);

    logMsg(LogBeagle, LogInfo, "Ngen exitCode = ", run.exitCode, "\n");

    if (run.exitCode != 0) {
        AllTestsPassed = false;
    }

    if (AllTestsPassed) {
        logMsg(LogBeagle, LogInfo, "} ngenBVT passed.\n");
        return 0;
    } else {
        logMsg(LogBeagle, LogInfo, "} ngenBVT failed.\n");
        return -1;
    }
}

/****************************************************************************/
/* Task to run ngen of mscorlib.

  logs go to %outDir%\<bldArch><bldType>\test.ngenBVT

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    binDir      : The directory to get the runtime binaries from
    targetIP    : The IP address of the beagle board (default: uses getBeagleIP.js)
    dependencies: Any dependents you want for this task (like a build)
*/

function _ngenBVTTask(bldType, binDir, targetIP, dependencies) {

    if (bldType == undefined)
    {
        bldType = "core" + Env("_BuildType");
        if (!Env("_BuildType"))
            throw Error(1, "Arg bldType not supplied");
    }

    if (dependencies == undefined)
    {
        dependencies = [_fxpBeagleSetupTask(bldType, binDir, targetIP), _prepBeagleBoardTask()];
    }

    if (targetIP == undefined)
        targetIP = "_";

    var taskName = "test.ngenBVT@ARM" + bldType;
    var logDir = "%outDir%\\ARM" + bldType + "\\test.ngenBVT";
    var extRootOverride = _constructInstallPath(bldType);

    var ret = taskNew(taskName,
        "runjs ngenBVT " + logDir + " "
                    + "%srcBase%\\ddsuites "
                    + extRootOverride + " "
                    + targetIP + " ",
        dependencies, "local");

    ret.description = "Attempts to ngen mscorlib.dll to detect any regressions affecting ngen.";

    return ret;
}
