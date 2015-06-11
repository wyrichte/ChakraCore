/******************************************************************************/
/*                               retailLab.js                                 */
/******************************************************************************/

/* The task infrastructure defined in task.js is about providing the ability
   to name a large chunk of work to do, describe the dependencies among these
   chunks, and produce reasonable reports from these tasks.   

   The rolling automation defined here builds on this by defining wrapper logic
   that will run a particular task on a series of inputs to form a series of
   outputs.  For example, if you have a series of builds that represent checkins,
   the rolling process can run a battery of tests on each of these builds to see 
   where failures are introduced.   

   The logic was build in such a way that it is allowable that many machines can
   be processing the same list of builds.  Thus the first machine takes the 
   most recient build, then next machine tries to take the first build, but 
   notices that it is already been taken, so moves on to the next most recient,
   etc.  In this way the process naturally scales.  The more machines you have
   the faster you get a complete set of test runs.  If the average rate of 
   generating new builds is less than the average rate that the whole set of
   workers is processing the builds, then you will keep up and have a complete 
   set of information.  Typically you keep adding machines until this is true.

   This script provides functionality for the CLR Retail Lab. For generic rolling
   functionality, see rolling.js.
*/

// AUTHOR: Andrew Paroski 
// OWNER: sanket
// DATE: 3/22/2009
/******************************************************************************/

var retailLabModuleDefined = 1;                     // Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include ClrAutomation.js");
if (!clrTaskModuleDefined) throw new Error(1, "Need to include clrTask.js");

var LogRetailLab = logNewFacility("retailLab");
var supportedArchs = ["x86", "amd64"];
    
logSetFacilityLevel(LogRetailLab, LogInfo100000);

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

/******************************************************************************/
/****************** Retail Lab Snap/Private Jobs Functionality ****************/
/******************************************************************************/

/********************************************************************************************************/
/*
   This script is no longer functional.

   To submit a job to the Retail Lab, please use the "Private Jobs" 
   tab of the "Retail Lab" page linked at the top of the Snap website.  
   For any questions, you can contact 'clrrlnot'.
*/
function queuePrivateGenOptBuildTask() {
    throw new Error("\n\n" + 
   "This script is no longer functional.\n" +
   "\n" +
   "To submit a job to the Retail Lab, please use the 'Private Jobs'\n" +
   "tab of the 'Retail Lab' page linked at the top of the Snap website.\n" +
   "For any questions, you can contact 'clrrlnot'.\n"
   );
}

/********************************************************************************************************/
/*
   This script is no longer functional.

   To submit a performance run, please use the "Private Jobs" tab of the 
   "Retail Lab" page linked at the top of the Snap website.  This will 
   involve running your change through the retail optimization lab before 
   submitting it for a perf run, which will ensure that you get a valid 
   comparison.  For any questions, you can contact 'clrrlnot'.
*/
function queuePerfGenOptBuildTask() {
    throw new Error("\n\n" + 
   "This script is no longer functional.\n" +
   "\n" +
   "To submit a performance run, please use the 'Private Jobs' tab of the\n" +
   "'Retail Lab' page linked at the top of the Snap website.  This will\n" +
   "involve running your change through the retail optimization lab before\n" +
   "submitting it for a perf run, which will ensure that you get a valid\n" +
   "comparison.  For any questions, you can contact 'clrrlnot'.\n"
   );
}

