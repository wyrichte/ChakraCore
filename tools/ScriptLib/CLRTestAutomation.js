/*********************************************************************************
 * ClrTestAutomation.js
 *********************************************************************************

 This file contains subroutines that automate installation, syncing
 and building of the test tree.
*/

var _CLRTest = logNewFacility("CLRTestAutomation");

if (WshShell == undefined)
	var WshShell = WScript.CreateObject("WScript.Shell");

if(Env == undefined)
    var Env = WshShell.Environment("PROCESS");

// A table of mappings between dev and test branches (for cases that they do not live together).
var devTestMappings = {
    "devdiv/private/clr_debugger": "devdiv/private/clr_debugger",
    "devdiv/pu/clr": "devdiv/feature/Silverlight_W2_CLR"
};

/*****************************************************************************/
/* 
The command install clrtest tree onto your machine. The command needs
to be invoked in the CLR Env windows. The script will automatically
detect what branch your CLREnv window is from and installs the
corresponding test tree to the machine. If your enlistment is in:
C:\vbl\puclr, the tests will be installed into c:\vbl\puclr\clrtest.

If the directory clrtest already exists under the dev tree root, the
function fails. You either have to remove the directory if you want to
reinstall the test tree fresh. Alternatively, if you already have the
test tree, you can just update it with command runjs testsSync.
*/
function testsInstallTests() 
{
    var testRoot = _FindTestRoot(true);
    var devBranchRoot = FSOGetFolder( testRoot + "\\..\\.." ).Path;

    // By convention we place the client root one level up
    if (!devBranchRoot.match(/(.*)\\([^\\]*$)/))
        throw new Error(1, "devBranchRoot '" + devBranchRoot + "' needs a backslash in it");
    var clientRoot = RegExp.$1;
    var prefix = RegExp.$2;
    var branchName = _GetDevBranchName();

    logMsg( _CLRTest, LogInfo, "Adding new mappings ...\n" );

    if( devTestMappings[branchName.toLowerCase()] != undefined ) {
        branchName = devTestMappings[branchName.toLowerCase()];
    }

    var depotRoot = "//depot/" + branchName;
    sdClientMap( clientRoot, depotRoot + "/qa/clr/testbin", prefix + "\\qa\\clr\\testbin" );
    sdClientMap( clientRoot, depotRoot + "/qa/clr/testsrc", prefix + "\\qa\\clr\\testsrc" );
    sdClientMap( clientRoot, depotRoot + "/qa/clr/env.tst", prefix + "\\qa\\clr\\env.tst" );
    sdClientMap( clientRoot, depotRoot + "/qa/clr/testteams", prefix + "\\qa\\clr\\testteams" );
    sdClientMap( clientRoot, "-" + depotRoot + "/qa/clr/testsrc/Desktop/AppCompat", prefix + "\\qa\\clr\\testsrc\\Desktop\\AppCompat" );
    sdClientMap( clientRoot, "-" + depotRoot + "/qa/clr/testbin/Desktop/AppCompat", prefix + "\\qa\\clr\\testbin\\Desktop\\AppCompat" );

    logMsg( _CLRTest, LogInfo, "Syncing the test tree...\n" );
    sdSync( testRoot, undefined, undefined, 1, true);
    logMsg( _CLRTest, LogInfo, "The test tree is synchronized. To build the tree, please use command \n" +
            "'runjs testsBuild\n" );
    return 0;
}


/*
The command installs the CLR product using clrsetup.bat file.  When
the installLocation argument is passed in, the command just invokes
%installLocation%\clrsetup.bat file. If the installLocation is
undefined, the script automatically finds the build drop location for
the branch of clrEnv window and installs the CLR.

This command is intended mainly for the test team so that testers do
not have to build their own CLR before compiling tests. Instead, they
just issue "runjs testsInsallCLR" followed by "runjs testsBuildWindow
x86"
*/
function testsInstallCLR(installLocation)
{
    if( installLocation==undefined )
    {
        var branchName = _GetDevBranchName();
        installLocation = _GetInstallRoot( branchName );
    }
    logMsg( _CLRTest, LogInfo, "Installing CLR from '" + installLocation + "' ...\n" );
    var runEnv = runSetTimeout(60*20, runSetStream());
    try {
        var res = runCmd( installLocation+"\\clrsetup.bat", runEnv );
    }
    catch(e)
    {
        logMsg(_CLRTest, LogError, "Install command failed.\n" );
        throw Error("Installation failed\n");
    }
    return 0;
}

/*
  This command is identical to calling:
     testsClean()
     testsBuild(buildArch, buildType, clrVersionDir, options, subtreeToBuild)

 */
