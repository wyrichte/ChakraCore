/*
    This file provides layout functionality for the retail lab
    OWNER: sanket
*/

var layoutModuleDefined = 1; // Indicate that this module exists

if (!fsoModuleDefined)   throw new Error(1, "Need to include fso.js");
if (!logModuleDefined)   throw new Error(1, "Need to include log.js");
if (!runModuleDefined)   throw new Error(1, "Need to include run.js");
if (!Env)                throw new Error(1, "Need to have Env defined. This script expects it to be defined by run.js");

var g_LayoutLogObject = logNewFacility("layoutBuild");

var g_LayoutSupportedArchs = ["x86", "amd64"];
var g_LayoutInstallRoots = ["%WINDIR%\\Microsoft.NET\\Framework\\", "%WINDIR%\\Microsoft.NET\\Framework64\\"];

/******************************************************************************/
/*
   Calculates some parameters that are needed for a layout install.
   Since we sometimes calculate them a couple of times, we have a
   function here.

   Arguments
   vblRoot: vbl path
   layoutRoot: root of the layout installer (Default: vblRoot + "\..\srcSetup")
   remoteStore: root of the remote store (Default: value in clrsetupinfo.bat in the raw local drop)
   buildType: Build flavor (Default: ret)
*/
function _calculateLayoutInstallParameters(vblRoot, layoutRoot, remoteStore, buildType)
{
    logCall(g_LayoutLogObject, LogInfo, "_calculateLayoutInstallParameters", arguments, "{");

    if (vblRoot == undefined)
        throw new Error(1, "vblRoot must be specified to calculate the layout install parameters\r\n");
    if (layoutRoot == undefined)
        layoutRoot = _toLocalLayoutFolder(vblRoot);
    if (buildType == undefined)
        buildType = "ret";    

    var buildNumberIndex = undefined;
    var layoutInstaller = undefined;
    var layoutInstallerName = undefined;
    var layoutExtendedUninstallerName = undefined;
    var layoutClientUninstallerName = undefined;
    var buildNumber = undefined;
    var sPathToFileVerTool = vblRoot + "\\tools\\x86\\filever.exe";
    var sPathToClrDll = undefined;
    var rawBuildsRoot = _toLocalBinariesFolder(vblRoot);
    var sProcessorArchitecture = Env("PROCESSOR_ARCHITEW6432");
    var sFrameworkInstallPath = undefined;
    var s32bitFrameworkInstallPath = undefined;
    if (sProcessorArchitecture == undefined || sProcessorArchitecture == null || sProcessorArchitecture == "")
    {
        sProcessorArchitecture = Env("PROCESSOR_ARCHITECTURE");
    }
    sProcessorArchitecture = String(sProcessorArchitecture).toUpperCase();

    var rawBuild = rawBuildsRoot + "\\" + sProcessorArchitecture + buildType;
    var sPathToClrsetupInfo = rawBuildsRoot + "\\" + sProcessorArchitecture + buildType + "\\clrsetupinfo.bat";
    // If clrsetupinfo.bat not found in the binaries folder corresponding to the processor architecture, we look for that file for the x86 architecture
    // This is required when we are optimizing a runtime from a non-WOW window on a machine with AMD64 or IA64 based hardware/OS
    var sAlternatePathToClrsetupInfo = rawBuildsRoot + "\\" + "x86" + buildType + "\\clrsetupinfo.bat";
    if (remoteStore == undefined || remoteStore == null || String(remoteStore).toUpperCase() == "NULL" || String(remoteStore) == "")
    {
        logMsg(g_LayoutLogObject, LogInfo, "sPathToClrsetupInfo: " + sPathToClrsetupInfo + "\r\n");
        if (!FSOFileExists(sPathToClrsetupInfo)) {
            logMsg(g_LayoutLogObject, LogInfo, "sAlternatePathToClrsetupInfo: " + sAlternatePathToClrsetupInfo + "\r\n");
            sPathToClrsetupInfo = sAlternatePathToClrsetupInfo;
            if (!FSOFileExists(sPathToClrsetupInfo))
                throw new Error(1, "Either remoteStore must be specified or a clrsetupinfo.bat file must be present in the drop\r\n");
        }
        logMsg(g_LayoutLogObject, LogInfo, "Will be extracting the location of the remoteStore from clrsetupinfo.bat\r\n");
        var setupInfo = parseBatchFileForSetStatements(sPathToClrsetupInfo);
        remoteStore = setupInfo["CLR_DROP_PATH_HEAD"] + "\\" + setupInfo["CLR_CERTIFIED_VERSION"];
    }

    if (sProcessorArchitecture == "X86")
    {
        layoutInstallerName = "dotNetFx45_Full_x86.exe";
        layoutExtendedUninstallerName = "netfx_extended_x86.msi";
        layoutClientUninstallerName = "netfx_core_x86.msi";
        layoutInstaller = layoutRoot + "\\x86" + buildType + "\\";
        sPathToClrDll = rawBuildsRoot + "\\" + "x86" + buildType + "\\clr.dll";
        buildNumberIndex = 10;
        sFrameworkInstallPath = g_LayoutInstallRoots[0];        
    }
    else if (sProcessorArchitecture == "AMD64")
    {
        layoutInstallerName = "dotNetFx45_Full_x86_x64.exe";
        layoutExtendedUninstallerName = "netfx_extended_x64.msi";
        layoutClientUninstallerName = "netfx_core_x64.msi";
        layoutInstaller = layoutRoot + "\\amd64" + buildType + "\\";
        sPathToClrDll = rawBuildsRoot + "\\" + "amd64" + buildType + "\\clr.dll";
        buildNumberIndex = 8;
        sFrameworkInstallPath = g_LayoutInstallRoots[1];
    }
    else
    {
        layoutInstallerName = "dotNetFx45_Full_x86_ia64.exe";
        layoutExtendedUninstallerName = "netfx_extended_ia64.msi";
        layoutClientUninstallerName = "netfx_core_ia64.msi";
        layoutInstaller = layoutRoot + "\\ia64" + buildType + "\\";
        sPathToClrDll = rawBuildsRoot + "\\" + "ia64" + buildType + "\\clr.dll";
        buildNumberIndex = 8;
        sFrameworkInstallPath = g_LayoutInstallRoots[2];
    }
    s32bitFrameworkInstallPath = g_LayoutInstallRoots[0];

    // Since .NET 4.5 is an in-place release, we have the same build number as .NET 4.0
    buildNumber = "30319";
    logMsg(g_LayoutLogObject, LogInfo, "Build Number set to: " + buildNumber + " since this is an in-place release\r\n");
    sFrameworkInstallPath = sFrameworkInstallPath + "v4.0." + buildNumber;
    s32bitFrameworkInstallPath = s32bitFrameworkInstallPath + "v4.0." + buildNumber;

    var localLayoutInstaller = layoutInstaller + "\\enu\\netfx\\fullredist\\box\\" + layoutInstallerName;
    layoutInstaller = layoutInstaller + buildNumber + ".00\\enu\\netfx\\fullredist\\box\\" + layoutInstallerName;

    logMsg(g_LayoutLogObject, LogInfo, "} _calculateLayoutInstallParameters\r\n");    

    return {InstallerName : layoutInstallerName, 
            Architecture : sProcessorArchitecture,
            BuildLabLayoutInstallerPath : layoutInstaller, 
            DevLayoutInstallerPath : localLayoutInstaller, 
            BuildNumber : buildNumber, 
            FrameworkInstallPath : sFrameworkInstallPath,                         // This will be the path to the 64-bit installation (if applicable) or otherwise be the path the 32-bit installation
            AdditionalFrameworkInstallPath : s32bitFrameworkInstallPath,        // This will be the path to the 32-bit installation
            ExtendedUninstallerName : layoutExtendedUninstallerName, 
            ClientUninstallerName : layoutClientUninstallerName};
}

