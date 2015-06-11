/******************************************************************************/
/*                                 integrate.js                                     */
/******************************************************************************/

/* automation for doing the testing for doing forward and reverse integrations
   among the VBLs */
   
// AUTHOR: Joshua Stein
// DATE: 11/1/2003
//
// Updated 2004-2005 by:
//     Chris Tracy (enhancements)
//     Jay Gray (symbols)

var integrateModuleDefined = 1; 			// Indicate that this module exist

    // FSO   => file system;   log     => logCall, LogMsg;
    // email =>  mailSendHtml; symbols => updateSymbolsServer
if (!fsoModuleDefined)     throw new Error(1, "Need to include fso.js");
if (!logModuleDefined)     throw new Error(1, "Need to include log.js");
if (!emailModuleDefined)   throw new Error(1, "Need to include email.js");
if (!symbolsModuleDefined) throw new Error(1, "Need to include symbols.js");

var LogIntegration = logNewFacility("integration");

function fileHistory(filename, startdate, enddate)
{
    var filehistorylist
    var command = "sd filelog -i " + filename;
    var cmdResult = runCmd(command);
    if(startdate == undefined)
    {
        logMsg(LogUtil, LogInfo, "\n\nFile History (" + filename + ")\n---------------------\n");
        logMsg(LogUtil, LogInfo, cmdResult.output + "\n");
        return 0;
    }

    var cmp_year;
    var cmp_month;
    var cmp_day;

    var end_year;
    var end_month;
    var end_day;
    var startDate;
    var endDate;
    var tdate;
    try
    {
        tdate = startdate;
        startdate = startdate + "";
        if(startdate.match(/\d\d\d\d\/\d\d\/\d\d/) == null)
            throw new Exception;

        cmp_year = (startdate.substr(0, 4));
        cmp_month = (startdate.substr(5, 2));
        cmp_day = (startdate.substr(8, 2));
        startDate = new Date(cmp_year, cmp_month, cmp_day);
    }
    catch(Exception)
    {
        logMsg(LogUtil, LogWarn, "Unable to parse start date (" + startdate + "). Expected: yyyy/mm/dd\n");
        return -1;
    }

    if(enddate != undefined)
    {
        try
        {
            if(enddate.match(/\d\d\d\d\/\d\d\/\d\d/) == null)
                throw new Exception;

            enddate = enddate + "";
            end_year = enddate.substr(0, 4);
            end_month = enddate.substr(5, 2);
            end_day = enddate.substr(8, 2);
            endDate = new Date(end_year, end_month, end_day);
        }
        catch(Exception)
        {
            logMsg(LogUtil, LogWarn, "Unable to parse end date (" + enddate + "). Expected: yyyy/mm/dd\n");
            return -1;
        }
    }
        
    var filehistoryarray = cmdResult.output.split("\n");
    var filedate;
    var day;
    var month;
    var year;
    var cmpDate;
    for(var i = 0; i < filehistoryarray.length; i++)
    {
        filedate = filehistoryarray[i].match(/ \d\d\d\d\/\d\d\/\d\d /);
        if(filedate != null)
        {
            filedate = filedate + "";   //cast to string
            year = filedate.substr(1, 4);
            month = filedate.substr(6, 2);
            day = filedate.substr(9, 2);
            cmpDate = new Date(year, month, day);
            if(enddate == undefined)
            {
                if(cmpDate.getYear() >= startDate.getYear() && cmpDate.getMonth() >= startDate.getMonth() && cmpDate.getDay() >= startDate.getDay())
                    logMsg(LogUtil, LogInfo, filehistoryarray[i] + "\n");
            }
            else
            {
                if(cmpDate.getYear() >= startDate.getYear() && cmpDate.getYear() <= endDate.getYear() && cmpDate.getMonth() >= startDate.getMonth() && cmpDate.getMonth() <= endDate.getMonth() && cmpDate.getDay() >= startDate.getDay() && cmpDate.getDay() <= endDate.getDay())

                    logMsg(LogUtil, LogInfo, filehistoryarray[i] + "\n");
            }
        }
        
    }
    return 0;

}

function watchFile(filename)
{
    var envComputerName = Env("COMPUTERNAME");
    var envUsername = Env("USERNAME");
    var file;
    var fileContents;
    if(filename == undefined)
    {
        logMsg(LogUtil, LogWarn, "Usage: runjs watchFile [filename]\n");
        return 0;
    }
    else
    {
        try{
            while(!FSOFileExists(filename));
        }
        catch(Exception)
        {
            throwWithStackTrace(new Error("Error accessing file."));
            return 0;
        }

        // File has been created, monitor its size.
        while(FSOFileExists(filename))
        try{
           if(FSOGetFile(filename).size != 0)
               break;
        }
        catch(Exception) {}
        
        if(FSOFileExists(filename))                       
        {
            file = FSOGetFile(filename);
            fileContents = FSOReadFromFile(filename);
            return mailSendText(envUsername, envComputerName+" has failures in " + filename, envComputerName+" has failures\n" + filename + " contains " + file.Size + " bytes.\n\n" + fileContents);
        }
        else
        {
            //If GetFile fails then the file has been deleted again.
            fileContents = filename + " has been deleted.";
            return mailSendText(envUsername, envComputerName+" has passed successfully " + filename, fileContents);
        }
    }
    return 0;
}

function quickMail() {
    var envComputerName = Env("COMPUTERNAME");
    var envUsername = Env("USERNAME");

    return mailSendText(envUsername, envComputerName+" task complete", envComputerName+" task complete");
}

