/*********************************************************************************/
/*                              privateStress.js                                 */
/*********************************************************************************/

/* This file contains the functions that prepare private builds for a stress run.
   Currently, this involves only two steps:

    1) Build ndp\clr for all required architectures and flavors.
       To update this list, see task group "prepPrivateStress" in clrtask.js

    2) Robocopy the privates to a central location, managing the number of these
       left around.
*/

/*  AUTHOR: Shu Shen
    Date:   4/22/2010 
    Dependancies: doRun.js, 
                  clrtask.js
                  fso.js
                  util.js          for sortDateFileNamesByDate
                  clrAutomation.js for robocopy
    Owner: ShuShen
*/
/*********************************************************************************/

var MAXPRIVATESTRESSDIRS = 8;  // Maximum private stress runs to save.
var privateStressDirectory = "\\\\clrstresstb\\privates";
var automationDirectory = "\\\\clrstresstb\\automation\\ready";
var stressOptions = undefined;

/******************************************************************************/
/*                            GUI functionality                               */
/******************************************************************************/

// launches GUI and validates inputs
function submitPrivateStress(outDirBase) {
    var initValues = undefined;
    
    IEData = IEFormNew(ScriptDir + "\\scriptLib\\html\\privateStress\\privateStress.html",
        864, 1152, initValues, validateInputData, "validationOptions", initFields, gatherDataFromFields);
    
    IEPoll(IEData); // makes the form visible

    var inputDoc = IEData.inputDoc;

    // wire up events for various UI elements

    // stressOptions will contain all of the validated form data
    stressOptions = IEFormResult(IEData);
    
    // BuildFlavors should be between 1 to 6(all)
    var buildTasks = undefined;

    if (stressOptions.BuildFlavors.length == 6) {
        buildTasks = "prepPrivateStress";
    }
    else {
        var buildTasksPrefix = "prepPrivateStress_";
        buildTasks = buildTasksPrefix + stressOptions.BuildFlavors[0];
        for (var i = 1; i < stressOptions.BuildFlavors.length; i++) {
            buildTasks += "|" + buildTasksPrefix + stressOptions.BuildFlavors[i];
        }
    }
    
    return prepPrivateStress(buildTasks, outDirBase);
}

// called when the Submit button is clicked
function validateInputData(values, validationArgs) {
    if (values.BuildFlavors.length == 0)
        return "Please select at least one build flavor.";
}

// initializes form fields with proper default values
function initFields(IEData, initValues) {
    var MDAs = new Array( "asynchronousThreadAbort", "bindingFailure", "callbackOnCollectedDelegate",
                          "contextSwitchDeadlock", "dangerousThreadingAPI", "dateTimeInvalidLocalFormat",
                          "dirtyCastAndCallOnInterface", "disconnectedContext", "dllMainReturnsFalse",
                          "exceptionSwallowedOnCallFromCom", "failedQI", "fatalExecutionEngineError",
                          // "gcManagedToUnmanaged", "gcUnmanagedToManaged",
                          "illegalPrepareConstrainedRegion", "invalidApartmentStateChange", "invalidCERCall",
                          "invalidFunctionPointerInDelegate", "invalidGCHandleCookie", "invalidIUnknown",
                          "invalidMemberDeclaration", // "invalidOverlappedToPinvoke",
                          "invalidVariant", "jitCompilationStart", "loaderLock",
                          "loadFromContext", "marshalCleanupError", "marshaling",
                          "memberInfoCacheCreation", "moduloObjectHashcode", "nonComVisibleBaseClass",
                          "notMarshalable", "openGenericCERCall", "overlappedFreeError",
                          "pInvokeLog", // "pInvokeStackImbalance",
                          "raceOnRCWCleanup", "reentrancy", // "releaseHandleFailed",
                          "reportAvOnComRelease", "streamWriterBufferedDataLost", "virtualCERCall" );
    
    var GFlags = new Array( "bhd", "cse", "d32", "ddp", "dhc", "dic", "dpd", "dps", "dse", "dwl", "ece",
                            "eel", "eot", "hfc", "hpc", "htc", "htd", "htg", "hvc", "ksl", "kst", "lpg",
                            "ltd", "otl", "ptg", "scb", "shg", "sls", "soe", "vrf"  );
    var GFlagsLabel = new Array ( "Enable bad handles detection", "Early critical section event creation",
                                  "Enable debugging of Win32 subsystem", "Disable kernel mode DbgPrint output",
                                  "Disable heap coalesce on free", "Debug initial command",
                                  "Disable protected DLL verification", "Disable paging of kernel stacks",
                                  "Disable stack extensions", "Debug WINLOGON",
                                  "Enable close exception", "Enable exception logging",
                                  "Enable object handle type tagging", "Enable heap free checking",
                                  "Enable heap parameter checking", "Enable heap tail checking",
                                  "Enable heap tagging by DLL", "Enable heap tagging",
                                  "Enable heap validation on call", "Enable loading of kernel debugger symbols",
                                  "Create kernel mode stack trace database", "Load image using large pages if possible",
                                  "Load DLLs top-down", "Maintain a list of objects for each type",
                                  "Enable pool tagging", "Enable system critical breaks",
                                  "Stop on hung GUI", "Show loader snaps",
                                  "Stop on exception", "Enable application verifier" );
    
    var dom = LoadDOM(privateStressDirectory + "\\stressTestMixes.xml");
    var mixes = dom.selectNodes("//testmixes[1]/*");
    
    var selectElement = IEData.inputDoc.getElementById("TestMixes_input");
    
    // populate TestMix selection options
    for (var i = 0; i < mixes.length; i++) {
        var mix = mixes.nextNode;
        
        if (mix != null)
            addSelectOption(IEData, selectElement, mix.attributes[0].nodeValue, mix.attributes[0].nodeValue, "TestMixes");
    } 
    
    // populate GFlags selection options
    selectElement = IEData.inputDoc.getElementById("GFlagsOptions_input");
    for (var i = 0; i < GFlags.length; i++) {
        addSelectOption(IEData, selectElement, GFlags[i], GFlags[i] + " - " + GFlagsLabel[i], "GFlagsOptions");
    }
    
    // populate MDAs selection options
    selectElement = IEData.inputDoc.getElementById("MDAsOptions_input");
    for (var i = 0; i < MDAs.length; i++) {
        addSelectOption(IEData, selectElement, MDAs[i], MDAs[i], "MDAsOptions");
    }
}

