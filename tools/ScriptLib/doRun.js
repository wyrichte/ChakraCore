/****************************************************************************/
/*                              doRun.js                                    */
/****************************************************************************/

/* doRun is the user interface to the infrastructure defined in task.js.
  
   task.js provides a way of defining and running a set of tasks. This works
   fine when the task system is being called from jscript.  However this
   does not work well from the command line.  DoRun allows you to specify 
   tasks by name, pattern match against them, which makes it useful from
   the command line.

   Centeral to this is a list of well known tasks. this list is the 
   _tasksAll array.  This contains all the interesting tasks a user may
   want to run.  You can make private list of tasks, but there really
   should be no need for this, just put them on the global list.  

   Once you have this global list, you can specify which tasks you want
   by pattern matching.  This is what 'doRun' and 'doRunHere' do.  There
   is also a routine 'doRunShow' which displays what tasks are available
 
   Major functionality Provided here

		doRunHere: Given a pattern, run all tasks with that pattern on a
  				   given source base and output directory.

		doRun    : Generates a new unique output directory (throwing old
                   ones away), and calling 'doRunHere' on that directory)

		doRunShow : Display the list of tasks (so you can call doRunHere)

*/
// AUTHOR: Vance Morrison 
// DATE: 11/1/2003
/****************************************************************************/

var doRunDefined = 1; 					// Indicate that this module exist

if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!taskModuleDefined) throw new Error(1, "Need to include clrTask.js");

	// We really assume that other modules (in our case ClrTask), will
    // override and add to this array of interesting tasks.
if (!_tasksAll)
	var _tasksAll = [];

var LogDoRun = logNewFacility("doRun");

/****************************************************************************/
/* Run a set of tasks that match a regular expression pattern and put the 
   output in a new directory.  This new directory will be of the form 
   <outDirBase>.run.<date> and their will be a symbolic link from 
   <outDirBase>.run.current to this new directory.  The task will have
   the '%srcBase%' parameter set to 'srcbase'.  The meaning of the srcBase
   parameter is dependent on the task, but is typically the base of the VBL.

   Tasks generate a file called  <outDirBase>.run.current\taskReport.html.
   This is a report of what happened during the run.  

   Parameters:
     patStr:     The pattern of tasks to match.  Use 'runjs doRunShow' to show
                 what is available.  Full regular expressions can be used.  In 
                 particular, (x|y) will run both the x and y task. 
     outDirBase: The directory where the new run.* and run.currnet directory 
                 is created. 

     srcBase:    A root of all source files passed to the task.  This becomes
                 the %srcBase% variable in the task command.  Use the command
                 'runjs doRunShow /verbose' to see the commands that are run. 
                 Any uses of %srcBase% in the command become this value.  

     envStr:     Environment string to use (if any).  (More details on this
                 parameter via 'runjs /? dailyDevRun'.)

     numDirsToKeep: How many directories of past runs to keep around, any above
                 this number are deleted.

   The default values for outDirBase and srcBase are derived from the path
   to the script itself (see runjs /? srcBaseFromScript)
*/
function doRun(patStr, outDirBase, srcBase, envStr, numDirsToKeep) {

    // TODO: Vista: Add support for running task elevated

    if (patStr == undefined)
        patStr = "dailyDevRun";

    if (outDirBase == undefined) 
        outDirBase = srcBaseFromScript() + "\\automation";

    if (srcBase == undefined) 
        srcBase = srcBaseFromScript();

    if (numDirsToKeep == undefined) {
        numDirsToKeep = 3;
    }

    // Open a lock so that 2 instances of doRun will not stomp on each other.
    // One scenario this helps with is if a dev manually starts dailyDevRun,
    // and he also has automation set up to start dailyDevRun.
    FSOCreatePath(outDirBase);
    var lock = openFileAsLock(outDirBase + "\\doRunLock.txt");

    var result = undefined;

    try {
        var outDir = newRunDir(outDirBase, numDirsToKeep);
        result = doRunHere(patStr, outDir, srcBase, envStr);
    }
    finally {
        lock.Close();
    }

    return result;
}

var _myMachineMan = machManNew();

/*****************************************************************************/
/* Run a set of tasks that match a regular expression pattern and put the 
   in the directory 'outDir'. 

   Parameters:
     patStr:     The pattern of tasks to match.  Use 'runjs doRunShow' to show
                 what is available.  Full regular expressions can be used.  In 
                 particular, (x|y) will run both the x and y task. 
     outDir:     The directory where the task output files are placed.  In 
                 particular this is where the taskReport.html file is placed. 

     srcBase:    A root of all source files passed to the task.  This becomes
                 the %srcBase% variable in the task command.  Use the command
                 'runjs doRunShow /verbose' to see the commands that are run. 
                 Any uses of %srcBase% in the command become this value.  

   The default values for outDirBase and srcBase are derived from the path
   to the script itself (see runjs /? srcBaseFromScript)
*/