function parseTrunXmlToHtml(input_filename, output_filename)
{
    if(input_filename == undefined)
    {
        logMsg(LogUtil, LogWarn, "Usage: parseTrunXmlToHtml filename [ouput filename]\n");
        return 1;
    }
    if(!FSOFileExists(input_filename))
    {
        logMsg(LogUtil, LogWarn, "File " + input_filename + " not found\n");
        return 1;
    }
    var fileBody = FSOReadFromFile(input_filename);
    var outputBody = "";
    fileBody.match(/ PROCESSED="(\d+)" /);
    outputBody += "Total VS Tests Run: " + RegExp.$1 + "\n";
    fileBody.match(/ PASSED="(\d+)" /);
    outputBody += "VS Tests Passed: " + RegExp.$1 + "\n";
    fileBody.match(/ FAILED="(\d+)" /);
    outputBody += "VS Tests Failed: " + RegExp.$1 + "\n\n";
    if(RegExp.$1 != "0")
    {
        while(fileBody.match(/ FULLPATH="(\S*)"? /))
        {
            outputBody += RegExp.$1 + "\n";    
            fileBody = fileBody.replace(/ FULLPATH="(\S*)"? /, "");
        }
    }

    if(output_filename != undefined)
    {
        FSOWriteToFile(outputBody, output_filename, false)
    }
    logMsg(LogUtil, LogWarn, outputBody);
    
    
}
function sendMailFile(sendTo, subject, body, filename) {

    var envComputerName = Env("COMPUTERNAME");
    var envUsername = Env("USERNAME");
    var fileBody = "";
    if(sendTo == undefined)
    {
        sendTo = "a-ccosta";
    }
    if(subject == undefined)
    {
        subject = "Message from " + envComputerName;
    }
    if(body == undefined)
        body = "";
    if(filename != undefined)
    {
        var filelist = filename.split(",");
    
        if(filelist.length > 0)
        {
           for(var i = 0; i < filelist.length; i++)
           {
                filename = filelist[i];
                if(filename != undefined)
                {
                    if(FSOFileExists(filename))
                        fileBody = FSOReadFromFile(filename);
                    else
                        fileBody = "Unable to find file " + filename;
                    body = body + "<br>---" + filename + "---<br>" + fileBody;    
                }
            }
        }
    }
    
    
    body = body.replace("\\n", "<br>");
    
    return mailSendHtml(sendTo, subject, body);
}