function addSelectOption(IEData, element, value, label, title) {
    var selectOption = IEData.inputDoc.createElement("option");

    selectOption.id = value + "_input";
    selectOption.value = value;
    selectOption.label = label;
    selectOption.title = title;
    selectOption.selected = false;
    element.add(selectOption);
}

// walks all the fields on the form and gather their values
function gatherDataFromFields(IEData) {
    var ret = {};
    
    ret.BuildFlavors = new Array();
    ret.GFlagsOptions = new Array();
    ret.MDAsOptions = new Array();
    ret.TestMixes = new Array();
    ret.EnvironmentVariables = IEData.inputDoc.all.EnvironmentVariables.value;
    
    for (var i = 0; i < IEData.inputDoc.all.length; i++)
    {
        var inputElement = IEData.inputDoc.all[i];
        var id = inputElement.id;
        
        if (!id.match(/^(\w+)_input$/))
            continue;
            
        if (inputElement.type == "checkbox")
        {
            if (inputElement.checked)
            {
                if (inputElement.name == "BuildFlavors")
                    ret.BuildFlavors.push(inputElement.value);
                else if (inputElement.name == "GFlagsOptions")
                    ret.GFlagsOptions.push(inputElement.value);
                else if (inputElement.name == "MDAsOptions")
                    ret.MDAsOptions.push(inputElement.value);
            }
        }
        else if (inputElement.title == "TestMixes")
        {
            if (inputElement.selected)
                ret.TestMixes.push(inputElement.value);
        }
        else if (inputElement.title == "MDAsOptions")
        {
            if (inputElement.selected)
                ret.MDAsOptions.push(inputElement.value);
        }
        else if (inputElement.title == "GFlagsOptions")
        {
            if (inputElement.selected)
                ret.GFlagsOptions.push(inputElement.value);
        }
    }
    
    return ret;
}

/****************************************************************************/
/* Copy the given directory to the stress server.  It maintains a maximum 
   number of directories to be kept on the server.  
   Return 0 on success. Can throw on error.
   sourceDirectory:  The directory containing the builds in subdirectories (Required argument).
*/

