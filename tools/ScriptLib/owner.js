/*****************************************************************************/
/*                              owner.js                                     */
/*****************************************************************************/
/*
   Provides utilities related to CLR source ownership information, such as 
   code review requests, batch updating owners, and generating a spreadsheet
   view. 
   
   pu/clr is the master location for source owners, so these functions will 
   either route through pu/clr or throw if they shouldn't be called from 
   other branches. Basically, getting code review is the only operation 
   that's supported across branches. Other operations, like batch updating 
   owners or generating a spreadsheet view of all owners, will throw if 
   you call from another branch...this helps to avoid stale data floating
   around.
                                                                             */
                                                                             
var ownerModuleDefined = 1;              // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");
if (!ClrAutomationModuleDefined) throw new Error(1, "Need to include CLRAutomation.js");
if (!TfsModuleDefined) throw new Error(1, "Need to include tfs.js");
if (!emailModuleDefined)   throw new Error(1, "Need to include email.js");


var LogOwner = logNewFacility("Owner");

if (Env == undefined)
        var Env = WshShell.Environment("PROCESS");
        
var OWNER_STATE_NO_TAG = 0;
var OWNER_STATE_UNASSIGNED = 1;
var OWNER_STATE_ASSIGNED = 2;  

var validOwnerHash;

/****************************************************************************/
/* Create csv file with file/owner info. Owners are comma-separated on same
   line and files start on new lines.
   
   Example of output in .csv file: 
   
   file_i, owner_i1, owner_i2
   file_j, owner_j
   etc

   options:
       outputFile    - fully qualified output file name. If empty, saved
                       as owners.csv in current dir
*/
function createOwnerList(outputFile) {
    validOwnerHash = WScript.CreateObject("Scripting.Dictionary");

    var srcBase = Env("_NTBINDIR");
    if (srcBase == undefined) {
        throw Error(1, "_NTBINDIR not set");
    }
        
    // We can add exceptions here for those
    // residing in unusual domains.
    // validOwnerHash.Add("cosminr", true);
    // validOwnerHash.Add("simonn", true);

    var ownerFileName;
    if (outputFile == undefined) {
        ownerFileName = "owners.csv";
    }
    else {
        ownerFileName = outputFile;
    }
    var ownerFile = FSOCreateTextFile(ownerFileName, true, false);

    var metricList = [];
    var folderNameList = [];

    
    // we only search 1-level for these
    var flatFolders = [];
    flatFolders.push(srcBase + "\\ndp\\clr\\src\\Utilcode");
    flatFolders.push(srcBase + "\\ndp\\clr\\src\\VM");
        
    for (var j = 0; j < flatFolders.length; j++) {
        var ownerListAndMetrics = getOwnersInFolder(flatFolders[j]);
        var filesAndOwners = ownerListAndMetrics.filesAndOwners;
        for (var i = 0; i < filesAndOwners.length; i++) {
            ownerFile.WriteLine(filesAndOwners[i]);
        }  
        metricList.push(ownerListAndMetrics.folderMetrics);  
        folderNameList.push(flatFolders[j]);
    }
  
    // recursive search for these
    var deepFolders = [];
    ////deepFolders.push(srcBase + "\\ndp\\clr\\src\\BCL");
    deepFolders.push(srcBase + "\\ndp\\clr\\src\\binder");
    ////deepFolders.push(srcBase + "\\ndp\\clr\\src\\coreclr");
    deepFolders.push(srcBase + "\\ndp\\clr\\src\\ClassLibNative");
    deepFolders.push(srcBase + "\\ndp\\clr\\src\\debug");
    deepFolders.push(srcBase + "\\ndp\\clr\\src\\fusion");
    deepFolders.push(srcBase + "\\ndp\\clr\\src\\jit");
 //   deepFolders.push(srcBase + "\\ndp\\clr\\src\\jit64");
    deepFolders.push(srcBase + "\\ndp\\clr\\src\\zap"); 
    deepFolders.push(srcBase + "\\rotor\\pal");
    deepFolders.push(srcBase + "\\rotor\\palrt");

    for (var j = 0; j < deepFolders.length; j++) {
        var ownerListAndMetrics = getOwnersInFolder(deepFolders[j], true);
        var filesAndOwners = ownerListAndMetrics.filesAndOwners;
        for (var i = 0; i < filesAndOwners.length; i++) {
            ownerFile.WriteLine(filesAndOwners[i]);
        }        
        metricList.push(ownerListAndMetrics.folderMetrics); 
        folderNameList.push(deepFolders[j]); 
    }

    ownerFile.Close();

    // report metrics
    var iRetCode = 0;
    for (var i = 0; i < metricList.length; i++) {
        var percent = 0;
        if (metricList[i].fileCount  > 0) {
            percent = metricList[i].ownedCount / metricList[i].fileCount ;
            if (percent < 1) {
                iRetCode = 1;
                logMsg(LogClrTask, LogWarn, folderNameList[i], " should have 100% ownership.\n");
            }
            logMsg(LogClrTask, LogInfo, "Directory: ", folderNameList[i], "\n");
            logMsg(LogClrTask, LogInfo, "    file count = ", metricList[i].fileCount, "\t owned count = " + metricList[i].ownedCount + "\t percent owned = " , percent, "\n\n");
        }
    }

    logMsg(LogOwner, LogInfo, "Owner list saved as ", ownerFileName, ". \n");
 
    return iRetCode;
}

