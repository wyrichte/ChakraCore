/*****************************************************************************/
/*                               shortcuts.js                                */
/*****************************************************************************/

/* Contains routines dependent on your cmd window's environment variables.
   These are helpers for users, and shouldn't be used by other scripts.

   This includes scripts for doing incremental builds of parts of the CLR from 
   a clrenv window.
*/

/* AUTHOR: Brian Grunkemeyer
   DATE: 4/8/2005 */

/******************************************************************************/

/* options for the various commands  */
var userBuildOptions = ["quickBuild", "buildArch", "buildType", "buildArgs"]
var installCLRBinaryOptions = ["mz", "ngenAll", "gacInstall", "checkNGen"];
var buildInstallOptions = userBuildOptions.concat(installCLRBinaryOptions);


function _msBuild(dir, options)
{
    var base = srcBaseFromScript();
    options = runSetCwd(base + "\\"+ dir, options);

    logMsg(LogClrAutomation, LogInfo, "Running rbuild in " + dir +"\n");                    
    var run = runCmdToStream(
        "rbuild",
        undefined, //  log to stdout
        options);

    return run.ExitCode;
}

/****************************************************************************/
/* Incrementally rebuilds clr (& DAC) then installs it.

   runjs buildVM [/noDac /mz /ngenAll /noInstall]

   options:  
        /noDac        Don't rebuild DAC, to save time.
        /noInstall    Don't install clr/mscordacwks, useful for static analysis
        See installCLRBinary function for more options

   Excluding DAC is faster because it is the bare minimum build necessary, but 
   you sacrifice the ability to debug managed code in windbg pleasantly.
*/
function buildVM(options) {

    var bldType = Env("_BuildType");
    if (!bldType)
        bldType = "chk";

    options = getOptions(buildInstallOptions.concat(["noDac", "noInstall"]), options);

    // Applying BBT data to clr can take minutes, so skip it for this shortcut build and warn below
    // Note that the build system only applies BBT data to retail builds
    if (bldType == "ret") {
        options = runSetEnv("NO_POST_LINK_OPTIMIZE", 1, options);
    }

    if (options.buildType != undefined)
        bldType = options.buildType;

    var clr_file;
    if (isCoreCLRBuild(bldType)) {
        clr_file = "coreclr.dll";
    }
    else {
        clr_file = "clr.dll";
    }

    if (options.noDac)
    {
        if (isCoreCLRBuild(bldType)) {
            _msBuild("ndp\\clr\\src\\dlls\\mscoree\\coreclr", options);
        }
        else {
            _msBuild("ndp\\clr\\src\\dlls\\mscoree\\wks", options);
        }
    }
    else
    {
        _msBuild("ndp\\clr\\src\\dacupdatedll", options);
        if (!options.noInstall) {
            installCLRBinary("mscordac*.dll", options);
        }
    }
    
    if (!options.noInstall) {
        // "Install" clr.dll/coreclr.dll
        options.checkNGen=true;
        installCLRBinary(clr_file, options);
    }

    if (bldType == "ret") {
        logMsg(LogClrAutomation, LogWarn, "Ignored BBT data; use rbuild if you want to include it.\n");
    }

    logMsg(LogClrAutomation, LogInfo, "Incremental build of clr completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuild DAC (but not clr)

   runjs buildDac

   This just rebuilds the DAC dll (useful if you change dac infrastructure).
   This does not rebuild the VM (clr). Use 'buildVM' if you change 
   clr.dll.
   
   building DAC depends on having clr.pdb (for a post-build step), 
   so you may need to do a buildVM first to ensure the clr build is up
   to date and the pdbs are in the right spot.
*/
function buildDac(options)
{
    userBuild("ndp\\clr\\src\\debug\\daccess", options);
//REM still we don't get a rebuilt cordacwks.dll, unless we delete it by hand before doing
// this step.  What gives with that?  mscordacwks.dll *is* the updated version whether
// cordacwks is replaced or not...
    userBuild("ndp\\clr\\src\\dlls\\mscordac", options);
    userBuild("ndp\\clr\\src\\dacupdatedll", options);

    // The two files on desktop will be mscordacwks.dll
    // and mscordacwks_<arch>_<arch>_<version string>.<build type>.dll
    installCLRBinary("*dacwks*.dll", options);
}

/****************************************************************************/
/* Incrementally rebuild SOS

   runjs buildSOS

   This rebuilds the SOS dll.  Nothing else is built as a result.
*/
function buildSOS(options)
{
    userBuild("ndp\\clr\\src\\ToolBox\\SOS\\Strike", options);
    installCLRBinary("SOS.dll", options);
}

/****************************************************************************/
/* Incrementally rebuilds ZAP and VM, and then installs it.

   Parameters:
   options:  
        See userBuild function for more options
        See installCLRBinary function for more options
*/

function buildZap(options) {
    options = getOptions(buildInstallOptions, options);

    userBuild("ndp\\clr\\src\\zap", options);
    return buildVM(options);
}

function buildSys(options) {
    options = getOptions(buildInstallOptions, options);

    userBuild("ndp\\fx\\src\\sys", options);
 
    // Install system 
    options.gacInstall = true;
    options.checkNGen = true;
    installCLRBinary("System.dll", options);

    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.dll completed.\n");
    return _checkRunEnv(options);
}
    

/****************************************************************************/
/* Incrementally rebuilds mscorlib & installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildBcl(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "mscorlib.dll")) {

        var coreCLRBuild=false;
        if (Env("Build_project_name") == "coreclr") {
            coreCLRBuild=true;
        }
        if (coreCLRBuild) {
            var dirsFile = Env("_NTBINDIR") + "\\ndp\\clr\\src\\dirs";        
            checkOut(dirsFile);
            
            // re-add BCL to dirs and save original in dirs.bak
            _editDirsFileToAddBcl();
        }
        
        // Applying IBC data to mscorlib can take minutes, so skip it for this shortcut build and warn below
        // Note that the build system applies IBC data to builds of all flavors
        options = runSetEnv("NO_POST_LINK_OPTIMIZE", 1, options);

        // Build the BCL
        _msBuild("ndp\\clr\\src\\bcl", options);

        if (coreCLRBuild) {
            FSOCopyFileIfExists(dirsFile + ".bak" , dirsFile, true);
            FSODeleteFile(dirsFile + ".bak", "FORCE");
            // If there are no changes, un-checkout the file. 
            uncheckoutUnchanged(dirsFile);
        }
        
        if (!coreCLRBuild) {
            // First copy the NLP files
            installCLRBinary("*.nlp", options);
            
            // Install mscorlib
            options.gacInstall = true;
            options.checkNGen = true;
            installCLRBinary("mscorlib.dll", options);
        }

        logMsg(LogClrAutomation, LogWarn, "Ignored IBC data; use rbuild if you want to include it.\n");

    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of mscorlib completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuilds mscoreei (the runtime shim) and installs it

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                   are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildShim(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "mscoreei.dll")) {
        
        // Build the shim impl
        _msBuild("ndp\\clr\\src\\dlls\\shim", options);
        _msBuild("ndp\\clr\\src\\dlls\\shimr", options);

        if (!_isCompatibleArch(options)) {
            logMsg(LogClrAutomation, LogInfo, "Wrong architecture for build, skipping install\n");
            return;
        }
        
        //Install the implementation
        installCLRBinary("mscoreei.dll", options);
        installCLRBinary("mscoreeis.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build and deployment of and mscoreei.dll completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuilds clrjit.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        /mz         ngen mscorlib
        /ngenAll    ngen all the previously-ngenned assemblies

        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildJit(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "clrjit.dll")) {

        options = _getBuildOptions(options);
        
        // Applying BBT data to clrjit can take half of the time, so skip it for this shortcut build and warn below
        // Note that the build system only applies BBT data to retail builds
        if (options.buildType.match(/ret/)) {
            options = runSetEnv("NO_POST_LINK_OPTIMIZE", 1, options);
        }

        // Build the JIT
        if (options.buildArch.match(/64/))
            _msBuild("ndp\\clr\\src\\jit64", options);

        _msBuild("ndp\\clr\\src\\jit", options);
        
        // Install clrjit
        options.gacInstall = false;
        if (options.buildArch.match(/64/)) {
            options.checkNGen = false;
            installCLRBinary("peverify.dll", options);
            installCLRBinary("jitext.dll", options);
        }

        options.checkNGen=true;
        installCLRBinary("clrjit.dll", options);
        installCLRBinary("altjit.dll", options);

        if (options.buildType.match(/ret/)) {
            logMsg(LogClrAutomation, LogWarn, "Ignored BBT data; use rbuild if you want to include it.\n");
        }
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of clrjit completed.\n");

    // no need to verify that environment matches, since a debug JIT will 
    // run just fine with a retail clr
    return 0;
}

function buildAltJit(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "altjit.dll")) {

        options = _getBuildOptions(options);
        
        // Build the JIT
        userBuild("ndp\\clr\\src\\jit", options);
        
        // Install clrjit
        options.gacInstall = false;

        // Never check ngen
        installCLRBinary("altjit.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of altjit completed.\n");

    // no need to verify that environment matches, since a debug JIT will 
    // run just fine with a retail clr
    return 0;

}

/****************************************************************************/
/* rebuilds Mdbg (Src\toolbox\mdbg) and installs it to the SDK directory
   Mdbg is small enough that we rebuild the whole thing.
*/
function buildMDbg(options)
{
    options = getOptions(buildInstallOptions, options);
    
    // Mdbg requires a clean build. It is managed code using .NetModules, which don't play well
    // with the build system's depedency analysis.
    options.buildArgs = "-cC";

    userBuild("ndp\\clr\\src\\toolbox\\mdbg", options);
    installSDKBinary("mdbg*", options);    
}

/****************************************************************************/
/* Incrementally rebuilds mscordbi.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildDbi(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "mscordbi.dll")) {
        
        // Build mscordbi.dll
        _msBuild("ndp\\clr\\src\\debug\\di", options);
        _msBuild("ndp\\clr\\src\\dlls\\mscordbi", options);

        // Also rebuild the DacUpdateDll directory to embed the timestamp and size 
        // of the new mscordbi.dll into clr.dll
        _msBuild("ndp\\clr\\src\\dacupdatedll", options);
        
        // Install mscordbi.dll
        installCLRBinary("mscordbi.dll", options);

        // Also install the clr.dll with the properties of the new mscordbi.dll
        var bldType = Env("_BuildType");
        if (!bldType)
            bldType = "chk";

        if (options.buildType != undefined)
            bldType = options.buildType;

        var clr_file;
        if (isCoreCLRBuild(bldType)) {
            clr_file = "coreclr.dll";
        }
        else {
            clr_file = "clr.dll";
        }

        installCLRBinary(clr_file, options);
        installCLRBinary("mscordac*.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of mscordbi completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuilds System.Core.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildCore(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "System.Core.dll")) {
        
        // Build System.Core.dll
        userBuild("ndp\\fx\\src\\Core", options);
        
        // Install System.Core.dll
        options.gacInstall = true;
        options.checkNGen = true;
        installCLRBinary("System.Core.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.Core.dll completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuilds System.Numerics.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildNumerics(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "System.Numerics.dll")) {
        
        // Build System.Numerics.dll
        userBuild("ndp\\fx\\src\\Numerics", options);
        
        // Install System.Core.dll
        options.gacInstall = true;
        options.checkNGen = true;
        installCLRBinary("System.Numerics.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.Numerics.dll completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuilds System.Device.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildDevice(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "System.Device.dll")) {
        
        // Build System.Device.dll
        userBuild("ndp\\fx\\src\\Device", options);
        
        // Install System.Core.dll
        options.gacInstall = true;
        options.checkNGen = true;
        installCLRBinary("System.Device.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.Device.dll completed.\n");
    return _checkRunEnv(options);
}


/****************************************************************************/
/* Incrementally rebuilds System.AddIn.Contract.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildAddInContract(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "System.AddIn.Contract.dll")) {
        
        // Build System.AddIn.Contract.dll
        userBuild("ndp\\fx\\src\\AddIn\\Contract", options);
        
        // Install System.AddIn.Contract.dll
        options.gacInstall = true;
        installCLRBinary("System.AddIn.Contract.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.AddIn.Contract.dll completed.\n");
    return _checkRunEnv(options);
}


/****************************************************************************/
/* Incrementally rebuilds npctrl.dll and npctrlui.dll and installs them. 
   Installing only works if you have a valid install already. This function
   always builds with OACR turned off.

   Parameters:
   options:  
        See userBuild function for more options
*/
function buildNpctrl(options) {

    options = getOptions(buildInstallOptions, options);

    bldDir = Env("_NTTREE");    

    logMsg(LogClrAutomation, LogInfo, "Building slr:\n");
    userBuild("xcp\\slr", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building common:\n");
    userBuild("xcp\\common", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building control\\common:\n");
    userBuild("xcp\\control\\common", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building core:\n");
    userBuild("xcp\\core", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building control\\download:\n");
    userBuild("xcp\\control\\download", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building xml:\n");
    userBuild("xcp\\xml", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building updater:\n");
    userBuild("xcp\\updater", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building drmloader:\n");
    userBuild("xcp\\drmloader", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Building win:\n");
    userBuild("xcp\\win", options, bldDir, "no_oacr");

    logMsg(LogClrAutomation, LogInfo, "Copying files:\n");
        
    installSLBinary("bin\\i386\\npctrl.dll",   options);
    installSLBinary("bin\\i386\\npctrlui.dll", options);

    logMsg(LogClrAutomation, LogInfo, "Incremental build of npctrl.dll completed.\n");

    return 0;
}

/****************************************************************************/
/* Incrementally rebuilds System.AddIn.dll and installs it.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildAddIn(options) {
    options = getOptions(buildInstallOptions, options);
    
    // If the user asked for quick-build check, see if we can quickly return
    if (!_checkQuickBuild(options, "System.AddIn.dll")) {
        
        // Build System.AddIn.dll
        userBuild("ndp\\fx\\src\\AddIn\\AddIn", options);
        
        // Install System.AddIn.dll
        options.gacInstall = true;
        installCLRBinary("System.AddIn.dll", options);
    }
    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.AddIn.dll completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* Incrementally rebuilds System.AddIn tools and installs them.

   Parameters:
   options:  
        /quickBuild:<vcProjFile> quickly check if all files in 'vcProjFile'
                    are up to date with respect to dll, and return success
        See userBuild function for more options
        See installCLRBinary function for more options
*/
function buildAddInTools(options) {
    options = getOptions(buildInstallOptions, options);
    
    // Build System.AddIn tools
    userBuild("ndp\\fx\\src\\AddIn\\Tools", options);
    
    // Install System.AddIn tools
    options.gacInstall = false;
    installCLRBinary("AddInProcess.exe", options);
    installCLRBinary("AddInProcess32.exe", options);
    installCLRBinary("AddInUtil.exe", options);
    
    logMsg(LogClrAutomation, LogInfo, "Incremental build of System.AddIn tools completed.\n");
    return _checkRunEnv(options);
}

/****************************************************************************/
/* copies a file 'srcPath' to the target 'targetDir'.  Will rename a file
   that is locked and it will also place the pdb file next to it if it is
   a DLL.
*/

function installFile(srcPath, targetDir) {

    if (!FSOFolderExists(targetDir))
        throw Error(1, "Could not find target folder : " + targetDir);

    logMsg(LogClrAutomation, LogInfo, "Copying ", srcPath, "\n     -> ", targetDir, "\n");
    FSOCopyFile(srcPath, targetDir, "FORCE");

    if (!srcPath.match(/((.*)\\)?(.*)\.((dll)|(exe))$/i))
        return;

    var fileDir   = RegExp.$2;
    var fileNoExt = RegExp.$3;
    var fileExt   = RegExp.$4;
    
    if (fileDir == "")
        fileDir = ".";

    fileDir = fileDir + "\\Symbols.pri\\retail\\" + fileExt;
    var fileName = fileNoExt + ".pdb";
    var pdbPath = fileDir + "\\" + fileName;

    if (fileName.match(/\*/))
    {
        fileName = fileName.replace(/(\.|\$)/g, "\\$1");
        fileName = fileName.replace(/\*/, ".*");
        
        if (FSOGetFilePattern(fileDir, "^" + fileName + "$").length == 0)
        {
            logMsg(LogClrAutomation, LogInfo, "Warning could not find PDBs matching the mask ", pdbPath, "\n");
            return;
        }
    } else if (!FSOFileExists(pdbPath))
    {
        logMsg(LogClrAutomation, LogInfo, "Warning could not find PDB ", pdbPath, "\n");
        return;
    }
    logMsg(LogClrAutomation, LogInfo, "Copying ", pdbPath, "\n     -> ", targetDir, "\n");
    FSOCopyFile(pdbPath, targetDir, "FORCE");
}


/****************************************************************************/
/* Install a binary to the SDK.
*/
function installSDKBinary(fileName, options)
{
    if (!_isCompatibleArch(options)) {
        logMsg(LogClrAutomation, LogInfo, "Wrong architecture for build, skipping install\n");
        return;
    }
    
    // This is where the CLR binaries get built
    var binariesDir = _getBinariesDir(options);
    var sourceFilePath = binariesDir + "\\" + fileName;
        
    // This is where we copy to
    var targetDir = _getSDKBinaryDir(options);

    // install the file and PDB 
    installFile(sourceFilePath, targetDir);
}

/****************************************************************************/
/* Copy a binary to the Silverlight version folder.

   Parameters:
    fileName:   Name of the binary.
*/

function installSLBinary(fileName, options) {
    options = getOptions(buildInstallOptions, options);
    
    var binariesDir = Env("_NTTREE");
    var targetDir = _getSLRuntimeDir(options);

    var sourceFilePath = binariesDir + "\\" + fileName;

    // install the file
    if (FSOFolderExists(targetDir))
    {
        FSOCopyFile(sourceFilePath, targetDir, "FORCE");
    }
    else
    {
        logMsg(LogClrAutomation, LogError, "Unable to find a valid install at: " + targetDir + "\n");
        throw Error(1, "Unable to find a valid install at: " + targetDir);
    }
}
    


/****************************************************************************/
/* Copy a binary to the CLR version folder, and install it as appropriate.

   Parameters:
    fileName:   Name of the binary.
    options:
        /mz          ngen mscorlib
        /ngenAll     ngen all the previously-ngenned assemblies

        /gacInstall  Should the binary be installed into the GAC?
        /checkNGen   Should we check if we need to ngen?
*/

function installCLRBinary(fileName, options) {
    options = getOptions(buildInstallOptions, options);
    
    if (!_isCompatibleArch(options)) {
        logMsg(LogClrAutomation, LogInfo, "Wrong architecture for build, skipping install\n");
        return;
    }
    
    var binariesDir = _getBinariesDir(options);
    var targetDir = _getRuntimeDir(options);

    var sourceFilePath = binariesDir + "\\" + fileName;

    // install the file and PDB 

    installFile(sourceFilePath, targetDir);

    // Install to the GAC if requested

    if (options.gacInstall) {
        logMsg(LogClrAutomation, LogInfo, "gacutil /i /f ", fileName, "\n"); 
        runCmd("gacutil /i " + fileName + " /f", runSetCwd(targetDir));
    }

    // NGen if needed 

    if (options.checkNGen) {
        if (options.ngenAll) {
            logMsg(LogClrAutomation, LogInfo, "ngen update\n");
            runCmd("ngen update");
        }
        else if (options.mz) {
            logMsg(LogClrAutomation, LogInfo, "ngen install mscorlib\n");
            runCmd("ngen install mscorlib");
        }
        else {
            // All ngen images are probably invalidated.
            logMsg(LogClrAutomation, LogWarn, "mscorlib's ngen image is invalid.\n");
            logMsg(LogClrAutomation, LogWarn, "Consider using \"runjs build<Binary> /mz\" or \"ngen install mscorlib\".\n");
        }
    }
}

/******************************************************************************/
/* Copies and registers the specified MAN or MOF file.  Specific steps:
    1. Copy specified MAN / MOF file to framework install directory (by
       default this is inferred from the clrenv environment variables).  You
       may specify one of the following files, for example:
            ClrEtwAll.man (our complete public + private ETW manifest file)
            Clr-Etw.man   (our public-only, shipped ETW manifest file)
            CLREtwAll.mof (our pre-Vista, complete MOF file)
            CLR.mof       (our pre-Vista, public-only MOF file)
    2. MAN file only: Repair path to clretwrc.dll specified in the MAN file.
       The build generates the MAN file with the path to clretwrc.dll as the
       binaries directory.  This step replaces the binaries directory with the
       framework install directory.
    3. Register the MAN or MOF by unregistering the previous file and
       registering the current one via calls to wevtutil.exe or mofcomp.exe.

Parameters
    sourcePath    Relative or full path to MAN / MOF to install.  For
                  example, if you do a 'go bin', then you can just specify
                  the filename.  Or, you can specify a full path (including
                  UNC paths to install the MAN / MOF from a network share).
    installDir    If unspecified, the framework install dir is inferred (i.e.,
                  same dir that 'go tgt' would go to).  Otherwise, you may
                  override with your own path
                  (e.g., C:\Windows\Microsoft.NET\Framework\v4.0.x86ret)
*/
function installETWManOrMof(sourcePath, installDir) {
    // MAN or MOF?
    var fIsMan;
    var extension = FSOGetExtension(sourcePath).toLowerCase();
    if (extension == "man") {
        fIsMan = true;
    } else if (extension == "mof") {
        fIsMan = false;
    } else {
        throw (new Error(1, "installETWManOrMof was called with a file with unknown extension :'" + extension + "'.  Expected 'man' or 'mof'"));
    }
    
    // Copy file to the installation directory
    if (installDir == undefined) {
        installDir = _getRuntimeDir(buildInstallOptions);
    }
    installFile(sourcePath, installDir);
    if (!fIsMan) {
        // For MOFs, remember to copy over the uninstall file, too
        installFile(sourcePath + ".uninstall", installDir);
    }
    var destPath = installDir + "\\" + FSOGetFileName(sourcePath);
    
    if (fIsMan) {
        // Update clretwrc.dll path in manifest so it points to installDir
        logMsg(LogClrAutomation, LogInfo, "Repairing path to clretwrc.dll inside " + destPath + "\n");                    

        var replacePath = installDir + "\\clretwrc.dll";

        // Read current manifest contents
        var manifestFile = FSOOpenTextFile(destPath, FSOForReading); // 1 means open file for reading
        var lines = manifestFile.ReadAll();
        lines = lines.replace(/resourceFileName[\r\t\n ]*?=.*?clretwrc.dll/gi, "resourceFileName=\"" + replacePath);
        lines = lines.replace(/messageFileName[\r\t\n ]*?=.*?clretwrc.dll/gi, "messageFileName=\"" + replacePath);
        manifestFile.Close();

        // Write updated manifest contents
        manifestFile = FSOCreateTextFile(destPath, true /* overwrite */);
        manifestFile.Write(lines);
        manifestFile.Close();

        // Now Install the .man file (which requires uninstalling the previous one first)
        logMsg(LogClrAutomation, LogInfo, "Running wevutil.exe to install " + destPath + "\n");
        runCmd("wevtutil.exe um \"" + destPath + "\"");
        runCmd("wevtutil.exe im \"" + destPath + "\"");
    }
    else {
        // Install clr.mof (which requires uninstalling the previous one first)
        logMsg(LogClrAutomation, LogInfo, "Running mofcomp.exe to install " + destPath + "\n");
        runCmd("mofcomp.exe \"" + destPath + ".uninstall\"");
        runCmd("mofcomp.exe \"" + destPath + "\"");
    }

    return 0;
}



/******************************************************************************/
/* Basically this is razzleBuild (rbuild) with filtering so you just see what 
   is actually being compiled or linked and any error messages.  It is 
   more reasonable output for humans.   By default it works like rbuild.  
   (in fact 'runjs userBuild' should work just like rbuild).  

   Parameters
        bldDir                   The directory to build
        razzleArgs               Additional arguments for razzle, for example:
                                 no_oacr

   Options
        /buildType:<bldType>     Override the build type (chk, dbg ...)
        /buildArch:<bldArch>     Override the build arch (x86 amd64 ...)
        /buildArgs:<args>        Pass extra args to build.exe (eg -c)

        binDir                   The directory to drop binaries to
*/
function userBuild(bldDir, options, binDir, razzleArgs) {

    if (bldDir == undefined)
        bldDir = ".";
    options = getOptions(userBuildOptions, options);

    if (bldDir.match(/^\w/))
        bldDir = srcBaseFromScript() + "\\" + bldDir;

    var logName = bldDir + "\\build." + options.buildArch + options.buildType +".stdout.txt";
    var logFile = FSOCreateTextFile(logName, true);
    
    var prevOutputFtn;
    try {
        prevOutputFtn = logSetOutputFunction({ftn:_userBuildFilter, logFile:logFile});
        razzleBuild(options.buildType, options.buildArch, bldDir, options.buildArgs, undefined, binDir, undefined, undefined, razzleArgs)
    }
    catch(e) {
        logSetOutputFunction(prevOutputFtn);
        logMsg(LogScript, LogInfo, "Transcript: ", logName, "\n");
        throw e;
    }
    logSetOutputFunction(prevOutputFtn);
    logMsg(LogScript, LogInfo, "Done. Transcript: ", logName, "\n");
    return 0
}

/****************************************************************************/
/* Helper function needed in userBuild.  It gets called line
   by line and its job is to show only what a user is likely to want to
   see, filtering out anything else.  
*/
function _userBuildFilter(line, context) {
    
    // If you do /log:script:LogInfo10 you will see what we normally hide
    var shouldPrint = false;
    if (logGetFacilityLevel(LogScript) >= LogInfo10)
        shouldPrint = true;

    if (context.logFile)
        context.logFile.WriteLine(line);
        
    // These are the other things that are interesting to the user. 
    //if (line.match(/^\d*\>?Compiling -/)) 
    //    shouldPrint = true;
    if (line.match(/^\d*\>?Linking \w+ -/))
        shouldPrint = true;
    if (line.match(/^\d*\>?Building Library -/))
        shouldPrint = true;
    if (line.match(/\(\d+\)\s*:\s*error\b/)) 
        shouldPrint = true;

    if (shouldPrint) {
        var sav = logSetPrefixTime(false);
        logMsg(LogScript, LogInfo, line, "\n");
        if (line.match(/\(\d+\)\s*:\s*error\b.*Cannot open.*clrinternal.h/))
            logMsg(LogScript, LogInfo, "build(0) : error B0000: This error message is usually cause by the failure to do a full build before using the incremental build scripts.\n");
        logSetPrefixTime(sav);
    }
    else {
        if (line.match(/RUNCMD.*pushd (\S+) .*razzle\S+ (\S+) (\S+)/)) 
            logMsg(LogScript, LogInfo, "Building for ", RegExp.$2, RegExp.$3, " in ", RegExp.$1, " ...\n");
    }
}

/******************************************************************************/
function _doesVSProjectNeedRebuilding(vcProjName, targetFileName) {
    
    var vcProjDir = ".";
    if (vcProjName.match(/(.*)\\.*$/))
        vcProjDir   = RegExp.$1;
    
    if (!FSOFileExists(targetFileName))
        return true;
    var targetFileDate = FSOGetFile(targetFileName).DateLastModified;
    
    var file = FSOOpenTextFile(vcProjName, FSOForReading);
    while(!file.AtEndOfStream) {
        line = file.readLine();
        if (line.match(/RelativePath="(.*)"/)) {
            var relName = RegExp.$1;
            var srcFileName     = vcProjDir + "\\" + relName;
            // logMsg(LogClrAutomation, LogInfo, "Got srcName = ", srcFileName, "\n");
            
            if (FSOFileExists(srcFileName)) {
                var srcFileDate = FSOGetFile(srcFileName).DateLastModified;
                if (srcFileDate > targetFileDate) {
                    logMsg(LogClrAutomation, LogInfo, "File ", relName, " needs to be rebuilt\n");
                    return true;
                }
            }
            else 
                logMsg(LogClrAutomation, LogWarn, "File ", srcFileName, " does not exist");
        }
    }
    logMsg(LogClrAutomation, LogInfo, "Project ", vcProjName, " up to date\n");
    return false;
}

/****************************************************************************/
/* Verify that the environment that we are running for VS 
   matches the build that we are currently performing
*/
function _checkRunEnv(options) {

    options = _getBuildOptions(options);
    
    var defaultVer = Env("Complus_DefaultVersion").toLowerCase();
    var expectedDefaultVer = ("v4.0." + options.buildArch + options.buildType).toLowerCase();
    var sav = logSetPrefixTime(false);
    
    var ret =0;  // Successful return is zero

    if (!defaultVer) {
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: The build completed and installed successfully, but the environment is not set up to run.\n");
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: COMPLUS_DefaultVersion not set, you will not run the runtime you just built.\n");
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: In Visual Studio, this can be fixed by running 'start devenv' from an environment with this variable set (eg CLREnv).\n");
        ret = 1;
    }
    else if (defaultVer != expectedDefaultVer) {
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: The build completed and installed successfully, but the environment is not set up to run.\n");
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: COMPLUS_DefaultVersion = ", defaultVer, " != ", expectedDefaultVer, " (the version just built).\n");
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: Either - Set the build type to match COMPLUS_DefaultVersion.\n");
        logMsg(LogClrAutomation, LogInfo, "build(0) : error B0000: Or     - Restart VS ('start devenv') in an environment with matching COMPLUS_DefaultVersion set.\n");
        ret = 1;
    }
    
    logSetPrefixTime(sav);
    return ret;
}

/****************************************************************************/
/* returns true if we can install on the current machine (the architecture
   is compatible with the current build flavor.
*/
function _isCompatibleArch(options) {

    options = _getBuildOptions(options);
    var buildArch  = options.buildArch.toLowerCase();
    var procArch64 = Env("PROCESSOR_ARCHITEW6432").toLowerCase();
    var procArch   = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
    
    if (procArch64 && buildArch == procArch64)  // we are in the wow. 
        return true;
    
    return (buildArch == procArch);
}

/****************************************************************************/
/* Checks if the /quickBuild option is selected in 'options' and if so 
   checks if all the files in the VC project specified by 'quickbuild' 
   are older than 'targetBinaryName'.  If so, we assume that there is no 
   need to build, and return true (you can skip the build).  This allows
   for really fast builds when nothing has changed */

function _checkQuickBuild(options, targetBinaryName) {

    // User did not ask for quick build option, nothing to do
    if (!options.quickBuild)
        return false;
    
    if (options.quickBuild == "")
        throw new Error(1, "Error: the /quickBuild option must have a value of the vcproj file"); 
    
    var binariesDir = _getBinariesDir(options); 
    var targetBinaryPath = binariesDir + "\\" + targetBinaryName;
    
    return !_doesVSProjectNeedRebuilding(options.quickBuild, targetBinaryPath);
}

/****************************************************************************/
/* givn options, insure that the build options are set, and if not set them
   to the appropriate defaults.  */

function _getBuildOptions(options) {
    if (!options.buildArch) {
        options.buildArch = Env("_BuildArch");
        if (!options.buildArch)
            options.buildArch = "x86";
    }
    if (!options.buildType) {
        options.buildType = Env("_BuildType");
        if (!options.buildType)
            options.buildType = "chk";
    }
    
    return options;
}

/****************************************************************************/
/* given options (which might include buildArch and buildType options find
   the directory where the build put the binaries 
*/
function _getBinariesDir(options) {
    options = _getBuildOptions(options);
    return srcBaseFromScript() + "\\binaries\\" + options.buildArch + options.buildType;
}

/****************************************************************************/
/* find the directory where the SDK lives. 
*/
function _getSDKBinaryDir(options)
{
    return _getRuntimeDir(options) + "\\sdk\\bin";
}

/****************************************************************************/
/* Find the directory where the real Silverlight runtime lives. This requires 
   an actual install of Silverlight be present.
*/
function _getSLRuntimeDir(options) {
    var pfiles = Env("ProgramFiles(x86)");
    if (undefined == pfiles)
    {
        pfiles = Env("ProgramFiles");
    }
    var version = WshShell.RegRead("HKLM\\SOFTWARE\\Microsoft\\Silverlight\\version");
    if (undefined == version)
    {
        version = WshShell.RegRead("HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Silverlight\\version");
    }
    return pfiles + "\\Microsoft Silverlight\\" + version;
}

/****************************************************************************/
/* Find the version of IE installed on the machine. 
*/
function _getIEVersion(options) {
    var version = WshShell.RegRead("HKLM\\SOFTWARE\\Microsoft\\Internet Explorer\\Version Vector\\IE");
    if (undefined == version)
    {
        version = WshShell.RegRead("HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\Version Vector\\IE");
    }
    return version;
}


/****************************************************************************/
/* find the directory where the runtime lives.  Note that we don't use the
   shim here because for CLR devs we are specifically NOT refering to the default
   runtime on the machine, but we mean a private runtime setup by CLRSetup 
*/
var warnedAboutMajorVersion = false;
var warnedAboutUrtTarget;
function _getRuntimeDir(options)
{
    var targetDir;
    var urtTarget = Env("URTTARGET");
    var defaultVer = Env("COMPLUS_DefaultVersion");

    options = _getBuildOptions(options);

    // Order of preference:
    // URTTARGET
    // construct string using the COMPLUS_DefaultVersion
    // construct string from build info
    if (urtTarget)
    {
        targetDir = urtTarget;
    }
    else if (options.buildArch && options.buildType)
    {
        targetDir = Env("WINDIR") + "\\Microsoft.NET\\Framework";
        if (options.buildArch.match(/64/))
            targetDir += "64";

        if (defaultVer)
        {
            targetDir +=  "\\" + defaultVer;
        }
        else
        {
            var version = Env("DD_NDPMajorVersion");
            targetDir += "\\" + version + "." + options.buildArch + options.buildType;

            if (!warnedAboutMajorVersion && (version == undefined || version == ""))
            {
                logMsg(LogClrAutomation, LogInfo, "WARNING: DD_NDPMajorVersion not set.\n");
                warnedAboutMajorVersion = true;
                throw Error(1, "DD_NDPMajorVersion not set\n" );
            }
        }

        // If URTTARGET was not set, spit out a warning.
        // We are likely to fail the FSOFolderExists check, so this will
        // give the user a reason why.
        if (!warnedAboutUrtTarget)
        {
            if (defaultVer)
            {
                logMsg(LogClrAutomation, LogInfo, "WARNING: URTTARGET not set. Infering from COMPLUS_DefaultVersion.\n");
            }
            else
            {
                logMsg(LogClrAutomation, LogInfo, "WARNING: URTTARGET not set. Infering from the build type.\n");
            }
            logMsg(LogClrAutomation, LogInfo, "Assuming URTTARGET: ", targetDir, "\n");
            warnedAboutUrtTarget = true;
        }
    }

    if (!FSOFolderExists(targetDir))
    {
        var sav = logSetPrefixTime(false);
        logMsg(LogClrAutomation, LogInfo, "install(0) : error I0000: target directory ", targetDir, " does not exist\n");
        logSetPrefixTime(sav);
        throw Error(1, "Could not find installation directory " + targetDir, "\n" );
    }
    return targetDir;
}

/****************************************************************************/
/* currently experiemental (probably belongs in someones run.local.js file */

function getImageBase(peFileName) {

    var objStream = WScript.CreateObject("ADODB.Stream");
    objStream.Open();
    objStream.Type = 1; // Binary file type
    objStream.LoadFromFile(peFileName);
    var data = objStream.Read(512);
    objStream.Close();
    dumpStr(data);
}

function checkOut(fileName) {
    if (_inTFS())
    {
        runCmd("tf checkout " + fileName);
    }
    else
    {
        runCmd("sd edit " + fileName);
    }
}

/* Don't check out files that don't have any changes to them. */
function uncheckoutUnchanged(fileName) {
    if (_inTFS())
    {
        runCmd("tfpt uu /noget " + fileName);
    }
    else
    {
        runCmd("sd revert -a " + fileName);
    }
}
