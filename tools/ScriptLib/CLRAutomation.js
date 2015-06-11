/*****************************************************************************/
/*                              CLRAutomation.js                             */
/*****************************************************************************/

/* Generic routines that are useful, but may be specific to the CLR automation
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

/* AUTHOR: Vance Morrison
   DATE: 1/2/03 */

/******************************************************************************/
var ClrAutomationModuleDefined = 1;              // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");

var LogClrAutomation = logNewFacility("CLRAutomation");

if (ScriptDir == undefined)
        var ScriptDir  = WshFSO.GetParentFolderName(WshFSO.GetParentFolderName(WScript.ScriptFullName));

if (WshShell == undefined)
        var WshShell = WScript.CreateObject("WScript.Shell");

if (Env == undefined)
        var Env = WshShell.Environment("PROCESS");

    // This variable sets defaults that will hold true for all
    // commands spawned by CLRAutomation.  Note that we really
    // want these to be things that never need to change.
var clrRunTemplate =
    runSetEnv("COMPLUS_StressLog", 1,
    runSetEnv("COMPLUS_DbgJITDebugLaunchSetting", 0));        // cause a popup on unhandled exceptions

var MINUTE = 60;
var HOUR   = 60*60;

// logSetFacilityLevel(LogProc, LogInfo10000);


/******************************************************************************/
/* Returns the real type of processor, not the WOW version even if running
   in a WOW environment.
*/

function getRealProcessorArchitecture() {
    // First check if we are running on WOW
    var w6432Arch = Env("PROCESSOR_ARCHITEW6432").toLowerCase();
    if (w6432Arch)
        return w6432Arch;

    // If this is not WOW, check the architecture
    var procArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    if (!procArch)
        throw new Error(1, "PROCESSOR_ARCHITECTURE is not set");
    return procArch;
}

// returns the ldr64.exe location
function getLdr64Exe( ) {
    var ldr64Exe = ScriptDir + "\\" + getRealProcessorArchitecture() + "\\ldr64.exe";

    if (!FSOFileExists(ldr64Exe)) 
    {
        throwWithStackTrace(new Error(1, "Could not find " + ldr64Exe + " to run bvts in the WOW"));
    }
    return ldr64Exe;
}

function StandardBinDir(srcBase, bldType, bldArch)
{
    return srcBase + ".binaries." + bldArch + bldType;
}

/****************************************************************************/
/* Run a command, and handle running in WoW
   In the case that 'targetArch' is x86 and the machine is 64 bit, it runs
   the command under the x86 WoW subsystem.  It also uses ldr64 to force all
   runtime programs (which have not been compiled for a specific platform)
   to run under the x86 version of the runtime.

  Parameters
    command    : Command to execute
    targetArch : architecture to use for executing the command (eg. x86, amd64, ia64)
    runOpts    : options for executing the command
*/
function runWithRuntimeArch(command, targetArch, runOpts) {
 
    if (targetArch == undefined) 
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    if (runOpts == undefined)
        runOpts = runSetLog(LogRun, LogInfo, clrRunTemplate);
 
    var ldr64ExeForWoW = undefined;

    // Check for WoW presence
    var isWowCurrentlyOn = false;
    var shouldUseWow = ShouldUseWow64(targetArch);
    if (shouldUseWow) {
        // this machine has a wow, check if wow is on.
        // ldr64 query returns 0 if in setwow mode, and 1 if in set64 mode.
        // note that the IA64 ldr64 current has a problem where it can occasionally
        // return STATUS_ILLEGAL_INSTRUCTION.  Until this is resolved, we'll err on the side
        // of caution (for the current scenarios where this could be an issue) and assume that 
        // we're in set64 mode.  Currently the only scripts that actually expect to run in 
        // wow mode most of the time are test automation scripts (snap and DDR are in 64-bit
        // mode by default).
        ldr64ExeForWoW = getLdr64Exe();
        var queryRun = runCmd(ldr64ExeForWoW + " query", runSetNoThrow());
        var queryExit = queryRun.exitCode;
        if (queryExit == 0)
            isWowCurrentlyOn = true;
        else if (queryExit != 1)
            logMsg(LogClrAutomation, LogWarn,
                "Unexpected return code from ldr64: " + queryExit + ", assuming 64-bit mode is enabled.\n" +
                "  ldr64 output: \"" + queryRun.output + "\"\n"); 
    }
            
    if (shouldUseWow != isWowCurrentlyOn) {
        // We need to set the machine to the right state
        logMsg(LogClrAutomation, LogWarn, "Setting registry so that " + 
            (shouldUseWow ? "ALL managed code runs in the WOW\n" : "managed code can run as 64-bit\n"));
        logMsg(LogClrAutomation, LogWarn, "use '", ldr64ExeForWoW, "' to revert\n");

        logMsg(LogClrTask, LogWarn, ldr64ExeForWoW + (shouldUseWow?" setwow":" set64") + "\n");
        runCmd(ldr64ExeForWoW + (shouldUseWow?" setwow":" set64") );
    }

    if (shouldUseWow) {
        // use runSet32Bit to force the runCmd to use a 32-bit command shell 
        runOpts = runSet32Bit(runOpts);
    } else if (ShouldUseNative64(targetArch)) {
        // This is x64 installation from WOW64 cmd window, open correct native (x64) cmd window
        runOpts = runSet64Bit(runOpts);
    }
    
    //
    // Do the real work
    //
    try { 
        var run = runCmd(command, runOpts);
    } 
    finally {
        // Check if we need to do cleanup the WoW settings
        if ( shouldUseWow != isWowCurrentlyOn ) {
            logMsg(LogClrAutomation, LogInfo, "Resetting registry to run " + (isWowCurrentlyOn?"WOW":"64 bit") + " version of runtime\n");
            logMsg(LogClrTask, LogWarn, ldr64ExeForWoW + (isWowCurrentlyOn?" setwow":" set64") + "\n");
            runCmd(ldr64ExeForWoW + (isWowCurrentlyOn?" setwow":" set64"));
        }
    }

    return run;
}
 

/******************************************************************************/
/* calcluate the best SNAP job directory for the source base 'srcBase' and
   return that informaton as a structre with three fields 

    branchName        - The name of the branch associated with the CLR (eg puclr)
    changeNumber    - The change number that the CLR and FX trees are synced to
    snapJobDir        - The SNAP directory where the checked in baseline builds are
 */
function getClrFxBaselineData(srcBase) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase is missing");
    }
        
    var whereSyncMinFx = sdWhereSynced(srcBase + "\\ndp\\fx\\src", srcBase);
    var whereSyncMinClr = sdWhereSynced(srcBase + "\\ndp\\clr\\src", srcBase);

        // I want smallest region, which means I want the maximum of the CLR and FX sync points
    var syncChange = whereSyncMinFx.minChange;
    var syncChangeTime = whereSyncMinFx.minChangeTime;
    if (syncChange < whereSyncMinClr.minChange) {
        syncChange = whereSyncMinClr.minChange;
        syncChangeTime = whereSyncMinClr.minChangeTime;
    }

    logMsg(LogClrAutomation, LogInfo, "source base seems to be synced to ", syncChange, "\n");
    
    var branchName;  // The name of the folder on clrsnap0, NOT the TFS server path.
    if (_inTFS())
        branchName = "puclr";
    else
        branchName = srcBase.match(/\\([^\\]*)$/)[1];

    if (FSOFolderExists("\\\\clrsnap0\\public") && !FSOFolderExists("\\\\clrsnap0\\public\\" + branchName)) {
        logMsg(LogClrAutomation, LogInfo, "Warning: souce base directory: ", srcBase, " does not seem to be a branch name, defaulting to tfs_puclr\n");
        branchName = "tfs_puclr";
    }

    logMsg(LogClrAutomation, LogInfo, "The branch name is ", branchName,  "\n")
    var publicBuilds = "\\\\clrsnap0\\public\\" + branchName + "\\builds";

    var builds = FSOGetDirPattern(publicBuilds, /^\d+$/);
    logMsg(LogClrAutomation, LogInfo, "Searching ", publicBuilds, ".  Found ", builds.length, " builds\n");
    if (builds.length > 0) {
        logMsg(LogClrAutomation, LogInfo10, "FirstBuild = ", builds[0], "\n");
        logMsg(LogClrAutomation, LogInfo10, "LastBuild  = ", builds[builds.length-1], "\n");
    }

    var snapJobDir = undefined;
    var snapChange;
    for(var i = 0; i < builds.length; i++) {
        if (!builds[i].match(/(\d+)$/))
            continue;
        var nextSnapChange = parseInt(RegExp.$1);
        if (nextSnapChange > syncChange) 
            break;
        snapChange = nextSnapChange;
        snapJobDir = builds[i];
    }

    if (snapJobDir == undefined)
        return undefined;
    
    return { snapJobDir:snapJobDir, 
             snapChange:snapChange, 
             syncChange:syncChange, 
             syncTime:syncChangeTime, 
             branchName:branchName 
            };
    }

 
/******************************************************************************/
/* run a command, wait for it to idle, then send it a windows QUIT message
   and wait for it to close.  This allows you to run GUI apps as end-to-end
   scenarios.  Note that this works fine for non-gui apps too (since it
   dies before it idles

   Parameters:
     cmd        : command to run
     runOpts    : options to pass to 'runCmd (env vars etc)'
 */

function runToIdleAndQuit(cmd, runOpts) {

    if (runOpts == undefined)
        runOpts = runSetLog(LogRun, LogInfo, clrRunTemplate);

    var cmdName = cmd.match(/(\S+)/)[1];
    var run = runDo(cmd, runOpts);
    if(runWaitForIdle(run, 5)) {
            // run 'kill' which because it is not /f, will set a QUIT EVENT to it to signal 
            // the gui app to close cleanly.  This is important since profile data is stored 
            // during orderly shutdown. 
        var pid = procDeepestChild(run.getPid());
        logMsg(LogClrAutomation, LogInfo, "runToIdleAndQuit: RUNNING kill ", pid, " to send quit signal\n");
        runCmd(ScriptDir + "\\kill " + pid);
        runWait(run);
    }

    return run.exitCode;
}

/******************************************************************************/
/* Returns string that contains the path to the registry root for
   HKLM\Software\Microsoft\.NETFramework for the specified targetArch
   Handles the case where targetArch is supported by using the Wow64
*/
function GetNetFrameworkRoot(targetArch)
{
    // default is currently active processor architecture
    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    var netFrameworkRoot = "HKLM\\SOFTWARE\\";

    if (NeedToEnterWow64(targetArch)) {
        netFrameworkRoot += "Wow6432Node\\";
    }
    netFrameworkRoot += "Microsoft\\.NETFramework\\";

    return netFrameworkRoot;
}

/* Returns a boolean indicating whether a given architecture is 64bit or not,
 "in context", of the CLR
  
  targetArch - The target architecture (x86, amd64 ia64) to be validated
         clr - Type of CLR in whose context validation needs to be done.
               For desktop CLR, leave this undefined. 
               For CoreCLR, pass "coreclr".
  
  The context is required as some CLR implementations may not support a
  64bit platform. An example of this is CoreCLR, which simply supports 
  AMD64. In its case, returning true for IA64 will be semantic 
  incorrectness.

*/
function Is64bitArch(targetArch, clr)
{
    if (targetArch == undefined)
        return false;
    
    var fIs64Bit = false;
        
    var arch = targetArch.toLowerCase();
    var clrType = "";
    if (clr != undefined)
    {
        clrType = clr.toLowerCase();
    }
    
    if (arch == "amd64")
        fIs64Bit = true;
    
    // If we are not on CoreCLR, then also validate for IA64
    if (clrType != "coreclr")
    {
        // Desktop CLr supports IA64 too
        if (arch == "ia64")
           fIs64Bit = true;
    }
    
    return fIs64Bit;
}

/******************************************************************************/
/*   NeedToEnterWow64:  returns true when we should use the Wow64 

   Parameters:
     targetArch - The target architecture (x86, amd64 ia64) that we want to execute for

   Returns true if targetArch can run on this machine using the Wow64
     and our current environment is a 64-bit command shell

   returns false if we don't need to use the Wow64 for targetArch 
    also returns false if we are already in a Wow64 x86 environment and targetArch is x86

   throws an Error() if we can not execute using targetArch on this processor
*/

function NeedToEnterWow64(targetArch)
{
    if (targetArch == undefined) 
        throw new Error(1, "Required argument targetArch is missing");
    
    targetArch = targetArch.toLowerCase();
    //
    // Is targetArch different that our current architecture environment?
    //
    if (targetArch != Env("PROCESSOR_ARCHITECTURE").toLowerCase())
    {
        var realArch = getRealProcessorArchitecture().toLowerCase();
        
        // Do we have different architectures?
        if (targetArch != realArch) {
            
            // WoW64 only supports running X86 processes
            if (!targetArch.match(/x86/i)) 
                throw Error(1, "Mismatch architectures, want " + targetArch + " running on " + realArch);

            return true;
        }
    }
    return false;
}

function ShouldUseWow64(targetArch)
{
    if (targetArch == undefined) 
        throw new Error(1, "Required argument targetArch is missing");

    //
    // Is targetArch different that our current architecture environment?
    //
    var realArch = getRealProcessorArchitecture().toLowerCase();
    targetArch = targetArch.toLowerCase();
        
    // Do we have different architectures?
    if (targetArch != realArch) {
        
        // WoW64 only supports running X86 processes
        if (!targetArch.match(/x86/i)) 
            throw Error(1, "Mismatch architectures, want " + targetArch + " running on " + realArch);

        return true;
    }
    return false;
}

/******************************************************************************/
/* ShouldUseNative64: Returns true when we are in WOW64, but we want to target x64.
*/
function ShouldUseNative64(targetArch)
{
    if (targetArch == undefined) 
        throw new Error(1, "Required argument targetArch is missing");
    
    targetArch = targetArch.toLowerCase();
    
    if (targetArch != Env("PROCESSOR_ARCHITECTURE").toLowerCase())
    {
        if (targetArch == getRealProcessorArchitecture().toLowerCase())
        {   // We should escape this WOW64 window
            return true;
        }
        // Is it architecture mismatch? (It is OK to target x86 = WOW64 from x64 window)
        if (!targetArch.match(/x86/i))
            throw Error(1, "Mismatch architectures, want " + targetArch + " running on " + realArch);
    }
    return false;
}

/******************************************************************************/
/********************************* BUILDING ***********************************/
/******************************************************************************/

/*****************************************************************************/
/* Build some part of of the VBL using razzle.

  Parameters
    bldType: Type of build (e.g., dbg, chk, fre, ret, coredbg, corechk, corefre, coreret.
             The "core" versions are pseudo-types to build CoreCLR, and are not understood by razzle.)
    bldArch: Architecture (e.g., x86, ia64, amd64)
    bldDirList: Directory to build (relative to srcBase if does not being with '.' or \), or a list of dirs separated by ';'
    bldArgs: Extra args to build.exe (e.g., -P tells build to print times)
    srcBase: The base of the VBL (so we can find razzle)
    binDir : Where to place the binaries
    logDir : Where to put the build*.* files
    timeOut: time out (default 140 minutes)
    rArgs  : Additional args to razzle (e.g., Offline) (default none)
    runOpts: Additional options to pass to 'runCmd (env vars etc)' (default none)
    preBld : Additional commands to execute within razzle just before invoking the build (default none)
    oacrAll: OACR check all files to simulate TVS gate run. (default only check sd opened files)
*/
function razzleBuild(bldType, bldArch, bldDirList, bldArgs, srcBase, binDir, logDir, timeOut, rArgs, runOpts, preBld, oacrAll) {

    if (bldType == undefined) {
        bldType = Env("_BuildType");
        if (!bldType)
            bldType = "chk";
    }
    if (bldArch == undefined) {
        bldArch = Env("_BuildArch");
        if (!bldArch)
            bldArch = "x86";
    }
    if (bldArgs == undefined)
        bldArgs = "";

    if (bldArgs != "")
    {
        if (!bldArgs.match(/^\s*-/)) 
        {
            throw new Error(1, "Illegal bldArgs: \"" + bldArgs + "\""); 
        }
    }

    var multiDirBuild = false;
    var bldDir = ".";
    if (bldDirList != undefined) {
        var components = bldDirList.split(';');

        if (components.length > 1) {
            multiDirBuild = true;
            bldDir = components[0];
            logMsg(LogClrAutomation, LogInfo, "Multidir build- using " + bldDir + " as the build directory for log names\n");
        }
        else {
            bldDir = bldDirList;
        }
    }

    if (srcBase == undefined)
        srcBase = srcBaseFromScript();

    if (binDir == undefined || binDir == "razzleBuildDefaultBinDir")
        binDir = StandardBinDir(srcBase, bldArch, bldType);
    if (rArgs == undefined)
        rArgs = "";
    if (timeOut == undefined)
        timeOut = 140 * MINUTE;
    if (runOpts == undefined)
        runOpts = clrRunTemplate;
    if (preBld == undefined)
        preBld = "";

    // if it starts with a dot, it is a relative path to the current directory
    if (bldDir.match(/^\.((\\.*)|$)/))
        bldDir = WshShell.CurrentDirectory + RegExp.$1;
    // is it an absolute path?
    if (bldDir.match(/^((\w\:)|\\)/)) {
        var prefix = srcBase.toLowerCase();
        bldDir = bldDir.toLowerCase();
        if (bldDir.indexOf(prefix) != 0)
            throw new Error(1, "If a true path is given for directory: " + bldDir + " it must begin with srcBase: " + prefix); 
        bldDir = bldDir.substr(prefix.length);
        bldDir = bldDir.replace(/^\\/, "");
        logMsg(LogClrAutomation, LogInfo, "Got relative bldDir ", bldDir, "\n");
    }
    if (logDir == undefined)
        logDir = srcBase + "\\" + bldDir;

    bldArch = bldArch.toLowerCase();
    
    logMsg(LogClrAutomation, LogInfo, "razzleBuild: BUILDING ", srcBase, "\\", bldDir, " for architecture ", bldArch, "\n");
    if (!bldArch.match(/^(x86)|(amd64)|(ia64)|(arm)/i))
        throw new Error(1, "Unknown build architecture: " + bldArch);

    // Linking and compiling take up to three times longer on IA64 systems
    if (bldArch.match(/ia64/i))
        timeOut = timeOut * 3;

    var verifyBuild = false;
    if (bldDir == "ndp\\verify")
        verifyBuild = true;

    FSOCreatePath(binDir);
    binDir = FSOGetFolder(binDir).Path;             // Get absolute path  since command below changes CD
    FSOCreatePath(logDir);
    logDir = FSOGetFolder(logDir).Path;
    srcBase = FSOGetFolder(srcBase).Path;

        // This is a bit of a hack.  I have had problems time to time with linking because looking for symbols like _IID_ICorDebugValue2
        // I tracked this down the following error in BUILD
        //        BUILD: c:\vbl\lab21s\ndp\clr\src\inc\obj1r\i386: ignoring second instance of cordebug_i.c
        //        BUILD: c:\vbl\lab21s\ndp\clr\src\inc: Ignoring invalid SOURCES= entry: obj1r\i386\cordebug_i.c
        // Because of this cordebug_i.c does not get compiled and thus the link errors.
        // Looking at the source to BUILD, it believe an entry in the build.dat file is redundant, when clearly it is not
        // As a hack to work around, we will delete the build.dat file, becasue the most likely issue is some corruption
        // there from previous builds -vancem
    var buildFile = srcBase + "\\build.dat";
    if (FSOFileExists(buildFile) && bldArgs.match(/-\S*c/)) {
        logMsg(LogClrAutomation, LogInfo, "razzleBuild: deleting ", buildFile, " file\n");
        FSODeleteFile(buildFile, true);
    }

    var bldLogFileName = "razzleBuild." + bldDir.replace(/[\\]/g, "-") + "." + bldArch + bldType;
    var bldCmd = "build.exe -PMF " + bldArgs + " -M " + Env("NUMBER_OF_PROCESSORS") + " -jpath " + logDir + " -j " + bldLogFileName;
    if (multiDirBuild) {
        bldCmd += " /dir " + bldDirList;
    }

    logMsg(LogClrAutomation, LogInfo, "Build command: " + bldCmd);

    var preBatch;
    preBld += "\npushd " + srcBase + "\\" + bldDir;
    preBld += "\n@echo on\ncall oacr clean all";
    if (oacrAll == "oacrAll") {
        preBld += "\ncall oacr set all\ncall oacr set batch all\n";
    }
    if (preBld != undefined)
    {
        // We want to execute some commands before invoking build.exe - use a temporary batch file
        // Usually we could just pass this to razzle via Exec option (followed with ^&), but razzle's mechanism for implementing
        // Exec is stupid (%1 .. %9) - it filters out some characters (like '=') and is limited to 9 arguments.  If razzle.cmd
        // were to use "%*" instead, none of this would be necessary
        preBatch = FSOGetTempPath("runjsPreBuild-") + ".bat";
        FSOWriteToFile(preBld + "\n" + bldCmd + "\n", preBatch);
        bldCmd = "call " + preBatch;
    }
    var baseBldType;
    if (isCoreCLRBuild(bldType)) {
        baseBldType = bldType.substr(4);
        rArgs += " CoreCLR";
    } else {
        baseBldType = bldType;
    }
    // Actually execute the build under razzle
    var run = runCmdToLog("call " +
        srcBase + "\\tools\\razzle.cmd sharepublic " + bldArch + " " + baseBldType + 
                  " " + rArgs + " Exec " + bldCmd,
        runSetDump("runjs dumpProc %p " + logDir + "\\" + bldLogFileName + ".dmpRpt",
        runSetTimeout(timeOut,
        runSetNoThrow(
        runSetEnv("_NT_SYMBOL_PATH", runGetEnv("_NT_SYMBOL_PATH") + ";" + binDir  + "\\symbols.pri\\retail" +
            ";" + srcBase + "\\tools\\x86\\managed\\v4.0",  // in case we fail while running our runtime
        runSet32Bit(
        runOpts))))));

    if (preBatch != undefined)
        FSODeleteFile(preBatch);
    
    // Sigh, this is a hack, but if you do a full build to a non-standard binDir, it basically
    // leaves the standard binDir in a state that needs to be rebuilt from scratch.  Thus we
    // handle the common case, and synchronize these by copying what we just built back
    // to the standard place.   -vancem
    // We do NOT do this for link-only builds or the verifyBuild used by optimizeNDP.js -- briansul
    var standardBinDir = StandardBinDir(srcBase, bldArch, bldType);
    if (binDir != standardBinDir && FSOFolderExists(standardBinDir) && !bldArgs.match(/-\S*l/) && !verifyBuild) {
        try {
            logMsg(LogClrAutomation, LogInfo, "To allow normal rbuild to the standard binaries directory to work,\n");
            logMsg(LogClrAutomation, LogInfo, "copying files from ", binDir, " -> ", standardBinDir, "\n");
            FSOCopyFolder(binDir, standardBinDir, true);
        } catch(e) {
            logMsg(LogClrAutomation, LogWarn, "Could not copy all the files to the standard binaries directory, exception: ", e.description, "\n");
        }
    }

    var errLogs = FSOGetFilePattern(logDir, bldLogFileName + ".err");
    for(var i = 0; i < errLogs.length; i++) {
        var errLog = errLogs[i];
        logMsg(LogClrAutomation, LogInfo, "razzleBuild: building error report for ", errLog, "\n");
        try { buildReport(errLog); }
        catch(e) { logMsg(LogClrAutomation, LogWarn, "razzleBuild: could not build log report for ", errLog, " exception = ", e.description, "\n"); }
    }

    if (run.exitCode != 0)
        throw new Error(1, "razzleBuild: main build command returned non-zero exit code, build logs in " + logDir + "\\" + bldLogFileName + ".*");
}


//---------------------------------------------------------------------------------------
// 
// Gets path in the enlistment from the file name (relative to basePath).
// Deals also with Mac path (converts it to Windows separator '\').
// 
// fileName - incl. relative or absolute path.
// 
function _GetRelativePathInEnlistment(fileName, basePath)
{
    var path = fileName;
    if (!path.match(/^\/|.:\\/) && (basePath != undefined))
    {   // It is relative path
        path = basePath + "\\" + fileName;
    }
    
    // Convert Mac path to Windows directory separator
    path = path.replace(/[\/]/g, "\\");
    // Replace '\.\' and '\\' with '\'
    path = path.replace(/\\\.?\\/g, "\\");
    // Replace '\dir\..\' with '\'
    do
    {
        var oldPath = path;
        path = path.replace(/\\[^\\]+\\\.\.\\/, "\\");
    }
    while (path != oldPath);
    
    // Guess the enlistment root from known enlistment root subdirectories
    path = path.replace(/^.*\\((ndp|rotor|public|tools)\\)/, "$1");
    
    return path;
}

//---------------------------------------------------------------------------------------
// 
// Create directoryBuild object from logLines (array of lines from .log file for a directory) 
// and list of errors (parsed .err file).
// 
function _CreateDirectoryBuild(logLines, processor, errors)
{
    // Array of LineInfo objects
    var lines = [];
    
    logMsg(LogClrAutomation, LogInfo100, "_CreateDirectoryBuild() {\n");
    
    var directoryBuild = new Object();
    directoryBuild.processor = processor;
    directoryBuild.lines = lines;
    
    // Parse basic line information
    for (var i = 0; i < logLines.length; i++)
    {
        var lineInfo = new Object();
        lines[i] = lineInfo;
        
        lineInfo.logLine = logLines[i];
        // Remove processor prefix '4>' from the line
        lineInfo.line = lineInfo.logLine.replace(/^(\d+)>/, "");
        
        var line = lineInfo.line;
        
        // Detect lines with reported errors
        for (var j = 0; j < errors.length; j++)
        {
            if (line.indexOf(errors[j].message) >= 0)
            {
                lineInfo.hasError = true;
                break;
            }
        }
        
        // Parse file name and line number prefix information
        // Typical Windows error line:
        //     d:\vbl\clr_next\src\ndp\clr\src\vm\class.h(4448) : error C2039: 'Iterator' : is not a member of 'MethodTableBuilder::bmtMethodSlotTable'
        //             d:\vbl\clr_next\src\ndp\clr\src\vm\class.h(4234) : see declaration of 'MethodTableBuilder::bmtMethodSlotTable'
        // Typical Mac error line:
        //     ../class.h:4448: error: Iterator in class MethodTableBuilder::bmtMethodSlotTable does not name a type
        if (line.match(/^\s*(\S+)[\(:](\d+)[\):]\s*:?\s*(.*)$/))
        {
            lineInfo.fileNameOriginal = RegExp.$1;
            lineInfo.fileLineNumber = RegExp.$2;
            lineInfo.fileMessage = RegExp.$3;
            lineInfo.hasMessage = true;
        }
        // Parse standalone file name on the line
        else if (line.match(/^\s*(\S+\.\S+)\s*$/))
        {
            lineInfo.fileNameOriginal = RegExp.$1;
            lineInfo.isStandaloneFile = true;
        }
        // Parse compiler command line
        else if (line.match(/^\s*(\S*)(cl|link|rc|csc|vbc|gcc|perl|resourcecompiler|g\+\+)\s+/))
        {
            lineInfo.compilerPath = RegExp.$1;
            lineInfo.compiler = RegExp.$2;
            lineInfo.isCompilerCommand = true;
        }
        // Detect empty lines
        else if (line.match(/^\s*$/))
        {
            lineInfo.isEmptyLine = true;
        }
    }
    // Basic line info parsed
    
    // First line could contain the directory we build
    if (lines[0].line.match(/^BUILDMSG: *Processing +(.*)$/i))
    {
        directoryBuild.directory = RegExp.$1;
        lines[0].isDirectoryProcessing = true;
    }
    
    // Parse advanced information:
    // For lines with file name:
    //     - fileNameOnly
    //     - fileNameInEnlistment
    //     - nPreviousLineWithFileNameIndex
    // For lines with errors:
    //     - contextFileNameOriginal
    //     - contextFileNameOnly
    //     - contextFileNameInEnlistment
    for (var i = 0; i < lines.length; i++)
    {
        // Update file name info
        var lineInfo = lines[i];
        if (lineInfo.fileNameOriginal != undefined)
        {
            lineInfo.fileNameOnly = lineInfo.fileNameOriginal.replace(/^.*[\\\/]/, "");
            
            var fileName = lineInfo.fileNameOriginal;
            // Try to find the file name in previous compiler commands
            for (var j = i - 1; j >= 0; j--)
            {
                var previousLineInfo = lines[j];
                // Skip all recognized lines, keep only compiler commands and unknown lines (possibly 
                // missed compiler command line)
                if (previousLineInfo.hasError || previousLineInfo.isStandaloneFile || previousLineInfo.hasMessage)
                    continue;
                if (previousLineInfo.line.indexOf(lineInfo.fileNameOnly) != -1)
                {   // Found the file name on previous line in log
                    lineInfo.nPreviousLineWithFileNameIndex = j;
                    re = new RegExp("\\s(\\S+" + lineInfo.fileNameOnly.replace(/\./, "\\.") + ")(\\s.*|)$");
                    if (previousLineInfo.line.match(re))
                    {
                        fileName = RegExp.$1;
                    }
                    break;
                }
                if (previousLineInfo.isCompilerCommand)
                {   // We don't need to search beyond the nearest compiler command
                    break;
                }
            }
            lineInfo.fileNameInEnlistment = _GetRelativePathInEnlistment(fileName, directoryBuild.directory);
        }
        
        // Update error context info (the last standalone file)
        if (lineInfo.hasError)
        {
            // Try to find the nearest previous line with standalone file
            for (var j = i - 1; j >= 0; j--)
            {
                var previousLineInfo = lines[j];
                if (previousLineInfo.isStandaloneFile)
                {
                    lineInfo.contextFileNameOriginal = previousLineInfo.fileNameOriginal;
                    lineInfo.contextFileNameOnly = previousLineInfo.fileNameOnly;
                    lineInfo.contextFileNameInEnlistment = previousLineInfo.fileNameInEnlistment;
                    break;
                }
                if (previousLineInfo.isCompilerCommand)
                {   // We don't need to search beyond the nearest compiler command
                    break;
                }
            }
        }
    }
    // Advanced line info is parsed
    
    logMsg(LogClrAutomation, LogInfo100, "}\n");
    
    return directoryBuild;
} // _CreateDirectoryBuild

//---------------------------------------------------------------------------------------
// 
// Mark mini-report lines (set isMiniReport = true).
// Set directoryBuild.cErrorsExcludedFromMiniReport.
// Mark first line in the last non-error block (set isMiniReportLastBlockStart = true).
// 
function _AddMiniReportInfo(directoryBuild)
{
    logMsg(LogClrAutomation, LogInfo100, "_AddMiniReportInfo() {\n");
    
    var lines = directoryBuild.lines;
    if (lines[0].isDirectoryProcessing)
    {
        lines[0].isMiniReport = true;
    }
    var cErrorsInMiniReportMax = 2;
    var cErrorsInMiniReport = 0;
    for (var i = 0; i < lines.length; i++)
    {
        var lineInfo = lines[i];
        if (!lineInfo.hasError)
            continue;
        if (lineInfo.isMiniReport)
            continue;
        
        lineInfo.isMiniReport = true;
        // Include previous line as context
        if (i > 0)
        {
            if (!lines[i - 1].isEmptyLine)
            {
                lines[i - 1].isMiniReport = true;
            }
        }
        
        // Include nearest previous line with standalone file and with compiler command
        var fStandaloneFileIncluded = false;
        for (var j = i - 1; j >= 0; j--)
        {
            var previousLineInfo = lines[j];
            if (!fStandaloneFileIncluded && previousLineInfo.isStandaloneFile)
            {
                fStandaloneFileIncluded = true;
                previousLineInfo.isMiniReport = true;
                // Include the line where the file name was found
                if (previousLineInfo.nPreviousLineWithFileNameIndex != undefined)
                {
                    lines[previousLineInfo.nPreviousLineWithFileNameIndex].isMiniReport = true;
                }
            }
            if (previousLineInfo.isCompilerCommand)
            {
                previousLineInfo.isMiniReport = true;
                break;
            }
        }
        
        // Include next lines with errors on the same line or with messages
        for (var j = i + 1; j < lines.length; j++)
        {
            var nextLineInfo = lines[j];
            if (nextLineInfo.hasError)
            {
                if ((nextLineInfo.fileNameOriginal != lineInfo.fileNameOriginal) || 
                    (nextLineInfo.fileLineNumber != lineInfo.fileLineNumber))
                {   // It is different error
                    break;
                }
            }
            else if (!nextLineInfo.hasMessage)
            {
                // Include next non-empty line as context
                if (!nextLineInfo.isEmptyLine)
                {
                    nextLineInfo.isMiniReport = true;
                }
                break;
            }
            nextLineInfo.isMiniReport = true;
        }
        
        // We added another error to the mini-report
        cErrorsInMiniReport++;
        if (cErrorsInMiniReport == cErrorsInMiniReportMax)
        {   // We want only first cErrorsInMiniReportMax errors
            break;
        }
    }
    
    // Add single excluded lines to the mini-report
    var isPreviousLineInMiniReport = true;
    for (var i = 0; i < lines.length; i++)
    {
        var lineInfo = lines[i];
        if (isPreviousLineInMiniReport && !lineInfo.isMiniReport)
        {
            if ((i == (lines.length - 1)) || 
                lines[i + 1].isMiniReport)
            {   // Next line is also in mini-report (or this is the last line)
                // Add the single line to mini-report
                lineInfo.isMiniReport = true;
            }
        }
        isPreviousLineInMiniReport = lineInfo.isMiniReport;
    }
    
    // Count how many errors are not in the mini-report
    var cErrorsExcludedFromMiniReport = 0;
    for (var i = 0; i < lines.length; i++)
    {
        var lineInfo = lines[i];
        if (lineInfo.hasError && !lineInfo.isMiniReport)
        {
            cErrorsExcludedFromMiniReport++;
        }
    }
    directoryBuild.cErrorsExcludedFromMiniReport = cErrorsExcludedFromMiniReport;
    
    // Mark the first line of the last (non-error) block
    var nLastLineInMiniReportIndex = 0;
    for (var i = 1; i < lines.length; i++)
    {
        if (lines[i].isMiniReport)
        {
            nLastLineInMiniReportIndex = i;
        }
    }
    
    logMsg(LogClrAutomation, LogInfo100, "}\n");
} // _AddMiniReportInfo

