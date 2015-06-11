//-------------------------------------------------------------------
// Script to create failure error report,
// groups failure instances by test-id across multiple test sources.
// 
// Updated for Arrowhead
//-------------------------------------------------------------------


var LogTestSummary = logNewFacility("TestSummary");

// primitive assert
function _Check(cond, hint)
{
    if (!cond)
    {
        WScript.Echo("### ASSERT FAILED:" + hint);
    }
}

function _htmlAnchor2(name) {
    return "<A HREF='" + name + "'>" + name + "</A>";
}

function _htmlAliasAnchor2(nameDisplay, url) {
    return "<A HREF='" + url + "'>" + nameDisplay + "</A>";
}

/****************************************************************************/ 
/* Given a database + bug id, return a url link to a bug
This takes advantage of an existing web-based query service
eg:  http://vbqa/raid/BugForm.aspx?sBugDB=VSWhidbey&lBugID=220109 
    parameters
        database - string name of the database.
        id - integer bug id.
*/
function _htmlBugLink(database, id)
{
    if (database == undefined)
        throw new Error("Need database parameter");

    if (id == undefined)
        throw new Error("Need id parameter");    
        
    return "http://vbqa/raid/BugForm.aspx?sBugDB="+database + "&lBugID=" + id;
}

/*****************************************************************************/
/*
Ge an alias link to the bug (just displays the bug id, not the whole url).
*/
function _htmlAliasBugLink(database, id)
{
    return _htmlAliasAnchor2(id, _htmlBugLink(database, id));
}

/*****************************************************************************/
/* Link to a bookmark 
*/
function _htmlBookmarkLink(nameDisplay, bookmark)
{
	return "<A HREF=#"+bookmark+">" + nameDisplay + "</A>";
}

/*****************************************************************************/
/* Target for a bookmark
*/
function _htmlBookmarkHere(bookmark)
{
	return "<A NAME='" + bookmark + "'>";
}


/*****************************************************************************/
/* Create a hashtable 
*/
function _newHashtable()
{
    var hash = WScript.CreateObject("Scripting.Dictionary");
    return hash;
}

/*
Have Point system to rank failures.
Principles:
- T(x) - depreciates failures as they age. T(x) = x * r^age. 0 < r < 1.
- Test Type: CheckinBVTs are really serious (10), then devuint (7), then buildbvt (5).
- Source: Recognize failures from ddr may be due to local changes. A single failure on 1 dev machine 
          is noise. The same failure on multiple dev machines is more serious.
        - "Real" - Rolling Build + successful snap jobs are the more serious then dev machines.
        - "Suspect" - Isolated DDR failures (only fail on 1 machine) are less points . If a DDR failure 
          occurs on 2 different machines, it's in the same class as a "real" failure.

Apply to a given FailTestGroup object to rank it.
*/

//-----------------------------------------------------------------------------
// Objects
//-----------------------------------------------------------------------------

/*
Ctor for FailTestGroup object.
This represents a single test and has entries for each failing instance.
*/
function _FailTestGroupCTOR(sGuid)
{
    this.cTotal = 0; // total number of reports having this failure
    this.stringTestGuid = sGuid; // guid for the test.

    // More information to identify & categorize the test.
    this.Categories = "";
    this.CmdLine = "";
    this.SDET = "";

    this.points = 0; // subjective ranking of "how bad is it". 

    this.instances = new Array(); // array of FailEntryInstance objects.

    this.bugs = null; // array of ints for associated bug ids.

    this.team = "None"; // each test belongs to a particular team.

	this._isLink = false;
}

/**********************************************************************/
/* New TestTeam object
*/
function _TestTeamCTOR(szTeam)
{
    this.name = szTeam;
    this.totalGroups = 0;
    this.totalInstances = 0;
}

// Types of sources that we could get an instance of a test failure.
var SOURCE_DDR     = 1;  // Daily Dev Run
var SOURCE_ROLLING = 2;  // Rolling Build
var SOURCE_SNAP    = 3;  // Snap job.


var DAYS_TO_INCLUDE = 20; // don't include reports older than X days.

/**********************************************************************/
/* Calculate "points" - how much a test group has failed.
This takes into account that:
- older failures are worth less than recent failures.
- a failure on a single DDR run may be due to local changes and so 
  is less important than other failures.
*/
function _calcPoints(group)
{
    var i;
    var fReal = false; // Has this only failed on a single DDR run?
    var fRecent = false;

    var szLastDDR = "";

    var totalPoints = 0;

    for(i = 0; i < group.instances.length; i++)
    {
        var test = group.instances[i];

        // Is it "real"?
        if (test.source != SOURCE_DDR)
        {
            fReal = true;
        }
        else
        {
            // Consider it a "real" failure if it happened on multiple DDR runs.
            // DDR hints are html link containing this test: jmstall_JMSTALL3#3
            // test text is of the format "machine#run", where a single machine machine have
            // multiple runs.
            // (<A HREF='\\...'>davidgut_DAVIDGUT5#2</A>)
            // In arrowhead, format is:
            //    "andparo.ANDPARO02.06-10-23#0"
            // which is (username).(machine).(timestamp)#(iteration)

            // Get the machine name (eg: davidgut_DAVIDGUT5) from the hint HTML string.
            // So if a failure happens on jmstall_JMSTALL3#3, jmstall_JMSTALL3#4, jmstall_JMSTALL3#7
            // then they're all on jmstall_JMSTALL3, and may all be failing due to a single set of local changes.
            // But if one failure is on both jmstall_JMSTALL3 and davidgut_DAVIDGUT5, then it's probably real.
            var re = />(.+?)\.(.+?)\..+?#(\d+)/;
            var m = test.hint.match(re);
                        
            var username = m[1];
            var szMachineName = m[2];

            if (szLastDDR == "") { szLastDDR = szMachineName; }
            if (szMachineName != szLastDDR)
            {
                fReal = true;
            }
        }

        var ageDays = test.ageHours / 24; // how old is it. Recent is worth more

        if (ageDays <= 1)
        {
            totalPoints += 10;
            fRecent = true;
        } 
        else if (ageDays <= 3)
        {
            totalPoints += 8; // "red failures"
            fRecent = true;
        }
        else if (ageDays <= 5)
        {
            totalPoints += 5;
        }
        else if (ageDays <= 10)
        {
            // If a test hasn't failed within 5 days, it's probably fixed,
            // so we weight it a lot less.
            totalPoints += 2; // "gray' failures
        }
        else
        {
            // Very old failures. Likely here from a stale DDR report - not worth much.
            totalPoints += 1;
        }
        
    } // end for each instance


    // Give "recent" failures a boost to help drive them to the top.
    if (fRecent)
    {
        totalPoints += 15;
    }

    // Give "real" failures a point boost b/c they're more important.
    // This effectively filters tests failing on a single DDR machine to the bottom.
    if (fReal)
    {
        totalPoints += 100;
    

        // Raise priority of "real"  checkin bvt failure because those should be the most stable tests,
        // and they can reject Snap entries.
        if (group.Categories.toLowerCase().indexOf("checkinbvt") >= 0)
        {
            totalPoints = Math.round((totalPoints + 60) * 1.5);
        }
    }
    
    // Lower points if all associated bugs are "Closed", and the resolved-date is recent.
    // (No affect if there aren't any bugs at all)
    if (group.bugs != null)
    {
        var dateRecentFix = null;
        
        var fixed = true;
        for(var iBug = 0; iBug < group.bugs.length; iBug++)
        {
            var bug = group.bugs[iBug];
            //WScript.Echo("group:" + group.CmdLine +","+ dump(bug));

            var fBugActive = (bug.AssignedTo != "Closed");
            
            if (fBugActive)
            {
                fixed = false;
            }

            // Track the date of the most recent bug-fix.
            if ((dateRecentFix == null) || (dateRecentFix < bug.ResolvedDate))
            {
                dateRecentFix = bug.ResolvedDate;
            }            
        }

        // If the recent failure occurs after a certain grace period from when the last bug was
        // resolved, then the bug didn't fix it.
        if (fixed && (dateRecentFix != null))
        {
            var dateRecentFailure = group.dateRecentFailure;

            var diffMilliseconds = group.dateRecentFailure - dateRecentFix;
            var diffDays = diffMilliseconds / (1000 * 60 * 60 * 24);                

            var daysGrace = 3;
            if (diffDays > daysGrace)
            {
                fixed = false;
            }
        }

        if (fixed)
        {
            totalPoints -= 100;
            if (totalPoints < 0)
            {
                totalPoints = 0;
            }
        }

    }

    return totalPoints;
}