/***********************************************************************/
/* Updates owner tag in specified folder. 

   This expects to be called out of pu/clr, since that's the master 
   location for owner info. It throws otherwise.
   
   options:
       folder    - which folder to update (non-recursive)
       alias     - email alias of owner
       mode      - (optional) 
                   "overwrite" if you want to update the existing 
                   (1st if multiple) owner with the new one.
                   "giveup" won't add the owner if the file already
                   contains an owner
                   Otherwise, it adds this alias to the end of the owner
                   list.
       reformat  - (optional) "true" to reformat header
*/
function updateOwnerInFolder(folder, alias, mode, reformat) 
{
    if (folder == undefined) {
        logMsg(LogOwner, LogWarn, "Specify a folder to update. \n");
        return 0;
    }
    if (alias == undefined) {
        logMsg(LogOwner, LogWarn, "Specify an alias. \n");
        return 0;
    }
    
    var files = FSOGetFilePattern(folder, /\.(cpp|hpp|c|h|inl|cs)$/i, true);

    // check that first file is in puclr
    ensureWorkingInMainBranch(files[0]);

    logMsg(LogOwner, LogInfo, files.length, " files length.\n")
    updateOwnerInFolder_sub(files, alias, mode, reformat);
    
    logMsg(LogOwner, LogInfo, "Updates made.\n");    
    
    return 0;
}


function getCodeReview(shelveset) {

    if (shelveset == undefined) {
        throw Error(1, "Required argument 'shelveset' not present.");
    }
    
    var srcBase = Env("_NTBINDIR");
    if (srcBase == undefined) {
        throw Error(1, "_NTBINDIR not set");
    }
    
    // find owners for each file in change list
    var owners = getOwnersForShelveset(shelveset);
    if (owners == undefined || owners.length == 0) {
        logMsg(LogOwner, LogWarn, "There are no owners for these files. \n");
        return 0;
    }
    
    // send email to owners
    var user = Env("USERNAME");
    var subject = "Code review request from " + user;
    var body = "\n" + user + " would like a code review. Please review and send your feedback. \n" + 
               "tfpt review /shelveset:" + shelveset + ";" + user + "\n" + 
               "\n" +
               "About CLR code reviews:\n" + 
               "http://mswikis/clr/dev/Pages/CLR%20Code%20Reviews.aspx";
    try {               
        logMsg(LogOwner, LogWarn, "owners = " + owners +"\n");
        logMsg(LogOwner, LogWarn, "owners  count = " + owners.length +"\n");
        mailSendHtml(owners, subject, body, undefined); 
        logMsg(LogOwner, LogWarn, "Sent code review request. \n");
    }
    catch(e) {
        logMsg(LogOwner, LogWarn, "Didn't send code review request. " + e.description + "\n");
    }
    return 0;
}


