/*********************************************************************************
 * ClrTestTask.js
 *********************************************************************************

 This file contains subroutines that automate building of product and test tree and
 creates a drop location.
*/


/*
This command build the both product and its tests. The command is
suitable as a build script for the rolling system machines. It builds
three flavors of the CLR and three flavors of the tests. Format of the
drop location is:

	CLR runtimes are dropped at:
	Automation\run.XXX\x86ret\...
	Automation\run.XXX\Amd64ret\...
	Automation\run.XXX\ia64ret\...

	CLR tests are dopped at:
	Automation\run.XXX\clrtest\x86ret\testbin\...
	Automation\run.XXX\clrtest\amd64ret\testbin\...
	Automation\run.XXX\clrtest\ia64ret\testbin\...

Arguments:

	testsTreeSubTree: If specified, the script build only portion of
	the test tree. The option has the same meaning as option
	subtreeToBuild in the command testsBuild. If left empty, the
	entire test tree is built.

	taskNameToRun: Optinal argument which is the name of the task to
	execute. If empty the default build task will be used.
	
*/
function SyncBuildProductAndTests(testTreePortion, taskNameToRun)
{
    var masterTaskName = _registerDailyProductTestsBuildTask(testTreePortion);

    if( taskNameToRun == undefined )
        taskNameToRun = masterTaskName;
    
    doRun( taskNameToRun );
}

/*
This command build the both product and its tests. The command is
suitable as a build script for the rolling system machines. It builds
three flavors of the CLR and three flavors of the tests. Format of the
drop location is:

	CLR runtimes are dropped at:
	Automation\run.XXX\x86ret\...
	Automation\run.XXX\Amd64ret\...
	Automation\run.XXX\ia64ret\...

	CLR tests are dopped at:
	Automation\run.XXX\clrtest\x86ret\testbin\...
	Automation\run.XXX\clrtest\amd64ret\testbin\...
	Automation\run.XXX\clrtest\ia64ret\testbin\...

Arguments:
    outDirBase: location of the base of drop output (required)

    numDirsToKeep: number of the builds to keep in the drop location.
          The oldest drops are removed.
          
	testTreePortion: If specified, the script build only portion of
	      the test tree. The option has the same meaning as option
          subtreeToBuild in the command testsBuild. If left empty, the
          entire test tree is built.

    addressToMailFailures: alias to send failures about build problems.
*/
function SyncBuildProductAndTestsToDrop(outDirBase, numDirsToKeep, testTreePortion, addressToMailFailures)
{
    var result = undefined;
    var outDir;
    try
    {
        var baseDirectory = srcBaseFromScript();
        var changeListNumber = _GetLatestChangeList(baseDirectory);

        if (numDirsToKeep == undefined || numDirsToKeep == "") {
            numDirsToKeep = 5;
        }

        var taskName = _registerDailyProductTestsBuildTask(testTreePortion, changeListNumber);
        // Open a lock so that 2 instances of doRun will not stomp on each other.
        // One scenario this helps with is if a dev manually starts dailyDevRun,
        // and he also has automation set up to start dailyDevRun.
        FSOCreatePath(outDirBase);

        var lock = openFileAsLock(outDirBase + "\\doRunLock.txt");
        var result = undefined;
        try {
            outDir = _newBuildDir(outDirBase, changeListNumber, numDirsToKeep);
            result = doRunHere(taskName, outDir, baseDirectory, undefined);
        }
        finally {
            lock.Close();
        }
    }
    finally
    {
        if( addressToMailFailures!=undefined &&
            result!=0 )
        {
            mailSendText( addressToMailFailures, "ATTENTION: Failures in the build machine", 
                          "Build Failed: " + outDir + "\\taskReport.html \n" +
                          "build returned: " + result );
        }
    }
    return result;
}

/*
The command sets the machine as a build machine. Every day on time 'timeToGoOff' the
machine starts command SyncBuildProductAndTestsToDrop. The command builds the test tree
and the product tree into drop location.
Arguments:
	timeToGoOff - time to start the build. 
    	'22:00' stands for 10pm
    dropLocation - a location where to drop the final builds

    testSubTreeToBuild - a name of test subtree to build. Building tests takes very
    	long time (currently about 7 hours). The build system builds 3 flavors of the
    	tests amd64, ia64 & x86. Building all three flavors sequentially would take too
    	long. Valid names are root-level subtrees. e.g. 'devsvcs'.

    addressToMailFailures - an email address to send information about
        failures from the build process.
 */
