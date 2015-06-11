/*********************************************************************************/
/*                                 log.js                                        */
/*********************************************************************************/

/* A simple logging mechanism.  It allows you to segregate logging by a 'facility'.
   Each facility has a logging level (low means more important), and only
   messages with a logging level lower than the level specififed by the facilty are
   actually output. */

/* The first three levels are intended to be on by default (they say something
   is wrong or something really important happened).  The ones above that
   are user defined, however it is suggested that the number of output messages
   in a canonical use of the facility be used as a way of deciding what the 
   right level should be for a particular message. */

/* The logging mechanism pretty prints the output by watching for {}.  This 
   makes it trivial to make nicely nested output.  Since exceptions can screw
   this up (since some of the braces will not be balanced, there is a 'logTry'
   and 'logCatch' that allow you to fix it up by remembering what the level
   is at the try, and then coming back to that in the catch.
*/
   
/* AUTHOR: Vance Morrison
   Date:   1/26/03 */
/*********************************************************************************/
var logModuleDefined = 1; 		// Indicate that this module exists

	// These are the logging levels.  By default LogInfo is on
var LogError        = 1;
var LogWarn         = 2;
var LogInfo         = 3;			// This is the last level that is on by default
var LogInfo10       = 4;			// From here it is the rough number of messages you expect this logMsg call to make
var LogInfo100      = 5;
var LogInfo1000     = 6;
var LogInfo10000    = 7;
var LogInfo100000   = 8;
var LogInfo1000000  = 9;

//*************************  Private global state.  *******************************
var _LogFacilityId = 0;			// keeps track of the next facility ID to give out
var _LogFacilityNames = [];		// maps facility Ids to text names 
var _LogFacilityCallBacks = [];	// functions to call if the facility's level changes
var _LogLogLevels = [];			// maps facility to logging level for that facility
var _LogFile = WScript.StdOut;	// where to send output
var _LogTranscript = undefined;	// send it to this file too
var _LogStart = new Date().getTime();
var _LogFirstWrite = undefined;

	// Internal state
var _LogState  = { 
	atCol0:true,
	indent:"",
	printTime:true
}
var _LogStateOrig = _LogState;

var _LogLog = 0; _LogLog = logNewFacility("log");

	// This is a generic one that is useful for short scripts.  This should not
	// be used for infrastructure.
var LogScript = logNewFacility("script");

// Uncomment to debug logging facility itself 
// logSetFacilityLevel(_LogLog, 10);

if (!WScript.FullName.match(/cscript.exe/i)) {
	WScript.Echo("This script is intended to run by cscript.exe.\nRun CScript //H:CSCRIPT to set the default");
	WScript.Quit(1);
}

/**********************************************************************************/
/* create a new facility name for logging.  Name is the name of the facility
   (can be any unique tag).  'callBack' is optional.  If present it indicates
   a function to be called when this facility's level is changed.  This allows
   each facility to set the levels of other facilities that it uses.  Be sure
   to avoid recursive issues here */

function logNewFacility(name, callBack) {

	var ret = _LogFacilityId++;
	_LogFacilityNames[ret] = name;
	_LogLogLevels[ret] = LogInfo;
	if (callBack != undefined)
		_LogFacilityCallBacks[ret] = callBack;
	logMsg(_LogLog, LogInfo10, "logNewFacility(", name, ", ", callBack, ") assigns ID ", ret, "\n"); 
	return ret;
}

/**********************************************************************************/
/* given a regular expression, find all facilities that match it completely */

function logGetFacilityByPat(facilityPat) {

	var ret = []
	var pat = new RegExp("^" + facilityPat + "$", "i");
	for(var i = 0; i < _LogFacilityNames.length; i++) {
		name = _LogFacilityNames[i];
		if (name.match(pat)) 
			ret.push(i);
	}
	logMsg(_LogLog, LogInfo100000, "logGetFacilityByPat(", facilityPat, ") = [", ret, "]\n"); 
	return ret;
}

/**********************************************************************************/
/* sets the indent level explicitly. indent is a string of spaces.  The funciton
   returns the previous indent */

function logSetIndent(indent) {

	var temp = _LogState.indent;
	_LogState.indent = indent;
	return temp;
}