/***********************************************************************/
/* Returns an array of owners for the specified shelveset. 
 */
function getOwnersForShelveset(shelveset) 
{   
    var srcBase = Env("_NTBINDIR");
    if (srcBase == undefined) {
        throw Error(1, "_NTBINDIR not set");
    }
    
    var files = _tfsGetShelvesetEditFiles(shelveset)
    var owners = [];
    
    for(var j = 0; j < files.length ; j++) {
        logMsg(LogOwner, LogInfo, "file: ", files[j], "\n");
    
        // For each file that changed, try to map it to something in puclr and get the owner there.
        // Don't bother with returning stale owner data in current branch. We don't want to encourage
        // people to maintain it there.   
        var changedFile = files[j]
        if (changedFile.match(/(.*)\/(ndp|rotor|tools|developer)\/(.*)/)) {  
            var base = RegExp.$1;
            var dir = RegExp.$2;
            var rest = RegExp.$3;
            var file = srcBase + "/" + dir + "/" + rest;

            var fileOwners;
            // file may not exist in the other branch so handle this possibility and consider it unassigned
            try {
                fileOwners = getOwners(file);
            }
            catch (e) {
                logMsg(LogOwner, LogInfo, "Can't find corresponding file in puclr: ", changedFile, "\n");
                logMsg(LogOwner, LogInfo, "Looked for: ", file, "\n");
            }
            if (fileOwners == undefined) {
                logMsg(LogOwner, LogInfo, "Owner of ", changedFile, " is unassigned", "\n");
            }
            else {
                for (var i = 0; i < fileOwners.length; i++) {
                    logMsg(LogOwner, LogInfo, "Owner of ", file, " is ", fileOwners[i], "\n");
                    owners.push(fileOwners[i]);
                }
            }
        
        }
        // didn't match expected format; but keep trying subsequent files in change list
        else {
            logMsg(LogOwner, LogInfo, "Can't find corresponding file: ", changedFile, "\n");
            logMsg(LogOwner, LogInfo, "Looked for: ", file, "\n");
        }
    }

    return owners;
}



function guessOwners() 
{
    validOwnerHash = WScript.CreateObject("Scripting.Dictionary");
    var srcBase = Env("_NTBINDIR");
    if (srcBase == undefined) {
        throw Error(1, "_NTBINDIR not set");
    }
        
    var ownerFileName = "guess_owners.csv";
    var ownerFile = FSOCreateTextFile(ownerFileName, true, false);
    
    // we only search 1-level for these
    var flatFolders = [];
    flatFolders.push(srcBase + "\\ndp\\clr\\src\\Utilcode");
    flatFolders.push(srcBase + "\\ndp\\clr\\src\\VM");

    ownerFile.WriteLine(_getGuessOwnerHeader());
    for (var j = 0; j < flatFolders.length; j++) {
        var ownerListAndMetrics = getOwnersInFolder(flatFolders[j], false, true);
        var filesAndOwners = ownerListAndMetrics.filesAndOwners;
        for (var i = 0; i < filesAndOwners.length; i++) {
            ownerFile.WriteLine(filesAndOwners[i]);
        }  
    }

    ownerFile.WriteLine();

    ownerFile.Close();
    logMsg(LogOwner, LogInfo, "Created ", ownerFileName, ". \n");
    
    return 0;

}

//////////////////////////////////////////////////////////////////////////////
// Helpers below
//////////////////////////////////////////////////////////////////////////////