/*********************************************************************************************************************************************/
/* This is the entry level function that is invoked by the startup script on a retail lab worker machine 
   For snap pu\clr jobs, the retail lab builds are maintained in \\clrsnap0\public\tfs_puclr\retailLab\<arch>\<changeset>
   For private pu\clr jobs, the retail lab builds are maintained in \\clrsnap0\public\tfs_puclr\retailLab\privateJobs\<arch>\<changeset_alias#>
                            where changeset_alias# is the changeset followed by '_' followed by a run number
*/
function RetailLabWorkerScript(outDirBase, jobDirBase, srcBase, bldArch, bldType, contactAliasWithDomain) {
    var statusCode = 0;
    logCall(LogRetailLab, LogInfo10, "RetailLabWorkerScript", arguments, "{");

    if (contactAliasWithDomain == undefined)
        contactAliasWithDomain = "REDMOND\\sanket";
        
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if(srcBase == undefined)
            throw Error(1, "srcBase not defined\n")
    }
    
    // If just the username is specified, log a warning
    if (contactAliasWithDomain.split("\\").length == 1) {
        logMsg(LogRetailLab, LogInfo10, "RetailLabWorkerScript: No user domain specified, assuming user belongs to the REDMOND domain\n");
        contactAliasWithDomain = Env("USERDOMAIN") + "\\" + contactAliasWithDomain;
    }
    var workerLogDir = jobDirBase + "\\..\\retailLab\\workerLogs";    

    // Initialize some parameters used for sending mails
    var mailUserDomain = contactAliasWithDomain.split("\\")[0];
    var mailUsername = contactAliasWithDomain.split("\\")[1];
    var mailTo = mailUsername + "@microsoft.com";
    var mailSubject = "CLR Retail Lab Job Report";
    var mailBody = undefined;
    var mailCC = "sanket@microsoft.com";
    var mailOwner = mailUsername + "@microsoft.com";
    var mailFrom = "clrgnt@microsoft.com";
    var mailJobText = "\n\nA summary of the job is below:\n==============================\n\n";
    var mailJobErrors = "";
    var mailJobWarnings = "";

    // Do house cleaning for the logs stored on clrsnap0
    var numKeep = 100;
    logMsg(LogRetailLab, LogInfo, "HouseCleaning ", outDirBase, ", removing so that we have at most ", numKeep, " builds\n");
    _deleteOldestDirs(outDirBase, "", numKeep); 
    mailJobText = mailJobText + "SnapHouseCleaningTask returned: " + statusCode + "\n";

    // retail lab worker house cleaning and setup
    _retailWorkerHouseCleanAndSetup(srcBase);
    mailJobText = mailJobText + "WorkerHouseCleaningTask returned: " + statusCode + "\n";
    
    // This allows you to pause the automation in the event of a scripting error
    var freezeFile = outDirBase + "\\Freeze.txt";
    while (FSOFileExists(freezeFile)) {
        logMsg(LogRolling, LogInfo10, "RetailLabWorkerScript: File ", freezeFile, " exists, Sleeping\n");
        WScript.Sleep(60000);
    }

    var localSrcBase = srcBase;
    var binariesBase = "binaries";
    var binariesShare = "\\\\" + Env("COMPUTERNAME") + "\\" + binariesBase;
    if (!localSrcBase.match(/^(\w:\\)/)) {
        throw Error(1, "localSrcBase " + localSrcBase + " must begin with drive specification");
    }
    var binariesLocal = RegExp.$1 + binariesBase;    

    logMsg(LogRetailLab, LogInfo, "RetailLabWorkerScript: Determining which task to run\n");

    var taskInfo = _getNextRetailLabTask(jobDirBase, outDirBase, bldArch, bldType);
    while (taskInfo == undefined)
    {
        logMsg(LogRetailLab, LogInfo, "RetailLabWorkerScript: No work to do, sleeping 60 seconds\n");
        WScript.Sleep(60000);
        taskInfo = _getNextRetailLabTask(jobDirBase, outDirBase, bldArch, bldType);
    }

    var ret = 1;

    if (taskInfo.taskType == "GenOptBuild" && taskInfo.privateJob == false)
    {
        try
        {
            var localBinDir = binariesLocal + "\\" + taskInfo.changeNum;
            var uncLocalBinDir = binariesShare + "\\" + taskInfo.changeNum;

            // Sync the enlistment to changeNum and unpack shelveset
            statusCode = doApplyChangesTask(taskInfo.outDir, taskInfo.changeNum, taskInfo.shelveset, srcBase);
            mailJobText = mailJobText + "ApplyChangesTask returned: " + statusCode + "\n";
            
            var relUnoptBuildDir = bldArch + bldType;
            var relOptBuildDir = bldArch + bldType + ".opt";
           
            // Call the GenOptBuild rolling task
            ret = _callGenOptBuildTask(bldArch, bldType, srcBase, taskInfo.outDir, localBinDir, relUnoptBuildDir,
                                      relOptBuildDir, taskInfo.changeNum, taskInfo.shelveset, taskInfo.privateJob);

            mailJobText = mailJobText + "GenOptBuildTask returned: " + ret + "\n";

            // Create the DFS link irrespective of whether the genopt build task succeeded or failed
            // This is needed because the perf lab will look for a build at the same location even if the task fails
            var optBuildLink = taskInfo.jobDir + "\\" + bldArch + "pret\\bin";
            {
                var dotLocalUnoptBuildDir = localBinDir + "\\" + relUnoptBuildDir;
                var dotLocalOptBuildDir = localBinDir + "\\" + relOptBuildDir;
                
                // if applyChanges task fails or genoptbuild task fails
                // we use the build generated by snap
                if (ret != 0 || statusCode != 0)
                {
                    logMsg(LogRetailLab, LogInfo, "ApplyChanges or Genoptbuild failed\n");
                    if (FSOFolderExists(dotLocalOptBuildDir))
                    {
                        logMsg(LogRetailLab, LogInfo, "Found ", dotLocalOptBuildDir," directory on the worker machine. Deleting it and all its subdirectories \n");
                        FSODeleteFolder(dotLocalOptBuildDir, true);
                    }
                }

                if (!FSOFolderExists(dotLocalOptBuildDir))
                {
                    logMsg(LogRetailLab, LogInfo, "Not found ", dotLocalOptBuildDir," directory on the worker machine. Making a copy using the snap build\n");
                    var snapBuildDir = jobDirBase + "\\" + taskInfo.changeNum + "\\" + bldArch + bldType + "\\bin";
                    if (!FSOFolderExists(snapBuildDir)) {
                        snapBuildDir = jobDirBase + "\\" + taskInfo.changeNum + "\\" + bldArch + bldType + ".unopt\\bin";
                    }
                    robocopy(snapBuildDir, dotLocalOptBuildDir, "/xo /nfl /ndl");
                    logMsg(LogRetailLab, LogInfo, "Created a copy of the optimized build from the unopt build\n");
                }
            
                var runOpts = clrRunTemplate;
                runOpts = runSetNoThrow(runOpts);
                if (FSOFolderExists(optBuildLink))
                    runCmdToLog(srcBase + "\\ndp\\clr\\snap2.4\\tasks\\dfscmd /unmap " + optBuildLink, runOpts);
                var run = runCmdToLog(srcBase + "\\ndp\\clr\\snap2.4\\tasks\\dfscmd /map " + optBuildLink + " " + uncLocalBinDir, runOpts);
                mailJobText = mailJobText + "DfsLinksSetupTask returned: " + run.exitCode + "\n";
                if (run.exitCode == 0)
                {
                    mailJobText += "\nYour retail job can be found through the pret folders in the snap directory of the job at " + optBuildLink + "\n";
                    logMsg(LogRetailLab, LogInfo, "Create DFS link successfully!\n");
                }
                else
                {
                    // TODO: We should include a warning this failure in the report that we email
                    logMsg(LogRetailLab, LogInfo, "Failed to create DFS link!\n");
                }                
            }

            // Log the result and send an email
            if (ret == 0)
            {
                // Log that the genOptBuild task succeeded
                logMsg(LogRetailLab, LogInfo, "genOptBuild task succeeded!\n");

                // Send a "SUCCESS" email with some job details
                mailBody = "Your retail lab job succeeded. You can access your job report at " + taskInfo.jobDir + "\n\n" + mailJobText;
                mailSubject = mailSubject + " - SUCCESS";
            }
            else
            {
                // Log that the genOptBuild task failed
                logMsg(LogRetailLab, LogInfo, "genOptBuild task failed with exit code ", ret, "\n");

                // Send a failure email with some job details
                mailBody = "Your retail lab job failed. You can access your job report at " + taskInfo.jobDir + ".\n\nIf you think there is a problem with the retail lab infrastructure, please send a mail to " + mailOwner + " with the necessary details" + "\n\n" + mailJobText;
                mailSubject = mailSubject + " - FAILURE";
            }
            
            // Send an email
            mailSendText(mailTo, mailSubject, mailBody, mailCC, mailFrom);
        }
        catch (e)
        {
            // Log that an exception was thrown
            logMsg(LogRetailLab, LogInfo, "Caught exception thrown by RetailLabWorkerScript():\n" + e.description + "\n");

            // Send a failure email with some job details
            mailBody = "Your retail lab job failed. You can access your job report at " + taskInfo.jobDir + ".\n\nIf you think there is a problem with the retail lab infrastructure, please send a mail to " + mailOwner + " with the necessary details" + "\n\n" + mailJobText;
            mailSendText(mailTo, mailSubject + " - FAILURE", mailBody, mailCC, mailFrom);
        }
        finally
        {
            // Generate the retailLab SNAP report
            logMsg(LogRetailLab, LogInfo, "RetailLabWorkerScript: Generating report in ", outDirBase, "\n");
            _retailLabSnapReport(outDirBase + "\\..", jobDirBase, workerLogDir, supportedArchs);
        }
    }
    else if (taskInfo.taskType == "GenOptBuild" && taskInfo.privateJob == true)
    {
        // for private job's, the mail must be sent to the owner of the job
        if(taskInfo.contact.split("\\").length == 1) {
            mailTo = taskInfo.contact.split("\\")[0];
        }
        else {
            mailTo = taskInfo.contact.split("\\")[1];
        }
        mailTo += "@microsoft.com";

        try
        {
            var localBinDir = binariesLocal + "\\privateJob." + taskInfo.jobId;
            var uncLocalBinDir = binariesShare + "\\privateJob." + taskInfo.jobId;

            // Sync the enlistment to changeNum and unpack shelveset
            statusCode = doApplyChangesTask(taskInfo.outDir, taskInfo.changeNum, taskInfo.shelveset, srcBase);           
            mailJobText = mailJobText + "ApplyChangesTask returned: " + statusCode + "\n";
            
            var relUnoptBuildDir = bldArch + bldType;
            var relOptBuildDir = bldArch + bldType + ".opt";
            
            if (statusCode == 0)
            {
                // Call the GenOptBuild rolling task
                ret = _callGenOptBuildTask(bldArch, bldType, srcBase, taskInfo.outDir, localBinDir, relUnoptBuildDir,
                                          relOptBuildDir, taskInfo.changeNum, taskInfo.shelveset, taskInfo.privateJob);
    
                mailJobText = mailJobText + "GenOptBuildTask returned: " + ret + "\n";
            }
            else
            {
                // Log that the applyChanges task failed
                logMsg(LogRetailLab, LogInfo, "applyChanges task failed with exit code ", statusCode, "\n");

                // Send a failure email with some job details
                mailBody = "Your private retail lab job failed. You can access your job report at " + taskInfo.jobDir + ".\n\nIf you think there is a problem with the retail lab infrastructure, please send a mail to " + mailOwner + " with the necessary details\n\n" + mailJobText;
                mailSendText(mailTo, mailSubject + " - FAILURE", mailBody, mailCC, mailFrom);
            }

            // Log the result and stuff
            if (ret == 0 && statusCode == 0)
            {
                logMsg(LogRetailLab, LogInfo, "genOptBuild task succeeded!\n");

                // Create the DFS link

                var optBuildLink = taskInfo.jobDir + "\\" + bldArch + "pret\\bin";

                {
                    var runOpts = clrRunTemplate;
                    runOpts = runSetNoThrow(runOpts);
                    if (FSOFolderExists(optBuildLink))
                        runCmdToLog(srcBase + "\\ndp\\clr\\snap2.4\\tasks\\dfscmd /unmap " + optBuildLink, runOpts);
                    var run = runCmdToLog(srcBase + "\\ndp\\clr\\snap2.4\\tasks\\dfscmd /map " + optBuildLink + " " + uncLocalBinDir, runOpts);
                    mailJobText = mailJobText + "DfsLinksSetupTask returned: " + run.exitCode + "\n";
                    if (run.exitCode == 0)
                    {
                        logMsg(LogRetailLab, LogInfo, "Create DFS link successfully!\n");
                    }
                    else
                    {
                        // Ack! We could not create the DFS link! We should include a warning about this
                        // this the report that we email out!
                        logMsg(LogRetailLab, LogInfo, "Failed to create DFS link!\n");
                    }
                }

                // How do we figure out what the job folder is? "taskInfo.jobDir" tells us!

                // Send a "SUCCESS" email with some job details
                mailBody = "Your private retail lab job succeeded. You can access your job report at " + taskInfo.jobDir + "\n\n" + mailJobText;
                mailSendText(mailTo, mailSubject + " - SUCCESS", mailBody, mailCC, mailFrom);
            }
            else
            {
                // Log that the genOptBuild task failed
                logMsg(LogRetailLab, LogInfo, "genOptBuild task failed with exit code ", ret, "\n");

                // Send a failure email with some job details
                mailBody = "Your private retail lab job failed. You can access your job report at " + taskInfo.jobDir + ".\n\nIf you think there is a problem with the retail lab infrastructure, please send a mail to " + mailOwner + " with the necessary details\n\n" + mailJobText;
                mailSendText(mailTo, mailSubject + " - FAILURE", mailBody, mailCC, mailFrom);
            }
        }
        catch (e)
        {
            // Log that an exception was thrown
            logMsg(LogRetailLab, LogInfo, "Caught exception thrown by RetailLabWorkerScript():\n" + e.description + "\n");

            // Send a failure email with some job details
            mailBody = "Your private retail lab job failed. You can access your job report at " + taskInfo.jobDir + ".\n\nIf you think there is a problem with the retail lab infrastructure, please send a mail to " + mailOwner + " with the necessary details\n\n" + mailJobText;
            mailSendText(mailTo, mailSubject + " - FAILURE", mailBody, mailCC, mailFrom);
        }
        finally
        {
            var privateJobBase = jobDirBase + "\\..\\retailLab\\privateJobs";
            // Generate the retailLab private job report
            logMsg(LogRetailLab, LogInfo, "RetailLabWorkerScript: Generating report in ", workerLogDir, "\n");
            _retailLabPrivateReport(privateJobBase, workerLogDir, supportedArchs);
        }
    }

    // Generate the machine report for the retail lab
    _retailLabMachineReport(workerLogDir, jobDirBase);


    logMsg(LogRetailLab, LogInfo10, "} RetailLabWorkerScript() = ", ret, "\n");
    return ret;
}

