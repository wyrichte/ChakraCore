/*********************************************************************************/
/*                                 proc.js                                        */
/*********************************************************************************/

/* utilities for accessing and controlling processes */ 

/* Note that that uses the windows WMI (comes with XP by default) */

/* The wbemServcies object for the local machine can be created by
   doing 

        wbemServices = GetObject("winmgmts://.");

   You can connect to the object for other machines by replacing the '.' with
   a machine name.
*/

/* The process Object returned by these API's is the Win32_Process WMI object
   Use 'Win32_Process' in a google search to find more information on what
   properties this has.  It has everything in taskManager and more.  Here are
   some of the more interesting ones

        var arr = procGetAll(wbemServices);
        arr[0].ProcessID;
        arr[0].CreationDate;     (returns a string YYYYMMDDHHMMSS.mmmmmmm)
        arr[0].KernelModeTime;
        arr[0].UserModeTime;
        arr[0].ParentProcessID;
        arr[0].CommandLine;
          arr[0].ExecutablePath;
        arr[0].TerminationDate;
        arr[0].Terminate(errorCode);
        arr[0].SetPriority(pri);
*/

// Author: Vance Morrison
// Date    1/1/2003
// DEPENDANCIES 
//         log.js

/*********************************************************************************/
var procModuleDefined = 1;         // Indicate that this module exists
if (!logModuleDefined) throw new Error(1, "Need to include log.js");

var procLocalWMI;
var LogProc = logNewFacility("proc");

/*********************************************************************************/
/* returns the WMI object needed by all the 'proc' routines.  The operations will
   be done on 'hostName' (if hostname is undefined, it is the current host).  
*/

function procGetWMI(hostName) {

    if (hostName == undefined)
        hostName = ".";            // The current host

    if (hostName == "." && procLocalWMI)
        return procLocalWMI;

    var ret = GetObject("winmgmts://" + hostName);
    if (!ret) 
        throw new Error(1, "Could not open the winmgmts object! (Someone corrupted XP system dlls, or WMI not installed (on Win9x)?)")
        
    if (hostName == ".")
        procLocalWMI = ret;
    return ret;
}

/******************************************************************************/
/* return a list of process ids that coorespond to proceses that have loaded
   'modulePath'.  ModulePath can be a complete path name or just the last 
   component */
function procGetWithModule(modulePath) {

    var ret = [];
    var tlistExe = ScriptDir + "\\" + Env("PROCESSOR_ARCHITECTURE") + "\\tlist.exe";
    var out = runCmd(tlistExe + " /m " + modulePath).output;
    if (out.match(/^No tasks found using/i))
        return ret;
    var lines = out.split(/\s*\n/);
    for(var i = 0; i < lines.length; i++) {
        var line = lines[i];
        if (line.match(/^(.*?)\s+-\s+(\d+)\s+(\w+)/)) 
            ret.push(RegExp.$2);
        else 
            logMsg(LogProc, LogWarn, "Could not parse tlist /m output ", line, "\n");
    }
    return ret;
}

/******************************************************************************/
/* kill all processes who have loaded module 'modulePath'.  
 */ 
function procKillWithModule(modulePath) {

    logMsg(LogProc, LogInfo, "procKillWithModule(", modulePath, ") {\n");
    var wbemServices = procGetWMI();
    var procIds = procGetWithModule(modulePath);
    for(var i = 0; i < procIds.length; i++) {
        var procId = procIds[i];
        var proc = procGetByPid(procId, wbemServices);
        procKill(procId, wbemServices, proc);
    }
    logMsg(LogProc, LogInfo, "} procKillWithModule\n");
}

/******************************************************************************/
/* kill all orphan processes (those whose parents have died).  Note that
   explorer and System are treated specially and are never killed */

function procKillOrphans(wbemServices) {

    logMsg(LogProc, LogInfo, "procKillOrphans\n");
    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    var procsAsTree = procGetProcsAsTree(wbemServices);
    for(var i = 0; i < procsAsTree.topProcsInfos.length; i++) {
        var topProcInfo = procsAsTree.topProcsInfos[i];
        if (topProcInfo.cmdLine == null || topProcInfo.cmdLine.match(/^\S*explorer/i))
            continue;

        logMsg(LogProc, LogInfo, "procKillOrphans: killing orphan ", topProcInfo.pid, " CMD: ", topProcInfo.cmdLine, "\n");
        procKill(topProcInfo.pid, wbemServices, topProcInfo.proc);
    }
}

