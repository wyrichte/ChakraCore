/*********************************************************************************/
/*                                  run.js                                       */
/*********************************************************************************/

/* Surprisingly, script does not have a convenient way to run a command and capture
   its output.   This module fixes that.  It turns out that there are a surprising
   number of options that you might want to use to control exactly how a command
   is run.  Thus this module has layered complexity.  At the simplest level there
   are three commands of interest

		runCmd 			Runs a command (throws on error).  Returns a 'run' 
                        structure which contains the output, exit code, etc.  
					    
		runCmdToLog		Just like runCmd, but IN ADDITION sends output to the 
						log.  This is VERY useful for debugging.

		runCmdToStream	Just like runCmd, but IN ADDITION sends output to a
						file stream (default stdout). 

	In practice, I recommend using runCmdToLog when calling anything that is 
    non-trivial.  The rationale here is that it is good to show the user what 
    is going on.   (It certainly makes debugging easier.)  By default commands
	will time out after 10 minutes (thus scripts can never hang indefinitely). 
	
	The 'run' structure that is returned by 'runCmd*' has the following properties:

		run.output		command output (stdout and stderr)
		run.startTime	time when the process started (msec from 1970)
		run.endTime		time when the process ended (msec from 1970)
		run.duration 	duration (msec) of run
		run.getPid()    the process's process id
		run.exitCode	the command's exit code
		run.terminated	the command was killed
		run.timedout	if defined, indicates how the run timed out (undefined means did not time out)
		ret.timeoutSec  the time this command is allowed to run
		ret.idleTimeout the time this command is allowed to be blocked 
		ret.done		true if the command has completed

	The commands above accept an 'options' parameter that allow you to control
    the behavior of the run.   These options can be created using the following APIs.
	Each of these qualifier APIs take an option parameter, so they can be chained
	together to mix and match whichever options you wish.  You can also create 
    a set of options, and pass this to a variety of 'runCmd' calls. 

	Qualifier APIs (these override the default behavior):

		runSetNoThrow 	specify don't throw on error (you check error code if desired)
		runSetTimeout   set timeout for run (default 10 minutes)
		runSetIdleTimeout sets time process can be continuously blocked before we kill it (default: infinity)
		runSetOutput  	caputure the output to a file instead of a string (undefined means no output)
		runSetEnv		specify an environment variable to be given to spawned process
		runSetCwd		specify a current working directory for the spawned process
		runSetInput		specify stdin string
		runSetDump		specify a command to take a dump on failure
		runSetNoShell	don't use 'cmd.exe' to spawn but create process directly
		runSet32Bit		Run in 32 bit WoW if possible (no-op on X86)
		runSet64Bit		Run in 64 bit window if possible (no-op on X86)
		runSetStream	specify a TextStream to send output to INCREMENTALLY
		runSetLog		specify that output also goes to the log

    *****************************************************************************
	                          ADVANCED APIS 

    The APIs above are good enough for 95% of uses.  However when you want to run
    more than one command at a time, you need something more flexible.   The 
    following APIs allow for this:

		runDo			specify a command to run, but don't start it.
		runPoll			check if a command is done (starts it if needed)
		runWait			Poll until the command is done.
		runTerminate	kill a command that is not yet complete.

	Using these APIs you can develop sophisticated concurrent apps.  A simple
    use of these are the following APIs:

		runCmds			run a ; separated list of commands simultaneously and 
 					    print a report when they all complete.

		runsDo			specify a list of commands to run but don't run them
		runsPoll		poll to see if all commands are done
		runsWait		wait until all commands are done

**/
// Author: Vance Morrison
// Date	1/1/2003
// Dependencies
//		log.js			for logging utilities
//		fso.js			for file system access (with good logging, and error messages)
//		util.js			for memberWiseClone
//		proc.js			for 'terminateChildren'
// 	

/*********************************************************************************/
var runModuleDefined = 1; 		// Indicate that this module exist

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");

if (WshShell == undefined)
	var WshShell = WScript.CreateObject("WScript.Shell");

if (Env == undefined)
	var Env = WshShell.Environment("PROCESS");

if (ScriptDir == undefined)
	var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

	// If you get an object expected error here, it is because you did not include log.js
var LogRun = logNewFacility("run");
// logSetFacilityLevel(LogRun, LogInfo100000); //DEBUG - this will log almost everything!


/******************************************************************************/
/* run the command 'command'.  return a 'run' structure that contains the 
   output exit code, etc.  Will throw if the command fails.  Will fail if a 
   timeout is exceeded (default 10 min).  If 'options' is present, it is used 
   to override the defaults (see runSet* functions) 
*/
function runCmd(command, options) {
	return runWait(runDo(command, options))
}