/***********************************************************************/
/* getOwnersInFolder and getOwnersInFolder_sub are helpers for 
   createOwnerList. They look for owners for each file in the specified 
   folder and return and array of strings formatted like: 
   "file_i, owner_i1, owner_i2" ...
   for each file in the folder. This is used in the csv output. 
 */

function getOwnersInFolder(folder, recursive, guess)
{
    
    logMsg(LogOwner, LogInfo, "Building owners in folder ", folder, ".\n");

    var files;
    // args arent' working?  
    if (recursive) {
        files = FSOGetFilePattern(folder, /\.(cpp|hpp|c|h|inl|cs)$/i, recursive);
    }
    else {
        files = FSOGetFilePattern(folder, /\.(cpp|hpp|c|h|inl|cs)$/i);
    }
    
    var filesAndOwners = [];
    var folderMetrics = getOwnersInFolder_sub(files, filesAndOwners, guess);
    
    var ownerListAndMetrics = {};
    ownerListAndMetrics.filesAndOwners = filesAndOwners;
    ownerListAndMetrics.folderMetrics = folderMetrics; 
    return ownerListAndMetrics;
}

function getOwnersInFolder_sub(files, filesAndOwners, guess) 
{
    var folderMetrics = {};
    folderMetrics.fileCount = 0;
    folderMetrics.ownedCount = 0;
    var ownerEntrySeparator = ",";

    for (var i = 0; i < files.length; i++ ) {
        var file = files[i];
        if (file instanceof Array) {
            var subFolderMetrics = getOwnersInFolder_sub(file, filesAndOwners);
            folderMetrics.fileCount += subFolderMetrics.fileCount;
            folderMetrics.ownedCount += subFolderMetrics.ownedCount;
        }
        else {
            // skip over generated build files
            if (isIgnorable(file)) {
                continue;
            }
            try {
                var ownersForFile = getOwners(file);
                var ownerList;
                
                // no owner for this file
                if (ownersForFile == undefined) {
                    if (guess) {
                        ownerList = _guessOwner(file);
                    }
                    else {
                        ownerList = "unassigned";
                    }
                    
                    logMsg(LogOwner, LogWarn, "This file doesn't have an owner: ", file, "\n");
                }
                // has an owner
                else {
                    if (guess) continue; // already has owner; continue

                    folderMetrics.ownedCount++;
                    for (var j = 0; j < ownersForFile.length; j++) {
                        ownerList = ownersForFile[j];

                        var isValid = _validateOwner(ownersForFile[j]);
                        if (!isValid) {
                            logMsg(LogOwner, LogWarn, "This owner isn't valid: ", ownersForFile[j], ", file name = ", file, "\n");
                        }

                        if (j < ownersForFile.length - 1) {
                            ownerList = ownerList + ownerEntrySeparator;
                        }
                    }
                } 
                var fileAndOwnersEntry = file + ownerEntrySeparator + ownerList;
                filesAndOwners.push(fileAndOwnersEntry);
                folderMetrics.fileCount++; 
            } catch(e) {
                // log it and continue
                logMsg(LogOwner, LogWarn, "Exception: ", e.description, file, "\n");
            }
        }
    }
    return folderMetrics;
}

/***********************************************************************/
/* Helper for getOwners. This was extracted from getOwners so it can 
   be reused by other functions that have already read all the file 
   lines.
 */
    
function getOwners_Helper(fileLines, endOfHeaderIndex) 
{
    var ownerState = OWNER_STATE_NO_TAG;
    var owners = [];
    
    for(var i = 0; i < endOfHeaderIndex; i++) {
        var line = fileLines[i];
        if ( line.match(/<OWNER>(.*)<\/OWNER>/i) ) {  

            var tempOwner = RegExp.$1;
            if (tempOwner == "" && ownerState != OWNER_STATE_ASSIGNED) {
                ownerState = OWNER_STATE_UNASSIGNED;
            }
            else {
                ownerState = OWNER_STATE_ASSIGNED;
                owners.push(tempOwner);
            }
        }

    }
    if (ownerState == OWNER_STATE_UNASSIGNED || ownerState == OWNER_STATE_NO_TAG) {
        owners = undefined;
    }
    
    return owners;
}

