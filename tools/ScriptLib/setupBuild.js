/*********************************************************************************/
/*                                setupBuild.js                                  */
/*********************************************************************************/

/* This file contains the functions to build real layouts.
*/

/*  AUTHOR: Joel Hendrix
    Date:   12/30/2009 
    Owner: jhendrix
*/
/*********************************************************************************/

// global variables
var SrcBase = Env("_NTDRIVE") + Env("_NTROOT");
var BinRoot = Env("_NTTREE").substr(0, Env("_NTTREE").lastIndexOf("\\"));
var SetupBuildLog; // initialized in buildLayouts
var CookieFileName = "setupBuildCookie.txt";
var StaleCookie = 1;
var FreshCookie = 0;
var NoCookie = -1;
var VerboseSpew = false;
var SetupBuildModuleDefined = 1;

/****************************************************************************/
/*  Automation for building layouts.

    Usage:

        runjs buildLayouts [sku] [architecture] [type] [binDir] [remoteStore] [setupRoot] [failOnPause] [skipFileCheck] [verbose]

    [sku] can be client, full, oob, all, or '_' (defaults to full)
    Specifying 'all' builds the full SKU for all applicable architectures.

    [architecture] can be x86, amd64, ia64, or '_' (defaults to razzle environment)

    [type] can be ret, chk, or '_' (defaults to razzle environment)

    [binDir] location of binaries to package up (defaults to %_NTTREE%)

    [remoteStore] points to a raw drop and is a location where non-CLR bits are included in setup.
    Example: \\cpvsbuild\drops\dev10\rtmrel\raw\30319.00
    If no remote store is specified then the LKG remote store value (specified in CLRSetupInfo.bat) will be used.

    [setupRoot] directory where all intermediate files and layouts will be stored (defautls to %_NTBINDIR%Setup).

    [failOnPause] Instances that would prompt for user input will throw an exception instead (defaults to false).

    [skipFileCheck] Disables checking for locally built files (defaults to false).

    [verbose] Enables full build spew (defaults to false).

    NOTE: When specifying a remote store make sure that it contains binaries that match the bitness and flavor
    of the layouts you need to build (e.g. some branches don't always build CHK, so if you're building CHK
    layouts ensure that your remote store contains CHK bits else your layouts will fail to build).  The build
    lab no longer builds FRE (only RET) so unless you've performed a full stack FRE build your setup build is
    guaranteed to fail.  For DBG builds the remote store will fall back to use CHK.

    No arguments specified to "runjs buildLayouts" will build the full redist SKU for the current razzle architecture.
*/
function buildLayouts(sku, architecture, type, binDir, remoteStore, setupRoot, failOnPause, skipFileCheck, verbose) {
    if (!nsIsNullOrEmpty(failOnPause)) {
        if (failOnPause.match(/^true$/))
            failOnPause = true;
        else if (failOnPause.match(/^false$/))
            failOnPause = false;
        else
            throw new Error("Unrecognized value '" + failOnPause + "' passed for failOnPause argument");
    } else {
        failOnPause = false;
    }

    if (Env("_BuildType").match(/^fre$/i)) {
        logMsg(LogClrAutomation, LogWarn, "The build lab no longer compiles FRE builds so remote stores on \\\\cpvsbuild won't provide non-CLR bits.\n");
        logMsg(LogClrAutomation, LogWarn, "Unless you've performed a full stack build (including VS for the compilers) your layouts will fail to build.\n");
        logMsg(LogClrAutomation, LogWarn, "To exit press CONTROL-BREAK now or ENTER to continue with the build.\n");
        if (failOnPause)
            throw new Error("Unrecoverable warning, build failed!");
        else
            WScript.StdIn.Read(1);
    }
    
    if (Env("_BuildType").match(/^dbg$/i))
        logMsg(LogClrAutomation, LogInfo, "NOTE: The build lab does not build DBG flavor so remote store will fall back to CHK.\n");

    // the basic idea is that we'll build a temporary SKU list based on the parameters passed in.
    // this way we get all the results in one log file which makes parsing for failures much easier.
    var skuList = "";

    if (nsIsNullOrEmpty(sku))
        sku = "full";
    else if (!sku.match(/^client$/i) && !sku.match(/^full$/i) && !sku.match(/^oob$/i) && !sku.match(/^all$/i))
        throw new Error("Unrecognized SKU \"" + sku + "\", valid values are [client, full, oob, all]");

    if (nsIsNullOrEmpty(architecture))
        architecture = Env("_BuildArch");
    else if (!architecture.match(/^amd64$/i) && !architecture.match(/^ia64$/i) && !architecture.match(/^x86$/i))
        throw new Error("Unrecognized architecture \"" + architecture + "\", valid values are [amd64, ia64, x86]");

    if (nsIsNullOrEmpty(type))
        type = Env("_BuildType");
    else if (!type.match(/^ret$/i) && !type.match(/^chk$/i) && !type.match(/^dbg$/i))
        throw new Error("Unsupported build type '" + type + "' specified, valid values are [ret, chk, dbg]");

    if (!nsIsNullOrEmpty(binDir)) {
        if (!FSOFolderExists(binDir))
            throw new Error("The specified binDir '" + binDir + "' does not exist!");
        
        //_validateBinDir(binDir);
        SetupBuildLog = WshFSO.BuildPath(binDir, "runjs.buildLayouts.log");
    }
    else {
        //_validateBinDir(Env("_NTTREE"));
        SetupBuildLog = WshFSO.BuildPath(Env("_NTTREE"), "runjs.buildLayouts.log");
    }

    if (!FSOFolderExists(WshFSO.GetParentFolderName(SetupBuildLog))) {
        FSOCreateFolder(WshFSO.GetParentFolderName(SetupBuildLog));
    }
    
    if (nsIsNullOrEmpty(remoteStore)) {
        remoteStore = _getLKGRemoteStore();

        if (!remoteStoresAvailable(remoteStore))
            throw new Error("The LKG remote stores '" + remoteStore + "' do not exist, please notify your BFD so they can update it.  You will need to specify a remote store until this is fixed.");
    }
    else if (!remoteStoresAvailable(remoteStore)) {
        throw new Error("The specified remote stores '" + remoteStore + "' do not exist; please specify a different remote store.");
    }

    if (!nsIsNullOrEmpty(skipFileCheck)) {
        if (skipFileCheck.match(/^true$/i))
            skipFileCheck = true;
        else if (skipFileCheck.match(/^false$/i))
            skipFileCheck = false;
        else
            throw new Error("Unrecognized value '" + skipFileCheck + "' specified for skipFileCheck, valid values are [true, false].");
    } else {
        skipFileCheck = false;
    }

    if (!nsIsNullOrEmpty(verbose)) {
        if (verbose.match(/^true$/i))
            VerboseSpew = true;
        else if (verbose.match(/^false$/i))
            VerboseSpew = false;
        else
            throw new Error("Unrecognized verbose specification '" + verbose + "', valid values are [true, false].");
    } else {
        VerboseSpew = false;
    }

    logMsg(LogClrAutomation, LogInfo, "Begin building SKU '" + sku + "' for " + architecture + type + "\n");

    // first see if there are locally compiled files
    _checkForLocalFiles(architecture, type, binDir, failOnPause, skipFileCheck);

    // NOTE: we always create the build environment from scratch, that means that it isn't safe to perform any
    // steps that rely on it until after the following function call
    _configureBuildEnvironment(remoteStore, architecture, type, binDir, setupRoot);

    var cookieState = _getBuildCookieState(type, remoteStore);
    if (cookieState == StaleCookie) {
        logMsg(LogClrAutomation, LogWarn, "Remote store settings have changed since the last time layouts were built, this will require purging existing binaries cache.\n");
        cleanLayouts("binaries", type, "", "false");
    }

    // the SKU list is case-sensitive and expects lower case values
    skuList = _createSkuList(architecture.toLowerCase(), sku.toLowerCase());
    logMsg(LogClrAutomation, LogInfo, "Created SKU list: " + skuList + "\n");

    var result = -1; // failure

    try {
        logMsg(LogClrAutomation, LogInfo, "Building layouts, please wait (this can take up to 10 minutes)...\n");
        _buildSkuList(skuList, type);

        // save values used to create this build so we can check on subsequent builds if they've changed thus prompting us to do a cleanstore
        if (cookieState != FreshCookie)
            _createBuildCookie(type, remoteStore);

        _binplaceLayouts(sku, architecture, type, binDir);

        _cleanTemporarySetupFiles(sku, architecture, type);

        logMsg(LogClrAutomation, LogInfo, "Done!\n");
        logMsg(LogClrAutomation, LogInfo, "Setup build log: " + SetupBuildLog + "\n");
        logMsg(LogClrAutomation, LogInfo, "Layouts directory: " + _getSetupRootDirectory(type) + "\\layouts\n");
        logMsg(LogClrAutomation, LogInfo, "Or execute 'go layouts' to CD to the layouts directory for this razzle window.\n");
        logMsg(LogClrAutomation, LogInfo, "Build completed successfully\n");
        result = 0; // success
    }
    catch (e) {
        logMsg(LogClrAutomation, LogError, "An error was encountered: " + e.message + "\n");

        var settings = {};
        settings.RemoteStore = remoteStore;
        settings.Type = type;

        try {
            _parseBuildLogForFailures(settings);
        }
        catch (e) {
            logMsg(LogClrAutomation, LogError, "Failed to parse the build log for failures: " + e.message + "\n");
        }

        logMsg(LogClrAutomation, LogError, "Build failed!\n");
        logMsg(LogClrAutomation, LogInfo, "Please see http://mswikis/clr/dev/Pages/How%20to%20build%20layouts.aspx for additional trouble-shooting tips.");
    }

    return result;
}