/*
CTOR for FailEntryInstance
This is an instance of a failing test. Includes instance information 
    - parent - backptr to FailTest
    - html report to the log (which is in smarty.xml)
    - source - source of test (dailyDevRun share, rolling-build, snap jobs, etc)
    - hint - extra parameter to desribe source (eg, which dev in ddr, which snap job, etc).
*/    
function _FailEntryInstanceCTOR(parent, source, szHtmlFailureLink, hint)
{
    this.szHtmlFailureLink = szHtmlFailureLink;

    this.source = source; // SOURCE_X
    this.hint   = hint;

    this.smartyXml = null; // complete smarty xml info.    

    this.date = null;
    this.ageHours = 0; // cache how old each test is in hours.
}

/**********************************************************************/
/* Find or create the FailTestGroup object in the hash.
- sGuid - the test guid for the group. 
- hashErrors - hashtable to insert the new test group into.
*/
function _findOrCreateGroup(sGuid, hashErrors)
{
    _Check(hashErrors != null, "hashErrors is null (1)");

    if (hashErrors.Exists(sGuid))
    {
        return hashErrors.Item(sGuid);
    }
    else
    {
        var group = new _FailTestGroupCTOR(sGuid);
        hashErrors.Add(sGuid, group);
        return group;
    }        
}

/**********************************************************************/
/* Scans the directory for the smarty failure report and 
adds failure entries to the hash. 
- szDevBvtDir - directory to a devbvt run. Could be from anywhere.
- source, hint - high-level description of where szDevBvtDir is from. This info is added
                  to each FailEntry.
- hashErrors - hash table of FailTest objects.
  Returns true if actually had a devbvt report, false if not.
*/
function _updateFailureList(szDevBvtDir, source, hint, hashErrors)
{
    _Check(hashErrors != null, "hashErrors is null (2)");

    logMsg(LogTestSummary,  LogInfo1000, "Looking in Failure list:" + szDevBvtDir +"\n");
    // Look through the smarty.xml file in the given szDevBvtDir and report all errors.
	try
    {
        var testRun = testRunFromSmarty(szDevBvtDir)
		if (testRun == undefined)
			return false;
        var testFailures = testRun.failures;
        var dateNow = new Date();

        for (var i = 0; i < testFailures.length; i++) 
        {
            // Add each test failure as an instance to the group.
            var testFailure = testFailures[i];

            var entry_date = new Date(testFailure.starttime);

            var milliseconds = dateNow - entry_date;
            var entry_ageHours = milliseconds / (1000 * 60 * 60);
            
            // If test is too old, forget it.
            // @todo - move this up higher. We could check it just once for the entire file.
            // Check this before we create a new test group.
            if (entry_ageHours > 24 * DAYS_TO_INCLUDE) 
            {
                continue;
            }
            

            var id = testFailure.testid;
            var group = _findOrCreateGroup(id, hashErrors);
            group.Categories = testFailure.description;
            group.CmdLine = testFailure.name;

            var szOwners = testFailure.devowner;        
            if ((testFailure.testowner) != "" && (testFailure.testowner != szOwners))
            {
                if (szOwners != "")
                {
                    szOwners += ";";
                }
                szOwners += testFailure.testowner;
            }
            group.SDET = szOwners;

            var szHtmlFailureLink = testFailure.testlog; 
            var entry = new _FailEntryInstanceCTOR(group, source, szHtmlFailureLink, hint);    

            entry.smartyXml = testFailure;

            entry.date = entry_date;
            entry.ageHours = entry_ageHours

            group.instances.push(entry);
            group.cTotal++;
        }

        logMsg(LogTestSummary,  LogInfo1000, "total failures:" + testFailures.length + "\n");
    
    }
    catch(e)
    {
        logMsg(LogTestSummary,  LogWarn, "_updateFailureList: Caught Exception: ", e.description, "\n");
        return false;
    }
    


    return true;
}


//-----------------------------------------------------------------------------
// We have several CollectXXX functions. These take a source and find the devbvt
// dir from it, and then call UpdateFailureList accordingly.
//-----------------------------------------------------------------------------


function _getDDRforBranch(branch)
{
    var sRoot = "\\\\clrmain\\public\\DailyDevRuns\\" + branch + "\\logs";
    return sRoot;
}

// Collect failures from arrowhead DDRs
// These are on a share like:
//     \\clrmain\public\DailyDevRuns\____\logs\____\x86chk\test.devBVT
//
// Returns number of errors.
function _collectArrowHeadDDRS(hashErrors)
{
    var cTotal = 0;
    
    cTotal += _collectArrowHeadBranchDDR(_getDDRforBranch("puclr"), "puclr", hashErrors);
    cTotal += _collectArrowHeadBranchDDR(_getDDRforBranch("undefined"), "private", hashErrors);
    
    return cTotal;    
}

