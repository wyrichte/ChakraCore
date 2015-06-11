/******************************************************************************/
/*                               ClrRolling.js                                */
/******************************************************************************/

/* defines CLR specific rolling tasks, using the rolling infrastructure */

// AUTHOR: Vance Morrison 
// DATE: 11/1/2003
/******************************************************************************/

var clrRollingModuleDefined = 1;                 // Indicate that this module exist
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include ClrAutomation.js");
if (!rollingModuleDefined) throw new Error(1, "Need to include rolling.js");

var LogClrRolling = logNewFacility("clrRolling");

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

// logSetFacilityLevel(LogClrRolling, LogInfo100000);

/**********************************************************************/
/* we give names to all the rolling tasks so that we can run them
   from the command line easily.  This table maps these names to 
   the actual task information */

var _rollingTasks  = [
    _devBVTRollingTask("chk", "x86"),
    _devBVTRollingTask("chk", "amd64"),
    _devBVTRollingTask("chk", "ia64"),
    _WSPerfTestsRollingTask(),
    _stressRollingTask("chk", "x86"),
    _stressRollingTask("chk", "amd64"),
    _stressRollingTask("chk", "ia64"),
    _ddIntRollingTask("chk", "x86"),
    _clrSetupRollingTask("ret", "x86"),
    ];

/**********************************************************************/
function _WSPerfTestsRollingTask() {

    var arch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    var unOptDir = "%rolling.jobDir%\\" + arch + "ret.unopt\\bin";

    var setupTasks = [
        _reportStart(),                                 // update the web page that we have started
        _tfSyncCleanTask("tools"),                      // for BBT
        _tfSyncCleanTask("ndp\\clr\\bin"),              // for clrenv (this is for convinience)
        _tfSyncCleanTask("ndp\\clr\\snap2.4"),          // for working set tests 
        _tfSyncCleanTask("ndp\\inc"),                   // needed by dac update
        _tfSyncCleanTask("ndp\\clr\\src\\DacUpdateDll") // for dac update utilities
    ];

    var binDir = "%outDir%\\" + arch + "ret" + "\\bin";
    var logDir = "%outDir%\\" + arch + "ret" + "\\WSPerfTests";

    var mainTask = _WSPerfTestsTask("ret", arch, binDir, undefined,
                                    [_retailBuildTask("ret", arch, undefined, unOptDir, setupTasks)]);

        // The rolling logic needs to know if the test run did not crash. 
        // This task put the task specific output in a canonical place 
        // This keeps the main rolling logic independent of the task being run. 
    var resultsTaskName = "rolling." + mainTask.name;
    var resultsFile = logDir + "\\ShowFormComplex.vadump.xml";
    var resultsTag = "PERF_RESULTS";
    var resultsTask = taskGroup(resultsTaskName, [        
        mainTask,
        taskNew("checkResults", "runjs _rollingResults " + resultsTag + " " + resultsFile + " %rolling.resultsFile%")]);

    resultsTask.description = "This task is a wrapper for the task " + mainTask.name + ".  The job of the rolling\r\n" +
                              "task simply to wait for the main task to complete, look for the results file\r\n" + 
                              "for that task, and if present and 'sane' to create a canonical results file so the\r\n" + 
                              "rolling process knows that this run does not need to be retried.  This is needed\r\n" +
                              "because multiple worker machines are competing for work to do\r\n";
    return resultsTask;
}

function _rollingSetupTasks(binDir, bldType, bldArch) {
    var setupTasks = [
        _reportStart(),                            // update the web page that we have started
        _tfSyncCleanTask("ddsuites"),    // Sync up the tests to %srcBase% using a special SD enlistment
        
            // We need the managed toolset and Vulcan in order to test incremental BBT (see DevUnit\JIT\BBT\Incremental for details)
        _tfSyncCleanTask("tools"),                
        _tfSyncCleanTask("ndp\\clr\\bin"),        // sync up for clrenv (this is for convinience)
        _clrSetupTask(bldType, bldArch, binDir, "/fx /az")
    ];
    
    return setupTasks;
}

