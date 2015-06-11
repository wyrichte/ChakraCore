/****************************************************************************/
/*                              clrtask.js                                  */
/****************************************************************************/

/* Support for CLR specific tasks.  The task infrastructure in task.js
   and 'doRun.js' defines a very general purpose framework.  Here we use
   that framework to define tasks that are interesting to CLR developers.

   Basically this is defining the '_tasksAll' array that is used by the
   'doRun*' functions to allow users to run things from the command line.

   The other piece of functionality provided here is a set of 'helper'
   routines that create CLR specific tasks of a certain form (like
   tasks that build with razzle, or tasks that run Smarty).
   We then use these helpers to populate the _tasksAll list.

*/

// AUTHOR: Vance Morrison
// DATE: 11/1/2003

/****************************************************************************/

var clrTaskModuleDefined = 1;                 // Indicate that this module exist

if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!taskModuleDefined) throw new Error(1, "Need to include clrTask.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include ClrAutomation.js");
if (!targetedPatchingModuleDefined) throw new Error(1, "Need to include targetedPatching.js");

var LogClrTask = logNewFacility("clrTask");

if (ScriptDir == undefined)
    var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

//Determine if nativeImageReusabilityDetection(nird) test needs to be run.
//More details in file targetedpatching.js
var runNird = _shouldRunNird();

/****************************************************************************/
/* A trivial wrapper that calls 'runjs doRun dailyDevRun'.  It is actually
   recommended that you use 'doRun' instead of this routine, as it is just
   as easy and you can control what you want to do better with 'doRun'

   envStr : List of variable name=value pairs (format:
            name1=value1;name2=value2;...).  This list is used to expand
            variables that appear in the command of a task.  A variable
            useful with devbvt tasks is extraSmartyArgs.  For example,
            to increase test timeout, and to prevent 10 failures from aborting
            the run, try this:

            runjs dailyDevRun "extraSmartyArgs=/ff:9999 /timeoutmultiple:100"

            (Note the double-quotes to ensure an envStr with spaces in it is
            treated as a single parameter to runjs dailyDevRun.)

   See 'runjs /? doRun' for details on what the task automation stuff does
   See 'runjs doRunShow' to see what can be run besides 'dailyDevRun.

*/
function dailyDevRun(envStr) {

    return _doDDR("dailyDevRun", envStr);
}


/****************************************************************************/
/* Same as runjs dailyDevRun, except on 64-bit boxes, this includes a full
   run of checkinbvt, buildbvt, and devunit bvts in the WOW.
   (Normally runjs dailyDevRun only runs checkinbvt and devunit bvts in
   the WOW.)

   See 'runjs /? dailyDevRun' for more details, including envStr usage
*/
function dailyDevRunFull64(envStr) {
    return _doDDR("dailyDevRunFull64", envStr);
}

function dailyDevRunSOC(envStr)
{
    return _doDDR("dailyDevRunSOC", envStr);
}


function _doDDR(ddrTaskStr, envStr) {

    if (!isElevated())
    {
        throw new Error(1, "dailyDevRun: Requires executing with elevated privilieges");
    }

    var srcBase = srcBaseFromScript();
    WScript.Echo("*****************************************************************************");
    WScript.Echo("*                        Starting dailyDevRun");
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

    // This is an unfortunate kind of wired in knowledge, but it can not cause failure at least
    // We may want to pull this
    var shortCutName = "\\\\CLRMain\\public\\Drops\\Whidbey\\dailyDevRuns\\" + Env("USERNAME") + "_" + Env("COMPUTERNAME");
    _publish(shortCutName, outDir + "\\taskReport.html");
    _publishLogs(outDir);

    return ret;
}


function coreCLRdailyTestRun() {
    return doRun("coreCLRdailyTestRun", undefined, undefined, undefined, 5000);
}


// function installDailyDevRun() has been moved to clrautomation.js

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

var myArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

function IF_RUN(cond, value) {
    if (cond)
        return value;
    return taskNew("nullTask", "exit 0", undefined, "local", "this is a dummy task that does nothing");
}

var _taskRetailBuild =
    taskGroup("retailBuild", [
        // retailBuild and WSPerf for the current machine's arch
        _retailBuildTask("ret", myArch),
    ]);

var _taskRetailBuildAndTest =
    taskGroup("retailBuildAndTest", [
        _WSPerfTestsTask("ret", myArch),
        _devBVTTask("ret", myArch, undefined, undefined,
                    [ _taskRetailBuild ])
    ]);

var _taskRetailBuild_x86 =
    taskGroup("retailBuild_x86", [
        // retailBuild and WSPerf for x86ret
        _retailBuildTask("ret", "x86"),
    ]);

var _taskRetailBuildAndTest_x86 =
    taskGroup("retailBuildAndTest_x86", [
        _WSPerfTestsTask("ret", "x86"),
        _devBVTTask("ret", "x86", undefined, undefined,
                    [ _taskRetailBuild_x86 ])
    ]);

var _taskRetailBuildBoth =
    taskGroup("retailBuildBoth", [
        _taskRetailBuild_x86,
        _taskRetailBuild,
    ]);

var _taskRetailBuildAndTestBoth =
    taskGroup("retailBuildAndTestBoth", [
        _taskRetailBuildAndTest_x86,
        _taskRetailBuildAndTest,
    ]);

var _taskBuildRetailLayouts =
    taskGroup("buildFullRetailLayouts", [
        _buildLayoutsTask("all", "amd64", "ret", "%outDir%\\x86ret\\bin", "%outDir%\\setup", [
            _razzleBuildTask("ret", "x86", "ndp"),
            _razzleBuildTask("ret", "amd64", "ndp")
        ])
    ]);

var _taskBuildChkLayouts =
    taskGroup("buildFullChkLayouts", [
        _buildLayoutsTask("all", "amd64", "chk", "%outDir%\\x86chk\\bin", "%outDir%\\setup", [
            _razzleBuildTask("chk", "x86", "ndp"),
            _razzleBuildTask("chk", "amd64", "ndp")
        ])
    ]);

var _taskClrSetup =
    taskGroup("rollingClrSetup", [
        _clrSetupTask("ret","x86", undefined, "/fx /nrad"),
    ]);

// The common tasks for all forms of DailyDevRun
// Note that we don't use a taskGroup here because we don't want to influence dependency relationships
// or the display in the task report.  This is just a mechanism to avoid duplicating thist task list
// for both _taskDailyDevRun and _taskDailyDevRunFull64
var _dailyDevRunCommonTasksDesktop =
    taskGroup("Desktop", [
        _verifyMachineStateTask(myArch, "chk"),

        IF_RUN(runNird == 1, _nirdTask("ret", "x86")),

        _razzleBuildTask("ret", "x86", "ndp"),

        _razzleBuildTask("ret", "arm", "ndp"),

        _razzleBuildTask("chk", "arm", "ndp"),

        _buildWarningTask("chk", "x86"),
        _scanRuntimeTask(),
        _devBVTTask("chk", "x86"),

        _razzleBuildTask("ret", myArch, "ndp"),
        _devBVTTask("chk", myArch),

// disabled due to break in FI        _taskDacCop("chk","x86"), // COMMENT THIS LINE TO DISABLE DACCOP in DDR
// Disabled due to incompatablilty with MSBuild.  Will be replaced by OACR ASAP        _prefastBuildTask(),

        taskGroup("ddIntFx", [
	        // Layering test is run against the retail build
            _ddIntTask("ret", "x86",undefined,"src\\Layering\\Tests\\clrv3","-g ddint",
                [_clrSetupTask("ret","x86", undefined, "/fx /nrad")],
                undefined,
                "This task scans CLR binaries for dependencies on other binaries and reports if there are any violations.",
                "Refer to http://sharepoint/sites/layering for details"),
        ]),

        _createOwnerList()

    ]);


var _taskSecurityConsistencyTest =
    taskGroup("SecurityConsistencyTest", [
        _coreCLRSecurityConsistencyTask("corefre", "x86", "mscorlib.dll", [_razzleBuildTask("corefre", "x86", "ndp")]),
        _coreCLRSecurityConsistencyTask("corefre", "x86", "System.dll", [_razzleBuildTask("corefre", "x86", "ndp")]),
        _coreCLRSecurityConsistencyTask("corefre", "x86", "System.Core.dll", [_razzleBuildTask("corefre", "x86", "ndp")]),
        _coreCLRSecurityConsistencyTask("corefre", "x86", "Microsoft.WlcProfile.dll", [_razzleBuildTask("corefre", "x86", "ndp")])
    ],
    "Runs a checks over platform assemblies to make sure they are in a good security state.  For details, see http://devdiv/sites/CLR/security/Shared%20Documents/CoreCLRSecurityConsistencyTest.aspx");

var _canRunAmd64 = (myArch == "amd64" || Env("PROCESSOR_ARCHITEW6432").toLowerCase() == "amd64");

// Define a separate SoC DDR tasks to enable "runjs dailyDevRunSOC".
var _taskDailyDevRunSOC =
    taskGroup("dailyDevRunSOC", [
         _razzleBuildTask("coredbg", "arm"),
         _razzleBuildTask("corechk", "arm"),
         _razzleBuildTask("corefre", "arm"),
         IF_RUN(!nsIsNullOrEmpty(_getBeagleIP()), _beagleBVTTask("corechk")),
         IF_RUN(!nsIsNullOrEmpty(_getBeagleIP()), _beagleBVTTask("corefre")),
         IF_RUN(!nsIsNullOrEmpty(_getBeagleIP()), _beagleBVTTask("coredbg"))
    ],
    "Runs dailyDevRun tests only for SoC.");

