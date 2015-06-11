/*********************************************************************************/
/*                                 dump.js                                        */
/*********************************************************************************/

/* Utilities for creating good dumps.  In particular, in addition to the dump:

    1) You want all the pdbs, so that the dump is useful
    2) mscordac* and sos.dll so that you can debug managed.
    3) You want to dump the whole tree of processes 
    4) You want it to do the initial triage.  
    
    dumpProc does all of this.  Running:

        runjs dumpProc 3435

    will create a directory with all the infomation about the process chain
    starting at process 3435.  
*/

/*********************************************************************************/
var dumpModuleDefined = 1;         // Indicate that this module exists

if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");

var LogDump = logNewFacility("dump");
// logSetFacilityLevel(LogDump, LogInfo100000);

if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");

// Sequence number of dump
var dumpSeq = 0;

// We can dump other processes that may be interesting, such as asp.net workers or the ngen service
var dumpSeqFirstOtherInterestingProcess = undefined;

/**************************************************************************/
/* finds a path to CDB */
function _dumpGetCDBDir(arch)
{
    if (arch == undefined)
        arch = Env("PROCESSOR_ARCHITECTURE");

    // First see if it is in <arch>\cdb.exe relative to Script
    var path = ScriptDir + "\\" + arch;
    if (FSOFileExists(path + "\\cdb.exe")) {
        logMsg(LogDump, LogInfo10, "Found CDB at " + path  + "\n");
	return path;
    }

    // Get it from the path (trying to find the right architecture)
    var cdbPath = findOnPath("cdb");
    if (cdbPath != null && cdbPath.match(/(.*)\\(.+)\\/))
    {
	// See if we have the right architecture
	path = RegExp.$1 + "\\" + arch;
        if (FSOFileExists(path + "\\cdb.exe")) {
            logMsg(LogDump, LogInfo10, "Found CDB at " + path  + "\n");
            return path;
        }
    }

    // See if we have it checked in (only works if we are in a CLREnv or Razzle ...)
    var path = Env("_NTDRIVE") + Env("_NTROOT") + "\\ddsuites\\src\\clr\\" + arch + "\\Common\\Tools\\";
    if (arch == "x86")
        path += "i386";
    else
        path += arch;
    path += "\\Debuggers";
    if (FSOFileExists(path + "\\cdb.exe")) {
	logMsg(LogDump, LogInfo10, "Found CDB at " + path  + "\n");
	return path;
    }
    throw Error(1, "Could not find CDB.exe for arch " + arch);
}

/**************************************************************************/
/* picks a good name to name the dump.  This name should be something that
   is related to the scenario (thus dumps with the same name are likely to
   be related, and meaningful to humands.  What I do here is 'compress' the
   command line to form a name, and then put the date on the end to 'uniqify'
   it (the time is also useful information) 
*/
function _dumpGetName(commandLine) {
    logMsg(LogDump, LogInfo100, "_dumpGetName(", commandLine, ")\n");
    var name = commandLine;

    if (name) {
            // compress every file name on the command line to the last component
        name = name.replace(/".*?([^\\ ]*)"/g, "$1");    // remove ""
        name = name.replace(/\.exe/g, "");                // remove .exe suffixes
        name = name.replace(/\S*\\([^\\ ]*)/g, "$1");    // only use the last part of the path
        name = name.replace(/\s*[-\/]/g, "-");            // eliminate spaces before - or /
        name = name.replace(/\s+/g, "_");                // replace spaces with _
        name = name.replace(/[^\w\d-]/g, "");            // get rid of any unidentifed chars ($#...)
        name = name.substr(0, 32);                        // limit to 30 chars
        name = "." + name;
    }
    else 
        name = "";

    var now = new Date();
    name += "." + (now.getMonth()+1) + "-" + now.getDate() + "." + now.getHours() + "." + now.getMinutes() + "." + now.getSeconds();
    name = "Dump" + name;

    logMsg(LogDump, LogInfo100, "_dumpGetName() = ", name, "\n");
    return name;
}

