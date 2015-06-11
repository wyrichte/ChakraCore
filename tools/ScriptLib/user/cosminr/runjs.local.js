eval(includeJS("user\\cosminr\\utils.js"));

/*
 * Constants used for Autobug processing
 */

var abugsrvr = "b11bgitwfrutl01";
var abugconn = "Provider='sqloledb';Data Source='" + abugsrvr + "';Initial Catalog='Autobug';Integrated Security='SSPI';";

var initialAssignedTo = "Cosmin Radu";

// specifies the fields requested from Autobug
var abugqrySel =
        " SELECT "
//	    + "DISTINCT(F.FailureID) "
	    + "F.FailureID "
	    + ",F.PSDatabase "
	    + ",ProblemClass "
	    + ",LEFT(szSymbol, 128) AS szSymbol "
// This is too slow, inlining the function call:
//	    + ",dbo.GetCHitsPerFailureId(F.FailureId) AS cHits "
	    + ", (SELECT     ROUND(SUM(hits), 0) AS Res FROM DailyFailureHits WITH (nolock) WHERE (FailureID = F.FailureID)) AS cHits"
	    + ",dbo.GetModuleVersionMaxByFailureId(F.FailureID) as ModVersion_Max "
	    + ",dbo.GetOSBuildMaxByFailureID(F.FailureId)  AS OSBuild_Max "
		+ ",dbo.AUTOBUG_GetRepresentativeCabPath(FailureID) as cabPath "
//	    + ",Comments "
//	    + ",ReproSteps "
	    + ",szExceptionCode "
	    + ",szModule "
	    + ",FileBug "
	    + ",iVertical "
	    + ",iFailureStatus "
	    + "FROM Failure AS F "
;

var abugqryReprocess =
        " INSERT INTO AutoCab.dbo.CabsToProcess ( "
        + "		RequestQueueDate,  "
        + "		GUID,  "
        + "		ReProcess,  "
        + "		DateCreated,  "
        + "		DateCracked,  "
        + "		Path,  "
        + "		iCab,  "
        + "		iBucketTable,  "
        + "		iBucket,  "
        + "		EventTypeName,  "
        + "		OsMajor,  "
        + "		X64Process "
        + ") "
        + "SELECT		GETDATE() AS RequestQueueDate,  "
        + "		C.GUID,  "
        + "		1 AS ReProcess,  "
        + "		C.DateCreated,  "
        + "		C.ENTRYDATE AS DateCracked,  "
        + "		C.PATH,  "
        + "		C.iCab,  "
        + "		C.iBucketTable,  "
        + "		C.iBucket,  "
        + "		C.EventTypeName,  "
        + "		C.OsMajor,  "
        + "		CASE  "
        + "			WHEN (EventTypeName LIKE '%64') THEN 1 "
        + "			ELSE 0 "
        + "		END AS X64Process "
        + "FROM AutoCab.dbo.vCab AS C INNER JOIN "
        + "		AutoBug.dbo.Cab AS AC ON AC.iCab = C.iCab INNER JOIN "
        + "		AutoBug.dbo.Failure AS F ON AC.FailureID = F.FailureID"
;

var modules =
{
//  clr: "('mscorwks.dll')",
    clr: "('mscorwks.dll', 'coreclr.dll', 'clr.dll')",
    bcl: "('mscorlib.ni.dll')",
    sys: "('System.ni.dll', 'System.Core.ni.dll')"
};

var abugfilters =
{
    sec:
    {
        sql:    " F.iVertical IN (4, 7) ",
        sqlExc: "          (ProblemClass NOT LIKE 'GS_FALSE_POSITIVE%') "
                + "    AND (ProblemClass NOT LIKE 'SOFTWARE_NX_FAULT%') "
                + "    AND (szSymbol NOT LIKE '%!Unknown') "
                + "    AND (szSymbol NOT LIKE '%System.Net.%') "
                + "    AND (FailureID NOT IN (735331, 5259146, 1767455, 1917882, 5596951, 6222373, 3405018))",
        jsExc:  function(f)
                {
                    return (   !f.ProblemClass.match(/GS_FALSE_POSITIVE/)
                            && !f.ProblemClass.match(/SOFTWARE_NX_FAULT/)
                            && !f.szSymbol.match(/!Unknown/)
                            && (f.FailureID != 735331));
                },
        topp:   95
    },
    nonsec:
    {
        sql:    " F.iVertical NOT IN (4, 7, 8, 10, 12) ",            // no hangs
        sqlExc: "          (szSymbol NOT LIKE '%!Unknown') "
                + "    AND (szSymbol NOT LIKE '%System.Net.%') "
                + "    AND (ProblemClass NOT LIKE 'APPLICATION_HANG%') "
                + "    AND (FailureID NOT IN (2931002, 3053834, 2894631, 2184802, "
                + "         2148574, 439611, 2902949, 686588, 1005509, 442327, 439531, "
                + "         1799162, 4938417, 1308519, 943258, 1657770, 5266926, 603828)) ",
        jsExc:  function(f) { return !f.szSymbol.match(/!Unknown/); },
        topp: 50
    }
};