/****************************************************************************/
/*  "Publishes" (i.e. copies) layouts to a directory for later use.

    Usage:

        runjs publishLayouts [all] [location]

    Specifying "all" will copy all SKUs for all architectures; omitting will default to copying all SKUs for the architecture/flavor specified by your razzle environment.
    By default "location" will be %_NTBINDIR%\\layouts\\created.%TIME%.
    This will fail if the specified location already exists.
*/
function publishLayouts(options, location) {
    if (!nsIsNullOrEmpty(options) && !options.match(/^all$/i))
        throw new Error("Unknown argument '" + options + "'.  Valid options are [all].");

    if (nsIsNullOrEmpty(location)) {
        location = SrcBase + "\\layouts";
        location = getUniqueDateFileName(location, "created.", "");
    }

    if (FSOFolderExists(location))
        throw new Error("Destination directory '" + location + "' already exists.");

    FSOCreatePath(location);

    var sources = new Array();

    if (!nsIsNullOrEmpty(options) && options.match(/^all$/i)) {
        // copy all the layouts.  for each architecture in the layouts dir
        // construct source and destination paths and add it to the array
        var layoutsBase = FSOGetFolder(_getSetupRootDirectory(Env("_BuildType")) + "\\layouts");
        var iter = new Enumerator(layoutsBase.SubFolders);

        while (!iter.atEnd()) {
            var subDir = iter.item();
            var layoutSubDir = subDir + "\\enu\\netfx";

            if (FSOFolderExists(layoutSubDir)) {
                var pathSpec = {};
                pathSpec.Source = layoutSubDir;
                pathSpec.Destination = location + "\\" + subDir.Name.match(/\w+$/);

                sources.push(pathSpec);
            }

            iter.moveNext();
        }
    }
    else {
        // copy just the layouts as specified by the razzle environment
        var subDir = _getSetupRootDirectory(Env("_BuildType")) + "\\layouts\\" + Env("_BuildArch") + Env("_BuildType") + "\\enu\\netfx";
        if (FSOFolderExists(subDir)) {
            var pathSpec = {};
            pathSpec.Source = subDir;
            pathSpec.Destination = location + "\\" + Env("_BuildArch") + Env("_BuildType");
            sources.push(pathSpec);
        }
    }

    if (sources.length == 0)
        throw new Error("There are no layouts to publish!");

    for (i = 0; i < sources.length; ++i) {
        var pathSpec = sources[i];
        logMsg(LogClrAutomation, LogInfo, "Copying layouts from '" + pathSpec.Source + "' to '" + pathSpec.Destination + "', please wait...\n");
        FSOCopyFolder(pathSpec.Source, pathSpec.Destination, true);
    }

    // copy the cookie file as it contains various settings used during the build
    // this makes it easy to look up the remote store used to create the layouts
    FSOCopyFile(_getBuildCookieFileName(Env("_BuildType")), location + "\\" + CookieFileName, false);

    // TODO: what about symbol files

    logMsg(LogClrAutomation, LogInfo, "Copy complete\n");

    return 0;
}

