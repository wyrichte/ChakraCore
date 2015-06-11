/*****************************************************************************/
/*                              PrefastPlugins.js                             */
/*****************************************************************************/

/*
  Support for user-written prefast plugins with optional post-processing steps.
 */
var PrefastPluginsModuleDefined = 1;        // Indicate that this module exists

if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!utilModuleDefined) throw new Error(1, "Need to include util.js");

var LogPrefastPlugins = logNewFacility("PrefastPlugins");

/*
  Create a bare bones PREfast plugin by taking a template directory and a new
  plugin name and generating a new plugin directory. Optionally provide a more
  verbose plugin name (two or three words) used in the registry and resources
  (default is to just use the plugin name).
  Note that the plugin name should be camel cased with a leading upper case
  letter (e.g. Foo or MyPlugin). This command will fail if it's given a plugin
  name that starts with a lower case letter.
*/
function createPrefastPlugin(plugin, pluginDesc)
{
    if (pluginDesc == undefined)
        pluginDesc = plugin;

    if (plugin.substr(0, 1) == plugin.substr(0, 1).toLowerCase())
    {
        logMsg(LogPrefastPlugins, LogError, "The 'plugin' parameter must be capitalized (e.g. 'MyPlugin').\n");
        return 1;
    }

    var pluginRootDir = Env('_NTBINDIR') + "\\ndp\\clr\\src\\prefastplugins";
    var templateDir = pluginRootDir + "\\template";
    var pluginDir = pluginRootDir + "\\" + plugin;

    if (FSOFolderExists(pluginDir))
    {
         logMsg(LogPrefastPlugins, LogError, "Plugin has already been created\n");
         return 1;
    }

    // Copy all the files from the template folder to the new plugin location.
    FSOCopyFolder(templateDir, pluginDir, false);

    // Scan for all files with _Template_ in their name and rename them appropriately.
    var files = FSOGetFilePattern(pluginDir, /_Template_/i);
    for (var i in files)
    {
        var srcFile = files[i];
        var dstFile = srcFile.replace(/_Template_/i, plugin);
        FSOMoveFile(srcFile, dstFile, false);
    }

    // Allocate a new CLSID for this plugin.
    var clsid = '{' + runCmd('uuidgen').output.match(/[0-9a-f-]+/i)[0] + '}';
    var clsidBytes = clsid.replace(/\{(.{8})-(.{4})-(.{4})-(.{2})(.{2})-(.{2})(.{2})(.{2})(.{2})(.{2})(.{2})\}/,
                                   "{0x$1,0x$2,0x$3,{0x$4,0x$5,0x$6,0x$7,0x$8,0x$9,0x$10,0x$11}}");

    // Now scan all the files filling in holder macros. These include:
    //    <<plugin>>        =>  Name of the plugin
    //    <<plugindesc>>    =>  Plugin description
    //    <<clsid>>         =>  CLSID in {01234567-0123-...} form
    //    <<clsidbytes>>    =>  CLSID in {0x1234567, 0x0123, ...} form
    files = FSOGetFilePattern(pluginDir);
    for (var i in files)
    {
        // reset the read-only bit for the copied files
	    if (FSOGetFile(files[i]).Attributes & 1)
	    {
	        FSOGetFile(files[i]).Attributes -= 1;
	    }
        var lines = FSOReadFromFile(files[i]);
        lines = lines.replace(/<<plugin>>/ig, plugin);
        lines = lines.replace(/<<plugindesc>>/ig, pluginDesc);
        lines = lines.replace(/<<clsid>>/ig, clsid);
        lines = lines.replace(/<<clsidbytes>>/ig, clsidBytes);
        FSOWriteToFile(lines, files[i], false);
    }

    // And we're done.
    logMsg(LogPrefastPlugins, LogInfo, "Plugin " + pluginDir + " created successfully\n");
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
  Constructor for _prefastPlugin objects. These are used to register each
  plugin so that prefastPluginBuild knows how to process them.
  
    name        => The name of the plugin (which should be the name of the
                   plugin .dll minus the extension)
    postbuild   => (optional) jscript function to run after the prefast build
    description => Brief (fits on a line with the plugin name) description of
                   the plugin
    ismanaged   => Set to true if the plugin is managed (needs to be run
                   through the ManagedAdapter plugin).

  The postbuild function (if defined) will be passed a string identifying the
  directory from which the plugin was loaded. The function is expected to
  return a boolean result: true for success and false for failure (which will
  fail the entire prefastPluginBuild).
*/
function _prefastPlugin(name, postbuild, description, ismanaged)
{
    this.name = name;
    this.postbuild = postbuild;
    this.description = description;
    this.ismanaged = ismanaged;
}

// Note: ManagedAdapter entry *must* be first in array.
var registeredPrefastPlugins = new Array(new _prefastPlugin('ManagedAdapter',
                                                            undefined,
                                                            "Interface allowing managed plugins to be run. Don't run this directly.",
                                                            false),
                                         new _prefastPlugin('CrstGraph',
                                                            _CrstGraphPostBuild,
                                                            "Look for Crst ranking violations.",
                                                            false),
                                         new _prefastPlugin('FunctionSize',
                                                            _FunctionSizePostBuild,
                                                            "Find largest functions based on code bearing lines.",
                                                            false),
                                         new _prefastPlugin('DumpAST',
                                                            undefined,
                                                            "Dump a very simple AST representation for each function.",
                                                            false),
                                         new _prefastPlugin('TestPlugin',
                                                            undefined,
                                                            "Testing a managed plugin.",
                                                            true),
                                         new _prefastPlugin('DacCop',
                                                            _DacCopAnalyze,
                                                            "Rick's test for DAC rule conformance.",
                                                            true),
                                         new _prefastPlugin('DumpAstManaged',
                                                            undefined,
                                                            "Managed implementation of DumpAST, includes more data.",
                                                            true),
                                         new _prefastPlugin('CodingGuidelines',
                                                            _CodingGuidelinesPostBuild,
                                                            "Check for violations of some CLR coding guidelines.",
                                                            true) );

/*
  Performs a clean build in the specified source directories (a semi-colon 
  delimited lists - default '.'), with the semi-colon delimited set of plugins provided. 
  Use describePrefastPlugins to see a list of installed plugins and what they do.
  If pluginReportDir is specified, generated log files will go there (instead of 
  in the default Reports sub-directory of the plugin binary directory).
  bldType and bldArch specify the type of build and default to x86 and ret respectively.
  Unless doPostProcess is false, plugin-specific post-build steps will be run.
*/
function prefastPluginBuild(plugins, srcDirs, pluginReportDir, bldType, bldArch, doPostProcess)
{
    if (srcDirs == undefined)
        srcDirs = ".";
        
    if (bldType == undefined)
        bldType = "ret";
    if (bldArch == undefined)
        bldArch = "x86";
        
    if (doPostProcess == undefined)
        doPostProcess = true;
    if (doPostProcess == "false")
        doPostProcess = false;

    var vblBase = Env("_NTBINDIR");
    if (!vblBase)
        throw new Error(1, "Required environment variable _NTBINDIR is missing");

    // Assume success;
    var result = true;

    // Determine where the plugin modules themselves live.
    var pluginModDir = vblBase + "\\ndp\\clr\\bin\\x86\\PrefastPlugins";
    var compilerLibDir = vblBase + "\\tools\\x86\\vc\\bin";
    pluginModDir = FSOGetFolder(pluginModDir).Path;

    // Validate the plugin list given before we do any other setup.
    plugins = plugins.split(";");
    var runPlugins = new Array();
    var managedPlugins;
    for (var i in plugins)
    {
        var match = false;
        for (var j in registeredPrefastPlugins)
            if (registeredPrefastPlugins[j].name.toLowerCase() == plugins[i].toLowerCase())
            {
                match = true;
                if (registeredPrefastPlugins[j].ismanaged)
                {
                    // We have a managed plugin. Don't add this to the list of
                    // plugins to run directly: all managed plugins will in
                    // fact be hosted by one unmanaged plugin, ManagedAdapter,
                    // which is guaranteed to be the first plugin in
                    // registeredPrefastPlugins. So add ManagedAdapter the
                    // first time we see a managed plugin and record each
                    // managed plugin in a separate list, managedPlugins. This
                    // list is communicated to ManagedAdapter via an
                    // environment variable at runtime.
                    if (!managedPlugins)
                    {
                        // First managed plugin we've seen.
                        runPlugins.push(registeredPrefastPlugins[0]);
                        managedPlugins = new Array();
                    }

                    // Record which managed plugins are to be run.
                    managedPlugins.push(registeredPrefastPlugins[j]);
                }
                else
                    runPlugins.push(registeredPrefastPlugins[j]);
                break;
            }
        if (!match)
        {
            // For testing purposes we allow the plugin to run anyway if the dll
            // is present (there will be no post-processing obviously, and we
            // guess that it's an unmanaged plugin).
            if (!FSOFileExists(pluginModDir + "\\" + plugins[i] + ".dll"))
                throw new Error(1, "Plugin " + plugins[i] + " not registered\n");
            runPlugins.push(new _prefastPlugin(plugins[i], undefined, undefined, false));
        }
    }

    // Prepare shared plugin resources. We collect plugin reports (if any) in
    // the Reports subdirectory of the directory used to house plugin binaries
    // unless this is overridden by supplying a specific pluginReportDir
    if( pluginReportDir == undefined )
    {
        pluginReportDir = pluginModDir + "\\Reports";
        pluginReportDir = FSOGetFolder(pluginReportDir).Path;
    }
        
    // Clean out data from prior runs first.
    FSOTryDeleteFile(pluginReportDir + "\\*.dat", true);
    FSOTryDeleteFile(pluginReportDir + "\\ManagedAdapterFatalError.txt", true);
    FSOTryDeleteFile(pluginReportDir + "\\*.bldrpt.html", true);

    // Temporary directory for output binaries (to avoid contaminating the real binaries).
    var pluginBinDir = pluginReportDir + "\\BuiltBinaries";
    FSOCreatePath(pluginBinDir);
    pluginBinDir = FSOGetFolder(pluginBinDir).Path;

    // Create an environment for the build.
    var options;

    // We set the _CL_ env var to enable the given prefast plugins, disable the
    // default, built-in plugins (they won't directly in our build anyway) and
    // replace one of the C/C++ front end components due to a bug (VSW 561616).
    var _cl_ = "/astfe:d1Adisablecoreplugins /astfe:Bx" + compilerLibDir + "\\c1xxast.dll /astfe:B1" + compilerLibDir + "\\c1ast.dll";
    for (var i in runPlugins)
        _cl_ += " /analyze:plugin" + pluginModDir + "\\" + runPlugins[i].name + ".dll";
    options = runSetEnv("_CL_", _cl_, options);

    // We set the CLR_PREFAST_PLUGIN_DIR env var so the plugins know where
    // their binaries are located and where reports should be stored.
    options = runSetEnv("CLR_PREFAST_PLUGIN_DIR", pluginModDir, options);
    options = runSetEnv("CLR_PREFAST_REPORT_DIR", pluginReportDir, options);

    // If we have any managed plugins to run we set the CLR_PREFAST_PLUGINS env
    // var to let the adapter plugin know which ones to activate.
    if (managedPlugins)
    {
        var managedPluginNames = '';
        for (var i in managedPlugins)
        {
            if (i > 0)
                managedPluginNames += ';';
            managedPluginNames += managedPlugins[i].name;
        }
        options = runSetEnv("CLR_PREFAST_PLUGINS", managedPluginNames, options);
        
        // Note that by default the plugin will be loaded into the toolset CLR.  If we wanted to change 
        // that (eg. use Whidbey RTM), we could pass something like the following as the last argument
        // to razzzleBuild
        // preBuild = "set MANAGED_TOOLS_ROOT=%WINDIR%\\Microsoft.NET\\Framework\r\n"
        //          + "set MANAGED_TOOLS_VERSION=v2.0.50727";
    }

    // Run the build in each specified source directory
    // Ideally there would be some way to tell build.exe that we want a list of directories to be built.
    // Unfortunately, it looks like build supports one and only one directory hierarchy - that specified by
    // the dirs files.  So we'll just have to kick-off separate builds.  Ideally we could run some of these
    // in parallel to take advantage of multi-proc machines, but then we'd have to worry about dependencies 
    // between directories.
    // Note that this build may depend on outputs that are not being built here, and so a clean 
    // build may be necessary before running this.
    srcDirs = srcDirs.split(";");
    for (var i in srcDirs)
    {
        razzleBuild(bldType, bldArch, srcDirs[i], "-cCL", vblBase, pluginBinDir, pluginReportDir, undefined, undefined, options);
    }

    // Run per-plugin postprocessing steps for unmanaged plugins.
    if( doPostProcess )
    {
        for (var i in runPlugins)
        {
            if (runPlugins[i].postbuild != undefined)
                result = runPlugins[i].postbuild(pluginModDir, pluginReportDir) && result;
        }
    }

    if (managedPlugins)
    {
        // If there was a fatal error at any point, then log that
        var errorFile = pluginReportDir + "\\ManagedAdapterFatalError.txt";
        if( FSOFileExists(errorFile) )
        {
            var error = FSOReadFromFile(errorFile);
            WScript.Echo("Managed prefast plugin FATAL ERROR:");
            WScript.Echo(error);
            WScript.Echo();            
            result = false; // failed
        }

        // Run per-plugin postprocessing steps for managed plugins.
        if( doPostProcess )
        {
            for (var i in managedPlugins)
            {
                if (managedPlugins[i].postbuild != undefined)
                    result = managedPlugins[i].postbuild(pluginModDir, pluginReportDir) && result;
            }
        }
    }

    // Get rid of the binaries we built, they're not needed for anything.
    FSODeleteFolder(pluginBinDir, true);

    return result ? 0 : 1;
}

/*
  List all plugins that can be used with prefastPluginBuild.
 */
function describePrefastPlugins()
{
    WScript.Echo();
    WScript.Echo("Prefast plugins:");
    for (var i in registeredPrefastPlugins)
        if (i > 0)
            WScript.Echo("  " + registeredPrefastPlugins[i].name + " : " + registeredPrefastPlugins[i].description);
    WScript.Echo();
    return 0;
}

//
// Plugin specific post build functions.
//

/*
  Post processing for the CrstGraph plugin.
*/
function _CrstGraphPostBuild(plugindir, reportDir)
{
    var result = runCmdToLog(plugindir + "\\CrstGraphScan.exe scan " + reportDir + "\\CrstGraph.dat " + plugindir + "\\CrstGraph.bsl");
    return result.exitCode == 0;
}

/*
  Post processing for the FunctionSize plugin.
*/
function _FunctionSizePostBuild(plugindir, reportDir)
{
    // Read all the function sizes from the report (one per line). Form an
    // array containing method objects (describing method name, code lines,
    // total lines and number of AST nodes).
    var methods = new Array();

    var file = FSOOpenTextFile(reportDir + "\\FunctionSize.dat", FSOForReading);
    while (!file.AtEndOfStream)
    {
        var line = file.ReadLine();
        if (line.match(/^(.*) (\d+) (\d+) (\d+)/))
        {
            var method = new Object();
            method.name = RegExp.$1;
            method.alllines = RegExp.$2;
            method.codelines = RegExp.$3;
            method.nodes = RegExp.$4;

            methods.push(method);
        }
    }
    file.Close();

    // Sort the methods in descending order by their codelines.
    methods.sort(function (a, b) { return (a.codelines == b.codelines) ? 0 : (a.codelines < b.codelines) ? 1 : -1; });

    // Display the top 20 largest methods (or less if we have less methods than that).
    var items = methods.length > 20 ? 20 : methods.length;
    WScript.Echo();
    WScript.Echo("FuncSize: Top " + items + " largest methods:");
    for (var i = 0; i < items; i++)
    {
        var method = methods[i];
        WScript.Echo("  " + method.name + ": " + method.codelines + " clines, " + method.alllines + " tlines, " + method.nodes + " nodes");
    }
    WScript.Echo();

    return true;
}

/* 
  Post process for the DacCop plugin
*/
function _DacCopAnalyze(pluginDir, reportDir)
{
    // We don't want to rely on having a stable development CLR or Whidbey RTM installed.  Therefore, we run
    // the tool in the toolset CLR (just like the plugin will have used).  Note that this makes it difficult
    // to find BCL assemblies.  For now we're relying on DacCop to use the NoGacResolver to locate the assemblies
    // itself.  Alternately we could rely on DEVPATH to point to the toolset CLR directory, but then we'd
    // have to enable developmentMode in the application or machine config file (which isn't convenient for cl.exe).

    var vblBase = Env("_NTBINDIR");
    if (!vblBase)
        throw new Error(1, "Required environment variable _NTBINDIR is missing");

    var dacCopDatFile = reportDir + "\\DacCop.dat";
    var analyzeCmd = pluginDir + "\\DacCopAnalyze.exe " + dacCopDatFile + " " + pluginDir + "\\DacCopExclusions.xml";
    //analyzeCmd += " -update";   // add this to update baseline
    
    // run DacCopAnalyze.exe, capturing the output
    var result = runCmd(
        analyzeCmd,
        runSetEnv("COMPLUS_InstallRoot", vblBase + "\\tools\\x86\\managed",    // Use the runtime the tools use
        runSetEnv("COMPLUS_Version", "v4.0",
        runSetLog(LogRun, LogInfo,
        runSetTimeout(300,             // 5 min max
        runSetDump("runjs dumpProc %p " + reportDir + "\\DacCopAnalyze.dmpRpt /noInterest",
        runSetNoThrow() ))))));

    // Write some nice output to a separate log file (convenient for tasks), in addition to the stdout run log
    var endMsg;
    if( result.exitCode == 0 )
    {
        endMsg = "DacCopAnalyze.exe succeeded.";
    }
    else
    {
        var s;
        if( result.exitCode > 0 && result.exitCode < 10 )
            s = result.exitCode.toString();
        else
            s = hex32(result.exitCode);
            
        endMsg = "DacCopAnalyze.exe failed with exit code " + s;
    }
            
    FSOWriteToFile( result.output + "\r\n" + endMsg + "\r\n", reportDir + "\\DacCopAnalyze.log", false );

    // Remove the DacCop.dat file produced by the PreFast plugin.  It's huge (~70MB) and leaving a bunch
    // around for all archived DDRs is a lot of wasted disk space.  Unfortunately this means you can't just
    // manually re-run DacCopAnalyze since it's input is gone (but this is normally only done by people
    // working on DacCop itself).
    // You may want to temporarily disable this in order to re-run DacCopAnalyze (eg. to update the
    // exclusion baseline).
    FSODeleteFile(dacCopDatFile);    
    
    return result.exitCode == 0;
}

/* 
  Post process for the CodingGuidelines plugin
  This just filters out duplicates that are created as a result of header files being used
  in multiple CPP files (perhaps in separate invocations of the compiler process).
*/
function _CodingGuidelinesPostBuild(pluginDir, reportDir)
{
    var inFileName = reportDir + "\\CodingGuidelines.dat";
    var inFile = FSOOpenTextFile(inFileName, FSOForReading);
    var outFileName = reportDir + "\\CodingGuidelines.log";
    var outFile = FSOCreateTextFile(outFileName, true);
    
    var linesSeen = new Object;
    var count = 0;
    while (!inFile.AtEndOfStream)
    {
        var line = inFile.ReadLine();
        if(linesSeen[line] == undefined)
        {
            linesSeen[line] = true;
            outFile.WriteLine(line);
            count++;
        }
    }
    
    inFile.Close();
    outFile.Close();
    FSOTryDeleteFile(inFileName);
    
    WScript.Echo();
    WScript.Echo("***** CodingGuidelines prefast plugin run complete *******");
    WScript.Echo("***** See '" + outFileName + "' for results (" + count + " warnings total).");
    WScript.Echo();
}

/*
// Add some standard DacCop tasks to the lists of all tasks, so that, eg. "runjs doRun dacCop@x86chk.dacCop" will work
// Note that only the x86chk task is kept clean against the exclusion list, so others will likely report violations.
_taskAdd( _taskDacCop("chk", "x86") );              //dacCop@x86chk.dacCop
_taskAdd( _taskDacCop("chk", "amd64") );            //dacCop@amd64chk.dacCop
_taskAdd( _taskDacCop("chk", "x86", true) );        //dacCop-inc@x86chk.dacCop
_taskAdd( _taskDacCop("chk", "amd64", true) );      //dacCop-inc@x86chk.dacCop
_taskAdd( _taskDacCop("ret", "x86") );              //dacCop@x86ret.dacCop
_taskAdd( _taskDacCop("ret", "amd64") );            //dacCop@amd64ret.dacCop
*/