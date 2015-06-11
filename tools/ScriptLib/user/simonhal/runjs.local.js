/*****************************************************************************/
/* Personal tasks for custom nightly runs.
*/

/*****************************************************************************/
function _srcBase()
{
    var srcBase = Env("_NTBINDIR");
    if (srcBase == "") {
        if (WScript.ScriptFullName.match(/^(.*?\\src)\\/))
        {
            srcBase = RegExp.$1;
        }
        else
        {
            return undefined;
        }
    }

    return srcBase;
}

/*****************************************************************************/
function _createBuildTask(buildType, buildArch, buildPath, buildArgs)
{
    if (buildPath == undefined) buildPath = "ndp\\src";
    if (buildArgs == undefined) buildArgs = "-CcZP";

    return taskNew("razzleBuild." + buildPath.replace(/[\\]/g, "-") + "@" + buildArch + buildType,
                   "runjs razzleBuild "
                      + buildType + " "
                      + buildArch + " "
                      + buildPath + " "
                      + buildArgs + " "
                      + _srcBase());

}

/*****************************************************************************/
function _simonhalDevBuildAndSetupTask(buildType, buildArch, buildArgs, clrsetupArgs)
{
    if (clrsetupArgs == undefined) clrsetupArgs = "_";

    var buildTask = _createBuildTask(buildType, buildArch, "ndp\\clr", buildArgs);

    var setupTask = taskNew("clrsetup@" + buildArch + buildType,
                            "runjs clrSetup "
	                         + _srcBase() + "\\binaries\\" + buildArch + buildType  // where the binaries got built to.
	                         + " " + clrsetupArgs,
			     [buildTask]);                                              // setup is dependent on the build task

    return [buildTask, setupTask];
}

function _buildLayoutsGroup(types, archs, skus, srcDir, binBase) {
    var builds = [];
    var layouts = [];
    types = types || [Env("_BuildType")];
    archs = archs || [Env("_BuildArch")];
    skus = skus || ["full"];
    srcBase = Env("BRANCH_PATH") || srcBaseFromScript();
    srcDir = srcDir || "ndp\\clr";
    binBase = binBase || "%outDir%";
    
    for (var i = 0; i < types.length; ++i) {
        for (var j = 0; j < archs.length; ++j) {
            var binDir = binBase + "\\" + archs[j] + types[i];
            var bldTask = _razzleBuildTask(types[i], archs[j], srcDir, undefined, undefined, binDir);
            builds.push(bldTask);
            for (var k = 0; k < skus.length; ++k) {
                var layoutTask = _buildLayoutsTask(skus[k], archs[j], types[i], binDir, binDir + "layouts", [bldTask]);
                layouts.push(layoutTask);
            }
        }
    }
    //return builds.concat(layouts);
    return layouts;
}

_taskAdd(taskGroup("sjh_buildLayout", _buildLayoutsGroup()));

var sjh_buildLayouts = _buildLayoutsGroup(["chk", "ret", "dbg"], ["x86", "amd64"], ["full"], "ndp\\clr");

_taskAdd(
    taskGroup(
        "sjh_buildLayouts",
        sjh_buildLayouts
    )
);

_taskAdd(
    taskGroup(
        "sjh_qdr",
        [_quickDevRunTask("chk", "x86", "ndp\\clr")].concat(
        sjh_buildLayouts)
    )
);
            

/*****************************************************************************/
/* Build the runtime.

   Parameters:
     buildType    : flavour to build.
     buildArch    : platform to build for.
     buildArgs    : arguments to the build command.
*/
function simonhalDevBuild(buildType, buildArch, buildArgs)
{
    if (buildType == undefined) buildType = Env("_BuildType");
    if (buildArch == undefined) buildArch = Env("_BuildArch");
    if (buildArgs == undefined) buildArgs = "-CcmP -M " + (parseInt(Env("NUMBER_OF_PROCESSORS")) + 1);

    // Throws on failure.
    razzleBuild(buildType, buildArch, "ndp\\clr\\src", buildArgs, _srcBase());
    return 0;
}