/****************************************************************************/
/*  This will delete files in the layouts build directory.

    Usage:

        runjs cleanLayouts [options] [type] [setupRoot] [cleanBuildConfig]
        
        [options] can be one of the following:
            intermediate - deletes all temporary dirs used when building layouts (default)
            binaries - deletes all cached binaries and intermediate directories
            all - deletes everything including layouts
            
        [type] is build type (chk, ret, defaults to %_BuildType%)
        
        [setupRoot] directory where all intermediate files and layouts reside (defautls to %_NTBINDIR%Setup).

        [cleanBuildConfig] indicates that build.config should be regenerated with default values (defaults to true).

    Normally you should not need to call this directly, the automation will do the right thing.  However like other software there are bugs in the Wix compiler etc
    and sometimes old intermediate files get used even though the setup authoring has changed which can cause spurious setup build failures.  If your setup build
    starts failing for strange reasons try doing a "runjs cleanLayouts all" to see if it fixes the problem (be sure to "runjs publishLayouts" first if you want to
    keep any existing layouts you've already built).
*/
function cleanLayouts(options, type, setupRoot, cleanBuildConfig) {
    if (!nsIsNullOrEmpty(options) && !options.match(/^intermediate$/i) && !options.match(/^binaries$/i) && !options.match(/^all$/i))
        throw new Error("Unknown argument '" + options + "'.  Valid options are [intermediate, binaries, all].");
    
    if (nsIsNullOrEmpty(type))
        type = Env("_BuildType");

    if (nsIsNullOrEmpty(cleanBuildConfig))
        cleanBuildConfig = "true";

    if (cleanBuildConfig == "true") {
        // generate the default build.config
        _cleanBuildConfig(type);

        var razzleCmd = "buildconfig.exe /list";
        _executeCmdInRazzle(razzleCmd, type);
    }

    if (nsIsNullOrEmpty(setupRoot)) {
        try {
            setupRoot = _getSetupRootDirectory(type);
        }
        catch (e) {
        }
    }

    if (!FSOFolderExists(setupRoot)) {
        logMsg(LogClrAutomation, LogInfo, "Layouts build root directory does not exist so there's nothing to clean up!");
    }
    else {
        // will contain the list of directories to save.
        // list should contain all lower case values
        var dirsToSave = new Array();
        var deleteFilesInRoot = false;
        
        if (nsIsNullOrEmpty(options) || options.match(/^intermediate$/i)) {
            logMsg(LogClrAutomation, LogInfo, "Cleaning up intermediate setup build subdirectories...\n");
            dirsToSave.push("binaries");
            dirsToSave.push("layouts");
        }
        else if (options.match(/^binaries$/i)) {
            logMsg(LogClrAutomation, LogInfo, "Cleaning up intermediate setup build subdirectories and binaries cache...\n");
            dirsToSave.push("layouts");
            deleteFilesInRoot = true;
        }
        else if (options.match(/^all$/i)) {
            logMsg(LogClrAutomation, LogInfo, "Cleaning up all setup build subdirectories...\n");
            deleteFilesInRoot = true;
        }
        
        _cleanLayoutsWorker(setupRoot, dirsToSave, type, deleteFilesInRoot);
        
        logMsg(LogClrAutomation, LogInfo, "Done!\n");
    }

    return 0;
}