function _rollingResultsTask(resultsFile, resultsTag, mainTask, usersToNotify) {
    if (usersToNotify == undefined)
        usersToNotify = "";

    // The rolling logic needs to know if the test run did not crash. 
    // This task put the task specific output (SMARTY in this case), in a canonical place (rolling.test.devBVT@x86chk.Results.txt)
    // This keeps the main rolling logic independent of the task being run. 
    var resultsTaskName = "rolling." + mainTask.name;
    var resultsTask = taskGroup(resultsTaskName, [        
        mainTask,
        taskNew("checkResults", "runjs _rollingResults " + resultsTag + " " + resultsFile + " %rolling.resultsFile% " + usersToNotify)]);
    
    resultsTask.description = "This task is a wrapper for the task " + mainTask.name + ".  The job of the rolling\r\n" +
                              "task simply to wait for the main task to complete, look for the results file\r\n" + 
                              "for that task, and if present and 'sane' to create a canonical results file so the\r\n" + 
                              "rolling process knows that this run does not need to be retried.  This is needed\r\n" +
                              "because multiple worker machines are competing for work to do\r\n";
                              
    return resultsTask;                              
}

/**********************************************************************/
/* This is the task for the rolling stress. Sync the ddsuites\src\clr 
   directory to a local directory, do a clrsetup of the given build, 
   and run devBVTs.  
*/

function _stressRollingTask(bldType, bldArch) {

    // Setup task
    var binDir = "%rolling.jobDir%\\" + bldArch + bldType + "\\bin";    
    var setupTasks = _rollingSetupTasks(binDir, bldType, bldArch);
    
    var mainTask = _clrStressTask(bldType, bldArch, "120", binDir, setupTasks);
    
    // Results task
    var resultsFile = "%outDir%\\" + bldArch + bldType + "\\test.clrStress\\clrStressResults.txt";
    var usersToNotify = "leculver@microsoft.com";

    var resultsTask = _rollingResultsTask(resultsFile, "STRESS_RESULTS", mainTask, usersToNotify);
    
    return resultsTask;
}

function _clrSetupRollingTask(bldType, bldArch) {
    var relOutDir = bldArch + bldType;
    var binDir = "%rolling.jobDir%\\binaries." + bldArch + bldType;

    var mainTask = _clrSetupTask(bldType,bldArch, binDir, "/fx /nrad");
    mainTask.name = "rollingClrSetup@" + bldArch + bldType;

    var resultsFile = "%outDir%\\" + relOutDir + "\\ndpsetup.log";    
    var usersToNotify = "clrinteg@microsoft.com";

    var resultsTaskName = "rolling." + mainTask.name;

    var resultsTask = taskGroup(resultsTaskName, [        
        mainTask,
        taskNew("checkResults", "runjs _rollingClrSetupResults NDPSETUP_RESULTS " + resultsFile + " %rolling.resultsFile% " + usersToNotify)]);
    
    resultsTask.description = "This task is a wrapper for the task " + mainTask.name + ".  The job of the rolling\r\n" +
                              "task simply to wait for the main task to complete, look for the results file\r\n" + 
                              "for that task, and if present and 'sane' to create a canonical results file so the\r\n" + 
                              "rolling process knows that this run does not need to be retried.  This is needed\r\n" +
                              "because multiple worker machines are competing for work to do\r\n";
                              
    return resultsTask;                 
}

/**********************************************************************/
/* This is the task for the rolling devBVTs. Sync the ddsuites\src\clr 
   directory to a local directory, do a clrsetup of the given build, 
   and run devBVTs.  
*/
function _devBVTRollingTask(bldType, bldArch) {            
    // Setup task
    var binDir = "%rolling.jobDir%\\" + bldArch + bldType + "\\bin";    
    var setupTasks = _rollingSetupTasks(binDir, bldType, bldArch);
            
    var mainTask = _devBVTTask(bldType, bldArch, binDir, "/clean /workers:2", setupTasks);
    
    var resultsFile = "%outDir%\\" + bldArch + bldType + "\\test.devBVT\\Smarty.XML";    
    var resultsTask = _rollingResultsTask(resultsFile, "SMARTY_RESULTS", mainTask)
    
    return resultsTask;
}

/**********************************************************************/
/* This is the task for the rolling ddIntFx. Sync the ddsuites 
   directory to a local directory, do a clrsetup of the given build, 
   and run DDIntFx.  
*/