/*****************************************************************************/
/* Build and install the runtime. This calls simonhalDevBuild first, then
   CLRSetup.

   Parameters:
     buildType    : flavour to build.
     buildArch    : platform to build for.
     buildArgs    : arguments to the build command.
     clrSetupArgs : arguments for clrsetup.
*/
function simonhalDevBuildAndSetup(buildType, buildArch, buildArgs, clrSetupArgs)
{
    if (buildType == undefined) buildType = Env("_BuildType");
    if (buildArch == undefined) buildArch = Env("_BuildArch");
    if (clrSetupArgs == undefined) clrSetupArgs = "/nomb";

    simonhalDevBuild(buildType, buildArch, buildArgs);

    // Throws on failure.
    razzleBuild(buildType, buildArch, "ndp\\clr\\src", buildArgs, _srcBase());

    // Throws on failure.
    clrSetup(_srcBase() + "\\binaries\\" + buildArch + buildType, clrSetupArgs);
    return 0;
}



/*****************************************************************************/
function runOnEditFiles(cmdToRun, sdFileSpec)
{
    if (cmdToRun == undefined) throw new Error("Must specify command to run.");
    if (sdFileSpec == undefined) sdFileSpec = "...";
    var lines = runCmd("sd opened -l").output.split(/\s*\n\s*/);
	for(var i = 0; i < lines.length; i++) {
        if (lines[i].match(/(.*)#\d+\s*-\s*edit/)) {
            var file = RegExp.$1;
            WScript.Echo(runCmd(cmdToRun + " " + file).output);
        }
	}
    return 0;
}

/*****************************************************************************/
function syncTree(root, label)
{
    if (root == undefined) {
        root = Env("_NTBINDIR");
        if (root == "") {
            root = ".";
        }
    }
    root = WshFSO.GetAbsolutePathName(root);

	if (label == undefined) {
		if (!(runCmd("sd counter change").output.match(/^(\d+)/)))
			throw new Error(-1, "Could not get latest sync point counter");
		label = RegExp.$1;
	}

    if (Env("_NTDRIVE") == "") throw new Error("Unknown log location.");

    var logFileName = Env("_NTDRIVE") + "\\syncLog\\" + root.match(/^.*vbl\d*\\(.*)$/i)[1].replace(/(\\|:)/g, "\.") + ".log";

    var logFile = FSOOpenTextFile(logFileName, FSOForAppending, true);
    logFile.WriteLine(_logCurDate() + ", " + _logCurTime() + ": " + label);

    sdSyncResolve(root, label);
//    sdSync(root, label, undefined, 1);

    return 0;
}

/*****************************************************************************/
function fixSymPath()
{
    var symPath = Env("_NT_SYMBOL_PATH");

    if (symPath != undefined)
    {
        // Normalize the string
        if (!symPath.match(/;$/)) symPath = symPath + ';';

        var tmpPath = Env("TMP");
        var symCachePath = tmpPath + "\\symCache";
        if (symPath.match(/SRV\*([^;*]+)\*[^;*]+;/i))
        {
            symCachePath = RegExp.$1;
        }
        symPath = symPath.replace(/SRV\*([^;*]+);/ig, "SRV*" + symCachePath + "*$1;");
        WScript.Echo(symPath);
    }

    return 0;
}

/*****************************************************************************/
function _getDepotFile(path)
{
    var filePath = path;
    if (path.match(/^\/\//))
    {
        var sdPath = path;
        filePath = Env("TEMP") + "\\" + sdPath.match(/([^/]+)#/)[1] +
                   "." + (Math.random()).toString().substring(2, 8) + ".sdv";
        //WScript.Echo("sd print -o \"" + filePath + "\" \"" + sdPath + "\"");
        runCmd("sd print -o \"" + filePath + "\" \"" + sdPath + "\"");
    }
    return filePath;
}

/*****************************************************************************/
function sdDiff(/*diffProgram,*/ path1, path2)
{
    var file1 = _getDepotFile(path1);
    var file2 = _getDepotFile(path2);
    runCmd(Env("SDDiff") + " \"" + file1 + "\" \"" + file2 + "\"");
    //WScript.Echo(Env("SDDiff") + " \"" + file1 + "\" \"" + file2 + "\"");
    if (file1 != path1) FSODeleteFile(file1);
    if (file2 != path2) FSODeleteFile(file2);

    return 0;
}