/******************************************************************************/
/* run the command 'command'.  return a 'run' structure that contains the 
   output exit code, etc.  Will throw if the command fails.  Will fail if a 
   timeout is exceeded (default 10 min). Dump the command and its output to 
   stdout. If 'options' is present, it is used to override the 
   defaults (see runSet* functions) 
*/
function runCmdAndDump(command, options) {
    WScript.Echo(command);
    var run = runCmd(command, options);
    WScript.Echo(run.output);
    return run;
}

/******************************************************************************/
/* run the command 'command'.  return a 'run' structure that contains the 
   output exit code, etc.  Will throw if the command fails.  Will fail if a 
   timeout is exceeded (default 10 min).  Send the output INCREMENTALLY to the 
   stream 'stream'.  If 'stream' is not specified, it defaults to StdOut.  If 
   'options' is present, it is used to override the defaults (see runSet* 
   functions) 
*/

function runCmdToStream(command, stream, options) {
	return runCmd(command, runSetStream(stream, options));
}

/******************************************************************************/
/* run the command 'command'.  return a 'run' structure that contains the 
   output exit code, etc.  Will throw if the command fails.  Will fail if a 
   timeout is exceeded (default 10 min).  Send the output INCREMENTALLY to log 
   (by default facility LogRun level LogInfo, which means that the output gets
   printed). If 'options' is present, it is used to override the defaults (see 
   runSet* functions) 
*/
function runCmdToLog(command, options) {
	return runCmd(command, runSetLog(LogRun, LogInfo, options));
}

/******************************************************************************/
/* creates a 'run' structure for running the command 'command', but does not 
   actually start the command running. If 'options' is present, it is used to 
   override the defaults (see runSet* functions) 
*/
function runDo(command, options) {
	logMsg(LogRun, LogInfo1000, "In runDo('", command, "', ", options, ")\n"); 

	var ret = {};
	ret.toString = function() { return this.command == undefined ? "ARunObj" : this.command; }
	ret._outputTo = "string";				// default caputure output as string
	ret.timeoutSec = 600;					// default timeout
	ret.getPid = function() { 
		if (!this._exec.Status) 
			return this._exec.ProcessID; 
		return undefined;
	}
	ret._startFtn = _runStart;
	ret._isDoneFtn = function(run) { 
		logMsg(LogRun, LogInfo100000, "runPoll: _isDoneFtn: checking exec, status = ", run._exec.Status, " exitcode = ", this._exec.ExitCode, "\n");
		if (run._exec.Status) {
			run.exitCode = this._exec.ExitCode;
			return true;	
		}
		return false;	
	}
	ret.isDoneRetries = 0;

	if (options != undefined) 
		memberWiseCopy(ret, options);

	ret.command = command;
	ret._exec = undefined;			// indicate that we have not started this command
	ret.output = "";
	ret.timedout = undefined;
	ret.terminated = false;
	ret.exitCode = undefined;
	ret.startTime = undefined;
	ret.done = false;

	return ret;
}

/******************************************************************************/
/******************  preferences for running the process **********************/

/******************************************************************************/
/* merge the run options 'options1' and 'options2' (produced by the runSet*)
   commands and return an option list that is the merge.  If both options1
   and options2 define a option, 'option1' has precedence.  */

function runMergeOptions(options1, options2) {

	var options = memberWiseCopy({}, options2);
	options = memberWiseCopy(options, options1);

		// we want to merge the environments variable by variable
	if (options1._env && options2._env) {
		newEnv = {};
		memberWiseCopy(newEnv, options2._env);

        for (var key in options1._env) {         // shallow copy the options
			newVal = options1._env[key];
				// if the variable has references to previous value
				// TODO generalize this to any variables in newEnv 
			if (options2._env[key] && typeof(newVal) == "string" && newVal.match(/%\w+%/))
				newVal = options1._env[key].replace(new RegExp("%" + key +"%", "ig"), options2._env[key]);

            newEnv[key] = newVal;
		}
		options._env = newEnv;
	}
	return options;
}

/******************************************************************************/
/* Create an object that indicates that 'dir' be the current working directory
   to be used for the run.  If 'options' is present all the options in
   this object are included in the returned options objects (allowing chaining)
*/
function runSetCwd(dir, options) {

	options = memberWiseCopy({}, options);
	options._cwd = dir;
	return options;
}

/******************************************************************************/
/* Create an object that indicates that environment variable 'varName' should
   be set to 'varValue' when running a command.  This options object can be 
   then be passed to 'runCmd'.  If 'options' is present all the options in
   this object are included in the returned options objects (allowing chaining)
   Any substrings in varValue of the form %VARNAME% will be substituted with the 
   actual value of the VARNAME in the environment AT THE TIME THE COMMAND IS 
   RUN (which may be on a remote computer).  
*/
function runSetEnv(varName, varValue, options) {

		// copy over the old environment
	var newEnv = {};
	if (options != undefined && options._env != undefined) 
		memberWiseCopy(newEnv, options._env);

		// update the new change
	newEnv[varName] = varValue;			// seting to undefined will remove a variable.
		
	options = memberWiseCopy({}, options);
	options._env = newEnv;
	return options;
}

