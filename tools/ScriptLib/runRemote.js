/*********************************************************************************/
/*                               runRemote.js                                    */
/*********************************************************************************/

/* runRemote extends the run infrastructure in run.js to that it can work on 
   remote machines.   This seems like it should be easy, but in fact is not
   because of security issues.   Most remote command utilities (rcmd and the like)
   have access to the remote machine, but do not have credentials when accessing
   other network resources from the remote machine.  This is by design (to avoid
   cascade security breach, but is very inconvinient.  

   After long searching, I believe the scheme used here is pretty good.  Basically
   it uses the scheduled tasks facilty in windows to run remote commands.  The
   idea is that you can specify a generic job in far future, which runs 'dispatch.js'
   When you create this scheduled task, you are prompted for your password and
   we pass it to the 'schtasks' windows command which encripts it and passes it
   to the target machine.   As long as you don't change the task, you are never
   prompted for the password again.   At this point the remote machine has your
   password, but got it in a secure way (no cleartext).   This one job will suffice
   for all remote commands from any source FOR THAT USER.  This job will live for
   over a year (longer that your password reset time), so you only have to 'prime'
   the system when your password changes. 

   The dispatch.js command looks in 

		%HOMEPATH%\AppData\Local\Runjs.Remote\PendingJobs 

	For new jobs to appear.  Since this directory is by default protected and 
    writable only by the user, it is reasonably secure (if a hacker can hack this
    directory, he can also hack your Explorer Startup folder).  

	The dispatcher keeps a log in 

		%HOMEPATH%\AppData\Local\dispatch.output.log

	The file in the pendingJobs directory just signals the fact that a job needs to
    be processed.  The information on what command should be run is in 

		%HOMEPATH%\AppData\Local\Runjs.Remote\Jobs\XXXXX.cmdInfo

	where XXXX is the name of the file found in 'PendingJobs'.  This file contains
    the command to run as well as a timeout.  The dispacher will dispatch all 
    work in Pending Jobs, and also wait for commands to complete.  It will kill
    any work that exceeds its timeout.   It saves output from the command into 

		%HOMEPATH%\AppData\Local\Runjs.Remote\Jobs\XXXXX.output.log

	And writes a status file

		%HOMEPATH%\AppData\Local\Runjs.Remote\Jobs\XXXXX.status.log

	That contains the process Exit value.

	It is not the lightest weight thing, but generally this is not a problem, and
    has some nice properties:

		1) The system is unlikely to hang (if a failure occures dispatch will 
		   die and the next command will respawn it).
		2) Generally orphans are not left behind (they time out and are killed).
		3) Logs are left behind.
		4) Logs are cleaned up after a time (should not grow over time)

    ------------------------------------------------------------------------------

	The runremote.js file builds on top of the 'run.js' infrastructure.  Effectively
	it add only one more public API.  Thus run.js can be used without runRemote.js
    or you can include both and get the additional APIs below.  Please see 'run.js'
 	for basic usage of the 'run' API. 

		runSetMachine(mach, options)	// set the machine on which the command executes

	In addition it provides a couple of 'administrative' functions

		runRemoteSetup					// set up a list of machines to be targets
		runRemoteTerminateAll			// kill all remote tasks on a machine


	EXAMPLES:

				// run a command on a particular machine, sending output to the log
			runCmdToLog("dir c:\\", runSetMachine("clrcert"));

*/


// Date	3/1/2004
// Dependancies
//		run.js			for logging utilities
//		log.js			for logging utilities
//		fso.js			for file system access (with good logging, and error messages)
//		proc.js			for process termination
// 	

/*********************************************************************************/
var runRemoteModuleDefined = 1; 		// Indicate that this module exist

if (!fsoModuleDefined) throw new Error(1, "Need to include run.js");
if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");

/******************************************************************************/
/* 
*/
function runSetMachine(machine, options) {

	if (machine == undefined)
		machine = Env("COMPUTERNAME");

	options = memberWiseCopy({}, options)
	options.machine = machine
	options.getPid = function() { if (!this.pid) _runRemoteGetPid(this); return this.pid; }
	options._startFtn = _runRemoteStart;
	options._isDoneFtn = _runRemoteIsDone;
	options._onTerminate = function(run) {
		FSOWriteToFile("EXITCODE: 254 (User Terminated)\r\n", run._statusFile);
	}

	return options;
}