/**********************************************************************************/
/* indicate to the logger if each line of the log should be prefixed by the time 
   or not.  Returns the previous value 
 */
 
function logSetPrefixTime(shouldPrintTime) {
	var temp = _LogState.printTime;
	_LogState.printTime = shouldPrintTime;
	return temp;
}

/**********************************************************************************/
/* indicate to the logger that output should be redirected to the output 
   function 'outputFunction'.  It returns the previous output function.
   if null, it resets it to stanadard output.  'outputFunction has to be
   an object that contains data files that the function might want to use
   along with a filed called 'ftn' that is the method to call. */

function logSetOutputFunction(outputFunctionObject) {
	if (typeof(outputFunctionObject) != "object" && outputFunctionObject.ftn == undefined)
		throw new Error(1, "outputFunctionObject needs 'ftn' field to point at target function");

	var temp = _LogState;

	outputFunctionObject.atCol0 = true;
	outputFunctionObject.indent = "";
	outputFunctionObject.printTime = temp.printTime;

	_LogState = outputFunctionObject;
	return temp;
}

/**********************************************************************************/
function logSetFile(openFile, fileName) {
	
	if (fileName == undefined) {
		fileName = "UNKNOWN";
		if (openFile === WScript.StdOut)
			fileName = "STDOUT";
	}
	logMsg(_LogLog, LogInfo, "logSetLogFile: Setting log to: ", fileName, "\n");

	var temp = _LogFile;
	_LogFile = openFile;

	logMsg(_LogLog, LogInfo, "****************************************************************************\n");
	logMsg(_LogLog, LogInfo, "******* logSetLogFile: Setting log to: ", fileName, " on ", new Date(), " *********\n");
	return temp;
}

/**********************************************************************************/
/* a transcript file is a file that all log messages go to in addition to
   the normal loggin (set by logSetFile).  This is useful for allowing 
   messages to go to STDout but also having a file 
*/
function logSetTranscript(fileName) {
	if (fileName == undefined)
		_LogTranscript = undefined;
	else {
		logMsg(_LogLog, LogInfo, "logSetTranscript: opening transcript file ", fileName, "\n");
		_LogTranscript = FSOOpenTextFile(fileName, FSOForWriting, true);
	}
}

/**********************************************************************************/
/* set all facilities matching 'facilityPat' to level 'level' */

function logSetFacilityLevelByPat(facilityPat, level) {

	var facilities = logGetFacilityByPat(facilityPat);
	if (facilities.length > 0) {
		for (var i = 0; i < facilities.length; i++) 
			logSetFacilityLevel(facilities[i], level);
	}
	else {
		var facilities = [];
		for(var i = 0; i < _LogFacilityNames.length; i++) 
			facilities.push(_LogFacilityNames[i]);

		logMsg(_LogLog, LogError, "logSetFacilityByPat: bad facilityPat '", facilityPat, "'\n",
								  "Existing facilities {\n", facilities.join("\n"), "\n}\n")
		throw Error(1, "Bad logging facility pattern '" + facilityPat + "'");
	}
}

/**********************************************************************************/
/* sets the logging level for 'facility' to 'level'.  */

function logSetFacilityLevel(facility, level) {

	logMsg(_LogLog, LogInfo100000, "logSetFacilityLevel(", facility, ", ", level, ")\n"); 
	if (_LogLogLevels[facility] != undefined) {
		_LogLogLevels[facility] = level;
		if (_LogFacilityCallBacks[level] != undefined)
			_LogFacilityCallBacks[level](level);
	}
	else 
		logMsg(_LogLog, LogError, "logSetFacility: bad facility '", facility, "'\n");
}

/**********************************************************************************/
/* get the logging level for 'facility' */

function logGetFacilityLevel(facility) {
	return _LogLogLevels[facility];
}

/**********************************************************************************/
/* given the WScript.Arguments object 'args' and a starting index (defaults to 0)
   look for arguments of the form "/log:<facility>=<n>" that indicate what logging
   facility should be set to what level.  Note 'facility' is a regular expression.
   Returns the new index to start looking for args (Thus there can be many */

