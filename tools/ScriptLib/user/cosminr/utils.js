eval(includeJS("user\\cosminr\\adojscl.inc"));

//////////////////////////////////////////////////////////////////////
///
/// Extensions to ECMA-262 Arrays
///
//////////////////////////////////////////////////////////////////////

if (!Array.prototype.map)
{
    // Parameters:
    //   fun Function that produces an element of the new Array from an element of the current one.
    // 
    //   thisp Object to use as this when executing callback. If it is provided to map, it will be used as
    //     the "this" for each invocation of the callback. If it is not provided, or is null, the global object
    //     associated with callback is used instead. 

    arrayMap = function(fun /*, thisp*/)
    {
        var len = this.length;
        if (typeof fun != "function")
            throw new TypeError();

        var res = new Array(len);
        var thisp = arguments[1];
        for (var i = 0; i < len; i++)
        {
            if (i in this)
                res[i] = fun.call(thisp, this[i], i, this);
        }

        return res;
    };
}
else
{
    arrayMap = Array.prototype.map;
}

if (!Array.prototype.filter)
{
    // Description:
    //   It calls a provided callback function once for each element in an array, and constructs a new array
    //   of all the values for which callback returns a true value. callback is invoked only for indexes of
    //   the array which have assigned values; it is not invoked for indexes which have been deleted or which
    //   have never been assigned values. Array elements which do not pass the callback test are simply
    //   skipped, and are not included in the new array.
    // 
    // Parameters:
    //   fun Function that produces an element of the new Array from an element of the current one.
    //     It is invoked with three arguments: the value of the element, the index of the element, and
    //     the Array object being traversed. 
    //   thisp Object to use as this when executing callback. If it is provided to map, it will be used as 
    //     the "this" for each invocation of the callback. If it is not provided, or is null, the global
    //     object associated with callback is used instead.

    arrayFilter = function(fun /*, thisp*/)
    {
        var len = this.length;
        if (typeof fun != "function")
            throw new TypeError();

        var res = new Array();
        var thisp = arguments[1];
        for (var i = 0; i < len; i++)
        {
            if (i in this)
            {
                var val = this[i]; // in case fun mutates this
                if (fun.call(thisp, val, i, this))
                    res.push(val);
            }
        }

        return res;
    };
}
else
{
    arrayFilter = Array.prototype.filter;
}

if (!Date.prototype.toSDateString)
{
    Date.prototype.toSDateString = function()
    { return this.getMonth() + 1 + "/" + this.getDate() + "/" + this.getFullYear(); }
}

if (!Date.prototype.toShortString)
{
    Date.prototype.toShortString = function()
    { return this.toSDateString() + " " + this.toTimeString(); }
}

//////////////////////////////////////////////////////////////////////
///
/// CDB utilities (taken from CDB.js in netfxsp branch)
///
//////////////////////////////////////////////////////////////////////

/**************************************************************************/
/* finds a path to CDB which should work from anywhere within microsoft
(This is a bit of a hack) */

function _cdbGetCDBDir(arch)
{
    if (arch == undefined)
        arch = "x86";
    // This is a hack (wires in a particular network path!
    var ret = "\\\\CLRMain\\public\\tools\\" + arch;
    // use the debuggers used by autobug
    // ret = "\\\\wcfs05\\Debuggers\\x86"
    // ret = "C:\\Debuggers";
    ret = "\\\\xmn-dev0\\abugdbg\\" + arch;
    return ret;
}

/*****************************************************************************/
/* return a handle that allows you to send commands to the debugger */

function cdbNew(cmdLine, arch)
{
    if (cmdLine == undefined)
        throw Error(1, "Required arg 'cmdLine' not present");

    var cdb = {};
    var oldPath = Env("PATH");
    var cdbDir = _cdbGetCDBDir(arch);
    logMsg(LogScript, LogInfo100, "CdbDir = ", cdbDir, "\n");
    Env("PATH") = cdbDir + ";" + oldPath
    cdb.exec = WshShell.Exec("cdb -2 " + cmdLine);
    Env("PATH") = oldPath;

    if (!cdb.exec.Status)
    {
        cdb.exec.StdErr.Close(); // we don't read from this so don't let it block the process
        // we'll read all the initial debugger spew with next line
        var out = cdbDo(cdb, ".echo");
        // we're using CDB to examine user dumps too, so we can't rely on it breaking in a live proc
        // if (!out.match(/ntdll.*!DbgBreakPoint:/m))
        //     throw new Error(1, "Failure starting application\nOuput:\n" + out.replace(/^/mg, "    "));
    }

    return cdb;
}

/*****************************************************************************/
/* Run a cdb command 'cmd' */