/*****************************************************************************/
/* terminates all processes associated remote commands on 'machine', that
   were launched by the curent machine.  Note that this does not do a 
   perfect job.  It kills all processes that are identifiable as children
   of the main task process.  It is possible that if a process dies and
   breaks the parent-child chain, there will be a 'orphan' that does not
   get cleaned up 
*/
function runRemoteTerminateAll(machine) {

	logMsg(LogRun, LogInfo10, "runRemoteTerminateAll(", machine, ")\n");
	var wbem = GetObject("winmgmts://" + machine);
	var me = Env("COMPUTERNAME");
	var pat = new RegExp("title RunJS.Remote: (Job\\S+" + me + ")\\b", "i");

	var procs = procFind(pat, wbem);
	if (procs.length == 0)
		logMsg(LogRun, LogInfo, "runRemoteTerminateAll: nothing to kill\n");

	for(var i = 0; i < procs.length; i++) {
		var proc = procs[i];
		var cmdLine = proc.CommandLine;
		logMsg(LogRun, LogInfo, "Killing Job: ", cmdLine.match(pat)[1], "\n");
		procKillAll(proc.ProcessID, wbem, proc);
	}
}

/*****************************************************************************/
/* runRemoteSetup configures the space separated list of computers 
  'remoteComputersStr' so that it will accept remote commands.  If the remote
   user is not the same as the current user, 'fullUser' can be specified
   to indicate the account of the remote machine to use when executing
   remote commands. 

   Parameters:
     remoteComputerStr : Name of the remote machine.
     fullUser : The DOMAIN\user format for the user to run as.
     password : password for said user.
*/
function runRemoteSetup(remoteComputersStr, fullUser, password) {

	if (fullUser == undefined)
		fullUser = Env("USERDOMAIN") + "\\" + Env("USERNAME");

	var user = fullUser.match(/([^@\\]*)$/)[1];

	var taskName = "RunJS.Remote." + fullUser.replace(/\W+/, ".");

	if (password == undefined) {
	    var pw;
	    
	    try 
	    {
            pw = WScript.CreateObject("ScriptPW.Password");
        }
        catch(e)
        {
            /*  ScriptPW.Password is missing in Vista and up, due to missing scriptpw.dll. */
    	    throw Error(1, "Could not create ScriptPW.Password; please specify password as an argument.");
        }
    	WScript.StdOut.Write("Password for " + fullUser + ": ");
    	var password = pw.GetPassword();
    	WScript.Echo("");

    	// An alternative might be to use powershell instead:
    	// powershell -Command "[System.Runtime.InteropServices.Marshal]::PtrToStringUni([System.Runtime.InteropServices.Marshal]::SecureStringToBSTR((Get-Credential -Credential foo).password))"
    }
    
	var ret = 0;
	var remoteComputers = remoteComputersStr.split(/\s+/);
	for(var i = 0; i < remoteComputers.length; i++) {
		var remoteComputer = remoteComputers[i];

		try { 
			var userProf = _getRemoteUserProfile(remoteComputer, fullUser)

			var runJSDirRel = "AppData\\Local\\RunJS.Remote";
			var runJSDirRemote = userProf + "\\" + runJSDirRel;
			FSOCreatePath(runJSDirRemote);

			/* Don't do this as it makes debugging a pain (you have to be the remote user)
			   and does not increase security that much (admins on the local box can already 
			   do bad things
				// set ACLS for this to be highly restrictive 
			var cacls = runCmd("cacls \"" + runJSDirRemote + "\" /P " + fullUser + ":F", runSetInput("y\r\n"))
			if (!cacls.output.match(/processed dir:/)) 
				throw Error(1, "Error setting ACLs for directory " + runJSDirRemote);

			*/

				// If the folder exists, delete it to prevent people from trying to fool us into
				// believing we should run their file. (cleans out the queue of pending jobs)
			var pending = runJSDirRemote + "\\PendingJobs";
			if (FSOFolderExists(pending))
				FSODeleteFolder(pending, true);

			FSOCreatePath(pending);
			FSOCreatePath(runJSDirRemote + "\\Jobs");

			var dispatchRemote = runJSDirRemote + "\\dispatch.js";
			writeRemoteServer(dispatchRemote);

			runCmd("schtasks /delete /s " + remoteComputer + " /f /tn " + taskName, runSetNoThrow());
			var now = new Date();
			cmd = "schtasks /create /s " + remoteComputer + " /tn " + taskName +
				  " /xml " + ScriptDir + "\\runRemote_schtask.xml" +
				  " /ru " + fullUser;
			var run = runCmd(cmd, runSetInput(password + "\r\n"));
			if (run.output.match(/has been created, but may not run/))
			    throw Error(1, "Could not set task impersonation (password is incorrect?)");
			if (run.exitCode != 0)
			    throw Error(1, "schtasks returned " + run.exitCode);

			run = undefined;
			logMsg(LogRun, LogInfo, "Setup for machine " + remoteComputer + " Complete\n");
		}
		catch(e) {
			logMsg(LogRun, LogError, "Setup for machine " + remoteComputer + " FAILED\n");
			logMsg(LogRun, LogError, e.description + "\n");
			if (e.description.match("The network path was not found."))
			    logMsg(LogRun, LogInfo, "Does " + remoteComputer + "'s firewall settings allow " +
			        "Remote Scheduled Task Management?\n");
			ret = 1;
		}
	}
	return ret;
}

