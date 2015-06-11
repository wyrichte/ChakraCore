/*********************************************************************************/
/*                                 queue.js                                      */
/*********************************************************************************/

/* a simple queue dispatching mechanism */
// DATE: 1/1/2003
// AUTHOR: Vance Morrison 
// 
/*********************************************************************************/

/* a queue is a very simple structure on disk.  Entries have job directories where
   job information can go and have the naming convention.  These persist 
   forever (as far as queue.js is concerned)

		<queue.base>\entries\<userName>.<num>

	Where 'num' is an ordinal number that increases monotonically.

	The queue also has a 'pending' directory that holds the entries that have
    not been processed, and have the name 

		<queue.base>\pending\pri<num>.<num>.user.url

	Where 'num' is an ordinal number that increases monotonically.
	These files are standard .URL files which point at the job directories.  
    The naming convention natually sorts them first by priority then by 
	submission order.  


	In addition there is 

		<queue.base>\pending.lck which is a file that is opened when the queue files
   		are being modified.  If everyone opens this file for exclusive writing
		before modifying the queue then you get mutual exclusion.  If you go 
		through the APIs here, you get this mutual exclusion

	The queue understands the folloing flags

		<queue.base>\paused.txt				pauses the queue (data inside gives reason)
		<queue.base>\priorityOnly.txt		Only high priority jobs allowed (again reason inside)

	There is also a 
		<queue.base>\queueReport.html		

	which can be generated by calling 'queueUpdateStatus'
*/

var queueModuleDefined = 1; 		// Indicate that this module exist
if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");

var LogQueue = logNewFacility("queue");

if (WshShell == undefined) 
	var WshShell   = new ActiveXObject("WScript.Shell"); 

/*********************************************************************************/
/* given a directory that represents the queue (it does not need to exist), 
    create a queue object that manages that queue */

function queueNew(queueBase) {
	logMsg(LogQueue, LogInfo10000, "In queueNew(" + queueBase + ")\n"); 
	var queue = new Object();

	queue.base = queueBase;						// The directory containing all queue data
	FSOCreatePath(queue.base);

	queue.entries = queueBase + "\\entries";	// The directory that contains all job entries
	FSOCreatePath(queue.entries);

	queue.pending = queueBase + "\\pending";	// The entries that are actually waiting
	FSOCreatePath(queue.pending);

	queue.processed = queueBase + "\\processed";// Entries that are processed (or active)
	FSOCreatePath(queue.processed);

	queue.lock = queueBase + "\\pending.lck";	// File used to insure exclusive access to queue
	queue.toString = function() { return this.base; }	// pretty printing
	queue.history = 20;							// Number of links keep in processed dir
	queue.entryHistory = 100;					// Number entry directories to keep around
	return queue;
}

/*********************************************************************************/
/*  returns a new queue entry in 'queue for user 'user' this entry contains the fields

		queue.name		// a short, unique name
		queue.jobDir	// a directory private to this entry
		queue.priority	// entries priority
		queue.user		// user this entry belongs to
		queue.pendingName	// (IF the entry is waiting in the queue, this is the name of the URL file for it)

	Initially a queue entry is NOT in waiting in the queue (since presumably more
	setup needs to be done in the job directory to make the entry complete)
	Use 'queueAddEntry to submit it 
*/

function queueNewEntry(queue, user, priority) {

	if (user == undefined)
		user = Env("USERNAME");
	if (priority == undefined)
		priority = 3;
	logMsg(LogQueue, LogInfo10000, "In createQueueEntry(", queue, ", ", user, ", ", priority, ") {\n"); // }

	var lock = openFileAsLock(queue.lock);
	var entry = new Object();
	entry.priority = priority;
	entry.user = user;

	entry.jobDir = getUniqueDateFileName(queue.entries, "", "." + user);
	entry.jobDir.match(/([^\\]+)$/);
	entry.name = RegExp.$1;
	FSOCreatePath(entry.jobDir);
	lock.Close();

	entry.toString = function() { return this.name; }	// pretty printing
	/* { */ logMsg(LogQueue, LogInfo10000, "} queueNewEntry() = ", entry.jobDir, "\n");
	return entry;
}

