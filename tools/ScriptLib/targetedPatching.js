/****************************************************************************/
/*                              targetedPatching.js                         */
/****************************************************************************/

/* Contains tasks for Native Image Reusability Detection(nird) test. This
   test is only run in servicing enlistment. It checks if VM changes cause 
   changes in ngen images. Steps involved in this test are:
     1. Build baseline sources
     2. Install baseline build
     3. copy ngen images of selected assemblies
     4. Unshelve the specified shelveset which contains servicing changes
     5. Build and install the changed VM
     6. copy new ngen images of same set of assemblies as in step 3
     7. Call nidiffer.exe to detect ngen changes caused by VM changes

   nidiffer.exe resides at tools\devdiv\targetedPatch\nird\bin\nidiffer.exe 
   and sources at qa\clr\testsrc\Desktop\Loader\ngen\TargetedPatching\Tools\NIDiffer
 
   Since this requires baseline build the enlistment should not have any 
   pending changes before starting this test. The test takes as input 
   shelveset which conatins changes to test. After building baseline
   binaries the shelveset is unshelved on the machine. Also the shelveset cannot
   contain changes to following assemblies:
                      "System",
                      "System.Xml",
                      "System.Data.SqlXml",
                      "System.Security",
                      "System.Numerics",
                      "System.Configuration",
                      "System.Core"
   If the fix also requires changes to above assemblies, then
   please shelve VM changes into a separate shelveset and pass that 
   shelveset to DDR. To avoid running this test as part of DDR, run 
   DDR as follows:
        runjs dailyDevRun "skipNird=1"
   It is advised not to do so. This is an essential test required for servicing.
*/

// AUTHOR: Rahul Kumar
// DATE: 11/20/2009

/****************************************************************************/

var targetedPatchingModuleDefined = 1; // Indicate that this module exist

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!TfsModuleDefined) throw new Error(1, "Need to include tfs.js");

var LogTargetedPatching = logNewFacility("targetedPatching");

var tpIncompatMscorlibChange = 0
    
/************************************************************
/* Unshelve the specified shelveset
*/
function _unshelve(shelveset) {
    var run={};
    run.exitCode = 0;
    if(shelveset!=undefined)
        run = runCmdToLog("tf unshelve /noprompt " + shelveset);
    else 
        logMsg(LogTargetedPatching, LogInfo, "No shelveset provided to unshelve.\n");

    if (run.exitCode != 0) {
        throw Error(1, "Could not unshelve shelveset " + shelveset + "\n");
    }

    return 0;
}

/************************************************************
/* Installs the binaries using clrsetup and copies ngen images 
   of selected assemblies to specified folder.
*/
function _captureNIs(outdir, verstr, targetDir, srcBase)
{

    var binDir = outdir + "\\" + verstr + "\\bin";

    var setupArgs = "/lg /fx /nrad /nowpf /noindigo /nongeninstallqueue /noz ";

    var installDir= clrSetup(binDir, setupArgs, outdir, verstr);

    FSOCreatePath(targetDir);

    _nirdStabilizeIL(installDir, srcBase, targetDir, binDir);

    var ngenCmd = installDir + "\\ngen.exe";

    var defaultVersion = installDir.match(/^(.*)\\(.*)/)[2];
    var ngenRunOpts = runSetEnv("COMPlus_DefaultVersion", defaultVersion);

    //Let's add version out of paranoia.
    ngenRunOpts = runSetEnv("COMPlus_Version", defaultVersion, ngenRunOpts);
        
    //and let's add a timeout too.
    ngenRunOpts = runSetTimeout(20 * MINUTE, ngenRunOpts);

    var assemblies = ["mscorlib",
                      "System",
                      "System.Xml",
                      "System.Data.SqlXml",
                      "System.Security",
                      "System.Numerics",
                      "System.Configuration",
                      "System.Core"];

    for(var j = 0; j < assemblies.length; ++j)
    {
        var cmdLine = ngenCmd + " install /nologo /NoDependencies";
        cmdLine += " " + assemblies[j];
        runCmdToLog(cmdLine, ngenRunOpts);
    }

    for(var j = 0; j < assemblies.length; ++j)
    {
        var cmdLine = ngenCmd + " display /verbose";
        cmdLine += " " + assemblies[j];
        var ngenImageFile = runCmdToLog(cmdLine, ngenRunOpts).output;
        if(!runCmdToLog(cmdLine, ngenRunOpts).output.match(/File: *(.*)/))
        {
            throw new Error(1, "NIDiffCaptureBaseline: Could not get Ngen Image location");
        }

        ngenImageFile = trim(RegExp.$1);

        if (!FSOFileExists(ngenImageFile)) {
            throw new Error(1, "NIDiffCaptureBaseline: Ngen Image not found");
        }

        FSOCopyFile(ngenImageFile, targetDir + "\\" + FSOGetFileName(ngenImageFile), true);
    }

    FSOCopyFile(installDir + "\\clr.dll", targetDir + "\\clr.dll", true);

    FSOCopyFile(installDir + "\\clrjit.dll", targetDir + "\\clrjit.dll", true);
}

