/*****************************************************************************/ 
/*                               sourceDepot.js                              */ 
/*****************************************************************************/ 

/* functions for interfacing with Source Depot within scripts */ 
/* the first set are simply wrappers of source depot functionality, however
   the functions after the 'VALUE ADD' comment indicate operations that 
   require non-trivial post-processing */

// AUTHOR: Vance Morrison 
// DATE: 11/1/2003 
/*****************************************************************************/

var sourcedepotModuleDefined = 1;     // Indicate that this module exists

if (!logModuleDefined)  throw new Error(1, "Need to include log.js");
if (!fsoModuleDefined)  throw new Error(1, "Need to include fso.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");

var LogSourceDepot = logNewFacility("sourceDepot");

var sdConnectCache = {};

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

/*****************************************************************************/
/********************** BASIC SD COMMANDS (from script)  *********************/
/*****************************************************************************/

/*****************************************************************************/
/* Most of the utilites below need an connection to the SD.  This routine 
   creates such a connection.  For convinience most of the routines below
   will implicitly call this routine if necessary. This is less efficient 
   however since you keep makeing new connections to the depot.  'sdSpec'
   is either the path to a sd.ini file, or a path to to a directory 'above'
   the sd.ini file (SD searched 'up' for the sd.ini file).  It can also 
   be a string of the form <computer>:port, but this is not recommended, 
   as it does not unambiguously define the SD client name */

function sdConnect(sdSpec) {
    if (typeof(sdSpec) == "object")
        return sdSpec;
        
    logMsg(LogSourceDepot, LogInfo10, "sdConnect(" + sdSpec + ")\n");
    try {
        var sd = WScript.CreateObject("SourceDepot.SDConnection");
        if (sd == undefined)
            throwWithStackTrace(Error(1, "Could not find 'SourceDepot.SDConnection' COM object (run regsvr32 sdapi.dll outside a razzle window. For 64-bit machines, use %sdxroot%\tools\amd64\sdapi.dll)")); 
    }
    catch(e) {
        logMsg(LogSourceDepot, LogError, "Error connecting to SD (you may need to run regsvr32 sdapi.dll outside a razzle window. For 64-bit machines, use %sdxroot%\tools\amd64\sdapi.dll\n)");
        throw e;
    }

    if (typeof(sdSpec) == "string") {
            // This first pattern is here for legacy.  
        if (sdSpec.match(/^\w+:\d+$/))
            sd.Port = sdSpec;
        else  {
                // If it is a file, get the directory    
            if (FSOFileExists(sdSpec) && sdSpec.match(/^(.*)\\[^\\]+/))
                sdSpec = RegExp.$1;

            logMsg(LogSourceDepot, LogInfo10, "Using ", sdSpec, " to find sd.ini file\n");
            try {
                sd.LoadIniFile(sdSpec);
            }
            catch(e) {
                throwWithStackTrace(Error(1, "Could not find sd.ini file from spec " + sdSpec))
            }
        }
    }

    return sd;
}

/*****************************************************************************/
/* Create an empty change with a description 'description'  returns the
   change number for the new change. You make a change, do mulitple 
   sdEdit's and then submit to make a change to the depot.

   if changeNum is defined, then that number is used, otherwise a new
   change number is generated for this change
 */
function sdChange(description, changeNum, sdObj) {

    if (changeNum == undefined)
        changeNum = "";

    sdObj = sdConnect(sdObj);

    var results = _sdRun(sdObj, "change -o " + changeNum);
    var specData = results.StructuredOutput(0).Variables.SpecData;

        // Set the description and files */
    specData("Description") = description;
    if (changeNum == "")
    specData("Files") = "";

    sdObj.SpecData = specData;

    results = _sdRun(sdObj, "change -i");
    var info = _sdResultsGetInfoMsgs(results);

    if (!info.match(/^Change *(\d+) *(created|updated)/im)) 
        throwWithStackTrace(Error(1, "Could not parse sd change output '" + info + "'"));
    
    var changeNum = RegExp.$1;
    logMsg(LogSourceDepot, LogInfo10, "sdChange() = ", changeNum, "\n");
    return changeNum;
}

/*****************************************************************************/
/* Adds a file under the change 'changeNum'.  File can be a space separated
   list of filenames.  They should not already exist in the depot */

function sdAdd(file, changeNum, sdObj) {

    if (file == undefined)
        throwWithStackTrace(Error(1, "sdAdd: required arg 'file' missing"));
    if (changeNum == undefined)
        throwWithStackTrace( Error(1, "sdAdd: required arg 'changeNum' missing"));
    sdObj = sdConnect(sdObj);

    var results = _sdRun(sdObj, "add -c " + changeNum + " " + file, false, true);

    var warn = ""
    for(var i = 0; i < results.WarningOutput.Count; i++) 
        warn += results.WarningOutput(i).Message + "\r\n";
    return 0;
}

/*****************************************************************************/
/* Open a file under the change 'changeNum'.  File can be a space separated
   list of filenames.  They should already exist in the depot */

function sdEdit(file, changeNum, sdObj) {

    if (file == undefined)
        throwWithStackTrace(Error(1, "sdEdit: required arg 'file' missing"));
    if (changeNum == undefined)
        changeNum = "default";
    if (sdObj == undefined)
        sdObj = sdConnect(file);

    var results = _sdRun(sdObj, "edit -c " + changeNum + " " + file, false, true);

    var warn = ""
    for(var i = 0; i < results.WarningOutput.Count; i++) 
        warn += results.WarningOutput(i).Message + "\r\n";
    if (warn.match(/use 'reopen'/i)) 
        throwWithStackTrace(Error(1, "\r\nsdEdit: Trying to open a file already opened, failing:\nSDMSG: " + warn));

    return 0;
}

/*****************************************************************************/
/* Open a currently open file under the change 'changeNum'.  File can be a 
   space separated list of fileanmes */

function sdReopen(file, changeNum, sdObj) {

    if (file == undefined)
        throwWithStackTrace(Error(1, "sdReopen: required arg 'file' missing"));
    if (changeNum == undefined)
        throwWithStackTrace(Error(1, "sdReopen: required arg 'changeNum' missing"));
    sdObj = sdConnect(sdObj);

    var results = _sdRun(sdObj, "reopen -c " + changeNum + " " + file, false, false);
    return 0;
}

/*****************************************************************************/
/* Revert a currently open file. File can be a space separated list of 
   filnames */

function sdRevert(file, sdObj) {

    if (file == undefined)
        throwWithStackTrace(Error(1, "sdRevert: required arg 'file' missing"));
    sdObj = sdConnect(sdObj);

    var results = _sdRun(sdObj, "revert " + file, false, false);
    return 0;
}

/*****************************************************************************/
/* Return a list of files that are opened under the directory 'dir' */

