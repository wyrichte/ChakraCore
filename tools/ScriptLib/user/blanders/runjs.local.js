

//******************************************************
//* 
//*    PS DDR SCRIPTS
//*
//******************************************************

function revertPublic() {

    var directory = "c:\\vbl\\puclr\\public";
    var sdObj = sdConnect(directory);
    var fileList = sdOpened(directory, sdObj)
    var i;
    for (i = 0; i < fileList.length; i++) {
        logMsg(LogClrAutomation, LogInfo, "reverting:  " + fileList[i] +  " \n");
        sdRevert(fileList[i], sdObj);
    }
    directory = "c:\\vbl\\redbits\\public";
    sdObj = sdConnect(directory);
    fileList = sdOpened(directory, sdObj)
    for (i = 0; i < fileList.length; i++) {
        logMsg(LogClrAutomation, LogInfo, "reverting:  " + fileList[i] +  " \n");
        sdRevert(fileList[i], sdObj);
    }        
    return 0;
}

//******************************************************
//* 
//*    PS AUTOMAIL SCRIPTS
//*
//******************************************************


//******************************************************
//* 
//*    TRUSCAN AUTOMATION SCRIPTS
//*
//******************************************************

/* 
   Run truscan on the .run files in a given directory
 */

// TODO: copy symbols for just the core CLR files.

// TODO: build up a nice symbol path

// TODO: Make a warning message about moving .run files and symbols with autoraid

// TODO: make autoraid optional

// TODO: Figure out a way to get LST GENERATOR UTILITY to make .lst files for idna runs.  
// Basically all tests will be idna.exe & the real test will be prepended to the args
// then I can add a function to convert a test GUID or a .lst to an idna .lst

// TODO: add results dir arg to fixlst



// We should have the results and symbols stored by build #
function doTruscan( buildNum, resultsDir, symbolCache) {

    var folder, fileList, out, i, folderEnum, logFile;
    var delFile, li, rootName, regexp, rootFiles;
    
    if (resultsDir == undefined)
        resultsDir = "\\\\blanders2\\truscan\\testing\\results";

    if (!FSOFolderExists(resultsDir)){
        throw new Error(1, "Can not find results directory " + resultsDir);
    }

    if(buildNum == undefined){
           folder = FSOGetFolder(resultsDir);
           folderEnum = new Enumerator(folder.SubFolders);
           resultsDir = folderEnum.item().Path;
    } else {
        resultsDir = resultsDir + "\\" + buildNum;
    }

    logFile = resultsDir + "\\log.txt";
    if (!FSOFileExists(logFile))
        FSOCreateTextFile(logFile, true);
    
    logSetTranscript(logFile);
    
    if (symbolCache == undefined)
       symbolCache = resultsDir + "\\symbols";

    if (!FSOFolderExists(symbolCache)){
        FSOCreateFolder(symbolCache);
    }

    folder = FSOGetFolder(resultsDir);

    for(folderEnum = new Enumerator(folder.SubFolders); !folderEnum.atEnd(); folderEnum.moveNext()) {
        fileList = FSOGetFilePattern(folderEnum.item().Path, /(.*)\.run$/);    
        logMsg(LogClrAutomation, LogInfo, "*** Running TruScan on .run files in " + folderEnum.item().Path +  " *** \n");
        for(i = 0;  i < fileList.length; i++)
            if ((fileList[i]!=undefined )&& (fileList[i].length > 0)){
                logMsg(LogClrAutomation, LogInfo, "       Running TruScan on  " + fileList[i] +  " \n");
                out = runCmdToLog("\\truscan\\analysis\\truscan.exe -TRACE  " + fileList[i] + " -ENABLECHECKER idnaRaceCondition -TWOPASS -AUTORAID -RETURNBUGCOUNT   -SUMMARIZE  -SYMBOLCACHE  " + symbolCache, runSet32Bit(runSetNoThrow(runSetTimeout(60 * 60))));
                //  -AUTORAID  
                if (out.exitCode == 0){
                    li = fileList[i].lastIndexOf("\\");
                    rootName = fileList[i].slice(li+1, -4); 
                    regexp = rootName + ".*";
                    logMsg(LogClrAutomation, LogInfo, "        No bugs for " + regexp +  ", deleting files \n");    
                    var rootFiles = FSOGetFilePattern(folderEnum.item().Path, regexp);
                    for (var j =0; j < rootFiles.length; j++){
                        FSODeleteFile(rootFiles[j], true);
                        logMsg(LogClrAutomation, LogInfo, "            Deleted " + rootFiles[j] +  " \n");  
                    }
                }
            }
        }
      return 0;
}

