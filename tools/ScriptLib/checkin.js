/*****************************************************************************/
/*                                  checkin.js                               */
/*****************************************************************************/

/* automation for the checkin process.  (What used to be done by SNAP).   */

/* AUTHOR: Vance Morrison 
   DATE: 4/18/04 */

/******************************************************************************/
var ClrCheckinDefined = 1;               // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");
if (!queueModuleDefined) throw new Error(1, "Need to include queue.js");
if (!runRemoteModuleDefined) throw new Error(1, "Need to include runremote.js");

if (ScriptDir == undefined)
    var ScriptDir  = WshFSO.GetParentFolderName(WshFSO.GetParentFolderName(WScript.ScriptFullName));

/*****************************************************************************/
/* Posts the code base delta associated with the Source Depot change number
   'changeNum' to the queue 'queueName'.
   The 'changeNum' arg can be 'default' (for the default change list, or use 
   'sd change' to create a new change list with exactly the needed files. 
   Use 'checkinQueues' to find the set of defined queues.  
*/
function checkinPost(queueName, changeNum) {

    var config = _checkinConfigGet(queueName);
    if (changeNum == undefined) 
        throw Error(1, "Required argument 'changeNum' not present (say 'default' if you want the default change Number)'");

    var queue = queueNew(config.queueDir);
    var entry = queueNewEntry(queue);
        
    var jobInfo = {};
    jobInfo.jobDir = entry.jobDir;
    jobInfo.user = Env("USERNAME");
    jobInfo.title = _userPrompt("Title (1 line description): ");
    
    var delta = {};
    delta.user = Env("USERNAME");
    delta.change = changeNum;
    delta.title = jobInfo.title;
    delta.codeReview = _userPrompt("Code Reviewer(s): ");
    delta.bbpack =jobInfo.jobDir + "\\delta.bbpack.cmd"; 
    delta.descriptionFile = jobInfo.jobDir + "\\delta.descr.txt";
    delta.changeNumFile = jobInfo.jobDir + "\\delta.changeNum.txt";     
    
    _checkinPostGetDescr(delta.descriptionFile, changeNum, config);
    runCmdToLog("bbpack -o " + delta.bbpack + " -c " + changeNum);
    
    // Sadly, bbpack sometimes returns success even when it fails)
    if (!FSOFileExists(delta.bbpack))
        throw Error(1, "Error creating BBPack file for changenum " + changeNum);
    
    // persist jobInfo to disk
    jobInfo.deltas = [delta];
    xmlWrite(jobInfo, entry.jobDir + "\\jobInfo.xml", "jobInfo");
    queueAddEntry(queue, entry);
    
    logMsg(LogScript, LogInfo, "Posted job ", entry.name, " to the queue\n");
    logMsg(LogScript, LogInfo, "See ", config.queueDir, "\\queueReport.html for queue status\n");
    queueReport(queue);
    return 0;
}

/*****************************************************************************/
/* Retries a submission 'entryName' for queue 'queueName'
   Use 'checkinQueues' to find the set of defined queues.   */

function checkinRetry(queueName, entryName) {

    var config = _checkinConfigGet(queueName);
    if (entryName == undefined) 
        throw Error(1, "Required argument 'entryName' (eg 04-05-21_07.19.0.clrgnt) not present'");
    
    var queue = queueNew(config.queueDir);
    var entry = queueNewEntry(queue);
        
    var newJobDir = entry.jobDir;
    var oldJobDir = queue.entries + "\\" + entryName;
    if (!FSOFileExists(oldJobDir + "\\jobInfo.xml")) 
        throw Error(1, "Bad entry name " + entryName);
    
    FSOCopyFile(oldJobDir + "\\*.xml", newJobDir);
    FSOCopyFile(oldJobDir + "\\*.descr.txt", newJobDir);
    FSOCopyFile(oldJobDir + "\\*.bbpack.cmd", newJobDir);
    queueAddEntry(queue, entry);
    
    logMsg(LogScript, LogInfo, "Posted job ", entry.name, " to the queue\n");
    logMsg(LogScript, LogInfo, "See ", config.queueDir, "\\queueReport.html for queue status\n");
    queueReport(queue);
}

/*****************************************************************************/
function _checkinPostGetDescr(descriptionFile, changeNum, config) {

/***
    var descr = "";
    if (config.submissionTemplate && FSOFileExists(config.submissionTemplate))
    descr = FSOReadFromFile(config.submissionTemplate);
***/
    
    descr = "<PLEASE FILL IN A DESCRIPTION>\r\n";
    if (changeNum.toString().match(/^\d+$/)) {
        WScript.StdOut.WriteLine("Getting description from SD change " + changeNum + " ...");
        var changeDesc = runCmd("sd describe -s " + changeNum).output;
        if (changeDesc.match(/Change.*\s*\n((.|\n)+?)\r*\nAffected/mi))
            descr = RegExp.$1;
        else 
            logMsg(LogScript, LogWarn, "Could not parse SD change description {\n", changeDesc, "\n} ignoring\n");
    }
    
    FSOWriteToFile(descr, descriptionFile);
    WScript.StdOut.WriteLine("Launching editor, please provide a description");
    WScript.StdOut.WriteLine("If this change fixes bugs, please paste in bug numbers here");
    runCmd("notepad " + descriptionFile);
}