// Collect the errors in a specific branch
// sRoot - share to root of DDR. 
// branchName - branch name servers as a hint
function _collectArrowHeadBranchDDR(sRoot, branchName, hashErrors)
{
    debugger; // $$$


    var f = FSOGetFolder(sRoot);   
    
    logMsg(LogTestSummary,  LogInfo1000, "Collecting failures from Arrowhead DDR at:" + f.Path + " {\n");

    var fJob = new Enumerator(f.SubFolders);
    var cTotal = 0;
    for (; !fJob.atEnd(); fJob.moveNext())
    {
        //  Shortcut hack for debugging
        //if (cTotal > 20) return cTotal;

        try
        {
            var fJobDir = fJob.item();

            logMsg(LogTestSummary,  LogInfo1000, "Looking in " + fJobDir.Name + "\n");
            
            var szDevBvtDir = fJobDir.Path + "\\x86chk\\test.devBVT";
            var szTaskReport = szDevBvtDir + "\\Smarty.rpt.0.html";

            var szStartFilename = fJobDir.Path;
            var szDev = szStartFilename;

            var szDevMatches = szStartFilename.match(/\\([^\\]*)_.*/);
            if (szDevMatches.length >= 1) { szDev = szDevMatches[1]; }
            
            var szMarker = branchName + ":" + _htmlAliasAnchor2(szDev + "#" + cTotal, fJobDir.Path + "\\taskReport.html") + ")(" + _htmlAliasAnchor2("Report", szTaskReport);
            
            if (_updateFailureList(szDevBvtDir, SOURCE_DDR, szMarker, hashErrors))
            {
                cTotal++;
            }
        }
        catch(e)
        {
            logMsg(LogTestSummary,  LogWarn, "_collectArrowHeadDDRS: Caught Exception: ", e.description, "\n");
            // ignore failures and move on to the next report.
        }
    }

    logMsg(LogTestSummary,  LogInfo1000, "} done with snap\n");
    return cTotal;
}


function _collectFailuresFromDailyDevRunLogs(hashErrors)
{
    var sRoot = "\\\\CLRMain\\public\\DailyDevRuns\\logs";
    var f = FSOGetFolder(sRoot);
    
    logMsg(LogTestSummary,  LogInfo1000, "Collecting failures from DDR at:" + f.Path + " {\n");

    var fJob = new Enumerator(f.SubFolders);
    var cTotal = 0;
    for (; !fJob.atEnd(); fJob.moveNext())
    {
        try
        {
            var fJobDir = fJob.item();

            logMsg(LogTestSummary,  LogInfo1000, "Looking in " + fJobDir.Name + "\n");
            
            var szDevBvtDir = fJobDir.Path + "\\x86chk\\test.devBVT";
            var szTaskReport = szDevBvtDir + "\\CHECKINBVT_BUILDBVT_DEVUNIT.rpt.0.html";

            var szStartFilename = fJobDir.Path;
            var szDev = szStartFilename;

            var szDevMatches = szStartFilename.match(/\\([^\\]*)_.*/);
            if (szDevMatches.length >= 1) { szDev = szDevMatches[1]; }
            var szMarker = _htmlAliasAnchor2(szDev + "#" + cTotal, fJobDir.Path + "\\taskReport.html") + ")(" + _htmlAliasAnchor2("Report", szTaskReport);
            if (_updateFailureList(szDevBvtDir, SOURCE_DDR, szMarker, hashErrors))
            {
                cTotal++;
            }
        }
        catch(e)
        {
            logMsg(LogTestSummary,  LogWarn, "_collectFailuresFromDailyDevRuns: Caught Exception: ", e.description, "\n");
            // ignore failures and move on to the next report.
        }
    }

    logMsg(LogTestSummary,  LogInfo1000, "} done with snap\n");
    return cTotal;
}

/**********************************************************************/
/* Collect all failures from the DailyDevRun share (\\CLRMain\public\Drops\Whidbey\dailyDevRuns)
This enumerates the share and calls UpdateFailureList on each failure.
*/
function _collectFailuresFromDailyDevRunShare(szShare, hashErrors)
{
    var sRoot = "\\\\CLRMain\\public\\Drops\\Whidbey\\dailyDevRuns";
    var f = FSOGetFolder(sRoot);

    logMsg(LogTestSummary,  LogInfo1000, "Collecting failures from DDR at:" + f.Path + " {\n");

    var cTotal = 0;
    var fc = new Enumerator(f.files);
    for (; !fc.atEnd(); fc.moveNext())
    {
		try 
        {
            //if (cTotal > 4) return cTotal;
        
            // Extra the devs name.
            // From: \\CLRMain\public\Drops\Whidbey\dailyDevRuns\jmstall_JMSTALL5.url 
            // To: jmstall_JMSTALL5
            var szStartFilename = fc.item().Path;
            var szDev = szStartFilename;

            var szDevMatches = szStartFilename.match(/\\([^\\]*_.*)\.url/);
            if (szDevMatches.length >= 1) { szDev = szDevMatches[1]; }

            var szRunJsDir = urlShortCutTarget(fc.item());

            // The target could point to either the most recent or a specific share.
            // From: \\VANCEM1\vbl\lab21s\automation\run.04-01-19_23.01.0\taskReport.html
            // From: \\JMSTALL4\whidbey6\automation\run.current\taskReport.html
            // To:   \\JMSTALL4\whidbey6\automation
            //var m = szRunJsDir.match(/^(.*)\\run\.current/);
            var m = szRunJsDir.match(/^(.*)\\\S+?\\taskReport\.html/i);
            var sLocalRoot = m[1];

            var fLocal = FSOGetFolder(sLocalRoot);


            var c = 0;
            var fcLocal = new Enumerator(fLocal.SubFolders);
            for (; !fcLocal.atEnd(); fcLocal.moveNext())
            {
                c++;
                var fn = fcLocal.item().Path;
                var re = /run\.current/;

                // "run.current" is just an alias, so ignore it.
                if (!re.test(fn))
                {
                    // Get the devbvt dir.
                    // From: \\JMSTALL4\whidbey6\automation\run.current\taskReport.html
                    // to:   \\JMSTALL4\whidbey6\automation\run.current\x86chk\test.devBVT
                    //var szDevBvt = szRunJsDir.replace(/taskReport\.html/, "x86chk\\test.devBVT");
                    var szDevBvt = fn + "\\x86chk\\test.devBVT";
                    

                    if (szDevBvt != "")
                    {
                        var szMarker = _htmlAliasAnchor2(szDev + "#" + c, fn + "\\taskReport.html") + ")(" + _htmlAliasAnchor2("Report", szDevBvt + "\\CHECKINBVT_BUILDBVT_DEVUNIT.rpt.0.html");
                        if (_updateFailureList(szDevBvt, SOURCE_DDR, szMarker, hashErrors))
                        {
                            cTotal++;
                        }
                    } // has devbvt dir
                 }
             }

         }
         catch(e)
         {
			logMsg(LogTestSummary,  LogWarn, "_collectFailuresFromDailyDevRunShare: Caught Exception: ", e.description, "\n");
            // If any of the file accesses fail, just move on to the next one.
         }
    } // loop for each link on the public share


    logMsg(LogTestSummary,  LogInfo1000, "} done with DDR\n");

    return cTotal;
}

