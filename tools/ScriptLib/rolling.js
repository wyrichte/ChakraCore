/******************************************************************************/
/*                                rolling.js                                  */
/******************************************************************************/

/* The task infrastructure defined in task.js is about providing the ability
   to name a large chunk of work to do, describe the dependancies among these
   chunks, and produce reasonable reports from these tasks.   

   The rolling automation defined here builds on this by defining wrapper logic
   that will run a particular task on a series of inputs to form a series of
   outputs.  For example, if you have a series of builds that represent checkins,
   the rolling process can run a battery of tests on each of these builds to see 
   where failures are introduced.   

   The logic uses the heuristic that we are most interested in the most recient 
   build.  Thus it processes this one first.  It then works its way back to 
   builds with smaller build numbers to a given limit (default is 20).  

   The logic was build in such a way that it is allowable that many machines can
   be processing the same list of builds.  Thus the first machine takes the 
   most recient build, then next machine tries to take the first build, but 
   notices that it is already been taken, so moves on to the next most recient,
   etc.  In this way the process naturally scales.  The more machines you have
   the faster you get a complete set of test runs.  If the average rate of 
   generating new builds is less than the average rate that the whole set of
   workers is processing the builds, then you will keep up and have a complete 
   set of information.  Typically you keep adding machines until this is true.   
    
   The value that this code adds is

		1) The algorithm for choosing the next build to process and the locking
           logic that allows multiple workers to participate in processing
  		   the work. 
		2) The reporting logic that shows the list of builds the the results
 		   for each of the test runs.  

   To use the rolling infrastructure, you need to define a task that represents
   what you want done for each build.  An good example is the _devBVTRollingTask.
	
   
Typically this rolling task is composed of three parts

	1) A task that sets up the machine for the main task.  This is typically
       something that syncs SD or otherwise copies files and installs things.
	   Tasks should never assume any setup on the machine they run on, but
       copy or install what they need.  

	2) The main task.  Typically a set of tests to run 
	
	3) A 'results' task.  This task confirms that the main task completed
       (there may be failures, but the rolling system did what it was supposed
       to do.  This task then marks the job as 'completed' so that it will
       not be attempted again.   Thus if a rolling task fails in the middle 
       it will be retried.  Only a complete run will allow the rolling system
       to move on.  
*/

// AUTHOR: Vance Morrison 
// DATE: 11/1/2003
/******************************************************************************/

var rollingModuleDefined = 1; 						// Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include ClrAutomation.js");
if (!clrTaskModuleDefined) throw new Error(1, "Need to include clrTask.js");

var LogRolling = logNewFacility("rolling");

if (WshShell == undefined)
	var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
	var Env = WshShell.Environment("PROCESS");

// logSetFacilityLevel(LogRolling, LogInfo100000);

/**********************************************************************/
/* entry point for starting a rolling run.  taskName is the name of the 
   task to run as a rolling process. (This routine is generic and 
   can do any rolling process) For example doRolling test.devBVT@x86chk
   will do a rolling run of the devBVTs. */

function doRollingRun(taskName, outDirBase, srcBase, jobDirBase) {

	if (taskName == undefined)
		taskName = "rolling.test.devBVT@x86chk";
	if (outDirBase == undefined)
		outDirBase = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests";
	if (srcBase == undefined)
		srcBase = "c:\\vbl\\lab21S";
	if (jobDirBase == undefined)
		jobDirBase = outDirBase.match(/(.*)\\[^\\]*/)[1] + "\\builds\\PerCheckin";

	logCall(LogRolling, LogInfo, "doRollingRun", arguments);
	logMsg(LogRolling, LogInfo, "doRollingRun: jobDirBase = ", jobDirBase, "\n");

		// This allows you to pause the automation in the event of a scripting error
	var freezeFile = outDirBase + "\\Freeze.txt";
	while (FSOFileExists(freezeFile)) {
		logMsg(LogRolling, LogInfo10, "doRollingRun: File ", freezeFile, " exists, Sleeping\n");
		WScript.Sleep(60000);
		}

	var rollingTask = taskNew("rollingRun." + Env("COMPUTERNAME"), 
								 "runjs _rollingRun " + taskName + " " + outDirBase + " " + jobDirBase + " " + srcBase);

    var env = {};
    env["taskClean"] = "taskClean";
    
    return tasksRun([rollingTask], outDirBase, srcBase, env);
}

/**********************************************************************/
/* _rollingRun is a task that selects a build that needs to have
   a rolling run done on it, spawns off the task, and generates
   the rolling report.  Tasks to be run in the rolling run must be
   placed in the '_rollingTask' table.  'taskName' is the name of
   task to run. 
 */
   