function _ddIntRollingTask(bldType, bldArch) {            
    // Setup task
    var relOutDir = bldArch + bldType;
    var binDir = "%rolling.jobDir%\\" + relOutDir + "\\bin";    
    var pubDir = "%rolling.jobDir%\\" + relOutDir + "\\public";    
    var srcBase = "%srcBase%"; //srcBaseFromScript();
    var localBinDir = srcBase + "\\binaries\\" + relOutDir;
    var suiteBinDir = localBinDir + "\\suitebin";
    var trunStartDir = "src\\fx";
    
    var setupTasks = [
            taskGroup("rollingSetup", _rollingSetupTasks(binDir, bldType, bldArch)),
            taskGroup("ddIntBuild", [
                taskGroup("ddIntBuildPreProcess", [
                    // This is less than optimal, ddsuites\src\fx has a makefile dependency 
                    // on ndp\* and ndp\fx\*. Ideally, we need to sync only these folders and
                    // not the whole tree rooted underneath them. But 'sdSync' function is
                    // not factored to deal with syncing just one folder or for that matter 
                    // just one file. It is hardcoded to sync with givenPath + "\\...@CL".
                    // So, for now sync the whole ndp tree. It shouldn't be too bad for 
                    // incremental sync though (but unecessary load on SD)...
                    _tfSyncCleanTask("ndp"),
        
                    // We probably can get away by doing just some subset of public cache...
                    _tfSyncCleanTask("public"),     
        
                    // DDsuite has build dependency in the \public\internal\NDP\ref\v2.0\ and 
                    // \public\sdk\NDP\ref\v2.0\ folders that gets updated locally when you build NDP. 
                    // These binaries are either not checked into SD or we may not pick up the right 
                    // version of the file if the checkin affected these binaries. So we need to copy 
                    // the checkin job's public folder locally to overwrite cached binaries.
                    
                    // Robocopy_task_note: Specify /NP (no progress) arg as optional arg to get around the terrible default of /PURGE inserted by the robocopy function
                    taskNew("publicRefPatch", "runJs robocopy " + pubDir + " " + srcBase + "\\public"  + " /NP "),
                ]),
                // Now we should be ready to build ddsuites
                _razzleBuildTask(bldType, bldArch, "ddsuites\\" + trunStartDir, undefined, undefined, localBinDir),
                
                // Copy tests to the job folder for reference. 
                // Robocopy_task_note: Specify /NP (no progress) arg as optional arg to get around the terrible default of /PURGE inserted by the robocopy function
                taskNew("ddIntBuildPostProcess", "runJs robocopy " + localBinDir + " " + binDir  + " /NP "),
            ]),
            _ddIntIESetupTask(),
        ];

    var mainTask = _ddIntTask(bldType, bldArch, binDir, trunStartDir, undefined, setupTasks, suiteBinDir);
    
    var taskNamePrefix = "test.ddInt." +  trunStartDir.replace(/[\\]/g, "-");
    var relOutDir = bldArch + bldType;
    var logDir = "%outDir%\\" + relOutDir + "\\" + taskNamePrefix;

    var resultsFile = logDir + "\\trun.xml";    
    var usersToNotify = "rkrish@microsoft.com";
    var resultsTask = _rollingResultsTask(resultsFile, "TRUN_RESULTS", mainTask, usersToNotify);
      
    return resultsTask;
}

/**********************************************************************/
/********                   INFRASTRUCTURE                   **********/
/**********************************************************************/

/**********************************************************************/
/* given a task name for a rolling task, find the task.  Throws if it 
   is not found */

function _rollingFindTask(name, tasks) {

    if (tasks == undefined) {
        tasks = _rollingTasks;
        if (tasks == undefined) 
            throw new Error (1, "_rollingTasks undefined, this means that _rollingTasks initialization failed");
    }
    logMsg(LogClrRolling, LogInfo10, "rollingFindTask(", name, ", ", tasks, ")\n");
    
    for (var i = 0; i < tasks.length; i++) {
        var task = tasks[i];
        if (task == undefined)
            continue;
        logMsg(LogClrRolling, LogInfo100, "rollingFindTask: looking for ", name, " found ", task.name, "\n");
        if (task.name == name)
            return task;
    }
    throw new Error(1, "Could not find rolling task " + name + " from _rollingTasks{" + _rollingTasks + "}");
}

/**********************************************************************/
/* Syncs a relative directory to the given (relative to the root of the source tree). */
function _tfSyncCleanTask(relDir)
{
    var taskName = "tfSyncClean." + relDir.replace(/[\\]/g, "-");
    var ret = taskNew(taskName, "runjs tfSyncClean '" + relDir + "' '%rolling.changeNum%' '%srcBase%'");
    ret.description = "This task syncronizes the directory " + relDir  + " in the source base to that in the depot.";
    return ret;
}

/**********************************************************************/
function tfSyncClean(relPaths, tfChangeNum, localRoot)
{
    if (localRoot == undefined)
        localRoot = Env("_NTDRIVE") + Env("_NTROOT");

    logCall(LogClrAutomation, LogInfo, "tfSyncClean", arguments);
    
    if (typeof(relPaths) == "string")
        relPaths = relPaths.split(";");
    
    for(var i = 0; i < relPaths.length; i++)
    {
        var localDir = localRoot;
        if (relPaths[i] != "")
            localDir += "\\" + relPaths[i];
        
        tfSync(localDir, tfChangeNum);
    }

    return 0;
}