/*****************************************************************************/
/* Displayes a web page showing the current queue status.
   Use 'checkinQueues' to find the set of defined queues.  
*/

function checkinShow(queueName) {

    var config = _checkinConfigGet(queueName);
    var queue = queueNew(config.queueDir);
    queueReport(queue);
    
    runCmd("start " + config.queueDir + "\\queueReport.html");
    return 0;
}

/*****************************************************************************/
/* Delete a particular entry from the queue 'queueName'  
   Use 'checkinQueues' to find the set of defined queues.  
*/
function checkinDelete(queueName, entryName) {

    var config = _checkinConfigGet(queueName);
    var queue = queueNew(config.queueDir);

    var entry = queueFindEntry(queue, entryName);
    if (entry == undefined) {
        WScript.Echo("Could not find entry '" + entryName + "' in the queue\n");
        return 1;
    }
    
    WScript.Echo("Removing entry '" + entry.name + "' from the queue\n");
    queueRemoveEntry(queue, entry);
    return 0;
}
/*****************************************************************************/
/****************       QUEUE ADMINISTRATION COMMANDS           **************/
/*****************************************************************************/

/*****************************************************************************/
/* shows the queues that are currently defined */

function checkinQueues() {
    var configDir = ScriptDir + "\\scriptlib\\checkinConfig";
    
    var files = FSOGetFilePattern(configDir, /\.js$/i);
    var out = "";
    for (var i = 0; i < files.length; i++) {
        var queueName = files[i].match(/([^\\]*)\.js$/i)[1];
        var config = _checkinConfigGet(queueName);
        out += "QUEUE: " + padRight(queueName, 10) + " TITLE: " + config.title + "\n";
    }
    logMsg(LogScript, LogInfo, "Checkin queues in ", configDir, " {\n", out, "}\n");
    return 0;
}

/*****************************************************************************/
/* Set queue prioirty.  Only jobs UNDER the given prioirty will be processed.
   Note that this means that setting to priority 0 will block all jobs.
   Use 'checkinQueues' to find the set of defined queues.  
*/
function checkinSetPriority(queueName, priority, reason) {

    var config = _checkinConfigGet(queueName);
    
    // TODO not finished
}

/*****************************************************************************/
/* Aborts the running job.  Indicates the reason 'reason'
   Use 'checkinQueues' to find the set of defined queues.  
*/
function checkinAbortRunning(queueName, reason) {

    var config = _checkinConfigGet(queueName);
    if (reason == undefined)
        reason = "No reason given";
    
    var queue = queueNew(config.queueDir);
    var runningEntry;
    try {
        runningEntry = urlShortCutTarget(queue.base + "\\running.url");
    } catch(e) {
        logMsg(LogScript, LogInfo, "checkinAbortRunning: No running job found\n");
        return 1;
    }
    
    FSOWriteToFile(
                   "abortTime: " + new Date() + "\r\n" +
                   "abortBy: " + Env("USERNAME") + "\r\n" +
                   "reason: " + reason.replace(/\n/, "\nReason: ") + "\r\n",
                   runningEntry + "\\abort.txt");
    
    // TODO this is full of races.  
    for(var i = 0; i < 120; i++) {
        WScript.Sleep(500);
        var curRunning;
        try {   
            if (!FSOFileExists(queue.base + "\\running.url"))
                return 0;
            var curRunning = urlShortCutTarget(queue.base + "\\running.url");
        } catch(e) { return 0; }
        if (curRunning != runningEntry)
            return 0;
        
        if (i % 10 == 0)
            logMsg(LogScript, LogInfo, "checkinAbortRunning: waiting for abort\n");
    }
    
    logMsg(LogScript, LogInfo, "checkinAbortRunning: checking proces not responding to abort request!\n");
    return 1;
}

function _checkinConfigGet(queueName) {

    if (queueName == undefined)
        throw Error(1, "Required argument 'queueName' not present. Use 'runjs checkinQueues' for list'");
    
    var configFile = ScriptDir + "\\scriptlib\\checkinConfig\\" + queueName + ".js";
    if (!FSOFileExists(configFile)) 
        throw Error(1, "Could not find configuration file " + configFile + "\n");
    
    logMsg(LogScript, LogInfo, "Reading the configuration file ", configFile, "\n");
    eval(FSOReadFromFile(configFile));  
    return configGet();
}

/*****************************************************************************/
/* The rest of this file are functions used to actually run jobs, not user
   commands  */
/*****************************************************************************/

/*****************************************************************************/
/* continuously monitor checkins from 'queueDir'.  Note we spawn a subprocess
   so that we are very robust to error here.  This outer loop should never
   fail because it is so simple.  Also respawning the command on every checkin
   insures that you always get the most up to date version if the jscript
   itself changes. */

function _checkinDoAll(queueName) {

    var config = _checkinConfigGet(queueName);
    for(;;) {
        if (runCmdToLog("runjs _checkinDo " + queueName, runSetNoThrow(runSetTimeout(3*3600))).exitCode != 0)
            WScript.Sleep(20*1000);             // retry, but wait a bit to avoid large output
    }
}