// Excluded by FID:
// sec/clr:
//     735331,
//     5259146 - reprocessed, now empty          - mscorwks.dll!CrawlFrame::SetCurGSCookie
//     1767455 - ? reprocessed, now 2cabs/80hits - mscorwks.dll!_except_handler4
//     1917882 - ? reprocessed, now 1cabs/18hits - mscorwks.dll!CorLaunchApplication
//     5596951 - ? reprocessed, now 1cabs/19hits - mscorwks.dll!LegacyNGenCompile
//     6222373 - ? reprocessed, now 1cabs/75hits - mscorwks.dll!WKS::WaitForFinalizerEvent
//     3405018 - ? reprocessed, now 2cabs/37hits - mscorwks.dll!CrawlFrame::SetCurGSCookie
// nonsec/clr
//     2931002 - mscorwks.dll!NT5WaitRoutine - mistriaged
//     3053834 - mscorwks.dll!PreStubWorker  - mistriaged
//     2894631 - mscorwks.dll!CrstBase::Destroy - mistriaged
//     2184802 - mscorwks.dll!PreStubWorker  - mistriaged
//     2148574 - mscorwks.dll!NT5WaitRoutine - mistriaged
//     439611  - mscorwks.dll!PreStubWorker  - mistriaged
//     2902949 - mscorwks.dll!SafeExitProcess - com+boot exc in BCMWLTRY (the mfc80 state issue)
//     686588  - mscorwks.dll!FastCallFinalize - mistriaged (should go to managed code above)
//     1005509 - mscorwks.dll!GetManagedNameForTypeInfo - mistriaged (is all eHome/ehrec)
//     442327  - mscorwks.dll!CEEInfo::getArgType - rethrows exception: filenotfound setup...
//     439531  - mscorwks.dll!CEEInfo::findMethodInternal - rethrows: setup issues
//     1799162 - mscorwks.dll!FastCallFinalize - missing symbols for managed frames above
//     4938417 - mscorwks.dll!Thread::_vftable_ - mistriaged, most likely 64bit
//     1308519 - mscorwks.dll!FastCallFinalizeWorker - missing symbols for managed frames above
//     943258  - mscorwks.dll!PreStubWorker - mistriaged
//     1657770 - mscorwks.dll!NakedThrowHelper_RspAligned - x64 mistriage
//     5266926 - mscorwks.dll!EEClassHashTable::FindItemHelper - not hit since 11/2008
//     603828  - mscorwks.dll!FastCallFinalize - missing symbols for managed frames above
//     
//

var _xrpt_beg = "<html><head>"
    + "<style type='text/css'>"
    + "table.xrpt { font-family:Tahoma, sans-serif;font-size:10pt;border-style:ridge;border-width:1px;border-color:black; } "
    + "table.xrpt tr:hover { background-color: #ffeeee } "
    + "table.xrpt td { vertical-align:top; padding-top:1pt; padding-bottom:1pt; padding-left:3pt; padding-right:3pt } "
    + "</style>"
    + "</head><body>"
;

var _xrpt_end = "</body></html>";
/*
 * Utility functions
 */

function getJSCaller()
{
    var ftn = arguments.callee.caller.caller;
    var ftnname = null;

    if (ftn != null)
    {
        if (ftn.toString().match(/^\s*function\s+(\w+)\s*\(\s*(.*)\)/))
        {
            ftnname = RegExp.$1;
        }
    }

    return ftnname;
}

function get_failure(failureOrFID)
{
    var failure = failureOrFID;
    if (typeof (failureOrFID) != "object")
    {
        failure = getAutobugByFID(failureOrFID);
        logMsg(LogScript, LogInfo, "Failure info:\n" + dump(failure) + "\n");
    }
    if (failure == undefined || failure == null)
        throw new Error("Couldn't find failure.\n");

    return failure;
}

// Returns a Failure object corresponding to the argument passed in for failureID
function getAutobugByFID(fid)
{
	var qry  =
		abugqrySel
		+ "WHERE FailureID = " + fid;

	var failures = _ExecuteQry(abugconn, qry);
	// this query should always return at most one failure...
	if (failures.length < 1)
	    return null;
	else
    	return failures[0];
}

function getBugIDFromFID(fid)
{
    var bugID = -1;

    var wiql = "SELECT [System.Id] FROM WorkItems WHERE [Source ID] = 'FID=" + fid + "'";
    var cmdString = "tfpt query /format:tsv /include:data /wiql:\"" + wiql + "\"";
    var result = runCmdToLog(cmdString);
    if (result.output.match(/(\d+)/))
    {
        bugID = RegExp.$1;
    }
    return bugID;
}

function getFIDFromBugID(bugID)
{
    var fid = -1;

    var wiql = "SELECT [Source ID] FROM WorkItems WHERE [System.Id] = '" + bugID + "'";
    var cmdString = "tfpt query /format:tsv /include:data /wiql:\"" + wiql + "\"";
    var result = runCmdToLog(cmdString);
    if (result.output.match(/FID=(\d+)/))
    {
        fid = RegExp.$1;
    }
    return fid;
}