/***********************************************************************/
/* Returns owners of the specified file as an array. Returns undefined if
   the file is unassigned. 
   
   This doesn't do any magic to the file name, such as trying to find the
   corresponding file in puclr.  
 */
function getOwners(fileName) 
{
    if (fileName == undefined) {
        logMsg(LogOwner, LogWarn, "Specify a file name. \n");
        return 0;
    }
    
    var fileLines = FSOReadFromFile(fileName).split(/\s*\n/);
    var endOfHeaderIndex = getEndOfHeaderPosition(fileLines);
    
    
    if (endOfHeaderIndex == undefined) {
        logMsg(LogOwner, LogWarn, "Undefined end of header, returning. ", fileName, "\n");
        return -1;
    }   
 
    var owners = getOwners_Helper(fileLines, endOfHeaderIndex);
    return owners;

}


/***********************************************************************/
/* Helper for updateOwnerInFolder 
 */
function updateOwnerInFolder_sub(files, alias, mode, reformat) 
{
    if (files == undefined) {
        logMsg(LogOwner, LogWarn, "Specify a folder to update. \n");
        return 0;
    }
    if (alias == undefined) {
        logMsg(LogOwner, LogWarn, "Specify an alias. \n");
        return 0;
    }
    
    for (var i = 0; i < files.length; i++ ) {
        var piece = files[i];
        if (piece instanceof Array) {
            logMsg(LogOwner, LogInfo, "recursing.\n");    
            updateOwnerInFolder_sub(piece, alias, mode, reformat);
        }
        else {
            logMsg(LogOwner, LogInfo, "updating file.\n");  
            updateOwnerForFile(piece, alias, mode, reformat)
        }
    }
    
    return 0;
}