function _cleanLayoutsWorker(setupRoot, dirsToSave, type, deleteFilesInRoot) {    
    var setupDirObj = FSOGetFolder(setupRoot);
    var subDirs = new Enumerator(setupDirObj.subFolders);
    
    while (!subDirs.atEnd()) {
        if (!_findMatchingSubDir(subDirs.item().Name.toString(), dirsToSave)) {
            try {
                logMsg(LogClrAutomation, LogInfo, "Deleting directory '" + subDirs.item().Path.toString() + "'\n");
                subDirs.item().Delete(true);
            }
            catch (e) {
                logMsg(LogClrAutomation, LogWarn, "Failed to delete the directory: " + e.message + "\n");
            }
        }
        
        subDirs.moveNext();
    }

    if (deleteFilesInRoot) {
        var files = new Enumerator(setupDirObj.Files);
        while (!files.atEnd()) {
            try {
                logMsg(LogClrAutomation, LogInfo, "Deleting file '" + files.item().Path.toString() + "'\n");
                files.item().Delete(true);
            }
            catch (e) {
                logMsg(LogClrAutomation, LogWarn, "Failed to delete the file: " + e.message + "\n");
            }
            
            files.moveNext();
        }
    }
}

function _findMatchingSubDir(subDir, dirList) {
    var foundMatch = false;
    subDir = subDir.toLowerCase();
    
    for (var index = 0; index < dirList.length; ++index) {
        if (subDir.indexOf(dirList[index]) > -1) {
            foundMatch = true;
            break;
        }
    }
    
    return foundMatch;
}

function _binplaceLayouts(sku, architecture, type, binDir) {
    if (!nsIsNullOrEmpty(sku) && !sku.match(/^client$/i) && !sku.match(/^full$/i) && !sku.match(/^oob$/i) && !sku.match(/^all$/i))
        throw new Error("Unknown SKU '" + sku + "'.  Valid options are [client, full, oob, all].");
    else if (nsIsNullOrEmpty(sku) || sku.match(/^all$/i))
        sku = "full";

    if (nsIsNullOrEmpty(architecture))
        architecture = Env("_BuildArch");

    if (nsIsNullOrEmpty(type))
        type = Env("_BuildType");

    if (nsIsNullOrEmpty(binDir))
        binDir = BinRoot + "\\" + architecture + type;

    if (binDir.charAt(binDir.length - 1) != "\\")
        binDir += "\\";

    var subDir = _getSetupRootDirectory(type);
    if (!FSOFolderExists(subDir))
        throw new Error("The directory '" + subDir + "' does not exist so it would appear that layouts have not yet been built.");

    subDir += "\\layouts\\" + architecture + type + "\\enu\\netfx";
    var msiArch = architecture;
    if (msiArch.match(/^amd64$/i))
        msiArch = "x86_x64";
    else if (msiArch.match(/^ia64$/i))
        msiArch = "x86_ia64";
	
    if (sku.match(/^client$/i))
        subDir += "\\coreredist\\box\\";
    else if (sku.match(/^oob$/i))
        subDir += "\\fulloob\\oob\\";
    else
        subDir += "\\fullredist\\box\\";

    var sourceFile = subDir;
    if (sku.match(/^oob$/i))
        sourceFile += "netfx_fulloob.msu";
    else
        sourceFile += "dotNetFx45_" + sku + "_" + msiArch + ".exe"

    if (!FSOFolderExists(binDir))
        FSOCreateFolder(binDir);

    logMsg(LogClrAutomation, LogInfo, "Copying '" + sourceFile + "' to '" + binDir + "'\n");
    FSOCopyFile(sourceFile, binDir, true);

    return 0;
}

