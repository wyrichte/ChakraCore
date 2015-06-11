/******************************************************************************/
/*                                                   clrRetailLab.js                                                   */
/******************************************************************************/
/* This file provides functionality for the CLR Retail Lab.                                     */
// OWNER: sanket
// CONTACT: clrrlnot
/******************************************************************************/

var g_nClrRetailLabModuleDefined = 1;                     // Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include ClrAutomation.js");
if (!clrTaskModuleDefined) throw new Error(1, "Need to include clrTask.js");

var g_oRetailLabLogger = logNewFacility("clrRetailLab");
logSetFacilityLevel(g_oRetailLabLogger, LogInfo100000);

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined) 
    var Env = WshShell.Environment("PROCESS");
var MACHINE_NAME = String(Env("COMPUTERNAME")).toUpperCase();

/****************************************************************************/
/* RETAIL LAB DATABASE CONSTANTS */
/****************************************************************************/
var g_sRetailLabJobTable = "RetailLab.dbo.Job_Table";
var g_sRetailLabMachineTable = "RetailLab.dbo.Machine_Table";
var g_sRetailLabBranchTable = "RetailLab.dbo.Branch_Table";
var g_sRetailLabTemporaryBranchTable = "RetailLab.dbo.Temporary_Branch_Table";
var g_sRetailLabSupportedBranchTable = "RetailLab.dbo.Supported_Branch_Table";
var g_sRetailLabShareTable = "RetailLab.dbo.Share_Table";

/****************************************************************************/
/* RETAIL LAB OTHER GENERAL CONSTANTS */
/****************************************************************************/
var INVALID_JOBID = 9999999;
var clrRetailLabOwnerMailAddress = "clrrlnot@microsoft.com";
var g_sRetailLabMaster = "CLRRET0";
var g_sRetailLabPublishRoot = "\\\\" + g_sRetailLabMaster + "\\PUBLIC";
var g_sRetailLabStoreRoot = g_sRetailLabPublishRoot + "\\DROPS";
var g_sRetailLabInvokerScript = g_sRetailLabPublishRoot + "\\SCRIPTS\\retailLabInvoker.wsf";
var g_sPerfLabPrivateDropRoot = g_sRetailLabStoreRoot;
var g_sPerfLabInvokerScript = "\\\\PERFDADDY\\AUTOMATION\\SCRIPT\\privateBuildQueue.wsf";
var g_oRetailLabSemaphores = { PreviousJobsCompleteSemaphore : "PreviousJobStates.semaphore", JobCompleteSemaphore : "JobState.semaphore", FreshOptimizationDataSemaphore : "OptFresh.semaphore"};

/****************************************************************************/
/* RETAIL LAB ENUMS */
/****************************************************************************/
var MachineStates = { Idle: "IDLE", Processing: "PROCESSING" };
var MachineModes = { Available : "AVAILABLE", Maintenance : "MAINTENANCE" };
var JobResults = { CompletedWithSuccess : "Completed", CompletedWithFailure : "Failed", CompletedWithWarnings : "Warnings", Incomplete : "Incomplete" };
var JobStates = { Queued : "QUEUED", Acquired : "ACQUIRED", BuildingUnopt : "BUILDING_UNOPT", Installing : "INSTALLING", Optimizing : "OPTIMIZING", BuildingOpt : "BUILDING_OPT", Publishing : "PUBLISHING", Complete : "COMPLETE", RetailComplete : "RETAILCOMPLETE" };
var g_oTaskStates = { Success : "Completed", Failure : "Failed", NotStarted : "Not started", Running : "Running", Aborted : "Aborted", Incomplete : "Incomplete" };

