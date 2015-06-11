/****************************************************************************/
/*                              EzeTask.js                                  */
/****************************************************************************/

/* Support for Eze-specific tasks.  The task infrastructure in task.js
   and 'doRun.js' defines a very general purpose framework.  Here we use
   that framework to define tasks that are interesting to Eze developers.

   Basically this is defining the '_tasksAll' array that is used by the
   'doRun*' functions to allow users to run things from the command line.

   The other piece of functionality provided here is a set of 'helper'
   routines that create Eze specific tasks of a certain form (like
   tasks that build with razzle, or tasks that run Smarty).
   We then use these helpers to populate the _tasksAll list.

*/

// AUTHOR: Vance Morrison
// DATE: 11/1/2003

/****************************************************************************/

var ezeTaskModuleDefined = 1;                 // Indicate that this module exist

if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!taskModuleDefined) throw new Error(1, "Need to include task.js");
if (!EzeAutomationModuleDefined) throw new Error(1, "Need to include EzeAutomation.js");
if (!targetedPatchingModuleDefined) throw new Error(1, "Need to include targetedPatching.js");

var LogEzeTask = logNewFacility("ezeTask");

if (ScriptDir == undefined)
    var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];


/****************************************************************************/
/* A trivial wrapper that calls 'runjs doRun preCheckinTests'.  It is actually
   recommended that you use 'doRun' instead of this routine, as it is just
   as easy and you can control what you want to do better with 'doRun'

   envStr : List of variable name=value pairs (format:
            name1=value1;name2=value2;...).  This list is used to expand
            variables that appear in the command of a task.  A variable
            useful with devbvt tasks is extraSmartyArgs.  For example,
            to increase test timeout, and to prevent 10 failures from aborting
            the run, try this:

            runjs preCheckinTests "extraSmartyArgs=/ff:9999 /timeoutmultiple:100"

            (Note the double-quotes to ensure an envStr with spaces in it is
            treated as a single parameter to runjs preCheckinTests.)

   See 'runjs /? doRun' for details on what the task automation stuff does
   See 'runjs doRunShow' to see what can be run besides 'preCheckinTests.

*/
function preCheckinTests(envStr) {

    return _doPCT("preCheckinTests", envStr);
}

function preCheckinTestsFull(envStr) {

    return _doPCT("preCheckinTestsFull", envStr);
}

function syncAndTvsOACRGate(envStr) {

    return _doPCT("syncAndTvsOACRGate", envStr);
}

function _doPCT(ddrTaskStr, envStr) {

//    if (!isElevated())
//    {
//        throw new Error(1, "preCheckinTests: Requires executing with elevated privilieges");
//    }

    var srcBase = srcBaseFromScript();
    WScript.Echo("*****************************************************************************");
    WScript.Echo("*                        Starting preCheckinTests");
    WScript.Echo("*");
    WScript.Echo("* Report: " + srcBase + "\\automation\\run.current\\taskReport.html");
    WScript.Echo("* The report can be viewed before the run is complete");
    WScript.Echo("*****************************************************************************");
    WScript.Echo("");

    var outDirBase = srcBaseFromScript() + "\\automation";
    var numRunsToKeep = Env("DDR_RUNS_TO_KEEP");
    if (numRunsToKeep == "")
        numRunsToKeep = 3;

    var outDir = newRunDir(outDirBase, numRunsToKeep);
    var ret = doRunHere(ddrTaskStr, outDir, undefined, envStr);

    /*
    // This is an unfortunate kind of wired in knowledge, but it can not cause failure at least
    // We may want to pull this
    var shortCutName = "\\\\CLRMain\\public\\Drops\\Whidbey\\preCheckinTestss\\" + Env("USERNAME") + "_" + Env("COMPUTERNAME");
    _publish(shortCutName, outDir + "\\taskReport.html");
    _publishLogs(outDir);
    */

    return ret;
}
/*****************************************************************************/
/*                             USEFUL TASKS                                  */
/*****************************************************************************/
/* a list of all tasks that are interesting to end users.  When you add new
   tasks that you believe are generally useful for everyone you put them here.
   You can then invoke a task (and all of its children), using doRun <taskName>
   taskGroup is a mechanism for grouping related tasks so that they can be
   invoked as a group.  Note that 'doRun' will find a task anywhere in the
   hierarchy of tasks in this list.
*/