function _createSkuList(arch, sku) {
    // creates a SKU list that is passed to setupbuild.exe
    // note that for 64-bit builds we will implicitly build 32-bit as well since at this time the 64-bit build consumes the 32-bit build
    var skuList = SrcBase + "\\tools\\devdiv\\settings\\skulists\\runjs.skulist.proj";
    if (FSOFileExists(skuList))
        FSODeleteFile(skuList, true);
    
    var fileObj = FSOCreateTextFile(skuList);
    fileObj.WriteLine("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    fileObj.WriteLine("<Project DefaultTargets=\"SetupBuildList\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");
    fileObj.WriteLine("    <Import Project=\"$(_NTBINDIR)\\tools\\devdiv\\decatur\\SetupBuild.targets\"/>");
    fileObj.WriteLine("    <ItemGroup>");

    // core SKUs are built for non-oob builds
    if (!sku.match(/^oob$/i)) {
        // these two SKUs are only built as x86
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreAll\\netfx_CoreAll.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreLib\\netfx_CoreLib.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        // always build this SKU even when building 64-bit SKUs (this way the 64-bit SKUs will contain local bits)
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Core\\netfx_Core.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Core\\netfx_Core.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=" + arch + ";Lang=enu;Media=box</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }

        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreRedist\\netfx_CoreRedist.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=" + arch + ";Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
    }

    if (sku.match(/^full$/i)) {
        // these two SKUs are only built as x86
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\ExtendedAll\\netfx_ExtendedAll.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\ExtendedLib\\netfx_ExtendedLib.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        // always build this SKU even when building 64-bit SKUs (this way the 64-bit SKUs will contain local bits)
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Extended\\netfx_Extended.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Extended\\netfx_Extended.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=" + arch + ";Lang=enu;Media=box</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }

        // this SKU is only built as x86
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\FullAll\\netfx_FullAll.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\FullRedist\\netfx_FullRedist.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=" + arch + ";Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
    }
    else if (sku.match(/^all$/i)) {
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreAll\\netfx_CoreAll.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreLib\\netfx_CoreLib.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Core\\netfx_Core.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        
        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Core\\netfx_Core.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=amd64;Lang=enu;Media=box</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }
        
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreRedist\\netfx_CoreRedist.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        
        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\CoreRedist\\netfx_CoreRedist.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=amd64;Lang=enu;Media=box</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }

        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\ExtendedAll\\netfx_ExtendedAll.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\ExtendedLib\\netfx_ExtendedLib.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Extended\\netfx_Extended.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        
        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Extended\\netfx_Extended.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=amd64;Lang=enu;Media=box</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }
        
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\FullAll\\netfx_FullAll.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=net</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\FullRedist\\netfx_FullRedist.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=box</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
        
        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\FullRedist\\netfx_FullRedist.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=amd64;Lang=enu;Media=box</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }
    }

    if (sku.match(/^oob$/i)) {        
        // Build manifest SKUs so that OOB skus use local man layout
        if (!arch.match(/^x86$/i)) {
            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Core\\netfx_Core.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=amd64;Lang=enu;Media=man</Properties>");
            fileObj.WriteLine("        </ProjectReference>");

            fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Extended\\netfx_Extended.wixproj\">");
            fileObj.WriteLine("            <Properties>" + "Chip=amd64;Lang=enu;Media=man</Properties>");
            fileObj.WriteLine("        </ProjectReference>");
        }

        // x86 manifest skus are needed for all OOBs
        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Core\\netfx_Core.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=man</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\Extended\\netfx_Extended.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=x86;Lang=enu;Media=man</Properties>");
        fileObj.WriteLine("        </ProjectReference>");

        fileObj.WriteLine("        <ProjectReference Include=\"$(SetupProjects)\\netfx\\FullOob\\netfx_fullOob.wixproj\">");
        fileObj.WriteLine("            <Properties>" + "Chip=" + arch + ";Lang=enu;Media=oob</Properties>");
        fileObj.WriteLine("        </ProjectReference>");
    }
  
    fileObj.WriteLine("    </ItemGroup>");
    fileObj.WriteLine("</Project>");
    fileObj.Close();
    
    return skuList;
}

function _cleanBuildConfig(type) {
    // first we need to create the build.config file, we'll just delete any existing one and recreate it from scratch
    // since I have seen instances where this file gets into an weird state and setupbuild.exe ceases to function.
    logMsg(LogClrAutomation, LogInfo, "Cleaning up build.config\n");
    var buildConfig = BinRoot + "\\x86" + type + "\\build.config";
    if (FSOFileExists(buildConfig))
        FSODeleteFile(buildConfig, true);

    buildConfig = BinRoot + "\\x86" + type + "\\build.config.settings.targets";
    if (FSOFileExists(buildConfig))
        FSODeleteFile(buildConfig, true);
}

// preps the build environment
function _configureBuildEnvironment(remoteStore, architecture, type, binDir, setupRoot) {
    logMsg(LogClrAutomation, LogInfo, "Configuring layouts build environment\n");
    logMsg(LogClrAutomation, LogInfo, "Using remote store: " + remoteStore + "\n");
    logMsg(LogClrAutomation, LogInfo, "Binaries directory: " + binDir + "\n");

    _cleanBuildConfig(type);
        
    var buildBoxesDotDatFile = _getBuildBoxesDotDatFile();
    if (FSOFileExists(buildBoxesDotDatFile))
        FSODeleteFile(buildBoxesDotDatFile, true);

    // this will create the build.config file and set the remote store
    var razzleCmd = "buildconfig.exe /alter:SetupsBuild.PartialBuild.RemoteStoreRoot /value:\"" + remoteStore + "\"";
    _executeCmdInRazzle(razzleCmd, type);

    if (!nsIsNullOrEmpty(setupRoot)) {
        // set the path where layouts (and the intermediate files) will be produced
        razzleCmd = "buildconfig.exe /alter:BinariesBuild.Setups /value:" + setupRoot;
        _executeCmdInRazzle(razzleCmd, type);
    }

    if (!nsIsNullOrEmpty(binDir)) {
        var datFile = _createBuildBoxesDotDat(type, binDir, setupRoot);
        razzleCmd = "buildconfig.exe /alter:Resources.BuildBoxesDataFile /value:" + datFile;
        _executeCmdInRazzle(razzleCmd, type);
    }

    // disable 7zip compression, it's not needed for functional testing and greatly reduces the build time
    razzleCmd = "buildconfig.exe /alter:SetupsBuild.MediaLayout.Cabbing.CompressLevel /value:none";
    _executeCmdInRazzle(razzleCmd, type);

    // this is required so that perf counters work properly (the pseudo-loc counters won't actually install)
    razzleCmd = "buildconfig.exe /alter:SetupsBuild.Localization.Translation /value:Full";
    _executeCmdInRazzle(razzleCmd, type);
}

