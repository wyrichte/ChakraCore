/******************************************************************************/
/*                                ClrProcess.js                               */
/******************************************************************************/

/* testProcess - represents a whole sequence of runs
        xmlFile     an xml file that is used to persist the information
        report      the html report for the whole test process
        name        Name of the test process (for reports)
        build:      maps SD ids => testBuilds
        failures:   maps (testRunName:testid) => testFailure
    
   testBuild - represents a single build of the runtime in the sequence)
        name        Name of SNAP submission
        ID          Source Depot change number
        runs        map of testRunName -> testRun
        submitters  list of devs checkin into SNAP
        report      SNAP HTML report 
        process_    pointer to testProcess
        
    testRun - represents a single run of a batch of tests
        name        name of the test run (eg 'test.devBVT@x86chk')
        report      SMARTY test run report (HTML)
        failures    list of testCase for failed tests
        build_      pointer to testBuild

    testCase - represents the run of a single test
        name        name of the test (not unique but user friendly)
        testid      guarenteed to be unique 
        report      SMARTY report for just that test run 
        testowner   the owner of the test case
        run_        pointer to testRun
        
    testFailure - represents all failures of a single test EXE (over many test 
                    testBuilds).  The (testRunName:testid), is a unique 
                    identifer for a testFailure.  
        runname     the name of the test run
        testname    name of the test (not unique but user friendly)
        testid      id of the test (must be unique)
        testCases   the list of particular testCase that failed     
**/

// AUTHOR: Vance Morrison 
// DATE: 1/30/2004

/******************************************************************************/

var clrProcessModuleDefined = 1;        // Indicate that this module exist
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include ClrAutomation.js");

var LogClrProcess = logNewFacility("rolling");

if (WshShell == undefined)
    var WshShell = WScript.CreateObject("WScript.Shell");
if (Env == undefined)
    var Env = WshShell.Environment("PROCESS");
if (ScriptDir == undefined)
    var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

// We should change rollingBranch to the new TFS branch, but it looks like some 
// code still depends on it right now.  So I'm adding a new global variable in
// order to avoid breaking existing functionality.
var rollingBranch = "puclr";
var rollingBranchTFS = "$/DevDiv/pu/clr";
var bugDatabase = "DevDiv Bugs";

// logSetFacilityLevel(LogClrProcess, LogInfo100000);

/*****************************************************************************/
function rollingMakeBug(
        bugTitle, 
        bugOpenedBy,
        bugAssignTo,
        bugTestCase, 
        bugDescription,
        bugAttachment,
        bugPromptUser) {

    
    logMsg(LogClrAutomation, LogInfo, "Creating new bug...\n");
    logMsg(LogClrAutomation, LogInfo, "Title: ", bugTitle, "\n");
    logMsg(LogClrAutomation, LogInfo, "Opened by: ", bugOpenedBy, "\n");
    logMsg(LogClrAutomation, LogInfo, "Assign to: ", bugAssignTo, "\n");
    logMsg(LogClrAutomation, LogInfo, "Test case: ", bugTestCase, "\n");
    logMsg(LogClrAutomation, LogInfo, "Description: ", bugDescription, "\n");
    logMsg(LogClrAutomation, LogInfo, "Attachment: ", bugAttachment, "\n");
        
    // TODO: convert to DevDiv Bugs (use bugDatabase)
    // It appears this function hasn't been used since 9/27/2005
        
    var bugDb = bugConnect("VSWhidbey");
    var bug = bugCreate(bugDb);

    var fixBy = "Whidbey RTM RC";
    
    
    bugFieldSet(bugDb, bug, "Title", bugTitle.substring(0, 192));
    bugFieldSet(bugDb, bug, "Assigned to", bugAssignTo);
    bugFieldSet(bugDb, bug, "Opened by", bugAssignTo);
    bugFieldSet(bugDb, bug, "Priority", 0);
    bugFieldSet(bugDb, bug, "Severity", 1);
    bugFieldSet(bugDb, bug, "Description", bugDescription);
    bugFieldSet(bugDb, bug, "Issue type", "Code Defect");
    bugFieldSet(bugDb, bug, "How found", "Rolling Test Run");
    bugFieldSet(bugDb, bug, "Source", "Development");
    bugFieldSet(bugDb, bug, "Open Build", rollingBranch + ".00000.00");
    bugFieldSet(bugDb, bug, "SKU", "Whidbey");
    bugFieldSet(bugDb, bug, "Fix By", fixBy);
    bugFieldSet(bugDb, bug, "Path", "Common Language Runtime\\JIT\\Code Gen");    
    bugFieldSet(bugDb, bug, "TestCase", bugTestCase.substring(0, 192));
    
    bugAddFile(bugDb, bug, bugAttachment);
        
    bugSave(bug);
    logMsg(LogClrAutomation, LogInfo, "Created bug with ID ", bugFieldGet(bugDb, bug, "ID"), "\n");
    
    return 0;
}

/****************************************************************************/
function snapJobInfo(snapJobDir) {

    var ret = undefined

    var jobReport = snapJobDir + "\\jobReport.html";
    if (FSOFileExists(jobReport)) {
        ret = {};
        ret.submitters = [];
        ret.report = jobReport;
        ret.jobDir = snapJobDir;
        var jobReportData = FSOReadFromFile(jobReport);

        if (jobReportData.match(/Job Start Time: (.*\S)/))
            ret.starttime = RegExp.$1;
        
        if (jobReportData.match(/Job End Time: (.*\S)/))
            ret.endtime = RegExp.$1;
            
        
        var data = jobReportData;
        while(data.match(/<LI>.*User: *(\S+)/im)) {
            data = RegExp.rightContext;
            var submitter = RegExp.$1;
            if (submitter != "clrgnt") {
                // logMsg(LogClrProcess, LogInfo, "FOUND submitter '", submitter, "'\n");
                ret.submitters.push(submitter);
            }
        }

        var entries = [];
        var data = jobReportData;
        while(data.match(/<LI>.*Entry Description:.*?\\(\d+\.[^\\]+)\\(\S+).xml/im)) {
            data = RegExp.rightContext;
            entries.push(RegExp.$1);
        }
        if (entries.length != 0) {
            ret.name = entries.join(" ");
        }
        else {
            logMsg(LogClrProcess, LogError, "Could not find names in ", jobReport, "\n");
            ret.name = "UNKNOWN";
        }
    }
    else 
        logMsg(LogClrProcess, LogError, "Could not find job report file ", jobReport, "\n");
    return ret;
}

/*****************************************************************************/
/* parses all test information from 'runDirsBase\(\d+), and returns it as
   a structure (calleda testProcess), It is relatively complicated, so
   it is best to just do a 'runjs testProcessGet' and see
   what it gives you */

function testProcessGet(runDirsBase) {

    if (runDirsBase == undefined)
        runDirsBase = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests";

    logMsg(LogClrAutomation, LogInfo, "testProcessGet(", runDirsBase, "){\n");
    var testProcessXml = runDirsBase + "\\testProcess.xml";
    var testProcess = readFromXml(testProcessXml, "testProcess");
    if (!testProcess) {
        testProcess = {};
        testProcess.objID = "testProcess_" + objIds; objIds++;
        testProcess.report = runDirsBase + "\\report.html";
        testProcess.name = "CLR Rolling Tests";
        testProcess.build = {};
        testProcess.failures = {};
        testProcess.xmlFile = testProcessXml;
    }

    var runDirs = FSOGetDirPattern(runDirsBase, /^[\d.]+$/);

    // FIX NOW - hack  these should be under rolling
    var hackRunDirsBase = runDirsBase + "AMD64";
    if (FSOFolderExists(hackRunDirsBase)) 
        runDirs = runDirs.concat(FSOGetDirPattern(hackRunDirsBase, /^[\d.]+$/));
    var hackRunDirsBase = runDirsBase + "IA64";
    if (FSOFolderExists(hackRunDirsBase)) 
        runDirs = runDirs.concat(FSOGetDirPattern(hackRunDirsBase, /^[\d.]+$/));

    runDirs.sort(function(x, y) { return (numSuffix(y, "") - numSuffix(x, "")); });
    var last = undefined;
    var updated = false;
    for (var j = 0; j < runDirs.length; j++) {
        var runDir = runDirs[j];
        var buildID = runDir.match(/([\d.]+)$/)[1];
        logMsg(LogClrAutomation, LogInfo, "testProcessGet: DBG Got dir ", runDir, "\n");

        var testBuild = testProcess.build[buildID];
        if (!testBuild) {
            var jobDirUrl = runDir + "\\jobDir.url";
            if (FSOFileExists(jobDirUrl)) {
                updated = true;
                var snapJobDir = urlShortCutTarget(jobDirUrl)
                logMsg(LogClrAutomation, LogInfo, "testProcessGet: WORK: Found new snap job ",  snapJobDir, "\n");
                testBuild = snapJobInfo(snapJobDir);
                if (testBuild == undefined) {
                    logMsg(LogClrAutomation, LogInfo, "testProcessGet: could not find SNAP job file ", snapJobDir, "\n");
                    continue;
                }
                testBuild.ID = buildID;
                testBuild.objID = "testBuild_" + objIds; objIds++;
                testBuild.process_ = testProcess;
                testBuild.runs = {};
                testProcess.build[buildID] = testBuild;
            }
            else {
                logMsg(LogClrAutomation, LogInfo, "testProcessGet: could not find url file ", jobDirUrl, "\n");
                continue;
            }
        }
        if (last && last.ID != testBuild.ID) {      // TODO the ID check is a hack
            testBuild.next_ = last;
            last.prev_ = testBuild;
        }
        last=testBuild;

        var testResultFiles = FSOGetFilePattern(runDir, /rolling\.(.*)\.results\.txt$/i);
        if (testResultFiles.length == 0)
            logMsg(LogClrAutomation, LogInfo, "testProcessGet: No test results files in ", runDir, "\n");

        for (var i = 0; i < testResultFiles.length; i++) {
            var testResultFile = testResultFiles[i];
            var name = testResultFile.match(/rolling\.(.*)\.results\.txt$/i)[1];

            if (!testBuild.runs[name]) {
                updated = true;
                logMsg(LogClrAutomation, LogInfo, "testProcessGet: WORK: Found new smarty results ",  testResultFile, "\n");
                var testResultData = parseFile(testResultFile);
                if (testResultData.SMARTY_RESULTS) {
                    var smartyDir = testResultData.SMARTY_RESULTS.match(/(.*)\\/)[1];

                    // FIX NOW HACK can removed after 5/15/04
                    smartyDir = smartyDir.replace(/\\\\clrsnapbackup2\\public\\rolling/gi, "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests");

                    logMsg(LogClrAutomation, LogInfo, "testProcessGet: for results ", name, " found SMARTY dir ", smartyDir, "\n");
                    var testRun = testRunFromSmarty(smartyDir, testProcess.failures, name);
                    testBuild.runs[name] = testRun;
                    testRun.build_ = testBuild;
                    testRun.name = name;
                }
                else 
                    logMsg(LogClrAutomation, LogInfo, "testProcessGet: no SMARTY_RESULTS tag in ", testResultFile, ".  Skipping\n");
            }
            else 
                logMsg(LogClrAutomation, LogInfo, "testProcessGet: already parsed test for ", testResultFile, "\n");
        }

    }

        // I need the failures sorted by build number 
    for(var i in testProcess.failures) {
        testFailure = testProcess.failures[i]
        if (testFailure.testCases)
            testFailure.testCases.sort(function(x, y) { return x.run_.build_.ID - y.run_.build_.ID });
    }

    if (updated) {
            // remove builds from the XML that are not in the runDirs directory any more  
        if (runDirs.length > 0 && runDirs[runDirs.length - 1].match(/(\d+(\.\d*)?)$/)) {
            var firstBuildID = RegExp.$1 - 0;

            for (var buildID in testProcess.build) {
                if ((buildID - 0) < firstBuildID) {
                    logMsg(LogClrAutomation, LogInfo, "removing build ", buildID, " < ", firstBuildID, " from XML\n");
                    testProcess.build[buildID] = undefined;
                }
            }
        }
        xmlWrite(testProcess, testProcess.xmlFile, "testProcess");
    }

    logMsg(LogClrAutomation, LogInfo, "} testProcessGet\n");
    return testProcess;
}