/******************************************************************************/
/*
   Un-installs the layout build

   Arguments
   installDir: Directory where the runtime is installed
   additionalInstallDir: Directory where the 32-bit runtime is installed
   extendedInstallerName: Name of the extended installer
   clientInstallerName: Name of the client installer
*/
function uninstallLayout(installDir, additionalInstallDir, extendedInstallerName, clientInstallerName) 
{
    logCall(g_LayoutLogObject, LogInfo, "uninstallLayout", arguments, "{");

    if (installDir == undefined)
        throw new Error(1, "Installation directory needed to uninstall it\r\n");
    if (extendedInstallerName == undefined || clientInstallerName == undefined)
        throw new Error(1, "Installer names needed to uninstall them\r\n");

    try {
        runCmdToLog("msiexec /uninstall " + installDir + "\\SetupCache\\Extended\\" + extendedInstallerName + " /q /norestart", 
                    runSet32Bit(runSetNoThrow(runSetIdleTimeout(60*60, runSetTimeout(60*60)))));
    } catch(e) {
        logMsg(g_LayoutLogObject, LogInfo, "Uninstallation of extended runtime failed with the following exception: " + e.description + "\r\n");
    }
    
    try {        
        runCmdToLog("msiexec /uninstall " + installDir + "\\SetupCache\\Client\\" + clientInstallerName + " /q /norestart", 
                    runSet32Bit(runSetNoThrow(runSetIdleTimeout(60*60, runSetTimeout(60*60)))));
    } catch(e) {
        logMsg(g_LayoutLogObject, LogInfo, "Uninstallation of client runtime failed with the following exception: " + e.description + "\r\n");
    } finally {
        // restore applicationHost.config
        try {
            var sSrcConfigDir = "%WINDIR%\\System32\\inetsrv\\Config";
            sSrcConfigDir = String(sSrcConfigDir).replace("%WINDIR%", String(Env("WINDIR")));
            logMsg(g_LayoutLogObject, LogInfo, "Restoring applicationHost.config in " + sSrcConfigDir + "\r\n");
            FSOCopyFileIfExists(sSrcConfigDir + "\\applicationHost.config.backup", sSrcConfigDir + "\\applicationHost.config", true);
        } catch(e1) {
            logMsg(g_LayoutLogObject, LogInfo, "Error while restoring the existing applicationHost.config: " + e1.description + "\r\n");
        }
    
        // IIS gets confused even with empty installation directories
        // so we delete them
        try {
            installDir = String(installDir).replace("%WINDIR%", String(Env("WINDIR")));
            if (FSOFolderExists(installDir)) {
                logMsg(g_LayoutLogObject, LogInfo, "Deleting " + installDir + "\r\n");
                FSODeleteFolder(installDir, true);
            } else {
                logMsg(g_LayoutLogObject, LogInfo, "No installation directory: " + installDir + " found \r\n");
            }
        } catch(e) {
            logMsg(g_LayoutLogObject, LogInfo, "Failure while deleting " + installDir + ". Error: " + e.description + "\r\n");
        }

        if (additionalInstallDir != undefined) {
            try {
                additionalInstallDir = String(additionalInstallDir).replace("%WINDIR%", String(Env("WINDIR")));
                if (FSOFolderExists(additionalInstallDir)) {
                    logMsg(g_LayoutLogObject, LogInfo, "Deleting " + additionalInstallDir + "\r\n");
                    FSODeleteFolder(additionalInstallDir, true);
                } else {
                    logMsg(g_LayoutLogObject, LogInfo, "No installation directory: " + additionalInstallDir + " found \r\n");
                }
            } catch(e) {
                logMsg(g_LayoutLogObject, LogInfo, "Failure while deleting " + additionalInstallDir + ". Error: " + e.description + "\r\n");
            }
        }
    }

    logMsg(g_LayoutLogObject, LogInfo, "} uninstallLayout\r\n");    
}