/****************************************************************************/
/* Get machine name that a rolling build was done on. 
*/
function _getRollingMachineName(szJobPath)
{
    // rollingReport.0.status.log
    // MACHINE: CLRSNAP3X6
    var szLogPath = szJobPath + "\\rollingReport.0.status.log";
    var szData = FSOReadFromFile(szLogPath);

    var re = /^MACHINE: ?(\S+?)$/mi;    
    if (re.test(szData))
    {
        return RegExp.$1;
    }
    return "";
}

/**********************************************************************/
/* Collect all failures from the rolling build (\\CLRMain\public\Drops\Whidbey\rollingTests)
*/
function _collectFailuresFromRollingBuild(hashErrors)
{
    var cTotal = 0;
    _Check(hashErrors != null, "hashErrors is null (3)");
    var sRoot = "\\\\CLRMain\\public\\Drops\\Whidbey\\rollingTests";

    var f = FSOGetFolder(sRoot);
    logMsg(LogTestSummary,  LogInfo1000, "Collecting failures from Rolling Build at:" + f.Path + " {\n");

    var fJob = new Enumerator(f.SubFolders);
    for (; !fJob.atEnd(); fJob.moveNext())
    {
        //if (cTotal > 30) return cTotal;
		try
        {
            var fJobDir = fJob.item();
            // Get Devbvt dir from this:
            // From: \\CLRMain\public\Drops\Whidbey\rollingTests\707448
            // To:   \\CLRMain\public\Drops\Whidbey\rollingTests\707448\x86chk\test.devBVT

            logMsg(LogTestSummary,  LogInfo1000, "Looking in " + fJobDir.Name + "\n");
            
            var szDevBvtDir = fJobDir.Path + "\\x86chk\\test.devBVT";
            var szTaskReport = szDevBvtDir + "\\CHECKINBVT_BUILDBVT_DEVUNIT.rpt.0.html";

            var szMachineName = _getRollingMachineName(fJobDir.Path);

            var szMarker = _htmlAliasAnchor2(szMachineName + "," + fJobDir.Name,  fJobDir.Path + "\\taskReport.html") + ")(" + _htmlAliasAnchor2("Report", szTaskReport);
            if (_updateFailureList(szDevBvtDir, SOURCE_ROLLING, szMarker, hashErrors))
            {
                cTotal++;
            }
        }            
        catch(e)
        {
			logMsg(LogTestSummary,  LogWarn, "_collectFailuresFromRollingBuild: Caught Exception: ", e.description, "\n");
            // ignore failures and move on to the next report.
        }
    }

    logMsg(LogTestSummary,  LogInfo1000, "} done w/ Rolling Build\n");
    return cTotal;
}


/**********************************************************************/
/* Get pretty name info from snap job.
*/
function _getSnapJobInfo(fJobDirPath)
{
    // It's in a 'jobinfo.bat' file, on a line like:
    // set CurrentJobDir=\\clrsnapbackup2\whidbey\6099.chrisk\job.9882
    // We want to get the '6099.chrisk' out    
    
    var fJobPath = fJobDirPath + "\\jobInfo.bat";
    if (FSOFileExists(fJobPath))
    {    
        var jobInfo = FSOReadFromFile(fJobPath);
        if (jobInfo != "")
        {
            var re = /whidbey\\(.*\..*)\\/;
            var x = jobInfo.match(re);
            if (x.length >= 1)
            {
                return x[1];
            }
        }
    }
    return "unknown";
}

/**********************************************************************/
/* Collect all failures from snap jobs (\\CLRMain\public\Drops\Whidbey\builds\PerCheckin\)
*/
function _collectFailuresFromSnapJobs(hashErrors)
{
    // Anything on builds is successful
    // Convert a snap dir to the dirs holding the smarty reports.
    // Snap splits it up so we have multiple places to look.
    // From: \\CLRMain\public\Drops\Whidbey\builds\PerCheckin\707448
    // To:   \\CLRMain\public\Drops\Whidbey\builds\PerCheckin\707448\x86chk\TestResults.CheckinBVT\
    // To:   \\CLRMain\public\Drops\Whidbey\builds\PerCheckin\707448\x86ret.unopt\TestResults.CheckinBVT\
    // To:   \\CLRMain\public\Drops\Whidbey\builds\PerCheckin\707448\x86ret.unopt\TestResults.BuildBVT\

    var cTotal = 0;
    
    var sRoot = "\\\\CLRMain\\public\\Drops\\Whidbey\\builds\\PerCheckin";
    var f = FSOGetFolder(sRoot);

    logMsg(LogTestSummary,  LogInfo1000, "Collecting data from snap builds at:" + f.Path + " {\n");

    var fJob = new Enumerator(f.SubFolders);
    for (; !fJob.atEnd(); fJob.moveNext())
    {
        // if (cTotal > 35) return cTotal;
		try
        {
            var fJobDir = fJob.item();
            // Get Devbvt dir from this:
            // From: \\CLRMain\public\Drops\Whidbey\rollingTests\707448
            // To:   \\CLRMain\public\Drops\Whidbey\rollingTests\707448\x86chk\test.devBVT

            // Get snap id.
            // It's in a 'jobinfo.bat' file, on a line like:
            // set CurrentJobDir=\\clrsnapbackup2\whidbey\6099.chrisk\job.9882
            var snapid = _getSnapJobInfo(fJobDir.Path);        

           
            var szDevBvtDir = fJobDir.Path + "\\x86ret.unopt\\TestResults.BuildBVT";
            var szTaskReport = szDevBvtDir + "\\BUILDBVT_DEVUNIT$FXCOP-COMBINED$HOSTING$IE_COMBINED$REMOTING_NOSNAP.rpt.0.html";

            var szMarker = _htmlAliasAnchor2(snapid, fJobDir + "\\JobReport.html") + ")(" + _htmlAliasAnchor2("Report", szTaskReport);

            if (_updateFailureList(szDevBvtDir, SOURCE_SNAP, szMarker, hashErrors)) // fJobDir.Name
            {
                cTotal++;
            }
        }
        catch(e)
        {
			logMsg(LogTestSummary,  LogWarn, "_collectFailuresFromSnapJobs: Caught Exception: ", e.description, "\n");
            // ignore failures and move on to the next report.
        }
    }

    logMsg(LogTestSummary,  LogInfo1000, "} done with snap\n");
    return cTotal;
}


/**********************************************************************/
/* Write the header for a single group to the html file
- htmlFile - the output stream to write html to.
- idx - integer number of which group this is (for pretty printing)
- group - which test are we spewing for.
*/
function _writeGroupHeaderToHtml(htmlFile, idx, group)
{
    // Dump group header
    htmlFile.Write("<pre> (#" + idx + ") " + group.cTotal + " failures, ");

	// if linked from top 10 list, add the link.
	if (group._isLink)
	{
		htmlFile.WriteLine(_htmlBookmarkHere(group.stringTestGuid));
	}

	htmlFile.WriteLine("<strong>" + group.stringTestGuid + "</strong>" +
       "  "+ "<font color=\"gray\">"+group.points + " points </font>");
    htmlFile.WriteLine("CATEGORIES:   " + group.Categories);
    htmlFile.WriteLine("COMMANDLINE:  " + group.CmdLine);
    htmlFile.WriteLine("SDET:         " + group.SDET);

}

