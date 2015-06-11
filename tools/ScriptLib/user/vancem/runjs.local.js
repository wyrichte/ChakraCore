function coldStartChurn()
{
	debugger;
	var sdObj = sdConnect();
	var changes = sdChanges("...@1293686,", sdObj)
	var teamChanges = [];
	for(var i = 0; i < changes.length; i++) {
		var change = changes[i];
		
		if (!change.user.match(/clrsnap0/i))
			continue;

		if (change.desc.match(/Update.*comments/i))
			continue;

		if (change.user.match(/jkotas/i) || change.user.match(/vancem/i) ||
			change.user.match(/briansul/i) || change.user.match(/joshwil/i)) {
			teamChanges.push(change.change);
		}
	}


	for(var i = 0; i < teamChanges.length; i++) {
		var changeNum = teamChanges[i];
		var descrition = sdDescribe(changeNum, sdObj)

	}
}


/******************************************************************************/
function upTime() {

    debugger;

    logMsg(LogScript, LogInfo, "\nUptime = ", runCmd("systemInfo").output.match(/System Up Time:\s*(.*)/)[1], "\n");
    return 0;
}

function incVM() {
    logSetTranscript("vmBuild.txt");
    runCmdToLog("rbuild", runSetCwd(Env("_NTBINDIR") + "\\ndp\\clr\\src\\vm"));
    runCmdToLog("rbuild", runSetCwd(Env("_NTBINDIR") + "\\ndp\\clr\\src\\dlls\\mscoree"));
    runCmdToLog(Env("_NTTREE") + "\\clrsetup /inc");
}

function incVMOld() {
    razzleBuild(Env("_BuildType"), Env("_BuildArch"), "ndp\\clr\\src\\vm");
    razzleBuild(Env("_BuildType"), Env("_BuildArch"), "ndp\\clr\\src\\dlls\\mscoree");
    clrSetupWithCache(Env("_NTTREE"), "/inc")
}


/******************************************************************************/
function delDupDumps(dir, dirPat) {
    
    if (dir == undefined)
        dir = "\\\\clrsnapbackup2\\public\\rollingIA64"
    if (dirPat == undefined)
        dirPat = /\d+(\.\d+)?/;

    var dumps = {};
    var dirs = FSOGetDirPattern(dir, dirPat)
    dirs.reverse();
    for(var i = 0; i < dirs.length; i++) {
        var dir = dirs[i];
        logMsg(LogScript, LogInfo, "Processing'", dir, "'\n");
        var run = runCmd("dir /b /s " + dir + "\\*.DumpProc", runSetNoThrow());
        if (run.exitCode != 0)
            continue;
        var out = run.output;
        while (out.match(/^(.+?([^\\]+\S))\s*$/m)) {
            var path = RegExp.$1;
            var fileName = RegExp.$2;
            out = RegExp.rightContext;
            logMsg(LogScript, LogInfo, "Found dump '", fileName, "'\n");

            if (!dumps[fileName])
                dumps[fileName] = [];
                
            if (dumps[fileName].length >= 3) {
                logMsg(LogScript, LogInfo, "Already have three dumps for ", fileName, "\n");
                runCmdToLog("del /q /s " + path + "\\*.*");

                var dumpStr = ""
                for (var j = 0; j < dumps.length; j++)
                    dumpStr += "<LI> " + htmlAnchor(dumps[i]) + "\r\n";

                FSOWriteToFile(
                    "<HTML><BODY>\r\n" +
                    "This dump has been removed because it is redundant.  Please see the following dumps instead\r\n" +
                    "Note that these dumps are on slightly different builds\n" +
                    "<UL>\r\n" + dumpStr + "</UL>\r\n" + 
                    "</BODY></HTML>\r\n",
                    path + "\\Report.html");
            }
            else 
                dumps[fileName].push(path);
        }
    }
    return 0;
}

/*********************************************************************************************/
/* check out windbg from all the right places in the tree and put in the latest version */

function updateWinDbg() {

    var arches = ["x86", "ia64", "amd64"];

    var clrbvts =Env("_NTBINDIR")+ "\\ddsuites\\src\\clr";
    for(var i = 0; i < arches.length; i++) {
        var arch =  arches[i];
        var iarch = arch;
        if (arch == "x86") 
            iarch = "i386";
        var targetDir = clrbvts + "\\" + arch + "\\tools\\" + iarch + "\\debuggers";
        updateWinDbgAt(arch, targetDir);
    }

    updateWinDbgAt("x86", Env("_NTBINDIR")+"\\ddsuites\\tools\\x86");
}

/*********************************************************************************************/
/* check out all the files needed for windbg from 'targetDir' and update them with the lastest
   version for arch 'arch'.  If 'help' is true, then include help files too. */

function updateWinDbgAt(arch, targetDir) {

    logMsg(LogScript, LogInfo, "**** Updating windbg at ", targetDir, " for arch ", arch, "{\n"); 

    var files = ["cdb.exe", "dbgeng.dll", "dbghelp.dll", "decem.dll", "ext.dll", "exts.dll",
                 "ntsd.exe", "ntsdexts.dll", "srcsrv.dll", "symsrv.dll", "uext.dll", 
                 "windbg.exe", "debugger.chi", "debugger.chm"];

    for(var j = 0; j < files.length; j++) {
        var file =  files[j];

        var srcDir = "\\\\dbg\\privates\\latest\\uncompressed\\" + arch + "\\";
        var srcFullPath = srcDir + file;
        if (!FSOFileExists(srcFullPath))
            var srcFullPath = srcDir + "winxp\\" + file;
        if (!FSOFileExists(srcFullPath))
            var srcFullPath = srcDir + "winext\\" + file;

        var destFullPath = targetDir + "\\" + file;

        if (FSOFileExists(destFullPath)) {
            runCmdToLog("sd edit " + destFullPath);
            runCmdToLog("xcopy /y " + srcFullPath + " " + destFullPath);
        }
        else {
            logMsg(LogScript, LogWarn, "Target ", destFullPath, " does not exist, skipping...\n");
        }
    }

    logMsg(LogScript, LogInfo, "}\n");
}

/******************************************************************************/
function updateSmarty(srcFile) {

    var arches = ["x86", "ia64", "amd64"];
    for(var i = 0; i < arches.length; i++) {
        var arch =  arches[i];
        var target = Env("_NTBINDIR")+ "\\ddsuites\\src\\clr\\" + arch + "\\productionTools\\Smarty\\smarty.pl";
        runCmdToLog("sd edit " + target);
        runCmdToLog("copy " + srcFile + " " + target);
    }

    return 0;
}