/**************************************************************************/
/* _dumpProcInfo takes a 'whatDump' (either -p <pid> or -z <dumpFile>) and a 
   path name 'targetPrefix', and creates a dump file <targetPrefix>.dmp
   and <targetPrefix>.cdb.txt.  It also copies over all PDB necessary
   as well as sos.dll, mscordac* and creates a batch file for launching
   windbg called <targetPrefix>.windbg.cmd.
*/
function _dumpProcInfo(whatToDump, targetPrefix)
{
    logMsg(LogDump, LogInfo100, "_dumpProcInfo(", whatToDump, ", ", targetPrefix, ", runTemplate) {\n");

    if (!targetPrefix.match(/(.*)\\([^\\]*)$/))
        throw Error(1, "Could not parse path " + targetPrefix);
    var outDir = RegExp.$1;
    var dumpFileName = RegExp.$2 + ".dmp";

    var arch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    var cdbTList = ".echo CMD-START .tlist /v; .tlist /v; .echo CMD-END;";
    var cdbDmp   = ".echo CMD-START .dump /ma; .dump /ma " + targetPrefix + ".dmp; .echo CMD-END;";
    var isDump = false;
    if (whatToDump.match(/^\s*-z *(.*)/))
    {
        var file = RegExp.$1;
        cdbTList = "";
        cdbDmp = "";
        isDump = true;
        logMsg(LogDump, LogInfo, "copying ", file, " to ", targetPrefix + ".dmp", "\n");
        FSOCopyFile(file, targetPrefix + ".dmp", true);
    }
    else
    {
        // There seems to be no easy way from script to tell if a process is running
        // under the WOW.  find out by attaching the debugger, and looking for the wow64win module
        // if the process is running under the wow, use the wow CDB instead    
        if (arch != "x86")
        {
            var cdbRun = runCmd(_dumpGetCDBDir() + "\\cdb.exe " + whatToDump + " -c \"lm;q\"", 
                null,
                runSetTimeout(60, 
                runSetNoThrow()));
            if (cdbRun.output.match(/\s+wow64win\s+\(.*\)/i))
            {
                logMsg(LogDump, LogInfo, "Detected that dumpProc is NOT running under the wow, but the process\n");
                logMsg(LogDump, LogInfo, "being debug is running under the wow.  Dumping with 32 bit cdb.\n");
                arch = "x86";
            }
        }
    }
    
    //dumping stress log (!dumplog)takes a lot of time (5 minutes or so) if it is full of exceptions
    // since at each exception we write the stack in the stress log.
    var cdbRunOpts = runSetTimeout(600, null);
    FSOCreatePath(outDir);
    var cmd = _dumpGetCDBDir(arch) + "\\cdb.exe " + whatToDump + " -c \"" + 
        ".enable_unicode 1; " +
        ".lines -e ; " +
        cdbDmp + 
        ".echo CMD-START ~*kp 50; ~*kp 50; .echo CMD-END;" +
        ".echo CMD-START ld; ld clr; .echo CMD-END;" +        // we always want clr even if not on the stack
        ".echo CMD-START lm v;lm v;.echo CMD-END;" +
        cdbTList + 
        ".echo CMD-START !peb; !peb ; .echo CMD-END;" +
        ".echo CMD-START !DumpLog; .loadby sos clr; !DumpLog " + targetPrefix + ".stressLog.txt;.echo CMD-END;" +
        "q\"";

    var cdbOutName = targetPrefix + ".cdb.txt";
    logMsg(LogDump, LogInfo, "Generating dump, log: ", cdbOutName, "\n");
    runCmd(cmd, runSetOutput(cdbOutName, cdbRunOpts));

    if (!FSOFileExists(cdbOutName))
        throw new Error(1, "Error, cdb command did not produce output!");

        // Parse the output into the output of individual commands and place in 'cmdOut'
    var cdbOut = FSOReadFromFile(cdbOutName);

    if (cdbOut.match(/(Unable to examine process .*)/))
    {
      var msg = RegExp.$1
      if (whatToDump.match(/-p *(\d+)/))
      {
        if (procGetByPid(RegExp.$1) == undefined)
          FSOWriteToFile(cdbOut + "\r\n" + 
            "The process ID was not found after the attach attempt.  Thus a likely\r\n" +
            "cause of the failure is that the process exited naturally before the\r\n" + 
            "debugger had time to attach to it.\r\n",
            cdbOutName);
      }
          throw new Error(1, "Error, cdb failed: " + msg + " See " + cdbOutName + " for more.");
    }
    
    if (!cdbOut.match(/Reading initial command/))
        throw new Error(1, "Error, cdb failed: See " + cdbOutName + " for more");

    var cmdOut = {};
    while (cdbOut.match(/\nCMD-START\s+(.+)\n((.|\n)*?\n)\s*\n*CMD-END/m))
    {
        cmdOut[RegExp.$1] = RegExp.$2;
        cdbOut = RegExp.rightContext;
    }

    logMsg(LogDump, LogInfo, "Copying pdbs...\n");
    var pdbs;
    var lmStr = cmdOut["lm v"];
    if (lmStr == undefined)
        throw new Error(1, "Error parsing " + cdbOutName + ": no lm v output");

    var modInfo = _dumpParseModules(lmStr);
    var clrInfo = undefined;
    
    // Copy exactly what is needed for WinDbg debugging.
    for (var i in modInfo)
    {
        var mod = modInfo[i];
        if (mod.name.match(/clr/i))
            clrInfo = mod;

        if ((mod.path.match(/^[:\w]*\\win\w+\\sys[^\\]+\\[^\\]+$/i) && !mod.name.match(/mscoree/i)) ||
             mod.path.match(/^[:\w]*\\win\w+\\WinSxS\\/i))
        {
            logMsg(LogDump, LogInfo1000, "Skipping system module ", mod.path, "\n");
        }
        else if (mod.pdbName && FSOFileExists(mod.pdbName))
        {
            logMsg(LogDump, LogInfo, "Copying PDB: ", mod.pdbName, "\n"); 
            try { FSOCopyFile(mod.pdbName, outDir + "\\", true); } catch(e) {}
        }
    }

    // If this dump is for CLR, parse out the clrDir and clrPdbPath.  (Since we dump multiple processes,
    // this is not the common case.)
    if (clrInfo != undefined)
    {
        var clrDir = undefined;
        if (!isDump && FSOFileExists(clrInfo.path))
        {
            clrDir = clrInfo.path.match(/(.*)\\[^\\]*$/)[1];
        }
        else
        {
            logMsg(LogDump, LogInfo, "Looking for clr near the PDB\n");
 
            // look for the dac dll somewhere up the hierarchy from the PDB.  
            var clrPdbPath = clrInfo.pdbName;
            if (clrPdbPath)
                while(clrPdbPath.match(/^(.*)\\.*$/))
                {
                    clrPdbPath = RegExp.$1
                    var candidate = RegExp.$1 + "\\clr.dll";
                    logMsg(LogDump, LogInfo100, "Looking at ", candidate, "\n");
                    if (FSOFileExists(candidate))
                    {
                        clrDir = clrPdbPath;
                        break;
                    }
                }
        }

        if (clrDir)
        {
            logMsg(LogDump, LogInfo, "Found clr in dir ", clrDir, "\n");
            var dac = clrDir + "\\mscordacwks.dll";
            if (FSOFileExists(dac))
            {
                logMsg(LogDump, LogInfo, "Copying: ", dac, "\n");
                try { FSOCopyFile(dac, outDir + "\\mscordacwks.dll", true); } catch(e) {}
                
            }
            else
            {
                logMsg(LogDump, LogInfo, "WARNING Could not find ", dac, "\n");
            }

            var sos = clrDir + "\\sos.dll";
            if (FSOFileExists(sos))
            {
                logMsg(LogDump, LogInfo, "Copying: ", sos, "\n");
                try { FSOCopyFile(sos, outDir + "\\", true); } catch(e) {}
            }
            else
            {
                logMsg(LogDump, LogInfo, "WARNING Could not find ", sos, "\n");
            }
        }
    }

    // Create the script that runs windbg
    if (FSOFileExists(targetPrefix + ".dmp"))
    {
        var script = 
            "setlocal\r\n" +
            "set path=%path%;%~dp0;\r\n";

        // Try to match the architecture of the target process, which can always be
        // done if the orginal process was X86 otherwise use the native arch
        if (arch == "x86")
        {
            script += "set path=" + _dumpGetCDBDir("x86") + ";%path%\r\n";
        }
        else
        {
            script += 
                "set ARCH=%PROCESSOR_ARCHITEW6432%\r\n" +
                "if '%ARCH%' == '' set ARCH=%PROCESSOR_ARCHITECTURE%\r\n" +
                "set path=" + _dumpGetCDBDir("%ARCH%") + ";%path%\r\n";
        }

        script += 
            "set workSpace=" + _dumpGetCDBDir("x86") + "\\windbg.default.wew\r\n" +
            "set workSpaceArg= \r\n" +
            "if EXIST %workSpace% set workSpaceArg=-WF %workSpace%\r\n" + 
            "windbg /Q %workSpaceArg% /y \"%~dp0;srv*\\\\symbols\\symbols\" /srcpath \"%_NTBINDIR%\\ndp;\\\\CLRMain\\public\\Drops\\puclr\\latestSrc\" /z \"%~dp0\\" + dumpFileName + "\"" + 
                " /c \".prefer_dml 1 ; .enable_unicode 1 ; .cordll -u -lp %~dp0 ; .load %~dp0\\sos.dll ; !sos.threads ; .echo ******************** SRCPATH set to files that may not be right ****************\"\r\n";
        FSOWriteToFile(script, targetPrefix + ".runWindbg.cmd");
    }
    else
    {
        logMsg(LogDump, LogInfo, "WARNING Dump file not created!\n");
    }

    logMsg(LogDump, LogInfo100, "} _dumpProcInfo()\n");
}