/*****************************************************************************/
/* gets the process ID (on the remote machine) of of the task associated with 
   'run' 
*/
function _runRemoteGetPid(run) {

	run._wbem = GetObject("winmgmts://" + run.machine);
	var jobID = run._jobFile.match(/\\([^\\]+).cmdInfo$/)[1];
	logMsg(LogRun, LogInfo100, "_runRemoteGetPid: got jobID: ", jobID, "\n");

	procs = procFind(new RegExp("title RunJS.Remote: " + jobID), run._wbem);
	if (procs.length == 1) 
		run.pid = procs[0].ProcessID;
	else if (procs.length > 0)
		throw Error(1, "Error: we found more than one process with a job title matching " + jobID);
	else 
		logMsg(LogRun, LogInfo10, "_runRemoteGetPid: could not find process ID for job ", jobID, "\n");

	logMsg(LogRun, LogInfo100, "_runRemoteGetPid: ", run.pid, "\n");
}

/*****************************************************************************/
/* returns true if the remote task specified by 'run' has complete */

function _runRemoteIsDone(run) {
    // If we are sending output to a file, copy what we can as soon as we can (like for string)
    if (run._outputTo == "file") {
        if (run._stdout == undefined && FSOFileExists(run._remoteOutName)) {
            run._stdout = FSOOpenTextFile(run._remoteOutName, FSOForReading);
        }
        if (run._stdout != undefined && !run._stdout.AtEndOfStream)
            run._stdoutTo.Write(run._stdout.ReadAll());
    }

    if (FSOFileExists(run._statusFile)) {
        var statusLine = FSOReadFromFile(run._statusFile);
        run.exitCode = 251; 	// bad remote status
        if (statusLine.match(/EXITCODE: *(\d+)/i))
            run.exitCode = (RegExp.$1 - 0);
        return true;
    }

    if (run.duration > 3000 && !FSOFileExists(run._remoteOutName)) {
        if (!run.retryTaskStart) {
            // There is a race starting the task up.  If the remote dispatcher is
            // exiting at the time the task was scheduled, it could be missed.  Try again.
            logMsg(LogRun, LogWarn, "No output file has been created yet by the remote system, Retrying the task\n");
            run.retryTaskStart = true;
            var taskName = "RunJS.Remote." + run.fullUser.replace(/\W+/, ".");
            var runSchtasks = runCmdToLog("schtasks /run /s " + run.machine + " /tn " + taskName, runSetNoThrow());
            if (runSchtasks.exitCode != 0)
                throw Error(1, "Could not start scheduled task " + taskName +
                " on machine " + run.machine + " (" + runSchtasks.exitCode + ")");
        }

        if (run.duration > 8000 && !run._remoteWarn) {
            run._remoteWarn = true;
            logMsg(LogRun, LogWarn, "Could not find output file ", run._remoteOutName, "\n\n");
            logMsg(LogRun, LogInfo, "This usually means that the scheduled task on the remote machine has\n");
            logMsg(LogRun, LogInfo, "failed to start.  This could just mean that the machine is heavily.\n");
            logMsg(LogRun, LogInfo, "loaded or it could mean that there is a problem in the configuration\n");
            logMsg(LogRun, LogInfo, "of the machine.  Do a 'schtasks /query /v /fo list /s ", run.machine, "'\n");
            logMsg(LogRun, LogInfo, "to see whether the Windows Scheduled Task has started or not\n");
            logMsg(LogRun, LogInfo, "Also on, ", run.machine, ", the files %SystemRoot%\\Tasks\\SchedLgU.Txt (W2K3)\n");
            logMsg(LogRun, LogInfo, "or %SystemRoot%\\SchedLgU.Txt (XP) have information on the last\n");
            logMsg(LogRun, LogInfo, "attempts to run a scheduled task.\n");
        }
    }

    return false;
}