/*****************************************************************************/
function updateDumpProc() {
    var src = Env("_NTBINDIR")+ "\\ndp\\clr\\bin\\runjs.wsf";

    var src = Env("_NTBINDIR")+ "\\ndp\\clr\\bin\\scriptlib\\dumpJS.wsf";
    var target = Env("_NTBINDIR")+ "\\ddsuites\\tools\\dumpJS.wsf";
    runCmdToLog("sd edit " + target);
    runCmdToLog("runjs package " + src + " " + target);
    
    var clrTest = Env("_NTBINDIR")+ "\\clrTest";
    var clrTestIni = clrTest + "\\sd.ini";
    var clrTestTarget = clrTest + "\\testBin\\productionTools\\Smarty\\dumpProc\\runjs.wsf";
    runCmdToLog("sd -i " + clrTestIni + " edit " + clrTestTarget);
    runCmdToLog("copy " + target + " " + clrTestTarget);

    var arches = ["x86", "ia64", "amd64"];
    for(var i = 0; i < arches.length; i++) {
        var arch =  arches[i];
        var smartyTarget = Env("_NTBINDIR")+ "\\ddsuites\\src\\clr\\" + arch + "\\productionTools\\Smarty\\dumpProc\\runjs.wsf";
        runCmdToLog("sd edit " + smartyTarget);
        runCmdToLog("copy " + target + " " + smartyTarget);
    }

    return 0;
}

/*****************************************************************************/
function ftnRefs(file) {

    var fileData = FSOReadFromFile(file);
    while(fileData.match(/function *(\w+).*{(\s(.|\n)*?)^}/m)) {
        fileData = RegExp.rightContext;
        var ftnName = RegExp.$1;
        var ftnBody = RegExp.$2;

        logMsg(LogUtil, LogInfo, "Got function: ", ftnName, " ids = {\n");
        var ids = ftnBody.split(/\W+/);
        var idTab = {};
        for(var i = 0; i < ids.length; i++) 
            idTab[ids[i]] = 1;

        for(var i in idTab) 
            logMsg(LogUtil, LogInfo, i, "\n");

        logMsg(LogUtil, LogInfo, "}\n");
    }
    return 0;
}

function ftnRefsInDir(dir) {

    var files = FSOGetFilePattern(dir);
    for(var i = 0; i < files.length; i++) 
        listFunctions(files[i]);
    return 0;
}
    
/*****************************************************************************/
function readMethodOrder(fileName) {
    var file = FSOOpenTextFile(fileName, 1)

    var inMethodProfile = false;
    while (!file.AtEndOfStream) {
        var line = file.ReadLine();
        if (line.match(/^(\w+\s?\w+\s?\w+):\s*$/)) {
            inMethodProfile = (RegExp.$1 == "MethodProfilingData");
            logMsg(LogUtil, LogInfo, "Section '", RegExp.$1, "'\n");
        }
        else if (inMethodProfile && line.match(/^ +Token: +\S+, +Flags: +\S+ *(.*)/)) {
            var method = RegExp.$1;
            logMsg(LogUtil, LogInfo, "Got method ", method, "\n");      
        }
    }
}

/*****************************************************************************/
function monitorLoads(pat) {

    var wbemServices = procGetWMI();
    var event = wbemServices.ExecNotificationQuery("select * from Win32_ModuleLoadTrace");
    
    if (pat && typeof(pat) == "string")
        pat = new RegExp(pat);

    for(;;) {
        var obj = event.NextEvent();
        var id = obj.ProcessID;
        var fileName = "" + (obj.FileName)
        if (!pat || fileName.match(pat))
            logMsg(LogScript, LogInfo, "Proc: ", id,  " Loaded: ", fileName, "\n");
    }
}

/*****************************************************************************/
/* monitor process creation and dieing on the system.  This was written to
   be self contained (does not rely on runjs infrastructure) */

function monitorProcs(machineName) {

    var wbemServices = procGetWMI(machineName);
    var event = wbemServices.ExecNotificationQuery("select * from Win32_ProcessTrace");
    var live = {};
    var startTime = undefined;
    var lastPrintProcs = 0;
    
    for(;;) {
        var now = new Date().getTime();
        if (now - lastPrintProcs > 600000) {    // every 10 min
            logMsg(LogScript, LogInfo, "***************** Current process snapshot **************\n")
            // procTreePrint("/fullCmdLine");
            procTreePrint();
            lastPrintProcs = now;
        }

        var obj = event.NextEvent();
        var id = obj.ProcessID;
        if (live[id]) {
            logMsg(LogScript, LogInfo, "DIE  : ");
            live[id] = undefined;
        }
        else {
            logMsg(LogScript, LogInfo, "START: ");
            live[id] = 1;
        }
        var time = obj.TIME_CREATED;
        if (!startTime)
            startTime = time;
        time -= startTime;

        logMsg(LogScript, LogInfo, "Class: ", obj.__CLASS, "\n");
        logMsg(LogScript, LogInfo, "Name: ", obj.ProcessName, " ID: ", id,  " Parent: ", obj.ParentProcessID, " Time: ",  (time / 10000000).toFixed(6), "\n");
        var e = new Enumerator(wbemServices.ExecQuery("select * from Win32_Process where ProcessID = " + obj.ProcessID));
        if (!e.atEnd()) {
            var ret = e.item();
            logMsg(LogScript, LogInfo, "    CMD: ",  ret.CommandLine, "\n");
        }
    //  logMsg(LogScript, LogInfo, "\n");
    }
}

/*****************************************************************************/
function _getLoginProfile(name) {

    var hostName = ".";
    var wbemServices = GetObject("winmgmts://" + hostName);
    if (!wbemServices) 
        throw new Error(1, "Could not open the winmgmts object! (Someone corrupted XP system dlls, or WMI not installed (on Win9x)?)")

    var wbemObjectSet = wbemServices.ExecQuery("select * from Win32_NetworkLoginProfile where Name contains " + name);
    
    logMsg(LogScript, LogInfo, "Got wbemObjectSet\n");
    for (var e = new Enumerator(wbemObjectSet); !e.atEnd(); e.moveNext()) {
        var profile = e.item();
        logMsg(LogScript, LogInfo, "Got home dir ", profile.HomeDirectory, "'\n");
    }
}

/*****************************************************************************/
function dumpAsJScript(data, fileName) {

    var file = FSOOpenTextFile(fileName, FSOForWriting, true);
    var objIDs = {};
    dumpAsJScriptHelper(data, file, objIDs);

    file.Write(";");
    for (var objID in objIDs) {
        file.Write("setFields(");
        file.Write(objID);
        file.Write(",");
        dumpFieldsAsJScript(objIDs[objID], file, objIDs);
        file.WriteLine(");");
    }

    file.Close();
}

function dumpAsJScriptHelper(data, file, objIDs) {

    var type = typeof(data);
    if (type  == "object") {
        if (data.objID) {
            file.Write(data.objID)
            if (!objIDS[data.objID]) {
                file.Write(" = {}");
                objIDS[data.objID] = data;
            }
        }
        else 
            dumpFieldsAsJScript(data, file, objIDs)
    }
    else if (type == "string")
        file.Write("\"" + data + "\""); // FIX now quote it properly
    else 
        file.Write(data);               // Not correct for function objects?
}