/****************************************************************************/
/* getAutobugIssues(issueType, mods, trimEarly, filterJsOrSql) 
   Retrieves the failures from Autobug, based on the issueType specified 
   (sec/nonsec), the mods (clr/bcl/sys), and trimEarly.

   Parameters:
      issueType: is a string either "sec" or "nonsec"

      mods:      is an ID for a list specified in T-SQL syntax: clr/bcl/sys 
                 see definition of global modules

      trimEarly: if true use filters[issueType].sqlExc in the WHERE clause, 
                 if false return everything, and trimming will be done later 
                 using filters[issueType].jsExc
            
      filterJsOrSql: may be an additional filter that will be added either 
                 to the WHERE SQL clause, or will be used as a JS filter 
                 function on the data returned from SQL

   Returns:
      Array of failure objects for issues hit in the specified list of modules.
       
   Example:
      runjs getAutobugIssues sec clr _ "g = function(f) { return f.cHits > 100; }"
*/
function getAutobugIssues(issueType, mods, trimEarly, filterJsOrSql)
{
    if (issueType != "sec" && issueType != "nonsec")
        throwWithStackTrace(Error("Illegal issueType argument"));

    if (mods == undefined)
        mods = "clr";

    if (trimEarly == undefined)
        trimEarly = true;

    var sfilter = new mfilter(filterJsOrSql);

    var qry =
		abugqrySel
	    + " WHERE  "
	    //     --- filter the appropriate issue type
		+      abugfilters[issueType].sql
		//     --- if we need to trim early add the SQL clause 
		+      (trimEarly ? (" AND " + abugfilters[issueType].sqlExc) : "")
		//     --- limit to the set of modules specified in the modules list
		+ "    AND szModule IN " + modules[mods]
		//     --- if a SQL filter was specified as an arg apply here
		//+      ((sfilter != null && sfilter.isSQL()) ? (" AND " + sfilter.SQL) : "")
		//+ "    -- AND FileBug = 1  "
		//+ "    -- AND iFailureStatus = 1 "
		+ "    AND iBug IS NULL "
	    + " ORDER BY  "
		+ "    cHits DESC, F.iVertical, F.FailureID ";

    logMsg(LogScript, LogInfo100, qry, "\n");

    var failures = _ExecuteQry(abugconn, qry);

    var ffailures = failures;
    if (sfilter != null && sfilter.isJS())
    {
        ffailures = arrayFilter(failures, sfilter.JS);
    }

    if (getJSCaller() == null)
        logMsg(LogScript, LogInfo, "Failures:\n" + dump(ffailures) + "\n");

    return ffailures;
}

function getAutobugSecurityIssues(mods, trimEarly, filterJsOrSql)
{
    return getAutobugIssues("sec", mods, trimEarly, filterJsOrSql);
}

/****************************************************************************/
/* Retrieves the top "percentage" failures from Autobug, based on the 
   issueType specified (sec/nonsec), the mods (clr/bcl/sys).

   Parameters:
      percentage: is the percentage to be used for deciding the "top" hits 

      issueType:  is a string either "sec" or "nonsec"

      mods:       is an ID for a list specified in T-SQL syntax: clr/bcl/sys 
                  see definition of global modules

      trimEarly:  if true use filters[issueType].sqlExc in the WHERE clause, 
                  if false return everything, and trimming will be done later 
                  using filters[issueType].jsExc

   Returns:
      Array of failures
*/
function getAutobugTopHits(percentage, issueType, mods, trimEarly)
{
    if (mods == undefined)
        mods = "clr";

    if (trimEarly == undefined)
        trimEarly = true;

    if (issueType != "sec" && issueType != "nonsec")
        throwWithStackTrace(Error("Illegal issueType argument"));

    logMsg(LogScript, LogInfo, "Getting Autobug top " + percentage + "% failures...{\n");

    var failures = getAutobugIssues(issueType, mods, trimEarly);

    // if we trimmed early we've already eliminated stuff we choose to ignore
    // otherwise we'll need to filter now, using abugfilters[issueType].jsExc

    logMsg(LogScript, LogInfo, "Count of failures: " + failures.length + ".\n");
    var sum = 0;
    var cnt = 0;
    for (var i = 0; i < failures.length; ++i)
    {
        if (!trimEarly)
        {
            if (!abugfilters[issueType].jsExc(failures[i]))
            {
                failures[i].Exclude = 1;
                continue;
            }
            failures[i].Exclude = 0;
        }
        ++cnt;
        sum += failures[i].cHits;
    }
    var targetSum = (sum * percentage / 100);
    logMsg(LogScript, LogInfo, "Relevant failures: ", cnt, ". Count of hits: ", sum, ". Target hits: ", targetSum, ".\n");
    var oSummer =
    {
        curSum: 0,
        tgtSum: targetSum,
        tester: function(val, idx, arr)
        {
            if (!trimEarly && val.Exclude) return false;
            var prev = this.curSum; this.curSum += val.cHits; 
            return prev <= this.tgtSum;
        },
        testerlog: function(val, idx, arr)
        {
            logMsg(LogScript, LogInfo100, dump(this) + "\n.tester(" + dump(val) + "\n)");
            return this.tester(val, idx, arr);
        }
    };

    var ffailures = arrayFilter(failures, oSummer.tester, oSummer);

    // Here failures contains all failures returned from SQL, w/ Exclude set to mark the failures that
    // shouldn't be considered; ffailures contains only the top percentage % of the non-Excluded failures

    if (getJSCaller() == null)
        logMsg(LogScript, LogInfo, dump(ffailures) + "\n");

    logMsg(LogScript, LogInfo, "} Done.\n");

    return ffailures;
}

