/*****************************************************************************/
/*                              SnapSniff.js                                  */
/*****************************************************************************/

/* Routines used by the Snap-Sniff pool.


   Conventions:  This file is designed to be used with the 'runjs.wsf'
   wrapper.  This wrapper allows you to call an arbitrary jscript function
   from the command line.   This is very nice becasue it allows you to
   run just pieces of the functionality in this file.  To make this work
   really well however, we need some conventions

                1) Break your routines into useful independant sub-pieces.  Then have
                   a routine that runs the pieces.  This gives users that capability
                   of runing just the pieces they need.

                2) DO put a nice comment right before the method explaining its args.
           This comment is put into the help for 'runjs' so you get user
                   documentation for free.

                3) DO think about defaults.  It makes your code much easier to use
                   from the command line.  If you follow the pattern used in the
                   functions below, the runjs /? will lift the defaults from the
                   source automatically.

                4) DO put a logMsg like the one in 'robocopy' in the begining of the
                   routine.  This allows users to see the pieces that they can
                   independantly run and and cut and past to runjs.

                5) DONT set environment variables in the source.  Instead use the
                   runSetEnv command to set environment variables for a particular
                   command.  This way the necessary environment is explicit.

                6) DONT build up state in once piece that is used by another.  This
                   makes it impossible to run the pieces independantly

                7) DONT wire in absolute paths.  It is fine to have defaults, but
                   generally people need control over where input and output goes.

                8) Any routines that are helpers should begin with an _, so show they
                   are not intended for calling directly from the command line

*/

/* AUTHOR: Sean Selitrennikoff
   DATE: 11/2/04 */

/******************************************************************************/
var SnapSniffModuleDefined = 1;              // Indicate that this module exists


if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include CLRAutomation.js");

var LogSnapSniff = logNewFacility("SnapSniff");

if (ScriptDir == undefined)
        var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

if (WshShell == undefined)
        var WshShell = WScript.CreateObject("WScript.Shell");

if (Env == undefined)
        var Env = WshShell.Environment("PROCESS");

        // This variable sets defaults that will hold true for all
        // commands spawned by SnapSniff.  Note that we really
        // want these to be things that never need to change.

/****************************************************************************/
/* Setup the machine for an auto reboot, and then to re-run this command
   after the reboot.

   Uses the clrgnt account to boot to and execute the command, thus this
   account must have admin right locally before the reboot completes.

   Parameters:
     srcBase - root directory of the enlistment
     command - the runJs command to execute after the reboot

*/