/**********************************************************************/
/* Write a single group to the html file
- htmlFile - the output stream to write html to.
- idx - integer number of which group this is (for pretty printing)
- group - which test are we spewing for.
*/
function _writeGroupToHtml(htmlFile, idx, group)
{

    // Dump group header
	_writeGroupHeaderToHtml(htmlFile, idx, group);

    // Now write each bug id.
    if (group.bugs != null)
    {
        htmlFile.Write("Bugs: ");

        for(var iBug = 0; iBug < group.bugs.length; iBug++)
        {
            // For readability, only print 3 bugs per line.
            if ((iBug > 0) && (iBug % 3 == 0))
            {
                htmlFile.WriteLine(".");
                htmlFile.Write("    : ");
            }
            
            var bug = group.bugs[iBug];

            //WScript.Echo("***:" + group.CmdLine + "," + bug.ID);
            htmlFile.Write(" " + _htmlAliasBugLink("vswhidbey", bug.ID));

            var owner = "?";
            if (bug.AssignedTo == "Closed") 
            {
                // Resolved bug. 
                var old = false;

                var diffMilliseconds = group.dateRecentFailure - bug.ResolvedDate;
                var diffDays = diffMilliseconds / (1000 * 60 * 60 * 24);                

                var daysGrace = 3;
                if (diffDays > daysGrace)
                {
                    old = true;
                }
                
                owner = (old ? "old_" : "") + "resolved:" + bug.ResolvedBy;
            }
            else
            {
                owner = "assigned:" + bug.AssignedTo;
            }
            htmlFile.Write("(" + owner + ")");
        }

        htmlFile.WriteLine(".");
    }  
        

    // Now dump each instance.
    var j = 0;
    for(j = 0; j < group.instances.length; j++)
    {
        var entry = group.instances[j];

                
        // Format
        var stSource = "";
        switch(entry.source)
        {
            case SOURCE_DDR:
                stSource = "DailyDevRun";
                break;
            case SOURCE_ROLLING:
                stSource = "Rolling Build";
                break;
            case SOURCE_SNAP:
                stSource = "Snap";
                break;                
            default:
                stSource = "Unknown source";
                break;               
        }            
        var szName = "<i>" + stSource + "</i>(" + entry.hint + ")";

        // Color code based off date of failure.
        var color = ""; // default
        var hours = entry.ageHours;

        if (hours > 24 * 5) {
            color = "color = \"gray\"";
        } else
        if (hours < 24 * 3) {
            color = "color = \"red\"";
        }

        var szDate = entry.date.toDateString() + " " + entry.date.toLocaleTimeString();
        
        // Write out this instance.
        // In case this group gets cut & pasted as text and loses hyperlinks, we want to include
        // at least one full hyperlink (display name == url target) in the text so things can be followed. 
        // But if we made everything full hyperlinks, that would be too verbose.
        htmlFile.Write("<font " + color + ">");
        htmlFile.Write("FAILED: ");
        htmlFile.Write("[" + szDate + "]");
        htmlFile.Write(szName + ",log: " + _htmlAnchor2(entry.szHtmlFailureLink));
        htmlFile.WriteLine("</font>");
    }

    htmlFile.WriteLine("</pre>");
}

/**********************************************************************/
/* Dumps errors from array of FailTestGroup objects into an html report
Caller can sort / filter list.
- htmlFile - output stream to write html to.
- list - array of test groups to, sorted in order they should be printed.
*/
function _writeErrorsToHtml(htmlFile, list)
{
    // Write out hash.
    var totalFailures = 0;

    // Errors are sorted by team.
    var lastTeam = "";

	//htmlFile.WriteLine("Click " + _htmlBookmarkLink("here", "SkipDevSvcs") + " to skip to non-devsvcs tests.<br>");
        
    htmlFile.WriteLine("<ul>");
    var i = 0;
        
    for(i = 0; i < list.length; i++)
    {        
        var group = list[i];


        // Print team banner
        var thisTeam = group.team;
        if (thisTeam != lastTeam)
        {
            htmlFile.WriteLine(_htmlBookmarkHere("Skip_" + thisTeam));
            htmlFile.WriteLine("========================================================<br>");
            htmlFile.WriteLine("Begin <b> " + thisTeam + "</b> failures<br>");
            htmlFile.WriteLine("========================================================<br>");            
        }
        lastTeam = thisTeam;
        
/*
        // @todo - draw a bar between devsvcs failures and non-devsvcs
        if ((i > 0) && !_IsDevSvcsFailure(group) && _IsDevSvcsFailure(list[i-1]))
        {
            htmlFile.WriteLine(_htmlBookmarkHere("SkipDevSvcs"));
            htmlFile.WriteLine("========================================================<br>");
            htmlFile.WriteLine("Begin Non-devsvcs failures<br>");
            htmlFile.WriteLine("========================================================<br>");
        }
*/        
        htmlFile.Write("<li>");
        _writeGroupToHtml(htmlFile, i, group);
        htmlFile.WriteLine("</li>");
       
        totalFailures += group.cTotal;
    }    

    htmlFile.WriteLine("</ul>");
}

/**********************************************************************/
/* Dumps errors from array of FailTestGroup objects into an html report
Caller can sort / filter list.
- htmlFile - output stream to write html to.
- list - array of test groups to, sorted in order they should be printed.
*/
function _writeTopNErrorsToHtml(htmlFile, list, num)
{

	//htmlFile.WriteLine("Click " + _htmlBookmarkLink("here", "SkipDevSvcs") + " to skip to non-devsvcs tests.<br>");
        
    htmlFile.WriteLine("<ul>");
    var i = 0;
	var iEnd = list.length;
	if (iEnd > num)
	{
		iEnd = num;
	}
        
    htmlFile.WriteLine("</ul>");
    htmlFile.WriteLine("<pre><b>Top " + iEnd + " failures</b>");
    htmlFile.WriteLine("<table border = 1>");

    htmlFile.WriteLine("<tr><td># Failures</td><td>Test name</td><td>Points</td>");
    var i;
    for(i = 0; i < iEnd; i++)
    {        
        var group = list[i];
        
		htmlFile.WriteLine("<tr>");
        htmlFile.WriteLine("<td> " + group.cTotal + " </td>");
        htmlFile.WriteLine("<td> " + _htmlBookmarkLink(group.stringTestGuid, group.stringTestGuid) + " </td>");
        htmlFile.WriteLine("<td> " + group.points + " </td>");
        htmlFile.WriteLine("</tr>");            

		group._isLink = true;
    }

    htmlFile.WriteLine("</table>");
    htmlFile.WriteLine("</pre>");
}