/******************************************************************************/
/* force a reboot of the computer to reboot the current computer do. returns 0 
   on success
*/

function procForceReboot(wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    logMsg(LogProc, LogInfo1000, "In procForceReboot(wbemServices)\n");

    // ShutdownPrivilege is required and I guess isn't included in default 'GetObject'
    // see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmisdk/wmi/wmi_tasks__desktop_management.asp
    // AddAsString returns null on failure, previous privilege on success
    var privilege = wbemServices.Security_.Privileges.AddAsString("SeShutdownPrivilege");
    if (privilege == undefined || privilege == null) {
        logMsg(LogProc, LogInfo, "Failed to set SeShutdownPrivilege for WMI object\n");
    }
    
    var wbemObjectSet = wbemServices.ExecQuery("select * from Win32_OperatingSystem where Primary=true");
    var ret = 0;
    for (var e = new Enumerator(wbemObjectSet); !e.atEnd(); e.moveNext()) {
        var osObj = e.item();
        var myRet = osObj.Win32Shutdown(6, 0);        // Force a reboot
        if (myRet != 0)
            ret = myRet;
    }
    logMsg(LogProc, LogInfo, "procForceReboot(wbemServices) = ", ret, "\n");
    return ret;
}

/******************************************************************************/
/* ping a computer.  returns true if alive 
   Note that wbemServices defines where the ping starts from 
   Will take 2 sec to fail if the computer is unresponsive, success is fast */

function ping(target, wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    logMsg(LogProc, LogInfo1000, "ping(wbemServices, ", target,  ")\n");
    var wbemObjectSet = wbemServices.ExecQuery("select * from Win32_PingStatus where Address='"+ target + "'");
    var ret = false;        // by default we failed
    for (var e = new Enumerator(wbemObjectSet); !e.atEnd(); e.moveNext()) {
        var reply = e.item();
        logMsg(LogProc, LogInfo1000, "Got Reply " + reply.StatusCode + "\n");
        if (reply.StatusCode === 0)
            ret = true;
    }
    logMsg(LogProc, LogInfo10, "ping(wbemServices, ", target,  ") = " + ret + "\n");
    return ret;
}
        
/******************************************************************************/
/* return an array of process objects every process currently running in the system */

function procGetAll(wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    logMsg(LogProc, LogInfo100, "procGetAll(wbemServices) {\n"); 

    var wbemObjectSet = wbemServices.InstancesOf("Win32_Process");
    var ret = [];
    for (var e = new Enumerator(wbemObjectSet); !e.atEnd(); e.moveNext()) {
        var procObj = e.item();
        logMsg(LogProc, LogInfo10000, "procGetAll: Got ", procObj.ProcessID, " Name ", procObj.Name, " Command: ", procObj.CommandLine, "\n");
        ret.push(procObj);
    }
    logMsg(LogProc, LogInfo100, "} procGetAll()\n"); 
    return ret;
}

/*****************************************************************************/
/* given a process ID, find the cooresponding process object */

function procGetByPid(pid, wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    var e = new Enumerator(wbemServices.ExecQuery("select * from Win32_Process where ProcessID = " + pid));
    var ret = undefined;
    if (!e.atEnd())
        ret = e.item();

    logMsg(LogProc, LogInfo10000, "procGetByPid(", pid, ", wbemServices) = ", (ret == undefined)?undefined:ret.Name, "\n");
    return ret;
}

/******************************************************************************/
/* return an array of process objects whose command line match 'pat' */

function procFind(pat, wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();
    logMsg(LogProc, LogInfo100, "procFind(", pat, ", wbemServices) {\n"); 

    var ret = [];
    var procs = procGetAll(wbemServices);
    for (var i = 0; i < procs.length; i++) {
        var proc = procs[i];
        var cmdLine = proc.CommandLine;
        logMsg(LogProc, LogInfo10000, "procFind CMD: ", cmdLine, "\n"); 

        if (cmdLine && cmdLine.match(pat)) {
            logMsg(LogProc, LogInfo1000, "procFind Matched: ", cmdLine, "\n"); 
            ret.push(proc);
        }
    }
    logMsg(LogProc, LogInfo100, "} procFind() length = ", ret.length, "\n"); 
    return ret;
}