function doRunHere(patStr, outDir, srcBase, envStr, tasks, machMan) {

	if (patStr == undefined)
		patStr = "dailyDevRun";
	if (outDir == undefined) 
		outDir = srcBaseFromScript() + "\\automation\\run.current"; 
	if (srcBase == undefined) 
		srcBase = srcBaseFromScript();
	if (tasks == undefined) 
		tasks = _tasksAll;
	if (machMan == undefined)
		machMan = _myMachineMan;		// defined above but can be overridden in runjs.local.js 

	logCall(LogScript, LogInfo, "doRunHere", arguments);

	WScript.Echo("*****************************************************************************");
	WScript.Echo("*                 Starting doRunHere for " + patStr);
	WScript.Echo("*");
	WScript.Echo("* Report: " + outDir + "\\taskReport.html");
	WScript.Echo("* The report can be viewed before the run is complete");
	WScript.Echo("*****************************************************************************");
	WScript.Echo("");

		// Find all the tasks in '_tasksAll (recursively) that match any of the patterns
	var tasks = _tasksFindPat(tasks, new RegExp("^(" + patStr + ")$", "i"));
	if (tasks.length == 0) 
		throw Error(1, "No tasks match pattern '" + patStr + "'");
		// Set up any environmental variables. Format is label=value;label=value etc.
	var env = {};
	if (envStr != undefined)
	{
		while (envStr.match(/^(\w+)=([^;]*);?/))
		{
			envStr = RegExp.rightContext;
			env[RegExp.$1] = RegExp.$2;
		}
	}
	
	var ret = tasksRun(tasks, outDir, srcBase, env, machMan);

	return ret;
}

/*****************************************************************************/
/* print the available tasks that can be pattern matched with doRunHere.  The
   output is a list of task names that can be used as parmeters to the 'patStr'
   parameter of 'doRun'.  

   Parameters:
     options: A string of qualifiers for this command.  Options include
			      /verbose  :  print the task command along with the name 
     tasks:   a list of tasks to print, if not supplied _tasksAll is assumed
              Most callers do not use this parameter. 

*/
function doRunShow(options, tasks) {
	
	if (tasks == undefined) {
		tasks = _tasksAll;
		WScript.Echo("");
		WScript.Echo("Below is a list of tasks names.  These names can be used as parameters to the");
		WScript.Echo("doRun or doRunHere commands (eg 'runjs doRun dailyDevRun' runs the dailyDevRun");
		WScript.Echo("task. Tasks can have 'child' tasks and these are run (and must succeed) before");
		WScript.Echo("the parent command runs.  Some tasks (like dailyDevRun) have no action ");
		WScript.Echo("associated with them but simply act as groupings of interesting child tasks.  ");
		WScript.Echo("");
		WScript.Echo("The doRun and doRunHere actually take regular expression patterns, so you can");
		WScript.Echo("use the | operator to specify groupings of tasks for example");
		WScript.Echo("");
		WScript.Echo("     runjs doRun \"sdxSync|preCheckinTestsFull\"");
		WScript.Echo("");
		WScript.Echo("Will run everything in the dailyDevRun, ia64Builds and fxSuites tasks.  A task");
		WScript.Echo("of a given name will only be run once even if it is specified several times");
		WScript.Echo("in the task tree (or pattern specification). ");
		WScript.Echo("");
		WScript.Echo("Often the name is descriptive enough to determine what the task does.  However");
		WScript.Echo("the command 'runjs doRunShow /verbose' will display the actual command");
		WScript.Echo("associated with a task if that is needed.  Typically the command is another");
		WScript.Echo("'runjs' invocation, and you can do 'runjs /? <command>' to dig into what that");
		WScript.Echo("command does.");
		WScript.Echo("-------------------------------------------------------------------------------");
	}

	var opts = getOptions(["verbose"], options);

	_tasksPrint(tasks, "    ", opts.verbose);
	return 0;
}

function _tasksPrint(tasks, indent, verbose) {

	if (tasks == undefined)
		return;
	for(var i = 0; i < tasks.length ; i++) 
		_taskPrint(tasks[i], indent, verbose);
}

function _taskPrint(task, indent, verbose) {

	if (task == undefined)
		return;
	WScript.Echo(indent + task.name);

	if (verbose && task.cmd != undefined)
		WScript.Echo(indent + "Cmd: " + task.cmd);

	_tasksPrint(task.dependents, indent + "    ", verbose);
}

/*****************************************************************************/
/* find all tasks whose name matches 'path'.which are in 'tasks' or any 
   dependent of those tasks */

function _tasksFindPat(tasks, pat, list, set) {

	if (list == undefined)
		list = [];
	if (set == undefined)
		set = {};

	for(var i = 0; i < tasks.length; i++) {
		var task = tasks[i];
		if ((task != undefined) && (task.name != undefined))  {
			if (task.name.match(pat)) {
				logMsg(LogTask, LogInfo10, "_taskFindPat: pat ", pat, " pushing ", task, "\n");
				if (!set[task.name]) {
					set[task.name] = task;	
					list.push(task);
				}
			}
			if (task.dependents != undefined)
				_tasksFindPat(task.dependents, pat, list, set);
		}
	}
	return list;
}

/*****************************************************************************/
/* Deduce the source base from the path to the script itself.  Basically it
   assumes that the script lives in a directory of the form %dir%\inetcore\jscript\tools.
   and returns %dir%.  It throws if the script path is not of this form 
*/

function srcBaseFromScript() 
{
    if (ScriptDir.match(/^(.*)\\inetcore\\jscript\\tools/i)) return RegExp.$1;

    var sdxRoot = Env("sdxroot");
    if (sdxRoot != "") return sdxRoot;
    
    throw new Error(1, "sdxroot not set, and could not deduce VBL from " + ScriptDir + "\n");
}