function _setupForRebootToCommand(srcBase, command)
{
    if (srcBase == undefined) {
        return 0;
    }

    if (command == undefined) {
        return 0;
    }

    var machineName = Env("COMPUTERNAME");
    if (!machineName)
    {
        throw new Error(1, "FAILED: RequiredCOMPUTERNAME environment variable is missing");
    }

    //
    // First get the password information from the password file
    //
    var userinfoFile = srcBase + "\\ndp\\clr\\snap2.4\\snapuser.cmd"
    var userinfoData = FSOReadFromFile(userinfoFile);
    if (!userinfoData.match(/set SNAP_MASTER=(\S+)\s*set SNAP_DOMAIN=(\S+)\s*set SNAP_USER=(\S+)/)) 
        throw Error(1, "Could not parse ", userinfoFile);
   
    var snapMaster   = RegExp.$1;
    var snapDomain   = RegExp.$2;
    var snapUser     = RegExp.$3;

    var passwordFile = "\\\\" + snapMaster + "\\" + snapUser + "$\\password.cmd"
    var passwordData = FSOReadFromFile(passwordFile);
    if (!passwordData.match(/set SNAP_PASSWORD=(\S+)/)) 
        throw Error(1, "Could not parse ", passwordFile);
    var snapPassword = RegExp.$1;

    //
    // Now write the reg keys necessary to do an autologon
    //
    logMsg(LogClrAutomation, LogInfo, "Configuring AutoLogin {\n");
    var winLogonKey = "\"\\\\" + machineName + "\\HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\"";
    runCmdToLog("reg add " + winLogonKey + " /v DefaultPassword /t REG_SZ /d \"" + snapPassword + "\" /f");
    runCmdToLog("reg add " + winLogonKey + " /v DefaultUser /t REG_SZ /d \"" + snapUser + "\" /f");
    runCmdToLog("reg add " + winLogonKey + " /v DefaultUserName /t REG_SZ /d \"" + snapUser + "\" /f");
    runCmdToLog("reg add " + winLogonKey + " /v DefaultDomainName /t REG_SZ /d \"" + snapDomain + "\" /f");
    runCmdToLog("reg add " + winLogonKey + " /v AutoAdminLogon /t REG_SZ /d \"1\" /f");
    logMsg(LogClrAutomation, LogInfo, "} autologin\n");

    //
    // Find the correct home directory for this user, to plop the script into
    //
    var home = "c:\\Documents and Settings\\clrgnt.REDMOND";
    if (!FSOFolderExists(home)) {
        home = "c:\\Documents and Settings\\clrgnt";
        if (!FSOFolderExists(home)) {
            home = "d:\\Documents and Settings\\clrgnt";
            if (!FSOFolderExists(home)) {
                home = "e:\\Documents and Settings\\clrgnt";
                if (!FSOFolderExists(home))
                    throw Error(1, "Could not locate home directory for clrgnt (logging in as clrgnt will create this)");
            }
        }
    }
    logMsg(LogClrAutomation, LogInfo, "Found Home directory ", home, "\n");

    //
    // Now write startup script
    //
    logMsg(LogClrAutomation, LogInfo, "Startup Scripts {\n");
    var startupScript =
         "REM This is just a sleep 35 to slow down infinite reboot loops\r\n" +
         "REM We are having problems with the machine taking 30 secs to configure itself\r\n" +
         "ipconfig \r\n" +
         "ping localhost /n 35\r\n" +
         "ipconfig \r\n" +
         srcBase + "\\ndp\\clr\\bin\\clrenv -cmd c:\\run.cmd\r\n";

    var startupFile = home + "\\Start Menu\\Programs\\Startup\\startUp.cmd";
    logMsg(LogClrAutomation, LogInfo, "writing startup file ", startupFile, "\n");
    FSOWriteToFile(startupScript, startupFile);

    var volume = srcBase.split("\\");
    var runScript = "";

    if (volume != undefined) {
        runscript = volume[0] + "\r\n cd " + srcBase + "\r\n";
    }

    runScript = runscript + "runjs " + command + "\r\n";
    var runFile = "c:\\run.cmd";

    logMsg(LogClrAutomation, LogInfo, "writing run file ", runFile, "\n");
    FSOWriteToFile(runScript, runFile);

    logMsg(LogClrAutomation, LogInfo, "} Starup Scripts\n");
}



/****************************************************************************/
/* Find a Snap job that is not listed as one that is currently "in use", as
   in some other machine ran this same function and grabbed that Snap job.

   Parameters:
       path - Path to the common file for machines to coordinate via.

   Returns:
       a string of "<JobID>.<Dev>" or undefined if no job available.

*/