function testsCleanAndBuild(buildArch, buildType, clrVersionDir, options, subtreeToBuild)
{
    logMsg( _CLRTest, LogInfo, "Cleaning the test tree ... \n");
    // @TODO kill all processes with "taskkill /f /im mspdbsrv.exe"
    runCmd("taskkill /f /im mspdbsrv.exe", runSetNoThrow());
    testsClean();

    logMsg( _CLRTest, LogInfo, "Starting to build test tree ... \n");
    testsBuild(buildArch ,buildType, clrVersionDir, options, subtreeToBuild);
    return 0;
}

function taskTestsCleanAndBuild(srcBase, buildArch, buildType, clrVersionDir, options, subtreeToBuild)
{
    WshShell.CurrentDirectory = srcBase;
    testsCleanAndBuild(buildArch, buildType, clrVersionDir, options, subtreeToBuild);
    return 0;
}

/*
The command builds the test tree. The command prepares the build
environment similarly as command testsBuildWindow but it also build
the test tree.

Arguments:
	buildArch:
	buildType:
	clrVersionDir:
	options:

The first four arguments are identical to the testsBuildWindow
command. See the command for details.

	subtreeeToBuild: 
    If left empty the testBuild command build the entire test
    tree. This operation takes fairly long time - building entire test
    tree can take up to 8 hours.  Therefore this options allow
    building only parts of the tree. The argument could be a name of
    any top-level subdirectory under clrtest\testsrc. (e.g. devsvcs).
    The script is smart to identify dependencies and build additional
    portions of the test tree required for a successful test run.
    
    buildOptions: optional arguments to pass to build.exe. (eg, "cC")
*/
function testsBuild(buildArch, buildType, clrVersionDir, options, subtreeToBuild, buildOptions)
{
    var testRoot = _FindTestRoot();
    var buildEnv = runSetStream(undefined, _testsGetBuildEnvironment(buildArch, buildType, clrVersionDir, options, testRoot));
    var dirs = _GetListOfDirectoriesToBuild( testRoot, subtreeToBuild );
    var dir;
    
    try
    {        
        var buildCmd = " build -mM";
        if (buildOptions != undefined)
        {
            buildCmd += buildOptions;
        }
        
        for(var i=0; i< dirs.length; ++i) {
            dir = testRoot + "\\testsrc\\" + dirs[i];
            
            
            logMsg(_CLRTest, LogInfo, "Building directory " + dir + " ...\n");
            
            runWithRuntimeArch( buildCmd , "x86", runSetLog( LogRun, LogInfo, runSetCwd(dir, buildEnv) ) );
        }
    }
    catch(e) {
        logMsg( _CLRTest, LogError, "Building test tree failed with " + e.Message + "\n" );
        logMsg( _CLRTest, LogError, "Please inspect build log files located in " + dir + "\n");
        
        // Use a build-report. 
        // Output file must be of form 
        //    .*build.*\\.bldrpt.html 
        // and in the log directory.
        var errFile = dir + "\\build.err";
        var reportFile = dir + "\\test.bldrpt.html";
        
        try { 
            buildReport(errFile, reportFile); 
            logMsg( _CLRTest, LogError, "\nA HTML report is at " + reportFile + "\n\n");
        }
        catch(e) 
        { 
            logMsg(LogClrAutomation, LogWarn, "testsBuild: could not build log report. ", e.description, "\n"); 
        }        
                

        
        
        throw e;
    }
    logMsg(_CLRTest, LogInfo, "Build complete.\n");
    return 0;
}

/*
The command creates a new window that can be used to build the
tests. The command also copies the compilers required for the test
environment.  Before tests can be built, a 32 bit version of the CLR
runtime needs to be installed with clrsetup batch file on the machine.

Arguments:
	buildArch:  x86, amd64, ia64 (needs to be specified)
	buildType:  ret, fre, chk, dbg (defaults to ret)
	clrVersionDir: a path to the 32bit CLR that will be used in the test 
                   build process
		e.g. c:\WINDOWS\Microsoft.NET\Framework\v2.0.x86ret
		If not specified, defaults to:
			%SystemRoot%\Microsoft.NET\Framework\%COMPLUS_DEFAULTVERSION%
	options: noInstall, forceInstall, smartInstall (defaults to "smartInstall")
		Controls how the command updates the c++ compilers used during the 
        test build.

        noInstall - won't install any new compilers, you are assumed to be
                    responsible for copying the compilers responsible for yourself.

        forceInstall - copies the compilers even though the compilers appear
                       to be already in the test tree.

        smartInstall - installs compilers only when they are not available 
                       or appear to be outdated. 

*/
function testsBuildWindow(buildArch, buildType, clrVersionDir, options)
{
    var buildEnv = _testsGetBuildEnvironment(buildArch, buildType, clrVersionDir, options, undefined);
    for( var varName in buildEnv._env ) {
        var varValue = buildEnv._env[varName];
        Env(varName) = varValue;
    }
    WshShell.CurrentDirectory = buildEnv._cwd;
    WshShell.Run("cmd.exe /k title " + buildEnv.Title);
    return 0;
}

