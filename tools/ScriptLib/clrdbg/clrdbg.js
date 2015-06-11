//-----------------------------------------------------------------------------
// RunJS scripts for ClrDbg team.
//-----------------------------------------------------------------------------


/******************************************************************************/
/********************************* DDSuites Shortcuts *************************/
/******************************************************************************/

// Smarty category string for all Mdbg tests
var _MDbgSmartyCategories = " /inc DEVUNIT\\DEVSVCS\\MDBG /inc+ COMBINED\\DEVSVCS\\TOOLS\\MDBG /inc+ COMBINED\\DEVSVCS\\DEBUGGER\\MDBG /inc+ COMBINED\\DEVSVCS\\DEBUGGER\\INTEROP\\MDBG ";

// Smarty string for Mdbg fxcop. Since fxcop is deterministic, set RetryFailure=0.
var _MDbgFXCopSmartyCategories = " /retryFailure:0 /inc DevUnit\\DEVSVCS\\Mdbg\\fxcop ";

// Smarty string for debugger (not profiler) tests
var _DebuggerSmartyCategories = "/inc COMBINED\\DEVSVCS /inc+ devunit\\devsvcs /exc COMBINED\\devsvcs\\profiler";


// Runs smarty.
// Ldr64 must be set properly in order for this to work.
function _RunSmartyTestsAndLaunchIE(smartyString)
{
    return devBVT(smartyString + " /clean /ie");
}

/*****************************************************************************/
/* Run Mdbg Fx Cop tests, and launch IE with results.
*/
function RunMDbgFxCop()
{
    return _RunSmartyTestsAndLaunchIE(_MDbgFXCopSmartyCategories);
}

/*****************************************************************************/
/* Run all Mdbg tests in ddsuites (including FxCop), and launch IE with results.
*/
function RunMDbgTests()
{
    return _RunSmartyTestsAndLaunchIE(_MDbgSmartyCategories);
}

/*****************************************************************************/
/* Run ddsuites Debugger tests (not included profiler). This will also run Mdbg tests.
*/
function RunDebuggerTests()
{
    return _RunSmartyTestsAndLaunchIE(_DebuggerSmartyCategories);
}

function _BuildMDbgTask(bldType, bldArch, binDir)
{
    if (binDir == undefined)
    {
        var relOutDir = bldArch + bldType;
        var logDir = "%outDir%\\" + relOutDir;        
        binDir = logDir + "\\bin";
    }
    
    var taskBuild = _razzleBuildTask(bldType, bldArch, "ndp\\clr\\src\\toolbox\\mdbg", undefined, undefined, binDir);
    taskBuild.description = "Clean Build of MDbg for " + relOutDir;
    
    return taskBuild;
}

/*
 Run MSbuild. 
 Uses the Whidbey RTM version so that we can find it.
 
 projectFileDir - directory that projectFile is in. 
    This will be the current directory for MSBuild's execution.
 projectFile- full path to project file
 args - arguments to msbuild.
  
*/
function _msbuild(projectFileDir, projectFile, args)
{
    // Sample ms build execution may be:
    // C:\Windows\Microsoft.NET\Framework\v2.0.50727\msbuild MDbg.Sln /t:Clean /t:MDbgCore_devtree
    
    
    // Use whidbey msbuild since we know where to find it.
    var msbuild = Env("WINDIR") + "\\Microsoft.NET\\Framework\\v3.5\\msbuild.exe";
    if (!FSOFileExists(msbuild))
    {
        throw new Error("Can't find MSBuild. Looking at '" + msbuild + "'.");
    }
    
    var fullPath = projectFileDir + "\\" + projectFile;
    if (!FSOFileExists(fullPath))
    {
        throw new Error("Project '" + fullPath + "' not found.");
    }
    
    var runOpts = runSetCwd(projectFileDir, runSet32Bit());
    runCmdToLog(msbuild + " " + projectFile + " " + args, runOpts);
}

/*
    Execute MS build  on Mdbg
    This ensures that the VS Mdbg.sln solution continues to build.
*/
function msbuildMDbg()
{
    // This just builds the MdbgCore target, although that's by far the most important one.
    var dir = Env("_NTBINDIR") + "\\ndp\\clr\\src\\ToolBox\\Mdbg";
    return _msbuild(dir, "mdbg.sln", "/t:Clean /t:MDbgCore_devtree");
}

// This needs to get the binary dir to copy MDbg to. It needs to be the same dir that the tests
// will use to pull Mdbg from.
// This works fine for MDbg's DDR, but it's not a bullet proof function in general scenarios.
// - For example, the SDK dir can be installed anywhere.
// - don't want to be tainted by the environment. (eg _getSDKBinaryDir uses Env(URTTARGET) ).
// For our purposes, we just need to make sure it's the same SDK dir the tests will use. 
// So we'll call it Mdbg specific.
//
// A binary dir is like: 
//    C:\WINDOWS\Microsoft.NET\Framework64\v2.0.AMD64chk\sdk\bin
function _getSDKBinaryDirForMDbg(bldType, bldArch)
{
    if (bldArch == undefined)
        bldArch = "x86";
    if (bldType == undefined)
        bldType = "chk";
        
    var targetDir = Env("WINDIR") + "\\Microsoft.NET\\Framework";
    if (bldArch.match(/64/))
    {
            targetDir += "64";
    }
    
    var verStr= getRuntimeVersionDir(bldArch + bldType);
    targetDir += "\\" + verStr + "\\sdk\\bin";
    
    return targetDir;
}

// Task to build and install Mdbg.
function _InstallMdbgTask(bldType, bldArch, dependents)
{
    if (bldArch == undefined)
        bldArch = "x86";
    if (bldType == undefined)
        bldType = "chk";
        
    var relOutDir = bldArch + bldType;
    var logDir = "%outDir%\\" + relOutDir;        
    var binDir = logDir + "\\bin";
    
    if (dependents == undefined)
    {
        var taskBuild = _BuildMDbgTask(bldType, bldArch, binDir);
        dependents = [taskBuild];
    }

    // MDbg is xcopy deployable. Setup is just an xcopy from binDir to the target dir (in the SDK).
    // SDK dir depends on architecture
    var taskSetup = taskNew(
        "InstallMDbg_" + relOutDir, 
        "runjs installFile " + binDir + "\\mdbg* " + _getSDKBinaryDirForMDbg(bldType, bldArch),
        dependents,
        undefined,
        "Setup MDbg. MDbg is xcopy deployable, so this is just a fancy xcopy of Mdbg* from the binaries directory to the sdk directory (which is where Mdbg lives). "+
        "This assumes that there is already a valid runtime installation for " + relOutDir + ".");

    return taskSetup;
}

function _BuildAndRunMDbgTestsTask(bldType, bldArch)
{

    if (bldArch == undefined)
        bldArch = "x86";
    if (bldType == undefined)
        bldType = "chk";
        
    var relOutDir = bldArch + bldType;
    var logDir = "%outDir%\\" + relOutDir;        
    var binDir = logDir + "\\bin";

    var taskSetup = _InstallMdbgTask(bldType, bldArch); // this will get a build task too
    
    var taskTests = _devBVTTask(bldType, bldArch, undefined, _MDbgSmartyCategories, [taskSetup]);
    taskTests.description = "Run the ddsuites MDbg tests for " + relOutDir;
    return taskTests;    
}

// Task to build Mdbg tests in the test tree.
// This is depenend on Building MDbg in the product treee.
function _BuildMDbgTestsTask()
{
    var bldArch = "x86";
    var bldType = "chk";
    
    var taskBuildAndInstall;
    var taskBuildTests = taskNew(
        "MDbg.Tests.build",    // include "build" for task report output.    
        "clrdbgjs BuildMDbgTests",
        [_InstallMdbgTask(bldType, bldArch)],
        undefined,
        "Build MDbg tests in the CLR test tree. Devsvcs Tests use MDbg as a platform, so we must ensure that public MDbg changes didn't break the test build.");
        
    return taskBuildTests;
}



/*****************************************************************************/
/* Builds the mdbg tests in the test tree.
 Mdbg serves is a platform that our tests build on, not just an standalone debugger application.
 This is crucial when changing the public surface area of Mdbg to ensure we didn't break the tests.

This requires a CLR already installed (and Mdbg to be in the SDK).

You must have a test tree sync in order for this to work.
    Run this command to get a test tree.
        runjs testsInstallTests
    
*/
function BuildMDbgTests()
{
    var dir = "desktop\\devsvcs\\tests\\tools\\mdbg\\mdbgext";
    
    // Leave these undefined and just use the defaults.
    var buildArch = "x86";
    var buildType = "chk";
    
    var clrVersionDir = Env("WINDIR") + "\\Microsoft.NET\\Framework\\v4.0." + buildArch + buildType;
    
    var testenvOptions;
    
    // Always do a clean build, since we've likely changed MDbg but not any source files in the
    // test tree, and the build system is too dumb to realize that means we need a clean build.
    var buildOptions = "cC"; 
    
    // Do Clean as well? That takes a long time.
    return testsBuild(buildArch, buildType,  clrVersionDir, testenvOptions, dir, buildOptions);
}


//-----------------------------------------------------------------------------
// A scaled down DDR for Mdbg changes
// You can only use this if you're just testing changes 
// This is streamlined and assumes there is an existing CLR installed. Thus it avoids building the
// whole CLR and doing a clrsetup. When multipled across each platform, this can save a ton of time.