while (true)
{
    try
    {
        srcBaseFromScript();
    }
    catch (e)
    {
        WScript.Echo("Can't determine VBL root so skipping ezeTask setup (means 'runjs doRun' tasks won't be available");
        break; // can't deduce srcBaseFromScript so don't setup the tasks
    }

    var myArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    function IF_RUN(cond, value) {
        if (cond)
            return value;
        return taskNew("nullTask", "exit 0", undefined, "local", "this is a dummy task that does nothing");
    }

    var _taskBuildAllFull =
        taskGroup("buildAllFull", [
            _razzleBuildFullTaskGroup("chk", "x86"),
            _razzleBuildFullTaskGroup("fre", "x86"),
            _razzleBuildFullTaskGroup("chk", "amd64"),
            _razzleBuildFullTaskGroup("fre", "amd64"),
            _razzleBuildFullTaskGroup("chk", "arm"),
            _razzleBuildFullTaskGroup("fre", "arm"),
        ]);

    var _taskBuildAllHere =
        taskGroup("buildAllHere", [
            _buildHereTask("chk", "x86"),
            _buildHereTask("fre", "x86"),
            _buildHereTask("chk", "amd64"),
            _buildHereTask("fre", "amd64"),
            _buildHereTask("chk", "arm"),
            _buildHereTask("fre", "arm"),
        ]);

    var _taskBuildFull =
        taskGroup("buildFull", [
            _razzleBuildFullTaskGroup(),
        ]);

    var _taskSync = taskNew("sdxSync", "runjs sdxSyncAndResolve", undefined, undefined, "This task syncs our source code so that we are building/testing the latest product");
    var _taskSyncAndBuildFull =
        taskGroup("syncAndBuildFull", [
            _taskSync,
            _taskBuildFull,
        ]);

    var preCheckinTestsFullBuildRelDir = "inetcore";
    var _taskPreCheckinTestsFull =
        taskGroup("preCheckinTestsFull", [
            // _buildAndCheckOACRTask("chk", "x86", preCheckinTestsFullBuildRelDir), // Skip OACR on chk (same as snap)
            _buildAndUnitTestTask("chk", "x86", preCheckinTestsFullBuildRelDir),
            // Removing broken MsTest tests from preCheckinTests for now
            // _buildAndMSTestTestsTask("chk", "x86", preCheckinTestsFullBuildRelDir),
            _buildAndCheckOACRTask("fre", "x86", preCheckinTestsFullBuildRelDir),
            _buildAndUnitTestTask("fre", "x86", preCheckinTestsFullBuildRelDir),
            // Removing broken MsTest tests from preCheckinTests for now
            // _buildAndMSTestTestsTask("fre", "x86", preCheckinTestsFullBuildRelDir),
            // _buildAndCheckOACRTask("chk", "amd64", preCheckinTestsFullBuildRelDir), // Skip OACR on chk (same as snap)
            _buildAndUnitTestTask("chk", "amd64", preCheckinTestsFullBuildRelDir),
            _buildAndCheckOACRTask("fre", "amd64", preCheckinTestsFullBuildRelDir),
            _buildAndUnitTestTask("fre", "amd64", preCheckinTestsFullBuildRelDir),
            _razzleBuildFullTaskGroup("chk", "arm"),
            _razzleBuildFullTaskGroup("fre", "arm"),
        ]);

    var _taskSyncAndPreCheckinTestsFull =
        taskGroup("syncAndPreCheckinTestsFull", [
            _taskSync,
            _taskPreCheckinTestsFull,
        ]);

    var preCheckinTestsBuildRelDir = "inetcore\\jscript";
    var _taskPreCheckinTests =
        taskGroup("preCheckinTests", [
            _buildAndCheckOACRTask("fre", "x86", preCheckinTestsBuildRelDir),
            _buildAndUnitTestTask("fre", "x86", preCheckinTestsBuildRelDir),
            // Removing broken MsTest tests from preCheckinTests for now
            // _buildAndMSTestTestsTask("fre", "x86", preCheckinTestsBuildRelDir),
            // _buildAndCheckOACRTask("chk", "x86", preCheckinTestsBuildRelDir), // Skip OACR on chk (same as snap)
            _buildAndUnitTestTask("chk", "x86", preCheckinTestsBuildRelDir),
            // Removing broken MsTest tests from preCheckinTests for now
            // _buildAndMSTestTestsTask("chk", "x86", preCheckinTestsBuildRelDir),
            // _buildAndCheckOACRTask("chk", "amd64", preCheckinTestsBuildRelDir), // Skip OACR on chk (same as snap)
            _buildAndUnitTestTask("chk", "amd64", preCheckinTestsBuildRelDir),
            _buildAndCheckOACRTask("fre", "amd64", preCheckinTestsBuildRelDir),
            _buildAndUnitTestTask("fre", "amd64", preCheckinTestsBuildRelDir),
            _razzleBuildTask("chk", "arm", preCheckinTestsBuildRelDir),
            _razzleBuildTask("fre", "arm", preCheckinTestsBuildRelDir),
        ]);

    var _taskTvsOACRGate =
        taskGroup("tvsOACRGate", [
            _buildAndCheckOACRTask("fre", "x86", preCheckinTestsFullBuildRelDir, undefined, undefined, /*oacrAll*/true),
            _buildAndCheckOACRTask("fre", "amd64", preCheckinTestsFullBuildRelDir, undefined, undefined, /*oacrAll*/true),
        ]);
    var _taskSyncAndTvsOACRGate =
        taskGroup("syncAndTvsOACRGate", [
            _taskSync,
            _taskTvsOACRGate,
        ]);

    var preCheckinTestsBuildRelDirF12 = "inetcore\\devtoolbar";

    var _taskBuildF12Dependencies =
        taskGroup("buildF12Dependencies", [
            // Additional inetcore compilation required for F12 tools
            _razzleBuildF12DependenciesTaskGroup("fre", "x86"),
            _razzleBuildF12DependenciesTaskGroup("chk", "x86"),
            _razzleBuildF12DependenciesTaskGroup("fre", "amd64"),
            _razzleBuildF12DependenciesTaskGroup("chk", "amd64"),
            _razzleBuildF12DependenciesTaskGroup("fre", "arm"),
            _razzleBuildF12DependenciesTaskGroup("chk", "arm"),
        ]);

    var _taskBuildF12 =
        taskGroup("buildF12", [
            // F12 build and OACR verification. This task does no testing.
            _buildAndCheckOACRTask("fre", "x86",   preCheckinTestsBuildRelDirF12, "_buildF12"),
            _buildAndCheckOACRTask("chk", "x86",   preCheckinTestsBuildRelDirF12, "_buildF12"),
            _buildAndCheckOACRTask("fre", "amd64", preCheckinTestsBuildRelDirF12, "_buildF12"),
            _buildAndCheckOACRTask("chk", "amd64", preCheckinTestsBuildRelDirF12, "_buildF12"),

            // F12 build verification only for arm
            _razzleBuildTask("chk", "arm", preCheckinTestsBuildRelDirF12),
            _razzleBuildTask("fre", "arm", preCheckinTestsBuildRelDirF12),
        ]);

    var _taskBuildF12Full =
        taskGroup("buildF12Full", [
            _taskBuildF12Dependencies,
            _taskBuildF12
        ]);

    var _taskSyncAndPreCheckinTestsFullF12 =
        taskGroup("syncAndPreCheckinTestsFullF12", [
            _taskSyncAndPreCheckinTestsFull,
            _taskBuildF12Full
        ]);

    var _taskPreCheckinTestsFullF12 =
        taskGroup("preCheckinTestsFullF12", [
            _taskPreCheckinTestsFull,
            _taskBuildF12Full
        ]);

    var _taskPreCheckinTestsF12 =
        taskGroup("preCheckinTestsF12", [
            _taskPreCheckinTests,
            _taskBuildF12Full
        ]);

    var _taskBuildAndCheckOACRHere =
        taskGroup("buildAndCheckOACRHere", [
            _buildAndCheckOACRTask(),
        ]);

    var _tasksAll = [
        _taskSyncAndPreCheckinTestsFull,
        _taskPreCheckinTests,
        _taskSyncAndBuildFull,
        _taskBuildAndCheckOACRHere,
        _taskBuildAllHere,
        _taskBuildAllFull,
        _taskBuildF12,
        _taskBuildF12Dependencies,
        _taskPreCheckinTestsF12,
        _taskPreCheckinTestsFullF12,
        _taskSyncAndPreCheckinTestsFullF12,
        _taskSyncAndTvsOACRGate,
    ];

    break;
}

