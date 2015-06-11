/****************************************************************************/
/*                                ClrSelfHost.js                            */
/****************************************************************************/

/* stuff to enable running self-host */

var ClrSlefHostModuleDefined = 1;              // Indicate that this module exists

// Force an early error if these modules aren't defined yet.
if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!procModuleDefined) throw new Error(1, "Need to include proc.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");

var LogClrSelfHost = LogClrAutomation; // share Automation log setting


/****************************************************************************/ 
/*
    Get a default target directory (bvt_root). This will be where we copy 
    the tests too.
    Since the SelfHost binaries are platform specific, the returned dir
    is platform decorated. This allows both x86 and amd64 self-host tests
    to live sxs.
*/
function _getDefaultTargetDir(bldArch)
{
    if (bldArch == undefined)    
        bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    bldArch = bldArch.toLowerCase();

    var binDir = Env("_NTBINDIR");

    if (!nsIsNullOrEmpty(binDir))
    {
        // Stay out of the "src" tree
        return binDir + "\\..\\self_host_tests_" + bldArch;
    }

    binDir = Env("_TESTDIR");
    if (!nsIsNullOrEmpty(binDir))
    {
        return binDir + "\\self_host_tests_" + bldArch;
    }

    // HACK! This should be more configurable
    return "C:\\self_host_tests_" + bldArch;
}


/****************************************************************************/ 
/*
*/
function getURTTarget(bldArch, runtime)
{
    if (runtime == undefined)
        runtime = Env("COMPLUS_DEFAULTVERSION");

    if (bldArch == "x86")
        return Env("Windir") + "\\microsoft.net\\framework\\" + runtime;    
    else
        return Env("Windir") + "\\microsoft.net\\framework64\\" + runtime;    
}

/****************************************************************************/ 
/*
 This is a hack. 
 Many debugger tests  expect to NOT find symbols for Framework dlls. 
 This works great in the lab. But on dev machines, since we build the fx, the
 symbols are available. 
 This is a major hack, and the debugger needs to fix this (tests should be smart 
 enough to ignore symbols).
*/
function hideFXSymbols(bldArch, runtime)
{
    // Also hide _NT_SYMBOL_PATH since ClrEnv sets it to the symbols binary directory
    Env("_NT_SYMBOL_PATH") = "";

    // Also hide PERL5LIB since the dev environment points to the ones in _NTBINDIR (could be pointing to incorrect processor versions)
    Env("PERL5LIB") = "";

    var d = getURTTarget(bldArch, runtime);

    // & mkdir sym  | move sys*.pdb 
    var cmd = "pushd " + d + "  & rename mscorlib.pdb mscorlib_hideme.pdb & popd";

    var runOpts = clrRunTemplate;

    runOpts = runSetTimeout(60* 2, // number of seconds
        runSetNoThrow(runOpts));    

    logMsg(LogClrSelfHost, LogInfo, "Hiding fx symbols via:" + cmd + "\n");

    var run = runCmdToLog(
        cmd,
        runOpts);

    return run.exitCode;
}

/****************************************************************************/ 
/* 
    This is a hack to restore fx symbols hidden from hideFXSymbols.
*/
function restoreFXSymbols(bldArch, runtime)
{
    var d = getURTTarget(bldArch, runtime);
    
    var cmd = "pushd " + d + " & rename mscorlib_hideme.pdb mscorlib.pdb & popd";

    var runOpts = clrRunTemplate;

    logMsg(LogClrSelfHost, LogInfo, "Restoring FX symbols via:" + cmd + "\n");
    runOpts = runSetTimeout(60* 2, // number of seconds
        runSetNoThrow(runOpts));    
        
    var run = runCmdToLog(
        cmd,
        runOpts);

    return run.exitCode;        
}

/******************************************************************************/
/*********************** Certification    *************************************/
/******************************************************************************/


// Default share to keep the latest certified Self-Host list files.
var g_DefaultFailurDir = "\\\\bvtsrv\\Dump\\cert\\failures";


/****************************************************************************/ 
/*
    Sniff and delete old failure files    
*/
function deleteOldFailFiles()
{
}

/****************************************************************************/ 
/*
    Produce a failure file for a single failure and drop it on the share.
    This is a fast way of "invalidating" tests.
    
ex: 
  invalidateTest CERT_SELFHOSTBVT.LST=SELFHOSTBVT_4E75E159-E69C-44F9-A67B-EB80DBE5AA4C
*/ 
function invalidateSelfHostTest(szTestGuid)
{
    if (szTestGuid == undefined)
    {
        throw new Error("Expected 'szTestGuid' parameter");
    }
    szTestGuid = szTestGuid.replace(/cert_/i, "bvt_");
    
    // We need to create a fake ".fail.smrt" file and drop it on the certification share.
    // The file is a 1-line string that looks like:
    // BVT_SELFHOSTBVT.LST=SELFHOSTBVT_4E75E159-E69C-44F9-A67B-EB80DBE5AA4C, 0, 0, 0, 0, 0

    // Produce a log file in the cert directory.
    // Write a timestamp.
    if (!szTestGuid.match(/=(.*)$/))
    {
        throw new Error("Test guid in wrong form.");
    }
    var szStuff = RegExp.$1;

    var szContent = szTestGuid + ", 0, 0, 0, 0, 0";

    // Give it a well-decorated name.
    var szFilename = g_DefaultFailurDir + "\\private_" + Env("USERNAME") + "_" + szStuff + ".fail.smrt";
    FSOWriteToFile(szContent, szFilename);

    WScript.Echo("Invalidated test '" +  szTestGuid + "'");
    WScript.Echo("Wrote file:"  + szFilename);

    return 0;
}

/*
 Get a path to perl
*/
function _perlPath()
{
    // ScriptDir: $\ndp\clr\bin
    // Perl: $\tools\perl\bin\perl.exe    
    var perl = ScriptDir + "\\..\\..\\..\\tools\\perl\\bin\\perl.exe";
    return perl;
}