function cdbDo(cdb, cmd, expectedPat, echoCmd, callBack)
{
    if (echoCmd == undefined)
        echoCmd = false;

    logMsg(LogScript, LogInfo100, "cdbDo: ", cmd, " {\n");
    cdb.exec.StdIn.Write(cmd + "\r\n");
    if (cmd == "q")
        return "";
    cdb.exec.StdIn.Write(";.echo _CDB_CMD_DONE_ " + cmd + "\r\n");

    if (cdb.exec.Status)
    {
        throw Error(1, "cdbDo: ERROR cdb process has exited!");
    }

    var ret = "";
    if (echoCmd)
        ret += "==>> " + cmd + "\n";
    while (!cdb.exec.StdOut.AtEndOfStream)
    {
        var line = cdb.exec.StdOut.ReadLine();
        // logMsg(LogScript, LogInfo, "LINE:", line, "\n");

        while (line.match(/^\s*\d+:\d+>\s*/))
            line = RegExp.rightContext;

        if (line.match(/^\s*_CDB_CMD_DONE_ /))
            break;
        if (callBack)
            callBack(line)
        else
        {
            logMsg(LogScript, LogInfo100, "Got line ", line, "\n");
            ret += line + "\r";
        }
    }
    logMsg(LogScript, LogInfo100, "} cdbDo returning = {\n", ret, "}\n");
    if (expectedPat && !ret.match(expectedPat))
        throw new Error(1, "Command " + cmd + " output did not match pattern " + expectedPat + "\nOutput: '" + ret + "'\n");
    return ret;
}

/*****************************************************************************/
function cdbLoadStrike(cdb)
{
    if (!cdb.strikeLoaded)
    {
        cdbDo(cdb, ".loadby sos mscorwks", /^$/);
        cdb.strikeLoaded = true;
    }
}

function cdbBangAnalyze(dumpPath, arch)
{
    if (arch == undefined)
        arch = "x86";

    var symPath = "srv*\\\\symbols\\symbols;srv*\\\\bugtraq\\symcache";
    var cmdLine = " -loga theLog.txt -i " + symPath + " -y " + symPath + " -z " + dumpPath;

    logMsg(LogScript, LogInfo, "Analyzing " + dumpPath + ".\n");
    var cdb = cdbNew(cmdLine, arch);

    var tgt = cdbDo(cdb, "vertarget");
    if (tgt.match(/^.* x64/) && arch != "amd64")
    {
        // stop the non-x64 debugger
        cdbDo(cdb, "q");

        cdb = cdbNew(cmdLine, "amd64");
    }

    var res = cdbDo(cdb, "!analyze -v");
    cdbDo(cdb, "q");
    logMsg(LogScript, LogInfo, "Done.\n");
    return res;
}

//////////////////////////////////////////////////////////////////////
///
/// SQL utilities
///
//////////////////////////////////////////////////////////////////////

// Description:
//    This function connects to a database using "conn" connection string and
//    executes the "qry" SQL statement. It returns an array of objects with fields
//    corresponding to the ones specified in the SELECT statement of "qry".
function _ExecuteQry(conn, qry, loglvl)
{
    if (loglvl == undefined)
        loglvl = LogInfo;

    var rgResults = [];

    var connObj = WScript.CreateObject("ADODB.Connection");
	var rs      = WScript.CreateObject("ADODB.Recordset");

	logMsg(LogScript, loglvl, "Retrieving data...\n");
	logMsg(LogScript, LogInfo100, "Query:\n", qry, "\n");

	var indent = logTry();
	try
	{
	    connObj.CommandTimeout = 300;
	    connObj.Open(conn);

	    rs.ActiveConnection = connObj;
		rs.CursorLocation   = adUseClient;
		rs.CursorType       = adOpenKeyset;
		rs.LockType         = adLockOptimistic;
		rs.Open(qry);

		if (rs.RecordCount == 0)
		{
		    logMsg(LogScript, loglvl, "No records matched.\n");
		}
		else
		{
		    logMsg(LogScript, loglvl, "Retrieved ", rs.RecordCount, " records.\n");

		    rs.MoveFirst();

		    var fieldCnt = rs.Fields.Count;
		    var fieldNames = new Array(fieldCnt);
		    var fieldTypes = new Array(fieldCnt);
            for (var i = 0; i < fieldCnt; ++i)
            {
                fieldNames[i] = rs.Fields(i).name;
                fieldTypes[i] = rs.Fields(i).Type;
            }

            rgResults = [];  // new Array(fieldCnt);
            var ixResults = 0;

            // JScript doesn't support multi-dimensional arrays
            // so we'll convert the returned array to a single
            // dimensional JScript array and then display the data.
            tempArray = rs.GetRows();
            recArray = tempArray.toArray();

            var col = 1;
            var rec = new Object();

            for (var linearIdx = 0; linearIdx < recArray.length; linearIdx++)
            {
                if (recArray[linearIdx] == null)
                {
                    var typ = fieldTypes[linearIdx % fieldCnt];
                    if (typ >= adSmallInt && typ <= adCurrency || typ >= adDecimal && typ <= adFileTime)
                        recArray[linearIdx] = 0;
                    else
                        recArray[linearIdx] = 'null';
                }

                rec[fieldNames[linearIdx % fieldCnt]] = recArray[linearIdx];

                col++;
                if (col > fieldCnt)
                {
                    rgResults[ixResults++] = rec;
					rec = {};
                    col = 1;
                }
            }
		}
	}
	catch(e)
	{
	    logCatch(indent);
	    logMsg(LogScript, LogError, e.message + "\n");
	}
	finally
	{
		// cleanup
        if (rs.State == adStateOpen)
			rs.Close;
		if (connObj.State == adStateOpen)
			connObj.Close;
		rs = null;
		connObj = null;
	}

	logMsg(LogScript, loglvl, "Done.\n");

	return rgResults;
}

