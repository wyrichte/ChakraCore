/*****************************************************************************/
/*                               optimizeNdp.js                              */
/*****************************************************************************/

/* Routines for performing working set optimzation on either unmanaged
   dlls (using bbt and/or pogo)  or managed dlls (using ibc) */

/* AUTHOR: Brian Sullivan
   DATE: 1/28/05 */

/******************************************************************************/
var OptimizeModuleDefined = 1;          // Indicate that this module exists

if (!fsoModuleDefined)  throw new Error(1, "Need to include fso.js");
if (!logModuleDefined)  throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined)  throw new Error(1, "Need to include run.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include clrAutomation.js");

/******************************************************************************/
/*****      Script global variables                                       *****/
/******************************************************************************/
var gsDefaultRemoteLocalBase = "C:\\VBL\\remoteLocalBase\\";

var gsIbcEnabled  = true;
var gsPogoEnabled = false;
var gsBbtEnabled  = false;
var gsSwitchMscorwks = true;
var gsZapFileList = undefined;
var gsIbcDataDir  = undefined;

// for tools we always use the x86 one (thus use WOW on 64 bit)
var gsBbtToolsRelPath = "tools\\devdiv\\x86\\bbt\\toolset";

// HACK: we use these global variables to allow some functions to behave
// differently when running in the CLR Retail Lab
var gsRetailLabWorker = false;
var gsLocalBinDir = undefined;

/******************************************************************************/
/******************* Entry points for CLR Retail Lab Tasks ********************/
/******************************************************************************/

function genOptBuild(bldArch, bldType, srcBase, logDir, localBinDir, relUnoptBuildDir, relOptBuildDir, privateJob)
{
    var unoptBuildDir = localBinDir + "\\" + relUnoptBuildDir;
    var optBuildDir = localBinDir + "\\" + relOptBuildDir;

    // Set global variables for the Retail Lab
    gsRetailLabWorker = true;
    gsLocalBinDir = localBinDir;

    var ret = 0;

    // Turn on themes
    {
        var runOpts = clrRunTemplate;
        runOpts = runSetNoThrow(runOpts);
        runCmdToLog(srcBase + "\\OptimizationData\\scripts\\TurnOnThemesWin2003.bat", runOpts);
    }

    // Call retailBuild()
    try
    {
        retailBuild(unoptBuildDir, optBuildDir, bldArch, undefined, logDir, srcBase, undefined,
                    undefined, srcBase, bldType);
    }
    catch (e)
    {
        ret = 1;
        logMsg(LogClrAutomation, LogInfo, "Caught exception thrown by retailBuild():\n" + e.description + "\n");
    }

    // TODO: Turn off themes
    {
        var runOpts = clrRunTemplate;
        runOpts = runSetNoThrow(runOpts);
        runCmdToLog(srcBase + "\\OptimizationData\\scripts\\TurnOffThemesWin2003.bat", runOpts);
    }

    return ret;
}

/******************************************************************************/
/**************************** Utility functions *******************************/
/******************************************************************************/

/******************************************************************************/
/* formats a runjs command line that can be run on a remote machine
 */
function FormatCmdLine(/* vararg arguments[] */)
{
    var fullCmdLine = arguments[0];

    for(var i = 1; i < arguments.length; i++) {
        fullCmdLine = fullCmdLine + " " + arguments[i];
    }

    return fullCmdLine;
}

/******************************************************************************/
/* When running on a remote machine we populate the localBase directory
 * by initializing the localBase from the srcBase
 */
function InitLocalBase(localBase, srcBase, logDir)
{
    FSOCreatePath(localBase);

    robocopy(srcBase   + "\\OptimizationData\\scenarios",
             localBase + "\\OptimizationData\\scenarios",
             "/xo /nfl /ndl",
             logDir + "\\InitLocalBase.log.txt");
}

function SetSystemWideEnvVar(varName, value)
{
    logMsg(LogClrAutomation, LogInfo, "Setting system-wide env var: ", varName, " = ", value, "\n");

    try {
        WshShell.RegDelete(
            "HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\Environment\\" + varName);
    } catch(e) {};

    WshShell.RegWrite(
        "HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\Environment\\" + varName,
        value, "REG_SZ");
}

function ClearSystemWideEnvVar(varName)
{
    logMsg(LogClrAutomation, LogInfo, "Clearing system-wide env var: ", varName, "\n");

    try {
        WshShell.RegDelete(
            "HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\Environment\\" + varName);
    } catch(e) {};
}

function GetSystemWideEnvVar(varName)
{
    logMsg(LogClrAutomation, LogInfo, "Getting system-wide env var: ", varName, "\n");

    var varValue = undefined;
    try {
        varValue = WshShell.RegRead(
            "HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\Environment\\" + varName);
    } catch(e) {};
    return varValue;
}

/******************************************************************************/
/* given an 'exePath', an 'exeCmd' and arguments 'exeArgs', as well as an
   'installDir' that is installed, but not necessarily the default runtime,
   We set 'COMPLUS_DefaultVersion' to use that runtime regardless of anything else
   on the machine.   This would be straightforward except we also want to use
   the mscoree.dll in 'installDir\\isolated' not the one in system32.  To do this, we
   copy the exe (and any associated dlls), to a directory with a *.local file and
   a mscoree.dll to force it to use that mscoree.dll and not the one in system32.
   The upshot of this, is that the executable is run using the provided runtime
   and does not effect and is not affected by any other runtime installed on the
   system
*/
function runOnIsolatedRuntime(bldType, exePath, exeCmd, exeArgs, installDir, runOpts, isGui) {

    logCall(LogClrAutomation, LogInfo, "runOnIsolatedRuntime", arguments);

    if (installDir == undefined)
        throw new Error(1, "Required argument 'installDir' is missing");

    if (!isCoreCLRBuild(bldType) && !installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
        throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");

    var versionDir = RegExp.$2;
    var mscoreeDir = installDir + "\\isolated";

    var commandLine = exeCmd;

    if ((exePath != undefined) && (exePath != ""))
        commandLine = exePath + "\\" + exeCmd;

    // in order for us to use the correct mscoree.dll, we rely on windows DLL redirection.
    // this requires us to place a *.local file for the exe we are running. We don't do
    // this if the exe is in the runtime itself (like ngen.exe)

    // cleanup keeps a list of files that we created so that we can delete them later
    var cleanup = [];
    var targetFile;
    if (FSOFolderExists(mscoreeDir) && (exePath.toLowerCase() != installDir.toLowerCase())) {
        var sourceFile = mscoreeDir + "\\mscoree.dll";
        if (FSOFileExists(sourceFile)) {
            targetFile = exePath + "\\mscoree.dll";
            logMsg(LogClrAutomation, LogInfo, "Copying ", sourceFile, " to ", targetFile, "\n");
            FSOCopyFile(sourceFile, targetFile, true);
            cleanup.push(targetFile);
        }
        targetFile = commandLine + ".local";
        logMsg(LogClrAutomation, LogInfo, "creating ", targetFile, "\n");
        FSOWriteToFile("", targetFile);
        cleanup.push(targetFile);
    }

    if (runOpts == undefined)
        runOpts = clrRunTemplate;

    //Wait up to 5 minutes for the scenario to idle and upto 30mins for scenario to timeout
    runOpts = runSetEnv("COMPLUS_DefaultVersion", versionDir,
              runSetEnv("COMPLUS_Version", versionDir,
              runSetIdleTimeout(5 * MINUTE,
              runSetTimeout(30 * MINUTE,
              runSetLog(LogRun, LogInfo,
              runOpts)))));

    if (exeArgs)
        commandLine += " " + exeArgs;

    logMsg(LogClrAutomation, LogInfo, "runOnIsolatedRuntime: runCmd is ", commandLine, "\n");

    if (isGui) {
        runToIdleAndQuit(commandLine, runOpts);
        // Sleep for 10 seconds to let us finish writing any profile data
        WScript.Sleep(10*1000);
    }
    else {
        runCmd(commandLine, runOpts);
    }

    // If we created a exeCmd.local and put our mscoree.dll in the scenario directory
    // then we have to cleanup after ourselves otherwise the WSPerfTests will fail
    for(var i = 0; i < cleanup.length; i++) {
        var targetFile = cleanup[i];
        logMsg(LogClrAutomation, LogInfo, "deleting ", targetFile, "\n");
        FSODeleteFile(targetFile, "FORCE");
    }
}

/******************************************************************************/
/* SwitchRuntimeDlls:
   Copy the runtime dlls: clr, mscordacwks and clrjit
   found in inDir into installDir

  Parameters
    inDir        : We copy the prejit from here, replacing the one in installDir
    otherDir     : We require that both inDir and otherDir have a prejit
                   before we replace the one in installDir
    installDir   : The path to the installed binaries
    message      : Message to print if we copy the prejit
*/

function SwitchRuntimeDlls(inDir, otherDir, installDir, message)
{
    logCall(LogClrAutomation, LogInfo, "SwitchRuntimeDlls", arguments, " {");

    if (inDir == undefined)
        throw new Error(1, "Required argument 'inDir' is missing");
    if (otherDir == undefined)
        throw new Error(1, "Required argument 'otherDir' is missing");
    if (installDir == undefined)
        throw new Error(1, "Required argument 'installDir' is missing");

    var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    prejit = "clrjit";

    var runtimeDlls = [];
    runtimeDlls.push(prejit);

    if (gsSwitchMscorwks) {
        runtimeDlls.push("clr");
        runtimeDlls.push("mscordacwks");
    }

    for(var i = 0; i < runtimeDlls.length; i++) {
        var dll = runtimeDlls[i];

        var installDll  = installDir + "\\" + dll + ".dll";
        var installPdb  = installDir + "\\" + dll + ".pdb";
        var inDll       = inDir + "\\" + dll + ".dll";
        var inPdb       = inDir + "\\symbols.pri\\retail\\dll\\" + dll + ".pdb";
        var otherDll    = otherDir + "\\" + dll + ".dll";

        if (!FSOFileExists(inDll))
        {
            logMsg(LogClrAutomation, LogInfo, "SwitchRuntimeDlls: ", inDll, " does not exist.\n");
            continue;
        }
        if (!FSOFileExists(otherDll))
        {
            logMsg(LogClrAutomation, LogInfo, "SwitchRuntimeDlls: ", otherDll, " does not exist.\n");
            continue;
        }
        FSOCopyFile(inDll, installDll, "FORCE");

        FSOCopyFileIfExists(inPdb, installPdb, "FORCE");
    }

    logMsg(LogClrAutomation, LogInfo, "SwitchRuntimeDlls: ", message, "\n");

    logMsg(LogClrAutomation, LogInfo, "} SwitchRuntimeDlls\n");
    return 0;
}

function SetMismatchedRuntime() {

    // When starting up the w3wp service we don't have our environment variables set
    // so we set the system-wide registry keys instead.
    if (gsSwitchMscorwks) {
        logMsg(LogClrAutomation, LogInfo, "SetMismatchedRuntime: Setting system-wide registry keys: CheckNGenImageTimeStamp.\n");
        var netFrameworkRoot = GetNetFrameworkRoot();
        WshShell.RegWrite(netFrameworkRoot + "\\CheckNGenImageTimeStamp", 0, "REG_DWORD");
    }
}

function ClearMismatchedRuntime() {

    // Remove the system-wide registry keys that we set above
    //
    logMsg(LogClrAutomation, LogInfo, "ClearMismatchedRuntime: Deleting system-wide registry keys: CheckNGenImageTimeStamp.\n");
    var netFrameworkRoot = GetNetFrameworkRoot();
    try {
        WshShell.RegDelete(netFrameworkRoot + "\\CheckNGenImageTimeStamp");
    } catch(e) {};
}

function TryDeleteFolder(path)
{
    try {
        FSODeleteFolder(path, true);
    } catch(e) {}
}

/******************************************************************************/
/* ProcessProfileData:  Utility function used after running a Pogo, IBC or BBT
                        profile training scenario.  Waits a few seconds then
                        checks for the appropriate profile data file and for
                        pogo or bbt copies it to the outDir and then deletes it

  Parameters
    kind         : The kind of profiling that we are doing: "pogo", "ibc" or "bbt"
    inDir        : The directory that will contain the new profile data file(s)
    outDir       : The directory that we copy the new profile data file(s) to
    infoStr      : A string about the current scenario that is incorporated
                   into the destination file(s).
*/
function ProcessProfileData(kind, inDir, outDir, infoStr)
{
    logCall(LogClrAutomation, LogInfo, "ProcessProfileData", arguments);

    if (kind == "pogo") {
        // Sleep for 10 seconds to let Pogo finish writing its profile data
        WScript.Sleep(10*1000);

        var pgcFiles = FSOGetFilePattern(inDir, /\.pgc$/i);
        for(var i = 0; i < pgcFiles.length; i++)
        {
            var pgcItem = pgcFiles[i];

            if (!pgcItem.match(/(.*)\\(([^\\]*)\!(.+)\.(.+))$/))
                throw Error(1, "RegExp match failed for " + pgcItem);

            var pgcInDir    = RegExp.$1;
            var pgcFile     = RegExp.$2;
            var pgcFileBase = RegExp.$3;
            var pgcFileCnt  = RegExp.$4;
            var pgcFileExt  = RegExp.$5;

            var pgcDestFile = pgcFileBase + "\!" + infoStr + "_" + pgcFileCnt + "." + pgcFileExt;

            var pgcDestPath = outDir + "\\" + pgcDestFile;

            logMsg(LogClrAutomation, LogInfo, "ProcessProfileData: ", pgcDestFile, "\n");

            // Copy the file and then delete it
            FSOCreatePathForFile(pgcDestPath);
            FSOCopyFile(pgcItem, pgcDestPath, true);
            FSODeleteFile(pgcItem, true);
        }
    }
    else if (kind == "ibc") {
        // Sleep for 10 seconds to let IBC finish writing its profile data
        WScript.Sleep(10*1000);

        var ibcFiles = FSOGetFilePattern(outDir, /\.ibc$/i);
        for(var i = 0; i < ibcFiles.length; i++) {
            var ibcFile  = ibcFiles[i];
            var fileInfo = FSOGetFile(ibcFile);

            logMsg(LogClrAutomation, LogInfo, "ProcessProfileData: ", fileInfo.Name + " " + fileInfo.DateLastModified + " " + fileInfo.Size + "\n");

        }

    }
    else if (kind == "bbt") {
        // Sleep for 10 seconds to let BBT finish writing its profile data
        WScript.Sleep(10*1000);

        var idfFiles = FSOGetFilePattern(inDir, /\.idf$/i);
        for(var i = 0; i < idfFiles.length; i++) {
            var idfItem  = idfFiles[i];

            if (!idfItem.match(/(.*)\\(([^\\]*)\!(.+)\.(.+))$/))
                throw Error(1, "RegExp match failed for " + idfItem);

            var idfInDir    = RegExp.$1;
            var idfFile     = RegExp.$2;
            var idfFileBase = RegExp.$3;
            var idfFileCnt  = RegExp.$4;
            var idfFileExt  = RegExp.$5;

            var idfDestFile = idfFileBase + "\!" + infoStr + "_" + idfFileCnt + "." + idfFileExt;

            var idfDestPath = outDir + "\\" + idfDestFile;

            logMsg(LogClrAutomation, LogInfo, "ProcessProfileData: ", idfDestFile, "\n");

            // Copy the file and then delete it
            FSOCreatePathForFile(idfDestPath);
            FSOCopyFile(idfItem, idfDestPath, true);
            FSODeleteFile(idfItem, true);
        }
    }
}

/******************************************************************************/
/* CheckPgoProfileData: Utility function used after running a Pogo profile
                        training scenario. Waits a few seconds then looks
                        for profile data (pgc files), checking to make sure
                        profile data was generated for each module in the
                        speficied module list.

  Parameters
    inDir        : The directory that will contain the new profile data file(s)
    moduleList   : The list of modules that we expect profile data for
*/
function CheckPgoProfileData(inDir, moduleList)
{
    logCall(LogClrAutomation, LogInfo, "CheckPgoProfileData", arguments);

    var moduleLookup = new Array();

    // Sleep for 10 seconds to let Pogo finish writing its profile data
    WScript.Sleep(10*1000);

    var pgcFiles = FSOGetFilePattern(inDir, /\.pgc$/i);
    for(var i = 0; i < pgcFiles.length; i++)
    {
        var pgcItem = pgcFiles[i];

        if (!pgcItem.match(/(.*)\\(([^\\]*)\!(.+)\.(.+))$/))
            throw Error(1, "RegExp match failed for " + pgcItem);

        var moduleName = RegExp.$3;
        moduleName = moduleName.toLowerCase();
        moduleLookup[moduleName] = true;
    }

    var result = true;

    for (var i = 0; i < moduleList.length; i++)
    {
        var moduleName = moduleList[i].toLowerCase();
        if (moduleLookup[moduleName])
        {
            logMsg(LogClrAutomation, LogInfo, "CheckPgoProfileData: Found profile data for ", moduleName, "\n");
        }
        else
        {
            logMsg(LogClrAutomation, LogInfo, "CheckPgoProfileData: Could not find profile data for ", moduleName, "\n");
            result = false;
        }
    }

    return result;
}


/******************************************************************************/
/* CleanupProfileData:  Utility function used to cleanup files after a Pogo, IBC,
                        or BBT profile training scenario fails.  Waits a few
                        seconds, checks for the appropriate profile data files,
                        and then deletes them.

  Parameters
    kind         : The kind of profiling that we are doing: "pogo", "ibc" or "bbt"
    inDir        : The directory that will contain the new profile data file(s)
*/
function CleanupProfileData(kind, inDir)
{
    logCall(LogClrAutomation, LogInfo, "CleanupProfileData", arguments);

    if (kind == "pogo") {
        // Sleep for 20 seconds to let Pogo finish writing its profile data
        WScript.Sleep(20*1000);

        var pgcFiles = FSOGetFilePattern(inDir, /\.pgc$/i);
        for(var i = 0; i < pgcFiles.length; i++)
        {
            var pgcItem = pgcFiles[i];
            logMsg(LogClrAutomation, LogInfo, "CleanupProfileData: ", pgcItem, "\n");
            FSODeleteFile(pgcItem, true);
        }
    }
    else if (kind == "ibc") {
        // Sleep for 20 seconds to let IBC finish writing its profile data
        WScript.Sleep(20*1000);

        var ibcFiles = FSOGetFilePattern(outDir, /\.ibc$/i);
        for(var i = 0; i < ibcFiles.length; i++)
        {
            var ibcFile = ibcFiles[i];
            logMsg(LogClrAutomation, LogInfo, "CleanupProfileData: ", ibcFile, "\n");
            FSODeleteFile(ibcFile, true);
        }

    }
    else if (kind == "bbt") {
        // Sleep for 20 seconds to let BBT finish writing its profile data
        WScript.Sleep(20*1000);

        var idfFiles = FSOGetFilePattern(inDir, /\.idf$/i);
        for(var i = 0; i < idfFiles.length; i++)
        {
            var idfItem = idfFiles[i];
            logMsg(LogClrAutomation, LogInfo, "CleanupProfileData: ", idfItem, "\n");
            FSODeleteFile(idfItem, true);
        }
    }
}



/******************************************************************************/
/* NGenScenarios:  Utility used to ngen all of the scenarios and their dependent
                   system assemblies to prepare for running profiling scenarios
                   Typically this step produces profiling data files, but we
                   don't want to incorporate these profiling data files as part
                   of training data.

  Parameters
    bldType      : The type of build, eg ret, coreret, coreshp
    scenarios    : Scenarios to run, must have already been expanded by scenariosGet
    options      : special options to pass to every ngen command, typically this is ""
    installDir   : The path to the installed runtime
    runOpts      : Options to pass to every 'runCmd (env vars etc)'
    multiproc    : if 'true' then we use the ngen /queue multipeoc option to
                   use all availabel processors to do the ngens in parallel
    kind         : One of "pogo", "bbt" or "ibc"
    profileDataInDir  : The directory that the profile data files are created in
    profileDataOutDir : The directory that we move the profile data files to

  Returns 0 on success
*/
function NgenScenarios(bldType, scenarios, options, installDir, runOpts, multiproc, kind, profileDataInDir, profileDataOutDir) {

    logCall(LogClrAutomation, LogInfo, "NgenScenarios", arguments, " {");

    if (options == undefined)
        options = "";

    var ngenRunOpts = runSetTimeout(120 * MINUTE, runOpts);


    if (profileDataInDir)
        FSOCreatePath(profileDataInDir);

    if (profileDataOutDir)
        FSOCreatePath(profileDataOutDir);

    if ((kind != "ibc")  &&
        (kind != "bbt")  &&
        (kind != "pogo"))
    {
        throw new Error(1, "Invalid value for argument 'kind': " + kind);
    }

    // We explicitly ngen mscorlib first
    var ngenMscorlibArg = "install " + installDir + "\\mscorlib.dll " + options + " /NoDependencies";
    runOnIsolatedRuntime(bldType, installDir, "ngen.exe", ngenMscorlibArg,
                         installDir, ngenRunOpts);
    ProcessProfileData(kind, profileDataInDir, profileDataOutDir, "ngen_mscorlib");

    runOnIsolatedRuntime(bldType, installDir, "ngen.exe", "display",
                         installDir, ngenRunOpts);

    if (multiproc) {

        // enable multiproc ngen compiles
        options = options + " /queue";

        // Pause the ngen service so that it doesn't start up if the system goes idle
        runOnIsolatedRuntime(bldType, installDir, "ngen.exe", "queue pause",
                             installDir, ngenRunOpts);
    }

    for (var i = 0; i < scenarios.length; i++) {
        var scenario = scenarios[i];

        // Is this scenario selected as part of the ibc, bbt or pogo training?
        if ( scenario.categories.match(new RegExp("\\b" + kind + "\\b"), "i"))
        {
            // ngen the explicit scenario dependant system dlls, if specified
            if (scenario.depends) {
                // We will explicitly ngen the dependant system dlls
                for (var k = 0; k < scenario.depends.length; k++) {
                    var ngenDependsArg = "install " + installDir + "\\" + scenario.depends[k] + ".dll " + options + " /NoDependencies";
                    runOnIsolatedRuntime(bldType, installDir, "ngen.exe", ngenDependsArg,
                                         installDir, ngenRunOpts);
                    if (!multiproc)
                        ProcessProfileData(kind, profileDataInDir, profileDataOutDir, "ngen_" + scenario.depends[k] + "_for_" + scenario.name);
                }
            }

            // ngen the scenario exe and dlls
            for (var j = 0; j < scenario.ngen.length; j++) {
                var ngenScenarioArg = "install " + scenario.ngen[j] + " " + options;
                // If explicit depends were specified then we add /NoDependencies
                if (scenario.depends) {
                    ngenScenarioArg = ngenScenarioArg + " /NoDependencies";
                }
                // Set the current working directory to the scenario path
                logMsg(LogClrAutomation, LogInfo, "setCwd:", scenario.path, "\n");
                var scenarioRunOpts = runSetCwd(scenario.path, ngenRunOpts);
                runOnIsolatedRuntime(bldType, installDir, "ngen.exe", ngenScenarioArg,
                                     installDir, scenarioRunOpts);
                if (!multiproc)
                    ProcessProfileData(kind, profileDataInDir, profileDataOutDir, "ngen_" + scenario.ngen[j] + "_for_" + scenario.name);
            }
        }
    }

    // If multiproc is true then execute all of the multiproc ngen compiles
    if (multiproc) {
        runOnIsolatedRuntime(bldType, installDir, "ngen.exe", "display",
                             installDir, ngenRunOpts);

        ngenRunOpts = runSetEnv("COMPLUS_EnableMultiproc", 1, ngenRunOpts);

        runOnIsolatedRuntime(bldType, installDir, "ngen.exe", "executeQueuedItems",
                             installDir, ngenRunOpts);
        ProcessProfileData(kind, profileDataInDir, profileDataOutDir, "ngen_executeQueuedItems");

        // unpause the ngen service
        runOnIsolatedRuntime(bldType, installDir, "ngen.exe", "queue continue",
                             installDir, ngenRunOpts);
    }

    runOnIsolatedRuntime(bldType, installDir, "ngen.exe", "display",
                         installDir, ngenRunOpts);

    logMsg(LogClrAutomation, LogInfo, "} NgenScenarios\n");
    return 0;
}


/******************************************************************************/
/* RunScenarios:   Utility used to run all of the scenarios and gather their
                   profiling data files.

  Parameters
    bldType      : The type of build, eg ret, coreret, coreshp
    scenarios    : Scenarios to run, must have already been expanded by scenariosGet
    kind         : One of "pogo", "bbt" or "ibc"
    installDir   : The path to the installed runtime
    runOpts      : Options to pass to every 'runCmd (env vars etc)'
    profileDataInDir       : The directory that the profile data files are created in
    profileDataScenarioDir : The directory that we move the important profile data files to
    profileDataOtherDir    : The directory that ww move the unimportant profile data files to

  Returns 0 on success
*/
function RunScenarios(bldType, scenarios, kind, installDir, runOpts, profileDataInDir, profileDataScenarioDir, profileDataOtherDir) {

    logCall(LogClrAutomation, LogInfo, "RunScenarios", arguments, " {");

    if (kind == undefined)
        throw new Error(1, "Required argument 'kind' is missing");
    if (installDir == undefined)
        throw new Error(1, "Required argument 'installDir' is missing");
    if (scenarios == undefined)
        scenarios = scenariosGet();
    if (runOpts == undefined)
        runOpts = clrRunTemplate;

    if (!isCoreCLRBuild(bldType) && !installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
        throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");

    var versionDir = RegExp.$2;

    if (profileDataInDir)
        FSOCreatePath(profileDataInDir);

    if (profileDataScenarioDir)
        FSOCreatePath(profileDataScenarioDir);

    if (profileDataOtherDir)
        FSOCreatePath(profileDataOtherDir);

    var matchExpr;

    kind = kind.toLowerCase();

    if ((kind != "ibc")  &&
        (kind != "bbt")  &&
        (kind != "pogo"))
    {
        throw new Error(1, "Invalid value for argument 'kind': " + kind);
    }

    for (var i = 0; i < scenarios.length; i++) {
        var scenario = scenarios[i];

        // Is this scenario selected as part of the ibc, bbt or pogo training?
        if ( scenario.categories.match(new RegExp("\\b" + kind + "\\b"), "i"))
        {
            // Set the current working directory to the scenario path
            var scenarioRunOpts = runSetCwd(scenario.path, runOpts);

            logMsg(LogClrAutomation, LogInfo, "RunScenarios: running ", scenario.name, "{\n");

            var exeOrCmd = scenario.exe;
            if (scenario.cmd)
                exeOrCmd = scenario.cmd;

            if (scenario.envVar != undefined) {
                scenarioRunOpts = runSetEnv(scenario.envVar, scenario.envVal, scenarioRunOpts);
            }

            if (kind == "ibc") {
                if (scenario.ibcRuns > 0) {
                    // The IBuySpy scenario has a setup phase
                    if (scenario.setup_args) {

                        runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.setup_args,
                                             installDir, scenarioRunOpts, scenario.gui);
                        ProcessProfileData(kind, profileDataInDir, profileDataOtherDir, "setup_" + scenario.name);
                    }

                    for (var j = 0; j < scenario.ibcRuns; j++) {
                        // The IBuySpy scenario has a start phase
                        if (scenario.start_args) {
                            try {
                                // Set the ZapBBInstr and ZapBBInstrDir registry keys if we are running IBuySpy
                                SetZapBBInstr();

                                // This starts up IIS which reads the ZapBBInstr and ZapBBInstrDir registry keys
                                runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.start_args,
                                                     installDir, scenarioRunOpts, scenario.gui);

                                // Clear the ZapBBInstr and ZapBBInstrDir registry keys if we were running IBuySpy
                                ClearZapBBInstr();

                                ProcessProfileData(kind, profileDataInDir, profileDataOtherDir, "start_" + scenario.name + j);

                            } catch (e) {
                                // Clear the ZapBBInstr and ZapBBInstrDir registry keys if we were running IBuySpy
                                ClearZapBBInstr();

                                throwWithStackTrace(e);
                            }
                        }

                        // Most scenarios just have a run phase
                        runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.args,
                                             installDir, scenarioRunOpts, scenario.gui);
                        ProcessProfileData(kind, profileDataInDir, profileDataScenarioDir, "run_" + scenario.name + j);
                    }
                }
            }
            else if ((kind == "bbt") || (kind == "pogo")) {

                if ((scenario.wksRuns > 0) || (scenario.svrRuns > 0)) {

                    // The IBuySpy scenario has a setup phase
                    if (scenario.setup_args) {
                        runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.setup_args,
                                             installDir, scenarioRunOpts, scenario.gui);
                        ProcessProfileData(kind, profileDataInDir, profileDataOtherDir, "setup_" + scenario.name);
                    }

                    var wksRunOpts = runSetEnv("COMPLUS_BUILDFLAVOR", "WKS", scenarioRunOpts);
                    var svrRunOpts = runSetEnv("COMPLUS_BUILDFLAVOR", "SVR", scenarioRunOpts);

                    // There is a bunch of additional logic below for retrying a scenario
                    // several times when a scenario fails. This is a workaround for a
                    // known nondeterministic bug in the PGO instrumentation that causes
                    // spurious access violations. See VSW 610921.

                    for (var j = 0; j < scenario.wksRuns; j++) {

                        var scenarioSucceeded = false;
                        var maxTries = 10;
                        var numTries = 0;
                        while ((numTries < maxTries) && (!scenarioSucceeded)) {
                            ++numTries;
                            var setupStepSucceeded = true;
                            var runStepSucceeded = true;
                            // The IBuySpy scenario has a start phase
                            if (scenario.start_args) {

                                SetSystemWideEnvVar("COMPLUS_DefaultVersion", versionDir);

                                try {
                                    runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.start_args,
                                                         installDir, wksRunOpts, scenario.gui);
                                } catch (e) {
                                    CleanupProfileData(kind, profileDataInDir);
                                    if (numTries >= maxTries)
                                    {
                                        if (scenario.start_args)
                                            ClearSystemWideEnvVar("COMPLUS_DefaultVersion");
                                        throw e;
                                    }
                                    setupStepSucceeded = false;
                                }

                                // If the setup failed, retry
                                if (!setupStepSucceeded) {
                                    logMsg(LogClrAutomation, LogInfo, "Failure while running scenario ", scenario.name, "; retrying...\n");
                                    if (scenario.start_args)
                                        ClearSystemWideEnvVar("COMPLUS_DefaultVersion");
                                    continue;
                                }
                                ProcessProfileData(kind, profileDataInDir, profileDataOtherDir, "start_" + scenario.name + "_WKS" + j);
                            }
                            // Most scenarios just have a run phase
                            try {
                                runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.args,
                                                     installDir, wksRunOpts, scenario.gui);
                            } catch (e) {
                                CleanupProfileData(kind, profileDataInDir);
                                if (numTries >= maxTries)
                                {
                                    if (scenario.start_args)
                                        ClearSystemWideEnvVar("COMPLUS_DefaultVersion");
                                    throw e;
                                }
                                runStepSucceeded = false;
                            }

                            if (scenario.start_args)
                                ClearSystemWideEnvVar("COMPLUS_DefaultVersion");

                            // If the run failed, retry
                            if (!runStepSucceeded) {
                                logMsg(LogClrAutomation, LogInfo, "Failure while running scenario ", scenario.name, "; retrying...\n");
                                continue;
                            }

                            scenarioSucceeded = true;
                            if (scenario.pgoModuleList && kind == "pogo")
                            {
                                if (!CheckPgoProfileData(profileDataInDir, scenario.pgoModuleList))
                                    scenarioSucceeded = false;
                            }

                            ProcessProfileData(kind, profileDataInDir, profileDataScenarioDir, "run_" + scenario.name + "_WKS" + j);
                        }
                        if (!scenarioSucceeded)
                            throw new Error(1, "Failure while running scenario " + scenario.name + "; giving up!!!\n");
                    }

                    for (var j = 0; j < scenario.svrRuns; j++) {

                        var scenarioSucceeded = false;
                        var maxTries = 10;
                        var numTries = 0;
                        while ((numTries < maxTries) && (!scenarioSucceeded)) {
                            ++numTries;
                            var setupStepSucceeded = true;
                            var runStepSucceeded = true;
                            // The IBuySpy scenario has a start phase
                            if (scenario.start_args) {

                                SetSystemWideEnvVar("COMPLUS_DefaultVersion", versionDir);

                                try {
                                    runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.start_args,
                                                         installDir, svrRunOpts, scenario.gui);
                                } catch (e) {
                                    CleanupProfileData(kind, profileDataInDir);
                                    if (numTries >= maxTries)
                                    {
                                        if (scenario.start_args)
                                            ClearSystemWideEnvVar("COMPLUS_DefaultVersion");
                                        throw e;
                                    }
                                    setupStepSucceeded = false;
                                }
                                // If the setup failed, retry
                                if (!setupStepSucceeded) {
                                    logMsg(LogClrAutomation, LogInfo, "Failure while running scenario ", scenario.name, "; retrying...\n");
                                    if (scenario.start_args)
                                        ClearSystemWideEnvVar("COMPLUS_DefaultVersion");
                                    continue;
                                }
                                ProcessProfileData(kind, profileDataInDir, profileDataOtherDir, "start_" + scenario.name + "_SVR" + j);
                            }
                            // Most scenarios just have a run phase
                            try {
                                runOnIsolatedRuntime(bldType, scenario.path, exeOrCmd, scenario.args,
                                                     installDir, svrRunOpts, scenario.gui);
                            } catch (e) {
                                CleanupProfileData(kind, profileDataInDir);
                                if (numTries >= maxTries)
                                {
                                    if (scenario.start_args)
                                        ClearSystemWideEnvVar("COMPLUS_DefaultVersion");
                                    throw e;
                                }
                                runStepSucceeded = false;
                            }

                            if (scenario.start_args)
                                ClearSystemWideEnvVar("COMPLUS_DefaultVersion");

                            // If the run failed, retry
                            if (!runStepSucceeded) {
                                logMsg(LogClrAutomation, LogInfo, "Failure while running scenario ", scenario.name, "; retrying...\n");
                                continue;
                            }

                            scenarioSucceeded = true;
                            if (scenario.pgoModuleList && kind == "pogo")
                            {
                                if (!CheckPgoProfileData(profileDataInDir, scenario.pgoModuleList))
                                    scenarioSucceeded = false;
                            }

                            ProcessProfileData(kind, profileDataInDir, profileDataScenarioDir, "run_" + scenario.name + "_SVR" + j);
                        }
                        if (!scenarioSucceeded)
                            throw new Error(1, "Failure while running scenario " + scenario.name + "; giving up!!!\n");
                    }
                }
            }

            logMsg(LogClrAutomation, LogInfo, "} DONE Running Scenario: ", scenario.name, "\n");
        }
    }
    logMsg(LogClrAutomation, LogInfo, "} RunScenarios\n");
    return 0;
}