/***********************************************************************/
/* Add an owner. 
   This expects to be called out of pu/clr, since that's the master 
   location for owner info. It throws otherwise.
   
   options:
       fileName  - which file to update 
       alias     - email alias of owner
       mode      - (optional) 
                   "overwrite" if you want to update the existing 
                   (1st if multiple) owner with the new one.
                   "giveup" won't add the owner if the file already
                   contains an owner
                   Otherwise, it adds this alias to the end of the owner
                   list.
       reformat  - (optional) "true" to reformat header
*/
function updateOwnerForFile(fileName, alias, mode, reformatHeader)
{
    if (fileName == undefined || fileName == "") {
        logMsg(LogOwner, LogWarn, "Not updating owner for undefined file name. \n");
        return 0;
    }
    
    logMsg(LogOwner, LogInfo, "updateOwnerForFile: ", fileName, "\n");
    
    var fileLinesStr = FSOReadFromFile(fileName);
    var lineEnding = "\r\n";
    var reformat = (reformatHeader == "true");

    if (!fileLinesStr.match(lineEnding)) {

        logMsg(LogOwner, LogWarn, "didn't get expected line ending char 13 chr 10 in file ", fileName, ". Trying chr 10.\n");
        var lineEnding = "\r\n"; 
        if (!fileLinesStr.match(lineEnding)) {
            logMsg(LogOwner, LogWarn, "didn't get line ending chr 10 in file ", fileName, ". This file doesn't have either expected line ending, so skipping.\n");
        } 
        return -1;
    }
    
    var fileLines = fileLinesStr.split(lineEnding);
    var endOfHeaderIndex = getEndOfHeaderPosition(fileLines);
    if (endOfHeaderIndex == undefined) {
        logMsg(LogOwner, LogWarn, "Not updating owner for file: ", fileName, "\n");
        return -1;
    }

    var ownerState = OWNER_STATE_NO_TAG;
    var owners = [];
    var indexToRemember = -1;
    var markerStyle = "// ";
    var detectedMarkerStyle = undefined;
    var detectedBigFormatBlock = false;
    for(var i = 0; i < endOfHeaderIndex; i++) {
        var line = fileLines[i];
        if (detectedMarkerStyle == undefined) {
            if (line.match(/\*\*/)) {
                detectedMarkerStyle = "** ";
            }
            else if (line.match(/^\s*(\/\/.*)?$/)) {
                detectedMarkerStyle = "// ";
            }
            else if (line.match(/\/\*\+\+/)) {
                detectedMarkerStyle = "";
            }
        }
        if ( line.match(/<OWNER>(.*)<\/OWNER>/i) ) {  
            var tempOwner = RegExp.$1;
            logMsg(LogOwner, LogInfo, "found owner ", tempOwner, "\n");
            if (tempOwner == "" && ownerState != OWNER_STATE_ASSIGNED) {
                ownerState = OWNER_STATE_UNASSIGNED;
                indexToRemember = i;
            }
            else {
                ownerState = OWNER_STATE_ASSIGNED;
                indexToRemember = i;
                if ( tempOwner.toString().toLowerCase() == alias.toString().toLowerCase() ) {
                    logMsg(LogOwner, LogWarn, "File already assigned to this alias; returning ", "\n");
                    return 0;
                }
            }
        }
        else if (line.match(/^\/\*\*\*/)) { // remember if we found big formatting blocks like /*******
            detectedBigFormatBlock = true;
        }
    }

    if (!reformat && detectedMarkerStyle != undefined) {
        markerStyle = detectedMarkerStyle;
    }


    var ownerStr = markerStyle + "<OWNER>" + alias + "</OWNER>";

    logMsg(LogOwner, LogInfo, "Remembering index ", indexToRemember, " and owner state = ", ownerState, "\n");

    var newFileLines = []; 
    var seeCodeLine = undefined;
    // no tag
    if (ownerState == OWNER_STATE_NO_TAG) {
        // Skip over standard copyright, insignificant formatting lines at the
        // beginning. Break at words or lines of complete whitespace.
        var startPosition = 0;
        for(startPosition; startPosition < endOfHeaderIndex; startPosition++) {
            var line = fileLines[startPosition];
            if (line.match(/\w/)) {
                // skip over the standard copyright
                if (line.match(/(.*)Copyright(.*)Microsoft Corporation(.*)/)) {
                    newFileLines.push(line);
                    continue;   
                }
                else if (line.match(/See code\:/i)) {
                    if (reformat) {
                        seeCodeLine = line;
                    }
                    else {
                        newFileLines.push(line);
                    }  
                }
                else { 
                    break;
                }
            }
            else if (line.match(/^\s*$/) && detectedMarkerStyle != "") {
                break;
            }
            else {
                if (!reformat && detectedBigFormatBlock && line.match("==--==")) { 
                    newFileLines.push(line);
                    startPosition++;
                    break;
                }
                if (!reformat || !isDiscardableCommentLine(line)) {
                    newFileLines.push(line);
                }
            }
        }
        logMsg(LogOwner, LogInfo, "Start position = ", startPosition, " and end of header index = ", endOfHeaderIndex, "\n");  

        // reformat
        if (reformat) {
            var justFileName = FSOGetFileName(fileName);
            var fileNameLine = markerStyle + "File: " + justFileName;

            newFileLines.push( markerStyle );
            newFileLines.push( fileNameLine );
            newFileLines.push( markerStyle );
            newFileLines.push( ownerStr );

            if (startPosition < endOfHeaderIndex) {
                // The next line might have contained the file name, in which case, we already 
                // added the cleaned-up version. If not, we need to insert the next line.
                var regexStr = justFileName + "$/";              
                var myRegEx = new RegExp(justFileName, "i");
                if (!myRegEx.test(fileLines[startPosition])) { 
                    newFileLines.push( markerStyle );   
                    newFileLines.push( convertCommentLinePrefix(fileLines[startPosition], detectedMarkerStyle, markerStyle) );      
                }
                startPosition++;
            }

            for (startPosition; startPosition < endOfHeaderIndex; startPosition++) {
                var thisLine = fileLines[startPosition];
                if (!thisLine.match("=====")) { // skip crazy formatting lines
                    newFileLines.push( convertCommentLinePrefix(thisLine, detectedMarkerStyle, markerStyle) );
                }
            }
      
            if (seeCodeLine != undefined) {
                newFileLines.push( markerStyle ); 
                newFileLines.push( convertCommentLinePrefix(seeCodeLine, detectedMarkerStyle, markerStyle) );
            }
        }
        else { // reformat
            if (startPosition < endOfHeaderIndex) {
                // The next line might contain the file name. We'll put file, then owner if file 
                // name exists; otherwise we'll put owner before the next line
                var parsedFileName = FSOGetFileNameWithoutExtension(fileName);
                var regexStr = "(" + parsedFileName + ")(.*)";              
                var myRegEx = new RegExp(regexStr, "i");
                if (myRegEx.test(fileLines[startPosition])) {
                    newFileLines.push(fileLines[startPosition]);
                    newFileLines.push( markerStyle );
                    newFileLines.push( ownerStr );    
                }
                else {
                    newFileLines.push( ownerStr ); 
                    //newFileLines.push( detectedMarkerStyle );   
                    newFileLines.push(fileLines[startPosition]);      
                }
                startPosition++;
            }
            else {
                // add owner tag
                newFileLines.push( ownerStr );    
            }
        }

        // and now everything else
        for (var i = startPosition; i < fileLines.length-1; i++) { // regex split will return extra empty str at end
            newFileLines.push( fileLines[i] );
        } 
    }
    else { // no tag

        // If the file is already assigned and we're in "giveup" mode, return
        if ((mode != undefined && mode.toString().toLowerCase() == "giveup" && ownerState == OWNER_STATE_ASSIGNED)) {
            logMsg(LogOwner, LogInfo, "file ", fileName, " already has owner; returning.\n");
            return -1;
        }

        // add all lines up to indexToRemember
        for(var i = 0; i < indexToRemember; i++) {
            newFileLines.push(fileLines[i]);
        }         

        if (ownerState == OWNER_STATE_UNASSIGNED) {
            // has an empty owner tag; just replace old line with this one
            newFileLines.push(ownerStr);  
        }
        else if (ownerState == OWNER_STATE_ASSIGNED) {
            // if in overwrite mode, skip adding current owner
            if ((mode == undefined || mode.toString().toLowerCase() != "overwrite")) {
                newFileLines.push(fileLines[indexToRemember]);
            }  
            // now add new owner
            newFileLines.push(ownerStr);  
        }

        // now add rest of lines
        indexToRemember++;
        for(var i = indexToRemember; i < fileLines.length-1; i++) {
            newFileLines.push(fileLines[i]);
        }
    }

    
    var tempFileName = fileName + ".converted";
    var newFile = FSOCreateTextFile(tempFileName, true, false);
    for (var i=0; i<newFileLines.length;i++) {
        newFile.WriteLine(newFileLines[i]);
    } 
    newFile.close();
    
    if (_inTFS()) {
        _tfsEdit(fileName);
    }
    else {
        sdEdit(fileName, changeNum);
    }
    FSOCopyFile(tempFileName, fileName, true);
    logMsg(LogOwner, LogInfo, "Updated ", fileName, ".\n"); 
    // delete temp file
    FSODeleteFile(tempFileName);

    return 0;
}