/****************************************************************************/ 
/*
    Produces a local set of Certified SelfHost list files from known failures.
    
    Reads a set of *.fail.smrt files describing known failures from the 
    szInputFailurePath directory and then produces certified .lst files
    to the szOutCertPath directory.
    
    Testers own "Churning Systems" which run the self-host tests and drop the 
    *.fail.*smrt files to the well known directory.
    You can also run "runjs invalidateSelfHostTest" to produce a failure for
    a given file.
    
Parameters:
    szInputFailurePath - (optional) the full path containing the *.fail.smrt files.
    szInputLstPath - (optional) the full path of the incoming .lst files to certify
    szOutCertPath - (optional) the full path of which to write the certified .lst files to.
*/

function makeLocalSelfHostCert(szInputFailurePath, szInputLstPath, szOutCertPath)
{
    if (szInputFailurePath == undefined)
        szInputFailurePath = g_DefaultFailurDir;

    if (szInputLstPath == undefined)
        szInputLstPath  = _getDefaultTargetDir(); // get .lst files from default test bin dir.
        
    if (szOutCertPath == undefined)
        szOutCertPath = _getDefaultTargetDir();


    var runOpts = clrRunTemplate;
    runOpts = runSetTimeout(60*10, // number of seconds
        runSetNoThrow(runOpts));    


    // Clean out old cert_*.lst files.
    runCmdToLog("del cert_*.lst", runOpts);

    // The perl script won't produce cert files if there are no failures. So copy core cert
    // files and then let the perl script overwrite them.
    // Also, Dos is stupid and can't handle copy with wild cards, so copy each individually.
    runCmdToLog("copy /y BVT_BuildBVT.lst    cert_BuildBVT.lst", runOpts);    
    runCmdToLog("copy /y BVT_CheckinBVT.lst  cert_CheckinBVT.lst", runOpts);    
    runCmdToLog("copy /y BVT_SelfHostBVT.lst cert_SelfHostBVT.lst", runOpts);    


    // The real certification is done by a perl script.
    var cmd = _perlPath() + " " +  ScriptDir + "\\scriptlib\\clrdbg\\CertListFiles.pl " + 
        szInputFailurePath + " " +
        szInputLstPath + " " +
        szOutCertPath;

    logMsg(LogClrSelfHost, LogInfo, "Begin local certification");
    var run = runCmdToLog(
        cmd,
        undefined, //  log to stdout
        runOpts);
       

    logMsg(LogClrSelfHost, LogInfo, "Done with local certification");

    // Produce a log file in the cert directory.
    // Write a timestamp.
    var szdTimestamp = "Produced certified files on " +  new Date() + "\r\n" +
        "from command:\r\n" + 
        cmd + "\r\n" +
        "Command return code=" + run.exitCode + "\r\n";
    
    FSOWriteToFile(szdTimestamp, szOutCertPath +  "\\Timestamp_Cert.log");    

    return run.exitCode;
}



/******************************************************************************/
/*********************** TEST ALIAS STUFF *************************************/
/******************************************************************************/

/****************************************************************************/ 
/*
We define a set of test "aliases". Each alias refers to which tests to copy
and which tests categories to include/exclude.
The aliases are effectively a shorthand way of generating smarty args.
*/


// Ctor to build an alias
// szName - pretty name of the alias. Eg "BST".
// arrayCopy - array of strings for subdirs in the test tree to copy
// szInclude - smarty include string (semicolon delimeter)
// szExclude - smarty include string (semicolon delimeter)
function _ctorAlias(szName, szInclude, szExclude, arrayCopy)
{
    this.szName = szName;
    this.szInclude = szInclude;

    if (szExclude == undefined) szExclude = null;
    this.szExclude = szExclude;

    if (arrayCopy == undefined) arrayCopy = null;
    this.arrayDirsToCopy = arrayCopy;
}

// Joins 2 strings with a ';'. String is either null or a real value (not undefined).
function _join(string1, string2)
{   
    if (string1 == null) return string2;
    if (string2 == null) return string1;
    return string1 + ";" + string2;
}

// Return a new alias that merged the 2 given ones.
function _mergeAlias(alias1, alias2)
{
    return new _ctorAlias(
        _join(alias1.szName,    alias2.szName), // pretty name
        _join(alias1.szInclude, alias2.szInclude), // include
        _join(alias1.szExclude, alias2.szExclude), // exclude
        null // when merging, just copy down all the subdirs
        );
}

// Helper to expand string category.
// We want to explicitly avoid including the "Combined" category because 
// that effectively breaks Devbvt because it asks it to include test targets that
// are likely not supported on the machine. 
// Thus use "Checkinbvt\XYZ;Buildbvt\XYZ;SelfHost\XYZ" instead of "Combined\XYZ".
// This helper function simplifies that textual transform.
// 
// It's ok to Exclude Combined
function _SelfHostCat(name)
{
    return "checkinbvt\\" + name + ";buildbvt\\" + name + ";selfhostbvt\\" + name;
}

// @todo - maybe this should be XML that we read in?
// Global list of aliases. See printTests for details
var g_AliasList = new Array();


/****************************************************************************/ 
/*
    Method to add a new alias to the global list of aliases.
    This list can be viewed via the printTests function.
    User config files can call this to add their own specialized test aliases.
*/
function AddAliasToGlobalList(alias)
{
    if (alias == undefined)
        throw new Error("AddAliasItem: Must specify 'alias' parameter");
    
    g_AliasList.push(alias);
}


// Debugger MDbg tests.
AddAliasToGlobalList(new _ctorAlias(
        "MDbg", 
        "COMBINED\\DevSvcs\\Tools\\MDbg", // include
        "COMBINED\\BaseServices", // exclude
        ["DevSvcs", "Hosting", "BaseServices"] // copy down these subdirs.
    ));

// Debugger tests - exclude profiler since although these are under the same category (devsvcs),
// they're separate tests + codebases, owned by separate dev teams.
// Exclude watson because that needs to be run single-threaded.
// When .lst files extended to allow specifying this, we can re-include watson.
AddAliasToGlobalList(new _ctorAlias(
        "Debugger", 
        _SelfHostCat("DevSvcs"), // include
        "COMBINED\\DevSvcs\\Profiler;COMBINED\\DevSvcs\\Stress\\Profiler;COMBINED\\DevSvcs\\Debugger\\Watson", // exclude
        ["DevSvcs", "Hosting", "BaseServices"] // copy down these subdirs
    ));