/************************************************************
/* This is the main function which calls nidiffer.exe to 
   diff baseline and new ngen images of selected assemblies.
*/
function _diffNIs(srcBase, verstr, baselineVMDir, newVMDir)
{
    var baselineClrTimestamp = FSOGetDateCreated(baselineVMDir + "\\clr.dll");
    var newClrTimestamp = FSOGetDateCreated(newVMDir + "\\clr.dll");

    if(baselineClrTimestamp < newClrTimestamp || baselineClrTimestamp > newClrTimestamp) {
    
        var cmdLine = srcBase + "\\tools\\devdiv\\targetedPatch\\nird\\bin\\NIDiffer.exe VmChange";
        //clr version should not change just on rebuild. So passing dummy values.
        cmdLine += " /oldVerstr:v4.0.0.0 /newVerstr:v4.0.0.0";
        cmdLine += " /oldVm:" + baselineVMDir + "\\clr.dll /newVm:" + newVMDir + "\\clr.dll";
        cmdLine += " /oldJit:"+ baselineVMDir + "\\clrjit.dll /newJit:" + newVMDir + "\\clrjit.dll";
        cmdLine += " /oldNIs:" + baselineVMDir + " /newNIs:" + newVMDir;
        cmdLine += " /allowChangesCausedByILRecompilation ";

        //Assuming that mscorlib.dll only changes by changes in dir ndp\clr\src\bcl
        //If mscorlib.dll has changed then pass mscorlibILWillChange switch to nidiffer
        var openedFiles = _tfsOpened(srcBase+"\\ndp\\clr\\src\\bcl");
        if(openedFiles.length != 0)
            cmdLine += " /mscorlibILWillChange";

        var versionDir = "v4.0." + verstr;

        var runOpts =
            runSetEnv(
                "COMPlus_DefaultVersion",
                versionDir,

            runSetEnv(
                "COMPlus_Version",
                versionDir,

            runSetNoThrow()));

        var run = runCmdToLog(cmdLine, runOpts);

        if (run.exitCode != 0) {
            throw Error(
                1,

                "\n" +
                "\n" +
                "Changes in the current shelveset appear to cause ngen image changes.\n" +
                "These changes should cause re-ngen of all assemblies on customer's machine.\n" + 
                "However due to targeted patching changes this will not take place automatically.\n" + 
                "\n" +
                "For this to happen please follow below steps:\n" +
                "\n" +
                "      1. Increment TP (targeted patching) Band of mscorlib (obtained from your build)\n" + 
                "             tools\\devdiv\\targetedPatch\\ilca\\ilca.exe incrementband mscorlib.dll\n" +
                "\n" +
                "      2. Checkin mscorlib.dll at redist\\targetedPatch\\baseline.<arch>\\mscorlib.dll (and also pdb)\n" +
                "         along with your changes.\n" +
                "\n" +
                "    The above steps need to be done for all 3 architectures.\n" +
                "\n" +
                "But before please send email to tpreview alias stating that tp-incompatible VM changes are being made.\n" +
                "\n");
        }
    } 
    else {
        logMsg(LogScript, LogInfo, "Since clr.dll has not changed native images can be re-used wrt clr.dll. Hence not" + 
                                   " calling nidiffer.exe\n");
    }


    return 0;
}