function _findSnapJobToRun(path) {

    if (path == undefined) {
        return undefined;
    }

    //
    // Try to create a file on the path.  We use this creation as a kind of
    // "lock", i.e. if it succeeds, then we own the "lock" and can go ahead
    // and search for a Snap entry, record it, etc, otherwise, we spin until
    // the "lock" is "released" (i.e. the file is deleted)
    //
    var ownLock = false;
    var timeoutCount = 0;
    var file;
    var lockFileName = path + "\\lockfile.txt";
    var pauseFileName = path + "\\pausefile.txt";
    var paused = false;

    while (!ownLock) {

        //
        // Try to create the "lock" file
        //
        try {

            //
            // See if the entire Snap-Sniff pool is currently paused
            //
            if (FSOFileExists(pauseFileName)) {

                if (paused == false) {
                    logMsg(LogClrAutomation, LogInfo, "Snap-Sniff has been paused.\n");
                    paused = true;
                }
                //
                // Pause for a minute to let the other machine finish processing
                //
                WScript.Sleep(60000);
                continue;
            }

            if (!FSOFileExists(lockFileName)) {

                file = FSOCreateTextFile(lockFileName, false, false);
                file.Close();
                ownLock = true;

            }

        } catch (e) {

        }

        if (!ownLock) {

            //
            // Check if we have timed out of trying to find this.
            //
            if (timeoutCount > 5) {
                return undefined;
            }

            timeoutCount++;

            //
            // Pause for a minute to let the other machine finish processing
            //
            WScript.Sleep(60000);

        }
    }

    var usedFileName = path + "\\usedfile.txt";

    try {

        //
        // Read the file of 'used' snap entries
        //
        if (!FSOFileExists(usedFileName)) {

            FSOCreateTextFile(usedFileName, false, false).Close();

        }

        var fileData = FSOReadFromFile(usedFileName).split("\n");

        //
        // Get the live snap queue entries
        //
        var cmdResults = runCmd("snap queue").output;
        var jobs = cmdResults.split("\n");

        //
        // Build a new file string, removing any 'used' entries that
        // are no longer in the live snap queue.
        //
        var newFileData = "";
        var lineItem = undefined;
        var aJob = undefined;
        var jobNum = undefined;

        if (fileData != undefined) {

            //
            // There is an empty line in the last entry of the snap q command, dont process
            // that line :)
            //
            for (var i = jobs.length - 2; i > 2; i--) {

				if (jobs[i].match(/TEST ONLY/)) {
					//
					// Skip TEST ONLY jobs
					//
					continue;
				}

                aJob = jobs[i].split(/\s+/);

                if ((aJob[0].charAt(0) < '1') || (aJob[0].charAt(0) > '9')) {
                    
                    //
                    // Some kind of weird entry, skip it.
                    //
                    continue;
                }

                if (aJob[2] == "clrgnt") {

                    //
                    // This is a merged job, skip it
                    //
                    continue;
                }

                if (aJob[4] == "In") {

                    //
                    // Skip in progress jobs.
                    //
                    continue;
                }

                if (aJob[4] == "Not") {

                    //
                    // Skip not ready jobs.
                    //
                    continue;
                }

                //
                // Now search the data file to see if this job is already "in use".
                // This step also keeps jobs in the 'in use' file.
                //
                lineItem = aJob[0] + "." + aJob[2];

                var aLine;

                for (var j = 0; j < fileData.length; j++) {

                    aLine = fileData[j].split(/\s+/);

                    if (aLine[0] == lineItem) {

                        //
                        // Keep this one, still in the snap queue.
                        //
                        if (newFileData == "") {
                            newFileData = fileData[j];
                        } else {
                            newFileData = newFileData + "\n" + fileData[j];
                        }
                        aJob = undefined;
                        break;
                    }

                }

                //
                // If the job is not in the 'in use' file and is postponed,
                // we need to skip it.
                //
                if ((aJob != undefined) && (aJob[4] == "Postponed")) {

                    //
                    // Skip in postponed jobs, but remember we ran them, which
                    // is why we run the above code.
                    //
                    continue;
                }

                //
                // If we spun thru the file and it is not owned, remember that job
                //
                if ((aJob != undefined) && (jobNum == undefined)) {
                    jobNum = i;
                }

            }

        } else {

            //
            // There is no file out there right now, any job will do.  Take the last one.
            //
            jobNum = jobs.length - 2;
        }


        //
        // If we did not find an entry that is 'unused', exit now
        //
        if (jobNum == undefined) {

            //
            // Delete the "lock" file
            //
            FSODeleteFile(lockFileName);

            return undefined;
        }

        //
        // Add that entrie to the 'used' list.
        //
        aJob = jobs[jobNum].split(/\s+/);
        lineItem = aJob[0] + "." + aJob[2];

        if (newFileData == "") {
            newFileData = lineItem;
        } else {
            newFileData = newFileData + "\n" + lineItem;
        }
        newFileData = newFileData + " In Progress";

        //
        // Write out the new file of 'used' Snap entries
        //
        FSOWriteToFile(newFileData, usedFileName, false);

    } catch (e) {

        lineItem = undefined;

    }

    //
    // Delete the "lock" file
    //
    FSODeleteFile(lockFileName);

    return lineItem;
}