/******************************************************************************/
/* Get the environment variable value for 'varName' out of the options object
   'options'.  This is a rarely used API.
*/
function runGetEnv(varName, options) {

	if (options == undefined || options._env == undefined)
		return "%" + varName + "%";
	return options._env[varName];
}

/******************************************************************************/
/* Create an object that indicates the command to be invoked when a run times
   out.  This is intended to caputure a dump.  %p in the 'dumpCmd' string 
   is expanded into the process ID of the command being run.   %m will 
   expand to the machine for this process ID (if the command is remote).  
   This options object can be be passed to 'runCmd'.  If 'options' is present 
   all the options in this object are included in the returned options objects 
   (allowing chaining)
*/
function runSetDump(dumpCmd, options) {
	options = memberWiseCopy({}, options);
	options._dumpCmd = dumpCmd;		// command to run.  %p will be expanded to process ID to dump
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the timeout for the command should 
   be 'timeoutSec' seconds (default is 600sec = 10 min).  This options object 
   can be be passed to 'runCmd'.  If 'options' is present all the options in
   this object are included in the returned options objects (allowing chaining)
*/
function runSetTimeout(timeoutSec, options) {		
	options = memberWiseCopy({}, options);
	if (timeoutSec != undefined) 
		options.timeoutSec = timeoutSec;
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the idle timeout for the command should 
   be 'idleTimeout' seconds.  If the process (or its children, or any orphans)
   consumed no CPU or IO for that time, the run is terminated.  This works by 
   polling so it can take as much as 'idleTimeout' to determine that run is 
   idle.  Also the granularity of CPU time is msec, which means that processes 
   that do very little (but not zero) work might be considered idle.  Generally,
   however, specifying 30sec works very well for a wide variety of runs.
   If 'options' is present all the options in this object are included in 
   the returned options objects (allowing chaining)
*/
function runSetIdleTimeout(idleTimeout, options) {		
	options = memberWiseCopy({}, options);
	options.idleTimeout = idleTimeout;		
	return options;
}

/******************************************************************************/
/* Create an object that indicates that 'input' should be sent to the run
   as standard input.  Due to buffering issues, input is currently limited
   to 1K chars.  If 'options' is present all the options in this object are 
   included in the returned options objects (allowing chaining)
*/
function runSetInput(input, options) {		
	options = memberWiseCopy({}, options);
	options._input = input;						// give this input to process
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the command should be run in the 
   32 bit X86 emulation environment (WOW).   This is a no-op on X86
*/
function runSet32Bit(options) {
	options = memberWiseCopy({}, options);
	options._32bit = true;
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the command should be run in the 
   native 64 bit window. This is a no-op on x86.
*/
function runSet64Bit(options) {
	options = memberWiseCopy({}, options);
	options._64bit = true;
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the run output should be sent to the
   file 'outputFileName'.  Turning on this option means that the .output
   field will NOT be updated.  If this value is undefined this means that the
   output should be ignored.  Using this option improves efficiency.  
   If 'options' is present all the options in this object are included in 
   the returned options objects (allowing chaining)
*/
function runSetOutput(outputFileName, options) {		
	options = memberWiseCopy({}, options);
	if (outputFileName == undefined)
		options._outputTo = "none";
	else {
		options._outputTo = "file";
		options._stdoutName = outputFileName;
	}
	return options;
}

/******************************************************************************/
/* Create an object that sets the expected return value from the command.  
   If not set, the default is zero. If 'options' is present all the options 
   in this object are included in the returned options objects (allowing chaining)
*/
function runSetExpectedReturnCode(expRetCode, options) {		
	options = memberWiseCopy({}, options);
	options._expRetCode = expRetCode;
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the run should not throw an exception
   if the run fails (you can still check run.exitCode).  If 'options' is 
   present all the options in this object are included in the returned options 
   objects (allowing chaining)
*/
function runSetNoThrow(options) {		
	options = memberWiseCopy({}, options);
	options._noThrow = true;					// don't throw an exception on failure
	return options;
}

/******************************************************************************/
/* Create an object that indicates that the run should send its output to 
   the TextStream 'stream' incrementally, as it is generated.  If 'stream'
   is not present, standard output is assumed.  If 'options' is 
   present all the options in this object are included in the returned options 
   objects (allowing chaining)
*/
function runSetStream(stream, options) {		
	if (stream == undefined)
		stream = WScript.StdOut;

	options = memberWiseCopy({}, options);
	options._stream = stream;
	return options;
}

/*****************************************************************************/
/* Create an object that indicates that the run should send its output to 
   the debugging log (see log.js) as it is generated.  The data is send to
   'facility' at level 'level'.   If 'options' is present all the options in 
   this object are included in the returned options objects (allowing chaining)
*/
function runSetLog(facility, level, options) {		
	if (facility == undefined)
		facility = LogRun;
	if (level == undefined)
		level = LogInfo;

	options = memberWiseCopy({}, options);
	options._logFacility = facility;
	options._logLevel = level;
	return options;
}

/*****************************************************************************/
/* Normally the command string is passed to 'cmd /c' for interpretation 
   (that way file redirection, pipes, and primtive commands like 'dir' work).
   However this is inefficient, and can get in the way.  Indicate that
   the command should be passed directly to 'CreateProcess' This option 
   is accumulated on 'options' if that parameter is present.  

   Note due to a deficiency in the TextStream class (where it blocks on
   pipes), you may ONLY use this option if either you are ignoring the output
   or you guarantee that the output is less than 4K (amount output before
   pipe blocks).  Otherwise the process will block and the run will timeout).  
*/

function runSetNoShell(options) {
	options = memberWiseCopy({}, options);
	options._noShell = true;					// don't spawn cmd.exe command shell 
	return options;
}

/*****************************************************************************/
/* call runPoll repeatedly until the command completes.  */

function runWait(run) {
	while(!runPoll(run))
		WScript.Sleep(200);
	return run;
}

/*****************************************************************************/
/* given a run object (returned from runDo, not runCmd), wait until the
   process has been idle for 'idleTime' seconds (default 2). returns 
   true if the object is idle (false implies the process died or timed
   out */

function runWaitForIdle(run, idleTime) {
   
    if (idleTime == undefined)
        idleTime = 2;      

    if (runPoll(run))
        return false;
    
    var pid = run.getPid();
    var wbem = procGetWMI();
    var prevTree = procGetProcsAsTree(wbem);
    var i = 0;   
    var first = true;
    var totalIdleTime = 0;
    do {
        i++;
        WScript.Sleep(1000);   // We call runPoll to poll for completion, once per second
        if (i > idleTime) {
            totalIdleTime += idleTime;
            var nextTree = procGetProcsAsTree(wbem);
            if (procProcTreeIdle(prevTree, nextTree, pid, true)) {
                if (!first) {
                    logMsg(LogRun, LogInfo, "runWaitForIdle: command idle for ", idleTime, " secs.\n");
                    return true;
                }
                else {
                    logMsg(LogRun, LogWarn, "runWaitForIdle: idle in first try!  This is so unusal that we ignore it!\n");
                }
            }
            logMsg(LogRun, LogInfo, "runWaitForIdle: checking for idle after ", totalIdleTime, " secs...  Still not idle\n");
            prevTree = nextTree;
            i = 0;
            first = false;
        }
    } while(!runPoll(run));

    logMsg(LogRun, LogInfo, "runWaitForIdle: command completed before it idled\n");
    return false;
}

/*****************************************************************************/
/* given a run object, start it if necessary, and check on its status and 
   update the run structure (including output, exit code, etc.) Return 'true' 
   if the run is done (poll no longer needs to be called) */

function runPoll(run) {
	logMsg(LogRun, LogInfo100000, "In runPoll(", run, ") {\n"); 

		// Have we started the operation at all? 
	if (!run.done)  {
	    if (run.startTime == undefined) {	// have we started?
	        try {
	            run._startFtn(run); // actually start up the process;
	        } catch (e) {
	            logMsg(LogRun, LogError, "Run ", run, " failed to start.\n", e.description, "\n");
	            run.exitCode = 254; // failed to start
	            run.done = 1;
	        }

	        if (!run.done) {
	            if (run._logFacility)
	                if (!run._deferLogging) 			// are we logging?
	                _runLogStart(run)

	            run.endTime = run.startTime = new Date().getTime();
	            run.duration = 0;
	            logMsg(LogRun, LogInfo1000, "runPoll: Setting startTime = ", run.startTime, "\n");

	            if (run.idleTimeout)
	                run._nextIdleCheck = run.startTime + 1000; 	// get our baseline 1 second into the program
	        }
		}

		var now = run.endTime = new Date().getTime();
		run.duration = now - run.startTime;
		logMsg(LogRun, LogInfo100000, "runPoll: polling for completion, now = ", now, " duration = ", run.duration, " timeout ", run.timeoutSec, "\n");

		// should we be ending the command?
		if (!run.done) {
		    try {
		        if (run._isDoneFtn(run))
		            run.done = true;
		        run.isDoneRetries = 0;
		    } catch (e) {
    		    logMsg(LogRun, LogWarn, "An exception was thrown while checking if run ", run, " completed.\n", e.description, "\n");
    		    if (run.isDoneRetries > 3) {
    		        logMsg(LogRun, LogError, "Failed to determine whether run ", run, " completed three times.\nTreating as a failure.\n");
    		        runTerminate(run);
    		        run.exitCode = 255;
    		        run.done = true;
    		    } else {
    		        run.isDoneRetries++;
    		    }
		    }
		}
		
		if (run.done) {}
		else if (run.duration > run.timeoutSec * 1000) {
			logMsg(LogRun, LogWarn, "Run ", run, " Times out after ", run.timeoutSec, " sec at ", new Date(now), "\n");
			run.timedout = run.timeoutSec + " sec timeout expired";
			runTerminate(run);
			run.exitCode = 253;
		}
		else if (now > run._nextIdleCheck) {
			logMsg(LogRun, LogInfo10000, "runPoll: in idle check\n");
			run._nextIdleCheck = now + run.idleTimeout * 1000;
			try {
				if (!run._procs) {
					if (!run._wbem)
						run._wbem = procGetWMI(run.machine);
					run._procs = procGetProcsAsTree(run._wbem);
				}
				else {
					var newProcs = procGetProcsAsTree(run._wbem);
					if (procProcTreeIdle(run._procs, newProcs, run.getPid())) {
						logMsg(LogRun, LogError, "Run ", run, " has been idle for ", run.idleTimeout, " sec\n");

						run.timedout = run.idleTimeout + " sec idle timeout expired";
						runTerminate(run);
						run.exitCode = 252;
					}
					else 
						run._procs = newProcs;
				}
			} catch (e) {
				logMsg(LogRun, LogError, "****** Internal error, exception thrown while inspecting procs:", e.description);
			}
		}
			
			// process any output from the process 
		if (run._outputTo == "string" && run._stdoutName && run._stdout == undefined && FSOFileExists(run._stdoutName)) {
			logMsg(LogRun, LogInfo10000, "runPoll: opening ", run._stdoutName, "\n");
			run._stdout = FSOOpenTextFile(run._stdoutName, 1);
		}
		if (run._stdout != undefined && !run._stdout.AtEndOfStream) {
			var out = run._stdout.ReadAll();
			logMsg(LogRun, LogInfo100, "runPoll: command output {\n", out, "}\n");
			run.output += out;
			if (run._logFacility && !run._deferLogging)
				logMsg(run._logFacility, run._logLevel, out);
			if (run._stream) 
				run._stream.Write(out);
		}

			// If we are done, do final processing
		if (run.done)  {
			logMsg(LogRun, LogInfo10, "Run ", run, " completes. ExitCode: ", run.exitCode, "\n");
			if (!run.terminated)
				_runCleanup(run);

			if (run._logFacility) {
				if (run._deferLogging) {
					_runLogStart(run) 
					logMsg(run._logFacility, run._logLevel, run.output);
				}
				var status;
				
				// Check the exit code against the expected return code value, if the latter
				// is specified	
				if ((run.exitCode == 0) || ((run._expRetCode != undefined) && (run.exitCode == run._expRetCode)))
					status = "SUCCESS"
				else  {
					if (run.timedout) {
						if (run.timedout.match(/idle timeout/i))
							status = "IDLE_TIMEOUT(" + run.idleTimeout + " sec)";
						else 
							status = "TIMEOUT(" + run.timeoutSec + " sec)";
					}
					else 
						status = "FAILED(" + hex32(run.exitCode) + ")";
					if (run._noThrow)
						status += " (NOTHROW)";
				}
				/* { */ logMsg(run._logFacility, run._logLevel, "} = ", status, "     duration = ", (run.duration / 60000).toFixed(2) , " min\n"); 
			}

			if (!run._noThrow && run.exitCode != 0) {
				var explain = "non-zero exit code";
				if (run.timedout)
					explain = run.timedout;

				var out = run.output;
				if (out.length >= 220)
					out = out.substr(0, 110) + " ... " + out.substr(out.length-110, 110);
				out = out.replace(/[\r\n]+/g, " ");
				throwWithStackTrace(new Error(run.exitCode, "\n   Command Failed " + explain + "\n" +
								   "      Cmd: " + run.command + "\n" + 
								   "      Exit Code: " + hex32(run.exitCode) + "\n" +
								   "      CmdOut: '" + out + "'"));
			}
		}
	}

	logMsg(LogRun, LogInfo100000, "} runPoll() done=", run.done, " exit=", run.exitCode, "\n");
	return run.done
}

/*****************************************************************************/
/* print out the information that happens at the start of a commmand if logging
   is turned on */

function _runLogStart(run) {

	if (run._env) {
		for (var varName in  run._env) {
			var varVal = run._env[varName];
			logMsg(run._logFacility, run._logLevel, "RUNCMD: SET ", varName, "=", varVal, "\n");
		}
	}
	if (run.machine)
		logMsg(run._logFacility, run._logLevel, "RUNCMD: MACHINE ", run.machine, "\n");
	if (run._cwd)
		logMsg(run._logFacility, run._logLevel, "RUNCMD: CD ", run._cwd, "\n");
	if (run._32bit)
        logMsg(run._logFacility, run._logLevel, "RUNCMD: Running command under 32 bit CMD shell\n");

    if (run._outputTo == "none")
        logMsg(run._logFacility, run._logLevel, "RUNCMD: output discarded\n");
    else if (run._outputTo == "file")
        logMsg(run._logFacility, run._logLevel, "RUNCMD: output redirected to ", run._stdoutName, "\n");

	logMsg(run._logFacility, run._logLevel, "RUNCMD: ", run, " {\n");  // }
}

/*****************************************************************************/
/* Starts the process associated with the run object */

function _runStart(run) {

    
    var cmdBase = run.command;
	
		
    if (run._input && run._input != "")
    {
        // Input redirection is using the '<'notation on the commandline, which is really a feature of cmd.exe.
        // So this only works if we're running via cmd.exe, and so won't work in the noShell case.
        if (run._noShell)
        {
            // Perhaps we could work around this if we supplied the input via a handle.
            throwWithStackTrace(new Error(1, "Can't supply Input (runSetInput) when running as noShell (runSetNoShell)."));
            
            // This is the old way of providing input. It works for small sizes. 
            // This must be done after the WshShell.Exec() function (since we need an _exec to set the handle on).
            // There seems to be some bug here making this work with the noShell case. If we can fix that, then
            // we could use this as a way to redirect input for noShell.
	        /*
	        if ((run._input) && (run._input != ""))  {
		        logMsg(LogRun, LogInfo1000, "Writing input string '", run._input, "'\n");
		        if (run._input.length > 3000)		// We don't want to block on the write below so the data has to fit in the pipe
			        throwWithStackTrace(new Error(1, "Error currently input is limited to 3K (can be lifted)"));
		        run._exec.StdIn.Write(run._input);
	        }
	        run._exec.StdIn.Close();
            */
        }
        else
	    {
	        // Redirect through a file. runCleanup() will delete this file
	        run._inputFileName = FSOGetTempPath() + ".run.input";
            FSOWriteToFile(run._input, run._inputFileName);
        
            // Another DOS quirk: Input redirection apparently must come before out redirection.     
            logMsg(LogRun, LogInfo1000, "Writing input string (len=" + run._input.length + ") '", run._input, "'\n"); 
            logMsg(LogRun, LogInfo1000, "Redirecting input through file '" + run._inputFileName+ "'\n"); 
            cmdBase += " < " + run._inputFileName;
    	   
            // Don't call run._exec.StdIn.Close(). That will conflict with the input piping.
	    }
    }
	
	var cmd = cmdBase;
	
    run._execOutput = (run._outputTo != "none");    // execOutput is true if the Exec cmd itself has output
    if (!run._noShell) {
        run._execOutput = false;                    // one way or the other if we have the shell, we don't care about the Exec output
        logMsg(LogRun, LogInfo1000, "runPoll: using cmd.exe\n");
        cmd = "cmd.exe";
        if (run._32bit) {
            var wowCmd = Env("SystemRoot") + "\\SysWow64\\cmd.exe";
            if (FSOFileExists(wowCmd)) 
                cmd = wowCmd;
            else
                run._32bit = undefined;
        } else if (run._64bit) {
            var nativeCmd = Env("SystemRoot") + "\\SysNative\\cmd.exe";
            if (FSOFileExists(nativeCmd))
                cmd = nativeCmd;
            else
                run._64bit = undefined;
        }
        cmd = cmd + " /c " + cmdBase;
        if (run._outputTo != "none") {
            if (run._stdoutName == undefined) 
                run._stdoutName = FSOGetTempPath() + ".run.output";
            cmd += " 1> \"" + run._stdoutName + "\" 2>&1";
        }
    }
	if (run._outputTo == "file")
		logMsg(LogRun, LogInfo10, "redirecting output to ", run._stdoutName, "\n");

		// update the environment as indicated
	var oldEnv = undefined;
	if (run._env) {
		oldEnv = {};
		for (var varName in  run._env) {
			oldEnv[varName] = Env(varName);
			var varVal = run._env[varName];
        	while (typeof(varVal) == "string" && varVal.match(/%(\w+)%/)) {
				var key = RegExp.$1;
				varVal = RegExp.leftContext + Env(key) + RegExp.rightContext;
			}
			if (varVal != undefined) {
				Env(varName) = varVal;
				logMsg(LogRun, LogInfo10, "ENV set ", varName, "=", varVal, "\n");
			}
			else if (Env(varName) != '') {
				logMsg(LogRun, LogInfo10, "ENV set ", varName, "=\n");
				Env.Remove(varName);
			}
		}
	}
	var oldCwd = undefined;
	if (run._cwd) {
		logMsg(LogRun, LogInfo100, "setting CWD ", run._cwd, "\n");
		oldCwd = WshShell.CurrentDirectory;
		WshShell.CurrentDirectory = run._cwd;
	}

	logMsg(LogRun, LogInfo1000, "runPoll: EXEC ", cmd, "\n");
	run._exec = WshShell.Exec(cmd);

		// restore the original environment
	if (oldCwd) {
		logMsg(LogRun, LogInfo100, "restoring CWD ", oldCwd, "\n");
		WshShell.CurrentDirectory = oldCwd;
	}
		
	if (oldEnv) {
		for (var varName in  oldEnv) {
			var varVal = oldEnv[varName];		// TODO - because Env return empty string for undefined vars,
			if (varVal != '') {					// I have a bug with distigushing this case
				logMsg(LogRun, LogInfo100, "ENV restoring ", varName, "=", varVal, "\n");
				Env(varName) = varVal;
			}
			else if (Env(varName) != '') {
				logMsg(LogRun, LogInfo100, "ENV restoring ", varName, "=\n");
				Env.Remove(varName);
			}
		}
	}


	if (!run._execOutput) {
		logMsg(LogRun, LogInfo1000, "runPoll: closing exec output streams\n");
		run._exec.StdOut.Close();
		run._exec.StdErr.Close();
	}
}

/*****************************************************************************/
/* kills the process associated with the run and all its children */

function runTerminate(run, silent) {
	logMsg(LogRun, LogInfo1000, "In runTerminate('", run, "') {\n"); 

	var pid = run.getPid();
	if (pid) {
		if (run._dumpCmd && !silent) {
			var cmd = run._dumpCmd.replace(/%p/, pid).replace(/%m/, run.machine ? run.machine : Env("COMPUTERNAME"));
			logMsg(LogRun, LogWarn, "runTerminate: Dumping with command: ", cmd, "\n");
			var runOpts = runSetEnv("PATH", "%PATH%;" + ScriptDir,
					      runSetNoThrow(
						  runSetTimeout(900)));
			var exitCode = runCmdToLog(cmd, runOpts).exitCode;
			if (exitCode != 0)
				logMsg(LogRun, LogError, "runTerminate: CreateDump fails with exit code ", exitCode, "\n");
		} 
		if (!silent)
			logMsg(LogRun, LogInfo, "Terminating: ", run, "\n");

                do {
	                var retry = true;
			if (!run._wbem) {
				retry = false;
				logMsg(LogRun, LogInfo1000, "Grabbing new WMI\n");
				run._wbem = procGetWMI(run.machine);
	                }
                        

			try {
				procKillAll(pid, run._wbem, undefined, silent);
				break;
			}
			catch (e) {
				if (!retry) {
					logMsg(LogRun, LogInfo1000, "Already got a fresh WMI, but still got an exception:", e, "\n");
					logMsg(LogRun, LogInfo1000, "Giving up\n");
					logMsg(LogRun, LogWarn, "Could not kill process ID for '", run, "' due to exception: '", e, "'\n");
                                        break;
				}
				// If the WMI service was stopped between when we cached run._wbem
				// and now, we get an exception:
				//    SWbemServicesEx: The object invoked has disconnected from its clients.
				// The work-around is to re-cache run._wbem
				logMsg(LogRun, LogInfo1000, "Got an exception:", e, "\n");
				logMsg(LogRun, LogInfo1000, "Trying again\n");
				run._wbem = null;
			}
		} while (true);
	}
	else 
		logMsg(LogRun, LogWarn, "Could not find process ID for ", run, " (died already?)\n");

	if (run._onTerminate)
		run._onTerminate(run);

	run.done = true;
	run.terminated = true;
	run.exitCode = 254;
	_runCleanup(run);

	logMsg(LogRun, LogInfo1000, "} runTerminate()\n");
}

/*****************************************************************************/
/* an internal function that cleans up scarse resources */

function _runCleanup(run) {

	run._wbem = undefined;
	run._procs = undefined;
	if (run._exec != undefined) {
			// Note that we only check for exec output here in cleanup.
			// The reason is that we can't check if there is any output
			// without risking blocking if there is no output.  We CAN'T
			// block so we only try to read at the end.  This can lead
			// to timeouts if the command generates too much output 
			// (see runSetNoShell)
		if (run._execOutput) {
			var out = run._exec.StdOut.ReadAll() + run._exec.StdErr.ReadAll();
			if (run._outputTo == "string")
				run.output += out;
			else if (run._stdoutName)
				FSOWriteToFile(out, run._stdoutName, true);
		}
		run._exec = undefined;
	}

	if (run._stdout != undefined) {
		run._stdout.Close();
		run._stdout = undefined;
	}
	if (run._stdoutTo != undefined) {
		run._stdoutTo.Close();
		run._stdoutTo = undefined;
	}
	if (run._outputTo == "string" && run._stdoutName && !run._statusFile) {
		if (FSOFileExists(run._stdoutName)) {
			logMsg(LogRun, LogInfo100000, "_runCleanup: deleting file ", run._stdoutName, "\n");

				// Give the other process a chance to die before trying to delete the file
			if (run.terminated)
				WScript.Sleep(200); 
			var retry = 3;
			var e;
			for(;;) {
				try { 
					FSODeleteFile(run._stdoutName, true /* force retry on failure */); 
					break; 
				} catch(e) { 
					--retry;
					if (retry <= 0) {
						logMsg(LogRun, LogWarn, "_runCleanup: Leaving trash. Could not delete '", run._stdoutName, "' error = ", e.description, "\n");
						logMsg(LogRun, LogWarn, "This happen if a process whose output and errors were not redirected is still alive\n");
						break;
					}
					WScript.Sleep(300); 
				}
			}
		}
		run._stdoutName = undefined;
	}
	
	if (run._inputFileName != undefined)
	{
	    if (FSOFileExists(run._inputFileName))
	    {
	        logMsg(LogRun, LogInfo100000, "_runCleanup: deleting input file ", run._inputFileName, "\n");
	        try
	        {
	            FSODeleteFile(run._inputFileName);
	        }
	        catch(e)
	        {
	        	logMsg(LogRun, LogWarn, "_runCleanup: Leaving trash. Could not delete input file '", run._inputFileName, "' error = ", e.description, "\n");
                logMsg(LogRun, LogWarn, "This happen if a process whose input was redirected is still alive\n");
	        }
	    }
	    run._inputFileName = undefined;
	}
}



/*****************************************************************************/
/* 'Run each command in the semicolon list of commands 'commands' 
   simultaneously.  'numSimultaneous (default 3) indicates how many should
   run at once.  After the commands run, it prints a report on what happened,
   including the output of each command.  returns 0 if everything succeeded.
   If a command fails, the others are allowed to proceed.  You always get
   the output of all the commmands (but only after everyone is done)
*/
function runCmds(commands, numSimultaneous, runOpts) {

	if (numSimultaneous == undefined)
		numSimultaneous = 3;
	if (runOpts == undefined)
		runOpts = runSetLog(LogRun, LogInfo);
	
	if (typeof(commands) == "string")
		commands = commands.split(/;/)

	var e = undefined;	
	var runs = runsDo(commands, runOpts);
	runs = runsWait(runs, numSimultaneous);

	logMsg(LogRun, LogInfo, "runCmds: returning successfully\n");
	return runs;
}

/*****************************************************************************/
/* creates an array of run objects corresponding to the command array 
   'commands'.  Each run object has 'options' set */
   
function runsDo(commands, runOpts) {

	logMsg(LogRun, LogInfo1000, "runsDo()\n"); 
	var runs = [];
	for(var i = 0; i < commands.length; i++) {
		var run = runs[i] = runDo(commands[i], runOpts);
		run._deferLogging = true;		// indicate logging shoudl happen late
	}
	return runs;
}
		
/*****************************************************************************/
/* given an array of run stuctures, run them all, at most 'numSimultaneous' at 
   once.   Returns 'runs' */

function runsWait(runs, numSimultaneous) {

	var exceptions = [];;
	for(;;) {
		try {
			if (runsPoll(runs, numSimultaneous) <= 0)
				break;
			WScript.Sleep(200);
		} catch(e) {
			logMsg(LogRun, LogError, "Exception Thrown : ", e.description, "\nWaiting for other commands to complete...\n");
			exceptions.push(e);
		}
	}
	if (exceptions.length > 0)
		throwWithStackTrace(exceptions[0]);
	return runs;
}

/*****************************************************************************/
/* poll all the runs in 'runs' insuring that at no more than 'numSimultaneous'
   are running at once.  returns the number of tasks still running 
*/
function runsPoll(runs, numSimultaneous) {

	logMsg(LogRun, LogInfo1000, "In runsPoll([", runs, "], ", numSimultaneous, ") {\n");

	var e = undefined;
	var curSimultaneous = 0;
	for(var i = 0; i < runs.length; i++) {
		logMsg(LogRun, LogInfo1000, "runsPoll: index ", i, " curSimultaneous = ", curSimultaneous, "\n");
		if (curSimultaneous >= numSimultaneous)
			break;
		if (!runPoll(runs[i]))		// if its not done
			curSimultaneous++;		// remember that it is not done. 
	}

	logMsg(LogRun, LogInfo1000, "} runsPoll()\n");
	return curSimultaneous;
}