/*
The command cleans the clrtest tree.  The command erases all files
that are not checked-in into the source depot. The command is
especially useful when you want to build completely fresh tree or when
you want to build a clrtest tree for a different processor
architecture than you did previously. The clrtest build system places
binaries to the testbin portion of the clrtest tree and the binaries
are always placed into the same location.  Therefore it recommended to
clean the test tree when you switch to different architecture.
*/
function testsClean()
{
    var testRoot = _FindTestRoot(false);

    // The tests clean command is repeated 3 times, just in case when the "sd have"
    // command, which is executed during sdClean fails with sd timeout...
    var counter = 0;
    var retry;
    do {
        ++counter;
        retry  =false;
        try {
            logMsg( _CLRTest, LogInfo, "Cleaning the test tree @ '" + testRoot + "'... \n" );
            sdClean( testRoot, undefined, "sd.ini", testRoot );
        } catch(e) {
            logMsg( _CLRTest, LogError, "cleaning the test tree failed");
            retry = true;
        }
    } while(retry && counter<3);
    return 0;
}

/*
The command brings your test tree up to date. Can be invoked anywhere
from your clrEnv window.
*/
function testsSync()
{
    var testRoot = _FindTestRoot(false);

    logMsg( _CLRTest, LogInfo, "Syncing the test tree @ '" + testRoot + "'... \n" );
    sdSync( testRoot, undefined, runSetCwd(testRoot), undefined, undefined );
    return 0;
}


// buildArch: x86, amd64, ia64
// buildType: ret, fre, chk, dbg
// clrVersionDir: full path to 32bit version directory
// options can be:
//      noInstall       - will not install compilers
//      smartInstall    - install compilers only if needed
//      forceInstall    - forces compiler installation
// testRoot: optional

