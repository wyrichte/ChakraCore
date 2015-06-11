/*********************************************************************************/
/*                                 util.js                                        */
/*********************************************************************************/

/* small utilities that don't have a theme */

// AUTHOR: Vance Morrison 
// DATE: 1/1/2003
// DEPENDANCIES 
//         fso.js            // for file system access
//         log.js            // for logging

/*********************************************************************************/

var utilModuleDefined = 1;             // Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");

var LogUtil = logNewFacility("util");

if (ScriptDir == undefined)
    var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

    // This is just a counter that everyone can use to create
    // process global ids.  You post increment in use
var objIds = 0;

/****************************************************************************/
/* return a string of 'len' spaces */

function padSpace(len) {

    padStr = "                    ";
    if (len > padStr.length)
        return padStr + pad(padStr.length)
    return padStr.substr(0,len);
}

/*************************************************************************/
function padRight(str, num, withZero) {

    str = str.toString();
    num -= str.length;
    if (num < 0)
        return str;

    var fill = withZero ? "0000000000" : "          ";
    var pad = "";
    while (num >= 10) {
        pad += fill;
        num -= 10;
    }
    return str + pad + fill.substr(0, num) ;
}

/*************************************************************************/
function padLeft(str, num, withZero) {

    str = str.toString();
    num -= str.length;
    if (num < 0)
        return str;

    var fill = withZero ? "0000000000" : "          ";
    var pad = "";
    while (num >= 10) {
        pad += fill;
        num -= 10;
    }
    return pad + fill.substr(0, num) + str;
}

/*************************************************************************/
/* fetch the current time as XX:XX:XX string */
function curTime(time) {
    if (time == undefined)
        time = new Date()
    var timeStr = time.toString();
    if (timeStr.match(/ (\d+:\d+:\d+) /))
        timeStr = RegExp.$1;
    return timeStr;
}

/*************************************************************************/
/* represent 'num' as a C style (0x) hex number */

function hex32(num) {
    if (num < 0) 
        num += 0x100000000;
    return "0x" + num.toString(16);
}

/*************************************************************************/
/* copy all the fields of 'from' to 'to'.  Return 'to' */

function memberWiseCopy(to, from) {

    if (from != undefined) {
        for (var key in from)            // shallow copy the options
            to[key] = from[key];
    }
    return to;
}

/*************************************************************************/
/* Try to open a file with exclusive access, and in case of failure, keep 
   retrying for 10 seconds. Can be used for simple locking protocol.

   Returns a handle to the lock file. You need to call Close() on the file
   to release the lock, preferably guarded by a try-finally.
 */

function openFileAsLock(name) {
    
    logMsg(LogUtil, LogInfo10000, "openFileAsLock(", name, ") {\n"); 
    
    var err;
    var ret = undefined;
    for (var i = 0;; i++) {
        try {
            ret = FSOOpenTextFile(name, FSOForWriting, true); /* overwrite = true */
            break;
        } catch(err) {}
        WScript.Sleep(100);
        if (i % 10 == 0)
            logMsg(LogUtil, LogWarn, "openFileAsLock: failed with error: ", err.description, "\n");

        if (i >= 1000)  {         // try for 10 seconds
            logMsg(LogUtil, LogWarn, "openFileAsLock(", name, ") failed error: ", err.description, "\n");
            throwWithStackTrace(new Error(-1, "openFileAsLock: Could not obtain lock file '" + name + "'  error: " + err.description));
        }
    }

    logMsg(LogUtil, LogInfo10000, "} openFileAsLock()\n"); 
    return ret;
}

/*********************************************************************************/
/* gets a unique file name in 'dir' that has 'prefix' and 'suffix' attached to it.
   It does this by using the time it is called, and probing to insure uniqueness. */

function getUniqueDateFileName(dir, prefix, suffix) {

    logMsg(LogUtil, LogInfo100, "In getUniqueFileName(",  dir,  ", ",  prefix,  ", ",  suffix,  ") {\n"); 

	var timeStr = FSOTimeAsFileName();
    var fileNamePrefix = dir + "\\" + prefix + timeStr + ".";
    var fileName;

    for(var i = 0;;) {
        fileName = fileNamePrefix + i.toString() + suffix;
        if (!FSOFileExists(fileName) && !FSOFolderExists(fileName))
            break;

        i++;
        if (i >= 10) {
            fileNamePrefix += "9";
            i = 0;
        }
    }

    logMsg(LogUtil, LogInfo100, "} getUniqueFileName() = ", fileName, "\n"); 
    return fileName;
}

/*****************************************************************************/
/* Performs a comparison of two DateFileName strings.  A DateFileName is generated
   by the function getUniqueDateFileName.  This function compares the date part
  of the file names. */