//---------------------------------------------------------------------------------------
// 
// razzle build logs are a bit of a pain because the error file does not
// contain context and the log file interleaves the output of multiple
// processors.  This function fixes this by sorting the output by
// processor and showing the errors in the context of the directory being built.
// It wraps this all up into a nice html/xml report file.
// 
function buildReport(buildErrFile, buildReportFileHtml, buildReportFileXml) {

    if (buildErrFile == undefined)
        buildErrFile = "buildc.err";
    if (buildReportFileHtml == undefined)
        buildReportFileHtml = buildErrFile.match(/(.*).err$/i)[1] + ".bldrpt.html";
    
    logMsg(LogClrAutomation, LogInfo10, "buildReport(", buildErrFile, ", ", buildReportFileHtml, ", ", buildReportFileXml, ") {\n");
    
    var buildLogFile = buildErrFile.match(/(.*)\.err$/i)[1] + ".log";
    var buildWarnFile = buildErrFile.match(/(.*)\.err$/i)[1] + ".wrn";
    var errorLines = FSOReadFromFile(buildErrFile).split(/\s*\n/);
    
    var errors = [];
    for (var i = 0; i < errorLines.length; i++)
    {
        // Remove processor prefix '4>' from the error
        var errorLine = errorLines[i].replace(/^\d+>/, "");
        
        // Remove file name, line, colon and 'error' text
        //  * It is useful to strip file name, because .err file can contain just relative file path, while 
        //    .log file contains full file name
        //  * The line number format in .log and .err files are different on Mac
        // Typical Windows error lines:
        //     3>d:\vbl\clr_next\src\ndp\clr\src\vm\class.h(32) : error C2061: syntax error : identifier 'the'
        //     3>NMAKE : fatal error U1077: 'D:\vbl\clr_next\src\tools\x86\vc\bin\cl.EXE' : return code '0x2'
        //     1>D:\vbl\clr_next_1\src\tools\Microsoft.DevDiv.Native.Settings.targets(1012,5): error MSB4036: The "SetEnvironment" task ...
        // Typical Mac error lines:
        //     2>vm/class.h(32) : error  error: invalid function declaration
        //     2>NMAKE : fatal error U1077: '/Developer/usr/bin/gcc' : return code '0x1'
        errorLine = errorLine.replace(/^\s*\S+\(\d+(|,\d+)\)\s*:\s*error\s*/, ""); // Common part for Win & Mac
        errorLine = errorLine.replace(/^\s*error\s*:\s*/, ""); // Mac double 'error' text
        
        // Skip 'empty' lines with non-printable characters (typically the last line in the .err file)
        if (errorLine.match(/^[\x00-\x20\s]*$/))
        {
            continue;
        }
        
        var error = new Object();
        error.lineText = errorLines[i];
        error.message = errorLine;
        errors.push(error);
        logMsg(LogClrAutomation, LogInfo1000, "Recognized error line #", (i + 1), " as error.\n");
        logMsg(LogClrAutomation, LogInfo100000, "Error line #", (i + 1), ": ", errorLine, "\n");
    }
    if (errors.length == 0)
        logMsg(LogClrAutomation, LogWarn, "buildReport: No error messages found in ", buildErrFile, "!\n");

    if (!FSOFileExists(buildLogFile))
    {
        logMsg(LogClrAutomation, LogError, "buildReport: Could not find build log file ", buildLogFile, "!\n");
        return -1;
    }
    
    // Info about 'current' directory build per processor (0 ... no processor)
    var procRawDirectoryBuilds = [];
    // List of failed directory builds
    var failedRawDirectoryBuilds = [];
    
    var lines = FSOReadFromFile(buildLogFile).split(/\s*\n/);
    for (var i = 0; i < lines.length; i++)
    {
        logMsg(LogClrAutomation, LogInfo1000000, "Processing line #", (i + 1), "\n");
        var line = lines[i];

        var procNum = 0;
        // Parse 1-based processor number:
        //     4>obj1COREc\i386\delayload.obj
        if (line.match(/^(\d+)>/))
            procNum = RegExp.$1;

        // Parse line:
        //     4>BUILDMSG: Processing d:\vbl\clr_next\src\ndp\clr\src\vm\wks1
        if (line.match(/^.?.?BUILDMSG: *Processing /i))
        {   // Start new directory build for this processor
            procRawDirectoryBuilds[procNum] = undefined;
        }

        var rawDirectoryBuild = procRawDirectoryBuilds[procNum];
        if (rawDirectoryBuild == undefined)
        {   // Initialize directory build
            rawDirectoryBuild = procRawDirectoryBuilds[procNum] = new Object();
            rawDirectoryBuild.hasRecognizedError = false;
            rawDirectoryBuild.logLines = [];
        }
        rawDirectoryBuild.logLines.push(line);

        if (line.match(/^.?.?Stop\.$/i))
        {   // Start new directory build for this processor
            procRawDirectoryBuilds[procNum] = undefined;
            continue;
        }
        if (procNum == 0)
        {   // Sometimes race conditions in build.exe make the log invalid:
            //   3>4>d:\vbl\clr_next\src\ndp\clr\src\vm\class.h(4648) : error C3861: 'InterfaceSlotIterator': identifier not found
            //   d:\vbl\clr_next\src\ndp\clr\src\vm\class.h(4648) : error C3861: 'InterfaceSlotIterator': identifier not found
            // We don't want them to cary all the lines without prefix, so we recognize no-processor 
            // end-lines:
            //   Elapsed time [0:00:06.438] ********************
            if (line.match(/^Elapsed time +\[.*\] +\*+$/))
            {   // Start new directory build for this processor
                procRawDirectoryBuilds[procNum] = undefined;
                continue;
            }
        }
        if (!rawDirectoryBuild.hasRecognizedError)
        {
            // Check if this line contains an error (from .err file)
            for (var j = 0; j < errors.length; j++)
            {
                if (line.indexOf(errors[j].message) >= 0)
                {
                    logMsg(LogClrAutomation, LogInfo100000, "Found error #", (j + 1), " on line #", (i + 1), "\n");
                    rawDirectoryBuild.hasRecognizedError = true;
                    rawDirectoryBuild.processor = procNum;
                    failedRawDirectoryBuilds.push(rawDirectoryBuild);
                    break;
                }
            }
        }
    }
    
    // Process failed directory builds further - parse each line for minimal error report
    var failedDirectoryBuilds = [];
    for (var i = 0; i < failedRawDirectoryBuilds.length; i++)
    {
        // Adds 2 additional fields - directory and lines[] (with subfields)
        failedDirectoryBuilds[i] = _CreateDirectoryBuild(
            failedRawDirectoryBuilds[i].logLines, 
            failedRawDirectoryBuilds[i].processor, 
            errors);
        _AddMiniReportInfo(failedDirectoryBuilds[i]);
    }
    
    if (buildReportFileXml != undefined)
    {
        try
        {
            var root = new Object();
            root.failedDirectoryBuilds = failedDirectoryBuilds;
            logMsg(LogClrAutomation, LogInfo, "buildReport: Writing XML file ", buildReportFileXml, "\n");
            var excludeFields = { logLine : true, logLines : true };
            xmlWrite(root, buildReportFileXml, "root", excludeFields);
        }
        catch (ex)
        {
            logMsg(LogClrAutomation, LogError, "Could not write XML build report ", buildReportFileXml, ":\n", ex.description, "\n");
        }
    }
    
    var buildReportDir = ".\\";
    if (buildReportFileHtml.match(/(.*?)\\[^\\]*?$/))
        buildReportDir = RegExp.$1;
    
    var report = "";
    report += "<HTML>\r\n";
    report += "<TITLE> Build Failure Report: " + uncPath(buildReportFileHtml)  + "</TITLE>\r\n";
    report += "<BODY>\r\n";
    report += "<H2> Build Failure Report: " + uncPath(buildReportFileHtml)  + "</H2>\r\n";

    report += "Razzle build logs are pretty complete, but they are inconvenient because the\r\n";
    report += "error messages in the build*.err file are not placed in context.  In addition\r\n";
    report += "while the output in the build*.log file is in context, it is hard to read\r\n";
    report += "when building with multiple processes, since the output of unrelated parts \r\n";
    report += "of the build are interleaved.    The build report fixes both of these by\r\n";
    report += "parsing the build*.err and build*.log file and creates a build*.rpt.html\r\n";
    report += "file that focuses in on the part of the build that failed, places the \r\n";
    report += "failure messages in context, and places all the messages from a particular\r\n";
    report += "process together.    \r\n";
    
    report += "<UL>\r\n"
    report += "<LI> Report generated on: " + prettyTime() + "\r\n";
    report += "<LI> Report generated by: <b> runjs buildReport " + buildErrFile + " " + buildReportFileHtml + "</B>\r\n"
    report += "<LI> Build .err file : <A HREF='" + relPath(buildErrFile, buildReportDir) + "'> " + uncPath(buildErrFile) + "</A>\r\n";
    if (FSOFileExists(buildWarnFile))
        report += "<LI> Build .wrn file : <A HREF='" + relPath(buildWarnFile, buildReportDir) + "'> " + uncPath(buildWarnFile) + "</A>\r\n";
    report += "<LI> Build .log file : <A HREF='" + relPath(buildLogFile, buildReportDir) + "'> " + uncPath(buildLogFile) + "</A>\r\n";
    report += "</UL>\r\n"

    var failureID = 0;
    if (failedDirectoryBuilds.length == 0)
    {
        report += "<H2> Warning we did not find error strings in the log file.  Please check the raw logs above<H2>\r\n";
    }
    else
    {
        report += "<H2>" + failedDirectoryBuilds.length + " directories failed to build</H2>\r\n";
        report += "<A NAME=failure" + (failureID++) + " HREF=#failure" + failureID +  ">" +
          "First Failure</A>\r\n";
        report += "<OL>\r\n"
        for (var i = 0; i < failedDirectoryBuilds.length; i++)
        {
            var failedDirectoryBuild = failedDirectoryBuilds[i];
            report += "<LI> Failure";
            if (failedDirectoryBuild.directory != undefined)
            {
                report += " in <b>" + failedDirectoryBuild.directory + "</b>\r\n";
            }
            report += "<DL><DD>\r\n";
            report += "<TABLE BORDER><TD>\r\n";
            report += "<UL>\r\n";
            for (var j = 0; j < failedDirectoryBuild.lines.length; j++)
            {
                var logLine = failedDirectoryBuild.lines[j].logLine;
                var outLine = escapeHTML(logLine);
                for (var k = 0; k < errors.length; k++)
                {
                    if (logLine.indexOf(errors[k].message) >= 0)
                    {
                        outLine = "<A NAME=failure" + (failureID++) + ">" +
                           "<b><font color=red> " + outLine + " </font></b></A>";
                        outLine += "<A HREF=#failure" + failureID + "> next </A>";
                        break;
                    }
                }
                report += "<LI> " + outLine + "\r\n";
            }
            report += "</UL>\r\n";
            report += "</TD></TABLE>\r\n";
            report += "</DL>\r\n";
        }
        report += "</OL>\r\n"
        report += "<A NAME=failure" + failureID + " HREF=#failure" + (failureID-1) +
            "> Previous Failure</A>\r\n";
    }
    report += "</BODY>\r\n";
    report += "</HTML>\r\n";

    logMsg(LogClrAutomation, LogInfo, "buildReport: Writing file ", buildReportFileHtml, "\n");
    FSOWriteToFile(report, buildReportFileHtml);
    
    logMsg(LogClrAutomation, LogInfo10, "}\n");
    return 0;
}

/*****************************************************************************/
/* Return a current timestamp.

*/
function currentTimestamp() {

    var time = new Date();
    var strYear = time.getYear().toString();
    var strMonth = (time.getMonth()+1).toString();
    var strDate = time.getDate().toString();
    var strHours = time.getHours().toString();
    var strMinutes = time.getMinutes().toString();

    if (strMonth.length < 2) {
        strMonth = "0" + strMonth;
    }

    if (strDate.length < 2) {
        strDate = "0" + strDate;
    }

    if (strHours.length < 2) {
        strHours = "0" + strHours;
    }

    if (strMinutes.length < 2) {
        strMinutes = "0" + strMinutes;
    }

    var timestamp =  strYear + strMonth + strDate + "-" + strHours + strMinutes;
    logMsg(LogClrAutomation, LogInfo, "currentTimestamp: " + timestamp + "\n" );
    return timestamp;
}


/*****************************************************************************/
/* This will make *.h and *.c files from *.idl files for rotor builds.

   Parameters:
     projectType : rotor project name ('coreclr', 'coriolis' or '*')
     srcBase     : The base of the VBL (so we can find razzle)
     sdChange    : Where to pickup the lkgvc toolsets.

*/
function rotorMakeIdl(projectType,srcBase,sdChange) {

    if(projectType == undefined) {
        projectType="*"
    }

    if (srcBase == undefined) {
        srcBase = srcBaseFromScript();
    }

    var inTFS = _inTFS();
    if (sdChange == undefined) {
        sdChange = "default";
    }
    if (inTFS && sdChange != "default") {
        throw Error(1, "sdChange " + sdChange + " not supported in TFS branches.");
    }

    if(projectType == "*") {
        rotorMakeIdl("coreclr",srcBase,sdChange);
        rotorMakeIdl("coriolis",srcBase,sdChange);
        return;
    }

    logCall(LogClrAutomation, LogInfo, "rotorMakeIdl", arguments);

    if (inTFS)
        runCmdToLog("tf edit " + srcBase + "\\rotor\\prebuilt\\idl\\" + projectType + "\\*");
    else
        sdEdit(srcBase + "\\rotor\\prebuilt\\idl\\"+projectType+"\\*",sdChange);

    var run = runCmdToLog("cmd /k call " + srcBase + "\\rotor\\renv lkgvc",  // Build with lkgvc toolset.
                        runSetCwd(srcBase + "\\rotor",
                        runSetInput("pushd " + srcBase + "\\rotor\\prebuilt\\idl\\"+projectType+"\n" +
                        "makeidl.bat * \n" +
                        "copy obj%BUILD_ALT_DIR%\\%_BUILDARCH%\\*.h\n" +
                        "copy obj%BUILD_ALT_DIR%\\%_BUILDARCH%\\*.c\n" +
                        "exit %ERRORLEVEL%\n",
                        runSetEnv("_NTROOT", undefined
                        ))))
    if (inTFS)
        runCmdToLog("tfpt uu " + srcBase + "\\rotor\\prebuilt\\idl\\" + projectType + "\\*");
    else
        sdRevert("-a "+ srcBase + "\\rotor\\prebuilt\\idl\\"+projectType+"\\*");

}

/****************************************************************************
/* Build something using prefast.  This build is designed to find common
   programmer mistakes at compile time.  The results of this build is not
   binaries but a 'defects.xml' file that indicate the issues it found
   It also produces a 'prefastParse.log' file that compares the defects
   found against a baseline of what is currently checked in any only reports
   the new failures.

     Parameters:
       bldDir       : The directory to build (relative to srcBase)
       srcBase      : The base of the VBL (so we can find razzle)
       logDir       : Where to put the build*.* and defects.xml file
       rArgs        : Additional args to razzle (eg Offline)
       baseLineXml  : The baseline file (known prefast errors) (XML format)
       plugins      : semi-colon separated list of additional plugins to run
*/
function prefastBuild(bldType, bldDir, srcBase, logDir, rArgs, baseLineXml, plugins) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase for the prefastBuild method is missing");
    }
    if (bldType == undefined)
        bldType = "fre";
    if (bldDir == undefined)
        bldDir = "ndp\\clr\\src";
    if (logDir == undefined)
        logDir = srcBase + "\\" + bldDir;
    if (rArgs == undefined)
        rArgs = "";

    logCall(LogClrAutomation, LogInfo10, "prefastBuild", arguments);

    FSOCreatePath(logDir);
    logDir = FSOGetFolder(logDir).Path;
    srcBase = FSOGetFolder(srcBase).Path;

    var prefastBase  = srcBase + "\\tools\\x86\\prefast";
    var prefastBin   = logDir  + "\\prefastBinaries";

    var stdPluginDir = srcBase + "\\tools\\x86\\oacr\\prefast8\\bin\\plugins";

    // User written plugins can piggy on thie prefast run (which is much cheaper than using their own
    // dedicated run).
    var extraPrefastOptions = "";
    if (plugins != undefined)
    {
        extraPrefastOptions += "/plugins=";
        plugins = plugins.split(";");
        for (var i in plugins)
        {
            var pluginLoc = srcBase + "\\ndp\\clr\\bin\\x86\\prefastplugins\\" + plugins[i] + ".dll";
            if (!FSOFileExists(pluginLoc))
            {
                // check standard plugin location
                pluginLoc = stdPluginDir + "\\" + plugins[i] + ".dll";
                if (!FSOFileExists(pluginLoc))
                {
                    throw Error(1, "Plugin " + plugins[i] + " not found at " + pluginLoc);
                }
            }
            extraPrefastOptions += pluginLoc + ";";
        }
    }

    if (isCoreCLRBuild(bldType)) {
        rArgs += " CoreCLR";
    }

    runCmdToLog("pushd " + srcBase + "\\" + bldDir + " & call " +
        srcBase + "\\tools\\razzle.cmd x86 fre binaries_dir " + prefastBin +
        " " + rArgs + " Exec call " + srcBase + "\\tools\\x86\\prefast\\clr\\clrPrefastBuild.bat -cbCfL -jpath " + logDir,
        runSetDump("runjs dumpProc %p " + logDir + "\\PrefastBuild.dmpRpt",
        runSetEnv("PCOPY_NO_TIMESTAMP_CHANGE", 1,  // This says to pcopy, don't udpate public files if you dont have to.
        runSetEnv("PREFASTDIR", prefastBase,
        runSetEnv("EXTRA_PREFAST_OPTIONS", extraPrefastOptions,
        runSet32Bit(
        runSetTimeout(4 * HOUR, clrRunTemplate)))))));

    // Anything binplaced is not interesting so we get rid of it
    if (FSOFolderExists(prefastBin))
        FSODeleteFolder(prefastBin, true);

    var defectsFileIn  = prefastBase + "\\defects.xml";
    var defectsFileOut = logDir      + "\\defects.xml";

    if (!FSOFileExists(defectsFileIn))
        throw Error(1, "Could not find defects file " + defectsFileIn + "\n");

    // prefastfixXML.exe is a managed exe that just reformat the Xml file produced by prefast to be properly indented
    // source code is found at ndp\clr\src\ToolBox\PrefastFixXML\prefastFixXml.cs
    //
    var run = runCmdToLog(srcBase + "\\tools\\devdiv\\x86\\prefastfixXML.exe " + defectsFileIn + " " + defectsFileOut,
        runSetTimeout(1 * MINUTE, 
        runSetEnv("COMPLUS_InstallRoot", srcBase + "\\tools\\x86\\managed",    // Use the runtime the tools use
        runSetEnv("COMPLUS_Version", "v4.0",
        runSetNoThrow(
        clrRunTemplate)))));
    if (run.exitCode != 0) {
        logMsg(LogClrAutomation, LogError, "Prefast Defects parsing failed: Note that this can be due to not finding a CLR runtime (see the DEFAULT_VERSION variable set above)\n")
        throw Error(1, "Prefast Defects parsing failed: Note that this can be due to not finding a CLR runtime (see the DEFAULT_VERSION variable set above)");
    }
    logMsg(LogClrAutomation, LogInfo, "prefastBuild defects file = ", logDir, "\\defects.xml\n");

    var coreclrbuild = false;
    if (isCoreCLRBuild(bldType)) {
        coreclrbuild = true;
    }
    if (baseLineXml == undefined)
    {
        if (coreclrbuild) {
            baseLineXml = prefastBase + "\\clr\\coreclr_prefast_baseline.xml";
        } else {
        baseLineXml = prefastBase + "\\clr\\baseline.xml";
        }
    }
    var allowedXml = prefastBase + "\\clr\\allowed.xml";
    var reportFile;
    if (coreclrbuild) {
        reportFile = logDir + "\\coreclr_prefastParse.log";
    } else {
        reportFile = logDir + "\\clr_prefastParse.log";
    }

    // now generate the report against the baseline
    logMsg(LogClrAutomation, LogInfo, "prefastBuild: generating diff report: ", reportFile, "\n");

    // prefastParse.exe is a managed exe that compares the defectsFile with the baselineXml and returns an exit status
    // source code is found at ndp\clr\src\ToolBox\PrefastParse\prefastParse.cs
    //
    run = runCmdToLog(srcBase + "\\tools\\devdiv\\x86\\prefastParse.exe -v " + allowedXml + " " + baseLineXml + " " + defectsFileOut,
        runSetEnv("COMPLUS_InstallRoot", srcBase + "\\tools\\x86\\managed",    // Use the runtime the tools use
        runSetEnv("COMPLUS_Version", "v4.0",
        runSetNoThrow(
        runSetOutput(reportFile, 
        clrRunTemplate)))));

    // append investigation instructions to report file
    var investigationInstructions = FSOReadFromFile(prefastBase + "\\clr\\investigatingPrefastViolations.txt");
    FSOWriteToFile(investigationInstructions, reportFile, true);


    if (run.exitCode != 0) {
        logMsg(LogClrAutomation, LogError, "prefastBuild: Additional prefast warnings detected see ", reportFile, "\n");
        throw Error(1, "Additional prefast errors encountered, see " + reportFile + " for details\n")
    }

    return 0;
}

/*****************************************************************************/
/* Run a scanning tool that checks runtime specific invariants.
   This specifically scans clr.dll. See 'scanRuntimeWorker' for  scanning other dlls.
   The runtime has a number of invariants that are easy to violate.  These
   invariants have been formalized into contracts, and the scanner statically
   scans the runtime code looking for places were a contract of a callee was
   violated by a caller.   Current this only works on a x86chk build.
   and is now automatically done as part of a razzleBuild for x86chk.

     Parameters:
       binDir  : The razzle binaries directory of the runtime to check
       logFile : The name of the report file
       srcBase : The base of the VBL (so we can find the scan tool)
       options : Additional options to the scan tool
*/
function scanRuntime(binDir, logFile, srcBase, options) 
{

    if (logFile == undefined)
        logFile = "scan.report.log";
    if (binDir == undefined && Env("_NTTREE") != undefined)
        binDir = Env("_NTTREE");
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase for the scanRuntime method is missing");
    }

    var targetDll;
    if (FSOFileExists(binDir + "\\coreclr.dll"))
    {
        targetDll = "coreclr.dll";
    }
    else
    {
        targetDll = "clr.dll";
    }
    return scanRuntimeWorker(targetDll, binDir, logFile, srcBase, options);
}


/*****************************************************************************/
/* Build the VM and run SCAN on it
*/
function buildAndScan(options)
{
    if (options == null)
        options = "/noDac";

    var result = buildVM(options);
    if (result == 0)
        return scanRuntime();
    else
        return result;
}


/*****************************************************************************/
/* Create contract suggestions from SCAN
     Parameters:
       binDir  : The razzle binaries directory of the runtime to check
       logFile : The name of the report file
       srcBase : The base of the VBL (so we can find the scan tool)
       options : Additional options to the scan tool
*/
function scanSuggestions(options) 
{
    if (options == undefined)
        options = "";

    var srcBase = Env("_NTBINDIR");
    if (!srcBase)
        throw new Error(1, "srcBase not found.  Run from a ClrEnv window.");

    srcBase = FSOGetFolder(srcBase).Path;
    var scanDir = srcBase + "\\ndp\\clr\\snap2.4\\tasks\\scan";

    options += " -e " + scanDir + "\\KnownContractViolations.txt";
    options += " -s -m " + scanDir + "\\SuggestionExemptions.txt";

    return scanRuntime(undefined, undefined, undefined, options);
}

/*****************************************************************************/
/* Run a scanning tool. Current this only works on a x86chk build.

     Parameters:
       dll     :  the short name of the dll to scan.
       binDir  : (optional) The razzle binaries directory of the runtime to check
       logFile : (optional) The name of the report file
       srcBase : (optional) The base of the VBL (so we can find the scan tool)
       options: (optional) additional options to scan tool

    eg: 
        scanRuntimeWorker mscordbi.dll
*/
function scanRuntimeWorker(dll, binDir, logFile, srcBase, options)
{
debugger; 
    if (logFile == undefined)
        logFile = "scan.report.log";
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase for the scanRuntime method is missing");
    }

    FSOCreatePathForFile(logFile);
    srcBase = FSOGetFolder(srcBase).Path;

    var scanDir = srcBase + "\\ndp\\clr\\snap2.4\\tasks\\scan";
    if (options == undefined)
    {
        options = "-e " + scanDir + "\\KnownContractViolations.txt";

        // Check the suggestions on the full desktop, not coreclr
        if (dll == "clr.dll")
        {
            options += " -s -m " + scanDir + "\\SuggestionExemptions.txt";
        }
    }
    if (binDir == undefined)
        binDir = srcBase + "\\binaries\\x86chk";

    logCall(LogClrAutomation, LogInfo10, "scanRuntime", arguments);


    var dllFullPath = binDir + "\\" + dll;
    if (!FSOFileExists(dllFullPath)) {
        dllFullPath = binDir + "\\clr.dll";
    }
    var pdbPath = binDir + "\\symbols.Pri\\retail";
    
    runCmdToLog(scanDir + "\\scan.exe " + options + " " + dllFullPath,
        runSetTimeout(10 * MINUTE,
        runSetEnv("PATH", srcBase + "\\tools\\x86\\vc\\bin" + ";" + runGetEnv("PATH"),
        runSetEnv("_NT_SYMBOL_PATH", pdbPath,
        logFile ? runSetOutput(logFile) : undefined))));

    logMsg(LogClrAutomation, LogInfo10, "Report in ", logFile, "\n");
    return 0;
}


/******************************************************************************/
/****************************** RUNNING TESTS *********************************/
/******************************************************************************/

/****************************************************************************/
/* Installs a version of the runtime using clrSetup

  Parameters
    binDir    : The razzle binaries directory of the runtime to install
    setupArgs : Additional arguments to pass to ClrSetup
    logDir    : Where to put the ndpsetup.log file (if undefined, leaves it)
    verstr    : Optional private runtime /verstr value for ClrSetup.bat
                Only set this for a private runtime, "/nrad" will be added to ClrSetup.
                Leave this undefined for a standard build (will register as the default runtime)
    runOpts   : Additional options to pass to 'runCmd (env vars etc)' (default none)
*/
function clrSetup(binDir, setupArgs, logDir, verstr, runOpts) {

    if (!isElevated())
    {
        throw new Error(1, "clrSetup: Requires executing with elevated privilieges");
    }

    if (binDir == undefined)
        throw new Error(1, "clrSetup: Required argument binDir for the clrSetup command not present");
    if (setupArgs == undefined)
        setupArgs = "";
    if (runOpts == undefined)
        runOpts = clrRunTemplate;

    logCall(LogClrAutomation, LogInfo, "clrSetup", arguments, " {");

    var setupInfo = parseBatchFileForSetStatements(binDir + "\\ClrSetupInfo.bat");
    var bldType = setupInfo["CLR_BUILD_TYPE"];
    var bldArch = setupInfo["CLR_BUILD_ARCH"].toLowerCase();

        // Always have a verstr for ClrSetup.bat, because otherwise ClrSetup.bat does
        // not override COMPLUS_DefaultVersion if it does not match and we want that
        // behavior.
    if (verstr == undefined) {
        verstr = bldArch + bldType;
    }
    else {
        // Caller supplied a verstr, so don't register this runtime as the default runtime
        setupArgs = setupArgs + " /nrad";
        // Setting COMPLUS_DefaultVersion also implicit implies /nrad in ClrSetup.bat
        // Having it set when verstr is used suppresses the warnings messages in ndpSetup
        // Snap requires that the runtime be register as the default runtime, so don't
        // unconditionally set COMPLUS_DefaultVersion.
        var versionDir = "v4.0." + verstr;
        runOpts = runSetEnv("COMPLUS_DefaultVersion", versionDir, runOpts);
    }
    setupArgs = setupArgs + " /verstr " + verstr;

    var timeout = 120 * MINUTE;

    if (setupArgs.match(/\/refreshnativeimages/i))
        timeout += 20 * MINUTE;
    
    // TODO: when we fix CLRSetup to just do the right thing, then we can just
    // use runCmdToLog() instead of runWithRuntimeArch()
    //
    // ClrSetup does most of the installation
    var run = runWithRuntimeArch(binDir + "\\clrsetup.bat " + setupArgs,
        bldArch,
        runSetTimeout(timeout,
        runSetDump("runjs dumpProc %p " + logDir + "\\CLRSetup.dmpRpt",
        runSetNoThrow(
        runSetLog(LogRun, LogInfo, 
        runOpts)))));

    // CLRSetup tends to stop the WMI service, and so we can't cache any object across it
    procLocalWMI = null;

    // return the path to the installation directory
    if (!run.output.match(/Installing NDP to:\s*(\S.*[^\s\\])/))
        throw new Error(-1, "clrSetup: Could not parse clrsetup output to get the installation directory\n");
    var installDir = RegExp.$1;

    if (logDir != undefined) {
        FSOCreatePath(logDir);
        FSOCopyFile(installDir + "\\ndpsetup.log", logDir + "\\ndpsetup.log", true);
            // copy all the install logs too.
    }

    if (run.exitCode != 0)
        throw Error(1, "clrSetup: CLR Setup fails with exit code " + run.exitCode);

    logMsg(LogClrAutomation, LogInfo, "} clrSetup() = ", installDir, "\n");
    return installDir;
}

/****************************************************************************/
/* Installs N versions of the runtime as version_prefix0, version_prefix1, etc.
   This does an abbreviated runtime install, and is used for multi-runtime
   smarty.

  Parameters
    numInst   : Number of times to install the runtime.
    binDir    : The razzle binaries directory of the runtime to install
    verpfx    : Required runtime prefix.  runtimes will be installed as
                vX.0.verpfx#, where # is the nth install of the runtime.
    ngenRepo  : Optional location for the NGen Repository.
    logDir    : Where to put the ndpsetup.log file (if undefined, leaves it)
    runOpts   : Additional options to pass to 'runCmd (env vars etc)'
                (default none)
*/
function clrSetupN(numInst, binDir, verprefix, ngenRepo, logDir, runOpts)
{
    if (!isElevated())
    {
        throw new Error(1, "clrSetup: Requires executing with elevated privilieges");
    }

    if (numInst == undefined)
        throw new Error(1, "clrSetupN: Required argument numInst for the clrSetup command not present");
    if (binDir == undefined)
        throw new Error(1, "clrSetupN: Required argument binDir for the clrSetup command not present");
    if (runOpts == undefined)
        runOpts = clrRunTemplate;
    if (ngenRepo == undefined)
        ngenRepo = srcBaseFromScript() + "\\binaries\\ngenRepository";

    logCall(LogClrAutomation, LogInfo, "clrSetup", arguments, " {");

    var setupInfo = parseBatchFileForSetStatements(binDir + "\\ClrSetupInfo.bat");
    var bldType = setupInfo["CLR_BUILD_TYPE"];
    var bldArch = setupInfo["CLR_BUILD_ARCH"].toLowerCase();
    if (verprefix == undefined) {
        verprefix = bldArch + bldType + "_worker";
    }

    //NGen Multiworker is good.
    runOpts = runSetEnv("COMPlus_MultiWorker", "1", runOpts);

    for(var i = 0; i < numInst; i++)  {
        //var setupArgs = "/batch /lg /nrad /nongeninstallqueue /mz /nisp " + ngenRepo;
        //Try to match clrsetup better so we don't hose the cache.
        // Layout-based setup doesn't support these fancy NGEN options, so revert to batch setup
        var setupArgs = "/batch /lg /nrad /nongeninstallqueue /mz /nisp " + ngenRepo;

        var createRepository = (i == 0); //If 1 we "create" the respository, otherwise we use it.

        //The first time through I have to actually create the ngen repository.  Do that here.
        if(createRepository)
        {
            setupArgs += " /refreshnativeimages";
        }

        //otherwise we just use the repository.

        var verStr = verprefix + i;
        var installDir = clrSetupWithCache(binDir, setupArgs, logDir, undefined/*cache dir*/, verStr, runOpts);

        //Now that I've installed it, I've only generated an ngen image for mscorlib (I don't want
        //everything that clrsetup ngens).  ngen the rest of the relevant pri 1 assemblies now.
        
        var ngenCmd = installDir + "\\ngen.exe";

        var defaultVersion = installDir.match(/^(.*)\\(.*)/)[2];
        var ngenRunOpts = runSetEnv("COMPlus_DefaultVersion", defaultVersion, runOpts);

        //Let's add version out of paranoia.
        ngenRunOpts = runSetEnv("COMPlus_Version", defaultVersion, ngenRunOpts);
        
        //and let's add a timeout too.
        ngenRunOpts = runSetTimeout(20 * MINUTE, ngenRunOpts);

        logMsg(LogClrAutomation, LogInfo, "Ngen-ing important modules for " + defaultVersion + "\n");

        var assemblies = ["System",
                          "System.Drawing",
                          "System.Windows.Forms",
                          "System.Xml"];
        for(var j = 0; j < assemblies.length; ++j)
        {
            var cmdLine = ngenCmd + " install /nologo ";
            if(createRepository)
            {
                cmdLine += " /CopyToRepository:" + ngenRepo;
            }
            else
            {
                cmdLine += " /CopyFromRepository:" + ngenRepo;
            }
            cmdLine += " " + assemblies[j];

            runCmdToLog(cmdLine, ngenRunOpts);
        }
                          
    }

}


/****************************************************************************/
/* run fxpSetup from 'binDir' to 'targetDir'
*/
function fxpSetup(binDir, targetDir) {
    if (binDir == undefined)
        throw new Error(1, "fxpSetup: Required argument binDir for the clrSetup command not present");
        
    if (targetDir == undefined)
        targetDir = "";

    logMsg(LogClrAutomation, LogInfo, "fxpSetup(", binDir, ") {\n");

    var run = runCmdToLog(binDir + "\\fxpsetup.cmd " + targetDir, undefined);
    
    if (run.exitCode != 0)
        throw Error(1, "fxpSetup: setup failed with exit code " + run.exitCode);

    if (!run.output.match(/TARGETDIR *= (.*)\r\n/))
        throw new Error(-1, "Could not parse fxpsetup output to find installation directory\n");
    var installDir = RegExp.$1;
    
    logMsg(LogClrAutomation, LogInfo, "} fxpSetup() = ", installDir, "\n");
    return installDir;
}