function _testsGetBuildEnvironment(buildArch, buildType, clrVersionDir, options, testRoot)
{
    
    if( !options )
        options = "smartInstall";
    
    if( testRoot == undefined )
        testRoot = _FindTestRoot();

    if( buildType == undefined )
        buildType = "ret";

    if( buildArch == undefined )
    {
        buildArch = Env("_BuildArch");
        if( buildArch == undefined )
            throw Error("Unspecified build Architecture.");
    }

    // we always use cross-compilers....

    var envs = {};
    if( buildArch == "x86" ) {
        envs._TGTCPU="i386";    // NEEDED
        envs._TGTCPUTYPE="x86";
        envs._TGTOS="NT32";
    }
    else if( buildArch == "amd64" ) {
        envs._TGTCPU="AMD64";
        envs._TGTCPUTYPE="AMD64";
        envs._TGTOS="NT64";
        envs.BUILD_DEFAULT_TARGETS="-amd64";
        envs.NTAMD64DEFAULT="1";
        envs.AMD64="1";
        envs.cpu="amd64";
    }
    else if( buildArch == "ia64" ) {
        envs._TGTCPU="IA64";
        envs._TGTCPUTYPE="IA64";
        envs._TGTOS="NT64";
        envs.BUILD_DEFAULT_TARGETS="-ia64";
        envs.NTIA64DEFAULT="1";
        envs.IA64="1";
        envs.cpu="ia64";
    }
    else
        throw Error("Invalid build architecture '"+ buildArch + "'\n");

    if( clrVersionDir == undefined || clrVersionDir == "" )
    {
        // we will try to locate correct version directory...
        var verStr = Env("COMPLUS_DEFAULTVERSION");
        if( verStr=="" )
            throw Error("Do not know installed CLR version.\n" +
                        "Either set COMPLUS_DEFAULTVERSION or pass-in path to \n" +
                        "clrVersionDir\n");
        var v = Env("SystemRoot") + "\\Microsoft.NET\\Framework\\"+verStr;
        if( !FSOFolderExists(v) )
            throw Error("COMPLUS_DEFAULTVERSION points to an nonexistent CLR\n");
        clrVersionDir = v;
    }
    clrVersionDir = FSOGetFolder( clrVersionDir ).Path;
    var installedInfo = _Parse32NdpLogFile( clrVersionDir );
    envs._URT_VERSION= installedInfo.VersionString;

    envs.EXT_ROOT = clrVersionDir; // NEEDED
    envs.COMPLUS_DEFAULTVERSION = envs._URT_VERSION;
    envs.SDK_ROOT = envs.EXT_ROOT + "\\sdk\\"; // NEEDED
    envs.CORBASE  = testRoot;
    envs.DDROOT   = FSOGetFolder( testRoot + "\\..\\.." ).Path;
    envs.TESTBIN  = envs.CORBASE + "\\testbin";
    envs.TESTSRC  = envs.CORBASE + "\\testsrc";
    envs.BUILD_DEFAULT = "-mwe -nmake -i -a";
    envs.BUILD_OPTIONS = " DLLS";
    envs.NDPROOT  = envs.DDROOT + "\\ndp";
    envs.NTMAKEENV= envs.CORBASE + "\\env.tst";
    envs.PATH     = envs.CORBASE + "\\env.tst\\bin;" + Env("PATH");
    if( envs._TGTCPU != "i386" )
    {
        envs.PATH = envs.SDK_ROOT + "\\lib\\" + envs._TGTCPU + ";" + envs.PATH;

        // check if lib dirs exists, if not copy necessary files
        var libDir = envs.SDK_ROOT + "\\lib\\" + envs._TGTCPU;
        if( !FSOFolderExists( libDir ) ) {
            FSOCreateFolder( libDir );
            var from = installedInfo.SourcePath + "\\binaries." + envs._TGTCPU + "ret\\";
            var files = ["cordebug.tlb", "corguids.lib", "format.lib", "mscoree.lib", "mscorsn.lib"];
            for(var i=0; i< files.length; ++i) {
                FSOCopyFile( from + files[i], libDir + "\\" );
            }
        }
    }
    
    envs.INCLUDE  = envs.SDK_ROOT + "\\include;" + envs.EXT_ROOT;
    if( buildType == "fre" || buildType == "ret" )
    {
        envs.DDKBUILDENV="free";
    }
    else if( buildType == "chk" ) {
        envs.DDKBUILDENV="fastchecked";
        //envs.BUILD_ALT_DIR="df";
    }
    else if( buildType == "dbg" )
    {
        envs.DDKBUILDENV="checked";
        //envs.BUILD_ALT_DIR="d";
    }
    else
        throw Error("Invalid flavor type '" + buildType + "'. Valid options are ret, fre, chk, or dbg.\n");

    // HACK - HACK
    // the build environment now requires that a CoreCLR is present. We assume that the environment is installed in the root drive
    envs.CORE_ROOT = "\\CoreCLR";
    
    var runEnv = runSet32Bit( runSetTimeout( 60*60*10 ) ); // set time-out to 10 hours.....
    runEnv = runSetCwd( testRoot + "\\testsrc", runEnv );
    for( var key in envs )
        runEnv = runSetEnv( key, envs[key], runEnv );

    var shouldInstallCompilers = false;
    if( options == "noInstall" )
        shouldInstallCompilers = false;
    else if( options == "forceInstall" )
        shouldInstallCompilers = true;
    else if( options == "smartInstall" )
    {
        // @TODO improve the detection of outdated compilers
        shouldInstallCompilers = (FSOFileExists( testRoot + "\\env.tst\\Tools\\VC\\bin\\" + envs._TGTCPU + "\\cl.exe" ) ? false : true );
    }
    else
        throw Error( "Inavlid option '" + options + "'\n");

    if( shouldInstallCompilers )
    {

        var compilerPlatform = "";
        if( envs._TGTCPU != "i386" )
        {
            compilerPlatform = " /COMPILERPLATFORM x86_" + envs._TGTCPU + " ";
            installedInfo.BinariesPath = installedInfo.SourcePath + "binaries." + envs._TGTCPU + "ret\\";
        }

        // C:\vbl\ClrDbg\clrtest>cscript //nologo env.tst\CompilerScripts\CopyCompiler.js /SourcePath \\clrmain\public\drops\PUCLR\builds\ForClrSetup\60502.00\ /BINARIESPATH \\clrmain\public\drops\PUCLR\builds\ForClrSetup\60502.00\binaries.x86ret
        //var SourcePath = vars.CLR_DROP_PATH_HEAD + "\\" + vars.CLR_CERTIFIED_VERSION + "\\";
        //var BinaryBits = SourcePath + "binaries." + vars.CLR_BUILD_ARCH + "ret\\";

        // we provide bogus lab and version so that the script does not complain
        if( Env("_DEBUG_CopyCompiler") == "y" )
            runCmd("cmd /c start cmd",runEnv);

        runCmd("cscript //nologo ..\\env.tst\\CompilerScripts\\CopyCompiler.js /lab XX /v XX /sourcepath " + installedInfo.SourcePath + " /BINARIESPATH " + installedInfo.BinariesPath
               + compilerPlatform, runSetStream(undefined,runEnv));
    }

    runEnv.Title = "title CLR Test " + envs._TGTCPU + " - " + envs.DDKBUILDENV  + " - Complus=" + envs.EXT_ROOT;
    return runEnv;    
}