// Simple alias for 1 test which can be used to test our self-host infrastructure.
AddAliasToGlobalList(new _ctorAlias(
        "xyz",
        "SelfHostBVT\\BaseServices\\Exceptions\\Regressions\\WbyQFE\\587035",
        //"BuildBVT\\Jit\\Opt\\Perf\\CSE", //include
        //"checkinbvt\\security\\cas\\declarative\\bvt",  // include
        null, // don't exclude anything.
        ["Security\\CAS\\Declarative\\BVT"] // copy down these subdirs
    ));

AddAliasToGlobalList(new _ctorAlias(
        "BST", 
        _SelfHostCat("BaseServices"), // include
        "Combined\\ManagedServices" // exclude
    ));

AddAliasToGlobalList(new _ctorAlias(
        "JIT",
        _SelfHostCat("JIT"),  // include
        null, // no exclusions.
        ["JIT"] // only copy down these subdirs
    ));

AddAliasToGlobalList(new _ctorAlias(
        "LCG",
        "SelfHostBVT\\ManagedServices\\ReflectionEmit\\DynamicMethod",  // include
        null, // no exclusions
        ["ManagedServices\\ReflectionEmit\\DynamicMethod", "managedservices\\ReflectionEmit\\tools"] // only copy down these subdirs
    ));
    
AddAliasToGlobalList(new _ctorAlias(
        "NGEN",
        _SelfHostCat("Loader\\NGEN"),  // include categories
        null, // no exclusions
        ["Generics", "Loader\\Tools", "Loader\\Classloader",
         "Loader\\NGen", "JIT"] // There's a mouthful.
    ));
    
AddAliasToGlobalList(new _ctorAlias(
        "NGENService",
        _SelfHostCat("Loader\\NGEN\\Service") + ";" + _SelfHostCat("Loader\\NGEN\\OfflineQ"),  // include
        null, // no exclusions
        ["Loader"] // only copy down these subdirs
    ));
    
// Profiler tests.
AddAliasToGlobalList(new _ctorAlias(
        "Profiler", 
        "COMBINED\\DevSvcs\\Profiler", // include
        null, // no exclusions
        null // copy down these everything (mostly in devsvcs, but not all).
        ));

AddAliasToGlobalList(new _ctorAlias(
        "Reflection",
        "SelfHostBVT\\ManagedServices\\Reflection;SelfHostBVT\\ManagedServices\\ReflectionEmit",  // include
        null, // no exclusions
        null // copy everything
    ));

AddAliasToGlobalList(new _ctorAlias(
        "HostingAndThreading", 
        "SelfHostBVT\\Hosting\\SQL;SelfHostBVT\\BaseServices\\Threading", // include
        null, // no exclusions
        ["Hosting", "BaseServices", "DevSvcs", "ManagedServices"] // only copy down these subdirs
    ));

AddAliasToGlobalList(new _ctorAlias(
        "Security", 
        "SelfHostBVT\\Security;SelfHostBVT\\Security\PermissionElevation\\Hosting", // include
        "SelfHostBVT\\Security\\CodeDownload;SelfHostBVT\\Security\\Hosting\\IE;SelfHostBVT\\Security\\Hosting\\IEEverett;SelfHostBVT\\Security\\IsolatedStorage;SelfHostBVT\\Security\\PermissionElevation;SelfHostBVT\\Security\\Verifier", // exclusions
        ["AppDomains", "Hosting", "Security", "Verifier"] // only copy down these subdirs
    ));

AddAliasToGlobalList(new _ctorAlias(
        "APTCA",
        "BuildBVT\\Security\\CAS\\Declarative\\APTCA;BuildBVT\\Security\\CAS\\HostControlOfAPTCA;SelfHostBVT\\Security\\CAS\\Decllarative\\APTCA;SelfHostBVT\\Security\\CAS\\HostControlOfAPTCA;SelfHostBVT\\Security\\SEE\\V4Transparency\FullTrustAPTCA", // include
        null, // no exclusions
        ["Security"] // only copy down these subdirs
    ));        

AddAliasToGlobalList(new _ctorAlias(
        "All",
        "CheckinBVT;BuildBVT;SelfHostBVT",
        null,
        null
    ));

AddAliasToGlobalList(new _ctorAlias(
        "Interop", 
        "COMBINED\\ManagedServices\\Interop",  // include
        null, // no exclusions.
        null  // copy everything
    ));

AddAliasToGlobalList(new _ctorAlias(
        "NoPia", 
        "SelfHostBVT\\ManagedServices\\Interop\\NoPIA",  // include
        null, // no exclusions.
        ["ManagedServices\\Interop\\NoPIA"] // only copy down these subdirs
    ));

AddAliasToGlobalList(new _ctorAlias(
        "TypeSystem", 
        "COMBINED\\Loader\\Classloader;COMBINED\\Loader\\LowLevel",  // include
        "PrivateTest;LongRunningBVT", // exclude
        null // copy down everything
    ));

AddAliasToGlobalList(new _ctorAlias(
        "MetaData", 
        "COMBINED\\MetaData_FileFormats;COMBINED\\BaseServices\\ILASM_ILDASM",  // include
        "PrivateTest;LongRunningBVT", // exclude
        null // copy down everything
    ));

AddAliasToGlobalList(new _ctorAlias(
        "Verifier", 
        "COMBINED\\Verifier ", // include
        "LONGRUNNINGBVT\\verifier", // exclude
        ["Verifier", "Security"] // only copy down these subdirs
    ));

// To add a new alias, add a new entry to this array.



/****************************************************************************/ 
/*
    Return an alias object for the given name.
    throw exception if not matching alias, thus return value is guaranteed
      to be non-null.
*/

function _getAlias(szName)
{
    if (szName == undefined || szName == null)
        throw new Error("Unspecified test alias.");
        
    // Multiple aliases can be merged together via ';' char        
    
    var names = szName.split(";");        
    var alias = _getAliasAtom(names[0]);
    
    // Merge in any others
    for(var i = 1; i < names.length; i++)
    {
        alias = _mergeAlias(alias, _getAliasAtom(names[i]) );
    }
    
    return alias;
}

