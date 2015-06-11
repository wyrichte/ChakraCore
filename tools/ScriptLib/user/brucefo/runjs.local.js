var HOUR   = 60*60;

function tfsync(logDir, srcBase, pathToTfCmd)
{
    tfget(logDir, srcBase, pathToTfCmd);
}

function tfget(logDir, srcBase, pathToTfCmd)
{
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument 'srcBase' is missing and environment variable '_NTBINDIR' is not set");
    }

    if (pathToTfCmd == undefined) {
        pathToTfCmd = srcBase + "\\tools\\x86\\managed\\v4.0\\tf.cmd";
    }

    if (!FSOFileExists(pathToTfCmd)) {
        throwWithStackTrace(new Error(1, "tf.cmd could not be found at " + pathToTfCmd));
    }

    // Set up runOpts for the calls to tf.cmd
    var runOpts =   runSetNoThrow(
                    runSetTimeout(HOUR));

    var run = runCmdToLog(pathToTfCmd + " get /noprompt", runOpts);
    if (run.exitCode != 0)
    {
        logMsg(LogClrAutomation, LogInfo, "tf returned " + run.exitCode + "\r\n");
        return;
    }

    var run = runCmdToLog(pathToTfCmd + " resolve " + srcBase + "\\rotor\\prebuilt\\MacManagedAssemblies /noprompt /auto:OverwriteLocal /recursive", runOpts);
    if (run.exitCode != 0)
    {
        logMsg(LogClrAutomation, LogInfo, "tf returned " + run.exitCode + "\r\n");
        return;
    }

    var run = runCmdToLog(pathToTfCmd + " resolve " + srcBase + "\\public /noprompt /auto:OverwriteLocal /recursive", runOpts);
    if (run.exitCode != 0)
    {
        logMsg(LogClrAutomation, LogInfo, "tf returned " + run.exitCode + "\r\n");
        return;
    }

    // Now let the user do the rest interactively
    var run = runCmdToLog(pathToTfCmd + " resolve", runOpts);
    if (run.exitCode != 0)
    {
        logMsg(LogClrAutomation, LogInfo, "tf returned " + run.exitCode + "\r\n");
        return;
    }
}

/*****************************************************************************/
/*
    Finish a DevDiv enlistment in the Dev11 branch.

    Create CLR shortcuts, etc.
*/

function myFinishEnlistment(root)
{
    setSNSkipVerification();
    
    myCreateCLRShortcuts(root, true);

    return 0;
}

/*****************************************************************************/
/* Create useful desktop and IE shortcuts that a CLR developer should have.

   To create just CLREnv shortcuts, use either myCreateCLRShortcutSet or myCreateCLRShortcut.

     Parameters:
       srcBase : the enlistment base
       inTfs   : a boolean indicating whether this is a TFS enlistment.
*/