/**********************************************************************/
/* Write histogram to HTML
- htmlFile - ouput stream to write html to
- hHistogram - array of integers representing data.
*/
function _writeHistogramToHtml(htmlFile, hHistogram)
{
    //htmlFile.WriteLine("<pre>");
    htmlFile.WriteLine("Histogram of error frequency:");

    htmlFile.WriteLine("<table border=1>");

    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("<td># of tests:  </td>");
    for(i = 1; i < hHistogram.length; i++)
    {
        htmlFile.WriteLine("<td>" + hHistogram[i] + "</td>");
    }
    htmlFile.WriteLine("</tr>");

    
    htmlFile.WriteLine("<tr>");
    htmlFile.WriteLine("<td>with this many failures:</td>");
    for(i = 1; i < hHistogram.length - 1; i++)
    {
        htmlFile.WriteLine("<td>" + i + "</td>");
    }
    htmlFile.WriteLine("<td>" + i + "+ </td>");
    htmlFile.WriteLine("</tr>");
    
    htmlFile.WriteLine("</table>");

    //htmlFile.WriteLine("</pre>");
}

/**********************************************************************/
/* Return a 2-decimal digit average of (cTop / cBottom).
*/
function _writeTeamStatsToHtml(htmlFile, teamStats, cTotalGroups, cTotalInstances)
{
/*
    htmlFile.WriteLine("<pre>Per team stats:");    

    htmlFile.WriteLine(cTotalGroups + " total <i>different</i> tests failing.");
    htmlFile.WriteLine(cTotalInstances + " total <i>instances </i> of tests failing" +
        "(" + _average2(cTotalInstances, cTotalGroups) + " instances / test).");
    htmlFile.WriteLine("</pre>");
*/

    htmlFile.WriteLine("<pre><b>Per team Breakdown</b>");
    htmlFile.WriteLine("<table border = 1>");

    htmlFile.WriteLine("<tr><td>Team</td><td>Failing groups (% of total)</td><td>Failing instances (% of total)</td>");
    var i;
    for(i in teamStats)
    {
        //htmlFile.WriteLine("Click " + _htmlBookmarkLink("here", "SkipDevSvcs") + " to skip to non-devsvcs tests.<br>");
        htmlFile.WriteLine("<tr>");
        var team = teamStats[i];
/*        
        htmlFile.WriteLine("   " + _htmlBookmarkLink(team.name, "Skip_" + team.name) + ", " + 
            team.totalGroups + "( " + Math.floor(team.totalGroups * 100 / cTotalGroups) + "%), " +
            team.totalInstances + "( "+ Math.floor(team.totalInstances * 100 / cTotalInstances) + "%)");
*/
        htmlFile.WriteLine("<td> " + _htmlBookmarkLink(team.name, "Skip_" + team.name) + "</td>");
        htmlFile.WriteLine("<td> " + team.totalGroups + "( " + Math.floor(team.totalGroups * 100 / cTotalGroups) + "%) </td>");
        htmlFile.WriteLine("<td> " + team.totalInstances + "( "+ Math.floor(team.totalInstances * 100 / cTotalInstances) + "%) </td>");
        htmlFile.WriteLine("</tr>");            
    }

    htmlFile.WriteLine("<tr><td><b>Total</b></td><td>" +cTotalGroups +"</td><td>" + cTotalInstances +"</td></tr>");
    htmlFile.WriteLine("</table>");
    htmlFile.WriteLine("</pre>");
}

/**********************************************************************/
/* Return a 2-decimal digit average of (cTop / cBottom).
*/
function _average2(cTop, cBottom)
{
    var average = Math.floor(cTop * 100 / cBottom) * 1.0 / 100;
    return average;
}

//-----------------------------------------------------------------------------
// Main driver
// Produces a html report (szHtmlOutFil) of all failures. 
//-----------------------------------------------------------------------------