function _getBuildBoxesDotDatFile() {
    return datFile = Env("_NTBINDIR") + "\\developer\\" + Env("USERNAME") + "\\BuildBoxes.dat";
}

// creates a buildboxes.dat file used to specify non-default locations for binaries
function _createBuildBoxesDotDat(type, binDir, setupRoot) {
    var datFile = _getBuildBoxesDotDatFile();
    var datFileDir = WshFSO.GetParentFolderName(datFile);
    
    if (!FSOFolderExists(datFileDir))
        FSOCreateFolder(datFileDir);

    datFile = FSOCreateTextFile(datFile, true, false);
    // note the hard-coded architecture, this is because the tool will properly infer all
    // paths when a non-default x86 path is specified.  this implies that the pathing for
    // other architectures is consistent with the x86 pathing.  it also means that we must
    // ensure that binDir points to x86 and not some other architecture.
    /*if (!binDir.match(/x86/i)) {
        throw new Error("");
    }*/
    
    datFile.WriteLine(Env("COMPUTERNAME") + ",x86," + type + "," + Env("_NTBINDIR") + "," + binDir + "," + setupRoot);
    datFile.Close();

    // note that setupbuild.exe REQUIRES that the value specified begins with 
    // an environment variable (ugh) so we need this whacky escaping
    return "^^%_NTBINDIR^^%\\developer\\%USERNAME%\\BuildBoxes.dat";
}

// executes the actual build
function _buildSkuList(skuList, type) {
    // we will perform the build from a razzle window
    // note that setupbuild must be executed from an x86 window regardless of what you're building
 
    // execute the build
    var razzleCmd = "msbuild " + skuList + " /fl \"/flp:logfile=" + SetupBuildLog + "\"";
    _executeCmdInRazzle(razzleCmd, type);
}

// check for local copies of the specified files.  this is an attempt to prevent people from building
// layouts that don't contain any local changes (which is perfectly legal but probably accidental)
function _checkForLocalFiles(architecture, type, binDir, failOnPause, skipFileCheck) {
    logMsg(LogClrAutomation, LogInfo, "Checking for locally compiled binaries\n");
    if (nsIsNullOrEmpty(binDir))
        binDir = BinRoot + "\\" + architecture + type;

    // add files to this array as required.
    var files = new Array("clr.dll");

    for (var index = 0; index < files.length; ++index) {
        var fileProbe = WshFSO.BuildPath(binDir, files[index]);
        logMsg(LogClrAutomation, LogInfo, "Checking for existence of " + fileProbe + "\n");
        if (!FSOFileExists(fileProbe)) {
            logMsg(LogClrAutomation, LogWarn, "No locally compiled files were detected.  Your layouts will build but will not contain any changes.\n");
            if (!skipFileCheck) {
                if (failOnPause) {
                    throw new Error("Unrecoverable warning, build failed!");
                }
                else {
                    logMsg(LogClrAutomation, LogWarn, "If this is intentional press ENTER to continue otherwise press CONTROL-BREAK and compile sources first.\n");
                    WScript.StdIn.Read(1);
                }
            }
        }
    }
}

// retrieves that Last Known Good remote store value as specified in CLRSetupInfo.bat
function _getLKGRemoteStore() {
    var setupInfo = SrcBase + "\\ndp\\clr\\src\\misc\\CLRSetupInfo.bat";
    var fileObj = FSOOpenTextFile(setupInfo, FSOForReading, false, FSOTristateFalse);
    var certifiedVersion = "";
    var dropPathString = "";

    while (!fileObj.AtEndOfStream) {
        var line = fileObj.ReadLine();

        if (line.match(/^\s*set\s+REMOTE_STORE_ROOT/i))
            dropPathString = line.match(/\\\\.*/)[0];
    }

    fileObj.Close();

    if (nsIsNullOrEmpty(dropPathString))
        throw new Error("Failed to find 'REMOTE_STORE_ROOT' in file '" + setupInfo + "', please contact your BFD.");

    return dropPathString;
}

// creates a cookie file that contains various settings used to configure the setup build environment
function _createBuildCookie(type, remoteStore) {
    var cookieFile = _getBuildCookieFileName(type);
    cookieFile = FSOCreateTextFile(cookieFile, false, false);
    cookieFile.WriteLine("REMOTE_STORE=" + remoteStore);
    cookieFile.Close();
}

// checks the build cookie file to see if any volatile settings have changed.
// this is mainly needed when the remote store changes since the current build tools
// aren't smart enough to figure this out on their own.
function _getBuildCookieState(type, remoteStore) {
    var cookieFile = _getBuildCookieFileName(type);

    if (!FSOFileExists(cookieFile))
        return NoCookie;

    cookieFile = FSOOpenTextFile(cookieFile, FSOForReading, false, FSOTristateFalse);

    var isStale = false;
    while (!cookieFile.AtEndOfStream) {
        var line = cookieFile.ReadLine();
        if (line.match(/^REMOTE_STORE/) && line.toLowerCase().indexOf(remoteStore.toLowerCase()) == -1) {
            isStale = true;
            break;
        }
    }
    cookieFile.Close();

    if (isStale)
        return StaleCookie;
    else
        return FreshCookie;
}