/******************************************************************************/
/*********************** Retail Lab internal functionality ********************/
/* It is suggested that the following functions be used only through functions
   in this file. */
/******************************************************************************/

function doApplyChangesTask(outDir, changeNum, shelveset, srcBase, pathToTfCmd) {

    var mainTask = _applyChangesTask(changeNum, shelveset, pathToTfCmd);
    var env = {};
    env["taskClean"] = "taskClean";
    var ret = tasksRun([mainTask], outDir, srcBase, env);

    return ret;
}


function doRetailLabBuild(baseBldType, bldArch, bldRelDir, bldArgs, srcBase, binDir, logDir, timeOut) {
    // Make a .url file in logDir that points to the binDir
    FSOCreatePath(binDir);
    _publishFolder(logDir + "\\bin.url", binDir);

    // Do razzle build
    razzleBuild(baseBldType, bldArch, bldRelDir, bldArgs, srcBase, binDir, logDir, timeOut);
}


function doGenOptBuildTask(bldArch, bldType, srcBase, outDir, localBinDir, relUnoptBuildDir, relOptBuildDir, changeNum, shelveset, privateJob) {
    var mainTask = _genOptBuildTask(bldType, bldArch, localBinDir, relUnoptBuildDir, relOptBuildDir, privateJob);

    var env = {};
    env["taskClean"] = "taskClean";
    var ret = tasksRun([mainTask], outDir, srcBase, env);

    return ret;
}

function _callGenOptBuildTask(bldArch, bldType, srcBase, outDir, localBinDir, relUnoptBuildDir, relOptBuildDir, changeNum, shelveset, privateJob) {
    if (bldArch == undefined)
        throw new Error(1, "Required parameter bldArch is undefined");
    if (bldType == undefined)
        throw new Error(1, "Required parameter bldType is undefined");
    if (srcBase == undefined)
        throw new Error(1, "Required parameter srcBase is undefined");
    if (localBinDir == undefined)
        throw new Error(1, "Required parameter localBinDir is undefined");
    if (outDir == undefined)
        throw new Error(1, "Required parameter outDir is undefined");

    if (relUnoptBuildDir == undefined)
        relUnoptBuildDir = "_";
    if (relOptBuildDir == undefined)
        relOptBuildDir = "_";
    if (changeNum == undefined)
        changeNum = "_";
    if (shelveset == undefined)
        shelveset = "_";
    if (privateJob == undefined)
        privateJob = "_";

    var cmdLine = srcBase + "\\ndp\\clr\\bin\\runjs doGenOptBuildTask " + bldArch + " " + bldType + " " + srcBase + " " + 
                  outDir + " " + localBinDir + " " + relUnoptBuildDir + " " + relOptBuildDir + " " + changeNum + " " + 
                  shelveset + " " + privateJob;

    var runOpts = clrRunTemplate;
    runOpts = runSetNoThrow(runOpts);
    runOpts = runSetTimeout(259200, runOpts); // set timeout of 3 days
    runOpts = runSetEnv("_NTBINDIR", srcBase, runOpts);
    runOpts = runSetLog(LogRetailLab, LogInfo, runOpts);

    // Enter the WOW and call ldr64 if needed, then call "runjs doGenOptBuildTask"
    var run = runWithRuntimeArch(cmdLine, bldArch, runOpts);

    return run.exitCode;
}