function _getAliasAtom(szName)
{
    // We can specify aliases dynamically at runtime if it begins with '$'. Look for constructed forms:
    // "$loader\shim" --> _SelfHostCat("loader\shim") --> "CheckinBVT\loader\shim;BuildBVT\loader\shim;SelfHost\loader\shim"
    if (szName.charAt(0) == "$")
    {        
        var cat = _SelfHostCat(szName.substr(1));        
        return new _ctorAlias(szName, cat);
    }

    // Search through well known list        
    var i;
    szName = szName.toLowerCase();
    for(i = 0; i < g_AliasList.length; i++)
    {
        var x = g_AliasList[i];
        if (x.szName.toLowerCase() == szName)
        {
            return x;
        }
    }

    throw new Error("Test alias '" + szName + "' is not defined. Use 'printTests' to see list of valid test aliases.");
}


function _printTestAlias(alias)
{
    // We explicitly print these to the screen, instead of just log them.
    WScript.Echo("Alias name:" + alias.szName);
    WScript.Echo("   Include:" + alias.szInclude);
    WScript.Echo("   Exclude:" + alias.szExclude);
    if (alias.arrayDirsToCopy == null) 
    {
        WScript.Echo("   Directories: copy whole test tree.");
    }
    else
    {
        WScript.Echo("   Directories:" + alias.arrayDirsToCopy.join(","));
    }
}

/****************************************************************************/ 
/*
    Print all test aliases to the console.
    szName - (optional). If specified, just prints this test alias. 
    Else prints all. Aliases are not case sensitive.
    
    'Test Aliases' are just words that map to smarty args. 
    - An alias can be a predined item from a globla list (populated by calls
      to AddAliasToGlobalList). Eg "BST" or "MDBG"
    - Or it can map directly to smarty args. If the first char is '$', then 
      the rest of the string is the category string.
     "$loader\shim" --> "CheckinBVT\loader\shim;BuildBVT\loader\shim;SelfHost\loader\shim"
    - aliases can also be combined with ';'.
        eg: "BST;MDBG" or "$loader\shim;BST"
    
    You can add new aliases to the global list by calling AddAliasToGlobalList() 
    from clrSelfHost.js.
*/
function printTests(szName)
{
    try {
        // 
        var alias = _getAlias(szName); // throws on error
        _printTestAlias(alias);
    }
    catch(e)
    {        
        // Print all
        for(var i = 0; i < g_AliasList.length; i++)
        {        
            WScript.Echo("---------------------------------------------------");            
            _printTestAlias(g_AliasList[i]);
        }

        if (szName != undefined)
        {
            WScript.Echo("No matches found for '" + szName + "'. Run 'runjs printTests' to see all possible test alias categories.");
        }
    }
    
    WScript.Echo("");
    WScript.Echo("'Test Aliases' are just words that map to smarty args.");
    WScript.Echo("You can add new aliases by editing g_AliasList in clrSelfHost.js.");
    return 0;
}


/******************************************************************************/
/*********************** RUNNING SELF HOST ************************************/
/******************************************************************************/

function valOrUnderscore(v)
{
    if (v == undefined)
    {
        return "_";
    }
    return v;
}
/****************************************************************************/ 
/*
    Helper to run selfhost on already-existing self-host tree.
    szTestAliasName - alias for test set to run. The alias will translate to smartyArgs
        Use printTests to see available aliases.
    targetDir - (optional) is the bvt_root. If excluded, it uses the 
                default targetDir from copySelfHostTests().
    outDir    : (optional) - for redirecting smarty output. Where to place the test results.
    ddsDir    : (optional) The base of the ddsSuites tree.
    bldType   : (optional) The Type of the build (chk, ret, ...)
    bldArch   : (optional) The architecture ("x86", "amd64", etc)
    runtime   : (optional) which runtime to use (COMPLUS_DefaultVersion)
    useMonitor: (optional) Run tests under monitor.wsf


    Useful tip for running selfhost on CoreCLR.  Copy down the test tree
    yourself (takes <10 min).  Then run:

    runjs runSelfHost szTestAliasName bvt_root _ _ corechk x86

    and the test drop is:

    \\clrdrop\drops\silverlight2\w2_clr\clrtest\<buildnum>\<flavor>\

Returns 0 on success, non-zero on failure.

*/
function runSelfHost(szTestAliasName, targetDir, outDir, ddsDir, bldType, bldArch, runtime, useMonitor)
{
    if (bldArch == undefined)
    {
        bldArch = "x86";
    }
    if (bldType == undefined)
    {
        bldType = "chk";
    }
    var verStr;
    if (runtime == undefined )
    {
        verStr = bldArch + bldType;
    }
    else
    {
        verStr = runtime;
    }
    var relOutDir = bldArch + bldType;
    var taskKind = "test.devBVT.SelfHost." + szTestAliasName.replace(/;/i, "_");
    var relOutDir = verStr + ".selfhost";
    var taskName = taskKind +  "@" + relOutDir;
    if (outDir == undefined)
    {
        outDir = "%outDir%\\" + relOutDir + "\\" + taskKind;
    }
    var task = taskNew( taskName,
                        "runjs _runSelfHost " + szTestAliasName + " " +
                        valOrUnderscore(targetDir) + " " +
                        outDir + " " +
                        valOrUnderscore(ddsDir) + " " +
                        bldType + " " +
                        bldArch + " " +
                        valOrUnderscore(runtime) + " " +
                        "_ " + // numRuntimes
                        valOrUnderscore(useMonitor),
                        undefined,
                        _machPatForArch(bldArch),
                        "Run selfhost tests." );
    _taskAdd(task);
    doRun(task.name);
}

/****************************************************************************/ 
/*
    Helper to run selfhost on already-existing self-host tree using the multi-runtime feature of smarty.
    szTestAliasName - alias for test set to run. The alias will translate to smartyArgs
        Use printTests to see available aliases.
    targetDir   : (optional) Is the bvt_root. If excluded, it uses the 
                  default targetDir from copySelfHostTests().
    outDir      : (optional) For redirecting smarty output. Where to place the test results.
    ddsDir      : (optional) The base of the ddsSuites tree.
    bldType     : (optional) The Type of the build (chk, ret, ...)
    bldArch     : (optional) The architecture ("x86", "amd64", etc)
    rtPrefix    : (optional) The runtime prefix.  You must have rtPrefix[0-numRuntimes] installed.
    numRuntimes : (optional) The number of runtimes to use for this run
    useMonitor  : (optional) Run the tests under monitor.wsf

Returns 0 on success, non-zero on failure.
*/
function runSelfHostMultiRuntime(szTestAliasName, targetDir, outDir, ddsDir, bldType, bldArch, rtPrefix, numRuntimes, useMonitor)
{
    //a few defaults here to match what you'd get from clrSetupN
    if (numRuntimes == undefined)
        numRuntimes = Env("NUMBER_OF_PROCESSORS");

    if (bldArch == undefined)
        bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (bldType == undefined)
        bldType = "chk";

    if (rtPrefix == undefined)
        rtPrefix = getRuntimeVersionDir(bldArch + bldType) + "_worker";
     
    bldArch = bldArch.toLowerCase();
    _runSelfHost(szTestAliasName, targetDir, outDir, ddsDir, bldType, bldArch, rtPrefix, numRuntimes,
                 useMonitor);
}