function setupBuildMachine(timeToGoOff, dropLocation, testSubTreeToBuild, addressToMailFailures)
{
    if( dropLocation==null )
        throwWithStackTrace(new Error("missing Drop Location"));

    if( !FSOFolderExists(dropLocation) )
        throwWithStackTrace(new Error("Invalid dropLocation path!"));
    
    if( testSubTreeToBuild == undefined )
        testSubTreeToBuild = "_";
    if( addressToMailFailures == undefined )
        addressToMailFailures = "_";
    
    var runjsCommand = "cscript " + WScript.ScriptFullName + " SyncBuildProductAndTestsToDrop \"" + dropLocation + "\" 10 " + 
        testSubTreeToBuild + " " + addressToMailFailures;

    return installRunAtCommand(timeToGoOff, runjsCommand, "buildScript.cmd");
}

// creates a new directory in outDirBase. The name is constructed from time and
// a sd changelinst. numKeep says how many past directories to keep.
function _newBuildDir(outDirBase, changeListNumber, numKeep) {

    if (outDirBase == undefined) 
        throwWithStackTrace(new Error("newBuildDir: Required argument outDirBase missing"));

    if (changeListNumber == undefined) 
        throwWithStackTrace(new Error("newBuildDir: Required argument changeListNumber missing"));

    if (numKeep == undefined) 
        throwWithStackTrace(new Error("newBuildDir: Required argument numKeep missing"));


    logCall(LogUtil, LogInfo10, "newRunDir", arguments);
    FSOCreatePath(outDirBase);


    // Remove old runs.  Their naming is from oldest to newest, and FSOGetDirPattern sorts by name
    var runDirs = FSOGetDirPattern(outDirBase, /run\.\d.*/);
    for(var i = 0; i < runDirs.length-numKeep; i++) {
        var runDir = runDirs[i];
        logMsg(LogUtil, LogInfo, "newBuildDir: deleting old dir ", runDir, "\n");
        try { FSODeleteFolder(runDir, true); } catch(e) {}
    }
    var outDir
    {
        // generate the out dir name --- get file Name
        var time = new Date();
        var timeStr = "run.";
        timeStr += padLeft(time.getFullYear() % 100, 2, true);
        timeStr += "-";
        timeStr += padLeft(time.getMonth()+1, 2, true);
        timeStr += "-"; 
        timeStr += padLeft(time.getDate(), 2, true);
        timeStr += "_";
        timeStr += changeListNumber;
        outDir = outDirBase + "\\" + timeStr;
        if( FSOFolderExists( outDir ) )
            throwWithStackTrace(new Error("Directory already exists: '" + outDir + "'"));
    }
    logMsg(LogUtil, LogInfo10, "newRunDir: outDir: ", outDir, "\n");
    FSOCreatePath(outDir);

    // create a link to the current directory
    var current = outDirBase + "\\run.current";
    logMsg(LogUtil, LogInfo10, "newRunDir: updating link : ", current, "\n");
    try {
        if (FSOFolderExists(current))
            runCmd(ScriptDir + "\\linkd.exe \"" + current + "\" /d");
        runCmd(ScriptDir + "\\linkd.exe \"" + current + "\" \"" + outDir + "\"");

        // Place a file that indicates the full long name of this directory
        // We should use this name since it does not change over time.
        FSOWriteToFile("set STABLE_RUN_DIR=" + outDir + "\r\n", outDir + "\\runInfo.bat");
    }
    catch(e) {
        logMsg(LogUtil, LogWarn, "Could not create a symbolic link (do you have a FAT file system?)\n");
        logMsg(LogUtil, LogWarn, "Note that the auotmation will work, however you will not have the\n");
        logMsg(LogUtil, LogWarn, "convinience link 'run.current' which always points to the latest run\n");
    }
    return outDir;
}