/*****************************************************************************/
/* tries to do a single entry */

function _checkinDo(queueName) {

    var config = _checkinConfigGet(queueName);
    
    logSetLimitedFile(config.queueDir + "\\checkinDoAll.log");
    logMsg(LogScript, LogInfo, "_checkinDo: Starting queue = " + config.queueDir + "\n");
    var queue = queueNew(config.queueDir);
    
    // Check to see if there is a monitor running for this queue.  If so, die.
    var queueDispatcherRunning = queue.base + "\\monitorRunning.lock";
    var monitorRunning;
    try {
        monitorRunning = FSOOpenTextFile(queueDispatcherRunning, 2, true); /* 2 = forWriting, overwrite = true */
    } catch(e) {
        logMsg(LogScript, LogError, "_checkinDo: Could not open " + queueDispatcherRunning + " is another dispatcher running?\n");
        return(3);
    }
    
    var queueEntry = undefined;
    var job = undefined;
    var updateQueueHtml = false;
    for(var time= 0;;time++) {
        if (!queueEntry) {
            queueEntry = queueRunNextEntry(queue);
            if (queueEntry) {
                config = _checkinConfigGet(queueName);          // refresh the config information, (in case someone updated it)
                // thus every job starts with configuration information as of
                // the time the job starts.  
                job = _checkinStartJob(queueEntry.jobDir, config);
                updateQueueHtml = true; 
            }
        }
        if (job) {
            if (jobPoll(job)) {
                logMsg(LogScript, LogInfo, "_checkinDo: job complete!\n")
                    break;
            }
        }
        else if (time % 60 == 0) {
            logMsg(LogScript, LogInfo, "_checkinDo: no entry to process for 60 sec. queue state = " + queue.state + "\n");
            // Since monitorQueue has a timout on this process, we can't wait for a job
            // forever because we might the job near the end of our timeout and then 
            // fail becasue of the timeout.  Thus we only wait 10 min, and then let
            // monitorQueue recycle us
            if (time >= 600)  {
                logMsg(LogScript, LogInfo, "processQueue: no jobs in 10 min, exiting\n");
                break;
            }
        }
        if (updateQueueHtml || time % 60 == 0) 
            queueReport(queue);
        updateQueueHtml = false;
        WScript.Sleep(1000);            /* 1000 ms */
    }
    
    if (queueEntry)
        queueFinishEntry(queue, queueEntry);
    queueReport(queue);
    
    monitorRunning.Close();
    logMsg(LogScript, LogInfo, "_checkinDo: Exiting\n");
    return 0;
}

/******************************************************************************/
/* run the job in 'jobDir' without actually removing it from the queue.
   mostly useful for hand replaying of jobs */

function _checkinRunJob(queueName, jobDir) {

    var config = _checkinConfigGet(queueName);
    var job = _checkinStartJob(jobDir, config);
    while(!jobPoll(job)) {
        WScript.Sleep(1000);
    }
    return job.task.exitCode;
}

/******************************************************************************/
/* called to start up a job.  Right now this is where pretty much all the
   configuration for what the job does and what machines it uses. */

function _checkinStartJob(jobDir, config) {

    logMsg(LogScript, LogInfo, "_checkinStartJob: Processing job in ", jobDir, "\n");
    
    // var jobInfoName = jobDir + "\\jobInfo.xml";
    // var jobInfo = xmlRead(jobInfoName).jobInfo;
    // logMsg(LogScript, LogInfo, "_checkinStartJob: jobInfo = ", dump(jobInfo), "\n");
    
    var taskEnv = {};
    taskEnv.outDir = jobDir;
    taskEnv.srcBase = config.srcBase;
    taskEnv.localSrcBase = config.localSrcBase;
    
    job = jobNew(config.mainTask, taskEnv, config.machMan);
    return job;
}

/******************************************************************************/
function _userPrompt(prompt) {

    WScript.StdOut.Write(prompt);
    var ret = WScript.StdIn.ReadLine();
    return ret;
}

/*********************************************************************************/
/* The rest of this file are scripts that are used by the worker machines.  They
   are typically called from particular queue scripts.  
   (TODO put in a separete file? */
/*********************************************************************************/

/*********************************************************************************/
function _checkinSetupEnv(srcBase) {

    Env("PATH") = Env("PATH") + ";" + srcBase + "\\tools\\x86";                 // for sd
    Env("PATH") = Env("PATH") + ";" + srcBase + "\\tools\\perl\\bin";           // for perl
    Env("PATH") = Env("PATH") + ";" + srcBase + "\\ndp\\clr\\snap2.4\\bin";                // for dfsCmd
    Env("SDPORT") = "DDRTSD:4000";
}