/*********************************************************************************/
/* Add the entry to the list of waiting entries for the queue */

function queueAddEntry(queue, entry) {
	logMsg(LogQueue, LogInfo10000, "In queueAddEntry(", queue, ", ", entry, ") {\n");

	lock = openFileAsLock(queue.lock);
	entry.pendingName = queue.pending + "\\pri" + entry.priority + "." + entry.name + ".url";
	urlShortCutCreate(entry.pendingName, entry.jobDir);

	lock.Close();
	logMsg(LogQueue, LogInfo10000, "} queueAddEntry()\n");
}

/*********************************************************************************/
/* remove a entry from the list of waiting entries for the queue.  This entry
   does NOT go into the list of 'processed' entries */

function queueRemoveEntry(queue, entry) {

	logMsg(LogQueue, LogInfo1000, "In queueRemoveEntry(", queue, ", ", entry, ")\n"); 
	FSODeleteFile(entry.pendingName, true);
}


/*********************************************************************************/
/* finds an entry with the given name */

function queueFindEntry(queue, name) {

	var entries = queueGetEntries(queue);
	for (var j = 0; j < entries.length; j++) {
		if (entries[j] == name)
			return entries[j];
	}
	return undefined;
}

/*********************************************************************************/
/* returns an array of queue entries in the order they would be processed */

function queueGetEntries(queue) {
	var entries = [];

		// getFilePattern returns file names in sorted order, thus pri1 come first naturally */
	var pendingNames = FSOGetFilePattern(queue.pending, /.*\.url$/i);
	for (var i = 0; i < pendingNames.length; i++) {
		var entry = _queueParsePending(queue, pendingNames[i])
		if (entry != undefined)
			entries.push(entry);
	}
	logMsg(LogQueue, LogInfo10000, "In queueGetEntries(", queue, ") = ", entries, "\n"); 
	return entries;
}

/*****************************************************************************/
/* set the top entry to 'running' and return the entry.  This entry is then 
   returned.  You should call queueFinishEntry after the entry is finished
   running so that the queue can keep its report up to date */

function queueRunNextEntry(queue) {

	var entry = undefined;
	logMsg(LogQueue, LogInfo10000, "In queueGetNextEntry(", queue, ") {\n"); 
	_queueGetState(queue);

	if (queue.state != "PAUSED") {
		var lock = openFileAsLock(queue.lock);
			// FSOGetFilePattern returns file names in sorted order, thus pri1 come first naturally 
		var pendingNames = FSOGetFilePattern(queue.pending, /.*\.url$/i);
		for (var i = 0; i < pendingNames.length; i++) {
			var pendingName = pendingNames[i];
			entry = _queueParsePending(queue, pendingName);
			if (entry != undefined) {
				if (queue.state == "PRIORITY_ONLY" && entry.priority >= 2) {
					logMsg(LogQueue, LogInfo1000, "queueGetNextEntry: PRIORITY_ONLY on pri " + entry.priority + " above threshold of 2\n");
					entry = undefined;	
				}
				break;
			}

				// We have a bad entry, delete it this really should not happen
			logMsg(LogQueue, LogError, "queueGetNextEntry.  Bad entry, ", pendingName, "\n"); 
			FSODeleteFile(pendingName);
		}
		lock.Close();
	}

	if (entry) {
			// mark this entry as 'running'. We read and write it to update the time stamp
		FSOWriteToFile(FSOReadFromFile(entry.pendingName), queue.base + "\\running.url");

			// Put in in the processes bucket, clean out old entries
		FSOMoveFile(entry.pendingName, queue.processed + "\\" + entry.name + ".url");
		var files = FSOGetFilePattern(queue.processed, /.*.url/);
		while (files.length > queue.history) {
			var file = files.shift();
			logMsg(LogQueue, LogInfo, "queueRunNextEntry processed limit = ", queue.history, " cur = ", files.length, " deleting ", file, "\n");
			FSODeleteFile(file);
		}

		if (queue.entryHistory) {
			var dirs = FSOGetDirPattern(queue.entries);
			while (dirs.length > queue.entryHistory) {
				var dir = dirs.shift();
				logMsg(LogQueue, LogInfo, "queueRunNextEntry entry limit = ", queue.entryHistory, " cur = ", dirs.length, " deleting ", dir, "\n");
				try { 	
					FSODeleteFolder(dir, true); 
				} catch(e) {
			logMsg(LogQueue, LogWarn, "queueRunNextEntry: error deleting dir ", dir, " exception ", e.description, "\n");
				}
			}
		}
	}

	logMsg(LogQueue, LogInfo10000, "} queueGetNextEntry() = ", entry, "\n");
	return entry;
}