function sortDateFileNamesByDate(datefilename1, datefilename2)
{
    // Skip the folder part of filename
    var where = -1;
    var file1 = datefilename1;
    do {
        where = file1.indexOf("\\");
        if (where != -1) {
            file1 = file1.substring(where + 1);
        }
    } while (where != -1);
    
    var file2 = datefilename2;
    do {
        where = file2.indexOf("\\");
        if (where != -1) {
            file2 = file2.substring(where + 1);
        }
    } while (where != -1);
    
    var date1 = file1.match(/\d{2}-\d{2}-\d{2}_\d{2}\.\d{2}\./i);
    var date2 = file2.match(/\d{2}-\d{2}-\d{2}_\d{2}\.\d{2}\./i);

    if (date1 != null && date2 != null) {
        if(date1 == date2)
            return (file1 >= file2) ? 1 : -1;
        else if(date1 < date2)
            return -1;
        else
            return 1;
    }
    return (file1 >= file2) ? 1 : -1;
}


var _fileShares = undefined;
/**********************************************************************/
/* get the uncPath for 'dir' (which needs to be a full path name).
   if 'useHidden' is true, allows the use of hidden shares (begin with $)
   For eg. "c:\Temp\foo.txt" will become "\\mymachine\c$\Temp\foo.txt"
*/
function uncPath(dir, useHidden, updateCache) {

    // logMsg(LogUtil, LogInfo10, " uncPath ", dir, "\n");
    if (_fileShares == undefined || updateCache) {
        logMsg(LogUtil, LogInfo10, " Initing _fileShares\n");


		var wbemServices = GetObject("winmgmts://.");
		if (!wbemServices) 
			throw new Error(1, "Could not open the winmgmts object! (Someone corrupted XP system dlls, or WMI not installed (on Win9x)?)")

		var wbemObjectSet = wbemServices.InstancesOf("Win32_Share");
        _fileShares = {};
        var computerName = Env("COMPUTERNAME");
		for (var e = new Enumerator(wbemObjectSet); !e.atEnd(); e.moveNext()) {
			var shareObj = e.item();
			var shareName = shareObj.Name;
			var shareTarget = shareObj.Path.toLowerCase();

			logMsg(LogUtil, LogInfo1000, " Got share ", shareName, " = ", shareTarget, "\n");
			_fileShares[shareTarget] = "\\\\" + computerName  + "\\" + shareName;
		}
    }
    
    var maxTarget = 0;
    var ret = dir;
    for (var path in _fileShares) {
        if (dir.toLowerCase().indexOf(path) == 0) 
            if (useHidden || _fileShares[path].indexOf("$") < 0)
                if (maxTarget < path.length) {        // we choose the longest match as the best
                    var subpath = dir.substr(path.length);
                    // Drive shares end with a backslash, and so when
                    // we remove it to create a share-relative path
                    // we need to add the backslash back in
                    if (subpath.charAt(0) != '\\')
                        subpath = "\\" + subpath;
                    ret = _fileShares[path] + subpath;
                    maxTarget = path.length;
                }
    }

    logMsg(LogUtil, LogInfo1000, "uncPath = ", ret, "\n");
    return ret 
}

/**********************************************************************/
function relPath(dir, base) {

    logMsg(LogUtil, LogInfo10000, " relpath '", dir, "' base '", base, "'\n");
    
    // Does dir begin with base?
    if (dir.toLowerCase().indexOf(base.toLowerCase()) != 0)
    {
        // Sometimes we can't find a match because one of dir / base is UNC and
        // the other isn't.  Normalize both to UNC and try again.
        dir = uncPath(dir);
        base = uncPath(base);
        if (dir.toLowerCase().indexOf(base.toLowerCase()) != 0)
        {
            // Still no match; give up and just use the full dir
            return dir;
        }
    }

    // dir begins with base, so just return the portion of dir after base
    var ret = dir.substr(base.length+1);
    // logMsg(LogUtil, LogInfo, " returning ", ret, "\n");
    return ret;
}

/**********************************************************************/
function prettyTime(time) {

    if (time == undefined)
        time = new Date();
    return (time + "").match(/(\w+ *\d+ *\d+:\d+)/)[1];
}

/**********************************************************************/
function prettyTimeOnly(time) {

    if (time == undefined)
        time = new Date();
    return (time + "").match(/\w+ *\d+ *(\d+:\d+)/)[1];
}

/*************************************************************************/
/* returns the current date in MM/DD/YY format. */

function prettyDate(date) {
    if (date == undefined)
        date = new Date();
    return padLeft(date.getMonth()+1,2,true) + "/" + padLeft(date.getDate(),2,true) + "/" + padLeft(date.getFullYear() % 100,2,true);
}

/**********************************************************************/
/* returns a string representation of any object in a nice form,
   'indent' is normally not provided (it is for internal use) 
   similarly 'dumping' is also for internal use (it is the list of
   all the object currently being dumped, used to avoid infinite
   recursion loops.  'depth indicates the depth of subobjects
   to expand.  undefine means infinite, 0 means no [] in output. 
*/