/*********************************************************************************/
function _checkinApply(outDir, localSrcBase) {

    logCall(LogScript, LogInfo, "_checkinApply", arguments);
    _checkinSetupEnv(localSrcBase);
    
    logMsg(LogScript, LogInfo, "_checkinApply: revert any previous checkin\n");
    
    // FIX NOW harden sd commands to be robust on server being down
    runCmdToLog("sd revert " + localSrcBase + "\\...");
    
    logMsg(LogScript, LogInfo, "_checkinApply: syncing {\n");
    sdSync(localSrcBase);
    logMsg(LogScript, LogInfo, "} syncing \n");
    
    var jobInfoName = outDir + "\\jobInfo.xml";
    var jobInfo = xmlRead(jobInfoName).jobInfo;
    logMsg(LogScript, LogInfo, "_checkinReport: jobInfo = ", dump(jobInfo), "\n");
    
    logMsg(LogScript, LogInfo, "_checkinApply: applying deltas {\n");
    var deltas = jobInfo.deltas;
    for (var i = 0; i < deltas.length; i++) {
        var delta = deltas[i];
        logMsg(LogScript, LogInfo, "_checkinApply delta description file ", delta.descriptionFile, "\n");
        
        var changeNum = sdChange(FSOReadFromFile(delta.descriptionFile));
        runCmdToLog(delta.bbpack + " -u -f -s -c " + changeNum);
        FSOWriteToFile(changeNum, delta.changeNumFile);
    }
    logMsg(LogScript, LogInfo, "} _checkinApply: applying delta\n");
    
    runCmdToLog("sd resolve -am");
    if (!runCmdToLog("sd resolve -n").output.match(/No file.*to resolve/i)) 
        throw Error(1, "ERROR: files could not be auto-merged!");
    
    if (runCmdToLog("sd opened " + localSrcBase + "\\...").output.match(/file.*not opened/))
        throw Error(2, "ERROR: no files opened after appling changes");
    
    return 0;
}

/*****************************************************************************/
function _checkinScanRuntime(archDir, srcBase, localSrcBase) {

    logCall(LogScript, LogInfo, "_checkinScanRuntime", arguments);
    var binDir = urlShortCutTarget(archDir + "\\binaries.url");
    scanRuntime(binDir, archDir + "\\scan.report.log", srcBase);
}