function batext2html(text)
{
    if (FSOFileExists(text))
    {
        text = FSOReadFromFile(text);
    }

    // Eliminate all text between reporting the symbol path and the beginning of the analysis (marked by FAULTING_IP:)
    text = text.replace(/((Symbol Path:[^\r\n]*(\n|\r))([^\r\n]*(\n|\r))*(FAULTING_IP:))/m, "$2$6");

    // TFS expects HTML here and will run everything together unless we 
    // replace the newlines with break tags
    text = htmlescape(text);
    text = text.replace(/\n|\r/g, "<br/>\n");

    // highlight analysis labels
    text = text.replace(/^([A-Z_]+:)(.*)<br\/>$/gm, "<b>$1</b>$2<br/>");

    // highlight important  stuff
    text = text.replace(/^(<b>BUGCHECK_STR:<\/b>(.*)<br\/>)$/m,          "<font color='red'>$1</font>");
    text = text.replace(/^(<b>PRIMARY_PROBLEM_CLASS:<\/b>(.*)<br\/>)$/m, "<font color='red'>$1</font>");
    text = text.replace(/^(<b>SYMBOL_NAME:<\/b>(.*)<br\/>)$/m,           "<font color='red'>$1</font>");
    text = text.replace(/^(<b>FAILURE_BUCKET_ID:<\/b>(.*)<br\/>)$/m,     "<font color='red'>$1</font>");
    logMsg(LogScript, LogInfo1000, text);
    return "<div style='font-family:Consolas, monospace;font-size:10pt;'>" +
        text +
        "</div>";
}

// this function creates a file that can later be passed to "TFScriptCall() addDescription tfsID filename"
// to add an HTML description to an existing TFS workitem
function _createTfsBugDescForAutobugFailure(failureOrFID, descPath, cabPath) 
{
    var isTemp  = false;

    // handle arguments
    if (descPath == undefined)
    {
        descPath = FSOGetTempPath();
        isTemp = true;
    }

    logMsg(LogScript, LogInfo, "Creating bug description... {\n");

    var failure = get_failure(failureOrFID); 
    var fid = failure.FailureID;

    if (cabPath == undefined)
        cabPath = failure.cabPath;

    if (cabPath == null)
        throwWithStackTrace(Error("null cabPath for " + fid));

    var analysis = "";
    
    try
    {
        // for now always use the x86 debuggers
        analysis = cdbBangAnalyze(cabPath, "x86");
    }
    catch (exc) 
    {
        logMsg(LogTask, LogError, "Failure running cdb. ", exc.description, "\n");
    }

    FSOWriteToFile(
        "<div style='font-family:Tahoma, sans-serif;font-size:10pt;'>" +
        "Autobug failure report available at " + failureURL(fid) + ".<br><br>\n" +
        "Sample analysis for CAB file <a href='file://" + cabPath + "'>" + cabPath + "</a>.<br><br>\n" +
        "</div>", descPath, true);

    // TFS expects HTML in the Description field, so format the !analyze output
    FSOWriteToFile(batext2html(analysis), descPath, true);
    logMsg(LogScript, LogInfo, descPath, "\n");
    logMsg(LogScript, LogInfo, "} Done.\n");

    if (isTemp)
        FSOTryDeleteFile(descPath);

    return;
}

// this function creates a file that can later be passed to "tfpt workItem /new ..."
// to create a new bug
function _createTfsBugFieldsForAutobugFailure(failureOrFID, fieldsPath)
{
    var isTemp = false;

    // handle arguments
    if (fieldsPath == undefined)
    {
        fieldsPath = FSOGetTempPath();
        isTemp = true;
    }

    var failure = get_failure(failureOrFID);
    var fid = failure.FailureID;

    // constants
    var myBranch = "\$\/Dev10\/pu\/CLR";
    var bugSev = 1;
    var bugPri = 1;
    var iterationPath = "Dev10\\Dev10\\Beta1";

    // start setting the fields
    _tfsFieldSet(fieldsPath, "System.Title", "[Autobug] " + failure.ProblemClass + " in function: " + failure.szSymbol);
    _tfsFieldSet(fieldsPath, "Area Path", "Dev10\\NET Development Platform (NDP)\\CLR");
    _tfsFieldSet(fieldsPath, "Iteration Path", iterationPath);

    //TFS will not accept aliases.  Look up the Display Name
    // var assignedTo = displayNameLookup(failure.Owner);
    var assignedTo = initialAssignedTo;
    if (assignedTo != null)
        _tfsFieldSet(fieldsPath, "Assigned To", assignedTo);
    _tfsFieldSet(fieldsPath, "Issue Type", "Code Defect");
    if (failure.iVertical == 4 || failure.iVertical == 7)
    {
        _tfsFieldSet(fieldsPath, "Issue Level 01", "Security");
        _tfsFieldSet(fieldsPath, "Issue Level 02", "Other");
    }
    else
    {
        _tfsFieldSet(fieldsPath, "Issue Level 01", "Functional Defect");
    }
    _tfsFieldSet(fieldsPath, "Bug Type", "Product Bug");
    _tfsFieldSet(fieldsPath, "Severity", bugSev);
    _tfsFieldSet(fieldsPath, "Priority", bugPri);

    if (failure.ModVersion_Max.match(/^[2,4]\.0\.(.*)$/))
    {
        _tfsFieldSet(fieldsPath, "Build Number", RegExp.$1);
    }
    else
    {
        _tfsFieldSet(fieldsPath, "Build Number", failure.ModVersion_Max);
    }
    _tfsFieldSet(fieldsPath, "Opened Branch", myBranch);
    _tfsFieldSet(fieldsPath, "How Found", "Other");
    _tfsFieldSet(fieldsPath, "How Found Lv 01", "Autobug");
    _tfsFieldSet(fieldsPath, "Source", "Customer (Watson)");
    _tfsFieldSet(fieldsPath, "Source ID", "FID=" + fid);

    _tfsFieldSet(fieldsPath, "Regression", "Don't Know");

    _tfsFieldSet(fieldsPath, "Blocking", "Not Blocking");
    _tfsFieldSet(fieldsPath, "Custom01", "Hits=" + failure.cHits);
    // since the field below contains the character ';' we escape it by doubling them to become ';;'
    _tfsFieldSet(fieldsPath, "Custom02", failure.szSymbol + ";;" + failure.ProblemClass);
    // don't set the Tags field to include [Watson] yet
    //_tfsFieldSet(fieldsPath, "Tags", "[Watson]");
    _tfsFieldSet(fieldsPath, "TC Status", "Not Needed");

    var today = new Date();
    // since the field below contains the character ';' we escape it by doubling them to become ';;'
    _tfsFieldSet(fieldsPath, "Repro Steps",
        "<div style='font-family:Tahoma, sans-serif;;font-size:10pt;;'>" +
        "Autobug failure report available at " + failureURL(fid) + "<br>" +
        "Module affected - <b>" + failure.szModule + "</b><br>" +
        "Most recent module version - <b>" + failure.ModVersion_Max + "</b><br>" +
        "Estimated number of hits - <b>" + failure.cHits + "</b> as of " + today.toSDateString() + "</b><br>" + 
        "To request additional data fill in <a href='http://dgworker/simplereports/datawantedfailure.aspx'>this form</a><br>" +
        "</div>");

    if (isTemp)
        FSOTryDeleteFile(fieldsPath);

    return;
}