var DirectoriesToBuild = {};
DirectoriesToBuild.devsvcs = ["CoreCLRSDK\\devsvcs", "desktop\\hosting", "desktop\\devsvcs"];

var sharedDirectories = []; // list of directories that are alwasy built.

function _GetListOfDirectoriesToBuild(testRoot, tag)
{
    if( testRoot==undefined )
        throw Error("missing argument testRoot");
    
    
    // Sanity check that folder exists and print useful error. Many devs may not have the
    // test tree synced; so this may be a very common error.
    if (!FSOFolderExists(testRoot))
    {
        throw new Error("Test tree is not installed. Can't access folder " + dir + ". Run\n runjs testsInstallTests \n to install the test tree.");
    }
    
    var res;
    if( tag == undefined ) 
        return [""];
    else {
        var dirs = DirectoriesToBuild[tag.toLowerCase()];
        if( dirs == undefined )
        {
            var subdir = testRoot + "\\testsrc\\" + tag;
            if( FSOFolderExists( subdir ) )
            {
                res = new Array();
                for(i = 0; i < sharedDirectories.length; ++i)
                    res.push( sharedDirectories[i] );
                res.push( tag );
                return res;
            }
            else 
            {
                throw new Error("Only part of the test tree is installed. Can't access folder " + subdir + ". " +
                "add that folder to your sd client listing and sycn it. Or run\n   runjs testsInstallTests \nto install the full test tree.");                
            }
        }

        var res = new Array();
        for(i = 0; i < sharedDirectories.length; ++i)
            res.push( sharedDirectories[i] );
        for(i = 0; i < dirs.length; ++i)
            res.push( dirs[i] );
        return res;
    }
}


/* Parses a NpdSetup.log info from the version directory. The log contains
   information about location of non-runtime bits. Test tree needs this information
   when the environment is setting up and the script copies the c++ copilers.
*/

function _Parse32NdpLogFile(versionDir)
{
    var logFileLocation = versionDir + "\\NdpSetup.log";

    var ndpLog = FSOOpenTextFile(logFileLocation, 1);
    var result = {};
    while( !ndpLog.AtEndOfStream ) {
        var line = ndpLog.ReadLine();
        if( line.match( /^      NonNDP bits from: (.*)$/ ) )
        {
            result.BinariesPath = RegExp.$1;
            
            // Hack to work around caching. ClrSetup may cache some bits. NdpSetup.log will then
            // just give us the path to the Cache, which doesn't have all the bits we need. We
            // need to find the original path. 
            // So we reverse this mapping:
            // 	  c:\vbl\ClrDbg\binaries\clrSetupCache\60609.00	
            // to:
            //     \\clrmain\public\drops\PUCLR\builds\ForClrSetup\60609.00
            // This is fragile. Ideally, when ClrSetup does the cache, it would also drop some
            // file that tells us where the original source is; and then we can use that.
            if (result.BinariesPath)
            {
                var hackLocalPath = /^.:\\vbl\\\S+\\binaries\\clrSetupCache\\/;
                var hackGoodPath = "\\\\clrmain\\public\\drops\\PUCLR\\builds\\ForClrSetup\\";
                
                if (result.BinariesPath.match(hackLocalPath))
                {
                    result.BinariesPath = result.BinariesPath.replace(hackLocalPath, hackGoodPath);
                }
            }
            // End hack
            
            
            
            result.SourcePath   = result.BinariesPath.replace(/binaries.(.*)$/, "");
        }
        if( result.BinariesPath != undefined && result.SourcePath != undefined )
        {
            if( !versionDir.match(/microsoft\.net\\framework\\([^\\]*)/i) )
            {
                if( versionDir.match(/framework64/i) )
                    throw Error("Version Directory needs to be 32 bit version");
                
                throw Error("Version information cannot be extracted from the the versionDir\n");
            }
            result.VersionString = RegExp.$1;
            return result;
        }
    }
    throw Error("Could not parse log file at " + logFileLocation + "\n");
}

 
function _GetDevBranchName(devBranchRoot)
{
    if( devBranchRoot==undefined )
        devBranchRoot = FSOGetFolder( _FindTestRoot() + "\\..\\.." ).Path;

    var result = sdFileLog( devBranchRoot + "\\ndp\\clr\\src\\inc\\cor.h", devBranchRoot );
    if( ! result[0][0].depotFile.match( new RegExp("^//depot/(.*)/ndp/clr/src/inc/cor.h$","i" ) ) )
        throw Error("Cannot parse depot name " + result[0][0].depotFile + "\n" );
    var ret = RegExp.$1;
    return ret.toLowerCase();
}