/**************************************************************************/
function _dumpChildren(pid, outDir, procForPid, filter) {

    logMsg(LogDump, LogInfo100, "_dumpChildren(", pid, ", ", outDir, ") {\n");
    
    if (procForPid == undefined) 
        procForPid = procGetByPid(pid);

    if (filter == undefined)
        filter = false;     // don't filter out processes for the parent
        
    var children = procGetChildren(pid);
    for (var i = 0; i < children.length; i++)  {
        var child = children[i];
        // Recursively dump the children, but filter out uninteresting processes
        _dumpChildren(child.ProcessID, outDir, child, true);
    }

    // Some child processes may not be worth dumping
    var exePath = procForPid ? procForPid.ExecutablePath : "";
    if (filter &&
        exePath && 
        exePath.match(/\\((dwwin.exe)|(dw20.exe)|(windbg.exe)|(ntsd.exe)|(vsjit.exe)|(VsJITDebugger.exe)|(cmd.exe)|(perl.exe)|(cscript.exe))$/i)) 
    {
        logMsg(LogDump, LogInfo1000, "skipping uninteresting process ", pid, " exe = ", exePath, "\n");
    }
    else if (FSOGetFilePattern(outDir, new RegExp("Dump\\.\\d+\\.Proc\\." + pid + ".*")).length != 0){
        logMsg(LogDump, LogInfo1000, "process ", pid, " exe = ", exePath, " has already been dumped, skipping\n");
    }
    else{        
        targetPrefix = outDir + "\\Dump." + (dumpSeq++) + ".Proc." + pid;
        _dumpProcInfo("-pv -p " + pid, targetPrefix);                    
    }

    logMsg(LogDump, LogInfo100, "} _dumpChildren()\n");
}