/* Internal worker function to wrap the handling of multiple runtimes */
function _runSelfHost(szTestAliasName, targetDir, outDir, ddsDir, bldType, bldArch, runtime, numRuntimes,
                      useMonitor)
{
    if (targetDir == undefined)
        targetDir = _getDefaultTargetDir(bldArch);
    if (szTestAliasName == undefined)
        szTestAliasName = "Mdbg";

    if (ddsDir == undefined) {
        ddsDir = Env("_NTBINDIR") + "\\ddsuites";
        if (!Env("_NTBINDIR"))
            throw new Error(1, "runSelfHost: Required argument ddsDir is missing");
    }
    if (bldArch == undefined)
        bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (bldType == undefined)
        bldType = "chk";

    if (runtime == undefined)    
    {
        var verStr = bldArch + bldType;
        runtime = getRuntimeVersionDir(verStr);
    }
    if (useMonitor == undefined)
    {
        useMonitor = true;
    }
    
    bldArch = bldArch.toLowerCase();

    var alias = _getAlias(szTestAliasName);


    var inc = alias.szInclude;
    var exc = alias.szExclude;
    

    // Big hack for debugger tests...
    hideFXSymbols(bldArch, runtime);

    var bvt_root = targetDir;
    var ret = runSelfHostTestsWorker(bvt_root, inc, outDir, exc, ddsDir, bldType, bldArch, runtime, numRuntimes, useMonitor);    

    // Big hack for debugger tests...
    restoreFXSymbols(bldArch, runtime);

    return ret;

}

/*
    We can't have semicolons on the command line because cmd.exe thinks of it as a delimieter.
    And we can't escape semicolons either and we can't quote them because cmd.exe is stupid.
    We use semicolons in smarty category strings. So we transform the string to avoid semicolons.

    In:  /inc A;B;C
    Out: /inc A /inc+ B /inc+ C

    parameters
     input - the initial category input string (such as "A;B;C")
     sFirst - the command line switch for the first part (either "/inc" or "/exc")
     sRest - the command line switch to append categories (either "/inc+" or "/exc+")
     
*/
function _escapeCategoryString(input, sFirst, sRest)
{
    if (input == null || input == undefined)
        return "";

        
    var output = " ";
    var a = input.split(";");

    output += (sFirst + " " + a[0]);

    for(var i = 1; i < a.length; i++)
    {
        output += (" " + sRest + " " + a[i]);
    }
    return output;
}

/****************************************************************************/ 
/*
    Helper to invoke smarty to run an arbitrary set of selfHost tests.
    
    You could always go to the bvt_root, and invoke devbvt.bat directly with
    your favorite set of command line args.
    
    bvt_root - (optional). root. This is where smarty.bat will be.
             if excluded, this is assumed to be the same root to where 
             copySelfHostTests() used as default.
    inc - smarty include categories
    outDir - output dir for smarty.
    exc - smarty exclude categories.
    runtime - if numRuntimes == 1, this is the runtime to use.  If numRuntimes > 1, then this is the runtime
             prefix for smarty.
    numRuntimes - Number of runtimes to use for smarty.
    useMonitor - Run tests under monitor.wsf.

    ddsDir, bldType, bldArch, runtime - parameters we'll pass on to smartyArgs
Returns 0 on success, non-zero on failure.    
*/
function runSelfHostTestsWorker(bvt_root, inc, outDir, exc, ddsDir, bldType, bldArch, runtime, numRuntimes,
                                useMonitor)
{
    if (bvt_root == undefined)
    {
        bvt_root = _getDefaultTargetDir(bldArch);
    }
    if (ddsDir == undefined)
    {
        ddsDir = bvt_root;
    }

    if (inc == null) { inc = undefined; }
    if (exc == null) { exc = undefined; }

    var all_include_lst = "TESTS.lst";
    
    var smartyArgs = "/lst " + all_include_lst +
//        ((inc != undefined) ? (" /inc \"" + inc + "\"") : "") +
//        ((exc != undefined) ? (" /exc \"" + exc + "\"") : "") +
        _escapeCategoryString(inc, "/inc", "/inc+") + 
        _escapeCategoryString(exc, "/exc", "/exc+") + 
        " /clean";     

    if (Env("NUMBER_OF_PROCESSORS") > 1)
    {
        smartyArgs += " /workers:" + Env("NUMBER_OF_PROCESSORS");
    }
    if (numRuntimes > 1)
    {
        smartyArgs += " /runtimes:" + numRuntimes + " /runtimePrefix:" + runtime;
        //lie to the rest of the script.  We always set defaultversion to be "runtime 0".
        runtime = runtime + "0";
    }

    //Baselining is now on full time.  Do not specify /fileBugs for now.
    try
    {
        var srcBase = srcBaseFromScript();
        smartyArgs += " /baselineSrc:" + srcBase;
    }
    catch(e)
    {
        logMsg(LogClrSelfHost, LogInfo, "runSelfHostTestsWorker could not determine source base\n" );
    }

    if( useMonitor )
    {
        smartyArgs += " /ldr " + ScriptDir + "\\MonitorTestLdr.bat";
    }
    
    // temporary hack till SmartyMetaData.pm in the test drop is fixed to match the .lst files.
    if (isCoreCLRBuild(bldType))
    {
        smartyArgs += " /ttBT CoreCLR"; 
    }
    else
    {
        smartyArgs += " /ttBT FrameworkFull"; 
    }

    smartyArgs += " /noie";
    // Of course, this is a hack - the clean way to do this would be to send lab as 
    // an arg to both the copy and the run tasks.
    
    //We want to run the right number of tests too.
    //We do, but if you run this through ddsenv, it gets messed up.  Rather than bother putting it in here,
    //I'll only add it _smartyworker so I can skip it for bvt runs.
    //smartyArgs += " /ds Enabled;ConfirmFixed";

    if (outDir != undefined)
    {
        // If we're overriding the output dir, then assume they they're likely using a taskReport and
        // don't care about the Smarty report specifically.
        smartyArgs += " /noie";
    }   
    else
    {
        outDir = bvt_root + "\\smarty.run.0";
    }

    // Ensure outDir exists and translate it to a full path.
    FSOCreatePath(outDir);
    outDir = FSOGetFolder(outDir).Path;

    return _smartyWorker(smartyArgs, outDir, ddsDir, bvt_root, bldType, bldArch, runtime, "selfhost");
}