function _ExecuteNonQry(conn, qry, loglvl)
{
    if (loglvl == undefined)
        loglvl = LogInfo;

    var connObj = WScript.CreateObject("ADODB.Connection");

    logMsg(LogScript, loglvl, "Executing query...\n");
    var rs;
    var result = 0;
    var indent = logTry();
    try
    {
        connObj.CommandTimeout = 300;
        connObj.Open(conn);
        rs = connObj.Execute(qry);

        var rres = _ExecuteQry(conn, "SELECT @@rowcount AS Count", LogInfo100);
        logMsg(LogScript, LogInfo100, dump(rres), "\n");
        result = rres[0].Count;
        logMsg(LogScript, loglvl, result, " rows affected.\n");
    }
    catch (e)
    {
        logCatch(indent);
        logMsg(LogScript, LogError, e.message + "\n");
    }
    finally
    {
        // cleanup
        if (rs.State == adStateOpen)
            rs.Close;
        if (connObj.State == adStateOpen)
            connObj.Close;
        rs = null;
        connObj = null;
    }

    logMsg(LogScript, loglvl, "Done.\n");

    return result;
}

// HTML encoding utility
//
function htmlescape(htmltxt) 
{
    var re = /[<>"'&]/g;
    return htmltxt.replace(re, function(m) { return replacechar(m) })
}

function replacechar(match) 
{
    if (match == "<")
        return "&lt;";
    else if (match == ">")
        return "&gt;";
    else if (match == "\"")
        return "&quot;";
    else if (match == "'")
        return "&#039;";
    else if (match == "&")
        return "&amp;";
}

var ieDone = false;

function ViewHtml(htmlFileName, height, width)
{
    // logMsg(LogClrAutomation, LogInfo, "Creating IE object\n");
    IE = WScript.CreateObject("InternetExplorer.Application", "myIE_");

    IE.height = height;
    IE.width = width;
    IE.left = 0;
    IE.top = 0;
    IE.Visible = true;

    ieDone = false;

    //IE.menubar = 0; IE.toolbar = 0; IE.statusBar = 0;
    IE.Navigate(htmlFileName);

    try
    {
        // the call to IE.Visible guarantees an exception when the IE window is closed
        // since myIE_OnQuit doesn't get called ieDone is never set, therefore terminating 
        // the loop
        while (!ieDone && IE.Visible)
        {
            WScript.Sleep(1000);
        }
    }
    catch (exc) 
    {}

    return;
}

// for some reason this never gets called
function myIE_OnQuit()
{
    //logMsg(LogScript, LogInfo, "IE Quit\n");
    ieDone = true;
}


// Define a filter class that can hold either a JS function or a SQL string that can be used as part of a
// where clause
function mfilter(sFilt)
{
    if (sFilt == undefined)
    { this.JS = this.SQL = null; }
    else if (typeof (sFilt) == "string")
    {
        if (sFilt.match(/.*function\s*\(.*\)/))
        {
            var filt = eval(sFilt);
            if (typeof (filt) != "function")
                throwWithStackTrace(Error("Malformed JS function."));
            this.JS = filt;
            this.SQL = null;
        }
        else
        {
            this.SQL = sFilt;
            this.JS = null;
        }
    }
    else
    { throwWithStackTrace(Error("Unexpected argument.")); }
}

function bugURL(bugID)
{
    return "<a href='http://vstfdevdiv:8080/WorkItemTracking/WorkItem.aspx?artifactMoniker="+bugID+"'>" + bugID + "</a>";
}

function failureURL(fid)
{
    return "<a href='http://windowswatson/ReportServer/Pages/ReportViewer.aspx?/AutoBug%20Detail%20Reports/Failure&rs:Command=Render&FailureID=" + fid + "'>" + fid + "</a>";
}

mfilter.prototype.constructor = mfilter;
mfilter.prototype.isJS = function() { return this.JS != null; }
mfilter.prototype.isSQL = function() { return this.SQL != null; }