/****************************************************************************/
/* create an array of Folder objects sorted by modification time
*/
function _SubFoldersSortedByLastModified(folderPath) {
    var folder = FSOGetFolder(folderPath);
        // put the subfolders in an array an sort it by last modification time
    var subFolders = [];
    for (var subFoldersEnum = new Enumerator(folder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
        subFolders.push(subFoldersEnum.item());
    }
    subFolders.sort(function(x, y) { return (y.DateLastModified - x.DateLastModified); });
    return subFolders;
}

/****************************************************************************/
/* enforce a policy of at most 2 versions in the clrSetupCache at the same time
   parameters cacheDir and clrVersion will be combined to create a path like:
    %_NTBINDIR%\binaries\clrSetupCache\40131.00
*/
function _cacheClrSetupCleanup(cacheDir, clrVersion) {
    logCall(LogClrAutomation, LogInfo, "_cacheClrSetupCleanup", arguments);

    // don't let bugs here excape this function
    try {
        var subFolders = _SubFoldersSortedByLastModified(cacheDir);
        if (subFolders.length == 0)
            return;

        var foldersToKeep = 1;
        while (subFolders.length > foldersToKeep) {
            var subFolder = subFolders.pop();
            if (subFolder.Name == clrVersion) {
                logMsg(LogClrAutomation, LogInfo, "_cacheClrSetupCleanup: found same version ", subFolder.Path, "\n");
            }
            else {
                logMsg(LogClrAutomation, LogInfo, "_cacheClrSetupCleanup: deleting ", subFolder.Path, "\n");
                try {
                    FSODeleteFolder(subFolder.Path, true);
                }
                catch (e) {
                    logMsg(LogClrAutomation, LogInfo, "_cacheClrSetupCleanup: delete failed for ", subFolder.Path, "\n");
                }
            }
        }
        while (subFolders.length > 0) {
            var subFolder = subFolders.pop();
            logMsg(LogClrAutomation, LogInfo, "_cacheClrSetupCleanup: keeping ", subFolder.Path, "\n");
        }
    } 
    catch(e) {
        logMsg(LogClrAutomation, LogInfo, "_cacheClrSetupCleanup: exception ", e.description, "\n");
    }
}

/****************************************************************************/
/* parse 'ndpsetupLog' and create a cache of the needed files in 'cacheDir'.
   under the sub-directory 'clrVersion'.  Returns the string needed as the
   /dropPath qualifier to clrsetup to use this cache.  Note that this routine
   assumes it owns 'cacheDir' and will destroy old contents of this directory

   parameters cacheDir, clrVersion, and clrBuildArch will be combined to create a path like:
    %_NTBINDIR%\binaries\clrSetupCache\60609.00

*/
function _cacheClrSetup(ndpSetupLog, cacheDir, clrVersion, clrBuildArch) {

    if (ndpSetupLog == undefined)
        throw Error(1, "required arg ndpSetupLog not supplied");
    if (cacheDir == undefined)
        throw Error(1, "required arg cacheDir not supplied");
    if (clrVersion == undefined)
        throw Error(1, "required arg clrVersion not supplied");

    logCall(LogClrAutomation, LogInfo, "_cacheClrSetup", arguments);

    var newVersionDir = cacheDir + "\\new";
    if (FSOFolderExists(newVersionDir))
        FSODeleteFolder(newVersionDir, true);
    FSOCreatePath(newVersionDir);

    var ndpSetupFile = FSOOpenTextFile(ndpSetupLog, 1);
    var base;
    var pat;
    var numFiles = 0;
    var dropPath;
    while (!ndpSetupFile.AtEndOfStream) {
        var line = ndpSetupFile.ReadLine();
        if (!pat) {
            if (line.match(/^\s*(\S+) bits from:\s*((.*?)\\([^\\]*)\\binaries[^\\]*)\\*$/i)) {
                if (RegExp.$1 != "CLR") {
                    if (base == undefined) {
                        base = RegExp.$3;
                        dropPath = RegExp.$2;
                        var patStr = "(" + base.replace(/\\/g, "\\\\") + "(\\S+))";
                        logMsg(LogClrAutomation, LogInfo10, "using patStr ", patStr, "\n");
                        pat = RegExp(patStr, "i");
                        logMsg(LogClrAutomation, LogInfo, "Got base = ", base, "\n");
                    }
                    else {
                        if (RegExp.$2 != base)
                            throw new Error(-1, "Error: can't do cache if we get our bits from multiple sources " + RegExp.$2 + " and " + base);
                    }
                }
            }
        }
        else if (line.match(pat)) {
            var fromFile = RegExp.$1;
            var toFile = newVersionDir + RegExp.$2;

            if (FSOFileExists(fromFile)) {
                if (!FSOFileExists(toFile)) {
                    FSOCreatePathForFile(toFile);
                    FSOCopyFile(fromFile, toFile, true);
                    logMsg(LogClrAutomation, LogInfo, "copying ", fromFile, " -> ", toFile, "\n");
                    // logMsg(LogClrAutomation, LogInfo, ".");
                    numFiles++;
                }
            }
            else {
                if (fromFile.match(/[*?]/)) {
                    logMsg(LogClrAutomation, LogInfo, "Found path with wildcard ", fromFile, "\n");
                    var toDirectory = WshFSO.GetParentFolderName(toFile);
                    FSOCreatePath(toDirectory);
                    logMsg(LogClrAutomation, LogInfo, "copying ", fromFile, " -> ", toDirectory, "\n");
                    FSOCopyFile(fromFile, toDirectory, true);
                }
                else if (!FSOFolderExists(fromFile))
                    logMsg(LogClrAutomation, LogWarn, "Found path ", fromFile, " that does not exist!\n");
            }
        }
    }
    logMsg(LogClrAutomation, LogInfo, "\n");
    if (!pat || numFiles == 0) {
        var msg = "Could not find 'bits from' line in NDPSetup log file (bad ndpsetup log?)";
        logMsg(LogClrAutomation, LogError, msg, "\n")
        throw new Error(1, msg);
    }

    // cleanup binaries\clrSetupCache
    _cacheClrSetupCleanup(cacheDir, clrVersion);
    
    // Move copied files into their final resting place 
    //    new\sources
    //    new\binaries.<arch>ret 

    var targetDir = cacheDir + "\\" + clrVersion;
    FSOCreatePath(targetDir);
    try {
        logMsg(LogClrAutomation, LogInfo, "moving ", newVersionDir, " to ", targetDir, "\n");
        
            // Copy so that we get merge semantic  We are no longer atomic, but without a binaries directory we should be OK. 
        FSOCreatePath(targetDir + "\\sources");
        FSOMakeWriteable(targetDir + "\\sources");  // workaround for Windows OS Bug#1947841
        FSOCopyFolder(newVersionDir + "\\" + clrVersion + "\\sources", targetDir + "\\sources", true);

            // Atomically update the binaries directory
        FSOMoveFolder(newVersionDir + "\\" + clrVersion + "\\binaries." + clrBuildArch + "ret", targetDir + "\\binaries." + clrBuildArch + "ret", true);
        FSODeleteFolder(newVersionDir, true);
    } catch(e) {
        logMsg(LogClrAutomation, LogInfo, "_cacheClrSetup could not move ", newVersionDir, " to ", targetDir, "\n");
    }

    logMsg(LogClrAutomation, LogInfo, "_cacheClrSetup copied ", numFiles, " files returning ", dropPath, "\n");
    return dropPath;
}

/****************************************************************************/
/* run ClrSetup from 'binDir'.  Look to see what CLR_CERTIFIED_VERSION is being
   used, and if is cached in 'cacheDir' then use that.  Otherwise run ClrSetup
   and populate the cache so that next time it will use the cache.  The net
   effect here is that ClrSetup only goes to the network when the
   CLR_CERTIFIED_VERION changes.  Otherwise, clrSetupWithCache is a drop in
   replacement for clrSetup.

     Parameters:
       binDir    : The razzle binaries directory of the runtime to install
       setupArgs : Additional arguments to pass to Crlsetup
       logDir    : Where to put the ndpsetup.log file (if undefined, leaves it)
       cacheDir  : Where to put the image of the non-clr bits
       verstr    : Optional private runtime /verstr value for ClrSetup.bat
                   Only set this for a private runtime, "/nrad" will be added to ClrSetup
                   leave this undefined for a standard build (will register as the default runtime)
       runOpts   : Additional options to pass to 'runCmd (env vars etc)' (default none)
*/
function clrSetupWithCache(binDir, setupArgs, logDir, cacheDir, verstr, runOpts) {

    if (binDir == undefined)
        throw new Error(1, "clrSetupWithCache: Required argument binDir for the clrSetup command not present");
    if (setupArgs == undefined)
        setupArgs = "";
    if (cacheDir == undefined) 
        cacheDir = srcBaseFromScript() + "\\binaries\\clrSetupCache";

    if (!isElevated())
    {
        throw new Error(1, "clrSetupWithCache: Requires executing with elevated privilieges");
    }

    logCall(LogClrAutomation, LogInfo, "clrSetupWithCache", arguments, "{");

        // TODO we need something robust when snap clients are rebooting.
        // on network connections, it can take multiple tries, go ahead and try pretty hard 
    var clrSetupInfoFilename = binDir + "\\ClrSetupInfo.bat";
    for (var i = 0; !FSOFileExists(clrSetupInfoFilename); ++i) {
        logMsg(LogClrAutomation, LogInfo, clrSetupInfoFilename, " not found, retry ", i.toString(), ".  Sleeping 20 seconds before next retry.\n");
        WScript.Sleep(1000); 
        if (i > 60)
            throw new Error(1, "Could not find ", binDir + "\\ClrSetupInfo.bat");
    }

    // The fastcheck option indicate that we might be installing a runtime that is already installed, 
    // so do a quick check of clr timestamps to confirm and exit quickly in that case. 

    // Grab the details out of the ClrSetupInfo.bat file
    var setupInfo = parseBatchFileForSetStatements(clrSetupInfoFilename);
    var clrVersion = setupInfo["CLR_CERTIFIED_VERSION"];
    var clrBuildType = setupInfo["CLR_BUILD_TYPE"].toLowerCase();
    var clrBuildArch = setupInfo["CLR_BUILD_ARCH"].toLowerCase();

    var successFile = undefined;
    var runtimeRoot = undefined;
    if (setupArgs.match(/^(.*)\/fastCheck\b(.*)$/i)) {
        setupArgs = RegExp.$1 + RegExp.$2;
        
            // figure out where we think the runtime will go
        var ndpInstallRoot = Env("SystemRoot") + "\\Microsoft.NET\\Framework";
        if (clrBuildArch == "ia64" || clrBuildArch == "amd64")
            ndpInstallRoot += "64";
        runtimeRoot = ndpInstallRoot + "\\v4.0." + verstr;

        var clrFrom = binDir + "\\clr.dll";
        var clrTo = runtimeRoot + "\\clr.dll";
        logMsg(LogClrAutomation, LogInfo, "source clr = ", clrFrom , "\n");
        logMsg(LogClrAutomation, LogInfo, "target clr = ", clrTo , "\n");
        successFile = runtimeRoot + "\\success.txt";
        if (FSOFileExists(successFile)) { 
            if (FSOFileExists(clrFrom) && FSOFileExists(clrTo)) {
                var fromFile = FSOGetFile(clrFrom);
                var toFile = FSOGetFile(clrTo);
                if (fromFile.Size == toFile.Size  && (fromFile.DateLastModified - toFile.DateLastModified) == 0) { // {
                    logMsg(LogClrAutomation, LogInfo, "} clrsetupWithCache /fastCheck specified and clr match, returning\n");
                    return runtimeRoot;
                }
                else 
                    logMsg(LogClrAutomation, LogInfo, "clr timestamp mismatch, reinstalling\n");
            }
            else 
                logMsg(LogClrAutomation, LogInfo, "target (or source), clr, does not exist\n");
        }
        else 
            logMsg(LogClrAutomation, LogInfo, "file ", successFile, " does not exist\n");
    }

    logMsg(LogClrAutomation, LogInfo, "ClrSetup needs version ", clrVersion, " arch ", clrBuildArch, "\n");


    var dropPath   =  cacheDir + "\\" + clrVersion + "\\binaries." + clrBuildArch + "ret";
    if (FSOFolderExists(dropPath)) {
        logMsg(LogClrAutomation, LogInfo, "clrSetupWithCache: Found cache dir ", dropPath, "\n");

        // if using built FX binaries don't check cache (dailyDevRun case)
        if (!setupArgs.match(/\/fx/i) && !FSOFileExists(dropPath+"\\System.dll")) {
            logMsg(LogClrAutomation, LogInfo, "clrSetupWithCache: FX binaries not found in cache, trying without cache\n");
        } 
        else {
            logMsg(LogClrAutomation, LogInfo, "clrSetupWithCache: using cache binaries\n");
                
                // Yeah!  we can use the cache!!
            var setupArgsDropLoc = setupArgs + " /dropLoc " + dropPath;
            if (logDir == undefined)
                logDir = Env("TEMP");
            try {
                var installDir = clrSetup(binDir, setupArgsDropLoc, logDir, verstr, runOpts);

                    // Create a marker that indicates this install was successful
                if (successFile != undefined) {
                    FSOWriteToFile(binDir, successFile);
                    logMsg(LogClrAutomation, LogInfo, "Writing ", successFile, " to indicate success\n");
                    if (installDir.toLowerCase() != runtimeRoot.toLowerCase())
                        logMsg(LogClrAutomation, LogError, "Expecting runtime in ", runtimeDir, " actual ", installDir);
                }
                return installDir;
            } catch(e) {
                    // OK we failed, check the log file to see if it was a copy failure
                var logFile = logDir + "\\ndpsetup.log";
                if (!FSOFileExists(logFile)) {
                    logMsg(LogClrAutomation, LogError, "clrSetupWithCache: Could not find log file ", logFile, " after failure!\n");
                        // throw e;
                }
                var logFileData = FSOReadFromFile(logFile);
                if (!logFileData.match(/Error.*CopyFile:.*File not found/i) &&
                    !logFileData.match(/Error.*CopyFile:.*Path not found/i) &&
                    !logFileData.match(/Command returned error.*vcwinsxsinst.cmd/i)) {
                    logMsg(LogClrAutomation, LogInfo, "clrSetupWithCache: Failure did not seem to be a copy failure\n");
                    throw e
                }
                logMsg(LogClrAutomation, LogWarn, "clrSetupWithCache: Copy file failure, trying without cache\n");
            }
        }
    }
    else {
        logMsg(LogClrAutomation, LogInfo, "clrSetupWithCache: No cache files at ", dropPath, "\n");
    }

    // do a normal clrsetup
    var installDir = clrSetup(binDir, setupArgs, logDir, verstr, runOpts);
    
    // Create a marker that indicates this install was successful
    if (successFile != undefined) {
        FSOWriteToFile(binDir, successFile);
        logMsg(LogClrAutomation, LogInfo, "Writing ", successFile, " to indicate success\n");
        if (installDir.toLowerCase() != runtimeRoot.toLowerCase())
            logMsg(LogClrAutomation, LogError, "Expecting runtime in ", runtimeDir, " actual ", installDir);
    }

    if (logDir == undefined)
        logDir = installDir;

    var logFile = logDir + "\\ndpsetup.log";
    logMsg(LogClrAutomation, LogInfo, "clrSetupWithCache: using log ", logFile, " to cache into ", cacheDir, "\n");
            // populate the cache for next time.
    try {
        _cacheClrSetup(logFile, cacheDir, clrVersion, clrBuildArch);
    } catch(e) { logMsg(LogClrAutomation, LogWarn, "Could not cache CLR files in ", cacheDir, " exception = ", e.description, "\n"); }

    logMsg(LogClrAutomation, LogInfo, "}\n");
    return installDir;
}

/****************************************************************************/
/* Run prepArmSetup.bat

   Parameters:
     binDir   : Where to place the ts.bat and ts.reg files.
     targetWindowsDir  : What windows directory that the reg/bat files should refer to.
     verStr   : version to install
 */
function prepArmSetup(binDir, targetWindowsDir, verStr) {
    logMsg(LogClrAutomation, LogInfo, "prepArmSetup('", binDir, "', '", targetWindowsDir, "', '", verStr, "') {\n");

    if (binDir == undefined) 
        throw Error(1, "Arg binDir not supplied");

    var runOpts = runSetEnv("_NTTREE", binDir);
    runOpts = runSetEnv("_BuildArch", "arm", runOpts);
    runOpts = runSetLog(LogRun, LogInfo, runOpts);
    
    var command = Env("_NTDRIVE") + Env("_NTROOT") + "\\ndp\\clr\\bin\\PrepArmSetup --noprompt --verify --sfp --full";
    
    if (verStr != undefined)
        command += " --private " + verStr;
    if (targetWindowsDir != undefined)
        command += " --sysroot " + targetWindowsDir;
    
    // Speed up BVTs via ngening.
    command += " --ngen mscorlib.dll --ngen System.dll";
    
    var run = runCmd(command, runOpts);
    
    logMsg(LogClrAutomation, LogInfo, "} prepArmSetup()\n");
    return run.exitCode;
}


/*****************************************************************************/
/* clrStress - a way for devs to run a subset of the full CLRSTRESS program 
               on their own box.

   This routine will run on the currently installed runtime, so you need
   to have the runtime setup before calling. You can also use timeToRunMin 
   to increase the run duration. 

   If 'runtime' is defined, COMPLUS_DefaultVersion is set to this which
   will cause that runtime to be used.

     Parameters:
       timeToRunMin : Run duration in minutes (from 30 to 900).
       outDir       : Where to place the test results.
       clrStressDir : The base of the ClrStress tree.
       bldArch      : The build architecture.
       runtime      : which runtime to use (COMPLUS_DefaultVersion).
*/
function clrStress(timeToRunMin, outDir, clrStressDir, bldArch, runtime) {

    if (timeToRunMin == undefined)
        timeToRunMin = "30";

    timeToRunMin = parseInt(timeToRunMin);

    if (timeToRunMin < 30 || timeToRunMin > 900) {
        throw new Error(1, "clrStress: timeToRunMin should be between 30 and 900");
    }

    if (bldArch == undefined)
        bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (clrStressDir == undefined) {
        clrStressDir = Env("_NTBINDIR") + "\\ddsuites\\src\\clr\\stress\\" + bldArch;
        if (!Env("_NTBINDIR"))
            throw new Error(1, "clrStress: Required argument clrStressDir is missing");
    }

    if (!FSOFolderExists(clrStressDir)) {
        throw new Error(1, "Can not find clrStress dir (" + clrStressDir + "). Is it missing from your SD client mapping?");
    }

    if (outDir == undefined) {
        outDir = clrStressDir + "\\clrstressRun.0";
    }

    bldArch = bldArch.toLowerCase();

    var runOpts = clrRunTemplate;
    var ndpVer = "";
    if (runtime != undefined) {
        runOpts = runSetEnv("COMPLUS_DefaultVersion", runtime, runOpts);
        ndpVer = " ndpver " + runtime;
    }

    logCall(LogClrAutomation, LogInfo, "clrStress", arguments, " {");
    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    // Convert timeToRunMin to a format required by runClrStress.bat.
    // runClrStress.bat requires a 30 minute increment. (example, If timeToRunMin
    // is 35 minutes, we want to run for 1 hour.)
    var timeToRun = (timeToRunMin+29) / 30;
    timeToRun = parseInt(timeToRun);

    var timeOutTime = (timeToRun*30 + 20) * 60; // in seconds, with 20 minutes buffer
    var idleTimeOut = 100;

    FSOTryDeleteFile(clrStressDir  + "\\Harness\\Logs\\*",  true);
    FSOTryDeleteFile(outDir + "\\lastevent.log");

    var run = runWithRuntimeArch("pushd " + clrStressDir +  " & " +
                          "call runClrStress.bat " + timeToRun,
        bldArch,
        runSetEnv("OUTDIR", outDir,
        runSetDump("runjs dumpProc %p " + outDir + "\\StressFailure.dmpRpt",
        runSetTimeout(timeOutTime,
        runSetIdleTimeout(idleTimeOut,
        runSetNoThrow( 
        runSetLog(LogRun, LogInfo, 
        runOpts)))))));

    FSOWriteToFile(run.exitCode, outDir + "\\clrStressResults.txt");

    var logDir = clrStressDir  + "\\Harness\\Logs\\";
    var logFiles = FSOGetFilePattern(logDir);
    var timestamp = FSOTimeAsFileName();
    for (var i=0; i < logFiles.length; i++) {
        var fileFull = logFiles[i].toLowerCase();
        var fileShort = fileFull.slice(logDir.length);
        var newName = "\\" + fileShort + "-"+ timestamp + ".log";
        FSOMoveFile(fileFull, outDir + newName, true);
    }

    logMsg(LogClrAutomation, LogInfo10, "} clrStress() returning ", run.exitCode, "\n");
    if (run.exitCode != 0)
        throw new Error(1, "clrStress: run returned non-zero exit code, logs in " + outDir);

    return run.exitCode;
}

/*****************************************************************************/
/* Runs the devBvts (a bucket of test that need to be run before checkin).
   This routine will run on the currently installed runtime, so you need
   to have the runtime set up before calling this routine. You can also use
   smartyArgs to pass a different set of tests (e.g., /inc BuildBvt/JIT).
   If 'runtime' is defined, COMPLUS_DefaultVersion is set to this, which
   will cause that runtime to be used.

   TODO: we should not need bldType. it is for ddsenv

     Parameters:
       smartyArgs: Additional args to pass to SMARTY (e.g., /workers:2)
       outDir    : Where to place the test results
       ddsDir    : The base of the ddSuites tree
       bldType   : The type of the build (chk, ret, ...)
       bldArch   : The architecture ("x86", "amd64", etc.)
       runtime   : Which runtime to use (COMPLUS_DefaultVersion)
       testtype  : Type of tests ("fxcop" for FxCOP testing; undefined for everything else)
explicit_ext_root: Specify the root of BVT
  expReturnCode  : Expected return code from Smarty - this will be used as test of success or failure.
                   Default value is undefined (so expected return code in "only" zero)
        bvtRoot  : The test bvt root.
*/

function devBVT(smartyArgs, outDir, ddsDir, bldType, bldArch, runtime, explicit_ext_root, testtype, expReturnCode, bvtRoot) {

    if (smartyArgs == undefined)
        smartyArgs = "/clean";
    if (ddsDir == undefined) {
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "devBVT: Required argument ddsDir is missing");
    }
    if (bldArch == undefined)
        bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (outDir == undefined) {
        outDir = Env("_NTBINDIR") + "\\ddsuites\\src\\clr\\" + bldArch + "\\smarty.run.0";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "devBVT: Required argument outDir not is missing");
    }
    if (bldType == undefined)
        bldType = "chk";

    bldArch = bldArch.toLowerCase();
    
    // Convert /workers:* to /workers:%NUMBER_OF_PROCESSORS% if > 1
    var workersRe = /\/workers:\*/;
    if (smartyArgs.match(workersRe)) {
        if (Env("NUMBER_OF_PROCESSORS") == 1) 
            smartyArgs = smartyArgs.replace(workersRe, "");
        else
            smartyArgs = smartyArgs.replace(workersRe, "/workers:" + Env("NUMBER_OF_PROCESSORS"));
    }
    
    var clr = "clr";
    if (isCoreCLRBuild(bldType))
        clr = "coreclr";

    // Cache ddsDir locally.
    // THIS IS A KLUDGE -- there should be a more generic
    // run-based thing to control input directories and output directories so
    // tasks can happen on the remote machine entirely. TODO NathanH
    if (ddsDir.match(/^\\\\/)) {
        // This should only happen in the remote case anyway.
        //var cacheDir = "%USERPROFILE%\\AppData\\Local\\RunJS.Remote\\Cache\\ddsuites";
        var cacheDir = Env("_TESTDIR");
        if (nsIsNullOrEmpty(cacheDir))
        {
            cacheDir = "C:"
        }
        cacheDir += "\\ddsuites";
        var ddsDirs = [
            ["\\", "*.* /LEV:1 /PURGE"],
            ["tools", "*.* /LEV:1 /PURGE"],
            ["tools\\" + bldArch, "/MIR"],
            ["src\\" + clr + "\\" + bldArch, "/MIR"]];
                                
        robocopyDirsForBuild(ddsDir, cacheDir, ddsDirs, outDir);
        ddsDir = cacheDir;
    }

    // Set the WOW mode if applicable
    var realProcArch = getRealProcessorArchitecture(); 
    var ldr64ExeForWoW = undefined;
    var isWowCurrentlyOn = true;
    
    if (clr == "coreclr" && testtype == "fxcop")
    {
        if (Is64bitArch(realProcArch, "coreclr"))
        {
            // Set 32bit loader... else layering tools will fail
            ldr64ExeForWoW = getLdr64Exe();
            var queryRun = runCmdToLog(ldr64ExeForWoW + " query", runSetNoThrow());
            var queryExit = queryRun.exitCode;
            if (queryExit == 0)
                ldr64ExeForWoW = undefined;
            else
            {
                logMsg(LogClrTask, LogWarn, "ldr64 setwow\n\n");
                runCmd(ldr64ExeForWoW + " setwow");
                isWowCurrentlyOn = false;
            }
        }
    }

    
    if (explicit_ext_root != undefined)
    {
        // Dont set these environment variables else Smarty will use the loader!
        if (testtype != "fxcop")
        {
            Env("EXT_ROOT_OVERRIDE") = explicit_ext_root;
            
            // Set CORE_RUN - see preptests.bat
            Env("CORE_RUN") = explicit_ext_root + "\\fxprun.exe";
        }

        // Set the CORE_ROOT
        Env("CORE_ROOT") = explicit_ext_root; // For testlibrary.dll - see preptests.bat
    }
                    
    if (runtime == undefined)
        runtime = Env("COMPLUS_DefaultVersion");
        
    logCall(LogClrAutomation, LogInfo, "devBVT", arguments, " {");

    if (explicit_ext_root != undefined)
    {
        Env("EXT_ROOT_OVERRIDE") = explicit_ext_root;
    }

    if (bldArch != "arm") 
    {
        smartyArgs = "/noie /failfast:10000 " + smartyArgs;
    }
    // KLUDGE
    else
    {
        smartyArgs = "\"/Inc:CHECKINBVT;BUILDBVT\"";
    }

    if (bvtRoot == undefined)
    {
        bvtRoot = ddsDir + "\\src\\" + clr +"\\";
        if (clr == "coreclr")
        {
            // For CoreCLR, for the moment, force the AMD64 devBVT to use the same tests as X86
            if (bldArch.toLowerCase() == "amd64")
                bvtRoot = bvtRoot + "x86";
            else
                bvtRoot = bvtRoot + bldArch;
        }
        else
        {
            bvtRoot = bvtRoot + bldArch;
        }
    }

    var result = _smartyWorker(smartyArgs, outDir, ddsDir, bvtRoot, bldType, bldArch, runtime, testtype, expReturnCode);
    logMsg(LogClrAutomation, LogInfo, "\n}\n");
    
    // Reset loader mode
    if (clr == "coreclr" && testtype == "fxcop")
    {
        if (isWowCurrentlyOn == false) {
            logMsg(LogClrTask, LogWarn, "ldr64 set64\n\n");
            runCmd(ldr64ExeForWoW + " set64");
        }
    }

    return result;
}

/****************************************************************************/ 
/*
    Helper to return if the machine has 2 processors.
    This can be used to decide if some things should be run in parallel 
    (e.g., run smarty w/ "/workers:2")
*/
function _isDualProc()
{
// We run the systemInfo command, which prints out stuff like this:
//Processor(s):              2 Processor(s) Installed.
//                           [01]: x86 Family 6 Model 6 Stepping 2 AuthenticAMD ~1666 Mhz
//                           [02]: x86 Family 6 Model 6 Stepping 2 AuthenticAMD ~1666 Mhz
//BIOS Version:              ASUS   - 30303031
// We then look for the "[02]" substring which only exists in listing the 2nd processor.
// this doesn't work for dual-core machines, so also check NUMBER_OF_PROCESSORS
    if (Env("NUMBER_OF_PROCESSORS") > 1)
        return true;
    return (runCmd("systemInfo", runSetNoThrow()).output.match(/Processor(.|\n)*\[02\](.|\n)*BIOS/m));
}

/*****************************************************************************/
/* Internal worker to invoke Smarty. This can be used for either Devbvt
    or selfhost.
*/