// generates the path and file name of the build cookie file
function _getBuildCookieFileName(type) {
    return _getSetupRootDirectory(type) + "\\" + CookieFileName;
}

// looks up the root directory specified in build.config where the build tools will create the layouts
var _cachedSetupRootDirectory = null;
function _getSetupRootDirectory(type) {
    if (_cachedSetupRootDirectory == null) {
        // get the root location of the layouts directory from the build.config file
        var buildConfig = BinRoot + "\\x86" + type + "\\build.config";

        if (!FSOFileExists(buildConfig))
            throw new Error("The config file '" + buildConfig + "' does not exist");

        var xmlDoc = new ActiveXObject("Msxml2.DOMDocument");
        xmlDoc.load(buildConfig);
        var setupsNode = xmlDoc.documentElement.selectSingleNode("BinariesBuild/Setups");

        if (setupsNode == null)
            throw new Error("XPath failed to find element 'BinariesBuild/Setups' in config file '" + buildConfig + "'");

        _cachedSetupRootDirectory = setupsNode.text;
    }

    return _cachedSetupRootDirectory;
}

// deletes various uninteresting directories and files that take a lot of space and cause confusion as to their purpose
function _cleanTemporarySetupFiles(sku, architecture, type) {
    var rootDir = _getSetupRootDirectory(type);
    var dirsToDelete = new Array();
    
    dirsToDelete.push(rootDir + "\\setup");
    dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\core");
    dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\coreall");
    dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\coreredist\\net");
    
    if (sku.match(/^full|all$/i)) {
        dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\extended");
        dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\extendedall");
        dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\fullall");
        dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\fullredist\\net");
    }
    
    dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\msuvista");
    dirsToDelete.push(rootDir + "\\layouts\\x86" + type + "\\enu\\netfx\\msuwin7");
    
    if (!architecture.match(/^x86$/i)) {
        dirsToDelete.push(rootDir + "\\layouts\\" + architecture + type + "\\enu\\netfx\\core");
        dirsToDelete.push(rootDir + "\\layouts\\" + architecture + type + "\\enu\\netfx\\coreredist\\net");
        dirsToDelete.push(rootDir + "\\layouts\\" + architecture + type + "\\enu\\netfx\\extended");
        dirsToDelete.push(rootDir + "\\layouts\\" + architecture + type + "\\enu\\netfx\\fullredist\\net");
        dirsToDelete.push(rootDir + "\\layouts\\" + architecture + type + "\\enu\\netfx\\msuvista");
        dirsToDelete.push(rootDir + "\\layouts\\" + architecture + type + "\\enu\\netfx\\msuwin7");
    }
    
    for (var index = 0; index < dirsToDelete.length; ++index) {
        var targetDir = dirsToDelete[index];
        logMsg(LogClrAutomation, LogInfo, "Cleaning up directory '" + targetDir + "'...\n");
        try {
            FSODeleteFolder(targetDir, true);
        }
        catch (e) {
            logMsg(LogClrAutomation, LogWarn, "Failed to delete the directory, the error message was: " + e.message + "\n");
        }
    }
}

// parses the setup build log in an attempt to determine why the build failed and provide
// some guidance on how to fix it such that subsequent builds will succeed.  we pass in the
// build settings used (remote store etc) to help in the analysis.
function _parseBuildLogForFailures(settings) {
    if (!FSOFileExists(SetupBuildLog)) {
        logMsg(LogClrAutomation, LogWarn, "No setup build log file was found!\n");
        return;
    }

    logMsg(LogClrAutomation, LogInfo, "Parsing setup build log file '" + SetupBuildLog + "' to try and determine why the build failed...\n");
    var fileObj = FSOOpenTextFile(SetupBuildLog, FSOForReading, false, FSOTristateFalse);
    var foundReason = false;

    while (!fileObj.AtEndOfStream) {
        var line = fileObj.ReadLine();

        if (_checkBuildLogForMissingFiles(line, settings.RemoteStore)) {
            foundReason = true;
            break;
        }
        else if (_checkBuildLogForCompilationError(line)) {
            foundReason = true;
            break;
        }
        else if (_checkBuildLogForGenericError(line)) {
            foundReason = true;
            break;
        }
    }

    fileObj.Close();

    if (!foundReason)
        logMsg(LogClrAutomation, LogWarn, "The build script was unable to determine the reason why your build failed.\n");
}