/*****************************************************************************/
/* parses the SMARTY XML data an returns a list of test cases that failed
   If 'testFailures' is defined, it is assumed to be a table that maps 
   (testRunName:testid) strings to test cases, and it adds entries to this table too */

function testRunFromSmarty(smartyDir, testFailures, testRunName) {

    logMsg(LogClrAutomation, LogInfo, "testRunFromSmarty(", smartyDir, ")\n");
    var smartyXMLFile = smartyDir + "\\Smarty.xml";
    if (!FSOFileExists(smartyXMLFile))
        return undefined;
    var smartyXMLData = FSOReadFromFile(smartyXMLFile);
    var testRun = {};
    testRun.failures = [];
    testRun.objID = "testRun_" + objIds; objIds++;
    testRun.dir = smartyDir;

    if (!smartyXMLData.match(/HARNESS_LOG\s*=\s*"(.*?)"(.|\n)*?<TESTCASES\s+(.*?)\s*>/mi)) 
        throw Error(1, "Could find TESTCASES XML tag, bad smarty XML file " + smartyXMLFile);

    testRun.report = RegExp.$1;
    smartyXMLData = RegExp.rightContext;
    var header = RegExp.$1;

    if (header.match(/TOTAL *= *"?(\d+)"?/i))
        testRun.total = RegExp.$1;
    if (header.match(/PASSED *= *"?(\d+)"?/i))
        testRun.passed = RegExp.$1;

    var testCases = smartyXMLData.split(/<TESTCASE\b/);
    for(var i = 0; i < testCases.length; i++) {
        var testCaseXML = testCases[i];

        if ((!testCaseXML.match(/SUCCEEDED=.*yes/im) || testCaseXML.match(/SUCCEEDED=[\s"]*no/)) && !testCaseXML.match(/^\s*$/)) {
            var testCase = testCaseFromSmarty(testCaseXML);
            testRun.failures.push(testCase);
            testCase.run_ = testRun;
            if (testFailures && testCase.testid)  {
                var testCaseKey = testRunName + ":" + testCase.testid;
                if (!testFailures[testCaseKey])
                    testFailures[testCaseKey] = { 
                        testCases: [], 
                        runname: testRunName, 
                        testowner: testCase.testowner, 
                        testid: testCase.testid, 
                        testname: testCase.name 
                    };
                testFailures[testCaseKey].testCases.push(testCase);
            }
        }
    }
    // logMsg(LogClrAutomation, LogInfo, "**** INFO for test case ", testCase.name, " = ", dump(testFailures, 2), "\n");
    return testRun;
}

/*****************************************************************************/
/* parses a single smarty XML test case blob into a return structure where
   each XML tag is a field */

function testCaseFromSmarty(smartyXMLData) {

    var testCase = {};
    testCase.objID = "testCase_" + objIds; objIds++;
    while(smartyXMLData.match(/^\s*(\w+)\s*=\s*"?([^"]*)"?/)) {
        testCase[RegExp.$1.toLowerCase()] = RegExp.$2;
        smartyXMLData = RegExp.rightContext;
    }

        // FIX parse the RUN stuff properly
    if (smartyXMLData.match(/TESTLOG="(.*?)"/)) 
        testCase.testlog = RegExp.$1
    if (smartyXMLData.match(/STARTTIME="(.*?)"/)) 
        testCase.starttime = RegExp.$1
    if (smartyXMLData.match(/DMPRPT.*DIR="(.*?)"/)) 
        testCase.dmprpt = { dir:RegExp.$1 };

        // FIX make smarty do this stuff instead 
        // name to be something more reasonable (needs to be short)
    if (testCase.name) {
        testCase.commandLine = testCase.name
        testCase.name = nameFromCmdLine(testCase.name);
    }

    // logMsg(LogClrAutomation, LogInfo, "testCaseFromSmarty: remainder after parssing: ", smartyXMLData, "\n");
    // logMsg(LogClrAutomation, LogInfo, "Got testCase data = ", dump(testCase), "\n");
    return testCase;
}

/*****************************************************************************/
/* make a reasonable name to print (limited to 48 chars), out of a command line */

function nameFromCmdLine(cmdLine) {

    var size = 48;
    cmdLine = cmdLine.replace(/(\.bat)|(\.js)|(\.exe)|(\.cmd)|(\.dll)|(\.+\\)/g, "");   // remove uninteresting extentions
    cmdLine = cmdLine.replace(/\s+([-\/])/g, "$1");             // remove spaces before qualifiers

    // remove any directory names (strings before a '\') as long as the string is too long
    while (cmdLine.length > size && cmdLine.match(/\b\S+?\\/)) 
        cmdLine = RegExp.leftContext + RegExp.rightContext;

    cmdLine = cmdLine.replace(/\s+/g, "_");             // change whitespace to _
    cmdLine = cmdLine.replace(/[^\w.\\\/-]/g, "");      // remove non-arith chars
    cmdLine = cmdLine.replace(/[\\\/]/g, "-");          // change \ to - (so they can be file names)
    cmdLine = cmdLine.replace(/_*$/g, "");              // remove trailing _
    
    cmdLine = cmdLine.substr(0, size);                  //just truncate
    return cmdLine;
}

/****************************************************************************/ 
/* get an arrayd of bug objects for all test case failures.  (see 
   testCaseBugNew().  However a avoid querying the bug database more than 
   once an hour by caching the results in 'cacheFile' and using that for
   frequent lookups */

function testCaseBugWithCache(activeOnly, bugDb, cacheFile) {

	if (cacheFile == undefined) {
		var mod = activeOnly ? "active" : "all";
		cacheFile = "\\\\clrmain\\public\\drops\\puclr\\testCaseBugs." + mod + ".xml";
	}
	var testCaseBugs = undefined;
	if (FSOFileExists(cacheFile)) {
		logMsg(LogClrAutomation, LogInfo, "testCaseBugWithCache: Found cache ", cacheFile, "\n");
		try {
			var cacheData = xmlDeserialize(cacheFile);
			var now = new Date();
			if (now - cacheData.queryTime < 20*60*1000) 	// cache is stale after 20 min
				testCaseBugs = cacheData;
			else 
				logMsg(LogClrAutomation, LogInfo, "testCaseBugWithCache: CACHE STALE, need to regenerate\n");
		} catch(e) {
			logMsg(LogClrAutomation, LogWarn, "testCaseBugWithCache: Caught Exception during deserialization ", e.description);
		}
	}
	if (!testCaseBugs) {
        if (_inTFS()) {
		    testCaseBugs = testCaseBugNewTFS(activeOnly);
        }
        else {
            testCaseBugs = testCaseBugNew(activeOnly, bugDb);
        }
		if (testCaseBugs) {
			logMsg(LogClrAutomation, LogInfo, "testCaseBugWithCache: caching data in ", cacheFile, "\n");
			xmlSerialize(testCaseBugs, cacheFile );
		}
	}
	
	return testCaseBugs;
}


/****************************************************************************/ 
/* Get an array of bug objects for all test bugs that are associated with the 
   automation failures.  We can then perform future queries in memory.

   It returns a structure that can be passed to the other testCaseBug* 
   functions

  parameters:
    activeOnly  - if true, only get active bugs
    branch      - build lab bug was created in (ie puclr, lab21s, etc), if branch
                  is not defined, then all bugs are returned (results not trimmed
                  to branch)
*/
function testCaseBugNewTFS(activeOnly, branch)
{
    logMsg(LogClrAutomation, LogInfo100, "testCaseBugNewTFS(", activeOnly,")\n");
    var ret = {};
    ret.queryTime = new Date();
    var byID = ret.byID = {};
    var byTest = ret.byTest = {};

    var dataset = _bugQueryTFS(ScriptDir + "\\scriptlib\\ddr_automatic_active_bugs.wiq", 5);

    if (!activeOnly) {
        var resolvedDataSet = _bugQueryTFS(ScriptDir + "\\scriptlib\\ddr_automatic_resolved_bugs.wiq", 6);

        dataset = dataset.concat(resolvedDataSet);
    }

    for (var i = 0; i < dataset.length; i++) { 
        var row = dataset[i]; 

        var bug = {}; 
        bug.ID = row[0];
        // skip header row
        if (bug.ID == "ID")
            continue;
        bug.Status = row[1];
        bug.AssignedTo = row[2];  // Note this is Display Name, not alias
        bug.TestCase = row[3];    // ID of the test case
        
        bug.Branch = row[4];

        if (row[5] != undefined)
            bug.ResolvedDate = row[5];

        if(nsIsNullOrEmpty(branch) || nsIsNullOrEmpty(bug.Branch) || (bug.Branch.toUpperCase() == branch.toUpperCase()))
        {
            byID[bug.ID] = bug;
            var testCasesStr = bug.TestCase;
            while(testCasesStr.match(/^[\s;,]*([^\s;,]+)/)) {
                // testCase is usually a number but let's use string as index just in case 
                // the test case ID could be either number or string
                var testCase = RegExp.$1.toLowerCase();
                testCasesStr = RegExp.rightContext;

                var bugs = byTest[testCase];
                if (!bugs) 
                    byTest[testCase] = bugs = [];
                                                
                bugs.push(bug);
            }
        }
    }
    
    // Sort bugs by resolved date, then ID
    for (i in byTest) 
    {
        byTest[i].sort(
            function(x, y) 
            { 
                if (x.Status == "Active")
                {
                    if (y.Status == "Active")
                    {
                        // lower/smaller IDs first
                        return x.ID - y.ID;
                    }

                    return -1;
                }
                
                if (y.Status == "Active") 
                    return 1;
                
                var time1 = new Date(x.ResolvedDate).getTime();
                var time2 = new Date(y.ResolvedDate).getTime();
                                 
                return  time2 - time1;                                                                  
            }
        );         
        
        //logMsg(LogClrAutomation, LogInfo, dump(byTest[i], 3));      
    }
    return ret; 
}

/****************************************************************************/ 
/* Get an array of bug objects for all test bugs that are associated with the 
   automation failures.  We can then perform future queries in memory.

   It returns a structure that can be passed to the other testCaseBug* 
   functions

  parameters:
    activeOnly  - if true, only get active bugs
    bugDb       - connection object for bug database (obtained via bugConnect)
    branch      - build lab bug was created in (ie puclr, lab21s, etc), if branch
                  is not defined, then all bugs are returned (results not trimmed
                  to branch)
*/
function testCaseBugNew(activeOnly, bugDb, branch) {
    if (bugDb == undefined) 
        bugDb = bugConnect(bugDatabase); 

    logMsg(LogClrAutomation, LogInfo100, "testCaseBugNew(bugDb, ", activeOnly, ")\n");
    var activeClause = ""
    if (activeOnly) {
        activeClause = "<Expression Column='Status' Operator='Equals'> " + 
                           "<String> Active </String> "  + 
                       "</Expression> ";
    }
    var queryXML =  "<Query> " + 
                        "<Group GroupOperator='and'> " + 
                            activeClause + 
                            "<Group GroupOperator='or'> " + 
                                "<Expression Column='TC ID' Operator='contains'> " + 
                                        "<String> " + "checkinbvt" + " </String> "  + 
                                "</Expression> "  + 
                                "<Expression Column='TC ID' Operator='contains'> " + 
                                        "<String> " + "buildbvt" + " </String> "  + 
                                "</Expression> "  + 
                                "<Expression Column='TC ID' Operator='contains'> " + 
                                        "<String> " + "devunit" + " </String> "  + 
                                "</Expression> "  +                                 
                                "<Expression Column='TC ID' Operator='contains'> " + 
                                        "<String> " + "fxcop" + " </String> "  + 
                                "</Expression> "  +    
                            "</Group>" + 
                        "</Group> " + 
                    "</Query> ";         

    var ret = {};
	ret.queryTime = new Date();
    var byID = ret.byID = {};
    var byTest = ret.byTest = {};

    var results = bugQuery(bugDb, queryXML); 


    for (var i = 0; i < results.length; i++) { 
        var result = results[i]; 

        var bug = {}; 
        bug.psBug = result;
        bug.ID = bugFieldGet(bugDb, result, "ID"); 
        bug.Status = bugFieldGet(bugDb, result, "Status"); 
        bug.AssignedTo = bugFieldGet(bugDb, result, "Assigned to"); 
        bug.TestCase = bugFieldGet(bugDb, result, "TC ID"); 
        bug.Branch = bugFieldGet(bugDb, result, "Opened Branch");
        
        if((branch == undefined) || (bug.Branch.toUpperCase() == branch.toUpperCase()))
        {
            if (bug.Status != "Active")
            {
                bug.ResolvedDate = bugFieldGet(bugDb, result, "Resolved Date");
            }
            
            
            byID[bug.ID] = bug;
            var testCasesStr = bug.TestCase;
            while(testCasesStr.match(/^[\s;,]*([^\s;,]+)/)) {
                // testCase is usually a number but let's use string as index just in case 
                // the test case ID could be either number or string
                var testCase = RegExp.$1.toLowerCase();
                testCasesStr = RegExp.rightContext;

                var bugs = byTest[testCase];
                if (!bugs) 
                    byTest[testCase] = bugs = [];
                                                
                bugs.push(bug);
            }
        }
    }
    
    // Sort bugs by resolved date
    for (i in byTest) 
    {
        byTest[i].sort(
            function(x, y) 
            { 
                
                if (x.Status == "Active")
                {
                    return -1;
                }
                
                if (y.Status == "Active")
                {
                    return 1;
                }
                
                var time1 = new Date(x.ResolvedDate).getTime();
                var time2 = new Date(y.ResolvedDate).getTime();
                                 
                return  time2 - time1;                                                                  
            }
        );         
        
        //logMsg(LogClrAutomation, LogInfo, dump(byTest[i], 3));      
    }
    return ret; 
}

/*****************************************************************************/
function readFromXml(fileName, topTag, defaultValue) {

    var ret = defaultValue;
    if (FSOFileExists(fileName)) {
        var xmlRet = xmlRead(fileName);
        if (xmlRet && xmlRet[topTag]) 
            ret = xmlRet[topTag];
    }
    if (ret)
        ret.xmlFile = fileName;
    return ret;
}

/*****************************************************************************/
function testFailureKind(testFailure) {

    if (!testFailure.testCases)
        return { kind: "fixed", count: 0 };

    var testCase = testFailure.testCases[0];
    var testBuild = testCase.run_.build_;
    var testRunName = testCase.run_.name;

        // pretty, but slightly expensive logging
    if (logGetFacilityLevel(LogClrAutomation) >= LogInfo) {
        logMsg(LogClrAutomation, LogInfo, "testFailureKind: test ", testFailure.testname, " fails on : ", 
            joinMap(testFailure.testCases, " ", function(x) { return x.run_.build_.ID }), "\n");

        logMsg(LogClrAutomation, LogInfo, "testFailureKind: builds ");
        var ptr = testBuild;
        while(ptr != undefined) {
            logMsg(LogClrAutomation, LogInfo, ptr.ID, (ptr.runs[testRunName] ? "D " : " "));
            ptr = ptr.next_;
        }
        logMsg(LogClrAutomation, LogInfo, "\n");
    }

    var i = 0;
    var ret;
    for (;;) {
        
        // If passrate > 0%, it means this is a non deterministic failure
        if (testCase.passrate && testCase.passrate.match(/^(\d+)/) && RegExp.$1 > 0) {
            logMsg(LogClrAutomation, LogInfo, "test case passes on retry on build ", testBuild.ID, " => nondeterministic\n");
            ret = { kind: "nondeterministic" };
            break;
        }
        i++;
        
        // get to the next build where the failure is occuring
        testBuild = testBuild.next_;
        while(testBuild && !testBuild.runs[testRunName])        
            testBuild = testBuild.next_;
        
        // We reached the end of linked list of builds. The failure has not been fixed
        if (testBuild == undefined) {
            ret = { kind: "unfixed", count: i };
            break;
        }

        // We found the last occurrence of the bug and it is not our lastest build
        // This usually means the issue has been fixed
        if (i >= testFailure.testCases.length) {
            ret = { kind: "fixed", count: i };
            break;
        }

        // If testBuild doesn't match testCases[i], it means that
        // there were builds where the failure didn't happen, so it's
        // a non deterministic failure
        var testFailureBuild = testFailure.testCases[i].run_.build_;
        if (testBuild !== testFailureBuild) {
            ret = { kind: "nondeterministic" };
            break;
        }
    }

    return ret 
}

/*****************************************************************************/
/* get the list of submissions that could have cause 'testFailure'  
   It returns a structure which has the following fields.  This 
   list includes the build for 'testFailure' and all previous builds
   for which there is no data.  

        kind        - fix, unfixed, or non-deterministic 
        count       - for fixed or unfixed, how many builds have failed 
        preExisting - this failure is preexisting, no submission can be identified
        submissions - a list of testBuild structures
        submitters  - a list of user names associated with 'submissions 
*/      

function testFailureSubmissionInfo(testFailure) {
    
    var testRun = testFailure.testCases[0].run_;
    var testBuild = testRun.build_;
    var ret = testFailureKind(testFailure);

    ret.submissions = [];
    ret.submitters = [];
    for(;;) {
        ret.submissions.push(testBuild);
        ret.submitters = ret.submitters.concat(testBuild.submitters);       // add them to the list
        
        testBuild = testBuild.prev_;
        if (testBuild == undefined) {
            logMsg(LogClrAutomation, LogInfo, "testFailureSubmissionInfo: failure always existed\n");
            ret.preExisting = true;
            ret.submissions = [];
            ret.submitters = [];
            break;
        }
        logMsg(LogClrAutomation, LogInfo, "testFailureSubmissionInfo: have build ", testBuild.ID, "\n");

        if (testBuild.runs[testRun.name]) {     // did the previous run have results?
            break;
        }
    }

    ret.submitters = removeDups(ret.submitters);
    logMsg(LogClrAutomation, LogInfo, "testFailureSubmissionInfo: returning: ", dump(ret, 3), "\n");
    return ret;
}

/*****************************************************************************/
function testCaseBugByTest(testCaseBugs, testCaseID) {
    // testCaseID is usually a number but let's use string as index just in case 
    // the test case ID could be either number or string
    return testCaseBugs.byTest[testCaseID.toString().toLowerCase()];
}


/*****************************************************************************/
function sendRollingTestAlerts(openBugs, runDirsBase) {
    if (runDirsBase == undefined)
    {
        runDirsBase = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests";
    }
    
    if (openBugs == undefined)
    {
        openBugs = true;
    }
    
    var xmlCache = runDirsBase+"\\testProcess.xml";
    
    // delete cached XML file
    logMsg(LogClrAutomation, LogInfo, "sendAlertsAutomated: Deleting old xml cache file ", xmlCache, "\n");
    try { FSODeleteFile(xmlCache, true) } catch(e) {};
        
    var mailDir = runDirsBase+"\\mail";    
    
    // Delete all the .mail.html files    
    var reports = FSOGetFilePattern(mailDir , /(.*)\.mail\.html$/);    
    for(var i = 0; i < reports.length; i++) 
    {
        logMsg(LogClrAutomation, LogInfo, "deleting report ", reports[i], "\n");
        FSODeleteFile(reports[i], true);
    }
                
    var mails = FSOGetFilePattern(mailDir , /(.*)\.sent\.htm$/);    
    var maxChange = 0;
    var newestMailIndex = -1;
    for(var i = 0; i < mails.length; i++) 
    {   
        if (mails[i].match(/(\d+)\.(.*)/))
        {
            var change = RegExp.$1;            
            logMsg(LogClrAutomation, LogInfo, "found mail file: CL ", change, "\n");
            
            if (change > maxChange)
            {
                maxChange = change;
                newestMailIndex = i;
            }
        }
    }               
    
    logMsg(LogClrAutomation, LogInfo, "Max CL # ", maxChange, "\n");                
        
    for(var i = 0; i < mails.length; i++) 
    {
        if (newestMailIndex != i)
        {
            logMsg(LogClrAutomation, LogInfo, "sendAlertsAutomated: Deleting old mail file ", mails[i], "\n");
            FSODeleteFile(mails[i], true);
        }
        else
        {
            logMsg(LogClrAutomation, LogInfo, "sendAlertsAutomated: Latest mail file: ", mails[i], "\n");
        }
    }
    
    // Open the bugs and generate new mails
    sendAlerts(openBugs)
    
    
    var newMails = FSOGetFilePattern(mailDir , /(.*)\.mail\.html$/);
    logMsg(LogClrAutomation, LogInfo, "Max CL # ", maxChange, "\n");                
            
    for(var i = 0; i < newMails.length; i++) 
    {   
        
        if (newMails[i].match(/(\d+)\.(.*)/))
        {
            var change = RegExp.$1;            
            
            logMsg(LogClrAutomation, LogInfo, "candidate for mail: CL ", change, "\n");
            
            if (change > maxChange)
            {
                logMsg(LogClrAutomation, LogInfo, "sending mail for: CL ", change, "\n");
                sendAlertMail(newMails[i]);
            }            
        }
    }     
}


/*****************************************************************************/
function sendAlerts(openBugs, runDirsBase) {
           
    // TODO: fix this for DevDiv Bugs
    // This is currently broken because testCaseBugNew expects the database schema
    // of DevDiv Bugs not VSWhidbey.
    // It appears that this rolling system hasn't been used since 2/20/2006, so
    // perhaps this should all be deleted instead of being migrated.
           
    if (openBugs == undefined)
    {
        openBugs = false;
    }
    else
    {
        openBugs = true;
    }
    
    if (runDirsBase == undefined)
        runDirsBase = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests";
    
    var bugs = testCaseBugNew(false, bugDatabase, rollingBranch);
    
    logMsg(LogClrAutomation, LogInfo, "sendAlerts: Got bugs ", dump(bugs.byTest), "\n");
    
    var testProcess = testProcessGet(runDirsBase);
    //logMsg(LogClrAutomation, LogInfo, "sendAlerts: Got testProcess ", dump(testProcess, 2), "\n");

    
        // The testProcess structure as already grouped failures by id (testRunName:testid)
        // however we want to group together failures that occur together since they are
        // likely to have the same root cause.   Thus go through all the failures and
        // create 'byBuilds' which maps a list of builds that failed to tests 

    var byBuilds = {};
    for (var testFailureName in testProcess.failures) {
        var testFailure = testProcess.failures[testFailureName];

        var bugsForTestCase = testCaseBugByTest(bugs, testFailure.testid)
        if (bugsForTestCase) {
            if (bugsForTestCase[0].Status == "Active")
            {
                var bugDb = bugConnect(bugDatabase);
                var bugData = bugGetById(bugDb, bugsForTestCase[0].ID);
                // TODO: convert this to DevDiv Bugs schema
                var issueType = bugFieldGet(bugDb, bugData, "Issue Type");
                var testTool = bugFieldGet(bugDb, bugData, "Test Tool");
                if((issueType.toLowerCase() != "code defect") && (testTool.match(/ddr -/i) != null))
                {
                    var submitInfo = testFailureSubmissionInfo(testFailure);
                    var descriptionText = "Automated Rolling Test Entry: This bug was found in a rolling run and has been identified as a code defect.  "
                    descriptionText    += "The issue type and priority have been updated to reflect this change.";
                    descriptionText    += " Failures were probably caused by one of the following submissions: ";
                    for(var j = 0; j < submitInfo.submissions.length; j++) 
                    {            
                        descriptionText += submitInfo.submissions[j].name + " ";
                    }

                    bugEdit(bugData, "edit");
                    bugFieldSet(bugDb, bugData, "Issue Type", "Code Defect");
                    bugFieldSet(bugDb, bugData, "Description", descriptionText);
                    bugSave(bugData);

                }

                logMsg(LogClrAutomation, LogInfo, "sendAlerts: skipping test ", testFailure.testid, " because it has an active bug ", bugsForTestCase[0].ID, "\n");         
                continue;
            }           
            else
            {
                // bugs are sorted (active bugs go first, then sorted by resolved date). If bug was resolved after the last failure we've
                // seen, we dont want to report the failure
                resolvedDate = new Date(bugsForTestCase[0].ResolvedDate);
                
                latestTestFailure = testFailure.testCases[testFailure.testCases.length - 1];
                latestTestFailureBuildID = latestTestFailure.run_.build_.ID;
                lastTestFailureCheckinDate  = new Date(testProcess.build[latestTestFailureBuildID].endtime);
                                
                if (resolvedDate > lastTestFailureCheckinDate)
                {
                    logMsg(LogClrAutomation, LogInfo, "sendAlerts: skipping test ", testFailure.testid, " because it has been marked as fixed: ", bugsForTestCase[0].ID, "\n");         
                    continue;
                }                                     
                else
                {
                    logMsg(LogClrAutomation, LogInfo, "sendAlerts: a failure happened for ",  testFailure.testid," on ", lastTestFailureCheckinDate, 
                            " after most recent bug with this test case (",
                            bugsForTestCase[0].ID,") was resolved (", resolvedDate,")\n");          
                }               
            }
        }

        var buildStr = ""
        for (var i = 0; i < testFailure.testCases.length; i++) {
            var testCase = testFailure.testCases[i];
            buildStr += testCase.run_.build_.ID + ";"
        }
        var buildEntry = byBuilds[buildStr];
        if (!buildEntry) 
            buildEntry = byBuilds[buildStr] = [];
        buildEntry.push(testFailure);
    }
    //logMsg(LogClrAutomation, LogInfo100, "sendAlerts: Got byBuilds ", dump(byBuilds, 4), "\n");

        // OK we have grouped them together, go through the groups.
    for(var i in byBuilds) {
        var testFailures = byBuilds[i];
        logMsg(LogClrAutomation, LogInfo, "sendAlerts: Processing failures that repro on ", i, " {\n");
        logMsg(LogClrAutomation, LogInfo, "Failing tests {\n", joinMap(testFailures, "\n", function(x) { return x.testname+" ("+x.testid+")"; }), "\n}\n");
        
        if (!testFailures) continue;
        
        var submissionInfo = testFailureSubmissionInfo(testFailures[0]);    // This info does not change per failure

        var mailDir = runDirsBase + "\\mail";
        FSOCreatePath(mailDir);
        var lastTestCaseFailure = testFailures[0].testCases[testFailures[0].testCases.length-1];
        var fileName = mailDir + "\\" + lastTestCaseFailure.run_.build_.ID + "." + 
                   submissionInfo.kind + "." + 
                   testFailures[0].runname + "." + 
                   testFailures[0].testname + ".mail.html";
    
        sendMailForFailures(testFailures, submissionInfo, testProcess, fileName);
        
        if (openBugs)
        {
            openBugsForFailures(testFailures, submissionInfo, testProcess, fileName);
        }

        logMsg(LogClrAutomation, LogInfo, "}\n");
    }
}

/*****************************************************************************/
function openBugsForFailures(testFailures, submissionInfo, testProcess, bugAttachment) {
    
    // testFailures represents a single bucket of failures (ie, a set of tests that have failed in exactly the
    // same builds). We will create one bug for each of these buckets
    logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: opening bugs for test failures\n");
    
    
    var separateBugPerFailure = true;
    
    if (submissionInfo.count <= 1)
    {
        logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: Failures have only occured once, don't open a bug yet\n");                
        return;
    }

    if (submissionInfo.kind == "unfixed") 
    {
        if (!submissionInfo.preExisting)
        {
            // Looks like we can pin down these failures to one check in            
            logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: failures can be pointed to a checkin. Open bug against dev\n");
            separateBugPerFailure = false;            
        }
        else
        {
            // This is a preexisting failure, we can't blame any checkin. Testers should take a look
            logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: Preexisting failures. Open separate bug per issue\n");            
        }                
    }               
    else if (submissionInfo.kind == "nondeterministic")
    {
        // If we haven't seen the failure enough times, don't open a bug (involved parties will still get mail, though
        var instances = testFailures[0].testCases.length;
        
        if (instances < 5)
        {
            logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: non deterministic failures (", instances, " unstances) don't meet yet threshold for bug\n");                
            return;                                    
        }
        
        logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: non deterministic issue meets threshold for bug (", instances, " instances). Open separate bug per issue\n");
    }
    else
    {
        if (submissionInfo.kind == "fixed")
        {
            logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: Fixed issue, no need to open a bug\n");
            return;
        }
        else
        {
            logMsg(LogClrAutomation, LogInfo, "openBugsForFailures: Bug heuristics are missing something, we shouldn't get here\n");
            return;
        }        
    }
    
    var lastTestCaseFailure = testFailures[0].testCases[testFailures[0].testCases.length-1];
    var buildID = lastTestCaseFailure.run_.build_.ID; 
        
    // We have to decide if we open a bug per failure or if we open only one bug. We will open only one bug
    // if we can narrow down the failures to one checkin            
    if (separateBugPerFailure == true || (submissionInfo.submitters[0] == undefined))
    {
        for(var j = 0; j < testFailures.length; j++) 
        {
            var testFailure = testFailures[j];
            
            var testFailureCollection = [];
            testFailureCollection.push(testFailures[j]);
            
            var testOwners     = getAllTestOwners(testFailureCollection);
            var bugTitle       = "Rolling test build " + buildID  + ": failure for test " + testFailure.testname;        
            var bugOpenedBy    = testOwners[0];
            var bugAssignTo    = testOwners[0];
            var bugTestCase    = testFailure.testid;
            
            var bugDescription = "";
            bugDescription += bugTitle + "\n";
            bugDescription += "This test has been failing in the rolling test runs. Our system has been unable to associate the failures ";
            bugDescription += "to any checkin, thus we are asking you as the owner of the test to do an initial investigation.\n";
            bugDescription += "If the investigation is going to take more than one day, please disable the tests, as its important for ";
            bugDescription += "the devs and the rolling builds that we are passing cleanly devbvt.\n";
            bugDescription += "You can find all relevant information about the failures in the attached file.\n";
            bugDescription += "\n";        
            bugDescription += "Please send feedback to dnotario@microsoft.com";        
        
            rollingMakeBug(
                bugTitle, 
                bugOpenedBy,
                bugAssignTo,
                bugTestCase, 
                bugDescription,
                bugAttachment);              
        }    
    }
    else
    {
        var bugTitle    = "Rolling test build " + buildID  + ": Failures probably caused by one of the following submissions: ";
        
        for(var j = 0; j < submissionInfo.submissions.length; j++) 
        {            
            var testBuild = submissionInfo.submissions[j];
            bugTitle += testBuild.name + " ";
        }
                
        var bugOpenedBy = submissionInfo.submitters[0]; // For the moment, we will open the bug on behalf of the developer. TODO: anything better here?                       
        var bugAssignTo = submissionInfo.submitters[0]; // If there is one than more submitter, we will just have to choose one
                      
        // Build test case string
        var bugTestCase = "";
        for(var j = 0; j < testFailures.length; j++) 
        {
            var testFailure = testFailures[j];          
            bugTestCase = bugTestCase + testFailure.testid + ";";
        }        
        var bugDescription = "";
        bugDescription += bugTitle + "\n";
        bugDescription += "If there has been more than one submission, the bug has been assigned randomly to one of the submitters, ";
        bugDescription += "please reassign as appropiate\n";
        bugDescription += "You can find all relevant information about the failures in the attached file\n";
        bugDescription += "\n";        
        bugDescription += "Please send feedback to dnotario@microsoft.com";        
        
        // We're set, go ahead and open the bug
        rollingMakeBug(
        bugTitle, 
        bugOpenedBy,
        bugAssignTo,
        bugTestCase, 
        bugDescription,
        bugAttachment);              
    }   
    
   
}

/*****************************************************************************/
// Returns a list of the test owners for a set of failures

function getAllTestOwners(testFailures)
{
    var allTestOwners = [];
        
        
    for(var j = 0; j < testFailures.length; j++) {
        var testFailure = testFailures[j];
        
        logMsg(LogClrAutomation, LogInfo, "Original owner: ", testFailure.testowner, "\n");
        var testOwners = testFailure.testowner.split(/;/);
        
        for(var i = 0; i < testOwners.length; i++) {
        
            if (allTestOwners[testOwners[i]] == undefined)
            {
                allTestOwners[testOwners[i]] = true;
            }        
        }
    }
        
    allTestOwners = keys(allTestOwners);
    for(var i = 0 ; i < allTestOwners.length ; i++) {
        logMsg(LogClrAutomation, LogInfo, "Owner: ", allTestOwners[i], "\n");
    }
                
    return allTestOwners;
}

/*****************************************************************************/


/*****************************************************************************/
function sendMailForFailures(testFailures, submissionInfo, testProcess, fileName) {

    var testCasesWithDumps = [];
    for(var j = 0; j < testFailures.length; j++) {
        var testFailure = testFailures[j];
        for (i in testFailure.testCases) {
            if (testFailure.testCases[i].dmprpt)
                testCasesWithDumps.push(testFailure.testCases[i]);
        }
    }

    var testOwners = "";
    var testOwnersCollection = getAllTestOwners(testFailures);
    
    for (var j = 0 ; j < testOwnersCollection.length ; j++)
    {
        testOwners += testOwnersCollection[j] + "@microsoft.com;";
    }
    
    var submissionsStr = submissionInfo.submissions == 1 ?  submissionInfo.submissions[0].name : "";
    var ccList = "";

    var toList;
    var subject;
    var failuresSubject = joinMap(testFailures, ",", function(x) { return x.testname; })        
    var desc = "";
    if (submissionInfo.kind == "unfixed" && !submissionInfo.preExisting) {
        if (submissionInfo.count == 1) {
            toList = submissionInfo.submitters;
            subject = "Rolling Test Warning! "+failuresSubject+" failing in rolling tests after checkin " + submissionsStr;

            desc += "The following tests\r\n";
            desc += testFailureTable(testFailures);
            desc += "have failed in the in rolling test automation after the submission(s)\r\n";
            desc += testSubmittersList(submissionInfo);

            desc += "This failure has not occured any time in the recient past.\r\n";
            desc += "Thus there are two possibilities\r\n";
            desc += "<OL>\r\n";
            desc += "   <LI> This is a non-deterministic failure and your build is the first to see it.\r\n";
            desc += "   <LI> Your submission(s) have introduced a regression.  \r\n";
            desc += "</OL>\r\n";
            desc += "The rolling build system will continue to run tests on subsequent builds, so in \r\n";
            desc += "a few hours we will know if the problem goes away (which implies (1)) or continues \r\n";
            desc += "to fail (which implies (2)).\r\n";
            desc += "<P>\r\n";
            desc += "In the mean time, it would be best if you took a look at the test case failure\r\n";
            desc += "data below and see if the failure could be related to your change. \r\n";

            desc += testInvestigationInfo(testFailures, testProcess);
        }
        else {
            subject = "Rolling Test Error! " + failuresSubject + ": " + submissionInfo.count + " consecutive failures after checkin " + submissionsStr;
            toList = submissionInfo.submitters;
            ccList = testOwners;

            desc += "The following tests\r\n";
            desc += testFailureTable(testFailures);
            desc += "have failed in the rolling test automation " + submissionInfo.count + " consecutive test runs.  The introduction of this failure\r\n";
            desc += "has been narrowed to the submission(s)\r\n";
            desc += testSubmittersList(submissionInfo);

            desc += "While it is still possible that this regression is caused by issues unrelated to these \r\n";
            desc += "checkin(s), it is clear that we need someone to follow up on the issue, and our policy\r\n";
            desc += "is that it should be one of the submitters on the list above.\r\n";
            desc += "<P>\r\n";
            desc += "<b>Thus you need to investigate this issue</b>, if only to determine who is the proper owner\r\n";
            desc += "of the issue.   To help drive home the point that you own this issue, you will be sent mail\r\n";
            desc += "on every subsequent failure of this test until:\r\n";
            desc += "<OL>\r\n";
            desc += "   <LI> The test stops failing (because you fixed it or it was non-deterministic).\r\n";
            desc += "   <LI> A VSWhidbey bug is created which has the following Test ID(s) in its TestCase field.\r\n";
            desc += "   <UL>\r\n";
            desc +=          joinMap(testFailures, " ", function(failure) { return "<LI> " + failure.testid + "\r\n" });
            desc += "   </UL>\r\n";
            desc += "</OL>\r\n";
            desc += "If you believe that it will take even a few hours to fix this failure, it is recommended \r\n";
            desc += "that you open a bug (or get the test owner to do it for you).\r\n";

            desc += testInvestigationInfo(testFailures, testProcess);
        }
    }
    else if ((submissionInfo.kind == "fixed" && submissionInfo.count == 1) || 
             (submissionInfo.kind == "unfixed" && submissionInfo.preExisting) ||
             (submissionInfo.kind == "nondeterministic")) {

        var preExist = (submissionInfo.kind == "unfixed" && submissionInfo.preExisting);
            
        subject = "ERROR: "+failuresSubject+ " is experiencing " + (preExist ? "preexisting" : "non-deterministic") + " failures in the rolling tests";
        toList = testOwners;

        desc += "As can be seen from the <a href=#history>failure history table</a> below, the test(s)\r\n";

        desc += testFailureTable(testFailures);

        if (preExist)
            desc += "are failing on all builds for which we have records.\r\n";
        else 
            desc += "are failing on some builds but not others.\r\n";
        desc += "Thus we can not attribute this failure to\r\n";
        desc += "a particular checkin.  As the test owner we need your help getting this issue \r\n";
        desc += "resolved.   We need you to do an initial investigation to determine whether this\r\n";
        desc += "failure is related to the test or the product.  If you believe it is the product\r\n";
        desc += "we need you open a bug and get it assigned to a developer who can look at it.  \r\n";

        if (testCasesWithDumps.length) {
            desc += "<P>\r\n";
            desc += "There are process dump report(s) associated these failure(s).\r\n";

            desc += testDumpReports(testCasesWithDumps);

            desc += "This is very good as it increases the chances of a successful diagnosis dramatically.\r\n";
            desc += "There is no need to try to repro the failure, as that rarely works.  The dump is\r\n";
            desc += "usually sufficient to diagnose the problem.   When it it not, the best strategy\r\n";
            desc += "is for a developer to add additional STRESS_LOG logging messages to the build and\r\n";
            desc += "and wait for another instance of the failure.\r\n";
            desc += "<P>\r\n";
            desc += "Thus a reasonable course of action is to determine if this failure has already\r\n";
            desc += "been reported, and if not, file a bug, using the dump report as a repro case. \r\n";
            desc += "This bug should not be a Pri-0 bug unless this failure is occuring frequently\r\n";
            desc += "If you do this, you need to copy the dump report (directions in the report)\r\n";
            desc += "because the automation will delete the dumps in a day or so to reclaim the space. \r\n";
        }
        else {
            desc += "<P>\r\n";
            desc += "This test failure did not result in the harness createing a dump report.  Sadly\r\n";
            desc += "it is unlikely that the test output will be sufficient to diagnose the \r\n";
            desc += "problem.   Thus while there is a failure here, it is not usually useful to create\r\n";
            desc += "a bug for it (unless you assign it to yourself).  \r\n";
            desc += "<P>\r\n";
            desc += "The first order of business is to modify the test so that when the failure is \r\n";
            desc += "detected, a process dump is taken.  Since the test\r\n";
            desc += "harness will automatically take a process dump after a timeout interval, the\r\n";
            desc += "simplest thing to do is to cause a messagebox popup.   If the failure was\r\n";
            desc += "detected because an exception was thrown the solution is even easier: <b>don't \r\n";
            desc += "catch the exception</b>.  This will cause the unhandled exception dialog to \r\n";
            desc += "pop up which will cause a timeout and thus cause a process dump to be taken. \r\n";
            desc += "<P>\r\n";
            desc += "If you need to make make changes to the test, it is important that you update\r\n";
            desc += "the copy that the rolling process actually uses.  The binaries for these are checked\r\n";
            desc += "into the product depot (SDPORT=DDRTSD:4000) at //depot/DevDiv/private/Lab21S/ddsuites/src/clr/...\r\n";
            desc += "Since these tests are also used by SNAP, updates to them should go through\r\n";
            desc += "SNAP. See http://teams/sites/clrdev/Documents/CLR%20Checkin%20Process.doc\r\n";
            desc += "for details on doing this \r\n";
            desc += "<P>\r\n";
            desc += "If you have any questions you can contact dnotario@microsoft.com.\r\n";
        }
        desc += "<P>\r\n";
        desc += "By default, you will be sent mail each time a non-deterministic failure \r\n";
        desc += "occurs.  You can silence these messages by creating a VSWhibey bug\r\n";
        desc += "which has the following Test ID(s) in its TestCase field.\r\n";
        desc += "<UL>\r\n";
        desc +=     joinMap(testFailures, " ", function(failure) { return "<LI> " + failure.testid + "\r\n" });
        desc += "</UL>\r\n";
        desc += "This will tell the automation that this test failure is being tracked and\r\n";
        desc += "not to send additional mail.\r\n";

        desc += testInvestigationInfo(testFailures, testProcess);
    }
    else {
        logMsg(LogClrAutomation, LogInfo, "sendMailForFailures: nothing to do.  SubmissionInfo = ", dump(submissionInfo), "\n");
        return;
    }

        // OK put everthing together to form some valid HTML
    var body = "";

        // These comments are used to capture information outside the body
    body += "<!--- to: " + toList +  " -->\r\n";
    body += "<!--- cc: " + ccList +  " -->\r\n";
    body += "<!--- subject: " + subject +  " -->\r\n";

        // Here is the body as far as HTML is concerned.
    body += "<HTML><BODY>\r\n";
    body += desc;
    body += "</BODY></HTML>\r\n";

    logMsg(LogClrAutomation, LogInfo, "Writing to ", fileName, "\n");
    FSOWriteToFile(body, fileName);
}

/*****************************************************************************/
function testDumpReports(testCasesWithDumps) {
    
    var body = "";
    body += "<UL>\r\n";
    for (var i in testCasesWithDumps) {
        var testCase = testCasesWithDumps[i];
        body += "<LI> <a href='" + testCase.dmprpt.dir  + "\\Report.html'>";
        body += "<b>Build:</b> " + testCase.run_.build_.ID + "<b> TestRun: </b>" + testCase.run_.name + "<b> TestCase: </b>" + testCase.name;
        body += "</a>\r\n";
    }
    body += "</UL>\r\n";
    return body;
}

/*****************************************************************************/
function testSubmittersList(submissionInfo) {
    
    var body = "";
    body += "<OL>\r\n";
    body += joinMap(submissionInfo.submissions, "", function(build) { return "<LI> " + build.name + "\r\n"});
    body += "</OL>\r\n";
    return body;
}

/*****************************************************************************/
function testFailureTable(testFailures) {
    
    var body = "";
    body += "<DL><DD>\r\n";
    body += "<TABLE BORDER>\r\n";
    body += "<TR> <TH> Test Name  </TH> <TH> Test ID </TH> <TH> Test owner </TH> </TR>\r\n";
    for (var i = 0; i < testFailures.length; i++) {
        var testFailure = testFailures[i];
        body += "<TR>";
        body += "<TD ALIGN=CENTER>" + testFailure.testname + "</TD>";
        body += "<TD ALIGN=CENTER>" + testFailure.testid + "</TD>";
        body += "<TD ALIGN=CENTER>" + testFailure.testowner + "</TD>";
        body += "</TR>\r\n";
    }
    body += "</TABLE>\r\n";
    body += "</DL>\r\n";
    return body;
}


/*****************************************************************************/
function testInvestigationInfo(testFailures, testProcess) {

    var body = "";

    var buildFailures = {};     // maps SD Change number (buildID) to a this test case failure if it failed
    var firstBuildToFail = undefined;
    var testRunNames = {};
    for (var i = 0; i < testFailures.length; i++) {
        var testFailure = testFailures[i];
        testRunNames[testFailure.runname] = true;

        for (var j = 0; j < testFailure.testCases.length; j++) {
            var failureTestCase = testFailure.testCases[j];
            var buildID = failureTestCase.run_.build_.ID;

            var buildFail = buildFailures[buildID]
            if (!buildFail)
                buildFail = buildFailures[buildID] = [];
            buildFail.push(failureTestCase);
            if (!(firstBuildToFail < buildID))
                firstBuildToFail = buildID;
        }
    }
    
    body += "<HR>\r\n";
    body += "<H3>Test Investigation information</H3>\r\n";

    body += "<A NAME=History>\r\n";
    body += "The automation is constantly running tests on different builds of the product.\r\n";
    body += "<a href='" + testProcess.report + "'> " + testProcess.name + " Report </a> is a overall summary of this process.  \r\n";
    body += "The table below is basically this same information that has been pruned to what is relevant to \r\n";
    body += "this failure.   The fields of this table are\r\n";

    body += "<UL>\r\n";
    body += "<LI><b>Snap Submission:</b> Each build that test are run on is identified with a\r\n";
    body += "particular SNAP submission that updated the code base.\r\n";
    body += "The developers associated with this submission are also identifed.   The link in this field provides\r\n";
    body += "information including the purpose of the submission, the files changed, the results of test runs \r\n";
    body += "done at submission time, as well as links to the builds themselves.\r\n";

    body += "<LI><b>Build:</b> This is the Source Depot change number associated with this build.\r\n";
    body += "All changes before this change number have been incorperated into the build that was tested.\r\n";
    body += "This link leads to a directoy that contains all the builds.\r\n"
    body += "To install this build, navagate to the proper &lt;buildArch&gt;&lt;buildType&gt;\\bin directory and run 'clrsetup.bat'.\r\n";

    body += "<LI><b>Test Run:</b> This indicates the name of the batch of tests run as well as the platform and build type.\r\n";
    body += "that was installed when the tests where run.\r\n";
    body += "This link leads to a report for the entire test run.\r\n";

    body += "<LI><b><font color=red>Test Case Failure:</font></b> This indicates the name of test that failed.\r\n";
    body += "This link leads to a report for the actual failure.  It is by far the most interesting field.\r\n";
    body += "The report associated with the first build that failed is indicated in red.\r\n";

    body += "<LI><b>Pass Rate:</b> SMARTY will rerun a test on failure.  If this field is present then the test was run\r\n";
    body += "multiple times and the rate of success is indicated.  \r\n";
    body += "Follow the 'Test Run' link if you wish to see the other instances of the run.\r\n";
    body += "</UL>\r\n";

    body += "<CENTER>\r\n";
    body += "<TABLE BORDER>\r\n";
    body += "<TR> <TH COLSPAN=6>Abbreviated history for <a href='" + testProcess.report + "'> " + testProcess.name + " </a> </TH></TR>\r\n";
    body += "<TR> <TH> SNAP Submission </TH> <TH> Build </TH> <TH> Time Built </TH> <TH> Test Run </TH> <TH> Test Case Failure</TH> <TH> Pass Rate </TH></TR>\r\n";

    var buildIDs = keys(testProcess.build).sort(function(x, y) { return y - x; });
    for (var i = 0; i < buildIDs.length; i++) {
        var failBuildID = buildIDs[i];
        var failBuild = testProcess.build[failBuildID];

        body += "<TR>";
        if (failBuild.report)
            body += "<TD ALIGN=CENTER><a href='" + failBuild.report + "'> " + failBuild.name + " </a></TD>\r\n";
        else 
            body += "<TD ALIGN=CENTER>" + failBuild.name + "</TD>\r\n";
        body += "<TD ALIGN=CENTER><a href='" + failBuild.jobDir + "'> " + failBuild.ID + "</a></TD>";
        body += "<TD ALIGN=CENTER>" + failBuild.starttime + "</TD>";

        var failureTestCases = buildFailures[failBuildID];
        var missingData = false; 
        if (failureTestCases) {
            body += "<TD ALIGN=CENTER> ";
            for (var j = 0; j < failureTestCases.length; j++) {
                var failureRun = failureTestCases[j].run_;
                body += "<a href='" + failureRun.report + "'> " + failureRun.name + " </a><BR>";
            }
            body += "</TD>";

            body += "<TD ALIGN=CENTER> ";
            for (var j = 0; j < failureTestCases.length; j++) {
                var testCaseFailure = failureTestCases[j];
                var failureTestCaseName = testCaseFailure.name
                if (failBuildID == firstBuildToFail)
                    failureTestCaseName = "<font color=red>" + failureTestCaseName + "</font>"
                body += "<a href='" + testCaseFailure.testlog + "'> " + failureTestCaseName + "</a><BR>";
            }
            body += "</TD>";

            body += "<TD ALIGN=CENTER> ";
            for (var j = 0; j < failureTestCases.length; j++) {
                var testCaseFailure = failureTestCases[j];
                if (testCaseFailure.passrate)
                    body += testCaseFailure.passrate + "<br>";
                else 
                    body += "-<br>";
            }
            body += "</TD>\r\n"
        }
        else {
            body += "<TD ALIGN=CENTER> ";
            for (var testRunName in testRunNames) 
                if (failBuild.runs[testRunName])
                    body += "<a href='" + failBuild.runs[testRunName].report + "'> " + testRunName + " </a><BR>";
                else {
                    missingData = true;
                    body += testRunName + "<BR>";
                }
            body += "</TD>\r\n"

            body += "<TD ALIGN=CENTER> ";
            for (var testRunName in testRunNames) {
                if (failBuild.runs[testRunName])
                    body += "These tests did not fail.<BR>";
                else 
                    body += "No Data<BR>";
            }
            body += "</TD>\r\n"

            body += "<TD ALIGN=CENTER>-</TD>\r\n";
        }

        if (failBuildID < firstBuildToFail && !missingData)     // only go one past the first introduced failure
            break;
    }
    body += "</TABLE>\r\n";
    body += "</CENTER>\r\n";
    body += "</A>\r\n";


    body += "<H3>Typical Investigation procedures</H3>\r\n";

    body += "Most investigations pretty much start the same way.  \r\n";
    body += "<OL>\r\n";
    body += "<LI>Follow a test case report link in the table above (typically the one in red).  \r\n";

    body += "<LI>Look at the test case report.  Hopefully you will see that the test 'MANUALLY KILLED' and a line\r\n";
    body += "indicating 'TEST_DUMPS' associated with the failure.  There will be a link to dump report in this case.\r\n";

    body += "<LI>Look at the dump report.  Commonly there is an assert that caused the test to stop\r\n";
    body += "and the dump report should give the message in the assert as well as the stack trace.  \r\n";

    body += "<LI>The dump report includes a link that will launch windbg, so that you an inspect the\r\n";
    body += "stack.  Note that most chk and ret builds are optimized which make mean you can't always\r\n";
    body += "trust local variables (look at the disassembly to confirm what is happening).  \r\n";

    body += "<LI>The dump report also includes the stress log, which logs interesting, relatively rare \r\n";
    body += "events within the runtime.  For example if an exception was thrown, its processing will be \r\n";
    body += "logged.  \r\n";

    body += "<LI>The SoS commands do work when inspecting the dump, and are quite valuable.\r\n"
    body += "<LI>Do a !SoS.Help in windbg to get more information.  For 64 bit runs \r\n";
    body += "They will only work if you are running windbg on the same kind of machine as the run.  \r\n";
    body += "</OL>\r\n";
    body += "If there is a dump, it is usually best to try to diagnose the problem from that.  \r\n";
    body += "If the failure repros, you can also try to install dbg build associated with the test run\r\n";
    body += "and try to reproduce the error.   This generally takes a long time, however. \r\n";
    body += "<P>\r\n";
    body += "If there is no dump, you typically can't diagnose the problem from the test output.  \r\n";
    body += "Please talk to the tester about modifying the test so that the test freezes on failure.\r\n";
    body += "If the problem seems to repro, you can install the runtime on another machine and \r\n";
    body += "try running the test under the debugger.  \r\n";

    return body;
}

/****************************************************************************/

function sendAlertMail(htmlMailFileName) {
    
    htmlMailFile = FSOOpenTextFile(htmlMailFileName, FSOForReading);
    var header = {};
    while (!htmlMailFile.AtEndOfStream) {
        var line = htmlMailFile.ReadLine();
        if (!line.match(/^<!--- (\w+):\s*(.*?)\s*-->/))
            break;
        header[RegExp.$1] = RegExp.$2;
    }
    var body = line + "\r\n" + htmlMailFile.ReadAll();
    htmlMailFile.Close();

    if (!header.to) {
        logMsg(LogClrAutomation, LogError, "sendMailFile: Could not find <--- to: .* --> line\n");
        return;
    }

    if (!header.subject) {
        logMsg(LogClrAutomation, LogWarn, "sendMailFile: Could not find <--- Subject: .* --> line\n");
        header.subject = "Unknown subject";
    }

    if (header.cc && !header.cc.match(/^\s*$/))
        header.cc += "; dnotario@microsoft.com; corchk@microsoft.com";
    else 
        header.cc = "dnotario@microsoft.com; corchk@microsoft.com";

    logMsg(LogClrAutomation, LogInfo, "sendMailFile: got header ", dump(header), "\n");
    mailSendHtml(header.to, header.subject, body, header.cc);
    FSOMoveFile(htmlMailFileName, htmlMailFileName + ".sent.htm");
    return 0;
}

/****************************************************************************/

function sendAlertMail(htmlMailFileName) {
    
    htmlMailFile = FSOOpenTextFile(htmlMailFileName, FSOForReading);
    var header = {};
    while (!htmlMailFile.AtEndOfStream) {
        var line = htmlMailFile.ReadLine();
        if (!line.match(/^<!--- (\w+):\s*(.*?)\s*-->/))
            break;
        header[RegExp.$1] = RegExp.$2;
    }
    var body = line + "\r\n" + htmlMailFile.ReadAll();
    htmlMailFile.Close();

    if (!header.to) {
        logMsg(LogClrAutomation, LogError, "sendMailFile: Could not find <--- to: .* --> line\n");
        return;
    }

    if (!header.subject) {
        logMsg(LogClrAutomation, LogWarn, "sendMailFile: Could not find <--- Subject: .* --> line\n");
        header.subject = "Unknown subject";
    }

    if (header.cc && !header.cc.match(/^\s*$/))
        header.cc += "; dnotario@microsoft.com; corchk@microsoft.com";
    else 
        header.cc = "dnotario@microsoft.com; corchk@microsoft.com";

    logMsg(LogClrAutomation, LogInfo, "sendMailFile: got header ", dump(header), "\n");
    mailSendHtml(header.to, header.subject, body, header.cc);
    FSOMoveFile(htmlMailFileName, htmlMailFileName + ".sent.htm");
    return 0;
}

/****************************************************************************/
function sendAllRollingAlertMail(mailDirectory)
{
    if (mailDirectory == undefined)
        mailDirectory = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests\\mail";
    
    var mailFiles = FSOGetFilePattern(mailDirectory, /(.*)\.html$/i);
    if (mailFiles.length == 0)
    {
        logMsg(LogClrAutomation, LogInfo, "sendAllRollingAlertMail: No files to mail in ", mailDirectory, "\n");    
    }

    for (var i = 0; i < mailFiles.length; i++) 
    {
            var mailFileName = mailFiles[i];
            logMsg(LogClrAutomation, LogInfo, "sendAllRollingAlertMail: sending mail for  ", mailFileName, "\n");    
            sendAlertMail(mailFileName);
    }               
}
    
/*****************************************************************************/
/* Shows the list of security reviewers. This used to send email automatically
   to the next reviewer when the reviewers were not required to be specific
   to the feature area affected by the change. However, now, the Dev is required
   to manually identify a security reviewer who would be most familiar
   with the affected feature area.
 */

function requestSecurityReview()
{
    try
    {
        launchIECmd("http://team/sites/clrdev/Lists/Security%20Reviewers%20list/AllItems.aspx");
    }
    catch(e)
    {
        logMsg(LogUtil, LogError, "Send email to ClrSecCR for problems requesting a security review.\n");
        throw e;
    }
    return 0;
}

/*****************************************************************************/
/*  Create a product studio bug for a deterministic or nondeterministic test 
    failure found in a daily dev run.
    
    Input: bugObject - an object with the following fields
                SyncTime - time the developer sync'd there source to before running 
                           daily dev run
                DatabaseName - Product Studio database name (ie DevDiv Bugs)
                    note that although this is configuable, the code assumed the 
                    database fields will be compatible with the DevDiv Bugs database.
                Title - title for the bug
                Priority - bug priority
                Severity - bug severity
                CreateDescription - bug description when bug is created
                UpdateDescription - bug description when adding an entry
                BuildLabel - build date label (ie 60203.00)
                BranchLabel - Branch label (ie "CLR Post Orcas")
                TestCaseID - ID of the test case. Usually come from smarty.xml and are numbers.
                TestOwner - owner of test
                FilesToAttach - array of filenames to attach to bug.
                IsDeterministic - boolean indicating whether bug was hit every time in the run
    Returns: Returns the bug number associated with this daily dev run failure or 
             null if no bug exists or is created.
*/
function _createOrLogDDRBug(bugObject)
{
    try
    {
        var deterministicThresholdHitcount = 5;   // if we see the failure 5 times, then we change the priority and/or turn it into a real bug.
        var activeOnly = false;
        var ret = {};
        if(bugObject == undefined)
            return;
 
        if(bugObject.DatabaseName == undefined)
            bugObject.DatabaseName = bugDatabase;
        
        // Check for existing bug
        var testCaseActiveBugs;

        var ddrBugDb;

        if (_inTFS()) {
           testCaseActiveBugs = testCaseBugNewTFS(activeOnly, bugObject.BranchLabel);
        }
        else {
            ddrBugDb = bugConnect(bugObject.DatabaseName);
            testCaseActiveBugs = testCaseBugNew(activeOnly, ddrBugDb, bugObject.BranchLabel);
        }
        // We used to match deterministic/non-deterministic as well, but that relied on the "Test Tool" field
        // which doesn't exist in DevDiv Bugs.  It's also probably not very valuable now that we're unlikely to
        // file bugs for non-deterministic failures.
        var existingBugs = testCaseBugByTest(testCaseActiveBugs, bugObject.TestCaseID);

        // if exists, use existing bug
        if(existingBugs)
        {
            // bugs are sorted (active bugs go first, then sorted by resolved date). We are interested in any 
            // active bug, or the bug resolved most recently.  So we just grab the first one.
            var existingBug = existingBugs[0];
            
            if(existingBug.Status == "Active")
            {
                logMsg(LogClrAutomation, LogInfo, "Found existing active bug for test case " + bugObject.TestCaseID + " in branch " + bugObject.BranchLabel + ".\n");

                if (_inTFS()) {
                    var cmd = TFScriptCall() + " addhitcount " + existingBug.ID;

                    for(var f in bugObject.FilesToAttach)
                        cmd += " " + bugObject.FilesToAttach[f];
                    runCmdToLog(cmd);
                }
                else {
                     // retrieve psDataItem of actual bug
                    var bugData = bugGetById(ddrBugDb, existingBug.ID);
                    bugEdit(bugData, "edit");

                    // bump hit count
                    var customField = bugFieldGet(ddrBugDb, bugData, "Custom");
                    var customData = parseKeyValueString(customField);
                    if (customData.DDR_Hits)
                        hitCount = parseInt(customData.DDR_Hits);                
                    else
                        hitCount = 1;
                    hitCount++;
                    
                    // For non-deterministic failures, to prevent random intermittent failures with rare
                    // occurence from eventually becoming a pri-1 bug and for deterministic failures, to separate
                    // possible product bug or high occurance test issue from rare test issue, we will
                    // periodically check the "Changed Date" of the bug and if the "Changed Date" is more than
                    // 'n' days old, we will reset the hitCount.
                    var changedDate = new Date(bugFieldGet(ddrBugDb, bugData, "Changed Date"));
                    var todaysDate = new Date();
                    var diffDays = (todaysDate.getTime() - changedDate.getTime()) / (1000 * 60 * 60 * 24);
                    
                    if ((bugObject.IsDeterministic && (diffDays > 1))
                        || (!bugObject.IsDeterministic && (diffDays > 14)))
                    {
                        hitCount = 1;
                    }
                    // update hit count
                    customData.DDR_Hits = hitCount;
                    bugFieldSet(ddrBugDb, bugData, "Custom", toKeyValueString(customData));
                    var bugPriority = bugFieldGet(ddrBugDb, bugData, "Priority");
                    
                    // We will start all DDR bugs as pri2 and if the hitcount > threshold within 
                    // the specified time window, then we will bump the priority to pri1. This is 
                    // true for both deterministic and non-deterministic bugs. 
                    if( bugPriority != "1" && hitCount >= deterministicThresholdHitcount) 
                    {
                        // Note that we don't want to reset other data here (like "Assigned To") because someone
                        // may have already started working on this bug. We also don't want to add the extra log
                        // details more than once so this is only done when not already at pri1.
                        logMsg(LogClrAutomation, LogInfo, "This bug has occurred more than " + 
                                deterministicThresholdHitcount + " times.  Its priority is being set to 1.\n");
                        bugFieldSet(ddrBugDb, bugData, "Priority", "1");
                        bugFieldSet(ddrBugDb, bugData, "Description", "Automated Daily Dev Run entry.\n\n"+
                            "This test failure has occured more than " + deterministicThresholdHitcount + " times " +
                            "(within a predefined time period), and so it's priority is being set to 1. High occurance " +
                            "of the same failure in different machines is usally an indication that this is likely due " +
                            "to a product bug. It is also possible that the test is broken as well.\n\n" +
                            bugObject.UpdateDescription);
                    }
                    else
                    {
                        bugFieldSet(ddrBugDb, bugData, "Description", "Automated Daily Dev Run entry.\n\n" + bugObject.UpdateDescription);
                    }
                    for(var f in bugObject.FilesToAttach)
                        bugAddFile(ddrBugDb, bugData, bugObject.FilesToAttach[f]);
                        
                    bugSave(bugData);
                }

                logMsg(LogClrAutomation, LogInfo, "Updated existing bug #" + existingBug.ID + "\n");
                return existingBug.ID;
            }
            else
            {
                // bugs are sorted (active bugs go first, then sorted by resolved date). If bug was resolved after 
                // the sync time for our daily dev run, then we dont want to report the failure
                var resolvedDate = new Date(existingBug.ResolvedDate);
                if(resolvedDate > bugObject.SyncTime)
                {
                    // bug resolved after daily dev run sync.  skip
                    logMsg(LogClrAutomation, LogInfo, "Bug exists in TFS for this test case which was resolved after your current sync time.  It is likely that this is the same issue, so a new bug is not being created.\n");
                    return null;
                }
                else
                {
                    // else, there is a bug for this test which was resolved previously and our bug is different,
                    // so create a new bug.
                    var bugID = _createNewDDRBug(bugObject);
                    logMsg(LogClrAutomation, LogInfo, "Created new Test Issue bug.  Bug #" + bugID + "\n");
                    return bugID;
                }
            }
        }
        else    // else create a bug
        {
            logMsg(LogClrAutomation, LogInfo, "Unable to find existing active bug for test case " + bugObject.TestCaseID + " in branch " + bugObject.BranchLabel + ", creating a new bug.\n");
            var bugID = _createNewDDRBug(bugObject);
            logMsg(LogClrAutomation, LogInfo, "Created new Test Issue bug.  Bug #" + bugID + "\n");
            return bugID;
        }
    }
    catch(e)
    {
        logMsg(LogClrAutomation, LogWarn, "Failure in _createOrLogDDRBug()\n");
        logMsg(LogClrAutomation, LogWarn, e.message + "\n");
        return null;
    }
}

/*****************************************************************************/
/*  Create a bug for a test failure found in a daily dev run.
    
    Input: bugObject - an object with the following fields
                SyncTime - time the developer sync'd there source to before running 
                           daily dev run
                DatabaseName - Product Studio database name (ie VSWhidbey)
                Title - title for the bug
                Priority - bug priority
                Severity - bug severity
                CreateDescription - bug description when bug is created
                UpdateDescription - bug description when adding an entry
                BuildLabel - build date label (ie 60203.00)
                BranchLabel - Branch label (ie "CLR Post Orcas")
                TestCaseID - ID of the test case.
                TestOwner - owner of test
                FilesToAttach - array of filenames to attach to bug.
                IsDeterministic - boolean indicating whether bug was hit every time in the run
           issueType - "Tracking", "Test Issue", "Code Defect", etc...  Default is "Test Issue"
    Returns: Returns the bug number of the created bug or null if unable to create a bug 
             given the bug object.
*/
function _createNewDDRBug(bugObject, issueType)
{
    if (_inTFS())
    {
        if (issueType == undefined)
            issueType = "Test Defect";

        return _createNewDDRBugTFS(bugObject, issueType);
    }
    else {
        try
        {
            if(issueType == undefined)
            {
                // We don't create bugs unless we've seen a baseline failure, so all bugs should be real
                // issues (test or product) that need investigating.  We can't tell if the problem is a
                // test issue or product issue, so we start by asking the test owner to look at it.
                issueType = "Test Issue";
            }

            var ddrBugDb = bugConnect(bugObject.DatabaseName);
            var bug = bugCreate(ddrBugDb);
            bugFieldSet(ddrBugDb, bug, "Title", bugObject.Title);

            bugFieldSet(ddrBugDb, bug, "Assigned to", bugObject.TestOwner);

            bugFieldSet(ddrBugDb, bug, "Opened by", bugObject.OpenedBy);
            bugFieldSet(ddrBugDb, bug, "Priority", bugObject.Priority);
            bugFieldSet(ddrBugDb, bug, "Severity", bugObject.Severity);
            bugFieldSet(ddrBugDb, bug, "Description", bugObject.CreateDescription);
            bugFieldSet(ddrBugDb, bug, "Issue type", issueType);
            bugFieldSet(ddrBugDb, bug, "Bug Type", "Product Bug");
            bugFieldSet(ddrBugDb, bug, "How found", "Test Pass");
            bugFieldSet(ddrBugDb, bug, "Source", "Development");
            bugFieldSet(ddrBugDb, bug, "Open Build", bugObject.BuildLabel );
            bugFieldSet(ddrBugDb, bug, "Opened Branch", bugObject.BranchLabel);
            bugFieldSet(ddrBugDb, bug, "Path", "Internal Tools and Processes\\RunTime\\DailyDevRun");
            bugFieldSet(ddrBugDb, bug, "TC ID", bugObject.TestCaseID);
            bugFieldSet(ddrBugDb, bug, "Custom", "DDR_Hits=1");
            bugFieldSet(ddrBugDb, bug, "Product Unit", "CLR");
            bugFieldSet(ddrBugDb, bug, "Source ID", Env("USERNAME"));
            bugFieldSet(ddrBugDb, bug, "Release", "CLR 3.0");
            bugFieldSet(ddrBugDb, bug, "Fix By", "M5");
            bugFieldSet(ddrBugDb, bug, "Blocking", "Not Blocking");
            
            for(var f in bugObject.FilesToAttach)
                 bugAddFile(ddrBugDb, bug, bugObject.FilesToAttach[f]);
            bugSave(bug);
            var bugID = bugFieldGet(ddrBugDb, bug, "ID")
            return bugID;
        }
        catch(e)
        {
            logMsg(LogTask, LogError, "Unable to create Product Studio bug\n");
            logMsg(LogTask, LogError, e.description + "\n");
            return null;
        }
    }
}

function _inTFS()
{
    var path = srcBaseFromScript()+"\\ndp\\clr\\bin\\TfsFlag.txt";
    return FSOFileExists(path);
}


/*****************************************************************************/
/*  Create a bug for a test failure found in a daily dev run.
    
    Input: bugObject - an object with the following fields
                SyncTime - time the developer sync'd there source to before running 
                           daily dev run
                Title - title for the bug
                Priority - bug priority
                Severity - bug severity
                CreateDescription - bug description when bug is created
                UpdateDescription - bug description when adding an entry
                BuildLabel - build date label (ie 60203.00)
                BranchLabel - Branch label (ie "$/Dev10/pu/clr")
                TestCaseID - ID of the test case.
                TestOwner - owner of test
                FilesToAttach - array of filenames to attach to bug.
                IsDeterministic - boolean indicating whether bug was hit every time in the run
           issueType - "Tracking", "Test Issue", "Code Defect", etc...  Default is "Test Issue"
    Returns: Returns the bug number of the created bug or null if unable to create a bug 
             given the bug object.
*/
function _createNewDDRBugTFS(bugObject, issueType)
{
    try
    { 
        var tmpPath = FSOGetTempPath();
        _tfsFieldSet(tmpPath, "System.Title", bugObject.Title);
        _tfsFieldSet(tmpPath, "Iteration Path", "DevDiv\\Dev11\\M1");
        _tfsFieldSet(tmpPath, "Area Path", "DevDiv\\NDP\\CLR");
        _tfsFieldSet(tmpPath, "Product", ".NET 4.5");
        _tfsFieldSet(tmpPath, "Opened Branch", bugObject.BranchLabel);
        _tfsFieldSet(tmpPath, "Source", "PU/Feature team"); 
        _tfsFieldSet(tmpPath, "Severity", bugObject.Severity);
        _tfsFieldSet(tmpPath, "Priority", bugObject.Priority);
        _tfsFieldSet(tmpPath, "Issue Type", issueType);
        _tfsFieldSet(tmpPath, "Build Number", bugObject.BuildLabel);
        _tfsFieldSet(tmpPath, "How Found", "Automated test run");
                         
        //TFS will not accept aliases.  Look up the Display Name
        var displayName = displayNameLookup(bugObject.TestOwner);
        if (displayName != null)
            _tfsFieldSet(tmpPath, "Assigned To", displayName);

        // Lookup the "Created By" user separately if needed.
        // We don't have permission to set "Created By" in TFS.  
        // We will add a note at the end of the description.
        var createdByDisplayName = bugObject.OpenedBy == bugObject.TestOwner ? displayName :
                                                         displayNameLookup(bugObject.OpenedBy);

        if (bugObject.BranchLabel.match(/^\$\/Dev10\/pu\/CLR/i) ||
            bugObject.BranchLabel.match(/^\$\/DevDiv\/pu\/CLR/i) ||
            bugObject.BranchLabel.match(/puclr/i))
        {
            _tfsFieldSet(tmpPath, "Bug Type", "Product Bug");
        } else {
            _tfsFieldSet(tmpPath, "Bug Type", "Feature Bug");
            _tfsFieldSet(tmpPath, "Feature ID", "0");
        }
        _tfsFieldSet(tmpPath, "TC Status", "Exists");
        _tfsFieldSet(tmpPath, "TC ID", bugObject.TestCaseID);
        _tfsFieldSet(tmpPath, "Custom 01", "DDR_Hits=1"); 
        _tfsFieldSet(tmpPath, "Regression", "Don't Know"); 

        // Argument after "new" is of the form <project>\<workItemType>
        var commandString = "tfpt workItem /new \"DevDiv\\Bug\" /fields:@" + tmpPath;

        var result = runCmdToLog(commandString);

        // result is of the form "Work item 375824 created."
        var id = null;
        if (result.output.match(/item (\d*) created/)) {
           id = RegExp.$1;

            //Success! Add description separately because it contains newline characters and possibly 
            //semicolons and other verboten as vaguely defined for tfpt.
            descTempPath = FSOGetTempPath();
            var text = bugObject.CreateDescription;
            
            // TFS expects HTML here and will run everything together unless we 
            // replace the newlines with break tags
            text = text.replace(/\n/g, "<br />\n"); 

            FSOWriteToFile(text, descTempPath, true);
            runCmdToLog(TFScriptCall() + " addDescription "+id+" "+descTempPath);
            FSOTryDeleteFile(descTempPath, true);

            bugAddFileTFS(id, bugObject.FilesToAttach);

            logMsg(LogClrProcess, LogInfo, "Successfully created bug ", id, " and assigned to ", displayName, "\n");
        }
        else
            logMsg(LogTask, LogError, "ERROR - Couldn't match return string");

        FSOTryDeleteFile(tmpPath, true);
        return id;
    }
    catch(e)
    {
        logMsg(LogTask, LogError, "Unable to create TFS bug\n");
        logMsg(LogTask, LogError, e.description + "\n");
        return null;
    }   
}

/*
 * Convert an alias (e.g. "billg") to a display name (e.g. "Bill Gates")
 */
function displayNameLookup(alias)
{
    var commandString = "usernameLookup "+alias;
    var result = runCmdToLog(commandString);

    // result is of the form "found:Sam Smith" when successful
    var displayName = null;
    if (result.output.match(/^found:(.*)\r\n/)) {
       displayName = RegExp.$1;
    }
    else
       logMsg(LogClrProcess, LogError, "ERROR - Couldn't look up user's display name for alias "+alias);
    return displayName;
}

// Each field/value pair is terminated by a semicolon.
// Escape sequences are "==" for '=' in a field name and ";;" for ';' in a value.
function _tfsFieldSet(filePath, name, val)
{
    FSOWriteToFile(name + "=" + val+";", filePath, true);
}