/******************************************************************************/
/* given a runtime build in 'inDir', create the mscordac* dlls in 'optDir'
   that are consistant with the clr in 'outDir', and place them in
   'outDir'.  'srcBase is the base of the VBL (needed to get tools)
    targetArch   : The target archecture (defaults to the current processor architecture)
*/

function updateDacDlls(inDir, outDir, logDir, srcBase, targetArch, bldType) {

    logCall(LogClrAutomation, LogInfo, "updateDacDlls", arguments, " {");

    if (inDir == undefined)
        throw new Error(1, "Required argument 'inDir' is missing");

    if (outDir == undefined)
        throw new Error(1, "Required argument 'outDir' is missing");

    if (logDir == undefined)
        logDir = outDir;

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    logMsg(LogClrAutomation, LogInfo, "updateDacDlls: updating mscordac* dlls to match updated clr.dll\n");

    var perl = srcBase + "\\Tools\\perl\\bin\\perl.exe";
    var devDivTools = srcBase + "\\Tools\\devdiv";
    var devDivToolsBin = devDivTools + "\\x86";                // we will use x86 tools even on 64 bit
    var dacTableGen = devDivToolsBin + "\\dactablegen.exe";
    var dacTableApply = devDivToolsBin + "\\dactableapply.exe";
    var dacUpdate = srcBase + "\\ndp\\clr\\src\\DacUpdateDll\\update.pl";
    var injectResource = devDivToolsBin + "\\InjectResource.exe";
    var genClrDebugResource = devDivToolsBin + "\\GenClrDebugResource.exe";
    var clrDebugResourceName = "CLRDEBUGINFO";

    // Find the right clr.pdb file to use.
    // The BBT instrumentation process puts the updated PDB next to the binaries (leaving the original,
    // now stale one in Symbols.pri).  So if one exists there, use it.  However, this function may also
    // be called in other cases, so fall back to looking for Symbols.pri.
    clrPdb = outDir + "\\" + CLRDllName(bldType) + ".pdb";
    if (!FSOFileExists(clrPdb)) {
        var clrPdb = outDir + "\\Symbols.Pri\\retail\\dll\\" + CLRDllName(bldType) + ".pdb";
        if (!FSOFileExists(clrPdb))
            throwWithStackTrace(Error(1, "updateDacDll: could not find " + CLRDllName(bldType) + ".pdb at " + outDir + " or " + outDir + "\\Symbols.Pri\\retail\\dll"));
    }

    var clrDll = outDir + "\\" + CLRDllName(bldType) + ".dll";
    if (!FSOFileExists(clrDll)) {
        throwWithStackTrace(Error(1, "updateDacDll: could not find " + CLRDllName(bldType) + ".dll at " + outDir));
    }

    var mscorDbiDll = inDir + "\\mscordbi.dll";
    if (!FSOFileExists(clrDll)) {
        throwWithStackTrace(Error(1, "updateDacDll: could not find mscordbi.dll at " + inDir));
    }

        // use the runtime checked into to toolset (and run under wow if on 64 bit)
    var runOpts = runSetEnv("COMPLUS_InstallRoot", srcBase + "\\tools\\x86\\managed",
                  runSetEnv("COMPLUS_DefaultVersion", "v4.0",
                  runSetEnv("COMPLUS_Version", "v4.0",
                  clrRunTemplate)));

    var systemDir = Env("WINDIR");
    if (NeedToEnterWow64(targetArch)) {
        systemDir += "\\SysWow64";
    }
    else {
        systemDir += "\\System32";
    }

    //
    // Normally there will already be a mscoree.dll in the systemDir and we don't install this new one
    //
    var mscoree = systemDir + "\\mscoree.dll";
    if (!FSOFileExists(mscoree)) {
        logMsg(LogClrAutomation, LogInfo, "updateDacDlls: Installing ", mscoree, " so we can run managed code\n");
        FSOCopyFile(srcBase + "\\Tools\\x86\\managed\\v2.0\\shim\\mscoree.dll", mscoree);
    }


    // update the DAC dll with data from the optimized CLR dll

    runCmdToLog(dacTableGen + " " +
                    "/dac:" + inDir + "\\int_tools\\daccess.i " +       // input
                    "/pdb:" + clrPdb + " " +                       // input
                    "/dll:" + clrDll + " " +                       // input
                    "/bin:" + outDir + "\\wks.bin",                     // output
        runSetOutput(outDir + "\\wks.bin.log",
        runSetDump("runjs dumpProc %p " + logDir + "\\dacTableGen.dmpRpt",
        runSetIdleTimeout(60,
        runOpts))));


    var sPathToVersionH =  inDir + "\\Version\\oldvscommonversioninc.h";
    runCmdToLog(perl + " -I" + devDivTools + " " + dacUpdate + " " +
                     inDir + "\\" + DACDllName(bldType) + ".dll " +     // srcPath
                     DACDllName(bldType) + " " +                        // destName
                     targetArch + " " +                                 // host arch
                     targetArch + " " +                                 // target arch
                     sPathToVersionH + " " +                            // Ver
                     outDir + " " +                                     // destDir
                     dacTableApply + " /bin:" + outDir + "\\wks.bin",   // applyCmd
        runOpts);

    FSOCopyFile(inDir + "\\" + DACDllName(bldType) + ".dll", outDir + "\\" + DACDllName(bldType) + ".dll", true);
    runCmdToLog(dacTableApply + " " +
                    "/bin:" + outDir + "\\wks.bin " +                    // input
                    "/dll:" + outDir + "\\" + DACDllName(bldType) + ".dll",                // output
        runSetDump("runjs dumpProc %p " + logDir + "\\dacTableApply.dmpRpt",
        runSetIdleTimeout(60,
        runOpts)));


    // update the CLR dll with data from the new DAC dll

    runCmdToLog(genClrDebugResource + " " +
                    "/out:" + outDir + "\\clrDebugResource.bin " +             // output
                    "/dac:" + outDir + "\\" + DACDllName(bldType) + ".dll " +  // input
                    "/dbi:" + mscorDbiDll + " ",                               // input
        runSetOutput(outDir + "\\ClrGenDebugResource.log",
        runSetDump("runjs dumpProc %p " + logDir + "\\ClrGenDebugResource.dmpRpt",
        runSetIdleTimeout(60,
        runOpts))));

    runCmdToLog(injectResource + " " +
                    "/bin:" + outDir + "\\clrDebugResource.bin " +  // input
                    "/dll:" + clrDll + " " +                        // input/output
                    "/name:" + clrDebugResourceName,                // input
        runSetOutput(outDir + "\\InjectResource.log",
        runSetDump("runjs dumpProc %p " + logDir + "\\InjectResource.dmpRpt",
        runSetIdleTimeout(60,
        runOpts))));


    logMsg(LogClrAutomation, LogInfo, "} updateDacDlls SUCCESSFUL\n");
}

/******************************************************************************/
/* Remove the runtime that was installed in installDir.
   We do this to cleanup the system so that we don't leave the private runtime
   that we installed on the machine after the retail build completes.

  Parameters
    bldType      : The type of build, eg ret, coreret, coreshp
    installDir       : The directory where the runtime is installed
    aspnet_uninstall : Boolean flag that indicates whether ASP.NET uninstall
                       should be performed
    logDir           : Where to put the log files
*/
function UninstallRuntime(bldType, installDir, aspnet_uninstall, logDir, srcBase, arch) {
    if (isCoreCLRBuild(bldType)) {
        return 0;
    }
    if (logDir != undefined) {
        logCall(LogClrAutomation, LogInfo, "UninstallRuntime", arguments, " {");
    }

    if (installDir == undefined) {
        throw new Error(1, "Required argument 'installDir' is not set");
    }

    if (aspnet_uninstall == undefined) {
        aspnet_uninstall = "false";
    }

    if (aspnet_uninstall != "false")
    {
        var runOpts = clrRunTemplate;

        // Run the AspNet uninstaller first before deleting the installation directory
        runOnIsolatedRuntime(bldType, installDir, "aspnet_regiis.exe", "-u", installDir);
    }
    //stop the services which might hold onto the runtime dlls
    // for now, simple net stop of all know services that use .NETFramework
    //get the name of the .net runtime optimization service
    var records = installDir.split("\\");
    var folderName = records[records.length - 1];
    var NdpNgenServiceName = "clr_optimization_" + folderName + "_";
    if (arch == "x86")
        NdpNgenServiceName+="32";
    else
        NdpNgenServiceName+="64";
    logMsg(LogClrAutomation, LogInfo, "Ngen service Name ", NdpNgenServiceName, "\n");
    var services= new Array("aspnet_state",  "iisadmin", "mdm", "mdmd", "msdtc", "NetTcpPortSharing", "WPFFontCache_v0400", NdpNgenServiceName);
    var runOpts = runSetTimeout(60, runSetNoThrow());

    for (var i = 0; i < services.length; i++) {
        runCmdToLog("net stop " + services[i], runOpts);
    }
    var killPath = srcBase + "tools\\" + arch + "\\kill.exe";
    var exes= new Array("mdm.exe", "inetinfo.exe", "aspnet_wp.exe",
                "aspnet_state.exe", "msdtc.exe", "SMSvcHost.exe", "mscorsvw.exe", "WPFFontCache_v0400.exe");

    for (var i = 0; i < exes.length; i++) {
        runCmdToLog(killPath + " " + exes[i], runOpts);
        runCmdToLog(killPath + " -f " + exes[i], runOpts);
    }

    logMsg(LogClrAutomation, LogInfo, "UninstallRuntime: Removing directory ", installDir, "\n");

    var retriesLeft = 3;  //try 3 times before failing
    do {
        try {
            FSODeleteFolder(installDir, true);     // We just delete the installDir folder
            retriesLeft = 0;
        } catch(e) {
            retriesLeft--;
            logMsg(LogClrAutomation, LogWarn, "UninstallRuntime: could not delete " + installDir + "\n");

            // File lock issue on our last try.  Collect diagnostic information.
            if ((e.number+0x100000000) == 0x800a0046 && retriesLeft <= 0)
            {
                logMsg(LogClrAutomation, LogInfo, "The directory can not be deleted, possibly because another process has it locked\n")
                logMsg(LogClrAutomation, LogInfo, "Processes currently running\n")
                procTreePrint();
                logMsg(LogClrAutomation, LogInfo, "Running handle.exe to see what things have locked ", installDir, "\n");
                runCmdToLog("handle.exe " + installDir, runSetNoThrow(runSetTimeout(60)));
                }
            if (retriesLeft > 0) {
                logMsg(LogClrAutomation, LogInfo, "UninstallRuntime: Sleeping for 2 seconds before retrying FSODeleteFolder...\n");
                WScript.Sleep(2*1000); // sleep a couple secs and try again
            }
            else {
                logMsg(LogClrAutomation, LogError, "UninstallRuntime: failed several retries. Re-throwing exception to terminate task:\n    "+ e.description + "\n");
                throwWithStackTrace(e); // rethrow exception to fail task
            }
        }
    }while (retriesLeft > 0);

    if (logDir != undefined) {
        logMsg(LogClrAutomation, LogInfo, "} UninstallRuntime Success\n");
    }
    return 0;
}


/******************************************************************************/
/**************************** BBT OPTIMIZATION ********************************/
/******************************************************************************/

/******************************************************************************/
/* BbtGetRuntimeDllsToOptimize based on the targetArch this function returns
   the set of unmanaged dlls that that we should perform BBT optimizations on

  Parameters
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
*/
function BbtGetRuntimeDllsToOptimize(targetArch, bldType) {

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (isCoreCLRBuild(bldType)) {
        runtimeDlls = [ "coreclr.dll"];
        return runtimeDlls;
    }

    runtimeDlls = [ "clr.dll"];

    if (targetArch.match(/^(x86)/i)) {
        runtimeDlls.push("clrjit.dll");
        runtimeDlls.push("nlssorting.dll");
        runtimeDlls.push("culture.dll");
        runtimeDlls.push("webengine4.dll");
        runtimeDlls.push("aspnet_filter.dll");
        runtimeDlls.push("mscoreei.dll");
        runtimeDlls.push("msvcr110_clr0400.dll");
        // Currently no x86 BBT training scenarios loads mscorpe.dll  (IBuySpy)
        //
        // If it is included here BBT will fail with:
        //    BBOPT: fatal error: BB0192: The instrumentation data file 'mscorpe!*.idf' does not exist.
        //
        // runtimeDlls.push("mscorpe.dll");

        // Currently we cannot train mscoree.dll in Snap because we need to
        // run scenarios that assume a v2.0.50727 default runtime.
        // Instead we gather the data offline and use a checked-in mscoree.zip
    }
    else if (targetArch.match(/^(amd64)/i)) {
        runtimeDlls.push("clrjit.dll");
        runtimeDlls.push("nlssorting.dll");
        runtimeDlls.push("culture.dll");
        runtimeDlls.push("webengine4.dll");
        runtimeDlls.push("aspnet_filter.dll");
        runtimeDlls.push("mscoreei.dll");
        runtimeDlls.push("msvcr110_clr0400.dll");
    }
    else if (targetArch.match(/^(ia64)/i)) {
        runtimeDlls.push("mscoree.dll");
        runtimeDlls.push("clrjit.dll");
        runtimeDlls.push("mscorpe.dll");
    }

    return runtimeDlls;
}

/******************************************************************************/
/* BbtProfile:  Call OptProf to run the BBT scenarios

  Parameters
    inDir        : the starting unoptimize retail build.
                   We copy the prejit dll from here to use during the ngen step
    optDir       : the path to use for the bbt optimized build
    installDir   : The directory where the runtime is installed
    logDir       : Where to put the log files
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
*/