function dump(data, depth, indent, dumping, noexpand) {

    var ret;
    if (indent == undefined) 
        indent = "";
    if (data == undefined) {
        ret = "undefined";
    }
    else if (typeof(data) == "object") {
        if (dumping == undefined)
            dumping = [];
        else {
            for(var i = 0; i < dumping.length; i++) {
                if (dumping[i] === data) {
                    ret = "PARENT[" + (dumping.length-i-1) + "]";
                    if (data.objID) 
                        ret += " = objID(" + data.objID + ")";
                    return ret;
                }
            }
        }

        ret = "object";
        if (data != null) {
            if (data.objID) 
                ret = "objID(" + data.objID + ")";
            else if (data.length > 0)
                ret = "array of length " + data.length;
        }

        if (!(depth <= 0)) {        // this is true for undefined depth
            if (depth)
                --depth;

            if (ret == "object")
                ret = "";
            else
                ret += " = ";
            
            dumping.push(data);
            if (data.length >= 0)
                ret += "array ";

            var fields = [];            // sort them for consistancy
            for (var idx in data) 
                if (idx != "objID") 
                    fields.push(idx);
            fields.sort(function(x, y) {
                if (x.match(/^\d+$/) && y.match(/^\d+$/))
                    return (parseInt(x) - parseInt(y));         // sort numerically if you can
                else if (x < y)
                    return -1;
                return (x > y);
            });

            ret += "[\n";
            indent += "    ";
            for (var i = 0; i < fields.length; i++) {
                var idx = fields[i];
                ret += indent + idx + " => " + dump(data[idx], (idx.match(/_$/) ? 0 : depth), indent, dumping) + "\n";
            }
            ret += indent + "]";
            dumping.pop();
        }
    }
    else {
        if (typeof(data) == "string") 
            ret = "\"" + data + "\"";
        else if (typeof(data) == "function") 
            ret = "function";
        else 
            ret = "" + data;

        ret = ret.replace(/\n/g, indent + "    ");
    }
    return ret;
}

/**********************************************************************/
/* returns a number as a number (not a string), that it finds
   in the string of the form <num><suffix>.  Useful when you 
   have a set of time names that you want to sort numerically */

function numSuffix(x, suffix) {
    
    x.match(new RegExp("(\\d+\\.?\\d*)" + suffix + "$", "i"));
    ret = RegExp.$1 - 0;
    // logMsg(LogUtil, LogInfo10, "_numSuffix x = ", x, " digits = ", RegExp.$1, " ret = ", ret, "\n");
    return ret;
}

/**********************************************************************/
/* package is designed to package up a scripting (WSF) file, so that 
   any included files are embeded in the file directly.  This way the
   script is 'packaged' so that it is no longer dependant on external
   JS files (of course other utilitys are a different matter 

     Parameters
        inputName : the file name of the WSF file.
        outputName: the file name of the 'packaged WSF file.
*/
function package(inputName, outputName) {

    if (inputName == undefined)
        throwWithStackTrace(new Error(1, "Required parameter 'inputName' not provided"));
    if (outputName == undefined)
        throwWithStackTrace(new Error(1, "Required parameter 'outputName' not provided"));

    logCall(LogUtil, LogInfo10, "package", arguments);
    var inDir = WshFSO.GetParentFolderName(inputName);
    if (inDir.length > 0)
        inDir += "\\";

    logMsg(LogUtil, LogInfo1000, "inDir = ", inDir, "\n");
    var inFile = FSOOpenTextFile(inputName, FSOForReading);
    try {
        var outFile = FSOOpenTextFile(outputName, FSOForWriting, true);

        while (!inFile.AtEndOfStream) {
            var line = inFile.ReadLine();
            logMsg(LogUtil, LogInfo100000, "Got line = '" + line + "'\n");

                // look for <script .* src=<filename> 
            if (line.match(/(^\s*<\s*script\b[^>]*\b)src\s*=([^>]*)>(.*)/i)) {

                outFile.Write(RegExp.$1);
                var restOfScriptTag = RegExp.$2;
                var restOfLine = RegExp.$3;
                logMsg(LogUtil, LogInfo1000, "got script = '" + restOfScriptTag + "' rest of line = '" + restOfLine + "' \n");
                
                if (restOfScriptTag.match(/^\s*"([^"]*)"(.*)/)) {
                    fileName = RegExp.$1;
                    restOfScriptTag = RegExp.$2;
                }
                else if (restOfScriptTag.match(/^\s*(\S+)(.*)/)) {
                    fileName = RegExp.$1;
                    restOfScriptTag = RegExp.$2;
                }
                logMsg(LogUtil, LogInfo1000, "got fileName = '" + fileName + "' rest of script Tag = '" + restOfScriptTag + "' \n");

                if (restOfScriptTag.length > 0)
                    outFile.Write(restOfScriptTag);
                outFile.WriteLine(">");
                
                var refFileName = inDir + fileName;
                logMsg(LogUtil, LogInfo1000, "Opening fileName = '" + refFileName + "' \n");
                var refFile = FSOOpenTextFile(refFileName, FSOForReading);
                outFile.Write(refFile.readAll());
                refFile.Close();
                    
                if (restOfLine.length > 0)
                    outFile.WriteLine(restOfLine);
            }
            else { 
                if (line.match(/runJS *\( *\[/)) {
                    while (!inFile.AtEndOfStream && !line.match(/\[[^\]]*\]/)) 
                        line += inFile.ReadLine();

                    line = line.replace(/runJS *\( *(\[[^\]]*\]) *\)/m, "runJS([WScript.ScriptName])");
                }
                outFile.WriteLine(line);
            }
        }
        outFile.Close();
    }
    finally {
        inFile.Close();
    }

    return 0;
}

