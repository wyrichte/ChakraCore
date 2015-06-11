function run_sniffs(szAlias) {

    if (!isElevated())
    {
        throw new Error(1, "dailyDevRun: Requires executing with elevated privilieges");
    }

    var srcBase = srcBaseFromScript();
    var reportName = "";
    var result = 0;

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

        if (szAlias == undefined)
            szAlias = "JIT";
        result = copySelfHostTests(szAlias, undefined, testDir);
        WScript.Echo("copySelfHostTest returned " + result);
        result = _runSelfHost(szAlias, testDir, undefined, srcBase + "\\ddsuites");
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

        if (szAlias == undefined || szAlias == "JIT")
            result = doRun("Jit64_Run_Sniffs");
        else if (szAlias == "NGEN")
            result = doRun("Jit64_Run_Sniffs_NGEN");
        else if (szAlias == "JIT;NGEN")
            result = doRun("Jit64_Run_Sniffs_JIT_NGEN");
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


function _jit64SelfHostCopyAndRun(aliasName) {

    if (aliasName == undefined)
        aliasName = "JIT";

    return _createSelfHostTaskFromAlias(aliasName, false);
}    
        
function _jit32SelfHostCopyAndRun(aliasName) {

    if (aliasName == undefined)
        aliasName = "JIT";

    return _createSelfHostTaskFromAlias(aliasName, false, "x86");
}    
        
var _taskRunSniffs = 
        // This is the task we expect most people to use in their morning runs.  If you change this,
        // make sure you change dailyDevRunDevBVTLast also.
    taskGroup("Jit64_Run_Sniffs", [
        _razzleBuildTask("chk", "ia64",  "ndp"),
        _taskDailyDevRun,
        _jit64SelfHostCopyAndRun(),

    ], 
    "Jit64DailyDevRun is DDR + JIT64 self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskRunSniffs32 = 
        // This is the task we expect most people to use in their morning runs.  If you change this,
        // make sure you change dailyDevRunDevBVTLast also.
    taskGroup("Jit32_Run_Sniffs", [
        _taskDailyDevRun,
        _jit32SelfHostCopyAndRun(),

    ], 
    "Jit32DailyDevRun is DDR + JIT32 self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskRunSniffsAll = 
        // This is the task we expect most people to use in their morning runs.  If you change this,
        // make sure you change dailyDevRunDevBVTLast also.
    taskGroup("JitAll_Run_Sniffs", [
        _razzleBuildTask("chk", "ia64",  "ndp"),
        _taskDailyDevRun,
        _jit64SelfHostCopyAndRun(),
        _jit32SelfHostCopyAndRun(),
    ], 
    "Jit64DailyDevRun is DDR + JIT64 self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskRunSniffsNgen = 
        // This is the task we expect most people to use in their morning runs.  If you change this,
        // make sure you change dailyDevRunDevBVTLast also.
    taskGroup("Jit64_Run_Sniffs_NGEN", [
        _razzleBuildTask("chk", "ia64",  "ndp"),
        _taskDailyDevRun,
        _jit64SelfHostCopyAndRun("NGEN"),

    ], 
    "Jit64DailyDevRun is DDR + NGEN self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

var _taskRunSniffsBoth = 
        // This is the task we expect most people to use in their morning runs.  If you change this,
        // make sure you change dailyDevRunDevBVTLast also.
    taskGroup("Jit64_Run_Sniffs_JIT_NGEN", [
        _razzleBuildTask("chk", "ia64",  "ndp"),
        _taskDailyDevRun,
        _jit64SelfHostCopyAndRun("JIT;NGEN"),

    ], 
    "Jit64DailyDevRun is DDR + JIT & NGEN self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");


_taskAdd(_taskRunSniffs);
_taskAdd(_taskRunSniffs32);
_taskAdd(_taskRunSniffsAll);
_taskAdd(_taskRunSniffsNgen);
_taskAdd(_taskRunSniffsBoth);

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