/******************************************************************************/
/*********************** COPYING SELF HOST ************************************/
/******************************************************************************/

/****************************************************************************/ 
/*
    Given a test share, find the most recent build containing the given flavor.
    - testRoot - directory who's subdirs are the build numbers. Different labs
        have different test roots.
    - flavor to search for. (eg, "x86ret", "amd64ret")
    Returns build number (eg, "41115.00") which can be used to construct a 
        directory name. 
    Or null if none found

*/
function findLatestTestBin(testRoot, flavor)
{
    if (flavor == undefined)
    {
        flavor = "x86ret";
    }
    
    var folder = FSOGetFolder(testRoot);

    var szLatest = null
    
    for (var fileEnum = new Enumerator(folder.SubFolders); !fileEnum.atEnd(); fileEnum.moveNext()) 
    {    
        var name = fileEnum.item().Name;

        // Build numbers are of the form "YMMDD.XX", so they're conveniently sorted alphabetically.
        // So we can scan through the whole dir and just take the highest value.
        if (name.match(/\d{5}.\d{2}/))
        {
            if ((szLatest == null) || (name > szLatest))
            {
                // The build lab uses a "CLRTest_Complete.sem" file as a semaphore to ensure the testbin folder has been fully copied up.
                // We need to check this to prevent us from picking a partially created TestBin folder.
                // This check also ensures that the given flavor exists.
                var binroot = testRoot + name + "\\" + flavor;
                if (FSOFileExists(binroot + "\\CLRTest_Complete.sem"))
                {
                    // Remember
                    szLatest = name;
                }
            }            
        }
    }
    return szLatest;
}

/****************************************************************************/ 
/*
    Get a testroot dir for a given lab (eg "lab22dev").
    returns a string for a fully qualified directory share for the given test root.
    Throws exception if no such root is found for the given lab.
*/
function getTestRootForLab(lab)
{
    if (lab == undefined)
    {
        lab = _getBranchFromSrcBase(srcBaseFromScript(), true);
    }    

    var testRoot = null;
    if (lab == "clr_next" || lab == "Dev10_feature_clr_next")
    {
        testRoot = "\\\\clrdrop\\drops\\Feature\\CLR_Next\\CLRTest\\";
    }
    else if (lab == "lab21s") 
    {
        testRoot = "\\\\URTDist\\CLRDev\\Drops\\Whidbey\\LAB21S\\CLRTest\\";
    }
    else if (lab == "sl_w2_clr")
    {
        testroot = "\\\\clrdrop\\drops\\Silverlight2\\w2_CLR\\CLRTest\\";
    }
    else
    {
        // Whidbey used to be the default. Not anymore.
        // testRoot = "\\\\cpvsbuild\\drops\\Whidbey\\" + lab + "\\CLRTest\\";

        testRoot = "\\\\clrdrop\\drops\\Dev11\\PUCLR\\CLRTest\\";
    }

    if (!FSOFolderExists(testRoot))
    {
        throw new Error("Test root folder : '" + testRoot + "' does not exist.");
    }
    
    return testRoot;
}

/****************************************************************************/ 
/*
Get the TestBin directory for a given build number / flavor / lab
    buildNum - build number (eg, "41115") to get. Default will be the latest.
    flavor - (Optional) Flavor to get (default is "x86fre"/"amd64fre" for lab21S and x86ret/amd64ret for puclr). 
    lab - which lab to copy from? Default is "puclr".

Thus if all parameters are omitted, the default will be the latest puclr tests.
*/
function getTestBinDir(buildNum, flavor, lab)
{
    if (lab == undefined)
    {
        lab = _getBranchFromSrcBase(srcBaseFromScript(), true);
    }

    if (flavor == undefined)
    {
        // get flavor based on processor arch and lab: x64 => amd64***, x86 => x86***
        // lab21S uses fre builds and puclr uses ret.
        var bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();
        var testBldType = null;

        if (lab == "lab21s") 
        {
            testBldType = "fre";
        }
        else 
        {
            testBldType = "ret";
        }
        flavor = bldArch + testBldType;
        
    }


    var buildDir = null;    
    if (buildNum != undefined)
    {
        // @todo - this seems a little inconsistent.
        buildDir = "" + buildNum + ".00";
    }

    // Map arm - > eva
    flavor = flavor.replace(/arm/i, "eva");

    // @todo - check that it's a valid flavor? This will be implicitly checked when we look for the test root.

    logMsg(LogClrSelfHost, LogInfo, "Finding testbin dir for:" + buildNum + "," + flavor + "," + lab +"\n");  

    var testRoot = getTestRootForLab(lab); 

   
    // If no dir, pick the latest
    if (buildDir == null)
    {
        buildDir = findLatestTestBin(testRoot, flavor);
        if (buildDir == null)
        {
            throw new Error("Can't find valid testbin dir for flavor '" + flavor + "' in test root:" + testRoot);
        }
    }
    
    var binpath = testRoot + buildDir + "\\" + flavor + "\\TestBin";

    if (!FSOFolderExists(binpath))
    {
        throw new Error("Expected testbin folder does not exist:" + binpath);
    }

    return binpath;
}