function BbtProfile(inDir, optDir, installDir, logDir, srcBase, targetArch, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtProfile", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    var scenarioSpec = "\"/scenarios:CLRClient_Native,CLRServer_Native\"";
    var arch = targetArch;
    var iarch = arch;
    if (iarch == "x86")
        iarch = "i386";
    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + arch + bldType;
    if (optDir == undefined)
        optDir = srcBase + "\\binaries\\" + arch + bldType + ".opt";
    var bbtDataDir = optDir + "\\BbtData";
    if (logDir == undefined)
        logDir = optDir + "\\BbtLogs\\Profile";
    if (installDir == undefined)
        throw new Error(1, "Required argument 'installDir' is missing");

    var versionDir = undefined;
    var buildNumber = undefined;
    if (!isCoreCLRBuild(bldType)) {
        if (!installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
            throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");
        versionDir = RegExp.$2;
        buildNumber = versionDir.split(".")[2];
    }

    if (FSOFolderExists(bbtDataDir))
        FSODeleteFolder(bbtDataDir, true);
    FSOCreatePath(bbtDataDir);

    // Install IIS for ASP.NET scenarios
    SetupIIS(arch, installDir, srcBase);

    var optProfErrFile = logDir + "\\optProf.err";
    if (FSOFileExists(optProfErrFile))
        FSODeleteFile(optProfErrFile);

    var devDivTools = srcBase + "\\Tools\\devdiv";
    var optProfExe = devDivTools + "\\OptProf\\OptProf.exe";
    var optProfDataDir = bbtDataDir;

    // It's not good to set OptProf's "Work" dir and "Temp" dir to be
    // within logDir, since "Work" dir and "Temp" dir can be quite large,
    // and this eats up hard drive space on clrsnap0
    var optProfWorkDir = logDir + "\\Work";
    var optProfTempDir = optProfWorkDir + "\\Temp";

    // It is better to set OptProf's "Work" dir and "Temp" dir to be 
    // somewhere on the local machine
    if (gsRetailLabWorker)
    {
        optProfWorkDir = gsLocalBinDir + "\\BbtProfile\\Work";
        optProfTempDir = optProfWorkDir + "\\Temp";
    }

    var optProfConfigFile = srcBase + "\\tools\\devdiv\\Optprof\\Netfx." + arch + ".Optprof.xml";
    if (FSOFolderExists(optProfWorkDir))
        FSODeleteFolder(optProfWorkDir, true);
    FSOCreatePath(optProfTempDir);

    var runOpts = runSetTimeout(120 * MINUTE,
                  runSetIdleTimeout(5 * MINUTE,
                  runSetDump("runjs dumpProc %p " + logDir + "\\bbtProfile.dmpRpt",
                  runSetEnv("COMPLUS_EnableMultiproc", "1",
                  runSetEnv("DEVDIV_TOOLS", devDivTools,
                  runSetEnv("OptProf_PreBbtBin", inDir + "\\unopt",
                  runSetEnv("OptProf_Target", arch,
                  runSetEnv("SQLSERVER", "CLRSNAP0",
                  runSetEnv("_PERFTRUN_ROOTDIR", srcBase,
                  runSetEnv("_NTBINDIR", srcBase,
                  runSetEnv("_NTTREE", inDir,
                  runSetEnv("DD_NdpVersion", versionDir,
                  runSetEnv("DD_NDPINSTALLPATH", installDir,
                  runSetEnv("DD_NDPSDKINSTALLPATH", installDir + "\\sdk",
                  runSetEnv("DD_TargetDir", iarch,
                  runSetEnv("_BuildArch", arch,
                  clrRunTemplate))))))))))))))));

    // Make certain the WPF font cache is running and using our runtime
    StartServiceWithEnvironment("FontCache3.0.0.0", "COMPLUS_Version=" + versionDir + ",COMPLUS_InstallRoot=" + installDir.substring(0, installDir.lastIndexOf('\\')));

    // It is REALLY important that the "CheckNGenImageTimeStamp" environment variable is
    // set. If it not set, then EVIL things will happen when OptProf runs a BBT scenario:
    // OptProf will ngen the scenario with the non-instrumented binaries, swap in the
    // instrumented binaries, and then run the scenario; however the scenario will look
    // at the NGen images and determine that they are invalid, and so the scenario will
    // start jitting. Since the scenario runs without crashing, it will produce valid
    // but poor profile data. The end result is that the "optimized" binaries that get
    // produced will have very poor perf numbers (clr will have significantly higher
    // working set; for more info see DDB 155787)

    runOpts = runSetEnv("COMPLUS_CheckNGenImageTimeStamp", "0", runOpts);

    var optProfCmdLine = optProfExe + " /train /engines:BBT " + scenarioSpec + " " +
        "\"/data:" + optProfDataDir + "\" " +
        "\"/work:" + optProfWorkDir + "\" " +
        "\"/log:" + logDir + "\" " +
        "\"/config:" + optProfConfigFile + "\"";

    try {

        // Call OptProf
        runCmdToLog(optProfCmdLine, runOpts);

        logMsg(LogClrAutomation, LogInfo, "OptProf log file at ", logDir, "\\OptProf.log\n");

    } finally {
        // Make sure the ZapBBInstr reg key is cleared
        ClearZapBBInstr();
    }

    // Cloned from IbcProfile() - don't think we need this right now, but
    // it would be nice; the current options to not copy recursively, which
    // is good because I don't want to copy "CLR\Base"... however, the current
    // options will cause _README.html to get copied... maybe we should only
    // copy zip files... hmph...
    //var bbtCheckedIn = srcBase + "\\OptimizationData\\" + arch;
    //logMsg(LogClrAutomation, LogInfo, "Copying in the rest of the checked in profile data" + bbtCheckedIn + "\n");
    //robocopy(bbtCheckedIn, bbtDataDir, "/xo /nfl /ndl", logDir + "\\profileDataCopy.log.txt");

    if (FSOFileExists(optProfErrFile))
        throw new Error(1, "OptProf tool failed, see " + optProfErrFile + " for details");

    logMsg(LogClrAutomation, LogInfo, "} BbtProfile\n");
}


/******************************************************************************/
/* BbtOptimize: Create the bbt optimized runtime build and place it
                in 'optDir' (it destroyes what is there).

  Parameters
    inDir        : the starting unoptimize retail build.
    optDir       : the path to use for the bbt optimized build
    installDir   : The directory where the runtime is installed
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp

  Returns the list of optimized unmanaged dlls and exes when succesful
*/

function BbtOptimize(inDir, optDir, installDir, logDir, srcBase, targetArch, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtOptimize", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    var arch = targetArch;
    var iarch = arch;
    if (iarch == "x86")
        iarch = "i386";

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;

    if (optDir == undefined)
        optDir = srcBase + "\\binaries\\" + arch + bldType + ".opt";

    var bbtDataDir = optDir + "\\BbtData";

    if (logDir == undefined)
        logDir = optDir + "\\BbtLogs\\Optimize";

    logMsg(LogClrAutomation, LogInfo, "BbtOptimize: creating optimized version of unmanaged CLR binaries {\n");

    logMsg(LogClrAutomation, LogInfo, "BbtOptimize: starting unoptimized retail build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "BbtOptimize: bbt optimized build will be put in ", optDir, "\n");

    var optProfErrFile = logDir + "\\optProf.err";
    if (FSOFileExists(optProfErrFile))
        FSODeleteFile(optProfErrFile);

    var devDivTools = srcBase + "\\Tools\\devdiv";
    var optProfExe = devDivTools + "\\OptProf\\OptProf.exe";
    var optProfDataDir = bbtDataDir;

    // It's not good to set OptProf's "Work" dir, "Temp" dir, and "Opt" dir
    // to be  within logDir, since "Work" dir and "Temp" dir can be quite
    // large, and this eats up hard drive space on clrsnap0
    var optProfWorkDir = logDir + "\\Work";
    var optProfOptDir = logDir + "\\Opt";
    var optProfTempDir = optProfWorkDir + "\\Temp";

    // It is better to set OptProf's "Work" dir, "Temp" dir, and "Opt" dir
    // to be somewhere on the local machine
    if (gsRetailLabWorker)
    {
        optProfWorkDir = gsLocalBinDir + "\\BbtOptimize\\Work";
        optProfOptDir = gsLocalBinDir + "\\BbtOptimize\\Opt";
        optProfTempDir = optProfWorkDir + "\\Temp";
    }

    var optProfConfigFile = srcBase + "\\tools\\devdiv\\Optprof\\Netfx." + arch + ".OptProf.xml";

    if (FSOFolderExists(optProfWorkDir))
        FSODeleteFolder(optProfWorkDir, true);
    FSOCreatePath(optProfTempDir);

    var dllsToOptimize = BbtGetRuntimeDllsToOptimize(targetArch, bldType);
    var pdbsForAdditionalCopy = [];

    // OptProf does not support optimizing a runtime that has already been
    // optimized, so we have to manually rollback to the unoptimized versions
    // of various dll's being optimized
    var tempInDir = inDir;
    var cleanupTempInDir = false;
    if (FSOFolderExists(inDir + "\\unopt"))
    {
        cleanupTempInDir = true;
        tempInDir = Env("TEMP") + "\\" + targetArch + bldType;
        if (FSOFolderExists(tempInDir))
            FSODeleteFolder(tempInDir, true);
        FSOCreatePath(tempInDir);
        FSOCopyFolder(inDir, tempInDir, true);

        logMsg(LogClrAutomation, LogInfo, "Copying unoptimized versions of native dll's and their pdb's.\r\n");
        logMsg(LogClrAutomation, LogInfo, "since BBT cannot process already BBT optimized dlls\r\n");
        logMsg(LogClrAutomation, LogInfo, "It is expected that the right dll's for optimization are specified in Optprof_2k*.xml\r\n");
        for(var i = 0; i < dllsToOptimize.length; i++) {
            var dllName = dllsToOptimize[i];
            var pdbName = dllName.substring(0,dllName.length-4) + ".pdb";
            // PDB name of msvcr110_clr0400 is not the same as the DLL name
            if (dllName == "msvcr110_clr0400.dll") {
                pdbName = dllName.substring(0,dllName.length-4) + "." + iarch + ".pdb";
            }
            // if we encounter errors while copying any dll/pdb, we just log it as a warning and continue
            try {
                if (FSOFileExists(tempInDir + "\\unopt\\" + dllName) && FSOFileExists(tempInDir + "\\unopt\\" + pdbName))
                {
                    FSOCopyFile(tempInDir + "\\unopt\\" + dllName, tempInDir + "\\" + dllName, true);
                    // Optprof picks up the pdb in the following order
                    // 1. Search for pdb that may be sitting next to the dll
                    // 2. If 1 fails, search for pdb in one of the locations in _NT_SYMBOL_PATH (which is Symbols.pri\retail\dll here)
                    // So to respect that behavior, we copy the unopt files the location in 1. above if the input drop is like that
                    if (FSOFileExists(tempInDir + "\\" + pdbName)) {
                        FSOCopyFile(tempInDir + "\\unopt\\" + pdbName, tempInDir + "\\" + pdbName, true);
                        pdbsForAdditionalCopy.push(optDir + "\\" + pdbName);
                    } else {
                        pdbsForAdditionalCopy.push(undefined);
                    }
                    FSOCopyFile(tempInDir + "\\unopt\\" + pdbName, tempInDir + "\\Symbols.pri\\retail\\dll\\" + pdbName, true);
                }
            } catch (e) {
                logMsg(LogClrAutomation, LogWarn, "Unoptimized dll " + dllName + " or its pdb not found\r\n");
            }
        }
    }

    try {

        var versionDir = "v4.0." + arch + "ret";
        if (isNewRetailLab()) {
            if (!installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
                throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");
            versionDir = RegExp.$2;
        }

        runOpts = runSetTimeout(120 * MINUTE,
                  runSetIdleTimeout(5 * MINUTE,
                  runSetDump("runjs dumpProc %p " + logDir + "\\bbtOptimize.dmpRpt",
                  runSetEnv("COMPLUS_EnableMultiproc", "1",
                  runSetEnv("DEVDIV_TOOLS", devDivTools,
                  runSetEnv("OptProf_PreBbtBin", tempInDir + "\\unopt",
                  runSetEnv("OptProf_Target", arch,
                  runSetEnv("_PERFTRUN_ROOTDIR", srcBase,
                  runSetEnv("_NTBINDIR", srcBase,
                  runSetEnv("_NTTREE", tempInDir,
                  runSetEnv("DD_NdpVersion", versionDir,
                  runSetEnv("DD_NDPINSTALLPATH", installDir,
                  runSetEnv("DD_NDPSDKINSTALLPATH", installDir + "\\sdk",
                  runSetEnv("DD_TargetDir", iarch,
                  runSetEnv("_BuildArch", arch,
                  clrRunTemplate)))))))))))))));

        var optProfCmdLine = optProfExe + " /optimize /engines:BBT " +
            "\"/opt:" + optProfOptDir + "\" " +
            "\"/data:" + optProfDataDir + "\" " +
            "\"/work:" + optProfWorkDir + "\" " +
            "\"/log:" + logDir + "\" " +
            "\"/config:" + optProfConfigFile + "\"";

        // Call OptProf
        runCmdToLog(optProfCmdLine, runOpts);

        // OptProf is clueless about how to place DLLs and PDBs into the drop
        // folder, so we must manually copy these files
        if (!FSOFolderExists(optDir + "\\Symbols.pri\\retail\\dll"))
            FSOCreatePath(optDir + "\\Symbols.pri\\retail\\dll");

        logMsg(LogClrAutomation, LogInfo, "Copying optimized versions of native dll's and their pdb's.\r\n");
        logMsg(LogClrAutomation, LogInfo, "It is expected that the right dll's for optimization are specified in Optprof_2k*.xml\r\n");
        for(var i = 0; i < dllsToOptimize.length; i++) {
            // if we encounter errors while copying any dll/pdb, we just log it as a warning and continue
            var dllName = dllsToOptimize[i];
            var pdbName = dllName.substring(0,dllName.length-4) + ".pdb";
            // PDB name of msvcr110_clr0400 is not the same as the DLL name
            if (dllName == "msvcr110_clr0400.dll") {
                pdbName = dllName.substring(0,dllName.length-4) + "." + iarch + ".pdb";
            }
            try {
                FSOCopyFile(optProfOptDir + "\\" + dllName, optDir + "\\" + dllName, true);
                if(pdbsForAdditionalCopy[i] != undefined) {
                    FSOCopyFile(optProfOptDir + "\\" + pdbName, pdbsForAdditionalCopy[i], true);
                }
                FSOCopyFile(optProfOptDir + "\\" + pdbName, optDir + "\\Symbols.pri\\retail\\dll\\" + pdbName, true);
            } catch (e) {
                logMsg(LogClrAutomation, LogWarn, "Optimized dll " + dllName + " or its pdb not found\r\n");
            }
        }

        logMsg(LogClrAutomation, LogInfo, "OptProf log file at ", logDir, "\\OptProf.log\n");

    } catch(e) {
        // do nothing in the catch clause
        // but it is here because it seems that instructions after the finally clause are not executed
        // unless the catch clause is present
    } finally {
        ClearZapBBInstr();
    }

    // Cleanup temporary folder if needed
    if (cleanupTempInDir)
        TryDeleteFolder(tempInDir);

    logMsg(LogClrAutomation, LogInfo, "BbtOptimize: copying the unmodified files to create a complete drop in " + optDir + "\n");
    robocopy(inDir, optDir, "/xo", logDir + "\\bbtUnmodifiedCopy.log.txt");


    // Run ndp verify to rebase the dlls properly for their new sizes
    razzleBuild(bldType, targetArch, "ndp\\verify", "-cC", srcBase, optDir, optDir, 30*MINUTE, "", clrRunTemplate);


    // Since we just rebased mscorwks.dll we go ahead and run updateDacDlls
    logMsg(LogClrAutomation, LogInfo, "BbtOptimize: updating mscordac* to match " + CLRDllName(bldType) + ".dll \n");
    updateDacDlls(inDir, optDir, logDir, srcBase, targetArch, bldType);

    logMsg(LogClrAutomation, LogInfo, "} BbtOptimize\n");
}


/******************************************************************************/
/* BbtPhase: given an a set of runtime build in 'inDir' Create an BBT optimized version
   of that build in 'optDir'.  To do this, it needs to run scenarios, and thus
   needs the runtime installed.  'srcBase' is the base of the VBL (needed to get
   tools.  Only 'optDir' is written to, and in particular the inDir is read-only.
   The function returns the list of optimized dlls.
   'scenarioSpec' is the specification of what scenarios to run (see scenariosGet)

  Parameters
    inDir        : the starting retail build, this can be unoptimized or pogo optimized
    optDir       : the path to use for the bbt optimized build
    installDir   : The directory where the runtime is installed
    logDir       : Where to put the log files
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp

  Returns the list of optimized unmanaged dlls and exes when successful
*/

function BbtPhase(inDir, optDir, installDir, logDir, srcBase, targetArch, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtPhase", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;

    if (optDir == undefined)
        optDir = inDir + ".bbO"

    if (logDir == undefined)
        logDir = optDir + "\\BbtLogs"

    FSOCreatePath(logDir);

    if (installDir == undefined)
    {
        if (isNewRetailLab()) {
            // We need to have the input drop installed before starting the BbtPhase
            throw new Error(1, ".NET Framework must be installed before running the BBT phase\r\n");
        } else {
            installDir = clrSetupWithCache(inDir, "/fx", logDir, srcBase + "\\binaries\\clrSetupCache", targetArch + bldType);
            StopNgenService(targetArch + bldType, targetArch);
        }
    }

    if (NeedToEnterWow64(targetArch)) {
        runX86("BbtPhase", inDir, optDir, installDir, logDir, srcBase, targetArch, bldType);
    }
    else {
        // Do profile phase
        BbtProfile(inDir, optDir, installDir, logDir + "\\BbtProfile", srcBase, targetArch, bldType);
        
        // Do optimization phase
        BbtOptimize(inDir, optDir, installDir, logDir + "\\BbtOptimize", srcBase, targetArch, bldType);
    }

    logMsg(LogClrAutomation, LogInfo, "} BbtPhase\n");
}

/******************************************************************************/
/* starting with an unoptimized retail build in 'inDir' create an instrumented
   bbt runtime build and place it in 'instrDir' (it destroyes what is there).

  Parameters
    inDir        : the starting unoptimize retail build.  Must have been built
                   using /LTCG, (the default for amd64 and ia64)
    instrDir     : the path to use for the bbt instrumented build
                   (defaults to inDir + ".bbI")
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)
    rArgs        : Additional args to razzle (eg Offline) (default none)

  Returns the instrDir that we created to use for the bbt instrumented build
*/
function BbtInstrument_Old(inDir, instrDir, logDir, srcBase, targetArch, rArgs, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtInstrument_Old", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;
    if (instrDir == undefined)
        instrDir = inDir + ".bbI";
    if (logDir == undefined)
        logDir = instrDir;
    if (rArgs == undefined)
        rArgs = "";

    if (FSOFolderExists(instrDir)) {
        logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: deleting ", instrDir, "\n");
        FSODeleteFolder(instrDir, true);     // clear out any old stuff
    }
    FSOCreatePath(instrDir);

    logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: input build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: bbt instrumented build ", instrDir, "\n");

    // We must populate the instrDir to be an exact copy of inDir
    // instrDir will be a private runtime
    logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: populating " +  instrDir + "\n");
    robocopy(inDir, instrDir, "/xo /nfl /ndl", logDir + "\\Copy.log.txt");

    var runtimeDlls = BbtGetRuntimeDllsToOptimize(targetArch, bldType);

    logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: Instrumenting unmanaged CLR binaries {\n");

    // clr needs some special processing, set that up.
    var clrBBT = srcBase + "\\tools\\devdiv\\x86\\bbt\\scripts";
    var clrCmd = clrBBT + "\\" + "clr." + targetArch + ".bbtInstrCmd";

    FSOWriteToFile(" /cmd " + clrCmd + " /keepasmcodeinsequence /noinstrasmcode /endboot CorHost2::CreateAppDomainWithManager",
        instrDir + "\\" + CLRDllName(bldType) + ".bbtInstrCmdLine");

    var bbtToolsDir  = srcBase + "\\" + gsBbtToolsRelPath;
    var bbinstr = bbtToolsDir + "\\bbinstr.exe";

    if (!FSOFileExists(bbinstr)) {
       logMsg(LogClrAutomation, LogInfo,
              "The optimization script is unable to find the BBT tools in this\n",
              "enlistment.  The BBT tools are checked into source\n",
              "depot at ", bbtToolsDir, ". However,\n",
              "they require special access that can be obtained as follows:\n",
              "1) go to http://AutoSecure\n",
              "2) choose the link that says Choose a project\n",
              "3) expand the DevDiv group to see the projects\n",
              "4) select the last item BBTToolset\n",
              "5) look to the left ... notice the pane titled My AutoSecure and\n",
              "select the Request Access link\n");
       throw Error(1, "BbtInstrument_Old: Failed to find the BBT toolset");
    }

    var pdbPath = inDir + ";" + inDir + "\\unopt;" + inDir + "\\symbols.pri\\retail;%_NT_SYMBOL_PATH%";
    var cmds = [];
    for(var i = 0; i < runtimeDlls.length; i++) {
        var file = runtimeDlls[i];
        var base = file.match(/(.*)\.(dll|exe)$/i)[1];
        var outDllPath = instrDir + "\\" + file;
        var inDllPath = inDir + "\\unopt\\" + file;// First see if there is an unopt directory
    if (!FSOFileExists(inDllPath))
        inDllPath = inDir + "\\" + file;
        var outBase = instrDir + "\\" + base;
        var bbtInstrOpts = instrDir + "\\" + base + ".bbtInstrCmdLine";

        // the '/idfscalepct 100' does nothing, but if you change the 100 to 200
        // you double the size of the BBT run possible
        var cmd = bbinstr + " /idfscalepct 200 /o " + outDllPath +
                            " /pdb " + outBase + ".pdb" +
                            " /idf " + outBase + "!%BBT_Idf_Id%.idf" +
                            " /key " + outBase + ".key";

        if (FSOFileExists(bbtInstrOpts))
            cmd += " " + FSOReadFromFile(bbtInstrOpts)
        cmd += " " + inDllPath;
        cmds.push(cmd);
    }

    // we run these concurrently to increase turn around time
    runCmds(cmds, 2,
        runSetLog(LogRun, LogInfo,
        runSetEnv("_NT_SYMBOL_PATH", pdbPath,
        runSetDump("runjs dumpProc %p " + logDir + "\\bbinstr.dmpRpt",
        runSetIdleTimeout(60,
        clrRunTemplate)))));

    logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: } Done instrumenting unmanaged CLR binaries\n");

    // This is a bit of a hack on x86, clr.dll and System.dll collide when clr is intrumented
    // so we rebase clr.dll at 0x50000000
    var rebase = srcBase + "\\tools\\x86\\rebase.exe";
    if (targetArch.match(/^(x86)/i)) {
        runCmdToLog(rebase + " -b 0x50000000 " + instrDir + "\\" + CLRDllName(bldType) + ".dll");
    }
    // It is OK if clr.dll grows on 64-bit it won't collide with any other dll

    logMsg(LogClrAutomation, LogInfo, "BbtInstrument_Old: updating mscordac* to match " + CLRDllName(bldType) + ".dll in instrumented dir\n");
    updateDacDlls(inDir, instrDir, logDir, srcBase, targetArch, bldType);

    logMsg(LogClrAutomation, LogInfo, "} BbtInstrument_Old Success - returning ", instrDir, "\n");

    return instrDir;
}

/******************************************************************************/
/* starting with a bbt instrumented retail build in 'instrDir' install this runtime
  Parameters
    instrDir     : the path to the bbt instrumented retail build
    bbtBinDir    : The path where irt30.dll is located
    idfOtherDir  : the path to put the profiling run data *.idf files
                   (defaults to instrDir + "\\idf_data\\other")
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    isRemote     : true if we are performing a remote run

  We use the /nz argument to ClrSetup so that nothing is ngened during this install step.

  Returns the installDir where we installed the CLR binaries
*/
function BbtInstallInstrument_Old(instrDir, bbtBinDir, idfOtherDir, logDir, srcBase, isRemote, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtInstallInstrument_Old", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (instrDir == undefined) {
        var inDir = srcBase + "\\binaries\\" + targetArch + bldType;
        instrDir = inDir + ".bbI";
    }

    if (idfOtherDir == undefined)
        idfOtherDir = instrDir + "\\idf_data\\other";

    if (logDir == undefined)
        logDir = instrDir;

    logMsg(LogClrAutomation, LogInfo, "BbtInstallInstrument_Old: Installing the bbt instrumented runtime\n");

    // We have to have irt30.dll on our PATH whenever we run any bbt intrumented binary
    var bbtRunOpts = runSetEnv("PATH", runGetEnv("PATH") + ";" + bbtBinDir,
                     clrRunTemplate);

    var verString;
    if (isRemote)
        verString = currentArch + bldType + ".rbbI";
    else
        verString = currentArch + bldType + ".bbI";

    var installDir;
    if (!isCoreCLRBuild(bldType)) {
        installDir = clrSetupWithCache(instrDir, "/nz /fx /nosxs", logDir,
                                       srcBase + "\\binaries\\clrSetupCache",
                                       verString, bbtRunOpts);
    }
    else {
        installDir = Env("TEMP") + "\\v4.0." + verString;
        installDir = fxpSetup(instrDir, installDir);
    }
    ProcessProfileData("bbt", instrDir, idfOtherDir, "ClrSetup");

    logMsg(LogClrAutomation, LogInfo, "} BbtInstallInstrument_Old Success - returning ", installDir, "\n");
    return installDir;
}

/******************************************************************************/
/* BbtProfile_Old:  Using the bbt instrumemted build that is already installed
                     Run the Bbt profile scenarios and place the resulting *.idf
                     files in 'instrDir'

  Parameters
    inDir        : the starting unoptimize retail build.
                   We copy the prejit dll from here to use during the ngen step
    instrDir     : the path to the bbt instrumented build
                   (defaults to inDir + ".bbI")
    installDir   : The path to the installed bbt instrumented runtime, it must already be installed
    bbtBinDir    : The path where irt30.dll is located
    idfOtherDir  : the path to put the other (non-scenario) profiling run data *.idf files
                   (defaults to instrDir + "\\idf_data\\other")
    idfScenarioDir : the path to put the scenario profiling run data *.idf files
                   (defaults to instrDir + "\\idf_data\\scenario")
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    localBase    : The base of the local copy of files from the VBL
    isRemote     : true if we are performing a remote run
    rArgs        : Additional args to razzle (eg Offline) (default none)

*/

function BbtProfile_Old(inDir, instrDir, installDir, bbtBinDir, idfOtherDir, idfScenarioDir, scenarioSpec, logDir, srcBase, localBase, targetArch, isRemote, rArgs, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtProfile_Old", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (localBase == undefined)
        localBase = srcBase;

    var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (targetArch == undefined) {
        targetArch = currentArch;
    }

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + currentArch + bldType;

    if (instrDir == undefined)
        instrDir = inDir + ".bbI";

    if (idfOtherDir == undefined)
        idfOtherDir = instrDir + "\\idf_data\\other";

    if (idfScenarioDir == undefined)
        idfScenarioDir = instrDir + "\\idf_data\\scenario";

    if (bbtBinDir == undefined)
        bbtBinDir = srcBase + "\\tools\\devdiv\\" + currentArch + "\\bbt\\toolset";

    if (logDir == undefined)
        logDir = instrDir;

    if (installDir == undefined) {
        if (!isCoreCLRBuild(bldType)) {
        var frameworkDir = Env("WINDIR") + "\\Microsoft.NET\\Framework";
        if (!currentArch.match(/^(x86)/i))
            frameworkDir = frameworkDir + "64";
            var runtimeDir = frameworkDir + "\\v4.0." + currentArch + bldType;
        installDir = runtimeDir + ".bbI";
    }
        else {
            installDir = instrDir;
        }
    }

    var stdBbtRunOpts = runSetDump("runjs dumpProc %p " + logDir + "\\bbt.ngen.dmpRpt",
                        runSetIdleTimeout(60,
                        runSetEnv("PATH", runGetEnv("PATH") + ";" + bbtBinDir,
                        clrRunTemplate)));

    var scenarios = scenariosGet(scenarioSpec, localBase, targetArch, bldType, installDir);

    if (!isCoreCLRBuild(bldType)) {
    // We don't really care about the profile data generated during the NGEN step,
    // but we do need to use the instrumented clr.dll since the NGEN images
    // hard bind to it.  But we can use the non-instrumented <prejit>.dll, and it
    // is faster than the instrumented one.  So we copy the non-instrumented
    // <prejit>.dll into the install dir for the NGEN step and put back the
    // instrumented one when running the scenarios.

    // Use the runtime .dlls found in inDir
    SwitchRuntimeDlls(inDir, instrDir, installDir, "Using the non-instrumented runtime while ngen-ing");

    // ngen the scenarios.
    logMsg(LogClrAutomation, LogInfo, "BbtProfile_Old: ngening scenarios {\n");

    NgenScenarios(scenarios, "", installDir, stdBbtRunOpts, true, "bbt", instrDir, idfOtherDir);

    logMsg(LogClrAutomation, LogInfo, "} BbtProfile_Old: ngen scenarios complete\n");

    // Use the runtime .dlls found in instrDir
    SwitchRuntimeDlls(instrDir, inDir, installDir, "Using the instrumented runtime to run the scenarios");

    // Create a special subdirectory called isolated which will contain the mscoree.dll to use when BBT profiling
    var mscoreeDir = installDir + "\\isolated";
    FSOCreatePath(mscoreeDir);

    // We also need to copy mscoree.dll from the instr dir into the install\isolated dir,
    // since we want to use it when profiling
    FSOCopyFile(instrDir + "\\mscoree.dll", mscoreeDir + "\\mscoree.dll", "FORCE");
    FSOCopyFile(instrDir + "\\symbols.pri\\retail\\dll\\mscoree.pdb", mscoreeDir + "\\mscoree.pdb", "FORCE");
    }

    // Clear out the idfScenarioDir if it exists
    if (FSOFolderExists(idfScenarioDir)) {
        logMsg(LogClrAutomation, LogInfo, "BbtProfile_Old: deleting old ", idfScenarioDir, "\n");
        FSODeleteFolder(idfScenarioDir, true);     // clear out any old stuff
    }

    try {

        SetMismatchedRuntime();

        // run the scenarios
        logMsg(LogClrAutomation, LogInfo, "BbtProfile_Old: running scenarios on bbt instrumented runtime {\n");

        RunScenarios(bldType, scenarios, "bbt", installDir, stdBbtRunOpts,
                     instrDir, idfScenarioDir, idfOtherDir);

        ClearMismatchedRuntime();
    } catch (e) {
        ClearMismatchedRuntime();
        throwWithStackTrace(e);
    }


    logMsg(LogClrAutomation, LogInfo, "}\n");

    logMsg(LogClrAutomation, LogInfo, "} BbtProfile_Old Success\n");

    return 0;
}


/******************************************************************************/
/* BbtOptimize_Old: Create the bbt optimized runtime build and place it
                in 'optimDir' (it destroyes what is there).

  Parameters
    inDir        : the starting unoptimize retail build.
    instrDir     : the path to use for the bbt instrumented build
                   (defaults to inDir + ".bbI")
    optimDir     : the path to use for the bbt optimized build
                   (defaults to inDir + ".bbO")
    idfScenarioDir : the path where we have the scenario profiling run data *.idf files
                   (defaults to instrDir + "\\idf_data\\scenario")
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)

  Returns the list of optimized unmanaged dlls and exes when succesful
*/

function BbtOptimize_Old(inDir, instrDir, optimDir, idfScenarioDir, logDir, srcBase, targetArch, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtOptimize_Old", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;

    if (instrDir == undefined)
        instrDir = inDir + ".bbI";
    if (optimDir == undefined)
        optimDir = inDir + ".bbO";
    if (idfScenarioDir == undefined)
        idfScenarioDir = instrDir + "\\idf_data\\scenario";
    if (logDir == undefined)
        logDir = instrDir;

    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: creating optimized version of unmanaged CLR binaries {\n");

    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: starting unoptimized retail build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: bbt instrumented build ", instrDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: bbt optimized build will be put in ", optimDir, "\n");

    // The optimDir only gets populated with the Bbt optimized binaries by this routine
    // We populate optimDir with everything else in retailBuild
    // We can BBT optimize a build in place when inDir == optimDir

    // We also preserve the bbt scenario data by copying the idfScenarioDir (the idf files)
    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: copying the bbt scenario data to " +  optimDir + "\\BbtData\\scenario\n");
    robocopy(idfScenarioDir, optimDir + "\\BbtData\\scenario", "", logDir + "\\BbtDataScenarioCopy.log.txt");

    var runtimeDlls = BbtGetRuntimeDllsToOptimize(targetArch, bldType);

    // clr needs some special processing, set that up.
    var clrBBT = srcBase + "\\tools\\devdiv\\x86\\bbt\\scripts";
    var clrCmd = clrBBT + "\\clr." + targetArch + ".bbtOptCmd";

    FSOWriteToFile(" /keepasmcodeinsequence /cmd " + clrCmd, instrDir + "\\" + CLRDllName(bldType) + ".bbtOptCmdLine");

    var bbtToolsDir  = srcBase + "\\" + gsBbtToolsRelPath;
    var bbopt = bbtToolsDir + "\\bbopt.exe";

    if (!FSOFileExists(bbopt)) {
       logMsg(LogClrAutomation, LogInfo,
              "The optimization script is unable to find the BBT tools in this\n",
              "enlistment.  The BBT tools are checked into source\n",
              "depot at ", bbtToolsDir, ". However,\n",
              "they require special access that can be obtained as follows:\n",
              "1) go to http://AutoSecure\n",
              "2) choose the link that says Choose a project\n",
              "3) expand the DevDiv group to see the projects\n",
              "4) select the last item BBTToolset\n",
              "5) look to the left ... notice the pane titled My AutoSecure and\n",
              "select the Request Access link\n");
       throw Error(1, "BbtOptimize_Old: Failed to find the BBT toolset");
    }

    var pdbPath = inDir + ";" + inDir + "\\unopt;" + inDir + "\\symbols.pri\\retail;" + instrDir + ";%_NT_SYMBOL_PATH%";
    var cmds = []
    var optUnmanagedFiles = [];

    for(var i = 0; i < runtimeDlls.length; i++) {
        var currentFile = runtimeDlls[i];
        var base = currentFile.match(/(.*)\.(dll|exe)$/i)[1];
        var inFilePath = inDir + "\\unopt\\" + currentFile;// First see if there is an unopt directory
    if (!FSOFileExists(inFilePath))
        inFilePath = inDir + "\\" + currentFile;

        var outFilePath = optimDir + "\\" + currentFile;
        var idfBase = idfScenarioDir + "\\" + base;
        var pdbDir = optimDir + "\\symbols.pri\\Retail\\dll";

        if (!FSOFileExists(inFilePath)) {
            // If we don't have a matching dll then we have an exe instead
            currentFile = base + ".exe";
            inFilePath = inDir + "\\" + currentFile;
            outFilePath = optimDir + "\\" + currentFile;
            pdbDir = optimDir + "\\symbols.pri\\Retail\\exe";
        }

        if (FSOFileExists(inFilePath)) {
            var cmd = bbopt + " /pairwise /rereadable " +
                          " /pdb " + pdbDir + "\\" + base + ".pdb" +
                          " /idf " + idfBase + "!\*.idf" +
                          " /o " + outFilePath;
            FSOCreatePath(pdbDir);

            var bbtOptOpts = instrDir + "\\" + base + ".bbtOptCmdLine";
            if (FSOFileExists(bbtOptOpts))
                cmd += " " + FSOReadFromFile(bbtOptOpts);
            cmd += " " + inFilePath;

            logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: Running bbopt on ", currentFile, "\n");

            cmds.push(cmd);
            optUnmanagedFiles.push(outFilePath);
        }
        else {
            logMsg(LogClrAutomation, LogWarn, "BbtOptimize_Old: Could not find file ", currentFile, " skipping\n");
        }
    }

    // we run these concurrently to increase turn around time
    runCmds(cmds, 2,
        runSetLog(LogRun, LogInfo,
        runSetEnv("_NT_SYMBOL_PATH", pdbPath,
        runSetDump("runjs dumpProc %p " + logDir + "\\bbopt.dmpRpt",
        runSetIdleTimeout(60,
        clrRunTemplate)))));

    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: } Done optimizing unmanaged CLR binaries\n");


    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: copying the unmodified files to create a complete drop in " + optimDir + "\n");
    robocopy(inDir, optimDir, "/xo /nfl /ndl", logDir + "\\bbtUnmodifiedCopy.log.txt");

    // Run ndp verify to rebase the dlls properly for their new sizes
    razzleBuild(bldType, targetArch, "ndp\\verify", "-cC", srcBase, optimDir, optimDir, 30*MINUTE, "", clrRunTemplate);

    // Since we just rebased clr.dll we go ahead and run updateDacDlls
    logMsg(LogClrAutomation, LogInfo, "BbtOptimize_Old: updating mscordac* to match " + CLRDllName(bldType) + ".dll \n");
    updateDacDlls(inDir, optimDir, logDir, srcBase, targetArch, bldType);
    optUnmanagedFiles.push(optimDir + "\\" + DACDllName(bldType) + ".dll");     // add this to list to be updated.

    logMsg(LogClrAutomation, LogInfo, "} BbtOptimize_Old Success - returning ", optUnmanagedFiles, "\n");

    return optUnmanagedFiles;
}


/******************************************************************************/
/* Combines two BBT steps, BbtInstallInstrument_Old and BbtProfile_Old and
   handles the case of remote machine execution.

  Parameters
    inDir        : the starting retail build, this can be unoptimized or pogo optimized
    instrDir     : the path to the bbt instrumented retail build,
    idfOtherDir  : the path to put the other (non-scenario) profiling run data *.idf files
    idfScenarioDir : the path to put the scenario profiling run data *.idf files
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    localBase    : The base of the local copy of files from the VBL
    targetArch   : The target archecture (defaults to the current processor architecture)
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)
*/
function BbtInstallAndProfile_Old(inDir, instrDir, idfOtherDir, idfScenarioDir, scenarioSpec, logDir, srcBase, localBase, targetArch, remoteMachine, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtInstallAndProfile_Old", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (localBase == undefined)
        localBase = srcBase;

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    var isRemote = (remoteMachine == "true");
    var run;
    var runExitCode = undefined;

    if ((remoteMachine != undefined) && !isRemote)
    {
        // We forward this call to the remote machine via a recursive "runCmd runjs" call
        // but on the rescursive call we have remoteMachine set to true.

        var runJsCmd = uncPath(srcBase +"\\ndp\\clr\\bin\\runjs.bat");
        var logFileInfoPrefix = logDir +"\\BbtInstallAndProfile_Old." + remoteMachine;
        var logFile = logFileInfoPrefix + ".log" ;
        var remoteRunOpts = runSetOutput(logFile,
                            runSetDump(runJsCmd + " dumpProc %p " + logFileInfoPrefix + ".dmpRpt",
                            runSetTimeout(2 * HOUR,
                            runSetIdleTimeout(60,
                            runSetMachine(remoteMachine)))));

        var remoteLocalBase = localBase;
        if (srcBase == localBase)
            remoteLocalBase = gsDefaultRemoteLocalBase;

        var remoteCmdLine = FormatCmdLine(runJsCmd,
                                          "BbtInstallAndProfile_Old",
                                          uncPath(inDir),
                                          uncPath(instrDir),
                                          uncPath(idfOtherDir),
                                          uncPath(idfScenarioDir),
                                          scenarioSpec,
                                          uncPath(logDir),
                                          uncPath(srcBase),
                                          remoteLocalBase,
                                          targetArch,
                                          true);

        run = runCmd(remoteCmdLine, remoteRunOpts);
        runExitCode = run.exitCode;
    }
    else
    {
        if (isRemote) {
            logMsg(LogClrAutomation, LogInfo, "BbtInstallAndProfile_Old is running on remote\n");
            InitLocalBase(localBase, srcBase, logDir);
        }

        if (NeedToEnterWow64(targetArch)) {
            run = runX86("BbtInstallAndProfile_Old",
                         inDir, instrDir, idfOtherDir, idfScenarioDir,
                         scenarioSpec, logDir, srcBase, localBase);

            runExitCode = run.exitCode;
        }
        else
        {
            var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

            // We have to have irt30.dll on our PATH whenever we run any bbt intrumented binary
            var bbtBinDir = srcBase + "\\tools\\devdiv\\" + currentArch + "\\bbt\\toolset";

            // During ClrSetup our PATH is clobbered and when for w3wp starts up during IBuySpy
            // our path is clobbered,  so we force copy irt30.dll to %WINDIR%\System32
            //
            var systemDir = Env("WINDIR") + "\\System32";

            // First delete the file if it exists
            FSOTryDeleteFile(systemDir + "\\irt30.dll");

            logMsg(LogClrAutomation, LogInfo, "BbtInstallAndProfile_Old: Copying irt30.dll to ", systemDir, "\n");
            try {
                FSOCopyFile(bbtBinDir + "\\irt30.dll", systemDir + "\\irt30.dll", "FORCE");
            } catch(e) {
                logMsg(LogClrAutomation, LogError, "BbtInstallAndProfile_Old: Failure copying irt30.dll, procs = {\n", procTreeAsString(), "\n}\n");
                throwWithStackTrace(e);   // rethrow the orginal error
            }

            if (!isCoreCLRBuild(bldType)) {
            // We don't want to install the instrumented mscoree.dll into the system32 directory
            // so we temporarily save the instrumented one as mscoree.dll.bbI and replace it with the one from inDir
            logMsg(LogClrAutomation, LogInfo, "BbtInstallAndProfile_Old: renaming instrumented mscoree.dll to mscoree.dll.bbI\n");

            FSOCopyFile(instrDir + "\\mscoree.dll", instrDir + "\\mscoree.dll.bbI", "FORCE");
            FSOCopyFile(inDir    + "\\mscoree.dll", instrDir + "\\mscoree.dll",     "FORCE");
            }
            else
            {
                // irt30.dll is a system protected file.  For CoreClr we can copy it to
                // out target folder instead.
                FSOCopyFile(bbtBinDir + "\\irt30.dll", instrDir + "\\irt30.dll", true);
            }

            var installDir = BbtInstallInstrument_Old(instrDir, bbtBinDir, idfOtherDir, logDir, srcBase, isRemote, bldType);

            if (!isCoreCLRBuild(bldType)) {
            // Put the instrumented version of mscoree.dll back the way it was in instrDir
            FSOMoveFile(instrDir + "\\mscoree.dll.bbI", instrDir + "\\mscoree.dll", "FORCE");
            }

            try {
                runExitCode = BbtProfile_Old(inDir, instrDir, installDir, bbtBinDir,
                                         idfOtherDir, idfScenarioDir, scenarioSpec, logDir,
                                         srcBase, localBase, targetArch, isRemote,
                                         undefined, bldType);
            } catch(e) {
                logMsg(LogClrAutomation, LogError, "BbtInstallAndProfile_Old: Failure during BbtProfile_Old\n}\n");

                logMsg(LogClrAutomation, LogError, "BbtInstallAndProfile_Old: Attempting to delete irt30.dll from ", systemDir, "\n");

                WScript.Sleep(10*1000);   // Sleep for 10 seconds

                FSOTryDeleteFile(systemDir + "\\irt30.dll");

                throwWithStackTrace(e);   // rethrow the orginal error
            }

            // Remove the irt30.dll from System32
            //
            logMsg(LogClrAutomation, LogInfo, "BbtInstallAndProfile_Old: Deleting irt30.dll from ", systemDir, "\n");
            try {
                FSODeleteFile(systemDir + "\\irt30.dll", true);
            } catch(e) {
                logMsg(LogClrAutomation, LogError, "BbtInstallAndProfile_Old: Failure deleting irt30.dll, procs = {\n", procTreeAsString(), "\n}\n");
            }

            // Remove the BBT instrumented runtime from the system
            UninstallRuntime(bldType, installDir, false, logDir, srcBase, targetArch);
        }
    }

    logMsg(LogClrAutomation, LogInfo, "} BbtInstallAndProfile_Old ", (runExitCode==0) ? "Success" : "Failed", "\n");

    return runExitCode;
}


/******************************************************************************/
/* BbtPhase_Old: given an a set of runtime build in 'inDir' Create an BBT optimized version
   of that build in 'optimDir'.  To do this, it needs to run scenarios, and thus
   needs the runtime installed.  'srcBase' is the base of the VBL (needed to get
   tools.  Only 'optimDir' is written to, and in particular the inDir is read-only.
   The function returns the list of optimized dlls.
   'scenarioSpec' is the specification of what scenarios to run (see scenariosGet)

  Parameters
    inDir        : the starting retail build, this can be unoptimized or pogo optimized
    instrDir     : the path to the bbt instrumented retail build
                   if this is undefined we must create the bbt instrumented build
    optimDir     : the path to use for the bbt optimized build
                   (defaults to inDir + ".bbO")
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the optimDir)
    srcBase      : The base of the VBL (so we can find razzle)
    localBase    : The base of the local copy of files from the VBL
    targetArch   : The target archecture (defaults to the current processor architecture)
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)

  Returns the list of optimized unmanaged dlls and exes when successful
*/
function BbtPhase_Old(inDir, instrDir, optimDir, scenarioSpec, logDir, srcBase, localBase, targetArch, remoteMachine, bldType) {

    logCall(LogClrAutomation, LogInfo, "BbtPhase_Old", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;

    if (optimDir == undefined)
        optimDir = inDir + ".bbO"

    if (logDir == undefined)
        logDir = optimDir;
    FSOCreatePath(logDir);

    var removeInstrDir = false;
    if (instrDir == undefined) {
        instrDir = BbtInstrument_Old(inDir, undefined, logDir, srcBase, targetArch, undefined, bldType);
        if (!isCoreCLRBuild(bldType)) {
            // For CoreCLR, we don't install runtime.  Instead, we run against the instrumented version directly.
        removeInstrDir = true;
    }
    }

    var idfOtherDir    = instrDir + "\\idf_data\\other";
    var idfScenarioDir = instrDir + "\\idf_data\\scenario";

    // Clear out the idfOtherDir if it exists
    if (FSOFolderExists(idfOtherDir)) {
        logMsg(LogClrAutomation, LogInfo, "BbtPhase_Old: deleting old ", idfOtherDir, "\n");
        FSODeleteFolder(idfOtherDir, true);     // clear out any old stuff
    }

    BbtInstallAndProfile_Old(inDir, instrDir, idfOtherDir, idfScenarioDir, scenarioSpec, logDir, srcBase, localBase, targetArch, remoteMachine, bldType);

    var optUnmanagedFiles = BbtOptimize_Old(inDir, instrDir, optimDir, idfScenarioDir, logDir, srcBase, targetArch, bldType);

    // Clear out the idfOtherDir if it exists
    if (FSOFolderExists(idfOtherDir)) {
        logMsg(LogClrAutomation, LogInfo, "BbtPhase_Old: removing non-scenario bbt data directory ", idfOtherDir, "\n");
        FSODeleteFolder(idfOtherDir, true);
    }

    if (removeInstrDir) {
        // delete the bbt instrumented binaries dir
        logMsg(LogClrAutomation, LogInfo, "BbtPhase_Old: removing bbt instrumented binaries in", instrDir, "\n");
        TryDeleteFolder(instrDir);
    }

    logMsg(LogClrAutomation, LogInfo, "} BbtPhase_Old Success - returning ", optUnmanagedFiles, "\n");

    return optUnmanagedFiles;
}




/******************************************************************************/
/**************************** IBC OPTIMIZATION ********************************/
/******************************************************************************/

function RecordZapBBInstr(zapFileList, ibcDataDir) {

    gsZapFileList = zapFileList;
    gsIbcDataDir  = ibcDataDir;
}

function SetZapBBInstr()
{
    if (gsZapFileList != undefined) {
        //
        // When starting up the w3wp service we don't have our environment variables set
        // so we set the system-wide registry keys instead.
        //
        logMsg(LogClrAutomation, LogInfo, "SetZapBBInstr: Setting system-wide registry keys: ZapBBInstr and ZapBBInstrDir.\n");

        var netFrameworkRoot = GetNetFrameworkRoot();

        WshShell.RegWrite(netFrameworkRoot + "ZapBBInstr",    gsZapFileList, "REG_SZ");
        WshShell.RegWrite(netFrameworkRoot + "ZapBBInstrDir", gsIbcDataDir,  "REG_SZ");
    }
}

function ClearZapBBInstr()
{
    //
    // Remove the system-wide registry keys that we set above
    //

    var netFrameworkRoot = GetNetFrameworkRoot();

    try {
        WshShell.RegDelete(netFrameworkRoot + "\\ZapBBInstr");
    logMsg(LogClrAutomation, LogInfo, "ClearZapBBInstr: Deleting system-wide registry keys: ZapBBInstr.\n");
    } catch(e) {};

    try {
        WshShell.RegDelete(netFrameworkRoot + "\\ZapBBInstrDir");
    logMsg(LogClrAutomation, LogInfo, "ClearZapBBInstr: Deleting system-wide registry keys: ZapBBInstrDir.\n");
    } catch(e) {};
}

/******************************************************************************/
/* IbcGetProfileDlls:

  Parameters
    scenarioSpec : Scenarios to run, defaults to ndpScenarios

    Returns the expected list of dlls that will have profile data
*/

function IbcGetProfileDlls(scenarioSpec, noisy) {
    if (noisy == undefined)
        noisy = false;

    if (noisy)
        logCall(LogClrAutomation, LogInfo, "IbcGetProfileDlls", arguments, " {");

    var result      = ["mscorlib"];
    var profileDlls = "mscorlib";

    var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    // Currently we don't use scenarioSpec.

    for (var i = 0; i < ndpScenarios.length; i++) {
        var scenario = ndpScenarios[i];

        if (noisy)
            logMsg(LogClrAutomation, LogInfo, "IbcGetProfileDlls: Considering scenario: ", scenario.name, "\n");

        if (scenario.platforms.indexOf(currentArch) < 0) {
            if (noisy)
                logMsg(LogClrAutomation, LogInfo, "IbcGetProfileDlls: scenario is not enabled on ", currentArch, "\n");
            continue;
        }

        if (scenario.depends.length == 0) {
            if (noisy)
                logMsg(LogClrAutomation, LogInfo, "IbcGetProfileDlls: scenario depends is empty\n");
            continue;
        }

        if (scenario.ibcRuns == 0) {
            if (noisy)
                logMsg(LogClrAutomation, LogInfo, "IbcGetProfileDlls: scenario ibcRuns is zero\n");
            continue;
        }

        for (var k = 0; k < scenario.depends.length; k++) {
            var currDepend = scenario.depends[k];
            if (profileDlls.indexOf(currDepend) < 0)
            {
                if (noisy)
                    logMsg(LogClrAutomation, LogInfo, "IbcGetProfileDlls: Adding ", currDepend, " to list of profile dlls.\n");
                profileDlls += " " + currDepend;
                result.push(currDepend);
            }
        }
    }

    if (noisy)
        logMsg(LogClrAutomation, LogInfo, "} IbcGetProfileDlls returning ", profileDlls, "\n");

    return result;
}

/******************************************************************************/
/* IbcProfile:  Using the installed ibc instrumemted build
                Ngen the scenarions and their dependant managed libraries with
                ibc instrumentation.  Then run the ibc profile scenarios and
                place the results in the ibcDataDir

  Parameters
    scenarioSpec : Scenarios to run, defaults to all CLR scenarios.  This is a name of a ScenarioGroup in OptProf_2k*.xml
    ibcDataDir   : where to put the ibc profile data
    logDir       : Where to put the log files (defaults to .)
    installDir   : The directory where the runtime is installed.  Needed for ngen, and scenarios use it.
                   and needed by BBT for instrumentation
    srcBase      : The base of the VBL (so we can find tools)
    inDir        : where to get the binaries to add optimization data to
*/

function IbcProfile(scenarioSpec, ibcDataDir, logDir, installDir, srcBase, inDir) {

    logCall(LogClrAutomation, LogInfo, "IbcProfile", arguments, " {");
    var arch = Env("PROCESSOR_ARCHITECTURE");

    if (scenarioSpec == undefined)
        scenarioSpec = "\"/scenarios:CLR_Managed\"";
    else
        scenarioSpec = "\"/scenarios:" + scenarioSpec + "\"";

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (inDir == undefined) {
        var inDir = srcBase + "\\binaries\\" + arch + "ret";
    }

    var optDir = srcBase + "\\binaries\\" + arch + "ret.opt";
    if (ibcDataDir == undefined)
        ibcDataDir  = optDir + "\\IbcData";

    if (logDir == undefined)
        logDir = optDir + "\\IbcLogs\\Profile";

    if (installDir == undefined) {
        if (isNewRetailLab()) {
            // The input runtime must be installed before running IbcPhase
            throw new Error(1, "Required argument 'installDir' is missing\r\n");
        } else {
            installDir = Env("URTTARGET");
            if (!installDir)
                throw new Error(1, "Required argument 'installDir' is missing and environment variable 'URTTARGET' is not set");
        }
    }

    if (FSOFolderExists(ibcDataDir))
        FSODeleteFolder(ibcDataDir, true);     // clear out any old stuff
    FSOCreatePath(ibcDataDir);

    var errorFile = logDir + "\\optProf.err";
    if (FSOFileExists(errorFile))
        FSODeleteFile(errorFile);

    // ASP scenarios have very little happening on the client end, so increase the idle timeout
    var runOpts = clrRunTemplate;
    runOpts = runSetTimeout(3*3600, runOpts);
    runOpts = runSetIdleTimeout(600, runOpts);
    runOpts = runSetDump("runjs dumpProc %p " + logDir + "\\ibcProfile.dmpRpt", runOpts)

    if (!installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
        throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");
    var versionDir = RegExp.$2;
    // Make certain the WPF font cache is running and using our runtime
    StartServiceWithEnvironment("FontCache3.0.0.0", "COMPLUS_Version=" + versionDir + ",COMPLUS_InstallRoot=" + installDir.substring(0, installDir.lastIndexOf('\\')));

    // As an optimization, ngen /Tuning some of the scenarios with the Multi-proc, which takes advantage of parallel procs
    logMsg(LogClrAutomation, LogInfo, "Speculatively ngening some DLLs with Multi-proc\n");
    try {
        runCmdToLog(installDir + "\\ngen install System.Windows.Forms.dll /Tuning /queue", runSetCwd(installDir, runOpts));
        runCmdToLog(installDir + "\\ngen install System.Web.dll /Tuning /queue", runSetCwd(installDir, runOpts));
        runCmdToLog(installDir + "\\ngen executeQueuedItems", runSetEnv("COMPLUS_EnableMultiProc", "1", runOpts));
    } catch (e) {
        logMsg(LogClrAutomation, LogInfo, "Error in speculatively ngen'ing some dll's with multi-proc: " + e.description + "\r\n");
    }

    var devDivTools = srcBase + "\\Tools\\devdiv";
    var optProfExe = devDivTools + "\\OptProf\\OptProf.exe";

    // It's not good to set OptProf's "Work" dir and "Temp" dir to be
    // within logDir, since "Work" dir and "Temp" dir can be quite large,
    // and this eats up hard drive space on clrsnap0
    var optProfWorkDir = logDir + "\\Work";
    var optProfTempDir = optProfWorkDir + "\\Temp";
    // It is better to set OptProf's "Work" dir and "Temp" dir to be 
    // somewhere on the local machine
    if (gsRetailLabWorker)
    {
        optProfWorkDir = gsLocalBinDir + "\\IbcProfile\\Work";
        optProfTempDir = optProfWorkDir + "\\Temp";
    }

    var iarch = arch;
    if (iarch == "x86")
        iarch = "i386";

    if (FSOFolderExists(optProfWorkDir))
        FSODeleteFolder(optProfWorkDir, true);     // clear out any old stuff
    FSOCreatePath(optProfTempDir);

    // Install IIS for ASP.NET scenarios
    SetupIIS(arch, installDir, srcBase);

    // optProf does not do relative paths properly, create an absolute path
    FSOCreatePath(logDir);
    logDir = FSOGetFolder(logDir).Path;

    // var configFile = devDivTools + "\\OptProf\\OptProf.xml";
    // runOpts = runSetEnv("OptProf_Config", configFile, runOpts);

    var optProfDataDir = ibcDataDir;

    runOpts = runSetEnv("OptProf_Target", arch, runOpts);
    runOpts = runSetEnv("OptProf_Data", optProfDataDir, runOpts);
    runOpts = runSetEnv("OptProf_Paths", installDir, runOpts);    // So OptProf can find ngen.
    runOpts = runSetEnv("DEVDIV_TOOLS", devDivTools, runOpts);
    runOpts = runSetEnv("_NTBINDIR", srcBase, runOpts);
    runOpts = runSetEnv("DD_NDPINSTALLPATH", installDir, runOpts);
    runOpts = runSetEnv("DD_NDPSDKINSTALLPATH", installDir + "\\sdk", runOpts);
    runOpts = runSetEnv("_NTTREE", inDir, runOpts);
    runOpts = runSetEnv("DD_NdpVersion", versionDir, runOpts);
    runOpts = runSetEnv("DD_TargetDir", iarch, runOpts);
    runOpts = runSetEnv("_BuildArch", arch, runOpts);
    runOpts = runSetEnv("_PERFTRUN_ROOTDIR", srcBase, runOpts);
    runOpts = runSetEnv("SQLSERVER", "CLRSNAP0", runOpts);

    var optProfConfigFile = srcBase + "\\tools\\devdiv\\Optprof\\Netfx." + arch + ".Optprof.xml";

    // Disable strong name verification for certain modules
    runCmdToLog(srcBase + "\\OptimizationData\\scripts\\dosn.bat " +
                (arch == "x86" ? "32" : "64"), runOpts);

    // Call OptProf
    try {
        runCmdToLog(optProfExe + " /train /engines:IBC " + scenarioSpec +
            " /work:" + optProfWorkDir +
            " /log:" + logDir +
            " /config:" + optProfConfigFile,
            runOpts);
        logMsg(LogClrAutomation, LogInfo, "OptProf log file at ", logDir, "\\optProf.log\n");
    } finally {
        ClearZapBBInstr();        // Optprof can set the system wide reg key, clear it.
    };

    logMsg(LogClrAutomation, LogInfo, "} IbcProfile\n");
    if (FSOFileExists(errorFile))
        throw new Error(1, "OptProf tool failed, see " + errorFile + " for details");
}

/******************************************************************************/
/* IbcOptimize applies IBC data from 'ibcDataDir' to dlls in 'inDir' to form dlls
   with profile information in 'optDir'

  Parameters
    bldType      : The type of build, eg ret, coreret, coreshp
    scenarioSpec : scenarios specifing the profile data to attach (TODO do we need this?)
    inDir        : where to get the binaries to add optimization data to
    ibcDataDir   : where to get the ibc profile data
    optDir       : where to put the optimized binaries
    logDir       : Where to put the log files (defaults to <optDir>\IbcLogs)
    srcBase      : The base of the VBL (so we can find tools)
    installDir   : The directory where the runtime is installed
*/

function IbcOptimize(bldType, scenarioSpec, inDir, ibcDataDir, optDir, logDir, srcBase, installDir) {

    logCall(LogClrAutomation, LogInfo, "IbcOptimize", arguments, " {");

    if (scenarioSpec == undefined)
        scenarioSpec = "\"/scenarios:CLR_Managed\"";
    else
        scenarioSpec = "\"/scenarios:" + scenarioSpec + "\"";

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (installDir == undefined) {
        throw new Error(1, "Required argument 'installDir' is missing");
    }


    if (inDir == undefined)
        inDir  = srcBase + "\\binaries\\" + Env("PROCESSOR_ARCHITECTURE") + bldType;

    if (optDir == undefined)
        optDir = inDir + ".opt";

    if (ibcDataDir == undefined)
        ibcDataDir = optDir + "\\IbcData";

    if (logDir == undefined)
        logDir = optDir + "\\IbcLogs\\Optimize";

    var runOpts = runSetDump("runjs dumpProc %p " + logDir + "\\ibcOptimize.dmpRpt",
    runSetIdleTimeout(180, runSetTimeout(3*3600)));

    var devDivTools = srcBase + "\\Tools\\devdiv";
    var optProfExe = devDivTools + "\\OptProf\\OptProf.exe";

    // It's not good to set OptProf's "Work" dir and "Temp" dir to be
    // within logDir, since "Work" dir and "Temp" dir can be quite large,
    // and this eats up hard drive space on clrsnap0
    var optProfWorkDir = logDir + "\\Work";
    var optProfTempDir = optProfWorkDir + "\\Temp";

    // It is better to set OptProf's "Work" dir and "Temp" dir to be 
    // somewhere on the local machine
    if (gsRetailLabWorker)
    {
        optProfWorkDir = gsLocalBinDir + "\\IbcOptimize\\Work";
        optProfTempDir = optProfWorkDir + "\\Temp";
    }

    var optProfDataDir = ibcDataDir;
    var arch = Env("PROCESSOR_ARCHITECTURE");

    var iarch = arch;
    if (iarch == "x86")
        iarch = "i386";

    if (FSOFolderExists(optProfWorkDir))
        FSODeleteFolder(optProfWorkDir, true);     // clear out any old stuff

    // optProf does not do relative paths properly, create an absolute path
    FSOCreatePath(logDir);
    logDir = FSOGetFolder(logDir).Path;

    runOpts = runSetEnv("OptProf_Target", arch, runOpts);
    runOpts = runSetEnv("_NTBINDIR", srcBase, runOpts);
    runOpts = runSetEnv("_NTTREE", inDir, runOpts);
    runOpts = runSetEnv("DD_NDPINSTALLPATH", installDir, runOpts);
    runOpts = runSetEnv("DD_NDPSDKINSTALLPATH", installDir + "\\sdk", runOpts);
    runOpts = runSetEnv("DD_NdpVersion", versionDir, runOpts);
    runOpts = runSetEnv("DD_TargetDir", iarch, runOpts);
    runOpts = runSetEnv("DEVDIV_TOOLS", devDivTools, runOpts);
    runOpts = runSetEnv("_PERFTRUN_ROOTDIR", srcBase, runOpts);

    if (!installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
        throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");
    var versionDir = RegExp.$2;

    // Make certain we use the checked-in runtime
    runOpts = runSetEnv("COMPLUS_DefaultVersion", "v4.0",
              runSetEnv("COMPLUS_Version", "v4.0",
              runSetEnv("COMPLUS_InstallRoot", srcBase + "\\tools\\x86\\managed\\",
              runOpts)));

    // IbcMerge.exe must be on the path so that ApplyProfileData.exe can find it
    runOpts = runSetEnv("PATH", srcBase + "\\tools\\x86\\managed\\v4.0" + ";" + runGetEnv("PATH"), runOpts);

    if (inDir != optDir) {
        logMsg(LogClrAutomation, LogInfo, "Copying files to create a complete drop in " + optDir + "\n");
        robocopy(inDir, optDir, "/xo", logDir + "\\ibcUnmodifiedCopy.log.txt");

        logMsg(LogClrAutomation, LogInfo, "IbcOptimize: copying the extra unopt binaries to create a complete drop in " + optDir + "\n");
        robocopy(optDir + "\\unopt", optDir, "*.dll *.exe /XN /XO /XC", logDir + "\\bbtUnmodifiedUnoptCopy.log.txt");
    }

    var optDataFolder = srcBase + "\\OptimizationData\\" + arch;

    if (FSOFolderExists(optProfTempDir))
        FSODeleteFolder(optProfTempDir, true);
    FSOCreatePath(optProfTempDir);
    FSOCreatePath(optProfTempDir + "\\CLR");
    FSOCreatePath(optProfTempDir + "\\CLR\\Base");

    logMsg(LogClrAutomation, LogInfo, "Copying freshly generated IBC data to " + optProfTempDir + "\n");
    robocopy(optProfDataDir, optProfTempDir + "\\CLR\\Base", "/s", logDir + "\\freshIbcDataCopy.log.txt");

    logMsg(LogClrAutomation, LogInfo, "Merging checked-in IBC data from " + optDataFolder + " into " + optProfTempDir + "\n");
    robocopy(optDataFolder, optProfTempDir, "/xo", logDir + "\\checkedInIbcDataCopy.log.txt");

    var applyProfileDataExe = srcBase + "\\OptimizationData\\scripts\\ApplyProfileData.exe";

    runCmdToLog(applyProfileDataExe + " applyAll " + optDir + " " + optProfTempDir + " /managedOnly /verbose",
                runOpts);

    // ApplyProfileData does not search the binaries directory recursively, so repeat for optDir\wpf
    // (It easily could, but currently there are many extraneous DLLs in deep subtrees that cause name conflicts)
    runCmdToLog(applyProfileDataExe + " applyAll " + optDir + "\\wpf" + " " + optProfTempDir + " /managedOnly /verbose",
                runOpts);

    // clear out any temporary files
    if (FSOFolderExists(optProfTempDir))
        TryDeleteFolder(optProfTempDir, true);

    logMsg(LogClrAutomation, LogInfo, "} IbcOptimize\n");
}

/******************************************************************************/
/* given a runtime build 'inDir' create a optimized verison 'optDir' that has
   managed profile inforation (IBC), attached to it.
   Also handles the case of remote machine execution.

  Parameters
    inDir        : the starting binaries (read-only, unless inDir == optDir)
    optDir       : the path to place the optimized managed binaries,
                   defaults to inDir + ".opt".  This can be the same path
                   as the inDir, in which case the managed binaries are
                   modified in place.
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    installDir   : the installed path of 'inDir' if we have already installed
                   else undefined if we must install the runtime in 'inDir'
                   If undefined we also cleanup by removing the private runtime
                   that we installed.
    logDir       : Where to put the log files (defaults to the optDir)
    srcBase      : The base of the VBL (need to get the BBT tools)
    localBase    : The base of the local copy of files from the VBL
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)

    Returns 0 if successful, non zero for failure

*/

function IbcPhase(inDir, optDir, scenarioSpec, installDir, logDir, srcBase, localBase, targetArch, bldType, remoteMachine) {

    logCall(LogClrAutomation, LogInfo, "IbcPhase", arguments, " {");
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (localBase == undefined)
        localBase = srcBase;
    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;
    if (optDir == undefined)
        optDir = inDir + ".opt";
    if (logDir == undefined)
        logDir = optDir + "\\IbcLogs";

    var ibcDataDir = optDir + "\\IbcData";

    var isRemote = (remoteMachine == "true");
    var run;
    var runExitCode = undefined;

    // Ibc is not avilable for IA64 due to data misalignment issues
    if (targetArch == "ia64")
    {
        /* { */
        logMsg(LogClrAutomation, LogInfo, "} IbcPhase Skipped on IA64\n");
        return 0;
    }

    if (isNewRetailLab()) {
        // Input runtime must be installed before starting the IbcPhase
        if (installDir == undefined) {
            throw new Error(1, ".NET Framework must be installed before running the IBC phase\r\n");
        }
    }

    if ((remoteMachine != undefined) && !isRemote)
    {
        logMsg(LogClrAutomation, LogInfo, "IbcPhase will be run remotely on ", remoteMachine, "\n");

        // We forward this call to the remote machine via a recursive "runCmd runjs" call
        // but on the rescursive call we have remoteMachine set to true.

        var runJsCmd = uncPath(srcBase +"\\ndp\\clr\\bin\\runjs.bat");
        var logFileInfoPrefix = logDir +"\\IbcPhase." + remoteMachine;
        var logFile = logFileInfoPrefix + ".log";
        var remoteRunOpts = runSetOutput(logFile,
                            runSetDump(runJsCmd + " dumpProc %p " + logFileInfoPrefix + ".dmpRpt",
                            runSetTimeout(60 * MINUTE,
                            runSetIdleTimeout(60,
                            runSetMachine(remoteMachine)))));

        var remoteLocalBase = localBase;
        if (srcBase == localBase)
            remoteLocalBase = gsDefaultRemoteLocalBase;

        var remoteCmdLine = FormatCmdLine(runJsCmd,
                                          "IbcPhase",
                                          uncPath(inDir),
                                          uncPath(optDir),
                                          scenarioSpec,
                                          "_",
                                          uncPath(logDir),
                                          uncPath(srcBase),
                                          remoteLocalBase,
                                          targetArch,
                                          bldType,
                                          true);

        run = runCmd(remoteCmdLine, remoteRunOpts);
        runExitCode = run.exitCode;
    }
    else
    {
        if (isRemote) {
            logMsg(LogClrAutomation, LogInfo, "IbcPhase is running on remote\n");
            InitLocalBase(localBase, srcBase, logDir);
        }

        if (NeedToEnterWow64(targetArch)) {
            run = runX86("IbcPhase",
                         inDir, optDir, scenarioSpec, installDir,
                         logDir, srcBase, localBase, targetArch, bldType);
            runExitCode = run.exitCode;
        }
        else
        {
            if (!isNewRetailLab()) {
                if (!isCoreCLRBuild(bldType) && installDir == undefined)
                {
                    installDir = clrSetupWithCache(inDir, "/mz /fx", logDir, srcBase + "\\binaries\\clrSetupCache", targetArch + bldType);
                    StopNgenService(targetArch + bldType, targetArch);
                }
            }
            IbcProfile(scenarioSpec, ibcDataDir, logDir + "\\Profile", installDir, localBase, inDir);
            IbcOptimize(bldType, scenarioSpec, inDir, ibcDataDir, optDir, logDir + "\\Optimize", localBase, installDir);
            runExitCode = 0;
        }
    }

    logMsg(LogClrAutomation, LogInfo, "} IbcPhase ", (runExitCode==0) ? "Success" : "Failed", "\n");

    return runExitCode;
}

/******************************************************************************/
/* given a list of Dll path names, copy them into the runtime at 'installDir'
   if 'installInGac' is true, also install them into the GAC.
 */
function installDlls(dllList, installDir, logDir, installInGac) {

    logCall(LogClrAutomation, LogInfo, "installDlls", arguments, " {");

    if (!installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i))
        throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");

    var versionDir = RegExp.$2;

    for (var i = 0; i < dllList.length; i++) {
        var dll = dllList[i];
        if (!dll.match(/(.*)\\(([^\\]*)\.(.+))$/))
            throw Error(1, "Need path for " + dll);
        var dllDir = RegExp.$1;
        var dllFile = RegExp.$2;
        var dllFileBase = RegExp.$3;
        var dllFileExt = RegExp.$4;
        var pdb = dllDir + "\\symbols.pri\\retail\\" + dllFileExt + "\\" + dllFileBase + ".pdb";

        var target = installDir + "\\" + dllFile;
        FSOCopyFile(dll, target, "FORCE");

        var targetPdb = installDir + "\\" + dllFileBase + ".pdb";
        if (FSOFileExists(pdb))
            FSOCopyFile(pdb, targetPdb, "FORCE");
        else  {
            // The rational here is that for managed DLLs, the original PDB is still good
            if (!installInGac)
                logMsg(LogClrAutomation, LogWarn, "installDlls: could not find pdb file ", pdb, "\n");
        }

        if (installInGac)
            runCmdToLog(installDir + "\\gacutil -i " + target,
                runSetDump("runjs dumpProc %p " + logDir + "\\gacutil.dmpRpt",
                runSetIdleTimeout(60,
                runSetEnv("COMPLUS_DefaultVersion", versionDir,
                runSetEnv("COMPLUS_Version", versionDir,
                clrRunTemplate)))));
    }
    logMsg(LogClrAutomation, LogInfo, "} installDlls\n");
}

/******************************************************************************/
/* copies all of the "known" architecture neutral assemblies from inDir into outDir
 */
function copyArchNeutralDlls(inDir, outDir, targetArch, bldType, logDir) {

    logCall(LogClrAutomation, LogInfo, "copyArchNeutralDlls", arguments, " {");

    if (inDir == undefined)
        throw new Error(1, "Required argument 'inDir' is missing");

    if (outDir == undefined)
        throw new Error(1, "Required argument 'outDir' is missing");

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (logDir == undefined)
        logDir = outDir;

    var dllInPath  = inDir  + "\\";
    var pdbInPath  = inDir  + "\\symbols.pri\\retail\\dll\\";
    var dllOutPath = outDir + "\\";
    var pdbOutPath = outDir + "\\symbols.pri\\retail\\dll\\";
    var savePath   = outDir + "\\Saved_ArchNeutral_" + targetArch;

    // archNeutralDlls are the list of dlls that we expect
    //  to be in the Gac as architecture neutral
    var archNeutralDlls = ["Accessibility",
                           "AspNetMMCExt",
                           "IEExecRemote",
                           "IEHost",
                           "IIEHost",
                           "sysglobl",
                           "System",
                           "System.Configuration",
                           "System.Configuration.Install",
                           "System.Data.SqlXml",
                           "System.Deployment",
                           "System.Design",
                           "System.DirectoryServices",
                           "System.DirectoryServices.Protocols",
                           "System.Drawing",
                           "System.Drawing.Design",
                           "System.Management",
                           "System.Messaging",
                           "System.Runtime.Remoting",
                           "System.Runtime.Serialization.Formatters.Soap",
                           "System.Security",
                           "System.ServiceProcess",
                           "System.Web.Mobile",
                           "System.Web.RegularExpressions",
                           "System.Web.Services",
                           "System.Windows.Forms",
                           "System.Xml"];

    for(var i = 0; i < archNeutralDlls.length; i++) {
        var assembly = archNeutralDlls[i];
        var dllIn    = dllInPath  + assembly + ".dll";
        var pdbIn    = pdbInPath  + assembly + ".pdb";
        var dllOut   = dllOutPath + assembly + ".dll";
        var pdbOut   = pdbOutPath + assembly + ".pdb";
        var dllSave  = savePath   + assembly + ".dll";
        var pdbSave  = savePath   + assembly + ".pdb";

        if (!FSOFileExists(dllIn)) {
            logMsg(LogClrAutomation, LogWarn, "copyArchNeutralDlls: Could not find file ", dllIn, " skipping\n");
            continue;
        }

        FSOCopyFile(dllIn, dllSave, "FORCE");
        FSOCopyFile(pdbIn, pdbSave, "FORCE");
        FSOCopyFile(dllIn, dllOut,  "FORCE");
        FSOCopyFile(pdbIn, pdbOut,  "FORCE");
    }

    // @ToDo: where should we get these from?
    var otherArchNeutralDlls = ["cscompmgd",
                                "Microsoft.Build.Engine",
                                "Microsoft.Build.Framework",
                                "Microsoft.Build.Tasks",
                                "Microsoft.Build.Utilities",
                                "MS.ErrorReporting",
                                "Microsoft.JScript",
                                "Microsoft.VisualBasic",
                                "Microsoft.VisualC",
                                "Microsoft.Vsa"];

    logMsg(LogClrAutomation, LogInfo, "} copyArchNeutralDlls\n");
}


/******************************************************************************/
/* we need to run scenarios to be able to optimize for them.  A scenario
   contains all the information needed to run a particular application for
   BBT, IBC or performance testing
   These are the descriptions for the new scenario format */

var ndpScenarios = [];

ndpScenarios.push({
    name : "HelloWorld_CS",
    path: "%localBase%\\OptimizationData\\scenarios\\HelloWorld_CS",
    exe:  "HelloWorld.exe",
    ngen:  ["HelloWorld.exe"],
    depends: [],
    ibcRuns : 3,
    svrRuns : 1,
    wksRuns : 2,
    platforms: "x86 amd64 ia64",
    categories: "ibc pogo bbt ngen test"
    });
ndpScenarios.push({
    name : "ShowFormComplexIdle",
    path:  "%localBase%\\OptimizationData\\scenarios\\ShowFormComplex",
    exe:   "ShowFormComplexPJ.exe",
    ngen:  ["ShowFormComplexPJ.exe"],
    depends: ["System", "System.Drawing", "System.Windows.Forms"],
    ibcRuns : 3,
    svrRuns : 1,
    wksRuns : 2,
    gui: true,
    platforms: "x86 amd64 ia64",
    categories: "ibc pogo bbt ngen test"
    });
ndpScenarios.push({
    name : "ShowFormComplexExit",
    path: "%localBase%\\OptimizationData\\scenarios\\ShowFormComplex",
    exe: "ShowFormComplexPJ.exe",
    ngen:  ["ShowFormComplexPJ.exe"],
    args:  "SFC.ngen.output",
    depends: ["System", "System.Drawing", "System.Windows.Forms"],
    ibcRuns : 3,
    svrRuns : 1,
    wksRuns : 2,
    platforms: "x86 amd64 ia64",
    categories: "ibc pogo bbt ngen"
    });
ndpScenarios.push({
    name : "ShowFormComplexJit",
    path:  "%localBase%\\OptimizationData\\scenarios\\ShowFormComplex",
    exe:   "ShowFormComplexJit.exe",
    ngen:  [],
    args:  "SFC.jit.output",
    depends: ["System", "System.Drawing", "System.Windows.Forms"],
    ibcRuns : 3,
    svrRuns : 1,
    wksRuns : 2,
    platforms: "x86 amd64 ia64",
    categories: "test pogo bbt test"
    });
ndpScenarios.push({
    name : "CreateDomain",
    path: "%localBase%\\OptimizationData\\scenarios\\CreateDomain",
    exe:  "CreateDomain.exe",
    ngen:  ["CreateDomain.exe"],
    depends: [],
    ibcRuns : 3,
    svrRuns : 2,
    wksRuns : 1,
    platforms: "x86 amd64 ia64",
    categories: "ibc pogo bbt ngen test"
    });
ndpScenarios.push({
    name : "NgenTraining",
    path: "%localBase%\\OptimizationData\\scenarios\\NgenTraining",
    cmd:  "NgenTraining.cmd",
    args: "%INSTALL_DIR% install System.Runtime.Remoting.dll Microsoft.JScript.dll",
    depends: [],
        //    envVar: "COMPLUS_CheckNGenImageTimeStamp",
        //    envVal: "0",
    ibcRuns : 0,
    svrRuns : 1,
    wksRuns : 0,
    platforms: "amd64 ia64",
    categories: "pogo ngen"
    });
ndpScenarios.push({
    name : "GcBench",
    path: "%localBase%\\OptimizationData\\scenarios\\GCBench",
    exe: "GCBench.exe",
    ngen: ["GCBench.exe"],
    depends: [],
    ibcRuns : 1,
    svrRuns : 2,
    wksRuns : 1,
    platforms: "amd64 ia64",
    categories: "pogo bbt ngen test"
    });
ndpScenarios.push({
    name : "PaintDotNet",
    path: "%localBase%\\OptimizationData\\scenarios\\Paint.NET",
    exe: "PaintDotNet.exe",
    ngen: ["PaintDotNet.exe"],
    depends: ["System", "System.Drawing", "System.Windows.Forms"],
    ibcRuns : 1,
    svrRuns : 0,
    wksRuns : 1,
    gui: true,
    platforms: "amd64 ia64",
    categories: "ibc bbt ngen test"
    });
ndpScenarios.push({
    name : "BizTalk",
    path: "%localBase%\\OptimizationData\\scenarios\\BizTalk",
    exe:  "PerfMark.exe",
    ngen: ["PerfMark.exe", "biztalk.dll"],
    args: "-v -a biztalk.dll -c PerfTest.BizTalk.Biztalk -w 1 -d 2 -tp 100 -m Run",
    depends: ["System", "System.Xml", "System.Data.SqlXml"],
    ibcRuns : 1,
    svrRuns : 2,
    wksRuns : 1,
    platforms: "amd64 ia64",
    categories: "ibc pogo bbt ngen test"
    });
ndpScenarios.push({
    name : "SpecJBB",
    path: "%localBase%\\OptimizationData\\scenarios\\SpecJBB",
    exe:  "SpecJBB.exe",
    ngen:  ["SpecJBB.exe"],
    depends: [],
    ibcRuns : 1,
    svrRuns : 2,
    wksRuns : 0,
    platforms: "amd64 ia64",
    categories: "ibc pogo bbt ngen test"
    });
ndpScenarios.push({
    name : "IBuySpy",
    path: "%localBase%\\OptimizationData\\scenarios\\IBuySpy",
    cmd:  "runIBuySpy.cmd",
    setup_args: "setup",
    start_args: "start",
    args:    "run 30",
    pgoModuleList: ["clr", "clrjit"],
    depends: ["System", "System.Xml", "System.Data", "System.Web", "System.Data.SqlXml", "System.EnterpriseServices", "System.Configuration", "System.Drawing", "System.Web.RegularExpressions" ],
    ibcRuns : 1,
    svrRuns : 1,
    wksRuns : 0,
    platforms: "amd64 ia64",
    categories: "ibc pogo ngen"
    });

// This scenario should work correctly for the ret build, but it is not clear whether this will
// work correctly for the shp build (see the comment below about 'sandbox.dll'). However, it
// appears that the shp build does not use this script; instead it uses some other script that
// applys checked-in profile data.
//
// Note that for 'path' and 'args', the %localBase%, %installDir%, and %PROCESSOR_ARCHITECTURE%
// macros get resolved earlier inside ScenariosGet(), whereas %INSTALL_DIR% gets resolved later
// inside RunScenarios().

var coreCLRScenarios = [];
coreCLRScenarios.push({
    name : "HelloWorld_CS",
    path: "%INSTALL_DIR%",
    exe:  "fxprun.exe",
    args: "%localBase%\\OptimizationData\\scenarios\\coreclr\\HelloWorld_CS\\HelloWorld.exe",
    depends: [],
    ibcRuns : 5,
    svrRuns : 0,
    wksRuns : 2,
    platforms: "x86 amd64",
    categories: "pogo bbt test"
    });
/*
// Sandbox.dll is a managed dll used by fxprun.
// In order to use this dll on ship build, sandbox.dll needs to be signed with MS key.
// So until we figure out a story for sandbox, we will not use the following scenarios.
coreCLRScenarios.push({
    name : "HelloWorld_CS",
    path: "%installDir%",
    exe:  "fxprun.exe",
    args: "%localBase%\\OptimizationData\\scenarios\\coreclr\\HelloWorld_CS\\HelloWorld.exe",
    depends: [],
    ibcRuns : 5,
    svrRuns : 0,
    wksRuns : 2,
    categories: "pogo bbt test x86 amd64 ia64"
    });
coreCLRScenarios.push({
    name : "MD5_CS",
    path: "%installDir%",
    exe:  "fxprun.exe",
    args: "%localBase%\\OptimizationData\\scenarios\\coreclr\\MD5_CS\\11_MD5.exe",
    depends: [],
    ibcRuns : 5,
    svrRuns : 0,
    wksRuns : 2,
    categories: "pogo bbt test x86 amd64 ia64"
    });
coreCLRScenarios.push({
    name : "Tree_CS",
    path: "%installDir%",
    exe:  "fxprun.exe",
    args: "%localBase%\\OptimizationData\\scenarios\\coreclr\\Tree_CS\\13_Tree.exe",
    depends: [],
    ibcRuns : 5,
    svrRuns : 0,
    wksRuns : 1,
    categories: "pogo bbt test x86 amd64 ia64"
    });
*/

/******************************************************************************/
/* Return a list of scenarios to run given a string 'scenarioSpec'.  Also
   it is given the 'localBase' so that it can find the scenarios relative
   to this source base.  ScenarioSpec currently is defined as simply
   a regular expression that matchs the NAME of the scenario (use
   runjs scenariosShow to see the list
   installDir   : The path to the installed binaries
*/
var scenarioCache = undefined;

function scenariosGet(scenarioSpec, localBase, targetArch, bldType, installDir, silent) {

    logCall(LogClrAutomation, LogInfo, "scenariosGet", arguments, " {");

    if (targetArch == undefined) {
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    }

    if (localBase == undefined) {
        localBase = Env("_NTBINDIR");
        if (!localBase)
            throw new Error(1, "Required argument 'localBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    // Currently we don't use scenarioSpec.  We could treat it as
    // a pattern, or as a file name for XML, or other things.  I am
    // thinking of something like
    //  <pat>:<XMLFileName>
    // where the default pat is everthing, and default file name is
    // some XML file from the localBase
    //
    if (!scenarioCache) {
        scenarioCache = [];
        var scenarios;
        if (bldType != undefined && isCoreCLRBuild(bldType)) {
            scenarios = coreCLRScenarios;
        }
        else {
            scenarios = ndpScenarios;
        }
        for (var i = 0; i < scenarios.length; i++) {
            var scenario = scenarios[i];
            if (scenario.platforms.indexOf(targetArch) < 0) {
                if (!silent)
                    logMsg(LogClrAutomation, LogInfo, "scenariosGet: ", scenario.name, " not enabled for arch ", targetArch, "\n");
                continue;
            }

            var cmdName;

            if (scenario.exe)
                cmdName = scenario.exe;
            else if (scenario.cmd)
                cmdName = scenario.cmd;
            else
                logMsg(LogClrAutomation, LogInfo, "scenariosGet: ", scenario.name, " does not have an exe or cmd defined!\n");

            scenario.path = scenario.path.replace(/%localBase%/ig, localBase);
            scenario.path = scenario.path.replace(/%PROCESSOR_ARCHITECTURE%/ig, targetArch);
            scenario.path = scenario.path.replace(/%installDir%/ig, installDir);
            scenario.path = scenario.path.replace(/%INSTALL_DIR%/ig, installDir);

            if (scenario.ngen == undefined) {
                scenario.ngen = [];
            }

            if (scenario.args == undefined)
                scenario.args = "";
            else {
                scenario.args = scenario.args.replace(/%localBase%/ig, localBase);
                scenario.args = scenario.args.replace(/%PROCESSOR_ARCHITECTURE%/ig, targetArch);
                scenario.args = scenario.args.replace(/%installDir%/ig, installDir);
                scenario.args = scenario.args.replace(/%INSTALL_DIR%/ig, installDir);
            }

            if (scenario.ibcRuns == undefined)
                scenario.ibcRuns = 1;

            if (scenario.wksRuns == undefined)
                scenario.wksRuns = 1;

            if (scenario.svrRuns == undefined)
                scenario.svrRuns = 1;

            if (scenario.gui == undefined)
                scenario.gui = false;

            if (!silent) {
                logMsg(LogClrAutomation, LogInfo, "scenariosGet: ", scenario.name, ": ", scenario.path + "\\" + cmdName, " " +  scenario.args, "\n");
            }
            scenarioCache.push(scenario);
        }
    }

    logMsg(LogClrAutomation, LogInfo, "} scenariosGet\n");

    return scenarioCache;
}

/******************************************************************************/
/* Displays the current list of scenarios */
function scenariosShow(scenarioSpec, localBase) {

    if (localBase == undefined) {
        localBase = Env("_NTBINDIR");
        if (!localBase)
            throw new Error(1, "Required argument 'localBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    var scenarios = scenariosGet(scenarioSpec, localBase, undefined, undefined, true);

    for (var i = 0; i < scenarios.length; i++) {
        var scenario = scenarios[i];
        logMsg(LogClrAutomation, LogInfo, "Scenario ", scenario.name, " {\n");
        logMsg(LogClrAutomation, LogInfo, "platforms:  ", scenario.platforms, "\n");
        logMsg(LogClrAutomation, LogInfo, "categories: ", scenario.categories, "\n");
        logMsg(LogClrAutomation, LogInfo, "path:    ", scenario.path, "\n");
        logMsg(LogClrAutomation, LogInfo, "exe:     ", scenario.exe, "\n");
        logMsg(LogClrAutomation, LogInfo, "args:    ", scenario.args, "\n");
        logMsg(LogClrAutomation, LogInfo, "ngen:    ", scenario.ngen, "\n");
        logMsg(LogClrAutomation, LogInfo, "depends: ", scenario.depends, "\n");
        if (scenario.envVar != undefined) {
            logMsg(LogClrAutomation, LogInfo, "envVar:  ", scenario.envVar, "\n");
            logMsg(LogClrAutomation, LogInfo, "envVal:  ", scenario.envVal, "\n");
        }
        logMsg(LogClrAutomation, LogInfo, "ibcRuns: ", scenario.ibcRuns, "\n");
        logMsg(LogClrAutomation, LogInfo, "wksRuns: ", scenario.wksRuns, "\n");
        logMsg(LogClrAutomation, LogInfo, "svrRuns: ", scenario.svrRuns, "\n");
        logMsg(LogClrAutomation, LogInfo, "isGui (waits for user after startup): ", scenario.gui, "\n");
        logMsg(LogClrAutomation, LogInfo, "}\n");
    }
}

/******************************************************************************/
/* Parses the contents of ndpsetup.txt and returns it in an array. This function
   is based on ParseSetupList() from "tools\devdiv\BatchSetup\ndpsetup.js".

  Parameters
    listFilename : the path to ndpsetup.txt
*/

function ParseNdpSetupTxt(listFilename) {

    var setuplist = new Array();
    var listFile;

    if (!FSOFileExists(listFilename)) {
        throw new Error(1, "Could not find \"" + listFilename + "\"");
    }

    try {
        try {
            listFile = FSOOpenTextFile(listFilename, FSOForReading);
        }
        catch(e) {
            throw new Error(1, "Unable to open file \"" + listFilename + "\"");
        }

        var listLine;

        while (!listFile.AtEndOfStream) {
            var lineNumber = listFile.Line;
            listLine = listFile.ReadLine();

            // ignore comment lines
            if (listLine.substr(0, 1) == ";") {
                continue;
            }

            var errMsg = "Invalid format in ndpsetup.txt line " + lineNumber + ": " + listLine;

            var fi = new Object();
            var lineArgs = listLine.split(":");

            // ignore blank lines
            if (lineArgs.length == 1 && !listLine.match(/\S/)) {
                continue;
            }

            if (lineArgs.length < 4) {
                throw new Error(1,errMsg);
            }

            // process required fields for each file
            fi.file = lineArgs[0];
            fi.target = lineArgs[1];
            fi.copyFile = lineArgs[2];
            fi.group = lineArgs[3];

            // process options
            for (var i = 4; i < lineArgs.length; i++) {
                if (lineArgs[i]) {
                    if (lineArgs[i].match(/(\w+)=(\w+)/)) {
                        fi[RegExp.$1] = RegExp.$2;
                    }
                    else {
                        fi[lineArgs[i]] = true;
                    }
                }
            }

            setuplist.push(fi);

        } // while (!listFile.AtEndOfStream)

        listFile.Close();
    }
    catch (e) {
        if (listFile != null) {
            listFile.Close();
        }

        throw e;
    }

    return setuplist;
}

/******************************************************************************/
/* Gets the path to the share by parsing ClrSetupInfo.bat from the local
   drop folder.

  Parameters
    inDir        : the path to the local drop folder
*/

function GetPathToShare(inDir) {

    var setupInfo = parseBatchFileForSetStatements(inDir + "\\ClrSetupInfo.bat");
    var pathToShare = setupInfo["CLR_DROP_PATH_HEAD"] + "\\" +
        setupInfo["CLR_CERTIFIED_VERSION"] + "\\binaries." +
        setupInfo["CLR_BUILD_ARCH"] + "ret";
    return pathToShare;
}

/******************************************************************************/
/* Copies the WPF bits from the share to the local drop folder. This function
   does nothing for IA64.

  Parameters
    inDir        : the path to the local drop folder
    targetArch   : the target architecture
    logDir    : the directory where logs need to be saved
    certDrops : The location of the certified drops (Default: value in clrsetupinfo.bat in inDir)
*/

function CopyDownWPFBits(inDir, targetArch, logDir, certDrops) {

    logCall(LogClrAutomation, LogInfo, "CopyDownWPFBits", arguments, " {");

    // Nothing to do for IA64
    if (targetArch.match(/^(ia64)/i)) {
        logMsg(LogClrAutomation, LogInfo, "WPF is not supported on IA64\n");
        return;
    }

    // If WPF bits already exist so we don't need to copy them from the share
    var targetPath = inDir + "\\WPF";
    if (FSOFileExists(targetPath + "\\PresentationCore.dll")) {
        logMsg(LogClrAutomation, LogInfo, "WPF bits found at " + targetPath + "\n");
        return;
    }

    // If WPF bits do not exist, then we need to copy them from the share

    // If the WPF folder foes not exist, then we need to create it
    if (!FSOFolderExists(targetPath)) {
        FSOCreateFolder(targetPath, true);
    }
    var pathToShare = undefined;
    if (certDrops == undefined) {
        pathToShare = GetPathToShare(inDir);
    } else {
        pathToShare = certDrops;
    }
    var sourcePath = pathToShare + "\\WPF";

    logMsg(LogClrAutomation, LogInfo, "WPF bits could not found at " + targetPath + "\n");
    logMsg(LogClrAutomation, LogInfo, "Copying WPF bits:\n");
    logMsg(LogClrAutomation, LogInfo, "Source: " + sourcePath + "\n");
    logMsg(LogClrAutomation, LogInfo, "Dest:   " + targetPath + "\n");

    // Exclude the "WPF\Test" directory because it is very big
    // and we do not need it to install the runtime
    var options = "/nfl /ndl /xd " + sourcePath + "\\test ";

    robocopy(sourcePath, targetPath, options, logDir + "\\WpfBitsCopy.log.txt");

    logMsg(LogClrAutomation, LogInfo, "} CopyDownWPFBits\n");
}

/******************************************************************************/
/* Copies the VC bits from the share to the local drop folder.

  Parameters
    inDir        : the path to the local drop folder
    targetArch   : the target architecture
    certDrops : The location of the certified drops (Default: value in clrsetupinfo.bat in inDir)
*/
function CopyDownVCBits(inDir, targetArch, certDrops) {

    logCall(LogClrAutomation, LogInfo, "CopyDownVCBits", arguments, " {");

    // Nothing to do for IA64
    if (targetArch.match(/^(ia64)/i)) {
        logMsg(LogClrAutomation, LogInfo, "VC bits optimization is not supported on IA64\n");
        return;
    }

    var arch = targetArch;
    if (targetArch.match(/^(x86)/i)) {
        arch = "i386";
    }

    var targetPath = inDir;
    var pathToShare = undefined;
    if (certDrops == undefined) {
        pathToShare = GetPathToShare(inDir);
    } else {
        pathToShare = certDrops;
    }
    var sourcePath = pathToShare;
    var sMsvcrClrDllName =  "msvcr110_clr0400.dll";
    var sMsvcrClrPdbName = "msvcr110_clr0400." + arch + ".pdb";
    if (!FSOFileExists(targetPath + "\\" + sMsvcrClrDllName)) {
        logMsg(LogClrAutomation, LogInfo, "VC bits could not be found at " + targetPath + "\n");
        logMsg(LogClrAutomation, LogInfo, "Copying VC bits:\n");
        logMsg(LogClrAutomation, LogInfo, "Source: " + sourcePath + "\n");
        logMsg(LogClrAutomation, LogInfo, "Dest:   " + targetPath + "\n");

        // copy the optimized binaries
        FSOCopyFile(sourcePath + "\\" + sMsvcrClrDllName, targetPath + "\\" + sMsvcrClrDllName, true);
        FSOCopyFile(sourcePath + "\\Symbols.pri\\retail\\dll\\" + sMsvcrClrPdbName, targetPath + "\\Symbols.pri\\retail\\dll\\" + sMsvcrClrPdbName, true);

        // copy the unoptimized binaries
        if (FSOFileExists(sourcePath + "\\unopt\\" + sMsvcrClrDllName)) {
            FSOCopyFile(sourcePath + "\\unopt\\" + sMsvcrClrDllName, targetPath + "\\unopt\\" + sMsvcrClrDllName, true);
            FSOCopyFile(sourcePath + "\\unopt\\" + sMsvcrClrPdbName, targetPath + "\\unopt\\" + sMsvcrClrPdbName, true);
        } else {
            FSOCopyFile(sourcePath + "\\" + sMsvcrClrDllName, targetPath + "\\unopt\\" + sMsvcrClrDllName, true);
            FSOCopyFile(sourcePath + "\\" + sMsvcrClrPdbName, targetPath + "\\unopt\\" + sMsvcrClrPdbName, true);
        }
    } else {
        logMsg(LogClrAutomation, LogInfo, "VC bits found at " + targetPath + ". No need to copy from remoteStore\n");
    }

    logMsg(LogClrAutomation, LogInfo, "} CopyDownVCBits\n");
}

/******************************************************************************/
/* Starts or restarts a service with environment variables set.  It does so
   by temporarily setting them machine-wide (note that they will not persist after
   this function returns).  It will delete any existing values.

  Parameters
    svcName      : the name of the service to start/restart
    envString    : the environment variables, formatted as env1=value,env2=value,...
*/

function StartServiceWithEnvironment(svcName, envString) {

    logCall(LogClrAutomation, LogInfo, "StartServiceWithEnvironment", arguments, " {");

    var runOpts = runSetTimeout(60, runSetNoThrow());

    var envPairs = envString.split(",");

    runCmdToLog("net stop " + svcName, runOpts);

    for (var i = 0; i < envPairs.length; i++)
    {
        var envElements = envPairs[i].split("=");
        var envVarName = envElements[0];
        var envVarValue = envElements[1];

        runCmdToLog("reg add \"HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\Environment\" /v " + envVarName + " /d " + envVarValue + " /t REG_EXPAND_SZ /f", runOpts);
    }

    runCmdToLog("net start " + svcName, runOpts);

    for (var i = 0; i < envPairs.length; i++)
    {
        var envElements = envPairs[i].split("=");
        var envVarName = envElements[0];
        var envVarValue = envElements[1];

        runCmdToLog("reg delete \"HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\Environment\" /v " + envVarName + " /f", runOpts);
    }

    logMsg(LogClrAutomation, LogInfo, "} StartServiceWithEnvironment\n");
}

/******************************************************************************/
/* starting with an unoptimized runtime build in 'inDir' create an optimized
   runtime build and place it in 'optDir' (it destroyes what is there).
   'scenarioSpec' is a specification of the scenarios to be run for doing the
   profile run (see scenariosGet for details).  It can be left undefined
   to for the default set of scenarios.  'srcBase is the base of the vbl
   and is used to get tools needed for the optimization.
   Upon success completion of this function thereturn value will be set
   to the installDir that will contain the installed  optimized binaries
   (both managed and unmanaged) ready to run.  'finalOptDir'  will also contain
   a complete build with the optimized binaries.

  Parameters
    inDir        : the starting unoptimize retail build.
                   If Pogo is enabled the inDir must have been built
                   using /LTCG, (this is the default for amd64ret and ia64ret)
    finalOptDir  : the path to place the optimized binaries.
                   (defaults to inDir + ".opt")
    targetArch   : The target architecture (defaults to the current processor architecture)
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the finalOptDir)
    srcBase      : The base of the VBL (so we can find razzle)
    neutralInDir : the retail build containing the arch neutral managed assemblies (ignored if undefined)
                   This arg is used to simulate layout installs for 64-bit platforms
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)
    localBase    : The base of the local copy of files from the VBL
    bldType      : The type of build, eg ret, coreret, coreshp
    installDir    : The installation directory
    certDrops    : Location to the certified drop (Default: value in clrsetupinfo.bat)
*/

function retailBuild(inDir, finalOptDir, targetArch, scenarioSpec, logDir, srcBase, neutralInDir, remoteMachine, localBase, bldType, installDir, certDrops) {

    logCall(LogClrAutomation, LogInfo, "retailBuild", arguments, " {");
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    var versionDir = undefined;
    var buildNumber = undefined;
    if (isNewRetailLab()) {
        // The input runtime must be installed before running retailBuild
        if (installDir == undefined) {
            throw new Error(1, "Required argument 'installDir' is missing\r\n");
        }
        if (!installDir.match(/Microsoft.NET\\Framework(64)?\\([^\\]*)$/i)) {
            throw Error(1, "installDir '" + installDir + "' needs to be in %WINDIR%\Microsoft.NET\\Framework(64)");
        }
        versionDir = RegExp.$2;
        buildNumber = versionDir.split(".")[2];        
    }

    if (bldType == undefined) {
        if (FSOFileExists(inDir + "\\coreclr.dll")) {
            bldType = "coreret";
        }
        else {
        bldType = "ret";
        }
    }
    if (localBase == undefined)
        localBase = srcBase;
    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;
    if (finalOptDir == undefined)
        finalOptDir = inDir + ".opt";
    if (logDir == undefined)
        logDir = finalOptDir;
    if (neutralInDir == undefined)
        neutralInDir = inDir;    

    if (FSOFolderExists(finalOptDir)) {
        logMsg(LogClrAutomation, LogInfo, "retailBuild: deleting ", finalOptDir, "\n");
        FSODeleteFolder(finalOptDir, true);     // clear out any old stuff
    }
    logMsg(LogClrAutomation, LogInfo, "Creating Log Folder ", logDir, "\n");
    FSOCreatePath(logDir);

    logMsg(LogClrAutomation, LogInfo, "retailBuild: input build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "retailBuild: optimized will be put in ", finalOptDir, "\n");

    if (targetArch.match(/^(x86)/i)) {
        //
        // In order to enable Pogo for x86 the file ndp/clr/project.mk must be
        // modified to define LTCG=1 on x86 (see line 235 of project.mk)
        // Also note that on x86 the Bbt tools fail with asserts when processing
        // clr.dll that was built using LTCG
        //
        gsPogoEnabled = false;
        gsBbtEnabled  = true;
    }
    else if (targetArch.match(/^(amd64)/i)) {
        gsBbtEnabled  = true;
    }

    // We don't run Ibc for CoreClr build
    var clrDllName = CLRDllName(bldType);
    if (isCoreCLRBuild(bldType)) {
    logMsg(LogClrAutomation, LogInfo, "Disabling IBC because build type is CoreClr\n");
        gsIbcEnabled = false;
    }

    // We don't run Bbt or Pogo unless we have a retail build
    if (!inDir.match(/(.*)(ret|RET)(.*)$/)) {
        logMsg(LogClrAutomation, LogInfo, "Disabling BBT and PGO because build flavor is not retail\n");
        gsPogoEnabled = false;
        gsBbtEnabled = false;
    }

    // Disable IBC for IA64
    if (targetArch.match(/^(ia64)/i)) {
        gsIbcEnabled = false;
    logMsg(LogClrAutomation, LogInfo, "Disabling IBC because architecture is IA64\n");
    }

    // For Desktop builds, we need to copy down the WPF bits
    // from the share into the local drop folder
    if (!isCoreCLRBuild(bldType)) {
        CopyDownWPFBits(inDir, targetArch, logDir, certDrops);
        CopyDownVCBits(inDir, targetArch, certDrops);
    }

    // Setup ibcInDir and ibcOptDir
    var ibcInDir;
    var ibcOptDir;

    if (gsIbcEnabled)
    {
        ibcInDir   = inDir;
        ibcOptDir  = inDir + ".ibc";
    }
    else
    {
        ibcOptDir  = inDir;
    }

    // Setup pogoInDir and pogoOptDir
    var pogoInDir;
    var pogoOptDir;

    if (gsPogoEnabled)
    {
        pogoInDir   = ibcOptDir;
        pogoOptDir  = ibcOptDir + ".pgO";
    }
    else
    {
        pogoOptDir  = ibcOptDir;
    }

    // Setup pogoInDir and pogoOptDir
    var bbtInDir;
    var bbtOptDir;

    if (gsBbtEnabled)
    {
        bbtInDir   = pogoOptDir;
        bbtOptDir  = pogoOptDir + ".bbO";
    }
    else
    {
        bbtOptDir  = pogoOptDir;
    }

    // Determine which optimization happens last
    if (gsBbtEnabled) {
        bbtOptDir = finalOptDir;
    }
    else if (gsPogoEnabled) {
        pogoOptDir = finalOptDir;
    }
    else if (gsIbcEnabled) {
        ibcOptDir  = finalOptDir;
    }
    else {
        // No optimization selected, the final optimized build is just an exact copy of the inDir
        logMsg(LogClrAutomation, LogInfo, "retailBuild: copying the unmodified files to create a complete drop in " + finalOptDir + "\n");
        robocopy(inDir, finalOptDir, "/xo", logDir + "\\allUnmodifiedCopy.log.txt");
    }

    var profileDlls = undefined;
    var saveDir     = undefined;

    if (gsIbcEnabled) {
        // generate the optimized managed binaries
        logMsg(LogClrAutomation, LogInfo, "retailBuild: generate optimized managed Dlls\n");
        ibcFiles = IbcPhase(ibcInDir, ibcOptDir, scenarioSpec, installDir, logDir, srcBase, localBase, targetArch, bldType, remoteMachine);
    }

    // Note that the IBC data has to have already been applied so that we take the
    // same code paths in clr in both the Pogo and BBT phases.
    // Thus the IBC phase has to be first

    if (gsPogoEnabled) {
        // generate the optimized pogo binaries
        logMsg(LogClrAutomation, LogInfo, "retailBuild: generate pogo optimized binaries\n");
        PogoPhase(pogoInDir, undefined, pogoOptDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine);
    }

    // Note that the BbPhase must come after PogoPhase as it works directly on unmanaged PE files

    if (gsBbtEnabled) {
        // generate the optimized unmanaged binaries.
        logMsg(LogClrAutomation, LogInfo, "retailBuild: generate new bbt optimized unmanaged Dlls\n");

        if (!isCoreCLRBuild(bldType)) {
            BbtPhase(bbtInDir, bbtOptDir, installDir, logDir, srcBase, targetArch, bldType);
        } else {
            BbtPhase_Old(bbtInDir, undefined, bbtOptDir, scenarioSpec, logDir, srcBase, localBase, targetArch, remoteMachine, bldType);
        }
    }

    if (ibcOptDir != finalOptDir && ibcOptDir != inDir) {
        // delete the temporary ibc optimized dir
        logMsg(LogClrAutomation, LogInfo, "retailBuild: removing temporary ibc optimized binaries in ", ibcOptDir, "\n");
        TryDeleteFolder(ibcOptDir);
    }

    if (!isNewRetailLab() && inDir != neutralInDir) {
        logMsg(LogClrAutomation, LogInfo, "retailBuild: copying domain neutral assemblies from " + neutralInDir +" to simulate layout install\n");
        copyArchNeutralDlls(neutralInDir, finalOptDir, targetArch, bldType, logDir);
    }

    // Copy freshly generated coffbase and coffsize files to finalOptDir
    {
        var coffbaseFilename = "coffbase.txt";
        var coffsizeFilename = "coffsize.txt";
        if (isCoreCLRBuild(bldType))
        {
            coffbaseFilename = "coffbasecore.txt";
            coffsizeFilename = "coffsizecore.txt";
        }

        var iarch = targetArch.toLowerCase();
        if (iarch == "x86")
            iarch = "i386";

        var coffbaseSourcePath = srcBase + "\\public\\internal\\NDP\\coffbase\\" + iarch;
        var coffsizeSourcePath = srcBase + "\\public\\internal\\NDP\\coffsize\\" + iarch;

        var destPath = finalOptDir + "\\CoffBase";

        if (!FSOFolderExists(destPath))
        {
            FSOCreatePath(destPath);
        }

        logMsg(LogClrAutomation, LogInfo, "retailBuild: copying the coffbase/coffsize files to " + destPath + "\n");
        robocopyFiles(coffbaseSourcePath, destPath, "", logDir + "\\CoffbaseCopy.log.txt", coffbaseFilename);
        robocopyFiles(coffsizeSourcePath, destPath, "", logDir + "\\CoffsizeCopy.log.txt", coffsizeFilename);
    }

    logMsg(LogClrAutomation, LogInfo, "retailBuild: optimized bits in ", finalOptDir, "\n");
    if (!isNewRetailLab()) {
        logMsg(LogClrAutomation, LogInfo, "retailBuild: Installing the retail optimized runtime\n");
    }

    if (gsPogoEnabled && FSOFolderExists(inDir + ".pgI")) {
        TryDeleteFolder (inDir + ".pgI");
    }

    if (gsBbtEnabled && FSOFolderExists(inDir + ".bbI")) {
        TryDeleteFolder (inDir + ".bbI");
    }

    if (gsPogoEnabled) {
        if (pogoOptDir != finalOptDir && pogoOptDir != inDir) {
            // delete the temporary pgo optimized dir
        logMsg(LogClrAutomation, LogInfo, "retailBuild: removing temporary pgo optimized binaries in ", pogoOptDir, "\n");
            TryDeleteFolder(pogoOptDir);
        }
    }

    // copy optimized binaries to signed directory because that is what the setup build uses
    // this will only work for files that are in the root directory
    logMsg(LogClrAutomation, LogInfo, "retailBuild: Copying the optimized dll files from " + finalOptDir + " to " + finalOptDir + "\\signed" + "\n");
    robocopyFiles(finalOptDir, finalOptDir + "\\signed", "", logDir + "\\roboCopyToSigned.log.txt", "*.dll");

    logMsg(LogClrAutomation, LogInfo, "retailBuild: Copying the optimized dll files from " + finalOptDir + "\\WPF to " + finalOptDir + "\\WPF\\signed" + "\n");
    robocopyFiles(finalOptDir + "\\WPF", finalOptDir + "\\WPF\\signed", "", logDir + "\\roboCopyWPFToSigned.log.txt", "*.dll");

    if (!isNewRetailLab() && isCoreCLRBuild(bldType) == false && targetArch.toLowerCase() == Env("PROCESSOR_ARCHITECTURE").toLowerCase()) {
        var verString = targetArch + bldType;
        var installDir = clrSetupWithCache(finalOptDir, "/fx", logDir, srcBase + "\\binaries\\clrSetupCache", verString);
        StopNgenService(verString, targetArch);
    
        logMsg(LogClrAutomation, LogInfo, "retailBuild: installed in ", installDir, "\n");

        logMsg(LogClrAutomation, LogInfo, "} retailBuild Success - returning ", installDir, "\n");
        return installDir;
    }
    else if (isNewRetailLab() || isCoreCLRBuild(bldType)) {
        logMsg(LogClrAutomation, LogInfo, "} retailBuild Success - returning ", finalOptDir, "\n");
        return finalOptDir;
    }
}

/* starting with an unoptimized shp runtime build in 'inDir' create an optimized
   runtime build and place it in 'optDir' (it destroyes what is there).
   'scenarioSpec' is a specification of the scenarios to be run for doing the
   profile run (see scenariosGet for details).  It can be left undefined
   to for the default set of scenarios.  'srcBase is the base of the vbl
   and is used to get tools needed for the optimization.
   Upon success completion of this function thereturn value will be set
   to the installDir that will contain the installed  optimized binaries
   (both managed and unmanaged) ready to run.  'finalOptDir'  will also contain
   a complete build with the optimized binaries.

  Parameters
    inDir        : the starting unoptimize retail build.
                   If Pogo is enabled the inDir must have been built
                   using /LTCG, (this is the default for amd64ret and ia64ret)
    finalOptDir  : the path to place the optimized binaries.
                   (defaults to inDir + ".opt")
    targetArch   : The target architecture (defaults to the current processor architecture)
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the finalOptDir)
    srcBase      : The base of the VBL (so we can find razzle)
    neutralInDir : the retail build containing the arch neutral managed assemblies (ignored if undefined)
                   This arg is used to simulate layout installs for 64-bit platforms
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)
    localBase    : The base of the local copy of files from the VBL
*/
function shpRetailBuild(inDir, finalOptDir, targetArch, scenarioSpec, logDir, srcBase, neutralInDir, remoteMachine, localBase) {
    return retailBuild(inDir,finalOptDir,targetArch,scenarioSpec,logDir,srcBase,neutralInDir,remoteMachine,localBase,"coreshp");
}

/******************************************************************************/
/* starting with an unoptimized retail build in 'inDir' create an instrumented
   pogo runtime build and place it in 'instrDir' (it destroyes what is there).

  Parameters
    inDir        : the starting unoptimize retail build.  Must have been built
                   using /LTCG, (the default for amd64 and ia64)
    instrDir     : the path to use for the pogo instrumented build
                   (defaults to inDir + ".pgI")
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
    rArgs        : Additional args to razzle (eg Offline) (default none)

  Returns the instrDir that we created to use for the pogo instrumented build
*/
function PogoInstrument(inDir, instrDir, logDir, srcBase, targetArch, bldType, rArgs) {

    logCall(LogClrAutomation, LogInfo, "PogoInstrument", arguments, " {");
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    if (bldType == undefined)
        bldType = "ret";
    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;
    if (instrDir == undefined)
        instrDir = inDir + ".pgI";
    if (logDir == undefined)
        logDir = instrDir;
    if (rArgs == undefined)
        rArgs = "";

    if (FSOFolderExists(instrDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoInstrument: deleting ", instrDir, "\n");
        FSODeleteFolder(instrDir, true);     // clear out any old stuff
    }
    FSOCreatePath(instrDir);

    logMsg(LogClrAutomation, LogInfo, "PogoInstrument: input build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "PogoInstrument: pogo instrumented build will be put in ", instrDir, "\n");

    // We must populate the instrDir to be an exact copy of inDir
    // instrDir will be a private runtime
    logMsg(LogClrAutomation, LogInfo, "PogoInstrument: populating " +  instrDir + "\n");
    logMsg(LogClrAutomation, LogInfo, "PogoInstrument: Log FOlder is " +  logDir + "\n");
    robocopy(inDir, instrDir, "/s", logDir + "\\PogoInstrCopy.log.txt");

    logMsg(LogClrAutomation, LogInfo, "PogoInstrument: Relinking CLR binaries with Pogo Instrumentation\n");

    // Use razzle build to relink just the CLR binaries with _POGO_INS=1
    var buildRunOpts = runSetEnv("_POGO_INS", 1,
                       runSetEnv("PGO_PATH_TRANSLATION", srcBase + "=ntroot",
                       clrRunTemplate));

    razzleBuild(bldType, targetArch, "ndp\\clr\\src", "-cl", srcBase, instrDir, instrDir, 2*HOUR, rArgs, buildRunOpts);

    // Run ndp verify to rebase the dlls properly
    var verifyRunOpts = runSetEnv("PGO_PATH_TRANSLATION", srcBase + "=ntroot",
                        clrRunTemplate);

    razzleBuild(bldType, targetArch, "ndp\\verify", "-cC", srcBase, instrDir, instrDir, 1*HOUR, rArgs, verifyRunOpts);

    if (bldType.toLowerCase() == "coreshp") {
        // We need to use signed mscorlib.dll since delay signing is not supported on shp build
        FSOCopyFile(inDir + "\\mscorlib.dll", instrDir + "\\mscorlib.dll", true);
    }

    // This is a bit of a hack, with the Pogo instrumented build on x86, clr.dll and System.dll collide
    // so we rebase System.dll up to 0x7c000000
    //
    var rebase = srcBase + "\\tools\\x86\\rebase.exe";
    if (targetArch.match(/^(x86)/i)) {
        runCmdToLog(rebase + " -b 0x7c000000 " + instrDir + "\\System.dll");
    }

    // Since we just rebased clr.dll we go ahead and run updateDacDlls
    // just in case anyone want to run the debugger on the .pgI build
    logMsg(LogClrAutomation, LogInfo, "PogoInstrument: updating mscordac* to match " + CLRDllName(bldType) + ".dll in instrumented dir\n");
    updateDacDlls(inDir, instrDir, logDir, srcBase, targetArch, bldType);

    logMsg(LogClrAutomation, LogInfo, "} PogoInstrument Success - returning ", instrDir, "\n");

    return instrDir;
}

/******************************************************************************/
/* starting with an pogo instrumented retail build in 'instrDir'
   install this build

  Parameters
    instrDir     : the path to use for the pogo instrumented build
                   (defaults to inDir + ".pgI")
    pgcOtherDir  : the path to put the profiling run data *.pgc files
                   (defaults to instrDir + "\\pgc_data\\other")
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    bldType      : The type of build, eg ret, coreret, coreshp
    isRemote     : true if we are performing a remote run

  We use the /nz argument to ClrSetup so that nothing is ngened during this install step.

  Returns the installDir where we installed the instrumented pogo binaries and CLR binaries
*/
function PogoInstallInstrument(instrDir, pgcOtherDir, logDir, srcBase, bldType, isRemote) {

    logCall(LogClrAutomation, LogInfo, "PogoInstallInstrument", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if( bldType == undefined) {
    bldType = "ret";
    }

    if (instrDir == undefined) {
        var inDir = srcBase + "\\binaries\\" + currentArch + bldType;
        instrDir = inDir + ".pgI";
    }

    if (pgcOtherDir == undefined)
        pgcOtherDir = instrDir + "\\pgc_data\\other";

    if (logDir == undefined)
        logDir = instrDir;


    logMsg(LogClrAutomation, LogInfo, "PogoInstallInstrument: Installing from ", instrDir, "\n");

    var verString;
    if (isRemote)
        verString = currentArch + bldType + ".rpgI";
    else
        verString = currentArch + bldType + ".pgI";


    var installDir;
    if (!isCoreCLRBuild(bldType)) {

        var installRunOpts = runSetEnv("VCPROFILE_PATH", instrDir,
                             runSetEnv("PGO_PATH_TRANSLATION", srcBase + "=ntroot",
                             clrRunTemplate));

        // Currently we use the '/nz' switch when installing the pogo instrumented runtime, so
        // it is not necessary to call StopNgenService() because the NGen service never starts

        installDir = clrSetupWithCache(instrDir, "/nz /fx /nosxs", logDir,
                                                 srcBase + "\\binaries\\clrSetupCache",
                                                 verString, installRunOpts);
    }
    else {
        installDir = instrDir;
    }
    ProcessProfileData("pogo", instrDir, pgcOtherDir, "ClrSetup");
    logMsg(LogClrAutomation, LogInfo, "} PogoInstallInstrument Success - returning ", installDir, "\n");
    return installDir;
}

/*******************************************************************************************/
// WARNING: The PogoProfile function is called from $VBL/tools/devdiv/runpogo.js
//          any changes to this functions signature will need to be reflected
//          in runpogo.js otherwise the retail builds for 64-bit will break in Lab21.
/*******************************************************************************************/
/* PogoProfile:  Using the pogo instrumemted build that is already installed
                 Run the Pogo profile scenarios and place the results in
                 the pgcScenarioDir

  Parameters
    inDir        : the starting unoptimize retail build.  Must have been built
                   using /LTCG, (the default for amd64 and ia64)
                   We copy the prejit dll from here to use during the ngen step
    instrDir     : the path to use for the pogo instrumented build
                   (defaults to inDir + ".pgI")
    optDir       : the path to use for the pogo optimized build
    installDir   : The path to the installed pogo instrumented binaries, it must already be installed
    pgcOtherDir  : the path to put the other (non-scenario) profiling run data *.pgc files
                   (defaults to instrDir + "\\pgc_data\\other")
    pgcScenarioDir : the path to put the scenario profiling run data *.pgc files
                   (defaults to instrDir + "\\pgc_data\\scenario")
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    rArgs        : Additional args to razzle (eg Offline) (default none)
    localBase    : The base of the local copy of files from the VBL
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp

*/
function PogoProfile(inDir, instrDir, optDir, installDir, pgcOtherDir, pgcScenarioDir, scenarioSpec, logDir, srcBase, rArgs, localBase, targetArch, bldType) {

    logCall(LogClrAutomation, LogInfo, "PogoProfile", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (localBase == undefined)
        localBase = srcBase;

    if (bldType == undefined)
    bldType = "ret";

    var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + currentArch + bldType;

    if (instrDir == undefined)
        instrDir = inDir + ".pgI";
    if (optDir == undefined)
        optDir = inDir + ".pgO";
    if (pgcOtherDir == undefined)
        pgcOtherDir = instrDir + "\\pgc_data\\other";
    if (pgcScenarioDir == undefined)
        pgcScenarioDir = instrDir + "\\pgc_data\\scenario";
    if (logDir == undefined)
        logDir = instrDir;

    if (installDir == undefined) {
        if (!isCoreCLRBuild(bldType)) {
        var frameworkDir = Env("WINDIR") + "\\Microsoft.NET\\Framework";
        if (!currentArch.match(/^(x86)/i))
            frameworkDir = frameworkDir + "64";
            var runtimeDir = frameworkDir + "\\v4.0." + currentArch + bldType;
        installDir = runtimeDir + ".pgI";
    }
        else {
            installDir = instrDir + ".pgI";
        }
    }

    // We could put the 'installDir' in out PATH variable so that the Pogo profiling
    // dlls could be found while runing ngen and the scenarios, but since there are lots of other
    // binaries and dlls in this directory that are not normally on your PATH we instead create
    // a subdirectory, PogoBinDir and populate it with just the two Pogo profiling dlls.
    // Note I believe that the only one of these that we actually need is while profiling is pgort110.dll

    var pogoBinDir = installDir + "\\pogoBinDir";

    if (FSOFolderExists(pogoBinDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoProfile: deleting ", pogoBinDir, "\n");
        FSODeleteFolder(pogoBinDir, true);
    }
    FSOCreatePath(pogoBinDir);

    logMsg(LogClrAutomation, LogInfo, "PogoProfile: populating pogoBinDir: ", pogoBinDir, "\n");

    var pogoBinDirFiles = [ "pgodb110.dll", "pgort110.dll", "pgort110.pdb" ]; 
    var pogoBinSrcDir;
    if (isCoreCLRBuild(bldType)) {
        pogoBinSrcDir = srcBase + "\\OptimizationData\\pgo\\" + targetArch + "\\bin";
    }
    else {
        pogoBinSrcDir = installDir;
    }
    for(var i = 0; i < pogoBinDirFiles.length; i++) {
        var curFile  = pogoBinDirFiles[i];
        var fromFile = pogoBinSrcDir + "\\" + curFile;
        var toFile   = pogoBinDir + "\\" + curFile;

        FSOCopyFile(fromFile, toFile, true);
    }

    // pgort110 needs msvcr110.dll, copy that over too.  Arguably this file should be in the 
    // installDir and we should not pluck it from the tools dir directly, but that 
    // requires changing the pogo build. 
    //
    // Note that the VBL labs don't have a tools directory on the machines that run pogo. 
    // thus we only the DLL if the tools directory is there.   The VBLs don't still can 
    // work because they pollute system32 with an msvcr110.dll during clrsetup using the
    // special clrsetup flag /devsystem32. We don't use this approach since we prefer 
    // not to pollute system32 since this could run on dev machines.   -vancem
    //
    var systemDir = Env("WINDIR") + "\\System32";

    var msvcr110Dll = srcBase + "\\OptimizationData\\pgo\\" + targetArch + "\\bin\\msvcr110.dll";
    if (FSOFileExists(msvcr110Dll))
        FSOCopyFile(msvcr110Dll, pogoBinDir, "FORCE");

    logMsg(LogClrAutomation, LogInfo, "PogoProfile: copying pgort110.dll into System32 directory for IBuySpy web server scenario\n");

    var pogoFile        = "pgort110.dll";
    var systemPogoFile  = systemDir + "\\" + pogoFile;

    if (!isCoreCLRBuild(bldType)) {
    // Copy the pgort110.dll into the system32 directory for IBuySpy scenario
    FSOCopyFile(installDir + "\\" + pogoFile, systemPogoFile, true);
    }
    var stdPogoRunOpts = runSetDump("runjs dumpProc %p " + logDir + "\\pogo.ngen.dmpRpt",
                         runSetIdleTimeout(60,
                         runSetEnv("VCPROFILE_PATH", installDir,
                         runSetEnv("PATH", runGetEnv("PATH") + ";" + pogoBinDir,
                         runSetEnv("PGO_PATH_TRANSLATION", srcBase + "=ntroot",
                         clrRunTemplate)))));

    var scenarios = scenariosGet(scenarioSpec, localBase, targetArch, bldType, installDir);

    if (!isCoreCLRBuild(bldType)) {
    // We don't really care about the profile data generated during the NGEN step,
    // but we do need to use the instrumented clr.dll since the NGEN images
    // hard bind to it.  But we can use the non-instrumented <prejit>.dll, and it
    // is faster than the instrumented one.  So we copy the non-instrumented
    // <prejit>.dll into the install dir for the NGEN step and put back the
    // instrumented one when running the scenarios.

    // Use the runtime found in inDir
    SwitchRuntimeDlls(inDir, instrDir, installDir, "Using the non-instrumented runtime while ngen-ing");

    // ngen the scenarios.
    logMsg(LogClrAutomation, LogInfo, "PogoProfile: ngening scenarios {\n");

        NgenScenarios(bldType, scenarios, "", installDir, stdPogoRunOpts, true, "pogo", installDir, pgcOtherDir);

    logMsg(LogClrAutomation, LogInfo, "} PogoProfile: ngen scenarios complete\n");

    // Use the runtime found in instrDir
    SwitchRuntimeDlls(instrDir, inDir, installDir, "Using the instrumented runtime to run the scenarios");

    // Install IIS for ASP.NET scenarios
    SetupIIS(targetArch, installDir, srcBase);

    // Create a special subdirectory called isolated which will contain the mscoree.dll to use when Pogo profiling
    var mscoreeDir = installDir + "\\isolated";
    FSOCreatePath(mscoreeDir);

    // We also need to copy mscoree.dll from the instr dir into the install\isolated dir,
    // since we want to profile it and ClrSetup normally does not place it there
    FSOCopyFile(instrDir + "\\mscoree.dll", mscoreeDir + "\\mscoree.dll", "FORCE");
    FSOCopyFile(instrDir + "\\symbols.pri\\retail\\dll\\mscoree.pdb", mscoreeDir + "\\mscoree.pdb", "FORCE");
    }

    // Clear out the pgcScenarioDir if it exists
    if (FSOFolderExists(pgcScenarioDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoProfile: deleting old ", pgcScenarioDir, "\n");
        FSODeleteFolder(pgcScenarioDir, true);     // clear out any old stuff
    }

    try {
        SetMismatchedRuntime();

        // run the scenarios
        logMsg(LogClrAutomation, LogInfo, "PogoProfile: running scenarios on pogo instrumented runtime {\n");

        RunScenarios(bldType, scenarios, "pogo", installDir, stdPogoRunOpts,
                     installDir, pgcScenarioDir, pgcOtherDir);

    } finally {
        if (!isCoreCLRBuild(bldType)) {
        // Delete the pgort110.dll in the system32 directory for IBuySpy scenario
        FSODeleteFile(systemPogoFile, true);
        ClearMismatchedRuntime();
    }
    }

    logMsg(LogClrAutomation, LogInfo, "}\n");

    // For debugging purposes, we keep a copy of the PGD files and the PGC files without performing merging
    logMsg(LogClrAutomation, LogInfo, "PogoProfile: copying the PGD files to " + optDir + "\\PgoDataNotMerged\n");
    robocopy(instrDir + "\\PogoData", optDir + "\\PgoDataNotMerged", "", logDir + "\\PgoDataNotMergedPgdCopy.log.txt");
    logMsg(LogClrAutomation, LogInfo, "PogoProfile: copying the PGC files to " + optDir + "\\PgoDataNotMerged\\scenario\n");
    robocopy(pgcScenarioDir, optDir + "\\PgoDataNotMerged\\scenario", "", logDir + "\\PgoDataNotMergedPgcCopy.log.txt");

    // We need to copy the PGD files to "optDir\PgoDataMerged"
    logMsg(LogClrAutomation, LogInfo, "PogoProfile: copying the PGD files to " + optDir + "\\PgoDataMerged\n");
    robocopy(instrDir + "\\PogoData", optDir + "\\PgoDataMerged", "", logDir + "\\PgoDataMergedPgdCopy.log.txt");

    // Next, we need to merge the PGC files from pgcScenarioDir into the PGD files
    logMsg(LogClrAutomation, LogInfo, "PogoProfile: merging PGC files into " + optDir + "\\PgoDataMerged\n");
    var pgomgrExePath = Env("OSBuildToolsRoot")+"\\Pgo\\amd64\\pgomgr.exe";

    var srcFolder = FSOGetFolder(optDir + "\\PgoDataMerged");
    var files = new Enumerator(srcFolder.files);
    for(; !files.atEnd(); files.moveNext())
    {
        var pgdName = files.item().Name;
        var pgcWildcard = pgdName.substring(0,pgdName.length-4) + "!*.pgc";
        var pgomgrCmdLine = pgomgrExePath + " /merge " + pgcScenarioDir + "\\" +
                  pgcWildcard + " " + optDir + "\\PgoDataMerged\\" + pgdName;

        var runOpts = runSetTimeout(15 * MINUTE,
                  runSetIdleTimeout(5 * MINUTE,
                  runSetDump("runjs dumpProc %p " + logDir + "\\pgomgr.dmpRpt",
                  runSetEnv("_NTBINDIR", srcBase,
                  runSetEnv("_NTTREE", inDir,
                  clrRunTemplate)))));

        // Call pgomgr
        runCmdToLog(pgomgrCmdLine, runOpts);
    }

    logMsg(LogClrAutomation, LogInfo, "} PogoProfile Success\n");

    return 0;
}

/******************************************************************************/
/* PogoOptimize: Create the pogo optimized runtime build and place it in 'optDir' (it destroyes what is there).

  Parameters
    inDir        : the starting unoptimize retail build.  Must have been built
                   using /LTCG, (the default for amd64 and ia64)
    instrDir     : the path to use for the pogo instrumented build
                   (defaults to inDir + ".pgI")
    optDir       : the path to use for the pogo optimized build
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
    rArgs        : Additional args to razzle (eg Offline) (default none)

  Returns the optDir when successful
*/
function PogoOptimize(inDir, instrDir, optDir, logDir, srcBase, targetArch, bldType, rArgs) {

    logCall(LogClrAutomation, LogInfo, "PogoOptimize", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (bldType == undefined)
    bldType = "ret";
    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;
    if (instrDir == undefined)
        instrDir = inDir + ".pgI";
    if (optDir == undefined)
        optDir = inDir + ".pgO";
    if (logDir == undefined)
        logDir = instrDir;
    if (rArgs == undefined)
        rArgs = "";

    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: input retail build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: pogo instrumented build ", instrDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: pogo optimized build will be put in ", optDir, "\n");

    // We must populate the optDir to be an exact copy of inDir
    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: populating " +  optDir + "\n");
    robocopy(inDir, optDir, "/s", logDir + "\\PogoOptCopy.log.txt");

    // We need to copy the PGD files from "optDir\PgoDataMerged" to "optDir\PogoData" because the makefile
    // that razzle build uses is hard coded to look for pogo profile data in "optDir\PogoData". Why do we
    // have both the "PgoDataMerged" folder and the "PogoData" folder? Unfortunately, razzle build modifies
    // the PGD files in "PogoData" during the build process, making the PGD files unusable for future builds.
    // We would like to persist the unmodified version of the PGD files, so that is why we have both folders.
    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: copying the PogoData to " + optDir + "\\PogoData\n");
    robocopy(optDir + "\\PgoDataMerged", optDir + "\\PogoData", "", logDir + "\\PogoDataOptCopy.log.txt");

    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: Relinking CLR binaries with Pogo Optimizations\n");

    // Use razzle build to relink just the CLR binaries with _POGO_OPT=1
    var buildRunOpts = runSetEnv("_POGO_OPT", 1,
                       runSetEnv("PGO_PATH_TRANSLATION", srcBase + "=ntroot",
                       clrRunTemplate));

    razzleBuild(bldType, targetArch, "ndp\\clr\\src", "-cl", srcBase, optDir, optDir, 2*HOUR, rArgs, buildRunOpts);

    // Run ndp verify to rebase the dlls properly
    var verifyRunOpts = runSetEnv("PGO_PATH_TRANSLATION", srcBase + "=ntroot",
                        clrRunTemplate);

    razzleBuild(bldType, targetArch, "ndp\\verify", "-cC", srcBase, optDir, optDir, 30*MINUTE, rArgs, verifyRunOpts);

    if (bldType.toLowerCase() == "coreshp") {
        // We need to use signed mscorlib.dll since delay signing is not supported on shp build
        FSOCopyFile(inDir + "\\mscorlib.dll", instrDir + "\\mscorlib.dll", true);
    }

    // Since we just rebased clr.dll we go ahead and run updateDacDlls
    logMsg(LogClrAutomation, LogInfo, "PogoOptimize: updating mscordac* to match " + CLRDllName(bldType) + ".dll \n");
    updateDacDlls(inDir, optDir, logDir, srcBase, targetArch, bldType);

    logMsg(LogClrAutomation, LogInfo, "} PogoOptimize Success - returning ", optDir, "\n");

    return optDir;
}

/******************************************************************************/
/* Combines two Pogo steps, PogoInstallInstrument and PogoProfile and
   handles the case of remote machine execution.

  Parameters
    inDir        : the starting unoptimize retail build.  Must have been built
                   using /LTCG, (the default for amd64 and ia64)
    instrDir     : the path to use for the pogo instrumented build
    optDir       : the path to use for the pogo optimized build
    pgcOtherDir  : the path to put the other (non-scenario) profiling run data *.pgc files
    pgcScenarioDir : the path to put the scenario profiling run data *.pgc files
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    localBase    : The base of the local copy of files from the VBL
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)

*/

function PogoInstallAndProfile(inDir, instrDir, optDir, pgcOtherDir, pgcScenarioDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine) {

    logCall(LogClrAutomation, LogInfo, "PogoInstallAndProfile", arguments, " {");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }
    if (localBase == undefined)
        localBase = srcBase;

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    var isRemote = (remoteMachine == "true");
    var run;
    var runExitCode = undefined;

    if ((remoteMachine != undefined) && !isRemote)
    {
        // We forward this call to the remote machine via a recursive "runCmd runjs" call
        // but on the rescursive call we have remoteMachine set to true.

        var logFileInfoPrefix = logDir +"\\PogoInstallAndProfile." + remoteMachine;
        var remoteRunOpts  = runSetOutput(logFileInfoPrefix + ".log",
                             runSetDump("runjs dumpProc %p " + logFileInfoPrefix + ".dmpRpt",
                             runSetTimeout(3 * HOUR,
                             runSetIdleTimeout(60,
                             runSetMachine(remoteMachine)))));

        var remoteLocalBase = localBase;
        if (srcBase == localBase)
            remoteLocalBase = gsDefaultRemoteLocalBase;

        var remoteCmdLine = FormatCmdLine(uncPath(srcBase +"\\ndp\\clr\\bin\\runjs.bat"),
                                          "PogoInstallAndProfile",
                                          uncPath(inDir),
                                          uncPath(instrDir),
                                          uncPath(pgcOtherDir),
                                          uncPath(pgcScenarioDir),
                                          scenarioSpec,
                                          uncPath(logDir),
                                          uncPath(srcBase),
                                          remoteLocalBase,
                                          targetArch,
                                          bldType,
                                          true);

        run = runCmd(remoteCmdLine, remoteRunOpts);
        runExitCode = run.exitCode;
    }
    else
    {
        if (isRemote) {
            logMsg(LogClrAutomation, LogInfo, "PogoInstallAndProfile is running on remote\n");
            InitLocalBase(localBase, srcBase, logDir);
        }

        if (NeedToEnterWow64(targetArch)) {
            run = runX86("PogoInstallAndProfile",
                         inDir, instrDir, pgcOtherDir, pgcScenarioDir,
                         scenarioSpec, logDir, srcBase, localBase, targetArch, bldType);
            runExitCode = run.exitCode;
        }
        else
        {
            var currentArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

            if (!isCoreCLRBuild(bldType)) {
            // We don't ever want to install the instrumented mscoree.dll into the system32 directory
            // so we temporarily save the instrumented one as mscoree.dll.pgI and replace it with the one from inDir
            logMsg(LogClrAutomation, LogInfo, "PogoPhase: renaming instrumented mscoree.dll to mscoree.dll.pgI\n");

            FSOCopyFile(instrDir + "\\mscoree.dll", instrDir + "\\mscoree.dll.pgI", "FORCE");
            FSOCopyFile(inDir    + "\\mscoree.dll", instrDir + "\\mscoree.dll",     "FORCE");
            }

            var installDir = PogoInstallInstrument(instrDir, pgcOtherDir, logDir, srcBase, bldType, isRemote);

            if (!isCoreCLRBuild(bldType)) {
            // Put the instrumented version of mscoree.dll back the way is was in instrDir
            FSOMoveFile(instrDir + "\\mscoree.dll.pgI", instrDir + "\\mscoree.dll", "FORCE");
            }

            var rArgs = "";
            runExitCode = PogoProfile(inDir, instrDir, optDir, installDir,
                                      pgcOtherDir, pgcScenarioDir, scenarioSpec,
                                      logDir, srcBase, rArgs, localBase, targetArch, bldType);

            // Remove the Pogo instrumented runtime from the system
            UninstallRuntime(bldType, installDir, false, logDir, srcBase, targetArch);
        }
    }

    logMsg(LogClrAutomation, LogInfo, "} PogoInstallAndProfile ", (runExitCode==0) ? "Success" : "Failed", "\n");
    return runExitCode;
}

/******************************************************************************/
/* starting with an unoptimized retail build in 'inDir'
   first produce a pogo instrumented build in instrDir
   then produce a pogo optimized build in optDir

  Parameters
    inDir        : the starting unoptimize retail build.  Must have been built
                   using /LTCG, (the default for amd64 and ia64)
    instrDir     : the path to the pogo instrumented build
                   if this is undefined we must create the pogo instrumented build
    optDir       : the path to use for the pogo optimized build
    scenarioSpec : Scenarios to run, defaults to ndpScenarios
    logDir       : Where to put the log files (defaults to the instrDir)
    srcBase      : The base of the VBL (so we can find razzle)
    localBase    : The base of the local copy of files from the VBL
    targetArch   : The target archecture (defaults to the current processor architecture)
    bldType      : The type of build, eg ret, coreret, coreshp
    remoteMachine: The remote machine to use to perform the profile gathering (defaults to the current machine)

  Returns the optDir when successful
*/
function PogoPhase(inDir, instrDir, optDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine) {

    logCall(LogClrAutomation, LogInfo, "PogoPhase", arguments, " {");

    if (optDir == undefined)
        optDir = inDir + ".pgO";

    if (FSOFolderExists(optDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoPhase: deleting ", optDir, "\n");
        FSODeleteFolder(optDir, true);     // clear out any old stuff
    }
    if (!FSOFolderExists(logDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoPhase: Creating Log Folder ", logDir, "\n");
        FSOCreateFolder(logDir, true);     // clear out any old stuff
    }

    instrDir = PogoPhase_TH(inDir, instrDir, optDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine);
    optDir = PogoPhase_BH(inDir, instrDir, optDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine);

    logMsg(LogClrAutomation, LogInfo, "} PogoPhase Success - returning ", optDir, "\n");

    return optDir;
}

function PogoPhase_TH(inDir, instrDir, optDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine) {

    logCall(LogClrAutomation, LogInfo, "PogoPhase_TH", arguments, " {");

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (inDir == undefined) {
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;
    }

    if (optDir == undefined)
        throw new Error(1, "Required argument 'optDir' is missing");

    logMsg(LogClrAutomation, LogInfo, "PogoPhase_TH: input build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "PogoPhase_TH: pogo optimized will be put in ", optDir, "\n");

    var removeInstrDir = false;

    if (instrDir == undefined) {
        instrDir = PogoInstrument(inDir, undefined, logDir, srcBase, targetArch, bldType);
        if (!isCoreCLRBuild(bldType)) {
            // For CoreCLR, we don't install runtime.  Instead, we run against the instrumented version directly.
            removeInstrDir = true;
        }
    }

    if (logDir == undefined)
        logDir = instrDir;

    var pgcOtherDir    = instrDir + "\\pgc_data\\other";
    var pgcScenarioDir = instrDir + "\\pgc_data\\scenario";

    // Clear out the pgcOtherDir if it exists
    if (FSOFolderExists(pgcOtherDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoPhase_TH: deleting old ", pgcOtherDir, "\n");
        FSODeleteFolder(pgcOtherDir, true);     // clear out any old stuff
    }

    // Clear out the pgcScenarioDir if it exists
    if (FSOFolderExists(pgcScenarioDir)) {
        logMsg(LogClrAutomation, LogInfo, "PogoPhase_TH: deleting old ", pgcScenarioDir, "\n");
        FSODeleteFolder(pgcScenarioDir, true);     // clear out any old stuff
    }

    PogoInstallAndProfile(inDir, instrDir, optDir, pgcOtherDir, pgcScenarioDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine);

    logMsg(LogClrAutomation, LogInfo, "} PogoPhase_TH Success - returning ", instrDir, "\n");

    return instrDir;
}

function PogoPhase_BH(inDir, instrDir, optDir, scenarioSpec, logDir, srcBase, localBase, targetArch, bldType, remoteMachine) {

    logCall(LogClrAutomation, LogInfo, "PogoPhase_BH", arguments, " {");

    if (targetArch == undefined)
        targetArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (inDir == undefined)
        inDir = srcBase + "\\binaries\\" + targetArch + bldType;

    if (optDir == undefined)
        throw new Error(1, "Required argument 'optDir' is missing");

    logMsg(LogClrAutomation, LogInfo, "PogoPhase_BH: input build ", inDir, "\n");
    logMsg(LogClrAutomation, LogInfo, "PogoPhase_BH: pogo optimized will be put in ", optDir, "\n");

    if (logDir == undefined)
        logDir = instrDir;

    optDir = PogoOptimize(inDir, instrDir, optDir, logDir, srcBase, targetArch, bldType);

    logMsg(LogClrAutomation, LogInfo, "} PogoPhase_BH Success - returning ", optDir, "\n");

    return optDir;
}

function isCoreCLRBuild(bldType)
{
    if (bldType == undefined)
        throw new Error(1, "bldType is not set");
    if (bldType.substr(0,4).toLowerCase() == "core") {
        return true;
    }
    else
    {
        return false;
    }
}
/****************************************************************************/
// determine the name of CLR binary.
// When we integrate to main tree, this may be better implemented by having a different build target from x86chk
function CLRDllName(bldType) {
    if (bldType == undefined)
        throw new Error(1, "bldType is not set");
    if (bldType.substr(0,4).toLowerCase() == "core") {
        return "coreclr";
    }
    else
    {
        return "clr";
    }
}

/****************************************************************************/
// determine the name of DAC dll.
// When we integrate to main tree, this may be better implemented by having a different build target like x86chk
function DACDllName(bldType) {
    if (bldType == undefined)
        throw new Error(1, "bldType is not set");
    if (bldType.substr(0,4).toLowerCase() == "core") {
        return "mscordaccore";
    }
    else
    {
        return "mscordacwks";
    }
}

/****************************************************************************/
// Configure IIS for ASP.NET scenarios
function SetupIIS(arch, installDir, srcBase)
{
    var iarch = arch.toLowerCase();
    if (iarch == "x86")
        iarch = "i386";

    var runOpts = runSetTimeout(3*3600,
                  runSetIdleTimeout(600,
                  runSetEnv("COMPLUS_DefaultVersion", "",
                  runSetEnv("COMPLUS_Version", "",
                  clrRunTemplate))));

    // Some scenarios such as IBuySpy will fail if IIS is not running under the correct
    // bitness. The "set64bitmode" script does not appear to be sufficient. The code below
    // will ensure that IIS is actually running with the correct bitness.
    //
    // For more information see: http://support.microsoft.com/?id=894435

    runCmdToLog("cscript " + Env("SystemDrive") + "\\inetpub\\adminscripts\\adsutil.vbs " +
        "SET W3SVC/AppPools/Enable32bitAppOnWin64 " + (iarch == "i386" ? "1" : "0"), runOpts);

    try {
        // This will ensure that the loader mode and IIS mode match
        // I have seen cases when they differ for whatever reason - sanket
        runCmdToLog(srcBase + "\\ndp\\clr\\bin\\set64bitmode.bat " + (iarch == "i386" ? "setwow" : "set64"), runOpts);
    } catch(e) {
        logMsg(LogClrAutomation, LogInfo, "set64bitmode failed: " + e.descriptin + "\r\n");
    }

    // We need to call "aspnet_regiis -i" because clrsetup no longer does this for us
    runCmdToLog(installDir + "\\aspnet_regiis.exe -i", runOpts);

    // If ASP.NET v4.0 is marked as 'prohibitied', then our ASP.NET scenarios will fail with
    // error 404. We need to call "AllowAspNetExtensions.vbs" to ensure that ASP.NET v4.0 is
    // marked as 'allowed'. For more info, see IIS documentation on the "WebSvcExtRestrictionList"
    // metabase property.

    runCmdToLog("cscript //nologo " + srcBase + "\\OptimizationData\\scripts\\AllowAspNetExtensions.vbs", runOpts);
}

/****************************************************************************/
// Stop the NGen service for the specified runtime
function StopNgenService(verString, arch)
{
    if (arch == undefined)
        arch = Env("PROCESSOR_ARCHITECTURE");
    arch = arch.toLowerCase();

    var runOpts = runSetTimeout(5 * MINUTE,
                  runSetIdleTimeout(5 * MINUTE,
                  runSetNoThrow(
                  clrRunTemplate)));

    var cmdLine = "net stop clr_optimization_v4.0." + verString;
    if (arch == "x86" || arch == "i386")
        cmdLine += "_32";
    else
        cmdLine += "_64";

    runCmdToLog(cmdLine, runOpts);
}