/**************************************************************************/
// Stream class to get open files with.
function _Stream()
{
  this._text = "";
}

_Stream.prototype._text;

_Stream.prototype.Write = function (text)
{
  this._text += text;
}

_Stream.prototype.ReadLines = function ()
{
  return this._text.split("\n");
}


/**************************************************************************/
// Find open files in TFS.
function _getOpenFiles()
{
    var files = new Array();
    try 
    {
	var ts = new _Stream();
	runCmdToStream("tf status", ts);
	var lines = ts.ReadLines();
	
	
	for(var i = 0; i < lines.length; i++)
        if (lines[i].match(/[^\s]+\s+[^\s]+\s+([^\s]+)/i))
            files.push(RegExp.$1);
    }
    catch(e) {}
    return files;
}

/**************************************************************************/
// Creates the ReportStressBug.cmd file.
function _createCopyCommand(outDir)
{
    var script = 
        "@echo off\r\n" +
        "xcopy %~dp0* " + getUniqueDateFileName("\\\\clrstresstb\\dumpfiles", "ddrStress_", "_" + Env("USERNAME")) + "\\\r\n"
        ;
    FSOWriteToFile(script, outDir + "\\ReportStressBug.cmd");
}

/**************************************************************************/
function _printLine(htmlFile, value, text)
{
    htmlFile.WriteLine("<br/><b>" + value + "</b> " + text);
}

/**************************************************************************/
function _dumpReport(outDir)
{
    if (outDir == undefined)
        throw Error(1, "_htmlReport: 'outDir' parameter not given");
    outDir = FSOGetFolder(outDir).Path;
    reportDir = outDir;
    
    _createCopyCommand(outDir);

    var reportName = reportDir + "\\Report.html";
    logMsg(LogDump, LogInfo1000, "_dumpReport: creating report ", reportName, "\n");
    var htmlFile = FSOOpenTextFile(reportName, 2, true); 

    // Print the header
    htmlFile.WriteLine("<html>"); 
    htmlFile.WriteLine("<title>Dump Report</title>");
    htmlFile.WriteLine("<style>");
    htmlFile.WriteLine("body { font-family: calibri, arial, tahoma, sans-serif; font-size=11pt; }");
    htmlFile.WriteLine("tr.clr { background-color: white; color: black; }");
    htmlFile.WriteLine("tr.critical { font: bold; background-color: red;}");
    htmlFile.WriteLine("tr.open { background-color: yellow;}");
    htmlFile.WriteLine("tr.other { color: gray;}");
    htmlFile.WriteLine("table.stack { border: solid 1px black; border-collapse: collapse; }"); 
    htmlFile.WriteLine("td { font-size: 11pt; border: solid 1px black; padding-left: 0.3em; padding-right: 0.3em; }"); 
    htmlFile.WriteLine("th { border: solid 1px black; padding-left: 0.3em; padding-right: 0.3em; }"); 
    htmlFile.WriteLine("</style>"); 
    htmlFile.WriteLine("<body>"); 
    htmlFile.WriteLine("<h2>Dump Report</h2>");
    
    // Print info about the dump and crash.
    var cmdArgs="";
    for(var i = 0;i <WScript.Arguments.Length; i++)
        cmdArgs += " " + WScript.Arguments(i);
    
    _printLine(htmlFile, "Command that generated this report:", WScript.ScriptFullName + cmdArgs);
    _printLine(htmlFile, "Report generated on:", prettyTime() + " from machine " + Env("COMPUTERNAME"));
    _printLine(htmlFile, "Report path:", uncPath(reportName));
    _printLine(htmlFile, "Raw dump files:", "<a href="  + uncPath(reportDir) + ">" + uncPath(reportDir) + "</a>");
    
    var tlist_t = reportDir + "\\taskList.txt";
    if (FSOFileExists(tlist_t))
    {
        tlist_t = relPath(tlist_t, reportDir);
        _printLine(htmlFile, "Process list:", "<a href=" + tlist_t + ">" + tlist_t + "</a>");
    }

    // Now print out the callstack.
    cdbTxts = FSOGetFilePattern(reportDir, /cdb.txt$/i);
    
    var openFiles = _getOpenFiles();
    if (cdbTxts.length > 0)
        _dumpReportProc(cdbTxts[0], htmlFile, reportDir, openFiles);

    // Print the closing tags.
    htmlFile.WriteLine("</body>");
    htmlFile.WriteLine("</html>");
    htmlFile.Close();
    
    logMsg(LogDump, LogInfo, "Dump report in: "  + outDir + "\\Report.html\n");
}

function _isCLRModule(module)
{
  return module == "clr" || module == "mscoree" || module == "mscorjit" || module == "mscorlib_ni" || module == "mscorlib";
}

function _isCriticalFunction(module, funct)
{
  if (module == "ntdll")
    return funct == "RaiseException" || funct == "DbgBreakPoint" || funct == "NtRaiseHardError";
  
  // Control-C
  if (module == "kernel32")
    return funct == "CtrlRoutine";

  if (module == "clr")
    return funct == "DbgAssertDialog";
    
  if (module == "user32")
    return funct == "MessageBoxW" || funct == "MessageBoxA";

  return false;
}