/****************************************************************************/
/* RETAIL LAB GLOBAL VARIABLES */
/****************************************************************************/
var g_oJobBeginTime = undefined;
var g_oJobEndTime = undefined;
var g_oTaskObject = { 
                        Initialize : { TaskName : "Initialize", Summary : "Cleanup and initialization", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabInitialize.0.output.log" }, 
                        BuildCopy : { TaskName : "BuildCopy", Summary : "Input build copy", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabBuildCopy.0.output.log" }, 
                        Enlist : { TaskName : "Enlist", Summary : "Branch Enlistment", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabEnlist.0.output.log" }, 
                        SyncAndApplyChanges : { TaskName : "SyncAndApplyChanges", Summary : "Syncing and applying changes", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabSyncAndApplyChanges.0.output.log" }, 
                        CreateAndInstallUnoptLayout : { TaskName : "CreateAndInstallUnoptLayout", Summary : "Create layout of input drop and its installation", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabCreateAndInstallUnoptLayout.0.output.log" }, 
                        Optimize : { TaskName : "Optimize", Summary : "Retail optimization", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabOptimizeLayout.0.output.log" },
                        CreateOptLayout : { TaskName : "CreateOptLayout", Summary : "Create layout of output drop", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabCreateOptLayout.0.output.log" },
                        Publish : { TaskName : "Publish", Summary : "Publish result", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabPublish.0.output.log" },
                        Finalize : { TaskName : "Finalize", Summary : "Job finalization", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabFinalize.0.output.log" }
                    };

var g_oOptionalTaskObject = {
                                    PerfLabSubmit : { TaskName : "PerfLabSubmit", Summary : "Perf runs in the CLR perf lab", Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabFinalize.0.output.log" }
                            };

/****************************************************************************/
/* RETAIL LAB USER INTERFACE/INTERACTION FUNCTIONALITY */
/****************************************************************************/

/****************************************************************************/
/*
    Function to show the available scriptable retail lab functionality

    Arguments:
    None
  */    
function showPerfOptimizationFunctionality() {
    logCall(g_oRetailLabLogger, LogInfo10, "showPerfOptimizationFunctionality", arguments, "{");

    logMsg(g_oRetailLabLogger, LogInfo, "\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "==========================================================\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "The following retail lab functionality is available today:\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "==========================================================\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "Submit a Job: runjs submitPerfOptimizationJob\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "Delete a Job: runjs deletePerfOptimizationJob\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "Job Status: showPerfOptimizationsJobs\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "\r\n");    
    logMsg(g_oRetailLabLogger, LogInfo, "\r\n");    

    logMsg(g_oRetailLabLogger, LogInfo, "} showPerfOptimizationFunctionality\r\n");    
}

/****************************************************************************/
/*
    Function to submit a perf optimization job to the retail lab

    Arguments:
    sRawDrops: Location of the raw drops. It must specified in this format
        <arch>=<path to arch binaries>[;<arch>=<path to arch binaries>]
    sTitle: Title for the job (Default: Retail lab job for 'USERNAME')
    sBranch: The name of the branch (Default: $/DevDiv/pu/CLR)
        options: "$/DevDiv/pu/CLR"
    sSubmitToPerfLab: Submit the retail lab result to the perf lab with default parameters (Default: FALSE)
    sJobType: Type of the job (Default: CLR Private)
        options: "SNAP", "CLR Private"
    sOSType: The OS this job needs to run on (Default: ANY)
        options: "WIN2K8R2", "WIN2K8", "ANY"
                 Note that these values might change and you must check
                 for the latest values on the retail lab web page (http://clrsnap0 -> Retail Lab)
    sAdditionalOwners: Additional owners of this job (Default: 'USERNAME')
    sSyncpoint: Changeset to sync to before running this job (Default: current state of the tree)
    sShelveset: Shelveset to apply before running this job. Note that only changes in setup files, OptimizationData folder and re-base scripts will be applied
        The retail lab does not enlist into the sources and hence ignores changes from other parts of the tree.
        If shelveset is specified, then the owner of the shelveset must also be specified
 */
function submitPerfOptimizationJob(sRawDrops, sTitle, sBranch, sSubmitToPerfLab, sRemoteStore, sAdditionalOwners, sJobType, sOSType, sSyncpoint, sShelveset) {
    logCall(g_oRetailLabLogger, LogInfo10, "submitPerfOptimizationJob", arguments, "{");

    if (sRawDrops == undefined)
        throw new Error("Raw drops must be specified\r\n");
    if (sTitle == undefined)
        sTitle = "_";
    else
        sTitle = "\"" + sTitle + "\"";
    if (sJobType == undefined)
        sJobType = "_";
    else
        sJobType = "\"" + sJobType + "\"";
    if (sBranch == undefined)
        sBranch = "$/DevDiv/pu/CLR";
    if (sOSType == undefined)
        sOSType = "_";
    if (sAdditionalOwners == undefined)
        sAdditionalOwners = Env("USERNAME");
    if(sShelveset != undefined && sShelveset.split("\\").length < 2)
        throw new Error("If shelveset is specified, the owner of the shelveset must also be specified\r\n");
    if (sShelveset == undefined)
        sShelveset = "_";
    if (sSyncpoint == undefined)
        sSyncpoint = "_";
    if (sRemoteStore == undefined)
        sRemoteStore = "_";
    if (sSubmitToPerfLab == undefined)
        sSubmitToPerfLab = "FALSE";
    else
        sSubmitToPerfLab = String(sSubmitToPerfLab).toUpperCase();

    var sCommand = "cscript //nologo " + g_sRetailLabInvokerScript + " /USER /ADDJOB"
                                             + " " + sTitle
                                             + " " + sRawDrops
                                             + " " + sRemoteStore
                                             + " " + sAdditionalOwners
                                             + " " + sJobType
                                             + " " + sOSType
                                             + " " + sBranch
                                             + " " + sSyncpoint
                                             + " " + sShelveset
                                             + " " + sSubmitToPerfLab;
    runCmdToLog(sCommand,runSetTimeout(10*MINUTE));

    logMsg(g_oRetailLabLogger, LogInfo, "} submitPerfOptimizationJob\r\n");    
}

/****************************************************************************/
/*
    Function to delete a perf optimization job from the retail lab

    Arguments:
    sJobId: Job to be deleted. 
    If this user is not one of the owners of the job, this will be a no-op.
    Only when the job is in 'QUEUED' state can it be deleted
 */
function deletePerfOptimizationJob(sJobId) {
    logCall(g_oRetailLabLogger, LogInfo10, "deletePerfOptimizationJob", arguments, "{");

    if (sJobId == undefined)
        throw new Error(1, "Required field 'sJobId' must be specified to delete a job\r\n");

    var sCurrentUser = String(Env("USERNAME")).toUpperCase();
    var sQuery = "SELECT * FROM " + g_sRetailLabJobTable + " WHERE CONVERT(VARCHAR(100), Job_Id)=CONVERT(VARCHAR(100), '" + sJobId + "')";
    var oSqlResult = _executeDatabaseQuery(sQuery);
    while (!oSqlResult.EOF) {
        var sOwners = String(oSqlResult("Job_Owner")).toUpperCase();
        var sShelvesetWithOwner = String(oSqlResult("Job_ShelvesetWithOwner")).toUpperCase();
        if (sOwners == "NULL") {
            sOwners = "";
        }
        if (sShelvesetWithOwner == "NULL") {
            sShelvesetWithOwner = "";
        }
        if (sShelvesetWithOwner.indexOf(sCurrentUser) <0 && sOwners.indexOf(sCurrentUser) < 0) {
            logMsg(g_oRetailLabLogger,LogInfo, "Cannot delete job because either the job doesn't have an owner associated with it\r\n or the current user is not one of its owners...\r\n");
            return;
        }
        
        // there should be only 1 job with a job id, but if there are multiple
        // we ignore the rest
        break;
    }
    
    var sCommand = "cscript //nologo " + g_sRetailLabInvokerScript + " /USER /DELETEJOB"
                                             + " " + sJobId;
    runCmdToLog(sCommand,runSetTimeout(10*MINUTE));

    logMsg(g_oRetailLabLogger, LogInfo, "} deletePerfOptimizationJob\r\n");    
}

/****************************************************************************/
/*
    Function to view perf optimization jobs from the retail lab

    Arguments:
    None
 */
function showPerfOptimizationsJobs() {
    logCall(g_oRetailLabLogger, LogInfo10, "showPerfOptimizationsJobs", arguments, "{");
    var sCommand = "cscript //nologo " + g_sRetailLabInvokerScript + " /USER /VIEWJOBS";
    runCmdToLog(sCommand,runSetTimeout(20*MINUTE));

    logMsg(g_oRetailLabLogger, LogInfo, "} showPerfOptimizationsJobs\r\n");    
}

/****************************************************************************/
/* RETAIL LAB TASK FUNCTIONALITY */
/****************************************************************************/


/****************************************************************************/
/*
    Entry point function for treating the retail lab job as a runjs task

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function retailLabOptimizeTask(sJobId, dependents)
{
    if (dependents == undefined) {
        dependents = [
                        _retailLabInitializeTask(sJobId, dependents),
                        _retailLabCreateOptLayoutTask(sJobId, dependents),
                        _retailLabPublishTask(sJobId, dependents),
                        _retailLabFinalizeTask(sJobId, dependents)
                     ];
    }

    var ret = taskGroup("retailLabOptimize", 
                        dependents,
                        "Run the task to generate optimized layout builds for easy and reliable consumption",
                        "http://clrret0/lab");

    var sSharePath = _getJobParameter(sJobId, "Job_Destination");
    var sVblRoot = srcBaseFromScript();
    var sLocalLogPath = _toLocalLogsFolder(sVblRoot);
    if(FSOFolderExists(sLocalLogPath)) {
        FSODeleteFolder(sLocalLogPath, true);
    }
    
    var exitCode = tasksRun([ret], sLocalLogPath, sVblRoot);

    // Retail optimization failed somewhere, set the sub-tasks status'es appropriately
    if (exitCode != 0) { 
        _setJobParameter(sJobId, "Job_Result", JobResults.CompletedWithFailure);
        _correctTaskStatus(sJobId, "LabOptimize");
        _publishRetailJobLogs(sJobId);
    } else {
        _setJobParameter(sJobId, "Job_Result", JobResults.CompletedWithSuccess);
    }

    // Release the machine if this hasn't happened for some reason
    _setMachineParameter(MACHINE_NAME, "Machine_State", MachineStates.Idle);    
    
    return exitCode;
}

/****************************************************************************/
/*
    Task entry point to do a retail lab job's initialization

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabInitializeTask(sJobId, dependents)
{
    if (dependents == undefined) {        
        dependents = [];
    }

    var ret = taskNew("retailLabInitialize" , 
                      ScriptDir + "\\runjs _retailLabInitialize " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to initialize the retail lab job acquired by the worker machine",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to copy the input build for the retail lab job

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabBuildCopyTask(sJobId, dependents)
{
    if (dependents == undefined) {        
        dependents = [];
    }

    var ret = taskNew("retailLabBuildCopy" , 
                      ScriptDir + "\\runjs _retailLabBuildCopy " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to copy the input build",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to enlist into a branch for processing the retail lab input

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabEnlistTask(sJobId, dependents)
{
    if (dependents == undefined) {        
        dependents = [];
    }
    var ret = taskNew("retailLabEnlist", 
                      ScriptDir + "\\runjs _retailLabEnlist " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to enlist into a branch if there is no registered enlistment available",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to sync and apply the changes from the retail lab job

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task 
  */
function _retailLabSyncAndApplyChangesTask(sJobId, dependents)
{
    if (dependents == undefined) {        
        dependents = [];
    }
    var ret = taskNew("retailLabSyncAndApplyChanges", 
                      ScriptDir + "\\runjs _retailLabSyncAndApply " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to sync and apply user changes",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to create an (unopt) layout out of the input build and install it

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabCreateAndInstallUnoptLayoutTask(sJobId, dependents)
{
    if (dependents == undefined) {        
        dependents = [
                        _retailLabBuildCopyTask(sJobId, dependents),
                        _retailLabEnlistTask(sJobId, dependents),
                        _retailLabSyncAndApplyChangesTask(sJobId, dependents)
                     ];
    }
    var ret = taskNew("retailLabCreateAndInstallUnoptLayout" , 
                      ScriptDir + "\\runjs _retailLabCreateAndInstallUnoptLayout " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to create a layout build out of the input drop and install it with its pdb's",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to optimize the retail lab input build

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabOptimizeLayoutTask(sJobId, dependents)
{
    if (dependents == undefined) {        
        dependents = [
                        _retailLabCreateAndInstallUnoptLayoutTask(sJobId, dependents)
                     ];
    }
    var ret = taskNew("retailLabOptimizeLayout", 
                      ScriptDir + "\\runjs _retailLabOptimizeLayout " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to optimize a layout build",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to create the optimized output build

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabCreateOptLayoutTask(sJobId, dependents)
{
    if (dependents == undefined) {
        dependents = [
                        _retailLabOptimizeLayoutTask(sJobId, dependents)
                     ];
    }
    var ret = taskNew("retailLabCreateOptLayout", 
                      ScriptDir + "\\runjs _retailLabCreateOptLayout " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to create a layout build out of the optimized bits",
                      "http://clrret0/lab");
    return ret;
}

/****************************************************************************/
/*
    Task entry point to publish the retail lab output

    Arguments:
    sJobId: Job ID of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabPublishTask(sJobId, dependents)
{
    if (dependents == undefined)
        dependents = [];
    var ret = taskNew("retailLabPublish" , 
                      ScriptDir + "\\runjs _retailLabPublish " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to publish the retail lab job",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/*
    Task entry point to perform finalization tasks for the retail lab job

    Arguments:
    sJobId: Job ID of the job being processed
    sVblRoot: VBL root of the job being processed
    dependents: array of dependents of this task
 */
function _retailLabFinalizeTask(sJobId, dependents)
{
    if (dependents == undefined)
        dependents = [];
    var ret = taskNew("retailLabFinalize", 
                      ScriptDir + "\\runjs _retailLabFinalize " + sJobId,
                      dependents, 
                      undefined, 
                      "This task is to run any additional and trivial commands before the retail lab job finishes",
                      "http://clrret0/lab");

    return ret;
}

/****************************************************************************/
/* RETAIL LAB CORE FUNCTIONALITY */
/****************************************************************************/

/****************************************************************************/
/*
    Function that acts as an entry point into runjs for the retail lab worker
    Except for the sync of the toolset/scripts, everthing else must be handled
    here.

    Arguments:
    None, everything is figured out by querying the retail lab database
 */
function retailLabOptimize()
{
    logCall(g_oRetailLabLogger, LogInfo10, "retailLabOptimize", arguments, "{");
    SetSystemWideEnvVar("NEWRETAILLAB", "1");

    do
    {
        // find a suitable job for this machine to process
        var sJobId = _findSuitableRetailLabJob(MACHINE_NAME);
    
        // If this machine is in maintenance mode or is already processing a job, we just keep on looping doing nothing
        var sSqlQuery = "SELECT * FROM " + g_sRetailLabMachineTable + " WHERE CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + MACHINE_NAME + "')";
        var oSqlResult = _executeDatabaseQuery(sSqlQuery);
        var sMachineMode = undefined;
        var sMachineState = "";
        if (!oSqlResult.EOF) {
            sMachineMode = String(oSqlResult("Machine_Mode"));
            sMachineState = String(oSqlResult("Machine_State"));
            if (_isNullDatabaseEntry(sMachineMode) || sMachineMode == MachineModes.Maintenance) {
                logMsg(g_oRetailLabLogger, LogInfo, "Unknown or non-runnable machine mode, sleeping for 10 minutes...\r\n");
                WScript.Sleep(10 * 60 * 1000);
                continue;
            } else if (_isNullDatabaseEntry(sMachineState) || sMachineState == MachineStates.Processing) {
                // if we have another job in this machine's queue and the last job failed for some reason
                // we should mark the machine state as idle and continue
                if (sJobId != INVALID_JOBID) {
                    var sLastJobId = String(_getMachineParameter(MACHINE_NAME, "Machine_Job"));                    
                    logMsg(g_oRetailLabLogger, LogInfo, "Looks like this machine abruptly stopped processing the last job (ID: " + sLastJobId + ").\r\n");
                    _sendRetailLabMail(sLastJobId);

                    logMsg(g_oRetailLabLogger, LogInfo, "Setting job state to " + JobStates.Complete + "\r\n");
                    _setJobParameter(sLastJobId, "Job_State", JobStates.Complete);

                    logMsg(g_oRetailLabLogger, LogInfo, "Resetting machine state to idle\r\n");
                    _setMachineParameter(MACHINE_NAME, "Machine_State", MachineStates.Idle);

                    // Decrement this machine's queue count
                    _decrementMachineQueueCount();
                    continue;
                } else {
                    logMsg(g_oRetailLabLogger, LogInfo, "Machine is in an unknown state or it is already processing a job, sleeping for 10 minutes...\r\n");
                    WScript.Sleep(10 * 60 * 1000);
                    continue;
                }
            }
        } else {
            logMsg(g_oRetailLabLogger, LogInfo, "Possibly not a registered retail lab worker machine, sleeping for 30 minutes...\r\n");
            WScript.Sleep(30 * 60 * 1000);
            continue;
        }

        // At this point, we have deduced that this machine is IDLE
        // Let's grab our job to actually process it
        if (sJobId != INVALID_JOBID) {
            g_oJobBeginTime = new Date();

            var sJobType = _getJobParameter(sJobId, "Job_Type");
            var oJobDuration = undefined;
            var sJobShelvesetWithOwner = _getJobParameter(sJobId, "Job_ShelvesetWithOwner");
            var sJobSyncPoint = _getJobParameter(sJobId, "Job_Syncpoint");
            var sJobBranch = _getJobParameter(sJobId, "Job_Branch");
            var sJobBranchFriendlyName = _getSupportedBranchParameter(sJobBranch, "Friendly_Name");
            var sJobClientName = _getJobParameter(sJobId, "Job_EnlistmentName");
            var sJobClientType = _getJobParameter(sJobId, "Job_EnlistmentType");
            var sJobVblRoot = srcBaseFromScript();

            // update job state in database
            _setJobParameter(sJobId, "Job_State", JobStates.Acquired);
            _setJobParameter(sJobId, "Start_Time", _toSqlTime(g_oJobBeginTime));
            _setJobParameter(sJobId, "Job_Result", JobResults.Incomplete);
            logMsg(g_oRetailLabLogger, LogInfo, "Acquired Job_Id: " + sJobId + " at " + g_oJobBeginTime + "\r\n");
            logMsg(g_oRetailLabLogger, LogInfo, "Processing job: " + sJobId + "\r\n");

            // update machine state in database
            _setMachineParameter(MACHINE_NAME, "Machine_State", MachineStates.Processing);
            logMsg(g_oRetailLabLogger, LogInfo, "Updated Machine state to Processing...\r\n");                

            // update machine job to mean that the machine is processing a particular job
            _setMachineParameter(MACHINE_NAME, "Machine_Job", sJobId);
            logMsg(g_oRetailLabLogger, LogInfo, "Updated Machine job to " + sJobId + "...\r\n");                

            // initializing the task info object in the database
            _setTaskInfoObject(sJobId);
            logMsg(g_oRetailLabLogger, LogInfo, "Initializing Task info object in the database for JobId: " + sJobId + "...\r\n");

            var sSharePath = _ensureFreeSpaceOnShares();
            if (sSharePath == undefined) {
                var sLocalSharePath = "C:\\local_share";
                if (!FSOFolderExists(sLocalSharePath)) {
                    FSOCreateFolder(sLocalSharePath);                    
                }
                sSharePath = sLocalSharePath + "\\" + sJobId;
                logMsg(g_oRetailLabLogger, LogInfo, "Remote retail lab store could not be cleaned. Using: " + sSharePath + "\r\n");
            } else {
                var sBranchFolder = sSharePath + "\\" + sJobBranchFriendlyName;
                if (!FSOFolderExists(sBranchFolder))
                    FSOCreateFolder(sBranchFolder);
                if (sJobType == "SNAP") {
                    sSharePath = sBranchFolder + "\\" + sJobSyncPoint;
                } else {
                    sSharePath = sBranchFolder + "\\" + _toPerfJobId(sJobId);
                }
            }
            
            // set job destination path in database
            _setJobParameter(sJobId, "Job_Destination", sSharePath);
            logMsg(g_oRetailLabLogger, LogInfo, "Set job's output directory to: " + sSharePath + "\r\n");

            // find the enlistment which matches the job_branch
            // this could be an enlistment from the branch_table or the temporary branch table
            var oEnlistmentParameters = _getEnlistmentOnMachine(sJobBranch, MACHINE_NAME);
            if (oEnlistmentParameters.EnlistmentPresence == true)
            {
                logMsg(g_oRetailLabLogger, LogInfo, "An enlistment exists for " + sJobBranch + " branch...\r\n");
                sJobVblRoot = srcBaseFromScript();
            }
            else
            {
                logMsg(g_oRetailLabLogger, LogInfo, "No enlistment was found for this job. Looks like an infrastructure issue. Please investigate..." + "\r\n");
                return;
            }            

            var oRunEnvironments = _getRunEnvironments(sJobVblRoot);                
            var tfJobBranchOpts = oRunEnvironments.BranchEnvironment;
            var sPathToTfCmdForJobBranch = sJobVblRoot + "\\tools\\x86\\managed\\v4.0\\tf.cmd ";
            var sPathToTfptCmdForJobBranch = sJobVblRoot + "\\tools\\x86\\managed\\v4.0\\tfpt.cmd ";

            // check if a perf run was requested, add a sub-task for it if yes
            var sPerfLabCommandFlagValue = _getJobFlag(sJobId, "PERFLABCOMMAND");
            if (sPerfLabCommandFlagValue != null && sPerfLabCommandFlagValue.toUpperCase() == "TRUE") {
                _setJobParameter(sJobId, "Job_Perf_Id", _toPerfJobId(sJobId));
                _logTaskStatus(sJobId, "PerfLabSubmit", g_oTaskStates.NotStarted);
            }

            // kick-off all the sub-tasks through the task procedure
            retailLabOptimizeTask(sJobId);

            // By this time, all the tasks would have completed and the state and the result 
            // of the job properly set, to be able to copy the logs, generate the right html report
            // and send a mail

            // Move logs to their final location
            _publishRetailJobLogs(sJobId);

            // Generate html report for the job
            logMsg(g_oRetailLabLogger, LogInfo, "Generating html report for the job...\r\n");
            _generateRetailLabJobReport(sJobId);

            // Send mail to the owners
            logMsg(g_oRetailLabLogger, LogInfo, "Sending mails...\r\n");
            _sendRetailLabMail(sJobId); 

            // Scorching the enlistment is not needed for temporary enlistments, but we will add that change in the next set of changes
            // Errors while scorching the enlistment must not fail the job
            // we do this after sending an email to the job owners
            try {
                // scorch the enlistment so that future jobs do not have a problem with any changes in this job
                var oRunEnvironments = _getRunEnvironments(sJobVblRoot);                
                var tempBatchFile = FSOGetTempPath("runjsTempBatch-") + ".bat";
                var tempBatchFileContents = "call " + sPathToTfCmdForJobBranch + " undo " + sJobVblRoot + " /recursive /noprompt" + " & " +
                                            "call " + sPathToTfptCmdForJobBranch + " scorch " + sJobVblRoot + " /recursive /noprompt";
                FSOWriteToFile(tempBatchFileContents, tempBatchFile);
                runCmdToLog("pushd " + sJobVblRoot + " & " +
                            "call " + sJobVblRoot + "\\tools\\razzle.cmd x86 ret" + " exec " +
                            "call " + tempBatchFile, tfJobBranchOpts);
            } catch(e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error while scorching the enlistment...\r\n");
            }

            // Publish logs again to make sure we have the html report generation
            // and the mail sending part in the logs
            _publishRetailJobLogs(sJobId);
            break;
        } else {                
            // If there are no jobs for processing and we didn't process any job, let us not shutdown unnecessarily
            if (!_isNullDatabaseEntry(sMachineState) && sMachineState == MachineStates.Idle) {
                logMsg(g_oRetailLabLogger, LogInfo, "Machine is idle but does not have any job for processing, sleeping for 10 minutes...\r\n");
                WScript.Sleep(10 * 60 * 1000);
                continue;
            }
        }
    }while(true);

    logMsg(g_oRetailLabLogger, LogInfo, "} retailLabOptimize\r\n");
}

/****************************************************************************/
/*
    Function to do some initialization when a retail lab job starts executing on the 
    worker machine

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabInitialize(sJobId)
{
    _correctTaskStatus(sJobId, "Initialize");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "Initialize", g_oTaskStates.Running);

    if (sJobId == undefined) {
        _logTaskStatus(sJobId, "Initialize", g_oTaskStates.Failure);
        throw new Error(1, "JobId needed for job initialization\r\n");
    }        
    var sVblRoot = srcBaseFromScript();
    var sJobType = _getJobParameter(sJobId, "Job_Type");
    var sJobSyncPoint = _getJobParameter(sJobId, "Job_Syncpoint");
    var sJobBranch = _getJobParameter(sJobId, "Job_Branch");
    var sJobClientName = _getJobParameter(sJobId, "Job_EnlistmentName");
    var sJobClientType = _getJobParameter(sJobId, "Job_EnlistmentType");
    var sJobBranchFriendlyName = _getSupportedBranchParameter(sJobBranch, "Friendly_Name");
    var sSharePath = _getJobParameter(sJobId, "Job_Destination");
    var sZipPath = _toJobZipPath(sSharePath);
    var sLogPath = _toJobLogPath(sSharePath);
    var oRunEnvironments = _getRunEnvironments(sVblRoot);                
    var tfLocalBranchOpts = oRunEnvironments.BranchEnvironment;

    // update job paths in database
    _setJobParameter(sJobId, "Job_Destination", sSharePath);
    _setJobParameter(sJobId, "Job_Visible_Destination", sSharePath);
    _setJobParameter(sJobId, "Job_Zips", sZipPath);
    _setJobParameter(sJobId, "Job_Logs", sLogPath);

    if (!FSOFolderExists(sSharePath))
        FSOCreateFolder(sSharePath);
    if (!FSOFolderExists(sZipPath))
        FSOCreateFolder(sZipPath);

    // Save logs locally and copy them to the actual destination folder
    // at various times during the run
    var sLocalLogPath = _toLocalLogsFolder(sVblRoot);
    logMsg(g_oRetailLabLogger, LogInfo, "Created output drop location...\r\n");

    // Enable client OS themes
    try {
        logMsg(g_oRetailLabLogger, LogInfo, "Enabling Client OS themes...\r\n");
        runCmdToLog(sVblRoot + "\\OptimizationData\\scripts\\TurnOnThemesWin2003.bat", runSetNoThrow());
    } catch (e) {
        logMsg(g_oRetailLabLogger, LogInfo, "Error in enabling themes: " + e.description + "\r\n");
    }

    // turn off screen saver since retail build scenarios change if the desktop is not active
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF screen saver since retail build scenarios change if the desktop is not active...\r\n");
    runCmdToLog("reg add \"HKCU\\Control Panel\\Desktop\" /v \"ScreenSaveActive\" /t REG_SZ /d \"0\" /f");

    // allow WSF files to be run from the command line 
    logMsg(g_oRetailLabLogger, LogInfo, "Allowing .wsf files to be run from the command line...\r\n");
    runCmdToLog("cscript //H:CSCRIPT");

    // This runs managed code on server machines turn it off.
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF msdtc service...\r\n");
    try {
        runCmdToLog("net stop msdtc", runSetNoThrow());
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }

    // turn off reboot reason box on bootup
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF Reboot Reason Box...\r\n");
    try {
        runCmdToLog("sc config ersvc start= disabled");
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }

    // turn off firewall for XP SP2+ and 2k3 SP1 b 1289+
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF firewall...\r\n");
    try {
        runCmdToLog("sc stop SharedAccess", runSetNoThrow());
        runCmdToLog("sc config SharedAccess start= disabled", runSetNoThrow());
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }

    // shutdown ETrust
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF etrust...\r\n");
    try {
        runCmdToLog("net stop InoRT", runSetNoThrow());
        runCmdToLog("net stop InoTask", runSetNoThrow());
        runCmdToLog("net stop InoRpc", runSetNoThrow());
        runCmdToLog("kill Realmon.exe", runSetNoThrow());
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }

    //shutdown Microsoft forefront
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF Microsoft Forefront...\r\n");
    try {
        runCmdToLog("net stop FSysAgent", runSetNoThrow());
        runCmdToLog("net stop FCSAM", runSetNoThrow());    
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }

    // turn off windows update
    // this sometimes gives problem during .NET installation
    logMsg(g_oRetailLabLogger, LogInfo, "Turning OFF Windows Update...\r\n");
    try {
        runCmdToLog("net stop wuauserv", runSetNoThrow());
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();    
    _logTaskStatus(sJobId, "Initialize", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to perform the build copy step in the retail optimization task of the retail lab

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabBuildCopy(sJobId)
{
    _correctTaskStatus(sJobId, "BuildCopy");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "BuildCopy", g_oTaskStates.Running);

    if (sJobId ==undefined) {
        _logTaskStatus(sJobId, "BuildCopy", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to copy the build locally\r\n");
    }
    var sVblRoot = srcBaseFromScript();
    var rawBuildsRoot = _toLocalBinariesFolder(sVblRoot);
    if (FSOFolderExists(rawBuildsRoot))
    {
        logMsg(g_oRetailLabLogger, LogInfo, "Deleting local binaries folder: " + rawBuildsRoot + "\r\n");
        FSODeleteFolder(rawBuildsRoot, true);
    }

    // copy the raw bits locally because thats where setup needs the files
    var rawBuilds = _getJobParameter(sJobId, "Raw_Build");
    if (_isNullDatabaseEntry(rawBuilds)) {
        _logTaskStatus(sJobId, "BuildCopy", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("rawBuilds information not found in the database\r\n");
    }

    var buildType = _getJobParameter(sJobId, "Build_Type");
    var buildArchs = _getJobParameter(sJobId, "Job_BuildArchs");
    var buildInstallRoots = _getJobParameter(sJobId, "Job_BuildInstallRoots");

    var remoteBuilds = rawBuilds.split(";");
    for (var i=0; remoteBuilds != undefined && remoteBuilds != null && i<remoteBuilds.length; i++)
    {
        g_oTaskObject.BuildCopy.Details = g_oTaskObject.BuildCopy.Details + buildArchs[i] + ",";

        var srcPath = remoteBuilds[i].split("=")[1];
        var destPath = rawBuildsRoot + "\\" + buildArchs[i] + buildType;
        logMsg(g_oRetailLabLogger, LogInfo, "Copying raw drop from " + srcPath + " to " + destPath + "\r\n");

        // For SNAP jobs, we copy DesktopVersion.h file to the output location
        // so that the perf lab can find out when the job was processed by SNAP
        if (_getJobParameter(sJobId, "Job_Type") == "SNAP") {
            var sSrcDesktopVersion = srcPath + "\\..\\..\\DesktopVersion.h";
            var sDstDesktopVersion = _getJobParameter(sJobId, "Job_Destination") + "\\DesktopVersion.h";
            if (FSOFileExists(sSrcDesktopVersion)) {
                if (!FSOFileExists(sDstDesktopVersion)) {
                    logMsg(g_oRetailLabLogger, LogInfo, "Copying DesktopVersion.h file from " + sSrcDesktopVersion + " to " + sDstDesktopVersion + "\r\n");
                    FSOCopyFile(sSrcDesktopVersion, sDstDesktopVersion, true);
                }
            } else {
                logMsg(g_oRetailLabLogger, LogInfo, "DesktopVersion.h file not found in " + sSrcDesktopVersion + ". Continuing, as this does not affect retail optimization in any way\r\n");
            }
        }

        //var sLogDirectory = _getJobParameter(sJobId, "Job_Logs");
        var sLocalLogPath = _toLocalLogsFolder(sVblRoot);
        var sLogFilePath = sLocalLogPath + "\\" + "rawDropCopy." + buildArchs[i] + buildType + ".log";

        // If the input drop is not available at this time, wait for some time.
        // It could be a network problem because of which we don't want this job to fail
        // robocopy can deal with network problems, but it cannot handle the case
        // when the input drop is not available when robocopy started
        var nReTries = 0;
        while(!FSOFolderExists(srcPath)) {
            logMsg(g_oRetailLabLogger, LogInfo, "Input drop not available at: " + srcPath + ". Sleeping for 5 minutes...\r\n");
            WScript.Sleep(5 * 60);
            if((++nReTries % 3) == 0) {
                logMsg(g_oRetailLabLogger, LogInfo, "The input drop is not accessible to the retail lab. Please check if it is accessible to REDMOND\\clrgnt user...\r\n");
                break;
            }
        }
        robocopy(srcPath, destPath, "/E", sLogFilePath, true, true);

        if(!FSOFileExists(destPath + "\\clr.dll")) {
            _logTaskStatus(sJobId, "BuildCopy", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
            throw new Error("Input drop didn't have an accessible clr.dll. Nothing to optimize...\r\n");
        }

        // Augment the build with any extra unopt files from the remote store
        var remoteStore = _getJobParameter(sJobId, "Remote_Store");

        logMsg(g_LayoutLogObject, LogInfo, "Initial remote store: " + remoteStore + "\r\n");

        if (remoteStore == undefined || remoteStore == null || String(remoteStore).toUpperCase() == "NULL" || String(remoteStore) == "")
        {
            var sPathToClrsetupInfo = destPath + "\\clrsetupinfo.bat";
            // If clrsetupinfo.bat not found in the binaries folder corresponding to the processor architecture, we look for that file for the x86 architecture
            // This is required when we are optimizing a runtime from a non-WOW window on a machine with AMD64 or IA64 based hardware/OS
            var sAlternatePathToClrsetupInfo = rawBuildsRoot + "\\" + "x86" + buildType + "\\clrsetupinfo.bat";

            logMsg(g_LayoutLogObject, LogInfo, "sPathToClrsetupInfo: " + sPathToClrsetupInfo + "\r\n");
            if (!FSOFileExists(sPathToClrsetupInfo)) {
                logMsg(g_LayoutLogObject, LogInfo, "sAlternatePathToClrsetupInfo: " + sAlternatePathToClrsetupInfo + "\r\n");
                sPathToClrsetupInfo = sAlternatePathToClrsetupInfo;
                if (!FSOFileExists(sPathToClrsetupInfo)) {
                    _logTaskStatus(sJobId, "BuildCopy", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
                    throw new Error(1, "Either remoteStore must be specified or a clrsetupinfo.bat file must be present in the drop\r\n");
                }
            }
            logMsg(g_LayoutLogObject, LogInfo, "Will be extracting the location of the remoteStore from clrsetupinfo.bat\r\n");
            var setupInfo = parseBatchFileForSetStatements(sPathToClrsetupInfo);
            remoteStore = setupInfo["CLR_DROP_PATH_HEAD"] + "\\" + setupInfo["CLR_CERTIFIED_VERSION"];
        }

        var remoteUnoptPath = remoteStore + "\\binaries." + buildArchs[i] + buildType + "\\unopt";
        var localUnoptPath = destPath + "\\unopt";

        var sUnoptLogFilePath = sLocalLogPath + "\\" + "rawDropRemoteUnoptCopy." + buildArchs[i] + buildType + ".log";

        // Tell robocopy not to copy anything that already exists
        robocopy(remoteUnoptPath, localUnoptPath, "/XN /XO /XC", sUnoptLogFilePath);        
    }

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "BuildCopy", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to enlist into the branch for which the job is being run (if required)

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabEnlist(sJobId)
{
    _correctTaskStatus(sJobId, "Enlist");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "Enlist", g_oTaskStates.Running);

    if (sJobId ==undefined) {
        _logTaskStatus(sJobId, "Enlist", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to enlist for it\r\n");
    }
    g_oTaskObject.Enlist.Details = "No enlisting needed, an enlistment for this branch already exists";

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "Enlist", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to sync and apply changes to the enlistment for which the job is being run

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabSyncAndApply(sJobId)
{
    _correctTaskStatus(sJobId, "SyncAndApplyChanges");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "SyncAndApplyChanges", g_oTaskStates.Running);

    if (sJobId ==undefined) {
        _logTaskStatus(sJobId, "SyncAndApplyChanges", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to sync and apply the shelveset\r\n");
    }
    var sVblRoot = srcBaseFromScript();
    var sPatchShelveset = _getSupportedBranchParameter(_getJobParameter(sJobId, "Job_Branch"), "Branch_Patch_Shelveset");
    if(!_isNullDatabaseEntry(sPatchShelveset)) {
        logMsg(g_oRetailLabLogger, LogInfo, "This job has been patched with shelveset: " + sPatchShelveset + " for the retail lab to operate correctly...\r\nIf you have any questions about this, please contact clrrlnot alias\r\n");
    }

    var oRunEnvironments = _getRunEnvironments(sVblRoot);                
    var tfLocalBranchOpts = oRunEnvironments.BranchEnvironment;
    var sPathToTfCmdForEnlistedBranch = sVblRoot + "\\tools\\x86\\managed\\v4.0\\tf.cmd ";
    var sPathToTfptCmdForEnlistedBranch = sVblRoot + "\\tools\\x86\\managed\\v4.0\\tfpt.cmd ";
    var sPathToTfScriptCmdForEnlistedBranch = sVblRoot + "\\tools\\x86\\managed\\v4.0\\tfscript.cmd ";

    var tempBatchFile = undefined;
    var tempBatchFileContents = undefined;
    var sJobSyncPoint = _getJobParameter(sJobId, "Job_Syncpoint");
    var sJobShelvesetWithOwner = _getJobParameter(sJobId, "Job_ShelvesetWithOwner");

    // update the syncpoint to the latest for future use such as submission to the perf lab
    // because that's the syncpoint we've used
    if (_isNullDatabaseEntry(sJobSyncPoint)) {
        sJobSyncPoint = tfWhereSynced(sVblRoot).minChange;        
        if (!_isNullDatabaseEntry(sJobSyncPoint)) {        
            _setJobParameter(sJobId, "Job_Syncpoint", sJobSyncPoint);
            logMsg(g_oRetailLabLogger, LogInfo, "Updated syncpoint to the latest: " + sJobSyncPoint + "\r\n");
        } else {
            logMsg(g_oRetailLabLogger, LogInfo, "Could not determine the latest syncpoint...\r\n");
        }
    }

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "SyncAndApplyChanges", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to create a layout build out of the input drops (using the unopt bits)
    and install it so that the optimization can begin after that

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabCreateAndInstallUnoptLayout(sJobId)
{
    _correctTaskStatus(sJobId, "CreateAndInstallUnoptLayout");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "CreateAndInstallUnoptLayout", g_oTaskStates.Running);

    if (sJobId == undefined) {
        _logTaskStatus(sJobId, "CreateAndInstallUnoptLayout", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to create a layout build\r\n");
    }
    var sVblRoot = srcBaseFromScript();
    var buildType = _getJobParameter(sJobId, "Build_Type");
    var remoteStore = _getJobParameter(sJobId, "Remote_Store");
    var sSharePath = _getJobParameter(sJobId, "Job_Destination");
    var sLogDirectory = _getJobParameter(sJobId, "Job_Logs");
    var sLocalLogPath = _toLocalLogsFolder(sVblRoot);

    // call functionality to create unopt layout build from raw bits
    _setJobParameter(sJobId, "Job_State", JobStates.BuildingUnopt);
    logMsg(g_oRetailLabLogger, LogInfo, "BuildingUnopt Job_Id: " + sJobId + "\r\n");
    createLayoutFromRaw(sVblRoot, remoteStore, buildType);
    logMsg(g_oRetailLabLogger, LogInfo, "Copying layout build generated from input drop to output drop location. This will take some time...\r\n");
    var sLogFilePath = sLocalLogPath + "\\" + "unoptDropCopy." + buildType + ".log";
    robocopy(_toLocalSrcSetupFolder(sVblRoot), sSharePath + "\\unopt", "/E /XD binaries.ia64ret /XD ia64ret /XD localized /XD setup /XD signed /XD sources /XD wix", sLogFilePath, true, true);

    // we need to run this for installing dogfood builds
    logMsg(g_oRetailLabLogger, LogInfo, "Installing preinstall.cmd since this build is test signed\r\n");
    try {
        runCmdToLog("\\\\ddrelqa\\preinstall\\preinstall.cmd", runSet32Bit());
    } catch(e) {
        // if we fail, we just continue because all the retail lab worker machines have installed this tool
        // at some point of time before. So we assume that things will just succeed
        logMsg(g_oRetailLabLogger, LogInfo, "Preinstall.cmd failed somewhere: " + e.description + "\r\n");
    }

    // install layout build on the worker
    _setJobParameter(sJobId, "Job_State", JobStates.Installing);    
    logMsg(g_oRetailLabLogger, LogInfo, "Installing Job_Id: " + sJobId + "\r\n");
    installLayout(sVblRoot, _toLocalLayoutFolder(sVblRoot), remoteStore, buildType);    

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "CreateAndInstallUnoptLayout", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to optimize the installed unopt layout build

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabOptimizeLayout(sJobId)
{
    _correctTaskStatus(sJobId, "Optimize");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "Optimize", g_oTaskStates.Running);

    if (sJobId ==undefined) {
        _logTaskStatus(sJobId, "Optimize", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to optimize a layout build\r\n");
    }
    var sVblRoot = srcBaseFromScript();
    var buildArchs = _getJobParameter(sJobId, "Job_BuildArchs");
    var buildInstallRoots = _getJobParameter(sJobId, "Job_BuildInstallRoots");
    var remoteStore = _getJobParameter(sJobId, "Remote_Store");
    var buildType = _getJobParameter(sJobId, "Build_Type");
    var sZipPath = _getJobParameter(sJobId, "Job_Zips");

    // run scenarios through optprof to generate optimization data
    // apply optimization data to the unopt build to generate optimized build
    _setJobParameter(sJobId, "Job_State", JobStates.Optimizing);
    logMsg(g_oRetailLabLogger, LogInfo, "Optimizing Job_Id: " + sJobId + "\r\n");

    // optimize the runtime
    g_oTaskObject.Optimize.ErrorCode = g_oTaskStates.Success;
    var installParams = _calculateLayoutInstallParameters(sVblRoot, _toLocalLayoutFolder(sVblRoot), remoteStore, buildType);
    for (var k=0; k<buildArchs.length; k++)
    {    
        var sBackupUnoptDir = _toLocalBinariesBackupFolder(sVblRoot) + "\\" + buildArchs[k] + buildType;
        var sUnoptDir = _toLocalBinariesFolder(sVblRoot) + "\\" + buildArchs[k] + buildType;
        var sOptDir = _toLocalBinariesFolder(sVblRoot) + "\\" + buildArchs[k] + buildType + ".opt";
        var certDrops = undefined;
        try {
            if (!_isNullDatabaseEntry(remoteStore)) {
                certDrops = remoteStore + "\\binaries." + buildArchs[k] + buildType;
            }
            retailBuild(sUnoptDir, sOptDir, buildArchs[k], undefined, undefined, sVblRoot, undefined, undefined, undefined, buildType, buildInstallRoots[k] + "v4.0." + installParams.BuildNumber, certDrops);

            // modify the original raw drop to now contain the optimized drop
            // this is needed for generating the layout build
            logMsg(g_oRetailLabLogger, LogInfo, "Copying the optimized raw drop from " + sOptDir + " to " + sUnoptDir + "\r\n");
            FSODeleteFolder(sUnoptDir, true);
            FSOCopyFolder(sOptDir, sUnoptDir, true);

            var sArchZipDestPath = sZipPath + "\\" + buildArchs[k] + buildType;
            if (!FSOFolderExists(sArchZipDestPath)) {
                FSOCreatePath(sArchZipDestPath);                
            }

            // Treat the log file copy error as warning
            logMsg(g_oRetailLabLogger, LogInfo, "Copying the Optprof log files to the output directory\r\n");
            try {
                // Copy the IBC training Optprof log file
                FSOCopyFile(sOptDir + "\\Profile\\OptProf.log", _toLocalLogsFolder(sVblRoot) + "\\IBC.training.OptProf." + buildArchs[k] + ".log", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying IBC training Optprof log files: " + e.description + "\r\n");
            }
            try {
                // Copy the BBT training Optprof log file
                FSOCopyFile(sOptDir + "\\BbtProfile\\OptProf.log", _toLocalLogsFolder(sVblRoot) + "\\BBT.training.OptProf." + buildArchs[k] + ".log", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying BBT training Optprof log files: " + e.description + "\r\n");
            }            
            try {
                // Copy the BBT optimization Optprof log file
                FSOCopyFile(sOptDir + "\\BbtOptimize\\OptProf.log", _toLocalLogsFolder(sVblRoot) + "\\BBT.optimization.OptProf." + buildArchs[k] + ".log", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying BBT optimization Optprof log files: " + e.description + "\r\n");
            }            
            
            logMsg(g_oRetailLabLogger, LogInfo, "Copying the coffbase files to the output directory\r\n");
            try {
                // Copy the coffbase/coffsize files
                FSOCopyFile(sOptDir + "\\Coffbase\\coffbase.txt", _toLocalLogsFolder(sVblRoot) + "\\coffbase." + buildArchs[k] + ".txt", true);
                FSOCopyFile(sOptDir + "\\Coffbase\\coffsize.txt", _toLocalLogsFolder(sVblRoot) + "\\coffsize." + buildArchs[k] + ".txt", true);
            } catch(e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying coffbase/coffsize files to destination folder: " + e.description + "\r\n");
            }

            // Treat data files copy error as warning
            logMsg(g_oRetailLabLogger, LogInfo, "Copying the zip files to the output directory\r\n");
            try {
                // Copy the IBC data files
                FSOCreatePath(sArchZipDestPath + "\\IbcData");
                FSOCopyFolder(sOptDir + "\\Profile\\Work\\Data", sArchZipDestPath + "\\IbcData", true);                        
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying IBC data files to destination folder: " + e.description + "\r\n");
            }

            try {
                // Copy the BBT data files
                FSOCreatePath(sArchZipDestPath + "\\BbtData");
                FSOCopyFolder(sOptDir + "\\BbtData", sArchZipDestPath + "\\BbtData", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying BBT data files to destination folder: " + e.description + "\r\n");
            }

            // Move logs to their final location
            _publishRetailJobLogs(sJobId);
        } catch (e) {
            logMsg(g_oRetailLabLogger, LogInfo, "Failure while optimizing the " + buildArchs[k] + " runtime: " + e.description + "\r\n");
            _logTaskStatus(sJobId, "Optimize", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));

            // We could have done this in the finally clause, but since we throw in catch
            // we are doing it here
            // Treat the log file copy error as warning
            logMsg(g_oRetailLabLogger, LogInfo, "Copying the Optprof log files to the output directory\r\n");
            try {
                // Copy the IBC training Optprof log file
                FSOCopyFile(sOptDir + "\\Profile\\OptProf.log", _toLocalLogsFolder(sVblRoot) + "\\IBC.training.OptProf." + buildArchs[k] + ".log", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying IBC training Optprof log files: " + e.description + "\r\n");
            }
            try {
                // Copy the BBT training Optprof log file
                FSOCopyFile(sOptDir + "\\BbtProfile\\OptProf.log", _toLocalLogsFolder(sVblRoot) + "\\BBT.training.OptProf." + buildArchs[k] + ".log", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying BBT training Optprof log files: " + e.description + "\r\n");
            }            
            try {
                // Copy the BBT optimization Optprof log file
                FSOCopyFile(sOptDir + "\\BbtOptimize\\OptProf.log", _toLocalLogsFolder(sVblRoot) + "\\BBT.optimization.OptProf." + buildArchs[k] + ".log", true);
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying BBT optimization Optprof log files: " + e.description + "\r\n");
            }            
            try {
                // Copy any dumps
                // TODO: Verify the dmp locations
                if (FSOFolderExists(sOptDir + "\\BbtProfile\\bbtProfile.dmpRpt")) {
                    robocopy(sOptDir + "\\BbtProfile\\bbtProfile.dmpRpt", _toLocalLogsFolder(sVblRoot) + "\\dumps", undefined, undefined, undefined, true);
                }
                if (FSOFolderExists(sOptDir + "\\BbtOptimize\\bbtOptimize.dmpRpt")) {
                    robocopy(sOptDir + "\\BbtOptimize\\bbtOptimize.dmpRpt", _toLocalLogsFolder(sVblRoot) + "\\dumps", undefined, undefined, undefined, true);
                }
                if (FSOFolderExists(sOptDir + "\\IbcProfile\\ibcProfile.dmpRpt")) {
                    robocopy(sOptDir + "\\IbcProfile\\ibcProfile.dmpRpt", _toLocalLogsFolder(sVblRoot) + "\\dumps", undefined, undefined, undefined, true);
                }
            } catch (e) {
                logMsg(g_oRetailLabLogger, LogInfo, "Error copying dump files: " + e.description + "\r\n");
            }            
            
            throw new Error("Failure while optimizing the " + buildArchs[k] + " runtime\r\n");
        }
    }

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "Optimize", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to generate the layout build out of the optimized raw build

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabCreateOptLayout(sJobId)
{
    _correctTaskStatus(sJobId, "CreateOptLayout");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "CreateOptLayout", g_oTaskStates.Running);

    if (sJobId ==undefined) {
        _logTaskStatus(sJobId, "CreateOptLayout", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to create a layout build\r\n");
    }
    var sVblRoot = srcBaseFromScript();
    var remoteStore = _getJobParameter(sJobId, "Remote_Store");
    var buildType = _getJobParameter(sJobId, "Build_Type");

    // call functionality to create opt layout build from raw bits
    _setJobParameter(sJobId, "Job_State", JobStates.BuildingOpt);
    logMsg(g_oRetailLabLogger, LogInfo, "BuildingOpt Job_Id: " + sJobId + "\r\n");
    createLayoutFromRaw(sVblRoot, remoteStore, buildType);

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "CreateOptLayout", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to publish the job processed by the retail lab worker

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabPublish(sJobId)
{
    _correctTaskStatus(sJobId, "Publish");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "Publish", g_oTaskStates.Running);

    if (sJobId ==undefined) {
        _logTaskStatus(sJobId, "Publish", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error("JobId needed to publish the job results\r\n");
    }
    var sVblRoot = srcBaseFromScript();
    var sSharePath = _getJobParameter(sJobId, "Job_Destination");
    var sLogDirectory = _getJobParameter(sJobId, "Job_Logs");
    var sLocalLogPath = _toLocalLogsFolder(sVblRoot);
    var oRunEnvironments = _getRunEnvironments(sVblRoot);
    var tfLocalBranchOpts = oRunEnvironments.BranchEnvironment;
    var buildType = _getJobParameter(sJobId, "Build_Type");

    _setJobParameter(sJobId, "Job_State", JobStates.Publishing);
    logMsg(g_oRetailLabLogger, LogInfo, "Publishing Job_Id: " + sJobId + "\r\n");            

    // selectively copy the opt builds to the output drop location
    logMsg(g_oRetailLabLogger, LogInfo, "Copying opt layout build to output drop location. This will take some time...\r\n");
    var sLogFilePath = sLocalLogPath + "\\" + "optDropCopy." + buildType + ".log";
    robocopy(_toLocalSrcSetupFolder(sVblRoot), sSharePath + "\\opt", "/E /XD binaries.ia64ret /XD ia64ret /XD localized /XD setup /XD signed /XD sources /XD wix", sLogFilePath, true, true);

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    // We do DFS publishing at the very end so that the perf lab does not pick up an incomplete job by mistake
    var sDfsSharePath = _getJobDfsSharePath(sJobId);
    var sSharePath = _getJobParameter(sJobId, "Job_Destination");
    try {
        // DFS publishing
        runCmdToLog(sVblRoot + "\\ndp\\clr\\snap2.4\\tasks\\dfscmd /map " + sDfsSharePath + " " + sSharePath, tfLocalBranchOpts);

        // If we come here, DFS publishing succeeded and we want to publish the DFS paths instead of the actual paths
        var sZipPath = _toJobZipPath(sDfsSharePath);
        var sLogPath = _toJobLogPath(sDfsSharePath)
        _setJobParameter(sJobId, "Job_Visible_Destination", sDfsSharePath);
        _setJobParameter(sJobId, "Job_Zips", sZipPath);
        _setJobParameter(sJobId, "Job_Logs", sLogPath);
    } catch (e) {
        logMsg(g_oRetailLabLogger, LogInfo, "DFS publishing failed: " + e.description + "\r\n");
    }

    // if optimization failed for any reason, publish the input drop as the output drop
    var oTaskInfoObject = _getTaskInfoObject(sJobId);
    if(oTaskInfoObject["Optimize"].ErrorCode != g_oTaskStates.Success || oTaskInfoObject["CreateOptLayout"].ErrorCode != g_oTaskStates.Success) {
        var sInputDrop = sDfsSharePath + "\\unopt";
        var sOutputDrop = sDfsSharePath + "\\opt"; 
        runCmdToLog(sVblRoot + "\\ndp\\clr\\snap2.4\\tasks\\dfscmd /map " + sOutputDrop + " " + sInputDrop, tfLocalBranchOpts);
    } else {
        // Since we have successfully generated fresh optimization data, 
        // we drop a semaphore
        FSOCreateTextFile(sSharePath + "\\" + g_oRetailLabSemaphores.FreshOptimizationDataSemaphore, true);
    }

    // Since we have the job published, we can create a semaphore that is needed by the perf lab 
    // to say that the job has really finished
    FSOCreateTextFile(sSharePath + "\\" + g_oRetailLabSemaphores.JobCompleteSemaphore, true);

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "Publish", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/*
    Function to do perform any finalization jobs after the retail lab's optimization

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailLabFinalize(sJobId)
{
    _correctTaskStatus(sJobId, "Finalize");
    var oDurationStart = new Date();
    _logTaskStatus(sJobId, "Finalize", g_oTaskStates.Running);

    if (sJobId == undefined) {
        _logTaskStatus(sJobId, "Finalize", g_oTaskStates.Failure, ((new Date() - oDurationStart) / (60 * 1000)).toFixed(2));
        throw new Error(1, "JobId needed to run additional commands\r\n");
    }        
    var sVblRoot = srcBaseFromScript();
    var remoteStore = _getJobParameter(sJobId, "Remote_Store");
    var buildType = _getJobParameter(sJobId, "Build_Type");
    var sJobBranch = _getJobParameter(sJobId, "Job_Branch");    
    var oRunEnvironments = _getRunEnvironments(sVblRoot);
    var tfLocalBranchOpts = oRunEnvironments.BranchEnvironment;
    var sJobClientType = _getJobParameter(sJobId, "Job_EnlistmentType");
    var sMachineName = MACHINE_NAME;
    var jobFlags = _getJobParameter(sJobId, "Job_Flags");
    var sSharePath = _getJobParameter(sJobId, "Job_Destination");
    var sPathToTfCmdForLocalBranch = sVblRoot + "\\tools\\x86\\managed\\v4.0\\tf.cmd ";

    // Sometimes uninstall fails due to setup issues, better to run this step under try/catch
    logMsg(g_oRetailLabLogger, LogInfo, "Uninstalling the runtimes installed for this job to restore the machine state...\r\n");                
    try {
        var installParams = _calculateLayoutInstallParameters(sVblRoot, _toLocalLayoutFolder(sVblRoot), remoteStore, buildType);
        uninstallLayout(installParams.FrameworkInstallPath, installParams.AdditionalFrameworkInstallPath, installParams.ExtendedUninstallerName, installParams.ClientUninstallerName);
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, "Some error while uninstalling the layout: " + e.description + "\r\n");
    }

    // Disable client OS themes
    try {
        logMsg(g_oRetailLabLogger, LogInfo, "Disabling Client OS themes...\r\n");
        runCmdToLog(sVblRoot + "\\OptimizationData\\scripts\\TurnOffThemesWin2003.bat", runSetNoThrow());
    } catch (e) {
        logMsg(g_oRetailLabLogger, LogInfo, "Error in disabling themes: " + e.description + "\r\n");
    }

    // Submit a perf job after the retail build is generated if someone asks for it
    var fSubmitPerfJob = false;
    var sPerfLabCommandFlagValue = _getJobFlag(sJobId, "PERFLABCOMMAND");
    if (sPerfLabCommandFlagValue != null && sPerfLabCommandFlagValue.toUpperCase() == "TRUE") {
        fSubmitPerfJob = true;
    }

    // We only want to submit a perf job if everything succeeded
    // TODO: Fix this hack (we should only be considering perf submission if everything succeeded)
    if (fSubmitPerfJob)
    {
        var oTaskObject = _getTaskInfoObject(sJobId);

        if (oTaskObject["Initialize"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["BuildCopy"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["Enlist"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["SyncAndApplyChanges"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["Optimize"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["CreateOptLayout"].ErrorCode != g_oTaskStates.Success ||
            oTaskObject["Publish"].ErrorCode != g_oTaskStates.Success)
        {
            fSubmitPerfJob = false;
        }
    }

    if (fSubmitPerfJob) {
        _logTaskStatus(sJobId, "PerfLabSubmit", g_oTaskStates.NotStarted);

        var sPerfJobId = _toPerfJobId(sJobId);
        var sHidden = "/hidden";
        var sSource = "/source " + sPerfJobId;
        var sInstallType = "/installtype SNAPSetup";
        var sRelease = "/release PostOrcas";
        var sLab = _getSupportedBranchParameter(sJobBranch, "PerfJob_Lab_Name");
        if (!_isNullDatabaseEntry(sLab)) {
            sLab = "/lab " + sLab;
        } else {
            sLab = "";
        }
        var sContact = "/contact clrrlnot /contact " + _getJobParameter(sJobId, "Job_Owner").split(";")[0];

        // Invoke the perf lab command to queue the private job
        runCmdToLog("cscript //nologo " + g_sPerfLabInvokerScript + " " + sHidden 
                                                                  + " " + sSource
                                                                  + " " + sInstallType 
                                                                  + " " + sRelease
                                                                  + " " + sLab 
                                                                  + " " + sContact, 
                    runSetTimeout(60 * 60, runSetNoThrow()));
        _logTaskStatus(sJobId, "PerfLabSubmit", g_oTaskStates.Running);
    }
    else
    {
        // Mark that we could not start the perf job
        _logTaskStatus(sJobId, "PerfLabSubmit", g_oTaskStates.NotStarted);
    }

    _retailRelease(sJobId);

    // If a perf run was requested, the job is not really complete
    if (fSubmitPerfJob) {
        _setJobParameter(sJobId, "Job_State", JobStates.RetailComplete);
        logMsg(g_oRetailLabLogger, LogInfo, "RetailComplete Job_Id: " + sJobId + "\r\n");
    } else {
        _setJobParameter(sJobId, "Job_State", JobStates.Complete);
        logMsg(g_oRetailLabLogger, LogInfo, "Complete Job_Id: " + sJobId + "\r\n");
    }

    // turn on windows update again
    logMsg(g_oRetailLabLogger, LogInfo, "Turning ON Windows Update...\r\n");
    try {
        runCmdToLog("net start wuauserv", runSetNoThrow());
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, e.description + "\r\n");
    }    

    // Move logs to their final location
    _publishRetailJobLogs(sJobId);

    // We want task status of this task to be reflected in the job report and the mail
    // So we set the status before generating the report
    var oDurationEnd = new Date();
    _logTaskStatus(sJobId, "Finalize", g_oTaskStates.Success, ((oDurationEnd - oDurationStart) / (60 * 1000)).toFixed(2));
}

/****************************************************************************/
/* RETAIL LAB UTILITY FUNCTIONS TO PERFORM CERTAIN TARGETED OPERATIONS */
/****************************************************************************/

/****************************************************************************/
/*
    Function to determine if the new retail lab's functionality is being executed. 
    This functionality will later go away and is present here right now while we
    are running the older and newer retail lab side-by-side

    Arguments:
    None
  */
function isNewRetailLab() {
    var fIsNewRetailLab = GetSystemWideEnvVar("NEWRETAILLAB");
    if (fIsNewRetailLab != undefined && fIsNewRetailLab != "0") {
        fIsNewRetailLab = true;
    } else {
        fIsNewRetailLab = false;
    }
    logMsg(g_oRetailLabLogger, LogInfo, "isNewRetailLab returning: " + fIsNewRetailLab + "\r\n");    
    return fIsNewRetailLab;
}

/****************************************************************************/
/*
    Function to ensure that necessary free space is maintained on the share locations

    Arguments:
    None
  */
function _ensureFreeSpaceOnShares() {    
    logCall(g_oRetailLabLogger, LogInfo10, "_ensureFreeSpaceOnShares", arguments, "{");

    var sSharePath = undefined;
    try {
        var oSharePaths = [];
        var sSqlQuery = "SELECT * FROM " + g_sRetailLabShareTable;
        var oSqlResult = _executeDatabaseQuery(sSqlQuery);
        while(!oSqlResult.EOF) {
            // To keep the logic simple, we always make sure that all the share path's
            // have necessary free space
            var sCurrentSharePath = String(oSqlResult("Share_Path"));
            var nCurrentFreeRatio = (String(oSqlResult("Percentage_Allowed_Free_Space"))*1)/100;

            var nReTries = 0;
            var fFailure = false;
            while(true)
            {
                try
                {
                    // every share path has folders for each of the SNAP branches and a folder for
                    // all the private jobs
                    logMsg(g_oRetailLabLogger, LogInfo, "Removing oldest directories...\r\n");
                    _removeOldest(sCurrentSharePath, nCurrentFreeRatio, 2, 70, g_sRetailLabMaster, 1);
                    break;
                }
                catch(ex)
                {
                    // Try to execute 3 times (15 minutes). If we are still failing, bail out
                    if((++nReTries % 3) == 0) {
                        logMsg(g_oRetailLabLogger, LogInfo, "Failed to free space on share: " + sCurrentSharePath + " multiple times. Please investigate...\r\n");
                        fFailure = true;
                        break;
                    } else {
                        logMsg(g_oRetailLabLogger, LogInfo, "Free'ing space on share " + sCurrentSharePath + " .Failure was " + ex.description + ". Attempt number " + nReTries + ". Retrying in five minutes...");
                        WScript.Sleep(5 * 60);
                    }
                }
            }

            // We add this share path to the list only if we were
            // successfully able to clean it
            if (fFailure == false) {
                oSharePaths.push(sCurrentSharePath);
            }
            oSqlResult.MoveNext();
        }

        // pick up a random share location, if there are any available
        if (oSharePaths.length > 0) {
            var nRandomShareIndex = Math.floor(Math.random()*oSharePaths.length)*1;
            sSharePath = oSharePaths[nRandomShareIndex];
        }
    }catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, "Error while cleaning up the share..." + e.description + "\r\n");
    }

    logMsg(g_oRetailLabLogger, LogInfo, "} _ensureFreeSpaceOnShares\r\n");    
    return sSharePath;
}

/****************************************************************************/
/*
    Function to get the enlistment for processing on this machine, if present

    Arguments:
    sBranch: The enlistment from the branch we are looking for
    sMachine: The machine we are looking for an enlistment
 */
function _getEnlistmentOnMachine(sBranch, sMachine) {
    logCall(g_oRetailLabLogger, LogInfo10, "_getEnlistmentOnMachine", arguments, "{");

    var oEnlistmentRecord = { EnlistmentPresence : false, EnlistmentPath : undefined };
    var sSqlQuery = "SELECT * FROM " +  g_sRetailLabBranchTable;
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    while (!oSqlResult.EOF) {
        var sBranchFromDatabase = String(oSqlResult("Branch_Name"));
        var sMachineFromDatabase = String(oSqlResult("Machine_Name"));
        if (sBranch == sBranchFromDatabase && sMachine == sMachineFromDatabase) {
            oEnlistmentRecord.EnlistmentPresence = true;
            oEnlistmentRecord.EnlistmentPath = String(oSqlResult("Branch_Root"));
            break;
        }
        oSqlResult.MoveNext();
    }

    // If this is not one of our regular branches, we might have enlisted
    // temporarily for the sake of this job. Look for an enlistment
    // in the temporary branch table
    if (oEnlistmentRecord.EnlistmentPresence == false) {
        sSqlQuery = "SELECT * FROM " +  g_sRetailLabTemporaryBranchTable;
        oSqlResult = _executeDatabaseQuery(sSqlQuery);
        while (!oSqlResult.EOF) {
            var sBranchFromDatabase = String(oSqlResult("Branch_Name"));
            var sMachineFromDatabase = String(oSqlResult("Machine_Name"));
            if (sBranch == sBranchFromDatabase && sMachine == sMachineFromDatabase) {
                oEnlistmentRecord.EnlistmentPresence = true;
                oEnlistmentRecord.EnlistmentPath = String(oSqlResult("Branch_Root"));
            }
            oSqlResult.MoveNext();
        }
    }

    sSqlQuery = "SELECT * FROM " + g_sRetailLabSupportedBranchTable + " WHERE CONVERT(VARCHAR(100), Branch_Name)=CONVERT(VARCHAR(100), '" + sBranch + "')";
    oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        oEnlistmentRecord.FriendlyName = String(oSqlResult("Friendly_Name"));
    }

    logMsg(g_oRetailLabLogger, LogInfo, "} _getEnlistmentOnMachine\r\n");    
    return oEnlistmentRecord;
}

/****************************************************************************/
/*
    Helper function to get the run environments for running various commands

    Arguments:
    None
 */
function _getRunEnvironments(sVblRoot)
{
    if (sVblRoot == undefined)
        sVblRoot = srcBaseFromScript();
    var tfLocalBranchOpts = runSetEnv("COMPLUS_InstallRoot", sVblRoot + "\\tools\\x86\\managed",
                            runSetEnv("DEVDIV_TOOLS", sVblRoot + "\\Tools\\\devdiv",
                            runSetEnv("SDXROOT", sVblRoot,
                            runSetEnv("COMPLUS_Version", "v4.0",
                            runSetEnv("SourceControl", "TeamFoundation",
                            runSetEnv("_NTBINDIR", sVblRoot,
                            runSetEnv("_NTTREEBASE", sVblRoot + "\\..\\binaries",
                            runSetEnv("_NTDRIVE", sVblRoot.split(":")[0] + ":",
                            runSetEnv("_NTROOT", sVblRoot.split(":")[1],
                            runSet32Bit(
                            runSetNoThrow(tfLocalBranchOpts)))))))))));

    return { BranchEnvironment : tfLocalBranchOpts};
}

/****************************************************************************/
/*
    Function to perform some operations before the job releases the machine. This includes
    updating some information in the database and other cleanup activity

    Arguments:
    sJobId: ID of the job being processed
 */
function _retailRelease(sJobId) {
    // set the machine state to idle and decrement its queue count
    _setMachineParameter(MACHINE_NAME, "Machine_State", MachineStates.Idle);    
    logMsg(g_oRetailLabLogger, LogInfo, "Updated Machine state to Idle...\r\n");

    // Decrement this machine's queue count
    _decrementMachineQueueCount();

    // save the job's end time
    g_oJobEndTime = new Date();    
    g_oJobBeginTime = new Date(Date.parse(_getJobParameter(sJobId, "Start_Time")));
    logMsg(g_oRetailLabLogger, LogInfo, "Job Begin Time: " + _toSqlTime(g_oJobBeginTime) + "\r\n");
    logMsg(g_oRetailLabLogger, LogInfo, "Job End Time: " + _toSqlTime(g_oJobEndTime) + "\r\n");   
    var oJobDuration = ((g_oJobEndTime - g_oJobBeginTime) / (60 * 1000)).toFixed(2);
    _setJobParameter(sJobId, "End_Time", _toSqlTime(g_oJobEndTime));
    _setJobParameter(sJobId, "Job_Duration", oJobDuration);

    // clear the system wide environment variable that says that
    // this is the new retail lab infrastructure
    ClearSystemWideEnvVar("NEWRETAILLAB");
}

/****************************************************************************/
/*
    Function to find a job suitable for processing for this machine

    Arguments:
    sMachineName: Machine for which the next job for processing is to be found
 */
function _findSuitableRetailLabJob(sMachineName) {
    // find all jobs that are currently assigned to this machine
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabJobTable + " WHERE CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + sMachineName + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);

    var nSelectedJobId = INVALID_JOBID;
    var nSelectedJobPosition = nSelectedJobId;
    while (!oSqlResult.EOF) {
        var sJobState = String(oSqlResult("Job_State"));
        if (!_isNullDatabaseEntry(sJobState) && sJobState.toUpperCase() == "QUEUED")
        {
            var nCurrentJobId = String(oSqlResult("Job_Id")) * 1;
            var nCurrentJobPosition = String(oSqlResult("Job_Position"));
            if (_isNullDatabaseEntry(nCurrentJobPosition)) {
                nCurrentJobPosition = nCurrentJobId;
            }
            nCurrentJobPosition = nCurrentJobPosition * 1;

            if (nCurrentJobPosition < nSelectedJobPosition) {
                nSelectedJobId = nCurrentJobId;
                nSelectedJobPosition = nCurrentJobPosition;
                //logMsg(g_oRetailLabLogger, LogInfo, "Found: " + nSelectedJobId + "\r\n");
            }
        }
        oSqlResult.MoveNext();
    }

    return nSelectedJobId;
}

/****************************************************************************/
/*
    Function to publish the logs to their actual final destination
    This functionality is called when the job finishes its processing

    Arguments:
    sJobId: ID of the job being processed
 */
function _publishRetailJobLogs(sJobId) {
    // We are not using Job_Logs since we do not want to copy the logs to the DFS publishing server
    var sLogDirectory = _toJobLogPath(_getJobParameter(sJobId, "Job_Destination"));
    var sVblRoot = srcBaseFromScript();
    var sLocalLogPath = _toLocalLogsFolder(sVblRoot);

    try {
        if (!FSOFolderExists(sLogDirectory)) {
            FSOCreateFolder(sLogDirectory);
        }
        robocopy(sLocalLogPath, sLogDirectory, "/E", sLogDirectory + "\\logsCopy.log", true, true);
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, "Failure in copying logs to their final location: " + e.description + "\r\n");
    }

    try {
        FSOCopyFile(ScriptDir + "\\retailLabJob.log", sLogDirectory + "\\retailLabJob.log", true);
    } catch(e) {
        logMsg(g_oRetailLabLogger, LogInfo, "Failure in copying runjs transcript file to its final location: " + e.description + "\r\n");
    }
}

/****************************************************************************/
/*
    Function to generate a html report about a finished retail lab job. We expect this
    function to be called after a job has finished, so if the call is made on an unfinished
    job, it will return an empty retail lab job report

    Arguments:
    sJobId: Job ID as assigned by the retail lab server
 */
function _generateRetailLabJobReport(sJobId) {
    logCall(g_oRetailLabLogger, LogInfo10, "_generateRetailLabJobReport", arguments, "{");

    var sTitle = _getJobParameter(sJobId, "Job_Title");
    var sRawDrops = _getJobParameter(sJobId, "Raw_Build");
    var sJobType = _getJobParameter(sJobId, "Job_Type");
    var sJobResult = _getJobParameter(sJobId, "Job_Result");
    var sJobBranch = _getJobParameter(sJobId, "Job_Branch");
    var sJobDisplayResult = "Succeeded";
    if (sJobResult == JobResults.CompletedWithFailure || sJobResult == JobResults.Incomplete) {
        sJobDisplayResult = "Failed";
    }
    var sRemotestore = _getJobParameter(sJobId, "Remote_Store");
    var sJobShelveset = _getJobParameter(sJobId, "Job_ShelvesetWithOwner");
    var sJobSyncpoint = _getJobParameter(sJobId, "Job_Syncpoint");
    if (_isNullDatabaseEntry(sJobSyncpoint))
        sJobSyncpoint = "Latest";
    var sJobDuration = _getJobParameter(sJobId, "Job_Duration");
    var sJobEndTime = _getJobParameter(sJobId, "End_Time");
    if(_isNullDatabaseEntry(sJobEndTime)) {
        sJobEndTime = "";
    } else {
        sJobEndTime = ": " + sJobEndTime;
    }

    var sJobZips = _getJobParameter(sJobId, "Job_Zips");
    var sJobLogs = _getJobParameter(sJobId, "Job_Logs");
    var sJobDirectory = _getJobParameter(sJobId, "Job_Visible_Destination");
    var sJobReportPath = sJobDirectory + "\\JobReport.html";

    var perfLabSubmit = "No";
    var sPerfLabCommandFlagValue = _getJobFlag(sJobId, "PERFLABCOMMAND");
    if (sPerfLabCommandFlagValue != null && sPerfLabCommandFlagValue.toUpperCase() == "TRUE") {
        // A job is submitted to the perf lab only when it succeeds
        if (sJobResult == JobResults.CompletedWithSuccess || sJobResult == JobResults.CompletedWithWarnings) {
            perfLabSubmit = "Yes";
        }
    }    
    var sHtmlBody = "";

    // create a semicolon seperated list of job owners
    var sJobOwners = _getJobParameter(sJobId, "Job_Owner");
    
    if (!_isNullDatabaseEntry(sJobOwners)) {
        sJobOwners = sJobOwners + ";";
    } else {
        sJobOwners = "";
    }
    if (sJobShelveset.split(";").length > 1) {
        var sOwnerInShelveset = sJobShelveset.split(";")[1];
        if (sOwnerInShelveset.split("\\").length > 1) {
            if (sJobOwners != "") {
                sJobOwners = sJobOwners + ";";
            }
            sJobOwners = sJobOwners + sOwnerInShelveset.split("\\")[1];
        }
    }

    var oTaskObject = _getTaskInfoObject(sJobId);
    
    var sHtmlReport = FSOCreateTextFile(sJobReportPath, true);

    sHtmlBody += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">";
    sHtmlBody += ""; 
    sHtmlBody += "<html xmlns=\"http://www.w3.org/1999/xhtml\" >";
    sHtmlBody += "<head><title>";
    sHtmlBody += "";
    sHtmlBody += "</title></head>";
    sHtmlBody += "<body>";
    sHtmlBody += "    <form name=\"form1\" method=\"post\" action=\"Mail.aspx\" id=\"form1\">";
    sHtmlBody += "<div>";
    sHtmlBody += "<input type=\"hidden\" name=\"__VIEWSTATE\" id=\"__VIEWSTATE\" value=\"/wEPDwUKMTMxNTAxNDgzMmRke+G/ite6gzwIb5L0stozUVdtmzk=\" />";
    sHtmlBody += "</div>";
    sHtmlBody += " ";
    sHtmlBody += "    <div>";

    sHtmlBody += "        <table id=\"SummaryTable\" border=\"0\" style=\"width:500px;\">";

    if (sJobResult == JobResults.CompletedWithFailure) {    
        sHtmlBody += "            <tr id=\"SummaryHeaderRow\" style=\"color:White;background-color:Red;border-color:Red;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;font-weight:bold;\">";
        sHtmlBody += "                <th id=\"SummaryHeaderCell\" colspan=\"2\">Job " + sJobId + " Failed" + sJobEndTime + "</th>";
        sHtmlBody += "            </tr><tr id=\"SummaryRow\" style=\"border-color:Red;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;\">";
        sHtmlBody += "                <td id=\"SummaryCell\" colspan=\"2\">The Retail Lab system has aborted your job. Please review the explanation below and correct any problems</td>";
        sHtmlBody += "            </tr>";
    }
    else if (sJobResult == JobResults.CompletedWithSuccess) {    
        sHtmlBody += "            <tr id=\"SummaryHeaderRow\" style=\"color:White;background-color:#33CC33;border-color:#33CC33;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;font-weight:bold;\">";
        sHtmlBody += "                <th id=\"SummaryHeaderCell\" colspan=\"2\">Job " + sJobId + " Succeeded" + sJobEndTime + "</th>";
        sHtmlBody += "            </tr><tr id=\"SummaryRow\" style=\"border-color:#33CC33;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;\">";
        sHtmlBody += "                <td id=\"SummaryCell\" colspan=\"2\">The Retail Lab system has completed your job successfully. If a perf run was asked, your job was submitted to the perf lab. Please check the status on the web page</td>";
        sHtmlBody += "            </tr>";
    }
    else if (sJobResult == JobResults.CompletedWithWarnings) {    
        sHtmlBody += "            <tr id=\"SummaryHeaderRow\" style=\"color:White;background-color:#FFAA00;border-color:#FFAA00;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;font-weight:bold;\">";
        sHtmlBody += "                <th id=\"SummaryHeaderCell\" colspan=\"2\">Job " + sJobId + " Completed with Warnings" + sJobEndTime + "</th>";
        sHtmlBody += "            </tr><tr id=\"SummaryRow\" style=\"border-color:#FFAA00;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;\">";
        sHtmlBody += "                <td id=\"SummaryCell\" colspan=\"2\">The Retail Lab system has completed your job with warnings. Please review the explanation below and correct any problems</td>";
        sHtmlBody += "            </tr>";
    }
    else if (sJobResult == JobResults.Incomplete) {
        sHtmlBody += "            <tr id=\"SummaryHeaderRow\" style=\"color:#004000;background-color:#B2B2B2;border-color:#004000;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;font-weight:bold;\">";
        sHtmlBody += "                <th id=\"SummaryHeaderCell\" colspan=\"2\">Job " + sJobId + " did not complete" + sJobEndTime + "</th>";
        sHtmlBody += "            </tr><tr id=\"SummaryRow\" style=\"border-color:#004000;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;\">";
        sHtmlBody += "                <td id=\"SummaryCell\" colspan=\"2\">The Retail Lab system has not completed your job. It may be either running your job or it failed unexpectedly</td>";
        sHtmlBody += "            </tr>";
    }    
    
    sHtmlBody += "        </table>";
    sHtmlBody += "        <br />";
    sHtmlBody += "        <br />";
    sHtmlBody += "        <table id=\"JobTable\" border=\"0\" style=\"width:500px;\">";
    sHtmlBody += "    <tr id=\"JobTableHeaderRow\" style=\"color:#004000;background-color:#B2B2B2;border-color:#004000;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;font-weight:bold;\">";
    sHtmlBody += "        <th id=\"TableHeaderCell4\" colspan=\"2\">Job Summary</th>";
    sHtmlBody += "    </tr><tr id=\"TitleRow\">";
    sHtmlBody += "        <td id=\"TableCell1\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Title</td>";
    sHtmlBody += "        <td id=\"TableCell14\" style=\"font-family:Verdana;font-size:X-Small;\">" + sTitle + "</td>";
    sHtmlBody += "    </tr><tr id=\"RawDropsRow\">";
    sHtmlBody += "        <td id=\"TableCell9\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Raw Drops</td>";
    sHtmlBody += "        <td id=\"TableCell15\" style=\"font-family:Verdana;font-size:X-Small;\">" + sRawDrops + "</td>";
    sHtmlBody += "    </tr><tr id=\"JobBranchRow\">";
    sHtmlBody += "        <td id=\"TableCell27\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Job Branch</td>";
    sHtmlBody += "        <td id=\"TableCell28\" style=\"font-family:Verdana;font-size:X-Small;\">" + sJobBranch + "</td>";
    sHtmlBody += "    </tr><tr id=\"JobTypeRow\">";
    sHtmlBody += "        <td id=\"TableCell8\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Job Type</td>";
    sHtmlBody += "        <td id=\"TableCell16\" style=\"font-family:Verdana;font-size:X-Small;\">" + sJobType + "</td>";
    sHtmlBody += "    </tr><tr id=\"RemoteStoreRow\">";
    sHtmlBody += "        <td id=\"TableCell10\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Remote store</td>";
    sHtmlBody += "        <td id=\"TableCell17\" style=\"font-family:Verdana;font-size:X-Small;\">" + sRemotestore + "</td>";
    sHtmlBody += "    </tr><tr id=\"ShelvesetRow\">";
    sHtmlBody += "        <td id=\"TableCell11\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Shelveset</td>";
    sHtmlBody += "        <td id=\"TableCell18\" style=\"font-family:Verdana;font-size:X-Small;\">" + sJobShelveset + "</td>";
    sHtmlBody += "    </tr><tr id=\"SyncpointRow\">";
    sHtmlBody += "        <td id=\"TableCell12\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Syncpoint</td>";
    sHtmlBody += "        <td id=\"TableCell19\" style=\"font-family:Verdana;font-size:X-Small;\">" + sJobSyncpoint + "</td>";
    sHtmlBody += "    </tr><tr id=\"PerfLabRow\">";
    sHtmlBody += "        <td id=\"TableCell13\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Submitted to perf lab?</td>";
    sHtmlBody += "        <td id=\"TableCell20\" style=\"font-family:Verdana;font-size:X-Small;\">" + perfLabSubmit + "</td>";
    sHtmlBody += "    </tr><tr id=\"LogRow\">";
    sHtmlBody += "        <td id=\"TableCell2\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Logs</td>";
    sHtmlBody += "        <td id=\"TableCell21\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"JobLogsLink\" href=\"" + sJobLogs + "\">" + sJobLogs + "</a></td>";
    sHtmlBody += "    </tr><tr id=\"JobDirectoryRow\">";
    sHtmlBody += "        <td id=\"TableCell3\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Job Directory</td>";
    sHtmlBody += "        <td id=\"TableCell22\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"JobDirectoryLink\" href=\"" + sJobDirectory + "\">" + sJobDirectory + "</a></td>";
    sHtmlBody += "    </tr><tr id=\"DurationRow\">";
    sHtmlBody += "        <td id=\"TableCell4\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Duration</td>";
    sHtmlBody += "        <td id=\"TableCell23\" style=\"font-family:Verdana;font-size:X-Small;\">" + sJobDuration + "</td>";
    sHtmlBody += "    </tr><tr id=\"JobResultRow\">";
    sHtmlBody += "        <td id=\"TableCell5\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Job Result</td>";
    sHtmlBody += "        <td id=\"TableCell24\" style=\"font-family:Verdana;font-size:X-Small;\">" + sJobDisplayResult + "</td>";
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"JobZipsRow\">";
    sHtmlBody += "        <td id=\"TableCell25\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Job Zips</td>";
    sHtmlBody += "        <td id=\"TableCell26\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"JobZipsLink\" href=\"" + sJobZips + "\">" + sJobZips + "</a></td>";
    sHtmlBody += "    </tr>";
    sHtmlBody += "</table>";
    sHtmlBody += "        ";
    sHtmlBody += "        <br />";
    sHtmlBody += "        <br />";
    sHtmlBody += " ";
    sHtmlBody += "        <table id=\"TaskTable\" border=\"0\" style=\"width:500px;\">";
    sHtmlBody += "    <tr id=\"TaskTableHeaderRow\" style=\"color:#004000;background-color:#B2B2B2;border-color:#004000;border-width:1px;border-style:Solid;font-family:Verdana;font-size:Small;font-weight:bold;\">";
    sHtmlBody += "        <th id=\"TaskTableHeaderCell\">Task</th><th id=\"DetailsTableHeaderCell\">Details</th><th id=\"TableHeaderCell1\">Duration</th><th id=\"ErrorCodeTableHeaderCell\">ErrorCode</th>";
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"InitializeRow\">";
    sHtmlBody += "        <td id=\"TableCell31\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Initialize</td>";
    sHtmlBody += "        <td id=\"TableCell32\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Initialize.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell33\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Initialize.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell34\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"InitializeLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.Initialize.LogFileName + "\">" + oTaskObject.Initialize.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"BuildCopyRow\">";
    sHtmlBody += "        <td id=\"TableCell36\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">BuildCopy</td>";
    sHtmlBody += "        <td id=\"TableCell37\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.BuildCopy.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell38\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.BuildCopy.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell39\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"BuildCopyLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.BuildCopy.LogFileName + "\">" + oTaskObject.BuildCopy.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"EnlistRow\">";
    sHtmlBody += "        <td id=\"TableCell41\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Enlist</td>";
    sHtmlBody += "        <td id=\"TableCell42\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Enlist.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell43\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Enlist.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell44\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"EnlistLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.Enlist.LogFileName + "\">" + oTaskObject.Enlist.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"SyncAndApplyChangesRow\">";
    sHtmlBody += "        <td id=\"TableCell46\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">SyncAndApplyChanges</td>";
    sHtmlBody += "        <td id=\"TableCell47\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.SyncAndApplyChanges.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell48\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.SyncAndApplyChanges.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell49\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"SyncAndApplyChangesLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.SyncAndApplyChanges.LogFileName + "\">" + oTaskObject.SyncAndApplyChanges.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"CreateUnoptLayoutRow\">";
    sHtmlBody += "        <td id=\"TableCell51\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">CreateAndInstallUnoptLayout</td>";
    sHtmlBody += "        <td id=\"TableCell52\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.CreateAndInstallUnoptLayout.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell53\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.CreateAndInstallUnoptLayout.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell54\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"CreateAndInstallUnoptLayoutLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.CreateAndInstallUnoptLayout.LogFileName + "\">" + oTaskObject.CreateAndInstallUnoptLayout.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"OptimizeRow\">";
    sHtmlBody += "        <td id=\"TableCell61\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Optimize</td>";
    sHtmlBody += "        <td id=\"TableCell62\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Optimize.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell63\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Optimize.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell64\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"OptimizeLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.Optimize.LogFileName + "\">" + oTaskObject.Optimize.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"CreateOptLayoutRow\">";
    sHtmlBody += "        <td id=\"TableCell76\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">CreateOptLayout</td>";
    sHtmlBody += "        <td id=\"TableCell77\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.CreateOptLayout.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell78\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.CreateOptLayout.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell79\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"CreateOptLayoutLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.CreateOptLayout.LogFileName + "\">" + oTaskObject.CreateOptLayout.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"PublishRow\">";
    sHtmlBody += "        <td id=\"TableCell81\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Publish</td>";
    sHtmlBody += "        <td id=\"TableCell82\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Publish.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell83\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Publish.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell84\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"PublishLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.Publish.LogFileName + "\">" + oTaskObject.Publish.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "    </tr><tr id=\"FinalizeRow\">";
    sHtmlBody += "        <td id=\"TableCell86\" style=\"font-family:Verdana;font-size:X-Small;font-weight:bold;\">Finalize</td>";
    sHtmlBody += "        <td id=\"TableCell87\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Finalize.Details + "</td>";        
    sHtmlBody += "        <td id=\"TableCell88\" style=\"font-family:Verdana;font-size:X-Small;\">" + oTaskObject.Finalize.Duration + "</td>";        
    sHtmlBody += "        <td id=\"TableCell89\" style=\"font-family:Verdana;font-size:X-Small;\">" + "<a id=\"FinalizeLogLink\" href=\"" + sJobDirectory + "\\logs\\" + oTaskObject.Finalize.LogFileName + "\">" + oTaskObject.Finalize.ErrorCode + "</a></td>";        
    sHtmlBody += "    </tr>";
    sHtmlBody += "</table>";
    sHtmlBody += "    </div>";
    sHtmlBody += "    </form>";
    sHtmlBody += "</body>";
    sHtmlBody += "</html>";

    sHtmlReport.WriteLine(sHtmlBody);
    sHtmlReport.Close();

    logMsg(g_oRetailLabLogger, LogInfo, "HTML Report generated at " + sJobReportPath + "\r\n");
    logMsg(g_oRetailLabLogger, LogInfo, "} _generateRetailLabJobReport\r\n");
    return sJobReportPath;
}

/****************************************************************************/
/*
    Function to send an email to the job owners about a finished retail lab job

    Arguments:
    sJobId: Job ID as assigned by the retail lab server
 */
function _sendRetailLabMail(sJobId) {

    logCall(g_oRetailLabLogger, LogInfo10, "_sendRetailLabMail", arguments, "{");

    var sJobReportPath = _generateRetailLabJobReport(sJobId);

    // create a semicolon seperated list of job owners for sending an email to them
    var sJobShelveset = _getJobParameter(sJobId, "Job_ShelvesetWithOwner");
    var sJobOwners = _getJobParameter(sJobId, "Job_Owner");
    var sJobType = _getJobParameter(sJobId, "Job_Type");
    var sJobResult = _getJobParameter(sJobId, "Job_Result");
    
    if (!_isNullDatabaseEntry(sJobOwners)) {
        sJobOwners = sJobOwners + ";";
    } else {
        sJobOwners = "";
    }
    if (sJobShelveset.split(";").length > 1) {
        var sOwnerInShelveset = sJobShelveset.split(";")[1];
        if (sOwnerInShelveset.split("\\").length > 1) {
            if (sJobOwners != "") {
                sJobOwners = sJobOwners + ";";
            }
            sJobOwners = sJobOwners + sOwnerInShelveset.split("\\")[1];
        }
    }

    // for familiarity, we have a subject that is similar to SNAP
    var sMailSubject = "(ClrRetailLab:" + sJobType + " job:" + sJobId + ") " + sJobResult;
    var sMailBody = FSOReadFromFile(sJobReportPath);
    var sOwners = sJobOwners.split(";");
    var sUniqueOwners = {};
    for (var cnt=0; cnt<sOwners.length; cnt++) {
        if ((sOwners[cnt] == "") || 
            (sUniqueOwners[sOwners[cnt]] != undefined && sUniqueOwners[sOwners[cnt]] != null)) {
            continue;
        }
        sUniqueOwners[sOwners[cnt]] = sOwners[cnt];
        var sMailTo = sOwners[cnt] + "@microsoft.com";
        var sMailCc = "clrrlnot@microsoft.com";
        var sMailFrom = "clrgnt@microsoft.com";
        var sMailReplyTo = "clrrlnot@microsoft.com";
        var sMailServer = undefined;
        logMsg(g_oRetailLabLogger, LogInfo, "Sending mail to: " + sMailTo + "\r\n");

        // The job should not fail because of some mail server glitches or incorrect mail address
        // We log and ignore any mail sending errors
        try {
            mailSendHtml(sMailTo, sMailSubject, sMailBody, sMailCc, sMailFrom, sMailServer);
        } catch (e) {
            logMsg(g_oRetailLabLogger, LogInfo, "Mail delivery failed: " + e.description + "\r\n");
        }
    }        

    logMsg(g_oRetailLabLogger, LogInfo, "} _sendRetailLabMail\r\n");
}

/****************************************************************************/
/* RETAIL LAB HELPERS TO CONVERT FROM ONE PATH TO ANOTHER */
/****************************************************************************/
function _toJobZipPath(sJobOutputPath)
{
    return sJobOutputPath + "\\zips";
}

function _toJobLogPath(sJobOutputPath)
{
    return sJobOutputPath + "\\logs";
}

function _toLocalLogsFolder(sVblRoot) 
{
    return sVblRoot + "\\..\\logs";
}

function _toLocalBinariesFolder(sVblRoot)
{
    return sVblRoot + "\\binaries";
}

function _toLocalBinariesBackupFolder(sVblRoot)
{
    return sVblRoot + "\\binaries.backup";
}

function _toLocalSrcSetupFolder(sVblRoot)
{
    return sVblRoot + "\\..\\srcSetup";
}

function _toLocalLayoutFolder(sVblRoot)
{
    return _toLocalSrcSetupFolder(sVblRoot) + "\\layouts";
}

/****************************************************************************/
/* RETAIL LAB DATABASE HELPERS */
/****************************************************************************/

/****************************************************************************/
/*
    Function to set a parameter for a job

    Arguments:
    sJobId: ID of the job in question
    sParameterName: parameter in question
    sParameterValue: parameter value
 */
function _setJobParameter(sJobId, sParameterName, sParameterValue)
{
    if (sJobId == undefined)
        throw new Error(1, "JobId needed to set its parameter value\r\n");
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to set its value\r\n");

    var sSqlQuery = "UPDATE " + g_sRetailLabJobTable + " SET " + sParameterName + "='" + sParameterValue + "' WHERE CONVERT(VARCHAR(100), Job_Id)=CONVERT(VARCHAR(100), '" + sJobId + "')";
    _executeDatabaseQuery(sSqlQuery);
}

/****************************************************************************/
/*
    Function to get a parameter for a job

    Arguments:
    sJobId: ID of the job in question
    sParameterName: parameter in question
 */
function _getJobParameter(sJobId, sParameterName)
{
    if (sJobId == undefined)
        throw new Error(1, "JobId needed to determine its type\r\n");
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to determine its value\r\n");

    var sJobParameter = undefined;
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabJobTable + " WHERE CONVERT(VARCHAR(100), Job_Id)=CONVERT(VARCHAR(100), '" + sJobId + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        try {
            sJobParameter = String(oSqlResult(sParameterName));
        } catch(e) {
            // The following 2 entries are not present in the database
            // so we generate them here
            if (sParameterName == "Job_EnlistmentName")
                sJobParameter = MACHINE_NAME + "_TEMP_";
            else if (sParameterName == "Job_EnlistmentType")
                sJobParameter = "Minimum";            
            else if (sParameterName == "Job_BuildArchs")
            {
                var rawBuilds = _getJobParameter(sJobId, "Raw_Build");
                var remoteBuilds = rawBuilds.split(";");
                sJobParameter = [];
                for (var i=0; remoteBuilds != undefined && remoteBuilds != null && i<remoteBuilds.length; i++)
                {
                    var remoteBuild = remoteBuilds[i].split("=");
                    if ((String(getRealProcessorArchitecture()).toUpperCase() == "IA64" &&  String(remoteBuild[0]).toUpperCase() == "AMD64") ||
                        (String(getRealProcessorArchitecture()).toUpperCase() == "AMD64" &&  String(remoteBuild[0]).toUpperCase() == "IA64") ||
                        (String(getRealProcessorArchitecture()).toUpperCase() == "X86" &&  
                            (String(remoteBuild[0]).toUpperCase() == "IA64" || String(remoteBuild[0]).toUpperCase() == "AMD64"))) {
                        throw new Error(1, "Some of the provided raw builds cannot be processed by this machine\r\n");
                    }                    
                    sJobParameter.push(remoteBuild[0]);
                }
            }
            else if (sParameterName == "Job_BuildInstallRoots")
            {
                var rawBuilds = _getJobParameter(sJobId, "Raw_Build");
                var remoteBuilds = rawBuilds.split(";");
                sJobParameter = [];
                for (var i=0; remoteBuilds != undefined && remoteBuilds != null && i<remoteBuilds.length; i++)
                {
                    var remoteBuild = remoteBuilds[i].split("=");
                    if ((String(getRealProcessorArchitecture()).toUpperCase() == "IA64" &&  String(remoteBuild[0]).toUpperCase() == "AMD64") ||
                        (String(getRealProcessorArchitecture()).toUpperCase() == "AMD64" &&  String(remoteBuild[0]).toUpperCase() == "IA64") ||
                        (String(getRealProcessorArchitecture()).toUpperCase() == "X86" &&  
                            (String(remoteBuild[0]).toUpperCase() == "IA64" || String(remoteBuild[0]).toUpperCase() == "AMD64"))) {
                        throw new Error(1, "Some of the provided raw builds cannot be processed by this machine\r\n");
                    }                    
                    if (String(remoteBuild[0]).toUpperCase() == "X86") {
                        sJobParameter.push("%WINDIR%\\Microsoft.NET\\Framework\\");
                    } else {
                        sJobParameter.push("%WINDIR%\\Microsoft.NET\\Framework64\\");
                    }
                }
            }
        }
    }
    return sJobParameter;
}

/****************************************************************************/
/*
    Function to get the DFS share path for a job

    Arguments:
    sJobId: The id of the job
 */
function _getJobDfsSharePath(sJobId)
{
    if (sJobId == undefined)
        throw new Error(1, "JobId needed to determine its DFS share path\r\n");

    var sJobType = _getJobParameter(sJobId, "Job_Type");
    var sJobSyncPoint = _getJobParameter(sJobId, "Job_Syncpoint");
    var sJobBranchFriendlyName = _getSupportedBranchParameter(_getJobParameter(sJobId, "Job_Branch"), "Friendly_Name");
    var sDfsSharePath = undefined;

    if (sJobType == "SNAP") {
        sDfsSharePath = g_sRetailLabStoreRoot + "\\" + sJobBranchFriendlyName + "\\" + sJobSyncPoint;
    } else {
        sDfsSharePath = g_sRetailLabStoreRoot + "\\" + sJobBranchFriendlyName + "\\" + _toPerfJobId(sJobId);
    }

    return sDfsSharePath;
}

/****************************************************************************/
/*
    Function to get the value of one of the additional flags that may have been passed

    Arguments:
    sJobId: ID of the retail lab job
    sFlagName: Name of the flag whose value we want

    Returns:
    The value of the flag. If the flag name was not found, we return null
 */
function _getJobFlag(sJobId, sFlagName) {
    if (sJobId == undefined) {
        throw new Error(1, "JobId needed to get flag details");
    }
    if (sFlagName == undefined) {
        throw new Error(1, "Flag name needed to get its value");
    }

    logMsg(g_oRetailLabLogger, LogInfo, "Looking for flag: " + sFlagName + "\r\n")
    var sFlagValue = null;

    // 2 flags are seperated by a semi-colon (;)
    // every flag is of the format: (<flag_name>=<flag_value>)
    var sJobFlags = _getJobParameter(sJobId, "Job_Flags");
    if (!_isNullDatabaseEntry(sJobFlags)) {
        var individualJobFlags = sJobFlags.split(";");
        for (var i=0; i<individualJobFlags.length; i++) {
            var currentJobFlag = individualJobFlags[i];
            var sCurrentFlagName = currentJobFlag.split("=")[0];
            var sCurrentFlagValue = currentJobFlag.split("=")[1]; 

            if (sFlagName == sCurrentFlagName) {
                sFlagValue = sCurrentFlagValue;
                break;
            }
        }
    }

    logMsg(g_oRetailLabLogger, LogInfo, "Returning flag value: " + sFlagValue + "\r\n")        
    return sFlagValue;
}

/****************************************************************************/
/*
    Function to get the perf job id from the retail job.
    The format of a perf job's id is
    <syncpoint>_<owneralias><#> (where <owneralias><#>'s length is 8 characters long)

    Note: We return this ID based on the current state of the perf private job directory
             It is the responsiblity of the caller of this function to make sure a folder of that name
             is created immediately on the perf drop location

    Arguments:
    sJobId: ID of the retail lab job for which we want the perf job id
 */
function _toPerfJobId(sJobId) {
    if (sJobId == undefined) {
        throw new Error(1, "JobId needed to convert to perf job id");
    }

    // If we have already assigned a perf id for this job, we use that
    var sPerfJobId = _getJobParameter(sJobId, "Job_Perf_Id");
    if (_isNullDatabaseEntry(sPerfJobId)) {
        sPerfJobId = "";
        var sJobBranch = _getJobParameter(sJobId, "Job_Branch");
        var sJobBranchFriendlyName = _getSupportedBranchParameter(sJobBranch, "Friendly_Name");
        var sSyncpoint = _getJobParameter(sJobId, "Job_Syncpoint");
        if (_isNullDatabaseEntry(sSyncpoint)) {
            sSyncpoint = tfWhereSynced(srcBaseFromScript()).minChange;
            _setJobParameter(sJobId, "Job_Syncpoint", sSyncpoint);
            logMsg(g_oRetailLabLogger, LogInfo, "Updated syncpoint to: " + sSyncpoint + "\r\n");
        }
        var sAnOwner = (_getJobParameter(sJobId, "Job_Owner").split(";")[0]).substr(0, 4);
        for (var j=0; j<100; j++) {
            var sPotentialJobId = sSyncpoint + "_" + sAnOwner + j;
            if (!FSOFolderExists(g_sPerfLabPrivateDropRoot + "\\" + sJobBranchFriendlyName + "\\" + sPotentialJobId)) {
                sPerfJobId = sPotentialJobId;
                break;
            }
        }
    }

    return sPerfJobId;
}

/****************************************************************************/
/*
    Function to set a parameter for a machine

    Arguments:
    sMachineId: ID of the machine in question
    sParameterName: parameter in question
    sParameterValue: parameter value
 */
function _setMachineParameter(sMachineId, sParameterName, sParameterValue)
{
    if (sMachineId == undefined)
        throw new Error(1, "MachineId needed to set its parameter value\r\n");
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to set its value\r\n");

    var sSqlQuery = "UPDATE " + g_sRetailLabMachineTable + " SET " + sParameterName + "='" + sParameterValue + "' WHERE CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + sMachineId + "')";
    _executeDatabaseQuery(sSqlQuery);
}


/****************************************************************************/
/*
    Function to get a parameter for a machine

    Arguments:
    sMachineId: ID of the machine in question
    sParameterName: parameter in question
 */
function _getMachineParameter(sMachineId, sParameterName)
{
    if (sMachineId == undefined)
        throw new Error(1, "MachineId needed to determine its parameter value\r\n");
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to determine its value\r\n");
    var sMachineParameter = undefined;
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabMachineTable + " WHERE CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + sMachineId + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        sMachineParameter = String(oSqlResult(sParameterName));
    }
    return sMachineParameter;
}

/****************************************************************************/
/*
    Function to decrement this machine's queue count

    Arguments:
    None
 */
function _decrementMachineQueueCount() {
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabMachineTable + " WHERE CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + MACHINE_NAME + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if(!oSqlResult.EOF) {
        var nCurrentQueueCount = String(oSqlResult("Machine_Queue_Count")) * 1;
        if (nCurrentQueueCount > 0) {
            nCurrentQueueCount -= 1;
            _setMachineParameter(MACHINE_NAME, "Machine_Queue_Count", nCurrentQueueCount);
            logMsg(g_oRetailLabLogger, LogInfo, "Decrementing Machine_Queue_Count to " + nCurrentQueueCount + "\r\n");
        }
    }
}

/****************************************************************************/
/*
    Function to get a parameter for a branch from the Branch_Table in the retail lab database

    Arguments:
    sBranchName: Branch for which the parameter is to be found
    sMachineName: Machine for which the parameter is to be found
    sParameterName: parameter in question
 */
function _getBranchParameter(sBranchName, sMachineName, sParameterName)
{
    if (sBranchName == undefined)
        throw new Error(1, "BranchName needed to determine its parameter value\r\n");
    if (sMachineName == undefined)
        sMachineName = MACHINE_NAME;
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to determine its value\r\n");
    var sBranchParameter = undefined;
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabBranchTable + " WHERE CONVERT(VARCHAR(100), Branch_Name)=CONVERT(VARCHAR(100), '" + sBranchName + "') AND CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + sMachineName + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        sBranchParameter = String(oSqlResult(sParameterName));
    }
    return sBranchParameter;
}

/****************************************************************************/
/*
    Function to get a parameter for a branch from the Temporary_Branch_Table in the retail lab database

    Arguments:
    sBranchName: Branch for which the parameter is to be found
    sMachineName: Machine for which the parameter is to be found
    sParameterName: parameter in question
 */
function _getTemporaryBranchParameter(sBranchName, sMachineName, sParameterName)
{
    if (sBranchName == undefined)
        throw new Error(1, "BranchName needed to determine its parameter value\r\n");
    if (sMachineName == undefined)
        sMachineName = MACHINE_NAME;
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to determine its value\r\n");
    var sBranchParameter = undefined;
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabTemporaryBranchTable + " WHERE CONVERT(VARCHAR(100), Branch_Name)=CONVERT(VARCHAR(100), '" + sBranchName + "') AND CONVERT(VARCHAR(100), Machine_Name)=CONVERT(VARCHAR(100), '" + sMachineName + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        sBranchParameter = String(oSqlResult(sParameterName));
    }
    return sBranchParameter;
}

/****************************************************************************/
/*
    Function to get a parameter for a branch from the Supported_Branch_Table

    Arguments:
    sBranchName: Branch for which the parameter is to be found
    sParameterName: parameter in question
 */
function _getSupportedBranchParameter(sBranchName, sParameterName)
{
    if (sBranchName == undefined)
        throw new Error(1, "BranchName needed to determine its parameter value\r\n");
    if (sParameterName == undefined)
        throw new Error(1, "Parameter needed to determine its value\r\n");
    var sBranchParameter = undefined;
    var sSqlQuery = "SELECT * FROM " + g_sRetailLabSupportedBranchTable + " WHERE CONVERT(VARCHAR(100), Branch_Name)=CONVERT(VARCHAR(100), '" + sBranchName + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        sBranchParameter = String(oSqlResult(sParameterName));
    }
    return sBranchParameter;
}

/****************************************************************************/
/* RETAIL LAB LOGGING FUNCTIONALITY */
/****************************************************************************/

/****************************************************************************/
/*
    Function to get the task info object for this job. This object is used by the website to show 
    up-to-date information about the job on the web page

    Arguments:
    sJobId: ID of the job being processed
 */
function _getTaskInfoObject(sJobId)
{
    logCall(g_oRetailLabLogger, LogInfo10, "_getTaskInfoObject", arguments, "{");
    if (sJobId ==undefined)
        throw new Error("JobId needed to get the task object\r\n");

    var oTaskObject = g_oTaskObject;
    var sSqlQuery = "SELECT Job_Additional_Details=CONVERT(text, CONVERT(VARCHAR(MAX), Job_Additional_Details)) FROM " + g_sRetailLabJobTable + " WHERE CONVERT(VARCHAR(100), Job_Id)=CONVERT(VARCHAR(100), '" + sJobId + "')";
    var oSqlResult = _executeDatabaseQuery(sSqlQuery);
    if (!oSqlResult.EOF) {
        var sTaskInfo = String(oSqlResult("Job_Additional_Details"));
        if (!_isNullDatabaseEntry(sTaskInfo)) {            
            var sTempTaskReportPath = FSOGetTempPath();
            var sTempTaskReportFile = FSOCreateTextFile(sTempTaskReportPath, true);
            sTempTaskReportFile.WriteLine(sTaskInfo);
            sTempTaskReportFile.Close();
            oTaskObject = xmlDeserialize(sTempTaskReportPath);
        } else {
            oTaskObject = g_oTaskObject;
        }
    }
    logMsg(g_oRetailLabLogger, LogInfo, "}\r\n");
    return oTaskObject;
}

/****************************************************************************/
/*
    Function to set the task info object for this job. This object is used by the website to show 
    up-to-date information about the job on the web page

    Arguments:
    sJobId: ID of the job being processed
    oTaskObject: The task info object to be set
 */
function _setTaskInfoObject(sJobId, oTaskObject)
{
    logCall(g_oRetailLabLogger, LogInfo10, "_getTaskInfoObject", arguments, "{");
    if (sJobId ==undefined)
        throw new Error("JobId needed to set the task object\r\n");
    if (oTaskObject == undefined)
        oTaskObject = _getTaskInfoObject(sJobId);

    var sTempTaskReportPath = FSOGetTempPath();
    xmlSerialize(oTaskObject, sTempTaskReportPath, undefined, "\"");
    var sTaskInfo = FSOReadFromFile(sTempTaskReportPath);
    var sSqlQuery = "UPDATE " + g_sRetailLabJobTable + " SET Job_Additional_Details=CONVERT(xml, CONVERT(VARCHAR(MAX), N'" + sTaskInfo + "')) WHERE CONVERT(VARCHAR(100), Job_Id)=CONVERT(VARCHAR(100), '" + sJobId + "')";
    _executeDatabaseQuery(sSqlQuery);    
    logMsg(g_oRetailLabLogger, LogInfo, "}\r\n");
}

/****************************************************************************/
/*
    This functionality is now de-funct, but it is here since it may be useful for debugging purposes
    Function to convert a task info object to a string
    TODO: Accept sJobId as a parameter

    Arguments:
    None
 */
function _toStringTasksObject()
{
    var sDetailsString = "";
    if (g_oTaskObject != undefined)
    {
        sDetailsString = "[TaskName=" + g_oTaskObject.Initialize.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.Initialize.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.BuildCopy.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.BuildCopy.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.Enlist.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.Enlist.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.SyncAndApplyChanges.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.SyncAndApplyChanges.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.CreateAndInstallUnoptLayout.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.CreateAndInstallUnoptLayout.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.Optimize.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.Optimize.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.CreateOptLayout.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.CreateOptLayout.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.Publish.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.Publish.ErrorCode + "]" + "\r\n" +
                         "[TaskName=" + g_oTaskObject.Finalize.TaskName + "]" +
                         "[TaskStatus=" + g_oTaskObject.Finalize.ErrorCode + "]";    
    }
    return sDetailsString;
}

/****************************************************************************/
/*
    Function to set the status of a sub-task

    Arguments:
    sJobId: ID of the job being processed
    sSubTaskName: Sub-Task name under question
    sStatus: Actual status of the sub-task
    sDuration: Duration of this sub-task so far
 */
function _logTaskStatus(sJobId, sSubTaskName, sStatus, sDuration)
{
    if (sJobId == undefined)
        throw new Error(1, "JobId needed to update its sub-task status\r\n");
    if (sSubTaskName == undefined)
        throw new Error(1, "SubTask Name needed to update its status\r\n");
    if (sStatus == undefined)
        sStatus = g_oTaskStates.NotStarted;
    if (sDuration == undefined)
        sDuration = "0";
    var oTaskObject = _getTaskInfoObject(sJobId);

    // append a sub-task structure for any new sub-tasks that we don't know about    
    if (oTaskObject[sSubTaskName] == undefined || oTaskObject[sSubTaskName] == null) {
        if (g_oOptionalTaskObject[sSubTaskName] != undefined && g_oOptionalTaskObject[sSubTaskName] != null) {
            oTaskObject[sSubTaskName] = g_oOptionalTaskObject[sSubTaskName];
        } else {
            var oSubTaskObject = { TaskName : sSubTaskName, Summary : sSubTaskName, Details : "", Duration : 0, ErrorCode : g_oTaskStates.NotStarted, LogFileName : "retailLabFinalize.0.output.log" }; 
            oTaskObject[sSubTaskName] = oSubTaskObject;            
        }
    }

    oTaskObject[sSubTaskName].ErrorCode = sStatus;
    oTaskObject[sSubTaskName].Duration = sDuration;
    _setTaskInfoObject(sJobId, oTaskObject);
}

/****************************************************************************/
/*
    Function to correct the status of the sub-tasks in the event some error occurs and 
    some part of the code does not get executed

    Arguments:
    sJobId: ID of the job being processed
    sCurrentTaskName: Sub-Task name under question
 */
function _correctTaskStatus(sJobId, sCurrentTaskName)
{
    logCall(g_oRetailLabLogger, LogInfo10, "_correctTaskStatus", arguments, "{");
    if (sJobId ==undefined)
        throw new Error("JobId needed to set the task object\r\n");

    // get the current task object
    var oTaskObject = _getTaskInfoObject(sJobId);

    // update the task object
    if (sCurrentTaskName == "LabOptimize") {
        if (oTaskObject["Finalize"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["Finalize"].ErrorCode = g_oTaskStates.Failure;
        }
    } else if (sCurrentTaskName == "BuildCopy") {
        if (oTaskObject["Initialize"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["Initialize"].ErrorCode = g_oTaskStates.Failure;
        }
    } else if (sCurrentTaskName == "Enlist") {
        if (oTaskObject["BuildCopy"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["BuildCopy"].ErrorCode = g_oTaskStates.Failure;
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Incomplete;
        }
    } else if (sCurrentTaskName == "SyncAndApplyChanges") {
        if (oTaskObject["Enlist"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["Enlist"].ErrorCode = g_oTaskStates.Failure;
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Incomplete;
        }
    } else if (sCurrentTaskName == "CreateAndInstallUnoptLayout") {
        if (oTaskObject["SyncAndApplyChanges"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["SyncAndApplyChanges"].ErrorCode = g_oTaskStates.Failure;
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Incomplete;
        }
    } else if (sCurrentTaskName == "Publish") {
        if (oTaskObject["BuildCopy"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["BuildCopy"].ErrorCode = g_oTaskStates.Failure;
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Incomplete;
        }
        if (oTaskObject["Enlist"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["Enlist"].ErrorCode = g_oTaskStates.Failure;
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Incomplete;
        }
        if (oTaskObject["SyncAndApplyChanges"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["SyncAndApplyChanges"].ErrorCode = g_oTaskStates.Failure;
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Incomplete;
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Incomplete;
        }
                
        if (oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["CreateAndInstallUnoptLayout"].ErrorCode = g_oTaskStates.Failure;
        }
        if (oTaskObject["CreateOptLayout"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["CreateOptLayout"].ErrorCode = g_oTaskStates.Failure;
        }
        if (oTaskObject["Optimize"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["Optimize"].ErrorCode = g_oTaskStates.Failure;
        }
    } else if (sCurrentTaskName == "Finalize") {
        if (oTaskObject["Publish"].ErrorCode == g_oTaskStates.Running) {
            oTaskObject["Publish"].ErrorCode = g_oTaskStates.Failure;
        }
    }

    // set the updated task object
    _setTaskInfoObject(sJobId, oTaskObject);

    logMsg(g_oRetailLabLogger, LogInfo, "}\r\n");
}