// returns the latest changelist number from the devdiv source depot.
function _GetLatestChangeList(depotDirectory)
{
  var runEnv = runSetEnv("PATH", "%PATH%;\\\\clrmain\\tools\\x86");
  if( depotDirectory )
    runEnv = runSetCwd(depotDirectory, runEnv);

  var num = runCmd("sd counter change", runEnv).output;
  if( ! num.match(/^(\d+)/) )
    throw Error("Unparsable output: '" + num + "' ");
  num = RegExp.$1;
  return num;
}

/*
 * Creates a task that builds both CLR and tests.
 */
function _registerDailyProductTestsBuildTask(testTreePortion,changeListNumber)
{
    var titleSuffix = "";
    if( testTreePortion != undefined )
        titleSuffix = "-" + testTreePortion;

    if( changeListNumber == undefined )
        changeListNumber = "";
    else
        titleSuffix += "@" + changeListNumber;
    
    
    var _taskSyncAll = taskGroup("Sync Dev&Test Trees",
                                 [ 
                                  taskNew("sdRevert-PublicChanges", "runjs taskRevert %srcBase% %srcBase%\\public\\...", undefined, undefined,
                                          "This task reverts all public files"),
                                  taskNew("sdSync-DevTree" + changeListNumber, "runjs sdSync %srcBase% " + changeListNumber, undefined, undefined,
                                          "This task synchronizes the dev tree."),
                                  taskNew("sdSync-TestTree", "runjs sdSync %srcBase%\\qa\\clr", undefined, undefined,
                                          "This task synchronizes the test tree.")
                                  ]);
    var build1 = _razzleBuildTask("chk", "x86", "ndp");
    var build2 = _razzleBuildTask("chk", "amd64", "ndp");
    var build3 = _razzleBuildTask("chk", "ia64", "ndp");
    var build4 = _razzleBuildTask("ret", "x86", "ndp");
    var build5 = _razzleBuildTask("ret", "amd64", "ndp");
    var build6 = _razzleBuildTask("ret", "ia64", "ndp");
    build1.dependents = build2.dependents = build3.dependents = build4.dependents = build5.dependents = build6.dependents =
	[ _taskSyncAll ];

    clrSetup.dependents = [ build1 ];
    var taskName = "SyncBuildProductAndTests" + titleSuffix;
    var _taskTestsBuild = 
        taskGroup(taskName, 
                  [
                   _taskSyncAll,
                   // build all test flavors
                   taskGroup("Build All CLR Flavors",
                             [
                              build1 , build2, build3 //, build4, build5, build6
                              ]),
                   taskGroup("Build All Test Flavors",
                             [
                              testsBuildTask("ret","x86", testTreePortion),
                              testsBuildTask("ret","amd64", testTreePortion),
                              testsBuildTask("ret","ia64", testTreePortion)
                              ]),
                   taskGroup("Copy Sources to the drop location",
                             [
                              testsCopySourcesTask("CLR_sources-publish", "%srcBase%\\ndp\\clr\\src", "%outDir%\\sources\\product\\src"),
                              testsCopySourcesTask("Test_sources-publish", "%srcBase%\\qa\\clr\\testsrc", "%outDir%\\sources\\tests\\testsrc")
                              ]),
                   ]);
 var _taskTestsBuildProduct = 
        taskGroup(taskName, 
                  [
                   _taskSyncAll,
                   // build product flavors
                   taskGroup("Build All CLR Flavors",
                             [
                              build1 , build2,  build4, build5 
                              ]),
                   
                   taskGroup("Copy Sources to the drop location",
                             [
                              testsCopySourcesTask("CLR_sources-publish", "%srcBase%\\ndp\\clr\\src", "%outDir%\\sources\\product\\src")
                              ]),
                   ]);
    _taskAdd( _taskTestsBuildProduct );
    return taskName;
}


/*
 * Creates a task that builds the test tree.
 */