function _getRowClass(isClr, isCritical, isOpen)
{
  if (isCritical)
    return "critical";

  if (isOpen)
    return "open";

  if (isClr)
    return "clr";
  
  return "other";
}

function _inArray(needle, haystack)
{
  if (needle != null)
    for (var i = 0; i < haystack.length; ++i)
      if (haystack[i].toLowerCase() == needle.toLowerCase())
        return true;
   
  return false;
}

/**************************************************************************/
/* given a cdb.txt, pretty print the output */

function _dumpReportProc(cdbTxt, htmlFile, reportDir, openFiles)
{
    if (!cdbTxt.match(/((.*)\\Dump\.\d+\.Proc\.(\w+))\.cdb\.txt/i))
    {
        htmlFile.WriteLine("<b>Error! Could not parse " + cdbTxt + "</b><br />");
        return;
    }
    
    var base = RegExp.$1;
    var dir = RegExp.$2;
    var pid = RegExp.$3;
    var dmpFile = base + ".dmp";
    var windbgCmd = base + ".runWinDbg.cmd";
    var stressLog = base + ".StressLog.txt";
    var sos = dir + "\\sos.dll";
    var taskListFile = reportDir + "\\taskList.xml";
    
    var procTree = undefined;
    if (FSOFileExists(taskListFile))
        procTree = xmlDeserialize(taskListFile);

    // Parse the output into the output of individual commands and place in 'cmdOut'
    var cdbOut = FSOReadFromFile(cdbTxt);
    var cmdOut = {};
    while (cdbOut.match(/\nCMD-START\s+(.+)\n((.|\n)*?\n)\s*\n*CMD-END/m))
    {
        cmdOut[RegExp.$1] = RegExp.$2;
        cdbOut = RegExp.rightContext;
    }

    // Get command line info
    var commandLine;
    var exe = "{UNKNOWN}";
    var pebData = cmdOut["!peb"];
    if (pebData && pebData.match(/^\s*ImageFile:\s*'(.*)'\s*\n\s*CommandLine:\s*'(.*)'\s*$/im))
    {
        exe = RegExp.$1;
        commandLine = RegExp.$2;
    }

    // Could not find it in the !peb in the image, try to get it from the .tlist data
    if (commandLine == undefined)
    {
        var tlistData =  cmdOut[".tlist /v"];
        if (tlistData && tlistData.match(new RegExp("^\\s*0n" + pid + "\\s+(.*)\\n\\s*Command Line:\\s*([^\n]*)", "mi")))
        {
            exe = RegExp.$1;
            commandLine = RegExp.$2;
        }
    }

    
    // ******************* Command line info
    _printLine(htmlFile, "Process:", pid);
    _printLine(htmlFile, "Executable:", exe);

    if (procTree != undefined && procTree.pid2ProcInfo != undefined)
    {
        var procInfo = procTree.pid2ProcInfo[pid];
        if (procInfo != undefined)
        {
            _printLine(htmlFile, "CPU utilization at time of dump:", procInfo.cpuPercent);
            _printLine(htmlFile, "Total Run Time:", _procTreeTime(procInfo.runTimeMs-0));
            _printLine(htmlFile, "Total Cpu Time:", _procTreeTime(procInfo.cpuTimeMs-0));
        }
        
        var lastStressMsgMs = undefined;
        if (FSOFileExists(stressLog))
        {
            _printLine(htmlFile, "Stress Log:", "<a href=" + relPath(stressLog, reportDir) + ">" + relPath(stressLog, reportDir) + "</a>");
            
            var stressText = FSOReadFromFile(stressLog);
            if (stressText.match(/Total elapsed time *([\d\.]+) *sec/i)) 
                lastStressMsgMs = RegExp.$1 * 1000
        }
        
        if (lastStressMsgMs != undefined)
            _printLine(htmlFile, "Process run time:", _procTreeTime(lastStressMsgMs));
    }


    if (FSOFileExists(cdbTxt))
        _printLine(htmlFile, "Raw CDB Output:", "<a href='" + relPath(cdbTxt, reportDir) + "'>" + relPath(cdbTxt, reportDir) + "</a>");

    if (FSOFileExists(dmpFile))
        _printLine(htmlFile, "Dump File:", "<a href='"+ relPath(dmpFile, reportDir) + "'>" + relPath(dmpFile, reportDir) + "</a>");
    
    if (FSOFileExists(windbgCmd))
        _printLine(htmlFile, "Run WinDbg:", "<a href='" + relPath(windbgCmd, reportDir) + "'>" + relPath(windbgCmd, reportDir) + "</a>");

    if (commandLine != undefined)
        _printLine(htmlFile, "Command Line:", commandLine);
    
    // Get the last event:
    var lastEventFile = reportDir + "\\..\\lastevent.log";
    if (FSOFileExists(lastEventFile))
    {
        var lastEventOut = FSOReadFromFile(lastEventFile);
        if (lastEventOut.match(/Last event: [a-zA-Z0-9]+\.([a-zA-Z0-9]+):([^\n]*)/))
            _printLine(htmlFile, "<font color=red>Last Event (cause of the crash):</font>", "<b>" + RegExp.$2 + "</b> on thread <a href=#thread" + RegExp.$1 + ">" + RegExp.$1 + "</a>");
        else
            htmlFile.WriteLine("<!-- Regular expression didn't match.  Result was: " + lastEventOut + " -->");
    }
    else
    {
        htmlFile.WriteLine("<!-- Could not find '" + lastEventFile + "'. -->");
        _printLine(htmlFile, "Last Event (cause of the crash):", "No last event information found, this was likely a deadlock/hang or a dialog box which popped up.");
    }
    
    _printLine(htmlFile, "<font color=red>If this is not your bug:</font>", "<a href=\"ReportStressBug.cmd\">Report Stress Bug</a>");

    var output = "";
    var criticalThreads = new Array();
    var threads = _dumpParseStack(cmdOut["~*kp 50"]);
    var dllsOnStack = {};
    for (var threadId in threads)
    {
        var thread = threads[threadId];
        var osID = thread.OSTid;
        var threadAnchor = "thread" + osID;
        var criticalThread = false;

        output += "<br/><hr />\r\n";
        output += "<a name=" + threadAnchor + "><b>Stack for thread " + threadId + ", OSID " + thread.OSTid + ":</b></a>\r\n";
        output += "<table class=stack>\r\n";
        output += "<tr><th align=left>Method</th><th align=left>Source</th></tr>\r\n";
        
        for (var i = 0; i <thread.stack.length; i++)
        {
            var entry = thread.stack[i];
            var lineInfo = "";
            var currFile = null;
            var isClr = _isCLRModule(entry.module);
            var isCritical = _isCriticalFunction(entry.module, entry.method);
            
            if (isCritical && !criticalThread)
            {
                criticalThread = true;
                criticalThreads[criticalThreads.length] = "<a href=#" + threadAnchor + ">" + osID + "</a>";
            }
            
            if (entry.rest.match(/\[\s*(\S*)\s*@\s*(\d*)\s*\]/))
            {
                lineInfo = RegExp.$1;
                currFile = lineInfo;
                if (isClr)
                  lineInfo = "<a href='" + lineInfo + "'>" + lineInfo + "</a>";
                lineInfo += " @ " + RegExp.$2;
            }
            
            output += "<tr class=" + _getRowClass(isClr, isCritical, _inArray(currFile, openFiles)) + ">\r\n";
            output += "  <td align=left>" + entry.module + "!" + entry.method + "</td>\r\n";
            output += "  <td align=left>" + lineInfo + "</td>\r\n";
            output += "</tr>\r\n";
        }
        
        output += "</table>\r\n";
    }
    
    if (criticalThreads.length > 0)
    {
        _printLine(htmlFile, "Critical threads:", criticalThreads.join(" "));
    }

    htmlFile.WriteLine(output);

        // ******************* Version info Any dlls on the stack
    var dllsOnStackList = [];
    for (var dllName in dllsOnStack) 
        dllsOnStackList.push(dllName);

    dllsPatternStr = "(" + dllsOnStackList.join(")|(") + ")";
    var dllPattern= new RegExp(dllsPatternStr, "i");

    var modInfo = _dumpParseModules(cmdOut["lm v"]);
    var verInfoRows = "";
    for (var i in modInfo)
    {
        var mod = modInfo[i];
        if (mod.name.match(dllPattern) && !(mod.path.match(/^[:\w]*\\win\w+\\system32\\[^\\]+$/i) && !mod.name.match(/mscoree/i)))
            verInfoRows += "    <tr><td> " + mod.name + " </td><td> " + mod.fileVersion + "</td></tr>\n";
    }
    
    if (verInfoRows != "")
    {
        htmlFile.WriteLine("<br/><hr />");
        htmlFile.WriteLine("Version information for modules on stack:");
        
        htmlFile.WriteLine("<table class=stack>");
        htmlFile.WriteLine("<tr><th>Module Name</th><th> File Version Info </th></tr>");
        htmlFile.WriteLine(verInfoRows);
        htmlFile.WriteLine("</table>");
    }
}

