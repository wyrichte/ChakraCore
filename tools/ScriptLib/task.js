/****************************************************************************/
/*                                task.js                                   */
/****************************************************************************/
/* 
    OK, you have a script or (ack batch file), that does some operation.
    What makes it a task?  A task is simply a command that has been also
    tagged with additional information that allows automation to add value.
    Today, tasks have the following attributes:

        name:           Something to identify the task
        cmd:            The command to run
        dependents:     Any tasks that need to be sucessfully run for this task 
                        to run (usually something that creates an input for this task)
    
    They will likely have more information tacked on them as time goes on
    (what kind of machine they need, priority, ...)  

    The added value you get from making something a task:

        1) It creates log files and HTML reports. (This is the most obvious benefit.)
        2) You can run many tasks that are logically independent of
           one another (a failure of one will allow the others to continue).  In 
           particular, the failure of a sibling task does not block the other sibling
           from continuing.
        3) You can refer to them by name, and thus specify them quickly.  The
           doRun command takes a regular expression, and thus you can quickly
           specify many different sets of tasks to run.
        4) By default, tasks that are run in the same location only replay the failed tasks.
           a) To force previous behavior use 'taskClean' 
              use "taskClean=taskClean" in env string or 
              env["taskClean"]="taskClean" from code.
        5) (FUTURE) You can have them dispatched on multiple machines.

    Tasks have the notion of a 'srcBase' and an 'outDir'.  These are not
    specified by the task itself but rather are specified at the time that
    the task is run.   The idea is 'srcBase' is the base directory where ALL
    inputs to the task can be found, and 'outDir' is the place where ALL
    outputs will be put.  This is simple but powerful.  It allows tasks to
    be run on different inputs and put their results in different places so
    the task becomes reusable.  

    To support this, tasks allow cmds to assume 'environment variables' called
    %srcBase% and %outDir%.  Before the task is executed these are filled
    in with the actual values of these directories.  In addition the following
    'variables' are also defined.

        %outDir%            The output directory that is passed to doRollingRun
        %srcBase%           The input directory that is passed to doRollingRun    
        %taskName%          The name of the task associated with the command 
        %statusFile%        The status file associated with the command
    
    When a task runs it always creates two files

        <taskName>.<retry>.output.log    Standard output for the task
        <taskName>.<retry>.status.log    A status file that contains name-value pairs 
                                         that are interesting. 

    The <retry> number is how many times this task was run.  When a task is
    replayed, the old output and status files are not destroyed, but new ones
    are created.   Only a certain number (3 right now) of these are kept, however.
    
    After tasks complete, a report is generated called 'taskReport.html' in 
    %outDir%.  This report tries to make all the information kept by the
    task system sensible.  

    The main APIs from this file are:
    
        taskNew     - create a new task
        taskGroup   - group tasks
        tasksRun    - run a set of tasks and produce a report

    This one may be useful to call directly:

        taskReport  - create a report.  Normally not needed since taskRun does this.
        
*/

// AUTHOR: Vance Morrison 
// DATE: 11/1/2003

/****************************************************************************/

var taskModuleDefined = 1;                     // Indicate that this module exist

if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");

var LogTask = logNewFacility("task");
//logSetFacilityLevel(LogTask, LogInfo100000); //DEBUG - this will log almost everything!
var bugDatabase = "DevDiv Bugs";

/* TODO the doc parser grabs too much without this comment here */

/****************************************************************************/
/* Create a new task.  'name' should be of the form <name>@<relOutDir>.  
   where 'relOutDir' is the directory where output from the task will be
   put. (relative to the source base).  cmd is the command line to run
   dependents is the list of tasks that need to complete before this one
   can run.  'machPat' is a regular expression that will be used to 
   decide what machines this task can run on (optional, if not present it
   means any machine).   
*/
function taskNew(name, cmd, dependents, machPat, description, moreInfoUrl) {

    logMsg(LogTask, LogInfo100, "taskNew(", name, ", ", cmd, ", ", dependents, ")\n");
    var task = {};
    task.objID = "task_" + objIds; objIds++;
    task.name = name;
    task.toString = function() { return this.name; };
    task.cmd = cmd;
    task.dependents = dependents;
    task.typePat = machPat;
    task.description = description;
    task.moreInfoUrl = moreInfoUrl;

    // Note that this creates a NEW task based off an existing task.
    task.concat_dependents = function(more_dependents, new_name, new_description, new_moreInfoUrl)
    {
        var new_task = {};
        memberWiseCopy(new_task, this);
        task.objID = "task_" + objIds; objIds++;
        new_task.dependents = new_task.dependents.concat(more_dependents);
        new_task.name = new_name == undefined ? this.name : new_name;
        new_task.description = new_description == undefined ? this.description : new_description;
        new_task.moreInfoUrl = new_moreInfoUrl == undefined ? this.moreInfoUrl : new_moreInfoUrl;
        return new_task;
    };

    return(task);
}

/****************************************************************************/
/* TaskGroup creates a composite task that simply requires that all its 
   dependents succeed.  
*/
function taskGroup(name, dependents, description, moreInfoUrl) {
    
    return taskNew(name, undefined, dependents, undefined, description, moreInfoUrl);
}

/**************************************************************************/
/* refs allow you to refer to a task that may not exist yet.  Please be
   careful not to introduce loops this way */

function taskRef(name) {
    var task = {};
    task.name = name
    task.toString = function() { return this.name; };
    return task;
}

/**************************************************************************/
/* check if any task in the dependency chain of 'task' can be run and if
   so get them started.  It will return 'true' if 'task' is done (check 
   the exitCode property for result), or 'false' if we need to continue
   to _taskPoll the task.   'visitNum' is used to insure that a task is polled
   only once.  Each time you _taskPoll from the main job task this number
   should be different from any previous call.  _taskPoll uses this to mark 
   tasks as being visited  
*/

function _taskPoll(task, env, parent, machMan, job, visitNum) {

    if (task.endTime) {
        logMsg(LogTask, LogInfo1000, "_taskPoll task done exit = ", task.exitCode, " returning true\n"); 
        return true;
    }

    if (task.visitNum == visitNum) {
        logMsg(LogTask, LogInfo1000, "_taskPoll: task ", task.name, " has been visited already, returning false\n");
        return false
    }
    task.visitNum = visitNum;            /* mark this node as visited */

    logMsg(LogTask, LogInfo1000, "In _taskPoll(", task.name, ", env, ", visitNum, ") {\n"); 
    var done = false;
    if (task.run != undefined) {
        logMsg(LogTask, LogInfo1000, "_taskPoll task running\n"); 
        done = runPoll(task.run);
    }
    else {
        if (!task.outDir) {            /* if outDir not set, we have not inited */
            done = _taskInit(task, env, parent);
            job.nextReportTime = new Date().getTime();
        }

            /* This task has not been started.  Poll all of my dependents
                to see if we can start this one */
        if (!done) {
            logMsg(LogTask, LogInfo1000, "_taskPoll: Checking dependents for task ", task.name, "\n");
            if (!task.readyTime) {
                done = true;
                var depsDoneOK= true;
                if (task.dependents) {
                    for (var i = 0; i < task.dependents.length; i++) {
                        var dep = task.dependents[i];
                        if (dep == undefined) continue;

                        if (_taskPoll(dep, env, task, machMan, job, visitNum)) {
                            logMsg(LogTask, LogInfo1000, "_taskPoll: dependent ", dep.name, " done exit ", dep.exitCode, "\n");
                            if (dep.exitCode != 0) {
                                if (!task.ignoreChildFail)        // run even if children fail
                                    depsDoneOK = false;
                                if (task.eagerAbort) 
                                    _tasksTerminate(task.dependents);
                            }
                        }
                        else
                            done = depsDoneOK = false;
                        if (!task.inclusiveStartTime && dep.inclusiveStartTime) {
                            task.inclusiveStartTime = dep.inclusiveStartTime;
                            var status = "inclusiveStartTime: " + task.inclusiveStartTime + "\r\n";
                            FSOWriteToFile(status, task.fullStatusFileName, true);
                            job.nextReportTime = new Date().getTime();
                        }
                    }
                }
                if (depsDoneOK) {
                    logMsg(LogTask, LogInfo1000, "_taskPoll: ", task.name, " is ready\n");
                    task.readyTime = new Date();
                    var status = "readyTime: " + task.readyTime + "\r\n";
                    FSOWriteToFile(status, task.fullStatusFileName, true);
                    job.nextReportTime = new Date().getTime();
                }
            }
            if (task.readyTime) 
                done = _taskStart(task, env, machMan, job);     /* Start means running the command associated with the task */
        }
    }
    if (done && !task.endTime) {
        _taskEnd(task, machMan);
        job.nextReportTime = new Date().getTime();
    }

    logMsg(LogTask, LogInfo1000, "} _taskPoll = ", done, "\n"); 
    return done;
}

/****************************************************************************/
/* _taskInit gets called when a task is allowed to proceed.  It will not
   run its command immediately because it must wait for its dependents to
   finish first */

function _taskInit(task, env, parent) {

        // Find a unique file to use as status and log file

    task.outDir = _getStableDir(env.outDir);        // Normalize the diretory so that html paths are good
    task.exitCode = undefined;
    task.run = undefined;

    logMsg(LogTask, LogInfo100, "_taskInit: INITIALIZING task ", task, " ID = ", task.objID, " deps = [", 
        joinMap(task.dependents, " ", function(d) { if (d == undefined) throwWithStackTrace(Error(1, "_taskInit: " + task.name + " has a null dependent")); return(d.name) }), "]\n");
    FSOCreatePath(task.outDir);
            // only allow one process to run a particular task simultaneously
    task.lockFileName = task.outDir + "\\" + task.name + ".lock";
    try {
        task.lockFile = FSOOpenTextFile(task.lockFileName, FSOForWriting, true);
        task.lockFile.WriteLine("Locked by " + Env("COMPUTERNAME"));
    } catch(e) {
            // TODO look at exception
        logMsg(LogTask, LogWarn, "_taskInit: Could not open lock file ", task.lockFileName, " assuming another worker grabbed it\n");
        task.exitCode = 249;
        return true;
    }

        // remove old status files (keep only 3)
    var statusNames = FSOGetFilePattern(task.outDir, new RegExp("^" + task.name + "\\.(\\d+)\\.status\\.log$", "i"));
    statusNames.sort(function(x, y) { return (numSuffix(y, "status.log") - numSuffix(x, "status.log")); });
    var successStatusFile = undefined;
    for (var i = 0; i < statusNames.length; i++)  {
        var statusName = statusNames[i];
        if (i < 2) {
            var statusData = FSOReadFromFile(statusName)

            if (!statusData.match(/^result/im))
                FSOWriteToFile("result: FAILED: INCOMPLETE the task automation was killed before completion\r\n", statusName, true);
            else if (statusData.match(/^result: *SUCCESS/im))
                successStatusFile = statusName;
        }
        else {
            logMsg(LogTask, LogInfo100, "Deleting old status file ", statusName, "\n");
            FSODeleteFile(statusName, true);
            var outputName = statusName.replace(/\.status\.log$/i, ".output.log");
            if (FSOFileExists(outputName)) {
                logMsg(LogTask, LogInfo100, "Deleting old output file ", outputName, "\n");
                FSODeleteFile(outputName, true);
            }
        }
    }

        // incremental behavior
    
    if (successStatusFile && (env["taskClean"] == undefined)) {
        task.fullStatusFileName = successStatusFile;
        task.fullLogFileName = successStatusFile.replace(/\.status\.log$/i, ".output.log");
        task.statusFileName = task.fullStatusFileName.match(/([^\\]*)$/)[1]; 
        task.endTime = new Date();
        task.exitCode = 0;
        logMsg(LogTask, LogInfo, "_taskInit: task ", task.name, " has successful status file ", successStatusFile, ".  Skipping.\n");
        return true;
    }

    var baseName = task.outDir + "\\" + task.name;
    var retry = 0;
    if (statusNames.length > 0)
        retry = numSuffix(statusNames[0], "status.log") + 1;
    task.fullLogFileName = baseName + "." + retry + ".output.log";
    task.fullStatusFileName = baseName + "." + retry + ".status.log";
    task.statusFileName = task.fullStatusFileName.match(/([^\\]*)$/)[1];

    var status = "";
    if (task.description) 
        status += "description: " + task.description.replace(/\s*\n/g, "\r\ndescription: ") + "\r\n";
    if (task.moreInfoUrl)
        status += "moreInfoUrl: " + task.moreInfoUrl + "\r\n";

    status += "outDir: " + task.outDir + "\r\n" +
              "status: " + uncPath(task.fullStatusFileName) + "\r\n" + 
              "srcBase: " + env.srcBase + "\r\n";
    if (parent != undefined)
        status += "parent: " + parent.statusFileName + "\r\n";
    FSOWriteToFile(status, task.fullStatusFileName);

    return false;
}