/*****************************************************************************/
/* this finishes the processing of 'entry' which should have been what was
   returned by the last call to queueRunNextEntry  */

function queueFinishEntry(queue, entry) {

	logMsg(LogQueue, LogInfo1000, "In queueFinishEntry(", queue, ", ", entry, ")\n"); 

		// just delete the running 
	var runningFile = queue.base + "\\running.url";
	if (FSOFileExists(runningFile))
		FSODeleteFile(runningFile);
}

/***********************************************************************/
/* update the 'queueReport.html' file assocated with queue */

function queueReport(queue) {

	logMsg(LogQueue, LogInfo100, "queueReport(", queue, ")\n"); 
	var now = new Date();

	var statusHtmlFileName = queue.base + "\\queueReport.html";
	var tempName = statusHtmlFileName + ".new"; 
	var htmlFile = FSOOpenTextFile(tempName, 2, true); 
	htmlFile.WriteLine("<HTML>"); 
	htmlFile.WriteLine("<BODY>"); 
	htmlFile.WriteLine("<H2> Status of queue " + queue.base + "</H2>"); 
	htmlFile.WriteLine("<UL>");
	htmlFile.WriteLine("<LI>This page was last updated on " + now); 
	htmlFile.WriteLine("</UL>");

	htmlFile.WriteLine("The queue state is " + queue.state + "\n");
	if (FSOFileExists(queue.base + "\\paused.txt")) {
		htmlFile.WriteLine("The file " + queue.base + "\\paused.txt exists.  Explaination for pausing");
		htmlFile.WriteLine("<DL><DD>");
		htmlFile.Write(FSOReadFromFile(queue.base + "\\paused.txt"));
		htmlFile.WriteLine("</DL>");
	}
	else if (FSOFileExists(queue.base + "\\priorityOnly.txt")) {
		htmlFile.WriteLine("The file " + queue.base + "\\priorityOnly.txt exists.  Only high priority jobs will be processed");
		htmlFile.WriteLine("<DL><DD>");
		htmlFile.Write(FSOReadFromFile(queue.base + "\\priorityOnly.txt"));
		htmlFile.WriteLine("</DL>");
	}

	var runningFile = queue.base + "\\running.url";
	if (FSOFileExists(runningFile)) {
		var jobDir = urlShortCutTarget(runningFile);
		var jobStatusHtmlFile = jobDir + "\\taskReport.html";

		var started = FSOGetFile(runningFile).DateLastModified;
		var durationMin = ((now - started) / (60 * 1000)).toFixed(2);

		if (FSOFileExists(jobStatusHtmlFile))
			htmlFile.WriteLine("Processing job <a href='" + jobStatusHtmlFile + "'>" + jobDir + "</a> for " + durationMin + " min");
		else 
			htmlFile.WriteLine("Processing job " + jobDir + " for " + durationMin + " min");
	}

	var entries = queueGetEntries(queue);
	htmlFile.WriteLine("<P>There are " + entries.length + " pending entries in the queue\n");
	if (entries.length > 0) {
		
		htmlFile.WriteLine("<P><CENTER><TABLE BORDER>");
		htmlFile.WriteLine("<TR><TH>Ord</TH> <TH>Pri</TH> <TH> name </TH> <TH>Job Directory</TH></TR>");
		for (var j = 0; j < entries.length; j++) {
			var entry = entries[j];
			var htmlLink = entry.jobDir;
			var jobInfo = entry.jobDir + "\\jobInfo.xml";
			if (FSOFileExists(jobInfo))
				htmlLink = jobInfo;

			htmlFile.WriteLine("<TR><TD>" + j + 
				"</TD><TD>" + entry.priority + 
				"</TD><TD>" + entry.name +
				"</TD><TD> <A HREF='" + htmlLink + "'>" + entry.jobDir + "</A>" +
				"</TD></TR>");
		}
		htmlFile.WriteLine("</TABLE></CENTER>\n");
	} 
	var processed = FSOGetFilePattern(queue.processed, /.*.url/);
	if (processed.length > 0) {
		htmlFile.WriteLine("<P>Reciently processed jobs\n");
		
		htmlFile.WriteLine("<CENTER><TABLE BORDER>");
		htmlFile.WriteLine("<TR><TH>Job Report</TH></TR>");
		for (var j = processed.length-1; j >= 0; --j) {
			var entry = urlShortCutTarget(processed[j]);
			if (FSOFolderExists(entry)) {
				var status = entry + "\\taskReport.html";
				if (!FSOFileExists(status)) 
					status = entry;

				htmlFile.Write("<TR><TD><A href='" + status + "'>" + status + "</A></TD></TR>");
			}
			else {
				FSODeleteFile(processed[j]);
			}
		}
		htmlFile.WriteLine("</TABLE></CENTER>\n");
	}
		
	htmlFile.WriteLine("</BODY>");
	htmlFile.WriteLine("</HTML>");
	htmlFile.Close();
	for (var i = 0; i < 100; i++) {
		if (!FSOFileExists(statusHtmlFileName)) {
			FSOMoveFile(tempName, statusHtmlFileName);	
			break;
		}
		try { FSODeleteFile(statusHtmlFileName); } catch(e) {};
		WScript.Sleep(10);		// Only browsers should be fetching this file
	}
}