// possible additional tasks include:
// - Self-host tests? (copy down latest)
// - versioning: 
//     MDbg, binds to V2 CLR; debugs V3 app
//     MDbg, debugs V2 app
// - ensure the MDbg sample builds?
var _mdbgDDRFast = 
    taskGroup("MDbgDDR", [
        taskNew("Mdbg_msbuild", "clrdbgjs msbuildMDbg", [], undefined, "Build Mdbg.sln with MSbuild (instead of razzle). This ensures the Visual Studio Mdbg.sln continues to build."),
    
        // Build MDbg
        _BuildAndRunMDbgTestsTask(undefined, "x86"),
        _BuildAndRunMDbgTestsTask(undefined, "amd64"),
        
        // Build tests in TestTree that depend on Mdbg.
        _BuildMDbgTestsTask()
    ],
    "Run tasks for minimum Mdbg checkin bar. Send questions to <a href=\"mailto:clrdbg\">ClrDbg</a>" );

// Add this to the global task list.
_taskAdd(_mdbgDDRFast);  



//-----------------------------------------------------------------------------
// A scaled down DDR task for Right-side only changes
var _miniDDR = 
    taskGroup("dbiDDR", [
            // This task lets us see what sync point people were at when they built
        taskNew("sdWhereSynced", "runjs sdWhereSynced %srcBase%\\ndp\\clr\\src %srcBase%", undefined, undefined,
                "This task does a sdWhereSynced to log what sync point the run used to build."),
			
			// This task insures that people's dev directory is up to date
        taskNew("SyncDevDir", "runjs SyncDevDir %srcBase%", undefined, undefined,
                "This syncs the vbl\dev\... tree which holds documentation and tools."),

            // FIX, I have put amd64 build first because 64 bit builds update mscoree.h
            // which causes x86 builds to have to start over.  I am working on a real fix
            // however in the mean time do the 64 bit build first so that the x86 build is OK
        _razzleBuildTask("chk", "amd64",  "ndp"),

          
        _buildWarningTask("chk", "x86"),
        _scanRuntimeTask(),
        
// Disabled due to incompatablilty with MSBuild.  Will be replaced by OACR ASAP        _prefastBuildTask(),

	_devBVTTask("chk", "x86", undefined, "/inc Combined\\devsvcs /inc+ devunit\\devsvcs"),
  	_devBVTTask("chk", "amd64", undefined, "/inc Combined\\devsvcs"),

_scanMscordbiTask(),
_taskDacCop("chk","x86"),
_razzleBuildTask("chk", "ia64", "ndp\\clr")
    ]);


_taskAdd(_miniDDR);




// Traverse down task and remove task with the given name
// This is useful to have customized versions of DDR that strip out frivilous tasks 
// that have been added in that.
// 
// Sample usage:
//    removeTask(_taskDailyDevRun, "ddIntFx");
function removeTask(task, name)
{
    if (task == null) return;
    if (!task.dependents) return;


    var len = task.dependents.length;
    for (var i = 0; i < len; i++)
    {
        var t = task.dependents[i];
        if (t == undefined) continue;
        if (t.name == name)
        {
            task.dependents[i] = 
                taskNew("nulLTask", "exit 0", undefined, undefined, "Removed task " + name);
        }
        else
        {
            // Get recursive.       
            removeTask(t, name);
        }    
    }
}


// Add a task to DDR
// This updates both the main dailyDevRun task and also the dailyDevRunFull64 task.  
// Ideally we could just manipulate the _dailyDevRunCommonTasks list, but that's
// already been copied into these two tasks.
function addDDRTask(task)
{
    _taskDailyDevRun.dependents.push(task);
    _taskDailyDevRunFull64.dependents.push(task);
}

// Adjust daily dev run for debugger team.
// Note that we want to adjust both the normal and full64 version of DDR.
addDDRTask(_scanMscordbiTask());

// Add DacCop to DailyDevRun.  
// Ensures an x86chk build happens before DacCop.  This ensures any dependents are built,
// as well as causing any build errors to show up first in a build task.
// This build is already in DDR so it shouldn't add any extra time.
addDDRTask(_taskDacCop("chk","x86"));

// also build ia64chk 
addDDRTask(_razzleBuildTask("chk", "ia64", "ndp"));

// Run the x86 debugger tests if we're on amd64 and not already running full 64-bit tests
// Note that we don't use addDDRTask because we don't want to update dailyDevRunFull64 which
// already runs all x86 tests.
_taskDailyDevRun.dependents.push(IF_RUN(myArch != "x86", 
    _devBVTTask("chk", "x86", undefined, "/inc Combined\\devsvcs /exc devunit")
    )
    );


// Task to invoke SCAN on mscordbi
function _scanMscordbiTask() {

    var bldRelDir = "ndp\\clr";
    var relOutDir = "x86chk";
    var taskName = "scanMscordbi@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir;
    var binDir = logDir + "\\bin";

    var ret = taskNew(taskName,
        "runjs scanRuntimeWorker mscordbi.dll " + binDir + " "
                             + logDir + "\\scan.mscordbi.report.log "
                             + "%srcBase% ",
        [_razzleBuildTask("chk", "x86", "ndp")]); // task gets folded

    ret.description = "Run scanRuntime against the prebuilt mscordbi x86chk binaries.\r\n";
    return ret;
}


// Convenience function for running an incremental DacCop task.
// Note that this requires an x86chk clean build to have already been done.
function DacCop()
{
    doRun( "dacCop-inc@x86chk.dacCop" );
}