/****************************************************************************/
/* This routine marks a Snap-Sniff job with the status string given in the
 * given file.
*/

function _writeJobStatusToFile(fileName, aJob, statusString, add) {

    //
    // Read the file of 'used' snap entries
    //
    if (!FSOFileExists(fileName)) {

        return 1;
    }

    var fileData = FSOReadFromFile(fileName).split("\n");

    //
    // Now search the data file to find this job
    //
    var newFileData = "";
    var aLine;

    //
    // If we are supposed to blindly add this one to the history file, then skip
    // past finding the job and updating its entry and just copy everything over.
    //
    for (var j = 0; j < fileData.length; j++) {

        aLine = fileData[j].split(/\s+/);

        if ((aLine[0] == aJob) && (add == undefined)) {

            //
            // Update this one.
            //
            if (newFileData == "") {
                newFileData = aJob + " " + statusString;
            } else {
                newFileData = newFileData + "\n" + aJob + " " + statusString;
            }

        } else {

            if (newFileData == "") {
                newFileData = fileData[j];
            } else {
                newFileData = newFileData + "\n" + fileData[j];
            }
        }
    }

    if (add != undefined) {
        newFileData = newFileData + "\n" + aJob + " " + statusString;
    }

    //
    // Write out the new file.
    //
    FSOWriteToFile(newFileData, fileName, false);

    return 0;
}


/****************************************************************************/
/* This routine marks a Snap-Sniff job with the status string given.
*/

function _markJobAs(path, aJob, statusString) {

    if ((aJob == undefined) || (statusString == undefined)) {
        return 1;
    }

    //
    // Try to create the "lock" file that allows us to access the file
    // of jobs.
    //
    var ownLock = false;
    var file;
    var lockFileName = path + "\\lockfile.txt";

    while (!ownLock) {

        //
        // Try to create the "lock" file
        //
        try {

            if (!FSOFileExists(lockFileName)) {

                file = FSOCreateTextFile(lockFileName, false, false);
                file.Close();
                ownLock = true;

            }

        } catch (e) {

        }

        if (!ownLock) {

            //
            // Pause for a minute to let the other machine finish processing
            //
            WScript.Sleep(60000);

        }
    }

    try {

        var usedFileName = path + "\\usedfile.txt";
        var historyFileName = path + "\\historyfile.txt";

        _writeJobStatusToFile(usedFileName, aJob, statusString);
        _writeJobStatusToFile(historyFileName, aJob, statusString, "add");

    } catch (e) {

        //
        // Delete the "lock" file
        //
        FSODeleteFile(lockFileName);

        return 1;
    }

    //
    // Delete the "lock" file
    //
    FSODeleteFile(lockFileName);

    return 0;
}

/*****************************************************************************/
/* Common function for mailing the results of a failure to a specific person

*/
function _mailChangeListFailure(jobId, owner, appendStr, mailTo, path)
{

    var body = "<p>Job " + jobId + "." + owner + " FAILED the Snap-Sniff pool.</p>";

    body = body + "<p>Snap-Sniff successfully identifies problems over 95% of the time. " +
                  "If there is a smarty report, the failed task is labeled with FAILED, " +
                  "while all 'Manually Killed' and 'Child Killed' tasks are part of the " +
                  "abort process.</p>";

    if (appendStr != undefined) {
        body = body + appendStr;
    }

    body = body + "<p>Snap-Sniff pool is a pool of" +
               " machines by SeanSe that grab jobs from the snap queue and runs the SNAP tasks" +
               " against those jobs to help find jobs that will probably fail when SNAP tries" +
               " to process them.  If there is a smarty report" +
               " below use that to determine if you should pull your job from the queue now.  " +
               " You must first click on the link just above run.current and open that report, " +
               " as the machine continuously reboots and starts new jobs. </p>";

    if (path != undefined) {

        try {
            var htmlFile = FSOReadFromFile(path);

            if (htmlFile != undefined) {
                body = body + htmlFile;
            }

        } catch(e) {
        }

    }

    mailSendHtml(mailTo + "@microsoft.com",
                 "Your Snap job is postponed.",
                 body,
                 "seanse@microsoft.com",
                 "seanse@microsoft.com");
}