/******************************************************************************/
function _checkinBuild(bldKind, bldArch, bldType, archDir, srcBase, localSrcBase, binDirOverride, rzlbldTimeout, buildServer, binariesLocalOverride) {

    logCall(LogScript, LogInfo, "_checkinBuild", arguments);
    logMsg(LogScript, LogInfo, "Return\n");
    if(buildServer == undefined) {
        if(srcBase.match(/^\\\\([a-zA-Z0-9\-_]+)\\.*/))
             buildServer = RegExp.$1;
        else
             buildServer = null;
    }
 
    var binariesBase = "binaries";
    var binariesShare = "\\\\" + Env("COMPUTERNAME") + "\\" + binariesBase;
    if (!localSrcBase.match(/^(\w:\\)/))
        throw Error(1, "localSrcBase " + localSrcBase + " must begin with drive specification");
    var binariesLocal = RegExp.$1 + binariesBase;
    if (binariesLocalOverride != undefined)
        binariesLocal = binariesLocalOverride;
    
    if (!FSOFolderExists(binariesShare)) {
        var fullUser = Env("USERDOMAIN") + "\\" + Env("USERNAME");
        FSOCreatePath(binariesLocal);
        
        var netShareCmd = "net share " + binariesBase + "=" + binariesLocal + " /UNLIMITED";
        // on WinXP the /grant option is not present, however on W2K3 it is, we only need it on W2K3
        if (runCmd("net share /?", runSetNoThrow()).output.match(/GRANT:user/i))
            netShareCmd += " /grant:" + fullUser + ",FULL " + "/grant:everyone,READ";
        
        runCmdToLog(netShareCmd);
        runCmdToLog('cacls ' + binariesLocal + ' /G BUILTIN\\Administrators:F BUILTIN\\Users:R ' + fullUser + ':F',
                    runSetInput("y\r\n"));
    }
    
    // houseclean the local binaries cache
    _removeOldest(binariesLocal, 0.20, 2, 1000, buildServer);
    
    var jobID = archDir.match(/\\([^\\]*)\\[^\\]*$/)[1];
    var localBinDir = binariesLocal + "\\" + jobID + "\\" + bldArch + bldType;
    logMsg(LogScript, LogInfo, "_checkinBuild: localBinDir = ", localBinDir, "\n");
    FSOCreatePath(localBinDir);
    
    FSOCreatePath(archDir);
    
    logMsg(LogScript, LogInfo, "_checkinBuild: copying ", srcBase, " -> ", localSrcBase, "\n");
    robocopyClrSrc(srcBase, localSrcBase, archDir);
    _robocopySubTree(srcBase, localSrcBase, "..\\dev", archDir);
    
    runCmdToLog("attrib +r /s " + localSrcBase + "\\public\\*.*");
    
    if (bldKind == "razzleBuild") { 
        if (rzlbldTimeout == undefined)
            rzlbldTimeout = 5000;
        
        razzleBuild(bldType, bldArch, "ndp", "-cC", localSrcBase, localBinDir, archDir, rzlbldTimeout, "No_sdrefresh Offline");
    }
    else if (bldKind == "prefastBuild")
        prefastBuild(bldType, "ndp\\clr\\src", localSrcBase, archDir, "No_sdrefresh Offline");
    else if (bldKind = "retailBuild") {
        if (bldType != "ret") 
            throwWithStackTrace(Error(1, "retailBuild run on type " + bldType + " != ret"));
        if (bldArch.toLowerCase() != Env("PROCESSOR_ARCHITECTURE").toLowerCase())
            throwWithStackTrace(Error(1, "retailBuild run on arch " + bldArch + " != " + Env("PROCESSOR_ARCHITECTURE")));
        
        // FIX is a hack.  It seems like we should update the version number in ApplyChanges.
        // we are wiring knowledge of the source tree into the build infrastructure.  This seems bad. 
        verFile = archDir + "\\..\\DesktopVersion.h";
        if (FSOFileExists(verFile)) {
            logMsg(LogScript, LogInfo, "Copying ", verFile, " to ", localSrcBase + "\\ndp\\inc\\version\\version.h\n");
            runCmdToLog("xcopy /y /r " + verFile + " " + localSrcBase + "\\ndp\\inc\\version\\version.h");
            // FSOCopyFile(verFile, localSrcBase + "\\ndp\\inc\\version\\version.h", true);
        }
        
        var archDirUnopt = archDir;
        // var archDirUnopt = archDir + ".unopt"; // FIX when we rely on this for both change this
        
        var localBinDirUnopt = localBinDir + ".unopt";
        
        if (rzlbldTimeout == undefined)
            rzlbldTimeout = 45*60;
        
        logMsg(LogScript, LogInfo, "building unoptimized retail bits with razzleBuild {\n");
        razzleBuild(bldType, bldArch, "ndp", "-cC", localSrcBase, localBinDirUnopt, archDirUnopt, rzlbldTimeout, "No_sdrefresh Offline");
        logMsg(LogScript, LogInfo, "} razzleBuild\n");
        
        logMsg(LogScript, LogInfo, "Performaing working set optimizations on unoptimized retail bits with retailBuild {\n"); 
        retailBuild(localBinDirUnopt, localBinDir, bldArch, undefined, archDir, localSrcBase);
        logMsg(LogScript, LogInfo, "} retailBuild\n");
    }
    else 
        throw Error(1, "Unrecognised bldKind = " + bldKind);
    
    logMsg(LogScript, LogInfo, "_checkinBuild: ************* DONE WITH BUILD! ************\n");
    
    _checkinSetupEnv(localSrcBase);
    
    var uncLocalBinDir = binariesShare + "\\" + jobID + "\\" + bldArch + bldType;
    var binShortCut = archDir + "\\binaries.url";
    logMsg(LogScript, LogInfo, "_checkinBuild: publishing binaries dir ", uncLocalBinDir, " in  shortcut ", binShortCut, "\n");
    urlShortCutCreate(binShortCut, uncLocalBinDir); 
    
    // Publish to the world 
    var binDir = archDir + "\\binaries";
    // TODO this is a backward compatibility hack.  Can be removed when we move off of clrsnapbackup2
    if (binDir.match(/clrsnapbackup2/i))
        binDir = archDir + "\\bin";
    // This is a more general fix to allow a tool that expects a different binary directory to use this function        
    if (binDirOverride != undefined)
        binDir = binDirOverride;
    
    logMsg(LogScript, LogInfo, "_checkinBuild: Setting up ", binDir, " DFS link\n");
    if (FSOFolderExists(binDir))
        runCmdToLog("dfscmd /unmap " + binDir);
    runCmdToLog("dfscmd /map " + binDir + " " + uncLocalBinDir);
    
    return 0;
}