function _smartyWorker(smartyArgs, outDir, ddsDir, bvtRoot, bldType, bldArch, runtime, testtype, expReturnCode) {
    // TODO: Vista: We should update this worker to support forking to an elevated window if needed by the test

    if (smartyArgs == undefined)
        throw new Error("_smartyWorker: Need argument 'smartyArgs'");
    if (outDir == undefined)
        throw new Error("_smartyWorker: Need argument 'outDir'");
    if (ddsDir == undefined)
        throw new Error("_smartyWorker: Need argument 'ddsDir'");
    if (bvtRoot == undefined)
        throw new Error("_smartyWorker: Need argument 'bvtRoot'");
    if (bldType == undefined)
        throw new Error("_smartyWorker: Need argument 'buildType'");
    if (bldArch == undefined)
        throw new Error("_smartyWorker: Need argument 'bldArch'");
    if (runtime == undefined)
        throw new Error("_smartyWorker: Need argument 'runtime'");
    var clr;
    if (isCoreCLRBuild(bldType)) {
        clr = "coreclr";
    } else {
        clr = "clr";
    }

    bldArch = bldArch.toLowerCase();

    var runOpts = runSetEnv("COMPLUS_DefaultVersion", runtime, runOpts);
    //If COMPlus_Version is set currently, make sure to set COMPlus_Version to runtime.  If we're running a
    //baseline test pass we've changed complus_defaultversion.  We should change version to match if and
    //only if it is already set.  This should make the baseline match the original test pass.
    if (Env("COMPlus_Version") != undefined)
    {
        runOpts = runSetEnv("COMPlus_Version", runtime, runOpts);
    }

    //SDK_ROOT is stored in the registry by clrsetup, so it's set to whatever your last clrsetup did.
    //Let's come up with a more consistent set of variables so that PrepTests doesn't mess up the test
    //environment.
    var ndpInstallRoot = Env("SystemRoot") + "\\Microsoft.NET\\Framework";
    if ((bldArch == "ia64") || (bldArch == "amd64"))
        ndpInstallRoot += "64";
    runOpts = runSetEnv("SDK_ROOT", ndpInstallRoot + "\\" + runtime + "\\sdk\\", runOpts);
    //And now clear out the rest of the variables PrepTests uses (smarty.pl uses the same logic).
    //Some of this logic exists in devbvt.bat.
    runOpts = runSetEnv("EXT_ROOT", '', runOpts);
    runOpts = runSetEnv("BVT_ROOT", '', runOpts);
    runOpts = runSetEnv("EXT_ROOT_ORCASGREEN", '', runOpts);
    runOpts = runSetEnv("REFERENCE_ASSEMBLIES_ORCASGREEN", '', runOpts);
    
    if (expReturnCode != undefined)
    {
        // Set the expected return code value for smarty
        runOpts = runSetExpectedReturnCode(100, runOpts);
    }
    
    var ndpVer = " ndpver " + runtime;

    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    var baselineSrc = undefined;
    var baselineBinDir = undefined;
    var skipMain = false;
    var fileBugs = false;
    if (smartyArgs.match(/^(.*)\/baselineSrc:(\S+)(.*)$/)) {
        smartyArgs = RegExp.$1 + RegExp.$3;
        baselineSrc = RegExp.$2;
    }
    if (smartyArgs.match(/^(.*)\/baselineBin:(\S+)(.*)$/)) {
        smartyArgs = RegExp.$1 + RegExp.$3;
        baselineBinDir = RegExp.$2;
    }
    if (smartyArgs.match(/^(.*)\/skipMain\b(.*)$/)) {
        skipMain = true;
    }
    if (smartyArgs.match(/^(.*)\/fileBugs\b(.*)$/)) {
        smartyArgs = RegExp.$1 + RegExp.$2;
        if (baselineSrc == undefined && baselineBinDir == undefined) {
            logMsg(LogClrAutomation, LogInfo, "Cannot file bugs without a baseline to run, ignoring /fileBugs.\n");
        } else {
            fileBugs = true;
        }
    }

    if (smartyArgs.match(/^(.*)\/workers:\d+(.*)$/)) {
        var newSmartyArgs = RegExp.$1 + RegExp.$2;
        // turn off workers:2 if we are not on a multi-proc box
        if (!_isDualProc())
            smartyArgs = newSmartyArgs;
    }

        // We need to enter ddsEnv because some dev unit tests depend on this
        // (Although arguably they shouldn't they use it simply to find the
        // directory their source lives in (which %dp0 gives you for free)
        // Anyway, they should not be using the variables below, so we set them
        // to something that will cause a fast failure.
        // set them to dummy values that hopefully will fail quickly if they are used
    ddsBuilt   = "runjs_devBVT_does_not_define_DD_BuiltTarget";             // Where the binaries are (but you really should not care)
    ddsTarget  = "runjs_devBVT_does_not_define_DD_SuiteTarget";             // Where to build tests to (but we are not building tests)

    //Let's start dhandler before running the tests.
    var iarch = bldArch == "x86" ? "i386" : bldArch;

    //in ddsuites dhandler is in one place, and it's in a different place in the live test drop.
    var dhandlerPath = bvtRoot + "\\Desktop\\tools\\" + iarch + "\\dhandler.exe";

    //Where is dhandler.exe in the live test drop???
    if (!FSOFileExists(dhandlerPath))
    {
        //try the other location.
        dhandlerPath = bvtRoot + "\\tools\\" + iarch + "\\dhandler.exe";
    }
    var dhandler = runDo(dhandlerPath, runSetLog(LogRun, LogInfo, runSetNoThrow()));
    runPoll(dhandler); //This actually starts dhandler

    try
    {
        // devbvt must be called from the directory we're going to run it in. 
        // So we pushd to that dir, and then call the ddsenv.bat file to setup our environment.
        var ddsenvBat = ddsDir + "\\ddsenv.bat";

        var exitCode = 1;
        if (skipMain) {
            // Just check for the output file
            logMsg(LogClrAutomation, LogInfo, "skipMain qualifer specified, not running main devbvt\n");
            if (FSOFileExists(outDir + "\\Smarty.xml") && FSOGetFilePattern(outDir,"Smarty.*fail.smrt").length == 0)
                exitCode = 0;
        }
        else {
            var run;
            
            var clr;
            if (isCoreCLRBuild(bldType)) {
                clr = "coreclr";
            } else {
                clr = "clr";
            }

            if (clr == "coreclr" || testtype == "selfhost")
            {
                //We want to run the right number of tests too.
                //This doesn't work when run through ddsenv.  If we just removed ddsenv, we'd all be
                //happier.
                smartyArgs += " /ds Enabled;ConfirmFixed";

            }
            if (clr == "coreclr")
            {
                var envTestType;
                if (testtype == "fxcop")
                    envTestType = "fxcop";
                else
                    envTestType = "regular";
                
                //This can be removed when we get a ddsuites update.
                var devbvtScript = FSOFileExists(bvtRoot + "\\devbvt_coreclr.bat") ? "devbvt_coreclr.bat"
                                                                                   : "devbvt.bat";
                run = runWithRuntimeArch("pushd "
                                             + bvtRoot
                                             +  " & "
                                             + "call " + devbvtScript + " /outputDir "
                                             + outDir
                                             + " "
                                             + smartyArgs,
                                         bldArch,
                                         runSetEnv("CORECLR_TEST_TYPE",envTestType,
                                         runSetTimeout(2 * HOUR,
                                             runSetNoThrow(
                                             runSetLog(LogRun, LogInfo, 
                                             runOpts)))));
            }
            else if (testtype == "selfhost")
            {
                //Don't invoke ddsenv for selfhost runs.  It has special requirements for various environment
                //variables.
                run = runWithRuntimeArch( ".\\devbvt /outputDir " + outDir + " " + smartyArgs,
                                             bldArch,
                                             runSetTimeout(20 * HOUR,
                                               runSetNoThrow(
                                               runSetCwd(bvtRoot,
                                               runSetLog(LogRun, LogInfo, 
                                               runOpts)))));

            }
            else if (testtype == "verifystate")
            {
                //Don't invoke preptests for verifystate runs.  The tests use smarty but do not use the
                //runtime itself.  This way it runs on clean machines.
                //
                //BUGBUG Tue 9/9/2008
                //I feel pretty bad about this.  I'm actually running smarty without preptests.  So I've
                //captured all the state I think I need to set to run this.  But that's pretty brittle.  If
                //you break this, don't feel guilty.  It's ultimately my fault.
                //
                //In need to set BVT_ROOT, PERL5LIB, and _TGTCPU
                runOpts = runSetEnv("BVT_ROOT", bvtRoot, runOpts);
                runOpts = runSetEnv("_TGTCPU", iarch, runOpts);
                runOpts = runSetEnv("PERL5LIB", bvtRoot + "\\Common\\tools\\" + iarch + "\\perl\\lib;" +
                                                bvtRoot + "\\Common\\tools\\" + iarch + "\\perl\\site\\lib;" +
                                                bvtRoot + "\\Common\\Smarty;" +
                                                Env("PERL5LIB"), runOpts);
                run = runCmd(".\\smarty.bat /outputDir " + outDir + " " + smartyArgs,
                             runSetTimeout(5 * MINUTE,
                                           runSetNoThrow(
                                           runSetCwd(bvtRoot,
                                           runSetLog(LogRun, LogInfo,
                                           runOpts)))));
            }
            else
            {
                // The dev unit tests require ddsEnv to run so we launch devbvt under ddsenv.bat
                run = runWithRuntimeArch("pushd " + bvtRoot +  " & " +
                                             "call " + ddsenvBat + " " +
                                             ndpVer +
                                             " suiteroot " + ddsDir +
                                             " built " + ddsBuilt +
                                             " suitetarget " + ddsTarget +
                                             " flavor " + bldType + " nocolor nostart exec " +
                                             ".\\devbvt /outputDir" + (bldArch == "arm" ? ":" : " ") + outDir + " " + smartyArgs,
                                             bldArch,
                                             runSetTimeout(20 * HOUR,
                                               runSetEnv("RUN_FXCOP", 1,
                                               runSetNoThrow(
                                               runSetLog(LogRun, LogInfo, 
                                               runOpts)))));
            }

            exitCode = run.exitCode;
            
            // HACK: smarty can return 0 or 100 for success depending on which version is used.
            if( exitCode == 100)
                exitCode = 0;
        }
        
        var smartyFile = outDir + "\\Smarty.xml";
        if (FSOFileExists(smartyFile))
            _createSmartyErrFile(smartyFile);

                // we should not be creating anything here but you never know about the future
        if (FSOFolderExists(ddsTarget)) {
            logMsg(LogClrAutomation, LogWarn, "_smartyWorker: someone used ddsTarget!\n");
            FSODeleteFolder(ddsTarget, true);
        }
        logMsg(LogClrAutomation, LogInfo, "_smartyWorker: Main run exit code = ", exitCode, "\n");
        var machineStateOK = true;
        if (exitCode != 0) {
            if (baselineSrc != undefined) {
                /* XXX Fri 8/8/2008
                 * Protect this with baselineSrc to make sure we don't cause infinite recursion.
                 */
                //If we have failures, immediately check to see if the machine state is ok.
                /* BUGBUG Fri 8/8/2008
                 * Try to reuse the result of a run as a top level task.
                 */
                if (0 != verifyMachineState(bldArch, bldType, undefined, undefined,
                                            outDir + "\\VerifyMachineState"))
                {
                    logMsg(LogClrAutomation, LogError, "verifyMachineState failed to verify machine state.\n");
                    machineStateOK = false;
                }

                logMsg(LogClrAutomation, LogInfo, "_smartyWorker: Looking for a suitable CLR baseline for source at ", baselineSrc, "\n");
                var baselineData = getClrFxBaselineData(baselineSrc);
                if (baselineData != undefined) {
                    //Snap builds the coreclr fre build as "ret" because it was renamed recently.
                    if (clr == "coreclr" && bldType == "corefre")
                    {
                        baselineBinDir = baselineData.snapJobDir + "\\" + bldArch + "coreret" + "\\bin";
                    }
                    else
                    {
                        baselineBinDir = baselineData.snapJobDir + "\\" + bldArch + bldType + "\\bin";
                    }
                    var semaphoreFile = (clr == "coreclr") ? "\\coreclr.dll" : "\\clr.dll";
                    for(var i = 0; !FSOFileExists(baselineBinDir + semaphoreFile); i++)  {
                        WScript.Sleep(200); 
                        if (i > 50) {
                            logMsg(LogClrAutomation, LogWarn, "_smartyWorker: Build ", baselineBinDir, " is not accessible, failing\n");
                            baselineBinDir = undefined;
                            break;
                        }
                    }
                }
                else 
                    logMsg(LogClrAutomation, LogInfo, "_smartyWorker: could not find a suitable CLR baseline for ", baselineSrc, "\n");
            }

            //Before we install the baseline build, verify the machine state.
            if (baselineBinDir != undefined && fileBugs && !machineStateOK)
            {
                //We failed to verify the machine state.  Disable bug filing.
                fileBugs = false;
                logMsg(LogClrAutomation, LogError, "Will not file bugs due to bad machine state.\n");
            }
            fileBugs = false;
            if (baselineBinDir != undefined) {
                var verstrBaseline = bldArch + bldType + ".baseline";
                exitCode = 1;

                logMsg(LogClrAutomation, LogInfo, "_smartyWorker: Installing the baseline ", baselineBinDir, "\n");
                var coreclrInstallRoot = undefined;
                if (clr == "coreclr")
                {
                    coreclrInstallRoot = outDir + "\\CoreCLR\\v2.0." + bldArch + bldType;
                    fxpSetup(baselineBinDir, coreclrInstallRoot);
                }
                else
                {
                    var baselineUrttarget = clrSetupWithCache(
                        baselineBinDir, 
                        "/fx /nrad", 
                        outDir + "\\baseline_setup", 
                        undefined, 
                        verstrBaseline);
                    
                    if (baselineUrttarget == undefined) { 
                        logMsg(LogClrAutomation, LogInfo, " Error installing baseline runtime, returning ", exitCode, "\n");
                        return exitCode;
                    }
                }
                
                var smartyFailFiles = FSOGetFilePattern(outDir,"Smarty.*fail.smrt")
                if (smartyFailFiles.length > 0) {
                    var smartyFailFile = smartyFailFiles[smartyFailFiles.length-1];
                    var baselineOutDir = outDir + "\\baseline";
                    FSOCreatePath(baselineOutDir);
                    FSOWriteToFile("BASELINE_BINARIES: " + baselineBinDir + "\r\n", baselineOutDir + "\\baselineBinDir.txt");
                    
                    logMsg(LogClrAutomation, LogInfo, "Running failed tests '" + smartyFailFile + "'\n");
                    try {
                        devBVT(
                            "/retryFailure:0 /clean /testFile " + smartyFailFile, 
                            baselineOutDir,
                            ddsDir, 
                            bldType, 
                            bldArch, 
                            "v4.0." + verstrBaseline,
                            coreclrInstallRoot, /*explicit_ext_root*/
                            testtype,
                            undefined, /*expReturnCode*/
                            bvtRoot);
                    }
                    catch(BaselineException) { }

                    // Set any machine global state back to that for the original (non-baseline) CLR
                    // In theory this should not be necessary - anyone relying on such state (eg. ASP.Net settings)
                    // can't safely assume the machine is in the state they expect.  However, for now we'll also undo
                    // our changes here, in addition to setting the machine state at known appropriate points.
                    if (clr != "coreclr")
                    {
                        publishRunOpts = runSetEnv("COMPLUS_DefaultVersion", runtime);
                        if (NeedToEnterWow64(bldArch)) {
                            // If we're switching to a 32bit CLR on a 64-bit machine, need to use 32-bit shell
                            publishRunOpts = runSet32Bit(publishRunOpts);
                        }
                        runCmdToLog("call " + ddsDir + "\\tools\\publishClr.bat", publishRunOpts );
                    }
                    
                    var baselineFailFiles = FSOGetFilePattern(baselineOutDir,"Smarty.*fail.smrt")
                    if (baselineFailFiles.length > 0) {
                        var baselineFailFile = baselineFailFiles[baselineFailFiles.length-1];
                        logMsg(LogClrAutomation, LogInfo, "Baseline failed tests '" + baselineFailFile + "'\n");
                        //It would be nice to filter out non-deterministic failures from this list, but this
                        //is too low tech for that.
                        if (FSOReadFromFile(baselineFailFile) == FSOReadFromFile(smartyFailFile)) { 
                            logMsg(LogClrAutomation, LogInfo, "Errors same as baseline, returning special error code = 2\n");
                            exitCode = 2;
                        }
                    }

                    // Consider removing try/catch once bug creation is visible
                    try {
                        _fileBugsForDDRFailures(outDir, baselineOutDir, baselineSrc, !fileBugs);
                    }
                    catch(BugCreationException) {
                        logMsg(LogClrAutomation, LogInfo, "Error filing bugs for failures.\n");
                    }
                }
                else
                    logMsg(LogClrAutomation, LogInfo, "No smarty fail file was found in ", outDir, "\n");
            }
        }
        else
        {
            logMsg(LogClrAutomation, LogInfo10, "} devBVT() returning ", run.exitCode, "\n");
            if (exitCode != 0)
                throw new Error(1, "devBVT: run returned non-zero exit code, logs in " + outDir);
        }

        return exitCode;
    }
    finally
    {
        runTerminate(dhandler); //We don't need dhandler anymore
    }
}

function verifyMachineState(bldArch, bldType, ddsuitesRoot, ddsuitesDir, outDir, abortFile)
{
    //The bldArch is not that relevant and bldType doesn't really matter.  It exists only 
    if (bldArch == undefined)
    {
        bldArch = Env("_BuildArch");
        if( bldArch == undefined )
        {
            bldArch = "x86";
        }
        bldArch = bldArch.toLowerCase();
    }
    if (bldType == undefined)
        bldType = "chk";
    if (ddsuitesRoot == undefined)
        ddsuitesRoot = srcBaseFromScript() + "\\ddsuites";
    if (ddsuitesDir == undefined)
        ddsuitesDir = ddsuitesRoot + "\\src\\clr\\" + bldArch;
    if (outDir == undefined)
        outDir = ddsuitesDir + "\\Smarty.ivt.run.0";

    bldArch = bldArch.toLowerCase();

    var smartyArgs = "/lst " + ddsuitesDir + "\\IVT.lst " +
                     "/inc Infrastructure\\private\\ddr " + 
                     "/noie /clean";
    var code = _smartyWorker(smartyArgs, outDir, ddsuitesRoot, ddsuitesDir, bldType, bldArch,
                             "v4.0." + bldArch + bldType, "verifystate", 100);
    logMsg(LogClrAutomation, LogInfo, "verifyMachineState: _smartyWorker returns " + code + "\n");

    //Cover all our bases.
    if (code == 100 || code == 0)
        return 0;

    //This is the easiest way to bring DDR to a halt.
    if (abortFile)
    {
        logMsg(LogClrAutomation, LogInfo, "verifyMachineState: Creating abort file\n");
        FSOWriteToFile("verifyMachineState: Aborting jobs\n", abortFile);
    }
    return code;

}

//Since this both formats the page and actually files the bugs, pageOnly produces the report (without
//actually giving the NewBug link.
function _fileBugsForDDRFailures(outDir, baselineOutDir, baselineSrc, pageOnly) {
    if (outDir == undefined) {
        logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: Need argument 'outDir', returning.\n");
        return;
    }
    if (baselineOutDir == undefined) {
        logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: Need argument 'baselineOutDir', returning.\n");
        return;
    }
    if (baselineSrc == undefined) {
        logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: Need argument 'baselineSrc', returning.\n");
        return;
    }

    var smartyDataFileName = outDir + "\\Smarty.err.xml";
    if (!FSOFileExists(smartyDataFileName)) {
        logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: ", smartyDataFileName, " does not exist, returning.\n");
        return;
    }

    var baselineDataFileName = baselineOutDir + "\\Smarty.xml";
    if (!FSOFileExists(baselineDataFileName)) {
        logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: ", baselineDataFileName, " does not exist, returning.\n");
        return;
    }

    var branchName = _getBranchFromSrcBase(baselineSrc);
    if (branchName == undefined) {
        logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: could not resolve branch from 'baselineSrc' argument, returning.\n");
        return;
    }

    var syncLists = sdWhereSynced(baselineSrc + "\\ndp\\clr\\src", baselineSrc);
    var syncDate = new Date(syncLists.minChangeTime);

    var smartyData = xmlRead(smartyDataFileName);
    var testsInfo = smartyData.TESTRESULT.MYBLOCK.TESTCASES;
    var tests = toList(testsInfo.TESTCASE);

    var baselineData = xmlRead(baselineDataFileName);
    var baselineTestsInfo = baselineData.TESTRESULT.MYBLOCK.TESTCASES;
    var baselineTests = toList(baselineTestsInfo.TESTCASE);

    var baselineTable = {};
    for (var i = 0; i < baselineTests.length; i++) {
        var baselineTest = baselineTests[i];
        baselineTable[baselineTest.TESTID] = baselineTest;
    }

    for (var i = 0; i < tests.length; i++) {
        var test = tests[i];
        if (test.SUCCEEDED == "yes") {
            continue;
        }

        var baselineTest = baselineTable[test.TESTID];
        if (baselineTest == undefined) {
            logMsg(LogClrAutomation, LogInfo, "_fileBugsForDDRFailures: Baseline result not found for failure, skipping ", test.TESTID, "\n");
        }

        var isDeterministic = (test.PASSRATE == "0%");

        // Failures where the baseline passed could be due to bad local changes.
        // Note that non-deterministic failures that don't repro in the baseline may still be
        // product bugs.  However, experience suggests that the signal-to-noise ratio is too low
        // to justify filing bugs for them automatically.
        if (baselineTest.SUCCEEDED == "yes") {
            continue;
        }

        var testName = test.NAME;
        var testId = test.TESTID;
        var testOwner = test.TESTOWNER;

        // Test owner may be a semi-colon-delimited list.  If so, take the first alias.
        if (testOwner.match(/^(\S+);(.*)$/)) {
            testOwner = RegExp.$1;
        }
        
        var failedLogs = [];
        _addFailedLogs(failedLogs, test);
        _addFailedLogs(failedLogs, baselineTest, true);
        _addDumpFiles(failedLogs, test);
        failedLogs = failedLogs.join(",");
        if (pageOnly == undefined || pageOnly == false)
        {
            _formatDDRBugs(testName, testId, testOwner, branchName, syncDate, isDeterministic, failedLogs, smartyData.TESTRESULT.START_TIME.toString());
        }

        failedLogs = failedLogs.split(",");
        for (var j = 0; j < failedLogs.length; j++) {
            try {
                FSODeleteFile(failedLogs[j]);
            }
            catch (e) { continue; }
        }
    }
}
function _createTempFileName(index, isBaseline, time, extension)
{
    if(extension == undefined)
        extension = "html";
    if(time == undefined)
    {
        time = new Date().toString();
        time = time.replace(/ /g, "_");
        time = time.replace(/:/g, "_");
    }
    var baseFileName = Env("TEMP") + "\\" + Env("COMPUTERNAME") + "_" + time;
    var returnFileName = baseFileName + "_" + index;
    if(isBaseline) 
        returnFileName += "_baseline";

    returnFileName += "." + extension;
    return returnFileName;

}

function _convertLogsToTempFiles(failedLogs)
{
    var log;
    var newLogs = [];
    var isBaseline = false;
    var fileName;
    for(var i in failedLogs)
    {
        log = failedLogs[i];
        if (!FSOFileExists(log))
        {
            continue;
        }
        if(log.match(/\\baseline\\/i))
            isBaseline = true;
        else
            isBaseline = false;
        fileName = _createTempFileName(i, isBaseline);
        try
        {
            FSOCopyFile(log, fileName, true);
        }
        catch (e) {}

        newLogs[i] = fileName;
    }
    return newLogs;
}

function _addFailedLogs(failedLogs, test, isBaseline) {
    if (isBaseline == undefined) {
        isBaseline = false;
    }

    var runs = toList(test.RUN);

    for (var i = 0; i < runs.length; i++) {
        var run = runs[i];

        if (run.SUCCEEDED == "yes") {
            continue;
        }

        var log = run.TESTLOG;
        var time = run.STARTTIME.replace(/ /g, "_");
        time = time.replace(/:/g, "_");
        
        var baseFilePath = _createTempFileName(failedLogs.length, isBaseline, time);

        try {
            FSOCopyFile(log, baseFilePath, true);
        }
        catch (e) {}

        if (!FSOFileExists(baseFilePath)) {
            continue;
        }

        failedLogs[failedLogs.length] = baseFilePath;
    }
}
function _addDumpFiles(failedLogs, test) {
    var runs = toList(test.RUN);
    for(var i = 0; i < runs.length; i++) {
        var run = runs[i];
        if(run.SUCCEEDED == "yes") {
            logMsg(LogClrAutomation, LogInfo, "Skipping\n");
            continue;
        }
        if (run.DMPRPT == null) {
            // The test report doesn't have associtated dump report directory
            continue;
        }
        var basePath = run.DMPRPT.DIR;
        var dumpList = FSOGetFilePattern(basePath, /.*\.dmp$/i, true);
        var time = run.STARTTIME.replace(/ /g, "_");
        time = time.replace(/:/g, "_");
        for(var dumpPath in dumpList) {
            if(dumpList[dumpPath] == "")
                continue;
            var baseFilePath = _createTempFileName(failedLogs.length, false, time, "dmp");
            try {
                FSOCopyFile(dumpList[dumpPath], baseFilePath, true);
            }
            catch (e) {}
            if(!FSOFileExists(baseFilePath)) {
                continue;
            }
            failedLogs[failedLogs.length] = baseFilePath;
        }
    }
}