function cleanDumpDir(buildNum, resultsDir){

    var folder, folderEnum, fileList, i, bugDb, out, li, rootName, regexp, fileEnum, tmpFolder;

    if (resultsDir == undefined)
        resultsDir = "\\\\blanders2\\truscan\\testing\\results";

    if(buildNum == undefined){
        throw new Error(1, "Need a build directory ");
    } else {
        resultsDir = resultsDir + "\\" + buildNum;
    }
    
    if (!FSOFolderExists(resultsDir)){
        throw new Error(1, "Can not find results directory " + resultsDir);
    }

    folder = FSOGetFolder(resultsDir);
    bugDb = bugConnect("TruScan_staging");

    for(folderEnum = new Enumerator(folder.SubFolders); !folderEnum.atEnd(); folderEnum.moveNext()) {
        fileList = FSOGetFilePattern(folderEnum.item().Path, /(.*)\.run$/);    
        logMsg(LogClrAutomation, LogInfo, "*** Cleaning out dumpfiles in " + folderEnum.item().Path +  " *** \n");
        for(i = 0;  i < fileList.length; i++)
            if ((fileList[i]!=undefined )&& (fileList[i].length > 0)){
                logMsg(LogClrAutomation, LogInfo, "       Looking for bugs using  " + fileList[i] +  " \n");
                out = bugQuery(bugDb, bugQEquals("Trace File", fileList[i]));
                logMsg(LogClrAutomation, LogInfo, "       " + out.length +  " bugs found \n");
                if (out.length == 0){
                    li = fileList[i].lastIndexOf("\\");
                    rootName = fileList[i].slice(li+1, -4); 
                    regexp = rootName + ".*";
                    logMsg(LogClrAutomation, LogInfo, "        No bugs for " + regexp +  ", deleting files \n");    
                    var rootFiles = FSOGetFilePattern(folderEnum.item().Path, regexp);
                    for (var j =0; j < rootFiles.length; j++){
                        FSODeleteFile(rootFiles[j], true);
                        logMsg(LogClrAutomation, LogInfo, "            Deleted " + rootFiles[j] +  " \n");  
                    }
                }
            }
           tmpFolder = FSOGetFolder(folderEnum.item());   
           if (tmpFolder.files.count == 0){
                logMsg(LogClrAutomation, LogInfo, "        Folder  " + folderEnum.item() +  " is empty , deleting folder \n");    
                FSODeleteFolder(folderEnum.item());
            } else {
                logMsg(LogClrAutomation, LogInfo, "        Folder  " + folderEnum.item() +  " has " + tmpFolder.files.count + " items \n");
            }
           
        }
      return 0;
}

function fixLst(lstFileName, newFileName) {

    var lstFile, newFile;
    var nextLine, exeLine, testName;
    var inTestDesc = false;
    var outputArgs = true;

    if (lstFileName == undefined)
        lstFileName = "c:\\vbl\\puclr\\ddsuites\\src\\clr\\x86\\truscantests.lst";
    if (!FSOFileExists(lstFileName))
        throw new Error (1, "file " + lstFileName + " does not exist \n");
    else
        lstFile = FSOOpenTextFile(lstFileName, 1, false);
    
    if (newFileName == undefined)
        newFileName = "c:\\vbl\\puclr\\ddsuites\\src\\clr\\x86\\truscantests-new.lst";
    newFile = FSOCreateTextFile(newFileName, true);
      
    logMsg(LogClrAutomation, LogInfo, "Copying from " + lstFileName + " to " + newFileName + "\n");
    
   var i = -1;
   var eof = false;
    while (!lstFile.AtEndOfStream && ! eof ){
        i++
        nextLine = lstFile.ReadLine();
        if (!inTestDesc){
            // test descriptors section starts with "##=== Test Descriptors"
            if (nextLine.lastIndexOf("##=== Test Descriptors", 23) != -1){
                inTestDesc = true;
            }
        } else {
             // each test desc. starts with "[CheckinBVT"
            if (nextLine.lastIndexOf("[CheckinBVT", 12) != -1){
                testName = nextLine.slice(1, -1);
                
            }   else if (nextLine.lastIndexOf("RelativePath=", 13) != -1){
                // each test command starts with "RelativePath="                   
                newFile.WriteLine("WRAPPER=c:\\truscan\\testing\\runIDna.bat");
                newFile.WriteLine("WRAPPERARGS=" + testName);
                
            } else if (nextLine.lastIndexOf("MaxAllowedDurationSeconds=", 26) != -1){
                // if there is a "MaxAllowedDurationSeconds=" we should increase it
                nextLine = nextLine + "0";

            } else if (nextLine.lastIndexOf("Categories=", 11) != -1){
                nextLine = "Categories=TS";
                
            } else if (nextLine.lastIndexOf("TT_", 3) != -1){
                nextLine = "";                             
                
            } else if (nextLine.lastIndexOf("##=== Test Target", 17) != -1){
                eof = true;
                nextLine = "##=== EOF ==================================================";
            }
        }
        newFile.WriteLine(nextLine);
    }

    return 0;
    
}