var installRoots = {};
installRoots["devdiv/private/clr_debugger"] = ["\\\\dotnetdevelop\\drops\\Clr_Debugger", "DDRUN" ];
installRoots["devdiv/pu/clr"] = ["\\\\clrdrop\\drops\\CLRv3\\PUCLR\\Raw", "LAB_BUILD"];
    

function _GetInstallRoot(branchName) 
{
    if( branchName==undefined )
        branchName = _GetDevBranchName();
    
    var installRootInfo = installRoots[branchName];
    if( installRootInfo == undefined )
        throw Error( "Do not have information about install roots for branch '" + branchName + "'\n" +
                     "Either install the build manually or edit file CLRTestAutomation.js" );
    var buildType = Env("_BuildType");
    var buildArch = Env("_BuildArch");
    if( buildType==undefined || buildArch == undefined )
        throw Erorr( "Cannot find _BuildType or _BuildArch environment variables. Are you using dev window?\n");

    if( installRootInfo[1] == "LAB_BUILD" )
    {
        var dirs = FSOGetDirPattern( installRootInfo[0], /\d{4}\.\d{2}/ );
        // latest dir is the last in the array...
        for( var i= dirs.length-1; i>=0; --i )
        {
            var f = dirs[i] + "\\" + buildArch + buildType + "\\binaries";
            if( !FSOFileExists( f + "\\..\\build_complete.sem" ) )
                continue;
            return f;
        }
    }
    else if( installRootInfo[1] == "DDRUN" )
    {
        var dirs = FSOGetDirPattern( installRootInfo[0], /run.\d{2}-\d{2}-\d{2}_/ );
        // latest dir is the last in the array...
        for( var i= dirs.length-1; i>=0; --i )
        {
            var f = dirs[i] + "\\" + buildArch + buildType + "\\bin";
            if( !FSOFileExists( f + "\\clrsetup.bat" ) )
                continue;
            return f;
        }
    }
    else 
        throw Error("Invalid run type '" + installRootInfo[1] + "'\n");

    // we did not find any suitable builds.
    throw Error("Did not find any builds at '" + installRootInfo[0] + "'\n");
}
    


function _FindTestRoot(createTestRoot, pathToStartFrom)
{
    if( pathToStartFrom == undefined )
        pathToStartFrom = ".";
    
    var f = FSOGetFolder(pathToStartFrom);
    var path;
    do {

        if( f.IsRootFolder )
            throw Error( "Cannot find the root of the dev tree\n" );
        path = f.Path;
        logMsg(_CLRTest, LogInfo100000, "Traversing path " + path + "\n");
        // check if the path is a root folder for the test tree.
        if( FSOFileExists( path + "\\ndp\\clr\\src\\inc\\cor.h" ) )
            break;
        f = f.ParentFolder;
    } while( true );
    var qaclr = "\\qa\\clr";
    if( createTestRoot )
    {
        if( FSOFolderExists( path + qaclr ) )
            throw Error( "Tests directory already exists. Remove it first before repeating this operation\n" );
        if( !FSOFolderExists( path + "\\qa" ) )
            FSOCreateFolder( path + "\\qa" );
            
        FSOCreateFolder( path + qaclr );
    }
    else
    {
        if( !FSOFolderExists( path + qaclr ) )
            throw Error("Tests directory does not exist. Please use command 'runjs testsInstallTests'\n");
    }
    return path + qaclr;
}


/************************************************************************
 * Functions that support Merging the LstFiles.
 */