// Helper to return true iff a DDR status log succeed.
// This will load the status log produced from Runjs's task system and check for the 'success' entry. 
//   statusLogFilename - full filename to DDR status log.
function _statusLogSucceeded(statusLogFilename)
{
    var content = FSOReadFromFile(statusLogFilename);
    
    // A status log has this string if it succeed.
    var good = "result: SUCCESS command completes successfully";
    
    if (content.match(good)) 
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*****************************************************************************/
/*  Helper for copying Devsvcs selfhost.

   This finds the most recent devsvcs testbin dir from 
   \\dotnetdevelop\drops\Clr_Debugger and then calls copySelfHostTests.
   This copies flavor %PROCESSOR_ARCHITECTURE%ret, so run it in the right 
   ClrEnv window and it will do the right thing.
   
   If you need more control, you can call copySelfHostTests directly.
*/
function copyDbgSelfHost()
{
    // Devsvcs are on:
    // \\dotnetdevelop\drops\Clr_Debugger\run.current\clrtest\x86ret\testbin
    var srcTestBinDir = "";
    
    var testRoot = "\\\\dotnetdevelop\\drops\\Clr_Debugger\\";
    
    var bldArch = Env("PROCESSOR_ARCHITECTURE"); // x86, amd64, ia64
    var flavor = bldArch + "ret";
    
    var statusLogTemplate = "taskPublishTests-devsvcs@%flavor%.0.status.log";
    
    // Find latest in the root
    var folder = FSOGetFolder(testRoot);
    var szLatest = null;
    
   
    // Track the most recent 
    var mostRecent = null;
    
    // Each sub-directory represents a Runjs automation output dir. Enumerate through each directory searching
    // for the latests completed task. This iterates alphabetically. Files are named such that that means from oldest to newest.
    for (var fileEnum = new Enumerator(folder.SubFolders); !fileEnum.atEnd(); fileEnum.moveNext()) 
    {    
        // name will be just the directory name, like "run.07-03-15_2456767".
        var name = fileEnum.item().Name;
        
        // "run.current" is a junction to the latest, so just ignore it.
        if (name == "run.current") continue;
        
       
        // get a full path like:
        //    \\dotnetdevelop\drops\Clr_Debugger\run.07-03-14_2454327
        // This should be the root of the DDR reports
        var fulldir = testRoot + name;
        
        var statusLog = statusLogTemplate.replace("%flavor%", flavor);
       
        if (_statusLogSucceeded(fulldir + "\\" + statusLog))
        {
            // Remember
            mostRecent = name;            
        }
    }
    
    if (mostRecent == null)
    {
        throw new Error("No successful test builds on " + testRoot);
    }
    srcTestBinDir = testRoot + mostRecent + "\\clrtest\\" + flavor + "\\testbin";
    
    WScript.Echo("Copying selftests from private CLR dbg builds from \\DotNetDevelop");
    
    return copySelfHostTests(undefined, srcTestBinDir, undefined)
}



/*****************************************************************************/
/* Get the latest counter

    Returns a string lable of the latest changelist. Throws on error.
    
    Parameters:
    sdEnv  : run options (env vars) passed to run (by default set to 
            set current directory to 'dir' and adds \\clrmain\tools\x86 to 
            the path so that sd.exe will be found even on a clean machine
*/
function sdGetLatestChange(sdEnv)
{
    if (!(runCmd("sd counter change", sdEnv).output.match(/^\s*(\d+)/)))
    {
        throw new Error(-1, "Could not get latest sync point counter");
    }
    var label = RegExp.$1;
    return label;
}

/*****************************************************************************/
/* helper to resolve a branch
    branch - source-depot branch name. eg:  "private/CLR_Debugger" or "pu/clr"

    Throw an error if there are merge conflicts.
*/
function resolveBranch(branch)
{
    if (branch == undefined)
        branch = _getDefaultCurrentLab();
        
    // First resolve using -as, to get all the "safe" files out. 
    // Then do an -am to merge.
    // Then do another -am to detect merge conflicts.
    
    // 1. Do the "safe" files that can't possibly have a merge conflict.
    var cmd = "sd resolve -as //depot/devdiv/" + branch + "/...";
    runCmdToLog(cmd, runSetNoThrow());
    
    // 2. Now try merging. This will skip merge conflicts.
    cmd = "sd resolve -am //depot/devdiv/" + branch + "/...";
    runCmdToLog(cmd, runSetNoThrow());
    
    // 3. Now we want to see if there are any merge conflicts. So try merging again
    // and see if there's any work left.
    // If there are no conflicts, then the first "-am" would have done everything, and
    // this -am has nothing to do and will print 'no files' in the output.    
    var run = runCmdToLog(cmd, runSetNoThrow());
    var conflicts = run.output;
   
    var fMergeConflicts = !conflicts.match(/no file/i);
    
   
    if (fMergeConflicts)
    {
        // There were conflicts
        var marker = "\n--------------------------------\n";
        throw Error("There were conflicts:" + marker + conflicts + marker);
    }
    
    // No conflicts.
    return 0;
}


// Get the CLR's parent lab, in Source-depot server notation.
function _getClrParentLab()
{
    return "pu/clr";    
}

// Get the lab that the clrenv window is running in (based off _NTROOT)
function _getDefaultCurrentLab()
{
    // @todo - make this actually use the current lab instead of hard-coded
    // for debugger
    return "private/CLR_Debugger";
}

// Sync to the latest. Use server notation so that we can do this from any directory.    
function syncLatestAndResolveBranch(toLab)
{
    if (toLab == undefined)
        toLab = _getDefaultCurrentLab();

    // Revert public changes so that they don't conflict with the sync.
    runCmdToLog("revert_public.cmd");
        
    // Sync to the latest. Use server notation so that we can do this from any directory.    
    var cmd = "sd sync //depot/devdiv/" + toLab + "/...";
    runCmdToLog(cmd);
    
    resolveBranch(toLab);
}

/*****************************************************************************/
/* Integrate from one lab into another lab.
    fromLab - source (eg, "pu/clr" for an FI)
    toLab - dest (eg, "private/CLR_Debugger")
    labelSource - changelist in fromLab to intergate from
    changelistTarget - if specified, integration goes into this changelist. If
        unspecified, it's in the default changelist.
        
    Default parameters will do an FI from CLR branch into the current branch 
    specified by the ClrEnv window this is caleld from.
*/
function integrateClrBranchesAndResolve(toLab, fromLab, labelSource, changelistTarget)
{
    if (fromLab == undefined)
        fromLab = _getClrParentLab();
        
    if (toLab == undefined)
        toLab = _getDefaultCurrentLab();
    
    if (labelSource == undefined)
        labelSource = sdGetLatestChange();
            
    var sdRoot = Env("_NTROOT");                
    
    {
        var dir = undefined; // must do all directories to be correct.
        sdIntegrate(fromLab, toLab, labelSource, dir, sdRoot, undefined, changelistTarget);
    }
    
    // Now resolve.If there are any conflicts, send mail with conflicts and exit.
    resolveBranch(toLab);
}

/*****************************************************************************/
/* updateCLRTestLstFiles .lst files from  one lab into another lab.
    fromLab - source (eg, "pu/clr" for an FI)
    toLab - dest (eg, "private/CLR_Debugger")
    changelistTarget - if specified, integration goes into this changelist.
*/
function updateCLRTestLstFiles(toLab, fromLab, changelistTarget)
{
    if( fromLab == "pu/clr" &&
        toLab.match( /^private\// ) )
    {
        // we integrate prduct and test tree from public tree to the private branch.
        // When this tree includes test tree as well, we need to integrate the
        // .lst files from the test tree.
        testsFILstFiles(undefined, changelistTarget);
    }
    else
    {
        WScript.Echo("Skipping integration of .lst files");
    }
    return 0;
}

/****************************************************************************/
/* Validate that all files in the changelist is in the given lab
This can avoid accidentally submitting files in a different lab.

     changelist-  changelist to validate.
     branch - branch (can include path) that all files should be in. Use SD notation.
               eg "private/CLR_Debugger" or "pu/clr". You can also include a path
               like "pu/clr/ndp/clr/src/vm"

Throws exception if not valid.     
*/
function validateChangeListInLab(changelist, branch)
{
    var ret = sdDescribe(changelist);
    
    var files = ret.files;

    var prefix = "//depot/devdiv/" + branch
    for(var idx in files) 
    {
        var file = files[idx];
        var name = file.depotFile;
        if (name.indexOf(prefix) != 0)
        {
            throw Error("File '" +  name + "' in changelist " + changelist + " is not in branch:" + branch);
        }
    };
    // Success    
}


// changelist - changelist containing the integration
function submitAndSendMailForFICandidate(mailTo, changelist, changelistFISource)
{
    validateChangeListInLab(changelist, _getDefaultCurrentLab());

    // Throws on error. Else returns the real changelist that this is submitted as.
    //var changelistSubmitted;
    
    if (true)
    {
        changelistSubmitted = sdSubmit(changelist);
    }
    else
    {
        var changelistSubmitted = changelist; // uncomment if we want to shortcut the submit.
        WScript.Echo("*** SUBMIT IS SKIPPED ***");
    }
    
    // If the submit fails, then it will throw and no success mail will be sent.
    // The overall task will fail, so we'll get notification that way.

    //
    // Send status mail on checkin.
    //
        
    // Get description (files + text) of the changelist
    // Don't run this to a Log, for a large change, the logging could take over an hour!
    var ret = runCmd("sd describe -s " + changelistSubmitted);
    var description = ret.output;
    
    // If the mail message is too large, then we get back error: 0x800ccc6d    
    // So we pick an arbitrary limit here
    var limit = 10 * 1000;
    if (description.length > limit)
    {
        description = "File list is too large to send in email. Do 'sd describe -s " + changelistSubmitted + "' to see files.";
        
    }
    
   
    var parentRecentChanges = "<couldn't retrieve changes in parent branch at FI point>";
    if (changelistFISource != undefined)
    {
        // Get changelist from FI
        // How to show recent files in a given lab for a given changelist.
        ret = runCmdToLog("sd changes -m 30 //depot/devdiv/" + _getClrParentLab() + "/ndp/...@" + changelistFISource);        
        parentRecentChanges = ret.output;
    }
        
        
    var subject = "Checkin: FI from " +_getClrParentLab() + "@" + changelistFISource;
    
    var body = "<html><body>" + 
        "Description: FI successfully submitted<br>" +
        "Changelist: <b>" + changelistSubmitted + "</b><br>" +
        "<br>" + 
        "FI from parent:<i>" + _getClrParentLab() + "</i> branch at changelist <i>" + changelistFISource + "</i>, recent parent changes are:<br>" +
        "<pre>" + parentRecentChanges + "</pre>" +
        "<b>Files changed</b>:<br><pre>" + description + "</pre>" +
    "</body></html>";
    
    mailSendHtml(mailTo, subject, body);
    
    
    
    // Things to include in a mail:
    // - Changelist in pu/clr that we FI from
    // - N recent changes from pu/clr
    // - Files that we integrate
    // - tests that we ran.
    // - Changelist that we submit as
    return 0;
}

// Task-based FI
// Order:
//  Sync +  Resolve 
//  Integrate  (changelist)
//  resolve integrations
//  Submit
function taskDebuggerFI(toLab, fromLab, changelistWithIntegration, changelistSource)
{
    if (fromLab == undefined)
        fromLab = _getClrParentLab();
        
    if (toLab == undefined)
        toLab = _getDefaultCurrentLab();

    var taskUpdateLstFiles = undefined;
    var changelistTarget = undefined;

    if (changelistWithIntegration == undefined)
    {
        changelistSource = sdGetLatestChange();
    
        // Preparation
        changelistTarget = sdChange("Forward Integration from " + fromLab + " to " + toLab + " at " + changelistSource);
    
        // ensure default is empty
        //    sd opened -c default
        // If empty, it will print:
        // 'File(s) not opened in that changelist'   ('F' may be lowercase too)
        //
        // The problem is that you can't have files open because they may pick up
        // changes from the FI, but won't be in the FI changelist.     
   
        var taskRevertPublic = taskNew("RevertPublic",
            "revert_public.cmd");
        taskRevertPublic.Description = "Revert public changes so that they won't conflict with the integration.";
    
        var taskEnsureEmpty= taskNew("EnsureLabEmpty",
            "clrdbgjs ensureLabIsUnchanged " + toLab,
            [taskRevertPublic]);
        taskRevertPublic.description = "Ensure the lab is empty before we integrate. This is because if we integrate a file into an already edited file, then that file will not be part of the integration changelist.";


        // get label of sync from FI
        var taskSync=  taskNew("SyncAndResolve",
            "clrdbgjs syncLatestAndResolveBranch " + toLab,
            [taskEnsureEmpty]);
         
        
        taskSync.description = "Revert public changes, sync the branch to the latest build and resolve any open files.";
        

        var taskIntegrate = taskIntegrate = taskNew("Integrate",
            "clrdbgjs integrateClrBranchesAndResolve "+ toLab + " " + fromLab + " " + changelistSource + " " + changelistTarget,
            [taskSync]);
        taskIntegrate.description = "Do an FI from " + fromLab + "@" + changelistSource + " to " + toLab + " into changelist " + changelistTarget + ".";

        taskUpdateLstFiles = taskNew("updateCLRTestLstFiles",
                                     "clrdbgjs updateCLRTestLstFiles "+ toLab + " " + fromLab + " " + changelistTarget,
                                     [taskIntegrate]);
        taskUpdateLstFiles.description = "Do an FI of .lst files from " + fromLab + " to " + toLab + " into changelist " + changelistTarget + ".";

    }
    else
    {
        if (changelistSource == undefined)
        {
            throw Error("If you supply a changelist for " + toLab + ", you must supply a changelist source from " + fromLab); 
        }
        changelistTarget = changelistWithIntegration;
    }
    

    //var taskFIBuilds = taskIntegrate; // Use to skip test builds, for testing the script.
    
        
    var taskFIBuilds = taskGroup("FISanityBuilds", [
        _razzleBuildTask("chk", "amd64", "ndp\\clr"),
        //_razzleBuildTask("chk", "x86", "ndp\\clr"), // included in devBvt task
        _razzleBuildTask("ret", "x86", "ndp\\clr"), 
        _devBVTTask("chk", "x86", undefined, "/inc Combined\\devsvcs"),
        _devBVTTask("chk", "x86", undefined, "/inc devunit\\devsvcs"),
        _taskDacCop("chk","x86")        // shares x86chk build task done by devBVTTask
        ]);
    taskFIBuilds.description = " Do some sanity builds on the FI.";

    // Chain dependencies so that taskIntegrate executes before taskFIBuilds.
    dep = taskUpdateLstFiles;

    // @todo -  Must recursively chain...
    for(idx in taskFIBuilds.dependents)
    {   
	if (dep != undefined)
        {
            if (taskFIBuilds.dependents[idx].dependents)
            {
                taskFIBuilds.dependents[idx].dependents.push(dep);   
            }
            else 
            {
                taskFIBuilds.dependents[idx].dependents = [ dep ];
            }
        }
    }

        
    var mailToSuccess = "clrdbg@microsoft.com";
    var mailToFailure = "seanse@microsoft.com";
        
    var cmd = "clrdbgjs submitAndSendMailForFICandidate " + mailToSuccess + " " + changelistTarget + " " + changelistSource;
    var taskSubmit = taskNew("Submit",
        cmd,
        [taskFIBuilds]);
    taskSubmit.description =  "Submit the FI and send a detailed status mail if successful.\n" + "submit via: "  +cmd;

    
    var name = "FIfromPuClr2ClrDbg"; // name        
    var t1 = taskGroup(
        name,
        [taskSubmit]);
        

    t1.description = "Do an FI into a private branch and some sanity testing.\r\n";


       
    // this will get the proper default parameters to show up in our automation directory
    _taskAdd(t1);
    var ret = doRun(name, srcBaseFromScript() + "\\automation2"); 
    
   
    // On failure, send a mail, noting changelist for reverting.
    if (ret != 0)
    {
        _sendFailureMailure(mailToFailure, toLab, changelistTarget);
    }    
}

function _sendFailureMailure(mailTo, toLab, changelistTarget)
{    
    var subject = "FI to '" + toLab + "' failed";
    var body = "<html><body>" + 
        "The FI <b><font color=\"red\">failed</font></b>." +
        "The problematic changes are in the local pending changelist <i>" + changelistTarget  + "</i>." +
    "</body></html>";
    
    mailSendHtml(mailTo, subject, body);
}


// throw if any file is open int he specified lab
function ensureLabIsUnchanged(toLab)
{
    if (toLab == undefined)
        toLab = _getDefaultCurrentLab();
        
    
    // @todo - merge with sdOpened() command
    //var sdObj = sdConnect(sdObj);
    //var results = _sdRun(sdObj, "opened //depot/devdiv/" + toLab + "/...", true, true);
    
    var cmd = "sd opened //depot/devdiv/" + toLab + "/...";
    var run = runCmd(cmd);
    
    // If no files are opened, it will print a text message like below.
    var output = run.output;
    
    
    var empty= output.match(/file\(s\) not opened/i);

    if (!empty)
    {
        throw Error("Files are open on this client in " + toLab + ":\n" + output);
    }
}



/****************************************************************************/ 
/* Post to MDbg private branch.
This is a wrapper to guarantee that we're not accidentally submitting to the 
main branch.
*/
function MDbgPost(iChangeList)
{
	WScript.Echo("-----------------------------------------");
	WScript.Echo("Doing submit to Mdbg private branch for changelist:" + iChangeList);
	if (iChangeList == undefined) {
		throw new Error(1, "Missing changelist specification.");
	}

	// Run "sd describe -s %1" command and verify that all file are in mdbg_private branch.
	//var run = runCmd("sd describe -s " + iChangeList);

	var sdObj = sdConnect(Env("SDPORT"));

	var d = sdDescribe(iChangeList, sdObj);

	// Now go down each and verify they're in the MDbg private branch:
	var branch = "depot/devdiv/private/Lab21S_mdbg/ndp/clr/src/toolbox/mdbg";
	WScript.Echo("Confirming all files are in the MDBg private branch:" + branch);
	var files = d.files;
	for(idx in files) { 
		var file = files[idx].depotFile;
		if (file.indexOf(branch) <= 1) {
			throw new Error("**** File '" + file + "' is not in the MDbg private branch!!! ****");
			
		}
		WScript.Echo(file);
	}
	WScript.Echo("*** Confirmed all files are in the MDbg private branch ***");

	//WScript.Echo(dump(d));
	var clSubmit = sdSubmit(iChangeList, sdObj); 
	WScript.Echo("Submitted as changelist:" + clSubmit);
}



/****************************************************************************/ 
/* Get RS log from 
parameters:
    argsAttach - a windbg command to attach to the target. (likely "-pv -p %pid%" or "-z %file")
    szLogFile - output name for RS stresslog
    szExtraDataObj - object that we can add extra fields to containing the results of inspection.   
*/
function _getRSLogFromArgs(argsAttach, szLogFile, szExtraDataObj)
{

    if (FSOFileExists(szLogFile))
    {
        throw new Error(1, "File '" + szLogFile + "' already exists");
    }
        

    // Invoke windbg, dump logs

    //eg: "\\jmstall3\debuggers\cdb -pv -p %PID% -c \".dump /mfh %MY_FILENAME%;.detach;q\"";
    //var cdbPath = "\\\\jmstall3\\debuggers\\cdb";
    var cdbPath = "\\\\clrsnapbackup2\\src\\ddsuites\\src\\clr\\x86\\tools\\i386\\debuggers\\cdb";

    var attachCookie = "CMD_ATTACH_SUCCEEDED";
    // Intermediate commands to execute in cdb.
    var cmds = 
        ".echo " + attachCookie + ";" +  
        ".echo CMD_LOAD_MSCORDBI_START;" +
        "!sym noisy; .reload mscordbi.dll; ld mscordbi; !sym quiet;" + 
        ".echo CMD_LOAD_MSCORDBI_END;" +
        ".load \\\\jmstall5\\help\\strikers.dll;" +
        "!DumpRSStressLog " + szLogFile +";" +
        ".echo CMD_DUMP_VARS_START;" + 
        "dt mscordbi!g_pRSDebuggingInfo  m_MRUprocess .;" +
        ".echo CMD_DUMP_VARS_END;";

    var cmdFinal = cdbPath + " " + argsAttach + " -c \"" + cmds + ";.detach;q\"";


    var output = FSOGetTempPath();        

    WScript.Echo("Executing cdb");
    runCmd(cmdFinal, runSetOutput(output));
    WScript.Echo("Done w/ cdb");

    var outData = FSOReadFromFile(output);
    if (!FSOFileExists(szLogFile))
    {
        // If we failed, then look at CDB's output and try to provide the clear reason why.

        var reason = "unknown";

        // So we failed to get the log.
        // Was it b/c we didnt' even attach?
        if (outData.indexOf(attachCookie) == -1)
        {
            reason = "failed to attach (" + argsAttach + ")";
        }
        else if (outData.indexOf("Failed to find mscordbi!StressLog::theLog") >= 0)
        {
            reason = "couldn't find symbols for mscordbi.dll:";
         
            if (outData.match(/^CMD_LOAD_MSCORDBI_START((.|\n)*)CMD_LOAD_MSCORDBI_END$/m))
            {                
                var symInfo = RegExp.$1;
                reason += "\n" + symInfo;                
            }
        }   
    
        throw new Error(1, "Failed to get log file '" + szLogFile + "'.\nSee '" + output +"' for details.\nReason for failure:" + reason);
    }

    // Get the extra var info
    if (outData.match(/^CMD_DUMP_VARS_START((.|\n)*)CMD_DUMP_VARS_END$/m))
    {
        var varInfo = RegExp.$1;
        szExtraDataObj["proc"] = varInfo;
        //WScript.Echo("@@@@");
        //WScript.Echo(dump(szExtraDataObj));
        //WScript.Echo("Vars:" + varInfo);
    }

    return true;
}




/****************************************************************************/ 
/* Formats an ansi RS log into RTF. (Use RTF instead of HTML so we can edit it.) 
parameters:
    szPath - path to ansi stress log to be formatted.
    szRtfOutpath - path to new rtf file to be created 
       for pretty formatting the ansi file.
    info - optional object with extra information to inject into the rtfoutput..
*/
function formatRSLog(szPath, szRtfOutPath, info)
{
	if (szPath == undefined)
		throw new Error(1, "Required parameter 'szPath' missing");

    if (szRtfOutPath == undefined)
		throw new Error(1, "Required parameter 'szRtfOutPath' missing");
    

    var input= FSOReadFromFile(szPath);

    var rtfPath = szRtfOutPath;
    var rtfFile = FSOOpenTextFile(rtfPath, 2, true);     

    rtfWriteHeader(rtfFile);
    rtfFile.Write(rtfBold(rtf("Right-Side stress log report.")) + rtfEOL());
    //    rtfFile.WriteLine(rtf("This is a test:") + rtfBold(rtfColor(rtf("RED"), RTF_RED)));

    // find RCET thread id
    // Each row is: id, timestamp, data
    // 14e0 000b3615dfe39bb2: RCET::TP: refreshing process list.
    var rcetId = "";
    if (input.match(/^ *?(\S+) (\S+): (.*RCET::TP: refreshing process list).*$/m))
    {
        rcetId = RegExp.$1;
        //WScript.Echo("Helper:" + helperId + "," + RegExp.$2  + "," + RegExp.$3);
    }

    // Find W32ET id.
    var w32etId = "";
    if (input.match(/^ *?(\S+) (\S+): (.*Win32 Debug Event received).*$/m))
    {
        w32etId = RegExp.$1;
        //WScript.Echo("Helper:" + helperId + "," + RegExp.$2  + "," + RegExp.$3);
    }    


    // Format the input string. Since RTF is linear, we can search + apply formating on formatted data.

    // Make all public APIs in bold
    // [Public API 'CordbThread::EnumerateChains', this=0x001A7670]
    var rtfInput = rtf(input);
    rtfInput = rtfInput.replace(/(\[Public API .*?\])/g, rtfBold("$1"));
    rtfInput = rtfInput.replace(/(\[Dispatch.*?\])/g, rtfBold("$1"));
    
    // Make all lines for RCET in blue
    var rcetHelp = "Couldn't identify RCET";
    if (rcetId != "")
    {
        var re = new RegExp("^( *" + rcetId + " .*)$", "gm");
        rtfInput = rtfInput.replace(re, rtfColor("$1", RTF_BLUE));

        rcetHelp = "RCET (tid 0x"  +rcetId + ") is in blue.";
    }

    var w32etHelp = "Couldn't identify W32ET";
    if (w32etId != "")
    {
        var re = new RegExp("^( *" + w32etId + " .*)$", "gm");
        rtfInput = rtfInput.replace(re, rtfColor("$1", RTF_GREEN));

        w32etHelp = "W32ET (tid 0x"  +w32etId + ") is in green.";
    }

    // Write out info object
    WScript.Echo("**");
    if ((info != undefined) && (info != null))
    {
        rtfFile.writeLine(rtfBold(rtf("Field information")) + rtfEOL());
        //WScript.Echo(dump(info));
        //WScript.Echo("456");
        var info2 = dump(info);

        // CDB may put in blank lines when it dumps vars. Strip them out now.
        info2 = info2.replace(/\n\s+\n/g, "\n");

        // Hilight key fields.
        var rtfInfo = rtf(info2);
        rtfInfo = _colorizeField(rtfInfo, "m_stopCount");
        rtfInfo = _colorizeField(rtfInfo, "m_synchronized");
        rtfInfo = _colorizeField(rtfInfo, "m_syncCompleteReceived");
        rtfInfo = _colorizeField(rtfInfo, "m_continueCounter");
        rtfInfo = _colorizeField(rtfInfo, "m_outOfBandEventQueue");
        rtfInfo = _colorizeField(rtfInfo, "m_state");
        rtfInfo = _colorizeField(rtfInfo, "m_fIsNeutered");        
                    
        rtfFile.WriteLine(rtfInfo);
    }
    WScript.Echo("##");

    // Write out legend
    rtfFile.WriteLine(rtf("Threads are color coded. ") + rtfColor(rtf(rcetHelp), RTF_BLUE) + rtfColor(rtf(w32etHelp), RTF_GREEN) + rtfEOL());
    
    // Write out formatted stuff.
    rtfFile.writeLine(rtfInput);

    rtfWriteFooter(rtfFile);
}

/****************************************************************************/ 
/* Takes in a rtf CDB output stream for dumping an object, 
colorizes the given field.
*/
function _colorizeField(rtfData, szFieldName)
{
    // A CDB field looks lile:
    // +0x048 m_stopCount      : 1

    //var re = new RegExp("^\s*\S+\s+("+szFieldName+"\s*:.*)$", "gm");
    var re = new RegExp("^(.* "+szFieldName+" .*)$", "gm");

    var rtfOutput = rtfData.replace(re, rtfColor("$1", RTF_BLUE));
    return rtfOutput;
}

/****************************************************************************/ 
/* Get Right-side stress log from pid
Parameters:
    pid - process id to get stress log from
    szLogFile - file to output log to,
*/
function getRSLogFromDump(dumpFilename, szLogFile)
{
	if (dumpFilename == undefined)
		throw new Error(1, "Required parameter 'dumpFilename' missing");

    
	if (szLogFile == undefined) {
		szLogFile = "rs_test.log";
    }       

    var args = "-z " + dumpFilename;

    _getAndFormatLog(args, szLogFile)
}

/****************************************************************************/ 
/* Get Right-side stress log from pid
Parameters:
    pid - process id to get stress log from
    szLogFile - file to output log to,
*/
function getRSLogFromPid(pid, szLogFile)
{
	if (pid == undefined)
		throw new Error(1, "Required parameter 'pid' missing");
    

	if (szLogFile == undefined) {
		szLogFile = "rs_" +pid+ ".log";
    }       

    var args = "-pv -p " + pid;
    _getAndFormatLog(args, szLogFile);
  
    return 0;
}

function _getAndFormatLog(args, szLogFile)
{
    var info = { };
    var result = _getRSLogFromArgs(args, szLogFile, info);

    //WScript.Echo("1234:" + dump(info));
    if (result)
    {    
        // Color code it while we're at it.
        var szRtfOutPath = szLogFile + ".rtf";
        formatRSLog(szLogFile, szRtfOutPath, info);

        WScript.Echo("Successfully got + formatted RS Stresslog '" + szRtfOutPath + "' for " + args + ".");

        // @todo - would be nice to startup here, but there's a filelock on it since we never close it.
        //runCmd("start " + szRtfOutPath);
    }

}
/****************************************************************************/ 
/* Filter an RS stress log for stop + go information.
*/

function _t5(szInfoFilename)
{
    WScript.Echo("Filtering RS Stresslog");

    var file = FSOOpenTextFile(szInfoFilename, 1);

    while (!file.AtEndOfStream)
    {
        var str = file.ReadLine();
        
        // 6dc 00001a7dd681f854: [Dispatching 'DB_IPCE_STEP_COMPLETE']
        // 100 00001a7dd6cc3fc8: [Public API 'CordbProcess::Continue', this=0x0EA04068]    
        //  100 00001a7dd6c7d7d0: [Public API 'CordbProcess::Stop', this=0x0EA04068]
        // CRCET::SIPCE: sending DB_
        //var re = /^:(.+?)(,|$)/;

        var re = /(\[Public API 'CordbProcess::Continue')|(\[Public API 'CordbProcess::Stop')|(\[Dispatching )|(CRCET::SIPCE: sending DB_)/;
        if (re.test(str))
        {
            WScript.Echo(str);
        }             
    }    
}

/****************************************************************************/ 
/* Extract the commands from a Cordbg *.info file
parameters:
	szInfoFilename - .info file containting commands.
*/
function cordbgInfo(szInfoFilename)
{
    WScript.Echo("Extra from cordbg file");

    var file = FSOOpenTextFile(szInfoFilename, 1);

    while (!file.AtEndOfStream)
    {
        var str = file.ReadLine();
        // sample string is either:
        // :w, w, 0\).*
        // :si
        // Get command between ':' and (',' | EOL)
        var re = /^:(.+?)(,|$)/;
        if (re.test(str))
        {
            // Trim long comments.
            var cmd = RegExp.$1;

            if (cmd.length > 70)
            {
                if (/^#/.test(cmd))
                {
                    cmd = cmd.substr(0, 70) + "|...";
                }

            }

            WScript.Echo(cmd);
        }             
    }    
}


// Test task
function T4()
{
    if (runCmd("systemInfo", runSetNoThrow()).output.match(/Processor(.|\n)*\[02\](.|\n)*BIOS/m))
    {
        return true;
    }
    return false;
    //return "A".toLowerCase() == "a".toLowerCase();
}


/******************************************************************************/
/*********************** Certification    *************************************/
/******************************************************************************/


// Default share to keep the latest certified Self-Host list files.
var g_DefaultCertFileDir = "\\\\bvtsrv\\Dump\\cert";
var g_DefaultFailurDir = "\\\\bvtsrv\\Dump\\cert\\failures";


/****************************************************************************/ 
/*
    Sniff and delete old failure files    
*/
function deleteOldFailFiles()
{
}

/****************************************************************************/ 
/*
    Produce a failure file for a single failure and drop it on the share.
    This is a fast way of "invalidating" tests.
    
ex: 
  invalidateTest CERT_SELFHOSTBVT.LST=SELFHOSTBVT_4E75E159-E69C-44F9-A67B-EB80DBE5AA4C
*/ 
function invalidateTest(szTestGuid)
{
    if (szTestGuid == undefined)
    {
        throw new Error("Expected 'szTestGuid' parameter");
    }
    szTestGuid = szTestGuid.replace(/cert_/i, "bvt_");
    
    // We need to create a fake ".fail.smrt" file and drop it on the certification share.
    // The file is a 1-line string that looks like:
    // BVT_SELFHOSTBVT.LST=SELFHOSTBVT_4E75E159-E69C-44F9-A67B-EB80DBE5AA4C, 0, 0, 0, 0, 0

    // Produce a log file in the cert directory.
    // Write a timestamp.
    if (!szTestGuid.match(/=(.*)$/))
    {
        throw new Error("Test guid in wrong form.");
    }
    var szStuff = RegExp.$1;

    var szContent = szTestGuid + ", 0, 0, 0, 0, 0";

    // Give it a well-decorated name.
    var szFilename = g_DefaultFailurDir + "\\private_" + Env("USERNAME") + "_" + szStuff + ".fail.smrt";
    FSOWriteToFile(szContent, szFilename);

    WScript.Echo("Invalidated test '" +  szTestGuid + "'");
    WScript.Echo("Wrote file:"  + szFilename);

    return 0;
}



/****************************************************************************/ 
/*
    Produces Certified SelfHost list files from known failures.
    
    Reads a set of *.fail.smrt files describing known failures from the 
    szInputFailurePath directory and then produces certified .lst files
    to the szOutCertPath directory.
    
Parameters:
    szInputFailurePath - the full path containing the *.fail.smrt files.
    szInputLstPath - the full path of the incoming .lst files to certify
    szOutCertPath - the full path of which to write the certified .lst files to.
*/

function makeCert(szInputFailurePath, szInputLstPath, szOutCertPath)
{
    if (szInputFailurePath == undefined)
        szInputFailurePath = g_DefaultFailurDir; // "\\bvtsrv\Dump\cert\failures"

    if (szInputLstPath == undefined)
        szInputLstPath  = getTestBinDir(); // get .lst files from default test bin dir.
        
    if (szOutCertPath == undefined)
        szOutCertPath = g_DefaultCertFileDir; // \\bvtsrv\Dump\cert

    var runOpts = clrRunTemplate;
    runOpts = runSetTimeout(60*10, // number of seconds
        runSetNoThrow(runOpts));    


    // The real certification is done by a perl script.
    var cmd = "perl " +  ScriptDir + "\\scriptlib\\clrdbg\\CertListFiles.pl " + 
        szInputFailurePath + " " +
        szInputLstPath + " " +
        szOutCertPath;

    WScript.Echo("*** Begin cert");                    
    var run = runCmdToStream(
        cmd,
        undefined, //  log to stdout
        runOpts);
       

    WScript.Echo("*** done");    

    // Produce a log file in the cert directory.
    // Write a timestamp.
    var szdTimestamp = "Produced certified files on " +  new Date() + "\r\n" +
        "from command:\r\n" + 
        cmd + "\r\n" +
        "Command return code=" + run.exitCode + "\r\n";
    
    FSOWriteToFile(szdTimestamp, szOutCertPath +  "\\Timestamp_Cert.log");    

    return run.exitCode;
}


// check that we're in a ClrEnv window
function checkClrEnvWindow()
{
    if (Env("COMPLUS_DEFAULTVERSION") == "")
    {
        throw new Error("You must be in a clean 'clrenv' window to run this command.");
    }
    if (Env("BVT_ROOT") != "")
    {
        throw new Error("You must be in a clean 'clrenv' window to run this command. This window has been tainted since Prepbvt or devbvt.bat has already been run.");
    }
}



/****************************************************************************/ 
/*
    Get a default target directory (bvt_root). This will be where we copy 
    the tests too.
*/
function _getDefaultTargetDir()
{
    return Env("_NTBINDIR") + "\\self_host_tests\\";
}


/****************************************************************************/ 
/*
*/
function getURTTarget()
{
    return Env("Windir") + "\\microsoft.net\\framework\\" +  Env("COMPLUS_DEFAULTVERSION");    
}


/****************************************************************************/ 
/*  Enable or disable event multiplexing (redirection) for debugging the LS.
    This is designed to help debug the left-side. Only 1 native-debugger can be
    attached to the left-side. When enabled, this will:
    - shim all of ICorDebug's calls to the native-pipeline such that
    - when a process is launched / attached, ICD will actually spin up Windbg
    to debug the pipeline
    - Windbg will be the real OS debugger, it will load an extension (strikeRS) that will 
      forward all debug events to ICorDebug
    - thus Windbg can debug the Left-side, and ICD will still function normally.
    
Parameters:    
    option - 
        /init - initialize for first time use and enable.
        /enable - active (if disabled)
        /disable - to disable 
        /?  - launch wiki page with more details. 
    
    Only works on 64-bit chk/dbg builds.
    See http://mswikis/clr/dev/Pages/Event%20Mux%20for%20LeftSide.aspx for details.
*/
function enableEventMux(option)
{
    var base =  ScriptDir + "\\scriptlib\\clrdbg\\";
    
    var fEnable = undefined;
    
    if (option =="/?") 
    { 
        runCmd("start http://mswikis/clr/dev/Pages/Event%20Mux%20for%20LeftSide.aspx");
	return;
    }
    if (option == "/init")
    {
        runCmdToLog("reg import " + base + "initEventMux.reg");
        fEnable = true;
    } else if (option == "/disable") 
    {
        fEnable = false;
    }
    else if (option == undefined || option == "/enable")
    {
        fEnable = true;
    }
    else {
        throw Error("Unrecognized option:" + option);
    }
        
        
    if ((Env("_BuildArch") != "AMD64")  || (Env("_BuildType") != "chk"))          
    {
        WScript.Echo("*** Warning event multiplexing only works on amd64chk");
    }
        
    // We'd like to get fancier here, but runjs doesn't have good Reg support yet,
    // so it's tough to make the registry queries that we need.
    if (fEnable)
    {
        WScript.Echo("Enabling event multiplexing via Registry");
        WScript.Echo(" *** Be sure to disable this before running tests ***");        
        WScript.Echo("To disable, run:  clrdbgjs enableEventMux false:" );
        
        // It would be nice to add some Warnings: Eg, make sure windbg is really there?        
        runCmdToLog("reg import " + base + "enableEventMux.reg");
    }
    else
    {
        WScript.Echo("Disabling event multiplexing");
        WScript.Echo("To disable, run:  clrdbgjs enableEventMux false");
        
        runCmdToLog("reg import " + base + "disableEventMux.reg");
    }
    // What about an option to just show things?
}





/****************************************************************************/ 
/*
This starts an automation run which:
1) Copies down the latest self host tests
2) Installs one or more CLR build(s)
3) Runs self-host for one or more test runs

szTestRunNames -       (required) a semicolon delimited list of run definition
                       names for the test runs desired. These runs are defined
                       inside test run definition files (ie TestRun.xml) that
                       describe what tests to run, what architecture to run
                       on, what parts of the test drop are required,
                       additional smarty arguments, etc.
buildRoot -            (required) the path to the build for the CLR being
                       tested. This path should not include the arch/flavor
                       directory (ie. x86chk) as that portion will be
                       calculated automatically.
bldType -              (optional) the build type of the CLR to test. If not
                       specified this will default to any build type that is
                       present in the buildRoot for the appropriate
                       architecture
testRunDefSearchPath - (optional) a semicolon delimited set of files which
                       contain test run definitions. If not specified this
                       defaults to the TestRun.xml in your current directory
                       and then the TestRun.xml in the directory with this
                       script
*/
function testSelfHostOnBuild(szTestRunNames, buildRoot, bldType, testRunDefSearchPath) {

    if(!isElevated())
    {
        // In order to install the runtime (and maybe to run some of our tests?) the commands must be
        // issued from an admin command window. Check this up front so that folks don't waste time with
        // failures part way through the run
        throw new Error(1, "testSelfHostOnBuild: This script must be run from an elevated command prompt");
    }

    if (testRunDefSearchPath == undefined)
        testRunDefSearchPath = _getDefaultTestRunDefSearchPath();
 
    var runDefNames = szTestRunNames.split(";");
    var runDefs = [];
    for (var i = 0; i < runDefNames.length; i++)
        runDefs[i] = _findRunDefinition(runDefNames[i], testRunDefSearchPath); // throws on mismatch

    var runDefGroups = _sortRunDefinitionsByArch(runDefs);
    var testTasks = [];
    for (var i = 0; i < runDefGroups.length; i++)
    {
        var bldArch = runDefGroups[i].arch;
        var bldType = bldType;
        if(bldType == undefined) {
            bldType = _pickDefaultBuildType(buildRoot, bldArch)
        }
        if(bldType == undefined) {
            throw new Error("testSelfHostOnBuild: Could not find a " + bldArch + " build in " + buildRoot);
        }

        var buildDir = WshFSO.BuildPath(buildRoot, bldArch + bldType);
        buildDir = _adjustForDDRBuildDropIfNeeded(buildDir);
        var taskInstallClr = _clrSetupTask(bldType, bldArch, buildDir, "/nrad", undefined, undefined, false);
        var srcSelfHostDir = getTestBinDir(undefined, bldArch + "ret", undefined);
        var dstSelfHostDir = WshFSO.BuildPath(_getDefaultSelfHostTargetDir("%outDir%"), bldArch);
        
        
        for(var j = 0; j < runDefGroups[i].runs.length; j++)
        {
            var runDef = runDefGroups[i].runs[j];
            var verStr = bldArch + bldType;
            var relOutDir = "CopySelfhost." + runDef.szName + "." + verStr;
            var taskCopySelfHost = _createGetSelfHostTask(runDef, relOutDir, srcSelfHostDir, dstSelfHostDir);

            var dependencies = [taskCopySelfHost, taskInstallClr];
            var taskKind = "test.devBVT.SelfHost";
            relOutDir = "Selfhost." + runDef.szName + "." + verStr;
            var taskName = taskKind + "@" + relOutDir;
            var logDir = "%outDir%\\" + relOutDir;
            var verDir = getRuntimeVersionDir(verStr);
            testTasks[testTasks.length] = _createSimpleSelfHostTestsTask(taskName, runDef.szName, testRunDefSearchPath,
                                          logDir, dstSelfHostDir, bldType, verDir, dependencies);
        }
    }
    
    
    var allTestsTask = taskGroup("SelfHost Runs", testTasks);
    _taskAdd(allTestsTask);

    var scrBase;
    var outDirBase;
    try
    {
        srcBase = srcBaseFromScript();
        if(srcBase.indexOf("\\\\") == 0) // if its being run from a network share
        {
            outDirBase = WshFSO.BuildPath(WshShell.CurrentDirectory, "automation");
        }
        else
        {
            var outDirBase = WshFSO.BuildPath(srcBase, "automation");
        }
    }
    catch(e)
    {
        srcBase = WshFSO.GetParentFolderName(WScript.ScriptFullName);
        outDirBase = WshFSO.BuildPath(WshShell.CurrentDirectory, "automation");
    }

    // many of our scripts and tests don't appropriately escape paths
    // which have spaces in them. Rather than fight an uphill battle 
    // to quote everything properly it isn't that hard to just run from
    // a path with no spaces
    if(outDirBase.match(/\s/))
    {
        throw new Error("testSelfHostOnBuild: The default job directory \'" + outDirBase + 
            "\' has spaces in it which is not supported. Run this script from a path with no spaces");
    }

    logMsg(LogClrSelfHost, LogInfo, "Run outDir: " + outDirBase + "\n");
    logMsg(LogClrSelfHost, LogInfo1000, "Run srcBase: " + srcBase + "\n");
    doRun(allTestsTask, outDirBase, srcBase);
}

function _pickDefaultBuildType(buildRoot, bldArch)
{
    var buildPath = WshFSO.BuildPath(buildRoot, bldArch + "ret");
    if(FSOFolderExists(buildPath))
        return "ret";
    
    buildPath = WshFSO.BuildPath(buildRoot, bldArch + "fre");
    if(FSOFolderExists(buildPath))
        return "fre";

    buildPath = WshFSO.BuildPath(buildRoot, bldArch + "chk");
    if(FSOFolderExists(buildPath))
        return "chk";

    buildPath = WshFSO.BuildPath(buildRoot, bldArch + "dbg");
    if(FSOFolderExists(buildPath))
        return "dbg";

    return undefined;
}


// DDR has a different drop layout under the flavor/arch directory
// for the builds it produces. By looking for where clrSetup.bat is
// we can figure which layout we are dealing with
function _adjustForDDRBuildDropIfNeeded(buildPath)
{
    var ddrBinPath = WshFSO.BuildPath(buildPath, "bin");
    var ddrClrSetupPath = WshFSO.BuildPath(ddrBinPath, "ClrSetup.bat");
    if(FSOFileExists(ddrClrSetupPath))
    {
        return ddrBinPath;
    }
    else
    {
        return buildPath;
    }
}


function _getDefaultTestRunDefSearchPath() {

    var pathToScript = WScript.ScriptFullName;
    var scriptLibTestRun = WshFSO.BuildPath(WshFSO.GetParentFolderName(pathToScript), "scriptLib\\clrdbg\\TestRuns.xml");
    var curDirectoryTestRun = WshFSO.BuildPath(WshShell.CurrentDirectory, "TestRuns.xml");

    return curDirectoryTestRun + ";" + scriptLibTestRun;
}

// searches the semicolon delimited search path of test run files and searches for
// an xml testrun element within it matching the testRunName
// If there are 0 or more than 1 match it throws an error, otherwise it returns the
// RunDefinition
function _findRunDefinition(testRunName, testRunFileSearchPath) {

    if (testRunName == undefined)
        throw new Error("_findRunDefinition: Argument 'testRunName' is required");
    if (testRunFileSearchPath == undefined)
        throw new Error("_findRunDefinition: Argument 'testRunFileSearchPath' is required");
        
    var runDefFiles = testRunFileSearchPath.split(';');
    var runDefs = new Array();

    for (var i = 0; i < runDefFiles.length; i++) {
        var runDefinitions = [];
        if (FSOFileExists(runDefFiles[i])) {
            runDefinitions = _parseTestRuns(runDefFiles[i]);
            var fileRunDefs = _findRunDefinitionHelper(testRunName, runDefinitions);
            runDefs = runDefs.concat(fileRunDefs);
        }
    }

    if(runDefs.length == 0) {
        throw new Error("Run definition '" + testRunName + "' is not defined in " + testRunFileSearchPath + "\n");
    }
    else if(runDefs.length > 1) {
        throw new Error("Run definition '" + testRunName + "' is defined more than once in " + testRunFileSearchPath + "\n");
    }
    else {
        return runDefs[0];
    }
}

// returns an array of all run definitions matching the name
function _findRunDefinitionHelper(testRunName, runDefinitions) {


    // Search through well known list        
    var i;
    var ret = new Array();
    testRunName = testRunName.toLowerCase();
    for (i = 0; i < runDefinitions.length; i++) {
        var x = runDefinitions[i];
        if (x.szName.toLowerCase() == testRunName) {
	    ret.push(x);
        }
    }
    return ret;
    
}


// Returns an array of groups, each of which has a string 'arch' property and
// an array of RunDefinitions in the 'runs' property
function _sortRunDefinitionsByArch(arrayRunDefinitions)
{
    var runGroups = [];
    for(var i = 0; i < arrayRunDefinitions.length; i++)
    {
        var arch = arrayRunDefinitions[i].arch;
        var foundArch = false;
        for(var j = 0; j < runGroups.lengh; j++)
        {
            if(runGroups.arch == arch)
            {
                runGroups[i].runs[runGroups[i].runs.length] = arrayRunDefinitions[i];
                foundArch = true;
                break;
            }
	}
        
        if(!foundArch)
        {
            runGroups[runGroups.length] = new _ctorRunGroup(arrayRunDefinitions[i]);
        }
    }    
    return runGroups;
}

function _ctorRunGroup(runDefinition)
{
    this.arch = runDefinition.arch;
    this.runs = [runDefinition];
}

// Parses a test run xml file into an array of RunDefinition objects
function _parseTestRuns(testRunsFileName) {

    var runDefinitions = [];
    //logMsg(LogClrSelfHost, LogInfo, "_parseTestRuns: reading from:" + testRunsFileName + "\n");
    var xml = xmlDeserialize(testRunsFileName);

    var testRuns = xml.TestRuns.TestRun;
    if (testRuns.length == undefined)
        testRuns = [testRuns];
    
    for (var i = 0; i < testRuns.length; i++) {
        var testRun = testRuns[i];
        var includes = _parseArray(testRun.Include);
        var testDropDirs = _parseArray(testRun.TestDropDir);
        var excludes = _parseArray(testRun.Exclude);
        var smartyArgs = _parseArray(testRun.SmartyArgs);

        runDefinitions[i] = new _ctorRunDefinition(testRun.Name, testRun.Arch, includes, excludes, testDropDirs, smartyArgs);
    }

    return runDefinitions;
}

// Our xml parsing functions return either undefined, or an object with a body, or an array of object with bodies
// depending on how many of a given named xml sub-element there are. If we know in advance we want that represented
// as an array of 0-n elements this method does the transform
// input - the object returned by our xml parsing code
function _parseArray(input) {
    if (input == undefined)
        return [];
    else if (input.body != undefined)
        return [input.body];
    else {
        var items = [];
        for (var i = 0; i < input.length; i++) {
            items[i] = input[i].body;
        }
        return items;
    }
}


// Ctor to build a run definition
// szName - pretty name of the run. Eg "BST".
// arch - The architecture under which this test run is executed
// arrayCopy - array of strings for subdirs in the test tree to copy
// arrayIncludes - array of smarty category strings to include
// arrayExcludes - array of smarty category strings to exclude
// arrayAdditionalSmaryArgs - array of additional arguments for smarty
function _ctorRunDefinition(szName, arch, arrayIncludes, arrayExcludes, arrayCopy, additionalSmartyArgs)
{
    if(szName == undefined)
        throw new Error("_ctorRunDefinition: Need argument 'szName'");
    this.szName = szName;
 
    if(arch == undefined)
        throw new Error("_ctorRunDefinition: Need argument 'arch'");
    this.arch = arch;
   
    if(arrayIncludes == undefined)
        arrayIncludes = [];
    this.arrayIncludes = arrayIncludes;

    if (arrayExcludes == undefined)
        arrayExcludes = [];
    this.arrayExcludes = arrayExcludes;

    if (arrayCopy == undefined) 
        arrayCopy = [];
    this.arrayDirsToCopy = arrayCopy;

    if (additionalSmartyArgs == undefined)
        additionalSmartyArgs = [];
    this.additionalSmartyArgs = additionalSmartyArgs;
}

function _createSimpleSelfHostTestsTask(taskName, szRunDefName, szRunDefFile, logDir, bvtRoot, bldType, verDir, dependencies) {

    szRunDefFile = "\"" + szRunDefFile + "\"";
    logDir = "\"" + logDir + "\"";
    if (bvtRoot != undefined)
        bvtRoot = "\"" + bvtRoot + "\"";
    
    return taskNew(taskName,
                    "clrdbgjs.bat _runSimpleSelfHostTests "
                    + szRunDefName + " "
                    + szRunDefFile + " "
                    + valOrUnderscore(bvtRoot) + " "
                    + logDir + " "
                    + bldType + " "
                    + valOrUnderscore(verDir),
                    dependencies,  // undefined, // dependents.
                    undefined,
                    "Run the Self host tests for '" + szRunDefName + "'.");
}


function _formatCategorySmartyArgs(input, sFirst, sRest) {
    if (input == null || input == undefined)
        return "";

    var output = " ";

    output += (sFirst + " " + input[0]);

    for (var i = 1; i < input.length; i++) {
        output += (" " + sRest + " " + input[i]);
    }
    return output;
}

/****************************************************************************/
/*
Helper to invoke smarty to run an arbitrary set of selfHost tests.
    
bvt_root - (optional). root. This is where smarty.bat will be.
if excluded, this is assumed to be the same root to where 
copySelfHostTests() used as default.
outDir - output dir for smarty.
runtime - this is the runtime to use

Returns 0 on success, non-zero on failure.    
*/
function _runSimpleSelfHostTests(runDefName, runDefFile, bvt_root, outDir, bldType, runtime) {

    //logMsg(LogClrSelfHost, LogInfo, "Starting _runSimpleSelfHostTests.\n");
    if (runDefName == undefined)
        throw new Error("_simpleRunSelfHostTestsWorker: Need argument 'runDefName'");

    var runDefinition = _findRunDefinition(runDefName, runDefFile);
    var bldArch = runDefinition.arch;
    
    if (bvt_root == undefined) {
        bvt_root = _getDefaultTargetDir(bldArch);
    }

    if (bldType == undefined)
        bldType = "chk";

    if (runtime == undefined) {
        var verStr = bldArch + bldType;
        runtime = getRuntimeVersionDir(verStr);
    }

    var inc = runDefinition.arrayIncludes;
    var exc = runDefinition.arrayExcludes;

    // Try to go with a certified file if one exists.
    var all_include_lst = "CERT_ALL_INCLUDES.lst";
    if (!FSOFileExists(bvt_root + "\\" + all_include_lst)) {
        all_include_lst = "BVT_ALL_INCLUDES.lst";
    }

    var smartyArgs = "/lst " + all_include_lst +
        _formatCategorySmartyArgs(inc, "/inc", "/inc+") +
        _formatCategorySmartyArgs(exc, "/exc", "/exc+") +
        " /clean";

    // temporary hack till SmartyMetaData.pm in the test drop is fixed to match the .lst files.
    if (isCoreCLRBuild(bldType)) {
        smartyArgs += " /ttBT CoreCLR";
    }
    else {
        smartyArgs += " /ttBT FrameworkFull";
    }

    smartyArgs += " /noie";
    smartyArgs += " /ds Enabled;ConfirmFixed";

    if (outDir == undefined) {
        outDir = bvt_root + "\\smarty.run.0";
    }
    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;
    smartyArgs += " /outputDir \"" + outDir + "\"";

    for (var i = 0; i < runDefinition.additionalSmartyArgs.length;  i++)
        smartyArgs += " " + runDefinition.additionalSmartyArgs[i];

    var ret = _simpleSmartyWorker(smartyArgs, bvt_root, bldArch, runtime, outDir);
    //logMsg(LogClrSelfHost, LogInfo, "Ending _runSimpleSelfHostTests.\n");
    return ret;
}

function _simpleSmartyWorker(smartyArgs, bvtRoot, bldArch, runtime, outDir) {
    //logMsg(LogClrSelfHost, LogInfo, "Starting _simpleSmartyWorker.\n");
    // TODO: Vista: We should update this worker to support forking to an elevated window if needed by the test

    if (smartyArgs == undefined)
        throw new Error("_simpleSmartyWorker: Need argument 'smartyArgs'");
    if (bvtRoot == undefined)
        throw new Error("_simpleSmartyWorker: Need argument 'bvtRoot'");
    if (bldArch == undefined)
        throw new Error("_simpleSmartyWorker: Need argument 'bldArch'");
    if (runtime == undefined)
        throw new Error("_simpleSmartyWorker: Need argument 'runtime'");

    if (!FSOFolderExists(bvtRoot))
        throw new Error("_simpleSmartyWorker: bvtRoot '" + bvtRoot + "' doesn't exist. Is this the correct path to the testbin?");

    var runOpts = runSetEnv("COMPLUS_DefaultVersion", runtime, runOpts);
    //If COMPlus_Version is set currently, make sure to set COMPlus_Version to runtime.  If we're running a
    //baseline test pass we've changed complus_defaultversion.  We should change version to match if and
    //only if it is already set.  This should make the baseline match the original test pass.
    if (Env("COMPlus_Version") != undefined) {
        runOpts = runSetEnv("COMPlus_Version", runtime, runOpts);
    }

    //SDK_ROOT is stored in the registry by clrsetup, so it's set to whatever your last clrsetup did.
    //Let's come up with a more consistent set of variables so that PrepTests doesn't mess up the test
    //environment.
    var ndpInstallRoot = Env("SystemRoot") + "\\Microsoft.NET\\Framework";
    runOpts = runSetEnv("SDK_ROOT", ndpInstallRoot + "\\" + runtime + "\\sdk\\", runOpts);
    //And now clear out the rest of the variables PrepTests uses (smarty.pl uses the same logic).
    //Some of this logic exists in devbvt.bat.
    runOpts = runSetEnv("EXT_ROOT", '', runOpts);
    runOpts = runSetEnv("BVT_ROOT", '', runOpts);
    runOpts = runSetEnv("EXT_ROOT_ORCASGREEN", '', runOpts);
    runOpts = runSetEnv("REFERENCE_ASSEMBLIES_ORCASGREEN", '', runOpts);



    //Let's start dhandler before running the tests.
    var iarch = bldArch == "x86" ? "i386" : bldArch;

    //in ddsuites dhandler is in one place, and it's in a different place in the live test drop.
    var dhandlerPath = bvtRoot + "\\Desktop\\tools\\" + iarch + "\\dhandler.exe";

    //Where is dhandler.exe in the CoreCLR test drop???
    if (!FSOFileExists(dhandlerPath)) {
        //try the other location.
        dhandlerPath = bvtRoot + "\\Desktop\\tools\\" + iarch + "\\dhandler.exe";
    }
    var dhandler = runDo(dhandlerPath, runSetLog(LogRun, LogInfo, runSetNoThrow()));
    runPoll(dhandler); //This actually starts dhandler

    try {
        var run = runWithRuntimeArch("cmd /C \".\\PrepTests.bat & .\\smarty.bat " + smartyArgs + "\"",
                                             bldArch,
                                             runSetTimeout(20 * HOUR,
                                               runSetNoThrow(
                                               runSetCwd(bvtRoot,
                                               runSetLog(LogRun, LogInfo,
                                               runOpts)))));

        exitCode = run.exitCode;

        // HACK: smarty can return 0 or 100 for success depending on which version is used.
        if (exitCode == 100)
            exitCode = 0;

        var smartyFile = outDir + "\\Smarty.xml";
        if (FSOFileExists(smartyFile))
            _createSmartyErrFile(smartyFile);

        return exitCode;
    }
    finally {
        runTerminate(dhandler); //We don't need dhandler anymore
        //logMsg(LogClrSelfHost, LogInfo, "Ending _simpleSmartyWorker.\n");
    }
}

function _createGetSelfHostTask(runDef, relOutDir, srcSelfHostPath, dstSelfHostPath) {
    var szSubDirs = "\"" + runDef.arrayDirsToCopy.join(';') + "\"";
    srcSelfHostPath = "\"" + srcSelfHostPath + "\"";
    dstSelfHostPath = "\"" + dstSelfHostPath + "\"";
    var logDir = "\"%outDir%\\" + relOutDir + "\"";
    return taskNew(
        "CopySelfHost" + "@" + relOutDir,
        "clrdbgjs.bat GetSelfHostTests " + srcSelfHostPath +
            " " + dstSelfHostPath + " " + szSubDirs + " " +
            logDir,
        undefined,
        undefined,
        "Copy self host tests (for test run '" + runDef.szName + "') from public drop to local drive.");
}


/****************************************************************************/ 
/*
    Get a default target directory for self host tests. Under this directory
    we will make arch specific subdirectories, and in there is where we will
    copy tests
*/
function _getDefaultSelfHostTargetDir(outDir)
{
    if (outDir == undefined)
        throw new Error("_getDefaultSelfHostTargetDir: Argument 'outDir' must be specified");

    return WshFSO.BuildPath(outDir, "self_host_tests");
}

/****************************************************************************/
/*
Copy down tests from a file share. This uses robocopy, which can do an 
incremental copy. So a full test copy may take an hour, but a second copy
may only take seconds.
    
srcTestBinDir - (optional) src dir to copy tests from. 
                eg: \\clrdrop\drops\CLRv3\PUCLR\CLRTest\60123.00\x86fre\testbin
targetDir -     (optional) local target directory to copy tests too.
dirsToCopy -    (optional) a list of semicolon delimited directories to copy.
                If excluded, copies all tests
logDir -        (optional) the directory where robocopy logs get placed
                If unspecified this defaults to the targetDir
*/
function GetSelfHostTests(srcTestBinDir, targetDir, dirsToCopy, logDir) {
    if (srcTestBinDir == undefined)
        srcTestBinDir = getTestBinDir(); // get latest puclr bits

    if (targetDir == undefined)
        targetDir = _getDefaultTargetDir();

    if (logDir == undefined)
        logDir = targetDir;

    logMsg(LogClrSelfHost, LogInfo, "Copying tests from: " + srcTestBinDir + "\n");
    logMsg(LogClrSelfHost, LogInfo, "Copying to: " + targetDir + "\n");
    logMsg(LogClrSelfHost, LogInfo, "Robocopy logs in: " + logDir + "\n");

    var arrayDirsToCopy = undefined;
    if (dirsToCopy != undefined)
        arrayDirsToCopy = dirsToCopy.split(';');
    
    //
    // Now we have source + target, do the copy.
    //    
    if (!FSOFolderExists(srcTestBinDir)) {
        throw new Error("Test folder: '" + srcTestBinDir + "' does not exist.");
    }

    if (!FSOFolderExists(logDir)) {
        FSOCreateFolder(logDir);
    }

    // Don't worry about deleting because /PURGE will do that for us.
    // Else we could use FSOAtomicDeleteFolder before copy to make sure it's clean.


    // If no subdirs are specified in the alias, then just copy the whole tree
    if (arrayDirsToCopy == undefined) {
        robocopy(srcTestBinDir, targetDir, "/PURGE", targetDir + "\\copy.log");
    }
    else {
        // We just copy the toplevel set plus a subset of the folders.
        robocopy(srcTestBinDir, targetDir, "/LEV:1", logDir + "\\copy.log");
        robocopy(srcTestBinDir + "\\Desktop", targetDir + "\\Desktop", "/LEV:1", logDir + "\\copy.Desktop.log");


        // Copy subdirs that all tests will need.
        _GetTestSubDir(srcTestBinDir, targetDir, "Common", logDir);
        _GetTestSubDir(srcTestBinDir, targetDir, "Desktop\\ProductionTools", logDir);
        _GetTestSubDir(srcTestBinDir, targetDir, "Desktop\\Tools", logDir);

        // Copy sub dirs specific for our tests
        var i;
        for (i = 0; i < arrayDirsToCopy.length; i++) {
            _GetTestSubDir(srcTestBinDir, targetDir, arrayDirsToCopy[i], logDir);
        }
    }


    // Write a timestamp. Do this at the end because the roboCopy() will delete any foriegn files.
    FSOCreatePath(targetDir);
    var szdTimestamp = "Copy tests from:" + srcTestBinDir + "\r\n";
    FSOWriteToFile(szdTimestamp, targetDir + "\\Timestamp.log");

    return 0;
}

// Internal helper to copy 1 sub dir.
function _GetTestSubDir(srcTestBinDir, targetDir, subDir, logDir) {
    robocopy(srcTestBinDir + "\\" + subDir, targetDir + "\\" + subDir, "/PURGE", logDir + "\\copy." + subDir.replace(/\\/g, "_") + ".log");
}