/*********************************************************************************/
/* This is a helper that takes a link 'pendingName' which represents a pending
   queue entry, and forms an entry out of it */

function _queueParsePending(queue, pendingName) {
	logMsg(LogQueue, LogInfo100000, "In _queueParsePending(", queue, ", ", pendingName, ")\n"); 

	if (!pendingName.match(/\\pri(\d)\.(.*)\.([\w\s]+)\.url/i)) {
		logMsg(LogQueue, LogWarn, "_queueParsePending: pending Name '", pendingName, "' could not be parsed\n");
		return undefined;
	}

	var entry = new Object();
	entry.priority = RegExp.$1;
	entry.user = RegExp.$3;
	entry.pendingName = pendingName;
	entry.toString = function() { return this.name; }				// pretty printing

	var jobDir = urlShortCutTarget(pendingName);
	
	if (!FSOFolderExists(jobDir)) {
		logMsg(LogQueue, LogWarn, "_queueParsePending: Job Dir '", jobDir, "' does not exist\n");
		return undefined;
	}
	
	entry.jobDir = jobDir;
	if (!jobDir.match(/\\([^\\]+)$/)) {
		logMsg(LogQueue, LogWarn, "_queueParsePending: Could not parse job directory ", jobDir, "\n");
		return undefined;
	}

	entry.name = RegExp.$1;
	logMsg(LogQueue, LogInfo100000, "_queueParsePending() = ", entry.name, "\n"); 
	return entry;
}

/*********************************************************************************/
/* look at queue directory and determine what state we are in.  Set queue.State accordingly */

function _queueGetState(queue) {

	var state = "RUN";
	if (FSOFileExists(queue.base + "\\paused.txt"))
		state = "PAUSED";
	else if (FSOFileExists(queue.base + "\\priorityOnly.txt")) 
		state = "PRIORITY_ONLY";
	logMsg(LogQueue, LogInfo10000, "_queueGetState = ", state, "\n"); 
	queue.state = state;
}

