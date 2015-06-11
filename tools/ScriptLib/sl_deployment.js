/*****************************************************************************/
/*                               sl_deployment.js                            */
/*****************************************************************************/

/* Commands for building the Silverlight installers */

/* AUTHOR: dharvey (sldept) 
   Date:   6/18/07
   Dependancies: fso.js
                 log.js
                 run.js
*/

/*****************************************************************************/
var silverlightDeploymentModuleDefined = 1; // Indicate that this module exists

if (!fsoModuleDefined)   throw new Error(1, "Need to include fso.js");
if (!logModuleDefined)   throw new Error(1, "Need to include log.js");
if (!runModuleDefined)   throw new Error(1, "Need to include run.js");
if (!Env)                throw new Error(1, "Need to have Env define. This script expects it to be defined by run.js");

/******************************************************************************/
/**************************** Main functions **********************************/
/******************************************************************************/

/******************************************************************************/
/* 
   Builds the installer requested. This includes building directories, running
     setupbuild, and post-setupbuild steps.

   Arguments:
     layer:       optional. Valid layers are "main" and "dev". Choosing
                    "main" will build a normal runtime installer, choosing
                    "dev" will build a runtime installer which also lays down
                    debugger files. Defaults to "main".
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function buildInstaller(layer, buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, layer);
    _buildDirs(env);
    _makeMsis(env);
    _makeInstallers(env, tempFolder, false);
}

/******************************************************************************/
/* 
   Makes the installer requested. This includes running setupbuild, and
     post-setupbuild steps.

   Arguments:
     layer:       optional. Valid layers are "main" and "dev". Choosing
                    "main" will build a normal runtime installer, choosing
                    "dev" will build a runtime installer which also lays down
                    debugger files. Defaults to "main".
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function makeInstaller(layer, buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, layer);
    _makeMsis(env);
    _makeInstallers(env, tempFolder, false);
}

// Version that makes only a basic installer for fast builds
function makeInstallerFast(layer, buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, layer);
    _makeMsis(env);
    _makeInstallersWithCompression(env, tempFolder, false, "7zip", "");
}

/******************************************************************************/
/* 
   Packages the installer requested. This includes post-setupbuild steps.

   Arguments:
     layer:       optional. Valid layers are "main" and "dev". Choosing
                    "main" will build a normal runtime installer, choosing
                    "dev" will build a runtime installer which also lays down
                    debugger files. Defaults to "main".
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function packInstaller(layer, buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, layer);
    _makeInstallers(env, tempFolder, false);
}

/******************************************************************************/
/* 
   Makes all the installers. This includes running setupbuild, and
     post-setupbuild steps.

   Arguments:
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function makeAllInstallers(buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, "main");
    _makeMsis(env);
    _makeInstallers(env, tempFolder, false);
    env.layer = "dev";
    _makeInstallers(env, tempFolder, false);
}

/******************************************************************************/
/* 
   Builds the layer 1 installer (corresponds to the 2.0 release) this includes 
     just the Jolt binaries/updater, CoreCLR, slr, and various extra 
     components, like the DLR. The installer gets placed in
     <setupsRoot>\setup\sfxs\<buildArch><buildType>\enu\silverlight\2\
     Silverlight.2.0.exe

   Arguments:
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function buildLayer1Installer(buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, "main");
    _buildDirs(env);
    _makeMsis(env);
    _makeInstallers(env, tempFolder, false);
}

/******************************************************************************/
/* 
   Makes the layer 1 installer (corresponds to the 2.0 release) this includes 
     the Jolt binaries/updater, CoreCLR, slr, and various extra 
     components, like the DLR. The installer gets placed in
     <setupsRoot>\setup\sfxs\<buildArch><buildType>\enu\silverlight\2\
     Silverlight.2.0.exe

   Expects:
     a full build has been performed and is in <buildPath>

   Arguments:
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function makeLayer1Installer(buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, "main");
    _makeMsis(env);
    _makeInstallers(env, tempFolder, false);
}

/******************************************************************************/
/* 
   Packages the layer 1 installer (corresponds to the 2.0 release) this 
     includes the Jolt binaries/updater, CoreCLR, slr, and various extra 
     components, like the DLR. The installer gets placed in
     <setupsRoot>\setup\sfxs\<buildArch><buildType>\enu\silverlight\2\
     Silverlight.2.0.exe

   Expects:
     a full build has been performed and is in <buildPath>
     setupbuild -all has been run
     The install executables should be placed in <buildPath>\signed\23\. It is
       not necessary for them to be signed

   Arguments:
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function packLayer1Installer(buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{    
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, "main");
    _makeInstallers(env, tempFolder, true);
}

/******************************************************************************/
/* 
   Runs the Deployments tests

   Expects:
     A build of the installer is available

   Arguments:
     categories:  optional. the categories of tests that should be run,
                    defaults to checkinbvt
     buildArch:   optional. bitness of build, defaults to x86.
     buildType:   optional. Type of build, defaults to chk. 
     buildPath:   optional. Folder that contains binaries. This folder should be
                    writable and contain a full build. If this is not set, it
                    will expect binaries to be in 
                    <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:  optional. Folder where the MSIs, installer, and setupbuild 
                    intermediate files get placed. If this is not set, it 
                    defaults to <vblRoot>Setup\
     vblRoot:     optional. Folder that contains this enlistment. If this is not
                    set, the script will attempt to guess it based on the 
                    location of this script.
     tempFolder:  optional. Folder to write temporary files to. If this is 
                    specified, temporary files will not be deleted.
     buildBranch: optional. Name of the branch this is being built in, defaults
                    to Silverlight_W2
 */