function _nirdStabilizeIL(installDir, srcBase, targetDir, binDir)
{
    var cmdLine;
    var run;
    var ilca = srcBase + "\\tools\\devdiv\\targetedPatch\\ilca\\ilca.exe";
    var ibcmerge = srcBase + "\\tools\\x86\\managed\\v4.0\\ibcmerge.exe";

    var assemblies = ["mscorlib",
                      "System",
                      "System.Xml",
                      "System.Data.SqlXml",
                      "System.Security",
                      "System.Numerics",
                      "System.Configuration",
                      "System.Core"];

    FSOCreatePath(targetDir + "\\temp");

    var defaultVersion = installDir.match(/^(.*)\\(.*)/)[2];
    var complus_runOpts = runSetEnv("COMPlus_DefaultVersion", defaultVersion);

    var ibcmerge_complus_runOpts = runSetEnv("COMPlus_InstallRoot", srcBase + "\\tools\\x86\\managed\\");
    ibcmerge_complus_runOpts = runSetEnv("COMPlus_Version", "v4.0", ibcmerge_complus_runOpts);

    for(var j = 0; j < assemblies.length; ++j) {

        var tempFile = targetDir + "\\temp\\" + assemblies[j] + ".dll";

        var assembly = installDir + "\\" + assemblies[j] + ".dll";

        if(targetDir.match(/baselineVM$/))
            var contractAssembly = assembly;
        else if(targetDir.match(/newVM$/))
            var contractAssembly = targetDir + "\\..\\baselineVM\\temp\\" + assemblies[j] + ".dll";
        else
            throw new Error(1, "nirdStabilizeIL: Unknown target directory " + targetDir);

        if(!_isIBCDataAttached(assembly)) {
            cmdLine = ilca + " applyContract " + assembly + 
                             " /contract:" + contractAssembly + 
                             " /allpublic /out:" + tempFile;
            run = runCmdToLog(cmdLine, ibcmerge_complus_runOpts);


        }
        else {
            cmdLine = ibcmerge + " -f -delete " + assembly;
            run = runCmdToLog(cmdLine, ibcmerge_complus_runOpts);
        
            cmdLine = ilca + " applyContract " + assembly + 
                             " /contract:" + contractAssembly + 
                             " /allpublic /out:" + tempFile;
            run = runCmdToLog(cmdLine);

            cmdLine = ibcmerge + " -mo " + tempFile + " -incremental " + binDir+"\\"+assemblies[j]+".dll";
            run = runCmdToLog(cmdLine, ibcmerge_complus_runOpts);
        }    

        cmdLine = "binplace -R " + installDir + " -:DEST retail " + tempFile;
        run = runCmdToLog(cmdLine);

        runCmdToLog(installDir + "\\gacutil -i " + assembly, complus_runOpts);
    }
}

function _isIBCDataAttached(assembly)
{
    var cmdLine = "ibcmerge -mi " + assembly;
    var ibcmerge_runOpts = runSetEnv("COMPlus_DefaultVersion", "", runSetNoThrow());
    ibcmerge_runOpts = runSetEnv("COMPlus_Version", "", ibcmerge_runOpts);
    var run = runCmdToLog(cmdLine, ibcmerge_runOpts);

    if (run.exitCode==0) 
        return 1;
    else 
        return 0;
}

/************************************************************
/* Verifies that there are no pending changes
*/
function _nirdPendingChanges(srcBase)
{
    var openedFiles = _tfsOpened(srcBase);
    if(openedFiles.length != 0)
        throw new Error(1, "There are pending changes in dir " + srcBase+"\\ndp. " + 
                           "Please undo the changes to start DDR. DDR in servicing enlistment " + 
                           "has a new test for targeted patching which requires baseline build.\n");

    logMsg(LogTargetedPatching, LogInfo, "There are no pending changes.\n");

    return 0;
}