/******************************************************************************/
/* returns the creation time of a process 'proc' the number of millisecionds
   from 1970 (normal JSCRIPT Date format).  */

function _procGetCreateTime(proc) {
    var time = proc.CreationDate;
    if (time == undefined)
        return 0;

            // WMI returns dates as a string of this form
    if (!time.match(/(\d\d\d\d)(\d\d)(\d\d)(\d\d)(\d\d)(\d\d)\.(\d\d\d)/)) 
        return 0

    var date = new Date(RegExp.$1, RegExp.$2-1, RegExp.$3, RegExp.$4, RegExp.$5, RegExp.$6, RegExp.$7);
    return date.getTime();         // return ms from 1970
}

/******************************************************************************/
/* given a process ID 'pid', return an array of process objects for the direct 
   children.  If the WMI process object for 'pid' is available, it can be 
   optionally passed as the 3rd parameter, which speeds up the routine */

function procGetChildren(pid, proc, wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    var ret = []
    logMsg(LogProc, LogInfo100, "procGetChildren(", pid, ", proc, wbemServices) {\n"); 

    if (proc == undefined) {
        proc = procGetByPid(pid, wbemServices);
    }
    var procCreateTime = 0;
    if (proc != undefined)
        procCreateTime = _procGetCreateTime(proc);

    var e = new Enumerator(wbemServices.ExecQuery("select * from Win32_Process where ParentProcessID = " + pid));
    while (!e.atEnd()) {
        var child = e.item();
        var childCreateTime = _procGetCreateTime(child);
        logMsg(LogProc, LogInfo10000, "procGetChildren: Got ", child.ProcessID, " ", child.Name, " Create ", childCreateTime, " Command: ", child.CommandLine, "\n");

            /* Since process IDs can be recycled, we confirm by checking the creation time */
        if (childCreateTime >= procCreateTime)
            ret.push(child);
        else 
            logMsg(LogProc, LogInfo100, "procGetChildren: Filtered out ", child.ProcessId, 
                                        " because ", childCreateTime, " < ", procCreateTime);
        e.moveNext();
    }
    logMsg(LogProc, LogInfo100, "} procGetChildren()\n");
    return ret;
}

/*****************************************************************************/
/* get the process ID the Deepest child from 'pid' (warns if this is 
   ambiguous) */

function procDeepestChild(pid) {

    for(;;) {
        var children = procGetChildren(pid);
        if (children.length == 0)
            break;
        if (children.length > 1)
            logMsg(LogClrAutomation, LogWarn, "procDeepestChild: process ", pid, " has more than one child!, picking first one\n");
        pid = children[0].ProcessID;
    }
    return pid;
}

/*****************************************************************************/
/* Given a windows managment service hande and a process ID, all its decendants.
   (but not itself, so you can all this on your process ID). It kills a parent
   before killing the child so the race you have is if the parent's process ID
   is reused before this all the children are dead.  This works reasonably
   well because the ParentProcessID field is never changed even if the parent
   dies.  Thus you can kill from the top down. If proc is defined it should
   be the process object for 'pid' and simply improves the efficiency of the
   routine.  

   Note that if processes die after spawning children this can break the 
   tree of creation, and thus not all processes may be killed that is desired.
   If 'includeOrphans' is true, this routine will also kill all orphans that
   have a creation time > the process with 'pid'  This is now an overestimation
   of 

Parameter:
      pid          : process id (decimal)
      wbemServices : handle to WMI object (returned from procGetWMI)
      proc           : proc object for 'pid' (usually left undefined).
*/
   
function procKillChildren(pid, wbemServices, proc, silent) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    logMsg(LogProc, LogInfo10, "procKillChildren(", pid, ", wbemServces) {\n"); 

    var children = procGetChildren(pid, proc, wbemServices);
    for (var i = 0; i < children.length; i ++) {
        var child = children[i];
        procKill(child.ProcessID, wbemServices, child, silent);
        procKillChildren(child.ProcessID, wbemServices, child, silent);
    }
    
   logMsg(LogProc, LogInfo10, "} procKillChildren()\n");
}


/*****************************************************************************/
/* kills the process 'pid' on the  machine specified by 'wbemServices'
   'proc' is optional, and is the WMI object associated with 'pid' */