function createAncilaryTfsFilesForFailure(failureOrFID, viewDesc, subdir)
{
    if (viewDesc == undefined)
        viewDesc = true;

    if (subdir == undefined)
        subdir = "";

    logMsg(LogScript, LogInfo, "Creating ancillary TFS files... {\n");

    var failure = get_failure(failureOrFID);
    var fid = failure.FailureID;

    var fieldsPath = WshFSO.GetAbsolutePathName(subdir + "fid" + fid + "_fields.txt");
    var descPath   = WshFSO.GetAbsolutePathName(subdir + "fid" + fid + "_desc.htm");

    _createTfsBugFieldsForAutobugFailure(failure, fieldsPath);
    _createTfsBugDescForAutobugFailure(failure, descPath);

    // display the description file if requested...
    if (viewDesc)
        ViewHtml(WshFSO.GetAbsolutePathName(descPath), 950, 800);

    logMsg(LogScript, LogInfo, "} Done.\n");

    return [fieldsPath, descPath];
}

function createTfsBugFromFiles(fieldsPath, descPath, deleteFiles)
{
    if (deleteFiles == undefined)
        deleteFiles = true;

    logMsg(LogScript, LogInfo, "Creating TFS bug from files... {\n");

    var commandString = "tfpt workItem /new \"Dev10\\Dev10 Bug\" /fields:@" + fieldsPath;
    var result = runCmdToLog(commandString);

    // result is of the form "Work item 375824 created."
    var id = null;
    if (result.output.match(/item (\d*) created/))
    {
        id = RegExp.$1;

        //Success! Add description separately because it contains newline characters and possibly 
        //semicolons and other verboten as vaguely defined for tfpt.
        runCmdToLog(TFScriptCall() + " addDescription " + id + " " + descPath);

        logMsg(LogClrProcess, LogInfo, "Successfully created bug ", id, ".\n");

        // we only delete the files if the bug was successfully created and updated
        if (deleteFiles)
        {
            FSOTryDeleteFile(fieldsPath);
            FSOTryDeleteFile(descPath);
        }
    }
    else
    {
        logMsg(LogTask, LogError, "ERROR - Couldn't match return string");
    }
    logMsg(LogScript, LogInfo, "} Done.\n");
}

function createTfsBugFromFailureAndFiles()
{
    for (var i = 0; i < arguments.length;  ++i)
    {
        var fid = arguments[i];
        // logMsg(LogScript, LogInfo, fid, "\n");
        createTfsBugFromFiles("fid" + fid + "_fields.txt", "fid" + fid + "_desc.htm", false);
    }
}

function createTfsBugFromFailure(fid, keepTempFiles)
{
    var fieldsPath;
    var descPath;

    if (keepTempFiles == undefined)
        keepTempFiles = false;

    logMsg(LogScript, LogInfo, "Creating TFS bug from autobug failure... {\n");

    if (keepTempFiles)
    {
        fieldsPath = WshFSO.GetAbsolutePathName("fid" + fid + "_fields.txt");
        descPath   = WshFSO.GetAbsolutePathName("fid" + fid + "_desc.htm");
    }
    else
    {
        fieldsPath = FSOGetTempPath();
        descPath   = FSOGetTempPath();
    }

    var failure = getAutobugByFID(fid);
    var id = null;

    try
    {
        if (failure == null)
            throw new Error("Couldn't find failure " + fid + "\n");

        _createTfsBugFieldsForAutobugFailure(failure, fieldsPath);

        // Argument after "new" is of the form <project>\<workItemType>
        var commandString = "tfpt workItem /new \"Dev10\\Dev10 Bug\" /fields:@" + fieldsPath;
        var result = runCmdToLog(commandString);

        // result is of the form "Work item 375824 created."
        if (result.output.match(/item (\d*) created/))
        {
            id = RegExp.$1;

            //Success! Add description separately because it contains newline characters and possibly 
            //semicolons and other verboten as vaguely defined for tfpt.

            _createTfsBugDescForAutobugFailure(failure, descPath);
            // display the description file...
            ViewHtml(WshFSO.GetAbsolutePathName(descPath), 950, 800);

            runCmdToLog(TFScriptCall() + " addDescription " + id + " " + descPath);

            logMsg(LogClrProcess, LogInfo, "Successfully created bug ", id, " and assigned to ", displayName, "\n");
        }
        else
        {
            logMsg(LogTask, LogError, "ERROR - Couldn't match return string.\n");
        }

        if (!keepTempFiles)
        {
            FSOTryDeleteFile(fieldsPath, true);
            FSOTryDeleteFile(descPath, true);
        }

    }
    catch (e)
    {
        logMsg(LogTask, LogError, "Unable to create TFS bug\n");
        logMsg(LogTask, LogError, e.description + "\n");
    }

    logMsg(LogScript, LogInfo, "} Done.\n");
    return id;
}