/**************************************************************************/
/* parse the 'lm v' information into a structure and return it */

function _dumpParseModules(lmStr) {

    var ret = [];
    if (lmStr != undefined) {
        var lmsOut = lmStr.split("\n");
        var curInfo = undefined; 
        for(var i = 0; i < lmsOut.length; i++) {
            var line = lmsOut[i];
            if (line.match(/^(\S+)\s+(\S+)\s+(.+?)\s+\((.*?)\)\s*(\S*)/)) {
                curInfo = {};
                curInfo.imageBase = RegExp.$1;
                curInfo.name = RegExp.$3;
                curInfo.symbolType = RegExp.$4;
                if (RegExp.$5.length > 0)
                    curInfo.pdbName = RegExp.$5.toLowerCase();
                ret.push(curInfo);
            }
            else if (curInfo && line.match(/^\s+Image path:\s+(.*)/i)) {
                curInfo.path = RegExp.$1;
            }
            else if (curInfo && line.match(/^\s+Product version:\s+(.*)/i)) {
                curInfo.productVersion = RegExp.$1;
            }
            else if (curInfo && line.match(/^\s+File version:\s+(.*)/))
                curInfo.fileVersion = RegExp.$1;
            else if (line.match(/^Unloaded modules:/)) 
                break;
            else if (line.match(/^(\S+)/) && RegExp.$1 != "start") {
                logMsg(LogDump, LogError, "Unexpected lm -v line ", line, "\n");
                curInfo = {};
            }
        }
        return ret;
    }
}