function procKill(pid, wbemServices, proc, silent) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    if (proc == undefined)
        proc = procGetByPid(pid, wbemServices);

    if (proc == undefined) {
        logMsg(LogProc, LogWarn, "procKill: pid ", pid, " not found, assuming it just died.\n");
        return;
    }
    
    var cmdLine = proc.CommandLine;
    if (cmdLine == undefined)
        cmdLine = proc.Name
    if (cmdLine == undefined)
        cmdLine = "UNKNOWN";
    if (!silent)
        logMsg(LogProc, LogWarn, "procKill: ", pid, " CMD: " + cmdLine.substr(0, 48) + "\n");

    var createDate = proc.CreationDate;
        // TODO this error return is not the best, since this process
        // is not a scheduled task necessarily.  See if we can find a better one
    var SCHED_S_TASK_TERMINATED = 0x00041306;
    try { proc.Terminate(SCHED_S_TASK_TERMINATED); } catch(e) {}

    // give it time to die
    for(var i =0; ; i++) {
        WScript.Sleep(100);        
        proc = procGetByPid(pid, wbemServices);
        if (!proc || proc.CreationDate == createDate)
            break;

        if (i > 10) {
            logMsg(LogProc, LogError, "procKill: process still alive after 100ms...\n");
            break;
        }
        else if (i == 5)  {
            logMsg(LogProc, LogWarn, "procKill: process still alive afer 500ms, trying taskkill.exe.\n");
            runCmdToLog("taskkill /f /pid " + pid, runSetNoThrow());
        }
    }
}

/*****************************************************************************/
/* kills a process indicated by 'pid' (decimal) and all its children */

function procKillAll(pid, wbemServices, proc, silent) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    if (proc == undefined)
        proc = procGetByPid(pid, wbemServices);
    procKill(pid, wbemServices, proc, silent);
    procKillChildren(pid, wbemServices, proc, silent);
}

/*****************************************************************************/
/* GetProcsAsTree defines a processInfo Structure which has the fields
    
    processInfo: { 
        proc:         process object                 (the wbem structure above)
        pid:        process id
        cmdLine:    command line
        createTime:    creation time                (msec from 1970)
          children:     list of procInfo structures
    }

    It returns the following structure
    {
        pid2ProcInfo:     object mapping pids to processInfo stuctures above
        topProcsInfos:     list of processInfo's for processes without no parent
        date:            time/date this structure was capured
    }
*/

// TODO remove dbgLog
var dbgLog = [];
var dbgLogs = [];
var seq = 0;