/****************************************************************************/
/* Adds a task to the list of tasks available to doRun
   Parameters
     task               : A task or task group created with any of the _*Task
                  methods available or taskGroup.
*/
function _taskAdd(task) {
    if (task == undefined) {
        throw Error(1, "Arg task not supplied");
    }
    _tasksAll.push(task);
    return;
}

function _razzleBuildFullTaskGroup(bldType, bldArch, oacrAll)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    return taskGroup("buildFull_" + (oacrAll ? "oacrAll_" : "") + bldArch + bldType, [
        _razzleBuildTask(bldType, bldArch, {
            taskNameString: "inetcore-dep",
            directories: [
                "inetcore\\published\\sdk\\inc",
                "inetcore\\published\\sdk\\uuid",
                "inetcore\\published\\internal\\inc",
                "inetcore\\published\\internal\\uuid",
                "inetcore\\manifests\\inbox",
                "inetcore\\lib\\codex",
                "inetcore\\lib\\dep",
                "inetcore\\lib\\nav\\fck\\iel2",
                "inetcore\\lib\\filepath\\ids",
                "inetcore\\lib\\inc",
                "inetcore\\lib\\common\\iel1",
                "inetcore\\lib\\common\\iel1_sp",
                "inetcore\\lib\\common\\iel1_mc",
                "inetcore\\lib\\nav\\iel2",
                "inetcore\\lib\\devtb\\dtbhost",
                "inetcore\\lib\\indexeddb\\scalookup",
                "inetcore\\ieframe\\shlwapi\\private_xp",
                "inetcore\\lib\\com\\iel1",
                "inetcore\\lib\\security\\iel2",
                "inetcore\\lib\\trace",
                "inetcore\\lib\\setting",
                "inetcore\\lib\\aboutconfig",
                "inetcore\\lib\\ScriptProjectionHost\\iel3_edge"]
        }, undefined, 3 * HOUR), // Give this a long timeout as publics can take a while
        _razzleBuildTask(bldType, bldArch, "inetcore\\jscript", undefined, undefined, undefined, undefined, oacrAll)
    ]);
}