function dumpFieldsAsJScript(data, file, objIDs) {
    file.WriteLine("{");
    var first = true;
    for (var idx in data) {
        if (!first) 
            file.WriteLine(",");
        first = false;
        file.Write(idx);
        file.Write(":");
        dumpAsJScriptHelper(data[idx], file, objIDs);
    }
    file.WriteLine("}");
}

/************************************************************************************/
function workItem(fileName) {
    logSetFacilityLevel(LogProductStudio, LogInfo1000);

    var bugDbName = "DevDiv Schedule";
    var file = FSOOpenTextFile(fileName, FSOForReading);
    var bugData = {};
    var lastKey = undefined;
    while (!file.AtEndOfStream) {
        var line = file.ReadLine();
        if (line.match(/^\s*([\w\s]*\w)\s*:\s*(.*)/)) {
            var key = RegExp.$1;
            var val = RegExp.$2;
            if (bugData[key] && key == lastKey)
                bugData[key] += "\r\n" + val;
            else 
                bugData[key] = val;
            lastKey = key;
        }
        else if (line.match(/^\s*---*\s*$/)) {
            _workItemHelper(bugDbName, bugData);
            bugData.Title = undefined;
        }
        else if (!line.match(/^\s*$/))
            logMsg(LogProductStudio, LogWarn, "Skipping line ", line, "\n");
    }
    
    if (bugData.Title != undefined)
        _workItemHelper(bugDbName, bugData);

    return 0;
}

function _workItemHelper(bugDbName, bugData) {

    var bugDb = bugConnect(bugDbName);
    var bug = bugCreate(bugDb);
    for (var field in bugData) {
        var value = bugData[field];
        bugFieldSet(bugDb, bug, field, value);
    }
    bugSave(bug);
    logMsg(LogProductStudio, LogInfo, "Created bug with ID ", bugFieldGet(bugDb, bug, "ID"), "\n");
}

function moduleLoad() {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    var wbemObjectSet = wbemServices.ExecQuery("select * from Win32_ModuleLoadTrace");
    logMsg(LogProc, LogInfo, "modules:\n");
    for (var e = new Enumerator(wbemObjectSet); !e.atEnd(); e.moveNext()) {
        var modObj = e.item();
        logMsg(LogProc, LogInfo, "modules: Got ", modObj.FileName, " ProcessID ", modObj.ProcessID, "\n")
    }
}

function xmlTest() {
    var bigStruct;

/***
    logMsg(LogScript, LogInfo, "reading bigstruct\n");
    timeIt(function() { 
        bigStruct = xmlRead('test.xml').object;
    });

    logMsg(LogScript, LogInfo, "writing bigstruct\n");
    timeIt(function() { 
        xmlWrite(bigStruct, 'bigstruct.xml') 
    });
***/
    var bigStruct = { hello: { there: 3, x: 5 } };
    logMsg(LogScript, LogInfo, "writing as JS\n");
    timeIt(function() { 
        dumpAsJScript(bigStruct, 'bigstruct.js');
    });

    logMsg(LogScript, LogInfo, "reading as JS\n");
    var bigstructFromJS;
    timeIt(function() { 
        bigstructFromJS = eval(FSOReadFromFile('bigstruct.js'));
    });
}

var done = false;

function runIE() {

    var ie = WScript.CreateObject("InternetExplorer.Application", "IE_");
    ie.Navigate("about:blank");
    ie.ToolBar = 0;
    ie.StatusBar = 0;
/**
    ie.Width = 800;
    ie.Height = 570;
    ie.Left = 0;
    ie.Top = 0;
**/
    ie.Visible = 1;
    
    while(ie.Busy) 
        WScript.Sleep(10);
    
    var doc = ie.Document;
    doc.open();
    doc.writeln("<html><body>");
    doc.writeln("this is a test");
    doc.writeln("</body></html>");
    doc.close();
    
    while (!done)
        WScript.Sleep(200);
}

function IE_OnQuit() {
    logMsg(LogScript, LogInfo, "IE Quit\n");
    done = true;
}

function d(x) {
    WScript.Echo(dump(x));
}

// TODO : Use sdObj for sd print (no dependancy on runCmd())
// TODO : Filter out white space diffs
// TODO : Show deletes that happened
// TODO : Show not just the last change to a line (need concept of 'close')