function _rollingRun(taskName, outDirBase, jobDirBase, srcBase) {

	logCall(LogRolling, LogInfo10, "_rollingRun", arguments, "{");

	var numKeep = 50;
		// It is important that the maxHistory (the number of builds we seach back before we
		// decide a .1 test run is more interesing) be less than half the number kept, since every
		// build could have a .1 run with it.  Otherwise, you could be fighting over
		// who deletes the record while some other machine is trying to create it. 
	var maxHistory = (numKeep / 2) - 2;
	logMsg(LogRolling, LogInfo, "HouseCleaning, removing so that we have at most ", numKeep, " builds\n");
	_deleteOldestDirs(outDirBase, "", numKeep); 

	var task = _rollingFindTask(taskName, _rollingTasks);

	logMsg(LogRolling, LogInfo, "_rollingRun: Determining which build to run\n");
	var resultsFile = task.name + ".results.txt";
	var rollInfo = _getRollingBuild(jobDirBase, outDirBase, task.name, maxHistory);

		// Pass on additional information to worker tasks
	var env = {};
  env["taskClean"] = "taskClean";
	env["rolling.outDirBase"] = outDirBase;
	env["rolling.jobDirBase"] = jobDirBase;
	env["rolling.resultsFile"] = rollInfo.outDir + "\\" + resultsFile;
	env["rolling.changeNum"] = rollInfo.changeNum;
	env["rolling.jobDir"] = rollInfo.jobDir;

	logMsg(LogRolling, LogInfo, "_rollingRun: running the main task\n");
	var ret = tasksRun([task], rollInfo.outDir, srcBase, env);
	
	logMsg(LogRolling, LogInfo, "_rollingRun: Generating report to ", outDirBase, "\n");
	_rollingTestReport(outDirBase, jobDirBase);		// generate the final rolling report

	logMsg(LogRolling, LogInfo, "} _rollingRun() = ", ret, "\n");
	return ret;
}

/**********************************************************************/
/* find a good build to run, this is defined as the latest build
   that does not already have output in outDirBase.  Note that
   if the rolling build system has gone back 'maxHistory' builds, then 
   it will start repeating the most recent one instead.  
   'taskName' is the name of the task that is being run in the
   rolling process */

function _getRollingBuild(jobDirBase, outDirBase, taskName, maxHistory) {

	logMsg(LogRolling, LogInfo, "_getRollingBuild(", jobDirBase, ", ", outDirBase, ", ", taskName, ", ", maxHistory, ") { \n");

	var jobDirs = FSOGetDirPattern(jobDirBase, /[\.\d]+/);
	jobDirs.sort(function(x, y) { return (numSuffix(y, "") - numSuffix(x, "")); });
	var ret = undefined;
	var foundDir = false;
	var changeNum = undefined;
	for (j = 0; j < jobDirs.length && j < maxHistory; j++)  {
		var jobDir = jobDirs[j];
		foundDir = false;
		if (jobDir.match(/\\(\d+)$/)) {
		    foundDir = true;
		    changeNum = RegExp.$1;
			logMsg(LogRolling, LogInfo, "_getRollingBuild: j=", j, " found SNAP jobDir: ", jobDir, ", changeNum: ", changeNum, "\n");
		}
		else if(jobDir.match(/\\(\d+\.\d+)$/)) {
			foundDir = true;
			changeNum = RegExp.$1;
			logMsg(LogRolling, LogInfo, "_getRollingBuild: j=", j, " found official build jobDir: ", jobDir, ", build label: ", changeNum, "\n");
		}
		if(foundDir) {
			var outDir = outDirBase + "\\" + changeNum;

				// Always make the folder, so we know when there are builds with no test results
			if (!FSOFolderExists(outDir)) {
				FSOCreatePath(outDir);
				urlShortCutCreate(outDir + "\\jobDir.url", jobDir);
			}

			if (ret == undefined && _dirNeedsRun(outDir, taskName)) {
				ret = { jobDir:jobDir, outDir: outDir, changeNum: changeNum };
				break;
			}
		}
	}

	if (!ret) {
		logMsg(LogRolling, LogInfo, "_getRollingBuild: did not find any dirs to do, trying to do a .1\n");
		if (!jobDirs.length || !jobDirs[0].match(/\\(\d+)$/)) 
			throw new Error(-1, "No job available to remove in " + jobDirBase + "\n");
		
		var changeNum = RegExp.$1;
		var i = 0;
		var outDir;
		do {
			i++;
			outDir = outDirBase + "\\" + changeNum + "." + i;
			if (i > 1) {		// make at most 1 extra runs of a build
				logMsg(LogRolling, LogInfo, "_getRollingBuild: Nothing to do, sleeping 5 min\n");
				WScript.Sleep(5*60*1000);	
				throw new Error(1, "There is nothing to do");
			}
		} while (FSOFolderExists(outDir) && !_dirNeedsRun(outDir, taskName));
		ret = { jobDir: jobDirs[0], outDir: outDir, changeNum: changeNum };
	}
	
	logMsg(LogRolling, LogInfo, "} _getRollingBuild: RETURNING jobDir ", ret.jobDir, " outDir: ", ret.outDir, "\n");
	return ret
}