/**********************************************************************/
/* Function to get the type of the next job for retail lab processing */
/* We switch between job types alternately to avoid starving          */
/* Currently we only have the following 2 job types:                  */
/* 1. SNAP                                                            */
/* 2. PRIVATE                                                         */
function _getNextJobType(taskName, jobDirBase, outDirBase, bldArch, bldType) {
    var privateJobDirs = [];
    var snapJobDirs = []; 
    
    var privateJobDirBase = jobDirBase + "\\..\\retailLab\\privateJobs\\" + bldArch;    

    // Sort the private jobs by the time they were last accessed
    // The job which has been accessed more recently will have a higher value
    // and will be at the front of the array
    var privateJobsFolder = FSOGetFolder(privateJobDirBase);
    for (var subFoldersEnum = new Enumerator(privateJobsFolder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
        if(_checkForRetailBuild(subFoldersEnum.item().Path, taskName, subFoldersEnum.item().Path, bldArch, bldType)) {
            privateJobDirs.push(subFoldersEnum.item());
        }
    }
    privateJobDirs.sort(function(x, y) { return (y.DateLastAccessed - x.DateLastAccessed); });
    
    // Sort the snap jobs by the time they were last accessed
    // The job which has been accessed more recently will have a higher value
    // and will be at the front of the array
    var snapJobDirs = [];
    var snapJobsFolder = FSOGetFolder(outDirBase);
    for (var subFoldersEnum = new Enumerator(snapJobsFolder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
        var jobDirName = subFoldersEnum.item().Name;
        if(jobDirName.match(/^(\d+)$/)) {
            if(_checkForRetailBuild(subFoldersEnum.item().Path, taskName, jobDirBase + "\\" + subFoldersEnum.item().Name, bldArch, bldType)) {
                snapJobDirs.push(subFoldersEnum.item());
            }
        }
    }
    snapJobDirs.sort(function(x, y) { return (y.DateLastAccessed - x.DateLastAccessed); });
    
    // Compare the 1st element of both the arrays
    // The result will determine which type of job will be processed
    // by the retail lab next
    var jobType = "SNAP";
    if(snapJobDirs.length <= 0 || privateJobDirs.length <= 0 || snapJobDirs[0].DateLastAccessed > privateJobDirs[0].DateLastAccessed) {
       jobType = "PRIVATE" ;
    }
    return jobType;
}

/**********************************************************************************/
/* Function to get the the next job for retail lab processing                     */
/* Currently we have only 2 types of jobs that can be processed by the retail lab */
function _getNextRetailLabTask(jobDirBase, outDirBase, bldArch, bldType) {

    logMsg(LogRetailLab, LogInfo, "_getNextRetailLabTask(", jobDirBase, ", ", outDirBase, ", ",
           bldArch, ", ", bldType, ") { \n");

    var privateJobDirBase = jobDirBase + "\\..\\retailLab\\privateJobs\\" + bldArch;

    var taskType = "GenOptBuild";
    var taskName = taskType + "@" + bldArch + bldType;
    var taskInfo = undefined;

    // Process a private job if a snap job was the last to be processed
    if(_getNextJobType(taskName, jobDirBase, outDirBase, bldArch, bldType) == "PRIVATE") {    
        var jobDirs = [];    
    
        var privateJobsFolder = FSOGetFolder(privateJobDirBase);
        for (var subFoldersEnum = new Enumerator(privateJobsFolder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
            jobDirs.push(subFoldersEnum.item());
        }
    
        jobDirs.sort(function(x, y) { return (x.DateCreated - y.DateCreated); });
    
        for (j = 0; j < jobDirs.length; j++)  {
            var jobDir = jobDirs[j];
            var jobDirName = jobDir.Name;
           
            // look for patterns like #_alias# (where # is a number)        
            if (jobDirName.match(/^(\d+)_([-a-zA-Z\.]+)(\d+)$/)) {
                var jobId = RegExp.$1 + "_" + RegExp.$2 + RegExp.$3;
    
                logMsg(LogRetailLab, LogInfo, "_getNextRetailLabTask: Found private jobDir: ", jobDir, "\n");
                var outDir = jobDir;
    
                // Read info from jobInfo.bat
                var changeset = undefined;
                var shelveset = undefined;
                var contact = undefined;
    
                var jobInfo = FSOReadFromFile(jobDir + "\\jobInfo.bat");
                if (jobInfo.match(/Changeset=(\S+)/m))
                    changeset = RegExp.$1;
                if (jobInfo.match(/Shelveset=(\S+)/m))
                    shelveset = RegExp.$1;
                if (jobInfo.match(/Contact=(\S+)/m))
                    contact = RegExp.$1;
                if(shelveset != undefined) {
                    var shelvesetWithUserName = shelveset.split(";");
                    if(shelvesetWithUserName.length == 1)
                        shelveset = shelveset + ";" + contact;
                }
    
                if (_checkAndAcquireForRetailBuild(outDir, taskName, jobDir, bldArch, bldType)) {
                    taskInfo = { taskName: taskName, taskType: taskType, bldArch: bldArch,
                                 bldType: bldType, jobDir: jobDir, outDir: outDir, jobId: jobId, 
                                 changeNum: changeset, shelveset: shelveset, privateJob: true,
                                 contact: contact };
                    break;
                }
            }
        }
    
        if (taskInfo != undefined)
        {
            logMsg(LogRetailLab, LogInfo, "} _getNextRetailLabTask: RETURNING taskName=", taskName, " jobDir=", jobDir, "\n");
            return taskInfo;
        }
        else
        {
            logMsg(LogRetailLab, LogInfo, "_getNextRetailLabTask: No private jobs found\n");
        }
    }

    // Get the skip changeset if it is available        
    var skipChangeset = undefined;
    var skipFilePath = outDirBase + "\\skipJobsTill.bat";
    if (FSOFileExists(skipFilePath) && (FSOReadFromFile(skipFilePath).match(/SkipChangeset=(\d+)/m))) {
        skipChangeset = RegExp.$1 * 1;
    }

    jobDirs = FSOGetDirPattern(jobDirBase, /\d+/);
    jobDirs.sort(function(x, y) { return (numSuffix(x, "") - numSuffix(y, "")); });
    for (j = 0; j < jobDirs.length; j++)  {
        var jobDir = jobDirs[j];

        if (jobDir.match(/\\(\d+)$/)) {
            var jobId = RegExp.$1;
            var jobIdNum = jobId * 1;
       
            // skip jobs till the skip changeset     
            if (skipChangeset != undefined && jobIdNum <= skipChangeset) {
                logMsg(LogRetailLab, LogInfo, "_getNextRetailLabTask: Skipping changeset: ", jobId, "\n");
                continue;
            }

            // We need to look at "JobInfo.bat" to get the original job directory . The original job
            // directory  should reside somwhere in "\\clrsnap0\queues\". We need the path to the original
            // job directory to set up DFS links for "x86pret\bin" and "amd64pret\bin".

            var jobInfo = jobDir + "\\JobInfo.bat";
            if (!FSOFileExists(jobInfo) || !FSOReadFromFile(jobInfo).match(/CurrentJobDir=(\S+)/m))
                continue;
            jobDir = RegExp.$1;

            logMsg(LogRetailLab, LogInfo, "_getNextRetailLabTask: Found SNAP jobDir: ", jobDir, "\n");
            var outDir = outDirBase + "\\" + jobId;

            // Always make the folder, so we know when there are builds with no test results
            if (!FSOFolderExists(outDir)) {
                FSOCreatePath(outDir);
                urlShortCutCreate(outDir + "\\jobDir.url", jobDir);
            }

            if (_checkAndAcquireForRetailBuild(outDir, taskName, jobDir, bldArch, bldType)) {
                taskInfo = { taskName: taskName, taskType: taskType, bldArch: bldArch,
                             bldType: bldType, jobDir: jobDir, outDir: outDir, jobId: jobId, 
                             changeNum: jobId, shelveset: undefined, privateJob: false,
                             contact: undefined };
                break;
            }
        }
    }

    if (taskInfo != undefined)
        logMsg(LogRetailLab, LogInfo, "} _getNextRetailLabTask: RETURNING taskName=", taskName, " jobDir=", jobDir, "\n");
    else
        logMsg(LogRetailLab, LogInfo, "} _getNextRetailLabTask: RETURNING undefined\n");

    return taskInfo;
}

/* Function to check if a job needs retail build and lock it if so */
function _checkAndAcquireForRetailBuild(outDir, taskName, jobDir, bldArch, bldType) {

    logMsg(LogRolling, LogInfo, "_checkAndAcquireForRetailBuild(", outDir, ", ", taskName, ") {\n");
    
    if (_checkForRetailBuild(outDir, taskName, jobDir, bldArch, bldType)) {
        // Check for the existence of the lock file
        var lockFile = outDir + "\\" + taskName + ".lock";
        try {
            WshFSO.OpenTextFile(lockFile, 8, true).Close();
        } catch(e) { // {
            logMsg(LogRolling, LogInfo, "} _checkAndAcquireForRetailBuild: Log file ", lockFile, " is locked\n");
            return false;
        }
        logMsg(LogRolling, LogInfo, "} _checkAndAcquireForRetailBuild: returning true\n");
        return true;
    }
    else {
        logMsg(LogRolling, LogInfo, "} _checkAndAcquireForRetailBuild: returning false\n");
        return false;
    }
}

/* Function to check if a job needs retail build */
function _checkForRetailBuild(outDir, taskName, jobDir, bldArch, bldType) {

    logMsg(LogRolling, LogInfo, "_checkForRetailBuild(", outDir, ", ", taskName, ") {\n");

    // Check for the existence of the results.txt file
    // If it exists, we skip this job
    // Note that this file is not generated by any job today 
    // and hence the following condition fails
    var resultsFile = outDir + "\\" + taskName + ".results.txt";
    if (FSOFileExists(resultsFile))  { // {
        logMsg(LogRolling, LogInfo, "} _checkForRetailBuild: results file: ",resultsFile, " exists, done\n");
        return false;
    }

    // Check for the existence of the status log files
    // If these exist, this job was already run
    var logPaths = new RegExp("^" + taskName + "\\.\\d+\\.status\\.log$", "i");
    var statusFiles = FSOGetFilePattern(outDir, logPaths);
    if (statusFiles.length > 0) {
        logMsg(LogRolling, LogInfo, "} _checkForRetailBuild: already run\n");
        return false;
    }

    // Check for the existence of the lock files
    // If these exist, another worker is processing this job    
    var lockPaths = new RegExp(".*\.lock$", "i");
    var lockFiles = FSOGetFilePattern(outDir, lockPaths);
    if (lockFiles.length > 0) {
        logMsg(LogRolling, LogInfo, "} _checkForRetailBuild: was either already run or is currently running on another worker. It is also possible that the job terminated unexpectedly\n");
        return false;
    }

    // Check for the existence of the pret folder in the snap job
    // This takes a dependency on the job folder, but for now we will continue to do this
    var finalOptBuildLink = jobDir + "\\" + bldArch + "p" + bldType;
    if(FSOFolderExists(finalOptBuildLink)) {
        logMsg(LogRolling, LogInfo, "} _checkForRetailBuild: already run\n");
        return false;
    }

    logMsg(LogRolling, LogInfo, "} _checkForRetailBuild: returning true\n");
    return true;
}

/* Function to refresh the retail lab reports every 10 minutes */
function checkAndUpdateRetailLabReports() {
    for (;;) {
        _retailLabStatisticsReport();
        _retailLabMachineReport();
        _retailLabPrivateReport();
        _retailLabSnapReport();
        logMsg(LogRetailLab, LogInfo, "Sleeping for 10 minutes...\n");
        WScript.Sleep(600000);
    }
}

/* Function to be called internally to begin writing a retail lab report */
function _retailLabReportBegin(htmlFile) {
    if (htmlFile == undefined)
        throw new Error(1, "HTML Report object not initialized");

    htmlFile.WriteLine("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3c.org/TR/1999/REC-html401-19991224/loose.dtd\">"); 
    htmlFile.WriteLine("<!-- saved from url=(0074)file://\\sanket2-vista\public\in\sanket\web\CLR Retail Lab Status Page.htm -->");
    htmlFile.WriteLine("<HTML xml:lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\"><HEAD><TITLE>CLR Retail Optimization Lab Status Page</TITLE>");
    htmlFile.WriteLine("<META content=\"text/html; charset=windows-1252\" http-equiv=Content-Type>");
    htmlFile.WriteLine("<STYLE type=text/css media=all>BODY {");
    htmlFile.WriteLine("FONT: small arial, helvetica, sans-serif");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#header UL {");
    htmlFile.WriteLine("    PADDING-BOTTOM: 0px; LIST-STYLE-TYPE: none; MARGIN: 0px; PADDING-LEFT: 0px; PADDING-RIGHT: 0px; PADDING-TOP: 0px");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#header LI {");
    htmlFile.WriteLine("    MARGIN: 0px 2px 0px 0px; DISPLAY: inline");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#header A {");
    htmlFile.WriteLine("    PADDING-BOTTOM: 0px; PADDING-LEFT: 1em; PADDING-RIGHT: 1em; BACKGROUND: #c3d9ff; TEXT-DECORATION: none; PADDING-TOP: 0px");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#header A:hover {");
    htmlFile.WriteLine("    BACKGROUND: #c3d9ff");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("0123456789a #header #selected {");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#header #selected A {");
    htmlFile.WriteLine("    PADDING-BOTTOM: 2px; BACKGROUND: #c3d9ff; COLOR: black; FONT-WEIGHT: bold");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#content {");
    htmlFile.WriteLine("    PADDING-BOTTOM: 1em; PADDING-LEFT: 1em; PADDING-RIGHT: 1em; BACKGROUND: #c3d9ff; BORDER-TOP: white 2px solid; PADDING-TOP: 1em");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("#content P {");
    htmlFile.WriteLine("    PADDING-BOTTOM: 1em; MARGIN: 0px; PADDING-LEFT: 1em; PADDING-RIGHT: 1em; BACKGROUND: white; PADDING-TOP: 1em");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("H1 {");
    htmlFile.WriteLine("    FONT-SIZE: 1.5em");
    htmlFile.WriteLine("}");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("table.helpT");
    htmlFile.WriteLine("{ text-align: center;");
    htmlFile.WriteLine("font-family: Verdana;");
    htmlFile.WriteLine("font-weight: normal;");
    htmlFile.WriteLine("font-size: 11px;");
    htmlFile.WriteLine("color: #404040;");
    htmlFile.WriteLine("width: 500px;");
    htmlFile.WriteLine("background-color: #fafafa;");
    htmlFile.WriteLine("border: 1px #6699CC solid;");
    htmlFile.WriteLine("border-collapse: collapse;");
    htmlFile.WriteLine("border-spacing: 0px; }");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("td.helpHed");
    htmlFile.WriteLine("{ border-bottom: 2px solid #6699CC;");
    htmlFile.WriteLine("border-left: 1px solid #6699CC;");
    htmlFile.WriteLine("background-color: #BEC8D1;");
    htmlFile.WriteLine("text-align: center;");
    htmlFile.WriteLine("text-indent: 5px;");
    htmlFile.WriteLine("font-family: Verdana;");
    htmlFile.WriteLine("font-weight: bold;");
    htmlFile.WriteLine("font-size: 11px;");
    htmlFile.WriteLine("color: #404040; }");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("td.helpBod");
    htmlFile.WriteLine("{ border-bottom: 1px solid #9CF;");
    htmlFile.WriteLine("border-top: 0px;");
    htmlFile.WriteLine("border-left: 1px solid #9CF;");
    htmlFile.WriteLine("border-right: 0px;");
    htmlFile.WriteLine("text-align: left;");
    htmlFile.WriteLine("text-indent: 10px;");
    htmlFile.WriteLine("font-family: Verdana, sans-serif, Arial;");
    htmlFile.WriteLine("font-weight: normal;");
    htmlFile.WriteLine("font-size: 11px;");
    htmlFile.WriteLine("color: #404040;");
    htmlFile.WriteLine("background-color: #fafafa; }");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("table.sofT");
    htmlFile.WriteLine("{ text-align: center;");
    htmlFile.WriteLine("font-family: Verdana;");
    htmlFile.WriteLine("font-weight: normal;");
    htmlFile.WriteLine("font-size: 11px;");
    htmlFile.WriteLine("color: #404040;");
    htmlFile.WriteLine("width: 580px;");
    htmlFile.WriteLine("background-color: #fafafa;");
    htmlFile.WriteLine("border: 1px #6699CC solid;");
    htmlFile.WriteLine("border-collapse: collapse;");
    htmlFile.WriteLine("border-spacing: 0px; }");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("</STYLE>");
}

/* Function to be called internally to end writing a retail lab report */
function _retailLabReportEnd(htmlFile) {
    if (htmlFile == undefined)
        throw new Error(1, "HTML Report object not initialized");
    htmlFile.WriteLine("</BODY>");
    htmlFile.WriteLine("</HTML>");
    htmlFile.Close();
}

/* Function to generate a html report for the statistics of the retail lab job */
function _retailLabStatisticsReport(workerLogDir, jobDirBase, reportHtmlFile) {

    if (jobDirBase == undefined) 
        jobDirBase = "\\\\clrsnap0\\public\\tfs_puclr\\builds";       
    if (workerLogDir == undefined)
        workerLogDir = jobDirBase + "\\..\\retailLab\\workerLogs";
    if (reportHtmlFile == undefined)
        reportHtmlFile = workerLogDir + "\\RetailLabStatisticsReport.html";

    logCall(LogRetailLab, LogInfo, "_retailLabStatisticsReport", arguments);

    var tempName = reportHtmlFile + "." + Env("COMPUTERNAME") + ".new"; 
    var htmlFile = FSOOpenTextFile(tempName, 2, true); 

    // Begin writing to the html report    
    _retailLabReportBegin(htmlFile);
    
    // Write type specific information to the html report
    htmlFile.WriteLine("<META name=GENERATOR content=\"MSHTML 8.00.7000.0\"></HEAD>");
    htmlFile.WriteLine("<BODY>");
    htmlFile.WriteLine("<DIV id=header>");
    htmlFile.WriteLine("<H1>CLR Retail Optimization Lab</H1>");
    htmlFile.WriteLine("<UL>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabSnapReport.html\">" + "Snap Jobs</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabPrivateReport.html\">" + "Private Jobs</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabMachineReport.html\">" + "Machine Status</A>");
    htmlFile.WriteLine("  <LI id=selected>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabStatisticsReport.html\">" + "Statistics</A>");
    htmlFile.WriteLine("  </LI>");
    htmlFile.WriteLine("  </UL></DIV>");
    
    // End writing to the html report    
    _retailLabReportEnd(htmlFile);
    for (var i = 0; i < 1000; i++) {
        if (!FSOFileExists(reportHtmlFile)) {
            FSOMoveFile(tempName, reportHtmlFile);
            break;
        }
        try { FSODeleteFile(reportHtmlFile); } catch(e) {};
        WScript.Sleep(10);        // Only browsers should be fetching this file
    }
}

/* Function to generate a html report for the machines owned by the retail lab job */
function _retailLabMachineReport(workerLogDir, jobDirBase, reportHtmlFile) {

    if (jobDirBase == undefined) 
        jobDirBase = "\\\\clrsnap0\\public\\tfs_puclr\\builds";       
    if (workerLogDir == undefined)
        workerLogDir = jobDirBase + "\\..\\retailLab\\workerLogs";
    if (reportHtmlFile == undefined)
        reportHtmlFile = workerLogDir + "\\RetailLabMachineReport.html";

    logCall(LogRetailLab, LogInfo, "_retailLabMachineReport", arguments);

    var tempName = reportHtmlFile + "." + Env("COMPUTERNAME") + ".new"; 
    var htmlFile = FSOOpenTextFile(tempName, 2, true); 

    // Begin writing to the html report    
    _retailLabReportBegin(htmlFile);
    
    // Write type specific information to the html report
    htmlFile.WriteLine("<META name=GENERATOR content=\"MSHTML 8.00.7000.0\"></HEAD>");
    htmlFile.WriteLine("<BODY>");
    htmlFile.WriteLine("<DIV id=header>");
    htmlFile.WriteLine("<H1>CLR Retail Optimization Lab</H1>");
    htmlFile.WriteLine("<UL>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabSnapReport.html\">" + "Snap Jobs</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabPrivateReport.html\">" + "Private Jobs</A>");
    htmlFile.WriteLine("  <LI id=selected>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabMachineReport.html\">" + "Machine Status</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabStatisticsReport.html\">" + "Statistics</A>");
    htmlFile.WriteLine("  </LI>");
    htmlFile.WriteLine("  </UL></DIV>");
    htmlFile.WriteLine("<DIV id=content>");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("<table summary=\"Machines used by the retail lab\" class=\"sofT\" cellspacing=\"0\">");
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("    <td colspan=\"4\" class=\"helpHed\">Retail Lab Machines</td>");
    htmlFile.WriteLine("</tr>");
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Job</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Machine</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Architecture</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Start Time</td>");
    htmlFile.WriteLine("</tr>");

    var workerFiles = FSOGetFilePattern(workerLogDir, /.*\.runjs.log$/i);
    for(var i=0; i<workerFiles.length; i++) {
        var startTime = "IDLE";
        var arch = "UNKNOWN";
        var machine = FSOGetFileNameWithoutExtension(workerFiles[i]);
        var jobName = "NO JOB FOR PROCESSING";
        var changeset = undefined;
        var workerData = FSOReadFromFile(workerFiles[i]);
        if (workerData.match(/RETURNING\staskName=GenOptBuild@amd64ret\sjobDir=(.*)\n/i) ||
            workerData.match(/RETURNING\staskName=GenOptBuild@x86ret\sjobDir=(.*)\n/i)) {
            var jobName = trim(RegExp.$1);
            if (workerData.match(/inclusiveStartTime:\s*(.*)$/im)) {
                startTime = RegExp.$1;
            }
            if (workerData.match(/doGenOptBuildTask\s*(\S+)/im)) {
                arch = RegExp.$1;
            }
            
            // Read info from jobInfo.bat
            var jobInfo = FSOReadFromFile(jobName + "\\jobInfo.bat");
            if (jobInfo.match(/Changeset=(\S+)/m)) {
                changeset = RegExp.$1;
                if (jobInfo.match(/Shelveset=(\S+)/m)) {
                    changeset = changeset + " - " + RegExp.$1;
                    if (jobInfo.match(/Contact=(\S+)/m)) {
                        changeset = changeset + ";" + RegExp.$1;
                    }
                }
            } else if (jobInfo.match(/ChangeNum=(\S+)/m)) {
                changeset = RegExp.$1;
            }
            jobName = "<A href=" + jobName + ">" + changeset;
        }
        htmlFile.WriteLine("<tr>");
        htmlFile.WriteLine("    <td>" + jobName + "</td>");
        htmlFile.WriteLine("    <td>" + "<A href=\\\\" + machine + "\\binaries>" + machine + "</td>");
        htmlFile.WriteLine("    <td>" + arch + "</td>");
        htmlFile.WriteLine("    <td>" + startTime + "</td>");
        htmlFile.WriteLine("</tr>");
    }
            
    // End writing to the html report    
    _retailLabReportEnd(htmlFile);
    for (var i = 0; i < 1000; i++) {
        if (!FSOFileExists(reportHtmlFile)) {
            FSOMoveFile(tempName, reportHtmlFile);
            break;
        }
        try { FSODeleteFile(reportHtmlFile); } catch(e) {};
        WScript.Sleep(10);        // Only browsers should be fetching this file
    }
}

/* Function to generate a html report for the private jobs processed by the retail lab job */
function _retailLabPrivateReport(outDirBase, workerLogDir, archs, reportHtmlFile) {

    if (outDirBase == undefined) 
        outDirBase = "\\\\clrsnap0\\public\\tfs_puclr\\retailLab\\privateJobs";
    if (workerLogDir == undefined)
        workerLogDir = outDirBase + "\\..\\workerLogs";
    if (reportHtmlFile == undefined) 
        reportHtmlFile = workerLogDir + "\\RetailLabPrivateReport.html";
    if (archs == undefined)
        archs = supportedArchs;

    logCall(LogRetailLab, LogInfo, "_retailLabPrivateReport", arguments);

    var tempName = reportHtmlFile + "." + Env("COMPUTERNAME") + ".new"; 
    var htmlFile = FSOOpenTextFile(tempName, 2, true); 

    // Begin writing to the html report    
    _retailLabReportBegin(htmlFile);
    
    // Write type specific information to the html report
    htmlFile.WriteLine("<META name=GENERATOR content=\"MSHTML 8.00.7000.0\"></HEAD>");
    htmlFile.WriteLine("<BODY>");
    htmlFile.WriteLine("<DIV id=header>");
    htmlFile.WriteLine("<H1>CLR Retail Optimization Lab</H1>");
    htmlFile.WriteLine("<UL>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabSnapReport.html\">" + "Snap Jobs</A>");
    htmlFile.WriteLine("  <LI id=selected>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabPrivateReport.html\">" + "Private Jobs</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabMachineReport.html\">" + "Machine Status</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabStatisticsReport.html\">" + "Statistics</A>");
    htmlFile.WriteLine("  </LI>");
    htmlFile.WriteLine("  </UL></DIV>");
    htmlFile.WriteLine("<DIV id=content>");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("<table summary=\"Private Jobs processed by the retail lab\" class=\"sofT\" cellspacing=\"0\">");
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("    <td colspan=\"9\" class=\"helpHed\">Private Jobs</td>");
    htmlFile.WriteLine("</tr>");
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Changeset</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Shelveset</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Domain\\Alias</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Description</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Machine</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Architecture</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Start Time</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Duration</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Failures</td>");
    htmlFile.WriteLine("</tr>");
    
    var jobDirs = [];    
    var privateJobsFolder = undefined;
    for (var i=0; i<archs.length; i++) {
        privateJobsFolder = FSOGetFolder(outDirBase + "\\" + archs[i]);
        for (var subFoldersEnum = new Enumerator(privateJobsFolder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
            jobDirs.push(subFoldersEnum.item());
        }
    }
    jobDirs.sort(function(x, y) { return (x.DateCreated - y.DateCreated); });

    for (var j = 0; j < jobDirs.length; j++)  {
        var jobDir = jobDirs[j];
        var jobDirName = jobDir.Name;
       
        // look for patterns like #_alias# (where # is a number)        
        if (jobDirName.match(/^(\d+)_([-a-zA-Z\.]+)(\d+)$/)) {
            var jobId = RegExp.$1 + "_" + RegExp.$2 + RegExp.$3;
            var outDir = jobDir;

            // Read info from jobInfo.bat
            var changeset = undefined;
            var shelveset = undefined;
            var contact = undefined;
            var description = undefined;
            var jobInfo = FSOReadFromFile(jobDir + "\\jobInfo.bat");
            if (jobInfo.match(/Changeset=(\S+)/m))
                changeset = RegExp.$1;
            if (jobInfo.match(/Shelveset=(\S+)/m))
                shelveset = RegExp.$1;
            if (jobInfo.match(/Contact=(\S+)/m))
                contact = RegExp.$1;
            if (jobInfo.match(/Description=(.*)/m))
                description = RegExp.$1;

            // Read info from log files            
            var date = "NODATA";
            var duration = "NODATA";
            var machine = "NODATA";
            var failures = "NODATA";
            var statusFiles = FSOGetFilePattern(outDir, /genOptBuild.*.status.log/i);
            if (statusFiles.length > 0) {            
                var statusData = FSOReadFromFile(statusFiles[statusFiles.length-1]);
                if (statusData.match(/^machine:\s*(.*)$/im))
                    machine = RegExp.$1;
                if (statusData.match(/^startTime:.*?(\w+ \d+ \d+:\d\d)/im))
                    date = RegExp.$1;

                if (statusData.match(/^inclusiveStartTime:\s*(.*)$/im)) {

                    var startTime = RegExp.$1;
                    var startTicks = new Date(RegExp.$1).getTime();

                    if (startTime.match(/w+ \d+ \d+:\d\d/))
                        date = RegExp.$1;
                
                    if (statusData.match(/^endTime:\s*(.*)$/im)) {
                        var endTicks = new Date(RegExp.$1).getTime();
                        duration = ((endTicks - startTicks) / 60000).toFixed(2) + " min";
                    }
                    if (statusData.match(/^result:\s*(\S+)/im)) 
                        failures = RegExp.$1;
                }
                numNoData = 0;
            }
            else  {
                numNoData++;            
            }
            
            // Calculate remaining info
            var arch = jobDir.ParentFolder.Name;
            var dispArch = "<A href=" + jobDir.Path + "\\taskReport.html" + ">" + arch + "</A>";
            
            htmlFile.WriteLine("<tr>");
            htmlFile.WriteLine("    <td>" + changeset + "</td>");
            htmlFile.WriteLine("    <td>" + shelveset + "</td>");
            htmlFile.WriteLine("    <td>" + contact + "</td>");
            htmlFile.WriteLine("    <td>" + description + "</td>");
            htmlFile.WriteLine("    <td>" + machine + "</td>");
            htmlFile.WriteLine("    <td>" + dispArch + "</td>");
            htmlFile.WriteLine("    <td>" + date + "</td>");
            htmlFile.WriteLine("    <td>" + duration + "</td>");
            htmlFile.WriteLine("    <td>" + failures + "</td>");
            htmlFile.WriteLine("</tr>");            
        }        
    }
    
    // End writing to the html report    
    _retailLabReportEnd(htmlFile);
    for (var i = 0; i < 1000; i++) {
        if (!FSOFileExists(reportHtmlFile)) {
            FSOMoveFile(tempName, reportHtmlFile);
            break;
        }
        try { FSODeleteFile(reportHtmlFile); } catch(e) {};
        WScript.Sleep(10);        // Only browsers should be fetching this file
    }
}

/* Function to generate a snap html report for a retail lab job */
function _retailLabSnapReport(outDirBase, jobDirBase, workerLogDir, archs, reportHtmlFile) {

    if (jobDirBase == undefined) 
        jobDirBase = "\\\\clrsnap0\\public\\tfs_puclr\\builds";       
    if (outDirBase == undefined) 
        outDirBase = jobDirBase + "\\..\\retailLab";
    if (workerLogDir == undefined)
        workerLogDir = jobDirBase + "\\..\\retailLab\\workerLogs";
    if (reportHtmlFile == undefined) 
        reportHtmlFile = workerLogDir + "\\RetailLabSnapReport.html";
    if (archs == undefined)
        archs = supportedArchs;

    logCall(LogRetailLab, LogInfo, "_retailLabSnapReport", arguments);

    var tempName = reportHtmlFile + "." + Env("COMPUTERNAME") + ".new"; 
    var htmlFile = FSOOpenTextFile(tempName, 2, true); 

    // Begin writing to the html report    
    _retailLabReportBegin(htmlFile);

    // Write type specific information to the html report
    htmlFile.WriteLine("<META name=GENERATOR content=\"MSHTML 8.00.7000.0\"></HEAD>");
    htmlFile.WriteLine("<BODY>");
    htmlFile.WriteLine("<DIV id=header>");
    htmlFile.WriteLine("<H1>CLR Retail Optimization Lab</H1>");
    htmlFile.WriteLine("<UL>");
    htmlFile.WriteLine("  <LI id=selected>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabSnapReport.html\">" + "Snap Jobs</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabPrivateReport.html\">" + "Private Jobs</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabMachineReport.html\">" + "Machine Status</A>");
    htmlFile.WriteLine("  <LI>");
    htmlFile.WriteLine("      <A href=\"" + workerLogDir + "\\RetailLabStatisticsReport.html\">" + "Statistics</A>");
    htmlFile.WriteLine("  </LI>");
    htmlFile.WriteLine("  </UL></DIV>");
    htmlFile.WriteLine("<DIV id=content>");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("<table summary=\"Snap Jobs processed by the retail lab\" class=\"sofT\" cellspacing=\"0\">");
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("    <td colspan=\"7\" class=\"helpHed\">Snap Jobs</td>");
    htmlFile.WriteLine("</tr>");
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Changeset</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">SNAP Job</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Machine</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Architectures</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Start Time</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Duration</td>");
    htmlFile.WriteLine("    <td class=\"helpHed\">Failures</td>");
    htmlFile.WriteLine("</tr>");

    var jobDirs = FSOGetDirPattern(jobDirBase, /^\d+$/);
    jobDirs.sort(function(x, y) { return (numSuffix(y, "") - numSuffix(x, "")); });

    // for every snap job directory
    var numNoData = 0;
    for (var j = 0; j < jobDirs.length; j++) {
        var jobDir = jobDirs[j];
        var changeNum = "UNKNOWN";
        if (jobDir.match(/\\(\d+)$/))
            changeNum = RegExp.$1;
        var dispChangeNum = changeNum;

        if (j > 10 && numNoData > 2)
            break;

        var jobInfo = jobDir + "\\JobInfo.bat";

        if (FSOFileExists(jobInfo) && FSOReadFromFile(jobInfo).match(/CurrentJobDir=(\S+)/m))
            jobDir = RegExp.$1;

        var dispJobDir = "<A href=" + relPath(jobDir, workerLogDir) + ">" + jobDir;
        var jobReport = jobDir + "\\JobReport.html";
        if (FSOFileExists(jobReport))
            dispJobDir = "<A href=" + relPath(jobReport, workerLogDir) + ">" + 
                         _parseSnapJobReport(jobReport).entries.join(" <BR> ") + "</A>";

        // for every architecture processed by the retail lab for this job
        for (var i=0; i<archs.length; i++) {
            var dispArch = archs[i];
            var outDirChange = outDirBase + "\\" + dispArch + "\\" + changeNum;
            var outDir = outDirChange;
            // we have a loop here to take care of multiple runs of the same job
            var runNum = 0;
            for(;;) {
                var date = "NODATA";
                var duration = "NODATA";
                var machine = "NODATA";
                var failures = "NODATA";
                var dispOutDir = "NODATA";
    
                // in general, this condition will fail in the 1st iteration
                // and pass in the 2nd iteration
                if (!FSOFolderExists(outDir)) {
                    if (runNum > 0)
                        break;
                } else {
                    outDir.match(/(\d+\.?\d*)$/);
                    var ref = outDir + "\\taskReport.html";
                    if (!FSOFileExists(ref)) 
                        ref = outDir;
                        dispOutDir = "<A href=" + relPath(ref, workerLogDir) + ">" + dispArch + "</A>";
                    failures = "INCOMPLETE";
                    date = duration = machine = "UNKNOWN";
                }
    
                var statusFiles = FSOGetFilePattern(outDir, /genOptBuild.*.status.log/i);
                if (statusFiles.length > 0) {
                    
                    var statusData = FSOReadFromFile(statusFiles[statusFiles.length-1]);
                    if (statusData.match(/^machine:\s*(.*)$/im))
                        machine = RegExp.$1;
                    if (statusData.match(/^startTime:.*?(\w+ \d+ \d+:\d\d)/im))
                        date = RegExp.$1;
                }
    
                var statusFiles = FSOGetFilePattern(outDir, /genOptBuild.*.status.log/i);
                if (statusFiles.length > 0) {
                    var statusData = FSOReadFromFile(statusFiles[statusFiles.length-1]);
    
                    if (statusData.match(/^inclusiveStartTime:\s*(.*)$/im)) {
    
                        var startTime = RegExp.$1;
                        var startTicks = new Date(RegExp.$1).getTime();
    
                        if (startTime.match(/w+ \d+ \d+:\d\d/))
                            date = RegExp.$1;
                    
                        if (statusData.match(/^endTime:\s*(.*)$/im)) {
                            var endTicks = new Date(RegExp.$1).getTime();
                            duration = ((endTicks - startTicks) / 60000).toFixed(2) + " min";
                        }
                        if (statusData.match(/^result:\s*(\S+)/im)) 
                            failures = RegExp.$1;
                    }
                    numNoData = 0;
                }
                else 
                    numNoData++;
    
                outDir.match(/([^\\]+)$/);
                var dispDir = RegExp.$1;
                
                htmlFile.WriteLine("<tr>");
                htmlFile.WriteLine("    <td>" + dispChangeNum + "</td>");
                htmlFile.WriteLine("    <td>" + dispJobDir + "</td>");
                htmlFile.WriteLine("    <td>" + machine + "</td>");
                htmlFile.WriteLine("    <td>" + dispOutDir + "</td>");
                htmlFile.WriteLine("    <td>" + date + "</td>");
                htmlFile.WriteLine("    <td>" + duration + "</td>");
                htmlFile.WriteLine("    <td>" + failures + "</td>");
                htmlFile.WriteLine("</tr>");
      
                runNum++;
                outDir = outDirChange + "." + runNum;
            } 
            dispChangeNum = "";
        }
    }

    htmlFile.WriteLine("</table>");
    htmlFile.WriteLine("");
    htmlFile.WriteLine("</DIV>");

    var baseTaskFile = outDirBase + "\\taskReport.html";
    if (FSOFileExists(baseTaskFile)) {
        htmlFile.WriteLine("<P>");
        htmlFile.WriteLine("If there is a problem with the generation of the rolling test reports itself");
        htmlFile.WriteLine("it can be debugged by looking at the following task report: ");
        htmlFile.WriteLine("<A href=" + relPath(baseTaskFile, workerLogDir)  + ">" + uncPath(baseTaskFile) + "</A>");
    }

    // End writing to the html report    
    _retailLabReportEnd(htmlFile);
    for (var i = 0; i < 1000; i++) {
        if (!FSOFileExists(reportHtmlFile)) {
            FSOMoveFile(tempName, reportHtmlFile);
            break;
        }
        try { FSODeleteFile(reportHtmlFile); } catch(e) {};
        WScript.Sleep(10);        // Only browsers should be fetching this file
    }
}

/* Function to cleanup a retail lab worker. This functionality is supposed to used
   from within this script and it is suggested that this behavior be maintained */
function _retailWorkerHouseCleanAndSetup(srcBase) {
    logCall(LogRetailLab, LogInfo10, "_retailWorkerHouseCleanAndSetup", arguments, "{");

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if(srcBase == undefined)
            throw Error(1, "srcBase not defined\n")
    }

    var machine = Env("COMPUTERNAME").toLowerCase();

    // remove the temp directory on this worker
    logMsg(LogRetailLab, LogInfo, "Removing the %TEMP% directory on this worker\n");
    runCmdToLog("del /s /q %TEMP%\\*.*", runSetNoThrow());    

    // Do housecleaning on this machine's local binaries folder (c:\binaries)
    logMsg(LogRetailLab, LogInfo, "HouseCleaning ", binariesLocal, ", removing so that the hard drive has 20% free space\n");

    // Set up the binaries folder if it does not already exist
    var localSrcBase = srcBase;
    var binariesBase = "binaries";
    var binariesShare = "\\\\" + Env("COMPUTERNAME") + "\\" + binariesBase;
    if (!localSrcBase.match(/^(\w:\\)/)) {
        throw Error(1, "localSrcBase " + localSrcBase + " must begin with drive specification");
    }
    var binariesLocal = RegExp.$1 + binariesBase;
    
    if (!FSOFolderExists(binariesShare)) {    
        logMsg(LogRetailLab, LogInfo, "_retailWorkerHouseCleanAndSetup: Creating binaries folder\n");
        
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
    _removeOldest(binariesLocal, 0.20, 2, 1000, "clrsnap0");
    
    // turn off system restore because it fragments the disk
    logMsg(LogRetailLab, LogInfo, "Turning OFF System Restore because it fragments the disk\n");
    try {
        var systemRestore = GetObject("winmgmts:root/default:SystemRestore");
        var systemDrive = Env("SystemDrive") + "\\";
        logMsg(LogRetailLab, LogInfo10, "System Drive ", systemDrive, "\n");
        systemRestore.disable(systemDrive);
        logMsg(LogRetailLab, LogWarn, "warning: SystemRestore.disable completed, this should only be seen once after OS install\n");
    } catch(e) {
        logMsg(LogRetailLab, LogWarn, "failure turning off SystemRestore this is normal on W2k3\n");
    }

    // turn off screen saver since retail build scenarios change if the desktop is not active
    logMsg(LogRetailLab, LogInfo, "Turning OFF screen saver since retail build scenarios change if the desktop is not active\n");
    runCmdToLog("reg add \"\\\\" + machine + "\\HKU\\S-1-5-21-2127521184-1604012920-1887927527-717868\\Control Panel\\Desktop\" /v ScreenSaveActive /t REG_SZ /d \"0\" /f");

    // allow WSF files to be run from the command line 
    logMsg(LogRetailLab, LogInfo, "Allowing .wsf files to be run from the command line\n");
    runCmdToLog("CScript //H:CSCRIPT");

    // This runs managed code on server machines turn it off.
    logMsg(LogRetailLab, LogInfo, "Turning OFF msdtc service\n");
    runCmdToLog("net stop msdtc", runSetNoThrow());
    
    // turn off reboot reason box on bootup
    logMsg(LogRetailLab, LogInfo, "Turning OFF Reboot Reason Box\n");
    runCmdToLog("sc config ersvc start= disabled");
    
    // turn off firewall for XP SP2+ and 2k3 SP1 b 1289+
    logMsg(LogRetailLab, LogInfo, "Turning OFF firewall\n");
    runCmdToLog("sc stop SharedAccess", runSetNoThrow());
    runCmdToLog("sc config SharedAccess start= disabled", runSetNoThrow());
    
    // shutdown ETrust
    logMsg(LogRetailLab, LogInfo, "Turning OFF etrust\n");
    runCmdToLog("net stop InoRT", runSetNoThrow());
    runCmdToLog("net stop InoTask", runSetNoThrow());
    runCmdToLog("net stop InoRpc", runSetNoThrow());
    runCmdToLog("kill Realmon.exe", runSetNoThrow());

    logMsg(LogRetailLab, LogInfo10, "} _retailWorkerHouseCleanAndSetup(srcBase)", "\n");
}

/* Function to get the status of a retail lab job */
function _getRetailLabJobStatus(jobDir, taskName) {
    var jobStatus = "Pending";
    var lockPaths = new RegExp(".*\.lock$", "i");
    var logPaths = new RegExp("^" + taskName + "\\.\\d+\\.status\\.log$", "i");
    var resultsFile = jobDir + "\\" + taskName + ".results.txt";        
    if (FSOFileExists(resultsFile)) {
        jobStatus = "Completed";
    }
    else if(FSOGetFilePattern(jobDir, lockPaths).length > 0) {
        jobStatus = "Running";
    }
    else if (FSOGetFilePattern(jobDir, logPaths).length > 0) {
        jobStatus = "Completed";
    }
    return jobStatus;
}


/* Function to get all the retail lab jobs in the specified directory */
function _getRetailLabJobsInDirectory(jobDirBase, bldArch, taskName) {
    var jobs = [];

    var jobPattern = new RegExp("^(\\d+)$", "i");
    var jobType = "SNAP PU\\CLR";
    var jobInfo = {};
    var jobCount = 0;
    var outDirBase = jobDirBase + "\\..\\retailLab\\" + bldArch;
    if(!FSOFolderExists(outDirBase)) {
        jobType = "PRIVATE PU\\CLR";
        jobPattern = new RegExp("^(\\d+)_([-a-zA-Z\.]+)(\\d+)$", "i");
    }

    var jobDirs = [];
    var allJobsFolder = FSOGetFolder(jobDirBase);
    for (var subFoldersEnum = new Enumerator(allJobsFolder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
        var name = subFoldersEnum.item().Name;
        if (name.match(jobPattern)) {
            jobDirs.push(subFoldersEnum.item());
        }
    }
    jobDirs.sort(function(x, y) { return (x.DateCreated - y.DateCreated); });

    for (j = 0; j < jobDirs.length; j++)  {
        var jobDir = jobDirs[j];
        var jobContact = "REDMOND\\clrgnt";
        var jobStatus = undefined;
        var jobId = undefined;
        var jobDescription = undefined;  
        if (jobType == "SNAP PU\\CLR") {
            jobDir.Name.match(jobPattern);
            jobId = RegExp.$1;
        } else {
            jobDir.Name.match(jobPattern);
            jobId = RegExp.$1 + "_" + RegExp.$2 + RegExp.$3;
        }

        var outDir = jobDir;
        var contact = undefined;
        var jobInfoFile = jobDir + "\\jobInfo.bat";
        if(!FSOFileExists(jobInfoFile))
            continue;
        var jobInfo = FSOReadFromFile(jobInfoFile);
        if (jobInfo.match(/CurrentJobDir=(\S+)/m)) {            
            jobDir = RegExp.$1;
            jobStatus = _getRetailLabJobStatus(outDirBase + "\\" + jobId, taskName);
            jobDescription = "Job checked-in through changeset#: " + jobId;
        }
        else if (jobInfo.match(/Contact=(\S+)/m)) {
            jobContact = RegExp.$1;
            jobStatus = _getRetailLabJobStatus(jobDirBase + "\\" + jobId, taskName);
            jobInfo.match(/Description=(\S+)/m);
            jobDescription = RegExp.$1;
        }

        jobInfo = {jobId: jobId, jobType: jobType, jobContact: jobContact, jobStatus: jobStatus, jobDir: jobDir};            
        jobs.push(jobInfo);
   }
    
    return jobs;
}

/* Function to get the list of all the retail lab tasks. 
   Currently we only have genopt build tasks, but later we may have more */
function _getAvailableRetailLabJobs(jobDirBase, bldArch, bldType) {
    if(jobDirBase == undefined)
        jobDirBase = "\\\\clrsnap0\\public\\tfs_puclr\\builds";
    if(bldArch == undefined) {
        bldArch = Env("_BuildArch");
        if(bldArch == undefined)
            throw Error("bldArch is not specified\n");
    }    
    if(bldType == undefined) {
        bldType = "ret";
    }    
    
    var jobs = [];
    var privateJobDirBase = jobDirBase + "\\..\\retailLab\\privateJobs";

    var taskType = "GenOptBuild";
    var taskName = taskType + "@" + bldArch + bldType;

    // Get all the private jobs    
    var privateJobs = _getRetailLabJobsInDirectory(privateJobDirBase + "\\" + bldArch, bldArch, taskName);
    for(j = 0; j < privateJobs.length; j++) {
        jobs.push(privateJobs[j]);
    }
   
    // Get all the checkin jobs
    var snapJobs = _getRetailLabJobsInDirectory(jobDirBase, bldArch, taskName);
    for(j = 0; j < snapJobs.length; j++) {
        jobs.push(snapJobs[j]);
    }
    
    return jobs;    
}