function runDeploymentTests(categories, buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{    
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, "main");
    if (undefined == categories)
    {
        categories = "checkinbvt";
    }
    runCmdToLog(env.vblRoot + "\\ddsuites\\src\\Deployment\\x86\\RunSLInts.bat silverlight_w2 00000.00 " + env.buildType + " " + env.setupsRoot + "\\setup\\sfxs\\" + env.buildArch + env.buildType + "\\enu\\silverlight\\ " + categories, 
                runSetCwd(env.vblRoot + "\\ddsuites\\src\\Deployment\\x86\\",
                runSetTimeout(60 * 60))); // Sets timeout to 60 mintues
}

/******************************************************************************/
/**************************** Helper functions ********************************/
/******************************************************************************/

/******************************************************************************/
/* These functions can be used on their own, but are probably not very interesting
 * stand alone. 
 */


/******************************************************************************/
/* 
   This builds multiple installers and is put under "Helper functions" because
     it should only be interesting to members of the Silverlight Deployment 
     team.

   Arguments:
     buildNumber:      optional. Build number to start at, defaults to 21210
     count:            optional. Numbers of builds to make, defaults to 3.
     performFullBuild: optional. Set this to "true" if a full build should be
                         performed, defaults to "false"
     buildSavePath:    optional. Location to save installers, defaults to 
                         <buildPath>
     buildArch:        optional. bitness of build, defaults to x86.
     buildType:        optional. Type of build, defaults to chk. 
     buildPath:        optional. Folder that contains binaries. This folder should be
                         writable and contain a full build. If this is not set, it
                         will expect binaries to be in 
                         <vblRoot>\binaries\<buildArch><buildType>\
     setupsRoot:       optional. Folder where the MSIs, installer, and setupbuild 
                         intermediate files get placed. If this is not set, it 
                         defaults to <vblRoot>Setup\
     vblRoot:          optional. Folder that contains this enlistment. If this is not
                         set, the script will attempt to guess it based on the 
                         location of this script.
     tempFolder:       optional. Folder to write temporary files to. If this is 
                         specified, temporary files will not be deleted.
     buildBranch:      optional. Name of the branch this is being built in, defaults
                         to Silverlight_W2
 */