/****************************************************************************/
/* Scan multiple devbvt hives and create a html report that groups 
    failure instances of the same test across multiple runs.
    This report is useful for seeing how non-deterministic a test is.

   Parameters:
       szHtmlOutFile: Filename for html report to generate.
*/
function makeFailureSummary(szHtmlOutFile)
{
    if (szHtmlOutFile == undefined)
        throw new Error("Need szHtmlOutFile parameter");

    logCall(LogTestSummary, LogInfo10, "makeFailureSummary", arguments, "{");

    // If under a debugger, stop here so that we have a chance to put BPs in the file.
    debugger;

    var szMainRoot = "\\\\CLRMain\\public";
    var szDDRShare = szMainRoot + "\\DailyDevRuns";
    var szPUCLRDropsShare = szMainRoot + "\\Drops\\PUCLR";
    var szJobHistoryShare = szPUCLRDropsShare + "\\Snap\\jobHistory_puclr.html";
    var szRollingReportShare = szPUCLRDropsShare + "\\rollingTests\\report.html";    
    

    var htmlFile = FSOOpenTextFile(szHtmlOutFile, 2, true); 
    htmlFile.WriteLine("<HTML>"); 
    htmlFile.WriteLine("<BODY>"); 
    htmlFile.WriteLine("<H2> Arrowhead DevBvt Failure summary (" + new Date() + ") </H2>"); 

    // Write out summary stats
    htmlFile.WriteLine("<p>");

    htmlFile.WriteLine("The goal of this page is to group common failures from different devbvt runs. It's assumed that ");
    htmlFile.WriteLine("broken tests will be disabled, so the only failures here should be the non-determinstic ones. Thus this");
    htmlFile.WriteLine("report aims to measure the non determinism in the product.");
    htmlFile.WriteLine("This page shows <i>failures</i> across devbvt runs potentially collected from several sources (x86 only):");
    htmlFile.WriteLine("<ul>");

    htmlFile.WriteLine("<li> <b>The Rolling Build</b> (" +  _htmlAnchor2(szRollingReportShare) +").");
    htmlFile.WriteLine("  This is a process that just loops around syncing to live bits & running tests. ");
    htmlFile.WriteLine("This runs all of devbvt (including devunit tests) on the live bits.");
    htmlFile.WriteLine("</li>");
    
    htmlFile.WriteLine("<li> <b>Daily Dev Run</b> (ddr) (" + _htmlAnchor2(szDDRShare) + ").");
    htmlFile.WriteLine("DDRs are devbvt runs on local dev machines from 'runjs dailyDevRun'. They're run against the current source on the devs' machine.");
    htmlFile.WriteLine("An isolated failure in a ddr report");
    htmlFile.WriteLine("  may be due to a local change on a dev's machine or to a partially sycned client mapping.");
    htmlFile.WriteLine("  DDRs include all of devbvt (devunit, checkin+buildbvt), ");
    htmlFile.WriteLine("  but may not be synced to the latest live bits. Thus a fixed test may still show lingering failures");
    htmlFile.WriteLine("  in ddr runs.")
    htmlFile.WriteLine("The same test failing on multiple");
    htmlFile.WriteLine("  DDR from different machines is suspect of a real bug. A single dev machine may contain multiple DDR reports.");
    htmlFile.WriteLine("</li>");
    
    htmlFile.WriteLine("<li> <b>Snap jobs</b> (" + _htmlAnchor2(szJobHistoryShare) + ").");
    htmlFile.WriteLine("This contains successful snap jobs. Snap jobs run on the live bits, but do not run devunit tests.");
    htmlFile.WriteLine("</li>");
    
    htmlFile.WriteLine("</ul>");
    htmlFile.WriteLine("This report is not live. It was generated on:" +  new Date() + " by user:" + Env("USERNAME") + " on machine:" + Env("COMPUTERNAME"));
    htmlFile.WriteLine("</p>");
    

    // Hash of all errors. Input is the test guid; output is HTML.
    var hashErrors = _newHashtable(); // hash of FailTestGroup objects

    // Get error info.
    var quick = false; // for debugging.

	//quick = true;
    //var cTotalDailyDevRunReports  = quick ? 0 : _collectFailuresFromDailyDevRunLogs(hashErrors);
    //var cTotalRollingBuildReports = _collectFailuresFromRollingBuild(hashErrors);
    var cTotalDailyDevRunReports = _collectArrowHeadDDRS(hashErrors);
    //var cTotalSnapReports         = quick ? 0 : _collectFailuresFromSnapJobs(hashErrors);
    logMsg(LogTestSummary,  LogInfo1000, "Done collecting data\n");
    
    // Convert hash --> array to do more processing on.
    var list = (new VBArray(hashErrors.Items())).toArray();   // Get the keys.


    // Print statistics.
    htmlFile.WriteLine("<pre><h3>Some stats:</h3>");
    var cTotalReports = cTotalDailyDevRunReports // cTotalDailyDevRunReports + cTotalRollingBuildReports + cTotalSnapReports;
    htmlFile.WriteLine(cTotalReports + " Total reports scanned for failures, split across:");
    htmlFile.WriteLine("    " + cTotalDailyDevRunReports + " Daily Dev Run reports");
    //htmlFile.WriteLine("    " + cTotalRollingBuildReports + " rolling build reports");
    //htmlFile.WriteLine("    " + cTotalSnapReports + " snap reports"); 

    // Get the bug ids for all the failures.
    try 
	{
        _getBugIdsForFailures(list);
	}
    catch(e)
    {
		logMsg(LogTestSummary,  LogWarn, "makeFailureSummary: Caught Exception: ", e.description, "\n");
        htmlFile.WriteLine("<B>Produce Studio unavailable. No bug information:" + e.message + "</b>");
    }

    // Do some processing on each group.
    var i;        

    var hHistogram = new Array(12);
    for(i = 0; i < hHistogram.length;i++) { hHistogram[i] = 0; }

    // Hash of all errors. Input is the test guid; output is HTML.
    var teamStatsHash = _newHashtable(); // hash of FailTestGroup objects
    
    
    var cInstanceFailures = 0;
    for(i = 0; i < list.length; i++)
    {        
        var group = list[i];

        // Add to per-team stats.
        var teamName = getTeamFromString(group.Categories);

        var team = null;
        if (teamStatsHash.Exists(teamName))
        {
            team = teamStatsHash.Item(teamName);
        }
        else
        {
            team = new _TestTeamCTOR(teamName);
            teamStatsHash.Add(teamName, team);
        }        
    
        //WScript.Echo("Got team object:");
        team.totalGroups++;
        team.totalInstances+=group.cTotal;

        group.team = teamName;
        
        cInstanceFailures += group.cTotal;

        // Add to histogram
        var idx = group.cTotal;
        if (idx >= hHistogram.length) 
        {
            idx = hHistogram.length - 1;
        }        
        hHistogram[idx]++;

        // Sort instances by date and record most recent failure.
        group.instances.sort(_SortInstancesByDate);
        group.dateRecentFailure = group.instances[0].date

        // Compute "points" for each test
        group.points = _calcPoints(group);
    }    
    htmlFile.WriteLine(_average2(cInstanceFailures, cTotalReports) + " failures / devbvt");
    htmlFile.WriteLine("</pre>");

    // At this point, we have all the Team Stats. Sort them by frequency
    // Convert hash --> array to do more processing on.
    var teamStats = (new VBArray(teamStatsHash.Items())).toArray();   // Get the keys.
    
    teamStats.sort(_sortTeamsByFailures);

    //WScript.Echo("Team:" + dump(teamStats));
    _writeTeamStatsToHtml(htmlFile, teamStats, list.length, cInstanceFailures);

    // Write out histogram.
    _writeHistogramToHtml(htmlFile, hHistogram);

    // Sort (highest hits first)
    //htmlFile.WriteLine("<pre>");
    htmlFile.WriteLine("<h3>Lists of failing tests.</h3>");
    htmlFile.WriteLine("<p>");
    htmlFile.WriteLine("Tests are sorted by team and then by <i>points</i>, which roughly correspond to number of instances a test fails.");
    htmlFile.WriteLine("Points are weighted to incorporate additional heuristics designed to force recent or real failures to the top of the list.");
    htmlFile.WriteLine("These heuristics include weighting older failures less (since it may have been fixed),");
    htmlFile.WriteLine("and weighting isolated ddr failures less (since it may be due to local changes on a machine).<br>");
    htmlFile.WriteLine("All bug links are in the VSWhidbey database. A bug is associated with a test failure by adding the test guid");
    htmlFile.WriteLine("to the 'testcase' field.<br>");
    htmlFile.WriteLine("Instances within each test group are sorted by date; recent dates are red; older dates are gray. Reports older than " + DAYS_TO_INCLUDE + " day(s) are excluded. </p>");
    htmlFile.WriteLine("</p>");
    //htmlFile.WriteLine("</pre>");
    
    // Generate "top 10" list.
    list.sort(_SortErrorsByPoints);
	_writeTopNErrorsToHtml(htmlFile, list, 10);

    
	htmlFile.WriteLine("<h4>All failures, by team, then points.</h4>");
    list.sort(_SortErrorsByHits2);


    // Print actual errors.
    _writeErrorsToHtml(htmlFile, list);    


    // Closing 
    htmlFile.WriteLine("<pre>End of report.</pre>");
    htmlFile.WriteLine("</BODY>");
    htmlFile.WriteLine("</HTML>"); 


    logMsg(LogTestSummary, LogInfo10, "} makeFailureSummary()\n");

    return 0;
}


/****************************************************************************/
/* Pretty wrapper around MakeFailureSummary to supply good default parameters.
   Publishes report to szHtmlOutFile.

   Parameters
       szHtmlOutFile - output file to generate report at.
*/
function publishFailureSummary(szHtmlOutFile)
{
    logCall(LogTestSummary, LogInfo10, "publishFailureSummary", arguments, "{");

    if (szHtmlOutFile == undefined)
    {
        szHtmlOutFile = "\\\\CLRMain\\public\\Drops\\puclr\\testRunFailureStats.html";
    }        

    // Making the failure summary could take a while, and may even fail.
    // So we make the summary to a temp file and then copy that to the public share.
    
    var szTemp = FSOGetTempPath();
    makeFailureSummary(szTemp);

    // Make a backup of the old one.
    //"TestRunFailureArchive"

    // Now copy to the final share.
    FSOCopyFile(szTemp, szHtmlOutFile, true);

    logMsg(LogTestSummary, LogInfo10, "} publishFailureSummary()\n");

    return 0;
}