function linkDFullPath()
{
    return srcBaseFromScript() + "\\tools\\x86\\linkd.exe";
}

/****************************************************************************/
/* generate a new directory to place automation results in.  Note that it
   does not generate any new results, it just create the new directory
   (and returns it).  It creates a new diretory of the form run.<DATE>
   and creates a JUNCTION point (symbolic link) run.current to it.  It
   also house cleans, deleting older directories so that only 'numKeep'
   old directories are kept 

     Parameters:
       outDirBase : The directory to create the run dirs in 
       numKeep    : The number of old runs to keep around 
*/

function newRunDir(outDirBase, numKeep) {

    if (outDirBase == undefined) {
        outDirBase = Env("_NTBINDIR") + "\\automation"; 
        if (!Env("_NTBINDIR"))
            throwWithStackTrace(new Error(1, "newRunDir: Required argument outDirBase missing"));
    }
    if (numKeep == undefined)
        numKeep = 3;

    logCall(LogUtil, LogInfo10, "newRunDir", arguments);
    FSOCreatePath(outDirBase);

        // Remove old runs.  Their naming is from oldest to newest, and FSOGetDirPattern sorts by name
    var runDirs = FSOGetDirPattern(outDirBase, /run\.\d.*/);
    for(var i = 0; i < runDirs.length-numKeep; i++) {
        var runDir = runDirs[i];
        logMsg(LogUtil, LogInfo, "newRunDir: deleting old dir ", runDir, "\n");
        try { FSODeleteFolder(runDir, true); } catch(e) {}
    }

    var outDir = getUniqueDateFileName(outDirBase, "run.", "");
    logMsg(LogUtil, LogInfo10, "newRunDir: outDir: ", outDir, "\n");
    FSOCreatePath(outDir);

            // create a link to the current directory
    var current = outDirBase + "\\run.current";
    logMsg(LogUtil, LogInfo10, "newRunDir: updating link : ", current, "\n");
    try {
        if (FSOFolderExists(current))
            runCmd(linkDFullPath() + " \"" + current + "\" /d");
        runCmd(linkDFullPath() + " \"" + current + "\" \"" + outDir + "\"");

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

/****************************************************************************/
/* parses a file of key: pairs into a structure */

function parseFile(fileName) {

    var ret = {};
    ret.fileName = fileName;
    var data = FSOReadFromFile(fileName).split(/\s*\n/);
    // logMsg(LogTask, LogInfo, "parseFile: Parsing '", fileName, "'\n");
    for(var j = 0; j < data.length; j++) {
        if (data[j].match(/^(\w+)\s*:\s*(.*)/)) {
            var key = RegExp.$1;
            var val = RegExp.$2;
            if (ret[key])
                ret[key] += "\r\n" + val;
            else 
                ret[key] = val;

            // logMsg(LogTask, LogInfo, "parseFile:     ", key, " = ", val, "\n"); 
        }
    }
    return ret;
}

/****************************************************************************/
/* parses a string like "key1=value1; key2=value2; key3" into a structure   */
/* mapping keys to values.  Omitted values are given the empty string       */

function parseKeyValueString(string)
{
    var ret = {};
    var pairs = string.split(/;\s*/);
    for(var i = 0; i < pairs.length; i++)
    {
        if (pairs[i].match(/^(\w+)\s*=\s*(.*)/) )
        {
            var key = RegExp.$1;
            var val = RegExp.$2;
        }
        else
        {
            var key = pairs[i];
            var val = "";
        }
        ret[key] = val;
    }
    
    return ret;
}

/********************************************************************************/
/* convert a structure to a string of the form "key1=value1; key2=value2; key3" */
/* this is the inverse of parseKeyValueString                                   */

function toKeyValueString(obj)
{
    var ret = "";
    var isFirst = true;
    for (var key in obj) 
    {
        if( !isFirst )
            ret += "; ";
            
        ret += key;
        var val = obj[key];
        if( val && val != "" )
        {
            ret += "=" + val
        }
        isFirst = false;
    }
    return ret;
}

/****************************************************************************/
/* Reads all "set x=y" statements in fileName, and creates a hash with each
   entry.*/

function parseBatchFileForSetStatements(fileName) {
    if (!FSOFileExists(fileName)) {
        throwWithStackTrace(new Error(1, fileName + " not found."));        
    }
    var contents = {};
    var setupInfo = FSOReadFromFile(fileName).split(/\n/);
    for (var i = 0; i < setupInfo.length; i++) {
        //Skip over comment lines.
        if (setupInfo[i].match(/^rem/i)) {
            continue;
        }
        if (setupInfo[i].match(/set\s*(\w*)\s*=\s*(.*?)\s*$/im)) {
            contents[RegExp.$1.toUpperCase()] = RegExp.$2;
        }
    }
    return contents;
}
    
/***********************************************************************/
/* This will read and return a variable containing the contents of
   fileName for the purpose of importing custom .js files. To use this
   function, put a line such as "eval(includeJS(<filename>))" in your
   runjs.local.js file. All such files must be located relative to
   the scriptLib directory. */

function includeJS(fileName) {
    var scriptLibDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1] + "\\scriptlib";
    var fullPath = scriptLibDir + "\\" + fileName;
    if (FSOFileExists(fullPath)) {
        logMsg(LogJS, LogInfo, "Including: ", fullPath, "\n");
        var fileData = FSOReadFromFile(fullPath);
        return fileData;
    }
    else {
        logMsg(LogJS, LogInfo, "Include file not found: ", fullPath, "\n");
    }
}

/*****************************************************************************/
/* parses options of the form /option:value.  optDesc is an array of strings 
   reprsenting option names.  This parses options into a table keyed by option 
   name whose value is the value.  Note that the :value is optional in which
   case the option is true if present */

function getOptions(optDescs, options) {

    logMsg(LogUtil, LogInfo100, "getOptions([", optDescs, "], ", options, ")\n");
	if (typeof(options) == "object")
		return options;

    var ret = {};
    if (options != undefined) {
        for(var i = 0; i < optDescs.length; i++) {
            var optDesc = optDescs[i];
            if (options.match(new RegExp("\\/" + optDesc + "([:=]([^\\/]+))?\\b", "i"))) {
                options = RegExp.leftContext + RegExp.rightContext;
                if (RegExp.$1 == "")
                    ret[optDesc] = true;
                else 
                    ret[optDesc] = RegExp.$2;
                logMsg(LogUtil, LogInfo100, "getOptions: option ", optDesc, " = ", ret[optDesc], "\n");
            }
        }
        if (!options.match(/^\s*$/)) {
            throw (new Error(1, "Unrecognised option '" + options + "'\r\nValid Options: /" + optDescs.join(" /")));
		}
    }

    return ret;
}

/*****************************************************************************/
/* make a shortcut that is easy to parse progamatically but the windows
   shell also knows about (the .url shortcut).  */

function urlShortCutCreate(src, target) {

    if (!src.match(/\.url$/i))
        src += ".url";

    if (!target.match(/^\\\\/)) {
        if (FSOFileExists(target))                // get absolute name
            target = FSOGetFile(target).Path;
        else 
            target = FSOGetFolder(target).Path;
        target = uncPath(target);                // try to get UNC name (eg. \\machine\...)
    }

    target = target.replace(/\\/g, "/");    // replace \ with /
    FSOWriteToFile("[InternetShortcut]\r\nURL=file:" + target + "\r\n", src);
}

/*****************************************************************************/
/* returns the path name inside a url shortcut */

function urlShortCutTarget(shortCut) {

    var data = FSOReadFromFile(shortCut);
    if (!data.match(/\[InternetShortcut\]\s*URL=file:(.*?)\s*$/im))
        throwWithStackTrace(Error(1, "Could not parse shortcut data '" + data + "'"));
    var path = RegExp.$1;
    path = path.replace(/\//g, "\\");
    logMsg(LogUtil, LogInfo100, "urlShortCutTarget() = ", path, "\n");
    return path;
}

/*****************************************************************************/
/* a trivial htmlAnchor helper */

function htmlAnchor(name) {
    return "<A HREF='" + name + "'> " + name + " </A>";
}

/****************************************************************************/
/* Get a html tag for a relative UNC hyperlink
parameters:
    fullPath - the local full-path
    relDir - directory that full path is relative to. 
*/
function htmlRelLink(fullPath, relDir)
{
    return "<A HREF='" + relPath(fullPath, relDir) + "'> " + uncPath(fullPath) + " </A>";
}

/*****************************************************************************/
/* escape HTML meta characters so that they are rendered correctly as HTML */

function escapeHTML(str)
{
        // avoid making strings if no escaping is needed.
    if (!str.match(/[^ -~\n\r\t]|&|<|>|"/))
        return str;

    str = str.replace(/[^ -~\n\r\t]/g, "");
    str = str.replace(/&/g,"&amp;");
    str = str.replace(/</g,"&lt;");
    str = str.replace(/>/g,"&gt;");
    str = str.replace(/"/g,"&quot;");
    return str
}

/*****************************************************************************/
/* remove any duplicats in the list of strings 'strList' and return the 
   resulting list */

function removeDups(strList) {

    var ret = [];
    var set = {};
    for(var i in strList) {
        var elem = strList[i];
        if (!set[elem]) {
            ret.push(elem)
            set[elem] = true;
        }
    }
    return ret;
}

/*****************************************************************************/
/* make a set (object with elements for field names with values true or false)
   from a list */

function setNew(list) {

    var ret = {};
    if (list) {
        for(var i in list) {
            ret[list[i]] = true;
        }
    }
    return ret;
}

/*****************************************************************************/
/* form the union of two sets */ 
function setUnion(set1, set2) {

    var ret = {};
    for (var key in set2) 
        ret[key] = set2[key];
    for (var key in set1) 
        ret[key] = set1[key];
    return ret;
}

/*****************************************************************************/
/* form the instersection of two sets */ 
function setIntersect(set1, set2) {

    var ret = {};
    for (var key in set1) 
        if (set2[key])
            ret[key] = set1[key];
    return ret;
}

/*****************************************************************************/
/* get the difference of two sets */
function setDiff(set1, set2) {

    var ret = {};
    for (var key in set1) 
        if (!set2[key])
            ret[key] = set1[key];
    return ret;
}

/*************************************************************************/
/* return a list of keys in the object 'obj' */

function keys(obj) {
    var keys = [];
    for (var key in obj) 
        keys.push(key);
    return keys;
}

/*************************************************************************/
/* return a list of keys in the object 'obj' */

function values(obj) {
    var values = [];
    for (var key in obj) 
        values.push(obj[key]);
    return values;
}

/*****************************************************************************/
/* returns true if 'elem' is a member of 'list' */
function member(list, elem) {

	for (var i = 0; i < list.length; i++)
		if (list[i] == elem)
			return true;

	return false;
}

/*****************************************************************************/
/* call 'ftn' on each element in 'list' and for a string out of the result 
   separated by 'sep' */

function joinMap(list, sep, ftn) {

    if (list ==  undefined)
        return "";
    var outList = [];
    for(var i in list) 
        outList.push(ftn(list[i]));
    
    return outList.join(sep);
}

/*************************************************************************/

function throwWithStackTrace(e) {
    var descr = e
    try {
        descr = e.description;
    } catch(ex) {}

    var ftn = arguments.callee;
    var stackTrace = "";
    var maxTrace = 100;
    var ftns = {};
    while (ftn != undefined) {

        var name = "UNKNOWN";
        var paramNames = "";
        var ftnBody = ftn.toString();
        if (ftnBody.match(/^\s*function\s+(\w+)\s*\(\s*(.*)\)/)) {
            name = RegExp.$1;
            paramNames = RegExp.$2
        }

        var argsStr = "";
        var args = ftn.arguments;
        for(var i = 0; i < args.length; i++) {
            if (argsStr != "")
                argsStr += ", ";
            if (paramNames.match(/(\w+)(\s*,\s*)?/)) {
                argsStr += RegExp.$1 + "=";
                paramNames = RegExp.rightContext;
            }
            argsStr += args[i];
        }
        var fullName = name + "(" + argsStr + ")\n";
        if (ftns[fullName]) {
            stackTrace += "Can't show rest of stack trace (recursion). (JScript limitation)\n";
            break;
        }
        ftns[fullName] = true;
        stackTrace += fullName
        ftn = ftn.caller;
        --maxTrace;
        if (maxTrace <= 0)
            break;
    }

    logMsg(LogUtil, LogWarn, "**** EXCEPTION('", descr, "') THROWN at STACKTRACE {\n", stackTrace, "}\n");
    throw e;
}

/*****************************************************************************/
/* returns a string that displays the hex as well as the chars of a string */

function dumpStr(s) {

    var i = 0;
    var ret = "";
    var done = false;
    while(!done) {
        var hex = "";
        var chars = "";
        for (var j = 0; j < 16; j++) {
            if (i < s.length) {
                var charCode = s.charCodeAt(i);
                if (charCode < 16)
                    hex += " ";
                hex += charCode.toString(16);
                hex += " ";
                if (charCode > 32 && charCode < 128)
                    chars += s.charAt(i);
                else
                    chars += ".";
            }
            else {
                hex += "   ";
                done = true;
            }
            if (j == 8)
                hex += " ";
            i++;
        }
        ret += hex + "  " + chars + "\r\n";
    }
    return ret;
}

/*****************************************************************************/
/* removes tabs from 'line' assuming a tabstop at 'tabSize' and returns the
   resulting string */

function detab(line, tabSize) {

    if (tabSize == undefined)
        tabSize = 4;

    var pieces = line.split("\t");
    if (pieces.length == 1)
        return line;

    var ret = pieces[0];
    var curCol = pieces[0].length;
    for(i = 1 ; i < pieces.length; i++) {
        var piece = pieces[i];

        var tabCount = 1 + (tabSize - (curCol+1) % tabSize);
        ret += "          ".substr(0, tabCount);
        curCol += tabCount;

        curCol += piece.length;
        ret += piece;
    }

    return ret

}

/****************************************************************************/
/* Launch the InternetExplorer browser on htmlFile and wait for the result

  Parameters
    htmlFile : URL of the web page to open. 
               This can be a file (eg. "c:\Temp\foo.htm" or "\\mymachine\share\foo.htm"),
               or it can be a URL (eg. "http://webserver/webpage.htm")
*/

function launchIECmd(htmlFile) {

    if (!htmlFile.match(/http:/)) {
        if (!FSOFileExists(htmlFile))
            throwWithStackTrace(new Error(1, htmlFile + " not found."));
        htmlFile = "file:" + uncPath(htmlFile);
    }

    var browserCmd = WshShell.regRead("HKCR\\applications\\iexplore.exe\\shell\\open\\command\\");
    browserCmd = browserCmd.replace(/%1/, htmlFile);

    logMsg(LogUtil, LogInfo10, "launchIECmd: found browserCmd ", browserCmd, "\n");
    return runCmdToLog(browserCmd, runSetTimeout(3600, runSetNoShell(runSetOutput(undefined))));
}

/*****************************************************************************/
/* given a name and a url, add that url as a favorite for the current user */

function makeFavorite(favoriteName, url) {

    var favoriteDir = WshShell.SpecialFolders("Favorites");
    var fullFavorite = favoriteDir + "\\" + favoriteName + ".url";
    logMsg(LogUtil, LogInfo, "Making IE Favorite: ", favoriteName, " -> ", url, "\n");
    FSOWriteToFile("[InternetShortcut]\r\nURL=" + url + "\r\n", fullFavorite);
    return 0;
}

/*****************************************************************************/
/* creates a command window shortcut called 'name' passing 'cmdArgs' to the
   command (usually /k <batchFile>), and setting the working directory to
   'workingDir', have setting the shortuct description to 'description' */

function desktopCmdShortCut(name, cmdArgs, workingDir, description, systemDir) {

    if (systemDir == undefined)
    systemDir = "system32";

    strDesktop = WshShell.SpecialFolders("Desktop");
    var oShellLink = WshShell.CreateShortcut(strDesktop + "\\" + name)

    oShellLink.TargetPath = "%SystemRoot%\\" + systemDir + "\\cmd.exe";
    oShellLink.Arguments = cmdArgs;
    oShellLink.WindowStyle = 1;

    // oShellLink.Hotkey = "Ctrl+Alt+E";
    oShellLink.IconLocation = "%SystemRoot%\\" + systemDir + "\\cmd.exe, 0";
    oShellLink.Description = description;
    oShellLink.WorkingDirectory = workingDir;

    oShellLink.Save();
    return 0;
}

/****************************************************************************/
/* returns the full path for 'fileName' that will be used to run the command
   if it was executed */

function findOnPath(fileName) {

    var paths = Env("PATH").split(";");
    var exts = Env("PATHEXT").split(";");

    paths.unshift(".");    
    for (var i=0; i < paths.length; i++) {
        var path = paths[i] + "\\" + fileName;

        if (FSOFileExists(path))
            return path;

        for (var j=0; j < exts.length; j++) {
            var pathWithExt = path + exts[j];
            if (FSOFileExists(pathWithExt))
                return pathWithExt;
        }
    }

    return undefined;
}

/****************************************************************************/
/* return a string where 'str' which has had newlines added to it so that
   no line is greater than 'len' characters long'.  After each newline
   'indent' spaces are added (which don't count toward the length.  Note
   that if a space can not be found within 15 chars of where a break is
   needed, we break even if there is no space */

function splitToFit(str, len, indent) {

    str = str.replace(/\t+/g, " ");
    outStr = "";
	var indentStr = padSpace(indent);
    while(str.length > len) {
        for(var i = len - 1; ; --i) {
            if (str.charAt(i) == ' ') {
                outStr += str.substr(0, i) + "\r\n";
                str = indentStr + str.substr(i+1);
                break;
            }
            if (i < len-15 || i == 0) {
                outStr += str.substr(0, len) + "\r\n";
                str = indentStr + str.substr(len);
                break;
            }
        }
    }
    outStr += str;
    return outStr;
}

function nsIsNullOrEmpty(value) {
    if (value == undefined || value == null || value == "")
        return true;

    return false;
}

// On Win9x, this variable doesn't exist, but we're counting on that.
var OS = Env("OS");
var PlatformIsWin9x = nsIsNullOrEmpty(OS);

function IsWin9x() {
    return PlatformIsWin9x;
}

var PlatformIsWinLH = -1;

function IsWinLH() {
    if (PlatformIsWinLH == -1) {
        PlatformIsWinLH = nsGetOSVersion().slice(0,3) == "6.0" ? 1 : 0;
    }

    return PlatformIsWinLH == 1;
}

var PlatformIsWinLHOrLater = -1;

function IsWinLHOrLater() {
    if (PlatformIsWinLHOrLater == -1) {
        PlatformIsWinLHOrLater = nsGetOSVersion().slice(0,1) >= "6" ? 1 : 0;
    }

    return PlatformIsWinLHOrLater == 1;
}

var PlatformIsWin7 = -1;

function IsWin7() {
    if (PlatformIsWin7 == -1) {
        PlatformIsWin7 = nsGetOSVersion().slice(0,3) == "6.1" ? 1 : 0;
    }

    return PlatformIsWin7 == 1;
}

var PlatformIsWin8 = -1;

function IsWin8() {
    if (PlatformIsWin8 == -1) {
        PlatformIsWin8 = nsGetOSVersion().slice(0,3) == "6.2" ? 1 : 0;
    }

    return PlatformIsWin8 == 1;
}

var PlatformIsWinBlue = -1;

function IsWinBlue() {
    if (PlatformIsWinBlue == -1) {
        PlatformIsWinBlue = nsGetOSVersion().slice(0,3) == "6.3" ? 1 : -1;
    }
	
	if (PlatformIsWinBlue == -1 && IsWin8()) {
	    var buildVersion = nsGetOSVersion().split(".")[2];
		if (buildVersion >= 9293) {
			// build 9293 or above has the breaking BLUE change (delay load dependancy on api-ms-win-core-winrt-errorprivate-l1-1-1.dll)
			PlatformIsWinBlue = 1
		}
	}

	if (PlatformIsWinBlue == 1)
	{
		// blue only is possible, even if detected with 6.2 (win8)
		PlatformIsWin8 = 0;
	}
	else
	{
		PlatformIsWinBlue = 0;
	}

    return PlatformIsWinBlue == 1;
}

var PlatformIsWinTH = -1;

function IsWinTH() {
    if (PlatformIsWinTH == -1) {
        PlatformIsWinTH = nsGetOSVersion().slice(0, 4) == "10.0" ? 1 : 0;
    }

    return PlatformIsWinTH == 1;
}

function IsWin8OrLater() {
    return IsWin8() || IsWinBlueOrLater();
}

function IsWinBlueOrLater() {
    return IsWinBlue() || IsWinTH();
}

var ProcessorCount = null;
function nsGetProcessorCount() {
    if (!ProcessorCount) {
        var oExec = WshShell.Exec(nsGetCommandProcessor() + " echo %NUMBER_OF_PROCESSORS%");
        var sOutput = "";

        while (!oExec.StdOut.AtEndOfStream) {
            sOutput += oExec.StdOut.Read(1);
        }

        // echo outputs the environment variable with a newline - strip the newline
        ProcessorCount = Number(sOutput);
    }

    return ProcessorCount;
}

function nsGetCommandProcessor() {
    // command doesn't exist on the 64bit OS, and cmd doesn't exist on
    // Win9x, so we really don't have a common shell to use.

    if (IsWin9x())
        return "command /C";
    else
        return "cmd /C";
}// nsGetShell

var OSVersion = null;
function nsGetOSVersion() {
    if (OSVersion == null) {
        var oExec = WshShell.Exec(nsGetCommandProcessor() + " ver");
        var sOutput="";

        while (!oExec.StdOut.AtEndOfStream) {
            sOutput += oExec.StdOut.Read(1);
        }

        // The output of ver looks like this, with newlines before and after the printed string
        //
        // Microsoft Windows XP [Version 5.1.2600]
        //
        // We want to grab the version number.

        var reLine =  /[\s\S]*\[Version (.*)\]/;
        reLine.exec(sOutput);
        OSVersion = RegExp.$1;
    }

    return OSVersion;
}// nsGetOSVersion

var elevated;
function isElevated() {
    if (elevated != undefined)
        return elevated;
        
    if (IsWinLHOrLater()) {
        var oShell = WScript.CreateObject("WScript.Shell");
        var oExec = oShell.Exec("whoami /groups");
        var szStdOut = "";

        while (oExec.Status == WScript.WshRunning)
        {
            WScript.Sleep(100);
            if (!oExec.StdOut.AtEndOfStream)
            {
                szStdOut += oExec.StdOut.ReadAll();
            }
        }

        switch (oExec.ExitCode)
        {
        case 0:
            if (!oExec.StdOut.AtEndOfStream)
            {
                szStdOut += oExec.StdOut.ReadAll();
            }
            if (szStdOut.search(new RegExp("S-1-16-12288")) != -1)
            {
                elevated = true;
            }
            else if (szStdOut.search(new RegExp("S-1-16-8192")) != -1)
            {
                elevated = false;
            }
            else
            {
                elevated = null;
            }
            break;

        default:
            if (!oExec.StdErr.AtEndOfStream())
            {
                oExec.StdErr.ReadAll();
            }
            elevated = null;
            break;
        }
    }
    else {
        // non-Longhorn, assume elevated/admin
        elevated = true;
    }
    return elevated;
}

function trim(str) {
    return str.replace(/^\s*/, "").replace(/\s*$/, "");
}