/**********************************************************************/
/* returns true if 'outDir' looks like it did not complete or in progress */

function _dirNeedsRun(outDir, taskName) {

	logMsg(LogRolling, LogInfo, "_dirNeedsRun(", outDir, ", ", taskName, ") {\n");

	var resultsFile = outDir + "\\" + taskName + ".results.txt";

	if (FSOFileExists(resultsFile))  { // {
		logMsg(LogRolling, LogInfo, "} _dirNeedsRun: results file: ",resultsFile, " exists, done\n");
		return false;
	}

	var pat = new RegExp("^" + taskName + "\\.\\d+\\.status\\.log$", "i");
	var statusFiles = FSOGetFilePattern(outDir, pat);
	if (statusFiles.length > 0) {
		var statusFile = statusFiles[statusFiles.length-1];
		logMsg(LogRolling, LogInfo, "_dirNeedsRun: status file ", statusFile, " exists\n");

		statusFile.match(/(\d+)\.status\.log$/i, "lock");
		var retry = RegExp.$1;

		logMsg(LogRolling, LogInfo, "_dirNeedsRun: retry = ", retry, "\n");
		if (retry >= 5) { // {
			logMsg(LogRolling, LogInfo, "} _dirNeedsRun: have retried 5 times, giving up\n");
			return false;
		}
	}

	var lockFile = outDir + "\\" + taskName + ".lock";
	try {
		WshFSO.OpenTextFile(lockFile, 8, true).Close();
	} catch(e) { // {
		logMsg(LogRolling, LogInfo, "} _dirNeedsRun: Log file ", lockFile, " is locked\n");
		return false;
	}
	
	var statusFiles = FSOGetFilePattern(outDir, new RegExp("^" + taskName + "\\.\\d+\\.status\\.log$", "i"));

	logMsg(LogRolling, LogInfo, "} _dirNeedsRun: returning true\n");
	return true;
}

/**********************************************************************/
/* given a job report file, Find all the queue entries (submissions)
   that make it up */

function _parseSnapJobReport(jobReport) {

	var jobReportData = FSOReadFromFile(jobReport);
	var ret = {};
	ret.report = jobReport;
	ret.entries = [];
	if (jobReportData.match(/Job End Time: *([\d\/]+) +([\d:]+)/i)) {
		ret.endDate = RegExp.$1;
		ret.endTime = RegExp.$2;
	}

	while (jobReportData.match(/Queue Entry Description:.*?HREF='(.*?)\\(\d+\.\S+)\\[^\\]*xml'/im)) {
		ret.queueDir = RegExp.$1;
		var entryDir = RegExp.$2;
		jobReportData = RegExp.rightContext;
		ret.entries.push(entryDir);
	}

	return ret;
}