/**********************************************************************/
/* Syncs localDir to the given change number.  If tfChangeNum is unset, this function syncs to the
   most recent version of the file or directory. */
function tfSync(localDir, tfChangeNum)
{
    var versionSpec = "";
    if (tfChangeNum != undefined &&
        tfChangeNum != '')
        versionSpec = "/version:C" + tfChangeNum + " ";
    
    var tfCmd = Env("_NTDRIVE") + Env("_NTROOT") + "\\tools\\x86\\managed\\v4.0\\tf.cmd";
    runCmd(tfCmd + " get /recursive " + versionSpec + localDir);
}

/**********************************************************************/
/* A trivial helper that is meant to be called at the end of the rolling
   process.  Its job is to see if the task produced good information 
   (there was no infrastructure failure).  This routine looks for the 
   file 'testResultFile' (which was generated by the user defined task)
   If it exists, it assumes that the test pass was successful (the harness
   completed, there may be failures however).  It then logs this fact in
   the file 'resultsFileName' in the tag 'tag' */

function _rollingResults(tag, testResultFile, resultsFileName, usersToNotify) {

    logMsg(LogClrRolling, LogInfo10, "_rollingResults(", tag, ", ", testResultFile, ", ", resultsFileName, ")\n");
    
    var ret = 1; // assume failure

        // did we get test results?  If so mark the fact and don't rerun the harness
    if (FSOFileExists(testResultFile)) {
        FSOWriteToFile(tag + ": " + testResultFile + "\r\n", resultsFileName, true);
        ret = 0;

        if (usersToNotify != undefined) {
            logMsg(LogClrRolling, LogInfo, "_rollingResults: Notifying ", usersToNotify, "\n");

            // TODO we really need the rolling results task to more formally 'attached' to the
            // task it monitors
            try {
                if (resultsFileName.match(/^(.*)\\rolling.(.*).results.txt/i)) {
                    var outDir = RegExp.$1;
                    var taskName = RegExp.$2;
                    // logMsg(LogClrRolling, LogInfo, "_rollingResults: outDir:", outDir, " task ", taskName, "\n");
                    var statusFileNames = FSOGetFilePattern(outDir, new RegExp("^" + taskName + ".\\d+.status.log$", "i"));
                    if (statusFileNames.length > 0) {
                        var statusFileName = statusFileNames[statusFileNames.length-1];
                        // logMsg(LogClrRolling, LogInfo, "_rollingResults: statusName: ", statusFileName, "\n");

                        var status = parseFile(statusFileName);
                        // logMsg(LogClrRolling, LogInfo, "_rollingResults: status: ", dump(status), "\n");

                        if (!status.result || !status.result.match(/SUCCESS/i)) {
                            var taskReport = outDir + "\\taskReport.html";

                            var message = ""
                            message += "A rolling test run has failed <br>\r\n";
                            message += "Date: " + new Date() + " <br>\r\n";
                            message += "Failed Task: " + taskName + " <br>\r\n";
                            message += "Output Dir: " + outDir + " <br>\r\n";
                            if (FSOFileExists(taskReport)) 
                                message += "taskReport: <A HREF='" + taskReport + "'>" + taskReport + " </A> <br>\r\n";
                            logMsg(LogClrRolling, LogInfo, "_rollingResults: sending mail to : ", usersToNotify, "\n");
                            mailSendHtml(usersToNotify, "Rolling test run failed", message);
                        }
                    }
                }
            }
            catch(e) {
                logMsg(LogClrRolling, LogError, "_rollingResults: exception during mail: ", e.description, "\n");
            }
        }
    }
    else 
        logMsg(LogClrRolling, LogError, "_rollingResults: Could not find results ", testResultFile, "\n");

    return ret;
}

/**********************************************************************/
/* An NDPSetup helper that is meant to be called at the end of the rolling
   process.  Its job is to see if the task produced good information 
   (there was no infrastructure failure).  This routine looks for the 
   file 'testResultFile' (which was generated by the user defined task)
   If it exists, it assumes that the test pass was successful (the harness
   completed, there may be failures however).  It then logs this fact in
   the file 'resultsFileName' in the tag 'tag' */