function _razzleBuildF12DependenciesTaskGroup(bldType, bldArch)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);

    return taskGroup("buildFull_" + bldArch + bldType + "_F12Dependencies", [
        _razzleBuildTask(bldType, bldArch, "inetcore\\\published\\internal\\uuid"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\com\\iel1"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\common"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\devtb\\inputobject"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\security\\iel2"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\ext"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\trace\\lib"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\lib\\ux"),
        _razzleBuildTask(bldType, bldArch, "inetcore\\\ieframe\\shlwapi\\private_xp")
    ]);
}

function _buildAndUnitTestTask(bldType, bldArch, bldRelDir)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    if (bldRelDir == undefined)
    {
        bldRelDir = "inetcore\\jscript";
    }
    var timeOut = 30 * MINUTE;
    var subTasks = new Array();
    subTasks.push( (bldRelDir == preCheckinTestsFullBuildRelDir) ? _razzleBuildFullTaskGroup(bldType, bldArch) : _razzleBuildTask(bldType, bldArch, bldRelDir) );
    return (taskNew("consoleUnitTests_" + bldArch + bldType, "runjs runConsoleUnitTests " + bldType + " " + bldArch  + ' \"' + timeOut + '\"', subTasks));
}

function _buildAndMSTestTestsTask(bldType, bldArch, bldRelDir)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    if (bldRelDir == undefined)
    {
        bldRelDir = "inetcore\\jscript";
    }
    var subTasks = new Array();
    subTasks.push( (bldRelDir == preCheckinTestsFullBuildRelDir) ? _razzleBuildFullTaskGroup(bldType, bldArch) : _razzleBuildTask(bldType, bldArch, bldRelDir) );
    return (taskNew("MSTestUnitTests_" + bldArch + bldType, "runjs runMSTestUnitTests " + bldType + " " + bldArch, subTasks));
}