/**********************************************************************/
/* create a report for the rolling test runs
*/
function _rollingTestReport(outDirBase, jobDirBase, reportHtmlFile) {

	if (outDirBase == undefined) 
		outDirBase = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests";
	if (jobDirBase == undefined) 
		jobDirBase = "\\\\CLRMain\\public\\Drops\\Whidbey\\builds\\PerCheckin";
	if (reportHtmlFile == undefined) 
		reportHtmlFile = outDirBase + "\\report.html";	 // TODO probably a bad name

	var reportDir = reportHtmlFile.match(/(.*)\\[^\\]*$/)[1];

	logCall(LogRolling, LogInfo, "_rollingTestReport", arguments);

	var tempName = reportHtmlFile + "." + Env("COMPUTERNAME") + ".new"; 
	var htmlFile = FSOOpenTextFile(tempName, 2, true); 
	htmlFile.WriteLine("<HTML>"); 
	htmlFile.WriteLine("<BODY>"); 
	htmlFile.WriteLine("<H2> Rolling devbvt runs as of " + new Date() + "</H2>"); 
	htmlFile.WriteLine("Developers are required to run 'devbvt' with no arguments on a checked build before checking in.\n");
	htmlFile.WriteLine("Invoking devbvt in this way will run CheckinBVTs BuildBVTs as well as Dev unit tests.\n");
	htmlFile.WriteLine("We would liked to have had SNAP run these test before submitting the checkin, however this takes too long.\n");
	htmlFile.WriteLine("Instead we have a 'rolling test run' which constantly takes the latest x86chk build from SNAP\n");
	htmlFile.WriteLine("and runs these tests.   Below are the results.   If all devlopers are following the rules,\n");
	htmlFile.WriteLine("there are no interactions between checkins, and there is no non-determinism, then there should be no failures\n");
	htmlFile.WriteLine("during these runs\n");
	
	htmlFile.WriteLine("<P>");
	htmlFile.WriteLine("The list below show SNAP jobs and their cooresponding rolling test data.  NODATA means that a rolling\n");
	htmlFile.WriteLine("test run was not done.\n");

	htmlFile.WriteLine("<P><CENTER><TABLE BORDER>");
	htmlFile.WriteLine("<TR><TH> SNAP Job</TH> <TH> Logs/Results </TH> <TH> Machine </TH> <TH>Start Time</TH> <TH>Duration</TH> <TH>Failures</TH></TR>");

	var jobDirs = FSOGetDirPattern(jobDirBase, /^\d+$/);
	jobDirs.sort(function(x, y) { return (numSuffix(y, "") - numSuffix(x, "")); });

	var numNoData = 0;
	for (var j = 0; j < jobDirs.length; j++) {
		var jobDir = jobDirs[j];
		logMsg(LogRolling, LogInfo100, "_rollingTestReport: jobDir = ", jobDir, "\n");

		var changeNum = "UNKNOWN";
		if (jobDir.match(/\\(\d+)$/))
			changeNum = RegExp.$1;

		if (j > 10 && numNoData > 2)
			break;

		var jobInfo = jobDir + "\\JobInfo.bat";
		if (!FSOFileExists(jobInfo) || !FSOReadFromFile(jobInfo).match(/CurrentJobDir=(\S+)/m))
			continue;
		jobDir = RegExp.$1;

		var jobReport = jobDir + "\\JobReport.html";
		if (!FSOFileExists(jobReport))
			continue;
		var dispJobDir = "<A href=" + relPath(jobReport, reportDir) + ">" + _parseSnapJobReport(jobReport).entries.join(" <BR> ") + "</A>";

		var outDirChange = outDirBase + "\\" + changeNum;
		var outDir = outDirChange;
		var runNum = 0;
		for(;;) {
			var date = "NODATA";
			var duration = "NODATA";
			var machine = "NODATA";
			var failures = "NODATA";
			var dispOutDir = "NODATA";

			logMsg(LogRolling, LogInfo100, "_rollingTestReport: outDir = ", outDir, "\n");
			if (!FSOFolderExists(outDir)) {
				if (runNum > 0)
					break;
			} else {
				outDir.match(/(\d+\.?\d*)$/);
				var ref = outDir + "\\taskReport.html";
				if (!FSOFileExists(ref)) 
					ref = outDir;
				dispOutDir = "<A href=" + relPath(ref, reportDir) + ">" + uncPath(RegExp.$1) + "</A>";
				failures = "INCOMPLETE";
				date = duration = machine = "UNKNOWN";
			}

			var statusFiles = FSOGetFilePattern(outDir, /rollingReport\..*.status.log/i);
			if (statusFiles.length > 0) {
				var statusData = FSOReadFromFile(statusFiles[statusFiles.length-1]);
				if (statusData.match(/^machine:\s*(.*)$/im))
					machine = RegExp.$1;
				if (statusData.match(/^startTime:.*?(\w+ \d+ \d+:\d\d)/im))
					date = RegExp.$1;
			}

			var statusFiles = FSOGetFilePattern(outDir, /rolling\..*.status.log/i);
			if (statusFiles.length > 0) {
				var statusData = FSOReadFromFile(statusFiles[statusFiles.length-1]);
				// logMsg(LogClrAutomation, LogInfo, "rollingReport: Got Data {\n", statusData, "\n}\n");

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


			// logMsg(LogClrAutomation, LogInfo, "Checking ", outDir, "\n");
			if (failures.match(/FAIL/i)) {
				    // HACK for smarty results
				var testOutDir = outDir + "\\x86chk\\test.devbvt";
				if (!FSOFolderExists(testOutDir))
					testOutDir = outDir + "\\ia64chk\\test.devbvt";
				if (!FSOFolderExists(testOutDir))
					testOutDir = outDir + "\\amd64chk\\test.devbvt";

				var smartyReport = FSOGetFilePattern(testOutDir, /.*html$/);
				if (smartyReport.length > 0) {
					var failuresText = "NONE";
					var failFiles = FSOGetFilePattern(testOutDir, /.*fail.smrt$/);
					if (failFiles.length > 0) {
						failuresText = FSOReadFromFile(failFiles[0]);
						failuresText = failuresText.replace(/.*=([^-, ]+).*/gm, "$1");
						failuresText = failuresText.replace(/[\s\n]+/gm, " <BR> ");
					}
					failures = "<A href=" + relPath(smartyReport[0], reportDir)  + ">" + uncPath(failuresText) + "</A>";
				}

				    // HACK for trun results 
				var trunXmlFileName = outDir + "\\x86chk\\test.ddInt.src-fx\\trun.xml";
				if (FSOFileExists(trunXmlFileName)) {
					logMsg(LogClrAutomation, LogInfo, "Opened trun file ", trunXmlFileName, "\n");
					var trunXml = xmlRead(trunXmlFileName).TRUN; 
					var trunTests = trunXml.TESTSPROCESSED.TESTPROCESSED;
					var testFailureNames = "";
					var numTestFailures = 0;
					for(var i = 0; i < trunTests.length; i++) {
						var trunTest = trunTests[i];
						if (!trunTest.SUCCEEDED.match(/yes/i)) {
							logMsg(LogClrAutomation, LogInfo, "Found failure ", trunTest.TESTNAME, "\n");
							testFailureNames += trunTest.TESTNAME + "<br>\r\n";
							numTestFailures++;
							if (numTestFailures >= 10)
								break;
						}
					}
					var failuresText = failures;
					if (testFailureNames != "")
						failuresText = testFailureNames;

					failures = "<A href=" + relPath(trunXmlFileName, reportDir)  + ">" + uncPath(failuresText) + "</A>";
				}
			}

			outDir.match(/([^\\]+)$/);
			var dispDir = RegExp.$1;
			htmlFile.WriteLine("<TR><TD>" + dispJobDir + "</TD>" +
							   "<TD ALIGN=CENTER>" + dispOutDir + "</TD>" +
							   "<TD ALIGN=CENTER>" + machine + "</TD>" +
							   "<TD ALIGN=CENTER>" + date + "</TD>" +
							   "<TD ALIGN=CENTER>" + duration + "</TD>" +
							   "<TD ALIGN=CENTER>" + failures + "</TD></TR>");

			
			runNum++;
			outDir = outDirChange + "." + runNum;
		} 
	}
	htmlFile.WriteLine("</TABLE></CENTER>\n"); 

	var baseTaskFile = outDirBase + "\\taskReport.html";
	if (FSOFileExists(baseTaskFile)) {
		htmlFile.WriteLine("<P>");
		htmlFile.WriteLine("If there is a problem with the generation of the rolling test reports itself");
		htmlFile.WriteLine("it can be debugged by looking at the following task report: ");
		htmlFile.WriteLine("<A href=" + relPath(baseTaskFile, reportDir)  + ">" + uncPath(baseTaskFile) + "</A>");
	}
		
	htmlFile.WriteLine("</BODY>");
	htmlFile.WriteLine("</HTML>");
	htmlFile.Close();
	for (var i = 0; i < 1000; i++) {
		if (!FSOFileExists(reportHtmlFile)) {
			FSOMoveFile(tempName, reportHtmlFile);	
			break;
		}
		try { FSODeleteFile(reportHtmlFile); } catch(e) {};
		WScript.Sleep(10);		// Only browsers should be fetching this file
	}
}

/*****************************************************************************/
/* delete the oldest (smallest number) directory of the form  .*<num><suffix>
   so that there are only max total */

function _deleteOldestDirs(dirBase, suffix, max) {

	var dirs = FSOGetDirPattern(dirBase, new RegExp("\\d+\\.?\\d*" + suffix + "$", "i"));
	dirs.sort(function(x, y) { return (numSuffix(x, suffix) - numSuffix(y, suffix)); });
	logMsg(LogClrAutomation, LogInfo10, "_deleteOldestDirs: cur ", dirs.length, " max ", max, " dirs ", dirBase, "\\*\d+", suffix, "\n");
	for (var i = 0; i < dirs.length - max; i++) {
		var dir = dirs[i];
		logMsg(LogClrAutomation, LogInfo10, "_deleteOldestDirs: deleting directory ", dir, "\n");
		try { FSOAtomicDeleteFolder(dir); } catch(e) {}
	}
}

