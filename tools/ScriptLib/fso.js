/*********************************************************************************/
/*                                 fso.js                                        */
/*********************************************************************************/

/* This is a wrapper of the FileSystemObject that is defined for use in
   JScript.   This wrapper serves three important functions.

    1) It fixes the error reporting in the built in FileSystemObject.  When
       those commands fail, they DON'T tell you the arguments to the routine
       being called, which is REALLY important information.

    2) It allows file system access to be logged using the log.js utility.
       This is a really important way of logging (I want to see all touches
       to the file system.

*/

/*  AUTHOR: Vance Morrison
    Date:   1/26/03 
    Dependancies: log.js
*/
/*********************************************************************************/
var fsoModuleDefined = 1;         // Indicate that this module exists
if (!logModuleDefined) throw new Error(1, "Need to include log.js");

if (WshFSO == undefined)
    var WshFSO = WScript.CreateObject("Scripting.FileSystemObject");

    // If you get an object expected error here, it is because you did not include log.js
var LogFSO = logNewFacility("fso");

var FSOForReading = 1;
var FSOForWriting = 2;
var FSOForAppending = 8;
var FSOTristateUseDefault = -2;  // "open" format use the system default.
var FSOTristateTrue = -1;        // "open" format Unicode.
var FSOTristateFalse = 0;        // "open" format ASCII.


/*********************************************************************************/
/* returns true if 'fileName' exists as a file */

function FSOFileExists(fileName) {

    var ret = WshFSO.FileExists(fileName);
    logMsg(LogFSO, LogInfo1000, "FSOFileExists(", fileName, ") = ", ret, "\n");
    return ret;
}

/*********************************************************************************/
/* returns true if 'folderName' exists as a folder */

function FSOFolderExists(folderName) {
    var ret = WshFSO.FolderExists(folderName);
    logMsg(LogFSO, LogInfo1000, "FSOFolderExists(", folderName, ") = ", ret, "\n");
    return ret;
}

/*****************************************************************************/
/* returns a computer wide unique string.  Useful for generating unique file 
   names */

function FSOUniqueTag() {
    var ret = WshFSO.GetTempName();
    ret = ret.replace(/\.tmp/, "");
    return ret;
}

/*****************************************************************************/
function FSOTimeAsFileName(fineGrained) {

    var time = new Date();
    var timeStr = "";
    timeStr += padLeft(time.getFullYear() % 100, 2, true);
    timeStr += "-";
    timeStr += padLeft(time.getMonth()+1, 2, true);
    timeStr += "-"; 
    timeStr += padLeft(time.getDate(), 2, true);
    timeStr += "_"; 
    timeStr += padLeft(time.getHours(), 2, true);
    timeStr += "."; 
    timeStr += padLeft(time.getMinutes(), 2, true);

    if (fineGrained) {
        timeStr += "."; 
        timeStr += padLeft(time.getSeconds(), 2, true);
        timeStr += "."; 
        timeStr += padLeft(time.getMilliseconds(), 2, true);
    }
    return timeStr;
}

/*****************************************************************************/
/* Get the temporary directory path name */

function FSOGetTempDir() {
    return WshFSO.GetSpecialFolder(2).Path;
}

/*****************************************************************************/
/* return a path name that can be used to create a unique file.  'prefix' is 
   a optional prefix for the file name that is placed before the unique part 
   of the name */

function FSOGetTempPath(prefix) {
    if (prefix == undefined)
        prefix = "";
    var ret = FSOGetTempDir() +  "\\" + prefix + WshFSO.GetTempName();
    logMsg(LogFSO, LogInfo1000, "FSOGetTempPath() = ", ret, "\n");
    return ret;
}