function _rollingClrSetupResults(tag, testResultFile, resultsFileName, usersToNotify) {

    logMsg(LogClrRolling, LogInfo10, "_rollingResults(", tag, ", ", testResultFile, ", ", resultsFileName, ")\n");
    
    var ret = 1; // assume failure

        // did we get test results?  If so mark the fact and don't rerun the harness
    if (FSOFileExists(testResultFile)) {
        FSOWriteToFile(tag + ": " + testResultFile + "\r\n", resultsFileName, true);
        ret = 0;

        if (usersToNotify != undefined) {
            logMsg(LogClrRolling, LogInfo, "_rollingResults: Notifying ", usersToNotify, "\n");

            try {
                if (resultsFileName.match(/^(.*)\\rolling.(.*).results.txt/i)) {
                    var outDir = RegExp.$1;
                    var taskName = RegExp.$2;

                    var statusFileNames = FSOGetFilePattern(outDir, new RegExp("^" + taskName + ".\\d+.status.log$", "i"));
                    if (statusFileNames.length > 0) {
                        var statusFileName = statusFileNames[statusFileNames.length-1];

                        var status = parseFile(statusFileName);

                        if (!status.result || !status.result.match(/SUCCESS/i)) {
                            var taskReport = outDir + "\\taskReport.html";
                            var message = ""
                            message += "A rolling test run has <b style='color:Red'>failed!</b><br>\r\n<br>\r\n";
                            message += "This message is sent from a rolling system running on " + Env("COMPUTERNAME") + " which is still under development<br>\r\n";
                            message += "The intent of this system is to ensure that NDPSetup is not broken.  This rolling run will begin execution<br>\r\n";
                            message += "every day at 9 am.<br>\r\n<br>\r\n";
                            message += "Date: " + new Date() + " <br>\r\n";
                            message += "Failed Task: " + taskName + " <br>\r\n";
                            if (FSOFileExists(testResultFile))
                                message += "NDPSetup log: " + testResultFile + " <br>\r\n";
                            message += "Output Dir: " + outDir + " <br>\r\n";
                            if (FSOFileExists(taskReport)) 
                                message += "taskReport: <A HREF='" + taskReport + "'>" + taskReport + " </A> <br>\r\n";
                            if (FSOFileExists(testResultFile)) {
                                message += "<br>\r\n NdpSetup Errors <br>\r\n";
                                message += "--------------<br>\r\n";

                                var ndpsetupContents = FSOReadFromFile(testResultFile);

                                var lines = ndpsetupContents.split("\n");
                                for(var j = 0; j < lines.length; j++) {
                                    if (lines[j].match(/^.*ndpsetup\.js\: error.*/i)) {
                                        message += lines[j] + "<br>\r\n";
                                    }
                                }
                            }
                            logMsg(LogClrRolling, LogInfo, "_rollingResults: sending mail to : ", usersToNotify, "\n");
                            mailSendHtml(usersToNotify, "Rolling test run results", message);
                        }
                        else if(status.result.match(/SUCCESS/i)) {
                            var taskReport = outDir + "\\taskReport.html";
                            var message = ""
                            message += "A rolling test run has completed <b style='color:Green'>successfully!</b><br>\r\n<br>\r\n";
                            message += "This message is sent from a rolling system running on " + Env("COMPUTERNAME") + " which is still under development<br>\r\n";
                            message += "The intent of this system is to ensure that NDPSetup is not broken.  This rolling run will begin execution<br>\r\n";
                            message += "every day at 9 am.<br>\r\n<br>\r\n";
                            message += "Date: " + new Date() + " <br>\r\n";
                            message += "Task: " + taskName + " <br>\r\n";
                            if (FSOFileExists(testResultFile))
                                message += "NDPSetup log: " + testResultFile + " <br>\r\n";
                            message += "Output Dir: " + outDir + " <br>\r\n";
                            if (FSOFileExists(taskReport)) 
                                message += "taskReport: <A HREF='" + taskReport + "'>" + taskReport + " </A> <br>\r\n";

                            logMsg(LogClrRolling, LogInfo, "_rollingResults: sending mail to : ", usersToNotify, "\n");
                            mailSendHtml(usersToNotify, "Rolling test run results", message);
                        }
                    }
                }
            }
            catch(e) {
                logMsg(LogClrRolling, LogError, "_rollingResults: exception during mail: ", e.description, "\n");
            }
        }
    }
    else 
        logMsg(LogClrRolling, LogError, "_rollingResults: Could not find results ", testResultFile, "\n");

    return ret;
}

/**********************************************************************/
/* a task that just forces a _rollingTestReport to be made.  This is
   useful to do right after a rolling run has started so that the 
   web page shows this fact */

function _reportStart() {

    return taskNew("rollingReport", "runjs _rollingTestReport %rolling.outDirBase% %rolling.jobDirBase%");
}