/****************************************************************************/
/* _taskStart gets called when all the dependents have returned succesfully
   and the main task is ready to be called.  Returns true if done (can 
   happen if there is no command to run) */

function _taskStart(task, env, machMan, job) {

    var cmd = task.cmd;
    var done = true;
    var status = "";
    if (cmd != undefined) {
        task.machine = machManFind(machMan, task.typePat, task);
        if (!task.machine) {

            var now = new Date().getTime();
            var deltaMin = (now - task.logStartAttempt) / 60000;
            if (!(deltaMin < 3)) {
                if (task.logStartAttempt) 
                    logMsg(LogTask, LogInfo10, "_taskStart: ", task, " could run if we had more machines\n");
                else 
                    logMsg(LogTask, LogInfo, "_taskStart: ", task, " could run if we had more machines\n");
                task.logStartAttempt = now;
            }
            logMsg(LogTask, LogInfo10000, "_taskStart: task ", machMan.inUse, " is still running, DO NOTHING\n");
            return false;
        }
        else 
            task.machineName = task.machine.name;    // this is so XML reports look nice

        env.statusFileName = task.statusFileName;
        env.taskName = task.name;
        while(cmd.match(/%((\w|\.)+)%/)) {
            var key = RegExp.$1;
            var value = env[key];
            if (value == undefined) {
                // If no value was specified, just use "".  (Useful for %extraSmartyArgs%.)
                value = "";
            }
            cmd = cmd.replace(new RegExp("%" + key + "%", "g"), value);
        }
        status += "command: " + cmd + "\r\n";
        status += "output: " + uncPath(task.fullLogFileName) + "\r\n";
        status += "machine: " + task.machine.name + "\r\n";

        var runOptions = 
            runSetOutput(task.fullLogFileName, 
            runSetLog(LogRun, LogInfo, 
            runSetTimeout(259200,         // timeout of 3 days, the task themselves have to decide this!
            runSetNoThrow())));
        if (task.machine.self)
            runOptions = runSetEnv("PATH", ScriptDir + ";" + runGetEnv("PATH"), runOptions);
        else  {
            runOptions = runSetMachine(task.machine.name, runOptions);

            // FIX NOW this is probably a hack
            newPath = "%PATH%;" + ScriptDir.replace(/\b(\w):\\/g, "\\\\" + Env("COMPUTERNAME") + "\\$1$\\");
            logMsg(LogTask, LogInfo10, "_taskStart: adding to path: ", newPath, "\n");
            runOptions = runSetEnv("PATH", newPath, runOptions);
        }

        task.run = runDo(cmd, runOptions);
        done = runPoll(task.run);        // you have to do a poll to start the process going. 
        WScript.Sleep(100);                // Give the output file a chance to be created, we get a better report
    }

    task.startTime = new Date();
    status += "startTime: " + task.startTime + "\r\n";
    if (!task.inclusiveStartTime) {
        task.inclusiveStartTime = task.startTime;
        status += "inclusiveStartTime: " + task.startTime + "\r\n";
    }

    logMsg(LogTask, LogInfo, "_taskStart: STARTING TASK ", task, " {\n", status, "}\n");
    FSOWriteToFile(status, task.fullStatusFileName, true);
    job.nextReportTime = new Date().getTime();
    return done;
}

/****************************************************************************/
/* _taskEnd is called after the task has completed one way or the other.
   its job is to set the 'endTime' and 'exitCode' field as well as 
   output log files */
 
function _taskEnd(task, machMan) {

    var status = "";
    if (task.run == undefined || task.ignoreChildFail) {

                // Process all the dependent tasks
        var failedTaskNames = [];
        var dependStatusFiles = [];

        if (task.exitCode == 249) 
            status += "result: FAILED command could not open lock file " + task.lockFileName + "\r\n";
        else {
            if (!task.exitCode)
                task.exitCode = 0;
            if (task.dependents != undefined) {
                logMsg(LogTask, LogInfo100, "_taskEnd: processing ", task.dependents.length, " dependents\n");
                for (var i = 0; i < task.dependents.length; i++) {
                    var dependent = task.dependents[i];
                    if (dependent == undefined) continue;

                    if (dependent.exitCode != 0)
                    {
                        // A dependent failed. What kind of failure is it?

                        if (task.exitCode == 0) {
                            // First dependent failure is stored as parent failure code
                            task.exitCode = dependent.exitCode;
                        }
                        else if ((task.exitCode == 2) && (dependent.exitCode != 2)) {
                            // Always degrade code from 2 ("fail with mitigating factors") to
                            // something else.
                            task.exitCode = dependent.exitCode;
                        }
                    }

                    if (dependent.exitCode != 0 && !dependent.terminated) {
                        failedTaskNames.push(dependent.name);
                    }
                    if (dependent.statusFileName != undefined)
                        dependStatusFiles.push(dependent.statusFileName);
                }
                status += "dependents: " + dependStatusFiles.join(' ') + "\r\n";
            }

            if (task.exitCode == 0)
                status += "result: SUCCESS all dependents succeeded\r\n";
            else if (task.exitCode == 2) {
                status += "result: FAILED dependents [" + failedTaskNames.join(", ") + "] failed with special exit code indicating mitigating factors: errorCode: " + task.exitCode + "\r\n";
            } else {
                status += "result: FAILED dependents [" + failedTaskNames.join(", ") + "] failed\r\n";
            }
        }
    }
    else {
        task.exitCode = task.run.exitCode;
        if (task.exitCode == 0)  
            status += "result: SUCCESS command completed successfully\r\n";
        else if (task.run.terminated) 
            status += "result: FAILED command was terminated manually\r\n";
        else if (task.exitCode == 2)
            status += "result: FAILED command failed with special exit code indicating mitigating factors: errorCode: " + task.exitCode + "\r\n";
        else 
            status += "result: FAILED command failed with errorCode: " + task.exitCode + "\r\n";
    }

    task.endTime = new Date();
    status += "endTime: " + task.endTime + "\r\n";

    if (task.startTime) {
        var duration = ((task.endTime - task.startTime) / 60000).toFixed(2);
        status += "duration: " + duration + " min\r\n";
    }

    logMsg(LogTask, LogInfo, "_taskEnd: ENDING TASK ", task, " {\n", status, "}\n");
    FSOWriteToFile(status, task.fullStatusFileName, true);

    if (task.machine) {
        machManRelease(machMan, task.machine);
        task.machine = undefined;
    }
    task.lockFile.Close();
    task.lockFile = undefined;
    try { FSODeleteFile(task.lockFileName, true) } catch(e) {};
}

/****************************************************************************/
/* you need to call _taskPoll() after this */
function _tasksTerminate(taskList) {

    logMsg(LogTask, LogInfo1000, "_tasksTerminate: list = ", dump(taskList, 2), "\n");
    for (var i = 0; i < taskList.length; i++) {
        var task = taskList[i];
        if (task == undefined) continue;

        if (task.visitNum != -1) {
            task.visitNum = -1;
            if (task.run && !task.run.done) {
                logMsg(LogTask, LogWarn, "_tasksTerminate: TERMINATING ", task.name, " pid ", task.run.getPid(), "\n");
                task.terminated = true;
                runTerminate(task.run);
            }
            else if (task.dependents) {
                task.terminated = true;
                _tasksTerminate(task.dependents)
            }
        }
    }
}

/****************************************************************************/
/* Same as taskReport, however it catches all exceptions so they wont be fatal */

function _taskReportNoFail(dir, reportName) {

    try { 
        taskReport(dir, reportName); 
    } 
    catch(e) { 
        logMsg(LogTask, LogWarn, "Failure generating task report for ", dir, "exception = ", e.description, "\n"); 
    }
}