// Input: suitableForFolderName - Strip out foldername disallowed characters '$' and '/'
function _getBranchFromSrcBase(srcBase, suitableForFolderName) {
    if (_inTFS())
    {
        var result = "";
        if (srcBase.match(/^\\\\/))
        {
            // TFScript doesn't work so well remotely, so just assume pu/CLR
            result = "$/DevDiv/pu/clr";
        }
        else
        {
            var cmdString = TFScriptCall() + " getServerPath "+srcBase;
            result = runCmdToLog(cmdString);
            result = trim(result.output);
        }
        if (!result.match(/\$.*/))
        {
            throw new Error(1, "Couldn't get server path from TFS.  Received: "+result);
        }
        if (suitableForFolderName) {
            // Change $/Dev10/pu/clr to Dev10_pu_clr
            result = result.replace(/\$\//g, "");
            result = result.replace(/\//g, "_");
        }
        return result;
    }

    var depotPath = sdWhere(srcBase + "\\ndp\\clr\\src", srcBase).toLowerCase();
    if (depotPath.match(/^\/\/depot\/devdiv\/(\S+)\/ndp\/clr\/src$/)) {
        var branchPath = RegExp.$1;

        // TODO: Find or decide the one place this mapping should live
        // These branch names match those used in the DevDiv Bugs PS database
        if (branchPath == "pu\/clr") return "puclr";
    } 
    
    // no match
    return undefined;
}
/***********************************************************************/
/*  Create a DDR bug for a test failure.
    Input: testName - name of test.
           testId - ID of the test case.
           testOwner - owner of test
           branchName - branch DDR is testing (puclr, Lab21s, etc)
           syncDate - Date DDR was run, in form "Thu Apr 13 11:46:59 PDT 2006"
           isDeterministic - "true" or "false" indicating whether bug was hit every time in the run
           failedLogs - comma separated list of failure logs.
           startTimeStr - Time test failed, in form "Thu Apr 13 11:46:59 PDT 2006" (optional)
           createNew - "true" or "false", "true" or undefined indicates you should always create a new bug,
                        "false" indicates you should only create a new bug if,
                        a current bug does not exist and is active.
*/
function formatAndCreateDDRBug(testName, testId, testOwner, branchName, syncDate, isDeterministic, failedLogs, startTimeStr, createNew)
{
    if(startTimeStr == undefined)
        startTimeStr = new Date().toString();
    if(createNew == undefined)
        createNew = true;
    var originalLogs = failedLogs;
        // create local temporary files for test logs
    if(failedLogs)
    {
        failedLogs = failedLogs.split(",");
        failedLogs = _convertLogsToTempFiles(failedLogs);
        failedLogs = failedLogs.join(",");
    }

    // file DDR bug
    _formatDDRBugs(testName, testId, testOwner, branchName, syncDate, isDeterministic, failedLogs, startTimeStr, createNew, originalLogs);
    // delete any local temp files we created
    if(failedLogs)
    {
        failedLogs = failedLogs.split(",");
        for (var j = 0; j < failedLogs.length; j++) {
            try {
                FSODeleteFile(failedLogs[j]);
            }
            catch (e) { continue; }
        }
    }
}

function _parseEnvironmentInfoFromLogfiles(originalLogs)
{
    var environmentInfo = {};
    environmentInfo.FilingFromTestMachine = true;
    environmentInfo.String = undefined;
    environmentInfo.TestMachine = Env("COMPUTERNAME").toUpperCase();

        // and break out environment components from filename
    if( originalLogs != undefined && originalLogs.length > 0)
    {
        var logComponents = originalLogs[0].split("\\");
            // if logs coming from network share...
        if (originalLogs[0].indexOf("\\\\") == 0)
        {
            if (logComponents.length >= 2)
            {
                environmentInfo.TestMachine = logComponents[2].toUpperCase();
                environmentInfo.FilingFromTestMachine = (environmentInfo.TestMachine == Env("COMPUTERNAME").toUpperCase());
                    // the 5th slot in the split url is:
                    // 0 1      2            3            4           5
                    //  \ \ foo-server \ automation \ run.current \ x86chk
                if (logComponents.length >= 5)
                {
                    environmentInfo.String = logComponents[5];
                }
            }
        }
        else // assume local machine, so search for automation folder
        {
            for (var i in logComponents)
            {
                    // the i+2 slot in the split path is:
                    //  0     1      2         3 (i)       4 (i+1)     5 (i+2)
                    //  c: \ vbl \ puclr \ automation \ run.current \ x86chk
                if (logComponents[i].toUpperCase() == "AUTOMATION" && (logComponents.length > (i + 2)))
                {
                    environmentInfo.String = logComponents[i+2];
                }
            }
        }
    }
    return environmentInfo;
}

function _parseEnvironmentString(filingFromTestMachine, testEnvironmentString)
{
        // we can't trust proc-arch since we may not be on test machine, and we may be running under wow
        // get processor architecture from logfile url
    var environment = "";
    if (testEnvironmentString != undefined)
    {
        if (testEnvironmentString.toUpperCase().indexOf("X86") != -1)
        {
            environment = "Processor_Architecture=x86";
        }
        else
        if (testEnvironmentString.toUpperCase().indexOf("IA64") != -1) {
            environment = "Processor_Architecture=ia64";
        }
        else
        if (testEnvironmentString.toUpperCase().indexOf("AMD64") != -1) {
            environment = "Processor_Architecture=amd64";
        }

            // get build type from logfile url
        if (testEnvironmentString.toUpperCase().indexOf("CHK") != -1)
        {
            if (environment != "") environment += ",";
            environment += "BuildType=CHK";
        }
        else
        if (testEnvironmentString.toUpperCase().indexOf("FRE") != -1) {
            if (environment != "") environment += ",";
            environment += "BuildType=FRE";
        }
        else
        if (testEnvironmentString.toUpperCase().indexOf("RET") != -1) {
            if (environment != "") environment += ",";
            environment += "BuildType=RET";
        }
    }
        // Add OS environment and architecture
    if (filingFromTestMachine)
    {
        if (environment != "") environment += ",";
        environment += "OS=" + nsGetOSVersion();
        if (IsWinLH())
            environment += "[Vista]";
        if (IsWin7())
            environment += "[Windows 7]";

    }
    return environment;
}

function _formatDDRBugs(testName, testId, testOwner, branchName, syncDate, isDeterministic, failedLogs, startTimeStr, createNew, originalLogs) {
    if(Env("PROCESSOR_ARCHITECTURE").toLowerCase() != "x86")
    {
        var cscript = "cscript";
        if (Env("PROCESSOR_ARCHITECTURE").toLowerCase() != "x86")
            cscript = Env("WINDIR") + "\\SysWow64\\cscript.exe";
        WScript.ScriptFullName.search(/([^"].+)\\[^\\]+$/);
        var runjs = RegExp.$1 + "\\runjs.wsf";
        var commandString = cscript + " " + runjs + " _formatDDRBugs \"" + testName + "\" \"" + testId + "\" \"" + testOwner + "\" \"" + branchName + "\" \"" + syncDate + "\" \"" + isDeterministic + "\" \"" + failedLogs + "\" \"" + startTimeStr + "\"";
        if(createNew != undefined)
            commandString += " \"" + createNew + "\"";
        if (originalLogs != undefined)
            commandString += " \"" + originalLogs + "\"";
        var result = runCmd(commandString);
        WScript.Echo(result.output);
        return;
    }
    else
        logMsg(LogClrAutomation, LogInfo, "_formatDDRBugs(" + testName + ", " + testId + ", " + testOwner + ", " + branchName + ", " + syncDate + ", " + isDeterministic + ", " + failedLogs + ", " + startTimeStr + ", " + createNew + ", " + originalLogs + ")");

    var bug = {};
    if(failedLogs)
        failedLogs = failedLogs.split(",");
    if(originalLogs)
        originalLogs = originalLogs.split(",");
    isDeterministic = (isDeterministic.toString().toLowerCase() == "true");
    var primaryTestOwner = testOwner.split(",")[0];

    syncDate = new Date(syncDate);
    
    if(createNew == undefined)
        createNew = false;

    if(failedLogs == "_")
        failedLogs = undefined;

    if(originalLogs == undefined)
        originalLogs = failedLogs;

    if(startTimeStr == "_")
        startTimeStr = new Date().toString();

    if(branchName == undefined || branchName == "undefined") {
        logMsg(LogClrAutomation, LogInfo, "Cannot file bug to undefined branch.");
        return;
    }

    bug.SyncTime = syncDate;
    bug.DatabaseName = "DevDiv Bugs";

    // We will start all DDR bugs as pri2 and if the hitcount > threshold within 
    // specified time window, then we will bump the priority to pri1. This is 
    // true for both deterministic and non-deterministic bugs. For deterministic
    // bugs that start out as "test issue" reaching the hitcount threshold is a 
    // good indication that it is probably a product bug. On the other hand,
    // for non-deterministic bug, reaching the threshold means it is a frequent
    // enough race that it needs to be looked at. 
    bug.Priority = 2;
    bug.Severity = bug.Priority;
    bug.BuildLabel = _formatBuildLabelForBug(syncDate);
    bug.BranchLabel = branchName;
    bug.TestCaseID = testId;
    bug.TestOwner = primaryTestOwner;
    bug.FilesToAttach = failedLogs;
    bug.IsDeterministic = isDeterministic;

        // retrieve test environment from log file names
    var environmentInfo = _parseEnvironmentInfoFromLogfiles(originalLogs);
    var environment = _parseEnvironmentString(environmentInfo.FilingFromTestMachine, environmentInfo.String);

    if (environment != undefined && environment != "")
        bug.Environment = environment;

    bug.FixBy = "Dev10";

    var header = "This bug was filed automatically with relevant smarty logs from the test run attached. " +
    "Please refer to the 'What is provided in the bug' section of the "+
    "<a href=http://devdiv/sites/CLR/Bugs%20and%20DCRs/Bug%20Processes/Daily%20Dev%20Run%20Bug%20FAQ.doc>" +
    "DDR Bug FAQs</a> doc.\n";

    var title = " failure on " + bug.BuildLabel + " for test [" + testName + "]";

    // Trim down title to 255 characters to fit in ProductStudio database
    var MaximumTitleLength = 238; // NOT 255, the PS title max is 238
    var TitleOverflowSuffix = "...";
    if (title.length > MaximumTitleLength)
    {
        title = title.substring(0, MaximumTitleLength - TitleOverflowSuffix.length);
        title = title + TitleOverflowSuffix;
    }

    var investigationRecommendation = 
                          "We are asking you as the owner of the test to do an initial investigation.  " +
                          "In the event that this is a product bug, please assign it to the appropriate developer " +
                          "and alter the \"Issue Type\" field to \"Code Defect\".\n\n" + 
                          "Note: If this is a known issue and you resolve this bug as a duplicate before a fix is checked in, " +
                          "please update the \"TC ID\" field of the active parent bug to: \n\n" +
                          bug.TestCaseID + "\n\n" +
                          "Bugs are created on a per branch basis. To prevent duplication of this issue, set the " +
                          "Opened \"Branch\" to: \n" +
                          branchName + "\n" +
                          "Otherwise we cannot determine that a bug exists for a similar failure of this test " +
                          "and so a future failure in DDR will result in another bug assigned to you.\n\n";

    var failureFrequency;
    var failureFrequencyRecommendation;

    if (isDeterministic) {
        title = "DDR Deterministic" + title;
        failureFrequency = "all";
        failureFrequencyRecommendation =
                          "It could be because of:\n" + 
                          "   a) A product bug\n " + 
                          "      In an ideal world, any product bugs that managed to get through the checkin validation system" +
                          "      will be caught by post-checkin validation systems such as rolling test run or in the BVT lab." +
                          "      But it is possible that either one of the validation system is not online or a DDR run caught " +
                          "      this first.\n\n" + 
                          "   b) A machine/config issue\n" + 
                          "      This usually suggests a problem that the test may need to handle.  The test should either be" +
                          "      updated to handle this case OR you should create an IVT in IVT.lst in the " +
                          "      Infrastructure\\Private\\DDR category.  Those tests are run by verifyMachineState to alert"
                          "      the developer of the issue.\n\n";

        bug.OpenedBy = Env("USERNAME");
    } else {
        title = "DDR Non-Deterministic" + title;
        failureFrequency = "some but not all";
        failureFrequencyRecommendation =
                          "If this bug is assigned to you, then enough of these intermittent failures have occurred " +
                          "in DDR for this test that we believe the problem should be escalated.\n\n";
       bug.OpenedBy = primaryTestOwner;
    }

    bug.Title = title;

    header = header +  "\nThis test has failed " + failureFrequency + " executions in a DDR smarty run on a developer machine, " +
                       "including a \"baseline\" execution on a SNAP build with no local changes.\n\n";

    var resolveInstructions = isDeterministic ? "" : "\nWhen resolving, please assign to "+displayNameLookup(bug.OpenedBy)+".\n\n";
    
    bug.CreateDescription = header + failureFrequencyRecommendation + investigationRecommendation + resolveInstructions;

    // If the host name does not match the logname, then we are filing from a different machine, so user may be wrong
    var username = Env("USERNAME");
    if (!environmentInfo.FilingFromTestMachine)
    {
        username = username + " (machine owner?)";
    }

    var machineDescription ="Test failed on\n" +
                            "    MACHINE: " + environmentInfo.TestMachine + "\n" +
                            "    USER: " + username + "\n" +
                            "    TIME: " + startTimeStr + "\n" +
                            "    ENVIRONMENT: " + environment + "\n\n" +
                            "Smarty logs for all failed executions have been attached to this bug.\n";
    if (!environmentInfo.FilingFromTestMachine)
        machineDescription = "**Bug Filed from non-test machine**\n" + machineDescription;

    bug.UpdateDescription = machineDescription;

    bug.CreateDescription += bug.UpdateDescription;

    var dateStr = (syncDate.getMonth()+1) + "/" + syncDate.getDate() + "/" + syncDate.getYear() + " " + 
                  syncDate.getHours() + ":" + syncDate.getMinutes() + ":" + syncDate.getSeconds();


    WScript.Echo("Filing bug");
    WScript.Echo("    Title = " + bug.Title);
    WScript.Echo("    Priority = " + bug.Priority);
    WScript.Echo("    Severity = " + bug.Severity);
    WScript.Echo("    TestCaseID = " + bug.TestCaseID);
    WScript.Echo("    TestOwner = " + bug.TestOwner);
    WScript.Echo("    Opened By = " + bug.OpenedBy);
    WScript.Echo("    BuildLabel = " + bug.BuildLabel);
    WScript.Echo("    BranchLabel = " + bug.BranchLabel);
    WScript.Echo("    Environment = " + bug.Environment);
    // WScript.Echo("    Description = " + bug.CreateDescription);
    // WScript.Echo("    Update Desc = \n" + bug.UpdateDescription);
    WScript.Echo("    SyncTime = " + dateStr);

    if(failedLogs)
    {
        WScript.Echo("    FilesToAttach: ");

        for (var i = 0; i < failedLogs.length; i++) {
            WScript.Echo("        " + failedLogs[i]);
        }
    }

    WScript.Echo("Creating new Test Defect bug.");
   
    if(createNew == true)
    {
        WScript.Echo("Creating new Test Issue bug.");
        var result = _createNewDDRBug(bug);
    }
    else
    {
        WScript.Echo("Create or Log DDR Bug.");
        var result = _createOrLogDDRBug(bug);
    }
    if(result != null)
        WScript.Echo("Bug: " + result);
    else
        WScript.Echo("Error: Failed to create new bug.");
}
function _formatBuildLabelForBug(syncDate) {
    var str;
    
    str = (syncDate.getYear() - 2000).toString();
    str += padLeft( syncDate.getMonth() + 1, 2, true );
    str += padLeft( syncDate.getDate(), 2, true );
    str += ".00";

    return str;
}


/*****************************************************************************/
/* given a smarty.xml file 'smartyFile, create on that has stripped out
   testcase results for successfull tests.  This makes the file MUCH smaller.
   Place this in a 'err.xml' file */

function _createSmartyErrFile(smartyFile, smartyErrFile) {

    if (smartyErrFile == undefined)
        smartyErrFile = smartyFile.replace(/\.xml$/i, ".err.xml");

    logMsg(LogClrAutomation, LogInfo, "_creatSmartyErrFile: Creating ", smartyErrFile, "\n");

    var smarty = xmlRead(smartyFile);
    var errCases = [];
    var testCases = smarty.TESTRESULT.MYBLOCK.TESTCASES.TESTCASE;
    for(var i = 0; i < testCases.length; i++) {
        var testCase = testCases[i];
        if (!testCase.SUCCEEDED.match(/yes/i))
            errCases.push(testCase);
    }
    if (errCases.length == 0)
        errCases = undefined;
    smarty.TESTRESULT.MYBLOCK.TESTCASES.TESTCASE = errCases;
    xmlWrite(smarty.TESTRESULT, smartyErrFile, "TESTRESULT");
}

/*****************************************************************************/
/* runs trun (a test harness)
     Parameters:
        trunArgs  : extra args to trun (e.g.: "-g DDInt")
        trunDir   : Relative path in the ddSuites tree where trun will be run. (Default: src\fx. For CoreCLR: src\fx\bcl)
        outDir    : Where to place the test results.
        ddsDir    : The base of the ddSuites tree. (Default: %_NTBINDIR%\ddsuites)
        binDir    : Folder for product binaries, with the "suitebin" folder underneath it.
        bldType   : Build flavor (e.g.: dbg, chk, ret. Default: chk)
        bldArch   : Architecture (e.g.: x86, ia64, amd64. Default: %_BuildArch%)
        runtimeVersion : Which version of the installed runtime to use (COMPLUS_DefaultVersion, e.g.: v2.0.x86chk, v2.0.50131)
        suiteBinDir : (optional) If suitebin is not under binDir. Default is binDir\suitebin

    TODO: Vista: We should update this worker to support forking to an elevated window if needed by the test
*/
function trun(trunArgs, trunDir, outDir, ddsDir, binDir, bldType, bldArch, runtimeVersion, suiteBinDir) {

    if (trunArgs == undefined)
        trunArgs = "";
    if (bldType == undefined)
        bldType = "chk";
    if (bldArch == undefined) {
        bldArch = Env("_BuildArch");
        if (!bldArch)
            throw new Error(1, "trun: Required argument bldArch is missing");
    }
    if (outDir == undefined) {
        outDir = ".";
    }

    // The following can't be parsed by "runjs /?" help

    if (ddsDir == undefined) {
        if (!Env("_NTBINDIR"))
            throw new Error(1, "trun: Required argument ddsDir is missing");
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
    }
    if (trunDir == undefined) {
        if (isCoreCLRBuild(bldType)) {
            trunDir = "src\\fx\\bcl";
        }
        else {
            trunDir = "src\\fx";
        }
    }

    logCall(LogClrAutomation, LogInfo, "trun", arguments, " {");

    bldArch = bldArch.toLowerCase();

    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    // We need quotes around -!summarylogs=path or else ddsenv mangles it
    // REVIEW: consider adding '-n' option to the trun arg list to sequence the log files, 
    // This requires some special logic to generate the taskreport correctly for retry tasks
    trunArgs = trunArgs +   " -st -ksr 10 -A -S -T -X \"-!summarylogs=" + outDir + "\"";

    var vsSuites = false;
    if (trunDir.match(/src\\vs/i))
        vsSuites = true;
    
    if (binDir == undefined) {
        if (vsSuites) {
            binDir = ddsDir.substr(0, 2) + "\vs";
        }
        else {
            throw new Error(1, "trun: Required argument binDir is missing");
        }
    }
    ddsBuilt = binDir;
    
    // Where the test binaries are
    if (suiteBinDir == undefined)
        ddsTarget  = binDir + "\\suitebin";
    else
        ddsTarget  = suiteBinDir;

    // The build type (we are really guessing here)
    ddsBldType = bldType;

    // Trun generates a read only schema file which it won't overwrite on a rerun.  This is a hack
    // to delete that file so that retrying a trun task will work.
    if (FSOFileExists(outDir + "\\trunschema.xml"))
        FSODeleteFile(outDir + "\\trunschema.xml", true);

    var runOpts = clrRunTemplate;
    var ndpVer = "";
    if (runtimeVersion != undefined) {
        logCall(LogClrAutomation, LogInfo, "setting runSetEnv(\"COMPLUS_DefaultVersion\", \"", runtimeVersion, "\") to use with runWithRuntimeArch\n");
        runOpts = runSetEnv("COMPLUS_DefaultVersion", runtimeVersion, runOpts);
        ndpVer = " ndpver " + runtimeVersion;
    }

    // If we are running the Layering tests, then ensure that _NT_SYMBOL_PATH points to the
    // right location, based upon the build/architecture...
    var targetPath = trunDir.toLowerCase();
    if (targetPath.indexOf("src\\layering\\tests") >= 0)
    {
        var symbolPath = binDir + "\\Symbols.pri\\retail";
        runOpts = runSetEnv("_NT_SYMBOL_PATH", symbolPath, runOpts);
        logMsg(LogClrAutomation, LogInfo, "setting _NT_SYMBOL_PATH for running layering test to ", symbolPath, "\n");
    }
    
    // Trun tests require ddsEnv to run so we launch it under ddsenv.bat
    // but the exitcode from this cmd process is proving to be very unreliable for trun.exe.
    // Long story short we need to ignore the exitcode from this cmd and instead rely on 
    // postprocess to figure out the pass/fail status of trun
    // 
    
    /* Relevant comments from ddsenv.bat
    REM Exitcode of a process and %errorlevel% are do different entity. The exitcode from the 'exec cmd' doesn't 
    REM always persist in ddsenv.bat's exitcode. This means either every command executed needs
    REM to be batched so that they set the exit code explicitly using 'exit /b' dos cmd
    REM or we do it here. We are hoping that the %errorlevel% right after the return of 'exec cmd' 
    REM captures the exitcode from it. This is true if the cmd were a batch script that sets the exitcode explicitly
    REM using 'exit /b', we will always do the right thing by explicitly setting this batch script's exitcode here 
    REM with that value. However, if the cmd were an exe, I observe varying results for different executables.
    REM For some, the exitcode is reliably relayed in the %errorlevel% upon return but for others, the %errorlevel%
    REM retains the old value. In short, our attempt here is to get it right most of the times. This means that
    REM the exitcode from 'ddsenv.bat exec cmd' can never be relied for down level scripting to work correctly,
    REM You need to explicitly add a post process to figure out the pass/fail status in some cases (for ex, trun.exe)
    REM exit %ERRORLEVEL%
    */
    
    var run = runWithRuntimeArch("pushd " + ddsDir + "\\" + trunDir + " & " +
                    "call " + ddsDir + "\\ddsenv.bat " +
                    " suiteroot " + ddsDir +
                    " built " + ddsBuilt +
                    " suitetarget " + ddsTarget +
                    " flavor " + ddsBldType + " nocolor nostart exec " +
                    "trun " + trunArgs,
        bldArch,
        runSetNoThrow(
        runSetTimeout(5 * HOUR,
        runSetLog(LogRun, LogInfo, 
        runOpts))));
    logMsg(LogClrAutomation, LogInfo, "ddsenv.bat returning ", run.exitCode, "\n");

    if (!FSOFileExists(outDir + "\\trun.sum")) {
        throw new Error(1, "'" + outDir + "\\trun.sum" + "' does not exist, assuming test run did not complete!  Logs in " + outDir);
    }
        
    // Grep the summary file for 'Failed' string match....ugly but works reliably!
    // exitcode of 0 (success) from findstr means, we found a match which means failure of trun
    var run2 = runCmdToLog("findstr.exe /i \"failed\" " + outDir + "\\trun.sum", runSetNoThrow());
    logMsg(LogClrAutomation, LogInfo, "} trun() returning ", !run2.exitCode, "\n");

    if (!run2.exitCode)
        throw new Error(1, "trun failed with errors! logs in " + outDir);

    return !run2.exitCode;
}

/******************************************************************************/
/*********************** RUNNING PERFORMANCE BENCHMARKS ***********************/
/******************************************************************************/

/****************************************************************************/
/* Dump IBC data for a single dll */

function dumpIBCData(installName, dllBase, outDir) {

    var dll = installName + "\\" + dllBase + ".dll";
    var outFile = outDir + "\\" + dllBase + "ibcdump.log";

    runCmd("ibcmerge -d -mi " + dll, runSetOutput(outFile));
}

/****************************************************************************/
/* Dump the IBC data for system Dlls */

function dumpSystemIBCData(installDir, outDir) {
    logMsg(LogClrAutomation, LogInfo, "dumpSystemIBCData('", installDir, "', '", outDir, "') {\n");

    dumpDllIBCData(installDir, "mscorlib", outDir);
    dumpDllIBCData(installDir, "System", outDir);

    logMsg(LogClrAutomation, LogInfo, "} dumpSystemIBCData()\n");
}

/****************************************************************************/
/* Capture a vadump of a particular app,  It runs a command, waits for
   it to stabilize, takes a vadump, and then terminates the app.  It
   also creates a XML version of the output VADump file.

   Parameters:
     command  : The command to run
     outFile  : The name of the file in which to put the vadump data
     runOpts  : optional options to pass to 'runCmd (env vars etc)'
 */
function vaDumpCmd(command, outFile, runOpts) {
    logMsg(LogClrAutomation, LogInfo, "vaDumpCmd('", command, "', '", outFile, "') {\n");

    if (command == undefined)
        throw new Error(-1, "vadumpCmd: undefined parameter command");
    if (outFile == undefined)
        throw new Error(-1, "vadumpCmd: undefined parameter outFile");
    if (runOpts == undefined)
        runOpts = clrRunTemplate;

    var isVista = nsGetOSVersion().slice(0,2) == "6.";

    runOpts = runSetEnv("COMPLUS_SleepOnExit", 20,
              runSetNoThrow(
              runSetLog(LogRun, LogInfo,
              runOpts)));

    var run = runDo(command,runOpts);

    if (runWaitForIdle(run)) {
        logMsg(LogClrAutomation, LogInfo, "} = IDLE\n");

        var pid = procDeepestChild(run.getPid());
        var cmd = ScriptDir + "\\" + Env("PROCESSOR_ARCHITECTURE").toLowerCase();

        if (isVista)
           cmd += "\\vadump.vista.exe";
        else 
           cmd += "\\vadump.exe";

        var options = " -op " + pid;

        logMsg(LogClrAutomation, LogInfo, "RUNNING ", cmd, options, "\n");
        runCmd(cmd + options, runSetNoThrow(runSetOutput(outFile)));
    }
    else
        throw new Error(-1, "Command terminated before it went idle");

    if (!run.done)
        runTerminate(run, true);

    if (!FSOFileExists(outFile))
        throw new Error(-1, "Failure generating vaDump " + outFile);

    logMsg(LogClrAutomation, LogInfo, "} vaDumpCmd()\n");
    return parseVADumpToXml(outFile);
}

/**********************************************************************/
/* parse a vadump file (vadump -o) into a nice parsed form. sutable
   for dumping as XML (xmlWrite) do a 'runjs parseVADump <file>' to
   see the format of the structure it creates.   All numbers are given
   in KB.
 */

function parseVADump(vaDumpFileName) {
    logMsg(LogClrAutomation, LogInfo1000, "parseVADump(", vaDumpFileName, ")\n");
    var data = FSOReadFromFile(vaDumpFileName);
    if (!data.match(/(Cat[ae]gory\s*Total(.|\n)*?)(^Module Working Set(.|\n)*?)(^Heap Working Set)/m))
        throwWithStackTrace(new Error(1, "Could not parse vadump file " + vaDumpFileName));
    var summary = RegExp.$1;
    var modules = RegExp.$3;

    var isVista = nsGetOSVersion().slice(0,2) == "6.";

    var ret = {};
    ret.fileName = vaDumpFileName;
    var total = ret.WS_Total_KB = {};
    var pri   = ret.WS_Private_KB = {};

    while (summary.match(/^\s*(\S+\s*?\w*?\s*?\w*?\s*?\w*?)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*$/m)) {
        var key = RegExp.$1;
        var totalVal = parseInt(RegExp.$3);
        var privateVal = parseInt(RegExp.$4);
        summary = RegExp.rightContext
        key = key.replace(/[\s|/]/g, "_");
        total[key] = totalVal;
        pri[key] = privateVal;
    }

    var mod = total.Modules = {};
    var modPri = pri.Modules = {};

    if (!isVista) {
    while (modules.match(/^\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\S+)\s*$/m)) {
        var key = RegExp.$5.toLowerCase();
        if (mod[key] == undefined)
            mod[key] = modPri[key] = 0;
        mod[key] += RegExp.$1 * 4;
        modPri[key] += RegExp.$2 * 4;
        modules = RegExp.rightContext;
    }
    } else {
        while (modules.match(/^\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s*$/m)) {
            var key = RegExp.$7.toLowerCase();
            key = key.substring(key.lastIndexOf('\\') + 1);
            if (mod[key] == undefined)
                mod[key] = modPri[key] = 0;
            mod[key] += RegExp.$1 * 4;
            modPri[key] += RegExp.$2 * 4;
            modules = RegExp.rightContext;
        }
    }

    return ret;
}

/****************************************************************************/
/* convert a vaDump file into a nice XML file.  also returns the vaDump
   information from the 'vaDumpFileName'.

   Parameters
     vaDumpFileName : VaDump file to convert
     xmlFileName    : Output xml file (defaults to *.xml)
*/

function parseVADumpToXml(vaDumpFileName, xmlFileName) {

    if (xmlFileName == undefined)
        xmlFileName = vaDumpFileName + ".xml";

    var ret = parseVADump(vaDumpFileName);
    logMsg(LogClrAutomation, LogInfo, "parseVADumpToXml: writing VADump information to XML file ", xmlFileName, "\n");
    xmlWrite(ret, xmlFileName, "VADump");
    return ret;
}

/****************************************************************************/
/* Run all working set tests, putting results in outDir.  It ngens the test,
   primes the security cache, if it is a gui app (does not run exit on its 
   own) it runs vadump after it idles.

     Parameters:
     outDir       : The name of the file in which to put the vadump data
     scenarioSpec : Specify the scenarios (defaults to all), see scenariosGet 
                    for more on what this specification is
     srcBase      : Needed to find scenarios 
     runOpts      : optional additional environement to pass to WSPerfTests.
*/

function WSPerfTests(outDir, scenarioSpec, srcBase, runOpts) {

    if (outDir == undefined) 
    throw new Error(1, "Required argument outDir is missing");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase is missing");
    }

    if (runOpts == undefined)
        runOpts = clrRunTemplate;

    logCall(LogClrAutomation, LogInfo, "WSPerfTests", "{");

    var scenarios = scenariosGet(scenarioSpec, srcBase);
    // scenariosShow(scenarioSpec, srcBase);

    var netFrameworkRoot = GetNetFrameworkRoot();
    var sdkRoot = WshShell.RegRead(netFrameworkRoot + "\\SDKInstallRootv2.0");
    if (!sdkRoot)
        throwWithStackTrace(Error(1, "Could not find sdkRoot SDKInstallRootv2.0 registry key"));

    var installDir     = sdkRoot.match(/^(.*)\\sdk\\/)[1];
    var defaultVersion = installDir.match(/^(.*)\\(.*)/)[2];

    logMsg(LogClrAutomation, LogInfo, "Using the installed runtime in ", installDir, "\n");

    var ngenCmd = installDir + "\\ngen.exe";
    
    runOpts = runSetEnv("COMPLUS_DefaultVersion", defaultVersion, runOpts);

    runCmdToLog(ngenCmd + " display", runOpts);

    // Add a twenty minute timeout 
    var commonRunOpts = runSetTimeout(20 * MINUTE, runOpts);

    FSOCreatePath(outDir);
    for (var i = 0; i < scenarios.length; i++) {
        var scenario = scenarios[i];
        if (!scenario.categories.match(/\btest\b/i))
            continue;

        logMsg(LogClrAutomation, LogInfo, "Running Scenario: ", scenario.name, " {\n");

        var outBase = outDir + "\\" + scenario.name;

        // Set the current working directory to the installDir
        var ngenRunOpts = runSetCwd(installDir, commonRunOpts);

        logMsg(LogClrAutomation, LogInfo, "Ngen-ing the explicit system dependencies\n");

        // ngen the explicit scenario dependant system dlls, if specified
        if (scenario.depends) {
            // We will explicitly ngen the dependant system dlls
            for (var k = 0; k < scenario.depends.length; k++) {
                var ngenDependsArgs = "install " + scenario.depends[k] + ".dll " + " /NoDependencies";
                runCmdToLog(ngenCmd + " " + ngenDependsArgs, ngenRunOpts);
            }
        }

        // Set the current working directory to the scenario path
        var scenarioRunOpts = runSetCwd(scenario.path, commonRunOpts);

        logMsg(LogClrAutomation, LogInfo, "Ngen-ing the scenario's exe and dlls\n");

        // ngen the scenario exe and dlls
        for (var j = 0; j < scenario.ngen.length; j++) {
            var ngenScenarioArgs = "install " + scenario.ngen[j];
            // If explicit depends were specified then we add /NoDependencies
            if (scenario.depends) {
                ngenScenarioArgs = ngenScenarioArgs + " /NoDependencies";
            }
            runCmdToLog(ngenCmd + " " + ngenScenarioArgs, scenarioRunOpts);
        }

        var exeOrCmd = scenario.exe;
        if (scenario.cmd)
            exeOrCmd = scenario.cmd;

        // run it once to prep the security cache
        logMsg(LogClrAutomation, LogInfo, "Running ", scenario.name, " to prep security cache\n");
        runToIdleAndQuit(exeOrCmd, scenarioRunOpts);

        var outFile = outBase + ".vadump";
        vaDumpOpts = scenarioRunOpts;
        if (!scenario.gui) {
            vaDumpOpts = runSetEnv("COMPLUS_SleepOnExit", 30,
                         runSetEnv("COMPLUS_StressLog", 1, vaDumpOpts));
        }
        var indent = logTry();
        try {
            vaDumpCmd(exeOrCmd, outFile, vaDumpOpts);
        } catch(e) {
            logCatch(indent);
            if (!e.description.match(/terminated before.*idle/))
                throw e;
            logMsg(LogClrAutomation, LogInfo, "Warning, command ", exeOrCmd, " did not go idle\n");
        }
        logMsg(LogClrAutomation, LogInfo, "} DONE Running Scenario: ", scenario.name, "\n");
    }
    logMsg(LogClrAutomation, LogInfo, "} WSPerfTests()\n");
}

/******************************************************************************/
/********************************* SD RELATED *********************************/
/******************************************************************************/

/*****************************************************************************/
/* Do an integrate in chunks to avoid SD maximums. if 'label' is undefined
   throw an error

   Parameters:
     fromLab     : The name of the "from" branch (e.g. lab21)
     toLab       : The name of the "to" branch   (e.g. private/lab21s)
     fromLabel   : The change list number(s) to integrate from.
     dir         : The sub directories to integrate (e.g. ndp/clr/src)
     sdRoot      : The base of the SD client mapping on the local machine
                 : This argument is needed if you call sdIntegrate from
                 : a dir that is not the client view root.
     sdEnv       : run options (env vars) passed to run (normally null)
     changelist  : optional changelist to put integrations into. If omitted, uses default

*/

function sdIntegrate(fromLab, toLab, fromLabel, dir, sdRoot, sdEnv, changelist) {

        //      there must be a label
    if (fromLabel == undefined) {
        if (!(runCmd("sd counter change", sdEnv).output.match(/^(\d+)/)))
                throw new Error(-1, "Could not get latest sync point counter");
        fromLabel = RegExp.$1;
    }

    logCall(LogClrAutomation, LogInfo10, "sdIntegrate", arguments);
    
    // Support for integrating directly into a changelist.
    var changelistCmd = " ";
    if (changelist != undefined)
    {
        changelistCmd = " -c " + changelist + " ";
    }
    var cmdPrefix = "sd integrate " + changelistCmd + " -t -a ";

    //      create the from and to stubs
    var fromStub = fromLab;
    var toStub =  toLab;
    //      add the directory if it exists
    if (dir != undefined && dir != "")
    {
        fromStub = fromStub+ "/" + dir;
        toStub= toStub+ "/" + dir;
    }
    //      create the origin and destination paths
    //      this syntax is for the top level only. '/...' works for subdirs (see below)
    var     origin = fromStub + "/*";
    var     destination = toStub + "/*";

    WScript.StdOut.WriteLine(""); // for the newline
    WScript.StdOut.WriteLine("******************* INTEGRATING FILES *******************");
    WScript.StdOut.WriteLine(""); // for the newline
    //      integrate the top level of this subtree
    var cmd = cmdPrefix + "//depot/devdiv/" + origin+ "@" + fromLabel+ " //depot/devdiv/" + destination;
    //      let everyone know what is being done
    WScript.StdOut.WriteLine("Running: " + cmd);
    //      do it
    WScript.StdOut.Write(runCmd(cmd, runSetTimeout(4 * HOUR, sdEnv)).output);

    //      integrate the sub directories of this subtree
    //      NOTE: have to feed FSOGetDirPattern a directory
    if (dir == undefined || dir == "") dir = ".";
    if (sdRoot != undefined && sdRoot != "") {
        // If the caller has provided sdRoot, that means the caller is not calling sdIntegrate
        // from the client root dir.
        // Then we need to prepend "sdRoot" to the "dir" so that FSOGetDirPattern won't
        // return 0 surdir.
        dir = sdRoot + "\\" + dir;
    }
    var subDirs = FSOGetDirPattern(dir, /.*/);
    logMsg(LogClrAutomation, LogInfo10, "dir=" + dir + "\n");
    logMsg(LogClrAutomation, LogInfo10, "subDirs.length=" + subDirs.length + "\n");

    for(var i = 0; i < subDirs.length; i++)
    {
        //      get the last part of the full path which is the actual directory
        subDirs[i].match(/\S*\\(\S*)$/);
        directory = RegExp.$1;
        //      reset origin and destination
        origin= fromStub+ "/" + directory + "/...";
        destination= toStub+ "/" + directory + "/...";
        //      do the integration
        cmd = cmdPrefix +"//depot/devdiv/" + origin+ "@" + fromLabel + " //depot/devdiv/" + destination;
        //      let everyone know what is being done
        WScript.StdOut.WriteLine("Running: " + cmd);
        //      do it
        WScript.StdOut.Write(runCmd(cmd, runSetTimeout(4 * HOUR, sdEnv)).output);
    }
    return 0
}


/******************************************************************************/
/* Touch a SD.

   Parameters:
     sdClntName  : The name of the sd client for this sync
     sdPort      : The SD port to connect to the SD server
*/
function sdTouchSD(sdClntName, sdPort) {

    if (sdClntName == undefined)
        sdClntName = Env("COMPUTERNAME");
    if (sdPort == undefined)
        sdPort = "DDRTSD:4000";
    logCall(LogClrAutomation, LogInfo10, "sdTouchSD", arguments);

    var touchCommand = "sd.exe" +
                       " -p " + sdPort +
                       " -c \"" + sdClntName +"\"" +
                       " have \"//\"" + sdClntName + "\"/*\""

    logMsg(LogClrAutomation, LogInfo, "touchCommand: " + touchCommand + "\n");

    // We need a client mapping, Copy the one from clrsnapbackup2.  Note we are bashing the exiting <COMPUTERNAME>_ROLLING client
    runCmdToLog(touchCommand);

    return 0;
}

/******************************************************************************/
/****************************** MISCELLANEOUS *********************************/
/******************************************************************************/

/*****************************************************************************/
/* Post changes from a given VBL enlistment to a Mac Buddy Build Server.

     Parameters:
       srcBase : The base of the VBL to test changes for
       task    : The task to run
       options : Additional options to pass to bbpost
*/
function postMacBuddyBuild(srcBase, task, options)
{
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Missing argument srcBase for postMacBuddyBuild");
    }
    if (task == undefined)
        task = "DailyDevRun_Full";
    if (options == undefined)
        options = "";

    logMsg(LogClrAutomation, LogInfo, "postMacBuddyBuild(", arguments, ") {\n");

    srcBase = FSOGetFolder(srcBase).Path;
    var run = runCmdToLog("perl " + srcBase + "\\rotor\\automation\\bbpost --task=" + task + " " + options,
        runSetTimeout(10 * MINUTE));
    if (run.exitCode != 0)
        throw Error(1, "bbpost failed with exit code " + run.exitCode);
    if (!run.output.match(/(http:\/\/.*)/))
        throw new Error(-1, "Could not parse bbpost output to find task report URL\n");

    var taskReportUrl = RegExp.$1;

    logMsg(LogClrAutomation, LogInfo, "} postMacBuddyBuild() = ", taskReportUrl, "\n");

    return 0;
}

/*****************************************************************************/
/*
    stops and disables all installed NGEN services
    
    This is useful for snap machines, where we don't want any leftovers between runs. 
    We disable as opposed to uninstalling to make more obvious what happened (and get a nice message 
    if somebody tries to start the service).
*/
function disableAllNGENServices() {

    if (!isElevated())
    {
        throw new Error(1, "disableAllNGENServices: Requires executing with elevated privilieges");
    }
    
    var out = runCmd("sc query state= all").output;
    var lines = out.split(/\r*\n/);
    
    for(var i = 0; i < lines.length; i++) {
        
        if (lines[i].match(/.*(clr_optimization.*)/)) {
            var serviceName = RegExp.$1;
            logMsg(LogClrAutomation, LogInfo, "stopping service ", serviceName, "\n");
            runCmdToLog("net stop " + serviceName, runSetNoThrow());
            logMsg(LogClrAutomation, LogInfo, "disabling service ", serviceName, "\n");
            runCmdToLog("sc config " + serviceName + " start= disabled", runSetNoThrow());
        }
    }
}

/*****************************************************************************/
/*
    Adds strong name skip verification entries to the registry.
    
    The function adds skip verification entries to both 32 and 64-bit registry
    locations, from both 32 and 64-bit environments.
    
    Parameters: None
 */

function setSNSkipVerification()
{
    logMsg(LogClrAutomation, LogInfo, "Adding strong name skip verification entries to the registry.\n");

    var keys = [ "31bf3856ad364e35",
                 "7cec85d7bea7798e",
                 "b03f5f7f11d50a3a",
                 "b77a5c561934e089" ];

    var options = runSetTimeout(60, runSetNoThrow());
    var failuresOccured = false;      // We failed to add at least one of the desired entries
    var sysNativeUnavailable = false; // We are in a WOW64 process, but the directory %WINDIR%\Sysnative doesn't exist
    var systemStateChanged = false;   // At least one key we added didn't previously exist
    
    var sys32 =     Env("SystemRoot") + "\\System32\\";
    var sysnative = Env("SystemRoot") + "\\sysnative\\";
    var syswow64 =  Env("SystemRoot") + "\\SysWOW64\\";

    for (var i in keys)
    {
        // First run reg.exe from System32. If we're on 32-bit Windows, this simply does the right thing.
        // On 64-bit Windows, if the script runs natively, it adds the entries to the native hive and if
        // it runs in the WOW, it goes to the Wow6432Node hive. We later make sure we also add the entries
        // to the other hive as well.

        var query = "reg.exe query HKLM\\Software\\Microsoft\\StrongName\\Verification\\*," + keys[i] + " /ve";

        var keyExisted = false;
        var run = runCmd(sys32 + query, options);
        if (run.exitCode == 0)
        {
            keyExisted = true;
        }

        var command = "reg.exe add HKLM\\Software\\Microsoft\\StrongName\\Verification\\*," + keys[i] + " /t REG_SZ /f";

        var run = runCmd(sys32 + command, options);
        if (run.exitCode != 0)
        {
            failuresOccured = true;
        }
        else if (!keyExisted)
        {
            systemStateChanged = true;
        }

        // If we're on a 64-bit box, we need to determine whether we're in the WOW or native
        // and then add the registry entries to the pair registry as well.
        if (Is64bitArch(getRealProcessorArchitecture()))
        {
            keyExisted = false;

            // Are we in the WOW?
            if (Env("PROCESSOR_ARCHITEW6432"))
            {
                // We're in a 32-bit process.
                // Set the 64-bit skip verifications entries
                // This code only works on Vista and newer, which is ok since all devs are supposed
                // to be on Vista and newer OS-es anyway. Everyone else can simply run this command
                // from a 64-bit command prompt.
                if (FSOFileExists(sysnative + "reg.exe"))
                {
                    run = runCmd(sysnative + query, options);
                    if (run.exitCode == 0)
                    {
                        keyExisted = true;
                    }

                    run = runCmd(sysnative + command, options);
                    if (run.exitCode != 0)
                    {
                        failuresOccured = true;
                    }
                    else if (!keyExisted)
                    {
                        systemStateChanged = true;
                    }
                }
                else
                {
                    sysNativeUnavailable = true;
                }
            }
            else
            {
                // We're in a 64-bit process.
                // Set the WOW skip verification entries
                run = runCmd(syswow64 + query, options);
                if (run.exitCode == 0)
                {
                    keyExisted = true;
                }

                run = runCmd(syswow64 + command, options);
                if (run.exitCode != 0)
                {
                    failuresOccured = true;
                }
                else if (!keyExisted)
                {
                    systemStateChanged = true;
                }
            }
            
        }
    }
    
    if (failuresOccured)
    {
        logMsg(LogClrAutomation, LogWarn, "Unable to add strong name skip verification entries to the registry.\n");
        logMsg(LogClrAutomation, LogWarn, "Make sure you have write access to HKLM\\Software\\Microsoft\\StrongName\\Verification\\.\n");
        return 1;
    }
    
    if (sysNativeUnavailable)
    {
        logMsg(LogClrAutomation, LogWarn, "Setting native 64-bit registry keys from a 32-bit process only works on Vista and newer OS-es.\n");
        logMsg(LogClrAutomation, LogWarn, "You need to run this command from a 64-bit process to add the skip verification entries in both places in the registry.\n");
        return 1;
    }
    
    if (systemStateChanged)
    {
        logMsg(LogClrAutomation, LogInfo, "Strong name skip verification entries were successfully added.\n");
        logMsg(LogClrAutomation, LogInfo, "Test signed FX binaries will be allowed to load.\n");
    }

    return 0;
}

/*****************************************************************************/
/*
    Create shortcuts for ClrEnv windows.  This is called automatically
    by a hook in fbenlist after the initial sync, before the big sync.
*/

function _fbEnlistCommands(root)
{
    setSNSkipVerification();
    
    createCLRShortcuts(root, true);

    return 0;
}

/*****************************************************************************/
/* Set up a brand new CLR developer enlistment for Dev10 (TFS), assuming nothing is on
   the current machine.  This basically means bootstrap syncing
   the source base.

   Finally, runjs is available from CLRMain so you can set up a CLR Environment
   using the command 

      \\CLRMain\tools\runjs setupCLRDevTFS

   Parameters:  None
*/

function setupCLRDevTFS() {
    if (!isElevated()) {
        logMsg(LogClrAutomation, LogInfo, "Error - must be run with Administrator permissions.\n");
        logMsg(LogClrAutomation, LogInfo, "Launch an elevated window and retry the command.\n");
        return 1;
    }

    //clean up any old install log
    var logFilePath = FSOGetTempDir() + "\\fbenlist.log";
    FSOTryDeleteFile(logFilePath, true);

    logMsg(LogClrAutomation, LogInfo, "*********************************************\n");
    logMsg(LogClrAutomation, LogInfo, "Launching 'fb enlist' to enlist in a TFS branch.\n");
    logMsg(LogClrAutomation, LogInfo, "A series of settings dialogs will appear.\n");
    logMsg(LogClrAutomation, LogInfo, "Change 'Type' to 'Product Unit' and select the branch named 'clr'\n");
    logMsg(LogClrAutomation, LogInfo, "to select the 'clr' Product Unit branch.\n");
    logMsg(LogClrAutomation, LogInfo, "For the 'Client Mapping' setting, choose Normal Client. \n");
    logMsg(LogClrAutomation, LogInfo, "Uncheck the 'Create desktop shortcut' box \n");
    logMsg(LogClrAutomation, LogInfo, "to avoid creating a Razzle shortcut.\n");
    logMsg(LogClrAutomation, LogInfo, "Fb is based on MSBuild.\n");
    logMsg(LogClrAutomation, LogInfo, "*********************************************\n");
    logCall(LogClrAutomation, LogInfo10, "setupCLRDevTFS", arguments);

    // cleanup so we don't see a spew of deletions from fbenlist.cmd
    var tempFbFolder = FSOGetTempDir() + "\\fb";
    if (FSOFolderExists(tempFbFolder))
        FSODeleteFolder(tempFbFolder, true);

    // set skip SN verification keys
    setSNSkipVerification();

    // expand out env variable
    var command = "cmd /k \\\\ddfiles\\fbtools\\fbenlist.cmd";

    logMsg(LogClrAutomation, LogInfo, "enlist command: " + command + "\n");

    var options = runSetTimeout(24 * 60 * 60);  // don't want to timeout on long sync
    options = runSetInput("Don't pause waiting for a keypress at end.", options);  // workaround for call to 'pause' that hangs 
    var run = runCmdToLog(command, options);
    logMsg(LogClrAutomation, LogInfo, "fbenlist returned: "+run.exitCode +"\n");
    if (run.exitCode == 0) {
        // success 
        var root = _getEnlistDirFromLog();
        if (root) {
            logMsg(LogClrAutomation, LogInfo, "Creating a shortcut to the CLREnv command environment\n");
            createCLRShortcuts(root, true);

            WScript.Echo("");
            WScript.Echo("");
            WScript.Echo("Your developer environment has been successfully set up.");
            WScript.Echo("You will want to run commands in a 'CLREnv' environment window.  ");
            WScript.Echo("You can do this by running " + root + "\\ndp\\clr\\bin\\clrEnv.bat");
            WScript.Echo("A shortcut that creates such a window has been created on the desktop");
            WScript.Echo("");
            WScript.Echo("");
            WScript.Echo("You should strongly consider configuring an automated build/test system");
            WScript.Echo("called daily dev run to run every night.   To do this you need only run");
            WScript.Echo("the command.");
            WScript.Echo("");
            WScript.Echo("    runjs installDailyDevRun");
            WScript.Echo("");
            WScript.Echo("For more information do 'runjs /? installDailyDevRun' and see");
            WScript.Echo("http://teams/sites/clrdev/Documents/clr automation users guide.doc");
            WScript.Echo("");
        }
        else {
            logMsg(LogClrAutomation, LogInfo, "Couldn't determine the enlistment root from the fbenlist.log file\n");
        } 
    }

    return run.exitCode;
}

/* 
    Search through the log file created by fbenlist to find enlistment root
    */
function _getEnlistDirFromLog()
{
    var logFilePath = FSOGetTempDir() + "\\fbenlist.log";

    var logFile = FSOReadFromFile(logFilePath);
    if (logFile) {
        if (logFile.match(/Enlist Directory:\s+(\S+)/)) {
           var dir = RegExp.$1;
           return dir;
        }
    }
    return null;
}


/*****************************************************************************/
/* Set up a brand new CLR developer enlistment, assuming that nothing is on
   the current machine.  This basically means bootstrap syncing
   the source base.    The defaults are such that using it with no args
   is the correct thing to do for a 'standard' enlistment.  Note that this
   routine can be run again without ill effect (it will sync your enlistment), 
   and will be very fast the second time around.   It will also NOT destroy
   any client mappings you already have, so it is safe to use even if you
   already have a client defined.

   Finally, runjs is available from CLRMain so you can set up a CLR Environment
   using the command 

      \\CLRMain\tools\runjs setupCLRDev c:\vbl

   By passing extra parameters, however you can set up enlistments in other 
   useful ways 

     runjs setupCLRDev c:\vbl devdiv/pu/clr                // same as default
     runjs setupCLRDev c:\vbl devdiv/releases/whidbey/redbits redbits
     runjs setupCLRDev c:\vbl devdiv/orcas/pu/clrgre clrgre
     runjs setupCLRDev c:\vbl devdiv/releases/whidbey/qfe qfe
     runjs setupCLRDev c:\vbl devdiv/feature/Silverlight_W2_CLR silverLightW2
     runjs setupCLRDev c:\vbl devdiv/private/whidbey/netfxsp_clr netfxsp_clr

   In particular, running all the commands above allow you to set up multple
   labs in one c:\vbl hierarchy.

   If you want separate enlistments to the same depot you can do that too
   but you have to specify a non-default client name.  For example

      setupCLRDev c:\vbl1 devdiv/pu/clr puclr %COMPUTERNAME%_2

   Parameters:
     sdRoot      : Root dir on the local machine (eg. c:\vbl).
     depotRoot   : Root of lab in the depot for example.  These should NOT
                   include the //depot prefix, for example 
                      devdiv/private/Lab21S
                      devdiv/pu/clr
     localBranch : The name of the local branch under the root.  Defaults
                   to puclr if depotRoot is the default devdiv/pu/clr
     sdClntName  : The name of the sd client for this sync (defaults to machine)
     sdPort      : The machine and port number of the SD depot to connect to.
*/

function setupCLRDev(sdRoot, depotRoot, localBranch, sdClntName, sdPort) {

    if (sdRoot == undefined) {
        sdRoot = "c:\\vbl";
        logMsg(LogClrAutomation, LogInfo, "*** A CLR enlistment will be created on ", sdRoot, "\n");
        logMsg(LogClrAutomation, LogInfo, "*** quit now if it should be placed somewhere else.\n");
        WScript.Sleep(5000);
    }
    if (depotRoot == undefined)
        depotRoot = "devdiv/pu/clr";
    if (localBranch == undefined) {
        if (depotRoot == "devdiv/pu/clr")
            localBranch = "puclr";
        else
            throw new Error(1, "Required argument localBranch not given");
    }
    if (sdClntName == undefined)
        sdClntName = Env("COMPUTERNAME");
    if (sdPort == undefined)
        sdPort = "DDRTSD:4000";

    depotRoot = "//depot/" + depotRoot;        // put the depot prefix on 
    logCall(LogClrAutomation, LogInfo10, "setupCLRDev", arguments);

        // Get the amount of free space from the drive.  If we don't have 10 Gig
        // fail, because the user is probably putting it in a bad place
    FSOCreatePath(sdRoot);
    var folder = FSOGetFolder(sdRoot);
    var drive = folder.Drive;
    var gigSpace = Math.floor(drive.FreeSpace / 1E9);
    if (gigSpace < 10) {
        logMsg(LogClrAutomation, LogError, "A CLR enlistment needs at least 10 Gig of free space, however\n")
        logMsg(LogClrAutomation, LogError, "there is only ", gigSpace, " Gig on the drive for ", sdRoot, ".\n\n");
        logMsg(LogClrAutomation, LogError, "Use setupClrDev <sdRoot> to change root to a location that has\n");
        logMsg(LogClrAutomation, LogError, "sufficient disk space\n");
        return 1;
    }

        // we put sd.exe on the path so that it will work from an absolutely clean machine
        // we run the SD commands from the sdRoot directory so that the SD.INI file gets used to find the depot
    var sdEnv = runSetCwd(sdRoot);
    sdEnv = runSetEnv("PATH", "\\\\clrmain\\public\\tools\\x86" + ";%PATH%", sdEnv);

    // set skip SN verification keys
    setSNSkipVerification();

        // create the client and the mappings for a CLR dev
    sdClientCreate(sdPort, sdRoot, sdClntName, sdEnv);
    sdClientMap(sdRoot, depotRoot + "/public", localBranch + "\\public", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/tools", localBranch + "\\tools", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/developer/" + Env("USERNAME"), localBranch + "\\developer\\" + Env("USERNAME"), sdEnv);
    sdClientMap(sdRoot, depotRoot + "/rotor", localBranch + "\\rotor", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/setupauthoring", localBranch + "\\setupauthoring", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/ndp", localBranch + "\\ndp", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/OptimizationData", localBranch + "\\OptimizationData", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/suitesrc", localBranch + "\\suitesrc", sdEnv);
    sdClientMap(sdRoot, depotRoot + "/ddsuites", localBranch + "\\ddsuites", sdEnv);
    sdClientMap(sdRoot, "//depot/dev", "dev", sdEnv);

        // Do a forced sync if the lab path does not exist, or has nothing in it.
        // That way even if the client exists, but the local files do not (wiped machine)
        // sdSync we will pull in the files.
    var labPath = sdRoot + "\\" + localBranch;
    var forceSync = undefined;
    if (!FSOFolderExists(labPath) || FSOGetDirPattern(labPath).length == 0)
        var forceSync = true;

        // These syncs redundant, however it allows people to start using the CLRENV.
        // since ddsuites takes forever to sync up 
    logMsg(LogClrAutomation, LogInfo, "Syncing high priority directories\n");
    var procArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    
        
    var run = runCmd("sd.exe sync \"" + sdRoot + "\\" + localBranch + "\\ndp\\clr\\bin\\scriptlib\\...\"", sdEnv);
    if (run.output.match(/no permission to protected namespace/))
    {
        logMsg(LogClrAutomation, LogInfo, "\n");
        logMsg(LogClrAutomation, LogInfo, "*********************************************************\n")
        logMsg(LogClrAutomation, LogInfo, "**** Error: permission problem with the depot.\n");
        logMsg(LogClrAutomation, LogInfo, "**** Have you gotten permission at http://ramweb?\n");
        logMsg(LogClrAutomation, LogInfo, "**** Retry after getting permission.\n");
        logMsg(LogClrAutomation, LogInfo, "*********************************************************\n")
        logMsg(LogClrAutomation, LogInfo, "\n");
        return 1;
    }

    sdSync(sdRoot + "\\" + localBranch + "\\ndp\\clr\\bin", undefined, sdEnv, 1);
    sdSync(sdRoot + "\\" + localBranch + "\\tools\\" + procArch, undefined, sdEnv, 1);
    sdSync(sdRoot + "\\" + localBranch + "\\ddsuites\\tools\\" + procArch, undefined, sdEnv, 1);
    sdSync(sdRoot + "\\" + localBranch + "\\ndp\\clr\\src", undefined, sdEnv, 1);
    sdSync(sdRoot + "\\dev", undefined, sdEnv, 1);

    logMsg(LogClrAutomation, LogInfo, "Creating a shortcut to the CLREnv command environment\n");
    var root = sdRoot + "\\" + localBranch;
    createCLRShortcuts(root, false);

    logMsg(LogClrAutomation, LogInfo, "Syncing rest of code and tests.  This will take a while.\n");
    logMsg(LogClrAutomation, LogInfo, "In the mean time, however, the most critical commands and CLR source have been\n");
    logMsg(LogClrAutomation, LogInfo, "fetched.   The CLREnv shortcut can be used and you can begin browsing documentation\n");
    logMsg(LogClrAutomation, LogInfo, "(e.g. in ", sdRoot, "\\dev\\docs), as well as the source.\n");

        // sync up the main source 
    logMsg(LogClrAutomation, LogInfo, "\n");
    sdSync(sdRoot + "\\" + localBranch, undefined, sdEnv, 1, forceSync);
    logMsg(LogClrAutomation, LogInfo, "Done!\n")

    WScript.Echo("");
    WScript.Echo("Your Developer environment has been successfully set up in " + root);
    WScript.Echo("You will want to run commands in a 'CLREnv' environment window.  ");
    WScript.Echo("You can do this by running " + root + "\\ndp\\clr\\bin\\clrEnv.bat");
    WScript.Echo("A shortcut that creates such a window has been created on the desktop");
    WScript.Echo("");
    WScript.Echo("You should strongly consider configuring an automated build/test system");
    WScript.Echo("called daily dev run to run every night.   To do this you need only run");
    WScript.Echo("the command.");
    WScript.Echo("");
    WScript.Echo("    runjs installDailyDevRun");
    WScript.Echo("");
    WScript.Echo("For more information do 'runjs /? installDailyDevRun' and see");
    WScript.Echo("http://teams/sites/clrdev/Documents/clr automation users guide.doc");
    return 0;
}

/*****************************************************************************/
/* Create useful desktop and IE shortcuts that a CLR developer should have.

   To create just CLREnv shortcuts, use either createCLRShortcutSet or createCLRShortcut.

     Parameters:
       srcBase : the enlistment base
       inTfs   : a boolean indicating whether this is a TFS enlistment.
*/

function createCLRShortcuts(srcBase, inTfs) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase for the createCLRShortcuts method is missing");
    }
    if (inTfs == undefined) {
        inTfs = true;
    }

    createCLRShortcutSet("CHK", srcBase, inTfs);

    logMsg(LogClrAutomation, LogInfo, "Creating useful CLR specific IE Favorites\n");
    makeFavorite("CLR New Dev Page", "http://mswikis/clr/dev/Pages/New%20Developer.aspx");
    makeFavorite("CLR Dev Home Page", "http://devdiv/sites/clr/dev/default.aspx");
    makeFavorite("CLR SNAP Checkin Reports", "file:////CLRMain/public/drops/tfs_puclr/snap/JobHistory_tfs_puclr.html");
    makeFavorite("CLR Dev Wiki Home Page", "http://mswikis/clr/dev/Pages/Home.aspx");
    makeFavorite("CLR Dev Visual Studio Page", "http://wiki/default.aspx/Microsoft.Projects.CLRDev/VisualStudio.html");

    var strDesktop = WshShell.SpecialFolders("Desktop");
    logMsg(LogClrAutomation, LogInfo, "Creating shortcut to clr solution\n");
    var oShellLink = WshShell.CreateShortcut(strDesktop + "\\clr.sln.lnk")
    oShellLink.TargetPath = srcBase + "\\ndp\\clr\\src\\clr.sln"
    oShellLink.WorkingDirectory = srcBase + "\\ndp\\clr\\src\\vm"
    oShellLink.Save();

    logMsg(LogClrAutomation, LogInfo, "Creating automation directory...");
    var automationDir = srcBase + "\\automation"
    FSOCreatePath(automationDir);

    if (isElevated())
    {
        logMsg(LogClrAutomation, LogInfo, "and publishing it\n");

        // TODO can remove after a time, allows multiple labs 
        runCmd("net share automation /delete", runSetNoThrow());    // remove old style ones
        FSODeleteFile(Env("HOMEDRIVE") + Env("HOMEPATH") + "\\Favorites\\DailyDevRun Report.url");
    
        var lab = displayName(srcBase, inTfs);
        runCmd("net share automation-" + lab + " /delete", runSetNoThrow());
        runCmdToLog("net share automation-"+ lab +"=" + automationDir + " /UNLIMITED");
        makeFavorite("DailyDevRun Report " + lab, "file://" + Env("COMPUTERNAME") + "/automation-" + lab + "/run.current/taskReport.html");
    }
    else {
        logMsg(LogClrAutomation, LogInfo, "Running unelevated - rerun elevated to publish automation directories\n");
    }

}