/******************************************************************************/
/*
   Installs the layout build

   Arguments
   vblRoot: vbl path
   layoutRoot: root of the layout installer (Default: vblRoot + "\..\srcSetup\layouts")
   remoteStore: root of the remote store (Default: value in clrsetupinfo.bat in the local raw drop)
   buildType: Build flavor (Default: ret)
*/
function installLayout(vblRoot, layoutRoot, remoteStore, buildType)
{
    logCall(g_LayoutLogObject, LogInfo, "installLayout", arguments, "{");
    if (vblRoot == undefined)
        throw new Error(1, "vblRoot must be specified to install the layout build\r\n");
    if (layoutRoot == undefined)
        layoutRoot = _toLocalLayoutFolder(vblRoot);
    if (buildType == undefined)
        buildType = "ret";
    if ( _isNullDatabaseEntry(remoteStore)) {
        remoteStore = _extractRemoteStoreForLayout(vblRoot, buildType);
    }
    if (remoteStore == undefined) {
        throw new Error(1, "Either remoteStore must be specified or a clrsetupinfo.bat file must be present in the drop\r\n");
    }       

    // stop iisadmin service
    // sometimes the layout installation fails because of that
    try {
        runCmdToLog("net stop iisadmin", runSetIdleTimeout(3*60,runSetTimeout(3*60, runSetNoThrow())));
    } catch(e) {
        logMsg(g_LayoutLogObject, LogInfo, "Error while stopping the iisadmin service: " + e.description + "\r\n");
    }

    // back-up applicationHost.config
    try {
        var sSrcConfigDir = "%WINDIR%\\System32\\inetsrv\\Config";
        sSrcConfigDir = String(sSrcConfigDir).replace("%WINDIR%", String(Env("WINDIR")));
        logMsg(g_LayoutLogObject, LogInfo, "Backing up applicationHost.config in " + sSrcConfigDir + "\r\n");
        FSOCopyFileIfExists(sSrcConfigDir + "\\applicationHost.config", sSrcConfigDir + "\\applicationHost.config.backup", true);
    } catch(e) {
        logMsg(g_LayoutLogObject, LogInfo, "Error while saving the existing applicationHost.config: " + e.description + "\r\n");
    }

    var layoutInstallParameters = _calculateLayoutInstallParameters(vblRoot, layoutRoot, remoteStore, buildType);
    uninstallLayout(layoutInstallParameters.FrameworkInstallPath, layoutInstallParameters.AdditionalFrameworkInstallPath, layoutInstallParameters.ExtendedUninstallerName, layoutInstallParameters.ClientUninstallerName);

    if (FSOFileExists(layoutInstallParameters.BuildLabLayoutInstallerPath))
    {    
        runCmdToLog(layoutInstallParameters.BuildLabLayoutInstallerPath + " /q /norestart", runSet32Bit(runSetIdleTimeout(60*60, runSetTimeout(60*60))));
    }
    else if(FSOFileExists(layoutInstallParameters.DevLayoutInstallerPath))
    {
        runCmdToLog(layoutInstallParameters.DevLayoutInstallerPath + " /q /norestart", runSet32Bit(runSetIdleTimeout(60*60, runSetTimeout(60*60))));
    }
    else
    {
        throw new Error(1, "Installer executable not found at: " + layoutInstallParameters.BuildLabLayoutInstallerPath + " or " + layoutInstallParameters.DevLayoutInstallerPath);
    }

    // start iisadmin service
    // sometimes the layout installation fails because of that
    try {
        runCmdToLog("net start iisadmin", runSetIdleTimeout(3*60,runSetTimeout(3*60, runSetNoThrow())));
    } catch(e) {
        logMsg(g_LayoutLogObject, LogInfo, "Error while starting the iisadmin service: " + e.description + "\r\n");
    }


    logMsg(g_LayoutLogObject, LogInfo, "Layout build installed in the .NET Framework directory\r\n");
    logMsg(g_LayoutLogObject, LogInfo, "} installLayout\r\n");        
}