// Useful non-DDR SOC tasks
var _taskNonDailyDevRunSOC =
    taskGroup("nonDailyDevRunSOC", [
        // x86 AltJit tests: run all x86 CoreCLR tests with AltJit
        _devBVTTask("corechk", "x86", undefined, "/altjit /exc GC_STRESS;Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "x86") ]),

        IF_RUN(!nsIsNullOrEmpty(_getBeagleIP()), _beagleEHBVTTask("corechk")),
        IF_RUN(!nsIsNullOrEmpty(_getBeagleIP()), _beagleEHBVTTask("corefre"))
        // no need for _beagleEHBVTTask on coredbg
    ],
    "Runs non-dailyDevRun tests only for SoC.");

var _dailyDevRunCommonTasksCoreCLR =
    taskGroup("CoreCLR", [
        // FIX, I have put amd64 build first because 64 bit builds update mscoree.h
        // which causes x86 builds to have to start over.  I am working on a real fix
        // however in the mean time do the 64 bit build first so that the x86 build is OK
          _razzleBuildTask("corechk", "amd64", "ndp"),

         //
         // SoC DDR task is part of CoreCLR DDR
         //
         _taskDailyDevRunSOC,

        _buildWarningTask("corechk", "x86"),
        _scanRuntimeTask("corechk"),

        // X86 Tests
        // GCStress Tests (excluding debugger tests)
        _devBVTTask("corechk", "x86", undefined, "/inc GC_STRESS /exc Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "x86") ]),
        // Do the BVT for X86/Chk (excluding GC_STRESS and debugger tests)
        _devBVTTask("corechk", "x86", undefined, "/exc GC_STRESS;Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "x86") ]),

        // Run the FxCop Tests - the last argument specifies whether these are regular bvts [leave undefined] or fxcop tests.
        // This is so since for FxCOP tests, we have to make smarty not use the CoreCLR loader for testing.
        _devBVTTask("corechk", "x86", undefined, "/inc FXCOP", [ _fxpSetupTask("corechk", "x86") ], "fxcop"),

        // AMD64 Tests
        // GCStress Tests (excluding debugger tests)
        IF_RUN(_canRunAmd64, _devBVTTask("corechk", "amd64", undefined, "/inc GC_STRESS /exc Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "amd64") ])),
        // Do the BVT for X86/Chk (excluding GC_STRESS and debugger tests)
        IF_RUN(_canRunAmd64, _devBVTTask("corechk", "amd64", undefined, "/exc GC_STRESS;Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "amd64") ])),

        // AMD64 AltJit test(s)
        IF_RUN(_canRunAmd64, _devBVTTask("corechk", "amd64", undefined, "/altjit /iua AltJit", [ _fxpSetupTask("corechk", "amd64") ])),

        // Run the layering tests
        _coreCLRLayeringTask("corefre", "x86",undefined,"src\\Layering\\Tests\\CoreCLR","-g ddint", [_razzleBuildTask("corefre", "x86",  "ndp")]),

        // To add files for file size check, update <enlistment root>\tools\DDR\CoreCLRFileSizeCheck.txt
        _coreCLRFileSizeCheckTask("corefre", "x86", [_razzleBuildTask("corefre", "x86",  "ndp")]),

        // Ensure the platform binaries are setup properly for security
        _taskSecurityConsistencyTest,

        // Perform the prefast checks
// Disabled due to incompatablilty with MSBuild.  Will be replaced by OACR ASAP        _prefastBuildTask("corefre"),
        _razzleBuildTask("corefre", "x86", "ndp"),
    ]);

var _coreCLRdailyDevRun =
    taskGroup("coreCLRdailyDevRun", [
        taskNew("sdWhereSynced", "runjs sdWhereSynced %srcBase%\\ndp\\clr\\src %srcBase%", undefined, "local",
                "This task does a sdWhereSynced to log what sync point the run used to build"),
        _dailyDevRunCommonTasksCoreCLR
    ]);

var _dailyDevRunCommonTasks = [

        // This task checks if there are any pending changes
        IF_RUN(runNird==1, taskNew("PendingChanges", "runjs _nirdPendingChanges %srcBase%", undefined, "local",
                                   "Verifies that are no pending changes in the enlistment. Required for targeted patching test.")),

        // This task verifies that the shelveset does not modify selelcted fx assemblies
        IF_RUN(runNird==1 && tpIncompatMscorlibChange != 1, taskNew("VerifyShelveset", "runjs _nirdVerifyShelveset %shelveset%",
                                                                    undefined, "local",
                                                                    "Verifies that the shelveset does not modify certain assemblies." +
                                                                    "This is required for targetedpatching test.")),

            // This task lets us see what sync point people were at when they built
        taskNew("sdWhereSynced", "runjs sdWhereSynced %srcBase%\\ndp\\clr\\src %srcBase%", undefined, "local",
                "This task does a sdWhereSynced to log what sync point the run used to build."),
            // This task lets us see what sync point people's tests are at
        taskNew("sdWhereSynced_tests", "runjs sdWhereSynced %srcBase%\\ddsuites\\src\\clr %srcBase%", undefined, "local",
                "This task does a sdWhereSynced to log what sync point the run used for tests."),

			// This task insures that people's dev directory is up to date
        taskNew("SyncDevDir", "runjs SyncDevDir %srcBase%", undefined, "local",
                "This syncs the vbl\dev\... tree which holds documentation and tools."),

    ];

var _taskDailyDevRunFull64 =
    taskGroup(
        "dailyDevRunFull64",
        _dailyDevRunCommonTasks.concat(
            _dailyDevRunCommonTasksDesktop,
            _dailyDevRunCommonTasksCoreCLR
        ),
        "DailyDevRun is the group of tests that CLR developers are expected to run before check-in. " +
        "http://mswikis/clr/dev/Pages/Running%20Tests.aspx"
    );

var _taskDailyDevRun =
    taskGroup("dailyDevRun",
        _dailyDevRunCommonTasks.concat(
            _dailyDevRunCommonTasksDesktop,
            _dailyDevRunCommonTasksCoreCLR
        ),
        "DailyDevRun is the group of tests that CLR developers are expected to run before check-in. " +
        "http://mswikis/clr/dev/Pages/Running%20Tests.aspx"
    );

var _taskQuickDevRuns =
    taskGroup(
        "quickDevRunsAll",
        [
            _quickDevRunTask("chk", "x86", "ndp\\clr"),
            _quickDevRunTask("chk", "amd64", "ndp\\clr"),
            _quickDevRunTask("chk", "ia64", "ndp\\clr"),
            _quickDevRunTask("chk", "x86", "ndp"),
            _quickDevRunTask("chk", "amd64", "ndp"),
            _quickDevRunTask("chk", "ia64", "ndp")
        ]
    );

// To add files for file size check, update <enlistment root>\tools\DDR\CoreCLRFileSizeCheck.txt
var _taskCoreCLRFileSizeCheck =
    taskGroup("CoreCLRFilesizeCheck", [

    _coreCLRFileSizeCheckTask("corefre", "x86", [_razzleBuildTask("corefre", "x86",  "ndp")])
    ],
    "Runs CoreCLR file size check test.");

var _taskCoreCLRLayeringTest =
    taskGroup("LayeringTest", [

    _coreCLRLayeringTask("ret", "x86",undefined,"src\\Layering\\Tests\\CoreCLR","-g ddint", [_razzleBuildTask("ret", "x86",  "ndp")]),
    ],
    "Runs CLR Layering Quality Gate Test. For details, visit http://sharepoint/sites/layering/");

var _taskSecurityConsistencyTest =
    taskGroup("SecurityConsistencyTest", [
        _coreCLRSecurityConsistencyTask("ret", "x86", "mscorlib.dll", [_razzleBuildTask("ret", "x86", "ndp")]),
        _coreCLRSecurityConsistencyTask("ret", "x86", "System.dll", [_razzleBuildTask("ret", "x86", "ndp")]),
        _coreCLRSecurityConsistencyTask("ret", "x86", "System.Core.dll", [_razzleBuildTask("ret", "x86", "ndp")]),
        _coreCLRSecurityConsistencyTask("ret", "x86", "Microsoft.WlcProfile.dll", [_razzleBuildTask("ret", "x86", "ndp")])
    ],
    "Runs a checks over platform assemblies to make sure they are in a good security state.  For details, see http://devdiv/sites/CLR/security/Shared%20Documents/CoreCLRSecurityConsistencyTest.aspx");

var _taskCoreCLRDDRX86 =
    taskGroup("coreCLRdailyDevRunX86", [
        taskNew("sdWhereSynced", "runjs sdWhereSynced %srcBase%\\ndp\\clr\\src %srcBase%", undefined, "local",
                "This task does a sdWhereSynced to log what sync point the run used to build"),

        // This task insures that people's dev directory is up to date
        taskNew("SyncDevDir", "runjs SyncDevDir %srcBase%", undefined, "local",
                "This syncs the vbl\dev\... tree which holds documentation and tools."),

        // FIX, I have put amd64 build first because 64 bit builds update mscoree.h
        // which causes x86 builds to have to start over.  I am working on a real fix
        // however in the mean time do the 64 bit build first so that the x86 build is OK
        _razzleBuildTask("corechk", "amd64", "ndp"),

        _buildWarningTask("corechk", "x86"),
        _scanRuntimeTask("corechk"),

        // X86 Tests
        // GCStress Tests (excluding debugger tests)
        _devBVTTask("corechk", "x86", undefined, "/inc GC_STRESS /exc Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "x86") ]),
        // Do the BVT for X86/Chk (excluding GC_STRESS and debugger tests)
        _devBVTTask("corechk", "x86", undefined, "/exc GC_STRESS;Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "x86") ]),

        // Run the FxCop Tests - the last argument specifies whether these are regular bvts [leave undefined] or fxcop tests.
        // This is so since for FxCOP tests, we have to make smarty not use the CoreCLR loader for testing.
        _devBVTTask("corechk", "x86", undefined, "/inc FXCOP", [ _fxpSetupTask("corechk", "x86") ], "fxcop"),

        // Run the layering tests
        _taskCoreCLRLayeringTest,

        // To add files for file size check, update <enlistment root>\tools\DDR\CoreCLRFileSizeCheck.txt
        _taskCoreCLRFileSizeCheck,

        // Ensure the platform binaries are setup properly for security
        _taskSecurityConsistencyTest,

        // Perform the prefast checks
// Disabled due to incompatablilty with MSBuild.  Will be replaced by OACR ASAP        _prefastBuildTask("corefre")
    ],
    "Runs CoreCLR daily dev run on 32bit machines.");

var _taskCoreCLRDDR64 =
    taskGroup("coreCLRdailyDevRun64", [
        _taskCoreCLRDDRX86,

        // AMD64 Tests
        // GCStress Tests (excluding debugger tests)
        _devBVTTask("corechk", "amd64", undefined, "/inc GC_STRESS /exc Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "amd64") ]),
        // Do the BVT for X86/Chk (excluding GC_STRESS and debugger tests)
        _devBVTTask("corechk", "amd64", undefined, "/exc GC_STRESS;Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "amd64") ]),

    ],
    "Runs CoreCLR daily dev run on 64bit machines.");

var _coreCLRdailyDevRun_buildAll =
    taskGroup("coreCLRdailyDevRun_buildAll", [
        _coreCLRdailyDevRun,
        _razzleBuildTask("coredbg", "x86",   "ndp\\clr"),
        _razzleBuildTask("corefre", "x86",   "ndp\\clr"),
        _razzleBuildTask("corechk", "ia64",  "ndp\\clr"),
        _razzleBuildTask("coreret", "ia64",  "ndp\\clr"),
        _razzleBuildTask("corefre", "ia64",  "ndp\\clr"),
        _razzleBuildTask("corechk", "amd64", "ndp\\clr"),
        _razzleBuildTask("coreret", "amd64", "ndp\\clr"),
        _razzleBuildTask("corefre", "amd64", "ndp\\clr"),
    ]);

var _coreCLRdailyTestRun =
    taskGroup("coreCLRdailyTestRun", [
        taskNew("sdSync", "runjs sdSync %srcBase%", undefined, "local",
                    "This task syncs our source code so that we are building/testing the latest product"),
        taskNew("UpdateBuildVersion", "runjs coreClrUpdateBuildVersion", undefined, "local",
                    "This task updates our version"),
        _buildWarningTask("corechk", "x86"),
        _scanRuntimeTask("corechk"),
        _coreCLRCovBuildTask("corefre", "x86", [_razzleBuildTask("corefre", "x86", "ndp\\clr")]),
        _razzleBuildTask("corechk", "amd64", "ndp\\clr"),
        _razzleBuildTask("corefre", "amd64", "ndp\\clr"),
        _razzleBuildTask("corefre", "ia64", "ndp\\clr"),
        _devBVTTask("corechk", "x86", undefined, undefined, [ _fxpSetupTask("corechk", "x86") ])
    ]);

var _taskIntegrationSniff =
    taskGroup("taskIntegrationSniff", [
              _razzleBuildTask("corechk", "x86", "ndp"),
              _razzleBuildTask("chk", "x86", "ndp"),
              _devBVTTask("chk", "x86"),
              _devBVTTask("corechk", "x86")
              ],
              "Very basic smoke tests to ensure the quality of an integration build."
             );


/****************************************************************************/
var _tasksAll = [
    _taskDailyDevRun,

    _nirdTask("ret", "x86"),

    _WSPerfTestsTask("ret", myArch),

    //
    // Tasks for Snap-Sniff
    //
    setTaskEagerAbort(
        taskGroup("snapSniffTasksWithEagerAbort", [
            _razzleBuildTask("ret", "x86",   "ndp\\clr"),
            _razzleBuildTask("chk", "ia64",  "ndp\\clr"),
//          _prefastBuildTask(),
            _scanRuntimeTask(),
//          _buildWarningTask("chk", "x86"),
//          taskGroup("devBVT", [
//              _devBVTTask("chk", "x86", undefined, "/inc CHECKINBVT /exc NOSNAP /clean /workers:2"),
//              _scanRuntimeTask(),
//          ]),
        ])
    ),

    taskGroup("64bitClrBuilds", [
        _razzleBuildTask("chk", "ia64",  "ndp\\clr"),
        _razzleBuildTask("ret", "ia64",  "ndp\\clr"),
        _razzleBuildTask("chk", "amd64", "ndp\\clr"),
        _razzleBuildTask("ret", "amd64", "ndp\\clr")
    ]),
    taskGroup("64bitNdpBuilds", [
        _razzleBuildTask("chk", "ia64",  "ndp"),
        _razzleBuildTask("ret", "ia64",  "ndp"),
        _razzleBuildTask("chk", "amd64", "ndp"),
        _razzleBuildTask("ret", "amd64", "ndp")
    ]),
    taskGroup("x86NdpBuilds", [
        _razzleBuildTask("chk", "x86",  "ndp"),
        _razzleBuildTask("ret", "x86",  "ndp"),
    ]),
    taskGroup("x86ClrBuilds", [
        _razzleBuildTask("chk", "x86",  "ndp\\clr"),
        _razzleBuildTask("ret", "x86",  "ndp\\clr"),
        _razzleBuildTask("dbg", "x86",  "ndp\\clr"),
    ]),
    taskGroup("allClrBuilds", [
        _razzleBuildTask("chk", "x86",  "ndp\\clr"),
        _razzleBuildTask("ret", "x86",  "ndp\\clr"),
        _razzleBuildTask("dbg", "x86",  "ndp\\clr"),

        _razzleBuildTask("chk", "ia64",  "ndp\\clr"),
        _razzleBuildTask("ret", "ia64",  "ndp\\clr"),
        _razzleBuildTask("chk", "amd64", "ndp\\clr"),
        _razzleBuildTask("ret", "amd64", "ndp\\clr")
    ]),
    taskGroup("allCoreClrBuilds", [
        _razzleBuildTask("corechk", "x86",  "ndp\\clr"),
        _razzleBuildTask("corefre", "x86",  "ndp\\clr"),

        _razzleBuildTask("corechk", "ia64",  "ndp\\clr"),
        _razzleBuildTask("corefre", "ia64",  "ndp\\clr"),
        _razzleBuildTask("corechk", "amd64", "ndp\\clr"),
        _razzleBuildTask("corefre", "amd64", "ndp\\clr")
    ]),

    _taskBuildRetailLayouts,

    _taskBuildChkLayouts,

    // This task prepares all builds necessary for a private stress run
    taskGroup("prepPrivateStress", [
        taskGroup("prepPrivateStress_x86chk", [
            _razzleBuildTask("chk", "x86",   "ndp\\clr")
        ]),
        taskGroup("prepPrivateStress_x86ret", [
            _razzleBuildTask("ret", "x86", "ndp\\clr")
        ]),
        taskGroup("prepPrivateStress_amd64chk", [
            _razzleBuildTask("chk", "amd64", "ndp\\clr")
        ]),
        taskGroup("prepPrivateStress_amd64ret", [
            _razzleBuildTask("ret", "amd64", "ndp\\clr")
        ]),
        taskGroup("prepPrivateStress_ia64chk", [
            _razzleBuildTask("chk", "ia64",  "ndp\\clr")
        ]),
        taskGroup("prepPrivateStress_ia64ret", [
            _razzleBuildTask("ret", "ia64",  "ndp\\clr")
        ])
    ]),

    // This is the task prepares an integration
    // It does everything up to manual resolutions (sync, integrate, resolve)
    taskGroup("clrIntegrateWsfFI", [
        _intgNotificationTask("clrIntegrateWsfFI.Notification", "clrIntegrateWsfFI complete", [
            _integrateWsfTask("FI", "lab21", "lab21s"),
        ]),
    ]),
    taskGroup("clrIntegrateWsfRI", [
        _intgNotificationTask("clrIntegrateWsfRI.Notification", "clrIntegrateWsfRI complete", [
            _integrateWsfTask("RI", "lab21s", "lab21"),
        ]),
    ]),
    taskGroup("FI21Spreparation", [
        _prepIntgTask("FI", "lab21", "private/lab21s"),
        _intgNotificationTask("FI21SprepNotification", "Lab21S FI preparation complete"),
    ]),
    taskGroup("RI21preparation", [
        _prepIntgTask("RI", "private/lab21s", "lab21"),
        _intgNotificationTask("RI21prepNotification", "Lab21 RI preparation complete"),
    ]),

    // This is the task that certifies a forward integration
    taskGroup("FItests", [
        taskGroup("FIdevBVT", [
            // failfast is set to 10000 so that all tests will run regardless of the number of failures.
            _devBVTTask("chk", "x86", undefined, "/clean /failfast:10000 /workers:2" ),
            _buildWarningTask("chk", "x86"),
            _scanRuntimeTask(),
            _intgNotificationTask("FIdevBvtNotification", "FI dev BVT complete"),
        ]),
        taskGroup("FI.test.ddIntFx", [
            _ddIntTask("chk", "x86"),
        ]),
        taskGroup("FI.test.ddIntFxCop", [
            _ddIntTask("chk", "x86", undefined, "src\\fx\\bcl", "-g fxcop",
                [_clrSetupTask("chk", "x86", undefined, "/fx /nrad")]),
        ]),
        taskGroup("FIbuilds", [
            _razzleBuildTask("ret", "x86",   "ndp"),
            _razzleBuildTask("fre", "ia64",  "ndp"),
            _razzleBuildTask("fre", "amd64", "ndp"),
// Disabled due to incompatablilty with MSBuild.  Will be replaced by OACR ASAP            _prefastBuildTask(),
            _intgNotificationTask("FIbuildNotification", "FI builds complete"),
        ]),
        //DDInt FX Cop tests on ret build
        taskGroup("FI.test.ddIntFxCopRet", [
            _ddIntTask("ret", "x86", undefined, "src\\fx\\bcl", "-g fxcop",
                [_clrSetupTask("ret", "x86", undefined, "/fx /rad")]),
            _ddIntTask("ret", "x86", undefined, "src\\fx\\bcl", "-g fxcop",
                [_clrSetupTask("ret", "x86", undefined, "/fx /nrad")]),
        ]),

        _intgNotificationTask("FItestsNotification", "FI tests complete"),
    ]),
    taskGroup("RIx86chkTests", [
            // failfast is set to 10000 so that all tests will run regardless of the number of failures.
        _devBVTTask("chk", "x86", undefined, "/clean /failfast:10000 /inc checkinBVT"),
        _scanRuntimeTask(),
        _intgNotificationTask("RIdevBvtNotification", "RI devBVT complete"),
    ]),
    taskGroup("RIbuilds", [
        _razzleBuildTask("fre", "ia64",  "ndp"),
        _intgNotificationTask("RIbuildNotification", "RI builds complete"),
    ]),
    taskGroup("RIx86freTests", [
        _ddIntTask("fre", "x86", undefined, "src\\fx", "-st -g ddint"),
        _intgNotificationTask("RIddintNotification", "RI ddint complete"),
    ]),
/*              need to debug these before turning them on
    taskGroup("RIvsX86freTests", [
        _razzleBuildTask("fre", "x86",  "ndp"),
        _ddIntTask("fre", "x86", undefined, "src\\vs\\vb\\vbc\\codegen", ""),
        _ddIntTask("fre", "x86", undefined, "src\\vs\\vb\\vbc\\metadata", ""),
        _ddIntTask("fre", "x86", undefined, "src\\vs\\vb\\vbcrun\\latebound", ""),
        _ddIntTask("fre", "x86", undefined, "src\\vs\\vb\\vbcrun\\object", ""),
        _intgNotificationTask("RIvsTrunNotification", "RI vs trun complete"),
    ]),
*/
    // This is the task that runs a full stack build
    // *** timeout is set to 18 hours! ***
    taskGroup("fullStackBuild", [
        _razzleBuildTask("chk", "x86", ".", "razzleBuildDefaultBinDir", 18*60*60),
    ]),

    taskGroup("ddIntFx", [
        _ddIntTask("chk", "x86"),
    ]),

    // Configure IE to get around most ddInt IE setup issues.
    // DDR will do this for you automatically
    taskGroup("ddIntIESetup", [
        _ddIntIESetupTask(),
    ]),

    // install NDP
    // install VS to <source drive>:\VS
    // register VS
    // run VS suites
    taskGroup("ddIntVS", [
        _ddIntTask("chk", "x86", undefined, "src\\vs", undefined, [
            taskGroup("installVs", [
                _clrInstallVSTask("chk", "x86"),
            ]),
        ]),
    ]),

    _taskDailyDevRunFull64,
    //
    // CoreCLR daily dev run
    //
    _taskCoreCLRDDRX86,
    _taskCoreCLRDDR64,

    //
    // CoreCLR daily dev run
    //
    _coreCLRdailyDevRun,

    _coreCLRdailyDevRun_buildAll,

    //
    // CoreCLR daily test run
    //
    _coreCLRdailyTestRun,

    //
    // Integration check task
    //
    _taskIntegrationSniff,

    _taskQuickDevRuns,

    //
    // SoC DDR task
    //
    _taskDailyDevRunSOC,

    //
    // Useful non-DDR SoC tasks
    //
    _taskNonDailyDevRunSOC,

    //
    // Rolling clrsetup
    //
    _taskClrSetup,

];

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

// Add these tasks here, now that they can get the right dependencies
if (!nsIsNullOrEmpty(_getBeagleIP())) {
    _taskAdd(_beagleBVTTask("corechk"));
    _taskAdd(_beagleBVTTask("corefre"));
    _taskAdd(_beagleBVTTask("coredbg"));

    _taskAdd(_beagleEHBVTTask("corechk"));
    _taskAdd(_beagleEHBVTTask("corefre"));
    _taskAdd(_beagleEHBVTTask("coredbg"));
}

/******/
/* Returns the machine pattern to run a given architecture. */
function _machPatForArch(bldArch) {
    if (bldArch.match(/x86/))
        machPat = /(x86)|(amd64)/;
    else
        machPat = bldArch;
    return machPat;
}

/****************************************************************************/
/* Create a task that creates a coverage build from the target build.

  binaries go to %outDir%\<bldArch>cov\bin
  coverage binaries will be based on binaries found at %outDir%\<bldArch><srcType>\bin

  Parameters
    srcType     : The build type of the source binaries (chk, dbg, ret ...)
    bldArch     : The build architecture (x86, amd64, ia64)
*/
function _coreCLRCovBuildTask(srcType, bldArch, dependencies) {
    if (srcType == undefined)
        throw Error(1, "Arg srcType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    var relOutDir = bldArch + srcType;
    var srcDir = "%outDir%\\" + relOutDir + "\\bin";
    var tgtDir = "%outDir%\\" + bldArch + "cov\\bin";

//    var ver = "%outDir%".substr("%outdir%".lastIndexOf("run.") + 4,16);

    var taskName = "CovBuild." + bldArch + "cov";
    var ret = taskNew(taskName,
                    "call cover.bat " + srcDir + " "
                                      + tgtDir,
                    dependencies,
                    /(x86)|(amd64)/i);
    ret.description = "A coverage build accomplished by copying another build and instrumenting key CoreCLR assemblies.";
    return ret;
}

/****************************************************************************/
/* Create a task that does a razzle build.

  logs go to %outDir%\<bldArch><bldType>
  binaries go to %outDir%\<bldArch><bldType>\bin

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
                  This also can contain an optional suffix such as ".unopt"
    bldArch     : The build architecture (x86, amd64 ia64)
    bldRelDir   : The directory relative to %srcBase% to build in
    binDir      : The directory to place the built binaries in,
                  (defaults to "%outDir%\\" + bldType + bldArch + "\\bin")
    timeOut     : Set the timeout for the build, defaults to 90 minutes
    bldArgs     : additional parameters to pass to build (like -cC)
*/
function _razzleBuildTask(bldType, bldArch, bldRelDir, bldArgs, timeOut, binDir, dependents) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (bldRelDir == undefined)
        bldRelDir = "ndp\\clr";

    // Setup baseBldType by removing any suffix that may have been attached to bldType
    var baseBldType = bldType;
    if (baseBldType.match(/(.+)\.(.+)$/)) {
        baseBldType = RegExp.$1;
    }

    var relOutDir = bldArch + bldType;

    var taskName = "razzleBuild." + bldRelDir.replace(/[\\]/g, "-") + "@" + relOutDir;

    var logDir = "%outDir%\\" + relOutDir;

    if (binDir == undefined)
        binDir = logDir + "\\bin";

    if (timeOut == undefined)
        timeOut = 120 * MINUTE;

    if (bldArgs == undefined)
        bldArgs = "-cC";
    if (bldArgs == "")
        bldArgs = "\"\"";

    // Cannot perform build unless the shelveset is unshelved
    if(runNird == 1)
        if(dependents == undefined)
            dependents = [_unshelveTask("%shelveset%")];
        else
            dependents = dependents.concat(_unshelveTask("%shelveset%"));

    var ret = taskNew(taskName,
                    "runjs razzleBuild " + '\"' + baseBldType + '\"' + " "
                                         + '\"' + bldArch + '\"' + " "
                                         + '\"' + bldRelDir + '\"' + " "
                                         + '\"' + bldArgs + '\"' + " "
                                         + "\"%srcBase%\" "
                                         + '\"' + binDir + '\"' + " "
                                         + '\"' + logDir + '\"' + " "
                                         + '\"' + timeOut + '\"',
                                         dependents,
                                         /(x86)|(amd64)/i);

    ret.description = "A normal (razzle) build of some directory in the source base.";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Running%20Tests.aspx#_Toc117485070";
    return ret;
}

/****************************************************************************/
/* Do a scanRuntime against the CLR

  logs go to %outDir%\x86chk
*/
function _scanRuntimeTask(bldType, bldRelDir) {

    if (bldType == undefined) {
        bldType = "chk";
    }
    var relOutDir = "x86" + bldType;
    var taskName = "scanRuntime@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir;
    var binDir = logDir + "\\bin";

    var ret = taskNew(taskName,
        "runjs scanRuntime " + binDir + " "
                             + logDir + "\\scan.report.log "
                             + "%srcBase% ",
        [_razzleBuildTask(bldType, "x86", bldRelDir)],
        /(x86)|(amd64)/i);

    ret.description = "Run scanRuntime against the prebuilt x86" + bldType + " binaries.\r\n";
    return ret;
}

/****************************************************************************/
/* check our build warnings against the current baseline
*/
function _buildWarningTask(bldType, bldArch) {

    var relOutDir = bldArch + bldType;

    var taskName = "buildWarningTask" + "@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir;

    var bldWrnFileName = logDir + "\\razzleBuild.ndp." + bldArch + bldType + ".wrn";
    var baselineFileName = "%srcBase%\\ndp\\clr\\snap2.4\\tasks\\build\\baseline_warnings\\" + relOutDir + ".wrn";

    var ret = taskNew(taskName,
            "runjs buildWarningDiff " + bldWrnFileName + " " + baselineFileName,
            [_razzleBuildTask(bldType, bldArch, "ndp")],
            /(x86)|(amd64)/i);

    ret.description = "Checks for new build errors (above our baseline)\r\n";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Running%20Tests.aspx#_Toc117485071";
    return ret;
}

/****************************************************************************/
/* Do a prefastBuild of the CLR

  logs go to %outDir%\x86fre.prefast
*/
function _prefastBuildTask(bldType) {

    if (bldType == undefined) {
        bldType = "fre";
    }

    var bldRelDir = "ndp\\clr\\src";
    var relOutDir = "x86" + bldType + ".prefast"
    var taskName = "prefastBuild@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir;

    // IntegerOverflow codes 22011-22019 are SWI min bar required.
//    var plugins="BigFunc;IntegerOverflow"; // UNCOMMENT
    var plugins="_";                       // DELETE LINE
    if (isCoreCLRBuild(bldType)) {
//        plugins = plugins + ";msrc";  // UNCOMMENT
        plugins = "msrc";             // DELETE LINE
    }

    // Cannot perform build unless the shelveset is unshelved
    if(runNird == 1)
        dependents = [_unshelveTask("%shelveset%")];
    else
        dependents = [];

    var ret = taskNew(taskName,
              "runjs prefastBuild " + bldType + " "
                                    + bldRelDir + " "
                                    + "%srcBase% "
                                    + logDir + " "
                                    + "_ "
                                    + "_ "
                                    + plugins + " ",
              dependents,
              /(x86)|(amd64)/i);

    ret.description = "Run the PREFAST on the runtime sources. The prefast tool is designed to parse the code\r\n";
                      "as the normal compiler would and do an analysis, looking for common programming mistakes.\r\n";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Running%20Tests.aspx#_Toc117485074";
    return ret;
}

/*
 Task for the DacCop prefast analysis.  This builds all directories used to create mscordacwks.dll
 with the DacCop prefast plugin active.
 Note that this task expects that you have a clean x86chk CLR build already.  It builds only the directories
 needed for the analysis.  When used as part of DDR, an x86 clean build task should be a dependant.
 However, we don't want to make it a dependant here because that would cause a full clean build every
 time you wanted to re-run the analysis.  Instead we'll leave it up to our caller to ensure we can build.
 If this poses a problem, we could try and identify the dependencies (like src/inc) and build them in advance
 unless an incremental option is supplied.
*/
function _taskDacCop(bldType, bldArch, incBuild) {

    if (incBuild == undefined)
        incBuild = false;
    if (incBuild == "true" || incBuild == "incremental")
        incBuild = true;

    // Generate a task name.  We want different options (eg. inc) to result in different task names
    // so that we can identify which behavior we want using just a task name (eg. like _clrSetupTask).
    // note that other parts of the task infrastructure expect that the task name will be <name>@<relOutDir>
    // and we don't want to use the same relOutDir as a normal x86chk build, hence the duplicate "dacCop" in
    // the full task name.  This is similar to tasks like prefastBuild and rotorBuild.

    var relOutDir = bldArch + bldType + ".dacCop";
    var taskName = "dacCop";
    if (incBuild)
        taskName += "-inc";
    taskName += "@" + relOutDir;

    var logDir = "%outDir%\\" + relOutDir;
    var srcDir = "%srcBase%\\ndp\\clr\\src";

    // We want to do a build in every directory that goes into building mscordacwks.dll.
    // But mscordacwks.dll is built from a bunch of static libs
    //  From DLLS\mscordac\sources.inc, we get the following list (as of 9/14/06)
    var subDirs = [
        "vm\\dacwks",               // cee_wks_dac.lib
        "utilcode\\dac",            // utilcode_dac.lib
        "debug\\daccess",           // dac_wks.lib.
        "debug\\EE\\dacwks",        // cordbee_wks_dac.lib
        "IPCMan\\ipcman_dac" ];     // ipcmanager_dac.lib
        /*  We actually don't want to include metadata at the moment.  It's not dacized
            and so completely a host utility (with just a few functions stubbed out for some reason).
        "MD\\runtime\\dac",         // mdruntimerw_dac.lib
        "MD\\compiler\\DAC",        // mdruntime_dac.lib
        "MD\\enc\\dac" ];           // mdcompiler_dac.lib
        */

    for( var i in subDirs )
        subDirs[i] = "%srcBase%\\ndp\\clr\\src\\" + subDirs[i];
    var srcDirs = subDirs.join(";");

    // Unless our caller has asked for an incremental build, we need to do a clean build to ensure
    // any dependencies are up to date.  Normally this would happen in DDR in which case we should
    // already be doing a clean build, so this won't add any extra time.
    var dependents = undefined;
    if (!incBuild)
        dependents = [ _razzleBuildTask(bldType, bldArch, "ndp") ];

    return taskNew(
        taskName,
        "runjs prefastPluginBuild DacCop " + srcDirs + " " + logDir + " " + bldType + " " + bldArch,
        dependents,
        /(x86)|(amd64)/i,  // machine pattern
        "Run the DacCop prefast plugin and analysis to identify possible bugs in the usage of DAC.",
        "http://mswikis/clr/dev/Pages/DacCop.aspx" );
}

// Add some standard DacCop tasks to the lists of all tasks, so that, eg. "runjs doRun dacCop@x86chk.dacCop" will work
// Note that only the x86chk task is kept clean against the exclusion list, so others will likely report violations.
_taskAdd( _taskDacCop("chk", "x86") );              //dacCop@x86chk.dacCop

// Convenience function for running an incremental DacCop task.
// Note that this requires an x86chk clean build to have already been done.
function DacCop()
{
    doRun( "dacCop-inc@x86chk.dacCop" );
}

/****************************************************************************/
/* Do a full retail Build of the CLR.

    bldType     : The build type (ret, chk, dbg)
                  This also can contain an optional suffix such as ".opt"
    bldArch     : The build architecture (x86, amd64, ia64)
    neutralArch : the architecture {x86 or self} to fetch the arch neutral managed assemblies
                  This arg is used to simulate layout installs for 64-bit platforms
    relInBin    : If 'relInBin' is present, we get our its bits from there,
                  If undefined, we build the unoptimized bits.
    dependents  : Array of dependent tasks

*/
function _retailBuildTask(bldType, bldArch, neutralArch, relInBin, dependents) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    // Setup baseBldType by removing any suffix that may have been attached to bldType
    var baseBldType = bldType;
    if (bldType.match(/(.+)\.(.+)$/)) {
        baseBldType = RegExp.$1;
    }

    // Setup  razzleBldType and bldType:
    //
    // Requested
    // bldType(In)  => baseBldType  bldType(Out)  razzleBuildType
    // ----------------------------------------------------------
    //  ret         =>    ret           ret          ret.unopt
    //  chk         =>    chk         chk.opt          chk
    //  chk.opt     =>    chk         chk.opt          chk
    //
    //  ret.alt     =>    ret         ret.alt        ret.unopt
    //  chk.alt     =>    chk         chk.alt          chk
    //
    var razzleBldType;
    if (baseBldType == "ret") {
        razzleBldType = baseBldType + ".unopt";
    }
    else {
        if (baseBldType == bldType) {
            bldType = baseBldType + ".opt";
        }
    }

    var relInDir  = bldArch + razzleBldType;

    if (relInBin == undefined) {
        relInBin = "%outDir%\\" + relInDir + "\\bin";
    }

    // Setup our dependent tasks if the caller did not supply us with anything

    //
        if (dependents == undefined) {
            dependents = [];

        // if neutralArch is set then we do a full retailBuild for the neutralArch
        // This allows us to use the IBC optimize versions of the neutralArch assemblies
        //
        if (neutralArch != undefined) {
            dependents.push(_retailBuildTask(bldType, neutralArch));
        }

        // We also require a razzle build task to build the unoptimized bits
        dependents.push(_razzleBuildTask(razzleBldType, bldArch, "ndp", undefined, undefined, relInBin));
    }

    var relOutDir    = bldArch + bldType;
    var taskName     = "retailBuild@" + relOutDir;
    var relOutBin    = "%outDir%\\" + relOutDir + "\\bin";
    var logDir       = "%outDir%\\" + relOutDir;
    var neutralInBin = "_ ";
    if (neutralArch != undefined) {
        neutralInBin = "%outDir%\\" + neutralArch + bldType + "\\bin";
    }

    var ret = taskNew(taskName,
                      "runjs retailBuild "
                           + relInBin + " "
                           + relOutBin + " "
                           + bldArch + " "
                           + "_ "                  // scenarioSpec default scenarios
                           + logDir + " "
                           + "%srcBase% "
                           + neutralInBin,
                      dependents,
                      /(x86)|(amd64)/i);

    ret.description = "Create a version of the runtime that has had its code arranged for\r\n"+
                      "optimal working set performance.\r\n";
    return ret;


}

/****************************************************************************/
/* run the performance tests.
   If dependents is undefined it does a retailBuild for 'bldType' and 'bldArch'
   if dependents is defined it assumes that bldType and bldArch already
   has been built and that binDir contains the binaries to use.
   We then install those bits and run the working-set performance tests
*/
function _WSPerfTestsTask(bldType, bldArch, binDir, neutralArch, dependents) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    var relDir = bldArch + bldType;
    var taskName = "WSPerfTests@" + relDir;
    var logDir = "%outDir%\\" + relDir + "\\WSPerfTests";

    if (binDir == undefined) {
        binDir = "%outDir%\\" + relDir + "\\bin";
    }

    if (dependents == undefined) {
        dependents = [];
        dependents.push(_retailBuildTask(bldType, bldArch, neutralArch));
    }
    //
    // At this point binDir contains the built binaries to use
    //
    var setupDependents = dependents;

    perfDependents = [];
    perfDependents.push(_clrSetupTask(bldType, bldArch, binDir, "/fx /rad",
                                  setupDependents));

    var cmd = "runjs ";

    // If we are running WSPerf tests for the x86 arch on a 64-bit machine
    // then we use runX86 to setup a 32-bit Wow environment for WSPerfTests
    //
    if (NeedToEnterWow64(bldArch)) {
        cmd += "runX86 ";
    }

    var ret = taskNew(taskName,
                        cmd +
                        "WSPerfTests "
                         + logDir + " "
                         + "_ "
                         + "%srcBase% "
                         + bldArch,
                      perfDependents);

    ret.description = "Run the working set performance tests.\r\n" +
                      "This task assumes that the runtime has already been installed.\r\n";
    return ret;
}

/****************************************************************************/
/* Build NDP layouts

Parameters
sku         : The SKU to build (client, full)
bldArch     : The build architecture (x86, amd64 ia64)
bldType     : The build type (chk, dbg, ret ...)
binDir      : The directory to get the runtime binaries from
setupRoot   : The directory where layouts will be located
*/
function _buildLayoutsTask(sku, bldArch, bldType, binDir, setupRoot, dependents) {
    if (sku == undefined)
        throw Error(1, "Arg sku not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");

    var relOutDir = bldArch + bldType;
    var taskName = "buildLayouts." + sku + "@" + relOutDir;

    var logDir = "%outDir%\\" + relOutDir;

    if (dependents == undefined) {
        dependents = [];
    }

    if (binDir == undefined) {
        binDir = logDir + "\\bin";
        dependents.push(_razzleBuildTask(bldType, bldArch, "ndp\\clr"));
    }

    // buildLayouts(sku, architecture, type, binDir, remoteStore, setupRoot, failOnPause, skipFileCheck, verbose)
    var taskCommand = "runjs buildLayouts " + sku + " " + bldArch + " " + bldType + " " + binDir + " _ " /*remoteStore*/ + setupRoot + " true";

    // Building Layouts doesn't seem to work with UNC paths, so enforcing these on the local box.
    var buildTask = taskNew(taskName, taskCommand, dependents, "local");
    buildTask.description = "The buildLayouts task will build box layouts for the specified SKU, architecture, and type.";
    
    var cleanupTask = taskNew("cleanLayouts." + sku + "@" + relOutDir, "runjs cleanLayouts binaries " + bldType + " " + setupRoot, [buildTask], "local");
    cleanupTask.description = "Cleans up intermediate files and directories created by buildLayouts that take up space and don't add value.";

    return cleanupTask;
}

/****************************************************************************/
/* Create arm setup scripts using PrepArmSetup
*/
function _prepArmSetupTask(bldType) {
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    var ret = taskNew("prepArmSetup@arm" + bldType,
        "runjs prepArmSetup %outDir%\\arm" + bldType + "\\bin C:\\Windows arm" + bldType,
        undefined,
        /(x86)|(amd64)/);

    ret.description = "Preprocessing of setup scripts on an x86|amd64 box for arm.";
    return ret;
}


/****************************************************************************/
/* Install the runtime using CLRSetup

logs go to %outDir%\<bldArch><bldType>

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    bldArch     : The build architecture (x86, amd64 ia64)
    binDir      : The directory to get the runtime binaries from
    setupArgs   : additional parameters to pass to clrSetup
    dependents  : Any depenedents you want for this task (like a build)
    ignoreSetupCache : Flag whether to use clrSetupCache
*/

function _clrSetupTask(bldType, bldArch, binDir, setupArgs, dependents, verstr, ignoreSetupCache) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (setupArgs == undefined)
        setupArgs = "";
    if (verstr == undefined)
        verstr = "_";

    // no layouts support for ARM at this time
    var useLayouts = true;
    if (bldArch.match(/arm/i))
        useLayouts = false;

    var relOutDir = bldArch + bldType;
    var taskName = "clrSetup";

    if (!useLayouts) {
        if (setupArgs != "")
            taskName += "." + setupArgs.replace(/(\s|\\)*/g, "").replace(/\//g, "-");
    }
    else {
        taskName += ".layouts";
    }
    taskName += "@" + relOutDir;

    var logDir = "%outDir%\\" + relOutDir;

    if (dependents == undefined) {
        dependents = [];
    }

    if (useLayouts) {
        // there are a lot of arguments to ndpsetup.js that are not supported with the new installer.
        // for now we just whack any passed in args here since most of them are not needed.
        // TODO: fix the the correct way
        setupArgs = "/nrad";
    }

    if (binDir == undefined) {
        binDir = logDir + "\\bin";
        if (useLayouts) {
            dependents.push(_buildLayoutsTask("full", bldArch, bldType, binDir, "%outDir%\\setup", [_razzleBuildTask(bldType, bldArch, "ndp")]));
            ignoreSetupCache = true;
            useLayouts = false; // to avoid adding the task again in the case where binDir was provided
        }
        else {
            var bldTask = _razzleBuildTask(bldType, bldArch, "ndp");

            if (bldArch == "arm") {
                var prepArmSetupTask = _prepArmSetupTask(bldType);
                prepArmSetupTask.dependents = [bldTask];
                dependents.push(prepArmSetupTask);
            } else {
                dependents.push(bldTask);
            }
        }
    }

    if (useLayouts) {
        // any passed in dependents (ie build task) should be rewired such that build layouts task
        // is dependent on them (we don't want this task to be a peer with building the bits)
        dependents = [_buildLayoutsTask("full", bldArch, bldType, binDir, "%outDir%\\setup", dependents)];
        ignoreSetupCache = true;
    }
    else {
        var clrSetupTaskArgs = Env("clrSetupTaskArgs");
        if (clrSetupTaskArgs != undefined) {
            setupArgs = setupArgs + " " + clrSetupTaskArgs;
        }

        if (setupArgs == "" || setupArgs.match(/ /))
            setupArgs = "\"" + setupArgs + "\"";
    }

    binDir = "\"" + binDir + "\"";
    logDir = "\"" + logDir + "\"";

    var taskCommand;
    if (bldArch == "arm") {
        taskCommand = "%outDir%\\arm" + bldType + "\\bin\\ts.bat";
    }
    else {
        // if ignoreSetupCache is set, use clrSetup commmand, otherwise use clrSetupWithCache
        if (ignoreSetupCache)
            taskCommand = "runjs clrSetup " + binDir + " "
            + setupArgs + " "
            + logDir + " "
            + verstr;
        else
            taskCommand = "runjs clrSetupWithCache " + binDir + " "
            + setupArgs + " "
            + logDir + " "
            + "%srcBase%\\binaries\\clrSetupCache "
            + verstr;
    }

    var ret = taskNew(taskName, taskCommand, dependents, _machPatForArch(bldArch));

    ret.description = "The clrsetup task is responsible for installing given set of CLR binaries on the current machine.";
    return ret;
}

/****************************************************************************/
function getRuntimeVersionDir(verStr) {
    /*** FIX enable this.
        // Currently we always do our run in this priviate '.runjs' runtime of our own
        // this insures that we are not impacting other testing going on on the machine
    var verStr = relOutDir + ".runjs";
    ***/

    var verDir = "v4.0." + verStr;                          // Yuck!
    return verDir;
}


/****************************************************************************/
/* Run CLRSTRESS test

  any logs go to %outDir%\<bldArch><bldType>\test.devBVT

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    bldArch     : The build architecture (x86, amd64 ia64)
    timeToRunMin: Minutes, from 30 to 900 (default 30)
    binDir      : The directory to get the runtime binaries from
    dependencies: Any depenedents you want for this task (like a build)
*/

function _clrStressTask(bldType, bldArch, timeToRunMin, binDir, dependencies) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (timeToRunMin == undefined)
        timeToRunMin = 30;

    var relOutDir = bldArch + bldType;
    var taskName = "test.clrStress" +  "@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir + "\\test.clrStress";

    var verStr = relOutDir;

    var verDir = getRuntimeVersionDir(verStr);

    if (dependencies == undefined)
        dependencies = [_clrSetupTask(bldType, bldArch, binDir, "/fx /nrad", undefined, verStr)];

    var ret = taskNew(taskName,
                    "runjs clrStress "
                    + timeToRunMin + " "
                    + logDir + " "
                    + "%srcBase%\\ddsuites\\src\\clr\\stress\\" + bldArch + " "
                    + bldArch + " "
                    + verDir,
        dependencies,
        _machPatForArch(bldArch));

    ret.description = "Run the CLRSTRESS test.\r\n" +
                      "This task assumes that the runtime has already been installed.\r\n";
    return ret;
}

function _verifyMachineStateTask(bldArch, bldType) {
    /* BUGBUG Fri 8/8/2008
     * In the future modify this function to take a ddsdir if we ever check this stuff into the live drop.
     */
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    //Don't do this for coreclr.
    if (isCoreCLRBuild(bldType))
        return undefined;

    var ddsuitesRoot = Env("_NTDRIVE") + Env("_NTROOT") + "\\ddsuites";
    var ddsuitesDir = ddsuitesRoot + "\\src\\clr\\" + bldArch;
    var smartyArgs = "/lst " + ddsuitesDir + "\\IVT.lst " +
                     "/inc Infrastructure\\private\\ddr " +
                     "/noie";
    return taskNew("test.devBVT.VerifyMachineState@" + bldArch + bldType,
                   "runjs verifyMachineState " + bldArch + " " + bldType + " _ _ " +
                        "%outDir%\\" + bldArch + bldType + "\\test.devBVT.VerifyMachineState " +
                        //Do not abort all of DDR for a failure right now.  It'd be nice to have more
                        //confidence that it works at all.  Also, the current abort logic sort of stinks.
                        //"%outDir%\\abort.txt",
                        "",
                   undefined, //dependencies
                   _machPatForArch(bldArch), //machPat
                   "Verifies that your machine is in a good state to run tests.",
                   undefined // No more info url
                  );

}

/****************************************************************************/
/* Runtests using devBvt (smarty)

  smarty logs go to %outDir%\<bldArch><bldType>\test.devBVT.<Catagory>

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    bldArch     : The build architecture (x86, amd64 ia64)
    binDir      : The directory to get the runtime binaries from
    smartyArgs  : additional parameters to pass to devBvt
    dependencies: Any depenedents you want for this task (like a build)
    testtype    : Type of tests ("fxcop" for FxCOP testing; undefined for everything else)
*/

function _devBVTTask(bldType, bldArch, binDir, smartyArgs, dependencies, testtype) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (smartyArgs == undefined)
        smartyArgs = "";
    var clr;
    if (isCoreCLRBuild(bldType)) {
        clr = "coreclr";
    } else {
        clr = "clr";
    }

    var relOutDir = bldArch + bldType;

    var categories = "";
    if (smartyArgs.match(/\/inc +(\S+)/i))
        categories = "." + RegExp.$1;
    else if(smartyArgs.match(/\/gcstress +(\S+)/i))
    {
        categories = ".GC_STRESS";
    }
    else if(smartyArgs.match(/\/altjit/i))
    {
        categories = ".AltJit";
    }
    categories = categories.replace(/\\/g, "_"); // for sub-category names like "combined\devsvcs\debugger"

    var taskName = "test.devBVT" + categories + "@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir + "\\test.devBVT" + categories;

    if (bldArch != "arm")
    {
        smartyArgs += " /clean";
    }

    smartyArgs += " /workers:*";

    var extRootOverride;

    if (clr == "coreclr")
    {
        extRootOverride = "%outDir%\\" + bldArch + bldType + "\\CoreCLR\\v2.0." + bldArch + bldType;
        if (testtype != "fxcop") {
            smartyArgs += " /ldr " + extRootOverride + "\\fxprun.exe";
        }
    }

    var verStr = relOutDir;
    var verDir = getRuntimeVersionDir(verStr);

    if (dependencies == undefined)
    {
        if (clr == "coreclr") {
            dependencies = [_fxpSetupTask(bldType, bldArch)];
        }
        else 
        {
            dependencies = [_clrSetupTask(bldType, bldArch, binDir, "/fx /nrad", undefined, verStr)];
        }
    }

    if (clr == "clr")
        smartyArgs += " /fileBugs /baselineSrc:%srcBase% %extraSmartyArgs%"

    var ret = taskNew(taskName,
        "runjs devBVT \"" + smartyArgs + "\" "
                      + logDir + " "
                      + "%srcBase%\\ddsuites "
                      + bldType + " "
                      + bldArch + " "
                      + verDir + " "
                      + extRootOverride + " "
                      + testtype + " "
                      + "100",
        dependencies,
        _machPatForArch(bldArch));

    ret.description = "Run the group of tests that developers are required to run before checkin (the devBVTs).\r\n" +
                      "This task assumes that the runtime has already been installed.\r\n";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Running%20Tests.aspx#_Toc117485072";
    return ret;
}

/****************************************************************************/
/* Run ddSuites.ddInt tests using trun. By default this task will include the
   stable subset of DDInt tests that are expected to pass in DDR.

  Parameters
    bldType     : <required> The build type (chk, dbg, ret ...)
    bldArch     : <required> The build architecture (x86, amd64 ia64)
    binDir      : <optional> The directory to get the runtime binaries from
    trunStartDir: <optional> Relative path from ddsuites to start trun.
    trunArgs    : <optional> Additional args to trun (-g DDInt etc).
    dependencies: <optional> Any depenedents you want for this task (like a build)
    suiteBinDir : (optional) If suitebin is not under binDir. Default is binDir\\suitebin
    description : <optional> A description for the task.
    moreInfoUrl : <optional> A URL to more information about this batch of tests
*/

function _ddIntTask(bldType, bldArch, binDir, trunStartDir, trunArgs, dependencies, suiteBinDir, description, moreInfoUrl) {
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    bldType = bldType.toLowerCase();
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (description == undefined)
        description = "DDInts are a group of tests maintained by the division.  The division-wide trun is used to run these tests. ";
    if (moreInfoUrl == undefined)
        moreInfoUrl = "";
    var clr;
    if (isCoreCLRBuild(bldType)) {
        clr = "coreclr";
    } else {
        clr = "clr";
    }

    // By default run DDIntFx suites
    if (trunStartDir == undefined)
        trunStartDir = "src\\fx";

    var vsSuites = false;
    if (trunStartDir.match(/src\\vs/i))
        vsSuites = true;

    // Check for WoW. DDInt is supported on 64bit platform only under WoW
    var realArch = getRealProcessorArchitecture().toLowerCase();
    var testArch = bldArch.toLowerCase();

    var trunGroups = "-g bcl -g DDInt -g !DDInt_noWindow -g !noClrDDR";
    if (vsSuites)
        trunGroups = "-g DDIntVS";

    if (bldType == "chk" || bldType == "dbg")
        trunGroups += " -g !DDInt_noCHK";

    if (IsWinLHOrLater())
        trunGroups += " -g !DDInt_NoVista";

    if (testArch != realArch) {

        // WoW only supports running X86 processes
        if (!testArch.match(/x86/i))
            throw Error(1, "Mismatch architectures, want " + testArch + " running on " + realArch);

        trunGroups += " -g !noWOW";
    }

    // Include the stable subset of tests that are expected to pass in DDR
    if (trunArgs == undefined)
        trunArgs = trunGroups;

    // todo: add logDir param instead
    var taskNamePrefix = "test.ddInt." +  trunStartDir.replace(/[\\]/g, "-");

    // REMOVE WHEN TRUN LOG DIRECTORY BUG IS FIXED, expected 9/2005.
    // There is a bug in trun.exe that it interprets any instance of 'bc' in -!summaryLogs as -bc switch
    taskNamePrefix = taskNamePrefix.replace(/bcl/ig, "TRUN_ARG_BUG");

    var relOutDir = bldArch + bldType;
    var logDir = "%outDir%\\" + relOutDir + "\\" + taskNamePrefix;
    var taskName = taskNamePrefix + "@" + relOutDir;

    // leave binDir undefined if running VS suites
    if (binDir == undefined) {
        if (trunStartDir == "src\\vs")
            binDir = "_";
        else
            binDir = "%outDir%\\" + relOutDir + "\\bin";
    }

    if (suiteBinDir == undefined)
        suiteBinDir = "_";

    if (clr != "coreclr")
    {
    if (dependencies == undefined) {
        dependencies = [];
        if (vsSuites) {
            // for vs suites we expect the suite binaries to be copied down with the vs binaries
            // VS install and suite runs on a full stack build should be handled somewhere else
            dependencies.push(_clrInstallVSTask("chk", "x86"))
        }
        else {
            // We pass in "undefined" to the clrsetup task here so that it forces a build.  If we passed the
            // bindir it would assume the build was already done.
            dependencies.push(_clrSetupTask(bldType, bldArch, undefined, "/fx /nrad"));
            dependencies.push(_razzleBuildTask(bldType, bldArch, "ddsuites\\" + trunStartDir));
        }
    }
    }

    var ret = taskNew(taskName,"runjs trun \"" +
                            trunArgs + "\" " +
                            trunStartDir + " " +
                            logDir + " " +
                            "%srcBase%\\ddsuites" + " " +
                            binDir + " " +
                            bldType + " " +
                            bldArch + " " +
                            getRuntimeVersionDir(relOutDir + " " +
                            suiteBinDir),
                    dependencies,
                    _machPatForArch(bldArch));

    ret.description = description;
    ret.moreInfoUrl = moreInfoUrl;
    return ret;
}

/****************************************************************************/
/* Configures IE for running ddInt tests.
*/
function _ddIntIESetupTask() {
    var ret = taskNew("ddIntIESetupTask", "reg import " + ScriptDir + "\\scriptLib\\ddIntIEConfig.reg");
    ret.description = "Configures IE for running ddInt tests";
    return ret;
}

/****************************************************************************/
/* task to prepare an integration up to the manual conflict resolution

   Parameters:
                        intgType - type of integration (RI,FI)
                        fromLab - lab the bits are coming from
                        toLab - lab the bits are being integrated into

   For now these are used to construct the task name

*/

function _prepIntgTask(intgType, fromLab, toLab) {

    var taskName = intgType + ".integration." +
                   fromLab.replace(/[\/]/g, "-") + "." +
                   toLab.replace(/[\/]/g, "-");

    var task = taskNew(taskName, "runjs prepareIntegration %srcBase% "+fromLab+" %fromLabel% "+toLab+" %toLabel%");

//  logMsg(LogTask, LogInfo, "task =  \n", dump(task), "\n\n"); // useful for debugging
    return task;
}

/****************************************************************************/
/* task to send an integration notification

        notification will contain %outputDir%\run.current\taskReport.htm

   Parameters:
        taskName - task being notified about (ergo in subject)
        subject - subject line for email
*/

function _intgNotificationTask(taskName, subject, dependencies) {

    if (taskName == undefined || taskName == "")
        taskName = "task." + currentTimestamp() + ".completion";                  // create unique task name
    if (subject == undefined || subject == "")
        subject = taskName + " Completion Notification";        // generic subject

    var task = taskNew(taskName, "runjs sendMailForIntegration %srcBase% \"" + subject + "\"", dependencies);

    return task;
}

/****************************************************************************/
/* Task to start an integration using tools\devdiv\integrate.wsf
   Uses runjs integrateWsf wrapper to set config file, user,
   requestor, logdir, etc.

   Parameters:
        intgType - FI|RI
        sourceLabIntegrateWsfName - name of source lab, see tools\devdiv\integrationlabdata.js
        targetLabIntegrateWsfName - name of target lab, see tools\devdiv\integrationlabdata.js
        %srctime% - /srctime parameter for integrate.wsf

        runjs doRun clrIntegrateWsfFI _ _ "srctime=2005/05/09:18:32:13"
*/
function _integrateWsfTask(intgType, sourceLabIntegrateWsfName, targetLabIntegrateWsfName, dependencies) {
    var taskName = "integratewsf." + intgType + "." +
                   sourceLabIntegrateWsfName + "." +
                   targetLabIntegrateWsfName;

    if (dependencies == undefined) {
        dependencies = [_takePublicChangeList()];
    }

    var ret = taskNew(
                taskName,
                "runjs integrateWsf "+sourceLabIntegrateWsfName+" "+targetLabIntegrateWsfName+" %srctime%",
                dependencies);


    ret.description = "Run tools\\devdiv\\integrate.wsf which runs SD commands via SDAPI: integrate,\r\n"
                    + "resolve, sync, resolve, etc.  This task will loop until all resolves are complete\r\n"
                    + "and all warnings are verified on the integration web page for the integration\r\n"
                    + "session, http://ddit/vblintegration.";

    return ret;
}

/****************************************************************************/
/* Task for the current user to take ownership of the public CL */
function _takePublicChangeList() {

    var ret = taskNew("takePublicChangeList", "runjs takePublicChangeList");
    ret.description = "Task for the current user to take ownership of the public CL.\r\n";

    return ret;
}

function _clrInstallVSTask(bldType, bldArch, dependencies) {
    var taskName = "installVS." + bldType + "." + bldArch;

    if (dependencies == undefined) {
        dependencies = [_clrSetupTask(bldType, bldArch, undefined, "/fx /rad")];
    }

    var ret = taskNew(
                taskName,
                "cmd /c call ClrInstallVS",
                dependencies,
                "local");

    ret.description = "Install VS using //depot/dev/tools/clr/ClrInstallVS.cmd.\r\n";

    return ret;
}


/****************************************************************************/
/* sync the 'dev' directory of a source base.  This is a directory that lives
   either in srcBase\dev or srcBase\..\dev
*/
function SyncDevDir(srcBase)
{
    if (_inTFS())
    {
        logMsg(LogTask, LogInfo, "Not syncing devDir since this is a TFS enlistment:\n\t devDir is only required for SD enlistments.\n");
        return 0;
    }
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase is missing");
    }

		// Figure out where the dev tree lives
	var devDir = srcBase + "\\dev";
	if (!FSOFolderExists(devDir))
		devDir = srcBase + "\\..\\dev";

	if (!FSOFolderExists(devDir))
	{
		logMsg(LogTask, LogError, "Could not find devDir ", devDir, "\n");
		return 1;
	}
	logMsg(LogTask, LogInfo, "Syncing devDir ", devDir, "\n");

	sdSync(devDir);
	return 0
}

/****************************************************************************/
/* Run filesize check test on the CoreCLR DLLs

  Parameters
    bldType         : <required> The build type (chk, dbg, ret ...)
    bldArch         : <required> The build architecture (x86, amd64 ia64)
    dependencies    : <optional> Any depenedents you want for this task (like a build)
*/

function _coreCLRFileSizeCheckTask(bldType, bldArch, dependencies)
{
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    // Pato the file containing size check details
    var pathBaseLineFile = "%srcBase%\\tools\\DDR\\CoreCLRFileSizeCheck.txt";

    var taskName = "CoreCLRFileSizeCheck.Complete";

    // Folder [default] where the file is expected to be found
    var filePathFolder = "%outDir%\\" + bldArch + bldType + "\\bin";

    // Get the path to "cabarc.exe"
    var cabarcPath = "%srcBase%\\tools\\x86\\cabarc.exe";

    if (dependencies == undefined)
        dependencies = [_razzleBuildTask("corefre", "x86",  "ndp")];

    var ret = taskNew(taskName,
                      "runjs _coreCLRFileSizeCheck "+bldType+" "+bldArch+" "+pathBaseLineFile+" "+cabarcPath + " " +filePathFolder,
                      dependencies, /(x86)|(amd64)/);

    ret.description = "To update file size, edit <enlistment root>\\tools\\DDR\\CoreCLRFileSizeCheck.txt.";
    return ret;

}

/****************************************************************************/
/* Run layering test on the CoreCLR DLLs

   Parameters
    bldType         : <required> The build type (chk, dbg, ret ...)
    bldArch         : <required> The build architecture (x86, amd64 ia64)
*/

function _coreCLRLayeringTask(bldType, bldArch, binDir, trunStartDir, trunArgs, dependencies) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    // By default run location
    if (trunStartDir == undefined)
        trunStartDir = "src\\Layering\\Tests\\CoreCLR";

    // todo: add logDir param instead
    var taskNamePrefix = "test.coreCLRLayering." +  trunStartDir.replace(/[\\]/g, "-");

    // REMOVE WHEN TRUN LOG DIRECTORY BUG IS FIXED, expected 9/2005.
    // There is a bug in trun.exe that it interprets any instance of 'bc' in -!summaryLogs as -bc switch
    taskNamePrefix = taskNamePrefix.replace(/bcl/ig, "TRUN_ARG_BUG");

    var relOutDir = bldArch + bldType;
    var logDir = "%outDir%\\" + relOutDir + "\\" + taskNamePrefix;
    var taskName = taskNamePrefix + "@" + relOutDir;

    // leave binDir undefined if running VS suites
    if (binDir == undefined) {
        if (trunStartDir == "src\\vs")
            binDir = "_";
        else
            binDir = "%outDir%\\" + relOutDir + "\\bin";
    }

    if (dependencies == undefined)
        dependencies = [_razzleBuildTask("corefre", "x86",  "ndp")];

    // Path to the built binaries
    var builtPath = "%outDir%\\" + relOutDir + "\\bin";

    // Path to the batch file, RunLayerTest.cmd, that runs the test for us...
    var pathRunLayerTest = "%srcBase%\\ddsuites\\" + trunStartDir;

    var ret = taskNew(taskName,"runjs _coreCLRLayeringTest " +
                            builtPath + " " +
                            pathRunLayerTest + " " +
                            bldType + " " +
                            bldArch + " " +
                            "%srcBase%",
                            dependencies,
                            /(x86)|(amd64)/i);

	ret.description = "Layering test is used to validate the binary against CLR Layering Quality gate. See http://sharepoint/sites/layering/ for details.";
	return ret;
}

/****************************************************************************/
/* Run the security consistency test on a CoreCLR assembly

   Parameters
    bldType         : <required> The build type (chk, dbg, ret ...)
    bldArch         : <required> The build architecture (x86, amd64 ia64)
    assembly        : <required> The platform assembly to verify
*/

function _coreCLRSecurityConsistencyTask(bldType, bldArch, assembly, dependencies)
{
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (assembly == undefined)
        throw Error(1, "Arg assembly not supplied");

    var taskNamePrefix = "test.coreCLRSecurityConsistency." +  assembly;
    var relOutDir = bldArch + bldType;
    var logDir = "%outDir%\\" + relOutDir + "\\" + taskNamePrefix;
    var taskName = taskNamePrefix + "@" + relOutDir;

    if (dependencies == undefined)
        dependencies = [_razzleBuildTask("chk", "x86",  "ndp")];

    // Path to the built binaries
    var builtPath = "%outDir%\\" + relOutDir + "\\bin";

    var ret = taskNew(taskName,
                      "runjs _coreCLRSecurityConsistencyTest %srcBase% " + builtPath + " " + assembly,
                      dependencies,
                      /(x86)|(amd64)/i);
	ret.description = "Security consistency check for platform assemblies.";
	return ret;
}

/****************************************************************************/
/* Install the runtime using FXPSetup

logs go to %outDir%\<bldArch><bldType>

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
    bldArch     : The build architecture (x86, amd64 ia64)
    binDir      : The directory to get the runtime binaries from
    setupArgs   : Additional parameters to pass to clrSetup
    dependents  : Any dependents you want for this task (like a build)
*/

function _fxpSetupTask(bldType, bldArch, binDir, setupArgs, dependents, verstr) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (setupArgs == undefined)
        setupArgs = "";
    if (verstr == undefined)
        verstr = "_";

    var relOutDir = bldArch + bldType;
    var taskName = "fxpSetup";
    if (setupArgs != "")
            taskName += "." + setupArgs.replace(/(\s|\\)*/g, "").replace(/\//g, "-");
    taskName += "@" + relOutDir;

    var logDir = "%outDir%\\" + relOutDir;



    if (binDir == undefined) {
        if (dependents == undefined) {
            dependents = [];
        }
        binDir = logDir + "\\bin";
        dependents.push(_razzleBuildTask(bldType, bldArch, "ndp"));
    }

    var targetDir = logDir + "\\CoreCLR\\v2.0." + bldArch + bldType;

/*
    var clrSetupTaskArgs = Env("clrSetupTaskArgs");
    if (clrSetupTaskArgs != undefined) {
        setupArgs = setupArgs + " " + clrSetupTaskArgs;
    }
*/

    if (setupArgs == "" || setupArgs.match(/ /))
        setupArgs = "\"" + setupArgs + "\"";

    var ret = taskNew(taskName,
        "runjs fxpSetup " + binDir + " "
                          + targetDir,
        dependents,
        _machPatForArch(bldArch));

    ret.description = "The fxpsetup task is responsible for installing a given set of CLR binaries on the current machine.";
    return ret;
}

/****************************************************************************/
/* Does CoreCLR file size check


  Parameters:
    bldType     : The build type (chk, dbg, ret ...)
    bldArch     : The build architecture (x86, amd64 ia64)
    filenames   : path to the file containing file size check details
    cabarcPath  : Path to cabarc.exe
filePathFolder  : default path in which the file is expected to be found

*/
function _coreCLRFileSizeCheck(bldType, bldArch, baselineFile, cabarcPath, filePathFolder)
{

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (baselineFile == undefined)
        throw Error(1, "Arg filenames not supplied");
    if (cabarcPath == undefined)
        throw Error(1, "Arg cabarcPath not supplied");
    if (filePathFolder == undefined)
        throw Error(1, "Arg filePathFolder not supplied");

    // Inform about the baseline file we are using
    logMsg(LogClrTask, LogInfo, "Using file size details from " + baselineFile + "\n\n");

    // Read the contents of the filesize-check file
    var szFileSizeCheckContents = FSOReadFromFile(baselineFile);
    var arrLinesInFile = szFileSizeCheckContents.split("\n");
    var iLineCount = arrLinesInFile.length;
    var retString = "";
    var strLine = "";
    var KB = 1024;
    var MB = KB*KB;

    // Loop thru the lines in the file
    for(var iIndex = 0; iIndex < iLineCount; iIndex++)
    {
        // Is this a comment line?
        strLine = arrLinesInFile[iIndex].valueOf();
        if (strLine != undefined)
        {
           strLine = strLine.replace(" ","");
        }

        if (strLine != undefined && strLine.length > 3) // Has to be atleast REM
        {
            var strLineStart = strLine.substr(0,3).toUpperCase();
            if (strLineStart != undefined)
            {
               strLineStart = strLineStart.replace(" ","");
            }

            if (strLineStart != undefined && strLineStart != "REM")
            {
               // This is a regular line that we will need to process
              var arrLineDetails = strLine.split(";");

              // Ensure that the array length is 4
             if (arrLineDetails.length == 4)
             {
                    var filenames = arrLineDetails[0].valueOf().replace(" ","");
                    var maxSize = arrLineDetails[1].valueOf().replace(" ",""); // This value is in MB
                    var toleranceSize = arrLineDetails[2].valueOf().replace(" ",""); // This value is in KB
                    var fCompress = false;
                    if (arrLineDetails[3].valueOf().toUpperCase().replace(" ","").substr(0,3) == "YES")
                    {
                        fCompress = true;
                    }

                    // Get the number of files to be processed
                    var arrfilePath = filenames.split(",");
                    var iFileCount = arrfilePath.length;
                    var fileList = "";

                    // Initial uncompressed size
                    var targetFilesize = 0.0;
                    var targetFilename = "";

                    // Create the list of the files that will be added to the cab file
                    for(var i = 0; i < iFileCount; i++)
                    {
                        // First check if the filename already contains the path - we do this by checking the
                        // presence of any "\" in the filename
                        if (arrfilePath[i].indexOf("\\") == -1)
                        {
                           // This is just a file name - add the default path to it
                           targetFilename = filePathFolder + "\\" + arrfilePath[i];
                        }
                        else
                        {
                           // The file name already contains the path
                           targetFilename = arrfilePath[i];
                        }

                        if (FSOFileExists(targetFilename))
                        {
                            // Increment the uncompressed size
                            targetFilesize = targetFilesize + (FSOGetFile(targetFilename).Size);

                            // form the file list
                            fileList = fileList + "\"" + targetFilename + "\" ";
                        }
                        else
                        {
                             retString = retString + targetFilename + " does not exist!\n\n";
                        }
                    }

                    var fileCompressedPath = undefined;
                    if (fCompress == true && (cabarcPath != undefined) && (fileList != ""))
                    {
                        // Get the cab filename
                        fileCompressedPath = FSOGetTempPath("CoreCLR")+".compressed";
                        var run = runCmdToLog(cabarcPath + " -m LZX:21 N " + fileCompressedPath + " " + fileList);
                        if (run.exitCode != 0)
                        {
                            throw new Error("Unable to compress " + filenames + ". ExitCode=" + run.exitCode);
                        }

                        // Update the filepath
                        filePath = fileCompressedPath;

                        // Get the compressed file size
                        targetFilesize = FSOGetFile(filePath).Size;

                        // Delete the compressed file
                        FSODeleteFile(fileCompressedPath);
                    }

                    // Calculate the maximum allowed size in bytes
                    var allowedSize = ((maxSize*MB) + (toleranceSize*KB));
                    if (targetFilesize >=1 && (targetFilesize <= allowedSize))
                    {
                         // everything is good for this file(s)
                         var strMesg = "[" + filenames + "] size (" + (targetFilesize/MB) + "MB) is within the limit of ";
                         strMesg = strMesg + "the allowed size (" + (allowedSize/MB) + "MB)";
                         logMsg(LogClrTask, LogInfo, strMesg + "\n\n");
                    }
                    else
                    {
                        retString = retString + filenames;
                        // Prepare message for warning
                        if (targetFilesize >0)
                        {
                            // We are exceeding size limit boundary for this file [these files]
                            retString = retString + ", size " + (targetFilesize/MB) + "MB, ";
                            retString = retString + " is beyond the limit of " + maxSize + "MB, ";
                            retString = retString + "including tolerance of " + toleranceSize + "KB.";
                        }
                        else
                        {
                            // Zero size!
                            retString = retString + " has zero length!";
                        }
                        retString = retString + "\n\n";

                    }
               } // End of array length = 4 check
              else
              {
                  // Log warning about invalid line structure
                  logMsg(LogClrTask, LogWarn, "Invalid structure at line "+ (iIndex+1));
              }
           } // if (strLineStart != "REM")
        } // if (arrLinesInFile[iIndex].length > 3)
    } // For loop

    // return the size check status - it will be empty string on successful check!
    var iRetCode = 0;
    if (retString != "")
    {
        // log the return message - it contains details of the files that failed the test
        logMsg(LogClrTask, LogWarn, "Filesize check has failed! Details: \n\n" + retString);
        iRetCode = 1;
    }

    return iRetCode;
}

/****************************************************************************/
/* Does CoreCLR Layering Check


  Parameters:

*/
function _coreCLRLayeringTest(builtPath, pathRunLayerTest, bldType, bldArch, srcBase, dependencies)
{

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (builtPath == undefined)
        throw Error(1, "Arg builtPath not supplied");
    if (pathRunLayerTest == undefined)
        throw Error(1, "Arg pathRunLayerTest not supplied");
    if (srcBase == undefined)
        throw Error(1, "Arg srcBase not supplied");

    // Set WOW mode if applicable
    var realProcArch = getRealProcessorArchitecture();
    var ldr64ExeForWoW = undefined;
    var isNotWow = false;

    if (Is64bitArch(realProcArch, "coreclr"))
    {
        // Set 32bit loader... else layering tools will fail
        ldr64ExeForWoW = getLdr64Exe();
        isNotWow = runCmdToLog(ldr64ExeForWoW + " query", runSetNoThrow()).exitCode;
        if (isNotWow) {
            logMsg(LogClrTask, LogWarn, "ldr64 setwow \n\n");
            runCmd(ldr64ExeForWoW + " setwow");
        }
    }

    try
    {
        var envDDBuiltTarget = runSetEnv("DD_BuiltTarget", builtPath);
        var envSymbols = runSetEnv("_NT_SYMBOL_PATH", builtPath + "\\Symbols.Pri\\Retail", envDDBuiltTarget);
        var envSrcBase = runSetEnv("_NTBINDIR", srcBase, runSetTimeout(1200, envSymbols));
        var pathToTrun = srcBase + "\\ddsuites\\tools\\x86\\trun.exe";

        var run = runCmdToLog("pushd " + pathRunLayerTest + " & "
                                + pathToTrun,
                                envSrcBase);
    }
    finally
    {
    // Reset loader mode as applicable
        if (isNotWow) {
            logMsg(LogClrTask, LogWarn, "ldr64 set64 \n\n");
            runCmd(ldr64ExeForWoW + " set64");
        }
    }

    if (run.exitCode != 0)
    {
        throw new Error("Layering test failed. ExitCode=" + run.exitCode);
    }

    return run.exitCode;
}

/****************************************************************************
 * Does CoreCLR security consistency test
 *
 * Parameters:
 *
 *  builtPath  - <required> path to the CoreCLR build
 *  assembly   - <required> assembly to verify
 */
function  _coreCLRSecurityConsistencyTest(srcBase, builtPath, assembly)
{
    if (srcBase == undefined)
    {
        throw Error(1, "Arg srcBase not supplied");
    }
    if (builtPath == undefined)
    {
        throw Error(1, "Arg builtPath not supplied");
    }
    if (assembly == undefined)
    {
        throw Error(1, "Arg assembly not supplied");
    }

    var testRunPath = srcBase + "\\ndp\\clr\\src\\ToolBox\\SecurityAnnotations\\VerificationTest\\ClrTests";
    var testBinPath = srcBase + "\\ndp\\clr\\src\\ToolBox\\SecurityAnnotations\\VerificationTest\\VerifySecurityConsistency\\bin\\Debug";

    // Figure out which config file to use
    var assemblyConfigFile = testRunPath + "\\" + assembly + ".VerifySecurityConsistency.exe.config";
    if (!FSOFileExists(assemblyConfigFile))
    {
        assemblyConfigFile = testRunPath + "\\clr.VerifySecurityConsistency.exe.config";
    }

    try
    {
        // Copy the test binaries to the run directory
        FSOCopyFile(assemblyConfigFile, testRunPath + "\\VerifySecurityConsistency.exe.config", "FORCE");
        FSOCopyFile(testBinPath + "\\VerifySecurityConsistency.exe", testRunPath, "FORCE");
        FSOCopyFile(testBinPath + "\\VerifySecurityConsistency.pdb", testRunPath, "FORCE");

        // Since the test binaries are copied from checked-in binaries, they will have their read only bit
        // set.  Clear that here, so that we can erase the files when we're done.
        FSOClearReadOnly(testRunPath + "\\VerifySecurityConsistency.exe.config");
        FSOClearReadOnly(testRunPath + "\\VerifySecurityConsistency.exe");
        FSOClearReadOnly(testRunPath + "\\VerifySecurityConsistency.pdb");

        // Setup the execution environment
        var environment = WshShell.Environment("PROCESS");
        environment("_CoreCLRTestLocation") = builtPath;

        // Run the test
        var runCmd = testRunPath + "\\VerifySecurityConsistency.exe " + builtPath + "\\" + assembly;
        var runOpts = runSetEnv("COMPLUS_InstallRoot", srcBase + "\\tools\\x86\\managed",
                      runSetEnv("COMPLUS_Version", "v4.0",
                      runSetLog(LogRun, LogInfo)));
        var run = runWithRuntimeArch(runCmd, "x86", runOpts);

        if (run.exitCode != 0)
        {
            throw new Error("Security verification test test failed. ExitCode = " + run.exitCode);
        }

        return run.exitCode;
    }
    finally
    {
        // Cleanup the copies of the test
        FSODeleteFile(testRunPath + "\\VerifySecurityConsistency.exe", "FORCE");
        FSODeleteFile(testRunPath + "\\VerifySecurityConsistency.exe.config", "FORCE");
        FSODeleteFile(testRunPath + "\\VerifySecurityConsistency.pdb", "FORCE");
    }
}

/****************************************************************************/
/* Build the following Mac Managed Assemblies:
   - mscorlib.dll                 @ ndp\clr\src\bcl.small\mac
   - Sandboxhelper.dll            @ ndp\clr\coreclr\sandboxhelper\mac
   - System.dll                   @ ndp\fx\src\sys.small\mac
   - System.Core.dll              @ ndp\fx\src\core.small
   - Microsoft.WlcProfile.dll     @ ndp\fx\src\Sys.Prototype
*/
function buildMacManagedAssemblies() {
    var env_NTBINDIR = Env("_NTBINDIR");

    // build inc folder, files like fusion.h are used when building mac managed assemblies
    _rbuildFolder(env_NTBINDIR + "\\ndp\\clr\\src\\inc");

    // mscorlib.dll
    var bclSmallMyDirs = Env("_NTBINDIR") + "\\ndp\\clr\\src\\bcl.small\\mydirs";
    if(FSOFileExists(bclSmallMyDirs)) // If the mydirs file exists in the bcl.small folder, we build using bcl.small sources
        _rbuildFolder(env_NTBINDIR + "\\ndp\\clr\\src\\bcl.small\\mac");
    else // build in the regular folder using the binary rewriter
        _rbuildFolder(env_NTBINDIR + "\\ndp\\clr\\src\\bcl.small\\bcl.mac");

    // sandboxhelper.dll
    _rbuildFolder(env_NTBINDIR + "\\ndp\\clr\\src\\coreclr\\sandboxhelper\\mac");

    // System.dll
    _rbuildFolder(env_NTBINDIR + "\\ndp\\fx\\src\\sys.small\\mac");

    // System.Core.dll
    _rbuildFolder(env_NTBINDIR + "\\ndp\\fx\\src\\core.small");

    // Microsoft.WlcProfile.dll
    _rbuildFolder(env_NTBINDIR + "\\ndp\\fx\\src\\Sys.Prototype");

    return 0;
}

/****************************************************************************/
/* Create owner list
*/
function _createOwnerList() {

    var taskName = "createOwnerList";

    var ownerFile ="owners.csv";
    var logFile = "%outDir%\\" + ownerFile;

    var ret = taskNew(taskName,"runjs createOwnerList " + logFile, undefined, /(x86)|(amd64)/i);
    ret.description = "Creates a current list of CLR source owners, saved as owners.csv, and validates that source files have current owners\r\n";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Investigating%20DDR%20failures%20in%20createOwnerList%20task.aspx"
    return ret;
}

function _rbuildFolder(buildPath)
{
    var run = runCmdToLog("pushd " + buildPath + " && rbuild -cC ");
    if (run.exitCode != 0)
    {
        throw new Error("Got errors when executing rbuild -cC @" + buildPath + ". ExitCode = " + run.exitCode);
    }
}

/****************************************************************************/
/* publish a .url shortcut 'shortCutFile' that points to 'targetFile'.  It
   insures that the target is not dependent on the machine you are on (that
   is, the target is a UNC path).  If it is not it tries to make it one.
   It does nothing if there is no UNC path for the target.  This function never
   throws.  It returns 0 on success.
*/

function _publish(shortCutFile, targetFile) {

	var oldLog = undefined;
	try {
		var logFile = "\\\\CLRMain\\public\\Drops\\dailyDevRuns\\logs\\" + Env("USERNAME") + ".log";
		oldLog = logSetLimitedFile(logFile, 10000);

		if (!FSOFileExists(targetFile)) {
			logMsg(LogTask, LogWarn, "_publish: path ", targetFile, " does not exist\n");
			return -1;
		}

		var uncName = uncPath(targetFile, false, true);	// Get the full path to the share
		if (!uncName.match(/^\\\\/)) {
			logMsg(LogTask, LogWarn, "_publish: path ", targetFile, " is not shared out, can not publish it\n");
			return -1;
		}

		FSOCreatePathForFile(shortCutFile);
		urlShortCutCreate(shortCutFile, targetFile);
		logMsg(LogTask, LogInfo, "_publish: publishing link ", shortCutFile, " to file ", targetFile, "\n");

	}
	catch(e) {
		logMsg(LogTask, LogInfo, "_publish: failed publishing link\n");
		return -1
	}
	if (oldLog != undefined)
		logSetFile(oldLog);

	return 0;
}

function _publishFolder(shortCutFile, targetFolder) {

	try {
		if (!FSOFolderExists(targetFolder)) {
			logMsg(LogTask, LogWarn, "_publishFolder: path ", targetFolder, " does not exist\n");
			return -1;
		}

		var uncName = uncPath(targetFolder, false, true);  // Get the full path to the share
		if (!uncName.match(/^\\\\/)) {
			logMsg(LogTask, LogWarn, "_publishFolder: path ", targetFolder, " is not shared out, cannot publish it\n");
			return -1;
		}

		FSOCreatePathForFile(shortCutFile);
		urlShortCutCreate(shortCutFile, targetFolder);
		logMsg(LogTask, LogInfo, "_publishFolder: publishing link ", shortCutFile, " to file ", targetFolder, "\n");

	}
	catch(e) {
		logMsg(LogTask, LogInfo, "_publishFolder: failed publishing link\n");
		return -1;
	}

	return 0;
}

/****************************************************************************/
/* Publish the log files.  It attempts to copy all log files generated to the
   clrmain server.  It maintains a maximum number of logs to be kept on
   clrmain.  This function never throws.  It returns 0 on success.
*/

/* TODO: get \\clrmain server a different way, to allow for different servers
 *       based on location (e.g., Shanghai) or maybe some other reason
 */

function _publishLogs(sourceDirectory) {
    try {
        // Copy logs to clrmain
        var MAXDAILYDEVRUNDIRS = 250;  // Maximum daily devrun logs to store on clrmain.
        var branch = _getBranchFromSrcBase(srcBaseFromScript(), true);
        if(branch == "")
             branch = "puclr"
        // CreateFolder doesn't let us just create any folder, we have to do it in steps
        // This would probably be best implemented in FSOCreateFolder, but for now, we will
        // just verify the branch folder exists, and its child logs folder.
        var dailyDevRunDirectoryRoot = "\\\\clrmain\\public\\DailyDevRuns";
        var dailyDevRunDirectory = dailyDevRunDirectoryRoot + "\\" + branch;
        if(!FSOFolderExists(dailyDevRunDirectory))
        {
             FSOCreateFolder(dailyDevRunDirectory);
        }
        dailyDevRunDirectory += "\\logs";
        if(!FSOFolderExists(dailyDevRunDirectory))
        {
             FSOCreateFolder(dailyDevRunDirectory);
        }
        var devDirectory = dailyDevRunDirectory + "\\" + Env("USERNAME") + "." + Env("COMPUTERNAME") + "_" + sourceDirectory.match(/\d{2}-\d{2}-\d{2}_\d{2}\.\d{2}\./i) + "0";

        // Make sure we limit the total number of log files.
        var directoryList = [];

        var deleteAttemptIndex = 0;
        var length;

        {
            var dailyDevRunDirectoryList = FSOGetDirPattern(dailyDevRunDirectoryRoot, ".*");
            var index = dailyDevRunDirectoryList.length - 1;
            while (index >= 0) {
                var tmpList = FSOGetDirPattern(dailyDevRunDirectoryList[index--] + "\\logs", ".*");
                if (tmpList != null) {
                    var i = tmpList.length - 1;
                    while (i >= 0) {
                        directoryList.push(tmpList[i--]);
                    }
                }
            }
        }
        directoryList.sort(sortDateFileNamesByDate);

        length = directoryList.length;

        logMsg(LogTask, LogInfo, dailyDevRunDirectoryRoot + " has " + length + " saved DDR's\n");

        while(length >= MAXDAILYDEVRUNDIRS && deleteAttemptIndex < MAXDAILYDEVRUNDIRS) {
            try {
                logMsg(LogTask, LogInfo, "Attempting to delete: " + directoryList[deleteAttemptIndex] + "\n");
                FSODeleteFolder(directoryList[deleteAttemptIndex++], true);
                logMsg(LogTask, LogInfo, "Deletion attempt successful.\n");
                length--;
            }
            catch(e) {
                logMsg(LogTask, LogInfo, "Deletion attempt failed.\n");
            }
        }

        if(deleteAttemptIndex >= MAXDAILYDEVRUNDIRS) {
            logMsg(LogTask, LogInfo, "_publishLogs: failed to clear space for dailydevrun logs\n");
            logMsg(LogTask, LogInfo, e.description + "\n");
            return -1;
        }
        robocopy( sourceDirectory, devDirectory, "*.xml *.html *.dmp /NFL /S /R:3 /W:10", sourceDirectory + "\\robocopy.log");
    }
    catch(e) {
        logMsg(LogTask, LogInfo, "_publishLogs: failed copying log files to CLRMain\n");
        logMsg(LogTask, LogInfo, e.description + "\n");
        return -1;
    }
    return 0;
}


function _applyChangesTask(changesetNumber, shelveset, pathToTfCmd, dependents)
{
    if (changesetNumber == undefined)
        changesetNumber = "_";
    if (shelveset == undefined)
        shelveset = "_";
    if (pathToTfCmd == undefined)
        pathToTfCmd = "_";
    if (dependents == undefined)
        dependents = [];

    var taskName = "applyChanges";
    var ret = taskNew(taskName, ScriptDir + "\\runjs tfApplyChanges %outDir% " + changesetNumber + " " +
                      shelveset + " %srcBase% " + pathToTfCmd, dependents, "local");

    ret.description = "This task syncronizes the enlistment to the specified changeset, and if a " +
                      "shelveset is specified, it is unshelved.";
    return ret;
}


function _genOptBuildTask(bldType, bldArch, localBinDir, relUnoptBuildDir, relOptBuildDir, privateJob, dependents) {

    if (bldType == undefined)
        throw new Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw new Error(1, "Arg bldArch not supplied");
    if (localBinDir == undefined)
        throw new Error(1, "Arg localBinDir not supplied");
    if (relUnoptBuildDir == undefined)
        relUnoptBuildDir = bldArch + bldType;
    if (relOptBuildDir == undefined)
        relOptBuildDir = bldArch + bldType + ".opt";
    if (privateJob == undefined)
        privateJob = false;

    if (relUnoptBuildDir.toLowerCase() == relOptBuildDir.toLowerCase())
        throw new Error(1, "relUnoptBuildDir and relOptBuildDir cannot be the same");

    // Setup our dependent tasks if the caller did not supply us with anything

    if (dependents == undefined)
    {
        // We need to do razzle build to produce the unoptimized bits. Our enlistment
        // must be synced to the appropriate changeset

        var unoptBuildDir = localBinDir + "\\" + relUnoptBuildDir;

        dependents = [
            _retailLabBuildTask(bldType, bldArch, "ndp", undefined, undefined, unoptBuildDir, "%outDir%\\UnoptBuild")
        ];
    }

    var taskName     = "genOptBuild@" + bldArch + bldType;

    var ret = taskNew(taskName, "%srcBase%\\ndp\\clr\\bin\\runjs genOptBuild " + bldArch + " " + bldType +
                      " %srcBase% %outDir% " + localBinDir + " " + relUnoptBuildDir + " " + relOptBuildDir + " " +
                      privateJob, dependents, /(x86)|(amd64)/i);

    ret.description = "Create a version of the runtime that has had its code arranged for\r\n"+
                      "optimal working set performance.\r\n";
    return ret;
}


/****************************************************************************/
/* Create a task that does an optimized razzle build

  logs go to %outDir%\<bldArch><bldType>
  binaries go to %outDir%\<bldArch><bldType>\bin

  Parameters
    bldType     : The build type (chk, dbg, ret ...)
                  This also can contain an optional suffix such as ".unopt"
    bldArch     : The build architecture (x86, amd64 ia64)
    bldRelDir   : The directory relative to %srcBase% to build in
    binDir      : The directory to place the built binaries in,
                  (defaults to "%outDir%\\" + bldType + bldArch + "\\bin")
    timeOut     : Set the timeout for the build, defaults to 90 minutes
    bldArgs     : additional parameters to pass to build (like -cC)
*/

function _retailLabBuildTask(bldType, bldArch, bldRelDir, bldArgs, timeOut, binDir, logDir, dependents) {

    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");
    if (bldRelDir == undefined)
        bldRelDir = "ndp\\clr";
    if (logDir == undefined)
        logDir = "%outDir%\\" + bldArch + bldType;
    if (binDir == undefined)
        binDir = logDir + "\\bin";

    // Setup baseBldType by removing any suffix that may have been attached to bldType
    var baseBldType = bldType;
    if (baseBldType.match(/(.+)\.(.+)$/)) {
        baseBldType = RegExp.$1;
    }

    var relOutDir = bldArch + bldType;
    var taskName = "retailLabBuild." + bldRelDir.replace(/[\\]/g, "-") + "@" + relOutDir;

    if (timeOut == undefined)
        timeOut = 120 * MINUTE;

    if (bldArgs == undefined)
        bldArgs = "-cC";
    if (bldArgs == "")
        bldArgs = "\"\"";

    var ret = taskNew(taskName,
                      "%srcBase%\\ndp\\clr\\bin\\runjs doRetailLabBuild "
                              + '\"' + baseBldType + '\"' + " "
                              + '\"' + bldArch + '\"' + " "
                              + '\"' + bldRelDir + '\"' + " "
                              + '\"' + bldArgs + '\"' + " "
                              + "\"%srcBase%\" "
                              + '\"' + binDir + '\"' + " "
                              + '\"' + logDir + '\"' + " "
                              + '\"' + timeOut + '\"',
                              dependents,
                              "local");

    ret.description = "An unoptimized build of some directory in the source base.";
    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Running%20Tests.aspx#_Toc117485070";
    return ret;
}

/****************************************************************************/
/* Runs QDR (checkin verification suite for small fixes).
   This will build, install and run devBVTs for specified build type/arch.

    bldType   : The build type (chk, dbg, ret ...)
                clrenv value used as default
    bldArch   : The build architecture (x86, amd64 ia64)
                clrenv value used as default
    bldRelDir : The directory relative to %srcBase% to build (ndp, ndp\clr\src)
                ndp\clr\src is default
*/
function quickDevRun(bldType, bldArch, bldRelDir) {

    if (!isElevated()) {
        throw new Error(1, "quickDevRun: Requires executing with elevated priviledges");
    }

    if (bldType == undefined) {
        bldType = Env("_BuildType").toLowerCase();
        if (bldType == "") {
            // Running outside of clrenv - use chk
            bldType = "chk";
        }
    }
    if (bldArch == undefined) {
        bldArch = Env("_BuildArch").toLowerCase();
        if (bldArch == "") {
            // Running outside of clrenv - use current window architecture
            bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
        }
    }
    if (bldRelDir == undefined)
        bldRelDir = "ndp\\clr";
    var qdrTaskStr = _quickDevRunTaskName(bldType, bldArch, bldRelDir);

    var srcBase = srcBaseFromScript();
    WScript.Echo("*****************************************************************************");
    WScript.Echo("*                        Starting quickDevRun");
    WScript.Echo("*");
    WScript.Echo("* Report: " + srcBase + "\\automation\\run.current\\taskReport.html");
    WScript.Echo("* The report can be viewed before the run is complete");
    WScript.Echo("*****************************************************************************");
    WScript.Echo("");

    var outDirBase = srcBaseFromScript() + "\\automation";
    var numRunsToKeep = Env("DDR_RUNS_TO_KEEP");
    if (numRunsToKeep == "")
        numRunsToKeep = 5;

    var outDir = newRunDir(outDirBase, numRunsToKeep);
    var ret = doRunHere(qdrTaskStr, outDir);

    return ret;
}

function _quickDevRunTaskName(bldType, bldArch, bldRelDir) {
    return "quickDevRun." + bldRelDir.replace(/[\\]/g, "-") + "@" + bldArch + bldType;
}

/****************************************************************************/
/* Create a task that does QDR.

  logs go to %outDir%\<bldArch><bldType>
  binaries go to %outDir%\<bldArch><bldType>\bin

  Parameters
    bldType   : The build type (chk, dbg, ret ...), 'chk' value used as default
    bldArch   : The build architecture (x86, amd64 ia64), clrenv value used as default
    bldRelDir : The directory relative to %srcBase% to build in (ndp, ndp\clr\src), the ndp\clr\src is default
*/
function _quickDevRunTask(bldType, bldArch, bldRelDir) {

    if (bldType == undefined)
        bldType = "chk";
    if (bldArch == undefined)
        bldArch = Env("_BuildArch").toLowerCase();
    if (bldRelDir == undefined)
        bldRelDir = "ndp\\clr";

    var clr = "clr";
    if (isCoreCLRBuild(bldType))
        clr = "coreclr";

    var devBVTTask;
    if (clr == "clr")
    {   // Desktop CLR
        var buildTask = _razzleBuildTask(bldType, bldArch, bldRelDir);

        var binDir = "%outDir%\\" + bldArch + bldType + "\\bin";
        var setupArgs;
        if ((bldRelDir == "ndp\\clr\\src") || (bldRelDir == "ndp\\clr")) {
            setupArgs = "";
        } else {
            // We build also ndp\fx directory, so use the result binaries
            setupArgs = "/fx ";
        }
        setupArgs += "/nrad";   // NoRegisterAsDefault
        var setupTask = _clrSetupTask(
            bldType,
            bldArch,
            binDir,         // We have to pass binDir, otherwise _clrSetupTask will add _razzleBuildTask of ndp directory
            setupArgs,
            [buildTask]);   // dependents

        devBVTTask = _devBVTTask(
            bldType,
            bldArch,
            undefined,  // binDir ... it is needed only when dependents are not passed
            undefined,  // smartyArgs
            [setupTask]);   // dependents
    }
    else
    {   // CoreCLR
        var buildTask = _razzleBuildWithOptionalBclRewriterTask("win", bldType, bldArch, bldRelDir);

        var binDir = "%outDir%\\" + bldArch + bldType + "\\bin";
        var setupTask = _fxpSetupTask(
            bldType,
            bldArch,
            binDir,         // We have to pass binDir, otherwise _fxpSetupTask will add
                            // _razzleBuildWithOptionalBclRewriterTask of ndp directory
            undefined,      // setupArgs
            [buildTask],    // dependents
            undefined);     // verstr

        devBVTTask = _devBVTTask(
            bldType,
            bldArch,
            undefined,  // binDir ... it is needed only when dependents are not passed
            undefined,  // smartyArgs
            [setupTask]);   // dependents
    }

    return taskGroup(
        _quickDevRunTaskName(bldType, bldArch, bldRelDir),
        [devBVTTask],
        "QuickDevRun is the group of tests that CLR developers are expected to run before small check-ins");
}

/****************************************************************************/
/* Run multiple tasks using simplified syntax (aliases, task names/patterns,
   SelfHost categories).

  Parameters:
    List of aliases, tasks, SelfHost categories and modificators.
    Aliases can be simply customized - see below.

  Aliases:
    Most popular aliases: QDR, DDR, scan/scanRuntime, prefast, build.
    To see full list of aliases run 'runjs queueShow'.

    Aliases can be modified by modificators using . (dot) separator.
    For example 'alias.amd64', or 'alias.ndp.chk.x86'. See modificators
    section for full details.
    It's up to each alias implementation if it will use a modificator, or not
    (for example QDR will ignore build flavor modificators and always use
    chk-flavor).

    To implement your custom alias (just for yourself), add an alias into your
    runjs.local.js file:
        ndp\clr\bin\scriptLib\user\%USERNAME%\runjs.local.js
    See code:_queueAlias and code:_queueAliasAdd for more information.

  Modificators:
    There are 3 types of modificators:
      - Build flavor (chk, dbg, ret, fre, corechk, ...),
      - Architecture (x86, amd64, ia64),
      - Directory scope (ndp, ndp-clr-src).
      Note: Architecture can be combined with flavor (e.g. x86chk).
    Modificators can be used as:
      - Standalone aliases to modify remaining command list 'default' value.
        Example:
          runjs queue ndp QDR amd64 ret build
            ... This runs QDR with whole ndp directory build and then runs
                amd64ret build of the whole ndp directory.
      - Alias modificators to modify one alias (local) properties.
        Example:
          runjs queue QDR.ndp build.amd64.ndp.ret
            ... This will run the same tests as the previous command.

  Tasks:
    You can use task names or regexp of task names directly (the same way as
    in doRun command).
    For list of all task names run 'runjs doRunShow'.

  SelfHost categories:
    NOT YET IMPLEMENTED


  Examples:
    runjs queue QDR scan prefast
      ... Runs QDR with scanRuntime and prefastBuild tasks.

    runjs queue QDR.amd64 QDR.x86
      ... Runs 2 QDRs for amd64chk and x86chk builds.

    runjs queue QDR.ndp.amd64
      ... Runs QDR which builds whole ndp directory for amd64 platform.

    runjs queue DDR Security Interop JIT
      ... Runs DDR together with Security, Interop and JIT SelfHost categories.
          [NOT YET IMPLEMENTED]

    runjs queue QDR dacCop@x86chk.dacCop
      ... Runs QDR with dacCop@x86chk.dacCop task (see 'runjs doRunShow' for
          list of all tasks).
*/
function queue() {

    var tasks = new Array();

    //#EnvModificatorsParsing
    // Modificators from the environment (see code:#Modificators)
    var envMods = new Object();
    envMods.bldType = Env("_BuildType").toLowerCase();
    if (envMods.bldType == "") {
        // Running outside of clrenv - use chk
        envMods.bldType = "chk";
    }
    envMods.bldArch = Env("_BuildArch").toLowerCase();
    if (envMods.bldArch == "") {
        // Running outside of clrenv - use current window architecture
        envMods.bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    }
    envMods.bldRelDir = "ndp\\clr\\src";

    // Modificators from the command line
    var cmdLineMods = new Object();

    var commandStr = "runjs queue";
    for (var i = 0; i < arguments.length; i++) {
        var name = arguments[i];
        commandStr += " " + name;

        //#CmdLineModificatorsParsing
        // Try to parse command line modificators (code:#Modificators)
        if (_queue_ParseModificators([name], cmdLineMods, false)) {
            continue;
        }

        // Try to find alias of this name
        var alias = _queue_FindAlias(name);
        if (alias != undefined) {
            // We have matching alias
            var task = undefined;
            if (alias.task != undefined) {
                task = alias.task;
            } else if (alias.functionPtr != undefined) {
                var mods = new _queueModificators(alias.params, cmdLineMods, envMods);
                task = alias.functionPtr(name, mods);
            }
            if (task != undefined) {
                tasks.push(task);
            }
            continue;
        }

        // Try to find tasks matching the name as regexp
        var matchingTasks = _tasksFindPat(_tasksAll, new RegExp("^(" + name + ")$", "i"));
        if (matchingTasks.length >= 1) {
            // We have matching task(s)
            for (var matchingTask in matchingTasks) {
                tasks.push(matchingTasks[matchingTask]);
            }
            continue;
        }

        // TODO: Detect SelfHost categories
    }

    var mainTask = taskGroup(
        "QueuedTasks",
        tasks,
        "Queued mutiple tasks");
    _taskAdd(mainTask);

    return doRun(mainTask);
}

/****************************************************************************/
/* List all aliases available for usage in queue command.
   See 'runjs /? queue' for more details about the code:queue command.
*/
function queueShow() {
    // TODO: Nicely format code:_queueAliases table.
    WScript.Echo("NOT IMPLEMENTED YET.");
    WScript.Echo("See _queueAliases table in clrtask.js for now.");
}

/****************************************************************************/
/* Create alias for code:queue command.
   All aliases used from code:queue command are stored in code:_queueAliases table.
   Anyone can add custom aliases using code:_queueAliasAdd by simply passing result of this function as its
   parameter.

  Parameters:
    name   : The alias name which can be used in queue command.
    target : Target of the alias. It can be of 3 types:
        * string (alias name) ... Forwards alias 'name' to alias 'target'.
            Useful for abbreviations, e.g.:
              _queueAlias("QDR", "quickDevRun")
        * object (task) ... Runs this task when alias 'name' is used.
            Examples:
              _queueAlias("DDR", _taskDailyDevRun)
              _queueAlias("FxBuild", _razzleBuildTask("x86", "ret", "ndp\\fx\\src"))
        * function (pointer) ... Runs the function with modificators information.
            See code:#AliasImplementation and existing usage (e.g. code:_queueTasks_quickDevRun).
            Examples:
              _queueAlias("MyAlias", _queueTasks_myAliasImplementation)
*/
function _queueAlias(name, target) {
    var alias = new Object();
    alias.name = name;
    if (typeof(target) == "string") {
        alias.forwardToAlias = target;
    } else if (typeof(target) == "object") {
        alias.task = target;
    } else if (typeof(target) == "function") {
        alias.functionPtr = target;
        alias.params = new Object();
    } else {
        throw new Error(1, "_queueAlias: Invalid target type");
    }

    return alias;
}

/****************************************************************************/
/* Table of aliases for code:queue command.
   Anyone can add custom aliases using code:_queueAliasAdd.
*/
_queueAliases = [
        // [ alias, forward-to-alias | function pointer | task (object) ]
        _queueAlias("quickDevRun", _queueTasks_quickDevRun),
        _queueAlias("QDR", "quickDevRun"),
        _queueAlias("dailyDevRun", _queueTasks_dailyDevRun),
        _queueAlias("DDR", "dailyDevRun"),

        _queueAlias("scanRuntime", _queueTasks_scanRuntime),
        _queueAlias("scan", "scanRuntime"),
        _queueAlias("prefast", _prefastBuildTask()),

        _queueAlias("build", _queueTasks_build)
    ];

/****************************************************************************/
/* Helper functions for modificators manipulation.

  Alias implementation function gets a set of 3 modificators. Each set has the same members
  (see code:#Modificators). The 3 sets are:
    explicit : Parameters of the alias - the 'local' modificators used via dot-syntax.
               If certain modificator was not used in the syntax, then its value will be undefined.
               Example: 'runjs queue QDR.ndp'
                 ... explicit.bldRelDir = "ndp"
                     explicit.bldType = undefined
                     explicit.bldArch = undefined

    cmdLine  : Parameters specified on the command line. This set is specific for each alias on the command
               line as the modificators' values can change during parsing.
               If certain modificator was not used (yet) on the command line, then its value will be
               undefined.
               Example: 'runjs queue x86 QDR amd64 build'
                 ... For 'QDR' alias, the values are:
                     cmdLine.bldRelDir = undefined
                     cmdLine.bldType = undefined
                     cmdLine.bldArch = "x86"
                 ... For 'build' alias the values are:
                     cmdLine.bldRelDir = undefined
                     cmdLine.bldType = undefined
                     cmdLine.bldArch = "amd64"
               Note: cmdLine modificators are parsed continuosly - see code:#CmdLineModificatorsParsing.

    env      : Default modificators' value. Each modificator has some built-in default value.
               Example: 'runjs queue QDR'
                 ... env.bldRelDir = "ndp\clr\src".
                     env.bldType = clrenv's flavor or 'chk' (if running outside of clrenv).
                     env.bldArch = clrenv's target arch or current command window arch (if running outside
                                   of clrenv).
               Note: env modificators are parsed by code:#EnvModificatorsParsing.

  #Modificators
  The modificators are:
      bldType   : Build flavor (chk, dbg, fre, ret, corechk, ...).
      bldArch   : Architecture (x86, amd64, ia64, arm).
      bldRelDir : Directory scope (ndp, ndp-clr-src), used e.g. for build and setup parameters.
*/
function _queueModificators(explicit, cmdLine, env) {
    this.explicit = explicit;
    this.cmdLine = cmdLine;
    this.env = env;
}

// Create a copy of object with new copy of env member
function _queueModificators_CloneEnv(mods) {
    var env = new Object();
    memberWiseCopy(env, mods.env);
    return new _queueModificators(mods.explicit, mods.cmdLine, env);
}

// Helper to return first defined value out of 3 (possibly undefined) values.
function _GetFirstDefined(value1, value2, value3) {
    if (value1 != undefined)
        return value1;
    if (value2 != undefined)
        return value2;
    return value3;
}

function _queueModificators_GetBldType(mods) {
    return _GetFirstDefined(mods.explicit.bldType, mods.cmdLine.bldType, mods.env.bldType);
}
function _queueModificators_GetBldArch(mods) {
    return _GetFirstDefined(mods.explicit.bldArch, mods.cmdLine.bldArch, mods.env.bldArch);
}
function _queueModificators_GetBldRelDir(mods) {
    return _GetFirstDefined(mods.explicit.bldRelDir, mods.cmdLine.bldRelDir, mods.env.bldRelDir);
}

// Sets env.bldType to bldType value, but keeps original core prefix of env.bldType.
// Example:
//     mods.env.bldType = "coreret"
//     bldType = "dbg"
//   Returns: ret.env.bldType = "coredbg"
function _queueModificators_SetEnvBldType_KeepCore(mods, bldType) {
    if (isCoreCLRBuild(bldType))
        throw new Error(1, "Cannot set CoreCLR bldType here, the core prefix is kept from previous value");

    mods = _queueModificators_CloneEnv(mods);
    if (isCoreCLRBuild(mods.env.bldType)) {
        mods.env.bldType = "core" + bldType;
    } else {
        mods.env.bldType = bldType;
    }
    return mods;
}

/*
  #AliasImplementation

  Alias implementation functions - description of parameters of _queueTasks_* functions.

  Parameters:
    name : Name of the original alias used on the command line. Does not include modificators of the alias
           (set by dot-syntax).
    mods : Modificators - object with 3 sets of modificators - explicit, cmdLine, env (see
           code:_queueModificators).
*/
function _queueTasks_quickDevRun(name, mods) {
    // Ignore environment build flavor/type, use just explicit and command line bldType or use [core]chk
    mods = _queueModificators_SetEnvBldType_KeepCore(mods, "chk");
    return _quickDevRunTask(
        _queueModificators_GetBldType(mods),
        _queueModificators_GetBldArch(mods),
        _queueModificators_GetBldRelDir(mods));
}
function _queueTasks_scanRuntime(name, mods) {
    // Ignore environment build flavor/type, use just explicit and command line bldType or use [core]chk
    mods = _queueModificators_SetEnvBldType_KeepCore(mods, "chk");
    return _scanRuntimeTask(
        _queueModificators_GetBldType(mods),
        _queueModificators_GetBldRelDir(mods));
}
// Runs Desktop CLR DDR
function _queueTasks_dailyDevRun(name, mods) {
    // Ignore environment architecture, use just explicit and command line bldArch
    mods = _queueModificators_CloneEnv(mods);
    mods.env.bldArch = "non-x86";
    if (_queueModificators_GetBldArch(mods) == "x86") {
        return _taskDailyDevRun;
    } else {
        return _taskDailyDevRunFull64;
    }
}
function _queueTasks_build(name, mods) {
    return _razzleBuildTask(
        _queueModificators_GetBldType(mods),
        _queueModificators_GetBldArch(mods),
        _queueModificators_GetBldRelDir(mods));
}

/****************************************************************************/
/* Add custom alias for code:queue command.
   Should be used from ndp\clr\bin\scriptLib\user\%USERNAME%\runjs.local.js.

  Parameters:
    alias : Alias object returned by code:_queueAlias function.

  Example of usage from runjs.local.js:
    _queueAliasAdd(_queueAlias("myDDR", _taskDailyDevRun));
*/
function _queueAliasAdd(alias) {
    _queueAliases.push(alias);
}

/****************************************************************************/
/* Helper functions for finding alias for code:queue command and for parsing alias parameters.
*/
function _queue_FindAliasName(name) {
    name = name.toLowerCase();
    for (var i = 0; i < _queueAliases.length; i++) {
        var alias = _queueAliases[i];
        if (alias.name.toLowerCase() == name) {
            if (alias.forwardToAlias != undefined) {
                return _queue_FindAlias(alias.forwardToAlias);
            }
            return alias;
        }
    }
    return undefined;
}

/* Parse known modificators (code:#Modificators).
  Returns:
    true   ... If all names are recognized as modificators.
    false  ... If at least one name in names array is not valid modificator.
               Note: storage may be partially modified if names has more than 1 element.
    throws ... If a duplicate modificator was specified, or
               if throwIfDefined = true and some modificator in names is already defined in storage.

  Parameters:
    names   : Array of modificator names to be parsed.
    storage : Modificators will be set here.
    throwIfDefined : Should the method throw for modificators already defined in storage?
*/
function _queue_ParseModificators(names, storage, throwIfDefined) {

    if (throwIfDefined == undefined)
        throwIfDefined = true;

    // Keep track of already defined modificators (to avoid duplicates)
    var alreadySpecifiedModificators = new Object();
    if (throwIfDefined)
        memberWiseCopy(alreadySpecifiedModificators, storage);

    var arches = ["x86", "amd64", "ia64", "arm"];

    for (var i in names)
    {
        var value = names[i];
        var modificator;

        if (_IsMemberOf(value, ["ndp", "ndp-clr-src", "ndp\\clr\\src"]))
        {
            value.replace(/\\/g, "-");
            modificator = "bldRelDir";
        }
        else if (_IsMemberOf(value, ["chk", "dbg", "fre", "ret", "corechk", "coredbg", "corefre", "coreret"]))
        {
            modificator = "bldType";
        }
        else
        {
            var archFound = false;
            // Check for ArchType syntax (e.g. x86chk) without dot
            for (var i in arches)
            {
                if (_StartsWith(value, arches[i]))
                {
                    if (value.length != arches[i].length)
                    {
                        // It could be architecture prefix followed by bldType
                        var bldType = value.substr(arches[i].length);
                        if (!_queue_ParseModificators([bldType], storage, throwIfDefined))
                        {
                            return false;
                        }
                    }
                    value = arches[i];
                    modificator = "bldArch";
                    archFound = true;
                    break;
                }
            }

            if (!archFound)
            {
                // Modificator was not recognized
                return false;
            }
        }

        if (alreadySpecifiedModificators[modificator] != undefined)
            throw new Error(1, "_queue_ParseModificators: " + modificator + " already defined");

        storage[modificator] = value;
        // Keep track of duplicates
        alreadySpecifiedModificators[modificator] = value;
    }
    return true;
}

function _IsMemberOf(value, array) {
    for (var i in array) {
        if (array[i] == value)
            return true;
    }
    return false;
}

function _StartsWith(value, prefix) {
    if (value.substr(0, prefix.length) == prefix)
        return true;
    return false;
}

/****************************************************************************/
/* Finds alias for code:queue command.
*/
function _queue_FindAlias(name) {

    var alias = _queue_FindAliasName(name);
    if (alias == undefined) {
        // Parse parameters and try that
        var nameParts = name.split(".");
        if (nameParts.length <= 1)
            return undefined;

        var aliasName = nameParts.shift;
        alias = _queue_FindAliasName(nameParts[0]);
        if (alias == undefined)
            return undefined;
        nameParts = nameParts.slice(1);

        var params = new Object();
        if (!_queue_ParseModificators(nameParts, params))
            throw new Error(1, "Unknown alias modificator in " + name);

        var newAlias = new Object();
        memberWiseCopy(newAlias, alias);
        newAlias.params = params;
        return newAlias;
    }
    return alias;
}