function _buildMultipleInstallers(buildNumber, count, performFullBuild, buildSavePath, buildArch, buildType, buildPath, setupsRoot, vblRoot, tempFolder, buildBranch)
{   
    var env = _createEnv(vblRoot, setupsRoot, buildPath, buildArch, buildType, buildBranch, 1);
    if (performFullBuild == "true")
    {
        _buildLayer1Dirs(env);
    }
    if (undefined == count)
    {
        count = 3;
    }
    if (undefined == buildNumber)
    {
        buildNumber = 21300;
    }
    if (undefined == buildSavePath)
    {
        buildSavePath = env.buildPath;
    }

    runCmdToLog("sd edit " + env.vblRoot + "\\public\\products\\Silverlight\\wave2\\version.txt");
    
    for (var i = 0 ; i < count ; ++i)
    {
        FSOWriteToFile("1.1." + (buildNumber + i) + ".0", env.vblRoot + "\\public\\products\\Silverlight\\wave2\\version.txt");
        runCmdToLog("cleanstore -all");
        razzleBuild(env.buildType, env.buildArch, "versioning", undefined, env.vblRoot, env.buildPath);
        razzleBuild(env.buildType, env.buildArch, "deployment", undefined, env.vblRoot, env.buildPath);
        _makeInstallers(env, tempFolder, false);
        runCmdToLog("mkdir " + buildSavePath + "\\" + (buildNumber + i), runSetNoThrow());
        FSOCopyFile(env.buildPath + "\\bin\\i386\\vssetup\\uiwrapper\\installwave2.pdb", buildSavePath + "\\" + (buildNumber + i) + "\\", "FORCE");
        FSOCopyFile(env.setupsRoot + "\\setup\\sfxs\\" + env.buildArch + env.buildType + "\\enu\\silverlight\\main\\Silverlight.exe", buildSavePath + "\\" + (buildNumber + i) + "\\", "FORCE");
    }

    runCmdToLog("sd revert " + env.vblRoot + "\\public\\products\\Silverlight\\wave2\\version.txt");
}


/******************************************************************************/
/*
   Build the directories required for the runtime installer
*/
function _buildDirs(env, runOpts)
{ 
    razzleBuild(env.buildType, env.buildArch, "versioning", undefined, env.vblRoot, env.buildPath);
    razzleBuild(env.buildType, env.buildArch, "ndp",        undefined, env.vblRoot, env.buildPath);
    razzleBuild(env.buildType, env.buildArch, "xcp",        "-z",      env.vblRoot, env.buildPath);
    razzleBuild(env.buildType, env.buildArch, "deployment", undefined, env.vblRoot, env.buildPath);
    razzleBuild(env.buildType, env.buildArch, "slfx",       undefined, env.vblRoot, env.buildPath);
    razzleBuild(env.buildType, env.buildArch, "vb",         undefined, env.vblRoot, env.buildPath);
}

/******************************************************************************/
/* 
   Run the setupbuild commands to build the MSIs. The MSIs gets placed in
     <setupsRoot>\layouts\<buildArch><buildType>\enu\silverlight\[1|2mix]\cd\
     and
     <setupsRoot>\layouts\<buildArch><buildType>\enu\silverlight\Base\cd\

   Expects:
     a full build has been performed and is in <buildPath>
  
   Arguments:
     env:        Environment settings
     runOpts:    A context to run setupbuild in, it will be passed directly to 
                   runCmdToLog
 */