// function scans an .lst file and searches for tests that are disabled using INVALID keyword.
// The function returns a hash where key is the test guid and value is a bug that was used to
// disable the test.
function _ScanLstFileForDisabledTests(lstFileLines, cleanLstLines)
{
    var disabledTests = {};
    var totalLines = lstFileLines.length;
    for(var lineNo=0; lineNo < totalLines; ++lineNo ) 
    {
        var line = lstFileLines[lineNo];
        
        var testGuid;
        if( line.match(/^TestCommandLineGuid=\{(.*)\}/) )
        {
            testGuid = RegExp.$1;
        }
        if( line.match( /^Categories=/ ) )
        {
            if( line.match( /\;INVALID\\(.*)/ ) )
            {
                // this test is disabled using the protocol for CLR_Debugger branch
                // we need to store this information
                
                var invalidDescr = RegExp.$1;
                
                if( testGuid == undefined )
                    throw Error("Test disabled on missing test in line '" + line + "'");
                if( disabledTests[testGuid] != undefined )
                    throw Error("Test '" + testGuid + "' disabled on bugs: " + disabledTests[testGuid] + ", " + bugNumber);

                if( !invalidDescr.match(/^\d+$/) && !invalidDescr.match( /^REQUIRES\\.*/ ) )
                    throw Error("Wrong disabled pattern on line '" + line + "'");

                disabledTests[testGuid] = invalidDescr;
                testGuid = undefined;

                if( cleanLstLines )
                {
                    // we remove the INVALID string from the category.
                    line = line.replace( /\;INVALID\\(\d+)/, "" );
                    lstFileLines[lineNo] = line;
                }
            }
            else if( line.match( /INVALID/ ) )
                throw Error("Test disabled using INVALID category with unsupported syntax! - '" + line + "'");
        }
    }
    return disabledTests;
}

// _ApplyDisabledTestListToLstFile modifies the lines of .lst files
// passed in as argument lstFileLines so that the tests that
// correspond to testid's from disabledTests hash are properly
// disabled.
function _ApplyDisabledTestListToLstFile( lstFileLines, disabledTests)
{
    var totalLines = lstFileLines.length;
    for(var lineNo=0; lineNo < totalLines; ++lineNo) 
    {
        var line = lstFileLines[lineNo];

        var testGuid;
        if( line.match(/^TestCommandLineGuid=\{(.*)\}/) )
        {
            testGuid = RegExp.$1;
        }
        if( line.match( /^Categories=(.*)/ ) )
        {
            // this is categories line -- we should check if the test used to be
            // disabled in the original .lst file. If yes, we need to disable it
            // in the new file as well.
            if( testGuid == undefined )
                throw Error("Unmatched Categories entry in the .lst file in line '" + line + "'");

            if( disabledTests[testGuid] != undefined )
            {
                // we need to disable the test.
                line = line + ";INVALID\\"+disabledTests[testGuid] ;
                lstFileLines[lineNo] = line;
            }

            // there should be only one Category line per test.
            testGuid = undefined;
        }
    }
}

// _ReadLines reasds the file and returns the content of the file as a
// collection of lines.
function _ReadLines(file)
{
    var a = new Array();
    var f = FSOOpenTextFile(file,1);
    while( !f.AtEndOfStream )
        a.push( f.ReadLine() );
    f.Close();
    return a;
}

// _MergeLstFile scans targetLstFile for tests disabled with using the
// INVALID keyword, then it overrides the targetLstFile with
// sourceLstFile. Finally, the new list file is modified so that tests
// that were disabled in the original .lst file still stay disabled.
function _MergeLstFile(targetLstFile, sourceLstFile, changeNum)
{
    var lstFromLines = _ReadLines(sourceLstFile);
    var lstToLines = _ReadLines(targetLstFile);

    // scan through all disabled tests first
    var disabledTests = _ScanLstFileForDisabledTests( lstToLines );

    // disabledTests contains a list of disabled tests in the private branch
    // ..let's print the disabled tests.
    logMsg( _CLRTest, LogInfo, "Disabled tests in file " + targetLstFile + "...\n" );
    for(var test in disabledTests ) {
        logMsg( _CLRTest, LogInfo, "test: " + test + " - bug: " + disabledTests[test] + "\n");
    }

    _ApplyDisabledTestListToLstFile( lstFromLines, disabledTests );
    
    // sd edit the file
    if( changeNum == undefined )
        changeNum = sdChange("Test metadata (.lst files) FI");
    
    sdEdit( targetLstFile, changeNum );
    
    var f = FSOCreateTextFile(targetLstFile, true);
    for(var i=0; i< lstFromLines.length; ++i )
        f.WriteLine(lstFromLines[i]);
    f.Close();
    
    // completed!
    logMsg( _CLRTest, LogInfo, targetLstFile + " updated!\n" );
}