/******************************************************************************/
/*
   Function to extract the remote store from one of the clrsetupinfo.bat
   in the raw drops.

   Arguments
   vblRoot: vbl path
   buildType: Build flavor (Default: ret)
*/
function _extractRemoteStoreForLayout(vblRoot, buildType) {
    logCall(g_LayoutLogObject, LogInfo, "_extractRemoteStoreForLayout", arguments, "{");
    if (vblRoot == undefined)
        throw new Error(1, "vblRoot must be specified to get to the local builds");
    if (buildType == undefined)
        buildType = "ret";
    var rawBuildsRoot = _toLocalBinariesFolder(vblRoot);
    var remoteStore = undefined;
    for (var j=0; j<g_LayoutSupportedArchs.length; j++)
    {
        var rawBuild = rawBuildsRoot + "\\" + g_LayoutSupportedArchs[j] + buildType;
        var sPathToClrDll = rawBuildsRoot + "\\" + g_LayoutSupportedArchs[j] + buildType + "\\clr.dll";
        var sPathToClrsetupInfo = rawBuildsRoot + "\\" + g_LayoutSupportedArchs[j] + buildType + "\\clrsetupinfo.bat";
        if (remoteStore == undefined)
        {
            if (!FSOFileExists(sPathToClrsetupInfo))
                continue;
            logMsg(g_LayoutLogObject, LogInfo, "Extracting the location of the remoteStore from clrsetupinfo.bat\r\n");
            var setupInfo = parseBatchFileForSetStatements(sPathToClrsetupInfo);
            remoteStore = setupInfo["CLR_DROP_PATH_HEAD"] + "\\" + setupInfo["CLR_CERTIFIED_VERSION"];
        }
    }

    logMsg(g_LayoutLogObject, LogInfo, "} _extractRemoteStoreForLayout\r\n");        
    return remoteStore;
}