/**************************************************************************************/
function sdFileLogParse(fileName) {

    var ret = {};
    var file = FSOOpenTextFile(fileName, FSOForReading);
    var depotFile;
    var fileInfo;
    var rec;
    var i = 0;
    while(!file.AtEndOfStream) {
        var line = file.ReadLine();
        if (line.match(/^\.\.\. #(\d+) change (\d+) ([\w-]+) on (\S+\s+\S+) by (\S+)/)) {
            rec = {};
            rec.rev = parseInt(RegExp.$1);
            rec.change = parseInt(RegExp.$2);
            rec.action = RegExp.$3;
            rec.date = RegExp.$4;
            rec.user = RegExp.$5;
            // rec.depotFile = depotFile;
            fileInfo[rec.rev] = rec;
        }
        else if (line.match(/^\.\.\. \.\.\. (\S+\s*(\S*))\s+(.*)#(\d+)(,(\d+))?\s*$/)) {
            var subRec = {};
            subRec.how = RegExp.$1;
            subRec.fileName = RegExp.$3;
            subRec.srev = RegExp.$4;
            if (RegExp.$6)
                subRec.erev = RegExp.$6;
            else 
                subRec.erev = RegExp.$4
            
            if (RegExp.$2 == "from")
                rec.from = subRec;
            else {
                if (!rec.to)
                    rec.to = [];
                rec.to.push(subRec);
            }
        }
        else if (line.match(/^\/\/[^#]*$/)) {
            i++;
            if (i>= 100) {
                i = 0;
                WScript.StdOut.Write(".");
            }
            depotFile = line.toLowerCase();
            ret[depotFile] = fileInfo= {};
        }
        else {
            logMsg(LogSourceDepot, LogWarn, "sdFileLogParse: Could not parse line '", line, "'\n");
        }
    }

    file.Close();
    // return ret;

}

/*****************************************************************************/
/* Determine the integrations into and out of a directory hive
*/

function sdIntegrationSummary(path, srev, sdObj) {

    var depotPath = sdWhere(path);
    var integs = {};
    
        // I need the more recient revisions of a file to come
        // before older ones (for a given file)

    sdFileLog(path, sdObj, function(fileLog) {
        if (fileLog.from) {
            var fromBase = FIX;
            
            var integ = integs[fromBase];
            if (!integ) {
                integ = integs[fromBase] = {}
                integ.changes = {};
            }

            var changeInfo = integ.changes[fileLog.change];
            if (!changeInfo) {
                changeInfo = integ.changes[fileLog.change] = {};
                changeInfo.after = [];
            }
        
            for (var i in integ.changes) {
                if (parseInt(i) >= fileLog.change && !changeInfo.hasInteg) {
                    changeInfo.after.push(fileLog.from);
                    changeInfo.hasInteg = true;
                }
            }
        }
    });

    for (var fromBase in integs) {
        var integ = integs[fromBase];
        var changeNums = keys(integ.changes)
        changeNums.sort(function(x, y) { return parseInt(x) - parseInt(y) });
        for (var i in changeNums) {
            changeInfo = integ.changes[i];
            changeInfo.after
        }
    }
}


/**
            var integrationChange = integrationsChanges[fileLog.change];
            if (!integrationChange) {
                integrationChange = integrationsChanges[fileLog.change] = {};
                integrationChange.min = 0;
                integrationChange.max = 0x7FFFFFFF;
            }

***/

function toLower(inName, outName) {
    FSOWriteToFile(FSOReadFromFile(inName).toLowerCase(), outName);
}

function spin() {
    for(;;) {
        for(var i = 0; i < 10000000; i++) 
            if (i % 100000 == 0)
                WScript.Sleep(10);
    }
}

function stripMonBat(fileName) {

    var file = FSOOpenTextFile(fileName, FSOForReading);
    var out = "";
    while(!file.AtEndOfStream) {
        line = file.ReadLine(); // {
        if (line.match(/(#CMD)|(#ENTER)|(}\s*$)/))
            if (!line.match(/xcopy|cmd.*echo/)) 
                out += line + "\n";
    }

    /* { */ out = out.replace(/.*#ENTER.*setRetailBuild.*\n.*}.*\n/g, "");
    WScript.Echo(out);
    return 0;
}

/***************************************************************************/
function procPerfByPid(pid, wbemServices) {

    if (wbemServices == undefined)
        wbemServices = procGetWMI();

    var e = new Enumerator(wbemServices.ExecQuery("select * from Win32_PerfRawData_PerfProc_Process where IDProcess = " + pid));
    var ret = undefined;
    if (!e.atEnd())
        ret = e.item();

    logMsg(LogProc, LogInfo10000, "procPerfByPid(", pid, ", wbemServices) = ", (ret == undefined)?undefined:ret.Name, "\n");
    return ret;
}

function myDefrag() {

    var time = 3600;
    time = 10;
    cmd = "defrag /f /a c:";
    var run = runDo(cmd, runSetNoShell(runSetTimeout(time + 30, runSetLog())));
    
        // kill with kill (which is allows orderly shutdown)
    var killed = false;
    while (!runPoll(run)) {
        if (run.duration > time * 1000 && !killed) {
            logMsg(LogScript, LogInfo, "Reached timeout, killing defrager\n"); 
            runCmdToLog(ScriptDir + "\\kill.exe " + run.getPid())
        }
        WScript.Sleep(1000);
    }
}

function hexDump(file) {
    WScript.Echo(dumpStr(FSOReadFromFile(file)));
    return 0
}

function dirMunge() {

    var curDir = "";
    while (!WScript.StdIn.AtEndOfStream) {
        var line = WScript.StdIn.ReadLine();
        if (line.match(/Directory of (.*)/))
            curDir = RegExp.$1;
        else if (line.match(/\s*(\d+\/\d+\/\d+\s+.\d+:\d+\s+\S+)(\s+[\d,]+)\s+(.*)/))
            WScript.StdOut.WriteLine(RegExp.$1 + RegExp.$2 + " " + curDir + "\\" + RegExp.$3);
        else if (!(line.match(/^\s*$/) || 
                   line.match(/^\s*Volume/) || 
                   line.match(/<DIR>/) || 
                   line.match(/Dir.*bytes/) ||
                   line.match(/File.*bytes/)))
            WScript.StdOut.WriteLine("Unrecognized line " + line);
    }
}

/*****************************************************************************/
function parseTrace(trace, retAddrSet) {

    logMsg(LogScript, LogInfo, "parseTrace: trace len = ", trace.length, "\n");
    var ret = [];
    while(trace.match(/^([\da-zA-Z]+) ([\da-zA-Z]+) ([\da-zA-Z]+) ([\da-zA-Z]+) ([\da-zA-Z]+) (.*)/m)) {
        var newVal = { ebp: RegExp.$1, retAddr: RegExp.$2, funcName: RegExp.$6 };
        retAddrSet[newVal.retAddr] = newVal;
        ret.push(newVal);
        trace = RegExp.rightContext;
        // logMsg(LogScript, LogInfo, "trace len = ", trace.length, "\n");
    }
    // logMsg(LogScript, LogInfo, "Got Trace {\n", dump(ret), "\n}\n");
    return ret;
}

/*****************************************************************************/
function compareLop(dumpName) {

    var cdbExe = "C:\\nt.binaries.x86chk\\dbg\\files\\bin\\cdb.exe"
    var dumpDir = ".";
    if (dumpName.match(/(.*)\\[^\\]*$/))
        dumpDir = RegExp.$1;

    logMsg(LogScript, LogInfo, "running CDB, baseline to ", dumpName, ".baseline\n");
    runCmdToLog(cdbExe + " -c \" ~*kb500; q\" -y " + dumpDir + " -z " + dumpName,
                    runSetOutput(dumpName + ".baseline"))

    logMsg(LogScript, LogInfo, "running CDB, LOP mode to ", dumpName, ".lop\n");
    runCmd(cdbExe + " -c \"!stackdbg /c /Si /sl /sL; ~*kb500; q\" -y " + dumpDir + " -z " + dumpName,
                    runSetOutput(dumpName + ".lop"))

    logMsg(LogScript, LogInfo, "running CDB, LOP noisy mode to ", dumpName, ".lop.noisy\n");
    runCmd(cdbExe + " -c \"!stackdbg /c /Si /sl /sL 7; !sym noisy; ~*kb500; q\" -y " + dumpDir + " -z " + dumpName,
                    runSetOutput(dumpName + ".lop.noisy"))

    logMsg(LogScript, LogInfo, "fixing up output CDB\n");
    var fix = FSOReadFromFile(dumpName + ".lop")
    fix = fix.replace(/^(\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (.*)/mg, "$2 $6");
    FSOWriteToFile(fix, dumpName + ".lop.fix")

    logMsg(LogScript, LogInfo, "fixing up output CDB\n");
    var fix = FSOReadFromFile(dumpName + ".baseline")
    fix = fix.replace(/^(\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (\w\w\w\w\w\w\w\w) (.*)/mg, "$2 $6");
    FSOWriteToFile(fix,dumpName + ".baseline.fix") 

    runCmdToLog("windiff " + dumpName + ".baseline.fix" + " " + dumpName + ".lop.fix");
}

/*****************************************************************************/
function compareStacksBAD(fileName) {

    var baseLine = "";
    var test = ""
    var line;

    var file = FSOOpenTextFile(fileName, 1)
    while (!file.AtEndOfStream) {
        line = file.ReadLine();
        if (line.match(/ChildEBP RetAddr  Args to Child/))
            break;
    }

    while (!file.AtEndOfStream) {
        line = file.ReadLine();
        if (line.match(/Force LOP frame unwind - enabled/))
            break;
        baseLine += line + "\n";
    }

    while (!file.AtEndOfStream) {
        line = file.ReadLine();
        if (line.match(/ChildEBP RetAddr  Args to Child/))
            break;
    }

    var i = 0;
    while (!file.AtEndOfStream) {
        line = file.ReadLine();
        if (line.match(/^\s*LOP DONE/))
            break;
        test += line + "\n";
    }

    // logMsg(LogScript, LogInfo, "Got baseLine {", baseLine, "\n}\n");
    // logMsg(LogScript, LogInfo, "Got test {", test, "\n}\n");

    var baseLineSet = {};
    var baseLineParse = parseTrace(baseLine, baseLineSet);

    var testSet = {};
    var testParse = parseTrace(test, testSet);
    
    var out = "";
    for(var i = 0; i < baseLineParse.length; i++) {
        var entry = baseLineParse[i];
        out += entry.retAddr + " "  + entry.funcName + "\r\n";
    }
    FSOWriteToFile(out, "baseline.txt");

    var out = "";
    for(var i = 0; i < testParse.length; i++) {
        var entry = testParse[i];
        var funcName = entry.funcName;
        var baseLineEntry = baseLineSet[entry.retAddr];
        if (baseLineEntry)
            funcName = baseLineEntry.funcName;
        out += entry.retAddr + " "  + funcName + "\r\n";
    }
    FSOWriteToFile(out, "test.txt");

}

/*****************************************************************************/
function logger(cmd) {

    var cdb = "c:\\debuggers\\cdb.exe";

//      "!logexts.logc; " + 

    var cdbCmd = cdb + " -G -c \""  +
        "sxd av; " + 
        "sxd bpe; " + 
        "!logexts.logo e d; " + 
        "!logexts.logo d v; " + 
        "!logexts.logc d 16; " + 
        "!logexts.loge; " + 
        "g; q\" " + cmd

    runCmdToLog(cdbCmd);
}

/*****************************************************************************/
function attachLopToProc(patStr, num) {

    var wbemServices = procGetWMI();
    var event = wbemServices.ExecNotificationQuery("select * from Win32_ProcessTrace");
    var live = {};
    var pat = new RegExp(patStr, "i");
    if (num == undefined)
        num = 1;
    
    for(;;) {
        var obj = event.NextEvent();
        var id = obj.ProcessID;
        var name = "" + obj.ProcessName;
        if (live[id]) {
            logMsg(LogScript, LogInfo, "DIE  : ", name, " PID ", id, "\n");
            live[id] = undefined;
        }
        else {
            logMsg(LogScript, LogInfo, "START  : ", name, " PID ", id, "\n");
            live[id] = 1;

            if (name.match(pat)) {
                --num;
                if (num <= 0) {
                    var e = new Enumerator(wbemServices.ExecQuery("select * from Win32_Process where ProcessID = " + id));
                    if (!e.atEnd()) {
                        var ret = e.item();
                        logMsg(LogScript, LogInfo, "    CMD: ",  ret.CommandLine, "\n");
                    }

                    runCmdToLog("twexec /attach " + id);
                    break;
                }
            }
        }
    }
}

/*****************************************************************************/
function disasmDiff(fileName1, fileName2) {

    stripDisasmForDiff(fileName1);
    stripDisasmForDiff(fileName2);

    runCmdToLog("windiff " + fileName1 + ".disasmStrip " + fileName2 + ".disasmStrip");
}

/*****************************************************************************/
function stripDisasmForDiff(fileName, outFileName) {

    if (outFileName == undefined)
        outFileName = fileName + ".disasmStrip";

    var file = FSOOpenTextFile(fileName, FSOForReading);
    var outFile = FSOOpenTextFile(outFileName, FSOForWriting, true);

    while (!file.AtEndOfStream) {
        var line = file.ReadLine();
        
        line = line.replace(/^[\d\.:]+/, "");
        if (line.match(/^(\s*)[`\dA-Fa-f]+\s+[`\dA-Fa-f]+\s+((\S+).*)/)) {
            var space = RegExp.$1;
            line = space + RegExp.$2;
            instr = RegExp.$3;
            if (instr.match(/^j\S\S?$/i))
                line = space + instr;
        }
        line = line.replace(/_ni/g, "");

        line = line.replace(/\b(0x)?[\d`A-Fa-f][\d`A-Fa-f][\d`A-Fa-f][\d`A-Fa-f]+\b/gi, "XXXXXXXX");
        outFile.WriteLine(line);
    }
}

/*****************************************************************************/
function fixColumns(fileName, outFileName) {

    if (outFileName == undefined)
        outFileName = fileName + ".fix.txt";

    var file = FSOOpenTextFile(fileName, FSOForReading);
    var outFile = FSOOpenTextFile(outFileName, FSOForWriting, true);

    while (!file.AtEndOfStream) {
        var line = file.ReadLine();

        if (line.match(/^\s*([^()\s]+(\(.*\))?)\s+([\d\.%]+\s+[\d\.%]+\s+[\d\.%]+[\s\d\.%]*)/)) {
            var first  = RegExp.$1;
            var rest  = RegExp.$3;
            if (first.length > 75)
                first = first.substr(0, 75) + "...";

            line = padRight(first, 80) + " " + rest;
        }
        outFile.WriteLine(line);
    }
}

function setDebugger(exe, reset) {

    if (exe == undefined)
        throw Error(1, "Required parameter 'exe' not present");
    
    var key = "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\" + exe + "\\Debugger";
    if (reset)  {
        logMsg(LogScript, LogInfo, "Deleteing ", key, "\n");
        WshShell.RegDelete(key);
    }
    else  {
        var dbg = "cmd /c " + Env("_NTBINDIR") + "\\ddsuites\\tools\\windbg.cmd";
        logMsg(LogScript, LogInfo, "Writing ", key, " = ", dbg, "\n");
        WshShell.RegWrite (key, dbg);
    }
}


function getKernRateData(fileName, moduleName, asList) {

    logCall(LogScript, LogInfo, "getKernRateData", arguments);
    var file = FSOOpenTextFile(fileName, FSOForReading);
    var pat = new RegExp("Zoomed module " + moduleName, "i");
    logMsg(LogScript, LogInfo, "looking for pattern", pat, "\n");
    for(;;) {
        if (file.AtEndOfStream)  
            throw Error(1, "Could not find pattern " + pat);
        var line = file.ReadLine();
        if (line.match(new RegExp("Zoomed module.*" + moduleName, "i")))
            break;
    }

    var ret = asList?[]:{};

    var blank = false
    while (!file.AtEndOfStream) {
        var line = file.ReadLine();
        // logMsg(LogScript, LogInfo, "Got line ", line, "\n");
        if (line.match(/^((\S+)(\(.*\))?)\s+(\d+)\s+(\d+)\s+(\d+)\s+([\d\.]+)\s*%\s*([\d\.]+)/)) {
            // logMsg(LogScript, LogInfo, "line matches ", line, "\n");

            var name = RegExp.$1;
            var count = RegExp.$4;

            if (asList)
                ret.push({name:name, count:count});
            else {
                if (!ret[name]) 
                    ret[name] = count;
                else 
                    logMsg(LogScript, LogWarning, "Name ", name, " used twice!, ignoring second count \n");
            }
        }
        else if (line.match(/^\s*$/)) {
            if (blank)
                break;
            blank = true;
        }
        else 
            blank = false;
    }
    // logMsg(LogScript, LogInfo, "Name ", fileName, " data {", dump(ret), "}\n");
    return ret;
}

function kernRateRatios(fileName1, fileName2, module) {

    if (module == undefined)
        module = "System.Web.ni.dll";

    var data1 = getKernRateData(fileName1, module, true);
    var data2 = getKernRateData(fileName2, module);


    for(var i =0; i < data1.length; i++) {
        if (i > 40)
            break;
        var val = data1[i];
        var ratio = "INF";

        var count1 = data2[val.name];
        if (count1)
            ratio = (val.count / count1).toFixed(2);
        else
            count1 = 0;

        logMsg(LogScript, LogInfo, padRight(val.name, 60), " File1=", padLeft(val.count, 5), " File2=", padLeft(count1, 5), " ratio ", ratio, "\n");
    }
}

/*****************************************************************************/
function wtAccum(fileName, outFileName) {

    if (outFileName == undefined)
        outFileName = fileName + ".accum.txt";

    var file = FSOOpenTextFile(fileName, FSOForReading);
    var outFile = FSOOpenTextFile(outFileName, FSOForWriting, true);
    var lines = [];

    while (!file.AtEndOfStream) {
        var line = file.ReadLine();

        if (line.match(/^(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/)) {
            var total = parseInt(RegExp.$2) * parseInt(RegExp.$5);
            line = padLeft(total, 7) + " " + line + "\n";
            lines.push(line);
        }
    }
    
    lines.sort();
    outFile.WriteLine(lines.join(""));
}

/*****************************************************************************/
function asmByPhase(fileName) {

    var file = FSOOpenTextFile(fileName, FSOForReading);
    var numFiles = 0;
    var lines = [];
    var name = undefined;

    while (!file.AtEndOfStream) {
        var line = file.ReadLine();

        if (line.match(/IR after (.*)\(\S+\s+(\S+).*\)/i)) {
            var newName = RegExp.$2;            
            flush(fileName, numFiles, name, lines);
            numFiles++;
            name = newName;
            lines = [];
        }
        lines.push(line);
    }
    flush(fileName, numFiles, name, lines);
}

var lastFlushOutName;
/*****************************************************************************/
function flush(fileName, numFiles, name, lines) {

    if (name == undefined)
        return;
    var outName = fileName + "." + numFiles + "." + name + ".txt";
    logMsg(LogScript, LogInfo, "Writing ", lines.length, " lines to ", outName, "\n");
    FSOWriteToFile(lines.join("\n"), outName);

    if (lastFlushOutName) 
        runCmd("windiff " + lastFlushOutName + " " + outName);
    lastFlushOutName = outName;
}

/*****************************************************************************/
function cdbWriteBarrierCalls32(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    cdbDo(cdb, "bu clr!SystemDomain::Init");
    cdbDo(cdb, "g");            
    cdbLoadStrike(cdb);
    cdbDo(cdb, "bc *");         

    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[0]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[1]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[2]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[3]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[4]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[5]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[6]) 10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[7]) 10000000");

/**
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[0])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[1])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[2])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[3])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[4])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[5])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[6])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[7])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[8])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[8])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[10])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[11])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[12])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[13])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[14])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[15])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[16])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[17])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[18])    10000000");

    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicFCall[0])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[1])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[3])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[4])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[5])    10000000");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.DynamicHCall[6])    10000000");
***/

    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEAX 10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEBX 10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierECX 10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEBP 10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierESI 10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEDI 10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrier 10000000");
    cdbDo(cdb, "bp clr!JIT_ByRefWriteBarrier 10000000");

    cdbDo(cdb, "bp clr!JIT_ChkCastClass 10000000");
    cdbDo(cdb, "bp clr!JIT_ChkCastClassSpecial 10000000");
    cdbDo(cdb, "bp clr!JIT_ChkCastInterface 10000000");

    cdbDo(cdb, "bp clr!JIT_IsInstanceOfClass 10000000");
    cdbDo(cdb, "bp clr!JIT_IsInstanceOfClassSpecial 10000000");
    cdbDo(cdb, "bp clr!JIT_IsInstanceOfInterface 10000000");

    cdbDo(cdb, "bp clr!JIT_NewArr1 10000000");
    cdbDo(cdb, "bp clr!JIT_NewFast 10000000");

    cdbDo(cdb, "bp clr!JIT_MonEnter 10000000");
    cdbDo(cdb, "bp clr!JIT_MonExit 10000000");

    cdbDo(cdb, "bp clr!JIT_SetField32 10000000");
    cdbDo(cdb, "bp clr!JIT_SetFieldObj 10000000");
    cdbDo(cdb, "bp clr!JIT_Stelem_Ref 10000000");
    cdbDo(cdb, "bp clr!JIT_StrCns 10000000");

    cdbDo(cdb, "bp clr!AllocateArrayEx 10000000");

    // Need MonEnter, MonExit

    cdbDo(cdb, "bl")

    var HelperNames = [
        "CORINFO_HELP_DBL2INT", 
        "CORINFO_HELP_DBL2LNG", 
        "CORINFO_HELP_DBL2UINT", 
        "CORINFO_HELP_NEWSFAST", 
        "CORINFO_HELP_NEWSFAST_ALIGN8", 
        "CORINFO_HELP_NEWSFAST_CHKRESTORE", 
        "CORINFO_HELP_NEWARR_1_OBJ", 
        "CORINFO_HELP_NEWARR_1_VC", 
        "CORINFO_HELP_NEWARR_1_ALIGN8", 
        "CORINFO_HELP_BOX", 
        "CORINFO_HELP_GETSHARED_GCSTATIC_BASE", 
        "CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE", 
        "CORINFO_HELP_GETSHARED_GCSTATIC_BASE_NOCTOR", 
        "CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE_NOCTOR", 
        "CORINFO_HELP_PROF_FCN_ENTER", 
        "CORINFO_HELP_PROF_FCN_LEAVE", 
        "CORINFO_HELP_PROF_FCN_TAILCALL", 
        "CORINFO_HELP_PINVOKE_CALLI", 
        "CORINFO_HELP_GET_THREAD"
    ];

    cdbDo(cdb, "g");            
    var bl = cdbDo(cdb, "bl").split(/\r*\n/);
    var writeBarrier = 0;
    var writeBarrierChecked = 0;
    for(var i = 0; i < bl.length; i++) {
        var line = bl[i];
        // logMsg(LogScript, LogInfo, "Line ", line, "\n");

        if (line.match(/^\s*\d+\s+\w+\s+\S+\s+(\S+)\s+\((\S+)\)\s+\S+\s+(.*\S)/)) {
            var cnt = parseInt(RegExp.$2, 16) - parseInt(RegExp.$1, 16);
            var symbol = RegExp.$3;
            logMsg(LogScript, LogInfo, "cnt for ", symbol, " = ", cnt, "\n");
            if (symbol.match(/JIT_CheckedWriteBarrier/))
                writeBarrierChecked  += cnt;
            else if (symbol.match(/JIT_Writeable_Thunks_Buf/))
                writeBarrier += cnt;
        }
        else 
            logMsg(LogScript, LogError, "Could not parse line ", line, "\n");
    }
    
    logMsg(LogScript, LogInfo, "Write Barrier Calls = ", writeBarrier, "\n"); 
    logMsg(LogScript, LogInfo, "Checked Write Barrier Calls = ", writeBarrierChecked, "\n"); 
    logMsg(LogScript, LogInfo, "Checked / Unchecked Write Barrier Calls = ", writeBarrierChecked + writeBarrier, "\n"); 
}

/*****************************************************************************/
function cdbWriteBarrierCalls64(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    cdbDo(cdb, "bu clr!SystemDomain::Init");
    cdbDo(cdb, "g");            
    cdbDo(cdb, "bc *");         

    cdbDo(cdb, "bp clr!JIT_WriteBarrier_Fast   10000000");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrier 10000000");
    cdbDo(cdb, "bp clr!JIT_ByRefWriteBarrier   10000000");

    cdbDo(cdb, "bp clr!JIT_BoxFastMP_InlineGetThread 10000000");
    cdbDo(cdb, "bp clr!JIT_ChkCastClassSpecial 10000000");

    cdbDo(cdb, "bp clr!JIT_GetSharedGCStaticBase_InlineGetAppDomain 10000000");
    cdbDo(cdb, "bp clr!JIT_GetSharedGCStaticBaseNoCtor_InlineGetAppDomain 10000000");
    cdbDo(cdb, "bp clr!JIT_GetSharedNonGCStaticBaseNoCtor_InlineGetAppDomain 10000000");
    cdbDo(cdb, "bp clr!JIT_GetSharedNonGCStaticBase_InlineGetAppDomain 10000000");

    cdbDo(cdb, "bp clr!JIT_IsInstanceOfClass 10000000");
    cdbDo(cdb, "bp clr!JIT_IsInstanceOfInterface 10000000");
    cdbDo(cdb, "bp clr!JIT_IsInstanceOfClassSpecial 10000000");

    cdbDo(cdb, "bp clr!JIT_Stelem_Ref 10000000");

    cdbDo(cdb, "bp clr!JIT_TrialAllocSFastMP_InlineGetThread 10000000");           // NEWFAST
    cdbDo(cdb, "bp clr!JIT_TrialAllocSFastChkRestoreMP_InlineGetThread 10000000"); // NEWSFAST_CHKRESTORE
    cdbDo(cdb, "bp clr!JIT_NewFast 10000000");         // Not very fast
    cdbDo(cdb, "bp clr!JIT_NewCrossContext 10000000"); // Very slow

    cdbDo(cdb, "bp clr!JIT_NewArr1VC_MP_InlineGetThread 10000000");    // NEWARR_1_VC
    cdbDo(cdb, "bp clr!JIT_NewArr1 10000000");

    cdbDo(cdb, "bp clr!JIT_SetField32 10000000");
    cdbDo(cdb, "bp clr!JIT_SetFieldObj 10000000");

    cdbDo(cdb, "bp clr!JIT_MonExitWorker_InlineGetThread 10000000");
    cdbDo(cdb, "bp clr!JIT_MonExitStatic_InlineGetThread 10000000");
    cdbDo(cdb, "bp clr!JIT_MonEnterWorker_InlineGetThread 10000000");
    cdbDo(cdb, "bp clr!JIT_MonEnterStatic_InlineGetThread 10000000");

    cdbDo(cdb, "bl");

    cdbDo(cdb, "g");            

    var bl = cdbDo(cdb, "bl").split(/\r*\n/);
    var writeBarrier = 0;
    var writeBarrierChecked = 0;
    for(var i = 0; i < bl.length; i++) {
        var line = bl[i];
        // logMsg(LogScript, LogInfo, "Line ", line, "\n");

        if (line.match(/^\s*\d+\s+\w+\s+\S+\s+(\S+)\s+\((\S+)\)\s+\S+\s+(.*\S)/)) {
            var cnt = parseInt(RegExp.$2, 16) - parseInt(RegExp.$1, 16);
            var symbol = RegExp.$3;
            logMsg(LogScript, LogInfo, "cnt for ", symbol, " = ", cnt, "\n");

            if (symbol.match(/JIT_CheckedWriteBarrier/))
                writeBarrierChecked  += cnt;
            else if (symbol.match(/JIT_WriteBarrier_Fast/))
                writeBarrier += cnt;
        }

        else 
            logMsg(LogScript, LogError, "Could not parse line ", line, "\n");
    }
    logMsg(LogScript, LogInfo, "Write Barrier Calls = ", writeBarrier, "\n"); 
    logMsg(LogScript, LogInfo, "Checked Write Barrier Calls = ", writeBarrierChecked, "\n"); 
    logMsg(LogScript, LogInfo, "Checked / Unchecked Write Barrier Calls = ", writeBarrierChecked + writeBarrier, "\n"); 
}

/*****************************************************************************/
function cdbWriteBarrierCalls32WithCaller(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    cdbDo(cdb, "bu clr!SystemDomain::Init");
    cdbDo(cdb, "g");            
    cdbLoadStrike(cdb);
    cdbDo(cdb, "bc *");         

    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[0]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[1]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[2]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[3]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[4]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[5]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[6]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp @@(&JIT_Writeable_Thunks_Buf.WriteBarrierReg[7]) \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEAX \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEBX \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierECX \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEBP \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierESI \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrierEDI \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_ByRefWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bl")

    var callerName = "UNKNOWN";
    cdbDo(cdb, "g", undefined, undefined, function(line) {
        
        if (line.match(/^DONE/)) {
            WScript.StdOut.WriteLine(callerName);
            callerName = "UNKNOWN";
        }
        else if (line.match(/Method Name:\s+(.*)/i)) {
            callerName = RegExp.$1;
        }
        else if (line.match(/Evaluate expression:  *\S+  *= *(\S+)$/)) {
            callerName = "ADDR(" + RegExp.$1 + ")";
        }
        else if (!line.match(/^\w+:/) && !line.match(/Failed to request/)) {
            WScript.StdErr.WriteLine("Output: " + line);
        }
        });

    cdbDo(cdb, "q");
}

/*****************************************************************************/
function cdbWriteBarrierCalls64WithCaller(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    cdbDo(cdb, "bu clr!SystemDomain::Init");
    cdbDo(cdb, "g");            
    cdbLoadStrike(cdb);
    cdbDo(cdb, "bc *");         

    cdbDo(cdb, "bp clr!JIT_WriteBarrier_Fast  \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_ByRefWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bl")

    var callerName = "UNKNOWN";
    cdbDo(cdb, "g", undefined, undefined, function(line) {
        
        if (line.match(/^DONE/)) {
            WScript.StdOut.WriteLine(callerName);
            callerName = "UNKNOWN";
        }
        else if (line.match(/Method Name:\s+(.*)/i)) {
            callerName = RegExp.$1;
        }
        else if (line.match(/Evaluate expression:  *\S+  *= *(\S+)$/)) {
            callerName = "ADDR(" + RegExp.$1 + ")";
        }
        else if (!line.match(/^\w+:/) && !line.match(/Failed to request/)) {
            WScript.StdErr.WriteLine("Output: " + line);
        }
        });

    
    logMsg(LogScript, LogInfo, "Done, pausing to see output\n");
    WScript.Sleep(3000);
    cdbDo(cdb, "q");
}


/*****************************************************************************/
function cdbCheckedWriteBarrierCalls32WithCaller(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    cdbDo(cdb, "bu clr!SystemDomain::Init");
    cdbDo(cdb, "g");            
    cdbLoadStrike(cdb);
    cdbDo(cdb, "bc *");         

    cdbDo(cdb, "bp JIT_CheckedWriteBarrierEAX \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp JIT_CheckedWriteBarrierEBX \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp JIT_CheckedWriteBarrierECX \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp JIT_CheckedWriteBarrierEBP \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp JIT_CheckedWriteBarrierESI \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp JIT_CheckedWriteBarrierEDI \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_ByRefWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp JIT_CheckedWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bl")

    var callerName = "UNKNOWN";
    cdbDo(cdb, "g", undefined, undefined, function(line) {
        
        if (line.match(/^DONE/)) {
            WScript.StdOut.WriteLine(callerName);
            callerName = "UNKNOWN";
        }
        else if (line.match(/Method Name:\s+(.*)/i)) {
            callerName = RegExp.$1;
        }
        else if (line.match(/Evaluate expression:  *\S+  *= *(\S+)$/)) {
            callerName = "ADDR(" + RegExp.$1 + ")";
        }
        else if (!line.match(/^\w+:/) && !line.match(/Failed to request/)) {
            WScript.StdErr.WriteLine("Output: " + line);
        }
        });

    cdbDo(cdb, "q");
}

/*****************************************************************************/
function cdbCheckedWriteBarrierCalls64WithCaller(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    cdbDo(cdb, "bu clr!SystemDomain::Init");
    cdbDo(cdb, "g");            
    cdbLoadStrike(cdb);
    cdbDo(cdb, "bc *");         

    cdbDo(cdb, "bp clr!JIT_CheckedWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bp clr!JIT_ByRefWriteBarrier \"? poi(@esp); !IP2MD poi(@esp);.echo DONE;g\"");
    cdbDo(cdb, "bl")

    var callerName = "UNKNOWN";
    cdbDo(cdb, "g", undefined, undefined, function(line) {
        
        if (line.match(/^DONE/)) {
            WScript.StdOut.WriteLine(callerName);
            callerName = "UNKNOWN";
        }
        else if (line.match(/Method Name:\s+(.*)/i)) {
            callerName = RegExp.$1;
        }
        else if (line.match(/Evaluate expression:  *\S+  *= *(\S+)$/)) {
            callerName = "ADDR(" + RegExp.$1 + ")";
        }
        else if (!line.match(/^\w+:/) && !line.match(/Failed to request/)) {
            WScript.StdErr.WriteLine("Output: " + line);
        }
        });

    
    logMsg(LogScript, LogInfo, "Done, pausing to see output\n");
    WScript.Sleep(3000);
    cdbDo(cdb, "q");
}

/*****************************************************************************/
/* print statistics on GC after each GC */

function cdbGCStats(cmdLine) {

    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present")

    logMsg(LogScript, LogInfo, "Running comand '", cmdLine, "'\n");
    var cdb = cdbNew(cmdLine);  // launch the command

    var whatToDo = "gu; dt Perf_GC @@(&clr!PerfCounters::m_pPrivatePerf->m_GC) -b; g";

    cdbDo(cdb, "bu clr!SVR::GCHeap::UpdatePreGCCounters \"bd 0;.echo Before 1st GC;" + whatToDo + "\"");
    cdbDo(cdb, "bu clr!WKS::GCHeap::UpdatePreGCCounters \"bd 1;.echo Before 1st GC;" + whatToDo + "\"");

    cdbDo(cdb, "bu clr!SVR::GCHeap::UpdatePostGCCounters \".echo AFTER GC;" + whatToDo + "\"");
    cdbDo(cdb, "bu clr!WKS::GCHeap::UpdatePostGCCounters \".echo AFTER GC;" + whatToDo + "\"");
    cdbDo(cdb, "bl");
    cdbDo(cdb, "n 10");

    cdbDo(cdb, "g", undefined, undefined, function(line) {
        logMsg(LogScript, LogInfo, "Got: ", line, "\n");
    });
    
    logMsg(LogScript, LogInfo, "Done\n");
    cdbDo(cdb, "q");
}

/*****************************************************************************/
/*  detabs a infileName to outFileName with tabStop 'tabStop' */

function detabFile(inFileName, outFileName, tabStop) {

    if (tabStop == undefined)
        tabStop = 4;

    if (outFileName == undefined)
        outFileName = inFileName + ".detab.txt";

    var inFile = FSOOpenTextFile(inFileName, FSOForReading);
    var outFile = FSOOpenTextFile(outFileName, FSOForWriting, true);

    var lineNum = 0;
    while (!inFile.AtEndOfStream) {
        lineNum++;
        var line = inFile.ReadLine();
        var lineParts = line.split("\t");
        var outLine = "";
        var i = 0; 
        outLine += lineParts[i++];
        while(i < lineParts.length) {
            do {
                outLine += " ";
            } while (outLine.length % tabStop != 0);
            outLine += lineParts[i];
            i++;
        }
        outFile.WriteLine(outLine);
    }

    inFile.Close();
    outFile.Close();
    return 0;
}

/*****************************************************************************/
/* detabs a file in place, naming the original 'tabbled' */

function detab(fileName, tabStop) {

    var origName = fileName + ".tabbed";
    FSOCopyFile(fileName, origName, true);
    detabFile(origName, fileName, tabStop);
    return 0;
}

/*****************************************************************************/
function detabCheckedOut(dir, tabStop) {

    if (dir == undefined)
        dir = ".";

    var opened = runCmd("sd opened -l " + dir + "\\...").output.split("\n");
    for(var i = 0; i < opened.length; i++) {
        if (opened[i].match(/^(\w:.*)?#\d+/))
            var fileName = RegExp.$1;
            logMsg(LogScript, LogInfo, "Detabbing ", fileName, "\n");
            if (FSOFileExists(fileName))
                detab(fileName);
        }
    return 0;
}