/********************************************************************************/
/* Start a remote command specified by the 'run' object.  Basically this is
   just setting some files on the remote machine, and calling 'schedTasks /run'
   to kick it off
*/
function _runRemoteStart(run) {

	if (run.fullUser == undefined)
		run.fullUser = Env("USERDOMAIN") + "\\" + Env("USERNAME");
	
	var taskName = "RunJS.Remote." + run.fullUser.replace(/\W+/, ".");
	var userProf;
	try {
		userProf = _getRemoteUserProfile(run.machine, run.fullUser);
	} catch(e) {
		if (!ping(run.machine)) 
			throw Error(11, "ERROR: Machine " + run.machine + " is unreachable\n");
		throw e;
	}

	var runJSDirRel = "AppData\\Local\\RunJS.Remote";
	var runJSDirRemote = userProf + "\\" + runJSDirRel;
	var jobDir = runJSDirRemote + "\\Jobs"; 

	if (!FSOFolderExists(jobDir)) {
		throw Error(10, "Could not find the directory '" + runJSDirRel + "' on " + run.machine + "\n");
	}

	run._jobFile = getUniqueDateFileName(jobDir, "Job.", "." + Env("COMPUTERNAME") + ".cmdInfo");
	logMsg(LogRun, LogInfo100, "runRemoteStart: remote job file ", run._jobFile, "\n");

	var jobFileData = "";
					
	if (run._cwd)
		jobFileData += "CWD: " + run._cwd + "\r\n";
	if (run._env) {
		for (key in run._env)
			jobFileData += "ENV: " + key + "=" + run._env[key] + "\r\n";
    }

    // All the paths should be in terms of the parent computer 
    run.command = run.command.replace(/\b(\w):\\/g, "\\\\" + Env("COMPUTERNAME") + "\\$1$\\");
    
	jobFileData += "TIMEOUT: " + ((run.timeoutSec - 0) + 10) + "\r\n";
	jobFileData += "CMD: " + run.command + "\r\n";
	FSOWriteToFile(jobFileData, run._jobFile);

	var pendingFile = run._jobFile.replace(/Jobs\\(.*).cmdInfo$/i, "PendingJobs\\$1");
	FSOWriteToFile("Job " + run._jobFile + " needs to be run\r\n", pendingFile);

	run._remoteOutName = run._jobFile.replace(/\.cmdInfo$/i, ".output.log");
	if (run._outputTo == "string")
		run._stdoutName = run._remoteOutName;
	if (run._outputTo == "file")
		run._stdoutTo = FSOOpenTextFile(run._stdoutName, FSOForWriting, true);

	run._statusFile = run._jobFile.replace(/\.cmdInfo$/i, ".status.log");

	if (run._input) {
		inputName = run._jobFile.replace(/\.cmdInfo$/i, ".input.txt");
		FSOWriteToFile(run._input, inputName);
	}

	logMsg(LogRun, LogInfo100, "runRemoteStart: outputFile: ", run._stdoutName, "\n");
	logMsg(LogRun, LogInfo100, "runRemoteStart: statusFile: ", run._statusFile, "\n");

		// This command kicks off the scheduled task
	var runSchtasks = runCmdToLog("schtasks /run /s " + run.machine + " /tn " + taskName, runSetNoThrow());
    if (runSchtasks.exitCode != 0)
        throw Error(1, "Could not start scheduled task " + taskName + 
            " on machine " + run.machine + " (" + runSchtasks.exitCode + ")");	
}