function updatePublicsRI()
{
	var command;
	var publicchangelist;
	var cmdResult;
	var clrFiles;
	var publicFiles;

	// Step 1:  Take Public Change list.
	takePublicChangeList();

	// Step 2:  Move all publics into public change list.
        var changenumSDFileContents = FSOReadFromFile(Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD");
	
        changenumSDFileContents.match(/Change (\d+) created./)
        publicchangelist = RegExp.$1;

	command = "sd reopen -c " + publicchangelist + " " + Env("_NTBINDIR") + "\\public\\...";
       cmdResult = runCmd(command);
       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");

	// Step 3: Move *isolation_Whidbey* back into default CL;
	publicFiles = cmdResult.output.split("\n");
	if(publicFiles[0].search("- file(s) not opened on this client.")  == -1)
	{
		command = "sd reopen -c0 " + Env("_NTBINDIR") + "\\public\\sdk\\lib\\*\\isolation_whidbey*";
		cmdResult = runCmd(command);
	       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");

		// Step 4: revert public changelist.
		command = "sd revert -c " + publicchangelist + " " + Env("_NTBINDIR") + "\\public\\...";
		cmdResult = runCmd(command);
	       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");
	}
	else
	{
		logMsg(LogIntegration, LogInfo, "No public files opened on this client.");
	}
	return 0;
		
}

function updatePublicsFI()
{
	var command;
	var publicchangelist;
	var cmdResult;
	var clrFiles;
	var publicFiles;

	// Step 1:  Take Public Change list.
	takePublicChangeList();

	// Step 2: Reopen public changelist.
        var changenumSDFileContents = FSOReadFromFile(Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD");
	
        changenumSDFileContents.match(/Change (\d+) created./)
        publicchangelist = RegExp.$1;

	command = "sd reopen -c " + publicchangelist + " " + Env("_NTBINDIR") + "\\public\\...";
	cmdResult = runCmd(command);
       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");

	command = "sd resolve -ay " + Env("_NTBINDIR") + "\\public\\sdk\\lib\\*\\isolation_whidbey*";
	cmdResult = runCmd(command);
       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");
	
	command = "sd resolve -at " + Env("_NTBINDIR") + "\\public\\...";
	cmdResult = runCmd(command);
       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");
	
	return cmdResult.exitCode;
		
}

function updatePublicsBeforeSubmitFI()
{
	var command;
	var publicchangelist;
	var cmdResult;

        var changenumSDFileContents = FSOReadFromFile(Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD");
	
        changenumSDFileContents.match(/Change (\d+) created./)
        publicchangelist = RegExp.$1;

	command = "sd reopen -c0 " + Env("_NTBINDIR") + "\\public\\...";
	cmdResult = runCmd(command);
       logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n");

	return cmdResult.exitCode;

}
/****************************************************************************/
/*
*/
function takePublicChangeList() {
    //var takeCLLog = LogUtil
    //var command = "%COMSPEC% /C type " + Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD"
    //var cmdResult = runCmd(command, runSetNoThrow())
//    logMsg(LogIntegration, LogInfo, "command: " + command + "\n")
//    logMsg(LogIntegration, LogInfo, "output: " + cmdResult.exitCode + "\n")

    //result = FSOReadFromFilecmdResult.output
    var line
    var cl
    if(FSOFileExists(Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD")) {
        var changenumSDFileContents = FSOReadFromFile(Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD");
        if(changenumSDFileContents.match(/Change (\d+) created./) != null) {
            cl = RegExp.$1;
            logMsg(LogIntegration, LogInfo, "current CL is ", cl, "\n")
            var result = runCmd("sd describe -s " + cl); 
            if (result.exitCode != 0) {
                throwWithStackTrace(new Error("sd describe failed, check SD status"));
            }    

            line = result.output.split("\n")[0]
  //          logMsg(LogIntegration, LogInfo, "Saving line: " + line + "\n")
    
            if(line.match(/\\.+@/) != null) {
                user = line.match(/\\.+@/)[0]
                user = user.substr(1, user.length - 2)

                if(user.match(Env("USERNAME")) != null) {
                    logMsg(LogIntegration, LogInfo, user + " is current owner\n")
                }
                else {
                    logMsg(LogIntegration, LogWarn, Env("USERNAME") + " is not current owner, "+ user + " is current owner\n")

                    _updatePublicChangeList(cl)
                }       
                
            }
            // Could not determine owner, taking ownership
            else { 
                _updatePublicChangeList(cl)
           } 
        }
        else {
            logMsg(LogIntegration, LogWarn, "PUBLIC_CHANGENUM.SD does not exist or is invalid.\n")

            _updatePublicChangeList(cl, "createPublicChangenum")
        }
    
    }
    else
    {
       logMsg(LogIntegration, LogWarn, "PUBLIC_CHANGENUM.SD does not exist or is invalid.\n")

       _updatePublicChangeList(cl, "createPublicChangenum")
    }
    
    return 0;
}

function _updatePublicChangeList(cl, options) {

    if(options == undefined)
        options = "doAll"

    if(cl == undefined)
        var noRevert = true

    if((options == "revert" || options == "doAll") && noRevert != true ) {
        logMsg(LogIntegration, LogInfo, "reverting " + cl + "\n")
        command = "sd revert " + Env("_NTBINDIR") + "\\public\\..."
        cmdResult = runCmd(command, runSetNoThrow())
        logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n")
    }
    if(options == "delPublicChangenum" || options == "doAll") {
        logMsg(LogIntegration, LogInfo, "Deleting " + Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD\n")
        command = "del /f " + Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD"
        cmdResult = runCmd(command, runSetNoThrow())
        logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n")
    }
    if(options == "createPublicChangenum" || options == "doAll") {
        logMsg(LogIntegration, LogInfo, "Creating PUBLIC_CHANGENUM.SD\n")
        command = "%COMSPEC% /k " + Env("_NTBINDIR") + "\\tools\\razzle exec exit"
        cmdResult = runCmd(command, runSetNoThrow())
        logMsg(LogIntegration, LogInfo, "Command: " + command + "\n" + cmdResult.output + "\n")
        var publicChangenumSDFileContents = FSOReadFromFile(Env("_NTBINDIR") + "\\public\\PUBLIC_CHANGENUM.SD");
        if(publicChangenumSDFileContents.match(/Change (\d+) created./) != null) {
            logMsg(LogIntegration, LogInfo, "New public CL: " + RegExp.$1 + "\n");
        }
        else throwWithStackTrace(new Error("razzle did not correctly create a new public CL, check SD status"));
    }
}

function integrateWsf(sourceLab, targetLab, sourceLabTime, srcBase, integrationConfigFile) {
    var env_NTBINDIR = Env("_NTBINDIR");
    if (srcBase == undefined) {
        srcBase = env_NTBINDIR;
        if (srcBase == undefined || srcBase == "") {
            throwWithStackTrace(new Error("srcBase arg not given and _NTBINDIR not defined, run in CLREnv or razzle window"));
        }
    }
    else {
        if (srcBase == ".")
            srcBase = WshShell.CurrentDirectory;
    }
    srcBase = srcBase.toLowerCase();
    if (env_NTBINDIR != undefined && srcBase != env_NTBINDIR.toLowerCase()) {
        throwWithStackTrace(new Error("_NTBINDIR != srcBase, use /force option to override (NYI)"));
    }
    if (WshShell.CurrentDirectory.toLowerCase() != srcBase.toLowerCase()) {
        throwWithStackTrace(new Error("ERROR: must be run from base of elistment, use /force option to override (NYI)"));
    }
    if (sourceLab == undefined) {
        throwWithStackTrace(new Error("required argument sourceLab missing"));
    }
    if (targetLab == undefined) {
        throwWithStackTrace(new Error("required argument targetLab missing"));
    }

    var integrationLogDir = WshFSO.GetParentFolderName(srcBase) + "\\IntegrationLogs";
    // mkdir the log dir on the integration machine if it doesn't exist
    FSOCreatePath(integrationLogDir);
    var intgWsfArgs = "/tempdir:"+integrationLogDir;
    
    var envUsername = Env("USERNAME");
    if (envUsername == undefined || envUsername == "") {
        throwWithStackTrace(new Error("could not get USERNAME from Env"));
    }
    
    var username = envUsername;
    var requestor = envUsername;
    var IsCLRIntegration = false;

    if (targetLab == "lab21s" || sourceLab == "lab21s") {
        IsCLRIntegration = true; // in case we need this state later... consider removing
        requestor = "rkrish";
        if (integrationConfigFile == undefined) {
            if (Env("DEVDIR") != "") {
                integrationConfigFile = Env("DEVDIR")+"\\tools\\clr\\integrateWsfClrConfig.xml";
                logMsg(LogIntegration, LogInfo, "looking for CLR integration config file ", integrationConfigFile, "\n");
            }
            if (!FSOFileExists(integrationConfigFile)) {
                integrationConfigFile = WshFSO.GetParentFolderName(srcBase)+"\\dev\\tools\\clr\\integrateWsfClrConfig.xml";
                logMsg(LogIntegration, LogInfo, "looking for CLR integration config file ", integrationConfigFile, "\n");
            }
        }
        if (!FSOFileExists(integrationConfigFile)) {
            throwWithStackTrace(new Error("ERROR: lab21s integrations require CLR integration configuration file\r\n"
                                          + "CLR integration configuration file not found, "+integrationConfigFile+"\r\n"));
        }
    }

    if (integrationConfigFile != undefined) {
        intgWsfArgs += " /config:"+integrationConfigFile;
    }
    
    var integrationVerb = "/integrate";
    var integrateWsfSessionFile = integrationLogDir + "\\" + srcBase.replace(/[\\:]/g, '_') + "\\IntegrationSession.xml";
    if (FSOFileExists(integrateWsfSessionFile)) {
        integrationVerb = "/reintegrate";
        logMsg(LogIntegration, LogInfo, "using /reintegrate because integration session file exists at ", integrateWsfSessionFile, "\n");
    }

    intgWsfArgs = intgWsfArgs 
        + " " + integrationVerb
        + " /fldbcheck:no"
        + " /nobuildrequest"
        + " /loop:no"
        + " /user:"+username
        + " /requestor:"+requestor
        + " /source:"+sourceLab
        + " /target:"+targetLab
        + " /srctime:"+sourceLabTime;

    var integrateWsfCmd = "cscript //nologo "+srcBase+"\\tools\\devdiv\\integrate.wsf "+intgWsfArgs;
    var run = runCmdToLog(integrateWsfCmd, runSetNoThrow(runSetTimeout(72*60*60))); // will run resolve loop for 72 hours before timing out
    return run.exitCode;
}    

    
    
function _WrapCmdInRazzleEnv(cmdToWrap, srcBase) {
    logCall(LogIntegration, LogInfo, "_WrapCmdInRazzleEnv", arguments);
    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (srcBase == undefined || srcBase == "") {
            throwWithStackTrace(new Error("srcBase arg not given and _NTBINDIR not defined, run in CLREnv or razzle window"));
        }
    }
    else if (srcBase == ".") {
        srcBase = WshShell.CurrentDirectory;
    }
    srcBase = srcBase.toLowerCase();

    if (cmdToWrap == undefined) {
        throwWithStackTrace(new Error("required arg cmdToWrap missing"));
    }    

    var envDevDir = Env("DEVDIR");
    var envRazzleToolPath = Env("RazzleToolPath");
    if (envRazzleToolPath != "") {
        logMsg(LogIntegration, LogInfo, "Detected we are in razzle\n");
        return cmdToWrap;
    }
    else if (envDevDir != undefined) {
        var rbuild = envDevDir+"\\tools\\clr\\rbuild.bat";
        if (FSOFileExists(rbuild)) {
            return rbuild + " /exec " + cmdToWrap;
        }
    }
    else {
        return srcBase + "tools\\razzle exec " + cmdToWrap;
    }
}
   

function ClearSpace(srcBase) {
    return clearSpace(srcBase);
}

/*****************************************************************************/
/* clear space for a build (especially a full stack build). */

function clearSpace(srcBase) {
    logCall(LogIntegration, LogInfo, "clearSpace", arguments);

    if (srcBase == undefined) {
        srcBase = Env("_NTBINDIR");
        if (srcBase == undefined || srcBase == "") {
            throwWithStackTrace(new Error("srcBase arg not given and _NTBINDIR not defined, run in CLREnv or razzle window"));
        }
    }
    else if (srcBase == ".") {
        srcBase = WshShell.CurrentDirectory;
    }
    srcBase = srcBase.toLowerCase();

    if (WshShell.CurrentDirectory.toLowerCase() != srcBase) {
        logMsg(LogIntegration, LogWarn, "Not running from root of enlistment!\n");
    }

    var srcRootBinaries = srcBase+"\\binaries";
    var srcDriveRootBinariesPat = "binaries.*";
    var srcDriveRootBinaries = FSOGetDirPattern(WshFSO.GetDriveName(srcBase) + "\\", srcDriveRootBinariesPat);
    var srcRootAutomation = srcBase+"\\automation";
    logMsg(LogIntegration, LogInfo, "This will delete the following folders and scorch the enlistment.\n");
    logMsg(LogIntegration, LogInfo, "  ", srcRootBinaries, "\n");
    logMsg(LogIntegration, LogInfo, "  ", srcRootAutomation, "\n");
    logMsg(LogIntegration, LogInfo, "  ", WshFSO.GetDriveName(srcBase) + "\\" + srcDriveRootBinariesPat, "\n");
    for (var i = 0; i < srcDriveRootBinaries.length; ++i) {
        logMsg(LogIntegration, LogInfo, "  found ", srcDriveRootBinaries[i], "\n");
    }
    if (FSOFolderExists(srcRootBinaries)) {
        logMsg(LogIntegration, LogInfo, "Deleting folder ", srcRootBinaries, "\n");
        FSODeleteFolder(srcRootBinaries, true);
    }

    var srcRootAutomation = srcBase+"\\automation";
    if (FSOFolderExists(srcRootAutomation)) {
        logMsg(LogIntegration, LogInfo, "Deleting folder ", srcRootAutomation, "\n");
        if (FSOFolderExists(srcRootAutomation+"\\run.current")) {
            runCmdToLog("linkd " + srcRootAutomation + "\\run.current /d");
        }
        FSODeleteFolder(srcRootAutomation, true);
    }

    for (var i = 0; i < srcDriveRootBinaries.length; ++i) {
        if (FSOFolderExists(srcDriveRootBinaries[i])) {
            logMsg(LogIntegration, LogInfo, "Deleting folder ", srcDriveRootBinaries[i], "\n");
            FSODeleteFolder(srcDriveRootBinaries[i], true);
        }
    }
    var scorchCmd = _WrapCmdInRazzleEnv(srcBase + "\\tools\\devdiv\\ddscorch");
    runCmdToLog(scorchCmd, runSetNoThrow(runSetTimeout(5*60*60))); // five hours in case of SD perf issues
}

/*****************************************************************************/
/* do all the automatic resolves and list the manual ones still to be done   */

function sdResolve(toLab, ResolveArgs) {

    logCall(LogIntegration, LogInfo10, "sdResolve", arguments);
    if (ResolveArgs == "-as")
        logMsg(LogIntegration, LogInfo,
        "\n******************* RESOLVING NON-CONFLICTING FILES *******************\n")
    if (ResolveArgs == "-n")
            logMsg(LogIntegration, LogInfo,
            "\n******************* FILES REQUIRING RESOLUTION *******************\n")
    var cmd = "sd resolve " + ResolveArgs +  " //depot/devdiv/" + toLab + "/...";
    var conflicts = runCmdToLog(cmd, runSetTimeout(14400)).output;
    if (!conflicts.match(/no file/i))   // sd resolve output does not indicate 'no file(s)'
        logMsg(LogIntegration,
        LogInfo, "\nUNRESOLVED CONFLICTS MUST BE MANUALLY RESOLVED BEFORE SUBMITTING\n")

    return 0                
}

/*****************************************************************************/
/* do all the sync, integrate, automatic resolves and list the manual ones still to be done					  */

function prepareIntegration(srcBase, fromLab, fromLabel, toLab, toLabel)
{
	if (fromLab == undefined)
		throw Error(1, "Arg fromLab not supplied");
	if (toLab == undefined)
		throw Error(1, "Arg toLab not supplied");
	// validate the labs
	if (fromLab.toString().toLowerCase() == toLab.toString().toLowerCase())
		throw Error(1, "fromLab " + fromLab + " is the same as toLab " + toLab);
	if (fromLabel == undefined)
		throw Error(1, "Arg fromLabel not supplied");
	if (toLabel == undefined)
		throw Error(1, "Arg toLabel not supplied");
	// validate the change lists
	if (fromLabel.toString().toLowerCase() == toLabel.toString().toLowerCase())
		throw Error(1, "fromLabel " + fromLabel + " is the same as toLabel " + toLabel);

	// create and run integration commands
	var cmds = [];
	cmds.push("runjs sdSync " + srcBase + " " + toLabel);
	cmds.push("runjs sdIntegrate " + fromLab + " " + toLab + " " + fromLabel);
	cmds.push("runjs sdResolve " + toLab + " -as");
	cmds.push("runjs sdResolve " + toLab + " -am");
	cmds.push("runjs sdResolve " + toLab + " -n");
//	logMsg(LogTask, LogInfo, dump(cmds), "\n");	// usefull for debugging

	for (var i = 0; i < cmds.length; i++) {
		runCmdToLog(cmds[i] , runSetTimeout(4*3600));
	}

	return 0;
}
/*****************************************************************************/
/* send the taskReport to the task owner by email									  */
/*	Note: this task will always show as Running Task in the email.						  */

function sendMailForIntegration(outDir, subject) {

	logCall(LogIntegration, LogInfo, "sendMailForIntegration", arguments);

	var integrationXMLFile = outDir + "\\automation\\run.current\\taskReport.html";	// get the current task's report

	if (!FSOFileExists(integrationXMLFile)) {
		logMsg(LogClrProcess, LogError, "Could not find integration automation report ", integrationXMLFile, "\n");
		return 0;	// don't report failure on this task because we don't know the job status and don't want a false failure reported
	}
	// get the subject
	if (subject == undefined || subject == "")
		subject = "Integration Automation Completion Notice";
	
	// get the submitter(s)
	var submitters = "";
	var jobReportData = "";
	if (FSOFileExists(integrationXMLFile)) {
		
		jobReportData = FSOReadFromFile(integrationXMLFile);			// read it in one chunk

		while(jobReportData.match(/<LI>.*User: *(.*)/im)) {			// look for 'User'
			jobReportData = RegExp.rightContext;						// move past the match
			var submitter = RegExp.$1;
			submitter = submitter.replace(/(\<\/b\>)/, "");				// remove the tag
			submitter = submitter.replace(/(\s)/, "");					// remove the newline
			submitter = submitter.replace(/(.*)/, "$1@microsoft.com");	// add the micorosft mail tag
			if (submitters != "")									// not 0 length
				submitters += "; ";									// add the separator
			submitters += submitter;			
		}
	}
	else 
		logMsg(LogIntegration, LogError, "Could not find task report file ", integrationXMLFile, "\n");

	if (submitters.length > 0)
		mailSendHtml(submitters, subject, jobReportData, undefined);
	else 
		logMsg(LogIntegration, LogError, "Could not find automation submitters\n");
	
	return 0;
}


/***********************************************************************/
/* starting with a drop base 'srcDir' copy just the files needed 
   for a setup to 'destDir 

   Parameters:
     srcDir  : The source directory
     destDir : The destination directory
     version: Runtime version
     logDir  : Where to put the log files
     cacheOptions : /noSS to skip source server reindexing

   Example:
    the following two commands are equivalent
    runjs cacheDropForCLR \\cpvsbuild\drops\orcas\Lab24\raw \\CLRMain\public\Drops\puclr\builds\ForClrSetup 41222.00 c:\vbl\cacheLogs\41222
    runjs cacheDropForCLR _ _ 41222.00 c:\vbl\cacheLogs\41222
    
*/
function cacheDropForCLR(srcDir, destDir, version, logDir, cacheOptions) {
    logCall(LogIntegration, LogInfo, "cacheDropForCLR", arguments, " { ");

    //  validate parameters
    if (srcDir == undefined)
        srcDir = "\\\\cpvsbuild\\drops\\orcas\\Lab24\\raw";
    if (destDir == undefined)
        destDir = "\\\\clrmain\\public\\drops\\puclr\\builds\\ForClrSetup";
    if (version == undefined)
        throw Error(1, "Required parmeter version not present");
    if (logDir == undefined)
        throw Error(1, "Required parmeter logDir not present");
    if (!FSOFolderExists(srcDir + "\\" + version))
        throw Error(1, "Source directory " + srcDir + "\\" + version + " does not exist!");
    if (!FSOFolderExists(logDir)) {
        FSOCreatePath(logDir);
        logMsg(LogIntegration, LogInfo, "Created log directory ", logDir, "\n");
    }
    var verDir = destDir + "\\" + version;
    if (!FSOFolderExists(verDir))
    {
        FSOCreatePath(verDir);
        logMsg(LogIntegration, LogInfo, "Created ", verDir, "\n");
    }
    var errorsList = [];

    var cacheOpts = getOptions(["noSS"], cacheOptions);

    var runtimeDirs = [ "binaries.amd64ret", "binaries.ia64ret", "binaries.x86chk", "binaries.x86ret", "sources" ];

    // create \sources and \sources\redist, which require special permissions
    for(var i = 0; i < runtimeDirs.length; i++)
    {
        var leaf = runtimeDirs[i];
        var destPath =  verDir + "\\" + leaf;
        logMsg(LogIntegration, LogInfo, "Checking for ", destPath, "\n");
        if (!FSOFolderExists(destPath))
        {
            if (destPath.match(/sources/))  // restrict access to sources
            {
                FSOCreatePath(destPath);
                logMsg(LogIntegration, LogInfo, "Created ", destPath, "\n");
                var sourcesRoot = destPath;
                var cmd = "cacls " + sourcesRoot + " /E /R  Everyone";  
                var run = runCmdToLog(cmd, runSetTimeout(10000, runSetNoThrow(runSetOutput(undefined))));
                
                //  create restricted source subtree - directories inherit parent permissions
                var sourcesDirs = FSOGetDirPattern(srcDir + "\\" + version + "\\sources", /.*/);
                for(var i = 0; i < sourcesDirs.length; i++)
                {
                    sourcesDirs[i].match(/.*\\(.*)$/);  // pick up the destination path leaf
                    var leaf = RegExp.$1;
                    var sourcesPath = verDir + "\\sources\\" + leaf;
                    logMsg(LogIntegration, LogInfo, "Checking for ", sourcesPath, "\n");
                    if (sourcesPath.match(/redist/))    //  unrestrict this one but only for read access
                    {
                        if (!FSOFolderExists(sourcesPath))
                        {
                            FSOCreatePath(sourcesPath);
                            logMsg(LogIntegration, LogInfo, "Created ", sourcesPath, "\n");
                            cmd = "cacls " + sourcesPath + " /E /G  Everyone:R";    
                            run = runCmdToLog(cmd, runSetTimeout(10000, runSetNoThrow(runSetOutput(undefined))));
                            logMsg(LogIntegration, LogInfo, "Set ACLS to /E /G for Everyone", "\n");
                        }
                    }
                }
            }
        }
    }
    
    //  now robocopy the files
    var optionsNoSubDirs = "/np /v /eta /R:3 /W:10";
    var options = optionsNoSubDirs + " /e";
    var robocopyRunOptions = runSetTimeout(15 * HOUR, runSetNoThrow(runSetOutput(undefined)));
    
    for(var i = 0; i < runtimeDirs.length; i++)
    {
        var destPath =  verDir + "\\" + runtimeDirs[i];
        var logFile = logDir + "\\copy." + runtimeDirs[i] + ".log ";
        var srcPath = srcDir + "\\" + version + "\\" + runtimeDirs[i];
        logMsg(LogIntegration, LogInfo, "Processing ", srcPath, "\n");

        if (FSOFileExists(logFile)) {
            FSODeleteFile(logFile);
        }    
        if (runtimeDirs[i] == "sources") {
            var cmd = "robocopy " + srcPath + " " + destPath + " " + options + " /LOG+:" + logFile;
            // Now that we've built the cmd to run, run it!
            var run = runCmdToLog(cmd, robocopyRunOptions);
            logMsg(LogIntegration, LogInfo, "run.exitCode= ", run.exitCode, "\n");
        }
        else {
            if (!FSOFileExists(srcPath + "\\clr.dll")) {
                var errorMsg = "ERROR: build not yet published, clr missing from "+srcPath + "\\clr.dll\n";
                errorsList.push(errorMsg);
                logMsg(LogIntegration, LogError, errorMsg);
            }    
            var cmd = "robocopy " + srcPath + " " + destPath + " " + options + " /LOG+:" + logFile;
            var run = runCmdToLog(cmd, robocopyRunOptions);
            logMsg(LogIntegration, LogInfo, "run.exitCode= ", run.exitCode, "\n");

         }
        
    }

       if (cacheOpts.noSS == undefined) {
        // posts a \\symbols\symbols indexing request for each valid 'runtimedir'
        updateSymbolsServer("post", version, "lab21", verDir, "CLR");

        logMsg(LogIntegration, LogInfo, "Symbols index requests for " +
            verDir + " builds are posted to \\\\symbols\\symbols\n");
    }
    
    logMsg(LogIntegration, LogInfo, "} cacheDropForCLR\n");

    return 0;
}
       
                
/***********************************************************************/
/* deletes a \\clrmain drop 'destDir', without breaking 10,000 symbol
   links on \\symbols\symbols.  Redirects to 'deleteDropSymbolsAware'.

   Parameters:
     version : The build version (#####.##)
     dir     : The drop directory
     lab     : The build lab that created the drop
     
   Defaults:
     destDir : \\CLRMain\public\Drops\Whidbey\builds\ForClrSetup
     lab     : lab21

   Examples:
    runjs deleteDropForCLR 41225.00 _
    runjs deleteDropForCLR 41225.00 \\CLRMain\public\Drops\Whidbey\builds\ForClrSetup

*/
function deleteDropForCLR(version, destDir, lab) {
    //  validate parameters
    if (version == undefined) {
        logMsg(LogIntegration, LogError, "No build version to delete?");
        throwWithStackTrace(new Error(1, "Missing parameter, 'build version' in not optional"));
    }

    if (destDir == undefined) {
        destDir = "\\\\CLRMain\\public\\Drops\\Whidbey\\builds\\ForClrSetup";
        if (!FSOFolderExists(destDir + "\\" + version)) {
            logMsg(LogIntegration, LogError, "No build drop directory to delete?");
            throwWithStackTrace(new Error(1, "The drop directory, '" + destDir + "\\" + version + "' does not exist."));
        }    
    }

    if (lab == undefined)
        lab = "lab21";

    deleteDropSymbolsAware(destDir + "\\" + version, lab);

}


/***********************************************************************/
/* deletes a \\clrmain drop 'dropDir', without breaking 10,000 symbol
   links on \\symbols\symbols.

   Parameters:
     dropDir   : The destination root directory, including version (i.e. #####.##)
     lab       : The build lab that created the drop (default = "lab21s")
     symProject: The symbols server project (default = "clr"; rarely used)

   Examples:
    runjs deleteDropSymbolsAware \\CLRMain\public\Drops\Whidbey\builds\ForClrSetup\41225.00 lab21
    runjs deleteDropSymbolsAware \\clrmain\public\drops\whidbey\builds\Lab21S\50222.00
    
*/
function deleteDropSymbolsAware(dropDir, lab, symProject) {

        //  validate parameters
    if ((dropDir == undefined) || (!FSOFolderExists(dropDir))) {
        logMsg(LogIntegration, LogError, "No build drop directory to delete?");
        throw Error(1, "The drop directory, '" + dropDir + "' does not exist.");
    }

    if (lab == undefined)
        lab = "lab21s";

    if (symProject == undefined)
        symProject = "CLR";

    logMsg(LogIntegration, LogInfo10, "deleteDropSymbolsAware()\n{\n"); /* } */ 

        // 1. extract 'version' string from dropDir 
    var leafDirPattern = /\\([^\\]+)\\?$/i;
    var version = dropDir.match(leafDirPattern)[1];

        // drop the symbols
    updateSymbolsServer("delete", version, lab, dropDir, symProject);
        logMsg(LogIntegration,LogInfo, "A symbols de-indexing request for the '" + version +
                "' build has been posted to \\\\symbols\\symbols\n");

        // drop the share
        var indent = logTry();
        try {
            FSODeleteFolder(dropDir, true);
        }
        catch(e) {
            logCatch(indent);
            logMsg(LogSymbols, LogError, "Failed to delete folder '" + dropDir + "' error '" + e.description + "'\n");
        }
    
        if (FSOFolderExists(dropDir)) {
            var rdCmd = "rd /s/q \""+dropDir+"\""
            logMsg(LogIntegration, LogInfo,  "Folder '" + dropDir + "' still exists after FSODeleteFolder(), using "+rdCmd+"\n");
            var runObj = runCmdToLog(rdCmd, runSetTimeout(4*HOUR, runSetNoThrow()));
            logMsg(LogIntegration, LogInfo,  "exitcode=" + runObj.exitcode + " from "+rdCmd+"\n");
        }
    
        if (FSOFolderExists(dropDir)) {
            logMsg(LogIntegration, LogWarn,  "Folder '" + dropDir + "' needs to be deleted manually.\n");
            logMsg(LogIntegration, LogError, "Folder '" + dropDir + "' was not successfully deleted\n");
            throw Error(1, "Folder '" + dropDir + "' was not successfully deleted");
        }

    
    /* { */ logMsg(LogIntegration, LogInfo10, "} deleteDropSymbolsAware()\n");
        
	
    return 0;

}
        
        
/************************************************************************/
/* wrapper for updateVBL web page procedure - this allows me to enter the info without
/* having to deal with the  /label=value syntax and all the quoting of strings except for 
/* time stamps which have to be quoted because of a blank space (sigh).

  Parameters:
   intgType : integration type (RI|FI)
   fromLab : originating lab
   toLab: destination lab
   submitDate : date of the integration change list
   submitChangeList: integration change list
   srcDate : date of the source change list for fromLab
   srcChangeList  : sourc change list
*/
function updateVBLWebPage(intgType, fromLab, toLab, submitDate, submitChangeList, srcDate, srcChangeList)
{
	var toolDir = ScriptDir + "\\..\\..\\..\\tools\\devdiv\\"
	var cmd = toolDir + "updateprivateDB /fromlab:\"" + fromLab + "\" /tolab:\"" +
		toLab + "\" /Action:\"" + intgType + "\" /SubmitDate:\"" +
		submitDate + "\" /SubmitChange:\"" + submitChangeList +
		"\" /SyncDate:\"" + srcDate + "\" /SyncChange:\"" + srcChangeList + "\"";
//	logMsg(LogIntegration, LogInfo, cmd, "\n");
	
	var run = runCmdToLog(cmd, runSetTimeout(10000, runSetNoThrow()));
	if (run.exitCode) 
		throw new Error(-1, "updateVBLWebPage failed with error code " + run.exitCode);
	
	logMsg(LogIntegration, LogInfo10, "} updateVBLWebPage()\n");
	
	return 0;
}


/****************************************************************************/
/* former a-ctracy local runjs doRun tasks                                  */
/****************************************************************************/

var LogCTracy = logNewFacility("a-ctracy");

/****************************************************************************/
/* create a dot build, assumes links have already been created

  Parameters:
    fileList     List, one filename per line, paths are relative to 
                 binaries directory.
    fromDir      Where to get the private bits.  A UNC path to a SNAP job.

    toDir        The dot build directory pre-populated with links to a full 
                 drop.  The script will delete the links and copy the new
                 files from fromDir to toDir.

  Create Links: 
    net use z: \\clrmain\e$\public1\drops\whidbey\builds\ForClrSetup
    subst y: Z:\public1\drops\whidbey\builds\ForClrSetup
    y:
    REM e-execute l-link n-create target dir w-preserve case $-use SIS links if possible
    compdir /elnw$ 41011.00 41011.01 > compdir.log || echo error

  Create File List: 
    go bin
    dir /s/b /a-d | findstr /v binplace.log > ..\filelist.%_BuildArch%%_BuildType%.txt
    rep -i %_NTTREE%\ "" ..\filelist.%_BuildArch%%_BuildType%.txt

  example: 
    runjs /log:fso=logInfo10000 applyDotBuild dotFileList.txt \\cpvsbuild\drops\whidbey\lab21\raw\50119.00 \\clrmain\public\drops\whidbey\builds\ForClrSetup\50112.01

  dotFileList.txt
    bin\i386\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.dll
    bin\i386\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.pdb
    bin\i386\bbt\Org\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.dll
    bin\i386\bbt\Org\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.pdb
    bin\i386\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.dll
    bin\i386\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.dll.IbcMergeOrder
    bin\i386\Microsoft.VisualStudio.Modeling.ArtifactMapper.VSHost.pdb

*/

function applyDotBuild(fileList, fromDir, toDir, fake) {

    if (fake == undefined) {
        fake = false;
    }    
    if (fileList == undefined || !FSOFileExists(fileList)) {
        throw Error(-1, "missing required parameter toUpdateFile or file doesn't exist");
    }
    if (fromDir == undefined || !FSOFolderExists(fromDir)) {
        throw Error(-1, "missing required parameter fromDir or path doesn't exist");
    }
    if (toDir == undefined || !FSOFolderExists(toDir)) {
        throw Error(-1, "missing required parameter toDir or path doesn't exist");
    }

    var files = FSOReadFromFile(fileList).split("\r\n");

    var logVerbose = LogInfo;
    var notCopiedList = [];

    for (var j = 0; j < files.length; ++j) {
        if (files[j] == "")
            continue;
        
        var fromFile = fromDir + "\\" + files[j];
        var toFile = toDir + "\\" + files[j];

        if (!FSOFileExists(fromFile)) {
            logMsg(LogCTracy, LogError, "cannot copy source file "+fromFile+"\n");
            notCopiedList.push([fromFile, toDir]);
            continue;
        }
        try {
            if (FSOFileExists(toFile)) {
                if (fake)
                    logMsg(LogCTracy, logVerbose, "FSODeleteFile("+toFile+");\n");
                else
                    FSODeleteFile(toFile, true);
            }
            
            if (fake)
                logMsg(LogCTracy, logVerbose, "FSOCopyFile("+fromFile+", "+toFile+", /*overwrite*/ false);\n");
            else
                FSOCopyFile(fromFile, toFile, /*overwrite*/ false);
        }
        catch (e) {
            logMsg(LogCTracy, LogError, "exception occured ["+e.description+"] copying "+fromFile+"\n");
            notCopiedList.push([fromFile, toDir]);
            continue;
        }
    }
    if (notCopiedList.length == 0) {
        logMsg(LogCTracy, LogInfo, "applyDotBuild Successful\n");
    }
    else {
        logMsg(LogCTracy, LogInfo, "    files not copied:\n");
        
        for (var i = 0; i < notCopiedList.length; ++i) {
            logMsg(LogCTracy, LogInfo, notCopiedList[0] + " " + notCopiedList[1] + "\n");
        }
        return 1;
    }
}    


/*
    applyDotBuildFX

 Example:
    runjs /log:fso=logInfo10000 applyDotBuildFX dotFileList.txt \\clrsnapbackup2\whidbey\12641.a-ctracy\job.23515 \\clrmain\public\drops\whidbey\builds\ForClrSetup\50222.01

 Create Dot List:
    rd /s/q %_NTTREE%
    go fxsrc sys
    rbuild -c
    go bin
    dir /s/b /a-d | findstr /v binplace.log > ..\filelist.%_BuildArch%%_BuildType%.txt
    rep -i %_NTTREE%\ "" ..\filelist.%_BuildArch%%_BuildType%.txt
*/    

function applyDotBuildFX(fileList, fromDir, toDir) {

    var flavorMap = [
        ["amd64ret",      "amd64ret"],
        ["ia64ret.unopt", "ia64ret"],
        ["x86chk",        "x86chk"],
        ["x86ret",        "x86ret"]
    ];
    
    /*
    // fromSnap
    flavorMap = [
        ["amd64fre",    "amd64fre"],
        ["amd64fre",    "amd64ret"],
        ["ia64fre",     "ia64fre"],
        ["ia64fre",     "ia64ret"],
        ["x86chk",      "x86chk"],
        ["x86ret.unopt","x86fre"],
        ["x86ret.unopt","x86ret"] // dot build has FX bits, which aren't built in x86ret
    ];
    */

    if (fileList == undefined || !FSOFileExists(fileList)) {
        throw Error(-1, "missing required parameter toUpdateFile or file doesn't exist");
    }
    if (fromDir == undefined || !FSOFolderExists(fromDir)) {
        throw Error(-1, "missing required parameter fromDir or path doesn't exist");
    }
    if (toDir == undefined || !FSOFolderExists(toDir)) {
        throw Error(-1, "missing required parameter toDir or path doesn't exist");
    }

    var files = FSOReadFromFile(fileList).split("\r\n");

    var logVerbose = LogInfo;
    var notCopiedList = [];

    for (var i = 0; i < flavorMap.length; ++i) {
        
        logMsg(LogCTracy, LogInfo, flavorMap[i][0] + " => " + flavorMap[i][1] + "\n");
        var fromPath = fromDir + "\\" + flavorMap[i][0] + "\\bin"; 
        var toPath = toDir + "\\" + flavorMap[i][1] + "\\binaries";
        
        for (var j = 0; j < files.length; ++j) {
            if (files[j] == "")
                continue;
            
            var fromFile = fromPath + "\\" + files[j];
            var toFile = toPath + "\\" + files[j];

            if (!FSOFileExists(fromFile)) {
                logMsg(LogCTracy, LogError, "cannot copy source file "+fromFile+"\n");
                notCopiedList.push([fromFile, toPath]);
                continue;
            }
            try {
                if (FSOFileExists(toFile)) {
                    logMsg(LogCTracy, logVerbose, "FSODeleteFile("+toFile+");\n");
                    FSODeleteFile(toFile, true);
                }
                else {
                    logMsg(LogCTracy, LogInfo, "skipping delete of "+toFile+"\n");
                }
                logMsg(LogCTracy, logVerbose, "FSOCopyFile("+fromFile+", "+toPath+", /*overwrite*/ false);\n");
                FSOCopyFile(fromFile, toFile, /*overwrite*/ false);
            }
            catch (e) {
                logMsg(LogCTracy, LogError, "exception occured ["+e.description+"] copying "+fromFile+"\n");
                notCopiedList.push([fromFile, toPath]);
                continue;
            }
        }
    }

    if (notCopiedList.length == 0) {
        logMsg(LogCTracy, LogInfo, "applyDotBuild Successful\n");
    }
    else {
        logMsg(LogCTracy, LogInfo, "    files not copied:\n");
        
        for (var i = 0; i < notCopiedList.length; ++i) {
            logMsg(LogCTracy, LogInfo, notCopiedList[0] + " " + notCopiedList[1] + "\n");
        }
        return 1;
    }
}    


