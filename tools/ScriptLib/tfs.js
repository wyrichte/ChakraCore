/******************************************************************************/
/*                                tfs.js                               */
/******************************************************************************/

/* Helper methods for working with TFS.  These methods call into managed executables
 * such as tf, tfpt, tfscript and help to parse the output
 *
 * Note:  Before adding methods to this file, think carefully whether it should be
 * written in managed code instead, such as in tfscript (located in toolbox) for
 * better maintainability and error handling.  Most new functionality should go there,
 * with wrappers here only as needed.
**/

// AUTHOR: Pete Sheill 
// DATE: 4/30/2008

var TfsModuleDefined = 1;              // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");

var LogTfs = logNewFacility("Tfs");

if (ScriptDir == undefined)
        var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

if (WshShell == undefined)
        var WshShell = WScript.CreateObject("WScript.Shell");

if (Env == undefined)
        var Env = WshShell.Environment("PROCESS");


function TFScriptCall()
{
    // tfscript.cmd is preferred as it correctly setup the COMPlus_InstallRoot & COMPlus_Version
    var path = srcBaseFromScript()+"\\tools\\x86\\managed\\v4.0\\tfscript.cmd ";
    return path;
}

/* Query TFS and bring back the results in a tabular format
 
  parameters:
    wiqlfile - name of file containing XML-based representation of query. Export
               a query from VS for an example. 
    columncount - the expected number of columns in each row returned.
 */
function _bugQueryTFS(wiqlfile, columncount)
{
    var commandString = "tfpt query /wiqfile:" + wiqlfile;
    var cmdResult = runCmdToLog(commandString);

    // result has tab-separated fields by default
    var lines = cmdResult.output.split("\r\n");
    var data = new Array();
    for (i=0; i < lines.length; i++)
    {
        var line = lines[i];
        var values = line.split(/\t/);
        if (values.length == columncount) {
            data.push(values);
        }
    }
    return data;
}

/*****************************************************************************/
/* Attach a file attachment to a bug 
 
   returns 0 if successful, 1 if not

   Parameters:
     id           : The id for the bug 
     fileNames    : The names of the files to attach.
*/
function bugAddFileTFS(id, fileNames)
{
    logMsg(LogTfs , LogInfo100, "bugAddFileTFS(", id, ")\n");    
    var commandString = TFScriptCall() + " addAttachments " + id + " ";

    for(var f in fileNames)
    {
        /* BUGBUG Mon 8/11/2008
         * We're apparently filling up the TFS bug server faster than they can add storage.  Try not to
         * attach files that are "big".
         */
        var file = FSOGetFile(fileNames[f]);
        //half a mebibyte sounds like a perfectly good hack.
        if (file.Size < 500000)
        {
            commandString += fileNames[f] + " ";
        }
    }

    // successful upload should be silent
    var cmdResult = runCmdToLog(commandString);
    return cmdResult.exitCode;
}

function _resultStringToArray(resultString)
{
    if ((resultString == null) || (resultString == ""))
    {
        return new Array(0);
    }
    
    return resultString.split("\r\n");
}

/* Open a file for edit in tfs */
function _tfsEdit(filename)
{
    var commandString = "tf edit "+filename;

    var cmdResult = runCmdToLog(commandString);
    return cmdResult.exitCode;

}

// Returns an array of local paths to files that are opened in the specified directory
function _tfsOpened(directory)
{
    var cmdString = TFScriptCall() +" getpendingchanges \"" + directory + "\"";
    var result = runCmdToLog(cmdString);
    return _resultStringToArray(result.output);
}

// Returns the history of the specified file, in the form of an array of strings, with
// each string containing the server path and changeset of the file version (e.g.,
// "$/Dev10/blah/MyFile.txt;C12345"). The caller may specify versionspecs for the from /
// to range of the history to fetch, using the same syntax as allowed on the command
// line.
function _tfsHistory(localPath, versionFrom, versionTo)
{
    var cmdString = TFScriptCall() + " gethistory \"" + localPath + "\" " + versionFrom + " " + versionTo;
    var result = runCmdToLog(cmdString);
    return _resultStringToArray(result.output);
}

// Given a shelveset, returns an array of server paths of the files contained in the
// shelveset marked with an operation of "edit". (Files with other operations, like
// branch or add are intentionally omitted.)
function _tfsGetShelvesetEditFiles(shelveset)
{
    var cmdString = TFScriptCall() + " GetShelvesetEditFiles \"" + shelveset + "\"";
    var result = runCmdToLog(cmdString);
    return _resultStringToArray(result.output);
}

function _tfsQueryWorkItems(query)
{
    var cmdString = TFScriptCall() + " QueryWorkItems \"" + query + "\"";
    var result = runCmdToLog(cmdString);
    return _resultStringToArray(result.output);
}

// Dumps the contents of the TFS file specified by localPath into temp.
function _tfsFetch(localPath, temp)
{
    var cmdString = "tf view /console \"" + localPath + "\"";
    var cmdResult = runCmdToLog(cmdString, runSetOutput(temp));
    return cmdResult.exitCode;
}

// Checks in the specified array of files (given by their server paths).  Returns the
// changeset number for the checkin.
function _tfsCheckin(comment, serverFiles)
{
    var cmdString = TFScriptCall() + " checkin \"" + comment + "\" ";
    for(var i = 0; i < serverFiles.length; i++)
    {
        cmdString += "\"" + serverFiles[i] + "\" ";
    }
    var cmdResult = runCmdToLog(cmdString);
    return cmdResult.output;
}

// Converts a local path on the user's machine to the TFS server path.
function _tfsLocalPathToServerPath(localPath)
{
    var cmdResult = runCmdToLog(TFScriptCall() + " getserverpath \"" + localPath + "\"");
    return cmdResult.output;
}