/****************************************************************************/ 
/*
    Copy down tests from a file share. This uses robocopy, which can do an 
    incremental copy. So a full test copy may take an hour, but a second copy
    may only take seconds.

    szTestAliasName - (optional) name of alias set to copy. If excluded, copies
       all tests
    arch - "x86" or "amd64", etc.

    Just a wrapper for copySelfHostTests providing defaults for srcTestBinDir
    and targetDir based on the given arch
*/
function copySelfHostTestsForArch(szTestAliasName, arch)
{
    return copySelfHostTests(szTestAliasName, getTestBinDir(undefined, arch + "ret", undefined), 
        _getDefaultTargetDir(arch));
}


/****************************************************************************/ 
/*
    Copy down tests from a file share. This uses robocopy, which can do an 
    incremental copy. So a full test copy may take an hour, but a second copy
    may only take seconds.

    szTestAliasName - (optional) name of alias set to copy. If excluded, copies
       all tests
    srcTestBinDir - src dir to copy tests from. 
        eg: \\clrdrop\drops\CLRv3\PUCLR\CLRTest\60123.00\x86fre\testbin
        This can also be gotten via helper functions, eg: getTestBinDir()
    targetDir - (optional) local target directory to copy tests too.
*/
function copySelfHostTests(szTestAliasName, srcTestBinDir, targetDir)
{    
    if (srcTestBinDir == undefined)
        srcTestBinDir = getTestBinDir(); // get latest puclr bits
       
    if (targetDir == undefined)
        targetDir = _getDefaultTargetDir();

    var alias = null; // An alias can tell us to only copy a subset of the dirs.
    if (szTestAliasName != undefined)
    {
        try
        {
            alias = _getAlias(szTestAliasName); // throws on mismatch
        }
        catch(e)
        {
            // If we can't find the alias, just default to copying all the tests.
            logMsg(LogClrSelfHost, LogInfo, "Warning! Can't find alias '"  + szTestAliasName + "', so copying all tests.\n");
        }
    }
    
    //
    // Now we have source + target, do the copy.
    //    
    if (!FSOFolderExists(srcTestBinDir))
    {
        throw new Error("Test folder: '" + srcTestBinDir + "' does not exist.");
    }

    logMsg(LogClrSelfHost, LogInfo, "Copying tests from: "  + srcTestBinDir + "\n");
    logMsg(LogClrSelfHost, LogInfo, "Copying to: "+ targetDir + "\n");



    // Don't worry about deleting because /PURGE will do that for us.
    // Else we could use FSOAtomicDeleteFolder before copy to make sure it's clean.


    // If no subdirs are specified in the alias, then just copy the whole tree
    if ((alias == null) || (alias.arrayDirsToCopy == null) || (alias.arrayDirsToCopy == undefined))
    {
        robocopy(srcTestBinDir, targetDir, "/PURGE", targetDir + "\\copy.log");
    }
    else
    {
        // We just copy the toplevel set plus a subset of the folders.
        runCmdToLog("xcopy /Y /I "+ srcTestBinDir + "\\* " + targetDir, runSetNoThrow());


        // Copy subdirs that all tests will need.
        _CopyTestSubDir(srcTestBinDir, targetDir, "Common");
        _CopyTestSubDir(srcTestBinDir, targetDir, "ProductionTools");
        _CopyTestSubDir(srcTestBinDir, targetDir, "Tools");
        _CopyTestSubDir(srcTestBinDir, targetDir, "CoreCLRTestLibrary");

        // Copy sub dirs specific for our tests
        if (alias != null)
        {
            var i;
            for(i = 0; i < alias.arrayDirsToCopy.length; i++)
            {        
                _CopyTestSubDir(srcTestBinDir, targetDir, alias.arrayDirsToCopy[i]);
            }
        }
    }    


    // Write a timestamp. Do this at the end because the roboCopy() will delete any foriegn files.
    FSOCreatePath(targetDir);
    var szdTimestamp = "Copy tests from:" + srcTestBinDir + "\r\n";
    FSOWriteToFile(szdTimestamp, targetDir +  "\\Timestamp.log");

    // Now that we've copied the files, produce local cert files.
    //  makeLocalSelfHostCert();
    
    return 0;
}

// Internal helper to copy 1 sub dir.
function _CopyTestSubDir(srcTestBinDir, targetDir, subDir)
{    
    robocopy(srcTestBinDir + "\\" + subDir, targetDir + "\\" + subDir, "/PURGE", targetDir + "\\copy." + subDir.replace(/\\/g, "_") + ".log");
}


/******************************************************************************/
/*********************** TASKS FOR SELF HOST **********************************/
/******************************************************************************/


// Add task to run self host for given alias.
// szAliasName - name of the given test alias.
// Returns an object whos feilds are set to the new tasks we've created.
// This lets callers easily pick out the task they want.
function _createSelfHostTaskFromAlias(szAliasName, bldArch, bldType, copyTests, dependOnSetup, useMultiWorker)
{
    if (bldType == undefined)    
        bldType = "chk";

    if (bldArch == undefined)
        bldArch = Env("PROCESSOR_ARCHITECTURE").toLowerCase();

    if (copyTests == undefined)
        copyTests = true;

    if (dependOnSetup == undefined)
        dependOnSetup = true;

    if (useMultiWorker == undefined)
        useMultiWorker = false;

    var alias = _getAlias(szAliasName); // throws on mismatch
    
    // An alias name can be arbitrarily complex, so use it as a pretty name, but not in tasks.
    var szName = alias.szName;
    var taskName = szName.replace(";", "_");
    
    bldArch = bldArch.toLowerCase();

    var verStr = bldArch + bldType;

    // Don't write out to "x86chk" because that will overwrite DDR's devbvt smarty file already in x86chk.
    var relOutDir = verStr + ".selfhost";

    var taskCopy = taskNew(
        "copySelfHost." + taskName + "@" + relOutDir,
        "runjs copySelfHostTestsForArch " + szName + " " + bldArch,
        undefined,
        _machPatForArch(bldArch), // We're screwed if this doesn't pick the same machine to run the tests
        "Copy self host tests (for test alias '" + szName + "') from public drop to local drive.");

    // These are the default arguments for DDR, since they are *NOT* captured in the task name
    // during a DDR run, these arguments will probably be ignored in favor of the DDR
    // arguments
    var taskSetup = _clrSetupTask(bldType, bldArch, undefined, "/fx /nrad");

    
    // Inlclude "test.devBVT" so that taskReport looks for smarty.
    // This tell it to include the smarty report in the task output report.
    var taskKind = "test.devBVT.SelfHost." + taskName;
    var taskName = taskKind + "@" + relOutDir;
    var logDir = "%outDir%\\" + relOutDir + "\\" + taskKind;

    var verDir = getRuntimeVersionDir(verStr);

    var taskDependencies;
    if (useMultiWorker)
    {
        taskDependencies = [
            taskNew("clrSetupN." + verStr,
                    "runjs clrSetupN " + Env("NUMBER_OF_PROCESSORS") + " %outDir%\\" + verStr,
                    undefined,
                    _machPatForArch(bldArch), // We're screwed if this doesn't pick the same machine to run the tests
                    "Installs multiple runtimes (1 per processor)",
                    ""),
            taskCopy ];

        taskName = taskName + "_MW";

    }
    else if (copyTests && dependOnSetup)
    {
        taskDependencies = [ taskCopy, taskSetup];
    }
    else if (dependOnSetup)
    {
        taskDependencies = [ taskSetup];
        taskName = taskName + "_NoCopy";
    }
    else if (copyTests)
    {
        taskDependencies = [ taskCopy];
        taskName = taskName + "_NoSetup";
    }
    else
    {
        taskName = taskName + "_NoDeps";
    }
    
    var copyAndRun = _createSelfHostTask(szName, taskName, logDir, "%srcBase%\\ddsuites", bldType, bldArch,
                                           verDir, taskDependencies);
    return copyAndRun;
}