/*********************************************************************************/
function FSOOpenTextFile(fileName, iomode, create, format) {

    logMsg(LogFSO, LogInfo100, "FSOOpenTextFile(", fileName, ", ", iomode, ", ", create, ", ", format, ")\n");
    try {
        return WshFSO.OpenTextFile(fileName, iomode, create, format);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOOpenTextFile(", fileName, ", ", iomode, ", ", create, ", ", format, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOCreateTextFile(fileName, overwrite, unicode) {

    logMsg(LogFSO, LogInfo100, "FSOCreateTextFile(", fileName, ", ", overwrite, ", ", unicode, ")\n");
    try {
        return WshFSO.CreateTextFile(fileName, overwrite, unicode);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOCreateTextFile(", fileName, ", ", overwrite, ", ", unicode, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOGetFile(fileName) {

    logMsg(LogFSO, LogInfo100, "FSOGetFile(", fileName, ")\n");
    try {
        return WshFSO.GetFile(fileName);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOGetFile(", fileName, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
/* Try to delete the file, it will not throw an exception if it fails (but will
   return false */

function FSOTryDeleteFile(fileName, noWarn) {

    try {
        WshFSO.DeleteFile(fileName, true);
    }
    catch(e) {
        if (!noWarn)
            logMsg(LogFSO, LogWarn, "Warning, error deleting ", fileName, ": ", e.description, "\n");
        return false;
    }
    return true;
}

/*********************************************************************************/
/* Helper routine to make a folder, it's sub-folders, and all files writeable.
 */
function FSOMakeWriteable(dirName)
{
    logMsg(LogFSO, LogInfo100, "FSOMakeWriteable(", dirName, ")\n");

    try {
        var folder;
        folder = WshFSO.GetFolder(dirName);

        // Enumerate Folders 
        for (var folderEnum = new Enumerator(folder.SubFolders); !folderEnum.atEnd(); folderEnum.moveNext()) {
            FSOMakeWriteable(folderEnum.item().Path);
        }

        // Process This Folder's Files 
        for (var fileEnum = new Enumerator(folder.Files); !fileEnum.atEnd(); fileEnum.moveNext()) {
               // strip the read-only bit from File
            fileEnum.item().Attributes = (fileEnum.item().Attributes & ~FSOForReading);
        }
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOMakeWriteable(", dirName, ") FAILED\n",
               "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
/* if 'force' is non-null delete a read only file.  If it is 'FORCE' rename it
   out of the way it can't be deleted.  Note FORCE should only be used when
   there a no wildcards involved
   It is not an error if the file does not exist.
 */

function FSODeleteFile(fileName, force) {

    logMsg(LogFSO, LogInfo100, "FSODeleteFile(", fileName, ", ", force, ")\n");

    try {
        var result;

        if (force == "FORCE") {

            // I have had problems with a failure to delete a file leaves the file in 
            // a state were FileExists says it is not there, but it really is.  I am
            // going to try to avoid the problem by moving the file first  then
            // trying to delete it.  

            if (FSOFileExists(fileName)) {
                var deleting = fileName + "." + FSOTimeAsFileName(true) + ".deleting";
                FSOMoveFile(fileName, deleting); // don't force, the target should not exist
                result = FSOTryDeleteFile(deleting, true);
                if (!result)
                    logMsg(LogFSO, LogWarn, "In Use, only renamed ", fileName, "\n");
                
                // clean up stuff possibly left behind from previous usages
                var dir = ".";
                if (fileName.match(/^(.*)\\(.*)$/))
                    dir = RegExp.$1;
                // Pattern match the "." + FSOTimeAsFileName(true) + ".deleting"
                var cleanupFiles = FSOGetFilePattern(dir, /\d+\.\d+\.deleting$/i);
                for(var i = 0; i < cleanupFiles.length; i++) {
                    FSOTryDeleteFile(cleanupFiles[i], true);
                }
            }
            else {
                result = "file did not exist";
            }
                
	    return result;  // success
        }

        var retry   = true;

        if (force != undefined) {
            var delay   = 100;
            var retries = 8;

            while (retry) {
                retry = false;
                try {
                    if (FSOFileExists(fileName))
                        result = WshFSO.DeleteFile(fileName, force != undefined);
                    else
                        result = "file did not exist";
                }
                catch(e) {
                    retry = true;
                    retries--;
                    logMsg(LogFSO, LogWarn, "FSODeleteFile: ", fileName, " failed, will retry the delete in ",
                           delay/1000, " seconds.\n");
                    // Sleep for 'delay' milliseconds before retrying
                    WScript.Sleep(delay);
                    delay *= 2;                  // Double the delay time
                }
                if (retries <= 1) {
                    // Exit the while loop and try one last time without a try/catch
                    break;
                }
            }
        }
        // If force was undefined or we decremented retries to one
        if (retry) {
            if (FSOFileExists(fileName)) {
                result = WshFSO.DeleteFile(fileName, force != undefined);
            }
            else {
                result = "file did not exist";
            }
        }

        return result;
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSODeleteFile(", fileName, ", ", force, ") FAILED\n",
               "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOCopyFileIfExists(srcName, destName, overwrite) {
    if (FSOFileExists(srcName))
        return FSOCopyFile(srcName, destName, overwrite);
}    

/*********************************************************************************/
/* If 'overwrite' is non-null, overwrite an existing file.  If it is 'FORCE' then
   move the original file out of the way, before trying to copy it.  Even if
   the file is locked the new bits get copied.
*/
function FSOCopyFile(srcName, destName, overwrite) {

    logMsg(LogFSO, LogInfo100, "FSOCopyFile(", srcName, ", ", destName, ", ", overwrite, ")\n");

    try {
        var result;

        if (overwrite == "FORCE") {
            // Is the target just the directory?
            if (FSOFolderExists(destName)) {
                var dirName = ".";
                var fileName = srcName;
                if (srcName.match(/(.*)\\([^\\]+)$/)) {
                    dirName = RegExp.$1;
                    fileName = RegExp.$2;
                }
                
                // Does the source have wildcards? In that case we have to expand them.
                if (fileName.match(/\*/)) {
                    fileName = fileName.replace(/(\.|\$)/g, "\\$1");
                    fileName = fileName.replace(/\*/, ".*");
                    
                    var filePaths = FSOGetFilePattern(dirName, "^" + fileName + "$");
                    for (var i = 0; i < filePaths.length; i++) {
                        fileName = filePaths[i].match(/([^\\]+)$/)[1];
                        var destFile = destName + "\\" + fileName;
                        if (FSOFileExists(destFile))
                            FSODeleteFile(destFile, "FORCE");
                        result = FSOCopyFile(filePaths[i], destFile, true);
                    }
                    return result;
                }
                destName = destName  + "\\" + fileName;
            }
            if (destName)       // delete the target 
                FSODeleteFile(destName, "FORCE");
        }

        var retry   = true;

        if (overwrite != undefined) {
            var delay   = 100;
            var retries = 8;

            while (retry) {
                retry = false;
                try {
                    result = WshFSO.CopyFile(srcName, destName, overwrite != undefined);
                }
                catch(e) {
                    retry = true;
                    retries--;
                    logMsg(LogFSO, LogWarn, "FSOCopyFile: ", srcName, " to ", destName, " failed, will retry the copy in ",
                           delay/1000, " seconds.\n");
                    // Sleep for 'delay' milliseconds before retrying
                    WScript.Sleep(delay);
                    delay *= 2;                  // Double the delay time
                }
                if (retries <= 1) {
                    // Exit the while loop and try one last time without a try/catch
                    break;
                }
            }
        }
        // If overwite was undefined or we decremented retries to one
        if (retry) {
            result = WshFSO.CopyFile(srcName, destName, overwrite != undefined);
        }

        return result;

    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOCopyFile(", srcName, ", ", destName, ", ", overwrite, ") FAILED\n", 
               "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOCopyFolder(srcName, destName, overwrite) {

    logMsg(LogFSO, LogInfo100, "FSOCopyFolder(", srcName, ", ", destName, ", ", overwrite, ")\n");
    try {
        return WshFSO.CopyFolder(srcName, destName, overwrite);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOCopyFolder(", srcName, ", ", destName, ", ", overwrite, ") FAILED\n", 
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOMoveFileIfExists(srcName, destName, force) {
    if (FSOFileExists(srcName))
        return FSOMoveFile(srcName, destName, force);
}    

/*********************************************************************************/
/* if force is non-null, move the file even if the destination exists.  If force
   is 'FORCE' then move the target out of the way if it can't be deleted (eg if
   it is locked.  Note that move is not atomic if the destination exists. */

function FSOMoveFile(srcName, destName, force) {

    logMsg(LogFSO, LogInfo100, "FSOMoveFile(", srcName, ", ", destName, ")\n");

    try {
        if (force) 
            FSODeleteFile(destName, force);

        var result;
        var retry   = true;

        if (force != undefined) {
            var delay   = 100;
            var retries = 8;

            while (retry) {
                retry = false;
                try {
                    result = WshFSO.MoveFile(srcName, destName);
                }
                catch(e) {
                    retry = true;
                    retries--;
                    logMsg(LogFSO, LogWarn, "FSOMoveFile: ", srcName, " to ", destName, " failed, will retry the move in ",
                           delay/1000, " seconds.\n");
                    // Sleep for 'delay' milliseconds before retrying
                    WScript.Sleep(delay);
                    delay *= 2;                  // Double the delay time
                }
                if (retries <= 1) {
                    // Exit the while loop and try one last time without a try/catch
                    break;
                }
            }
        }
        // If overwite was undefined or we decremented retries to one
        if (retry) {
            result = WshFSO.MoveFile(srcName, destName);
        }

        return result;

    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOMoveFile(", srcName, ", ", destName, ", ", force, ") FAILED\n", 
               "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOMoveFolder(srcName, destName, force) {

    logMsg(LogFSO, LogInfo100, "FSOMoveFolder(", srcName, ", ", destName, ", ", force, ")\n");
    try {
        if (force && FSOFolderExists(destName)) {
                // To minimize the time that there is neither an old folder or a new folder
                // do the delete after the moves are done
            var deleting = destName + ".deleting";
            FSOMoveFolder(destName, deleting, true);
            var ret;
            try {
                ret = WshFSO.MoveFolder(srcName, destName);
            } finally {
                FSODeleteFolder(deleting, true);
            }
            return ret;
        }
        return WshFSO.MoveFolder(srcName, destName);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOMoveFolder(", srcName, ", ", destName, ") FAILED\n", 
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOCreateFolder(pathName) {
    logMsg(LogFSO, LogInfo100, "FSOCreateFolder(", pathName, ")\n");
    try {
        return WshFSO.CreateFolder(pathName);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOCreateFolder(", pathName, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
function FSOGetFolder(pathName) {
    logMsg(LogFSO, LogInfo100, "FSOGetFolder", "(", pathName, ")\n");
    try {
        return WshFSO.GetFolder(pathName);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSOGetFolder(", pathName, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*********************************************************************************/
/* Like FSODeleteFolder but move the folder before deleting, thus the delete seems
   atomic.  It is also nice in that if it fails, it does not touch the folder.
*/
function FSOAtomicDeleteFolder(folderName) {

    var tmpName = folderName + ".deleting";
    FSOMoveFolder(folderName, tmpName, true)
    FSODeleteFolder(tmpName, true);
}

/*********************************************************************************/
function FSODeleteFolder(folderName, force) {

    logMsg(LogFSO, LogInfo100, "FSODeleteFolder(", folderName, ", ", force, ")\n");
    try {
        return WshFSO.DeleteFolder(folderName, force);
    }
    catch(e) {
        logMsg(LogFSO, LogWarn, "FSODeleteFolder(", folderName, ", ", force, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }
}

/*****************************************************************************/
/* ensure that 'path' (which should be a directory) exists */

function FSOCreatePath(path) {

    var success = false;
    if (FSOFolderExists(path))
        return;

    // TODO we would not need this try, catch if 'throwWithStackTrace worked on recursive functions
    try {
        logMsg(LogFSO, LogInfo100, "FSOCreatePath(", path, ")\n");
        if (path.match(/^(.*)\\([^\\]*)$/)) 
            FSOCreatePath(RegExp.$1)

        if (!FSOFolderExists(path))    // Can happen if path has .. in it
            FSOCreateFolder(path);
        success = true;
    } finally {
        if (!success)
            logMsg(LogFSO, LogWarn, "FSOCreatePath(", path, ") FAILED\n");
    }
}

/*****************************************************************************/
/* ensure that all the directories in 'filepath' are present */

function FSOCreatePathForFile(filePath) {

    if (filePath.match(/^(.*)\\[^\\]*$/))
        FSOCreatePath(RegExp.$1);
}

/*****************************************************************************/
/* writes 'string' to a file named 'fileName'.  appends if 'append' is true */

function FSOWriteToFile(string, fileName, append) {
    
    if (!(LogInfo100 > logGetFacilityLevel(LogFSO))) 
        logMsg(LogFSO, LogInfo100, "FSOWriteToFile(", string.substr(0, 40).replace(/\n/m, " "), "... , ", fileName, ", ", append, ")\n");
    var file = FSOOpenTextFile(fileName, ((append == true) ? FSOForAppending : FSOForWriting), true);
    file.Write(string);
    file.Close();
}

/*****************************************************************************/
/* writes 'string' to a file named 'fileName'.  appends if 'append' is true. Unicode Version */

function FSOWriteToFileUnicode(string, fileName, append) {
    
    if (!(LogInfo100 > logGetFacilityLevel(LogFSO))) 
        logMsg(LogFSO, LogInfo100, "FSOWriteToFile(", string.substr(0, 40).replace(/\n/m, " "), "... , ", fileName, ", ", append, ")\n");
    var file = FSOOpenTextFile(fileName, ((append == true) ? FSOForAppending : FSOForWriting), true, FSOTristateTrue);
    file.Write(string);
    file.Close();
}

/*****************************************************************************/
/* reads an entire file and returns it as a string */

function FSOReadFromFile(fileName) {
    
    logMsg(LogFSO, LogInfo100, "FSOReadFile(", fileName, ")\n");
    var file = FSOOpenTextFile(fileName, FSOForReading);
    var ret = "";
    try { 
        ret = file.ReadAll(); 
    } catch(e) {
        if (e.number != -2146828226)    // we get this if the file is empty
            logMsg(LogFSO, LogWarn, "FSOReadFromFile(", fileName, ") ReadAll failed \n",
                "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
    }
    logMsg(LogFSO, LogInfo100000, "FSOReadFile() = {", ret.substr(0, 240), "... }\n");
    file.Close();
    return ret;
}

/*****************************************************************************/
/* reads an entire file and returns it as a string. Unicode version */

function FSOReadFromFileUnicode(fileName) {
    
    logMsg(LogFSO, LogInfo100, "FSOReadFile(", fileName, ")\n");
    var file = FSOOpenTextFile(fileName, FSOForReading, false, FSOTristateTrue);
    var ret = "";
    try { 
        ret = file.ReadAll(); 
    } catch(e) {
        if (e.number != -2146828226)    // we get this if the file is empty
            logMsg(LogFSO, LogWarn, "FSOReadFromFile(", fileName, ") ReadAll failed \n",
                "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
    }
    logMsg(LogFSO, LogInfo100000, "FSOReadFile() = {", ret.substr(0, 240), "... }\n");
    file.Close();
    return ret;
}

/*********************************************************************************/
/* returns a array of full file path strings for files in 'dirName' and match the regular
   expression pattern 'pat'.  If recursive is true, then search subfolders for matching files too.
   The array is sorted alphabetically */

function FSOGetFilePattern(dirName, pat, recursive) {

    logMsg(LogFSO, LogInfo1000, "FSOGetFilePattern(" + dirName + ", " + pat + ") {\n"); // }

    if (typeof(pat) == "string")
        pat = new RegExp(pat, "i");

    var ret = [];
    var folder;
    if (!FSOFolderExists(dirName))
        return ret;

    try {
        folder = WshFSO.GetFolder(dirName);
    }
    catch (e) {
        logMsg(LogFSO, LogWarn, "FSOGetFolder(", dirName, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }

        /* get files */
    for (var fileEnum = new Enumerator(folder.Files); !fileEnum.atEnd(); fileEnum.moveNext()) {
        var name = fileEnum.item().Name;
        if (pat == undefined || name.match(pat)) 
            ret.push(fileEnum.item().Path);
    }

    if(recursive) {
        for(var folderEnum = new Enumerator(folder.SubFolders); !folderEnum.atEnd(); folderEnum.moveNext()) {
            ret = ret.concat(FSOGetFilePattern(folderEnum.item().Path, pat, recursive));
        }
    }
    
    ret.sort();
    /* { */ logMsg(LogFSO, LogInfo1000, "} FSOGetFilePattern() = [\n    " + ret.join("\n    ") + "\n    ]\n");
    return ret;
}

/*********************************************************************************/
/* returns a array of full directory paths strings  are in 'dirName' and match the regular
   expression pattern 'pat'.  The array is sorted alphabetically */

function FSOGetDirPattern(dirName, pat) {

    logMsg(LogFSO, LogInfo1000, "FSOGetDirPattern(" + dirName + ", " + pat + ") {\n"); // }

    if (typeof(pat) == "string")
        pat = new RegExp(pat, "i");
    
    var ret = [];
    if (!FSOFolderExists(dirName))
        return ret;

    var folder;
    try {
        folder = WshFSO.GetFolder(dirName);
    }
    catch (e) {
        logMsg(LogFSO, LogWarn, "FSOGetFolder(", dirName, ") FAILED\n",
            "    Error(0x", (e.number+0x100000000).toString(16), "): ", e.description, "\n");
        throwWithStackTrace(e);
    }

        /* get directories */
    for (var subFoldersEnum = new Enumerator(folder.SubFolders); !subFoldersEnum.atEnd(); subFoldersEnum.moveNext()) {
        var name = subFoldersEnum.item().Name;
        if (pat == undefined || name.match(pat)) 
            ret.push(subFoldersEnum.item().Path);
    }

    ret.sort();
    /* { */ logMsg(LogFSO, LogInfo1000, "} FSOGetDirPattern() = [\n    " + ret.join("\n    ") + "\n    ]\n");
    return ret;
}

/*********************************************************************************/
/* Clear the read-only bit of a file */
   
function FSOClearReadOnly(fileName) {
    logMsg(LogFSO, LogInfo1000, "FSOClearReadOnly(" + fileName + ") {\n");

    var file = WshFSO.GetFile(fileName);
    
    // Bit 0 is the read-only bit. If it is set, clear it.
    if ((file.attributes & 0x0001) == 0x0001)
    {
        file.attributes = (file.attributes & ~0x0001);
    }    
}

/*****************************************************************************/
/* Returns the file name part of a (possibly) fully qualified path, including
   the extension. For example, if you pass in C:\temp\out.txt, this will 
   return out.txt. */
   
function FSOGetFileName(fullName) {
    return WshFSO.GetFileName(fullName);
}

/*****************************************************************************/
/* Returns the file name without the extension. Accepts fully or non-fully
   qualified paths. */
   
function FSOGetFileNameWithoutExtension(name) {
    // make sure we have only the file name (not the full path)
    var fileName = FSOGetFileName(name);
    var parts = fileName.split(/\./);
    return parts[0];
}

/*****************************************************************************/
/* Returns the extension of a file name. Returns empty string if no extension
   found. */
  
function FSOGetExtension(name) {
    return WshFSO.GetExtensionName(name);
}

function FSOGetDateCreated(name) {
    var f = FSOGetFile(name);
    return f.DateCreated;
}

/*****************************************************************************/
/* Returns the date a file was modified. */

function FSOGetDateLastModified(name) {
    var f = FSOGetFile(name);
    return f.DateLastModified;
}

/*****************************************************************************/
/* Returns newest created sub folder in the specified folder */
function FSOGetLatestSubFolder(name) {
    var folder = WshFSO.GetFolder(name);
    var subFolders = folder.SubFolders;
    var latestDate, latestFolderName;
    for (var myEnum = new Enumerator(subFolders); !myEnum.atEnd(); myEnum.moveNext()) {
        if (myEnum.item().DateCreated > latestDate || (latestDate == undefined)) {
            latestDate = myEnum.item().DateCreated;
            latestFolderName = myEnum.item().name;
        }
    }
    return latestFolderName;
}