function _makeMsis(env, runOpts)
{   
    var parts = env.vblRoot.split(":",2);
    logMsg(LogScript, LogInfo100, "Disabling partial builds\r\n");
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\buildconfig /alter:SetupsBuild.PartialBuild.Enabled /value:False",
                runSetEnv("_NTTREE", env.buildPath, 
                runSetEnv("_NTDRIVE", parts[0] + ":",
                runSetEnv("_NTROOT",  parts[1],
                runSetEnv("_BuildBranch", env.buildBranch,
                runSetEnv("_BuildArch", env.buildArch,
                runSetEnv("_BuildType", env.buildType,
                runSetEnv("DEVPATH", env.vblRoot + "\\tools\\x86\\managed\\v2.0\\", runOpts))))))));

    var setupBuildEnv = runSetEnv("_NTTREE", env.buildPath,
                        runSetEnv("COMPLUS_InstallRoot", env.vblRoot + "\\tools\\x86\\managed", 
                        runSetEnv("DEVDIV_TOOLS", env.vblRoot + "\\Tools\\\devdiv",
                        runSetEnv("SDXROOT", env.vblRoot,
                        runSetEnv("COMPLUS_Version", "v2.0",
                        runSetEnv("_NTDRIVE", parts[0] + ":",
                        runSetEnv("_NTROOT",  parts[1], runOpts)))))));

    logMsg(LogScript, LogInfo100, "Making Silverlight_Main\r\n");                        
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\setupbuild /name Silverlight_Main /lang enu /media cd /chip x86", setupBuildEnv);
    logMsg(LogScript, LogInfo100, "Making Silverlight_Base" + "\r\n");
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\setupbuild /name Silverlight_Base /lang enu /media cd /chip x86", setupBuildEnv);
    logMsg(LogScript, LogInfo100, "Making Silverlight_Install" + "\r\n");
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\setupbuild /name Silverlight_Install /lang enu /media cd /chip x86", setupBuildEnv);
    logMsg(LogScript, LogInfo100, "Making Silverlight_Dev" + "\r\n");
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\setupbuild /name Silverlight_Dev /lang enu /media cd /chip x86", setupBuildEnv);
}
  
/******************************************************************************/
/* 
   Run the candle/light commands to build the MSP. You most likely want to run 
     makeLayer[0|1]Installer, which will build the MSP and additionally the 
     self extracting executable. The MSP gets placed into
     <buildPath>\mspName

   Expects:
     the appropriate MSIs have been built
  
   Arguments:
     env:        Environment settings
     tempFolder: A folder to store temporary files in.
     runOpts:    A context to run setupbuild in, it will be passed directly 
                   to runCmdToLog
     compression: Optional parameter specifying compression for MSP file
                    if undefined, default msimsp compression ("lzx18") is used
                    Supported values: "uncompressed", "lzx21", "lzx18"
 */