function addNewTfsBugDescFromCab(bugID, cabPath)
{
    logMsg(LogScript, LogInfo, "Adding new dump analysis to bug " + bugID + "... {\n");
    var fid = getFIDFromBugID(bugID);
    if (fid != -1)
    {
        logMsg(LogScript, LogInfo, "Found corresponding failureID: ", fid, "\n");

        if (cabPath == undefined || cabPath == null)
        {
            // use the representative CAB for the failure ID
            cabPath = getAutobugByFID(fid).cabPath;
        }
        
        var descPath = WshFSO.GetAbsolutePathName("fid" + fid + "_desc.htm");
        _createTfsBugDescForAutobugFailure(fid, descPath, cabPath);

        // display the description file...
        ViewHtml(WshFSO.GetAbsolutePathName(descPath), 950, 800);

        runCmdToLog(TFScriptCall() + " addDescription " + bugID + " " + descPath);

        FSOTryDeleteFile(descPath);
        logMsg(LogClrProcess, LogInfo, "Successfully added analysis to bug ", bugID, "\n");
    }
    else
    {
        logMsg(LogScript, LogError, "Bug ", bugID, " does not have the Autobug failure ID specified in the Custom02 field, as expected.\n");
    }
    logMsg(LogScript, LogInfo, "} Done.\n");
}

// To invoke this from the command line:
//   runjs createTfsAncilaryFilesForTopFailures "f = function() { return getAutobugSecurityIssues(); }"
function createTfsAncilaryFilesForTopFailures(mods, issueType, percentage, trimEarly, subdir)
{
    if (issueType != "sec" && issueType != "nonsec")
        throwWithStackTrace(Error("Illegal issueType argument"));

    if (mods == undefined)
        mods = "clr";

    if (percentage == undefined)
        percentage = abugfilters[issueType].topp;

    if (trimEarly == undefined)
        trimEarly = true;

    if (subdir == undefined)
    {
        subdir = ".";
    }
    else if (subdir[subdir.length - 1] != '\\')
    {
        subdir = subdir + "\\";
    }

    logMsg(LogScript, LogInfo, "Creating TFS ancillary files for top ", percentage, "% ", issueType, 
           " failures (", mods, ")... {\n");

    var failures = getAutobugIssues(issueType, mods, trimEarly);
    if (failures == null || failures.length == 0)
    {
        logMsg(LogScript, LogInfo, "} Done.\n");
        return null;
    }

    var filedbugs = getFiledAutobugBugs();

    var sum = 0;
    for (var i = 0; i < failures.length; ++i)
    {
        // ignore external bugs
        if (getResolutionByFID(failures[i].FailureID, filedbugs) == "External")
            continue;
        sum += failures[i].cHits;
    }

    var cnt = 0;
    var rsum = 0;
    for (var i = 0; i < failures.length; ++i)
    {
        // ignore external bugs
        if (getResolutionByFID(failures[i].FailureID, filedbugs) == "External")
            continue;
        ++cnt;
        rsum += failures[i].cHits;
        // break out when the top failures add up to more than the percentage limit passed in
        if ((100 * rsum / sum) > percentage)
            break;
    }
    logMsg(LogScript, LogInfo, "Total hits: ", sum, " in ", failures.length, " failures. Top ", percentage, "% hits in ", cnt, " failures.\n");

    var summaryPath = WshFSO.GetAbsolutePathName(subdir + "index.htm");
    FSOWriteToFile(_xrpt_beg + "<div><h3>" + mods + ": Top " + percentage + "% Autobug " + issueType + " Issues</h3>\r\n",
        summaryPath, true);

    FSOWriteToFile("<table class='xrpt'>\r\n" +
        "<tr><th>Idx</th><th>FID</th><th>Title</th><th>Hits</th><th>Percent</th><th>ModVerMax</th><th>Analysis</th></tr>\r\n",
        summaryPath, true);
    var rsum = 0;
    var files = new Array(failures.length);
    var cnt = 0;
    for (var i = 0; i < failures.length; ++i)
    {
        logMsg(LogScript, LogInfo, "Handling failure ", dump(failures[i]), "\n");

        if (failures[i].cHits == 0)
            break;

        // ignore external bugs
        var resolution = getResolutionByFID(failures[i].FailureID, filedbugs);
        if (resolution == "External")
            continue;

        var bugnote = "none";
        var bug = filedbugs.f2b[failures[i].FailureID];
        var ancfiles = null;
        if (bug != null)
        {
            if (resolution != null && resolution != "")
                bugnote = "d(" + resolution + ")";
            else if (bug.Resolution != null && bug.Resolution != "")
                bugnote = bug.Resolution;
            else
                bugnote = bug.Status;
            bugnote = bugURL(bug.BugID) + " (" + bugnote + ")";
        }
        else
        {
            var indent = logTry();
            try
            {
                ancfiles = createAncilaryTfsFilesForFailure(failures[i], false, subdir);
            }
            catch (e)
            {
                logCatch(indent);
                logMsg(LogScript, LogInfo, e.description);
            }
        }
        var perc = 100 * failures[i].cHits / sum;
        ++cnt;
        FSOWriteToFile(
            "<tr><td>" + cnt +
            "</td><td>" + failureURL(failures[i].FailureID) + 
            "</td><td>" + failures[i].ProblemClass + " in function: " + failures[i].szSymbol +
            "</td><td>" + failures[i].cHits +
            "</td><td>" + perc.toFixed(1) + "%" +
            "</td><td>" + failures[i].ModVersion_Max +
            "</td><td>" + 
                ((ancfiles != null) ? "<a href='" + FSOGetFileName(ancfiles[1]) + "'>...</a>" : bugnote) +
            "</td></tr>\r\n",
            summaryPath, true);

        rsum += failures[i].cHits;
        files[cnt-1] = ancfiles;
        // break out when the top failures add up to more than the percentage limit passed in (report at least 3)
        if ((100 * rsum / sum > percentage) && (i >= 2))
            break;
    }

    FSOWriteToFile("</table></div>" + _xrpt_end, summaryPath, true);
    logMsg(LogScript, LogInfo, "} Done.\n");

    if (getJSCaller() == null)
        ViewHtml(summaryPath, 950, 1024);

    return files;
}