/******************************************************************************/
function _retailBuildWrapper(jobDir, bldArch, bldType, archDir, srcBase, localSrcBase, binDirOverride, rzlbldTimeout, buildServer) {

    logCall(LogScript, LogInfo, "_retailBuildWrapper", arguments);
    logMsg(LogScript, LogInfo, "Return\n");
    if(buildServer == undefined) {
        if(srcBase.match(/^\\\\([a-zA-Z0-9\-_]+)\\.*/))
             buildServer = RegExp.$1;
        else
             buildServer = null;
    }
 
    var binariesBase = "binaries";
    var binariesShare = "\\\\" + Env("COMPUTERNAME") + "\\" + binariesBase;
    if (!localSrcBase.match(/^(\w:\\)/))
        throw Error(1, "localSrcBase " + localSrcBase + " must begin with drive specification");
    var binariesLocal = RegExp.$1 + binariesBase;
    
    if (!FSOFolderExists(binariesShare)) {
        var fullUser = Env("USERDOMAIN") + "\\" + Env("USERNAME");
        FSOCreatePath(binariesLocal);
        
        var netShareCmd = "net share " + binariesBase + "=" + binariesLocal + " /UNLIMITED";
        // on WinXP the /grant option is not present, however on W2K3 it is, we only need it on W2K3
        if (runCmd("net share /?", runSetNoThrow()).output.match(/GRANT:user/i))
            netShareCmd += " /grant:" + fullUser + ",FULL " + "/grant:everyone,READ";
        
        runCmdToLog(netShareCmd);
        runCmdToLog('cacls ' + binariesLocal + ' /G BUILTIN\\Administrators:F BUILTIN\\Users:R ' + fullUser + ':F',
                    runSetInput("y\r\n"));
    }
    
    // houseclean the local binaries cache
    _removeOldest(binariesLocal, 0.20, 2, 1000, buildServer);
    
    var jobID = archDir.match(/\\([^\\]*)\\[^\\]*$/)[1];
    var localBinDir = binariesLocal + "\\" + jobID + "\\" + bldArch + bldType;
    logMsg(LogScript, LogInfo, "_retailBuildWrapper: localBinDir = ", localBinDir, "\n");
    FSOCreatePath(localBinDir);
    
    FSOCreatePath(archDir);
    
    logMsg(LogScript, LogInfo, "_retailBuildWrapper: copying ", srcBase, " -> ", localSrcBase, "\n");
    robocopyClrSrc(srcBase, localSrcBase, archDir);
    _robocopySubTree(srcBase, localSrcBase, "..\\dev", archDir);
    
    runCmdToLog("attrib +r /s " + localSrcBase + "\\public\\*.*");
    
    if (bldType != "ret") 
        throwWithStackTrace(Error(1, "retailBuild run on type " + bldType + " != ret"));
    if (bldArch.toLowerCase() != Env("PROCESSOR_ARCHITECTURE").toLowerCase())
        throwWithStackTrace(Error(1, "retailBuild run on arch " + bldArch + " != " + Env("PROCESSOR_ARCHITECTURE")));
        
    // FIX is a hack.  It seems like we should update the version number in ApplyChanges.
    // we are wiring knowledge of the source tree into the build infrastructure.  This seems bad. 
    verFile = archDir + "\\..\\DesktopVersion.h";
    if (FSOFileExists(verFile)) {
        logMsg(LogScript, LogInfo, "Copying ", verFile, " to ", localSrcBase + "\\ndp\\inc\\version\\version.h\n");
        runCmdToLog("xcopy /y /r " + verFile + " " + localSrcBase + "\\ndp\\inc\\version\\version.h");
        // FSOCopyFile(verFile, localSrcBase + "\\ndp\\inc\\version\\version.h", true);
    }
        
    var archDirUnopt = archDir;
    // var archDirUnopt = archDir + ".unopt"; // FIX when we rely on this for both change this
        
    var localBinDirUnopt = localBinDir + ".unopt";
        
    if (rzlbldTimeout == undefined)
        rzlbldTimeout = 45*60;
        
    logMsg(LogScript, LogInfo, "building unoptimized retail bits with razzleBuild {\n");
    razzleBuild(bldType, bldArch, "ndp", "-cC", localSrcBase, localBinDirUnopt, archDirUnopt, rzlbldTimeout, "No_sdrefresh Offline");
    logMsg(LogScript, LogInfo, "} razzleBuild\n");

    // We check for the existence of a special file in either job folder or in
    // the parent of the job folder. If it exists then we generate new profile
    // data. Otherwise we skip generating profile data, and instead we just copy
    // the "amd64ret.unopt" folder to the "amd64ret" folder  -andparo
      
    if (FSOFileExists(jobDir + "\\..\\GenOptData.txt") || FSOFileExists(jobDir + "\\GenOptData.txt"))
    {
        logMsg(LogScript, LogInfo, "Performaing working set optimizations on unoptimized retail bits with retailBuild {\n"); 
        retailBuild(localBinDirUnopt, localBinDir, bldArch, undefined, archDir, localSrcBase);
        logMsg(LogScript, LogInfo, "} retailBuild\n");
    }
    else
    {
        logMsg(LogScript, LogInfo, "Skipping working set optimizations on unoptimized retail bits with retailBuild, doing robocopy instead\n"); 
        robocopy(localBinDirUnopt, localBinDir, "/PURGE", archDir + "\\retailBuildCopy.log");
        runCmdToLog("del " + localBinDir + "\\BuildInfo.bat");
    }
    
    logMsg(LogScript, LogInfo, "_retailBuildWrapper: ************* DONE WITH BUILD! ************\n");
    
    _checkinSetupEnv(localSrcBase);
    
    var uncLocalBinDir = binariesShare + "\\" + jobID + "\\" + bldArch + bldType;
    var binShortCut = archDir + "\\binaries.url";
    logMsg(LogScript, LogInfo, "_retailBuildWrapper: publishing binaries dir ", uncLocalBinDir, " in  shortcut ", binShortCut, "\n");
    urlShortCutCreate(binShortCut, uncLocalBinDir); 
    
    // Publish to the world 
    var binDir = archDir + "\\binaries";
    // TODO this is a backward compatibility hack.  Can be removed when we move off of clrsnapbackup2
    if (binDir.match(/clrsnapbackup2/i))
        binDir = archDir + "\\bin";
    // This is a more general fix to allow a tool that expects a different binary directory to use this function        
    if (binDirOverride != undefined)
        binDir = binDirOverride;

    // If we are running this task outside of SNAP, then do not set up DFS mappings
    if (trim(Env("SnapLocal_RunTaskOnLocalMachine")) != "1")
    {
        logMsg(LogScript, LogInfo, "_retailBuildWrapper: Setting up ", binDir, " DFS link\n");
        if (FSOFolderExists(binDir))
            runCmdToLog("dfscmd /unmap " + binDir);
        runCmdToLog("dfscmd /map " + binDir + " " + uncLocalBinDir);
    }
  
    return 0;
}