/**************************************************************************/
/* parse the kp information into a structure and return it */

function _dumpParseStack(kpStr) {

    if (!kpStr)
        return {};

    var stackOut = kpStr.split("\n");
    var threadInfo = {};
    var threadId = undefined; 
    for(var i = 0; i < stackOut.length; i++) {
        var line = stackOut[i];
        if (line.match(/^\.?\s*(\d+)\s+Id: (\w+)\.(\w+) Suspend:\s*(\d+)\s+Teb:\s*(\w+)/i)) {
            var threadId = RegExp.$1;
            threadInfo[threadId] = {};
            threadInfo[threadId].OSTid = RegExp.$3;
            threadInfo[threadId].Suspend = RegExp.$4;
            threadInfo[threadId].Teb = RegExp.$5;
            threadInfo[threadId].stack = [];
        }
        else if (threadId != undefined && line.match(/^[`0-9A-Fa-f\s]* (\w+)!\s*([\w:.]+)(.*)/))
            threadInfo[threadId].stack.push({ module: RegExp.$1, method: RegExp.$2, rest: RegExp.$3 });
        else {
            if (!line.match(/^\s*(Child.*Call.Site.*)?$/)) 
                logMsg(LogDump, LogInfo100, "thread ", threadId, " Unrecognised stacktrace line '", line, "'\n");
        }

        if (threadId)
            threadInfo[threadId].rawStack += line;
    }

    return threadInfo;
}

/**************************************************************************/
/* Create a full memory dump all interesting processes which are descendants 
   of the process with ID 'pid'. A process is interesting if it is not some 
   scripting programs (cmd.exe, perl.exe, cscript). or a debugger (windbg, 
   cdb ..).   The PDBs needed for a stack trace and managed support Dlls are 
   also copied.  Thus the target directory contains everything you need to 
   effectively debug.  Note that this can easily take 1Gig of space.

     Parameters:
         pat    : A pattern.  This is matched against the name of the executable
         outDir : Where to put the information.  If not specified, dumpProc
                  will generate a unique name in the current directory.
*/
function dumpProcByPat(pat, outDir) {
    _dumpProcByPat(pat, outDir, true)
}

function _dumpProcByPat(pat, outDir, generateReport) {

    logCall(LogScript, LogInfo, "dumpProcByPat", arguments);
    if (typeof(pat) == "string")
        pat = new RegExp("^" + pat, "i");
            
    var procs = procGetAll();
    var numDumps = 0
    for (var i = 0; i < procs.length; i++) {
        var proc = procs[i];
        var exePath = proc.ExecutablePath;
        if (exePath && exePath.match(/([^\\]+)$/)) {
            var exe = RegExp.$1;
            if (exe.match(pat))  {
                numDumps++;
                logMsg(LogDump, LogInfo, "Found EXE ", exe, " that matched pattern", pat);
                try {
                    // Don't generate dumps for other interesting processes, as
                    // dumpProcByPat is usually targetted
                    outDir = _dumpProc(proc.ProcessID, outDir, false, generateReport);
                }
                catch(e) {
                    logMsg(LogDump, LogWarn, "dumpProc failed: message", e.description);
                }
            }
        }
    }
    if (numDumps == 0)
        logMsg(LogDump, LogWarn, "dumpProcByPat: No dumps match pattern ", pat, "\n");
    else
        logMsg(LogDump, LogWarn, "dumpProcByPat: ", numDumps, " dumps taken\n");

    return outDir
}

/**************************************************************************/
/* Dump the processes for any processes that match any of the patterns
   (we show 2 but it is really as many as you want).
*/
function dumpProcByPats(outDir, pat1, pat2) {

    for(var i = 1; i < arguments.length; i++) 
        dumpProcByPat(arguments[i], outDir);
    return 0;
}

/**************************************************************************/
/* Create dumps for other processes that may be interesting, as they interact
   with the runtime
*/
function dumpInterestingProcesses(outDir) {
    logMsg(LogDump, LogInfo, "Generating dumps of other interesting processes {\n");         
            
    // Generate dumps for interesting processes, but not reports            
    dumpProcByPat("mscorsvw", outDir, false);
    dumpProcByPat("aspnet_wp", outDir, false);        
    logMsg(LogDump, LogInfo, "} Generating dumps of other interesting processes\n");         
}

/**************************************************************************/
/* Create a full memory dump of all interesting processes which are descendants 
   of the process with ID 'pid'. A process is interesting if it is not some 
   scripting programs (cmd.exe, perl.exe, cscript). or a debugger (windbg, 
   cdb ..).   The PDBs needed for a stack trace and managed support Dlls are 
   also copied.  Thus the target directory contains everything you need to 
   effectively debug.  Note that this can easily take 1Gig of space.

     Parameters:
         pid    : All descendants of this process will be dumped
         outDir : Where to put the information.  If not specified, dumpProc
                  will generate a unique name in the current directory.
         optStr : options 

     Options
         /noInterest : don't dump other 'interesting' service processes 
                       TODO reverses this
*/
function dumpProc(pid, outDir, optStr) {    
    
    var opt = getOptions(["noInterest"], optStr);

    _dumpProc(pid, outDir, !opt.noInterest);
}

function _dumpProc(pid, outDir, dumpOtherInterestingProcesses, generateReport) {

    if (pid == undefined)
        throw new Error(1, "Required parameter 'pid' missing");
                    
    var procObj = procGetByPid(pid);
    if (!procObj) 
        throw new Error(1, "Could not find process '" + pid + "'");

    if (outDir == undefined) {
        var commandLine = procObj.CommandLine + "";
        outDir = _dumpGetName(commandLine);
        logMsg(LogDump, LogInfo, "Output Dir: ", outDir, "\n");
    }
    
    if (dumpOtherInterestingProcesses == undefined)
        dumpOtherInterestingProcesses = true;        

    if (generateReport == undefined)
        generateReport = true;            
    
    
    logCall(LogScript, LogInfo, "dumpProc", arguments);
    logMsg(LogDump, LogInfo, "dumpProc: CMD: ", procObj.CommandLine, "\n");

    var runtimeDirs = FSOGetDirPattern(Env("SystemRoot") + "\\Microsoft.NET\\Framework", /v\d+\..*/);
    runtimeDirs = runtimeDirs.concat(FSOGetDirPattern(Env("SystemRoot") + "\\Microsoft.NET\\Framework64", /v\d+\..*/));
    var symPath = Env("_NT_SYMBOL_PATH") + ";" + runtimeDirs.join(";") + ";c:\\windows\\system32;c:\\symbols;srv*\\\\symbols\\symbols;srv*\\\\cpvsbuild\\drops\\symbols";
    Env("_NT_SYMBOL_PATH") = symPath;

        // only print out the symbol path for the first dump, since it will be same for recursive calls
    if (dumpSeq == 0)
        logMsg(LogDump, LogInfo, "_dumpReport: _NT_SYMBOL_PATH= {\n", symPath.replace(/;+/g, "\n"), "\n}\n");

    FSOCreatePath(outDir);
    try {
        var procTree = _procTreeGetStats();

            // save it as a human readable string
        var procTreeStr = procTreeAsString("/fullCmdLine", procTree);
        FSOWriteToFile(procTreeStr, outDir + "\\taskList.txt");

            // also save it as XML to parse latter
        xmlSerialize(procTree, outDir + "\\taskList.xml", {proc:true});
    } catch(e) { 
        logMsg(LogDump, LogError, "_dumpReport: error taking process list, skipping\n"); 
        logMsg(LogDump, LogError, "_dumpReport: exception = ", e.description, "\n");
    }

    try {
        _dumpChildren(pid, outDir);
        
        if (dumpOtherInterestingProcesses == true) { 
            dumpSeqFirstOtherInterestingProcess = dumpSeq; 
            dumpInterestingProcesses(outDir)
            }
    } catch(e) {
        logMsg(LogDump, LogError, e.description, "\n");
    }
    if (generateReport == true) {        
        _dumpReport(outDir);
    }
    return outDir;
}

/**************************************************************************/
/* Create a dump report from a previously taken dump.  This is useful
   if someone did a dump by hand, and now you want to package it up 
   so that it includes the PDBs and stress log etc.  

     Parameters:
         dumpFile: All descendants of this process will be dumped
         outDir  : Where to put the information.  If not specified dumpProc
                   will generate a unique name in the current directory.
*/
function dumpFromFile(dumpFile, outDir) {

    if (dumpFile == undefined)
        throw new Error(1, "Required parameter 'dumpFile' missing");

    if (outDir == undefined) {
        outDir = _dumpGetName();
        logMsg(LogDump, LogInfo, "Output Dir: ", outDir, "\n");
    }

    var cdbDir = _dumpGetCDBDir();
    Env("Path") = cdbDir + ";" + Env("Path");

    var runtimeDirs = FSOGetDirPattern(Env("SystemRoot") + "\\Microsoft.NET\\Framework", /v\d+\..*/);
    runtimeDirs = runtimeDirs.concat(FSOGetDirPattern(Env("SystemRoot") + "\\Microsoft.NET\\Framework64", /v\d+\..*/));
    var symPath = Env("_NT_SYMBOL_PATH") + ";" + runtimeDirs.join(";") + ";srv*\\\\symbols\\symbols;srv*\\\\cpvsbuild\\drops\\symbols";
    Env("_NT_SYMBOL_PATH") = symPath;
    logMsg(LogDump, LogInfo1000, "_dumpReport: _NT_SYMBOL_PATH=", symPath, "\n");
    
    FSOCreatePath(outDir);

    _dumpProcInfo("-z " + dumpFile, outDir + "\\Dump.0.Proc.DUMP")
    _dumpReport(outDir);
    return 0;
}