/* 
   Get the interesting part of the path for use in shortcuts
*/
function displayName(srcBase, tfs) {
    var display = null;
    // Find the "name" part of c:\dd\name\src 
    // or the "name" in c:\vbl\name 
    srcBase.match(/.*\\(.*)\\(.*)/);
    if ("src" == RegExp.$2 || null == RegExp.$2)
        display = RegExp.$1;
    else
        display =RegExp.$2;
    return display;
}


/*****************************************************************************/
/* Creates a set of ClrEnv command environment shortcuts of type 'kind' (Kind can be
   any args passed to clrenv, e.g., 'ia64 chk'.  'srcBase' is the base of the
   source tree (e.g., c:\vbl\clr). A set of shortcuts includes desktop CLR and CoreCLR,
   and on Vista, elevated and non-elevated.
*/
function createCLRShortcutSet(kind, srcBase, tfs) {

    if (kind == undefined)
        kind = "CHK";
    if (srcBase == undefined)
        srcBase = "c:\\vbl\\clr";
    if (tfs == undefined)
        tfs = true;

    logMsg(LogClrAutomation, LogInfo, "Creating " + kind + " shortcuts to the CLREnv command environment\n");
    createCLRShortcut(kind, srcBase, false, false, tfs); //non-elevated desktop
    createCLRShortcut(kind, srcBase, false, true, tfs); //non-elevated CoreCLR
    if (IsWinLHOrLater()) {
        logMsg(LogClrAutomation, LogInfo, "Creating [Elevated] " + kind + " shortcuts to the CLREnv command environment\n");
        createCLRShortcut(kind, srcBase, true, false, tfs); // elevated desktop
        createCLRShortcut(kind, srcBase, true, true, tfs); // elevated coreclr
    }

    return 0;
}


/*****************************************************************************/
/* Creates a ClrEnv command environment shortcut of type 'kind' (Kind can be
   any args passed to clrenv, e.g., 'ia64 chk'.  'srcBase' is the base of the
   source tree (e.g., c:\vbl\clr).
*/
function createCLRShortcut(kind, srcBase, elevated, isCoreClr, tfs) {

    if (kind == undefined)
        kind = "CHK";
    if (srcBase == undefined)
        srcBase = "c:\\vbl\\clr";
    if (elevated == undefined)
        elevated = false;
    if (isCoreClr == undefined)
        isCoreClr = false;
    if (tfs == undefined)
        tfs = true;

    if (isCoreClr)
    {
        kind = kind + " CoreCLR";
    }
    
    var display = displayName(srcBase, tfs);
    
    var computer = Env("COMPUTERNAME");

    if (elevated && !IsWinLHOrLater())
        throw new Error(-1, "non-Vista host OS detected for Vista-shortcut creation");

    // Vista doesn't support Itanium, so we don't expect Vista to return ia64
    if (IsWinLH() && "ia64" == getRealProcessorArchitecture().toLowerCase())
        throw new Error(-1, "Vista on Itanium is not supported");

    // Windows 7 doesn't support Itanium, so we don't expect Windows 7 to return ia64
    if (IsWin7() && "ia64" == getRealProcessorArchitecture().toLowerCase())
        throw new Error(-1, "Windows 7 on Itanium is not supported");

    var filename = computer + " " + kind + " " + display + " ClrEnv.lnk";
    if (elevated)
        filename = "[ELEVATED] " + filename;
    desktopCmdShortCut(filename,
                        "/k " + srcBase + "\\ndp\\clr\\bin\\clrenv.bat " + kind,
                        srcBase + "\\ndp\\clr\\src",
                        kind + " CLREnv environment for " + srcBase);

    var strDesktop = WshShell.SpecialFolders("Desktop");
    if (elevated) {
        makeAdminShortcut(srcBase, strDesktop + "\\" + filename);
    }

    if (FSOFolderExists(Env("SystemRoot") + "\\SysWow64")) {
        filename = computer + " " + "32Bit " + kind + " " + display + " ClrEnv.lnk";
        if (elevated)
            filename = "[ELEVATED] " + filename;
        desktopCmdShortCut(filename,
                        "/k " + srcBase + "\\ndp\\clr\\bin\\clrenv.bat " + kind,
                        srcBase + "\\ndp\\clr\\src",
                        "32bit " + kind + " CLREnv environment for " + srcBase, "sysWow64");
        if (elevated) {
            makeAdminShortcut(srcBase, strDesktop + "\\" + filename);
        }
    }
    return 0;
}

function makeAdminShortcut(srcBase, filename) {
    var cmdExe = srcBase + "\\ndp\\clr\\bin\\makeRunAs.exe";
    runCmd(cmdExe + " \"" + filename + "\"", runSetNoThrow());
}

/*****************************************************************************/
/* fast, incremental, robust to network failure, deep copy of on directory
   to another.  Pass the /PURGE option if you want things that dont exist
   in the source to be deleted from the target

     Parameters:
       srcDir  : The source directory
       dstDir  : The destination directory
       options : additional options
       logFile : The log file for the copy. If not present log is Stdout
       doSync  : Controls if we sync the time on the local machine
                 (requires elevation on Vista). Default is true.
       recursive : Controls whether robocopy should be called with the 
                   "/S" option. Default is true.
*/
function robocopy(srcDir, dstDir, options, logFile, doSync, recursive) {

    if (srcDir == undefined)
        throw Error(1, "Required parameter srcDir not present");
    if (dstDir == undefined)
        throw Error(1, "Required parameter dstDir not present");
    if (options == undefined)
        options = "";
    if (doSync == undefined)
        doSync = true;
    if (recursive == undefined)
        recursive = true;

    var robocopyPath = Env("SystemRoot") + "\\System32\\robocopy.exe"
    if (!FSOFileExists(robocopyPath)) robocopyPath = srcBaseFromScript() + "\\tools\\" + Env("PROCESSOR_ARCHITECTURE") + "\\robocopy.exe"
    if (!FSOFileExists(robocopyPath)) {
        var errMessage = "roboCopy failed because " + robocopyPath + " does not exist";
        if (Env("PROCESSOR_ARCHITECTURE").toLowerCase() == "amd64") {
            robocopyPath = srcBaseFromScript() + "\\tools\\" + "\\x86\\robocopy.exe";
            if(!FSOFileExists(robocopyPath))
                errMessage = errMessage + ", nor does " + robocopyPath + "exist";
            else
                errMessage = undefined;
        }
        if (errMessage != undefined)
            throw new Error(errMessage);
    }

    logCall(LogClrAutomation, LogInfo10, "robocopy", arguments, "{");
    logMsg(LogClrAutomation, LogInfo, "robocopy: ", srcDir, " -> ", dstDir, " ", options, "\n");

    FSOCreatePath(dstDir);

    if (doSync) {
        _syncTime();
    }

    options += " /R:50 /W:10 /NP ";

    if (recursive)
        options += "/S ";

    var cmd = robocopyPath + " \"" + srcDir + "\" \"" + dstDir + "\" " + options;
    var runOpts = runSetTimeout(3 * HOUR, runSetNoThrow());
    if (logFile) {
        cmd += "/LOG:\"" + logFile + "\" ";
        runOpts = runSetOutput(undefined, runOpts);
        FSOCreatePathForFile(logFile);
    }
    else 
        runOpts = runSetLog(LogClrAutomation, LogInfo, runOpts);

    var run = runCmd(cmd, runOpts);
        // robocopy's exitCode give detailed status but I don't know what they mean.
        // Thus as I find code that seem to be benign, I add them to the list to ignore
    
    if (run.exitCode >= 4 && run.exitCode != 16)
        throw new Error(-1, "robocopy failed with error code " + run.exitCode + " log " + logFile);

    logMsg(LogClrAutomation, LogInfo10, "} robocopy\n");
    return 0;
}


/*****************************************************************************/
/* fast, incremental, robust to network failure, copy of files from one directory
   to another.  Pass the /PURGE option if you want things that dont exist
   in the source to be deleted from the target

     Parameters:
       srcDir   : The source directory
       dstDir   : The destination directory
       options  : additional options
       logFile  : The log file for the copy. If not present log is Stdout
       fileMask : The wildcard
       doSync   : Controls if we sync the time on the local machine
                  (requires elevation on Vista)

*/
function robocopyFiles(srcDir, dstDir, options, logFile, fileMask, doSync) {

    if (srcDir == undefined)
        throw Error(1, "Required parameter srcDir not present");
    if (dstDir == undefined)
        throw Error(1, "Required parameter dstDir not present");
    if (options == undefined)
        options = "";
    if (fileMask == undefined)
        fileMask = "*.*";
    if (doSync == undefined)
        doSync = true;
    else 
        doSync = false;

    logCall(LogClrAutomation, LogInfo10, "robocopyFiles", arguments, "{");
    if (fileMask == "*.*") {
        logMsg(LogClrAutomation, LogInfo, "robocopy: ", srcDir, " -> ", dstDir, "\n");
    } else {
        logMsg(LogClrAutomation, LogInfo, "robocopy: ", srcDir, "\\" + fileMask + " -> ", dstDir, "\n");
    }
    FSOCreatePath(dstDir);
    if (doSync) {
        _syncTime();
    }

    var cmd = ScriptDir + "\\robocopy \"" + srcDir + "\" \"" + dstDir + "\" " + fileMask + " /R:50 /W:10 /NP ";
    var runOpts = runSetTimeout(3 * HOUR, runSetNoThrow());
    if (logFile) {
        cmd += "/LOG:" + logFile + " ";
        runOpts = runSetOutput(undefined, runOpts);
    }
    else 
        runOpts = runSetLog(LogClrAutomation, LogInfo, runOpts);
    
    cmd += options;

    var run = runCmd(cmd, runOpts);
        // robocopy's exitCode give detailed status but I don't know what they mean.
        // Thus as I find code that seem to be benign, I add them to the list to ignore
    if (run.exitCode >= 4 && run.exitCode != 16)
        throw new Error(-1, "robocopy failed with error code " + run.exitCode + " log " + logFile);

    logMsg(LogClrAutomation, LogInfo10, "} robocopyFiles\n");
    return 0;
}

/*****************************************************************************/
/* delete all directories under baseDir that match the regular exp dirPatStr

     Parameters:
       baseDir  : The source directory
       dirPatStr: The pattern to delete (obj.*)
*/
function delDirs(baseDir, dirPatStr, rec) {

    if (baseDir == undefined)
        baseDir = ".";
    if (dirPatStr == undefined)
        dirPatStr = "^obj[0-9]?[a-z]?";
    logCall(LogClrAutomation, LogInfo10, "delDirs", arguments);

    _forDirPattern(baseDir, new RegExp(dirPatStr), function(dir) {
        logMsg(LogClrAutomation, LogInfo, "Deleting: ", dir, "\n");
        FSODeleteFolder(dir, true);
    });
}

/*****************************************************************************/
/* recursive directory search doing 'ftn' for every directory matching 'pat' */

function _forDirPattern(baseDir, pat, ftn) {
    logMsg(LogClrAutomation, LogInfo1000, "_forDirPattern(", baseDir,", dirPat) {\n");

    var dirs = FSOGetDirPattern(baseDir);
    for (var i = 0; i < dirs.length; i++) {
        var dir = dirs[i];
        var suffix = dir.match(/([^\\]*)$/)[1]; // get the last component
        if (suffix.match(pat)) {
            ftn(dir)
        }
        else
            _forDirPattern(dir, pat, ftn);
    }
    logMsg(LogClrAutomation, LogInfo1000, "} _forDirPattern()\n");
}

/*****************************************************************************/
/* Because robocopy uses timestamps it is important that the machines be
   closely synced in time.  Insure this by syncing to a time server */

var _timeSynced = undefined;

function _syncTime() {
    if (_timeSynced == undefined) {
        var sources = [
            "/domain:redmond",
            "\\\\clrsnap0",
            ""     // default time domain
            ];

        // Retry several times to guard against flaky servers
        for(var i = 0; ; i++) {
            var run = runCmd("net time " + sources[i%sources.length] + " /y /set", runSetNoThrow());
            if (run.exitCode == 0) {
               break;
            }
            logMsg(LogClrAutomation, LogInfo, "_syncTime(): FAILED with error code ", run.exitCode, " with server ", sources[i%sources.length], "\n");
            if (i == 100) {
                //TODO:FIX THIS!  net time is failing almost all jobs in snap.
                //throw Error(1, "Failed to sync time too many times - giving up");
                break;
            }
            WScript.Sleep(100);
        }
        _timeSynced = true;
    }
}

/*****************************************************************************/
/* Does a revert of all files, sd sync and then dailyDevRun.

   LET ME REPEAT THAT:  IT REVERTS ALL YOUR CHANGES!!

   Parameters:
      quiet - if not defined, it will prompt before continuing with the revert.
*/
function revertAndSyncAndRunDailyDevRun(quiet)
{
    var srcBase = Env("_NTBINDIR");
    if (!srcBase)
    {
        throw new Error(1, "Required _NTBINDIR environment variable is missing");
    }

    if (quiet == undefined)
    {
        //
        // Prompt before continuing
        //
        WScript.Echo("WARNING!  You are about to revert all your changes!");
        WScript.Echo("   Ctrl-C to exit, else <RETURN> to continue");
        WScript.StdIn.ReadLine();
    }

    //
    // Revert all files
    //
    var run = runCmdToLog("pushd " + srcBase + " & sd revert ...");

    if (run.exitCode != 0)
    {
        throw new Error(1, "revertDdsyncBuildAndDailyDevRun: revert returned non-zero exit code.");
    }

    //
    // Sync
    //
    sdSync(srcBase);

    //
    // DailyDevRun
    //
    return doRun("dailyDevRun");
}

/*****************************************************************************/
/* Loops infinitely repeating a command

   Parameters:
      arg1 - Command to run.
      arg2 thru argN - Arguments to pass as parameters to the command.

*/
function loop()
{
    var cmdString = "";
    var numArgs = arguments.length;
    var i;

    for (i = 0; i < numArgs; i++)
    {
        cmdString += arguments[i] + " ";
    }

    while (1)
    {
        try {
            runCmdToLog("runjs " + cmdString, runSetTimeout(10 * HOUR));
        } catch(err) {
        }
    }
}

/*****************************************************************************/
/* copy a Lab21 Build to cache used by clrsetup */

function copyDropForClrSetup(buildNum) {

    var dirs = ["x86fre", "x86chk", "x86ret", "amd64fre", "ia64fre", "sources"];
    var srcDirBase= "\\\\cpvsbuild\\drops\\whidbey\\Lab21\\raw\\" + buildNum;
    var destDirBase= "\\\\clrmain\\public\\drops\\whidbey\\builds\\ForClrSetup\\" + buildNum;

    if (FSOFolderExists(srcDir))
        throw Error(1, "Could not find source directory " + srcDir);

    for(var i = 0; i < dirs.length; i++) {
        var srcDir= srcDirBase + "\\" + dirs[i];
        var destDir = destDirBase + "\\" + dirs[i];
        logMsg(LogScript, LogInfo, "Copy ", srcDir, " -> ", destDir, "\n");
        FSOCreatePath(destDir);
        try {
            robocopy(srcDir, destDir, "/PURGE /R:2 ", destDirBase + "\\srcCopy." + dirs[i] + ".log ");
        } catch(e) {
            logMsg(LogScript, LogInfo, "Error ", e.description, "\n");
        }
    }
}

/***********************************************************************/
/* Copy all the directories in 'dirs' from 'srcDir' (the enlistment root)
   to 'destDir'. Plus some addition common tasks needed for razzle build.

   Parameters:
     srcDir  : The source directory
     destDir : The destination directory
     dirs    : Directories to copy with options
     logDir  : Where to put the log files 
*/

function robocopyDirsForBuild(srcDir, destDir, dirs, logDir) {

    if (srcDir == undefined)
        throw Error(1, "Required parameter srcDir not present");
    if (destDir == undefined)
        throw Error(1, "Required parameter dstDir not present");
    if (dirs == undefined)
        throw Error(1, "Required parameter dirs not present");
    if (logDir == undefined)
        throw Error(1, "Required parameter logDir not present");

    for(var i = 0; i < dirs.length; i++) {
        var dir = dirs[i][0];
        var option = dirs[i][1];
        logMsg(LogClrAutomation, LogInfo, "Processing ", srcDir, "\\", dir, "\n");
        robocopy(
            srcDir + "\\" + dir,
            destDir + "\\" + dir, 
            option + " /PURGE ",
            logDir + "\\srcCopy." + dir.replace(/\\/g, ".") + ".log ");
    }

    // razzle wants this file, and will block if it does not exist
    var setEnvFile = destDir + "\\developer\\clrgnt\\setenv.cmd" 
    if (!FSOFileExists(setEnvFile)) {
        FSOCreatePathForFile(setEnvFile);
        FSOWriteToFile("REM do nothing\r\n", setEnvFile);
    }

    // We don't want this file around
    var buildDat = destDir + "\\build.dat";
    if (FSOFileExists(buildDat)) {
        logMsg(LogClrAutomation, LogInfo, "Deleting ", buildDat, "\n");
        FSODeleteFile(buildDat, true);
    }

    // Also delete all obj* files at the top level
    var dirs = FSOGetDirPattern(destDir, /^obj/);
    for (var i = 0; i < dirs.length; i++) {
        logMsg(LogClrAutomation, LogInfo, "Deleting Directory", dirs[i], "\n");
        FSODeleteFolder(dirs[i], true);
    }

    return 0;
}
/***********************************************************************/
/* starting with a vbl lab base 'srcDir' copy just the files needed 
   for a build the CLR to 'destDir'

   Parameters:
     srcDir  : The source directory
     destDir : The destination directory
     logDir  : Where to put the log files 
*/

function robocopyClrSrc(srcDir, destDir, logDir) {

    if (srcDir == undefined)
        throw Error(1, "Required parameter srcDir not present");
    if (destDir == undefined)
        throw Error(1, "Required parameter dstDir not present");
    if (logDir == undefined)
        throw Error(1, "Required parameter logDir not present");

    var clrDirs = [
		    ["\\","partitions.proj /LEV:1"],

                    ["ndp", "/XD " + srcDir + "\\ndp\\clr\\snap2.4 " +
                                     srcDir + "\\ndp\\clr\\snap2.4_basline " +
                                     srcDir + "\\ndp\\clr\\snap2.4_staging " + 
                                     srcDir + "\\ndp\\clr\\snap " +
                                     srcDir + "\\ndp\\cdf " + 
                                     srcDir + "\\ndp\\fx\\test"],

                    ["OptimizationData", ""],

                    ["public", ""],

                    ["internalapis", ""],

                    ["externalapis", ""],

                    ["rotor", "/XD " + srcDir + "\\rotor\\PREfastBuildheaders " +
                                       srcDir + "\\rotor\\qa " + 
                                       srcDir + "\\rotor\\tests"],

                    ["tools", "/XD " + srcDir + "\\tools\\SafeIDE " +
                                       srcDir + "\\tools\\RascalPro"],

                    // For now we also want to copy "ddsuites\tools" so that we can build 
                    // the "clr\src\DotNetFx3" folder successfully  -andparo
                    ["ddsuites\\tools", ""],

                    // We need to copy "redist\common\netfx35a" so that we can build 
                    // the "clr\src\ToolBox\WPF" folder successfully  -andparo'
                    ["redist\\common\\netfx35a", ""],

                    // include setup authoring stuff for building NDP layouts.
                    ["SetupAuthoring", ""],
                    ["SetupProjects", ""]
                  ];

    // copy all the directories in clrDirs
    robocopyDirsForBuild(srcDir, destDir, clrDirs, logDir);

    return 0;
}