/********************************************************************************/
/* This function tries to find the profile (home directory), for 'fullUser' 
   (eg DOMAIN\USER), on the computer 'remoteComputer'.  It is currently 
   heuristic (although a very good heuristic).  
*/
function _getRemoteUserProfile(remoteComputer, fullUser) {

	var aDomainUser = fullUser.match(/^([^\\]*)\\(.*)$/);
	var user = aDomainUser[2];
	var domain = aDomainUser[1];

		// TODO Need to guess at where the user profile is 
		// I could probably look this up using WMI somewhere
		// Win32_NetworkLoginProfile looks interesting

	var drives = ["C", "D", "E"];
	var userProf;
	for(var i = 0; ; i++) {
		if (i >= drives.length) 
			throw Error(1, "\nCould not find user profile for " + user + " on machine " + remoteComputer + ".\n\n" + 
						     "This usually means that you have never logged on to this machine.  Simply\n" +
						     "logging on to the machine once will generate a user profile and fix this.");
							
		var drive = drives[i];

		// Prefer user.DOMAIN over plain user, since if there was a local user
		// with the same name when the domain user registered, that's the disambiguation.
		userProf = "\\\\" + remoteComputer + "\\" + drive + "$" + "\\Users\\" + user + "." + domain.toUpperCase();
		logMsg(LogRun, LogInfo1000, "getRemoteUserProfile: Looking for user Profile: ", userProf, "\n");
		if (FSOFolderExists(userProf))
			break;

		userProf = "\\\\" + remoteComputer + "\\" + drive + "$" + "\\Users\\" + user;
		logMsg(LogRun, LogInfo1000, "getRemoteUserProfile: Looking for user Profile: ", userProf, "\n");
		if (FSOFolderExists(userProf))
			break;
	}
	logMsg(LogRun, LogInfo100, "getRemoteUserProfile: returning : ", userProf, "\n");
	return userProf;
}

/********************************************************************************/
/* _embedCode is meant to only be called during development.  After you get 
   dispatch.js just the way you like it, you can call _embedCode to generate
   the function 'writeRemoteServer' that embeds it in this file.
*/
function _embedCode(file, putIn) {


	var data = FSOReadFromFile(file)
	data = data.replace(/\\/g, "\\\\");
	data = data.replace(/(.*?)\r*\n\r*/gm, "        '$1\\r\\n' +\r\n");
	data = "function writeRemoteServer(file) {\r\n" + 
		   "    var dispatchSrc =\r\n" +
			        data + "\r\n" +
		   "        '\\r\\n';\r\n" +
		   "    FSOWriteToFile(dispatchSrc, file);\r\n" +
		   "}  // writeRemoteServer\r\n";
	
	// logMsg(LogScript, LogInfo, "Got data = ", data, "\n");
	var putInData = FSOReadFromFile(putIn);

	var origFile = putIn + ".orig";
	logMsg(LogScript, LogInfo, "saving = ", origFile, "\n");
	FSOWriteToFile(putInData, origFile);

	if (putInData.match(/^ *function writeRemoteServer.*{(.|\n)*?^\s*}.*writeRemoteServer.*/m)) {
		putInData = RegExp.leftContext + data + RegExp.rightContext;
	}
	else {
		logMsg(LogScript, LogWarning, "Did not find writeRemoteServer.  placing at the end\n");
		putInData = putInData + data;
	}


	logMsg(LogScript, LogInfo, "writing to = ", putIn, "\n");
	FSOWriteToFile(putInData, putIn);
}

/*************************************************************************************/
/* writeRemoteServer is just a routine that writes some javascript to 'file'.  In 
   this case it is dispatch.js, which is the code that is run on the target machine.
   its job is to look for work, dispatch it, and kill it if it takes too long */