function myCreateCLRShortcuts(srcBase, inTfs) {

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (!srcBase)
            throw new Error(1, "Required argument srcBase for the createCLRShortcuts method is missing");
    }
    if (inTfs == undefined) {
        inTfs = true;
    }

    var lab = displayName(srcBase, inTfs);

    var extraClrEnvArgs = "-precmd c:\\User\\scripts\\setsympath.bat -cmd c:\\User\\developer\\brucefo\\setenv.cmd";

    myCreateCLRShortcutSet("DBG", srcBase, extraClrEnvArgs, inTfs);
    myCreateCLRShortcutSet("CHK", srcBase, extraClrEnvArgs, inTfs);
    myCreateCLRShortcutSet("RET", srcBase, extraClrEnvArgs, inTfs);

    logMsg(LogClrAutomation, LogInfo, "Creating useful CLR specific IE Favorites\n");
    makeFavorite("CLR New Dev Page",           "http://mswikis/clr/dev/Pages/New%20Developer.aspx");
    makeFavorite("CLR Dev Home Page",          "http://devdiv/sites/clr/dev/default.aspx");
    makeFavorite("CLR SNAP Checkin Reports",   "file:////CLRMain/public/drops/puclr/snap/JobHistory_puclr.html");
    makeFavorite("CLR Dev Wiki Home Page",     "http://mswikis/clr/dev/Pages/Home.aspx");
    makeFavorite("CLR Dev Visual Studio Page", "http://wiki/default.aspx/Microsoft.Projects.CLRDev/VisualStudio.html");

    var strDesktop = WshShell.SpecialFolders("Desktop");
    logMsg(LogClrAutomation, LogInfo, "Creating shortcut to clr solution\n");
    var oShellLink = WshShell.CreateShortcut(strDesktop + "\\" + lab + " clr.sln.lnk");
    oShellLink.TargetPath       = srcBase + "\\ndp\\clr\\src\\clr.sln";
    oShellLink.WorkingDirectory = srcBase + "\\ndp\\clr\\src";
    oShellLink.Save();

    logMsg(LogClrAutomation, LogInfo, "Creating automation directory...");
    var automationDir = srcBase + "\\automation";
    FSOCreatePath(automationDir);

    if (isElevated())
    {
        logMsg(LogClrAutomation, LogInfo, "and publishing it\n");
        runCmd("net share automation-" + lab + " /delete", runSetNoThrow());
        runCmdToLog("net share automation-"+ lab +"=" + automationDir + " /UNLIMITED");
        makeFavorite("DailyDevRun Report " + lab, "file://" + Env("COMPUTERNAME") + "/automation-" + lab + "/run.current/taskReport.html");
    }
    else {
        logMsg(LogClrAutomation, LogInfo, "Running unelevated - rerun elevated to publish automation directories\n");
    }
}


/*****************************************************************************/
/* Creates a set of ClrEnv command environment shortcuts of type 'kind' (Kind can be
   any args passed to clrenv, e.g., 'ia64 chk'.  'srcBase' is the base of the
   source tree (e.g., c:\vbl\clr). A set of shortcuts includes desktop CLR and CoreCLR,
   and on Vista, elevated and non-elevated.
*/
function myCreateCLRShortcutSet(kind, srcBase, extraClrEnvArgs, tfs) {

    if (kind == undefined)
        kind = "CHK";
    if (srcBase == undefined)
        srcBase = "c:\\vbl\\clr";
    if (extraClrEnvArgs == undefined)
        extraClrEnvArgs = "";
    if (tfs == undefined)
        tfs = true;

    logMsg(LogClrAutomation, LogInfo, "Creating " + kind + " shortcuts to the CLREnv command environment\n");
    myCreateCLRShortcut(kind, srcBase, false, false, extraClrEnvArgs, tfs); //non-elevated desktop
    myCreateCLRShortcut(kind, srcBase, false, true,  extraClrEnvArgs, tfs); //non-elevated CoreCLR

    if (IsWinLHOrLater()) {
        logMsg(LogClrAutomation, LogInfo, "Creating [Elevated] " + kind + " shortcuts to the CLREnv command environment\n");
        myCreateCLRShortcut(kind, srcBase, true, false, extraClrEnvArgs, tfs); // elevated desktop
        myCreateCLRShortcut(kind, srcBase, true, true,  extraClrEnvArgs, tfs); // elevated CoreCLR
    }

    return 0;
}