/***********************************************************************/
/* starting with a vbl lab base 'srcDir' copy just the files needed 
   for a build full stack CoreCLR to 'destDir'

   Parameters:
     srcDir  : The source directory
     destDir : The destination directory
     logDir  : Where to put the log files 
*/

function robocopyCoreClrFullStackSrc(srcDir, destDir, logDir) {

    if (srcDir == undefined)
        throw Error(1, "Required parameter srcDir not present");
    if (destDir == undefined)
        throw Error(1, "Required parameter dstDir not present");
    if (logDir == undefined)
        throw Error(1, "Required parameter logDir not present");

    // Copy only the files in the enlistment root folder, no "/S".
    logMsg(LogClrAutomation, LogInfo, "Processing ", srcDir, "\n");
    robocopy(srcDir, destDir, "/PURGE ", "SrcRootFiles.log", true, false);

    // The CoreCLR full stack build should build more projects than just ndp.
    // But this has been the way it is. We can review it and add more later.

    // /XD only accepts directory name or full path, "fx\\test" would not work.
    var clrDirs = [
                    ["vscommon", ""],

                    ["ndp", "/XD " + srcDir + "\\ndp\\clr\\snap2.4 " +
                                     srcDir + "\\ndp\\clr\\snap2.4_basline " +
                                     srcDir + "\\ndp\\clr\\snap2.4_staging " + 
                                     srcDir + "\\ndp\\clr\\snap " +
                                     srcDir + "\\ndp\\cdf " + 
                                     srcDir + "\\ndp\\fx\\test"],

                    ["public", ""],

                    ["rotor", "/XD " + srcDir + "\\rotor\\PREfastBuildheaders " +
                                       srcDir + "\\rotor\\qa " + 
                                       srcDir + "\\rotor\\tests"],

                    ["tools", "/XD " + srcDir + "\\tools\\SafeIDE " +
                                       srcDir + "\\tools\\RascalPro"]
                  ];

    // copy all the directories in clrDirs
    robocopyDirsForBuild(srcDir, destDir, clrDirs, logDir);

    return 0;
}

/*****************************************************************************/
/* canonicalize the warning for baseline comparison */

function canonicalizeWarning(line) {

    var key = line.replace(/^\d*>/, "")

    key = key.replace(/\[[^\[]+proj\]$/g, "");                  // remove end of line project filename for msbuild projects to get rid of unnecessary uniqueness for duplicate warnings.
    key = key.replace(/\S+\\(\S+\\\S+\\\S+?)/g, "$1");          // remove the prefix of paths
    key = key.replace(/\S+\\(ndp\\\S*?)/g, "$1");               // remove the prefix of short paths containing ndp
    key = key.replace(/\S+\\(internalapis\\\S+?)/g, "$1");      // remove the prefix of short paths containing internalapis
    key = key.replace(/\S+\\(tools\\\S+?)/g, "$1");             // remove the prefix of short paths containing tools
    key = key.replace(/\d+/g, "0");                             // replace numbers with 0 (line numbers etc)
    key = key.replace(/\d[\dA-Za-zxX]+/g, "0");                 // replace hex numbers with 0 (line numbers etc)
    key = key.replace(/\s/g, "");                               // remove white spaces (workaround for Dev10 #824638)

    return key;
}

/*****************************************************************************/
/* compare to build warning files and return non-zero if there are more
   warnings in 'warningFileName' than in 'baselineFileName'.  */

function buildWarningDiff(warningFileName, baselineFileName, ignorePat) {

    if (baselineFileName == undefined) {
        baselineFileName = Env("_NTBINDIR") + "\\ndp\\clr\\snap2.4\\baselines\\build\\" + Env("_BuildArch") + Env("_BuildType") + ".wrn";
        if (!FSOFileExists(baselineFileName))
            throw new Error(1, "Required argument baselineFileName is missing");
    }
    if (ignorePat == undefined)
        ignorePat = /Please check in this file when you submit/;

    var extraIgnorePat = /postlinkoptimize : warning > warning : Failed to map token/;

    var warnings = {};
    var ctr = 0;
    if (FSOFileExists(warningFileName)) {
        var warningFile = FSOOpenTextFile(warningFileName, FSOForReading);
        while (!warningFile.AtEndOfStream) {
            var line = warningFile.ReadLine();
            if (line.match(/^[\x00-\x1f\s]*$/) || line.match(ignorePat) || line.match(extraIgnorePat))
                continue;
            var key = canonicalizeWarning(line);
            warnings[key]= line;
            ctr++;
        }
        logMsg(LogScript, LogInfo, "buildWarningDiff: ", ctr, " warnings in ", warningFileName, "\n");
    }
    else {
        var logFileName = warningFileName.replace(/\.wrn/, ".log");
        if (!FSOFileExists(logFileName)) {
            logMsg(LogScript, LogError, "buildWarningDiff: ", warningFileName, " and neither does log file, failing\n");
            return -1;
        }
        logMsg(LogScript, LogInfo, "buildWarningDiff: ", warningFileName, " does not exist, assuming zero warnings\n");
    }

    var baseline = {};
    var ctr = 0;
    var baselineFile = FSOOpenTextFile(baselineFileName, FSOForReading);
    while (!baselineFile.AtEndOfStream) {
        var line = baselineFile.ReadLine();
        if (line.match(/^[\x00-\x1f\s]*$/) || line.match(ignorePat) || line.match(extraIgnorePat))
            continue;
        var key = canonicalizeWarning(line);
        baseline[key] = line;
        ctr++;
    }
    logMsg(LogScript, LogInfo, "buildWarningDiff: ", ctr, " warnings in baseline ", baselineFileName, "\n");

    var newWarnings = keys(setDiff(warnings, baseline));
    if (newWarnings.length > 0) {
        logMsg(LogScript, LogError, newWarnings.length, " unique new warnings compared to the baseline {\n",
            joinMap(newWarnings, "\n", function(x) { return warnings[x]; }), "\n}\n");

        /* { */ logMsg(LogScript, LogError, "\n}\n");

        logMsg(LogScript, LogError, "The baseline can be updated to supress this warning, however that should be done\n");
        logMsg(LogScript, LogError, "only as a last resort.\n");
        return -1;
    }

    var onlyInBaseline = keys(setDiff(baseline, warnings));
    if (onlyInBaseline.length > 0) {
        logMsg(LogScript, LogInfo, onlyInBaseline.length, " unique warnings can be removed from the baseline\n");
        logMsg(LogScript, LogInfo, "You can update the baseline simply by running the command\n");
        logMsg(LogScript, LogInfo, "    copy ", warningFileName, " ", baselineFileName, "\n");
    }

    logMsg(LogScript, LogInfo, "Congratulations no new warnings were introduced\n");
    logMsg(LogScript, LogInfo, "buildWarningDiff: returning success\n");
    return 0;
}

/****************************************************************************/
/* timeCmd is really just a simple wrapper over runCmdToLog that allows you
   to run things from 'runjs' directly.  Simply type

    runjs timeCmd <commandLine>

   and timeCmd runs the command prints the duration (accurate to msec),
   and exits with the same exit code as the command.  It also prepends the
   elapsed time (in hundreds of minutes to all output generated by the command
*/
function timeCmd(comandline) {

        // we are varargs, gather up the command line as an argument string
    var cmd = "";
    for(var i = 0; i < arguments.length; i++) {
        cmd += arguments[i];
        cmd += " ";
    }

    var run = runCmdToLog(cmd, runSetTimeout(3 * HOUR), runSetNoThrow());
    logMsg(LogScript, LogInfo, "timeCmd: duration = ", run.duration / 1000, " sec\n");
    return run.exitCode;
}

/****************************************************************************/
/* this trival helper is designed to cause 'runjs dailyDevRun' to go off
   every day at a particular time (timeToGoOff eg 23:00 is 11PM).  Basically
   all it does is write a 'dailyDev.cmd' in your startup folder so that 'runAt'
   starts up every time you log in.   (do runAt /? for details on runAt).
   Note that if you log off, runAt is killed and dailyDevRun is not run.

   Parameters
      timeToGoOff   The time of day to start the script.  By default is
                    a random time between midnight and 1AM
*/
function installDailyDevRun(timeToGoOff) {

    if (!isElevated())
    {
        throw new Error(1, "installDailyDevRun: Requires executing with elevated privilieges");
    }

    return installRunjsCommand(timeToGoOff, "dailyDevRun");
}

/****************************************************************************/
/* this helper is designed to cause 'runjs dailyDevRun' to go off
   every day at a particular time (timeToGoOff eg 23:00 is 11PM).  Basically
   all it does is write a 'dailyDev.cmd' in your startup folder so that 'runAt'
   starts up every time you log in.   (do runAt /? for details on runAt).
   Note that if you log off, runAt is killed and dailyDevRun is not run.

   Parameters
      timeToGoOff   The time of day to start the script.  By default is
                    a random time between midnight and 1AM
      runjsFunction runjs function to call
*/
function installRunjsCommand(timeToGoOff, runjsFunction) {
    var runjsCommand = "cscript " + WScript.ScriptFullName + " " + runjsFunction;
    return installRunAtCommand(timeToGoOff, runjsCommand, runjsFunction+".cmd");
}

/****************************************************************************/
/* this helper is designed to cause 'runjs dailyDevRun' to go off
   every day at a particular time (timeToGoOff eg 23:00 is 11PM).  Basically
   all it does is write a 'dailyDev.cmd' in your startup folder so that 'runAt'
   starts up every time you log in.   (do runAt /? for details on runAt).
   Note that if you log off, runAt is killed and dailyDevRun is not run.

   Parameters
      timeToGoOff   The time of day to start the script.  By default is
                    a random time between midnight and 1AM
      runjsFunction runjs function to call
*/
function installRunAtCommand(timeToGoOff, runatCommand, startupCmdFileName) {
    if (timeToGoOff == undefined)
        timeToGoOff = "00:" + ((new Date()).getMilliseconds() % 50 + 10).toString();

    if (runatCommand == undefined) {
        throw Error(1, "Required arg 'runatCommand' not supplied");
    }
    
    if (startupCmdFileName == undefined) {
        // use the command name as the startup folder .cmd name
        // skip cscript, runjs, doRun, cmd, or anything starting with '/' or '-'
        var matches = runatCommand.match(/\s*(\S+)\s*(\w*)\s*(\w*)\s*(\w*)/);
        
        for (var i = 1; i < matches.length; ++i) {
            logMsg(LogClrAutomation, LogInfo10, "match["+i+"] "+matches[i]+"\n");
        }
        for (var i = 1; i < matches.length; ++i) {
            var cmdMatch = matches[i].toLowerCase();
            logMsg(LogClrAutomation, LogInfo10, "cmdMatch "+cmdMatch+"\n");

            // if the command contains a full path just use the filename
            if (cmdMatch.match(/.+\\(.+)/)) {
                cmdMatch = RegExp.$1;
                logMsg(LogClrAutomation, LogInfo, "pruning "+matches[i]+" to "+cmdMatch+"\n");
            }
            if (
                cmdMatch.match(/cscript|runjs|doRun|cmd\.exe/i) // if these are anywhere skip this token
                || cmdMatch.match(/^cmd/)                       // if starts with cmd skip (it can end with it though)
                || cmdMatch[0]=="/"|| cmdMatch[0]== "-") {
                continue;
            }
            // we don't want the startup file to be clrfullstack.cmd.cmd (or .wsf.cmd or some such)
            cmdMatch = cmdMatch.replace(new RegExp("\\.(?:cmd|bat|exe|com|pl|wsf)$"), "");
            startupCmdFileName = cmdMatch+".cmd";
            break;
        }
        if (startupCmdFileName == undefined)
            startupCmdFileName = "runat.cmd";
    }
    logMsg(LogClrAutomation, LogInfo, "using CMD file: "+startupCmdFileName+"\n");

    if (Env("PROCESSOR_ARCHITEW6432") && Env("PROCESSOR_ARCHITECTURE").toLowerCase() == "x86") {
        logMsg(LogClrAutomation, LogError, "You are running this script using the 32 bit CScript\n");
        logMsg(LogClrAutomation, LogError, "Please don't do this.  Run the script from a 64 bit command shell\n");
        throw new Error(1, "Don't run installDailyDevRun using 32 bit CScript\n");
    }

    var runAt = ScriptDir +  "\\runAt.js";
    var dailyDevCmdData = "@start wscript " + runAt + " " + timeToGoOff + " " + runatCommand;

    var startupDir = Env("USERPROFILE") + "\\Start Menu\\Programs\\Startup";
    var startupCmdFile = startupDir + "\\" + startupCmdFileName;

    logMsg(LogClrAutomation, LogInfo, "Creating file ", startupCmdFile, "\n");
    logMsg(LogClrAutomation, LogInfo, "This file will cause 'runAt' to be started whenever you log in.\n");
    logMsg(LogClrAutomation, LogInfo, "Runat will run \"" + runatCommand + "\" every day at ", timeToGoOff, ".\n");
    logMsg(LogClrAutomation, LogInfo, "Use runat /? for more details on runAt.\n");
    FSOWriteToFile(dailyDevCmdData, startupCmdFile);

    // if runAt.log is not locked, then there is not one running, start it up
    var runAtLogFile = Env("USERPROFILE") + "\\RunAt.log";
    try {
        logMsg(LogClrAutomation, LogInfo, "Opening ", runAtLogFile, "\n")
        WshFSO.OpenTextFile(runAtLogFile, FSOForWriting, true).Close();
        logMsg(LogClrAutomation, LogInfo, "Starting the 'runAt' command\n");
        runCmd("start /i cmd /c \"" + startupCmdFile + "\"", runSetOutput());
    } catch(e) {
        logMsg(LogClrAutomation, LogInfo, "Runat already running, or we could not start it.\n");
    }
}

/****************************************************************************/
/* runX86 runs "runjs <commandLine>" using a 32 bit command shell.
   It does this by calling runWithRuntimeArch to execute the command.

   In runWithRuntimeArch we use ldr64 to set the machine to run all 
   managed code in the WOW, and also use runSet32Bit to force runCmdToLog
   to use a 32-bit command shell as the host for the "runjs <commandLine>".
   After completion it uses ldr64 to reset to run managed code using
   the 64-bit runtime.
*/
function runX86(commandLine) {

    if (commandLine == undefined)
        throw new Error(1, "required arg commandLine not supplied");

    // we are varargs, gather up the command line as an argument string
    var cmd = "runjs ";
    for(var i = 0; i < arguments.length; i++) {
        var arg = arguments[i];
        if (arg == undefined) {
            arg = "_";
        }
        else {
            arg = arg.replace(/"/g, "\\\"");           // Emacs:C++-mode  %%  " );
            if (arg.match(/[\s"]/))                    // Emacs:C++-mode  %%  " )))
                arg = "\"" + arg + "\"";
        }
        cmd += arg;
        cmd += " ";
    }

    var run = runWithRuntimeArch(cmd, "x86", 
                                 runSetStream(WScript.StdOut,
                                 runSetTimeout(1000 * HOUR), 
                                 runSetNoThrow()));

    logMsg(LogClrAutomation, LogInfo, "} runX86 Success - returning ", run.exitCode, "\n");
    return run;
}

/****************************************************************************/
/* browse the CLR source base on a read-only share.   To get intellisense to
   work, it copies the solution to a local directory which then points to 
   the share.   By default it picks the most recent verison of the CLR in the
   Dev 10 code base.  */
function browseClr(clrSlnFile, targetSlnFile)
{
    if (clrSlnFile == undefined)
        clrSlnFile = "\\\\cpvsbuild\\drops\\dev10\\clr\\raw\\current\\sources\\ndp\\clr\\src\\clr.sln";

    if (targetSlnFile == undefined)
        targetSlnFile = Env("TEMP") + "\\" + clrSlnFile.match(/([^\\]*$)/)[1];
    
    logMsg(LogClrAutomation, LogInfo, "Clr Solution ", clrSlnFile, "\n");
    logMsg(LogClrAutomation, LogInfo, "Local Solution ", targetSlnFile, "\n");

    if (!FSOFileExists(clrSlnFile))  {
        logMsg(LogClrAutomation, LogError, "Could not find file ", clrSlnFile, "\n");
        return 1;
    }
    var slnDir = clrSlnFile.match(/^(.*)\\.*$/)[1];

    // Get rid of the old NCB file as it is probably wrong.
    var ncbFile = slnDir + "\\clr.ncb";
    FSODeleteFile(ncbFile);

    var slnData = FSOReadFromFile(clrSlnFile);
    slnData = slnData.replace(/^(Project.*?,\s*")(\S*proj",)/mg, "$1" + slnDir + "\\$2");


    slnData = slnData.replace(/.*GlobalSection.*TeamFoundationVersionControl(.|\n)*?EndGlobalSection\s*\n/, "");

    FSOWriteToFile(slnData, targetSlnFile);
    logMsg(LogClrAutomation, LogInfo, "Launching solution file ", targetSlnFile, "\n");
    runCmd("start devenv " + targetSlnFile + " " + slnDir + "\\vm\\ceemain.cpp");
}

/****************************************************************************/
/* checks in all files that have changes that are only comments that are 
   under the directory 'directory'.  It will not check in 
   files that need to be synced, and it will not check in comments that
   will conflict with entries in the SNAP queue.  Once the list of files
   is determined, the user is prompted.  The user can view the diff, 
   and after assuming the user is happy, he can indicate that the checkin
   can be submitted.  'options' can be any of the options below (however
   you should put space between multiple options).

   options:
       /noPrompt - Don't prompt before submitting.
       /noChange - Just show what would have been checked in.
       /verbose  - More detailed output.
       /gui      - Prompt using a gui dialog box.
*/
function checkinComments(directory, options)
{
    var opts = getOptions(["noChange", "noPrompt", "verbose", "gui"], options);
    if (directory == null)
        directory = ".";

    directory = FSOGetFolder(directory).Path.toLowerCase();

    // allow this to be run from the runjs on the network share
    logMsg(LogClrAutomation, LogInfo, "ScriptDir " + ScriptDir + "\n");
    if (!ScriptDir.match(/^(.*)\\ndp\\clr\\bin/i)) {
        var vblBase =  directory.match(/^(.*?\\src)\\/i)[1];
        var runjs = vblBase + "\\ndp\\clr\\bin\\runjs.bat";
        // TODO currently there are ugly dependencies on clrenv variable (probably not too hard however)
        // But I did not want to deal with it now.   someday...
        var clrenv = vblBase + "\\ndp\\clr\\bin\\clrenv.bat";
        var cmd = runCmdToLog(clrenv + " -cmd " + runjs + " checkinComments " + directory + " " + options, runSetTimeout(7 * 24 * 3600, runSetNoThrow()))
        return cmd.ExitCode;
    }

    if (!_inTFS()) {
        logMsg(LogClrAutomation, LogError, "This version of checkinComments is supported on TFS only, not SD.\n");
        return;
    }

    logMsg(LogClrAutomation, LogInfo, "\nCheckin Comments under: " + directory + "\n");

    var fileList = _tfsOpened(directory);
    
    // select from fileList the files that differ in comments only put the in commentOnlyList
    var checkinList = [];
    var needSync = false;
    var temp = FSOGetTempPath("checkin") + ".txt";
    var vblRoot = findVblRoot(directory);
    if (vblRoot == undefined)
        throw Error(1, "Could not find vbl root from directory " + directory);
    vblRoot = vblRoot.toLowerCase();
    var vblRootSlash = vblRoot + "\\";
    var depotNames = {};        // remember the depot name associated with each file name
    var needSnapCheck = false;

    for(var i = 0; i < fileList.length; i++)  {
        var fileName = fileList[i].toLowerCase();

        var displayFileName = fileName;
        if (fileName.substr(0, vblRoot.length+1)  == vblRootSlash) {
            displayFileName = fileName.substr(vblRoot.length+1);
            if (displayFileName.match(/^public\\/i)) {
                if (opts.verbose)
                    logMsg(LogClrAutomation, LogInfo, "Skip-public file :  " + displayFileName + "\n");
                continue;
            }
        }
            
        if (!fileName.match(/\.((cpp)|(hpp)|(h)|(c)|(inl)|(cs)|(js))$/, "i")) {
            if (opts.verbose)
                logMsg(LogClrAutomation, LogInfo, "Skip-Suffix wrong:  " + displayFileName + "\n");
            continue;
        }

        // is this file up to date?

        // "W" = version in the user's local workspacae
        // "T" = version that is latest on the server (tip of trunk)
        var fileInfo = _tfsHistory(fileName, "W", "T");
        
        // When you query TFS history in a from/to range, the result is intentionally
        // exclusive on the "from" side iff from side = W. (Otherwise, TFS history is
        // always inclusive on both the from and to sides.) This is actually intentional,
        // though obviously not documented. So to determine if the file needs syncing,
        // check that the history in the range from "current version in workspace" (W) to
        // "latest version on server" (T) contains 0 items on TFS
        if (fileInfo.length != 0) {
            logMsg(LogClrAutomation, LogInfo, "Skip-Need sync:     " + displayFileName + "\n");
            needSync = true;
            continue;
        }
        
        var depotName = _tfsLocalPathToServerPath(fileName);
        
        depotNames[fileName] = depotName;
        needSnapCheck = needsSnapCheck(depotName, needSnapCheck);

        _tfsFetch(fileName, temp);

        if (!onlyDifferInComments(fileName, temp)) {
            logMsg(LogClrAutomation, LogInfo, "Skip-Non-Comments:  " + displayFileName + "\n");
            FSODeleteFile(temp);
            continue;
        }

        if (FSOReadFromFile(fileName) == FSOReadFromFile(temp)) {
            logMsg(LogClrAutomation, LogInfo, "Skip-No changes:    " + displayFileName + "\n");
            FSODeleteFile(temp);
            continue;
        }

        FSODeleteFile(temp);
        checkinList.push(fileName);
    }

    if (checkinList.length > 0) {
        var message =  "There are " + checkinList.length + " comment-only files that can be checked in\r\n"
        for(var i =0; i < checkinList.length; i++)
            message += "    " + checkinList[i] + "\r\n";
        logMsg(LogClrAutomation, LogInfo, message);
        
            // Have the user look over all the changes 
        var defaultVal = "d";
        if (!opts.noPrompt) {
            WScript.StdOut.WriteLine();
            for(;;)  {
                var answer;
                var promptText = "you wish to submit the files? (y/n, d diffs): ";
                if (opts.noChange) 
                    promptText = "Do you wish to view the differences? (y/n): ";

                if (opts.gui) {
                    var promptText = message + "\r\nDo you wish to view the differences?"
                    if (answer != "d" && promptYesNo(message + "\r\nDo you wish to view the differences?"))
                        answer = "d";
                    else if (!opts.noChange &&  promptYesNo(message + "\r\n\r\nDo you wish to submit?"))
                        answer = "y";
                    else 
                        answer = "n";
                }
                else {
                    WScript.StdOut.Write(promptText);
                    answer = WScript.StdIn.ReadLine();
                    if (answer == "")
                        answer = defaultVal;
                }

                if (answer == undefined || answer.match(/^n/i)) {
                    logMsg(LogClrAutomation, LogInfo, "User abort: No action taken.\n");
                    return 1;
                }
                if (answer.match(/^y/i)) {
                    if (opts.noChange)
                        answer = "d";
                    else 
                        break;
                }
                if (answer.match(/^d/i)) {
                    for(var i = 0; i < checkinList.length; i++) {
                        // tf diff refuses to launch windiff if you try to redirect
                        // output of the tf diff command. WShell.Exec always
                        // redirects output implicitly, even if the command you pass
                        // it does not explicitly redirect output. Therefore, we must
                        // use WShell Run() to invoke tf diff. The "true" forces
                        // Run() to wait until the diff is complete
                        WshShell.Run("tf.cmd diff " + checkinList[i], undefined, true);
                    }
                    defaultVal = "y";
                }
                else 
                    defaultVal = "n";
            }
        }

        if (!opts.noChange) {
                
            var finalCheckinList = checkinList;
            // Check for conflicts with the SNAP queue and remove those (do this late since it changes with time)
            if (needSnapCheck) {
                logMsg(LogClrAutomation, LogInfo, "Checking for conflicts with the SNAP queue.\n");
                var queueInfo = snapQueueInfo(vblRoot);
                var finalCheckinList = []
                for(var i =0; i < checkinList.length; i++) {
                    var fileName = checkinList[i];
                    if (conflictsWithSnapQueue(fileName, depotNames[fileName], queueInfo)) {
                        logMsg(LogClrAutomation, LogInfo, "    SNAP Conflict: " + fileName + "\n");
                        continue;
                    }
                    finalCheckinList.push(fileName);
                }
            }

            if (finalCheckinList.length > 0) {
                var checkinNum = _tfsCheckin("Updating comments", finalCheckinList);
                logMsg(LogClrAutomation, LogInfo, "Checkin successful as change ", checkinNum, ".\n");
            }
            else 
                logMsg(LogClrAutomation, LogInfo, "All files conflict with SNAP.  Try again later.\n");

            if (finalCheckinList.length != checkinList.length) {
                logMsg(LogClrAutomation, LogInfo, "\nSome files were removed from the check because they conflict with\n");
                logMsg(LogClrAutomation, LogInfo, "files that are pending in the SNAP queue.  Rerun this command at\n");
                logMsg(LogClrAutomation, LogInfo, "a later time to check in the rest.\n");
            }
        }
        else 
            logMsg(LogClrAutomation, LogInfo, "\n/noChange specified, No action taken.\n");
    }
    else 
        logMsg(LogClrAutomation, LogInfo, "No files to checkin.\n");
        
    if (needSync)
        logMsg(LogClrAutomation, LogInfo, "\nWARNING: You need to sync your code base!\n");
    return 0;
}

/***********************************************************************/
/* basically this is an oracle that tells you whether particular parts
   of the depot is controled by a SNAP checkin system */

function needsSnapCheck(depotFile, previousValue)
{
    if (depotFile.match(/^\$\/Dev10\/pu\/clr/i))
        return true;

    // we assume other are private branches and we assume we can skip the snap check
    return previousValue;
}

/***********************************************************************/
/* returns true if 'fileName' with depot name 'depotName' conflicts with
   any SNAP queue entry in 'snapQueueInfo */
function conflictsWithSnapQueue(fileName, depotName, snapQueueInfo)
{
    logMsg(LogClrAutomation, LogInfo10, "Snap Queue Check: ", fileName, " ", depotName, "\n");

    for(var i = 0; i < snapQueueInfo.length; i++)  {
        var snapEntry = snapQueueInfo[i];
        if (member(snapEntry.files, depotName)) {
            logMsg(LogClrAutomation, LogInfo10, "Conflicts with running SNAP job: " + fileName + "\n");
            return true;
        }

        // TODO: we could relax this to things that would merge well, 
    }

    return false;
}

/***********************************************************************/
/* returns an array of queue entries for the snap queue associated with
   'srcBase' (srcBase is base of a VBL).  Each entry in the array
   contains the following fields:

        entry.entryId         - the submission number (or m\d+ for merged jobs), 
        entry.user            - The user name
        entry.status          - status (In Progress, Merged, Posponed, ...)
        entry.entryDir        - The directory where xml and bbpacks are
        entry.files           - List of server paths of files in the jjpack/shelveset associated with the job
        entry.shelveset       - The shelveset associated with the job
*/
function snapQueueInfo(srcBase)
{
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase is missing");
    }

    var info = [];
    var snapQstr = runCmd(srcBase + "\\ndp\\clr\\bin\\snap q").output;
    
    if (!snapQstr.match(/^\s*(\S+)\s.*queue is (\d+) job.*deep/m))
        throw Error(1, "Unexpected snap command output:\r\n" + snapQstr);

    var queueName = RegExp.$1.toLowerCase();
    var numJobs = RegExp.$2;
    logMsg(LogClrAutomation, LogInfo10, "snapQueueInfo: queue = ", queueName, " numJobs = ", numJobs, "\n");
    if (numJobs > 0) {
        var lines = snapQstr.split("\n");
        for(var i = 0; i < lines.length; i++) {
            var line = lines[i];
            if (line.match(/^(.?\d+)\s+(\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s+((In )?\S+)/)) {
                var entry = {};
                entry.user    = RegExp.$3;
                entry.entryId = RegExp.$1;
                entry.status  = RegExp.$6;
                entry.entryDir  = "\\\\clrsnap0\\queues\\" + queueName + "\\" + entry.entryId + "." + entry.user;
                entry.spk = entry.entryDir + "\\" + entry.user + ".spk";
                if (!FSOFileExists(entry.spk)) {
                    logMsg(LogClrAutomation, LogInfo10, "Entry ", entry.entryDir, " does not contain '" + entry.spk + "', skipping\n");
                    --numJobs;
                    continue;
                }
                entry.shelveset = trim(FSOReadFromFile(entry.spk));

                    // get all the files of type "edit" (ignore new files like "add"
                    // or "branch")
                entry.files = _tfsGetShelvesetEditFiles(entry.shelveset);
                info.push(entry);
            }
        }
        if (info.length != numJobs)
            throw Error(1, "Error parsing snap jobs info: got " + info.length + " entries expected " + numJobs);
    }
    logMsg(LogClrAutomation, LogInfo10, "Got snap info\n");
    return info; 
}

/***********************************************************************/
/* Finds the base of the vbl (what _NTBINDIR would be set to) by searching
   in all the directories above 'path' for something that looks 
   like the vbl.  returns undefined if it could not be found.
   'path' can be a directory or a file name */

function findVblRoot(path)
{
        // if it is relative, add the current directory to it.
    if (!path.match(/^(\w:)?\\/))
        path = WshShell.CurrentDirectory + "\\" + path;

    for(;;) {
        var runjsFile = path + "\\ndp\\clr\\bin\\runjs.bat";
        logMsg(LogClrAutomation, LogInfo100, "Trying: ", runjsFile, "\n");
        if (FSOFileExists(runjsFile))
            return path;

        if (!path.match(/^(.*)\\.*?$/))
            return undefined;
        path = RegExp.$1;

        // if it is a UNC path, stop before we make nonsense (and cause delays)
        if (path.match(/^\\\\\w+$/))
            return undefined;
    }
}

/***********************************************************************/
/* returns true if fileName1 and fileName2 differ only by C++ style
   comment lines (or whitespace).  */

function onlyDifferInComments(fileName1, fileName2) 
{
    var file1 = FSOOpenTextFile(fileName1, FSOForReading);
    var file2 = FSOOpenTextFile(fileName2, FSOForReading);

    var line1;
    var line2;
    var ret = false
    for(;;) {
        line1 = readNonCommentLine(file1);
        line2 = readNonCommentLine(file2);
        if (line1 == undefined && line2 == undefined) {
            ret = true
            break;
        }
        if (line1 != line2) {
            logMsg(LogClrAutomation, LogInfo10, "Lines differ: ", fileName1, ":", file1.Line, "\n");
            logMsg(LogClrAutomation, LogInfo100, "   line<: ", line1,"\n");
            logMsg(LogClrAutomation, LogInfo100, "   line>: ", line2,"\n");
            break;
        }
    }
    file1.Close();
    file2.Close();
    return ret;
}

/***********************************************************************/
/* returns the next line in the file represented by the file handle 'file'
   That is not a comment or whitespace line.  Returns undefined at the
   end if a file */

function readNonCommentLine(file) 
{
    for(;;) {
        if (file.AtEndOfStream)
            return undefined;
        var line = file.ReadLine();
        // // Style comments are pretty easy, just skip them.
        if (line.match(/^\s*(\/\/.*)?$/))
            continue;

        //  Are we starting a /**/ style comment?
        if (line.match(/^\s*\/\*(.*)$/)) {
            line = RegExp.$1;        // skip past the /* 

            for(;;) {                // Read until we find the end. 

                if (line.match(/\*\/\s*(.*)/)) {
                    var rest = RegExp.$1
                    if (rest != "")
                        return rest;    // We found something beyond the */ terminator, return it
                    break;
                }
                if (file.AtEndOfStream)
                    throw new Error(1, "End of file in /**/ style comment!");
                line = file.ReadLine();
            }
            continue;
        }

        line.match(/^\s*(.*?)\s*$/);        // Remove leading and trailing space. 
        line = RegExp.$1;
        if (line.match(/^([^"]*?)\s*\/\//))
            line = RegExp.$1;
        return line
    }
}

/***********************************************************************/
/* A simple gui prompt Yes/No prompt. */

function promptYesNo(promptText, retry)
{
    logMsg(LogClrAutomation, LogInfo10, "Prompting ", promptText, "\n");
    var result = WshShell.Popup(promptText, 1000000, promptText, 4)
    logMsg(LogClrAutomation, LogInfo10, "Prompt returning ", result, "\n");
    return (result == 6);
}

/****************************************************************************/
/* run fxpSetup from 'binDir' to 'targetDir'
*/
function fxpSetup(binDir, targetDir) {
    if (binDir == undefined)
        throw new Error(1, "fxpSetup: Required argument binDir for the clrSetup command not present");
        
    if (targetDir == undefined)
        targetDir = "";

    logMsg(LogClrAutomation, LogInfo, "fxpSetup(", binDir, ") {\n");

    var run = runCmdToLog(binDir + "\\fxpsetup.cmd " + targetDir, undefined);
    
    if (run.exitCode != 0)
        throw Error(1, "fxpSetup: setup failed with exit code " + run.exitCode);

    if (!run.output.match(/TARGETDIR *= (.*)\r\n/))
        throw new Error(-1, "Could not parse fxpsetup output to find installation directory\n");
    var installDir = RegExp.$1;
    
    logMsg(LogClrAutomation, LogInfo, "} fxpSetup() = ", installDir, "\n");
    return installDir;
}

function getLatestJobDirSubmitted(srcBase) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    logCall(LogClrAutomation, LogInfo, "getLatestJobDirSubmitted", arguments, " {");

    var filename = srcBase + "\\OptimizationData\\LatestDataSubmittedToSnap.txt";
    var returnValue = "";

    if (FSOFileExists(filename))
    {
        var lines = FSOReadFromFile(filename).split(/\s*\n/)
        returnValue = lines[0];
    }

    logMsg(LogClrAutomation, LogInfo, "} getLatestJobDirSubmitted Success - returning ", returnValue, "\n");

    return returnValue;
}

function getLatestJobDateSubmitted(srcBase) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    logCall(LogClrAutomation, LogInfo, "getLatestJobDateSubmitted", arguments, " {");

    var filename = srcBase + "\\OptimizationData\\LatestDataSubmittedToSnap.txt";
    var returnValue = "";

    if (FSOFileExists(filename))
    {
        var lines = FSOReadFromFile(filename).split(/\s*\n/)
        returnValue = lines[1];
    }

    logMsg(LogClrAutomation, LogInfo, "} getLatestJobDateSubmitted Success - returning ", returnValue, "\n");

    return returnValue;
}

function markLatestDataSubmitted(srcBase, snapJobDir, snapJobDate) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (snapJobDir == undefined) {
        throw new Error(1, "Required argument 'snapJobDir' is missing");
    }

    if (snapJobDate == undefined) {
        throw new Error(1, "Required argument 'snapJobDate' is missing");
    }

    logCall(LogClrAutomation, LogInfo, "markLatestDataSubmitted", arguments, " {");

    FSOWriteToFile(snapJobDir + "\r\n" + snapJobDate + "\r\n", 
                   srcBase + "\\OptimizationData\\LatestDataSubmittedToSnap.txt", false);

    logMsg(LogClrAutomation, LogInfo, "} markLatestDataSubmitted\n");
}

function readMaxChangeset(changelistTxtFilename) {

    var maxChangeset = -1;
    if (FSOFileExists(changelistTxtFilename))
    {
        try {
            var lines = FSOReadFromFile(changelistTxtFilename).split(/\s*\n/);
            for (var i = 0; i < lines.length; ++i) {

                if (lines[i].match(/^([0-9]+) /i))
                {
                    var changeset = parseInt(RegExp.$1);
                    if (changeset > maxChangeset)
                        maxChangeset = changeset;
                }
            }
        }
        catch (e) {
            maxChangeset = -1;
        }
    }

    return maxChangeset;
}

function getOptDataDateFromJobDir(snapJobDir) {

    var returnValue = undefined;

    try {
        var mscorlibZip = snapJobDir + "\\x86ret\\bin\\IbcData\\mscorlib.zip";
        if (FSOFileExists(mscorlibZip))
        {
            returnValue = FSOGetFile(mscorlibZip).DateCreated;
        }
    }
    catch (e) {
        logMsg(LogClrAutomation, LogInfo, "WARNING: IO exceptions were thrown while getting the date for ", snapJobDir, ": ", e.description, "\n");
        returnValue = undefined;
    }

    return returnValue;
}

function getLatestJobWithOptDataFromSnap(snapPath) {

    if (snapPath == undefined) {
        snapPath = "\\\\clrsnap0\\queues\\tfs_puclr";
    }

    logCall(LogClrAutomation, LogInfo, "getLatestJobWithOptDataFromSnap", arguments, " {");

    var returnValue = "";

    try {

        var folder = FSOGetFolder(snapPath);
        var subFolders = [];
        for (var subFoldersEnum = new Enumerator(folder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
            var subFolder = subFoldersEnum.item();

            var changelistFilename = subFoldersEnum.item().Path + "\\changelist.txt";
            if (FSOFileExists(changelistFilename)) 
            {
                for (var subFoldersEnum2 = new Enumerator(subFolder.SubFolders); !subFoldersEnum2.atEnd(); subFoldersEnum2.moveNext()) {

                    var jobFolder = subFoldersEnum2.item();

                    if (FSOFolderExists(jobFolder.Path + "\\x86ret\\bin\\IbcData") &&
                        FSOFolderExists(jobFolder.Path + "\\x86ret\\bin\\BbtData") &&
                        FSOFolderExists(jobFolder.Path + "\\amd64ret\\bin\\IbcData") &&
                        FSOFolderExists(jobFolder.Path + "\\amd64ret\\bin\\PgoDataMerged"))
                    {
                        subFolders.push(jobFolder);
                    }
                }
            }
        }

        var latestJobDir = "";
        var latestJobDate = undefined;

        for (var i = 0; i < subFolders.length; ++i)
        {
            var jobDir = subFolders[i].Path;
            var jobDate = getOptDataDateFromJobDir(jobDir);
            if (jobDate != undefined)
            {
                if (latestJobDate == undefined || jobDate > latestJobDate)
                {
                    latestJobDir = jobDir;
                    latestJobDate = jobDate;
                }
            }
        }

        if (subFolders.length > 0)
            returnValue = latestJobDir;

    } catch(e) {
        logMsg(LogClrAutomation, LogInfo, "WARNING: IO exceptions were thrown while searching ", snapPath, ": ", e.description, "\n");
        returnValue = "";
    }

    logMsg(LogClrAutomation, LogInfo, "} getLatestJobWithOptDataFromSnap Success - returning ", returnValue, "\n");

    return returnValue;
}

function shouldSnapGenerateOptData(snapPath) {

    // Disable retail optimization from snap. We now have a retail lab to this
    // But let's not remove the logic yet, in case we have to ever flip the switch
    // Jobs submitted to snap can still generate retail optimization using the -genopdata switch
    // This would be useful during FI's because the retail lab does not map the entire tree
    // But the preferred way is to use the retail lab whenever possible
    // For any questions about this, please contact sanket
    return false;

    if (snapPath == undefined) {
        snapPath = "\\\\clrsnap0\\queues\\tfs_puclr";
    }

    logCall(LogClrAutomation, LogInfo, "shouldSnapGenerateOptData", arguments, " {");

    var returnValue = true;

    var snapJobDir = getLatestJobWithOptDataFromSnap();
    if (snapJobDir != "")
    {
        try {
            var snapJobDirTime = getOptDataDateFromJobDir(snapJobDir);
            var currentTime = new Date();
            var isPreferredTime = (currentTime.getHours() >= 10 &&  currentTime.getHours() < 22);
            var diffInHours = (currentTime - snapJobDirTime) / (60*60*1000);

            logMsg(LogClrAutomation, LogInfo, "Latest Opt Data Folder = ", snapJobDir, "\n");
            logMsg(LogClrAutomation, LogInfo, "Latest Opt Data Time = ", snapJobDirTime, "\n");
            logMsg(LogClrAutomation, LogInfo, "Current Time = ", currentTime, "\n");
            logMsg(LogClrAutomation, LogInfo, "Current Time is between 10AM and 10PM = ", isPreferredTime, "\n");
            logMsg(LogClrAutomation, LogInfo, "Difference (in hours) = ", diffInHours, "\n");

            if (diffInHours < 12 || (diffInHours < 24 && !isPreferredTime))
                returnValue = false;
        } catch(e) {
            logMsg(LogClrAutomation, LogInfo, "WARNING: IO exceptions were thrown while searching ", snapPath, ": ", e.description, "\n");
            returnValue = true;
        }
    }

    logMsg(LogClrAutomation, LogInfo, "} shouldSnapGenerateOptData Success - returning ", returnValue, "\n");

    return returnValue;
}

function getSnapJobNumFromDir(snapJobDir) {

    if (!snapJobDir.match(/(m?[0-9]+)\.[^\\]*\\job\.([0-9]+)$/i))
        return "";
    var snapJobNum = RegExp.$1 + "." + RegExp.$2;

    return snapJobNum;
}

function getOptDataTimestamp() {

    var time = new Date();
    var strYear = time.getYear().toString();
    var strMonth = (time.getMonth()+1).toString();
    var strDate = time.getDate().toString();
    var strHours = time.getHours().toString();
    var strMinutes = time.getMinutes().toString();
    var strSeconds = time.getSeconds().toString();

    if (strMonth.length < 2) {
        strMonth = "0" + strMonth;
    }

    if (strDate.length < 2) {
        strDate = "0" + strDate;
    }

    if (strHours.length < 2) {
        strHours = "0" + strHours;
    }

    if (strMinutes.length < 2) {
        strMinutes = "0" + strMinutes;
    }

    if (strSeconds.length < 2) {
        strSeconds = "0" + strSeconds;
    }

    var timestamp =  strYear + strMonth + strDate + "_" + strHours + strMinutes + strSeconds;

    return timestamp;
}

function submitDataToSnap(srcBase, snapJobDir) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (snapJobDir == undefined) {
        throw new Error(1, "Required argument 'snapJobDir' is missing");
    }

    logCall(LogClrAutomation, LogInfo, "submitDataToSnap", arguments, " {");

    logMsg(LogClrAutomation, LogInfo, "Undoing all changes under OptimizationData\\x86\\CLR\\Base...\n");
    runCmdToLog("tf undo " + srcBase + "\\OptimizationData\\x86\\CLR\\Base /r /noprompt", runSetNoThrow());
    runCmdToLog("rd /s/q " + srcBase + "\\OptimizationData\\x86\\CLR\\Base", runSetNoThrow());
    runCmdToLog("tf get " + srcBase + "\\OptimizationData\\x86\\CLR\\Base /r /force", runSetNoThrow());

    logMsg(LogClrAutomation, LogInfo, "Undoing all changes under OptimizationData\\amd64\\CLR\\Base...\n");
    runCmdToLog("tf undo " + srcBase + "\\OptimizationData\\amd64\\CLR\\Base /r /noprompt", runSetNoThrow());
    runCmdToLog("rd /s/q " + srcBase + "\\OptimizationData\\amd64\\CLR\\Base", runSetNoThrow());
    runCmdToLog("tf get " + srcBase + "\\OptimizationData\\amd64\\CLR\\Base /r /force", runSetNoThrow());

    var snapJobNum = getSnapJobNumFromDir(snapJobDir);

    var IbcSourceDir_x86   = snapJobDir + "\\x86ret\\bin\\IbcData";
    var IbcSourceDir_amd64 = snapJobDir + "\\amd64ret\\bin\\IbcData";
    var BbtSourceDir_x86   = snapJobDir + "\\x86ret\\bin\\BbtData";
    var BbtSourceDir_amd64 = snapJobDir + "\\amd64ret\\bin\\BbtData";
    var PgoSourceDir_amd64 = snapJobDir + "\\amd64ret\\bin\\PgoDataMerged";

    if (!FSOFolderExists(IbcSourceDir_x86)) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: Could not find the x86 IBC data in ", snapJobDir, "\n");
        return false;
    }

    if (!FSOFolderExists(BbtSourceDir_x86)) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: Could not find the x86 BBT data in ", snapJobDir, "\n");
        return false;
    }

    if (!FSOFolderExists(BbtSourceDir_amd64)) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: Could not find the amd64 BBT data in ", snapJobDir, "\n");
        return false;
    }

    if (!FSOFolderExists(IbcSourceDir_amd64)) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: Could not find the amd64 IBC data in ", snapJobDir, "\n");
        return false;
    }

    if (!FSOFolderExists(PgoSourceDir_amd64)) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: Could not find the amd64 PGO data in ", snapJobDir, "\n");
        return false;
    }

    // Robocopy optimization data from SNAP to a temporary folder

    try {
        var IbcTempDir_x86   = Env("TEMP") + "\\IbcData-x86";
        var BbtTempDir_x86   = Env("TEMP") + "\\BbtData-x86";
        var BbtTempDir_amd64 = Env("TEMP") + "\\BbtData-amd64";
        var IbcTempDir_amd64 = Env("TEMP") + "\\IbcData-amd64";
        var PgoTempDir_amd64 = Env("TEMP") + "\\PgoData-amd64";

        runCmd("rd /s/q " + IbcTempDir_x86, runSetNoThrow());
        runCmd("md " + IbcTempDir_x86, runSetNoThrow());
        runCmd("rd /s/q " + BbtTempDir_x86, runSetNoThrow());
        runCmd("md " + BbtTempDir_x86, runSetNoThrow());

        robocopy(IbcSourceDir_x86, IbcTempDir_x86, "/PURGE ", Env("TEMP") + "\\CopyIbcData-x86.log", false);
        robocopy(BbtSourceDir_x86, BbtTempDir_x86, "/PURGE ", Env("TEMP") + "\\CopyBbtData-x86.log", false);

        runCmd("rd /s/q " + BbtTempDir_amd64, runSetNoThrow());
        runCmd("md " + BbtTempDir_amd64, runSetNoThrow());
        runCmd("rd /s/q " + IbcTempDir_amd64, runSetNoThrow());
        runCmd("md " + IbcTempDir_amd64, runSetNoThrow());
        runCmd("rd /s/q " + PgoTempDir_amd64, runSetNoThrow());
        runCmd("md " + PgoTempDir_amd64, runSetNoThrow());

        robocopy(BbtSourceDir_amd64, BbtTempDir_amd64, "/PURGE ", Env("TEMP") + "\\CopyBbtData-amd64.log", false);
        robocopy(IbcSourceDir_amd64, IbcTempDir_amd64, "/PURGE ", Env("TEMP") + "\\CopyIbcData-amd64.log", false);
        robocopy(PgoSourceDir_amd64, PgoTempDir_amd64, "/PURGE ", Env("TEMP") + "\\CopyPgoData-amd64.log", false);
    }
    catch (e) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: IO exceptions were thrown while robocopying from SNAP: ", e.description, "\n");
        return false;
    }

    try {
        // Check out the optimization data files for x86

        var IbcTargetDir_x86   = srcBase + "\\OptimizationData\\x86\\CLR\\Base";
        var BbtTargetDir_x86   = srcBase + "\\OptimizationData\\x86\\CLR\\Base";
    
        logMsg(LogClrAutomation, LogInfo, "Checking out IBC data for x86...\n");
        checkOutDirectoryTfs(IbcTempDir_x86, IbcTargetDir_x86);

        logMsg(LogClrAutomation, LogInfo, "Checking out BBT data for x86...\n");
        checkOutDirectoryTfs(BbtTempDir_x86, BbtTargetDir_x86);

        // Check out the optimization data files for amd64

        var BbtTargetDir_amd64 = srcBase + "\\OptimizationData\\amd64\\CLR\\Base";
        var IbcTargetDir_amd64 = srcBase + "\\OptimizationData\\amd64\\CLR\\Base";
        var PgoTargetDir_amd64 = srcBase + "\\OptimizationData\\amd64\\CLR\\Base";

        logMsg(LogClrAutomation, LogInfo, "Checking out BBT data for amd64...\n");
        checkOutDirectoryTfs(BbtTempDir_amd64, BbtTargetDir_amd64);

        logMsg(LogClrAutomation, LogInfo, "Checking out IBC data for amd64...\n");
        checkOutDirectoryTfs(IbcTempDir_amd64, IbcTargetDir_amd64);

        logMsg(LogClrAutomation, LogInfo, "Checking out PGO data for amd64...\n");
        checkOutDirectoryTfs(PgoTempDir_amd64, PgoTargetDir_amd64);
    }
    catch (e) {
        logMsg(LogClrAutomation, LogInfo, "ERROR: Exceptions were thrown while checking out files: ", e.description, "\n");
        return false;
    }

    //Wait up to 10 minutes to post to SNAP
    var runOpts = runSetTimeout(10 * MINUTE,
                  runSetIdleTimeout(10 * MINUTE,
                  runSetNoThrow(),
                  clrRunTemplate));

    var timestamp = getOptDataTimestamp();
    var shelvesetName = "OptData_" + timestamp;

    var username = Env("USERNAME");
    if (username == undefined || username == "")
        username = "clrgnt";

    runCmdToLog("tf shelve /replace /comment:\"Title: New optimization data generated by SNAP job " + 
                snapJobNum + " Integrate into Silverlight: No Risk: Low Checkin tests: No DailyDevRun done. " +
                "Code review: clrpfdev Test review: na Checked in by: " + username + "\" " +
                shelvesetName + " /r " + 
                srcBase + "\\OptimizationData\\x86\\CLR\\Base\\* " +
                srcBase + "\\OptimizationData\\amd64\\CLR\\Base\\*",
                runSetNoThrow());

    var run = runCmdToLog(srcBase + "\\ndp\\clr\\bin\\snap.bat post -c " + shelvesetName + " -silent -high", runOpts);
    if (run.exitCode == 0)
        return true;

    return false;
}