function _makePatch(env, tempFolder, runOpts, compression)
{   
    var run = runCmd(env.vblRoot + "\\tools\\x86\\uuidgen");
    var guid = run.output;
    guid = guid.replace("\r\n","");

    // MsiMsp doesn't clean up after itself so we have to remove this directory
    runCmdToLog("rmdir /s /q " + _Env("temp") + "\\~pcw_tmp.tmp\\", runSetNoThrow(runOpts));
    
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\candle.exe -dwave=" + _GetWaveEnding(env.layer) + " -dflavor=" + env.buildType +
                      " -dsetupsroot=" + env.setupsRoot + " -dguid=" + guid + 
                      " -dversion=" + _GetVersion(env) + " " + env.vblRoot + 
                      "\\deployment\\installer\\win\\msi\\patch.wsx -out " + tempFolder + "patchv" + _GetWaveEnding(env.layer) + ".wixobj", 
                runSetEnv("DEVPATH", env.vblRoot + "\\tools\\x86\\managed\\v2.0\\", runOpts));
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\light " + tempFolder + "patchv" + _GetWaveEnding(env.layer) + ".wixobj -out " + tempFolder + "patchv" + _GetWaveEnding(env.layer) + ".pcp", 
                runSetEnv("COMPLUS_InstallRoot", env.vblRoot + "\\tools\\x86\\managed", runOpts));

    if (compression == "lzx21" || compression == "uncompressed")
    {
        // This is a hack to allow us to build MSP files with different compression algorithms
        // MsiMsp will invoke makecab.exe, so here we are saving original makecab.exe and
        // replacing it with a wrapper, which invokes the saved makecab.exe with the necessary parameters
        makecabWrapper = env.vblRoot + "\\deployment\\installer\\win\\makecab-wrappers\\makecab-" + compression + "\\makecab-" + compression;
        makecabOriginal = env.vblRoot + "\\tools\\devdiv\\decatur\\makecab";

        // Save the original makecab.exe (to restore it later)
        FSOCopyFile(makecabOriginal + ".exe", makecabOriginal + "-original.exe", "FORCE");
        // And replace it with our wrapper
        FSOCopyFile(makecabWrapper + ".exe", makecabOriginal + ".exe", "FORCE");
    }
    else if (compression == "lzx18" || compression == undefined)
    {
        // LZX:18 is default compression, we don't need to do anything
    }
    else
    {
       logMsg(LogScript, LogWarn, "Warning: Unknown compression specified in _makePatch: compression="+compression+", using LZX:18");
    }

    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\MsiMsp -s " + tempFolder + "patchv" + _GetWaveEnding(env.layer) + ".pcp -p " + env.buildPath + "\\" + _GetMspName(env.layer) + " -l " + tempFolder + "msplog.txt", runOpts);

    if (compression == "lzx21" || compression == "uncompressed")
    {
        // Restore the original makecab.exe
        FSOCopyFile(makecabOriginal + "-original.exe", makecabOriginal + ".exe", "FORCE");
    }
    
    var parts = env.vblRoot.split(":",2);
    
    runCmdToLog(env.vblRoot + "\\tools\\devdiv\\decatur\\SignSLFiles.exe " + env.buildPath + "\\" + " " + env.buildPath + "\\signed\\23\\" + " " + _GetMspName(env.layer), 
                runSetEnv("DEVPATH", env.vblRoot + "\\tools\\x86\\managed\\v2.0\\", 
                runSetEnv("_NTTREE", env.buildPath,
                runSetEnv("_NTDRIVE", parts[0] + ":",
                runSetEnv("_NTROOT",  parts[1],
                runSetEnv("_BuildBranch", env.buildBranch, 
                runSetEnv("_BuildArch", env.buildArch, 
                runSetEnv("_BuildType", env.buildType, runOpts))))))));

    runCmdToLog("mkdir " + env.setupsRoot + "\\setup\\sfxs\\" + env.buildArch + env.buildType + "\\enu\\silverlight\\Msp", runSetNoThrow(runOpts));
    FSOCopyFile(env.buildPath + "\\signed\\23\\" + _GetMspName(env.layer), env.setupsRoot + "\\setup\\sfxs\\" + env.buildArch + env.buildType + "\\enu\\silverlight\\Msp\\" + _GetMspName(env.layer), "FORCE");
}

/******************************************************************************/
/* 
   Makes an installer using functions provided.

   Arguments:
     env:            Environment settings
     tempFolder:     optional. Folder to write temporary files to. If this is 
                       specified, temporary files will not be deleted.
     signedInstall : true if install executables have already been signed
 */
function _makeInstallers(env, tempFolder, signedInstall)
{
    _makeInstallersWithCompression(env, tempFolder, signedInstall, "7zip",  "");
    _makeInstallersWithCompression(env, tempFolder, signedInstall, "lzx18", "_Offline");
    if (env.layer == "main")
    {
        _makeInstallersWithCompression(env, tempFolder, signedInstall, "7zip",  "_MSN");
    }
}

/******************************************************************************/
/* 
   Makes an installer using functions provided.

   Arguments:
     env:            Environment settings
     tempFolder:     optional. Folder to write temporary files to. If this is 
                       specified, temporary files will not be deleted.
     signedInstall : true if install executables have already been signed
     compression:    optional. Specifies compression ("7zip" or "lzx21"), if omitted default LZX:18 compression is used
     sfxComprSuffix: Suffix to be added to built SFX installer name (currently used to distinguish different compression types)
 */