function isIgnorable(fileName) {
    return isExcludedFile(fileName) || isBuiltFile(fileName);
}

/***********************************************************************/
/* Any files or dirs you want to exclude from the owner list and metrics
   should be added below.
 */
function isExcludedFile(fileName) {
    return (fileName.match(/(.*)\\rotor\\pal\\win32\\(.*)/i) ||
            fileName.match(/(.*)\\rotor\\pal\\corunix\\arch\\(.*)/i) ||
            fileName.match(/(.*)\\rotor\\pal\\corunix\\examples\\(.*)/i) ||
            fileName.match(/(.*)\\rotor\\pal\\corunix\\safecrt\\(.*)/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\Debug\\tools\\(.*)/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\Debug\\inc\\DDShared.h/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\VM\\ctxtcall.h/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\VM\\Microsoft.ComServices.h/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\VM\\Microsoft.ComServices_i.c/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\jit\\SMData.cpp/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\jit\\SMWeights.cpp/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\jit64\\rotor_ia64\\inc\\(.*)/i) ||
            fileName.match(/(.*)\\ndp\\clr\\src\\jit64\\cmtiming\\(.*)/i)
            );
}

function isBuiltFile(fileName) {
    return fileName.match(/(.*)\\obj1?(core)?(c|d|r)?\\(.*)/);
}