function checkAndSubmitDataToSnap(srcBase, snapPath) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (snapPath == undefined) {
        snapPath = "\\\\clrsnap0\\queues\\tfs_puclr";
    }

    logCall(LogClrAutomation, LogInfo, "checkAndSubmitDataToSnap", arguments, " {");

    for (;;) {

        var jobDirFromSnap = getLatestJobWithOptDataFromSnap(snapPath);

        // If we couldn't read the latest job from SNAP, bail out
        if (jobDirFromSnap == "") 
        {
            logMsg(LogClrAutomation, LogInfo, "WARNING: Could not read latest job from SNAP\n");
            logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
            WScript.Sleep(600 * 1000);
            continue;
        }

        var jobDateFromSnap = undefined;
        try {
            jobDateFromSnap = getOptDataDateFromJobDir(jobDirFromSnap);
        } 
        catch (e) {}

        // If we couldn't get the date of the latest job from SNAP, bail out
        if (jobDateFromSnap == undefined)
        {
            logMsg(LogClrAutomation, LogInfo, "WARNING: Could not get the date of latest job from SNAP\n");
            logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
            WScript.Sleep(600 * 1000);
            continue;
        }

        var jobNumFromSnap = getSnapJobNumFromDir(jobDirFromSnap);

        // If we couldn't get the job number from SNAP, bail out
        if (jobNumFromSnap == "") 
        {
            logMsg(LogClrAutomation, LogInfo, "WARNING: Could not get the job number from SNAP\n");
            logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
            WScript.Sleep(600 * 1000);
            continue;
        }

        var jobDirSubmitted = getLatestJobDirSubmitted(srcBase); 
        var jobDateSubmitted = getLatestJobDateSubmitted(srcBase); 
        var jobNumSubmitted = getSnapJobNumFromDir(jobDirSubmitted);

        if (jobDirSubmitted != "")
        {
            // If the job number from SNAP is equal to latest job number that was
            // submitted, then there is nothing to do
            if (jobDirFromSnap.toLowerCase() == jobDirSubmitted.toLowerCase())
            {
                logMsg(LogClrAutomation, LogInfo, "Job ", jobNumFromSnap, " has already been submitted, nothing to do\n");
                logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
                WScript.Sleep(600 * 1000);
                continue;
            }

            // If the latest data submitted is newer than the latest job from SNAP,
            // then there is nothing to do
            if (new Date(jobDateSubmitted) >= new Date(jobDateFromSnap))
            {
                logMsg(LogClrAutomation, LogInfo, "Job ", jobNumFromSnap, " is older than the latest data submitted (Job ", 
                    jobNumSubmitted, "), nothing to do\n");
                logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
                WScript.Sleep(600 * 1000);
                continue;
            }
        }

        // If the job folder is missing the x86ret binaries, then there is nothing to do
        if (!FSOFolderExists(jobDirFromSnap + "\\x86ret\\bin")) 
        {
            logMsg(LogClrAutomation, LogInfo, "WARNING: Could not find the x86ret binaries in folder ", jobDirFromSnap, "\n");
            logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
            WScript.Sleep(600 * 1000);
            continue;
        }

        // If the job folder is missing the amd64ret binaries, then there is nothing to do
        if (!FSOFolderExists(jobDirFromSnap + "\\amd64ret\\bin")) 
        {
            logMsg(LogClrAutomation, LogInfo, "WARNING: Could not find the amd64ret binaries in folder ", jobDirFromSnap, "\n");
            logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
            WScript.Sleep(600 * 1000);
            continue;
        }

        if (submitDataToSnap(srcBase, jobDirFromSnap)) 
        {
            markLatestDataSubmitted(srcBase, jobDirFromSnap, jobDateFromSnap);
        }
        else 
        {
            logMsg(LogClrAutomation, LogInfo, "ERROR: There was an error submitting to SNAP\n");
        }

        logMsg(LogClrAutomation, LogInfo, "Sleeping for 10 minutes...\n"); 
        WScript.Sleep(600 * 1000);
    }

    logMsg(LogClrAutomation, LogInfo, "} checkAndSubmitDataToSnap\n");
}

function checkOutFile(sourceFile, targetFile, changeNum)
{
    if (sourceFile == undefined)
        throw Error(1, "required arg sourceDir not supplied");
    if (targetFile == undefined)
        throw Error(1, "required arg targetDir not supplied");
    if (changeNum == undefined)
        throw Error(1, "required arg changeNum not supplied");

    logCall(LogClrAutomation, LogInfo, "checkOutFile", arguments, " {");

    if (!FSOFileExists(sourceFile))
        throw new Error(1, "Source file ", sourceFile, " does not exist");
        
    logMsg(LogClrAutomation, LogInfo, "Source File = ", sourceFile, "\n");
    logMsg(LogClrAutomation, LogInfo, "Target File = ", targetFile, "\n");

    if (FSOFileExists(targetFile)) {
        logMsg(LogClrAutomation, LogInfo, "Checking out ", targetFile, "\n");
        sdEdit(targetFile, changeNum);
        FSOCopyFile(sourceFile, targetFile, true);
    }
    else 
    {
        logMsg(LogClrAutomation, LogInfo, "Adding ", targetFile, "\n");
        FSOCopyFile(sourceFile, targetFile);
        sdAdd(targetFile, changeNum); 
    }

    logMsg(LogClrAutomation, LogInfo, "} checkOutFile Success - returning ", changeNum, "\n");

    return changeNum;
}

/***********************************************************************/
/* Given data files in 'sourceDir' that are supposed to replace files
   in the depot at local directory 'targetDir', check out (sd edit or
   sd add) the needed files in 'targetDir' into the change number 'changeNum'
   and copy the new data in.  No submission is actually done.

   Note this routine is not recursive and it will NOT delete existing
   target files that are not in the source directory.
*/

function checkOutDirectory(sourceDir, targetDir, changeNum)
{
    if (sourceDir == undefined)
        throw Error(1, "required arg sourceDir not supplied");
    if (targetDir == undefined)
        throw Error(1, "required arg targetDir not supplied");
    if (changeNum == undefined)
        throw Error(1, "required arg changeNum not supplied");

    if (!FSOFolderExists(sourceDir))
        throw new Error(1, "Source directory ", sourceDir, " does not exist");
    if (!FSOFolderExists(targetDir))
        throw new Error(1, "Target directory ", targetDir, " does not exist");

    logMsg(LogClrAutomation, LogInfo, "Checkin in: ", sourceDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "at the dir: ", targetDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "for checkin number: ", changeNum, "\n");
        
    var sourceFiles = FSOGetFilePattern(sourceDir);
    for (var i = 0; i < sourceFiles.length; i++) {
        var sourceFile = sourceFiles[i];
        sourceFile.match(/([^\\]*)$/);
        var targetFile = targetDir + "\\" + RegExp.$1;
        checkOutFile(sourceFile, targetFile, changeNum);
    }

    return changeNum;
}

function checkOutFileTfs(sourceFile, targetFile)
{
    if (sourceFile == undefined)
        throw Error(1, "required arg sourceDir not supplied");
    if (targetFile == undefined)
        throw Error(1, "required arg targetDir not supplied");

    logCall(LogClrAutomation, LogInfo, "checkOutFileTfs", arguments, " {");

    if (!FSOFileExists(sourceFile))
        throw new Error(1, "Source file ", sourceFile, " does not exist");
        
    logMsg(LogClrAutomation, LogInfo, "Source File = ", sourceFile, "\n");
    logMsg(LogClrAutomation, LogInfo, "Target File = ", targetFile, "\n");

    if (FSOFileExists(targetFile)) {
        logMsg(LogClrAutomation, LogInfo, "Checking out ", targetFile, "\n");
        runCmdToLog("tf edit " + targetFile);
        FSOCopyFile(sourceFile, targetFile, true);
    }
    else 
    {
        logMsg(LogClrAutomation, LogInfo, "Adding ", targetFile, "\n");
        FSOCopyFile(sourceFile, targetFile);
        runCmdToLog("tf add " + targetFile);
    }

    logMsg(LogClrAutomation, LogInfo, "} checkOutFileTfs\n");
}

/***********************************************************************/
/* Given data files in 'sourceDir' that are supposed to replace files
   in the depot at local directory 'targetDir', check out (sd edit or
   sd add) the needed files in 'targetDir'.  No submission is actually done.

   Note this routine is not recursive and it will NOT delete existing
   target files that are not in the source directory.
*/

function checkOutDirectoryTfs(sourceDir, targetDir)
{
    if (sourceDir == undefined)
        throw Error(1, "required arg sourceDir not supplied");
    if (targetDir == undefined)
        throw Error(1, "required arg targetDir not supplied");

    if (!FSOFolderExists(sourceDir))
        throw new Error(1, "Source directory ", sourceDir, " does not exist");
    if (!FSOFolderExists(targetDir))
        throw new Error(1, "Target directory ", targetDir, " does not exist");

    logMsg(LogClrAutomation, LogInfo, "Checkin in: ", sourceDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "at the dir: ", targetDir, "\n");
        
    var sourceFiles = FSOGetFilePattern(sourceDir);
    for (var i = 0; i < sourceFiles.length; i++) {
        var sourceFile = sourceFiles[i];
        sourceFile.match(/([^\\]*)$/);
        var targetFile = targetDir + "\\" + RegExp.$1;
        checkOutFileTfs(sourceFile, targetFile);
    }
}

/******************************************************************************/
function checkOptData(jobDir, srcBase)
{
    if (shouldSnapGenerateOptData())
    {
        logMsg(LogClrAutomation, LogInfo, "New optimization data will be generated\n");
        var genOptDataFile = jobDir + "\\GenOptData.txt";
        var msg = "Generate Optimization Data\r\n";

        try {
            FSOWriteToFile(msg, genOptDataFile, true);
        } 
        catch(e) {
            logMsg(LogClrAutomation, LogInfo, "WARNING: IO exceptions were thrown while writing to GenOptData.txt: ", e.description, "\n");
        }
    }
    else 
    {
        logMsg(LogClrAutomation, LogInfo, "New optimization data will NOT be generated\n");
    }
    
    return 0;
}

function tfApplyChanges(logDir, changesetNumber, shelvesetName, srcBase, pathToTfCmd)
{
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (pathToTfCmd == undefined) {
        pathToTfCmd = srcBase + "\\tools\\x86\\managed\\v4.0\\tf.cmd";
    }

    if (!FSOFileExists(pathToTfCmd)) {
        throwWithStackTrace(new Error(1, "tf.cmd could not be found at " + pathToTfCmd));
    }

    // Based on srcBase, figure out the values for the _NTDRIVE, _NTROOT, and _NTBINDIR environment variables
    if (!srcBase.match(/^([A-Z]\:)/i)) {
        throwWithStackTrace(new Error(1, "Specified srcBase does not start with a drive letter"));
    }
    var ntdriveValue = RegExp.$1;
    var ntrootValue = srcBase.substring(ntdriveValue.length);

    // Set up runOpts for the calls to tf.cmd
    var runOpts = clrRunTemplate;
    runOpts = runSetCwd(srcBase, runOpts);
    runOpts = runSetEnv("SourceControl", "TeamFoundation", runOpts);
    runOpts = runSetEnv("_NTBINDIR", srcBase, runOpts);
    runOpts = runSetEnv("_NTDRIVE", ntdriveValue, runOpts);
    runOpts = runSetEnv("_NTROOT", ntrootValue, runOpts);
    runOpts = runSetNoThrow(runOpts);

    // Set up other local vars
    var run = undefined;
    var changesetSuffix = "";
    if (changesetNumber != undefined)
        changesetSuffix = ";" + changesetNumber;

    // Make sure TF works - retry on failure
    for (;;)
    {
        run = runCmdToLog(pathToTfCmd, runOpts);
        if (run.exitCode != 0)
        {
            logMsg(LogClrAutomation, LogInfo, "tf.cmd returned " + run.exitCode + "; sleeping for 60 seconds and retrying...\n");
            WScript.Sleep(60000);
            continue;
        }

        logMsg(LogClrAutomation, LogInfo, "tf works!\n");
        break;
    }

    // Revert any open files in the enlistment
    for (;;)
    {
        // We might specify nothrow in the runOpts, and then check the return code...
        run = runCmdToLog(pathToTfCmd + " undo " + srcBase + " /noprompt /r", runOpts);

        // TODO: Check for the error code that tf returns when the server is not available
        //       or access denied errors. Otherwise, assume success
        // We cannot check the error code, because tf returns a non-zero error code even
        // when there are no changes to undo. Moreover, the error code is not constant
        // and changes depending on the options supplied to tf undo
        // e.g. tf undo * /r returns a different error code from
        //      tf undo * /noprompt /r

        logMsg(LogClrAutomation, LogInfo, "Revert succeeded!\n");
        break;
    }

    // Sync the enlistment to the specified changeset - retry on failure
    // tf get returns 0 even when all files are up to date
    for (;;)
    {
        // We might specify nothrow in the runOpts, and then check the return code...
        run = runCmdToLog(pathToTfCmd + " get " + srcBase + "\\ndp\\inc\\version\\version.h /noprompt /force", runOpts);

        if (run.exitCode != 0)
        {
            logMsg(LogClrAutomation, LogInfo, "tf returned " + run.exitCode + "; sleeping for 60 seconds and retrying...\n");
            WScript.Sleep(60000);
            continue;
        }

        // Syncing the whole enlistment might be problematic for multiple branch/Q server. We need to review this
        run = runCmdToLog(pathToTfCmd + " get " + srcBase + changesetSuffix + " /noprompt /overwrite /recursive", runOpts);

        if (run.exitCode != 0)
        {
            logMsg(LogClrAutomation, LogInfo, "tf returned " + run.exitCode + "; sleeping for 60 seconds and retrying...\n");
            WScript.Sleep(60000);
            continue;
        }

        logMsg(LogClrAutomation, LogInfo, "Sync succeeded!\n");
        break;
    }

    // If a shelveset was specified, unshelve it and check for version conflicts
    if (shelvesetName != undefined)
    {
        // Unshelve the shelveset
        run = runCmdToLog(pathToTfCmd + " unshelve " + shelvesetName + " /noprompt", runOpts);
        if (run.exitCode != 0)
            throw new Error(1, "tf returned " + run.exitCode + " when unshelving " + shelvesetName);

        // Check for version conflicts
        tfCheckForVersionConflicts(changesetNumber, srcBase, pathToTfCmd);

        logMsg(LogClrAutomation, LogInfo, "Unshelve succeeded!\n");
    }
}

function tfCheckForVersionConflicts(changesetNumber, srcBase, pathToTfCmd)
{
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (pathToTfCmd == undefined) {
        pathToTfCmd = srcBase + "\\tools\\x86\\managed\\v4.0\\tf.cmd";
    }

    if (!FSOFileExists(pathToTfCmd)) {
        throwWithStackTrace(new Error(1, "tf.cmd could not be found at " + pathToTfCmd));
    }

    // Based on srcBase, figure out the values for the _NTDRIVE and _NTROOT environment variables

    if (!srcBase.match(/^([A-Z]\:)/i)) {
        throwWithStackTrace(new Error(1, "Specified srcBase does not start with a drive letter"));
    }
    var ntdriveValue = RegExp.$1;
    var ntrootValue = srcBase.substring(ntdriveValue.length);

    // Set up runOpts for the calls to tf.cmd

    var runOpts = clrRunTemplate;
    runOpts = runSetCwd(srcBase, runOpts);
    runOpts = runSetEnv("SourceControl", "TeamFoundation", runOpts);
    runOpts = runSetEnv("_NTBINDIR", srcBase, runOpts);
    runOpts = runSetEnv("_NTDRIVE", ntdriveValue, runOpts);
    runOpts = runSetEnv("_NTROOT", ntrootValue, runOpts);
    runOpts = runSetNoThrow(runOpts);

    var changesetSuffix = "";
    if (changesetNumber != undefined)
        changesetSuffix = ";" + changesetNumber;

    // sync after applying changes used to be potentially optional with SD, but it always happened redundantly.
    // With TFS, since unshelve rolls back the have version, we must go ahead and force the version conflict now
    // and then auto resolve it with anything else that needs resolving.  This will allow a future
    // tf get to succeed, since if there is a version conflict, there'll be a non-zero errorlevel set.
    // because tfApplyChanges() already ran an explicit tf get /overwrite, we don't need to spend
    // extra time by including /overwrite here.

    logMsg(LogClrAutomation, LogInfo, "Doing a get to force version conflicts, if any...\n");
    runCmdToLog(pathToTfCmd + " get \"" + srcBase + "\"" + changesetSuffix + " /recursive /noprompt", runOpts);
    
    // Resolve any merge conflicts
    logMsg(LogClrAutomation, LogInfo, "Resolving the files (if necessary)\n");
    runCmdToLog(pathToTfCmd + " resolve /auto:AcceptMerge /recursive /noprompt", runOpts);

    logMsg(LogClrAutomation, LogInfo, "Checking for any unresolved files...\n");
    var run = runCmdToLog(pathToTfCmd + " resolve /recursive /noprompt", runOpts);

    // If there is anything left, then it's a conflict
    if (!run.output.match(/There are no conflicts to resolve/))
    {
        throwWithStackTrace(new Error(1, "Possible merge conflict"));
    }

    // At this point we need one last check. If there ISN'T anything open,
    // then something is probably wrong, so we emit a warning.
    run = runCmdToLog(pathToTfCmd + " status /noprompt /recursive", runOpts);

    if (run.output.match(/There are no pending changes/))
    {
        logMsg(LogClrAutomation, LogInfo, "Warning: There don't seem to be any files checked out.\n");
    }
}

/******************************************************************************/
/* 
   This script is no longer functional.

   To submit a performance run, please use the "Private Jobs" tab of the 
   "Retail Lab" page linked at the top of the Snap website.  This will 
   involve running your change through the retail optimization lab before 
   submitting it for a perf run, which will ensure that you get a valid 
   comparison.  For any questions, you can contact 'clrrlnot'.
*/
function perfRunSubmit()
{
    throw new Error("\n\n" + 
   "This script is no longer functional.\n" +
   "\n" +
   "To submit a performance run, please use the 'Private Jobs' tab of the\n" +
   "'Retail Lab' page linked at the top of the Snap website.  This will\n" +
   "involve running your change through the retail optimization lab before\n" +
   "submitting it for a perf run, which will ensure that you get a valid\n" +
   "comparison.  For any questions, you can contact 'clrrlnot'.\n"
   );
}