function _makeInstallersWithCompression(env, tempFolder, signedInstall, compression, sfxComprSuffix)
{
    var deleteTemps = false;

    if (tempFolder == undefined)
    {
        tempFolder = FSOGetTempPath("silverlightInstaller") + sfxComprSuffix + "\\";
        deleteTemps = true;
    }
    else
    {
        if (tempFolder[tempFolder.length-1] == '\\')
        {
            tempFolder = tempFolder.substr(0, tempFolder.length-1)
        }
        tempFolder = tempFolder + sfxComprSuffix + "\\";
    }

    if (FSOFolderExists(tempFolder))
    {
        logMsg(LogScript, LogWarn, "Warning: ", tempFolder, " already exists. Pre-existing files may cause installer generation to fail.\r\n");
    }
    else
    {
        FSOCreateFolder(tempFolder);
    }

    var runOpts = runSetTimeout(72 * 60); /* Sets the timeout to be 72 minutes */

    logMsg(LogScript, LogInfo100, "Making MSPs\r\n");
    
    var makePatchCompression = compression;
    if (makePatchCompression == "7zip")
    {
        makePatchCompression = "uncompressed";
    }

    _makePatch(env, tempFolder, runOpts, makePatchCompression);
    if (env.layer == "dev")
    {
        env.layer = "main";
        _makePatch(env, tempFolder, runOpts, makePatchCompression);
        env.layer = "dev";
    }

    logMsg(LogScript, LogInfo100, "Making sfxcab.exe with the right version number\r\n");
    FSOCopyFile(env.vblRoot + "\\tools\\x86\\sfxcab.exe", tempFolder);
    FSOMakeWriteable(tempFolder);

    runCmdToLog("echo FileVersion=" + _GetVersion(env), runSetOutput(tempFolder + "mapfile", runOpts));
    runCmdToLog(env.vblRoot + "\\ndp\\clr\\snap2.4\\tasks\\VerEdit.exe " + tempFolder + "sfxcab.exe " + tempFolder + "mapfile");

    logMsg(LogScript, LogInfo100, "Copying files the installer needs\r\n");
    FSOCreateFolder(tempFolder + "cab");
    FSOMakeWriteable(tempFolder + "cab");

    if (env.layer == "dev")
    {
        FSOCopyFile(env.buildPath + "\\signed\\23\\" + _GetMspName(env.layer), tempFolder + "cab\\SilverlightDev.msp");
        FSOCopyFile(env.buildPath + "\\signed\\23\\" + _GetMspName("main"),    tempFolder + "cab\\Silverlight.msp");
    }
    else
    {
        FSOCopyFile(env.buildPath + "\\signed\\23\\" + _GetMspName(env.layer), tempFolder + "cab\\Silverlight.msp");
    }

    FSOCopyFile(env.setupsRoot + "\\layouts\\" + env.buildArch + env.buildType + "\\enu\\Silverlight\\Install\\cd\\install.resWAVE2.dll", 
                tempFolder + "cab\\install.res.dll");

    // We have to sign the file manually because Decatur doesn't support setting the display name
    var installer = env.buildPath + "\\install" + _GetWaveName(env.layer);
    if (compression == "7zip")
    {
        // For 7zip based installers with use installer.exe with 7z decompression
        if (sfxComprSuffix == "_MSN")
        {
            FSOCopyFile(env.buildPath + "\\installWAVE2-7zip-MSN.exe", tempFolder + _GetUACName(env.layer), "FORCE");

            // Copy DefaultManagerSetup.exe
            FSOCopyFile(env.vblRoot + "\\deployment\\installer\\win\\msn\\DefaultManagerSetup.exe", tempFolder + "cab\\DefaultManagerSetup.exe");
        }
        else
        {
            FSOCopyFile(installer + "-7zip.exe", tempFolder + _GetUACName(env.layer), "FORCE");
        }
    }
    else
    {
        // For regular LZX based installers we use default installer.exe
        FSOCopyFile(installer + ".exe", tempFolder + _GetUACName(env.layer), "FORCE");
    }
    runCmdToLog("SignSLFiles.exe " + tempFolder + " " + tempFolder + "signed\\23\\ \"" + _GetUACName(env.layer) + "\"", runOpts);
    FSOCopyFile(tempFolder + "signed\\23\\" + _GetUACName(env.layer), tempFolder + "cab\\install.exe", "FORCE");

    FSOCopyFile(env.setupsRoot + "\\layouts\\" + env.buildArch + env.buildType + "\\enu\\silverlight\\Base\\cd\\Silverlight_Base.msi",
                tempFolder + "cab\\Silverlight.msi");

    if (compression == "7zip")
    {
        logMsg(LogScript, LogInfo100, "Compressing .MSP file using 7zip\r\n");
        var cmd7zip = env.vblRoot + "\\deployment\\installer\\win\\LzmaSDK\\7z.exe";
        var mspFileName = tempFolder + "cab\\Silverlight"
        runCmdToLog(cmd7zip + " a -m0=BCJ2 -m1=LZMA:d26:fb96:lc8:pb1 -m2=LZMA:fb96 -m3=LZMA:fb96 -mb0:1 -mb0s1:2 -mb0s2:3 " + mspFileName + ".7z " + mspFileName + ".msp");
        runCmdToLog("del /f " + mspFileName + ".msp");
    }


    if (sfxComprSuffix == "_Offline")
    {
        logMsg(LogScript, LogInfo100, "Copying localized licenses for _Offline installer\r\n");
        FSOCopyFolder(env.vblRoot + "\\deployment\\Resources\\Legal\\current", tempFolder + "cab", true);
    }

    logMsg(LogScript, LogInfo100, "Making descriptor for the self extracting executable\r\n");
    var runGenddfOpts = runSetOutput( tempFolder + "silverlight.ddf", runOpts);
    runCmdToLog(env.vblRoot + "\\tools\\x86\\genddf silverlight.cab " + tempFolder + "cab /run install.exe", runGenddfOpts);

    logMsg(LogScript, LogInfo100, "Making the self extracting executable\r\n");
    runCmdToLog(env.vblRoot + "\\tools\\x86\\makecab /f " + tempFolder + "silverlight.ddf", runOpts);
    /* This is a hack because makecab doesn't allow you to use absolute paths in the file genddf makes */
    FSOMoveFile("silverlight.cab", tempFolder + "silverlight.cab");
    runCmdToLog(env.vblRoot + "\\tools\\x86\\makesfx /run " + tempFolder + "silverlight.cab /stub " + tempFolder + "sfxcab.exe", runOpts);

    var outputDir = env.setupsRoot + "\\setup\\sfxs\\" + env.buildArch + env.buildType + "\\enu\\silverlight\\" + _GetWaveEnding(env.layer);
    var sfxName = "Silverlight" + _GetSfxEnding(env.layer) + sfxComprSuffix + ".exe";

    logMsg(LogScript, LogInfo100, "Copy the file into the appropriate place in setupsRoot");
    FSOCopyFile(tempFolder + "silverlight.cab.exe", tempFolder + _GetUACName(env.layer), "FORCE");
    runCmdToLog("SignSLFiles.exe " + tempFolder + " " + tempFolder + "signed\\23\\ \"" + _GetUACName(env.layer) + "\"", runOpts);
    runCmd("mkdir " + outputDir , runSetNoThrow(runOpts));
    FSOCopyFile(tempFolder + "signed\\23\\" + _GetUACName(env.layer), outputDir + "\\" + sfxName, "FORCE");

    FSOCopyFile(tempFolder + "signed\\23\\" + _GetUACName(env.layer), env.buildPath  + "\\" + sfxName, "FORCE");

    logMsg(LogScript, LogInfo, "******************************\n");
    logMsg(LogScript, LogInfo, "Installer generation was successful, you can get the installer from:\n");
    logMsg(LogScript, LogInfo, outputDir + "\\" + sfxName + "\n");
    logMsg(LogScript, LogInfo, "******************************\n");

    if(deleteTemps)
    {
        try
        {
            /* Up until now we have been using tempFolder with an trailing slash to make life easy,
               but FSOAtomicDeleteFolder requires you don't have a trailing slash (because of how it generates names) */
            FSOAtomicDeleteFolder(tempFolder.substring(0,tempFolder.length-1));
        }
        catch(e)
        {
            // FSOAtomicDeleteFolder throws unconditionally on failure, but we want to allow failure as it's just a temp directory
        }
    }
}