function copyPrivateStressBinaries(sourceDirectory) {
    if (sourceDirectory == undefined)
        throw Error(1, "Required argument 'sourceDirectory' not present");
    
    logMsg(LogTask, LogInfo, "START: copyPrivateStressBinaries \n");
    var devDirectory = getUniqueDateFileName(privateStressDirectory, Env("USERNAME") + "_", "");

    // Make sure we limit the total number of stress runs saved.
    var directoryList;

    // Let's be conservative and only delete things that look like an FSOTimeAsFileName
    // This way we can be more confident sortDateFileNamesByDate is appropriate
    directoryList = FSOGetDirPattern(privateStressDirectory, ".*_.*-.*-.*_.*")
    if(directoryList.length > MAXPRIVATESTRESSDIRS) {

        logMsg(LogTask, LogInfo, "Deleting old private stress binaries...\n");

        directoryList.sort(sortDateFileNamesByDate);

        for(var i = 0; i < directoryList.length-MAXPRIVATESTRESSDIRS; i++) {
            try {
                FSODeleteFolder(directoryList[i], true);
            }
            catch(e) {
                logMsg(LogTask, LogInfo, "copyPrivateStressBinaries: failed to remove old private " + directoryList[i] + "\n");
            }
        }
    }

    //Change the dev directory to following format
    // \\clrstresstb\privates\<customername>_blah\raw\20701.00\
    var stressDevDirectory = devDirectory + "\\raw\\20701.00";
    var privateStressInstructions = "Your private binaries have been successfully copied to:\n\n" +
                                    "    " + devDirectory + "\n\n" +
                                    "You dont need to send any emails to clrstres.\n" +
                                    "We will pick the binaries and run stress for you.\n" +
                                    "By default, the stress team will run ShortHaul, Daily GCStress,\n" +
                                    "and 12+ hours of SQLRPS on your checked builds for all architectures\n" +
                                    "(with the exception of IA64 SQLRPS, which can currently run only\n" +
                                    "on retail).  If you are certain that some of these runs are not\n" +
                                    "necessary and you don't want them done then include a justification and\n" +
                                    "send an email to clrstres.\n\n" +
                                    "We will let you know of all new failures found, and any that do not\n" +
                                    "appear to be related to your change can be assigned to DavidGut.\n\n" +
                                    "Remember that 'prepPrivateStress' builds only ndp\\clr.  Please let\n" +
                                    "clrstres know if you need a private stress run on other changes.\n\n";

    var result = robocopy(sourceDirectory, stressDevDirectory, "/NFL");
    if (result == 0)
        logMsg(LogTask, LogInfo, "SUCCESS: Robocopy completed successfully\n");
    else {
        logMsg(LogTask, LogInfo, "FAILED: Robocopy failed unexpectedly during copying the build. \n");
        return -1;
    }
    //We need to change the \\clrstresstb\privates\<customername>\raw\20701.00\<arch>\bin to
    // \\clrstresstb\privates\<customername>\raw\20701.00\binaries.<arch>
    result = _correctFolderNamesAndPlaceSemaphore(stressDevDirectory);
    if (result == -1){
        logMsg(LogTask, LogInfo, "_correctFolderNamesAndPlaceSemaphore: failed to correct the folder names and place semaphores\n");
        return result;
    }
    else     
        logMsg(LogTask, LogInfo, "SUCCESS: _correctFolderNamesAndPlaceSemaphore completed successfully\n");
    
    //output config file
    _outputConfigFile(devDirectory);
    
    //signal RTS that we are ready
    result = _signalRTS(devDirectory);
    if (result == -1) {
        logMsg(LogTask, LogInfo, "_signalRTS: failed to signal the stress server\n");
        return result;
    }
    else 
        logMsg(LogTask, LogInfo, "SUCCESS: _signalRTS completed successfully\n");
    logMsg(LogTask, LogInfo, "\n" + privateStressInstructions);
    return result;
}

function _outputConfigFile(devDirectory)
{
    var fileName = devDirectory + "\\privateStressOptions.config";
    var fileContents = '<?xml version="1.0" encoding="utf-8" ?>\n';
    
    fileContents += '<configuration>\n';
    
    fileContents += '  <buildFlavors>\n';
    for (var i = 0; i < stressOptions.BuildFlavors.length; i++) {
        fileContents += '    <build flavor="' + stressOptions.BuildFlavors[i] + '"/>\n';
    }
    fileContents += '  </buildFlavors>\n';
    
    fileContents += '  <testMixes>\n';
    for (var i = 0; i < stressOptions.TestMixes.length; i++) {
        fileContents += '    <add testmix="' + stressOptions.TestMixes[i] + '"/>\n';
    }
    fileContents += '  </testMixes>\n';

    fileContents += '  <gflagsOptions>\n';
    for (var i = 0; i < stressOptions.GFlagsOptions.length; i++) {
        fileContents += '    <add option="' + stressOptions.GFlagsOptions[i] + '"/>\n';
    }
    fileContents += '  </gflagsOptions>\n';

    fileContents += '  <mdasOptions>\n';
    for (var i = 0; i < stressOptions.MDAsOptions.length; i++) {
        fileContents += '    <add option="' + stressOptions.MDAsOptions[i] + '"/>\n';
    }
    fileContents += '  </mdasOptions>\n';

    var re = /(\w+)=(\w+)/g;
    var result = re.exec(stressOptions.EnvironmentVariables);
    
    fileContents += '  <environmentVariables>\n';
    while (result != null) {
        fileContents += '    <set name="' + result[0] + '"/>\n';
        result = re.exec(stressOptions.EnvironmentVariables);
    }
    fileContents += '  </environmentVariables>\n';
    
    fileContents += '</configuration>\n';
    
    FSOWriteToFile(fileContents, fileName);
}