/*********************************************************************/
/* Wrapper function for _taskReport.  The _taskReport must be run on x86
   architecture for interaction with product studio so this wrapper launches
   _taskReport in WoW if you are not on an x86 platform.

   Create a HTML report from a directory from a directory containing
   status files from a run that '_taskPoll' has generated.  

     Parameters:
        dir: the directory that contains the status files to pretty print
        reportName: the name of the html report to generate
*/
function taskReport(dir, reportName)
{
    var cscript = "cscript";
    if (Env("PROCESSOR_ARCHITECTURE") != "x86")
    {
//        logMsg(LogTask, LogInfo, "Task report requires 32 bit architecture.  Running taskReport under WoW\n");
        cscript = Env("WINDIR") + "\\SysWow64\\cscript.exe";
    }
    WScript.ScriptFullName.search(/([^"].+)\\[^\\]+$/);
    var runjs = RegExp.$1 + "\\runjs.wsf";
    var command = cscript + " " + runjs + " _taskReport";
    if(dir)
        command += " \"" + dir + "\"";
    if(reportName)
    {
        command += " \"" + reportName + "\"";
    }
    var output = runCmd(command);
}
 
/**********************************************************************/
/* Create a HTML report from a directory from a directory containing
   status files from a run that '_taskPoll' has generated.  

     Parameters:
        dir: the directory that contains the status files to pretty print
        reportName: the name of the html report to generate
*/

function _taskReport(dir, reportName) {

    if (dir == undefined) {
        srcBase = Env("_NTBINDIR");
        if (srcBase == undefined)
            throw Error(1, "taskReport: 'dir' parameter not given and _NTBINDIR not set");
        dir = srcBase + "\\automation\\run.current";
    }
    if (reportName == undefined) 
        reportName = dir + "\\taskReport.html";
    
    var xmlReportName = reportName.replace(/\.html$/i, ".xml");

    var reportDir = reportName.match(/(.*)\\[^\\]*$/)[1];
    logMsg(LogTask, LogInfo10, "taskReport: Refreshing ", reportName, "\n");
    
    // Create the report file if it doesn't exist yet, so it can be found by code:#TaskReportLinks
    if (!FSOFileExists(reportName)) {
        // Create empty temp file
        var tempName = reportName + "." + Env("COMPUTERNAME") + ".tmp";
        var htmlFile = FSOOpenTextFile(tempName, 2, true);
        htmlFile.Close();
        // Rename the empty temp file to output report file name
        if (!FSOFileExists(reportName)) {
            FSOMoveFile(tempName, reportName);
        }
    }
    
    var tempName = reportName + "." + Env("COMPUTERNAME") + ".tmp"; 
    var htmlFile = FSOOpenTextFile(tempName, 2, true); 
    var now = new Date();

    htmlFile.WriteLine("<HTML>"); 
    htmlFile.WriteLine("<HEAD>");     
    htmlFile.WriteLine("<TITLE>Automation report for " + uncPath(reportName) + "</TITLE>"); 
    htmlFile.WriteLine("<STYLE>"); 
    htmlFile.WriteLine("body { font-family: calibri, arial, tahoma, sans-serif; font-size=11pt; }");
    htmlFile.WriteLine("tr.task_success { background-color: #cfc;}");
    htmlFile.WriteLine("td.task_success { color: green; text-align: center; }");
    htmlFile.WriteLine("tr.task_successWarnings { background-color: #ffc;}"); 
    htmlFile.WriteLine("td.task_successWarnings { color: green; text-align: center; }");
    htmlFile.WriteLine("tr.task_failedDependent { background-color: #eca;}"); 
    htmlFile.WriteLine("td.task_failedDependent { color: brown; text-align: center; }");
    htmlFile.WriteLine("tr.task_failedDependentWarnings { background-color: #fca;}"); 
    htmlFile.WriteLine("td.task_failedDependentWarnings { color: brown; text-align: center; }");
    htmlFile.WriteLine("tr.task_manuallyTerminated { background-color: #fcc;}"); 
    htmlFile.WriteLine("td.task_manuallyTerminated { color: red; text-align: center; }");
    htmlFile.WriteLine("tr.task_failed { background-color: #f88;}"); 
    htmlFile.WriteLine("td.task_failed { color: red; text-align: center; }");
    htmlFile.WriteLine("tr.task_unknown { background-color: #fff;}"); 
    htmlFile.WriteLine("td.task_unknown { text-align: center; }");
    htmlFile.WriteLine("tr.task_running { background-color: #feb;}"); 
    htmlFile.WriteLine("td.task_running { color: orange; text-align: center; }");
    htmlFile.WriteLine("tr.task_waitingOnMachine { background-color: #fce;}"); 
    htmlFile.WriteLine("td.task_waitingOnMachine { color: magenta; text-align: center; }");
    htmlFile.WriteLine("tr.task_waitingOnChild { background-color: #ccf;}");    
    htmlFile.WriteLine("td.task_waitingOnChild { color: blue; text-align: center; }");
    htmlFile.WriteLine("table { border: solid 1px black; border-collapse: collapse; }");
    htmlFile.WriteLine("td { font-size: 11pt; border: solid 1px black; padding-left: 0.3em; padding-right: 0.3em; }");
    htmlFile.WriteLine("th { border: solid 1px black; padding-left: 0.3em; padding-right: 0.3em; }");
    
    htmlFile.WriteLine("</STYLE>"); 
    htmlFile.WriteLine("</HEAD>"); 
    htmlFile.WriteLine("<BODY>"); 
    htmlFile.WriteLine("<H2> Automation Report: " + uncPath(reportName)  + "</H2>");

    htmlFile.WriteLine("This web page is a summary of all the output of a 'runjs doRun' command.");
    htmlFile.WriteLine("The 'doRun' automation runs a set of tasks each of which can possibly have");
    htmlFile.WriteLine("dependent tasks (e.g., a test task depends on an install task that depends on build tasks).");
    htmlFile.WriteLine("The automation runs these tasks in the proper order (possibly on multiple machines)");
    htmlFile.WriteLine("ensuring that the output from the tasks is carefully segregated and generates this report.");
    htmlFile.WriteLine("<UL>");
    htmlFile.WriteLine("<LI><b>Date: </b>" + prettyTime(now));
    htmlFile.WriteLine("<LI><b>User: </b>" + Env("USERNAME"));
    htmlFile.WriteLine("<LI><b>Machine: </b>" + Env("COMPUTERNAME"));
    htmlFile.WriteLine("<LI><b>Report generated by command: </b> runjs taskReport " + dir);
    htmlFile.WriteLine("</UL>");

        // See if there are any other top level reports (This is a bit of a hack)
    if (reportDir.match(/\\run\.[^\\]*$/)) {
        var out = "";
        var dirs = FSOGetDirPattern(dir + "\\..", /.*/);
        for(var i = 0; i < dirs.length; i++) {
            var report = dirs[i] + "\\taskReport.html";
            if (FSOFileExists(report))
                out += "<LI> <A HREF='" + uncPath(report) + "'> " + uncPath(report) + "</A>";
        }
        if (out != "") {
            //#TaskReportLinks
            htmlFile.WriteLine("<H3> Links to related task reports</H3>");
            htmlFile.WriteLine("Typically, reports from previous 'runjs doRun' command are stored in");
            htmlFile.WriteLine("sibling directories.   For your convenience a list of such reports");
            htmlFile.WriteLine("is given here.  These links were valid when this report was generated,");
            htmlFile.WriteLine("however if a directory is deleted after this ")
            htmlFile.WriteLine("report is generated, a link below might be invalid.");
            htmlFile.WriteLine("<UL>");
            htmlFile.Write(out);
            htmlFile.WriteLine("</UL>");
        }
    }

    var handled = {};        // holds the name of all files we recognise
    handled[(dir + "\\runInfo.bat").toLowerCase()] = true;

    var parentReport = dir + "\\..\\taskReport.html";
    if (FSOFileExists(parentReport)) {
        htmlFile.WriteLine("<H3> Parent Task </H3>");
        htmlFile.WriteLine("<UL>");
        htmlFile.WriteLine("<LI> Parent: <A HREF='" + relPath(parentReport, reportDir) + "'> " + uncPath(parentReport) + "</A>");
        htmlFile.WriteLine("</UL>");
    }

        // parse the status files
    logMsg(LogTask, LogInfo10, "taskReport: Dir = ", dir, "\n");
    var statuses = [];
    var taskReport = { date: now, topTasks:[], tasks:statuses };
    var statusFileNames = FSOGetFilePattern(dir, /\.status\.log$/i);
    var statusesByName = {};
    for(var i = 0; i < statusFileNames.length; i++) {
        var statusFileName = statusFileNames[i]
        try {
            var status = parseFile(statusFileName);
        } catch(e) {
            logMsg(LogTask, LogWarn, "taskReport: could not parse status file ", statusFileName, " exception ", e.description, ".  Skipping\n");
            continue;
        }
        status.children = [];

        status.startTicks = 0;
        if (status.startTime != undefined) {
            status.startTicks = Date.parse(status.startTime); 
            if (isNaN(status.startTicks)) {        // is it a NaN
                logMsg(LogTask, LogWarn, "taskReport: Could not parse '", status.startTime, "' as date\n"); 
                status.startTicks = 0;
            }
        }
        
        var statusName = statusFileName.match(/([^\\]*)$/)[1];
        statusesByName[statusName] = status;

        status.objID = statusName;
        statuses.push(status);
    }
        // sort so the most recently generated one is first
    statuses.sort(function(x, y) { return (y.startTicks - x.startTicks); });

        // determine children from parent fields (we don't use dependents field because it may not be set yet)
    for(var i = 0; i < statuses.length; i++) {
        var status = statuses[i];
        if (status.parent) {
            var parentStatus = statusesByName[status.parent];
            if (parentStatus) 
                parentStatus.children.push(status);
        }        
        else 
            taskReport.topTasks.push(status);
    
        if (status.result == undefined || (!status.result.match(/SUCCESS/) && !status.result.match(/dependent\S* \[.*\] failed/)))
            taskReport.firstFailure = status;
    }

    htmlFile.WriteLine("<H3> Task summary </H3>");
    htmlFile.WriteLine("The task summary allows you to quickly determine what has failed in the run.  If a task requires");
    htmlFile.WriteLine("a 'child' task to complete first, that child task is indented in the list below.  Each task  ");
    htmlFile.WriteLine("displays its status (running, waiting, success, failure), and its output.  For running tasks");
    htmlFile.WriteLine("the stdout link is usually the most interesting.   It contains the most up to date record of");
    htmlFile.WriteLine("what the task sent to standard output.");
    htmlFile.WriteLine("For completed tasks, click on the task name to get ");
    htmlFile.WriteLine("detailed information about what happened during the run.");
    htmlFile.WriteLine("<P>");

    htmlFile.WriteLine("<UL><TABLE>");
    htmlFile.WriteLine("<TR><TH>Task</TH><TH> Current State </TH><TH> Stdout </TH><TH> In State Since </TH><TH> Duration </TH><TH> Machine </TH>");

    for(var i = 0; i < statuses.length; i++) 
        _taskReportTask(statuses[i], dir, reportDir, htmlFile, handled, statusesByName, true);
    htmlFile.WriteLine("<CAPTION ALIGN=BOTTOM>This table was last updated on " + prettyTime(now) + "</CAPTION>");
    htmlFile.WriteLine("</TABLE></UL>");
    
    htmlFile.WriteLine("<H3> Detailed Task report </H3>");
    htmlFile.WriteLine("The detailed task report creates links to all information that was generated");
    htmlFile.WriteLine("during the run.  Like the summary report, tasks are grouped according to");
    htmlFile.WriteLine("their parent-child relationship.");

    htmlFile.WriteLine("<UL>");
    for(var i = 0; i < statuses.length; i++) 
        _taskReportTask(statuses[i], dir, reportDir, htmlFile, handled, statusesByName, false);
    htmlFile.WriteLine("</UL>");

    files = FSOGetFilePattern(dir);
    var out = "";
    for(var i = 0; i < files.length; i++) {
        var file = files[i];
        if (file.match(/\\taskReport\.(html|xml)$/i))        // TODO - this is not correct except in the default case
            continue;
        if (file.match(/\.tmp$/))
            continue;
        if (file.match(/\.lock$/))
            continue;
        if (handled[file.toLowerCase()]) 
            continue;

        out += "<LI> ";

        if (file.match(/\.(log|err|wrn|txt)$/))
            out += "<A TYPE='text/plain' HREF='" + relPath(file, reportDir) + "'> " + uncPath(file) + "</A>";
        else if (file.match(/\.(html|xml)$/))
            out += "<A HREF='" + relPath(file, reportDir) + "'> " + uncPath(file) + "</A>";
        else 
            out += file;
        out += "\n"
    }
    if (out != "") {
        htmlFile.WriteLine("<H3> Other Files in " + dir + " that are not recognised </H3>");
        htmlFile.WriteLine("There were files in the directory that taskReport did not recognise.");
        htmlFile.WriteLine("They are included here for completeness.");
        htmlFile.WriteLine("<UL>");
        htmlFile.Write(out);
        htmlFile.WriteLine("</UL>");
    }

    htmlFile.WriteLine("</BODY>");
    htmlFile.WriteLine("</HTML>");

    xmlWrite(taskReport, xmlReportName, "taskReport", {indent:1, startTicks:1, _reported:1, dependents:1});

        // install the new html file as one atomic operation
    htmlFile.Close();
    for (var i = 0; i < 1000; i++) {
        if (!FSOFileExists(reportName)) {
            FSOMoveFile(tempName, reportName);    
            break;
        }
        try { FSODeleteFile(reportName); } catch(e) {};
        WScript.Sleep(10);        // Only browsers should be fetching this file
    }

    return 0;
}

/****************************************************************************/
/* reports on one task with parse status 'status' which came from 'dir' 
   The report goes in 'reportDir' and the open html File is 'htmlFile'.
   'handled' is a set of files that the report understood (thus it gets 
   updated by this routine.  'statusesByName' is a table indexed by 
   status file name that holds all the parsed status records in 'reportDir' */

function _taskReportTask(status, dir, reportDir, htmlFile, handled, statusesByName, summaryOnly) {

    var ret = false;
    logMsg(LogTask, LogInfo100, "_taskReportTask: reporting '", status.fileName, "' summaryOnly = ", summaryOnly, " {\n");
    handled[status.fileName.toLowerCase()] = true;

    if (summaryOnly == undefined)
        throw Error(1, "summaryOnly is a required argument!");

    if (status._reported == summaryOnly)  { // {
        logMsg(LogTask, LogInfo100, "} _taskReportTask() Quick Exit 1\n");
        return ret;
    }
    if (status.parent) {
        logMsg(LogTask, LogInfo1000, "_taskReportTask: has parent '", status.parent, "'\n");
        var parentStatus = statusesByName[status.parent];
        if (parentStatus != undefined) {
            ret |= _taskReportTask(parentStatus, dir, reportDir, htmlFile, handled, statusesByName, summaryOnly);
            if (status._reported == summaryOnly)  { // {
                logMsg(LogTask, LogInfo100, "} _taskReportTask() Quick Exit 2\n");
                return ret;
            }
        }
    }

    logMsg(LogTask, LogInfo1000, "_taskReportTask marking as reported for ", status.fileName, "\n")
    status._reported = summaryOnly;
    ret = true;            
    status.fileName.match(/([^\\]*)\.(\d+)\.status\.log$/i);
    var taskName = RegExp.$1;
    var retry = RegExp.$2;

    if (summaryOnly) {
        var taskStatus;
        var taskStatusText;
        if (status.result) {
            start = new Date(status.startTime);
            if (status.result.match(/SUCCESS/i)) {
                taskStatus = 'task_success';
                taskStatusText = 'Success';
            }
            else if (status.result.match(/FAILED/i)) {
                if (status.result.match(/dependents.*failed.*indicating mitigating factors/i)) {
                    taskStatus = 'task_failedDependentWarnings';
                    taskStatusText = 'Success <FONT COLOR=RED>(Warnings)</FONT>';
                } else if (status.result.match(/dependents.*failed/i)) {
                    taskStatus = 'task_failedDependent';
                    taskStatusText = 'Child Failed';
                } else if (status.result.match(/terminated manually/i)) {
                    taskStatus = 'task_manuallyTerminated';
                    taskStatusText = 'Manually Killed';
                } else if (status.result.match(/FAILED-PRI2/)) {
                    taskStatus = 'task_failed';
                    taskStatusText = 'Failed';
                } else if (status.result.match(/indicating mitigating factors/i)) {
                    taskStatus = 'task_successWarnings';
                    taskStatusText = 'Success <FONT COLOR=RED>(Warnings)</FONT>';
                } else {
                    taskStatus = 'task_failed';
                    taskStatusText = 'Failed';
                }
            }
            else {
                taskStatus = 'task_unknown';
                taskStatusText = '-';
            }
        }
        else if (status.startTime) {
            taskStatus = 'task_running';
            taskStatusText = 'Running Task';
            start = new Date(status.startTime);
        }
        else if (status.readyTime) {
            taskStatus = 'task_waitingOnMachine';
            taskStatusText = 'Waiting on Machine';
            start = new Date(status.readyTime);
        }
        else if (status.children.length > 0 && status.inclusiveStartTime) {
            taskStatus = 'task_waitingOnChild';
            taskStatusText = 'Waiting on Child';
            start = new Date(status.inclusiveStartTime);
        }
        else {
            taskStatus = 'task_unknown';
            taskStatusText = '-';
        }
        htmlFile.WriteLine('<TR CLASS="' + taskStatus + '"><TD>');
        
        if (status.indent == undefined) {
            status.indent = "";
            if (status.parent) 
                var parent = statusesByName[status.parent];
                if (parent)
                    status.indent = parent.indent + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
        }
        htmlFile.Write(status.indent);
        
        htmlFile.Write("<A HREF='#" + taskName + "." + retry + "'><B>" + taskName + "</B> retry " + retry + "</A>");

        htmlFile.Write('</TD><TD CLASS="' + taskStatus + '">' + taskStatusText);

        htmlFile.Write("</TD><TD ALIGN=CENTER>");

        if (FSOFileExists(status.output)) 
            htmlFile.WriteLine("<A TYPE='text/plain' HREF='" + relPath(status.output, reportDir) + "'> stdout </A>");
        else
            htmlFile.Write("-");
        htmlFile.Write("</TD><TD ALIGN=CENTER>");

        if (start && !isNaN(start)) 
            htmlFile.Write(prettyTime(start))
        else 
            htmlFile.Write("-");
        htmlFile.Write("</TD><TD ALIGN=CENTER>");

        if (start && !isNaN(start)) {
            var end = new Date(status.endTime);
            if (!end || isNaN(end))
                end = new Date();
            htmlFile.Write(((end - new Date(start)) / 60000).toFixed(2) + " min");
        }
        else 
            htmlFile.Write("-");
        htmlFile.Write("</TD><TD ALIGN=CENTER>");

        if (status.machine)
            htmlFile.Write(status.machine);
        else
            htmlFile.Write("-");
        htmlFile.Write("</TD><TD>");
        htmlFile.WriteLine("</TD></TR>");

        if (status.children.length > 0) {
            for(var i = 0; i < status.children.length; i++)  {
                if (status.children[i] == undefined) continue;
                logMsg(LogTask, LogInfo1000, "_taskReportTask doing child ", status.children[i].fileName);
                _taskReportTask(status.children[i], dir, reportDir, htmlFile, handled, statusesByName, summaryOnly);
            }
        }
    }
    else {
        htmlFile.WriteLine("<TABLE><TD>");
        htmlFile.WriteLine("<A  NAME='" + taskName + "." + retry + "'>");
        htmlFile.WriteLine("<LI>Task <B>" + taskName + "</B> retry " + retry);
        htmlFile.WriteLine("<UL>");

        if (status.description)
            htmlFile.WriteLine("<LI> <B>Description:</B> " + status.description);
        else if (!status.command) {
            htmlFile.WriteLine("<LI> <B>Description:</B> This task does not have a command associated with it.");
            htmlFile.WriteLine("Its purpose is simply to gather a set of related tasks together so they can be");
            htmlFile.WriteLine("run as a group.");
        }
        if (status.moreInfoUrl)
            htmlFile.WriteLine("<LI> <B>More information:</B> <A HREF=\"" + status.moreInfoUrl + "\">" + status.moreInfoUrl + "</A>" );

        if (status.result) {
            var result = status.result;
            result = result.replace(/(SUCCESS)/i, "<FONT COLOR=GREEN>$1</FONT>");
            if (result.match(/(.*)FAILED-PRI2(.*)/)) 
                result = RegExp.$1 + "<FONT COLOR=RED>FAILED</FONT>" + RegExp.$2;
            else if (status.result.match(/indicating mitigating factors/i))
                result = "<FONT COLOR=GREEN>Success</FONT> <FONT COLOR=RED>(Warnings)</FONT>";
            else 
                result = result.replace(/(FAILED)/i, "<FONT COLOR=RED>$1</FONT>");
            if (result.match(/(.*dependents \[)(.*)(\] failed.*)/i)) {
                var before = RegExp.$1;
                var depStr = RegExp.$2;
                var after  = RegExp.$3;
                result = before + depStr.replace(/([^, ]+)/g, "<A HREF=#$1." + retry +"> $1</A>") + after;
            }
            htmlFile.WriteLine("<LI> <B>Result:</B> " + result);
        }
        else if (status.startTime) 
            htmlFile.WriteLine("<LI> <B> <FONT COLOR=ORANGE> Started (Incomplete) </FONT> </B> " + status.startTime);

            // print out any task specific information 
        htmlFile.Write(_taskReportTaskOutput(taskName, dir, reportDir, handled, status, summaryOnly));

        var outputName = status.fileName.replace(/\.status\.log$/i, ".output.log");
        if (FSOFileExists(outputName)) {
            handled[outputName.toLowerCase()] = true;
            htmlFile.WriteLine("<LI> <B> Stdout: </B> <A TYPE='text/plain' HREF='" + relPath(outputName, reportDir) + "'> " + uncPath(outputName) + "</A>");
            htmlFile.WriteLine("<DL><DD>");
            htmlFile.WriteLine("The output file is the stdout and stderr of the command given above.  This\n");
            htmlFile.WriteLine("file is 'live' (updated as the task proceeds).\n");
            htmlFile.WriteLine("</DL>");
        }

        if (status.outDir && status.srcBase) {
            htmlFile.WriteLine("<LI> <B>To Replay Task:</B> runjs doRunHere " + taskName + " " + status.outDir + " " + status.srcBase);
            htmlFile.WriteLine("<DL><DD>");
            htmlFile.WriteLine("The command above will 'replay' only this task (and its dependents).");
            htmlFile.WriteLine("It will generate new log files (retry count goes up), and will not");
            htmlFile.WriteLine("destroy the old ones.  A new report page will also be generated.");
            htmlFile.WriteLine("It is the preferred method of retrying a task.");
            htmlFile.WriteLine("</DL>");
        }

        if (status.command) {
            htmlFile.WriteLine("<LI> <B>Task Cmd:</B> " + status.command);
            htmlFile.WriteLine("<DL><DD>");
            htmlFile.WriteLine("The command above the body of the task.  You may run this command");
            htmlFile.WriteLine("by hand to replay the task.  Replaying the the task in this way");
            htmlFile.WriteLine("does not create a new task report (this page will not be updated)");
            htmlFile.WriteLine("however any files generated by the command will be updated (which");
            htmlFile.WriteLine("mean links in the 'Output Data' section will be affected).");

            if (status.command.match(/runjs\s+(\w+)/)) {
                htmlFile.WriteLine("The command <B> runjs /? " + RegExp.$1 + " </B> can be used to get more");
                htmlFile.WriteLine("information on what the parameters to this command mean.");
            }
            htmlFile.WriteLine("</DL>");
        }

        if (status.children.length > 0 && status.inclusiveStartTime) 
            htmlFile.WriteLine("<LI> <B> Children started at: </B> " + status.inclusiveStartTime);

        if (status.readyTime) 
            htmlFile.WriteLine("<LI> <B> Children completed at: </B> " + status.readyTime);

        if (status.startTime) 
            htmlFile.WriteLine("<LI> <B> Started: </B> " + status.startTime);

        if (status.endTime) 
            htmlFile.WriteLine("<LI> <B> Ended: </B> " + status.endTime);

        if (status.duration)
            htmlFile.WriteLine("<LI> <B> Duration (excludes subtasks) : </B>" + status.duration);

        if (status.children.length > 0 && status.inclusiveStartTime) {
            var inclusiveDuration = ((new Date(status.endTime) - new Date(status.inclusiveStartTime)) / 60000).toFixed(2);
            if (!isNaN(inclusiveDuration))
                htmlFile.WriteLine("<LI> <B> Duration (includes subtasks) : </B>" + inclusiveDuration);
        }

        if (status.machine) 
            htmlFile.WriteLine("<LI> <B><FONT COLOR=blue> Machine: </FONT></B> " + status.machine);

        if (status.outDir && FSOFolderExists(status.outDir))
            htmlFile.WriteLine("<LI> <B> Data OutDir: </B> <A HREF='" + relPath(status.outDir, reportDir) + "'> " + uncPath(status.outDir) + "</A>");

        htmlFile.WriteLine("<LI> <B> Status File: </B> <A TYPE='text/plain' HREF='" + relPath(status.fileName, reportDir) + "'> " + uncPath(status.fileName) + "</A>");
        htmlFile.WriteLine("<DL><DD>");
        htmlFile.WriteLine("The status file contains the high level results of a task in a form that is");
        htmlFile.WriteLine("relatively easy to parse in scripts.  Generally the important parts of this");
        htmlFile.WriteLine("file are already pretty printed in this report, but a link here is provided");
        htmlFile.WriteLine("for completeness.");
        htmlFile.WriteLine("</DL>");

        logMsg(LogTask, LogInfo100, "Looking at child tasks ", status.dependents, "\n");
        if (status.children.length > 0) {
            htmlFile.WriteLine("<LI> <B> Child tasks: </B> total " + status.children.length);
            htmlFile.WriteLine("<UL>");
            for(var i = 0; i < status.children.length; i++) {
                var child = status.children[i];
                logMsg(LogTask, LogInfo1000, "_taskReportTask: found child ", child.fileName, "\n");
                if (!_taskReportTask(child, dir, reportDir, htmlFile, handled, statusesByName, summaryOnly)) {
                    if (child.fileName.match(/([\\]*).\d+.status.log$/i)) 
                        htmlFile.WriteLine("<LI>Task <B>" + RegExp.$1 + "</B> (already listed)");
                }
            }
            htmlFile.WriteLine("</UL>");
        }

        htmlFile.WriteLine("</UL>");
        htmlFile.WriteLine("</A>");
        htmlFile.WriteLine("</TD></TABLE><P>");
    }

    logMsg(LogTask, LogInfo100, "} _taskReportTask()\n");
    return ret;
}

/****************************************************************************/
/* A helper function for _taskReportTask.  This routine deals with task
   specific parsing (because I know it is a build, I look for certain files.
   In theory the task should provide this functionality, but in practice 
   there are not that many different kinds of log files to have to parse.
   It will also update 'status' with a 'outputData' field that contains
   a machine parsed version of the output. */

function _taskReportTaskOutput(taskName, dir, reportDir, handled, status) {
    var out = "";
    var outputData = {};

    // logMsg(LogTask, LogInfo, "Task Name ", taskName, " dir = ", dir, "\n");
    if (taskName.match(/sdWhereSynced/i)) {
        if (FSOFileExists(status.output)) {
            var stdout = FSOReadFromFile(status.output);
            if (stdout.match(/minChangeTime.*"([^"\n]*)"/m)) {
                outputData.minChangeTime = RegExp.$1;
                out += "<LI><b>MinSyncTime:</b> The depot was synced after " + outputData.minChangeTime + "\r\n";
            }

            if (stdout.match(/maxChangeTime.*"([^"\n]*)"/m)) {
                outputData.maxChangeTime = RegExp.$1;
                out += "<LI><b>MaxSyncTime:</b> The depot was synced before " + outputData.maxChangeTime + "\r\n";
            }
        }
        status.outputData = outputData;
        return out;
    }

    if (!taskName.match(/(.*)@(.*)/i))
        return out;
    var taskKind = RegExp.$1;
    var taskSuffix = RegExp.$2;
    logDir = dir + "\\" + taskSuffix;
    if (FSOFolderExists(logDir + "\\" + taskKind))
        logDir = logDir + "\\" + taskKind;

    // logMsg(LogTask, LogInfo, "_taskReportTaskOutput: taskKind ", taskKind, " logDir '", logDir, "'\n");
    logDir = uncPath(logDir);

    var dumpDirs = FSOGetDirPattern(logDir, /\.dmpRpt$/i);
    if (dumpDirs.length > 0)
        outputData.dumpDirs = dumpDirs;
    for(var j = 0; j < dumpDirs.length; j++) {
        var dumpDir = dumpDirs[j];
        var dumpReport = dumpDir + "\\Report.html";
        if (FSOFileExists(dumpReport)) 
            out += "<LI> <B><FONT COLOR=RED> A process dump was taken: </FONT></B> <A TYPE='text/plain' HREF='" + relPath(dumpReport, reportDir) + "'> " + dumpReport + " </A>.\r\n";
    }

    var highLightColor = (status.result && status.result.match(/SUCCESS/i)) ? "BLUE" : "RED";

    // Infer what kind of task we're reporting by its name.  For each kind, output
    // text particular to that kind of task.

    if (taskKind.match(/build/i) &&
    // Adding this check so we don't accidentally match to test.devBVT.BUILDBVT
        !taskKind.match(taskKind.match(/\btest\.devBVT/i))) {
        var prefastLog = logDir + "\\prefastParse.log";
        if (FSOFileExists(prefastLog)) {
            outputData.prefastLog = prefastLog;
            handled[prefastLog.toLowerCase()] = true;
            out += "<LI> <B><FONT COLOR=RED> Report on new Prefast errors: </FONT></B> <A TYPE='text/plain' HREF='" + relPath(prefastLog, reportDir) + "'> " + prefastLog + " </A>.\r\n";
            if (status.result != undefined && status.result.match(/FAILED/i)) {
                out += "<DL><DD>\r\n";
                out += "<b> If you get failures: </b> This report is always comparing the prefast warnings on this build\r\n";
                out += "against what the latest checked in build at the time this run was done.\r\n";
                out += "Thus if your  local build corresponds to a sync say 24 hours ago, and in the last 24 hours\r\n";
                out += "someone has checked in a prefast fix,  It will look like you introduced the prefast warning\r\n";
                out += "that was just fixed (since it IS in your build and not in the current sync).  \r\n";
                out += "Getting errors in files that you did not modify is a telltale sign of this effect.\r\n";
                out += "Sadly, fixing the automation to avoid this is not easy because it would have to know both when you synced as well as\r\n";
                out += "what the baseline was at that time (neither of these is easy to get today).\r\n";
                out += "<p> To confirm that your failures are caused by this effect, see if that file with the failure\r\n";
                out += "has been modified since you last did your sync (sd sync -n ... will tell you the files that are\r\n";
                out += "out of date without actually updating them).\r\n";
                out += "Alternatively, you can sync live, and replay the prefast task (paste the replay line below), and\r\n"
                out += "the errors should go away.\r\n "
                out += "</DL>\r\n";
            }
        }

        var buildReports = FSOGetFilePattern(logDir, new RegExp("^.*build.*\\.bldrpt.html$", "i"));
        if (buildReports.length > 0)
            outputData.buildReports = buildReports;
        for (var j = 0; j < buildReports.length; j++) {
            var buildReport = buildReports[j];
            var base = buildReport.match(/(.*)\.bldrpt.html$/i)[1].toLowerCase();
            handled[base + ".bldrpt.html"] = true;
            handled[base + ".wrn"] = true;
            handled[base + ".err"] = true;
            handled[base + ".log"] = true;
            out += "<LI> <B> Build Report: </B> <A HREF='" + relPath(buildReport, reportDir) + "'> " + uncPath(buildReport) + " </A>\r\n";
            out += "<DL><DD>\r\n";
            out += "<b> If you get failures: </b> The build report is a parsed form of build *.err and *.log files.\r\n";
            out += "*.err files have the problem that don't give the context for the failure, and *.log files have the\r\n";
            out += "problem that they interleave the output of concurrent builds.   The report tries to fix these\r\n";
            out += "deficiencies and make the output more user friendly.\r\n";
            out += "</DL>\r\n";
        }

        var buildLogs = FSOGetFilePattern(logDir, new RegExp("^.*build.*\\.(err|log|wrn)$", "i"));
        if (buildLogs.length > 0)
            outputData.buildLogs = buildLogs;
        for (var j = 0; j < buildLogs.length; j++) {
            var buildLog = buildLogs[j].toLowerCase();
            if (!handled[buildLog]) {
                handled[buildLog.toLowerCase()] = true;
                out += "<LI> <B> BuildLog: </B> <A TYPE='text/plain' HREF='" + relPath(buildLog, reportDir) + "'> " + uncPath(buildLog) + " </A>\r\n";
            }
        }
    }
    else if (taskKind.match(/\bscan/i)) {
        var scanLog = logDir + "\\scan.report.log";
        if (FSOFileExists(scanLog)) {
            handled[scanLog.toLowerCase()] = true;
            out += "<LI> <B> Scan Report: </B> <A TYPE='text/plain' HREF='" + relPath(scanLog, reportDir) + "'> " + uncPath(scanLog) + " </A>.\r\n";
        }
    }
    else if (taskKind.match(/\btest\.devBVT/i)) {

        //Do not show bug information for selfhost runs.
        var showBugInfo = !taskKind.match(/selfhost/i);

        var smartyData = {}
        smartyData.reportDir = logDir;
        outputData.smartyReports = [smartyData];

        var smartyErrFile = logDir + "\\Smarty.err.xml";
        var smartyXml;
        if (FSOFileExists(smartyErrFile)) {
            try {
                smartyXml = xmlRead(smartyErrFile);
                smartyData.TESTCASES = smartyXml.TESTRESULT.MYBLOCK.TESTCASES;
            } catch (e) {
                logMsg(LogTask, LogWarn, "taskReportTaskOutput: failure reading smarty XML: ", e.description, "\n");
            }
        }

        var smartyDiffReport = "";
        try {
            smartyDiffReport = _smartyDiffReport(logDir, logDir + "\\baseline", reportDir, showBugInfo, smartyXml);
        }
        catch (e) {
            logMsg(LogTask, LogWarn, "taskReportTaskOutput: failure running _smartyDiffReport: ", e.description, "\n");
        }
        if (smartyDiffReport != "") {
            out += "<BR><LI>" + smartyDiffReport;
        }
        /***
        // TODO these really need to be made better.  We want it to for the right architecture, 
        // corelate with bugs, They are also not currently up to date
        out += "<LI> <B>Rolling report:</B> <A HREF='\\\\CLRMain\\public\\drops\\whidbey\\rollingTests\\report.html'>\r\n";
        out += "\\\\CLRMain\\public\\drops\\whidbey\\rollingTests\\report.html</A>"
        out += "<DL><DD>\r\nShows the failures that have occured on latest SNAP builds."
        out += "You should compare your failures to see if they are on these clean builds.</DL>\r\n"  
         
        out += "<LI> <B>Recent team DDR reports:</B> <A HREF='\\\\CLRMain\\public\\drops\\whidbey\\testRunFailureStats.html'>";
        out += "\\\\CLRMain\\public\\drops\\whidbey\\testRunFailureStats.html</A>"
        out += "<DL><DD>\r\nShows failures in other people's daily dev runs."
        out += "It is worth talking to people who have experienced the same failure.\r\n</DL>\r\n";
        ***/
    }
    else if (taskKind.match(/^test.ddInt\./i)) {
        var trunReports = FSOGetFilePattern(logDir, /trun\.(xml|sum|err|log)$/i);
        if (trunReports.length > 0)
            outputData.trunReports = trunReports;

        var summaryFile = undefined;
        var errorFile = undefined;
        var logFile = undefined;
        var xmlFile = undefined;

        // Include trun.sum, trun.err, trun.log, trun.xml
        for (var j = 0; j < trunReports.length; j++) {
            var rtFilename = trunReports[j];

            if (rtFilename.match(/^.*\.xml$/i)) {
                xmlFile = "<LI><B><FONT COLOR=" + highLightColor + "> Summary report: </FONT></b>" + htmlRelLink(rtFilename, reportDir) + ". \r\n"
                        + "<DL><DD> This gives a pretty XSL summary of the test run with relevant links to individual tests. Most of the tests can be run from a ClrEnv window by changing into the test directory and type 'TRUN' without any args. However there might be some tests that need more house cleaning than this simple re-try approach.</DL>\r\n";
            }
            if (rtFilename.match(/^.*\.sum$/i)) {
                summaryFile = "<LI><B>Summary log: </b>" + htmlRelLink(rtFilename, reportDir) + ". \r\n"
                            + "<DL><DD> This gives a quick summary of failing tests. </DL>\r\n";
            }
            if (rtFilename.match(/^.*\.err$/i)) {
                errorFile = "<LI><B>Error log: </b>" + htmlRelLink(rtFilename, reportDir) + ". \r\n"
                          + "<DL><DD> This typically lists the errors from failing tests. However, mere existence of this file doesn't necessarily mean that the trun task or the test has failed. It is possible that some commands in the tsources [BODY] section of a test either didn't set the errorlevel properly or failed (causing the .err file to be generated) while the test itself passed. "
                          + "You should always start with the summary file for a quick list of failing tests and cross reference that with the error file. </DL> \r\n"
            }
            if (rtFilename.match(/^.*\.log$/i)) {
                logFile = "<LI><B>Output log: </b>" + htmlRelLink(rtFilename, reportDir) + ". \r\n"
                        + "<DL><DD> This lists the output from all the tests. It is quite verbose. </DL>\r\n";
            }
        } // end for

        if (xmlFile != undefined) {
            out += "<LI><B><FONT COLOR=ORANGE> Rolling DDIntFx results: </FONT></b>" + htmlAnchor("\\\\clrmain\\public\\drops\\whidbey\\rollingDDIntFx") + ". \r\n"
                    + xmlFile;
        }

        if (summaryFile != undefined)
            out += summaryFile;

        if (errorFile != undefined)
            out += errorFile;

        if (logFile != undefined)
            out += logFile;

        //logSetFacilityLevel(LogFSO, LogInfo);
    }
    else if (taskKind.match(/daccop/i)) {

        var fatalError = false;

        var buildReports = FSOGetFilePattern(logDir, new RegExp("^.*build.*\\.bldrpt.html$", "i"));
        if (buildReports.length > 0)
            outputData.buildReports = buildReports;
        for (var j = 0; j < buildReports.length; j++) {
            var buildReport = buildReports[j];
            var base = buildReport.match(/(.*)\.bldrpt.html$/i)[1].toLowerCase();
            handled[base + ".bldrpt.html"] = true;
            handled[base + ".wrn"] = true;
            handled[base + ".err"] = true;
            handled[base + ".log"] = true;
            out += "<LI> <B> Build Report: </B> <A HREF='" + relPath(buildReport, reportDir) + "'> " + uncPath(buildReport) + " </A>\r\n";
            fatalError = true;
        }

        var errorFile = logDir + "\\ManagedAdapterFatalError.txt";
        if (FSOFileExists(errorFile)) {
            handled[errorFile.toLowerCase()] = true;
            fatalError = true;
            out += "<LI> <B><FONT COLOR=RED> Managed PreFast Plugin Fatal Error Report: </FONT></B> <A TYPE='text/plain' HREF='" + relPath(errorFile, reportDir) + "'> " + uncPath(errorFile) + " </A>.\r\n";
        }

        var dacCopLog = logDir + "\\DacCopAnalyze.log";
        if (FSOFileExists(dacCopLog)) {
            handled[dacCopLog.toLowerCase()] = true;

            // To avoid confusion, we only want to show the DacCop report if there wasn't a fatal error
            if (!fatalError)
                out += "<LI> <B> DacCop Report: </B> <A TYPE='text/plain' HREF='" + relPath(dacCopLog, reportDir) + "'> " + uncPath(dacCopLog) + " </A>.\r\n";
        }
    }
    
    if (status.result != undefined && status.result.match(/^\s*SUCCESS/im)) {
        if (taskKind.match(/build/i)) {
            var clrSetupFile = logDir + "\\bin\\clrsetup.bat";
            if (FSOFileExists(clrSetupFile))
                out += "<LI> To Install: <A HREF='" + relPath(clrSetupFile, reportDir) + "'> " + uncPath(clrSetupFile) + " </A>\r\n";
        }
    }

    var binDir = logDir + "\\bin";
    if (FSOFolderExists(binDir)) {
        outputData.binDir = binDir;
        out += "<LI> <B> Built Binaries: </B> <A HREF='" + relPath(binDir, reportDir) + "'> " + uncPath(binDir) + " </A>\r\n";
        out += "<DL><DD>\r\n";
        out += "These are the binaries associated with this task.  The exes as well as the pdbs are here.  ";
        out += "There should also be a <B> clrsetup.bat </B> that can be run to install these binaries on another machine."
        out += "</DL>\r\n";
    }
    
    if (FSOFolderExists(logDir)) {
        outputData.logDir = logDir;
        out += "<LI> <B> Output Dir: </B> <A HREF='" + relPath(logDir, reportDir) + "'> " + uncPath(logDir) + " </A>\r\n";
    }

    if (out != "") {
        status.outputData = outputData;
        out = "<LI> <B> Output Data </B> for task " + taskName + " (generated by the task command itself)\r\n" 
            + "<DL><DD>\r\n"
            + "The following are output files that are generated by the task itself (not the task run harness).\r\n"
            + "These files are usually the most interesting to look at since they are tailored to the task.\r\n"
            + "</DL>\r\n"
            + "<UL>\r\n" + out + "</UL>\r\n"; 
    }
    return out;
}

/****************************************************************************/
function jobNew(task, env, machMan) {

    if (machMan == undefined)
        machMan = machManNew(false);

    var ret = {};
    ret.task = task;
    ret.env = env;
    ret.machMan = machMan;
    ret.visitNum = 1;
    ret.report = ret.env.outDir + "\\taskReport.html";

    return ret;
}

/****************************************************************************/
function machManNew(dontAddSelf) {

    var machMan = {};
    var randNum = (new Date()).getMilliseconds();
    machMan.name = "dispatch." + randNum;
    machMan.toString = function() { return this.name; }    // pretty printing
    machMan.byType = {};

    if (!dontAddSelf) {
        var machine = machManAdd(machMan, Env("COMPUTERNAME"));
        machine.self = true;
    }
    return machMan;
}

/*********************************************************************************/
function machManAdd(machMan, name, type, speed) {
    logMsg(LogTask, LogInfo10, "In addMachine(machMan, ", name, ", ", type, ", ", speed, ") {\n");

    if (type == undefined)
        type = Env("PROCESSOR_ARCHITEW6432") != "" ? 
            Env("PROCESSOR_ARCHITEW6432") : Env("PROCESSOR_ARCHITECTURE");
    if (speed == undefined)
        speed = 4000;             // TODO: get a real measure of the speed of this computer.

    type = type.toLowerCase();

    var machine = new Object();
    machine.toString = function() { return this.name; }    // pretty printing
    machine.name = name;
    machine.speed = speed;

    if (!machMan.byType[type]) 
        machMan.byType[type] = [];

    machMan.byType[type].push(machine);
    machMan.byType[type].sort(function(x, y) { return y.speed - x.speed; } );

        // The following fields changes over the life of a job
    machine.inUse = undefined;        
    logMsg(LogTask, LogInfo10, "} addMachine()\n");
    return machine;
}

/*********************************************************************************/
/* confirms that 'machine' is really free to be used.  It also 'preps' the machine
   if we feel it needs it. */
    
function _machManValidate(machMan, machine) {

    if (machine.self)            // the current machine is known to be good.
        return machine;

    if (machMan.wbem == undefined) 
        machMan.wbem = procGetWMI();

    if (!ping(machine.name, machMan.wbem)) {
        logMsg(LogTask, LogWarn, "_machManValidate: machine ", machine.name, " is not pingable\n");
        return undefined;
    }

    // insure that nothing was left over from before
    // We probably want to be a bit more precise about what we kill, however

    // FIX NOW put this back
    // runRemoteTerminateAll(machine.name);

    // Right now we just ping the machine, but you could imagine
    //         checking that its uptime is not long.
    //        That certain DLLs are not locked
    //        That some trial command worked.

    return machine;
}

/*********************************************************************************/
function machManFind(machMan, typePat, tag, shouldThrow) {

    logMsg(LogTask, LogInfo1000, "In machManFind(", machMan, ", ", typePat, ", ", tag, ", ", shouldThrow, "){\n");
    if (shouldThrow == undefined)
        shouldThrow = true;
    var retMach = undefined;
    var aMachineExists = false;
    var fLocalMachineSearch = (typeof (typePat) == "string" && typePat == "local");
    
    for (var type in machMan.byType) {
        var machines = machMan.byType[type];
        var matched = (typePat == undefined || fLocalMachineSearch);
        if (!matched) {
            if (typeof (typePat) == "string")
                matched = (type == typePat);
            else
                matched = type.match(typePat);
        }
        if (matched) {
            for (var i = 0; i < machines.length; i++) {
                var machine = machines[i];
                if (fLocalMachineSearch && !machine.self)
                    continue;
                if (!machine.inUse) {
                    logMsg(LogTask, LogInfo, "machManFind: name ", machine.name, " can be used for ", tag, "\n");
                    if (_machManValidate(machMan, machine)) {
                        aMachineExists = true;
                        retMach = machine;
                        retMach.inUse = tag
                        break;
                    }
                }
                else {
                    logMsg(LogTask, LogInfo1000, "machManFind: name ", machine.name, " matches ", typePat, " but is in use\n");
                    aMachineExists = true;
                }
            }
            if (retMach != undefined)
                break;
        }
    }

    if (!aMachineExists && shouldThrow) 
        throw new Error(1, "No machine of type '" + typePat + "' available");

    logMsg(LogTask, LogInfo1000, "} machManFind() = ", retMach, "\n");
    return retMach;
}

/****************************************************************************/
function machManRelease(machMan, machine) {
    
    logMsg(LogTask, LogInfo, "machManRelease: getting ", machine, " back\n");
    machine.inUse = undefined;
}

/****************************************************************************/
function jobPoll(job) {

    var abortFile = job.env.outDir + "\\abort.txt";
    if (FSOFileExists(abortFile)) {
        logMsg(LogTask, LogWarn, "jobPoll: ****** Found ", abortFile, " MANUALLY ABORTING ALL TASKS\n");
        job.abortFile = abortFile;
        _tasksTerminate([job.task]);
        job.task.exitCode = -1;
        return true;
    }

    var ret = _taskPoll(job.task, job.env, undefined, job.machMan, job, job.visitNum++);
    var now = new Date().getTime();        // update the web page if necessary

    if (!(now < job.nextReportTime)) {
        if (logGetFacilityLevel(LogTask) >= LogInfo100) 
            logMsg(LogTask, LogInfo100, "jobPoll:  job structure {\n", dump(job), "\n}\n");

        logMsg(LogTask, LogInfo10, "jobPoll: Refreshing ", job.report, " in ", job.task.outDir, "\n");
        _taskReportNoFail(job.task.outDir, job.report);
        job.nextReportTime = now + (2 * 60 * 1000);

    }
    return ret;
}

/****************************************************************************/
function _tasksRemoveDups(taskList, byName) {
    logMsg(LogTask, LogInfo100, "_tasksRemoveDups: len = ", taskList.length, " {\n");
    for (var i = 0; i < taskList.length; i++) {
        var task = taskList[i];
        if (task == undefined) continue;

        if (task.visitNum != -2) {
            task.visitNum = -2;
            var taskDef = byName[task.name.toLowerCase()];
            if (taskDef) {
                logMsg(LogTask, LogInfo100, "_tasksRemoveDups: found reference to ", task.name, "\n");
                if (!taskDef.objID && task.objID)
                    memberWiseCopy(taskDef, task);
                else 
                    logMsg(LogTask, LogInfo100, "_tasksRemoveDups: found a duplicate definition of ", task.name, " ids ", taskDef.objID, " and ", task.objID, "\n");
                
                taskList[i] = taskDef;
            }
            else {
                logMsg(LogTask, LogInfo100, "_tasksRemoveDups: task ", task.name, " defined by ", task.objID, "\n");
                byName[task.name.toLowerCase()] = task;
                if (task.dependents) 
                    _tasksRemoveDups(task.dependents, byName);
                }
        }
    }
    logMsg(LogTask, LogInfo100, "} _tasksRemoveDups\n");
}

/****************************************************************************/
/* Run a set of tasks  

  Parameters
    tasks    : an array of tasks to run
    outDir   : The directory to place output information. *.output.log 
               *.status.log, taskreport.html all go here.  This variable
               is also passes as %outDir% to the specific task command
    srcBase  : The directory passed as %srcBase% to the task
    env      : A table of name->value pairs. Any string of the form %name%
               on the task command line is replaced with its value from
               this mapping.
    machMan  : A machine manager that indicates which machines are available
               for the tasks.  If not provided, it is assumed only the
               current machine is available 
*/
function tasksRun(tasks, outDir, srcBase, env, machMan) {

    logMsg(LogTask, LogInfo10, "tasksRun([", tasks, "], ", outDir, ", ", srcBase, ", ", env, ")\n");
    if (machMan == undefined)
        machMan = machManNew();

    _tasksRemoveDups(tasks, {});

    // logMsg(LogTask, LogInfo, "tasks = ", dump(tasks), "\n");
    var task;
    if (tasks.length == 1)
        task = tasks[0];
    else 
        task = taskGroup("unnamed", tasks);

    taskEnv = memberWiseCopy({}, env);
    taskEnv.outDir = outDir;
    taskEnv.srcBase = srcBase;

    job = jobNew(task, taskEnv, machMan);

    while(!jobPoll(job)) {
        WScript.Sleep(1000);
    }

    return task.exitCode;
}

/****************************************************************************/
/* get the stable directory for 'dir' (since run.current can keep changing
   we want to get something that is stable for URLs */

function _getStableDir(dir) {

    var runInfo = dir + "\\runInfo.bat";
    if (FSOFileExists(runInfo)) {
        var runInfoData = FSOReadFromFile(runInfo);
        if (runInfoData.match(/^set STABLE_RUN_DIR=(.*?)\s*$/im)) {
            var stableDir = RegExp.$1;
             if (FSOFolderExists(stableDir))
                 dir = stableDir;
        }
    }
    logMsg(LogTask, LogInfo1000, "_getStableDir: returning: ", dir, "\n");
    return dir;
}

/****************************************************************************/
/* TODO remove */
function DBGtask() {


    var apply = taskNew("apply", "echo Applying");

    var task = taskNew("parent", "ping /n 1 localhost", [
        taskNew("child1", "ping /n 10 localhost"),
        // taskNew("child2", "ping /n 2 localhost & exit 4"),
        taskNew("child3", "ping /n 3 localhost"),
        // taskNew("child5", "ping /n 4 localhost"),
        // taskNew("child4", "ping /n 7 localhost"),
    ]);
    task.eagerAbort = true;
    task.ignoreChildFail = true;

        // task.eagerAbort = true;
    var machMan = machManNew();
    machManAdd(machMan, "vancem3", "x86", 2000);
    // machManAdd(machMan, "clrsnap3x8", "x86", 2000);
    // machManAdd(machMan, "clrsnap3x10", "x86", 2000);

    var taskEnv = {};
    taskEnv.outDir = "testing";
    taskEnv.srcBase = "srcBase";

    job = jobNew(task, taskEnv, machMan);
    while(!jobPoll(job)) {
        WScript.Sleep(1000);
    }

    logMsg(LogTask, LogInfo, "dump of job {\n", dump(job), "\n}\n");
    logMsg(LogTask, LogInfo, "writing XML to job.xml\n");
    xmlWrite(job, "job.xml", "job", {run:1, visitNum:1, typePat:1, nextReportTime:1, logFileName:1, machine:1, wbem:1} );
    return task.exitCode;
}

/****************************************************************************/
/* Sets the necessary flags on a task group so that if any child task 
   fails, then all tasks are aborted. */

function setTaskEagerAbort(task) {

    task.eagerAbort = true;

    if ((task.dependents != undefined) && (task.dependents.length != undefined)) {

        for (var i = 0; i < task.dependents.length; i++) {

            var dep = task.dependents[i];

            if (dep == undefined) {
                continue;
            }
        
            task.dependents[i] = setTaskEagerAbort(dep);
        }
    }

    return task;
}

/*****************************************************************************/
/* simple function that turns obj int a array (list).  If it is undefined it 
   list is empty, if it s not an array it is a one element list, otherwise it
   is obj itself */

function toList(obj) {
    if (obj == undefined)
        return [];
    if (obj.length > 0)
        return obj;
    else
        return [obj]
}

/*****************************************************************************/
function smartyDiffFile(snapDir, baselineDir, showBugInfo) {
    if (snapDir == undefined)
        throw new Error(1, "Required arg 'snapDir' not present");
    if (baselineDir == undefined)
        baselineDir = snapDir + "\\baseline";
    if (showBugInfo == undefined)
        showBugInfo = true;
    

    var reportDir = WshShell.CurrentDirectory;
    var htmlFileName = reportDir + "\\out.html";

    var htmlFile = FSOOpenTextFile(htmlFileName, 2, true); 
    htmlFile.WriteLine("<HTML><BODY>");
    htmlFile.Write(_smartyDiffReport(snapDir, baselineDir, reportDir, showBugInfo));
    htmlFile.WriteLine("</BODY></HTML>");
    htmlFile.Close();
    launchIECmd(htmlFileName);
}

/*****************************************************************************/
function _smartyDiffReport(snapDir, baselineDir, reportDir, showBugInfo, smartyData) {

    var html = "";

    var smartyDataFileName = snapDir + "\\Smarty.err.xml";
    if (!FSOFileExists(smartyDataFileName))
        return ""
    
    if (smartyData == undefined)
        smartyData = xmlRead(smartyDataFileName);

    var testsInfo = smartyData.TESTRESULT.MYBLOCK.TESTCASES;
    var tests = toList(testsInfo.TESTCASE);

    var baselineDataFileName = baselineDir + "\\Smarty.xml";
    var baselineTestsInfo = { TOTAL:0, PASSED:0, FAILED: 0}
    var baselineTests = [];
    var baselineData = undefined;
    if (FSOFileExists(baselineDataFileName)) {
        baselineData = xmlRead(baselineDataFileName);
        baselineTestsInfo = baselineData.TESTRESULT.MYBLOCK.TESTCASES;
        baselineTests = toList(baselineTestsInfo.TESTCASE);
    }

    var baselineTable = {}
    for(var i = 0; i < baselineTests.length; i++) {
        var baselineTest = baselineTests[i];
        baselineTable[baselineTest.TESTID] = baselineTest;
    }

    for(var i = 0; i < tests.length; i++) {
        var test = tests[i];
        if (test.SUCCEEDED == "yes")
            continue;

        var priority = "";

        if (test.PASSRATE == "0%")
            priority += "Deterministic    ";
        else 
            priority += "Nondeterministic "

        var baselineTest = baselineTable[test.TESTID];
        if (baselineTest != undefined) {
            if (baselineTest.SUCCEEDED == "yes")
                priority += "1:DifferFromBaseline";
            else 
                priority += "2:SameAsBaseline    ";
        }
        else 
            priority += "3:NoBaseline       ";

        test.INVESTIGATIONPRIORITY = priority;
        test.BASELINE = baselineTest;
    }

    tests.sort(function(test1, test2) { 
        if (test1.INVESTIGATIONPRIORITY > test2.INVESTIGATIONPRIORITY)
            return 1;
        else if (test1.INVESTIGATIONPRIORITY < test2.INVESTIGATIONPRIORITY)
            return -1;
        return 0;
    });


    var testsReport = smartyData.TESTRESULT.MYBLOCK.HARNESS_INFO.HARNESS_LOG;

    if (baselineData != undefined) {

        html += "Smarty test harness will rerun failed test again for flushing out non-deterministic failures.  ";
        html += "It also has logic for helping to determine if a failure ";
        html += "is caused by local changes or not.   If tests fail during the run ";
        html += "a 'baseline' runtime is installed, and all failed tests are rerun on the baseline.";
        html += "The following are the results of running the failed tests twice as well as running them ";
        html += "again against a baseline (a runtime installed from SNAP that has no local changes). <BR><BR>";
        
        html += "If a failed test passed against baseline runtime, then the root cause is most likely with your local changes. ";
        //I'm using "showBugInfo" as code for "I ran SelfHost" here.
        if (showBugInfo)
        {
            html += "On the other hand, if the test also failed against the baseline runtime, then it could be because of: <BR><BR>" + 
            "   a) A product bug <BR>" + 
            "      In an ideal world, any product bugs that managed to get through the checkin validation system" +
            "      will be caught by post-checkin validation systems such as rolling test run or in the BVT lab." +
            "      But it is possible that either one of the validation system is not online or your DDR run caught " +
            "      this first. <BR><BR>" + 
            "   b) A machine/config issue<BR>" + 
            "      This usually suggests a problem that the test may need to handle.  <BR><BR>";

            html += "If there are no existing bugs for baseline failures, a bug has been opened on your behalf. Usually, the test owner will do the " + 
                "      investigation and triage the bug. Once the bug is resolved, you will have the resposibility " +
                "      to close the bug. Collectively, this helps us to maintain a high quality product.<BR>"; 

            html += "<A HREF=http://devdiv/sites/CLR/Bugs%20and%20DCRs/Bug%20Processes/Daily%20Dev%20Run%20Bug%20FAQ.doc>DDR-DevBVT Bug FAQs</A> <BR>";
        }
        else
        {
            html += "On the other hand, if the test also failed against the baseline runtime, then it could be because of: <BR><BR>" + 
            "   a) A product bug <BR>" + 
            "      Since you are running SelfHost tests from the live test tree, these failures could be caused" +
            "      by either a new or existing product bug.  Since you just ran the same tests run by the" +
            "      Rolling Test System, it's likely that a tester is simultaneously investigating the same " +
            "      failure you are.<BR><BR>" +
            "   b) A machine/config issue<BR>" + 
            "      This could indicate something about your machine configuration that the test is unable " +
            "      handle.  Alternatively, the tests are designed to run in a particular configuration " +
            "      that may only be available in the lab.  If that's the case, then this test will never " +
            "      pass on your dev machine.<BR><BR>" +
            "   c) A test issue<BR>" +
            "      The tests in the self host suite are not held to the same standard of reliability to " +
            "      which we hold the Checkin BVT and Build BVT tests.  Because of this, you should expect " +
            "      more non-determinism from these tests.<BR><BR>" +
            "\r\n" +
            "Please keep this in mind before you investigate the failures (or engage your test counterpart)" +
            "<BR><BR>\r\n";
            html += "<A HREF=http://mswikis/clr/dev/Pages/Running%20Self-Host.aspx>Self Host FAQ</A><br>"
        }
    }
    else {
        html += "The following is full smarty result as well as a summary of the failures\r\n";
    }
    html += "<BR>\r\n";

    var numNotRun = testsInfo.TOTAL - testsInfo.FAILED - testsInfo.PASSED;
    html += "<A HREF='" + relPath(testsReport, reportDir) + "'> Smarty Report </A>&nbsp;&nbsp;";
    html += "<FONT COLOR=RED>   Tests Failed: " + testsInfo.FAILED + "</FONT>&nbsp;&nbsp;";
    html += "<FONT COLOR=990033>Tests NotRun: " + numNotRun + "</FONT>&nbsp;&nbsp;";
    html +=                    "Tests Passed: " + testsInfo.PASSED + "<BR>\r\n";

    if (baselineData != undefined) {
        var baselineReport = baselineData.TESTRESULT.MYBLOCK.HARNESS_INFO.HARNESS_LOG;
        var baselineNotRun = baselineTestsInfo.TOTAL - baselineTestsInfo.FAILED - baselineTestsInfo.PASSED;
        html += "<A HREF='" + relPath(baselineReport, reportDir) + "'> Baseline Report </A>&nbsp;&nbsp;";
        html += "<FONT COLOR=RED>   Tests Failed: " + baselineTestsInfo.FAILED + "</FONT>&nbsp;&nbsp;";
        html += "<FONT COLOR=990033>Tests NotRun: " + numNotRun + "</FONT>&nbsp;&nbsp;";
        html +=                    "Tests Passed: " + baselineTestsInfo.PASSED + "<BR>\r\n";

        var baselineInfoFile = baselineDir + "\\baselineBinDir.txt";
        if (FSOFileExists(baselineInfoFile)) {
            if (FSOReadFromFile(baselineInfoFile).match(/BASELINE_BINARIES: *(.*)/)) {
                var baselineBinDir = RegExp.$1;
                html += "The baseline binaries are from " + baselineBinDir + "<BR>\r\n";
            }
        }
    }

    var connectedToDatabase = false; 
    var testCaseActiveBugs = null;
    try
    {
        if (_inTFS()) {
            testCaseActiveBugs = testCaseBugWithCache(true);
            connectedToDatabase = true; // In the TFS case, this flag indicates there was no exception thrown.
        }
        else {
            var bugDb = bugConnect(bugDatabase);
            testCaseActiveBugs = testCaseBugWithCache(true, bugDb);
            connectedToDatabase = true;
        }
    }
    catch(e)
    {
        connectedToDatabase = false;
    }
    var branchName;
    var syncLists;
    var syncDate;
    var sdRoot;
    var sdObj = null;
    if (!_inTFS())
    {
        sdObj = sdConnect(smartyData.TESTRESULT.LST_FILE);
        sdRoot = sdGetRoot(reportDir, sdObj);
    }
    if(sdRoot != null)
        branchName = _getBranchFromSrcBase(sdRoot);
    if(branchName == undefined)
    {
        if (_inTFS())
            branchName = rollingBranchTFS;
        else
            branchName = rollingBranch;
    }
    if(sdRoot != null)
        syncLists = sdWhereSynced(sdRoot + "\\ndp\\clr\\src", sdObj);
    if (syncLists)
        syncDate = new Date(syncLists.minChangeTime);
    else
        syncDate = new Date();

    var createBugHtml;
    if (showBugInfo)
    {
        createBugHtml = "<LI><B>Create/Update bugs:</b> DDR no longer automatically updates bugs. ";
        createBugHtml+= "If you want to update the \"Related Bug\" by incrementing the hit count and ";
        createBugHtml+= "adding your DDR logs to the bug, cut and paste the following commands in a ";
        createBugHtml+= "in a clrenv window.<br> If the \"Related Bug\" does not appear to be relevant ";
        createBugHtml+= "to your test failures, you may create a new bug by cutting and pasting the ";
        createBugHtml+= "following commands in a clrenv window and adding a \"true\" argument to the end.<br>";
        createBugHtml+= "Task report generation is intentionally independent of a \"branch\". The command ";
        createBugHtml+= "below has determined that the correct branch to create a bug in is ";
        createBugHtml+= "<B>" + branchName + "</B>.  If this is incorrect you may update the relevant <B>bold</B> ";
        createBugHtml+= "section below.";
        createBugHtml+= "<UL>";
    }
    else
    {
        createBugHtml = "";
    }

    if (tests.length != 0) {
        html += "<TABLE BORDER>\r\n";
        html += "<TR><TH ROWSPAN=\"2\">Test Name</TH> <TH ROWSPAN=\"2\"> FailureType </TH> <TH COLSPAN=\"2\"> Test Results </TH>";
        html += "<TH COLSPAN=\"2\">Related Bugs </TH>";
        if (showBugInfo)
        {
            html += "<TH COLSPAN=\"2\">Follow-up</TH>";
        }
        else
        {
            html += "<TH ROWSPAN=\"2\">Mail (Test Owner)</TH>";
        }

        html += "</TR><TR><TH>Local </TH><TH> Baseline </TH><TH>Assigned to you</TH><TH>Others</TH>";
        if (showBugInfo)
        {
            html += "<TH>Create/Update Bug (" + branchName + ") </TH><TH>Mail (Test Owner) </TH>";
        }
        html += "</TR>\r\n";

        var myDisplayName = displayNameLookup(Env("USERNAME"));

        for(var i = 0; i < tests.length; i++) {
            var test = tests[i];
            if (test.NAME == undefined)
                continue;
            var color;
            if( test.INVESTIGATIONPRIORITY.match(/SameAsBaseline/i) )
            {
                color = "BLUE";
            }
            else
            {
                color = "RED";
            }

            html += "<TR>";

            // Test Name column
            html += "<TD><FONT COLOR=" + color + ">" + nameFromCmdLine(test.NAME) + "</FONT></TD>";

            // FailureType Column
            html += "<TD ALIGN=CENTER><FONT COLOR=" + color + ">" + test.INVESTIGATIONPRIORITY + "</FONT></TD>";

            // Test Results Columns
            var mailToBody = "DailyDevRun has encountered a ";
            if (test.PASSRATE == "0%") {
                mailToBody += "Deterministic ";
            } else {
                mailToBody += "Non-Deterministic ";
            }
            mailToBody += "failure for test " + test.TESTID + "\n\n";
            mailToBody += "Test failed on\n" +
                            "    MACHINE: " + Env("COMPUTERNAME") + "\n" +
                            "    USER: " + Env("USERNAME") + "\n\n" +
                            "Recent DDR logs are available at \\\\clrmain\\public\\DailyDevRuns\\logs\n\n";

            // Test Results Column -> Local Results sub-column
            html += "<TD ALIGN=CENTER>";
            if(test.RUN != undefined)
            {
                var runs = toList(test.RUN);
                for (var j = 0; j < runs.length; j++) {
                    var run = runs[j];
                    var result = (run.SUCCEEDED == 'yes') ? "PASS" : "FAIL";
                    html += "<A HREF='" + relPath(run.TESTLOG, reportDir) + "'> " + result + "</A>";
                    mailToBody += "DDR local failure log: " + relPath(run.TESTLOG, reportDir) + "\n";
                    if (j + 1 < runs.length)
                        html += ", ";
                }
            }
            else
            {
                html += "&nbsp";
            }
            html += "</TD>";

            // Test Results Column -> Baseline Results sub-column
            html += "<TD ALIGN=CENTER>";
            if (test.BASELINE != undefined && test.BASELINE.RUN != undefined) {
                var runs = toList(test.BASELINE.RUN);
                for (var j = 0; j < runs.length; j++) {
                    var run = runs[j];
                    var result = (run.SUCCEEDED == 'yes') ? "PASS" : "FAIL";
                    html += "<A HREF='" + relPath(run.TESTLOG, reportDir) + "'>" + result + "</A>";
                    mailToBody += "DDR baseline failure log: " + relPath(run.TESTLOG, reportDir) + "\n";
                    if (j + 1 < runs.length)
                        html += ", ";
                }
            }
            else
            {
                html += "&nbsp";
            }
            html += "</TD>";

            // Look for any bugs for this test 
            var haveBugs = false;
            var myBugs = "";
            var otherBugs = "";
            if(connectedToDatabase)
            {            
                var existingBugs = testCaseBugByTest(testCaseActiveBugs, test.TESTID);
                if(existingBugs)
                {
                    haveBugs = true;
                    var existingBug = "";
                    var bugIndex = "";
                    for(bugIndex = 0; bugIndex < existingBugs.length; bugIndex++)
                    {
                        existingBug = existingBugs[bugIndex];

                        //determine if I created it or if someone else did.
                        var bugHtml = "<A HREF='http://vstfdevdiv.redmond.corp.microsoft.com:8080/DevDiv2/web/wi.aspx?id=" + existingBug.ID + 
                            "' TARGET='_blank'>"+existingBug.ID +"</A>&nbsp;";

                        if (myDisplayName == existingBug.AssignedTo)
                        {
                            myBugs += bugHtml;
                        }
                        else {
                            otherBugs += bugHtml; 
                        }
                    }
                }
                if(myBugs == "")
                    myBugs = "&nbsp";
                if(otherBugs == "")
                    otherBugs = "&nbsp";

                // Related Bugs Columns
                html += "<TD ALIGN=CENTER>" + myBugs + "</TD><TD ALIGN=CENTER>" + otherBugs + "</TD>";
            }
            else
            {
                html += "<TD ALIGN=CENTER>&nbsp;</TD><TD ALIGN=CENTER>&nbsp;</TD>";
            }

            var mailToSubject = "DDR failure (" + test.TESTID + ")";
            var mailToRecipients = test.TESTOWNER.split(";");
            for(var m in mailToRecipients)
                mailToRecipients[m] += "@microsoft.com";
            mailToRecipients = mailToRecipients.join(",");

            // Follow-up/Create Bug Column
            if (showBugInfo)
            {
                html += "<TD ALIGN=CENTER><A HREF='#CreateBug." + test.TESTID + "'>Bug Commands</A></TD>";
            }
            // Follow-up/Mail Owner column
            html += "<TD ALIGN=CENTER><A HREF='mailto:" + mailToRecipients + "?subject=" + escape(mailToSubject) + "&body=" + escape(mailToBody) + "'>" + test.TESTOWNER + "</A></TD>";
            html += "</TR>\r\n";
            
            var failedLogs = [];
            _getLogs(failedLogs, test);
            _getLogs(failedLogs, baselineTable[test.TESTID]);

            var fileList = "";
            if(failedLogs.length > 0)
            {
                fileList = failedLogs.join(",");
            }
            var isDeterministic = (test.PASSRATE == "0%");
            if (showBugInfo)
            {
                createBugHtml += "<A NAME='#CreateBug." + test.TESTID + "'>";
                createBugHtml += "<LI><B>For test case (" + test.NAME + ") ";
                createBugHtml += "run the following command:</B>";
                createBugHtml += " <CODE><BR/>runjs formatAndCreateDDRBug";
                createBugHtml += " \"" +  test.NAME + "\"";
                createBugHtml += " \"" + test.TESTID + "\"";
                createBugHtml += " \"" + test.TESTOWNER + "\"";
                createBugHtml += " \"<B>" + branchName + "</B>\"";
                createBugHtml += " \"" + syncDate + "\"";
                createBugHtml += " \"" + isDeterministic + "\"";
                createBugHtml += " \"" + fileList + "\"";
                createBugHtml += " \"" + new Date().toString() + "\"</CODE>";
            }
        }
        if (showBugInfo)
        {
           createBugHtml += "</UL>";
        }
       html += "</TABLE>\r\n";
       html += createBugHtml;
    }
    return html;
}

function _getLogs(failedLogs, test)
{
    if(test != undefined && test.RUN != undefined)
    {
        var runs = toList(test.RUN);
        for (var i = 0; i < runs.length; i++) {
            var run = runs[i];

            if (run.SUCCEEDED == "yes") {
                continue;
            }

            var log = run.TESTLOG;
            failedLogs[failedLogs.length] = log;
        }
    }
}