function _createEnv(_vblRoot, _setupsRoot, _buildPath, _buildArch, _buildType, _buildBranch, _layer)
{
    var ret = {vblRoot:_vblRoot,
               setupsRoot:_setupsRoot,
               buildPath:_buildPath,
               buildType:_buildType,
               buildArch:_buildArch,
               buildBranch:_buildBranch,
               layer:_layer};  

    if ((_layer != "main") && (_layer != "dev"))
    {
        logMsg(LogScript, LogWarn, "Unrecognized layer, defaulting to main.");
        ret.layer = "main";
    }

    if (_layer == undefined)
    {
        ret.layer = "main";
    }   
            

    if (_vblRoot == undefined)
    {
        ret.vblRoot = findVblRoot(".");
        if (ret.vblRoot == undefined)
        {
            logMsg(LogScript, LogError, "Error: unable to determine root of depository, please specify at command line.");
            throw new Error(2, "Error: unable to determine root of depository, please specify at command line.");
        }
        // The format of the path changes depending on the environment, so remove the \. if it's there
        ret.vblRoot = ret.vblRoot.replace("\\.","");
    }
    
    if (_setupsRoot == undefined)
    {
        ret.setupsRoot = ret.vblRoot + "Setup";
    }
    
    if (_buildArch == undefined)
    {
        ret.buildArch = "x86";
    }
    
    if (_buildType == undefined)
    {
        ret.buildType = "chk";
    }
    
    if (_buildPath == undefined)
    {
        ret.buildPath = ret.vblRoot + "\\binaries\\" + ret.buildArch + ret.buildType;
    }
    
    if (_buildBranch == undefined)
    {
        ret.buildBranch = "Silverlight_W2";
    }
    
    return ret;
}