function logParseArgs(args, idx) {

	if (idx == undefined) 
		idx = 0;
	while (idx < args.length) {
		var arg = args(idx);
		if (arg.match(/[-\/]logT(ranscript)?:(\S*)/i)) {
			var fileName = RegExp.$2;
			logSetTranscript(fileName);
		}
		else if (arg.match(/[-\/]log:(\S*)=(\S*)/i)) {
			var facility= RegExp.$1;
			var levelStr= RegExp.$2;

			var level;
			if (levelStr.match(/^LogInfo1(0+)$/i))
				level = LogInfo10 + (RegExp.$1.length - 1);
			else if (levelStr.match(/^LogInfo$/i))
				level = LogInfo;
			else if (levelStr.match(/^LogError$/i))
				level = LogError;
			else if (levelStr.match(/^LogWarn$/i))
				level = LogWarn;
			else if (levelStr.match(/^(\d+)$/))
				level = RegExp.$1;
			else {
				logMsg(_LogLog, LogError, "logParseArgs: bad level '", levelStr, "'\n", 
										  "        Should be: LogError LogWarn LogInfo LogInfo10 LogInfo100, ...\n");
				throw Error(1, "Bad logging level '" + levelStr + "'");
			}
			logSetFacilityLevelByPat(facility, level);
		}
		else 
			break;
		idx++;
	}
	return idx;
}

/**********************************************************************************/
/* counts the number of log arguments. This is needed because we want to parse all
   of the arguments in case there are include files which themselves want to define
   logging. This would mean that we can't call logParseArgs until we have fully
   parsed all the additional arguments and included all extra files. */

function logCountArgs(args, idx) {

	if (idx == undefined) 
		idx = 0;
	while (idx < args.length && args(idx).match(/[-\/]log:(\S*)=(\S*)/)) {
		idx++;
	}
	return idx;
}

/**********************************************************************************/
/* log a message to stdout, indenting in a reasoanble way ({} cause indenting), this also 
   normalizes end of lines to \r\n. 'level' indicates the prioity.  Only messages below this
   level will be printed. Think of it as the log10 of the number of messages desired */

function logMsg(facility, level, msgs) {

	if (!(level-0 > _LogLogLevels[facility])) {
		if (!(level-0 > 0)) {
			logMsg.caller.toString().match(/function *(\w+)/);
			_logMsg("ERROR: logMsg: bad level '" + level + "' caller = " + RegExp.$1 + "\n");
		}
		if (!(_LogLogLevels[facility]-0 >= 0)) {
			logMsg.caller.toString().match(/function *(\w+)/);
			_logMsg("ERROR: logMsg: bad facility '" + facility + "' caller = " + RegExp.$1 + "\n");
		}

			// Concatinate all the rest of the arguments into the message
		var msg = "";
		if (level == LogError)
			msg += "ERROR: ";
		else if (level == LogWarn)
			msg += "WARNING: ";
			
		for(var i = 2; i < arguments.length; i++)
			msg += arguments[i];
		_logMsg(msg);
	}
}

/**********************************************************************************/
/* return a token that remembers the indent level, so to be passed to 'logCatch' */

function logTry() {
	return _LogState.indent;
}

/**********************************************************************************/
/* Given a token from a 'logTry' print any messages and return the indent to 
   that level. Note that 'msgs' can be any number of arguments */

function logCatch(tryToken) {
	while(_LogState.indent > tryToken) {
		/* { */ _logMsg("} ");
	}
}

/**********************************************************************/
/* give a function name and a list of arguments, return a string that
   is the correct syntax to pass to runjs */

function logCall(facility, level, ftnName, args, suffix) {

	if (level-0 > _LogLogLevels[facility])
		return;

	var ret = ftnName + " ";
	for(var i = 0; i < args.length; i++) {
		var arg = args[i];
		var argType = typeof(arg);
		if (arg == undefined)
			ret += "_";
		else if (argType == "string" && !arg.match(/^[\w\.\\\:\-]+$/))
			ret += "\"" + arg + "\"";
		else if (argType == "object" && argType.length > 0) 
			ret += "[ARRAY]";
		else 
			ret += arg;
		ret += " ";
	}
	if (suffix)
		ret += suffix;
	logMsg(facility, level, "TO REPLAY: runjs ", ret, "\n");
}



