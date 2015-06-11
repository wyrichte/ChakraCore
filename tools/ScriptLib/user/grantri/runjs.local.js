// Turn on Maximum logging for everything
//logSetFacilityLevelByPat(".*", 9);

function run_sniffs(szAlias) {

    if (!isElevated())
    {
        throw new Error(1, "dailyDevRun: Requires executing with elevated privilieges");
    }

    var srcBase = srcBaseFromScript();
    var reportName = "";
    var result = 0;

    if (szAlias == undefined)
        szAlias = "JIT";

    if (Env("_NTBINDIR") == "")
    {
        // If we're running this from a share, it means we don't
        // have an enlistment to build and run the full DDR
        // so just run the self host tests

        var testDir = "c:\\tests\\self_host_tests_" + Env("PROCESSOR_ARCHITECTURE");
        reportName = testDir + "\\Smarty.run.0\\Smarty.rpt.0.html";
        WScript.Echo("*****************************************************************************");
        WScript.Echo("*                        Starting run_sniffs");
        WScript.Echo("* Report: " + reportName);
        WScript.Echo("* The report can be viewed before the run is complete");
        WScript.Echo("*****************************************************************************");
        WScript.Echo("");

        result = copySelfHostTests(szAlias, undefined, testDir);
        WScript.Echo("copySelfHostTest returned " + result);
        result = _runSelfHost(szAlias, testDir, undefined, srcBase + "\\ddsuites",
                              undefined, undefined, undefined, undefined, false); // Monitor doesn't work so well remotely
        WScript.Echo("runSelfHost returned " + result);
    }
    else
    {
        reportName = srcBase + "\\automation\\run.current\\taskReport.html";
        WScript.Echo("*****************************************************************************");
        WScript.Echo("*                        Starting run_sniffs " + szAlias);
        WScript.Echo("*");
        WScript.Echo("* Report: " + reportName);
        WScript.Echo("* The report can be viewed before the run is complete");
        WScript.Echo("*****************************************************************************");
        WScript.Echo("");

        if (szAlias == "JIT")
            result = doRun("DDR_And_x64JitSelfHost");
        else if (szAlias == "NGEN")
            result = doRun("Jit64_Run_Sniffs_NGEN");
        else if (szAlias == "JIT;NGEN")
            result = doRun("Jit64_Run_Sniffs_JIT_NGEN");
        else if (szAlias == "JIT32")
            result = doRun("DDR_And_x86JitSelfHost");
        else if (szAlias == "JIT3264")
            result = doRun("DDR_And_JitSelfHost");
        else if (szAlias == "ARMJIT")
            result = doRun("DDR_SOC_JitSelfHost");
        else
            throw new Error(1, "run_sniffs: unrecognized test alias '" + szAlias + "'");
    }

    if (FSOFileExists(reportName)) {
        var to = Env("USERNAME") + "@microsoft.com";
        var subject = "Automail: run_sniffs Report for " + srcBase + "@" + Env("COMPUTERNAME");
        var body = "-x " + reportName;
        mailSendHtml(to, subject, body);
    }


    return result;
}

var myArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

function _jitSelfHostCopyAndRun(aliasName, dependOnSetup, arch, copyTests) {

    if (aliasName == undefined)
        aliasName = "JIT";
    if (arch == undefined)
        arch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    return _createSelfHostTaskFromAlias(aliasName, arch, undefined, copyTests, dependOnSetup, false);
}    