/******************************************************************************/
/*
   Creates the layout installer from the raw build. The raw build must be present
   in <vblRoot> + "\\..\\binaries" under the <arch><buildType> folders

   Arguments
   vblRoot: vbl path
   remoteStore: Location of the remote store for picking up the missing files.
                   So all the binaries would be in
                   remoteStore + "\\binaries.x86ret",
                   remoteStore + "\\binaries.x86chk",
                   remoteStore + "\\binaries.amd64ret",
                   remoteStore + "\\binaries.amd64chk",
                   remoteStore + "\\binaries.ia64ret" and
                   remoteStore + "\\binaries.ia64chk"
                   (Default: value in clrsetupinfo.bat)
   buildType: Build flavor (Default: ret)
   copySymbols: To instruct to put the symbol files in the output raw drop (Default: true)
*/
function createLayoutFromRaw(vblRoot, remoteStore, buildType, copySymbols, overwriteWithClrsetupDrop)
{
    logCall(g_LayoutLogObject, LogInfo, "createLayoutFromRaw", arguments, "{");
    if (vblRoot == undefined)
        throw new Error(1, "vblRoot must be specified to find the tools");
    if (buildType == undefined)
        buildType = "ret";
    if (copySymbols == undefined)
        copySymbols = true;
    if (overwriteWithClrsetupDrop == undefined)
        overwriteWithClrsetupDrop = true;

    var rawBuildsRoot = _toLocalBinariesFolder(vblRoot);
    var setupRoot = _toLocalSrcSetupFolder(vblRoot);
    if (FSOFolderExists(setupRoot)) {
        logMsg(g_LayoutLogObject, LogInfo, "Deleting srcSetup folder " + setupRoot + " before generating the layout\r\n");
        FSODeleteFolder(setupRoot, true);
    }
    for (var i = 0; i < g_LayoutSupportedArchs.length; i++)
    {
        if (_isNullDatabaseEntry(remoteStore)) {
            remoteStore = _extractRemoteStoreForLayout(vblRoot, buildType);
        }
        if (remoteStore == undefined) {
            throw new Error(1, "Either remoteStore must be specified or a clrsetupinfo.bat file must be present in the drop\r\n");
        }

        // HACK - sanket
        // I had some authenticode signature issues with mscoree.dll
        // and am therefore deleting it here assuming the change
        // doesn't really care about the mscoree version.
        var sPathToMscoreeDll = rawBuildsRoot + "\\" + g_LayoutSupportedArchs[i] + buildType + "\\mscoree.dll";
        if (FSOFileExists(sPathToMscoreeDll))
            FSODeleteFile(sPathToMscoreeDll, "FORCE");

        var sDispArch = g_LayoutSupportedArchs[i];
        if (sDispArch == "x86") {
            sDispArch = "i386";
        }

        // For .NET 4.5 the version number has now been locked down to 4.0.30319 and therefore 
        // we do not need the hack to copy the right csc.exe.config for the layouts
    }
    
    var layoutRoot = _toLocalLayoutFolder(vblRoot);
    var installParams = _calculateLayoutInstallParameters(vblRoot, layoutRoot, remoteStore, buildType);
    var sLayoutSku = "full";
    var sLayoutType = buildType;

    // we want to use the layout files from the branch we are dealing with
    var setupBuildEnv = runSetEnv("_NTROOT", vblRoot.split(":")[1],
                        runSetEnv("_NTDRIVE", vblRoot.split(":")[0] + ":",
                        runSetEnv("_NTBINDIR", vblRoot,
                        runSetEnv("_NTTREE", rawBuildsRoot + "\\" + installParams.Architecture + buildType,
                        runSetEnv("_BuildArch", installParams.Architecture,
                        runSetEnv("_BuildType", buildType,
                        runSetTimeout(60 * MINUTE,
                        runSet32Bit())))))));

    // I have seen buildLayouts functionality fail because of certain razzle environment variables set
    // We therefore run this step in a fresh environment window
    // Create a temporary batch file
    var tempBatchFile = FSOGetTempPath("runjsTempBatch-") + ".bat";
    var tempBatchFileContents = "call " + ScriptDir + "\\runjs buildLayouts" + " " + sLayoutSku + " " + 
                                                                                     installParams.Architecture + " " + 
                                                                                     sLayoutType + " " + 
                                                                                     "_" + " " + 
                                                                                     remoteStore + " " +
                                                                                     "_" + " " + 
                                                                                     "_" + " " + 
                                                                                     "true" + " " +
                                                                                     "true";
    // build the 32-bit layout image also
    if (installParams.Architecture != "X86") {
        tempBatchFileContents = tempBatchFileContents + " & call " + ScriptDir + "\\runjs buildLayouts" + " " + sLayoutSku + " " + 
                                                                                                                "x86" + " " + 
                                                                                                                sLayoutType + " " + 
                                                                                                                "_" + " " + 
                                                                                                                remoteStore + " " +
                                                                                                                "_" + " " + 
                                                                                                                "_" + " " + 
                                                                                                                "true" + " " +
                                                                                                                "true";
    }
    FSOWriteToFile(tempBatchFileContents, tempBatchFile);

    logMsg(g_LayoutLogObject, LogInfo, "Creating NetFX installer build\r\n");
    try {
        var run = runCmdToLog("pushd " + vblRoot + " & " + "call " + tempBatchFile, setupBuildEnv);
    } catch(e) {
        logMsg(g_LayoutLogObject, LogInfo, "Error while generating the installer build. Exception: " + e.description + "\r\n");
    } finally {
        // overwrite the drops with the clrsetup one's to workaround the perf lab's inability
        // to consume private layout builds today    
        if (overwriteWithClrsetupDrop)
        {
            for (var i = 0; i < g_LayoutSupportedArchs.length; i++) {
                var srcDropFolder = rawBuildsRoot + "\\" + g_LayoutSupportedArchs[i] + buildType;
                var destDropFolder = setupRoot + "\\binaries." + g_LayoutSupportedArchs[i] + buildType;
                var layoutInstallerFolder = layoutRoot + "\\" + g_LayoutSupportedArchs[i] + buildType + "\\enu\\netfx\\fullredist\\box";

                // Copy .NET installers to the raw binary drop as clrsetup uses the installer from there
                logMsg(g_LayoutLogObject, LogInfo, "Copying .NET installer from " + layoutInstallerFolder + " to " + srcDropFolder + "\r\n");
                robocopyFiles(layoutInstallerFolder, srcDropFolder, undefined, undefined, "dotNetFx*");

                // Copy raw drops
                if (FSOFolderExists(srcDropFolder)) {
                    logMsg(g_LayoutLogObject, LogInfo, "Copying drops for " + g_LayoutSupportedArchs[i] + " architecture from " + srcDropFolder + " to " + destDropFolder + "\r\n");
                    robocopy(srcDropFolder, destDropFolder);                
                }
                // mscoree.dll is still part of ndpsetup.txt 
                // the perf lab parses ndpsetup.txt for validating the build and would fail
                // because of the absence of this file.
                // Since we delete the original mscoree.dll (from the input drop) before
                // we put the mscoree.dll from remoteStore in its place
                var sSourceMscoreeDll = remoteStore + "\\binaries." + g_LayoutSupportedArchs[i] + buildType + "\\mscoree.dll";
                var sDestMscoreeDll = destDropFolder + "\\mscoree.dll";
                if (!FSOFileExists(sDestMscoreeDll)) {
                    FSOCopyFile(sSourceMscoreeDll, sDestMscoreeDll, false);
                }
            }
        }

        // copy symbol files which may be used for debugging or other purposes
        // we do not copy the symbols if overwriteWithClrsetupDrop is true
        // because that will include copy of the symbols too
        else if (copySymbols) {
            for (var i = 0; i < g_LayoutSupportedArchs.length; i++) {
                var srcSymbolFolder = rawBuildsRoot + "\\" + g_LayoutSupportedArchs[i] + buildType + "\\Symbols.pri";
                var destSymbolFolder = setupRoot + "\\binaries." + g_LayoutSupportedArchs[i] + buildType + "\\Symbols.pri";
                if (FSOFolderExists(srcSymbolFolder)) {
                    logMsg(g_LayoutLogObject, LogInfo, "Copying symbols for " + g_LayoutSupportedArchs[i] + " architecture from " + srcSymbolFolder + " to " + destSymbolFolder + "\r\n");

                    // We do not overwrite in case some day the setup is modified to put the symbol files 
                    // in the output directory in which case this copy step will become superfluous
                    FSOCopyFolder(srcSymbolFolder, destSymbolFolder, false);
                }
            }
        }
    }

    logMsg(g_LayoutLogObject, LogInfo, "Layout build in " + _toLocalSrcSetupFolder(vblRoot) + "\r\n");
    logMsg(g_LayoutLogObject, LogInfo, "} createLayoutFromRaw\r\n");        
}