function sdOpened(dir, sdObj) {

    sdObj = sdConnect(sdObj);
    if (dir == undefined)
        dir = ""
    else 
        dir = dir + "\\...";

    var results = _sdRun(sdObj, "opened -l " + dir, false, false);
    var ret = [];
    for (var i = 0; i < results.InfoOutput.Count; i++) {
        var msg = results.InfoOutput(i).Message;
        if (!msg.match(/(.*)#\d+ - /))
            throwWithStackTrace(Error(1, "sdOpened: Unexpected sd opened output " + msg));
        ret.push(RegExp.$1);
    }
    return ret;
}

/*****************************************************************************/
/* Given the file specification (which can be a local file name or a depot
   path), return the data in the file 'outFileName'
 */
function sdFetch(fileSpec, outFileName, sdObj) {

    sdObj = sdConnect(sdObj);

    if (FSOFileExists(outFileName))
        FSODeleteFile(outFileName);

    var results = _sdRun(sdObj, "print -o " + outFileName + " " + fileSpec, false, false);
}

/*****************************************************************************/
/* After you have made a change with sdChange, populated it with sdEdit
   you can call this routine to actually commit it.  returns the change 
   number of the submitted change (the orginal change is usually renamed).
   if 'asUser' is a user name (DOMAIN\USER), then the change is submitted
   under that user (you need SD impersonation rights to do this) */

function sdSubmit(changeNum, sdObj, asUser) {

    if (changeNum == undefined)
        throwWithStackTrace(Error(1, "sdSumbit: required arg changeNum missing"));
    sdObj = sdConnect(sdObj);

    var sdCmd = "submit -c " + changeNum;
    if (asUser != undefined)
        sdCmd += " -u " + asUser;

    results = _sdRun(sdObj, sdCmd);
    var info = _sdResultsGetInfoMsgs(results);

    // logMsg(LogSourceDepot, LogInfo, "sdSubmit() sd Command output {\n", info, "\n}\n");

    if (!info.match(/^Change +(\d+) +submitted/im) &&
        !info.match(/^Change.*renam.* +(\d+) +.*submitted/im))
        throwWithStackTrace(Error(1, "Could not parse sd submit output '" + info + "'"));
    
    var changeNum = RegExp.$1;
    logMsg(LogSourceDepot, LogInfo10, "sdSubmit() = ", changeNum, "\n");
    return changeNum;
}

/*****************************************************************************/
/* Given a SD file spec, return the full depot path associated with it */
   
function sdWhere(fileSpec, sdObj) {

    sdObj = sdConnect(sdObj);

    var ret = [];
    var results = _sdRun(sdObj, "where " + fileSpec);

    for(var i = 0; i < results.StructuredOutput.Count; i++)  {
        var vars = results.StructuredOutput(i).Variables;
        return "" + vars.Variable("depotFile");
    }
    return undefined;
}

/*****************************************************************************/
/* return a structure indicating the current version of 'fileSpec'.  FileSpec
   can be any legal SD file revision specification.  It returns a list of
   structures since fileSpec can represent multiple files */
 
function sdHave(fileSpec, sdObj) {

    sdObj = sdConnect(sdObj);

    logMsg(LogSourceDepot, LogInfo100, "sdHave(", fileSpec, ")\n");
    var results = _sdRun(sdObj, "have " + fileSpec);

    var ret = []
    for(var i = 0; i < results.StructuredOutput.Count; i++)  {
        var vars = results.StructuredOutput(i).Variables;

        retElem = {};
        retElem.depotFile     = "" + vars.Variable("depotFile");
        retElem.haveRev     = parseInt(vars.Variable("haveRev"));
        retElem.clientFile     = "" + vars.Variable("clientFile");
        retElem.type         = "" + vars.Variable("type");
        retElem.localFile     = "" + vars.Variable("localFile");
        ret.push(retElem);
    }
    return ret;
}

/*****************************************************************************/
/* does a 'sd dirs' of 'fileSpec' and returns the list of source depot paths
   that are returned */
 
function sdDirs(fileSpec, sdObj) {

    sdObj = sdConnect(sdObj);

    logMsg(LogSourceDepot, LogInfo100, "sdDirs(", fileSpec, ")\n");
    var results = _sdRun(sdObj, "dirs " + fileSpec);

    var ret = []
    for(var i = 0; i < results.StructuredOutput.Count; i++)  {
        var vars = results.StructuredOutput(i).Variables;

        ret.push(("" + vars.Variable("dir")).toLowerCase());
    }
    return ret;
}

/*****************************************************************************/
/* does a 'sd changes on 'fileSpec' and returns the list of change records */
 
function sdChanges(fileSpec, sdObj) {

    sdObj = sdConnect(sdObj);

    logMsg(LogSourceDepot, LogInfo100, "sdChanges(", fileSpec, ")\n");
    var results = _sdRun(sdObj, "changes " + fileSpec);

    var ret = []
    for(var i = 0; i < results.StructuredOutput.Count; i++)  {
        var vars = results.StructuredOutput(i).Variables;

        retElem = {};
        retElem.change         = parseInt(vars.Variable("change"));
        retElem.time         = parseInt(vars.Variable("time")) * 1000;
        retElem.desc        = "" + vars.Variable("desc");
        retElem.user        = "" + vars.Variable("user");
        ret.push(retElem);
    }
    return ret;
}

/*****************************************************************************/
/* Runs sd filelog on 'fileSpec' (which is thus any SD filename-revision spec.
   and returns an array of results (do a runjs sdFileLog <file> to see
   the format of the fields).  The top level structure that is returned is
   a bit intersting.  It is a map keyed by depotName, however it also lookes
   like an array (keyed by number and having a 'length' property).  Thus you
   can access it either way depending one which way is more convinient.  

   If 'ftn' is provided, it is assumed to be a 
   function to be called with each record fo the file log.  If 'ftn' is not
   provided, these records are accumulated are returned as a list.  If the
   data returned is large, performance can be better using 'ftn' since JScript
   thrashes on large live data sets.
*/
function sdFileLog(fileSpec, sdObj, ftn) {

    sdObj = sdConnect(sdObj);

    logMsg(LogSourceDepot, LogInfo100, "sdFileLog(", fileSpec, ")\n");
    var results = _sdRun(sdObj, "filelog " + fileSpec);

    var ret = undefined;
    if (!ftn)  {
        ret = {};
        ret.length = results.StructuredOutput.Count;
    }

    for(var i = 0; i < results.StructuredOutput.Count; i++)  {
        var vars = results.StructuredOutput(i).Variables;
        var depotFile = ("" + vars.Variable("depotFile")).toLowerCase();
        var revs;
        if (!ftn) 
            revs = ret[depotFile] = ret[i] = [];
        
        for(var j = 0; ; j++) {
            var rev = vars.VariableX("rev", j);
            if (rev == undefined)
                break;
            rev = parseInt(rev);

            var rec = {};
            rec.depotFile = depotFile;
            rec.change = parseInt(vars.VariableX("change", j));
            rec.action = "" + vars.VariableX("action", j);
            rec.rev = rev;
            // rec.type = "" + vars.VariableX("type", j);
            // rec.client = "" + vars.VariableX("client", j);
            rec.time = parseInt(vars.VariableX("time", j)) * 1000;
            rec.user = "" + vars.VariableX("user", j);
            rec.desc = "" + vars.VariableX("desc", j);

            for(k = 0; ; k++) {
                var how = vars.VariableXY("how", j, k);
                if (how == undefined)
                    break;
                how = "" + how;

                var subRec = {};
                subRec.how = how;
                subRec.file = ("" + vars.VariableXY("file", j, k)).toLowerCase();
                if (how.match(/^(delete from)|(branch from)|(copy from)|(merge from)|ignored$/)) {
                    subRec.srev = parseInt(vars.VariableXY("srev", j, k));
                    subRec.erev = parseInt(vars.VariableXY("erev", j, k));
                    rec.from = subRec;
                }
                else {
                    if (!how.match(/^(delete into)|(branch into)|(add into)|(edit into)|(copy into)|(merge into)|(ignored by)$/)) 
                        throwWithStackTrace(Error(1, "Unknown file log entry type " + how));

                    subRec.rev = parseInt(vars.VariableXY("erev", j, k));
                    if (!rec.to)
                        rec.to = [];
                    rec.to.push(subRec);
                }
            }
            if (ftn)
                ftn(rec)
            else 
                revs.push(rec)
        }
    }
    return ret;
}

/*****************************************************************************/
/* Runs sd describe on changeNumber 'changeNum', returns a structure containing
   the information ('do runjs sdDescribe <num>' for an example) */

function sdDescribe(changeNum, sdObj) {

    sdObj = sdConnect(sdObj);

    logMsg(LogSourceDepot, LogInfo100, "sdDescribe(", changeNum, ")\n");
    var results = _sdRun(sdObj, "describe -s " + changeNum);

    var ret = {};
    if (!(results.StructuredOutput.Count > 0)) 
        throwWithStackTrace(Error(1, "sd Describe fails on " + changeNum))

    var vars = results.StructuredOutput(0).Variables;

    ret.change = parseInt(vars.Variable("change"));
    ret.user = "" + vars.Variable("user");
    // ret.client = "" + vars.Variable("client");
    ret.time = parseInt(vars.Variable("time")) * 1000;
    ret.desc = "" + vars.Variable("desc");
    ret.status = "" + vars.Variable("status");
    ret.files = [];

    for(var j = 0; ; j++) {
        var rev = vars.VariableX("rev", j);
        if (rev == undefined)
            break;
        var rec = {};
        rec.rev = parseInt(rev);
        rec.depotFile = "" + vars.VariableX("depotFile", j);
        rec.action = "" + vars.VariableX("action", j);
        // rec.type = "" + vars.VariableX("type", j);
        ret.files.push(rec)
    }
    return ret;
}

/*****************************************************************************/
/* Given a SD file spec, return the full depot path associated with it */
   
function sdxSyncAndResolve() 
{
    var srcBase = srcBaseFromScript();
    var run = runCmdToLog("pushd " + srcBase + " & call " + srcBase + "\\tools\\razzle.cmd " + " Exec " + "sdx sync",
        runSetNoThrow(
        runSetTimeout(3 * HOUR,   // don't want to timeout on long sync
        runSet32Bit)));
    if (run.exitCode == 0)
    {
        run = runCmdToLog("pushd " + srcBase + " & call " + srcBase + "\\tools\\razzle.cmd " + " Exec " + "sdx resolve -am",
            runSetNoThrow(
            runSetTimeout(3 * HOUR,   // don't want to timeout on long sync
            runSet32Bit)));
    }
    return run.exitCode;
}


/*****************************************************************************/
/*************************  VALUE ADD FUNCTIONS ******************************/
/*****************************************************************************/

/*****************************************************************************/
/* Do a sync on a directory in chunks to avoid SD maximums.
     Parameters
    dir    : the directory to sync
    label  : a label to sync to
    sdEnv  : run options (env vars) passed to run (by default set to 
             set current directory to 'dir' and adds \\clrmain\tools\x86 to 
             the path so that sd.exe will be found even on a clean machine
    depth  : depth to recurse before doing a normal sync (default 1)
    force  : if true, do a forced sync (expensive!)
 */
function sdSync(dir, label, sdEnv, depth, force) {

    if (dir == undefined)
        dir = ".";
    if (sdEnv == undefined) {
        // we put sd.exe on the path so that it will work from an absolutely clean machine
        // we run the SD commands from the sdRoot directory so that the SD.INI file gets used to find the depot
        FSOCreatePath(dir);
        dir = FSOGetFolder(dir).Path;            // we are about to change current directory, so get a full path
        sdEnv = runSetCwd(dir);
        sdEnv = runSetEnv("PATH", "\\\\clrmain\\tools\\x86" + ";%PATH%", sdEnv);
    }
    if (label == undefined) {
        if (!(runCmd("sd counter change", sdEnv).output.match(/^\s*(\d+)/)))
            throw new Error(-1, "Could not get latest sync point counter");
        label = RegExp.$1;
    }
    if (depth == undefined)
        depth = 1;

    // logCall(LogSourceDepot, LogInfo, "sdSync", arguments);

    dir = dir.replace(/\\\.($|\\)/g, "$1");                 // XXX\.\YYY => XXX\YYY
    dir = dir.replace(/\\$/, "");                           // XXX\          => XXX

    // If the diretctory has no files, do a sync -f, because clearly everything needs to copied down.
    var fileSyncCmd = "sd sync ";
    if (force || !FSOFolderExists(dir) || FSOGetFilePattern(dir).length == 0)
        fileSyncCmd += "-f ";
    runCmd(fileSyncCmd + dir + "\\*@" + label, sdEnv);
    
    --depth;
    var subDirs = runCmd("sd -s dirs " + dir + "\\*", sdEnv).output.split(/\s*\n\s*/);

    for(var i = 0; i < subDirs.length; i++) {
        if (subDirs[i].match(/^info:.*\/([^/]*)$/)) {
            var subDir = dir + "\\" + RegExp.$1;
            
            if (depth > 0)
                sdSync(subDir, label, sdEnv, depth);
            else {
                var dirSyncCmd = "sd sync ";
                if (force || !FSOFolderExists(subDir) || (FSOGetFilePattern(subDir).length == 0 && FSOGetDirPattern(subDir).length == 0))
                    dirSyncCmd += "-f ";
                logMsg(LogSourceDepot, LogInfo, dirSyncCmd, subDir, "\\...");
    

                    // This is logically just a runCmd, but we want to do '.' to indicate progress
                run = runDo(dirSyncCmd + subDir + "\\...@" + label, runSetTimeout(14400, sdEnv));
                var j = 0;
                var lastSize = run.output.length;
                while(!runPoll(run)) {
                    WScript.Sleep(1000);
                    j++;
                    if (j > 20) {
                        if (run.output.length != lastSize) {
                            logMsg(LogSourceDepot, LogInfo, ".");
                            lastSize = run.output.length;
                        }
                        j = 0;    
                    }
                }

                var lines = run.output.split("\n");
                var count = 0;
                for (var j = 0; j < lines.length; j++) {
                    if (lines[j].match(/\/\/depot.* - /))
                        count++;
                }
                logMsg(LogSourceDepot, LogInfo, " ", count, " Files\n");
            }
        }
    }
    return 0;
}

/*****************************************************************************/
/* Do a sdSync of 'dir' and then do a resolve -am */

function sdSyncResolve(dir, label) {

    if (dir == undefined)
        dir = ".";

    sdSync(dir, label);

        // first do a resolve -as so that all the 'interesting' ones are at the end
    logMsg(LogSourceDepot, LogInfo, "Doing sd resolve -as to get rid of the 'easy' ones first\n");
    runCmdToLog("sd resolve -as " + dir + "\\...", runSetNoThrow());

    logMsg(LogSourceDepot, LogInfo, "Handling any conflicts\n");
    var run = runCmdToLog("sd resolve -am " + dir + "\\...", runSetNoThrow());

    if (run.exitCode != 0)
        logMsg(LogSourceDepot, LogError, "sd resolve -am failed, please run 'sd resolve' by hand\n");
    return run.exitCode
}

/*****************************************************************************/
/* Clean out any files in 'dir' that are not also in the depot.

   Parameters:
      dir     : The directory to clean out
      options : A list of qualifiers (no spaces between them).
      /quiet     : Don't show what you are doing
      /justShow  : Just show what you would have done
      excPat  : a pattern to exclude.  If the complete path to a file matches
            this pattern, then it will not be deleted.  Note that the file
            could still be deleted if it is in a directory being deleted.
      workingDirectory: a directory from which the sd.exe program should be
            started. Useful if sd.exe should use certain sd.ini file.
*/

function sdClean(dir, options, excPat, workingDirectory) {

    if (dir == undefined)
        dir = ".";

    var runOptions;
    if( workingDirectory != undefined )
        runOptions =     runSetCwd( workingDirectory, runSetEnv("PATH", "%PATH%;\\\\clrmain\\tools\\x86") );

    logCall(LogSourceDepot, LogInfo100, "sdClean", arguments);
    var opts = getOptions(["quiet", "justShow"], options);
    if (excPat != undefined)
        excPat = new RegExp(excPat, "i");

            // for the set of all files under dir
    var inSD = {};
    var have = runCmd("sd have \"" + dir + "\\...\"", runOptions).output.split("\n");
    for (var i=0; i < have.length; i++) {
        if (have[i] == "") continue;
    if (have[i].match(/^.* - server may not have accurate information about local revision/)) continue;
        if (!have[i].match(/^.*#\d+ - (.*?)\s*$/))
        {

            throw Error(-1, "Unparsable sd have line '" + have[i] + "'");
        }
        _addPath(inSD, RegExp.$1.toLowerCase());
    }

            // Also add in all opened files (otherwise files you added but not checked will be deleted!
    var opened = runCmd("sd opened -l \"" + dir + "\\...\"", runOptions).output.split("\n");
    for (var i=0; i < opened.length; i++) {
        if (i == 0 && opened[i].match(/not opened on this/im)) break;
        if (opened[i] == "") continue;
        if (!opened[i].match(/^(.*)#\d+ - /))
            throw Error(-1, "Unparsable sd opened line '" + opened[i] + "'");
        _addPath(inSD, RegExp.$1.toLowerCase());
    }

            // OK start the recurisve directory comparison
    _sdCleanWalk(dir, inSD, opts.quiet == undefined, opts.justShow, excPat);
    return 0;
}

/*****************************************************************************/
/*      Add all subPaths (parent directories) of 'path' to the set 'set' */

function _addPath(set, path) {

    for(;;) {
        if (set[path])
            break;
        logMsg(LogSourceDepot, LogInfo10000, "_addPath: adding '", path, "'\n");
        set[path] = true;
        if (!path.match(/(.*)\\[^\\]*$/))
                break;
        path = RegExp.$1;
    }
}

/*****************************************************************************/
/* recursive walker for sdClean */

function _sdCleanWalk(dir, inSD, verbose, justShow, excPat) {

    logMsg(LogSourceDepot, LogInfo1000, "sdCleanWalk(", dir, ", inSD, ", verbose, ", ", justShow, ") {\n");

    var files = FSOGetFilePattern(dir);
    for (var i=0; i < files.length; i++) {
        var file = files[i].toLowerCase();
        if (!inSD[file]) {
            if ((excPat != undefined && file.match(excPat))) {
                if (verbose)
                    WScript.Echo("echo Skipped excluded file " + file);
            }
            else {
                if (verbose)
                    WScript.Echo("del /f " + file);
                if (!justShow)
                    try { FSODeleteFile(file, true); } catch(e) {}
            }
        }
    }

    var subDirs = FSOGetDirPattern(dir);
    for (var i=0; i < subDirs.length; i++) {
        var subDir = subDirs[i].toLowerCase();
        if (inSD[subDir])
                _sdCleanWalk(subDirs[i], inSD, verbose, justShow)
        else  {
            if (verbose);
                WScript.Echo("rmdir /s /q " + subDir);
            if (!justShow)
                try { FSODeleteFolder(subDir, true); } catch(e) {}
        }
    }
    logMsg(LogSourceDepot, LogInfo1000, "} sdCleanWalk()\n");
}

/*****************************************************************************/
/* creates a new source depot client. It can be run on existing clients 
   without harm (it is a no-op).  It will create an sd.ini file in the
   localBase directory that contains the information to contact the depot.
   It does not create any mappings, (use sdClientMap to add these) 

   depotPort  : The source depot port specification (eg DDRTSD:4000)
   localBase  : The path where the local image goes (eg C:\src)
   clientName : The name of the client to create (defaults to machine name)
   sdEnv      : run options (env vars) passed to run (by default set to 
                set current directory to 'dir' and adds \\clrmain\tools\x86 
                to the path so that sd.exe will be found.
*/
function sdClientCreate(depotPort, localBase, clientName, sdEnv) {

    if (depotPort == undefined)
        throw new Error(1, "Required arg 'depotPort' not present");
    if (localBase == undefined)
        throw new Error(1, "Required arg 'localBase' not present");
    if (clientName == undefined)
        clientName = Env("COMPUTERNAME");
    if (sdEnv == undefined) {
        // we put sd.exe on the path so that it will work from an absolutely clean machine
        // we run the SD commands from the sdRoot directory so that the SD.INI file gets used to find the depot
        FSOCreatePath(localBase);
        localBase = FSOGetFolder(localBase).Path;    // we are about to change current directory, so get a full path
        sdEnv = runSetCwd(localBase);
        sdEnv = runSetEnv("PATH", "\\\\clrmain\\tools\\x86" + ";%PATH%", sdEnv);
    }

    var sdIniData = ""
    var sdIniName = localBase + "\\sd.ini";
    if (FSOFileExists(sdIniName)) {
        sdIniData = FSOReadFromFile(sdIniName)
        sdIniData = sdIniData.replace(/^SDPORT *=.*(\r|\n)*/m, "");
        sdIniData = sdIniData.replace(/^SDCLIENT *=.*(\r|\n)*/m, "");
    }

    var sdPortLine   = "SDPORT=" + depotPort + "\r\n";
    var sdClientLine = "SDCLIENT=" + clientName + "\r\n";
    if (!sdIniData.match(/^SDDIFF *=/m))
        sdIniData = "SDDIFF=windiff\r\n" + sdIniData;
    if (!sdIniData.match(/^SDMERGE *=/m))
        sdIniData = "SDMERGE=vssmerge\r\n" + sdIniData;

    FSOCreatePath(localBase);
    FSOWriteToFile(sdIniData + sdPortLine + sdClientLine, sdIniName);

    var sdClientCmd = "sd.exe -d " + localBase + " " + " -i " + sdIniName + " client";
    var clientTemplate = runCmd(sdClientCmd + " -o " + clientName, sdEnv).output;

    if (!clientTemplate.match(/^Root:\s*(.*\S)/im))
        throw Error(1, "Could not find root in the client specification");
    var root = RegExp.$1.toLowerCase();
    localbase = localBase.toLowerCase();
    if (localbase != root)
        throw Error(1, "Mismatch localBase = " + localBase + " != " + root + " == sd root\r\n" +
                       "This is usually caused because the client already exists and you are using\r\n" +
                       "a different root.  Either delete the client or use the existing root\r\n")

    logMsg(LogSourceDepot, LogInfo, "Creating client ", clientName, " at ", localBase, " to depot ", depotPort, "\n");
    runCmd(sdClientCmd + " -i", runSetInput(clientTemplate, sdEnv));
    return 0;
}

/*****************************************************************************/
/* Addes a mapping to an existing source depot client.  'localBase' is where
   the sd.ini file lives.  

   localBase     : The directory where the sd.ini file lives
   fullDepotPath : The path in the depot (can leave off the //depot/ )
   relLocalPath  : The file path relative to 'localBase'
   sdEnv      :    run options (env vars) passed to run (by default set to 
                   set current directory to 'dir'.  Adds \\clrmain\tools\x86 
                   to the path so that sd.exe will be found 
*/

function sdClientMap(localBase, fullDepotPath, relLocalPath, sdEnv) {

    // logCall(LogSourceDepot, LogInfo, "sdClientMap", arguments);

    if (localBase == undefined)
        throw new Error(1, "Required arg 'localBase' not present");
    if (fullDepotPath == undefined)
        throw new Error(1, "Required arg 'fullDepotPath' not present");
    if (relLocalPath == undefined)
        relLocalPath = fullDepotPath.match(/(\w+)$/)[1];
    if (sdEnv == undefined) {
        // we put sd.exe on the path so that it will work from an absolutely clean machine
        // we run the SD commands from the sdRoot directory so that the SD.INI file gets used to find the depot
        FSOCreatePath(localBase);
        localBase = FSOGetFolder(localBase).Path;        // make certain we have the full path
        sdEnv = runSetCwd(localBase);
        sdEnv = runSetEnv("PATH", "\\\\clrmain\\tools\\x86" + ";%PATH%", sdEnv);
    }

    // we allow people to leave off the //depot because the command line
    // parsing of cscript interprets // as qualifers to cscript!

    if (fullDepotPath.match(/^\w/i))
        fullDepotPath = "//depot/" + fullDepotPath;

    var sdIniName = localBase + "\\sd.ini";
    var sdClientCmd = "sd.exe -d " + localBase + " " + " -i " + sdIniName + " client";
    var clientData = runCmd(sdClientCmd + " -o ", sdEnv).output;

    if (!clientData.match(/((.|\n)*View:\s*\n)((.|\n)*)/i))
        throw new Error(1, "Could not find 'View' in client spec");
    var preView = RegExp.$1;
    var view = RegExp.$3;

    if (!clientData.match(/^Client:\s*(\S+)/mi)) 
        throw new Error(1, "Could not find 'Client' in client spec");
    var clientName = RegExp.$1;

        // remove redundant stuff
    var targetLocalPath = relLocalPath.toLowerCase();
    targetLocalPath = targetLocalPath.replace(/\\/g, "/");
    var fullDepotSpec = fullDepotPath.toLowerCase() + "/...";

    var newView = "";
    while(view.match(/^\s*(\S+) +(\S+)\s*\n/)) {
        var depotPath = RegExp.$1;
        var localPath = RegExp.$2;
        view = RegExp.rightContext;

        if (!localPath.match(/^\/\/.*?\/(.*)/))
            throw new Error(1, "Unexpected form for local path " + localPath);
        var mapLocalPath = RegExp.$1.toLowerCase();
        mapLocalPath = mapLocalPath.replace(/(\/)?\.\.\.$/, "");

            // remove the default complete depot mapping.  
        if (depotPath.toLowerCase() == "//depot/..." && view.match(/^\s*$/))
            break;

            // remove existing mappings  
        if (targetLocalPath == mapLocalPath)
            continue;

        if (depotPath.toLowerCase() == fullDepotSpec) {
            throw new Error(1, "\r\nError: The following mapping already exists in your client\r\n" +
                               "    " + depotPath + " " + localPath + "\r\n" + 
                               "and we want to change it to\r\n" + 
                               "    " + depotPath + " //" + clientName + "/" + targetLocalPath + "...\r\n" + 
                               "which would destroy the old mapping.  If it really is your intent to\r\n" + 
                               "change this mapping delete the old mappings first with 'sd client'.\r\n" +
                               "\r\n" +
                               "Normally however this error occurs when what you really wanted was another\r\n" +
                               "enlistment WITH ITS OWN CLIENT on the same machine\r\n" +
                               "To do this see runjs /? setupCLRDev for more\r\n");
        }

        newView += "    " + depotPath + " " + localPath + "\r\n";
    }
    if (!view.match(/^\s*$/))
        throw Error(1, "Could not parse view spec '" + view + "'");

        // Add the new spec to the view list
    if (targetLocalPath != "")
        targetLocalPath += "/";
    newView += "    " + fullDepotSpec + " //" + clientName + "/" + targetLocalPath + "...\r\n";

        // Send it off to SD
    clientData = preView + newView;
    var fullLocalPath = localBase + "\\" + relLocalPath;
    FSOCreatePath(fullLocalPath);
    logMsg(LogSourceDepot, LogInfo, "Adding SD mapping ", fullDepotPath, " -> ", fullLocalPath, "\n");
    runCmd(sdClientCmd + " -i", runSetInput(clientData, sdEnv));

    return 0;
}

/*****************************************************************************/
/* Determine when 'dir' was last synced.  Note that you can only determine
   this down to a range (since any sync in that range will yield the same
   set of files), it is also possible that there is no one sync point (if 
   you sync individual files to different points in time).  In this latter
   case.  Note that maxChange is not inclusive (that is syncing to this
   label will cause a change). 
*/
function sdWhereSynced(dir, sdObj) {

    if (_inTFS())
      return tfWhereSynced(dir);

    if (dir == undefined) 
        dir = ".";

    sdObj = sdConnect(sdObj);
    var results = _sdRun(sdObj, "filelog " + dir + "\\...#have,");


    var maxChange = 0x7FFFFFFF;
    var maxChangeTime = new Date().getTime().toString();
    var minChange = 0;
    var minChangeTime = 0;

    var srcBase = srcBaseFromScript();
    
    for(var i = 0; i < results.StructuredOutput.Count; i++)  {
        var vars = results.StructuredOutput(i).Variables;

        var depotFile = vars.Variable("depotFile")
        logMsg(LogSourceDepot, LogInfo1000, "depotFile: ", depotFile, "\n");

        var next = 0x7FFFFFFF;
        var nextTime;
        var have = 0x7FFFFFFF;
        var haveTime;
        for(var j = 0; ; j++) {
            var change = vars.VariableX("change", j);
            if (change == undefined)
                break;
            var change = parseInt(change);
            next = have;
            nextTime = haveTime
            have = change;
            haveTime = parseInt(vars.VariableX("time", j));
        }

        if (next < maxChange && next >= minChange) {
            maxChange = next;
            maxChangeTime = nextTime;
        }

        if (have > minChange && have <= maxChange) {
            minChange = have;
            minChangeTime = haveTime;
        }
        logMsg(LogSourceDepot, LogInfo10000, "    have: ", have, " minChange: ", minChange, " next: ", next, " maxChange: ", maxChange, "\n");
    }

    if (minChange >= maxChange) 
        return undefined;

    return { minChange: minChange, minChangeTime: minChangeTime,
             maxChange: maxChange, maxChangeTime: maxChangeTime
           };
}

function sdLastSync(dir, sdObj)
{
    var result = sdWhereSynced(dir, sdObj);
    WScript.Echo("Start: " + result.minChange);
    WScript.Echo("End: "   + result.maxChange);
    return true;
}

function tfWhereSynced(dir)
{
    if (dir == undefined) 
        dir = ".";
    
    var oExec = WshShell.Exec(TFScriptCall() + " whereSynced " + dir);
    var sOutput="";

    while (!oExec.StdOut.AtEndOfStream) {
        sOutput += oExec.StdOut.Read(1);
    }

    // The output should be a number comma date
    var reLine =  /(\d*),(.*)\r\n/;
    var matched = reLine.exec(sOutput);
    if (!matched)
        throw new Error(1, "Could not determine where synced from TFS running 'tfscript whereSynced "+dir +"'");

    minChange = RegExp.$1;
    minChangeTime = RegExp.$2;

    // max is not important
    var maxChange = 0x7FFFFFFF;
    var maxChangeTime = new Date().getTime().toString();

    return { minChange: minChange, minChangeTime: minChangeTime,
             maxChange: maxChange, maxChangeTime: maxChangeTime
           };

}

/*****************************************************************************/
/* Given 'fileOrChangeSpec', display a report on where that change has 
   propagated in the depot. 'fileOrChangeSpec' can either be a normal
   SD file specification (eg class.cpp@3435343) or a change number (eg 23423)
   In the later case sdFollow simply choses file in the change to follow 
   through the depot.   This works fine assuming that people always integrate 
   all files in a change (which we always do, but is not guarenteed). 
*/
function sdFollow(fileOrChangeSpec, sdObj) {

    var info = {};
    info.depotPaths = {};
    info.fileLogs = {};
    info.sdObj = sdConnect(sdObj);

    var fileSpec = fileOrChangeSpec;
    WScript.StdOut.WriteLine("");
    if (typeof(fileOrChangeSpec) == "number" || fileOrChangeSpec.match(/^\d+$/)) {
        var changeNum = fileOrChangeSpec;
        var desc = sdDescribe(changeNum, info.sdObj);
        fileSpec = desc.files[0].depotFile + "@" + changeNum;
        WScript.StdOut.WriteLine("Using file: " + desc.files[0].depotFile)
        WScript.StdOut.WriteLine("to follow change " + changeNum);
        WScript.StdOut.WriteLine("");
    }

    var fileLogs = sdFileLog(fileSpec + ",", info.sdObj)[0];
    if (fileLogs == undefined)
        return 0;
    var fileLog = fileLogs[fileLogs.length-1];
    info.depotPaths[fileLog.depotFile.toLowerCase()] = true;
    // logMsg(LogSourceDepot, LogInfo, "sdFollow: {\n", dump(fileLog), "}\n");
    WScript.StdOut.WriteLine("Following: " + fileSpec);
    WScript.StdOut.WriteLine("Depot Rev: " + fileLog.depotFile + "#" + fileLog.rev);
    WScript.StdOut.WriteLine("Change: " + fileLog.change + " Date: " + prettyDate(new Date(fileLog.time)) + " User: " + fileLog.user);
    WScript.StdOut.WriteLine("");
    WScript.StdOut.WriteLine("        integrated to                                  change   date     user ");
    WScript.StdOut.WriteLine("-------------------------------------------------------------------------------");
    info.baseNameHnd = _sdShortNameNew(fileLog.depotFile);
    _sdFollowHelper(fileLogs, info, "");
    return 0;
}

function _sdFollowHelper(fileLogs, info, indent) {

    var fileLog = fileLogs[fileLogs.length-1];

    for (var j = fileLogs.length-1; j >= 0; --j) {
        var fileLog = fileLogs[j];
        // logMsg(LogSourceDepot, LogInfo, "sdFollow: ", dump(fileLog), "\n");

        if (fileLog.to) {
            for (var i = 0; i < fileLog.to.length; i++) {
                var to = fileLog.to[i];
                var normToFile = to.file.toLowerCase();
                if (!info.depotPaths[normToFile]) {
                    info.depotPaths[normToFile] = true;
                    var toFileSpec = normToFile + "#" + to.rev;
                    var toFileLogs;
                    try
                    {
                        toFileLogs = sdFileLog(toFileSpec + ",", info.sdObj)[0];
                        var toFileLog = toFileLogs[toFileLogs.length-1];

                        var user = toFileLog.user;
                        if (user.match(/.*\\([^\\]+)$/))
                            user = RegExp.$1;
                        WScript.StdOut.WriteLine(
                            padRight(indent + _sdShortNameForPath(toFileLog.depotFile, info.baseNameHnd) + "#" + toFileLog.rev, 53) + " " +
                            padLeft(toFileLog.change, 7) + " " + prettyDate(new Date(toFileLog.time)) + " " + padLeft(user, 8));

                        _sdFollowHelper(toFileLogs, info, indent + " ");
                    }
                    catch(e)
                    {
                        WScript.StdOut.WriteLine(e);
                    }

                }
            }
        }
    }
}

/*****************************************************************************/
/* FIX document */

function removeCommonSuffix(str1, str2) {
    var i1 = str1.length-1;
    var i2 = str2.length-1;
    while (i1 >= 0 && i2 >= 0 && str1.charCodeAt(i1) == str2.charCodeAt(i2)) {
        --i1;
        --i2;
    }
    return str1.substr(0, i1+1)
}

/*****************************************************************************/
/* Given a name of lab, return its full source depot path.  If a full path
   is given, it is simply returned.  Note that this is based on devdiv 
   source depot conventions. */

function sdLabPath(labName, sdObj) {

    if (labName == undefined)
        throw new Error(1, "Required arg 'labName' not provided.");

    labName = labName.toLowerCase();
    if (!labName.match(/^[^\/]*$/))        // if the name is a path, then don't morph it.
        return labName;

    sdObj = sdConnect(sdObj);

        // search in the common places for lab names to be (this is obviously heuristic
    var dirs = sdDirs("//depot/devdiv/*", sdObj);
    var pat = new RegExp("\/" + labName + "$");
    dirs = dirs.concat(sdDirs("//depot/devdiv/private/*", sdObj));

    for (var i = 0; i < dirs.length; i++) {
        if (dirs[i].match(pat))
            return (dirs[i]);
    }
    throw new Error(1, "Could not find '" + labName + "' in the devdiv depot.\n" +
        "Do 'sd dirs //depot/devdiv/*' and //depot/devdiv/private/*' for list of labs.");
}

/*****************************************************************************/
/* FIX document */
function sdLabFollow(labName, daysAgo, sdObj) {

    sdObj = sdConnect(sdObj);
    var labPath = sdLabPath(labName);
    logMsg(LogSourceDepot, LogInfo, "labPath = ", labPath, "\n");

    sdDirFollow(labPath + "/ndp", daysAgo, sdObj);
}

/*****************************************************************************/
/* FIX document */

function sdDirFollow(dirSpec, sinceSpec, sdObj) {

    if (dirSpec == undefined)
        dirSpec = "";
    if (sinceSpec == undefined)
        daysAgo = "@-60";

    sdObj = sdConnect(sdObj);

        // These are the output of the iteration below
    var integTargets = {};            // Maps labs to the integration sync point range that works for ALL files that we have seen so far
    var changesInfo = {};            // Maps a change number what we know about it (list of files, time)

    var restIntegTargets = {        // represents all integration targets we have not seen so far.
        minChange:0,
        maxChange:0x7FFFFFFF
    };
        
        // These are variables that a reset on each new file  (thus they apply to the current file)
    var curFile = undefined;        // the name of hte currnet file.
    var haveIntegratedToTarget;        // maps lab targets to a boolean, have we done an integration for the current file?

        // These variables change on each distinct change
    var lastChanges;                // maps lab targets into the change number 'after' (in time), the current change
    var restLastChange;                // the value of lastChange for any labs not in the 'lastChange' map
                            
    sdFileLog(dirSpec + "..." + sinceSpec + ",", sdObj, function(fileInfo) {
        // logMsg(LogSourceDepot, LogInfo, "Got", dump(fileInfo), "\n");

        if (fileInfo.depotFile != curFile) {
                // when we reach the end of the changes for a particular file, we need
                // eliminate any target labs that have we did not integrate files to
                // since we only want labs in which ALL files have propagated to. 
            _updateTargetsNotIntegrated(haveIntegratedToTarget, integTargets, restIntegTargets, lastChanges, restLastChange, curFile);
            
            curFile = fileInfo.depotFile;
            restLastChange = 0x7FFFFFFF;
            lastChanges = {};
            haveIntegratedToTarget = {};
        }

            // accumulate the set of files modified for every change we see
        var changeInfo = changesInfo[fileInfo.change];
        if (!changeInfo) 
            changeInfo = changesInfo[fileInfo.change] = { time:fileInfo.time, fileSet:{} };
        changeInfo.fileSet[fileInfo.depotFile] = true;
        
        if (fileInfo.to) {
            for (var i = 0; i < fileInfo.to.length; i++) {
                var to = fileInfo.to[i];
                var target = removeCommonSuffix(to.file, fileInfo.depotFile);

                if (!haveIntegratedToTarget[target]) {
                    var integTarget = integTargets[target];
                        // we only create targets on the first file.  The reason is that we
                        // only care about integrations that are valid for ALL files, so we
                        // should only be making new ones on the very first file.  Otherwise
                        // we ignore this integration since it can't be part of one 
                    if (!integTarget) {
                        integTarget = integTargets[target] = {};
                        integTarget.maxChange = restIntegTargets.maxChange;
                        integTarget.minChange = restIntegTargets.minChange;
                    }

                    if (integTarget) {
                        if (integTarget.minChange < fileInfo.change && fileInfo.change < integTarget.maxChange) {
                            var lastChange = lastChanges[target];
                            if (!lastChange)
                                lastChange = restLastChange;

                            if (integTarget.maxChange > lastChange)
                                integTarget.maxChange = lastChange
                            integTarget.minChange = fileInfo.change;
                            integTarget.to = to;        // remember where to goes to to get the associated change number later
                        }
                    }
                    haveIntegratedToTarget[target] = true;
                }
            }

        }

            // update the last change, Sadly, this depends on the target as we want to
            // ignore changes that come from particular targets when calcuating the last change. (ugh).
        if (fileInfo.from) {
            var target = removeCommonSuffix(fileInfo.from.file, fileInfo.depotFile);
            var lastChange = lastChanges[target];
            if (!lastChange)
                lastChange = restLastChange;

            lastChanges = {};
            lastChanges[target] = lastChange;     // ignore this change for just this target.
        } 
        else {     // non-integrations update the last change for everyone.
            lastChanges = {};
        }
        restLastChange = fileInfo.change;
    });

    _updateTargetsNotIntegrated(haveIntegratedToTarget, integTargets, restIntegTargets, lastChanges, restLastChange, curFile);
    // logMsg(LogSourceDepot, LogInfo, "After file ", curFile, ", fileSetForChange = ", dump(fileSetForChange), "\n");
    logMsg(LogSourceDepot, LogInfo, "integTargets = ", dump(integTargets), "\n");

    WScript.StdOut.WriteLine("For dir: " + dirSpec);

    for (var target in integTargets) {
        var integTarget = integTargets[target];

        WScript.StdOut.WriteLine(" -> " + target);
        WScript.StdOut.Write("    [" + integTarget.minChange + ", " + integTarget.maxChange  + /* ( */ ")");

        if (integTarget.minChange != 0) {
            var changeInfo = changesInfo[integTarget.minChange];
            WScript.StdOut.Write(" date " + prettyDate((new Date(changeInfo.time))));
        }

        if (integTarget.to) {
            var fileInfoSpec = integTarget.to.file + "#" + integTarget.to.rev + "," + integTarget.to.rev;
            var fileInfo = sdFileLog(fileInfoSpec, sdObj)[0][0]
            WScript.StdOut.Write(" change " + fileInfo.change + " on " + prettyDate((new Date(fileInfo.time))));
        }
        WScript.StdOut.WriteLine("")
    }
}

/*****************************************************************************/
/* update any targets from 'integTargets' that is not in the set haveIntegratedToTarget */

function _updateTargetsNotIntegrated(haveIntegratedToTarget, integTargets, restIntegTargets, lastChanges, restLastChange, curFile) {

    for (var target in integTargets) {
        if (!haveIntegratedToTarget[target]) {
            var integTarget = integTargets[target];

            var lastChange = lastChanges[target];
            if (!lastChange)
                lastChange = restLastChange;

            if (integTarget.maxChange > lastChange)
                integTarget.maxChange = lastChange
        }
    }
        // also do the update for 'restIntegTargets' which represents all lab targets we have not seen yet. 
    if (restIntegTargets.maxChange > restLastChange)
        restIntegTargets.maxChange = restLastChange;

    // logMsg(LogSourceDepot, LogInfo, "After file ", curFile, ", integTargets = ", dump(integTargets), "\n");
}

/*****************************************************************************/
/* a helper function that runs a sd command 'cmd' (eg, sync, edit, open ...)
   and warning and error diagnostics.  If 'structured' is true the 
   information is returned as preparsed data in the 'StructuredOutput' field.
   If 'displayInfo' is true information level diagnositics are also printed.  
*/
function _sdRun(sdObj, cmd, structured, displayInfo) {

    if (structured == undefined)
        structured = true;

    logMsg(LogSourceDepot, LogInfo1000, "_sdRun(sdObj, ", cmd, ") {\n");
    var results = sdObj.Run(cmd, structured);
    var i = 0;
    var mult= 1;
    while(!results.IsFinished) {
        WScript.Sleep(100*mult);
        i++;
        if (i > 50) {
            mult = 4;
            if (i == 51)
                WScript.StdOut.Write("\r\nSD taking a while...");
            else if (i % 10 == 0)
                WScript.StdOut.Write(".");
        }
    }
    if (i > 50)
        WScript.StdOut.WriteLine("");

    logMsg(LogSourceDepot, LogInfo10000, "_sdRun: waiting\n");
    results.WaitUntilFinished();
    logMsg(LogSourceDepot, LogInfo10000, "_sdRun: done waiting\n");
    logMsg(LogSourceDepot, LogInfo10000, "BinaryOutput.Count = ", results.BinaryOutput.Count, "\n");
    logMsg(LogSourceDepot, LogInfo10000, "StructuredOutput.Count = ", results.StructuredOutput.Count, "\n");

            // See if there are errors
    if (results.ErrorOutput.Count > 0) {
        var msg = "";
        for(var i = 0; i < results.ErrorOutput.Count; i++)
            msg += "    " + results.ErrorOutput(i).Message + "\n";
        /* { */ logMsg(LogSourceDepot, LogInfo1000, "} _sdRun()\n");
        if(logGetFacilityLevel(LogSourceDepot) > 0)
            throwWithStackTrace(Error(1, "SD Error on command '" + cmd + "':\n" +  msg));
    }

    for(var i = 0; i < results.WarningOutput.Count; i++) 
        logMsg(LogSourceDepot, LogInfo, "SD Warning: ", results.WarningOutput(i).Message, "\n");

    if (displayInfo || logGetFacilityLevel(LogSourceDepot) >= LogInfo1000)
        for(var i = 0; i < results.InfoOutput.Count; i++)
            logMsg(LogSourceDepot, LogInfo, "SD Info: ", results.InfoOutput(i).Message, "\n");

    logMsg(LogSourceDepot, LogInfo1000, "} _sdRun()\n");
    return results;
}

/*****************************************************************************/
/* Call this after _sdRun to return all information messages as one big string */

function _sdResultsGetInfoMsgs(results) {

    var msgs = "";
    for (var i = 0; i < results.InfoOutput.Count; i++) {
        var msg = results.InfoOutput(i).Message;
        msgs += msg + "\r\n";
        logMsg(LogSourceDepot, LogInfo1000, "Msg SDCmd = ", msg, "\n");
    }
    return msgs;
}

/*****************************************************************************/
/* depot names are too long for display purposes.  Instead we just want to
   show the 'relevant' parts.  This is defined as those parts of the path
   that are not in 'baseName'.  For efficiency, this operation is broken
   into two parts, _sdShortNameNew(baseName), which parses the base, and
   and forms a cache, and _sdShorNameForPath, which take a name and a cache
   and returns the shortened name 
*/

function _sdShortNameNew(baseName) {

    var cache = {}
    cache.names = {};
    cache.elems = setNew(baseName.toLowerCase().split(/\/+/));
    return cache;
}

function _sdShortNameForPath(name, cache) {

    var ret = cache.names[name];
    var normName;
    if (!ret) {
        normName = name.toLowerCase();
        ret = cache.names[normName];
    }
    if (!ret) {
        var elems = normName.split(/\/+/);
        if (elems.length <= 1)
            ret = normName;
        else {
            ret = "*/";
            var wild=true;
            for(var i = 0; i < elems.length-1; i++) {
                var elem = elems[i];
                if (!cache.elems[elem]) {
                    ret += elem + "/";
                    wild = false;
                }
                else if (!wild) {
                    wild = true;
                    ret += "*/"
                }
            }
            ret += elems[elems.length-1];
            cache.names[name] = ret;
            cache.names[normName] = ret;
            // logMsg(LogSourceDepot, LogInfo, "_sdShortName: ", name, " base ", baseName, "\n");
            // logMsg(LogSourceDepot, LogInfo, "_sdShortName: {\n", dump(cache), "}\nret = ", ret, "\n");
        }
    }
    return ret;
}

/*****************************************************************************/
/* sdExplain gives a line by line report of the changes to a file.  For each
   line in 'fileName'. give the change number user, and date associated with
   the last change to that line.   And HTML report file is generated and IE
   is launched.   

   By default every line in 'fileName' is reported.  This can be very expensive
   for files that have a large number of changes.   Often you are only 
   interested in a particular line, or range, and thus you are encouraged to
   specify a 'targetLine' and 'targetCount' to specify this range.  This can
   speed things up dramatically.  

   revision 'rev' (default the curret sync point)

   rev can be any legal SD file revision specification (@-3, @34342, #head, 
   #34, ...)
 */

function sdExplain(fileName, targetLine, targetCount, rev, launchIe) {

    //logMsg(LogSourceDepot, LogInfo, "AGAIN sdExplain(", fileName, ", ", targetLine, ", ", targetCount, ", ", rev, ")\n");    
    if (targetCount == undefined) {
        targetCount = 1;
        if (targetLine == undefined)
            targetCount = 0x7FFFFFFF;
    }
    if (targetLine == undefined)
        targetLine = 1;
    if (rev == undefined)
        rev = "#have";

    if (targetLine == 1 && targetCount == 0x7FFFFFFF) {
        logMsg(LogSourceDepot, LogWarn, "You have not specified a range.  The whole file is assumed.\n");
        logMsg(LogSourceDepot, LogWarn, "This can take a while, and use significant Depot resources,\n");
        logMsg(LogSourceDepot, LogWarn, "especially if the file has a long history with many changes.\n");
        logMsg(LogSourceDepot, LogWarn, "Please consider using a range.\n");
        logMsg(LogSourceDepot, LogWarn, "    runjs sdExplain <fileName> <startLine> <lineCount>\n");
    }
    
    if (launchIe == undefined)
        launchIe = true;
        
    logMsg(LogSourceDepot, LogWarn, "launchIe = ", launchIe, ".\n");

        // TODO : allow limit back in time

    var sdFileLog = _sdFileLogNew();
    var groups = _sdFileLogGetLines(fileName, rev, targetLine, targetCount, sdFileLog);
    var lineNum = targetLine;

    var validateLines;
    var validationFailed = 0;
    validateLines = runCmd("sd print -q " + fileName + rev).output.split("\r\n");

    var runPath = Env('_NTBINDIR') + "\\tools\\x86;" + Env('_NTBINDIR') + "\\ndp\\clr\\bin";
    var htmlFile = FSOGetTempPath() + ".html";
    var out = FSOOpenTextFile(htmlFile, FSOForWriting, true);
    out.WriteLine("<HTML>");
    out.WriteLine("<SCRIPT LANGUAGE='javascript' DEFER='true'>");
    out.WriteLine("    var WshShell = undefined;");
    out.WriteLine("    function run(cmd) {");
    out.WriteLine("        if (WshShell == undefined) {");
    out.WriteLine("            WshShell = new ActiveXObject('WScript.Shell');");
    out.WriteLine("            var Env = WshShell.Environment('PROCESS');");
    out.WriteLine("            Env('SDPORT') = 'DDRTSD:4000';");
    out.WriteLine("            Env('PATH') = Env('PATH') + ';" + runPath.replace(/\\/g, "\\\\") + "';");
    out.WriteLine("        }");
    out.WriteLine("        var ret = WshShell.Run(cmd, 2, true);");
    out.WriteLine("        if (ret != 0) {");
    out.WriteLine("            WScript.Echo('Command failed')");
    out.WriteLine("            WScript.Sleep(3000);");
    out.WriteLine("        }");
    out.WriteLine("    }");

    out.WriteLine("</SCRIPT>");
    out.WriteLine("<BODY>");

    var baseName= _sdFileLogGet(fileName, 1, sdFileLog).depotFile;
    var baseNameHnd = _sdShortNameNew(baseName);
    out.WriteLine("<H3> Line by line change report for file " + 
        "<A HREF='javascript:run(\"sdv " + baseName + "\")'>" + baseName + rev + "</A></H3>");
    out.WriteLine("Each line in the table below is annotated by the change that last modified that line.");
    out.WriteLine("The date, user, and the name in the depot where the change was first introducted");
    out.WriteLine("is given in these annotations.   Because names in the depot can be quite long");
    out.WriteLine("any parts of the path that are common to the path specified by the user are ");
    out.WriteLine("converted to '*'.  ");
    out.WriteLine("<P>");
    out.WriteLine("The source depot number is a hyperlink that runs the 'sdv' command to display all");
    out.WriteLine("the files changed by that checkin.  The hyperlink assocated with the 'from' file");
    out.WriteLine("will bring up 'sdv' information so you can diff just that file quickly.");
    out.WriteLine("<P>");
    out.WriteLine("The lines in the display alternate colors to highlight which groups of lines were.");
    out.WriteLine("modified at the same time.  For each such block of lines the 'prev' field tries. ");
    out.WriteLine("provide information on the previous version of the block.  Obviously if the lines");
    out.WriteLine("added, this is impossible, however if the lines were modified, the prev field ");
    out.WriteLine("indicates how many lines were modified to form the current block of lines. ");
    out.WriteLine("The link associated with this field does a recurisive 'runjs sdExplain' on this");
    out.WriteLine("block of lines, allowing you to 'tunnel' into previous versions of the block.");
    out.WriteLine("<P>");
    out.WriteLine("All the hyperlinks run code on your local machine as as such are blocked by default. ");
    out.WriteLine("You need to enable the links by clicking on the IE banner near the top of the");
    out.WriteLine("page and selecting 'allow blocked content'.");
    out.WriteLine("<HR>");
    out.WriteLine("<TABLE BORDER=0 CELLPADDING=0>");
    out.WriteLine("<TR> <TH>Line</TH><TH>Change</TH><TH>Date</TH><TH>User</TH><TH>From file#rev:line</TH><TH>Prev</TH><TH>Line Data</TH></TR>");
    var nameCache = {};
    var lastColor = "";
    for(var i = 0; i < groups.length; i++) {
        var group = groups[i];

        var changeInfo = sdFileLog.changeCache[group.fileInfo.change];
        var user = changeInfo.user;
        if (user.match(/\\(.+?)$/))
            user = RegExp.$1;

        var shortName = _sdShortNameForPath(group.fileInfo.depotFile, baseNameHnd);
        var date = prettyDate(new Date(changeInfo.time));

        for(var j = 0; j < group.count; j++) {
            out.WriteLine("<TR>");
            out.WriteLine("<TD ALIGN=CENTER>" + lineNum + "</TD>");
            out.WriteLine("<TD ALIGN=CENTER>" + 
                "<A HREF='javascript:run(\"sdv " + group.fileInfo.change + "\")'>" + group.fileInfo.change + "</A></TD>");
            out.WriteLine("<TD ALIGN=CENTER>" + date +  "</TD>");
            out.WriteLine("<TD ALIGN=CENTER>" + user + "</TD>");

            out.WriteLine("<TD ALIGN=CENTER>" + 
                "<A HREF='javascript:run(\"sdv " + group.fileInfo.depotFile + "#" + group.fileInfo.rev + "\")'>" +
                    shortName + "#" + group.fileInfo.rev + "</A>:" + (group.srcLineNum + j) + "</TD>");

            var prev = "-";
            if (j == 0 && group.prev) {
                var filePath = group.prev.fileName;
                if (filePath.match(/^\s*\//))
                    filePath = "\\\" " + filePath + "\\\""
                else if (!filePath.match(/(\/)|(\\)|(\w:)/))
                    filePath = FSOGetFile(filePath).Path.replace(/\\/g, "\\\\");

                prev = "<A HREF='javascript:run(\"runjs sdExplain " + 
                            filePath + " " +
                            group.prev.srcLineNum + " " + 
                            group.prev.changeSize + " " + 
                            "#" + group.prev.rev + "\")'>" + group.prev.changeSize + "</A>";
            }
            out.WriteLine("<TD ALIGN=CENTER>" + prev + "</TD>");

            var line = detab(escapeHTML(group.lines[j]));
                // NOTE that if you end the <PRE> with </PRE> like you are supposed to, if formats poorly
                // (too much whitespace).  This really is using a bug, which is not great. :(
            out.WriteLine("<TD ALIGN=LEFT " + lastColor + "><PRE>| " + line + "<PRE></TD>");

            if (validationFailed < 3 && group.lines) {
                if (validateLines[lineNum-1] != group.lines[j]) {
                    var normValidate = validateLines[lineNum-1].replace(/(\s+)/g, "");
                    var normGroup    = group.lines[j].replace(/(\s+)/g, "");
                    if (normValidate != normGroup) {
                        WScript.StdErr.WriteLine("ERROR: Validation failed on " + fileName + ":" + lineNum);
                        WScript.StdErr.WriteLine("    Line    '" + group.lines[j] + "'");
                        WScript.StdErr.WriteLine("    Line != '" + vaydateLines[lineNum-1] + "'");
                    validationFailed++;
                }
            }
            }
            lineNum++;
            out.WriteLine("</TR>");
        }

        if (lastColor == "")                // Alternate colors to highlight change groups
            lastColor = "BGCOLOR=FFFF66";
        else 
            lastColor = "";

    }

    out.WriteLine("</TABLE>");
    out.WriteLine("</HTML></BODY>");
    out.Close();
    WScript.StdOut.WriteLine("");
    logMsg(LogSourceDepot, LogInfo, "Validation failures = ", validationFailed, "\n");

    if (launchIe == true)
    {
        logMsg(LogSourceDepot, LogInfo, "Output in ", htmlFile, ".  Launching IE on this file.\n");     
        launchIECmd(htmlFile)
        FSODeleteFile(htmlFile, true);
    }
    else
    {
        logMsg(LogSourceDepot, LogInfo, "Output in ", htmlFile, "\n");     
    }    
}

/*****************************************************************************/
/* For a given file name and file revision number (not sd change number), find 
   the changes associated region of the file starting at 'targetLine' which
   is 'targetLen' long. (the first line is 'targetLine == 1) 'sdAttrib' is a 
   object is created with _sdFileLogNew() and is a cache to avoid making SD 
   queries more than is necessary.  The function returns an array of 
   structures of the form
 
    returns a object of the form
    {     fileInfo: { depotFile: <string>        // name in the depot
                    rev:<int>                // file revision number (SD # numbers) local to this file
                    change:<int>            // SD global change number
                  }
        prev: { depotFile: <string>            // Prev is optional it is there only when this file has a predesessor
                rev: <int>                    // The maps the delta to line number before the delta, so you can
                srcLineNum: <int>            // see what souce lines looked like before the last change
                changeSize: <int>        
              }
        srcLineNum:                            // line number in the FILE THAT CREATED THE DELTA (not the target file)
        count: <int>                        // number of lines that are are with this change 
                                            // number (limited to targetLen)
        lines: [<string>]                     // the lines themselves 
    }

   The change number can be looked up in sdAttrib to get additional informat
   on the change. 
*/

function _sdFileLogGetLines(fileName, rev, targetLine, targetLen, sdAttrib) {

    //timeFtn("_sdFileLogGetLine {", fileName, rev, targetLine, targetLen);
//FORPERF    logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLines(", fileName, ", ", rev, ", ", targetLine, ", ", targetLen, ", sdAttrib) {\n");

        // This fills in the action an change field for us
    var fileChange = _sdFileLogGet(fileName, rev, sdAttrib);  // an sdAttrib is a sdFileLog
    if (fileChange == undefined)
        throwWithStackTrace(Error(1, "Could not find file '" + fileName + "#" + rev + "'"));
//FORPERF    logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: found change ", fileChange.change, " action ", fileChange.action, "\n");

        // if we have 'from' information, this indicates integration, which
        // means that the integrated file (there may be two) is the 'previous' $
    var ret = undefined;
    if (fileChange.from != undefined) {
         if (fileChange.from.how == "copy from" || fileChange.from.how == "branch from")  {
                // I have encounted cases where there is a delta even when it is marked copy from or branch from.
                // to to be safe, we assume their may be a delta associated with it.  It would be nice to
                // avoid having to call 'sd diff2' however. 
//FORPERF            logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: found branch from\n");
            fileChange.deltas = _sdDiff2(fileChange.from.file + "#" + fileChange.from.erev, fileName + "#" + fileChange.rev, sdAttrib.sdObj);
            ret = _sdFileLogGetLinesFromDeltas(fileChange.deltas, fileChange.from.file, fileChange.from.erev, targetLine, targetLen, fileChange, sdAttrib);
        }
        else {
//FORPERF            logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: found merged\n");
            if (fileChange.from.how != "merge from" && fileChange.from.how != "copy from")
                logMsg(LogSourceDepot, LogWarn, "Unexpected how ", fileChange.from.how, " for ", fileName, " at revision ", rev, ". Treating as an 'merge from'\n");

                // In the case of merge you have two predecessors, the previous revision and the 
                // 'from' revision.   Get both the second set of deltas.  We will merge these in below
            if (!fileChange.mergedDeltas)
                fileChange.mergedDeltas = _sdDiff2(fileChange.from.file + "#" + fileChange.from.erev, fileName + "#" + fileChange.rev, sdAttrib.sdObj);
        }
    }
    else if (fileChange.rev == 1) {
//FORPERF        logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: found revision 1 (assumed add)\n");

            // this is an 'add', there is no previous delta, so we cache the data, and just look up the lines needed
        if (fileChange.action != "add")
            logMsg(LogSourceDepot, LogWarn, "Unexpected action ", fileChange.action, " for ", fileName, " at revision 1. Treating as an 'add'\n");

        if (!fileChange.data)
            fileChange.data = runCmd("sd print -q " + fileName + "#" + fileChange.rev).output.split("\r\n");

        var retElem = { fileInfo: fileChange, srcLineNum: targetLine, count: targetLen };
        var start = targetLine - 1;
        var toEnd = fileChange.data.length - start;
        if (retElem.count > toEnd)
            retElem.count = toEnd;
        retElem.lines = fileChange.data.slice(start, start+retElem.count);
        logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: Got base  Pushed change {\n", dump(retElem, 2), "\n}\n");
        ret = [retElem];
    }
    else {
//FORPERF        logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: found edit\n");
        if (!fileChange.action.match(/^edit|integrate/))        // integrates without any action are 'ignored' integrations
            logMsg(LogSourceDepot, LogWarn, "Unexpected action ", fileChange.action, " for ", fileName, " at revision ", rev, ". Treating as an 'edit'\n");
    }

    if (!ret) {
            // We get here in the case of a edit or a merge.  In both cases, the one of the previous files
            // is the previous revision of the same name.   Fetch the deltas between the current rev
            // and this previous one (if we have not done so already) and then fetch the line information
        if (!fileChange.deltas)
            fileChange.deltas = _sdDiff2(fileName + "#" + (fileChange.rev-1), fileName + "#" + fileChange.rev, sdAttrib.sdObj);

        ret = _sdFileLogGetLinesFromDeltas(fileChange.deltas, fileName, fileChange.rev-1, targetLine, targetLen, fileChange, sdAttrib);

        if (fileChange.mergedDeltas) {
//FORPERF            logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: merging second set of deltas\n");

                // we get here if we have two predecessors. the 'ret' value above attributes any change merged from
                // the second predecessor to the current change.  Thus for all regions in 'ret' that are attributed
                // to the current change we should check if we have 'better' information that attributes the
                // the lines to the other predecessor.
            var regions = ret;
            ret = [];
            var curLine = targetLine;
            for (var i = 0; i < regions.length; i++) {
                var region = regions[i];
                if (region.change == fileChange.change) {
                    // logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: found region at ", curLine, " {\n", dump(region, 1), "\n}\n");
                    var fromMerge =_sdFileLogGetLinesFromDeltas(fileChange.mergedDeltas, fileChange.from.file, fileChange.from.erev, curLine, region.count, fileChange, sdAttrib);
                    // logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLine: replacing with region {\n", dump(fromMerge, 2), "\n}\n");
                    ret = ret.concat(fromMerge);
                }
                else 
                    ret.push(region);
                curLine += region.count;
            }
        }
    }

//FORPERF     logMsg(LogSourceDepot, LogInfo100, "} _sdFileLogGetLines() ret.length = ", ret.length, "\n");
    //timeFtn("}_sdFileLogGetLine");
    return ret;
}

/*****************************************************************************/
/* given a delta spec (see _sdDiff2 for format) deltas, the name and revision of 
   the 'prev' file, and the targetLine you are interested in, return a description 
   of who updated targetLine.  'fileChange' is the file revision information for
   the change associated with 'deltas' (see sdFileLogGet for details) .  
   See _sdFileLogGetLines for details on the ret value.  This is really a helper
   function for  _sdFileLogGetLines and is only called from there.
*/

function _sdFileLogGetLinesFromDeltas(deltas, prevFileName, prevRev, targetLine, targetLen, fileChange, sdAttrib) {

    //timeFtn("_sdFileLogGetLinesFromDeltas", fileChange.depotFile, fileChange, rev);
//FORPERF    logMsg(LogSourceDepot, LogInfo100, "_sdFileLogGetLineFromDeltas(deltas, ", prevFileName, ", ", prevRev, ", ", targetLine, ", ", targetLen, ", ", fileChange.change, ", sdAttrib) {\n");
    
    var lineNum = 1;        // holds 'result' line number after applying all deltas up to 'deltaIdx'
    var lineNumPrev = 1;    // holds the cooresponding line number for 'lineNum' in prevFileName.
    var deltaIdx = 0;        // holds index into the list of deltas;
    var prevDeleteSize = 0;

        // we remember where we left off the last time, start from there if we can
    if (deltas.lineNum <= targetLine) {
        lineNum = deltas.lineNum;
        lineNumPrev = deltas.lineNumPrev;
        deltaIdx = deltas.deltaIdx;
    }

    var ret = [];
    while (targetLen > 0) {
//FORPERF        logMsg(LogSourceDepot, LogInfo100, "looping: target ", targetLine, " len ", targetLen, " line ", lineNum, " prevLine ", lineNumPrev, "\n");

        if (deltaIdx >= deltas.deltas.length) {                     // Is the region after the last delta?  Get from previous file.
//FORPERF            logMsg(LogSourceDepot, LogInfo100, "target above last delta\n");
            ret = ret.concat(_sdFileLogGetLines(prevFileName, prevRev, (targetLine - lineNum + lineNumPrev), targetLen, sdAttrib));
            break;
        }
        var delta = deltas.deltas[deltaIdx];
//FORPERF        logMsg(LogSourceDepot, LogInfo100, "processing delta ", delta.oper, " ", delta.start, " ", delta.count, "\n");

            // Is the region of interest before the next delta?  If so get the information from the previous file. 
        var deltaStart = delta.start;
        if (delta.oper == "a")        // appends happen AFTER the indicate line, but it is more natural if they are ON the indicated line
            deltaStart++;
        var deltaStartPost = (deltaStart - lineNumPrev + lineNum);    // where the delta starts in file after the delta has been applied
        var relLineNum = targetLine - deltaStartPost;
        
//FORPERF        logMsg(LogSourceDepot, LogInfo100, "deltaStartPost ", deltaStartPost, " relLineNum ", relLineNum,"\n");
        if (relLineNum < 0) {
//FORPERF            logMsg(LogSourceDepot, LogInfo100, "target range before next delta\n");
            var toNextDelta = - relLineNum;
            var subTargetLen = targetLen;
            if (subTargetLen > toNextDelta)
                subTargetLen = toNextDelta;
            ret = ret.concat(_sdFileLogGetLines(prevFileName, prevRev, (targetLine - lineNum + lineNumPrev), subTargetLen, sdAttrib));

            targetLine += subTargetLen;
            targetLen -= subTargetLen;
            prevDelete = 0;
        }
        else {        // process the delta

            if (delta.oper == "d") {            // It is a delete
                lineNum = deltaStartPost 
                lineNumPrev = deltaStart + delta.count;
                deltaIdx++;
                prevDeleteSize = delta.count;
            }
            else {                             // It is an add
                var targetOverlap = delta.count - relLineNum;
//FORPERF                logMsg(LogSourceDepot, LogInfo100, "targetOverlap ", targetOverlap, " targetLen ", targetLen, "\n");
                var consumedDelta = true;
                if (targetOverlap > 0) {            // The target overlaps with this add, return what we can
                    if (targetOverlap > targetLen) {    // only return what is asked for
                        targetOverlap = targetLen;
                        consumedDelta = false;
                    }

                    var retElem = { fileInfo: fileChange,
                                    srcLineNum: targetLine, 
                                    count: targetOverlap, 
                                    lines: delta.lines.slice(relLineNum, relLineNum + targetOverlap) };
//FORPERF                    logMsg(LogSourceDepot, LogInfo100, "Pushed change ", retElem.change, " for ", retElem.count, " lines\n}\n");

                        // If this is a change, log info on where what the previous block looked like
                    if (prevDeleteSize) {
                        retElem.prev = { fileName: prevFileName,
                                         rev: prevRev,
                                         srcLineNum: lineNumPrev - prevDeleteSize,
                                         changeSize: prevDeleteSize
                                       };
                    }
                    ret.push(retElem);
                    targetLine += targetOverlap;
                    targetLen -= targetOverlap;
                }
                if (consumedDelta) {                    // don't update the delta unless we have consumed it all.  
//FORPERF                    logMsg(LogSourceDepot, LogInfo100, "consumed whole add delta\n");
                    lineNum = deltaStartPost + delta.count;    
                    lineNumPrev = deltaStart;
                    deltaIdx++;
                }
                prevDeleteSize = 0;
            }
        }
    }

        // remember where we were for next time.
    deltas.lineNum = lineNum;
    deltas.lineNumPrev = lineNumPrev;
    deltas.deltaIdx = deltaIdx;

//FORPERF     logMsg(LogSourceDepot, LogInfo100, "} _sdFileLogGetLineFromDeltas() ret.length = ", ret.length, "\n");
    return ret;
}

/*****************************************************************************/
/* sd is pretty inefficient if you don't do your queries in bulk.  Thus we
   create a sdFileLog obj, which basiclly wraps the 'sdObj' with a cache and
   we make bulk queries to sd and remembers the results */

function _sdFileLogNew(sdObj) {

    sdObj = sdConnect(sdObj);

    return { sdObj:sdObj, fileLogCache: {}, changeCache: {} };
}

/*****************************************************************************/
/* returns filelog information for a given fileName and revsion.  sdFileLog
   is created with _sdFileLogNew. 'rev' is either a file revision number, any
   source depot revision spec (#head, @-1, @32323, ...).   Returns a fileLog 
   structure of the form

    { depotFile: <string>    // Name in the depot
      rev:<int>                // file revision number (SD # numbers) local to this file
      change:<int>            // SD global change number
      action:<string>        // edit, integrate, branch ... 
      from: {                // present if action == integrate or action == branch
          how:<string>        // branch from 'copy from', 'merge from' ignored, 
          file:<string>        // fail we merged, copied, branched or ignored
          srev:<int>        // start revision of what we integrated
          erev:<int>        // end revision of what we integrated
      }
    }
*/
function _sdFileLogGet(fileName, rev, sdFileLog) {

    //timeFtn("_sdFileLogGet{", fileName, rev);
    // logMsg(LogSourceDepot, LogInfo, "_sdFileLogGet(", fileName, ", ", rev, ") {\n");
    var ret = undefined;
    
    var normFileName = fileName
    var fileCache = sdFileLog.fileLogCache[fileName]
    if (!fileCache) {
        normFileName = fileName.toLowerCase();
        fileCache = sdFileLog.fileLogCache[fileName];
    }
    if (fileCache) 
        ret = fileCache[rev];

    if (ret == undefined) {
        var fileWithRev = fileName;
        if (typeof(rev) != "number")         // for numeric revisions, do everything, else do up to that
            fileWithRev += rev;
        var results = _sdRun(sdFileLog.sdObj, "filelog " + fileWithRev);    // fetch all history about this file
            // TODO use sdFileLog for this part
        for(var i = 0; i < results.StructuredOutput.Count; i++)  {
            var vars = results.StructuredOutput(i).Variables;

            var depotFile = ("" + vars.Variable("depotFile"));
            var normDepotFile = depotFile.toLowerCase();
            
            var revCache = sdFileLog.fileLogCache[normDepotFile];
            if (!revCache) 
                revCache = {};
            sdFileLog.fileLogCache[normDepotFile] = revCache;
            sdFileLog.fileLogCache[depotFile] = revCache;
            sdFileLog.fileLogCache[fileName] = revCache;
            sdFileLog.fileLogCache[normFileName] = revCache;
            
            for(var j = 0; ; j++) {
                var fileRevRecNum = vars.VariableX("rev", j);
                if (fileRevRecNum == undefined)
                    break;

                if (revCache[fileRevRecNum])
                    break;

                fileRevRec = {};
                fileRevRec.depotFile = normDepotFile;
                fileRevRec.rev = parseInt(fileRevRecNum);
                fileRevRec.change = parseInt(vars.VariableX("change", j));
                fileRevRec.action = "" + vars.VariableX("action", j);

                if (!sdFileLog.changeCache[fileRevRec.change]) {
                    var change = {};
                    change.time = parseInt(vars.VariableX("time", j)) * 1000;    //  msec from 1970 (can be passed to new Date())
                    change.user = "" + vars.VariableX("user", j);
                    change.desc = "" + vars.VariableX("desc", j);
                    sdFileLog.changeCache[fileRevRec.change] = change;
                }
                for(k = 0; ; k++) {
                    var how = vars.VariableXY("how", j, k);
                    if (how == undefined)
                        break;
                    how = "" + how;
                    if (how.match(/^(.* from)|(ignored)^/)) {
                        var from = {}
                        from.how = how;
                        from.file = "" + vars.VariableXY("file", j, k);
                        from.srev = parseInt(vars.VariableXY("srev", j, k));
                        from.erev = parseInt(vars.VariableXY("erev", j, k));
                        fileRevRec.from = from;
                    }
                }
                revCache[fileRevRecNum] = fileRevRec;
                if (ret == undefined)
                    if (fileRevRecNum == rev || typeof(rev) != "number")
                        ret = fileRevRec;
            }
        }
    }

    //logMsg(LogSourceDepot, LogInfo100, "} _sdFileLogGet() = {\n", dump(ret, 1), "\n}\n");
    //logMsg(LogSourceDepot, LogInfo100, "} _sdFileLogGet()\n");
    //timeFtn("}_sdFileLogGet");
    return ret;
}

/*****************************************************************************/
/* given SD file specifications (eg //depot/devdiv/.../foo.cpp#2), as well
   as the handle to the depot sdObj (optional), return the difference spec
   This is a structures that looks like

    {   deltas : [
          { oper    : <string>         // (either "d" (delete), or "a" (add))
            start   : <int>            // line number for operation (first line is 1)
            count   : <int>           //
            lines   : [string]        // lines to add for (only present for add operation
                                    // The lines do NOT have terminating \n
          }]
    }
   Line numbers are always with respect to fileSpec1 and are in assending order.  
 */

function _sdDiff2(fileSpecBase, fileSpecNew, sdObj) {

    sdObj = sdConnect(sdObj);

//FORPERF    logMsg(LogSourceDepot, LogInfo100, "sdDiff2(", fileSpecBase, ", ", fileSpecNew, ", sdObj) {\n");
    var results = _sdRun(sdObj, "diff2 -dbn " + fileSpecBase + " " + fileSpecNew, true);

    for (var i = 0; i < results.InfoOutput.Count; i++) 
        logMsg(LogSourceDepot, LogInfo, "Out = '", results.InfoOutput(i).Message, "'\n");
    for (var i = 0; i < results.TextOutput.Count; i++) 
        logMsg(LogSourceDepot, LogInfo, "Out = '", results.TextOutput.Item(i).Message, "'\n");

    
    if (results.StructuredOutput.Count != 1)
        throwWithStackTrace(Error(1, "Unexpected diff2 count " + results.StructuredOutput.Count + " for sd diff2 -dbn " + fileSpecBase + " " + fileSpecNew));

    var vars = results.StructuredOutput(0).Variables;
    var diff = "" + vars.Variable("diff");

        // parse the string into a set of records.
    var ret = [];
    var diffLines =  diff.split("\n");

    for(var i =0; i < diffLines.length; i++) {
        if (!diffLines[i].match(/(d|a)(\d+)\s+(\d+)/)) {
            if (diffLines[i] == "")        // allow blank lines
                continue;
            throwWithStackTrace(Error(1, "Unexpected diff line '" + diffLines[i] + "' for sd diff2 -dbn " + fileSpecBase + " " + fileSpecNew));
        }

        var newDiffRec = { oper: RegExp.$1, start: parseInt(RegExp.$2), count: parseInt(RegExp.$3) };
        if (RegExp.$1 == "a") {
            newDiffRec.lines = diffLines.slice(i+1, i+1+newDiffRec.count);

            i += newDiffRec.count;
            if (i >= diffLines.length)
                throwWithStackTrace(Error(1, "Expected " + newDiffRec.count + " lines, we are short " + (i - diffLines.length) + " at sd diff2 -dbn " + fileSpecBase + " " + fileSpecNew));
        }
        ret.push(newDiffRec);
    }

//FORPERF    logMsg(LogSourceDepot, LogInfo100, "} sdDiff2()\n");
    return { deltas: ret };
}

/*****************************************************************************/
/*  Attempt to guess at where the source depot root is by checking for an expected
    location somewhere in the path of the current location.
*/
function sdGetRoot(dir, sdObj)
{
    var isUNC = false;
    var tempdir = dir.replace(/\"/, "");
    // preserve UNC path
    if(tempdir.match(/^\\\\/))
        isUNC = true;
    var dirs = tempdir.split('\\');
    var currentDir;
    currentDir = dirs[0];
    if (isUNC)
        currentDir = "\\\\" + currentDir;

    if (sdObj == undefined)
        sdObj = sdConnect(dir);

    try
    {
        // We don't want to see the warnings that will be generated in our attempt to find a valid source
        // depot directory, so we change the log facility level.
        var facilityLevel = logGetFacilityLevel(LogSourceDepot);
        logSetFacilityLevel(LogSourceDepot, 0)
        for(var i = 1; i < dirs.length; i++)
        {
            currentDir += "\\" + dirs[i];
            if(sdWhere("\"" + currentDir + "\\ndp\\clr\\src\\*\"", sdObj) != undefined)
            {
                return currentDir;
            }
        }
    }
    catch(e)
    {
    }
    finally
    {
        logSetFacilityLevel(LogSourceDepot, facilityLevel);
    }
    return null;
}

/*****************************************************************************/
/* Given 'fileOrChangeSpec', display a list of the changes with full title 
*/
function sdShowChanges(fileSpec)
{
    var changes = sdChanges(fileSpec);
    var sdObj = sdConnect();
  
    WScript.StdOut.WriteLine("\n\n Change  " + padRight("Date", 17) + padRight("User", 10) + "Desc");
    for (var i = 0; i < changes.length; i++)
    {
        var changeInfo = changes[i];
        var user = changeInfo.user;
        if (user.match(/\\(.+?)$/))
            user = RegExp.$1;
        var time = prettyTimeOnly(new Date(changeInfo.time));
        var date = prettyDate(new Date(changeInfo.time));
        var changeDescription = sdDescribe(changeInfo.change, sdObj);
        var desc = changeDescription.desc;
        if (desc.match(/^(.*)$/m))
            desc = RegExp.$1;
        WScript.StdOut.WriteLine(padLeft(changeInfo.change,7) + padSpace(2) + padRight(date + " " + time, 17) + padRight(user, 10) + desc);
    }       
    WScript.StdOut.WriteLine("\nTotal changes: " + changes.length + "\n\n");
}