function testsBuildTask(bldType, bldArch, treeSubDir, dependencies) 
{
    if (bldType == undefined)
        throw Error(1, "Arg bldType not supplied");
    if (bldArch == undefined)
        throw Error(1, "Arg bldArch not supplied");

    var relOutDir = bldArch+bldType;
    var taskName = "taskTestsCleanAndBuild" + (treeSubDir==undefined?"":"-"+treeSubDir) + "@" + relOutDir;
    var buildTestTree = taskNew(taskName,
                                "runjs _startW3svcAndTestsCleanAndBuild %srcBase% " + bldArch + " " + bldType + " " + Env("SystemRoot")+"\\Microsoft.NET\\Framework\\v2.0.x86ret \"\" " + treeSubDir,
                                [
                                 // in order to build the tree, we need to install x86ret
                                 _clrSetupTask("ret", "x86", undefined, "/fx /nrad", undefined, "x86ret", true /*ignore clrSetupCache*/)
                                 ]);
    buildTestTree.description = "Builds the test tree.\n";

    taskName = "taskPublishTests" + (treeSubDir==undefined?"":"-"+treeSubDir) + "@" + relOutDir;



    //function robocopy(srcDir, dstDir, options, logFile) {
    var copyFrom = "%srcBase%\\qa\\clr\\testbin";
    var copyTo   = "%outDir%\\clrtest\\" + relOutDir + "\\testbin";
    var copyTestTree = taskNew(taskName,
                               "runjs taskPublishTests " + copyFrom + " " + copyTo + " " + treeSubDir,
                               [ buildTestTree ]);
    copyTestTree.description = "Copies the test tree.\n";
    return copyTestTree;
}

/*
 * Copies the sources used to build the product from directory 'from'
 * to directory 'to'. Cleans all intermediate files inteh  'from' directory
 * using sd clean.
 *
 * arguments:
 * 	name - name of the task
 * 	from - source location
 * 	to   - where to copy the sources.
 */
function testsCopySourcesTask(name, from, to)
{
    var copyTask = taskNew(name, "runjs robocopy " + from + " " + to );
    copyTask.description = "Copies sources that this drop was build from";
    copyTask.dependents = [ taskNew("clean_" + name, "runjs sdClean " + from + " _ _ " + from) ];
    return copyTask;
}

function testsCopyTestSourcesTask(name, from, to, treeSubDir)
{
    var copyTask = taskNew(name, "runjs taskCopyTestSources " + from + " " + to + " " + treeSubDir );
    copyTask.description = "Copies test sources that this drop was build from";
    copyTask.dependents = [ taskNew("clean_" + name, "runjs sdClean " + from + " _ _ " + from) ];
    return copyTask;
}

/*
 * Creates a task that publishes (copies) the test tree to the drop
 * location.
 */
function taskCopyTestSources( from, to, treeSubDir )
{
    var testRoot = _FindTestRoot(false, from);
    var dirs = _GetListOfDirectoriesToBuild( testRoot, treeSubDir );
    var dir;
    
    for(var i=0; i< dirs.length; ++i) {
        var fromDir = testRoot + "\\testsrc\\" + dirs[i];
        var toDir = to + "\\" + dirs[i];
        robocopy( fromDir, toDir, "/PURGE" );
    }
    runCmd("copy /y " + testRoot+"\\testsrc\\* " + to);
}

/*
 * Creates a task that publishes (copies) the test tree to the drop
 * location.
 */
function taskPublishTests( from, to, treeSubDir )
{
    var testRoot = _FindTestRoot(false, from);
    var dirs = _GetListOfDirectoriesToBuild( testRoot, treeSubDir );
    var dir;
    
    for(var i=0; i< dirs.length; ++i) {
        var fromDir = testRoot + "\\testbin\\" + dirs[i];
        var toDir = to + "\\" + dirs[i];
        robocopy( fromDir, toDir, "/PURGE" );
    }
    runCmd("copy /y " + testRoot+"\\testbin\\* " + to);
    runCmd("copy /y " + testRoot+"\\testbin\\Desktop\\* " + to + "\\Desktop");
}

function taskRevert(workingDirectory,file)
{
  var wd = sdConnect(workingDirectory);
  return sdRevert(file, wd);
}


function  _startW3svcAndTestsCleanAndBuild(srcBase, buildArch, buildType, clrVersionDir, options, subtreeToBuild)
{
    // this automation is neccessary so that we enable w3svcs after CLR install.
    runCmd("net start w3svc", runSetNoThrow());
    return taskTestsCleanAndBuild(srcBase, buildArch, buildType, clrVersionDir, options, subtreeToBuild);
}