function _buildAndCheckOACRTask(bldType, bldArch, bldRelDir, taskName, timeOut, oacrAll)
{
    if (timeOut == undefined)
        timeOut = (oacrAll ? 1 * HOUR : 20 * MINUTE);
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    if (bldRelDir == undefined)
    {
        bldRelDir = relPath(WshShell.CurrentDirectory, srcBaseFromScript());
    }
    var subTasks = new Array();
    subTasks.push( (bldRelDir == preCheckinTestsFullBuildRelDir) ? _razzleBuildFullTaskGroup(bldType, bldArch, oacrAll) : _razzleBuildTask(bldType, bldArch, bldRelDir) );
    return (taskNew((oacrAll ? "checkOACR_All_" : "checkOACR_") + bldArch + bldType + (taskName == undefined ? "" : taskName), "runjs razzleCommand " + bldType + " " + bldArch + " " + "inetcore" +
                          " \"" + ScriptDir + "\\runjs checkOACR " + bldType + " " + bldArch + " " + "inetcore" + ' ' + timeOut + '\" ' + timeOut, subTasks
    ));
}

function _buildHereTask(bldType, bldArch, bldRelDir)
{
    bldType = getBldType(bldType);
    bldArch = getBldArch(bldArch);
    if (bldRelDir == undefined)
    {
        bldRelDir = relPath(WshShell.CurrentDirectory, srcBaseFromScript());
    }
    return _razzleBuildTask(bldType, bldArch, bldRelDir);
}

/****************************************************************************/
/* Create a task that does a razzle build.

  logs go to %outDir%\<bldArch><bldType>
  binaries go to %outDir%\<bldArch><bldType>\bin

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
                  This also can contain an optional suffix such as ".unopt"
    bldArch     : The build architecture (x86, amd64 ia64)
    bldRelDir   : The directory relative to %srcBase% to build in, or a build directory descriptor
    bldArgs     : additional parameters to pass to build (like -cC)
    timeOut     : Set the timeout for the build, defaults to 120 minutes
    binDir      : The directory to place the built binaries in,
                  (defaults to razzleBuildDefaultBinDir -> StandardBinDir -> srcBase + ".binaries." + bldArch + bldType)
    dependents  : dependents that this task depends on
    oacrAll     : Set oacr on all files (default only on sd opened files)
*/
function _razzleBuildTask(bldType, bldArch, bldRelDir, bldArgs, timeOut, binDir, dependents, oacrAll) {
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (bldRelDir == undefined)
        bldRelDir = "inetcore\\jscript";

    // Setup baseBldType by removing any suffix that may have been attached to bldType
    var baseBldType = bldType;
    if (baseBldType.match(/(.+)\.(.+)$/)) {
        baseBldType = RegExp.$1;
    }

    var relOutDir = bldArch + bldType;

    var taskNameString = bldRelDir.taskNameString || bldRelDir;
    var bldDirectories = bldRelDir.directories ? bldRelDir.directories.join(';') : bldRelDir;
    var taskName = "razzleBuild." + (oacrAll ? "oacrAll." : "") + taskNameString.replace(/[\\]/g, "-") + "@" + relOutDir;

    var logDir = "%outDir%\\" + relOutDir;

    if (binDir == undefined)
    {
        //binDir = logDir + "\\bin";
        binDir = "razzleBuildDefaultBinDir";
    }

    if (timeOut == undefined)
        timeOut = 120 * MINUTE;

    if (bldArgs == undefined)
        bldArgs = "-cZP";
    if (bldArgs == "")
        bldArgs = "\"\"";

    var ret = taskNew(taskName,
                    "runjs razzleBuild " + '\"' + baseBldType + '\"' + " "
                                         + '\"' + bldArch + '\"' + " "
                                         + '\"' + bldDirectories + '\"' + " "
                                         + '\"' + bldArgs + '\"' + " "
                                         + "\"%srcBase%\" "
                                         + '\"' + binDir + '\"' + " "
                                         + '\"' + logDir + '\"' + " "
                                         + '\"' + timeOut + '\" '
                                         + (oacrAll ? '_ _ _ oacrAll' : ''),  //rArgs, runOpts, preBld, oacrAll
                                         dependents,
                                         /(x86)|(amd64)/i);

    ret.description = "A normal (razzle) build of some directory in the source base.";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Running%20Tests.aspx#_Toc117485070";
    return ret;
}