/*****************************************************************************/
/* Creates a ClrEnv command environment shortcut of type 'kind' (Kind can be
   any args passed to clrenv, e.g., 'ia64 chk'.  'srcBase' is the base of the
   source tree (e.g., c:\vbl\clr).
*/
function myCreateCLRShortcut(kind, srcBase, elevated, isCoreClr, extraClrEnvArgs, tfs) {

    if (kind == undefined)
        kind = "CHK";
    if (srcBase == undefined)
        srcBase = "c:\\vbl\\clr";
    if (elevated == undefined)
        elevated = false;
    if (isCoreClr == undefined)
        isCoreClr = false;
    if (extraClrEnvArgs == undefined)
        extraClrEnvArgs = "";
    if (tfs == undefined)
        tfs = true;

    if (elevated && !IsWinLHOrLater())
        throw new Error(-1, "non-Vista host OS detected for Vista-shortcut creation");

    // Vista doesn't support Itanium, so we don't expect Vista to return ia64
    if (IsWinLH() && "ia64" == getRealProcessorArchitecture().toLowerCase())
        throw new Error(-1, "Vista on Itanium is not supported");

    // Windows 7 doesn't support Itanium, so we don't expect Windows 7 to return ia64
    if (IsWin7() && "ia64" == getRealProcessorArchitecture().toLowerCase())
        throw new Error(-1, "Windows 7 on Itanium is not supported");

    var elevatedTag;
    if (elevated) {
        elevatedTag = "[E] ";
    } else {
        elevatedTag = "";
    }

    var coreClrTag;
    if (isCoreClr) {
        coreClrTag = " CoreCLR";
    } else {
        coreClrTag = "";
    }

    var is64BitMachine;
    if (FSOFolderExists(Env("SystemRoot") + "\\SysWow64")) {
        is64BitMachine = true;
    } else {
        is64BitMachine = false;
    }

    var arch;
    var archFlag; // passed to clrenv.bat

    var lab = displayName(srcBase, tfs);

    var extraArgs;
    if (extraClrEnvArgs == "") {
        extraArgs = "";
    } else {
        extraArgs = " " + extraClrEnvArgs;
    }

    var systemDir;

    //////////////////////////////////////////////////////////////////////////////////////////

    arch = "ARM ";
    archFlag = "ARM ";
    if (is64BitMachine) {
        systemDir = "syswow64";     // make ARM shortcut in a 32-bit window
    } else {
        systemDir = undefined;
    }

    var filename = lab + " " + elevatedTag + arch + kind + coreClrTag + ".lnk";
    desktopCmdShortCut(filename,
                        "/k " + srcBase + "\\ndp\\clr\\bin\\clrenv.bat " + archFlag + kind + coreClrTag + extraArgs,
                        srcBase + "\\ndp\\clr\\src",
                        arch + kind + coreClrTag + " CLREnv environment for " + srcBase,
                        systemDir);

    var strDesktop = WshShell.SpecialFolders("Desktop");
    if (elevated) {
        makeAdminShortcut(srcBase, strDesktop + "\\" + filename);
    }

    //////////////////////////////////////////////////////////////////////////////////////////

    if (is64BitMachine) {
        arch = "64 ";
    } else {
        arch = "32 ";
    }
    archFlag = "";
    systemDir = undefined;

    var filename = lab + " " + elevatedTag + arch + kind + coreClrTag + ".lnk";
    desktopCmdShortCut(filename,
                        "/k " + srcBase + "\\ndp\\clr\\bin\\clrenv.bat " + archFlag + kind + coreClrTag + extraArgs,
                        srcBase + "\\ndp\\clr\\src",
                        arch + kind + coreClrTag + " CLREnv environment for " + srcBase,
                        systemDir);

    var strDesktop = WshShell.SpecialFolders("Desktop");
    if (elevated) {
        makeAdminShortcut(srcBase, strDesktop + "\\" + filename);
    }

    //////////////////////////////////////////////////////////////////////////////////////////

    if (is64BitMachine) {

        arch = "32 ";
        archFlag = "";
        systemDir = "sysWow64";

        filename = lab + " " + elevatedTag + arch + kind + coreClrTag + ".lnk";
        desktopCmdShortCut(filename,
                        "/k " + srcBase + "\\ndp\\clr\\bin\\clrenv.bat " + archFlag + kind + coreClrTag + extraArgs,
                        srcBase + "\\ndp\\clr\\src",
                        arch + kind + coreClrTag + " CLREnv environment for " + srcBase,
                        "sysWow64");
        if (elevated) {
            makeAdminShortcut(srcBase, strDesktop + "\\" + filename);
        }

    }

    return 0;
}