// the function prints tests that are disabled int the current .lst
// file but the bug on which they are disabled is closed. 
function _CheckForTestsToReenable(lstFile, operation, changeNum)
{
    var lstFileLines = _ReadLines(lstFile);
    var disabledTests = _ScanLstFileForDisabledTests(lstFileLines, true);

    var psHandle;
    var bugsClosedBugsQueryCache = {};
    function BugStatus(bugNumber) 
    {
        if( psHandle == undefined )
            psHandle = bugConnect("DevDiv Bugs");
        
        if( bugsClosedBugsQueryCache[bugNumber] != undefined )
            return bugsClosedBugsQueryCache[bugNumber];
        
        var bug;
        try {
            bug = bugGetById( psHandle, bugNumber );
            logMsg( _CLRTest, LogInfo, "bugGetById(" + bugNumber + ") = " + bugFieldGet(psHandle, bug, "Status") + "\n" );
            return bugsClosedBugsQueryCache[bugNumber] = bugFieldGet(psHandle, bug, "Status");
            bugsClosedBugsQueryCache[bugNumber] = bugFieldGet(psHandle, bug, "Status");
        } catch(e) {
            logMsg( _CLRTest, LogInfo, "Bug:"+bugNumber + " caused error:" + e.description + "\n");
            bugsClosedBugsQueryCache[bugNumber] = "Unknown";
        }
        return bugsClosedBugsQueryCache[bugNumber];
    }
    
               
    for( var test in disabledTests) {
        if( disabledTests[test].match(/REQUIRES/) )
        {
            // these tests are not reenabled with _ScanLstFileForDisabledTests
            disabledTests[test] = undefined;
            continue;           
        }
        
        if( BugStatus( disabledTests[test] ) == "Closed" )
        {
            logMsg( _CLRTest, LogInfo, "test " + test + " should be reenabled.\n" );
            disabledTests[test] = undefined;
        }
    }

    if( operation == "apply" )
    {
        _ApplyDisabledTestListToLstFile(lstFileLines, disabledTests);
        sdEdit( lstFile, changeNum );
        var f = FSOCreateTextFile(lstFile, true);
        for(var i=0; i< lstFileLines.length; ++i )
            f.WriteLine(lstFileLines[i]);
        f.Close();
    }
}

var lstFilesToUse = ["BVT_CheckinBVT.lst", "BVT_BuildBVT.lst", "BVT_SelfHostBVT.lst", "BVT_BranchSuite.lst" ];


// testsFILstFiles preforms an FI of .lst files into a private
// debugger branch.
// Arguments:
//     pathToLstFiles - path to a location of newly generated .lst
//     files (optional argument)
//     changeNum - a change list number to put the edited file
//     into. (optional argument)

function testsFILstFiles(pathToLstFiles, changeNum)
{
    if( pathToLstFiles == undefined )
    {
        var dirs = FSOGetDirPattern("\\\\clrdrop\\drops\\SilverLightW2CLR\\CLRTest", /^\d{5}\.\d{2}$/ );
        for(var i = dirs.length-1; i >= 0; --i) {
            pathToLstFiles = dirs[i] + "\\x86ret\\testbin";
            if( FSOFileExists( pathToLstFiles + "\\BVT_SelfHostBVT.lst" ) )
                break;
        }
    }
    if( !FSOFileExists( pathToLstFiles + "\\BVT_SelfHostBVT.lst" ) )
        throw Error("Could not locate .lst files in '" + pathToLstFiles + "'\n" );

    logMsg( _CLRTest, LogInfo, "FI'ing .lst files from: " + pathToLstFiles + "\n" );


    var testRoot = _FindTestRoot(false);

    if( changeNum == undefined )
        changeNum = sdChange("Test metadata (.lst files) FI");

    for( var i = 0; i< lstFilesToUse.length; ++i )
        _MergeLstFile( testRoot + "\\testbin\\" + lstFilesToUse[i], pathToLstFiles + "\\" + lstFilesToUse[i], changeNum );

    return 0;
}

function testsFI(pathToLstFiles, changeNum)
{
    if( changeNum == undefined )
        changeNum = sdChange("FI of test tTest metadata (.lst files) FI");
    
    runCmd("sd integrate -t -b private/CLR_Debugger -c "+ changeNum );
    testsFILstFiles(pathToLstFiles, changeNum);
    return 0;
}

    


function testsReenableTests( changeNum )
{
    if( changeNum == undefined )
        changeNum = sdChange("Test metadata (.lst files) - reenabling of disabled tests");

    var testRoot = _FindTestRoot(false);
    
    for( var i = 0; i< lstFilesToUse.length; ++i )
        _CheckForTestsToReenable( testRoot + "\\testbin\\" + lstFilesToUse[i], "apply", changeNum);

    return 0;
}