function createTfsBugsForTopFailures(percentage, failures)
{
    logMsg(LogScript, LogInfo, "Creating TFS bugs for top " + percentage + "% failures...{\n");

    // here we get all hits, and we'll use 'percentage' in the call to 
    // createTfsAncilaryFilesForTopFailures() below
    if (failures == undefined)
    {
        //failures = getAutobugTopHits(100, getAutobugIssues("1 = 1", false));
        failures = getAutobugIssues("1 = 1", false);
    }
    else if (typeof (failures) == "string")
    {
        var getfailures = eval(failures);
        if (typeof (getfailures) != "function")
            throwWithStackTrace(new TypeError());

        failures = getfailures();
    }

    if (failures == null || failures.length == 0)
        return;

    var files = createTfsAncilaryFilesForTopFailures(failures, percentage);
    logMsg(LogScript, LogInfo100, dump(files), "\n");

    for (var i = 0; i < files.length; ++i)
    {
        if (files[i] == undefined || files[i] == null)
            continue;

        // we already had a chance to examine the files. we can delete them now
        createTfsBugFromFiles(files[i][0], files[i][1], true);
    }

    logMsg(LogScript, LogInfo, "} Done.\n");
}

function doAbugTfsFiles()
{
    var tm = FSOTimeAsFileName().split("_")[0];
    var root = "abug_" + tm + "\\";
    FSOCreatePath(root);
    logSetTranscript(root + "abug_" + tm + ".log");
    for (mods in modules)
    {
        for (filt in abugfilters)
        {
            var subdir = root + filt + "." + mods + "\\";
            FSOCreatePath(subdir);
            // logMsg(LogScript, LogInfo, "createTfsAncilaryFilesForTopFailures(", abugfilters[filt].topp, ", ", filt, ", ", mods, ", ", true, ", ", subdir, ")\n");
            createTfsAncilaryFilesForTopFailures(mods, filt, abugfilters[filt].topp, true, subdir);
        }
    }
    logSetTranscript();
    FSOMoveFile("theLog.txt", root + "theLog.txt", "force");
}

function getFiledAutobugBugs()
{
    logMsg(LogScript, LogInfo, "Retrieving Autobug bugs already filed... {\n");
    var wiql = "SELECT [System.Id], [Source ID], [System.State], [Resolution], [Microsoft.PSM_Client_DuplicateTFSID] FROM WorkItems " 
             + " WHERE [System.TeamProject] = 'Dev10' " 
             + " AND  [System.AreaPath] UNDER 'Dev10\\NET Development Platform (NDP)\\CLR'  "
             + " AND  [Microsoft.VSTS.Dogfood.Source] = 'Customer (Watson)'  "
             + " AND  [Microsoft.VSTS.Phoenix.HowFound] = 'Other'  "
             + " AND  [Microsoft.DevDiv.HowFound_Lv_01] = 'Autobug'";
    var cmdString = "tfpt query /format:tsv /include:data /wiql:\"" + wiql + "\"";
    var tfptres = runCmd(cmdString);

    var output = tfptres.output.split('\r\n');
    var result = { f2b: [], b2f: [] };
    for (var i = 0; i < output.length; ++i)
    {
        if (output[i].match(/^(\d+)\tFID=(\d+)\t([^\t]+)\t([^\t]+)?\t(\d+)?$/))
        {
            var o = new Object;
            o.BugID = RegExp.$1;
            o.FailureID = RegExp.$2;
            o.Status = RegExp.$3;
            o.Resolution = RegExp.$4;
            o.DupTfsID = RegExp.$5;
            result.f2b[RegExp.$2] = o;
            result.b2f[RegExp.$1] = o;
        }
    }
    logMsg(LogScript, LogInfo100, dump(result), "\n");
    logMsg(LogScript, LogInfo, "} Done.\n");
    return result;
}