/******************************************************************************/
function _checkinDevBVT(bldArch, bldType, smartyArgs, testSetName, archDir, srcBase, localSrcBase) {

    FSOCreatePath(archDir);
    _robocopySubTree(srcBase, localSrcBase, "ddsuites\\src\\clr\\" + bldArch, archDir);
    _robocopySubTree(srcBase, localSrcBase, "ddsuites\\tools", archDir);
    runCmdToLog("xcopy /rcy " + srcBase + "\\ddsuites\\*.* " + localSrcBase + "\\ddsuites");
        
    var binDir = urlShortCutTarget(archDir + "\\binaries.url");
    var cacheDir = Env("HOMEDRIVE") + "\\ClrSetupCache";
    
    clrSetupWithCache(binDir, "", archDir, cacheDir);
    devBVT(smartyArgs, archDir + "\\" + testSetName, localSrcBase + "\\ddsuites", bldType);
}

/*********************************************************************************/
function _checkinSubmit(outDir, localSrcBase) {

    logCall(LogScript, LogInfo, "_checkinSubmit", arguments);
    _checkinSetupEnv(localSrcBase);
    
    var changes = FSOGetFilePattern(outDir, /.*\.changeNum.txt$/i);
    for (var i = 0; i < changes.length; i++) {
        var changeFileName = changes[i];
        var change = FSOReadFromFile(changeFileName);
        
        // Get the user from the bbpack file
        var bbPackFileName = changeFileName.match(/(.*)\.changeNum.txt$/i)[1] + ".bbpack.cmd";
        var asUser = runCmd(bbPackFileName + " -l").output.match(/User name:\s*(.*\S)/)[1];
        
        logMsg(LogScript, LogInfo, "_checkinSubmit: submitting change ", change, " on behalf of user ", asUser, "\n");
        changeNum = sdSubmit(change, undefined, asUser);        
        logMsg(LogScript, LogInfo, "_checkinSubmit: submitted as change ", changeNum, "\n");
        
        FSOWriteToFile(changeNum, changeFileName);              // the change number may have been updated
    }
    return 0;
}

/*********************************************************************************/
function _checkinReport(outDir, localSrcBase, buildDirBase) {

    logCall(LogScript, LogInfo, "_checkinReport", arguments);
    
    var submitStatusFile = outDir + "\\checkin.Submit.0.status.log";
    var submitStatus = parseFile(submitStatusFile);
    logMsg(LogScript, LogInfo, "_checkinReport: Submit status = {\n", dump(submitStatus), "\n}\n");
    
    var jobInfoName = outDir + "\\jobInfo.xml";
    var jobInfo = xmlRead(jobInfoName).jobInfo;
    logMsg(LogScript, LogInfo, "_checkinReport: jobInfo = ", dump(jobInfo), "\n");
    
    var status = "FAILED";
    if (submitStatus.result && submitStatus.result.match(/SUCCESS/i))
        status = "SUCCEEDED";
    
    if (status == "SUCCEEDED") {
        logMsg(LogScript, LogInfo, "The job was successful\n");
        
        // Publish the build to a directory named by the change number of the submission
        var changes = FSOGetFilePattern(outDir, /.*\.changeNum.txt$/i);
        var change = FSOReadFromFile(changes[changes.length-1]).match(/(.*\S)/)[1];
        var buildDir = buildDirBase + "\\" + change;
        logMsg(LogScript, LogInfo, "_checkinReport: publishing buildDir = ", buildDir, "\n");
        FSOCreatePath(buildDirBase);
        runCmdToLog("dfscmd /map " + buildDir + " " + outDir);
        
        // send checkin mail
    }
    else 
        logMsg(LogScript, LogWarn, "The job failed\n");
    
    var jobName = jobInfo.jobDir.match(/([^\\]*)$/)[1];
    var taskReport = outDir + "\\taskReport.html";
    var mailSubject = "Submission " + status + " for checkin " + jobName;
    var mailBody =  "<HTML><BODY>\r\n" + 
        "Job Title: " + jobInfo.title + "<P>\r\n" + 
        "Please see <A HREF='" + taskReport + "'> " + taskReport + "</A> for details\r\n" +
        "</BODY></HTML>\r\n";
    
    mailSendHtml(jobInfo.user + "@microsoft.com", mailSubject, mailBody);
    return 0;
}