var _taskRunSniffsNgen = 
    taskGroup("Jit64_Run_Sniffs_NGEN", [
        _taskDailyDevRun,
        _jitSelfHostCopyAndRun("NGEN", true, "amd64"),

    ], 
    "Jit64DailyDevRun is DDR + NGEN self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskRunSniffsBoth = 
    taskGroup("Jit64_Run_Sniffs_JIT_NGEN", [
        _taskDailyDevRun,
        _jitSelfHostCopyAndRun("JIT;NGEN", true, "amd64"),

    ], 
    "Jit64DailyDevRun is DDR + JIT & NGEN self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskTestOnly = 
    taskGroup("testOnly", [
        _devBVTTask("chk", myArch, undefined, undefined, [ ] ),
        _jitSelfHostCopyAndRun("JIT", false, myArch, false),

    ], 
    "testOnly assumes you've already built and installed your runtime",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskX64QDR = 
    taskGroup("myQDR", 
        _dailyDevRunCommonTasks.concat( [
            _verifyMachineStateTask("amd64", "chk"),
            _devBVTTask("chk", "amd64"),
            _jitSelfHostCopyAndRun("JIT", true, "amd64"),
            _razzleBuildTask("ret", "amd64", "ndp"),
            // GCStress Tests (excluding debugger tests)
            _devBVTTask("corechk", "amd64", undefined, "/inc GC_STRESS /exc Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "amd64") ]),
            // Do the BVT for amd64Chk (excluding GC_STRESS and debugger tests)
            _devBVTTask("corechk", "amd64", undefined, "/exc GC_STRESS;Combined\\DevSvcs\\Debugger", [ _fxpSetupTask("corechk", "amd64") ]),
        ]),
    "For changes to x64-only (not x86, peverify, reader, ia64, or shared code)",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskARMQDR = 
    taskGroup("armJITQDR", [
             _beagleBVTTask("coredbg"),
             _beagleBVTTask("corechk"),
             _beagleBVTTask("corefre"),
         ],
    "For changes to arm JIT only (not x86, x64, or shared code), only runs arm builds and beagle BVTS",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _canRunArm = !nsIsNullOrEmpty(Env("UseHarmony"));

var _taskDDR_SOC_JITSH =
    taskGroup("DDR_SOC_JitSelfHost", [
        IF_RUN(_canRunArm, _devBVTTask("chk", "arm")), // Make this run early
        _taskDailyDevRun,
        IF_RUN(_canRunArm, _createSelfHostTaskFromAlias("JIT", "arm", "chk", true, true)),
        _taskNonDailyDevRunSOC,
        _createSelfHostTaskFromAlias("JIT", "x86", "chk", true, true),
        IF_RUN(_canRunAmd64, _createSelfHostTaskFromAlias("JIT", "amd64", "chk", true, true)),
    ], 
    "DDR + extra SOC tests + x86 and x64 JIT self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");


_taskAdd(_devBVTTask("chk", myArch, undefined, "/GCStress /exc Combined\\DevSvcs\\Debugger", [ ] ));

_taskAdd(_taskRunSniffsNgen);
_taskAdd(_taskRunSniffsBoth);
_taskAdd(_taskTestOnly);
_taskAdd(_taskX64QDR);
_taskAdd(_taskARMQDR);
_taskAdd(_taskDDR_SOC_JITSH);

if (_canRunArm) {
    machManAdd(_myMachineMan, Env("UseHarmony"), "arm", 1000*1);
    // Maybe someday - This will add ARM jit self host as the first task to run
    // _taskDDR_And_JitSelfHost.dependents.unshift(_jitSelfHostCopyAndRun("JIT", true, "arm", true));
}


        

function showSdWhereSynced(dir) {
    var syncPoint = sdWhereSynced(dir, undefined);
    WScript.Echo("\tMinChange: " + syncPoint.minChange);
    WScript.Echo("\tMaxChange: " + syncPoint.maxChange);
    WScript.Echo("\tMinChangeTime: " + syncPoint.minChangeTime);
    WScript.Echo("\tMaxChangeTime: " + syncPoint.maxChangeTime);
    return 0;
}


function ShowSyncVM() {
    return showSdWhereSynced(Env("_NTBINDIR") + "\\ndp\\clr\\src\\vm");
}

function ShowSyncJIT() {
    return showSdWhereSynced(Env("_NTBINDIR") + "\\ndp\\clr\\src\\jit64");
}

/*****************************************************************************/
/* Marks the beagle board as being 'in use' by the current computer, thus
   preventing other computers from using it. (causes getBeagleIP to return "")
*/
function lockBeagle() {
    if (FSOFileExists("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt")) {
        var lockMachine = FSOReadFromFile("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt");
        if (lockMachine != "" && lockMachine != Env("COMPUTERNAME")) {
            logMsg(LogBeagle, LogError, "Can't lock beagle board because it is already locked by \'", lockMachine, "\'\n");
            return -1;
        }
    }

    FSOWriteToFile(Env("COMPUTERNAME"), "\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt");
    return 0;
}

/*****************************************************************************/
/* Unmarks the beagle board as being 'in use' by the current computer, thus
   allowing other computers to use it.

     Parameters:
       force     : Set to "FORCE" to break a lock owned by another computer
*/
function unlockBeagle(force) {
    if (force == undefined)
        force = "";

    if (FSOFileExists("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt")) {
        var lockMachine = FSOReadFromFile("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt");
        if (lockMachine != "" && lockMachine != Env("COMPUTERNAME")) {
            if (force == "FORCE") {
                logMsg(LogBeagle, LogWarn, "Overriding lock by \'", lockMachine, "\'\n");
            }
            else {
                logMsg(LogBeagle, LogError, "Can't unlock beagle board because it is locked by \'", lockMachine, "\' (try \'runjs unlock FORCE\')\n");
                return -1;
            }
        }
        FSODeleteFile("\\\\GRANTRI5\\C$\\dd\\BeagleBoard\\scripts\\armlocked.txt", "FORCE");
    }
    return 0;
}