/******************************************************************************/
/* Returns the sfx ending based on the layer
 */
function _GetSfxEnding(layer)
{
    if (layer == "main")
    {
        return "";
    }
    if (layer == "dev")
    {
        return "_Developer";
    }
    return undefined;
}

/******************************************************************************/
/* Returns the wave name based on the layer
 */
function _GetWaveName(layer)
{
    if (layer == "main")
    {
        return "wave2";
    }
    if (layer == "dev")
    {
        return "wave2dev";
    }
    return undefined;
}

/******************************************************************************/
/* Returns the UAC name based on the layer
 */
function _GetUACName(layer)
{
    if (layer == "main")
    {
        return "Microsoft Silverlight Installer";
    }
    if (layer == "dev")
    {
        return "Microsoft Silverlight for Developers Installer";
    }
    return undefined;
}

/******************************************************************************/
/* Returns 3 for installer, used on the endings of files 
 * and for product families.
 */
function _GetWaveEnding(layer)
{
    if (layer == "main")
    {
        return "Main";
    }
    if (layer == "dev") 
    {
        return "Dev";
    }
    return undefined;
}

/******************************************************************************/
/* Gets the MSP name based on the layer
 */
function _GetMspName(layer)
{
    return "Silverlight" + _GetWaveEnding(layer) + ".msp";
}

/******************************************************************************/
/* Gets the version from the version.txt file, given the wave name
 */
function _GetVersion(env)
{
    var version = FSOReadFromFile(env.vblRoot + "\\public\\products\\Silverlight\\wave2\\version.txt");
    version = version.replace("\r\n","");

    return version;
}

/******************************************************************************/
/* These functions should not depend on environment variables in general, but
 *   when it is necessary this wrapper does a fail fast if they aren't set.
 */
function _Env(variable)
{
    var result = Env(variable);
    if(!result)
    {
        throw new Error(1, "Required environment variable " + variable + " is missing.");
    }
    return result;
}