/*****************************************************************************/
/* Common function for mailing SeanSe that this Snap-Sniff machine needs looking at.

*/
function _mailSnapSniffMachineFailure(appendStr)
{
    var machineName = Env("COMPUTERNAME");

    var body = " Snap-Sniff pool machine " + machineName +
               " reports itself dead because: " + appendStr;

    body = body + "</p>";

    var toLine = "seanse@microsoft.com";


    mailSendHtml(toLine,
                 "Snap-Sniff Machine Dead!",
                 body,
                 "seanse@microsoft.com",
                 "seanse@microsoft.com");
}


/*****************************************************************************/
/* Function for taking a jobID for the snap queue and postponing it.  This
 * function does best attempt to unmerge and re-merge any job the failing job
 * is a part of.
 *
 * Return value is -1 if it fails to postpone the job, else it is the number
 * of jobs the given job was merged with.
*/
function _postponeJob(id)
{

    if (id == undefined) {
        return -1;
    }

    //
    // Get the live snap queue entries
    //
    var cmdResults = runCmd("snap queue").output;
    var jobs = cmdResults.split("\n");

    //
    // Search for this job in the snap queue
    //
    var lineItem = undefined;
    var aJob = undefined;

    //
    // There is an empty line in the last entry of the snap q command, dont process
    // that line :)
    //
    for (var i = jobs.length - 2; i > 2; i--) {

        aJob = jobs[i].split(/\s+/);

        //
        // Search for the job
        //

        if (aJob[0] != id) {

            //
            // Not this job, continue.
            //
            continue;
        }

        if (aJob[4] == "Postponed") {

            //
            // Someone already postponed it, excellent.
            //
            return 0;
        }

        if (aJob[4] == "In") {

            //
            // Oops, too late.
            //
            return -1;
        }

        if (aJob[4] == "Pending") {

            //
            // Simple case, the job is not merged.
            //

            cmdResults = runCmdToLog("snap postpone " + id);

            if (cmdResults.exitCode != 0) {
                return -1;
            }

            return 0;
        }

        //
        // Job is part of a merged job.
        //

        //
        // Work backwards to find the id of the merged job
        //
        for (var j = i; j > 2; j--) {

            aJob = jobs[j].split(/\s+/);

            if (aJob[2] != "clrgnt") {

                //
                // Not the merged job
                //
                continue;
            }

            //
            // Found the merged job, see if it is still pending.
            //

            if (aJob[4] != "Pending") {

                //
                // Too late
                //
                return -1;
            }

            break;
        }

        if (aJob[2] != "clrgnt") {

            //
            // Did not find the job for some reason.
            //
            return -1;
        }

        //
        // Cancel the merged job
        //
        cmdResults = runCmdToLog("snap cancel " + aJob[0]);

        if (cmdResults.exitCode != 0) {
            return -1
        }

        //
        // Create a list of all the jobs that were part of the merged job (excluding
        // the job we are postponing.
        //
        var jobList = "";
        var cMergedJobs = 0;

        for (i = j + 1; i <= jobs.length - 2; i++) {

            aJob = jobs[i].split(/\s+/);

            if (aJob[4] != "Merged") {

                //
                // End of the trail
                //
                break;
            }

            if (aJob[0] == id) {

                //
                // Skip merging in the job we are trying to postpone
                //
                continue;
            }

            jobList = jobList + aJob[0] + " ";
            cMergedJobs++;
        }

        //
        // Re-merge those jobs.  Note that if this fails because the first job
        // suddenly got started on us, or some such, the number of error conditions
        // is so high that instead we just leave the jobs unmerged.
        //
        cmdResults = runCmdToLog("snap merge " + jobList);

        if (cmdResults.exitCode != 0) {
            return -1;
        }


        //
        // Now postpone our job.
        //
        cmdResults = runCmdToLog("snap postpone " + id);

        if (cmdResults.exitCode != 0) {
            return -1;
        }

        return cMergedJobs;
    }

    //
    // Job was not found - return error
    //
    return -1;
}