/*****************************************************************************/
/* Get the subfolders at a particular depth */
function _getSubFolders(folderName, depth) {    
    var subFolders = [];
    if (FSOFolderExists(folderName)) {
        var folder = WshFSO.GetFolder(folderName);
        if (depth == 0) {
            for (var subFoldersEnum = new Enumerator(folder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
                subFolders.push(subFoldersEnum.item());
            }
        } else {
            for (var subFoldersEnum = new Enumerator(folder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
                var individualSubFolders = _getSubFolders(folderName + "\\" + subFoldersEnum.item().Name, depth-1);
                for(var i=0; i<individualSubFolders.length; i++) {
                    subFolders.push(individualSubFolders[i]);
                }
            }
        }
    }
    return subFolders;
}

/*****************************************************************************/
/* Remove the oldest subfolders from 'folder' until the amount of free space 
   on the drive is greater than 'desiredFreeRatio.  However it keep at least 
   'minNum' subFolders and no more than 'maxNum'  Returns the amount of free 
   space actually obtained. */

function _removeOldest(folderName, desiredFreeRatio, minNum, maxNum, buildServer, depth) {

    logMsg(LogScript, LogInfo, "removeOldest: ", folderName, " { \n");
    if (depth == undefined)
        depth = 0;
    var folder = WshFSO.GetFolder(folderName);
    var drive = folder.Drive;
    var desiredFreeSpace = desiredFreeRatio * drive.TotalSize;
    
    logMsg(LogScript, LogInfo, "For folder " + folderName + " we have " + 
           Math.round(drive.FreeSpace / (1024*1024)) + "M targeting at least " +
           Math.round(desiredFreeSpace / (1024*1024)) + "M\n");
    
    if (drive.FreeSpace >= desiredFreeSpace) {
        logMsg(LogScript, LogInfo, "Nothing to do\n");
    }
    else {
        if (minNum < 0)
            minNum = 0;

        // put the subfolders in an array an sort it by last modification time
        var subFolders = _getSubFolders(folderName, depth);
        subFolders.sort(function(x, y) { return (y.DateLastModified - x.DateLastModified); });
        
        while (subFolders.length > minNum) {
            logMsg(LogScript, LogInfo, "Desired free space = " +  Math.round(desiredFreeSpace / (1024*1024)) + " Meg\n");
            logMsg(LogScript, LogInfo, "Actual free space = " +  Math.round(drive.FreeSpace / (1024*1024)) + " Meg\n");
            if (subFolders.length < maxNum && drive.FreeSpace >= desiredFreeSpace)
                break;
            var subFolder = subFolders.pop();
            logMsg(LogScript, LogInfo, "Deleting Folder ", subFolder.Path, "\n");
            try
            {
                _deleteFolderDFSAware(subFolder.Path, true, buildServer);
            }
            catch (e)
            {
                logMsg(LogScript, LogInfo, "WARNING: _deleteFolderDFSAware threw exception: ", e.description, "\n");
            }
        }
    }
    
    var ret = drive.FreeSpace / (1024 * 1024);
    logMsg(LogScript, LogInfo, "} removeOldest() = ", ret.toFixed(2), "\n");
    return ret;
}

/********************************************************************************/
function _deleteFolderDFSAware(path, force, dfsHost) {

    logMsg(LogScript, LogInfo, "_deleteFolderDFSAware(" + path + ", " + dfsHost + ") {\n"); // }
    
    var dfsNodes = _getDfsNodesUnderPath(path, dfsHost);
    for(var i = 0; dfsNodes != null && i < dfsNodes.length; i++) {
        var dfsNode = dfsNodes[i];
        logMsg(LogScript, LogInfo, "_deleteFolderDFSAware: unmapping " + dfsNode + "\n");
        runCmdToLog("dfscmd /unmap " + dfsNode);
    }
    
    logMsg(LogScript, LogInfo, "deleting folder " + path + "\n")
        FSODeleteFolder(path, force);
    /* { */ logMsg(LogScript, LogInfo, "} _deleteFolderDFSAware()\n");
}

/********************************************************************************/
function _getDfsNodesUnderPath(path, dfsHost) {
    if(dfsHost == undefined || dfsHost == null || dfsHost == "") {
        return null;
    }
    logMsg(LogScript, LogInfo, "_getDfsNodesUnderPath(" + path + ", " + dfsHost + ")\n"); 
    var ret = [];
    var WbemServices = GetObject("winmgmts://" + dfsHost);
    
    // backslashes need to be escaped
    path = path.replace(/\\/g, "\\\\");
    var str = "select * from Win32_DFSNode where Name like '" + path + "%'";
    logMsg(LogScript, LogInfo1000, "str = " + str + "\n");
    var e = new Enumerator(WbemServices.ExecQuery(str));
    for (; !e.atEnd();e.moveNext()) {
        var dfs = e.item();
        logMsg(LogScript, LogInfo1000, "Got " + dfs.Name + "\n");
        ret.push(dfs.Name);
    }
    return ret;
}

/******************************************************************************/
/* copy 'relPath' from 'srcDirBase' to 'destDirBase' logging to 'logDir' */

function _robocopySubTree(srcDirBase, destDirBase, relPath, logDir) {

    var srcDir = srcDirBase + "\\" + relPath;
    var destDir = destDirBase + "\\" + relPath;
    var logFile = logDir + "\\copy." + relPath.replace(/\\/g, "-") + ".robocopy.log";
    
    robocopy(srcDir, destDir, "/PURGE", logFile);
}