function convertCommentLinePrefix(line, fromPrefix, toPrefix) {
    var returnLine = line.replace(fromPrefix, toPrefix);
    return returnLine;
}

function isDiscardableCommentLine(line) {
    if (line.match("=====")) 
        return true;
    if (line.match(/\*\*/))
        return true;
    return false;
}

/***********************************************************************/
/* Returns the line numbers that marks the end of the header region.
   This currently considers that position to be the first line that is 
   not a comment or whitespace line (by blatantly ripping off Vance's 
   readNonCommentLine), but returns the index instead of the line 
   contents. 
 */
function getEndOfHeaderPosition(fileLines) {
    var position = 0;

    for(;;) {
        if (position >= fileLines.length)
            return undefined;
        var line = fileLines[position];
        // // Style comments are pretty easy, just skip them.
        if (line.match(/^\s*(\/\/.*)?$/)) {
            position++;
            continue;
        }
        //  Are we starting a /**/ style comment?
        if (line.match(/^\s*\/\*(.*)$/)) {
            line = RegExp.$1;        // skip past the /* 

            for(;;) {                // Read until we find the end. 

                if (line.match(/\*\/\s*(.*)/)) {
                    var rest = RegExp.$1
                    if (rest != "")
                        return position;    // We found something beyond the */ terminator, return this position
                    break;    
                }
                if (position >= fileLines.length)
                    throw new Error(1, "End of file in /**/ style comment!");
 
                position++;
                line = fileLines[position];
            }
            position++;
            continue;
        }
        return position;
    }
 
}

/***********************************************************************/
/* Checks if the alias is a valid owner: member of the CLR team or a 
   group alias
 */
function _validateOwner(alias)
{
    if (validOwnerHash != null && validOwnerHash.Exists(alias)) {
        return validOwnerHash.Item(alias);
    }
    var cmdString = "sourceOwner validateowner \"" + alias + "\" ";
    var cmdResult = runCmd(cmdString);
    // just need first line
    var lines = cmdResult.output.split("\r\n");
    var valid = lines[0] == "True";
    validOwnerHash.Add(alias, valid);
    return valid;
}

/***********************************************************************/
/* Reviews tf history and guesses an owner for the file */
function _guessOwner(fileName)
{

    var cmdString = "sourceOwner guessowner \"" + fileName + "\" ";
    var cmdResult = runCmd(cmdString);
    var lines = cmdResult.output.split("\r\n");

    var formattedResult = lines[0];
    for(var i=1;i<lines.length;i++) 
    {
        formattedResult += ",";
        formattedResult += lines[i];
    }
    return formattedResult;
}

function _getGuessOwnerHeader() 
{
    return "source file,dev guess,dev freq,mgr guess,mgr freq,substring match";
}

function _getOwnerMetricHeader() 
{
    return "source folder,file count,owned count,percent owned";
}