/*****************************************************************************/
/* Main script for the Snap-Sniff pool of machines.

   Picks a changelist off the snap queue and runs the Snap tasks against it.
   This will really hork up your local machine, so you really should not be
   using this unless you know what you are doing.

*/
function confirmChangeListInSnapQueue()
{
    var srcBase = Env("_NTBINDIR");
    if (!srcBase)
    {
        throw new Error(1, "FAILED: Required _NTBINDIR environment variable is missing");
    }

    //
    // Revert all files
    //
    logMsg(LogClrAutomation, LogInfo, "Reverting local files...\n");
    sdRevert(srcBase + "\\...");

    //
    // Get the job to run.
    //
    var aJob = undefined;
    var pathToControlFiles = "\\\\snapsniff\\c\\temp";

    while (aJob == undefined) {

        aJob = _findSnapJobToRun(pathToControlFiles);

        if (aJob == undefined) {
            //
            // Sleep for five minutes and then try again
            //
            logMsg(LogClrAutomation, LogInfo, "No SNAP job to run, retry in 5 minutes...\n");
            WScript.Sleep(60000);
            logMsg(LogClrAutomation, LogInfo, "No SNAP job to run, retry in 4 minutes...\n");
            WScript.Sleep(60000);
            logMsg(LogClrAutomation, LogInfo, "No SNAP job to run, retry in 3 minutes...\n");
            WScript.Sleep(60000);
            logMsg(LogClrAutomation, LogInfo, "No SNAP job to run, retry in 2 minutes...\n");
            WScript.Sleep(60000);
            logMsg(LogClrAutomation, LogInfo, "No SNAP job to run, retry in 1 minute...\n");
            WScript.Sleep(60000);
        }
    }

    aJob = aJob.split(".");

    //
    // Log the changelist information
    //
    logMsg(LogClrAutomation, LogInfo, "Testing the following job: " + aJob[0] + "." + aJob[1] + "\n");


    //
    // Sync the tree to the live bits
    //
    logMsg(LogClrAutomation, LogInfo, "Syncing tree to live bits...\n");
    run = runCmdToLog(srcBase + "\\tools\\devdiv\\ddsync", runSetTimeout(3600));

    if (run.exitCode != 0)
    {
        _mailSnapSniffMachineFailure("ddsync of tree failed with " + run.exitCode);
        _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Infrastructure Failed");
        return 1;
    }

    runCmdToLog("cd " + srcBase);
    run = runCmdToLog("sd sync -w ...");

    //
    // Get the path to the snap queue where the bbpack is.
    //
    var changelistPath = Env("PathToSnapQueue");

    if (!changelistPath) {
        changelistPath = "\\\\clrsnapbackup2\\whidbey\\";
    }

    var bbpackCmd = changelistPath + aJob[0] + "." + aJob[1] + "\\" + aJob[1] + ".cmd -u -s";

    logMsg(LogClrAutomation, LogInfo, bbpackCmd + "\n");

    try {

        try {

            //
            // Apply that changelist locally
            //
            run = runCmdToLog(bbpackCmd);

            if (run.exitCode != 0)
            {
                _mailSnapSniffMachineFailure("unpacking failed with " + run.exitCode);
                _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Unpack command failed");
                return 1;
            }
        } catch (e) {

            _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Unpack command failed");
            return 1;

        }


        try {
            //
            // Ensure there are no conflict post application
            //
            var sdObj = sdConnect();
            sdResults = _sdRun(sdObj, "resolve -am", false, false);

            for (var i = 0; i < sdResults.WarningOutput.Count; i++) {

                var msg = sdResults.WarningOutput(i).Message;

                if (msg.match(/skipped/)) {

                    var details = " in ApplyChanges.  Warnings returned are:</p>";

                    for (var j = 0; j < sdResults.WarningOutput.Count; j++) {
                        details = details + "<p>" + sdResults.WarningOutput(i).Message + "</p>";
                    }

                    var mailTo = "seanse";
                    var statusString = "Failed";

                    //
                    // Postpone the job
                    //
                    var cMergedJobs = _postponeJob(aJob[0]);
                    if (cMergedJobs != -1) {
                        details = details +
                                   "<p>Your job has been postponed in the Snap queue.  Your job will not" +
                                   " be re-tested by the Snap-Sniff pool as long as it remains in the Snap queue" +
                                   " in any way, so after investigating the failure identified here, you may" +
                                   " reactivate your job if the reported failure was in error</p>";

                        mailTo = aJob[1];
                        statusString = statusString + " " + cMergedJobs;

                    } else {
                        statusString = statusString + " Too Late";
                    }


                    _mailChangeListFailure(aJob[0], aJob[1], details, mailTo);

                    //
                    // revert all changes and reboot
                    //

                    logMsg(LogClrAutomation, LogInfo, "Reverting change list...\n")
                    runCmdToLog("cd " + srcBase);
                    run = runCmdToLog("sd revert ...");

                    if (run.exitCode != 0)
                    {
                        _mailSnapSniffMachineFailure("Unable to revert change list with " + run.exitCode);
                        _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Infrastructure Failed");
                        return 1;
                    }

                    logMsg(LogClrAutomation, LogInfo, "COMPLETE.  Rebooting machine.\n");

                    _setupForRebootToCommand(srcBase, "confirmChangeListInSnapQueue | tee \lastrun.txt");

                    runCmdToLog("shutdown /r /t 20 /f", runSetNoThrow());

                    _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], statusString);
                    return 0;
                }

            }

            //
            // One last sync and resolve loop, to be sure
            //
            logMsg(LogClrAutomation, LogInfo, "Resyncing tree to live bits...\n");
            run = runCmdToLog(srcBase + "\\tools\\devdiv\\ddsync");

            if (run.exitCode != 0)
            {
                _mailSnapSniffMachineFailure("Unable to verify sync with " + run.exitCode);
                _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Infrastructure Failed");
                return 1;
            }

            var sdObj = sdConnect();
            sdResults = _sdRun(sdObj, "resolve -am", false, false);

            for (var i = 0; i < sdResults.WarningOutput.Count; i++) {

                var msg = sdResults.WarningOutput(i).Message;

                if (msg.match(/skipped/)) {

                    var details = " in ApplyChanges.  Warnings returned are:</p>";

                    for (var j = 0; j < sdResults.WarningOutput.Count; j++) {
                        details = details + "<p>" + sdResults.WarningOutput(i).Message + "</p>";
                    }

                    var mailTo = "seanse";
                    var statusString = "Failed";

                    //
                    // Postpone the job
                    //
                    var cMergedJobs = _postponeJob(aJob[0]);
                    if (cMergedJobs != -1) {
                        details = details +
                                   "<p>Your job has been postponed in the Snap queue.  Your job will not" +
                                   " be re-tested by the Snap-Sniff pool as long as it remains in the Snap queue" +
                                   " in any way, so after investigating the failure identified here, you may" +
                                   " reactivate your job if the reported failure was in error</p>";

                        mailTo = aJob[1];

                        statusString = statusString + " " + cMergedJobs;

                    } else {
                        statusString = statusString + " Too Late";
                    }

                    _mailChangeListFailure(aJob[0], aJob[1], details, mailTo);

                    //
                    // revert all changes and reboot
                    //

                    logMsg(LogClrAutomation, LogInfo, "Reverting change list...\n")
                    runCmdToLog("cd " + srcBase);
                    run = runCmdToLog("sd revert ...");

                    if (run.exitCode != 0)
                    {
                        _mailSnapSniffMachineFailure("Unable to revert change list with " + run.exitCode);
                        _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Infrastructure Failed");
                        return 1;
                    }

                    logMsg(LogClrAutomation, LogInfo, "COMPLETE.  Rebooting machine.\n");

                    _setupForRebootToCommand(srcBase, "confirmChangeListInSnapQueue | tee \lastrun.txt");

                    runCmdToLog("shutdown /r /t 20 /f", runSetNoThrow());

                    _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], statusString);
                    return 0;

                }
            }
        } catch(e) {

            _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "SourceDepot failed");
            return 1;

        }



        //
        //
        // Now run dailyDevRun
        //
        //
        try {

            //
            // Run dailyDevRun against the changes
            //
            run = doRun("snapSniffTasksWithEagerAbort",
                        undefined, // outputDirBase
                        undefined, // srcBase
                        undefined, // envStr
                        8);        // numDirsToKeep

            //
            // Log status
            //
            if (run == 0) {
                logMsg(LogClrAutomation, LogInfo, "PASSED confirmChangeList.\n");
                _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Passed");
            } else {

                var path = srcBase + "\\automation\\run.current\\taskReport.html";

                var details = "";

                var mailTo = "seanse";
                var statusString = "Failed";

                //
                // Postpone the job
                //
                var cMergedJobs = _postponeJob(aJob[0]);
                if (cMergedJobs != -1) {
                    details =  "<p>Your job has been postponed in the Snap queue.  Your job will not" +
                               " be re-tested by the Snap-Sniff pool as long as it remains in the Snap queue" +
                               " in any way, so after investigating the failure identified here, you may" +
                               " reactivate your job if the reported failure was in error</p>";

                    mailTo = aJob[1];

                    statusString = statusString + " " + cMergedJobs;

                } else {
                    statusString = statusString + " Too Late";
                }

                _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], statusString);

                _mailChangeListFailure(aJob[0], aJob[1], details, mailTo, path);
                logMsg(LogClrAutomation, LogInfo, "FAILED confirmChangeList.\n");
            }

        } catch (e) {

            //
            // Send an error e-mail out.
            //
            var path = srcBase + "\\automation\\run.current\\taskReport.html";

            var details = "";

            //
            // Postpone the job
            //

            var mailTo = "seanse";
            var statusString = "Failed";

            var cMergedJobs = _postponeJob(aJob[0]);
            if (cMergedJobs != -1) {
                details =  "<p>Your job has been postponed in the Snap queue.  Your job will not" +
                           " be re-tested by the Snap-Sniff pool as long as it remains in the Snap queue" +
                           " in any way, so after investigating the failure identified here, you may" +
                           " reactivate your job if the reported failure was in error</p>";

                mailTo = aJob[1];
                statusString = statusString + " " + cMergedJobs;

            } else {
                statusString = statusString + " Too Late";
            }

            _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], statusString);

            _mailChangeListFailure(aJob[0], aJob[1], details, mailTo, path);

        }

    } catch(e) {

    }

    //
    // revert all changes
    //

    logMsg(LogClrAutomation, LogInfo, "Reverting change list...\n")
    runCmdToLog("cd " + srcBase);
    run = runCmdToLog("sd revert ...");

    if (run.exitCode != 0)
    {
        _mailSnapSniffMachineFailure("Unable to revert change list with " + run.exitCode);
        _markJobAs(pathToControlFiles, aJob[0] + "." + aJob[1], "Infrastructure Failed");
        return 1;
    }

    logMsg(LogClrAutomation, LogInfo, "COMPLETE.  Rebooting machine.\n");

    _setupForRebootToCommand(srcBase, "confirmChangeListInSnapQueue | tee \lastrun.txt");

    runCmdToLog("shutdown /r /t 20 /f", runSetNoThrow());
    return 0;
}