/**********************************************************************/
/* Sort 2 FailEntryInstance objects by date.
*/
function _SortInstancesByDate(a, b)
{
    if (a.date < b.date) return 1;
    if (a.date > b.date) return -1;
    return 0;    
}

/**********************************************************************/
/* Sort Team stat objects by # of failures.
*/
function _sortTeamsByFailures(a, b)
{
    if (a.totalGroups > b.totalGroups) return -1;
    if (a.totalGroups < b.totalGroups) return 1;

    if (a.totalInstances > b.totalInstances) return -1;
    if (a.totalInstances < b.totalInstances) return 1;

    return 0;
}

/**********************************************************************/
/* Return true is this group is for a devsvcs failures.
 @todo - nice to find a way to customize this better.
*/
/*
function _IsDevSvcsFailure(group)
{
    return (group.Categories.toLowerCase().indexOf("devsvcs") != -1);
}
*/

/**********************************************************************/
/* Get the team responsible for a given test category string.
We define that as the 2nd token in the category. The 1st token is either
CheckinBvt, BuildBvt, or Devunit.
*/
function getTeamFromString(c)
{
    var team = null;

    if (c != undefined && c != null)
    {
        c = c.toLowerCase();
        if (c.match(/^.+?\\(.+?)(\\|$|;)/))
        {
            team = RegExp.$1;
        }
    }
    
    if (team != null)
    {
        // The devsvcs category actually contains both Debugger + Profiler + SOS tests
        // which are owned by different teams, so we'll split that out here too. 
        // Look at the 3rd token
        if (team == "devsvcs")
        {    
            if (c.match(/\\sos/))
            {
                team = "sos";
            }
            else if (c.match(/devsvcs\\(.+?)(\\|$|;)/))
            {            
                var subteam = RegExp.$1;

                // We only explicitly break out "profiler" b/c there's a ton of 
                // other stuff under devsvcs that we should group into it.
                if (subteam == "profiler")
                {
                    team = subteam;
                }
            }            
        }
        
        return team;
    }
    
    return "Unknown";

}

    
/**********************************************************************/
/* Sort array of FailTestGroup objects by team name. 
This lets the report easily coalesce them.
 Highest hit counts come first.
*/
function _SortErrorsByHits2(a, b)
{
    // Sort by team (we'll put devsvcs first).
    var aTeam = a.team;
    var bTeam = b.team;

    var aDevSvcs = (aTeam == "devsvcs");
    var bDevSvcs = (bTeam == "devsvcs");
    if (aDevSvcs && !bDevSvcs) return -1;
    if (!aDevSvcs && bDevSvcs) return 1;

  
    if (aTeam < bTeam) return 1;
    if (aTeam > bTeam) return -1;
    

    // Sort by "points". Points take into account a bunch of 
    // different heuristics to determine the more important failures.
    if (a.points == b.points) return 0;
    if (a.points <  b.points) return 1;
    return -1;    
}

/**********************************************************************/
/* Sort array of FailTestGroup objects by points. 
This lets the report easily print a "most wanted" list.
 Highest points come first.
*/
function _SortErrorsByPoints(a, b)
{
    // Sort by "points". Points take into account a bunch of 
    // different heuristics to determine the more important failures.
    if (a.points == b.points) return 0;
    if (a.points <  b.points) return 1;
    return -1;    
}

/****************************************************************************/ 
/* Given a list of failures, find the associated bug ids for each one.
*/
function _getBugIdsForFailures(list)
{
    logMsg(LogTestSummary, LogInfo10, "getting bug ids for failures {\n");

    var bugDb = bugConnect("VSWhidbey"); 

    // Only do 1 ProductStudio query here.
    var bugList = _getAllBugs(bugDb);

    var result = new Array();    

    for(var i = 0; i < list.length; i++)
    {        
        var group = list[i];

        // For each failure, see if there are any associated bugs.
        for(var j = 0; j < bugList.length; j++)
        {
            var bug = bugList[j];
            if (bug.TestCase.indexOf(group.stringTestGuid) >= 0)
            {
                result.push(bug);
            }
        }

        if (result.length > 0)
        {
            group.bugs = result;

            // Create a new array for the next time around.
            result = new Array();
        }
    }

    logMsg(LogTestSummary, LogInfo10, "} get bug ids\n");
}

/****************************************************************************/ 
/*
Get an array of bug objects for all test bugs that are associated with the automation failures.
We can then perform future queries in memory.

parameters:
    bugDb - connection object for bug database (obtained via bugConnect)
*/
function _getAllBugs(bugDb) { 
	if (bugDb == undefined)	
		bugDb = bugConnect("VSWhidbey"); 

    debugger;

    var queryXML =  "<Query> " + 
                        "<Group GroupOperator='and'> " + 
                        	"<Group GroupOperator='or'> " + 
		                        "<Expression Column='TestCase' Operator='contains'> " + 
		                                "<String> " + "checkinbvt" + " </String> "  + 
		                        "</Expression> "  + 
		                        "<Expression Column='TestCase' Operator='contains'> " + 
		                                "<String> " + "buildbvt" + " </String> "  + 
		                        "</Expression> "  + 
		                        "<Expression Column='TestCase' Operator='contains'> " + 
		                                "<String> " + "devunit" + " </String> "  + 
		                        "</Expression> "  + 		                        
	                        "</Group>" + 
                        "</Group> " + 
                    "</Query> ";         

    var ret = new Array(); 
    var results = bugQuery(bugDb, queryXML); 
    
    WScript.Echo("got " + results.length + " results\n"); 
    for (var i = 0; i < results.length; i++) { 
            var result = results[i]; 
			var bug = {}; 
			
			bug.ID = bugFieldGet(bugDb, result, "ID"); 
			bug.AssignedTo = bugFieldGet(bugDb, result, "Assigned to"); // this may be 'closed'
            bug.ResolvedBy = bugFieldGet(bugDb, result, "Resolved By"); 
			bug.TestCase = bugFieldGet(bugDb, result, "TestCase"); 
			var resolve_date = bugFieldGet(bugDb, result, "Resolved Date");
			bug.ResolvedDate = (resolve_date == undefined) ? null : new Date(resolve_date);
            
            ret.push(bug); 
            WScript.Echo("*** Bug " +  bug.ID + " Title: " + bug.Title + "\n"); 
    } 
    return ret; 
} 

function H3()
{
    var bugDb = bugConnect("VSWhidbey"); 

    var bugList = _getAllBugs(bugDb);

    for (var i = 0; i < bugList.length; i++) 
    { 
        var bug = bugList[i];

        WScript.Echo("*** Bug " +  bug.ID + " Assigned: " + bug.AssignedTo + ", rd=" + bug.ResolvedDate+ " \n"); 
    }

}