function procGetProcsAsTree(wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    var procInfos = {};        // maps process ID to proc strucutre and children
    var deadPids = {};        // process IDs in this set are dead, but have live children
    var now = new Date();

    var procObjs = procGetAll(wbemServices);        // get all the processes
    logMsg(LogProc, LogInfo100, "procGetProcsAsTree: numProcs = ", procObjs.length, "\n");

    for (var i = 0; i < procObjs.length; i++) {
        var childObj = procObjs[i];

        var childPid = childObj.ProcessID;
        var parentPid = childObj.ParentProcessID;
        if (childPid == 0)    // This id has itself as a parent.  Avoid it
            continue;

        logMsg(LogProc, LogInfo100000, "procGetProcsAsTree: PID ", childPid, " PPID ", parentPid, " TIME = ", 
            childObj.KernelModeTime, " + ", childObj.UserModeTime, "\n");

        parentInfo = procInfos[parentPid];
        if (parentInfo == undefined) {
            logMsg(LogProc, LogInfo100, "procGetProcsAsTree: adding to deadPids ", parentPid, "\n");
            parentInfo = { children: [], objID: parentPid };
            procInfos[parentPid] = parentInfo;
            deadPids[parentPid] = true;            // At the moment, I am assuming the parent is dead since it is not in the list so far
        }

        var childInfo = procInfos[childPid];
        if (childInfo == undefined) {
            childInfo = { children: [] };
            procInfos[childPid] = childInfo;
        }
        deadPids[childPid] = undefined;            // OK, it is in the list, so if we thought it was dead, correct this.
        logMsg(LogProc, LogInfo100, "procGetProcsAsTree: removing from deadPids ", childPid, "\n");

        childInfo.proc = childObj;
        childInfo.pid = childPid;
        childInfo.objID = childPid;                // This is for nice dumping and XML writing
        childInfo.cmdLine = childObj.CommandLine;
        childInfo.createTime = _procGetCreateTime(childObj);

            // CPU time is in 100ns units, convert to Msec (turns out Windows only give you
            // granularities of 10ms anyway, so we are not loosing precision)
        childInfo.cpuTimeMs = (parseInt(childInfo.proc.KernelModeTime) + parseInt(childInfo.proc.UserModeTime)) / 10000;
        childInfo.proc.ReadTransferCount;     // Fetch these so if the proces goes away we might still get them
        childInfo.proc.WriteTransferCount;                                
        childInfo.proc.OtherTransferCount;                            
        childInfo.proc.PageFaults;                            

        // logMsg(LogScript,LogInfo, "Got ", childPid, " parent ", parentPid, "\n");
        parentInfo.children.push(childInfo);
    }

        // top processes are those that have dead parents
    var pid;
    var topProcs = [];
    for (pid in deadPids) 
        if (deadPids[pid]) 
            topProcs = topProcs.concat(procInfos[pid].children);

        // remove any false children caused by process ID reuse
    for (pid in procInfos) {
        var procInfo = procInfos[pid];
        for(var i = 0; i <procInfo.children.length;) { 
            var childInfo = procInfo.children[i];
            logMsg(LogProc, LogInfo100000, "procGetProcsAsTree: found child ", 
                childInfo.pid, " parent ", procInfo.pid, " childTime ",
                childInfo.createTime, " parentTime ", procInfo.createTime, "\n");
            if (childInfo.createTime < procInfo.createTime) {
                logMsg(LogProc, LogInfo100, "procGetProcsAsTree: removing child ", 
                    childInfo.pid, " from ", procInfo.pid, " because ",
                    childInfo.createTime, " < ", procInfo.createTime, "\n");
                procInfo.children.splice(i, 1);
                topProcs.push(childInfo);
            }
            else 
                i++;
        }
    }

    /***
    for(var i = 0; i < topProcs.length; i++) 
        logMsg(LogProc, LogInfo100, "procGetProcsAsTree: topProc = ", topProcs[i].pid, "\n");
    ***/
    
    ret = { pid2ProcInfo:procInfos, topProcsInfos:topProcs, date:now };
    ret.myseq = seq++;
    logMsg(LogProc, LogInfo100, "procGetProcsAsTree: seq = ", ret.myseq, "\n");

    // dbgLog.push(_logCurTime() + " procGetProcsAsTree returning {\n" + procTreePrint(ret) + "}\n");
    return ret;
}

/*****************************************************************************/
/* sends the output procTreeAsString to a file or to standard output (see
   procTreeAsString for details.
 */
function procTreePrint(optStr, outFile) {

    var str = procTreeAsString(optStr)
    if (outFile == undefined)
        WScript.Echo(str);
    else 
        FSOWriteToFile(str, outFile);
    return 0
}

/*****************************************************************************/
/* prints a procTree (returned by procGetProcsAsTree, in a nice way.  Basically
   this is a slightly nicer version of 'tlist /t'.  Like tlist /t it prints
   children right after the parent process and indents the children.  Unlike
   tlist /t it also gives the following columns
       pid                The process ID (in decimal
       RunTime            The timethet process has been running
       CPUTime            The amount of CPUT time the process consumed
       WS                 The working set (memory used) in Megabytes
       Command Line       By default this is a 'shortened' command line

   If 'procTree' is given that that is printed, otherwise the current set of
   processes is printed 
    
   options
       /machine:<name>    Operate on 'machine' instead of the local machine
       /fullCmdLine       Don't trucate command lines to fit in 80 chars
*/
function procTreeAsString(optStr, procTree) {

    var options = getOptions(["machine", "fullCmdLine", "m", "f"], optStr);
    if (options) {
        if (options["m"] && !options["machine"])
            options.machine = options.m;
        if (options["f"] && !options["fullCmdLine"])
            options.fullCmdLine = options.f;
    }
        
    if (procTree == undefined) {
        var wbemServices = procGetWMI(options.machine);
        var procTree = _procTreeGetStats(wbemServices) 
    }

    var str = " Proc  Run    CPU    WS  %  Command Line      (children indented after parents)\r\n" + 
              "  ID   Time   Time  Meg CPU                        " + (new Date(procTree.date)).toString() + "\r\n" +
              "-------------------------------------------------------------------------------\r\n";

    var procInfoPrev = undefined;
    for(var i = 0; i < procTree.topProcsInfos.length; i++) {
        var procInfo = procTree.topProcsInfos[i];
        str += _procTreePrintHelper(procInfo, "", options, procTree.date.getTime());
    }
    
    return str;
}