// Add task to run self host for given alias.
// szAliasName - name of the given test alias.
// Returns an object whos feilds are set to the new tasks we've created.
// This lets callers easily pick out the task they want.
function _AddDDRSelfHostTask(szAliasName, useMultiWorker, bldArch, bldType)
{

    var tasks= { };

    tasks.copyAndRun = _createSelfHostTaskFromAlias(szAliasName, bldArch, bldType, true, true, useMultiWorker);

    _taskAdd(tasks.copyAndRun);    

    tasks.DDR = taskGroup("DDR_and_Selfhost",
        [
            _taskDailyDevRun, // global imported from global task list.
            tasks.copyAndRun
        ]);

    _taskAdd(tasks.DDR);

    return tasks;
}

function _createSelfHostTask(szName, taskName, logDir, bvtRoot, bldType, bldArch, verDir, dependencies)
{
    return taskNew( taskName, 
                    "runjs _runSelfHost " 
                    + szName
                    + " _ "
                    + logDir + " "
                    + bvtRoot + " "
                    + bldType + " "
                    + bldArch + " "
                    + verDir
                    ,
                    dependencies,  // undefined, // dependents.
                    _machPatForArch(bldArch),
                    "Run the Self host tests for '" + szName + "'. The SelfHost tests are a larger (~10x) set of tests than just DevBvt.");
}


/****************************************************************************/ 
/*
    Daily Dev run w/ self-host for the given alias string at the end.
    This will copy down the latest self host tests and run them.
    Use printTests to see available aliases.
*/
function dailyDevRunAndSelfHost(szAlias)
{
    // Do a quick check now for the alias.
    var tasks = _AddDDRSelfHostTask(szAlias, false);

    return doRun(tasks.DDR.name);
}

/****************************************************************************/ 
/*
    Copy down the latest self host tests and then run self-host for a 
    given test aliases.
    Use printTests to see available aliases.
*/
function copyAndRunSelfHost()
{
    var tasks = new Array();
    for (var i = 0; i < arguments.length; i++) {
        var task = _createSelfHostTaskFromAlias(arguments[i], undefined, undefined, true, false);
        tasks.push(task);
        _taskAdd(task);
    }
    var mainTask = taskGroup("SelfHost runs", tasks);
    _taskAdd(mainTask);
    
    return doRun(mainTask);
}

/****************************************************************************/ 
/*
    Daily Dev run w/ self-host for the given alias string at the end.
    This will copy down the latest self host tests and run them.
    Use printTests to see available aliases.

    This uses multiple runtimes to run selfhost
*/
function dailyDevRunAndSelfHostMW(szAlias)
{
    // Do a quick check now for the alias.
    var tasks = _AddDDRSelfHostTask(szAlias, true);

    return doRun(tasks.DDR.name);
}

/****************************************************************************/ 
/*
    Copy down the latest self host tests and then run self-host for a 
    given test alias.
    Use printTests to see available aliases.

    This uses multiple runtimes to run selfhost
*/
function copyAndRunSelfHostMW(szAlias)
{   
    var task = _createSelfHostTaskFromAlias(szAlias, undefined, undefined, true, true, true);
    _taskAdd(task);
    return doRun(task);
}

/***************************************************************************/
/* Some common tasks used by the JIT team */
_taskAdd(_createSelfHostTaskFromAlias("JIT", "x86", "chk", true, true));
_taskAdd(_createSelfHostTaskFromAlias("JIT", "x86", "chk", false, false));

_taskAdd(taskGroup("DDR_And_x86JitSelfHost", [
        _taskDailyDevRun,
        _createSelfHostTaskFromAlias("JIT", "x86", "chk", true, true),
    ], 
    "DDR + x86 JIT self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx"));

if (_canRunAmd64)
{
    _taskAdd(_createSelfHostTaskFromAlias("JIT", "amd64", "chk", true, true));
    _taskAdd(_createSelfHostTaskFromAlias("JIT", "amd64", "chk", false, false));
    _taskAdd(taskGroup("DDR_And_x64JitSelfHost", [
            _taskDailyDevRun,
            _createSelfHostTaskFromAlias("JIT", "amd64", "chk", true, true),
        ], 
        "DDR + x64 JIT self host tests",
        "http://mswikis/clr/dev/Pages/Running%20Tests.aspx"));
}

var _taskDDR_And_JitSelfHost = taskGroup("DDR_And_JitSelfHost", [
        _taskDailyDevRun,
        _createSelfHostTaskFromAlias("JIT", "x86", "chk", true, true),
        IF_RUN(_canRunAmd64, _createSelfHostTaskFromAlias("JIT", "amd64", "chk", true, true)),
    ], 
    "DDR + x86 and x64 JIT self host tests",
    "http://mswikis/clr/dev/Pages/Running%20Tests.aspx");

_taskAdd(_taskDDR_And_JitSelfHost);