/**********************************************************************************/
/* helper function that actually prints the messgage */

function _logMsg(msg) {

	if (!_LogFirstWrite) {
		_LogFirstWrite = true;
		if (logGetFacilityLevel(_LogLog) >= LogInfo)
			msg = "Logging started at " + (new Date()) + "\n" + msg;
	}

	var prefix = undefined;
	while (msg.length > 0) {
		msg.match(/^(.*?)(\r*(\n|\r)|$)/);
		var line = RegExp.$1;
		var ret = RegExp.$2;
		msg = RegExp.rightContext;

		// indent if we have a opening bracket { 
		if (_LogState.indent.length >= 4 && line.match(/^[\s\d:.\w-]*}/)) 
			_LogState.indent = _LogState.indent.substr(4, _LogState.indent.length)

		var hasRet = (ret.length > 0);
		var atCol0 = _LogState.atCol0;
		_LogState.atCol0 = hasRet;

		// If we are printing a prefix, figure that out. 
		if (prefix == undefined) {
			prefix = "";
			if (_LogState.printTime && atCol0) {
				var delta = ((new Date()).getTime() - _LogStart) / 60000;
				prefix = padLeft(delta.toFixed(2), 5, true) + ": " + _LogState.indent;
			}
		}
	
		// send it to wherever the line is going 
		if (_LogState.ftn != undefined) {
			if (_LogState.prevChars  != undefined) {
				line = _LogState.prevChars + line;
				_LogState.prevChars = undefined;
			}

			if (hasRet) {
				// turn off logging for the duration to avoid recursion
				var saveState = _LogState;
				_LogState = _LogStateOrig;
				saveState.ftn(line, saveState);
				_LogState = saveState;
			}
			else 
				_LogState.prevChars = line;
		}
		else {
			_LogFile.Write(prefix);
			_LogFile.Write(line);
			if (hasRet)
				_LogFile.WriteLine("");

			// send to transcript file if requested
			if (_LogTranscript) {
				_LogTranscript.Write(prefix);
				_LogTranscript.Write(line);
				if (hasRet)
					_LogTranscript.WriteLine("");
			}
		}

		// unindent if we have a closing bracket 
		if (line.match(/{\s*$/m)) /* } */
			_LogState.indent += "    ";
	}
}

/*************************************************************************/
/* returns the current time to second granularity as a string */

function _logCurTime() {
	var time = new Date().toString();
	if (time.match(/ (\d+:\d+:\d+) /))
		time = RegExp.$1;
	return time
}

/*************************************************************************/
/* returns the current date in YYYY/MM/DD format. */

function _logCurDate() {
	var date = new Date();
	return date.getFullYear() + "/" + (date.getMonth()+1) + "/" + date.getDate();
}

/*************************************************************************/
/* Given a filename and size, this will check to see if the file is
   larger than size, and if so will copy it to filename.old.log (overwriting
   any previous .old.log file).
*/

function logTruncateTextFile(fileName, maxFileSize, oldExt) {
	if (oldExt == undefined) oldExt = ".old.log";
	if (FSOFileExists(fileName)) {
		var file = FSOGetFile(fileName);
		var fileSize = file.Size;
		if (fileSize > maxFileSize) {
			FSOMoveFile(fileName, fileName + oldExt, true);
			FSOOpenTextFile(fileName, FSOForWriting, true).Close();
		}
	}
}

/*************************************************************************/
/* Check to see if 'fileName' is bigger than 'maxFileSize'.  If it has
   copy it it an 'old' version and truncate it to 0,  Then set the log
   to the file (for appending).  This has the effect of making a log
   file that can not grow indefinately but keeps a good amount of history.
   returns the previous log file handle */
 
function logSetLimitedFile(fileName, maxFileSize) {

	if (maxFileSize == undefined)
		maxFileSize = 1000000;

	FSOCreatePathForFile(fileName);
	logTruncateTextFile(fileName, maxFileSize);
	return logSetFile(FSOOpenTextFile(fileName, FSOForAppending, true), fileName);
}