function _procTreePrintHelper(procInfo, indent, options, nowMs) {
    var cmdLine = procInfo.cmdLine;
    if (typeof(cmdLine) != "string") {
        cmdLine = procInfo.proc.Name;
    }

    var ret = padLeft(procInfo.pid, 5) + " ";

    ret += padLeft(_procTreeTime(procInfo.runTimeMs), 5) + "  ";

    ret += padLeft(_procTreeTime(procInfo.cpuTimeMs), 5) + " ";

    var workingSet = Math.round(procInfo.proc.WorkingSetSize / (1024 * 1024)).toFixed(0);
    if (workingSet.length > 3)
        runTimeMin = "***";
    ret += padLeft(workingSet, 3) + "M ";

    var percentCPU = procInfo.cpuPercent.toFixed(0);
    if (percentCPU.length > 2)
        percentCPU = "**";
    ret += padLeft(percentCPU, 2) + " ";

    if (options && !options.fullCmdLine) {                    // morph the command line to make it fit in 80 cols
        var cmdLine = cmdLine.replace(/"[^"]+\\([^"\s]+)"/g, "$1");        
        cmdLine = cmdLine.replace(/^(\S+).exe/i, "$1");
        cmdLine = cmdLine.replace(/\S+\\/g, "");
        cmdLine = cmdLine.replace(/\s+/g, " ");

        cmdLine = indent + cmdLine;
        if (cmdLine.length + ret.length >= 80) 
            cmdLine = cmdLine.substr(0, 76 - ret.length) + "..."
    }
    else 
        cmdLine = indent + cmdLine;
    ret += cmdLine + "\r\n";

    for(var i = 0; i < procInfo.children.length; i++) {
        var child = procInfo.children[i];
        ret += _procTreePrintHelper(child, indent + " ", options, nowMs);
    }
    return ret;
}

/****************************************************************************/
/* returns a string that describes the time 'timeMs' that is at most 5 chars */
function _procTreeTime(timeMs) {

    if (timeMs < 0)
        timeMs = 0;
    var time = timeMs;
    if (time < 1000) 
        return time.toFixed(0) + "ms";
    time = time / 1000;
    if (time < 60) 
        return time.toFixed(1) + "s";
    time = time / 60;
    if (time < 60) 
        return time.toFixed(1) + "m";
    time = time / 60;
    if (time < 24) 
        return time.toFixed(1) + "h";
    time = time / 24;
    if (time < 100)
        return time.toFixed(1) + "d";
    if (time < 10000)
        return time.toFixed(0) + "d";
    return "*****";
}

/*****************************************************************************/
/* fetch the process tree in a short interval (200ms), and use those trees
   to compute process stats (like %CPU currently being used) 
*/
function _procTreeGetStats(wbemServices) {

    if (wbemServices == undefined)
        var wbemServices = procGetWMI();

    var procTree1 = procGetProcsAsTree(wbemServices);
    WScript.Sleep(200);
    var procTree2 = procGetProcsAsTree(wbemServices);

    _procTreeComputeStats(procTree1, procTree2);
    // xmlWrite(procTree1, "out.xml");
    return procTree1;
}

/*****************************************************************************/
/* given two process trees, compute some statistics about them, including
   CPU consumed during the delta, and inclusive CPU time (that is time
   all children are also consuming */

function _procTreeComputeStats(procTree, procTreePrev) {

    for(var i = 0; i < procTree.topProcsInfos.length; i++) {
        var procInfo = procTree.topProcsInfos[i];
        ret += _procTreeComputeStatsHelper(procInfo, procTree, procTreePrev);
    }
    procTree.topProcsInfos.sort(_procTreeSortFtn);
}

/*****************************************************************************/
function _procTreeComputeStatsHelper(procInfo, procTree, procTreePrev) {

    var procInfoPrev = procTreePrev.pid2ProcInfo[procInfo.pid]

    var deltaCpuTimeMs = procInfo.cpuTimeMs;
    if (procInfoPrev)
        deltaCpuTimeMs = procInfo.cpuTimeMs - procInfoPrev.cpuTimeMs;
    var deltaMs =procTree.date.getTime() - procTreePrev.date.getTime(); 
    procInfo.cpuPercent = 0;
    if (deltaMs != 0) 
        procInfo.cpuPercent = Math.round(deltaCpuTimeMs * 100 / deltaMs)

    procInfo.runTimeMs = procTree.date.getTime() - procInfo.createTime;

    procInfo.cpuTimeMsInc = procInfo.cpuTimeMs;
    procInfo.cpuPercentInc = procInfo.cpuPercent;
    for(var i = 0; i < procInfo.children.length; i++) { 
        var child = procInfo.children[i];
        _procTreeComputeStatsHelper(child, procTree, procTreePrev);
        procInfo.cpuTimeMsInc += child.cpuTimeMsInc;
        procInfo.cpuPercentInc += child.cpuPercentInc;
    }

    procInfo.children.sort(_procTreeSortFtn);
}

/*****************************************************************************/
/* sorts on percent CPU then by percent CPU time then by the LEAST
   amount of total CPU time.  This tends to put the most active processes
   last */

function _procTreeSortFtn(x, y) { 

        // The system processes are alwasy the smallest
        // This puts all the services first, which is nice
    if (y.pid <= 4)
        return 1;
    if (x.pid <= 4)
        return -1;
    if (x.cpuPercentInc > y.cpuPercentInc)
        return 1;
    if (x.cpuPercentInc < y.cpuPercentInc)
        return -1;
    if (x.cpuTimeMsInc < y.cpuTimeMsInc)
        return 1;
    if (x.cpuTimeMsInc > y.cpuTimeMsInc)
        return -1;
    return 0
}

/*****************************************************************************/
/* given two process trees (the structure returned from procGetProcsAsTree)
   and a process, determine if that process ID and all of its children were
   idle (that is unchanged between the two snapshots).  Return true if that
   is the case.   Unless 'excludeOrphans' is true, any orphan processes
   created after the create time of the process with 'pid' is assumed to be
   a child also  (depth is internal, and should not be set by callers */

function procProcTreeIdle(procsAsTree1, procsAsTree2, pid, excludeOrphans, depth) {

    dbgLog.push(" procProcTreeIdle(tree1, tree2, " + pid + ", " + excludeOrphans + ", ", depth, ") Time = ", _logCurTime(), " {\n");

    var procInfo1 = procsAsTree1.pid2ProcInfo[pid];
    var procInfo2 = procsAsTree2.pid2ProcInfo[pid];

    if (depth == undefined)
        depth = 0;

    var ret = false;
    if (procInfo1 == undefined) 
        dbgLog.push(" procProcTreeIdle: procInfo1 null\n");
    else if (procInfo2 == undefined) 
        dbgLog.push(" procProcTreeIdle: procInfo1 null\n");
    else {
        dbgLog.push(" procProcTreeIdle: Tree 1 Seq " + procsAsTree1.myseq + " Tree 2 " + procsAsTree2.myseq + "\n");
        dbgLog.push(" procProcTreeIdle: Proc PID " + procInfo1.pid + " CMD: " + procInfo1.cmdLine + "\n");

        var procInfoProc1 = procInfo1.proc;
        var procInfoProc2 = procInfo2.proc;
        if (procInfoProc1 == undefined) 
            dbgLog.push(" procProcTreeIdle: procInfo1.proc null\n");
        else if (procInfoProc2 == undefined) 
            dbgLog.push(" procProcTreeIdle: procInfo2.proc null\n");
        else {
            dbgLog.push(" procProcTreeIdle: Kernel times: proc1: " + procInfoProc1.KernelModeTime + " proc2: " + procInfoProc2.KernelModeTime + "\n");
            dbgLog.push(" procProcTreeIdle: User times: proc1: " + procInfoProc1.UserModeTime + " proc2: " + procInfoProc2.UserModeTime + "\n");
            dbgLog.push(" procProcTreeIdle: ReadTransfer: proc1: " + procInfoProc1.ReadTransferCount + " proc2: " + procInfoProc2.ReadTransferCount + "\n");
            dbgLog.push(" procProcTreeIdle: WriteTransfer: proc1: " + procInfoProc1.WriteTransferCount + " proc2: " + procInfoProc2.WriteTransferCount + "\n");
            dbgLog.push(" procProcTreeIdle: OtherTransfer: proc1: " + procInfoProc1.OtherTransferCount + " proc2: " + procInfoProc2.OtherTransferCount + "\n");
            dbgLog.push(" procProcTreeIdle: PageFaults: proc1: " + procInfoProc1.PageFaults + " proc2: " + procInfoProc2.PageFaults + "\n");

            if (procInfo1.cmdLine && procInfo1.cmdLine.match(/^\S*cscript(.exe)?\s/i) && procInfo1.children.length > 0) {
                dbgLog.push(" procProcTreeIdle: Found a cscript script with children.  Skipping its time (since it could be monitoring its children)\n");
                ret = true;
            }
            else if (procInfo1.cmdLine && procInfo1.cmdLine.match(/^\S*dw20(.exe)?\s/i)) {
                dbgLog.push(" procProcTreeIdle: Found Doctor Watson + skipping its time\n");
                ret = true;
            }
            else  {
                if (procInfoProc1.UserModeTime != procInfoProc2.UserModeTime) 
                    dbgLog.push(" procProcTreeIdle: user mode times differ, not idle\n");
                else if (procInfoProc1.KernelModeTime != procInfoProc2.KernelModeTime) 
                    dbgLog.push(" procProcTreeIdle: kernel mode times differ, not idle\n");
                else if (procInfoProc1.PageFaults != procInfoProc2.PageFaults) 
                    dbgLog.push(" procProcTreeIdle: PageFaults differ, not idle\n");
                else if (procInfoProc1.ReadTransferCount != procInfoProc2.ReadTransferCount) 
                    dbgLog.push(" procProcTreeIdle: ReadTransferCount differ, not idle\n");
                else if (procInfoProc1.WriteTransferCount != procInfoProc2.WriteTransferCount) 
                    dbgLog.push(" procProcTreeIdle: WriteTransferCount differ, not idle\n");
                else if (procInfoProc1.OtherTransferCount != procInfoProc2.OtherTransferCount) 
                    dbgLog.push(" procProcTreeIdle: OtherTransferCount differ, not idle\n");
                else if (procInfo1.createTime != procInfo2.createTime) 
                    dbgLog.push(" procProcTreeIdle: create times differ, not the same process!  Not idle\n");
                else {
                    dbgLog.push(" procProcTreeIdle: proc seems to be idle (CPU and IO times the same)\n");
                    ret = true;
                }
            }

            if (ret || 1) {        // FIX turned on all the time for better logging
                if (procInfo1.children.length != procInfo2.children.length) {
                    dbgLog.push(" procProcTreeIdle: childCounts differ, not idle\n");
                    ret = false;
                }
                else {
                    dbgLog.push(" procProcTreeIdle: checking " + procInfo1.children.length + " children\n");
                    for(var i = 0; i < procInfo1.children.length; i++) {
                        var childInfo1 = procInfo1.children[i];
                        var childPid1 = childInfo1.proc.ProcessID;
                        if (!procProcTreeIdle(procsAsTree1, procsAsTree2, childPid1, true, depth+1)) {
                            ret = false ;
                            // break; FIX turned on all the time for better logging
                        }
                    }
                }
            }
        }
    }

    if (ret && !excludeOrphans) {
        dbgLog.push(" procProcTreeIdle: checking orphan processes\n");

        for(var i = 0; i < procsAsTree2.topProcsInfos.length; i++) {
            var topProcInfo = procsAsTree2.topProcsInfos[i];
            dbgLog.push(" procProcTreeIdle: Orphan Candidate: " + topProcInfo.pid + " CMD: " + topProcInfo.cmdLine + "\n");
            if (topProcInfo.createTime >= procInfo2.createTime) {
                dbgLog.push(" procProcTreeIdle: found orphan " + topProcInfo.pid + "\n");
                if (!procProcTreeIdle(procsAsTree1, procsAsTree2, topProcInfo.pid, true, depth+1)) {
                    ret = false ;
                    break;
                }
            }
        }
    }
    
        // debug logging 
    dbgLog.push(" } procProcTreeIdle returning " + ret, "\n");
    if (depth == 0) {
        dbgLog.push("\n");
        dbgLogs.push(dbgLog.join(""));
        dbgLog = [];
/**        TODO - remove dbgLog completely when we have not needed it for a while
        if (ret) 
            logMsg(LogProc, LogInfo, " procProcTreeIdle: process is idle: logs of last 4 calls = {\n", dbgLogs.join(""), "}\n\n");
***/
        if (dbgLogs.length > 3)
            dbgLogs.shift();
    }

    return ret;
}