function writeRemoteServer(file) {
    var dispatchSrc =
        '/*****************************************************************************/\r\n' +
        '/*                               dispatch.js                                 */\r\n' +
        '/*****************************************************************************/\r\n' +
        '\r\n' +
        '/* dispatch.js is the dispatcher task for the runjs Remoting infrastruture. \r\n' +
        '   Its job is pretty simple.  Look for files to execute in the PendingJobs \r\n' +
        '   directory (under the directory where dispatch.js lives), if it finds one\r\n' +
        '   look in the same place in the Jobs directory for a cmdInfo file that\r\n' +
        '   indicates the command to run and the timeout to apply.  Run this command\r\n' +
        '   collecting the output and gathing up the process exit status. \r\n' +
        '*/\r\n' +
        '/*****************************************************************************/\r\n' +
        '\r\n' +
        'var WshShell = new ActiveXObject("WScript.Shell");\r\n' +
        'var WshFSO = new ActiveXObject("Scripting.FileSystemObject");\r\n' +
        'var scriptDirName  = WScript.ScriptFullName.match(/^(.*)\\\\/)[1];\r\n' +
        '\r\n' +
        '		// Get the log file, saving 100K of history\r\n' +
        'var logFileName = scriptDirName + "\\\\dispatch.output.log";\r\n' +
        'var logFile = WshFSO.OpenTextFile(logFileName, 8, true);			// append\r\n' +
        'logFile.WriteLine(new Date() + " ******** Starting a new dispatch process  ******* ");\r\n' +
        '\r\n' +
        'logStats = WshFSO.GetFile(logFileName);\r\n' +
        'logFile.WriteLine(new Date() + " Current log file size = " + logStats.Size);\r\n' +
        'if (logStats.Size > 100000) {\r\n' +
        '	logFile.WriteLine(new Date() + " Log size exceeded, copying this file to *.prev.output.log and truncating");\r\n' +
        '	var prevName = scriptDirName + "\\\\dispatch.prev.output.log";\r\n' +
        '	logFile.Close();\r\n' +
        '	WshFSO.CopyFile(logFileName, prevName, true);\r\n' +
        '	logFile = WshFSO.OpenTextFile(logFileName, 2, true);			// overwrite\r\n' +
        '	logFile.WriteLine(new Date() + " ********* Starting a new dispatch process  ******* ");\r\n' +
        '}\r\n' +
        '\r\n' +
        'logFile.WriteLine(new Date() + " Deleting old log files");\r\n' +
        'var jobDirName = scriptDirName + "\\\\Jobs";\r\n' +
        'var now = new Date();\r\n' +
        'var jobDir = WshFSO.GetFolder(jobDirName);\r\n' +
        'for (var fileEnum = new Enumerator(jobDir.Files); !fileEnum.atEnd(); fileEnum.moveNext()) {\r\n' +
        '	var file = fileEnum.item();\r\n' +
        '	if ((now - file.DateLastModified) / (3600000*24) > 1) {\r\n' +
        '		logFile.WriteLine(new Date() + " File " + file.Path + " older than 1 days, deleting\\n");\r\n' +
        '		WshFSO.DeleteFile(file.Path, true);\r\n' +
        '	}\r\n' +
        '}\r\n' +
        '\r\n' +
        'logFile.WriteLine(new Date() + " Main processing loop");\r\n' +
        'try {\r\n' +
        '	var pendingDirName =scriptDirName + "\\\\PendingJobs"; \r\n' +
        '	var runs = [];	 				// running processes\r\n' +
        '	var newRuns = [];\r\n' +
        '	for(;;) {\r\n' +
        '		var now = new Date().getTime();\r\n' +
        '		findJobs(pendingDirName, jobDirName, runs)\r\n' +
        '		if (runs.length == 0)\r\n' +
        '			break;\r\n' +
        '		\r\n' +
        '		while (run = runs.pop()) {\r\n' +
        '			if (run.exec.status) {\r\n' +
        '				logFile.WriteLine(new Date() + " " + run.jobId + " Exiting with code " + run.exec.ExitCode);\r\n' +
        '				// print Exit Code\r\n' +
        '				writeToFile("EXITCODE: " + run.exec.ExitCode + "\\r\\n", run.statusName);\r\n' +
        '			}\r\n' +
        '			else if (now > run.timeOut) {\r\n' +
        '				logFile.WriteLine(new Date() + " " + run.jobId + " ***** KILLING **** " + run.exec.ProcessID);\r\n' +
        '				WshShell.run("taskKill /t /pid " + run.exec.ProcessID, 7, true);\r\n' +
        '				writeToFile("EXITCODE: 267014 (Timeout)\\r\\n", run.statusName);\r\n' +
        '			}\r\n' +
        '			else \r\n' +
        '				newRuns.push(run);\r\n' +
        '		}\r\n' +
        '		var temp = runs;\r\n' +
        '		runs = newRuns;\r\n' +
        '		newRuns = temp;\r\n' +
        '\r\n' +
        '		WScript.Sleep(500);\r\n' +
        '	}\r\n' +
        '	logFile.WriteLine(new Date() + " Master exiting");\r\n' +
        '}\r\n' +
        'catch(e) {\r\n' +
        '	logFile.WriteLine(new Date() + " ERROR: caught exception: " + e.description);\r\n' +
        '	throw e;\r\n' +
        '}\r\n' +
        '\r\n' +
        '/****************************************************************************/\r\n' +
        '/* look in pendingDirName for any pending jobs.  Look in jobDirName for the\r\n' +
        '   command file to run.  Start the task, and put the info for it in a new\r\n' +
        '   element in "runs" */\r\n' +
        '\r\n' +
        'function findJobs(pendingDirName, jobDirName, runs) {\r\n' +
        '\r\n' +
        '	var pendingDir = WshFSO.GetFolder(pendingDirName);\r\n' +
        '	for (var fileEnum = new Enumerator(pendingDir.Files); !fileEnum.atEnd(); fileEnum.moveNext()) {\r\n' +
        '		var pendingName = fileEnum.item().Path;\r\n' +
        '		pendingName.match(/([^\\\\]*)$/);\r\n' +
        '		var run = {};\r\n' +
        '		var jobCmdName = jobDirName + "\\\\" + RegExp.$1 + ".cmdInfo"; \r\n' +
        '		var inputName = jobDirName + "\\\\" + RegExp.$1 + ".input.txt";\r\n' +
        '		var outputName = jobDirName + "\\\\" + RegExp.$1 + ".output.log";\r\n' +
        '		run.statusName = jobDirName + "\\\\" + RegExp.$1 + ".status.log";\r\n' +
        '		run.jobId = RegExp.$1;\r\n' +
        '\r\n' +
        '		logFile.WriteLine(new Date() + " " + run.jobId + " Deleting " + pendingName);\r\n' +
        '		WshFSO.DeleteFile(pendingName);\r\n' +
        '\r\n' +
        '		if (WshFSO.FileExists(jobCmdName)) {\r\n' +
        '			var now = new Date().getTime();\r\n' +
        '			var descr = readFromFile(jobCmdName);\r\n' +
        '			if (descr.match(/^TIMEOUT: (\\d+)\\s*^CMD: (.*?)\\r*\\n/im)) {\r\n' +
        '				run.timeOut = now + RegExp.$1 * 1000;\r\n' +
        '				var fullCmd = "cmd /c title RunJS.Remote: " + run.jobId + " & " +\r\n' +
        '							   RegExp.$2 + " >\\"" + outputName + "\\" 2>&1";\r\n' +
        '				logFile.WriteLine(new Date() + " " + run.jobId + " STARTING: " + fullCmd);\r\n' +
        '				run.exec = startCmdInEnv(fullCmd, descr, run);\r\n' +
        '				if (WshFSO.FileExists(inputName)) \r\n' +
        '					run.exec.StdIn.Write(readFromFile(inputName));\r\n' +
        '				run.exec.StdIn.Close();\r\n' +
        '				run.exec.StdOut.Close();\r\n' +
        '				run.exec.StdErr.Close();\r\n' +
        '				runs.push(run);\r\n' +
        '			}\r\n' +
        '			else\r\n' +
        '				logFile.WriteLine(new Date() + " " + run.jobId + " Could not parse data in " + jobCmdName);\r\n' +
        '		}\r\n' +
        '		else \r\n' +
        '			logFile.WriteLine(new Date() + " " + run.jobId + " Could not find job file " + jobCmdName);\r\n' +
        '	}\r\n' +
        '}\r\n' +
        '\r\n' +
        '/****************************************************************************/\r\n' +
        '/* start the environment (environment vars and current working directory)\r\n' +
        '   specified by the jobData string.  It returns back the WshShell Exec\r\n' +
        '   object */\r\n' +
        '   \r\n' +
        'function startCmdInEnv(cmd, jobData, run) {\r\n' +
        '\r\n' +
        '		// update the environment as indicated\r\n' +
        '	var oldCwd = undefined;\r\n' +
        '	if (jobData.match(/^CWD: *(.*\\S)/m)) {\r\n' +
        '		try { \r\n' +
        '			WshShell.CurrentDirectory = RegExp.$1 \r\n' +
        '		} catch(e) {\r\n' +
        '			logFile.WriteLine(new Date() + " " + run.jobId + " Could not change Dir to " + RegExp.$1 + " Exception: " + e.description);\r\n' +
        '		}\r\n' +
        '		oldCwd = WshShell.CurrentDirectory;\r\n' +
        '	}\r\n' +
        '\r\n' +
        '		// TODO I dont handle the case where I want to define empty environment variable\r\n' +
        '		// (but neither does CMD.EXE) \r\n' +
        '	var oldEnv = undefined;\r\n' +
        '	var Env;\r\n' +
        '	while (jobData.match(/^ENV: *(.*?)=(.*\\S)/m)) {\r\n' +
        '		var varName = RegExp.$1;\r\n' +
        '		var varVal = RegExp.$2;\r\n' +
        '		jobData = RegExp.rightContext;\r\n' +
        '		if (!oldEnv) {\r\n' +
        '			oldEnv = {};\r\n' +
        '			Env = WshShell.Environment("PROCESS");\r\n' +
        '		}\r\n' +
        '		oldEnv[varName] = Env(varName);\r\n' +
        '		if (varVal != "") {\r\n' +
        '			while (varVal.match(/%(\\w+)%/)) {		// allow people to use references to current env vars\r\n' +
        '				var key = RegExp.$1;\r\n' +
        '				varVal = RegExp.leftContext + Env(key) + RegExp.rightContext;\r\n' +
        '			}\r\n' +
        '			Env(varName) = varVal;	\r\n' +
        '			logFile.WriteLine(new Date() + " " + run.jobId + " Setting ENV " + varName + "=" + varVal);\r\n' +
        '		}\r\n' +
        '		else \r\n' +
        '			Env.Remove(varName);\r\n' +
        '	}\r\n' +
        '\r\n' +
        '	ret = WshShell.Exec(cmd);					// Actually execute the command !\r\n' +
        '\r\n' +
        '		// restore the original environment\r\n' +
        '	if (oldEnv) {\r\n' +
        '		for (var varName in  oldEnv) {\r\n' +
        '			var varVal = oldEnv[varName];		// TODO - because Env return empty string for undefined vars,\r\n' +
        '			if (varVal != "") 					// I have a bug with distigushing this case (I note that \r\n' +
        '				Env(varName) = varVal;			// CMD.EXE has this same problem). \r\n' +
        '			else \r\n' +
        '				Env.Remove(varName);\r\n' +
        '		}\r\n' +
        '	}\r\n' +
        '	if (oldCwd) \r\n' +
        '		WshShell.CurrentDirectory = oldCwd;\r\n' +
        '	return ret;\r\n' +
        '}\r\n' +
        '\r\n' +
        '/****************************************************************************/\r\n' +
        'function writeToFile(str, fileName) {\r\n' +
        '	var file = WshFSO.OpenTextFile(fileName, 2, true);			// overwrite\r\n' +
        '	file.Write(str);\r\n' +
        '	file.Close();\r\n' +
        '}\r\n' +
        '\r\n' +
        '/****************************************************************************/\r\n' +
        'function readFromFile(fileName) {\r\n' +
        '	var file = WshFSO.OpenTextFile(fileName, 1)\r\n' +
        '	var str = file.ReadAll();\r\n' +
        '	file.Close();\r\n' +
        '	return str\r\n' +
        '}\r\n' +
        '\r\n';
    FSOWriteToFile(dispatchSrc, file);
}  // writeRemoteServer