/************************************************************
/* Verifies that the shelveset does not change the following
   assemblies:
                      "System"
                      "System.Xml"
                      "System.Data.SqlXml"
                      "System.Security"
                      "System.Numerics"
                      "System.Configuration"
                      "System.Core"
*/
function _nirdVerifyShelveset(shelveset)
{

    var editFiles = _tfsGetShelvesetEditFiles(shelveset);
    for(var i = 0; i< editFiles.length; i++)
    {
        if(editFiles[i].match(/\/ndp\/fx\/src\//))
            throw new Error(1, "Shelveset changes files in  dir ndp\\fx\\src." + 
                               "Please remove changes in dir ndp\\fx\\src from the shelveset to run this.");

        if(editFiles[i].match(/\/ndp\/clr\/src\/ManagedLibraries\/Security\//))
            throw new Error(1, "Shelveset changes files in  dir ndp\\clr\\src\\ManagedLibraries\\Security." + 
                               "Please remove changes in dir ndp\\clr\\src\\ManagedLibraries\\Security" + 
                               " from the shelveset to run this.");

    }

    logMsg(LogTargetedPatching, LogInfo, "Shelveset does not modify assemblies System.dll, System.Xml.dll, " + 
                                         "System.Data.SqlXml.dll, System.Security.dll, System.Numerics.dll, " +
                                         "System.Configuration.dll, System.Core.dll.\n");

    return 0;
}


/************************************************************
/* Detects if NIRD test is to be run or not.
   NIRD is supposed to be run only in servicing enlistment 
   and for Desktop DDR. Servicing enlistment is detected by 
   the presence of folder "redist\targetedPatch".
*/
function _shouldRunNird()
{
    var ret = 0;
    var shelvesetProvided = 0;
    var shelveset;
    if(FSOFolderExists(srcBaseFromScript()+"\\redist\\targetedPatch"))
    {
        var myarg = WScript.Arguments;
        for(i=0;i<myarg.length;i++)
        {
            if(myarg.Item(i).match(/skipNird=1/i)) return 0;
            if(myarg.Item(i).match(/^dailyDevRun$/)) ret = 1;
            if(myarg.Item(i).match(/^ddr.Desktop$/)) ret = 1;
            if(myarg.Item(i).match(/^targetedPatchingCompatibilityTest$/)) ret = 1; 
            if(myarg.Item(i).match(/^StorePrevMscorlibTPBand$/)) ret = 1;
            if(ret == 1)
            {
                if(myarg.Item(i).match(/shelveset=(.*?)(;|$)/i)) {
                    shelveset = RegExp.$1;
                    shelvesetProvided = 1;
                }
            }
        }

        if(ret == 1 && shelvesetProvided == 0)
        {
            logMsg(LogClrTask, LogError, 
                   "No Shelveset specified. Please specify shelveset as below :\n" +
                   " runjs dailyDevRun \"shelveset=myShelvesetName\"\n" + 
                   " runjs doRun targetedPatchingCompatibilityTest _ _ \"shelveset=myShelvesetName\"\n" +
                   "DDR in servicing enlistment has a new test required for targeted patching. " + 
                   "This test requires baseline build. Hence ensure that there are no changes in " + 
                   "your enlistment and specify your changes in a shelveset as above. More details at " + 
                   "http://mswikis/clr/dev/Pages/Targeted%20Patching%20VM%20Compatibility%20Test.aspx \n");
            WScript.Quit();
        }

        if(ret == 1)
        {
            var openedFiles = _tfsOpened(srcBaseFromScript());
            if(openedFiles.length != 0)
            {
                logMsg(LogClrTask, LogError, 
                   "There are pending changes in your enlistment. Please undo those changes. " + 
                   "This is required for targeted patching test. More details at " +
                   "http://mswikis/clr/dev/Pages/Targeted%20Patching%20VM%20Compatibility%20Test.aspx \n");
                WScript.Quit();
            }
        }

        if(ret == 1) {
            var editFiles = _tfsGetShelvesetEditFiles(shelveset);
            for(var i = 0; i<editFiles.length; i++) {
                if(editFiles[i].match(/\/redist\/targetedPatch\/baseline\.(x86|ia64|amd64)\/mscorlib\.dll/i)) {
                    tpIncompatMscorlibChange = 1;
                    logMsg(LogClrTask, LogInfo, "native Image Reusability Detection test is not required as mscorlib.dll"+
                                                "is being serviced in a tp incompatible way. \n");
                    return 1;
                }
            }
        }
    }
    return ret;
}

/************************************************************
/* Main Nidiffer task.
*/
function _nirdTask(buildType, buildArch)
{
    if(tpIncompatMscorlibChange == 0) {

        var baselineVMDir = "%outDir%\\targetedPatchingCompatibilityTest\\baselineVM";
        var newVMDir = "%outDir%\\targetedPatchingCompatibilityTest\\newVM";
        var ret = taskNew("targetedPatchingCompatibilityTest",
                          "runjs _diffNIs %srcBase% " + buildArch+buildType + " " + baselineVMDir + " " + newVMDir,
                          [_nirdCaptureBaselineNITask(buildType, buildArch, baselineVMDir),
                           _nirdCaptureNewNITask(buildType, buildArch, newVMDir)
                  ]);

        ret.description = "Checks if VM changes causes ngen images to differ";
    } 
    else {

        var ret = taskNew("targetedPatchingCompatibilityTest",
                          "runjs _comparenewTPBand %srcBase% %outDir%",
                          [_unshelveTask("%shelveset%", [taskNew("StorePrevMscorlibTPBand", 
                                                                "runjs _storePrevTPBand %srcBase% %outDir%",
                                                                [taskNew("PendingChanges", "runjs _nirdPendingChanges %srcBase%")])
                  ])]);

        ret.description = "Checks that the tp band number in new contract mscorlib.dll changes.";
    }

    ret.moreInfoUrl = "http://mswikis/clr/dev/Pages/Targeted%20Patching%20VM%20Compatibility%20Test.aspx";
    return ret;
}

/************************************************************
/* Task to install baseline and copy NIs
*/
function _nirdCaptureBaselineNITask(buildType, buildArch, targetDir)
{
    var taskName = "captureBaselineNIs";
    var ret = taskNew(taskName,
                      "runjs _captureNIs " + "%outDir% " + buildArch+buildType + " \"" + targetDir + "\"" + " %srcBase%",
                      [_nirdBaselineBuildTask(buildType, buildArch)]);

    ret.description = "Causes ngen of selected assemblies and copies them over to " + 
                      "run.current\\targetedPatchingCompatibilityTest\\baselineVM";
    return ret;
}

/************************************************************
/* Task to install changed VM and copy new NIs
*/
function _nirdCaptureNewNITask(buildType, buildArch, targetDir)
{
    var taskName = "captureNewNIs";
    var ret = taskNew(taskName,
                      "runjs _captureNIs " + "%outDir% " + buildArch+buildType + " \"" + targetDir + "\"" + " %srcBase%",
                      [_nirdBuildNewVMTask(buildType, buildArch)]);

    ret.description = "Causes ngen of selected assemblies and copies them over to " + 
                      "run.current\\targetedPatchingCompatibilityTest\\newVM";
    return ret;
}

/************************************************************
/* Task to build baseline sources
*/
function _nirdBaselineBuildTask(buildType, buildArch)
{
    var ret = _razzleBuildTask(buildType, buildArch, "ndp", "-cZ");
    ret.dependents = [taskNew("PendingChanges", "runjs _nirdPendingChanges %srcBase%")];
    ret.name = "baseline_" + ret.name;
    return ret;
}

/************************************************************
/* Task to do incremental build of changed sources
*/
function _nirdBuildNewVMTask(buildType, buildArch)
{
    var dependants = [_unshelveTask("%shelveset%", [_nirdBaselineBuildTask(buildType, buildArch),
                                                    taskNew("VerifyShelveset", "runjs _nirdVerifyShelveset %shelveset%")
                     ])];
    var ret = _razzleBuildTask(buildType, buildArch, "ndp", "-cZ", undefined, undefined, dependants);
//    ret.name = "newVM_" + ret.name;
    return ret;
}

/************************************************************
/* Task to unshelve the changes
*/
function _unshelveTask(shelveset, dependents)
{
    if (shelveset == undefined)
        shelveset = "_";
    if (dependents == undefined)
        dependents = [];

    var taskName = "unshelve";
    var ret = taskNew(taskName, "runjs _unshelve " + shelveset, dependents);

    ret.description = "This task unshelves the specified shelveset.";
    return ret;
}

function _GetTPBand(srcBase)
{
    var contractAssemblyX86 = srcBase + "\\redist\\targetedPatch\\baseline.x86\\mscorlib.dll";
    var contractAssemblyAmd64 = srcBase + "\\redist\\targetedPatch\\baseline.amd64\\mscorlib.dll";
    var contractAssemblyIa64 = srcBase + "\\redist\\targetedPatch\\baseline.ia64\\mscorlib.dll";
    var ilca = srcBase + "\\tools\\devdiv\\targetedPatch\\ilca\\ilca.exe";
 
    var cmdLine = ilca + " showband /nologo " + contractAssemblyX86;
    var run = runCmdToLog(cmdLine, runSetNoThrow());
    x86TPBand = run.output;

    cmdLine = ilca + " showband /nologo " + contractAssemblyAmd64;

    //remove nothrow whrn ilca.exe is fixed to not return exitcode 1
    run = runCmdToLog(cmdLine, runSetNoThrow());
    amd64TPBand = run.output;

    var re = new RegExp("^" + amd64TPBand.replace(/\./g,"\\.") + "$") ;

    if(!x86TPBand.match(re))
        throw new Error(1, "TP Band number " + x86TPBand + " in file " + contractAssemblyX86 + " and tp band number " + 
                           amd64TPBand + " in file " + contractAssemblyAmd64 + " do not match.");

    cmdLine = ilca + " showband /nologo " + contractAssemblyIa64;
    run = runCmdToLog(cmdLine, runSetNoThrow());
    ia64TPBand = run.output;

    re = new RegExp("^" + ia64TPBand.replace(/\./g,"\\.") + "$") ;

    if(!x86TPBand.match(re))
        throw new Error(1, "TP Band number " + x86TPBand + " in file " + contractAssemblyX86 + " and tp band number " + 
                           ia64TPBand + " in file " + contractAssemblyIa64 + " do not match.");

    return x86TPBand;

}

function _storePrevTPBand(srcBase, outDir)
{
    var targetedPatchDir = outDir + "\\targetedPatchingCompatibilityTest";
    FSOCreatePath(targetedPatchDir);
    var tpBand = _GetTPBand(srcBase);

    FSOWriteToFile(tpBand, targetedPatchDir+"\\prevTPBand.txt");

    logMsg(LogTargetedPatching, LogInfo, "Mscorlib.dll is being patched in tp-incompatible manner." +
                                         " Bandnumber of current contract assembly is " + tpBand + 
                                         ". Stored the band number for verification in file " + 
                                         targetedPatchDir+"\\prevTPBand.txt.\n");

    return 0;
}

function _comparenewTPBand(srcBase, outDir)
{
    var targetedPatchDir = outDir + "\\targetedPatchingCompatibilityTest";
    var tpBandNew = _GetTPBand(srcBase);

    var tpBandPrev = FSOReadFromFile(targetedPatchDir+"\\prevTPBand.txt");

    var re = new RegExp("^" + tpBandPrev.replace(/\./g,"\\.") + "$") ;

    if(tpBandNew.match(re))
        throw new Error(1, "TP Band number " + tpBandPrev + " in baseline mscorlib matches the TPBand in mscorlib included in shelveset.");

    logMsg(LogTargetedPatching, LogInfo, "Mscorlib.dll is being patched in tp-incompatible manner." +
                                         " Verified that the bandnumber in new contract assembly is different" + 
                                         " from the previous assembly.\n");

    return 0;
}