// checks if the build failed due to missing binaries.  this usually indicates that the specified
// remote store doesn't contain the non-CLR bits required for the build.  the typical case is when
// you're building CHK flavor and the remote store doesn't contain any CHK bits.  if this appears
// to be the case print out some diagnostics info and return true, else return false so we'll keep looking.
function _checkBuildLogForMissingFiles(line, remoteStore) {
    var result = false;
    
    if (line.match(/file not found/i)) {
        logMsg(LogClrAutomation, LogInfo, "Found: '" + line + "'\n");

        // ok we have 'file not found', now get the architecture/flavor
        // so we can check if it's missing from the remote store
        var binRootIndex = line.toLowerCase().indexOf(BinRoot.toLowerCase());
        if (binRootIndex > -1) {
            var startIndex = binRootIndex + BinRoot.length + 1;
            var endIndex = startIndex + 1;
            while (line.charAt(endIndex) != '\\' && endIndex < 1000) // picking some arbitrary number to prevent possible infinite loop
                ++endIndex;

            if (endIndex - startIndex < 20) { // this is to test for the case where the above while loop didn't exit normally
                var arch = line.substr(startIndex, endIndex - startIndex);
                logMsg(LogClrAutomation, LogInfo, "Checking if remote store contains a directory for 'binaries." + arch + "'\n");

                if (!FSOFolderExists(remoteStore + "\\binaries." + arch)) {
                    logMsg(LogClrAutomation, LogError, "The remote store '" + remoteStore + "' doesn't contain a directory for 'binaries." + arch + "' which is causing your build failure.\n");
                }
                else {
                    logMsg(LogClrAutomation, LogError, "The remote store contains directory 'binaries." + arch + "' however it appears that the raw drop is incomplete (either the is not complete or has been abandoned).\n");
                }
            }
            else {
                logMsg(LogClrAutomation, LogWarn, "The build script was not able to determine the exact nature of the missing files (probably a parsing bug in the script).\n");
                logMsg(LogClrAutomation, LogWarn, "The typical reason for this failure is that your remote store doesn't contain the architecture/flavor of binaries you need.\n");
                logMsg(LogClrAutomation, LogWarn, "Other reasons incldue the specified build was abandoned, is incomplete, or this is some transient network connection issue.\n");
            }

            logMsg(LogClrAutomation, LogError, "If you specified this remote store you'll need to specify a different one.\n");
            logMsg(LogClrAutomation, LogError, "If you didn't specify this remote store (you used the default) please notify your BFD so the LKG store can be updated.\n");
        }

        result = true;
    }

    return result;
}

// checks if the build failed due to Wix compilation errors.
// returns true if this is the cause of the build failure.
function _checkBuildLogForCompilationError(line) {
    var result = false;

    if (line.match(/error LGHT\d+/i)) {
        logMsg(LogClrAutomation, LogInfo, "Found: '" + line + "'\n");
        logMsg(LogClrAutomation, LogError, "This is a Wix compiler error.  If you have made changes to the referenced file please correct the error.\n");
        logMsg(LogClrAutomation, LogError, "If this is happening after an FI please contact your BFD to get an ETA when this issue will be fixed.\n");
        result = true;
    }

    return result;
}

// checks for generic 'error' message, no prescriptive advice
function _checkBuildLogForGenericError(line) {
    var result = false;

    if (line.match(/error:/i)) {
        logMsg(LogClrAutomation, LogInfo, "Found error message: '" + line + "'\n");
        logMsg(LogClrAutomation, LogError, "This falls into the generic bucket so there is no prescriptive advice for the build script to provide.\n");
        logMsg(LogClrAutomation, LogError, "Please follow up with your setup liason or BFD to fix this issue.\n");
        result = true;
    }

    return result;
}

// this will come back once we move to razzle
/*function _validateBinDir(binDir) {
    var ntBinDir = Env("_NTBINDIR").toLowerCase();
    if (ntBinDir.charAt(ntBinDir.length - 1) != "\\")
        ntBinDir += "\\";
        
    if (binDir.toLowerCase().indexOf() > -1) {
        // if the binaries dir is anywhere under the sources dir the build process will use the remote store
        // instead of the locally built binaries (this change is supposed to prevent the consumption of binaries
        // from the sources location).
        throw new Error("Your _NTTREE is under _NTBINDIR which is not supported, please reconfigure your build environment such that _NTTREE and _NTBINDIR are disjoint.");
    }
}*/

// helper for executing commands under razzle.
// setupbuild must be run from x86 razzle window
function _executeCmdInRazzle(cmd, type) {
    if (nsIsNullOrEmpty(type))
        type = Env("_BuildType");

    var baseRazzleCmd = "call " + SrcBase + "\\tools\\razzle x86 " + type + " no_oacr sepchar \\ binaries_dir " + BinRoot;

    // check if we're already executing from a razzle environment
    if (nsIsNullOrEmpty(Env("RazzleToolPath")))
        cmd = baseRazzleCmd + " exec " + cmd;

    logMsg(LogClrAutomation, LogInfo, "About to execute command '" + cmd + "'\n");

    if (VerboseSpew) {
        runCmdToLog(cmd,
            runSetTimeout(1 * HOUR,
            runSet32Bit()));
    }
    else {
        runCmd(cmd,
            runSetTimeout(1 * HOUR,
            runSet32Bit()));
    }
}

// Validate that at least one remote store in a remote store string is available.
// Input: remoteStoreString - a semi-colon delimited list of remote store paths
// Output: availableRemoteStores - the total number of accessible remote stores ie 0 for none, 1 or more depending on how many paths are available
function remoteStoresAvailable(remoteStoreString) {
    var remoteStorePart = remoteStoreString.split(';');
    var availableRemoteStores = 0;
    for (var key in remoteStorePart) {
        if(FSOFolderExists(remoteStorePart[key]))
            availableRemoteStores++;
    }
    return availableRemoteStores;
}