function getResolutionByFID(fid, autobugBugs)
{
    if (autobugBugs == undefined)
        autobugBugs = getFiledAutobugBugs();

    var bug = autobugBugs.f2b[fid];
    while (true)
    {
        if (bug == null)
            return "";
        if (bug.Resolution != "Duplicate")
            return bug.Resolution;
        if (bug.DupTfsID == "")
            return "";
        bug = autobugBugs.b2f[bug.DupTfsID];
    }
    return "";
}

function massageFields()
{
    if (arguments.length == 0)
    {
        var files = FSOGetFilePattern(".", "fid(.*)_fields\.txt", false);
        for (var i = 0; i < files.length;  ++i)
        {
            var descPath = files[i];
            logMsg(LogScript, LogInfo, descPath, "\n");
            massageFields(descPath);
        }
    }
    else
    {
        var descPath = arguments[0];
        var txt = FSOReadFromFile(descPath);
        var newtxt = txt.replace(/^(.*);Source ID=([^;]*);(.*);Custom01=([^;]*);Custom02=Abug_Hits=(\d+);;(FID=\d+);(.*)$/, 
                                 "$1;Source ID=$6;$3;Custom01=Hits=$5;Custom02=$4;;$2;$7");
        logMsg(LogScript, LogInfo1000, newtxt);
        FSOWriteToFile(newtxt, descPath, false);
    }
}

function getCabsToReprocessCount()
{
    var cnt = _ExecuteQry(abugconn, "SELECT COUNT(*) as Count FROM AutoCab.dbo.CabsToProcess");
    var res = cnt[0].Count;
    logMsg(LogScript, LogInfo, "Cabs to process: ", res, "\n");
    return res;
}

/*
 * Reprocessing
 */

function requestReprocessingByFID()
{
    if (arguments.length == 0)
        return;

    var fids = arguments[0];
    for (var i = 1; i < arguments.length; ++i)
        fids += ", " + arguments[i];
    var where = " WHERE (F.FailureID IN ( " + fids + "))";

    logMsg(LogScript, LogInfo, "Requesting Autobug reprocessing for [", fids, "] {\n");
    getCabsToReprocessCount();
    var res = _ExecuteNonQry(abugconn, abugqryReprocess + where);
    logMsg(LogScript, LogInfo, "Submitted ", res, " cabs to reprocess\n");
    getCabsToReprocessCount();
    logMsg(LogScript, LogInfo, "} Done.\n");
}

function requestBatchReprocessing(issueType, mods)
{
    if (issueType != "sec" && issueType != "nonsec")
        throwWithStackTrace(Error("Illegal issueType argument"));

    if (mods == undefined)
        mods = "clr";

    var where =
	    " WHERE  "
    //     --- filter the appropriate issue type
		+ abugfilters[issueType].sql
    //     --- limit to the set of modules specified in the modules list
		+ "    AND F.szModule IN " + modules[mods]
		+ "    AND F.szSymbol LIKE '%!Unknown' "
    //	+ "    AND F.szModule IN ('mscorwks.dll', 'coreclr.dll', 'clr.dll', 'mscorlib.ni.dll', 'System.ni.dll') "
    //	+ "    AND F.ProblemClass LIKE 'GS_FALSE_POSITIVE%' "
        ;
    logMsg(LogScript, LogInfo100, where, "\n");

    logMsg(LogScript, LogInfo, "Requesting Autobug reprocessing... {\n");
    getCabsToReprocessCount();
    var res = _ExecuteNonQry(abugconn, abugqryReprocess + where);
    logMsg(LogScript, LogInfo, "Submitted ", res, " cabs to reprocess\n");
    getCabsToReprocessCount();
    logMsg(LogScript, LogInfo, "} Done.\n");
}

/*
 * Additional Autobug queries
 */

/*
 * Get most recent CLR failures
 */

function getRecentFailures(days)
{
    if (days == undefined)
        days = 1;
    days = -days;
    var qry = abugqrySel
      + " WHERE " 
      + "   F.RecordInsertDate > dateadd(day, " + days + ", current_timestamp) "
      + "   AND F.szModule IN ('mscorwks.dll', 'coreclr.dll', 'clr.dll', 'mscorlib.ni.dll', 'System.ni.dll') "
      ;

    var recentFailures = _ExecuteQry(abugconn, qry);
    logMsg(LogScript, LogInfo, dump(recentFailures), "\n");
}

function getFailuresByProblemClass(pc)
{
    if (pc == undefined)
        pc = "%ExecutionEngineException%";

    qry = abugqrySel
      + " WHERE "
      + "   F.ProblemClass LIKE '" + pc + "' "
      + "   AND F.szSymbol NOT LIKE '%GlobalizationAssembly.CreateGlobalizationAssembly%' "
      + "   AND F.szModule IN ('mscorwks.dll', 'coreclr.dll', 'clr.dll', 'mscorlib.ni.dll', 'System.ni.dll') "
      + " ORDER BY "
      + "   cHits DESC, F.FailureID ";

    var failures = _ExecuteQry(abugconn, qry);
    logMsg(LogScript, LogInfo, dump(failures), "\n");
}