function _signalRTS(devDirectory)
{
     /******************************************
     * drop a file to \\clrstresstb\automation\ready to signal StressRunManager 
     * to kick off stress against the private. 
     * File name “runprivatestress~<identification_moniker>” with following contents
     * Branch=puclr
     * Privatedrop=\\clrstresstb\privates\myname_06.07.whatever\ 
     * Emailalias=shushen, v-jx, anyone else to be CC’ed on private stress status mails
     ******************************************/
     logMsg(LogTask, LogInfo, "START: _signalRTS \n");
     var fileContents = "Branch = puclr " + " Privatedrop= " + devDirectory;
     fileContents += " Emailalias = shushen,v-jx," + Env("USERNAME");
     var fileName = automationDirectory+ "\\runprivatestress~" + Env("USERNAME");
     FSOWriteToFile(fileContents, fileName);
     return 0;
}

function _correctFolderNamesAndPlaceSemaphore(stressDevDirectory)
{
    // if any of the FSO* function throws we let it propogate out and 
    // print a nice message to the user as to what happened.
    //var archFolderPath = ["\\x86chk","\\x86ret","\\amd64chk","\\amd64ret","\\ia64chk","\\ia64ret"];
    //var binariesFolderPath = ["\\binaries.x86chk","\\binaries.x86ret","\\binaries.amd64chk","\\binaries.amd64ret","\\binaries.ia64chk","\\binaries.ia64ret"];
    var oldFolder;
    var newFolder;
    var temp;
    for (var i = 0 ; i < stressOptions.BuildFlavors.length; i++) {
        oldFolder = "\\" + stressOptions.BuildFlavors[i];
        newFolder = "\\binaries." + stressOptions.BuildFlavors[i];
        
        if (!FSOFolderExists(stressDevDirectory + oldFolder)) {
            logMsg(LogTask, LogInfo, "FAILED:" + stressDevDirectory + oldFolder+ " not found.\n");
            return -1;
        }
        else {
            FSOMoveFolder(stressDevDirectory + oldFolder + "\\bin", stressDevDirectory + newFolder);
            // move any files in the root of the <arch> dir (should only be build logs)
            var dir = WshFSO.GetFolder(stressDevDirectory + oldFolder);
            var e = new Enumerator(dir.files);
            for (; !e.atEnd(); e.moveNext()) {
                // be sure that destination ends with a '\' since we're moving to a directory not a different file name
                FSOMoveFile(e.item(), stressDevDirectory + newFolder + "\\", false);
            }
            // remove the now empty <arch> dir
            FSODeleteFolder(stressDevDirectory + oldFolder, false);
        }
    }
    return 0;
}

/*****************************************************************************/
/* Perform all builds necessary for a private stress run and copy them to the
   stress server.
   The stress server will automatically run the stress for you. 
   
   Parameters
     buildTasks : A regular expression that contains flavors to be built.
     outDirBase : The local base folder where the binaries will be built.
*/

function prepPrivateStress(buildTasks, outDirBase) {
    if (outDirBase == undefined)
        outDirBase = srcBaseFromScript() + "\\automation";
    var outDir = newRunDir(outDirBase);
    var result = 0;
    result = doRunHere(buildTasks, outDir);
    if (result == 0) 
        result = copyPrivateStressBinaries(outDir);
    return result;
}

function LoadDOM(file)
{
   var dom;
   try {
     dom = MakeDOM(null);
     dom.load(file);
   }
   catch (e) {
     alert(e.description);
   }
   return dom;
}

function MakeDOM(progID)
{
  if (progID == null) {
    progID = "msxml2.DOMDocument.6.0";
  }

  var dom;

  try {
    dom = new ActiveXObject(progID);
  dom.setProperty("ResolveExternals", true);  
    dom.async = false;
    dom.validateOnParse = false;
    dom.resolveExternals = true;
  }
  catch (e) {
    alert(e.description);
  }
  return dom;
}

function alert(str)
{
  WScript.Echo(str);
}
