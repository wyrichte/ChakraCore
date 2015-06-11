/*********************************************************************************/
/*                                symbols.js                                     */
/*********************************************************************************/


/* An important part of any build process is publishing symbols to the symbols
 * server, so devs do not waste time looking for something that could just as
 * easily be posted to the ubiquitous "\\symbols\symbols" share.
 *
 * Another neat trick is embedding SD source revision numbers in .PDB files.
 * Developers are finicky, but we can tease them into debugging more issues if
 * the debugger looks up the source file versions for them.
 *
 * These are the three basic symbols tasks:
 *
 *     1. Posting symbols tp \\symbols\symbols
 *     2. Dropping symbols from \]\symbols\symbols
 *     3. Adding SD info to the unstripped symbols (separate from #1 and #2)
 *
 */

// Author: Jay Gray, SDE/T
// Date	2/14/2005
// Dependencies
//		log.js			for logging utilities
//		fso.js			for file system access
//		run.js			for run* commands

/*********************************************************************************/
var symbolsModuleDefined = 1; 		// Indicate that this module exists

// exported global variables
var SYMBOLS_CONFIG_FILENAME       = "symbols_server.ssi";
var SYMBOLS_ALL_BUILD_DIRECTORIES = /^(x86|amd64|ia64)(chk|dbg|fre|ret|cov)/;


if (!fsoModuleDefined) throw new Error(1, "Need to include fso.js");
if (!logModuleDefined) throw new Error(1, "Need to include log.js");
if (!runModuleDefined) throw new Error(1, "Need to include run.js");


if (WshShell == undefined)
	var WshShell = WScript.CreateObject("WScript.Shell");

if (Env == undefined)
	var Env = WshShell.Environment("PROCESS");

if (ScriptDir == undefined)
	var ScriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];

    // If you get an 'object expected' error, here, you did not include 'log.js'
var LogSymbols = logNewFacility("symbols");


/*********************************************************************************/
/* The function updateSymbolsServer posts or removes links to the .PDB and
   binary files, for a given build, to \\symbols\symbols.

Parameters:
    submitOrRemove : A command for the symbols server.
    fullBuildNumber : the full build number
    branch : The branch you are working in
    sourceDir : The path to the binaries and symbols to be indexed
    symSrvProject : the symbols server project name (default == "CLR")
    
The options for the 'submitOrDelete' parameter are:
    create / make -- makes an index request, but does not send it to \\symbols
    submit / add --- submits an already created request to \\symbols
    post ----------- creates and submits a symbols request in one step
    remove /delete - deletes symbols from \\symbols\symbols (*)
                        
Examples:
    These commands are equivalent, and post symbols to \\symbols\symbols:
        1) * runJS updateSymbolsServer create 50214.01 Lab21s  // ~ 5 minutes
           * runJS updateSymbolsServer submit 50214.01 Lab21s  // ~ 5 seconds
        2) * runJS updateSymbolsServer post 50214.01
        3) * runJS updateSymbolsServer post 50214.01 Lab21s
                  \\clrmain\public\drops\whidbey\builds\Lab21s\50214.01 CLR
        
    These commands are equivalent and delete symbols from \\symbols\symbols:
        1) * runJS updateSymbolsServer remove 50214.01
           
        2) * runJS updateSymbolsServer remove 50214.01 Lab21s
                  \\clrmain\public\drops\whidbey\builds\Lab21s\50214.01 CLR

Additional (critical) information:
    * Delete symbols from \\symbols\symbols before deleting a file drop.
      Removing a build before delisting the links on \\symbols\symbols, leaves
      1,000's of dead DFS links on \\symbols\symbols.
    * If someone removes a build, without cleaning up \\symbols\symbols, first,
      the links can still be removed, but you have to do it "the hard way":
          * dir \\symbols\projects\CLR\add_requests\clr_50214.01*
          * \\symbols\tools\createrequest.cmd -t CLR -d %TEMP% -y
                 -m clr_50214.01_x86chk.ssi
          * (etc., for all .ssi files listed by 'dir')
*/ 

function updateSymbolsServer(submitOrRemove, fullBuildNumber, branch, sourceDir, symSrvProject) {

        // Validate function parameters

    if (submitOrRemove == undefined) 
        throw new Error(1,
               "updateSymbolsServer : error : No 'create'/'submit'/'remove' instruction.");

    if (fullBuildNumber == undefined) 
        throw new Error(1,
               "updateSymbolsServer : error : Missing argument, build number.");

    if (branch == undefined)
       branch = "Lab21s";

    if (sourceDir == undefined) {
        sourceDir = "\\\\clrmain\\public\\drops\\whidbey\\builds" +
                    branch + "\\" + fullBuildNumber;
    }

    if (symSrvProject == undefined)
        symSrvProject = "CLR";

     var symSrvQueue   = "\\\\symbols\\projects\\" + symSrvProject;

        // validate the source and queue directories

    if (!FSOFolderExists(sourceDir)) {
        logMsg(LogSymbols, LogError,
                    "updateSymbolsServer : error : Can not find the '" +
                    sourceDir + "' directory.\n");
        throw new Error(1,
               "Can not find the '" + sourceDir + "' directory.");
    }

    if (!FSOFolderExists(symSrvQueue)){
        logMsg(LogSymbols, LogError,
                    "updateSymbolsServer : error : '"+ symSrvProject +
                    "' is not a symbols server project.\n");
        throw new Error(1,
               "'"+ symSrvProject+ "' is not a symbols server project.");
    }

    var createrequestCreate   = " -c ";
    var createrequestSubmit   = " -s ";
    var createrequestPost     = " -c -s ";
    var createrequestRemove   = " -y ";
    var createrequestAction   = "";
	
    var submitter = Env("USERNAME");
    var ssiConfigFileName = sourceDir + "\\" + SYMBOLS_CONFIG_FILENAME;

    if (submitOrRemove.match(/create|make/i)) {
       createrequestAction = createrequestCreate;
    }
    else if (submitOrRemove.match(/submit|add/i)) {
       createrequestAction = createrequestSubmit;
    }
    else if (submitOrRemove.match(/post/i)) {
       createrequestAction = createrequestPost;
    }
    else if (submitOrRemove.match(/remove|delete/i)) {
       createrequestAction = createrequestRemove;
    }
    else {
        logMsg(LogSymbols, LogError,
                    "updateSymbolsServer : error : Invalid 'submit'/'remove' " +
                    "instruction, '" + submitOrRemove + "'\n");
        throw new Error(1,
               "Invalid 'submit'/'remove' instruction, '" + submitOrRemove + "'");

    }

        // For most 'createrequest' calls, use "" to signal no "spam" status mail
    _writeSymbolsConfig (ssiConfigFileName, fullBuildNumber, symSrvProject, submitter, "");

        // look through the build directory, indexing each "SKU\binaries" subdir
    var bldDirs = FSOGetDirPattern(sourceDir, SYMBOLS_ALL_BUILD_DIRECTORIES); // x86chk, amd64ret, etc.
    var binsDir = "";
    var buildArchType = "";
    var run = "";

         // change "C:\winnt\system32\cmd.exe" (aka %COMSPEC%) into "cacls.exe"
    var caclsExe = Env("ComSpec").replace(/([^\\]*)$/, "cacls.exe");

    for (var i=0; i < bldDirs.length; i++) {

             // re-write config file to return status for the last directory
        if ((i+1) == bldDirs.length) {
            _writeSymbolsConfig (ssiConfigFileName, fullBuildNumber, symSrvProject,
                    submitter, "send status mail");
        }

        binsDir = bldDirs[i] + "\\binaries"; // the binaries and symbols are here
        run = ""; // reset results from last run

            // buildArchType = bldDirs["this"].match("leaf dir")["1st match"]
        buildArchType = bldDirs[i].match(/\\([^\\]+)\\?$/)[1];

        logMsg(LogSymbols, LogInfo, "Indexing symbols in dir '" + binsDir + 
                "' (" + buildArchType + ")\n");

        try {
            var caclsCommand = caclsExe + " " + binsDir + " /E /T /G  REDMOND\DEBUG_RO:R";

            run = runCmd(caclsCommand,
                        runSetNoThrow(
                        clrRunTemplate));
            
            var ssiName = symSrvProject + "_" + fullBuildNumber + "_" + branch + "_" +  buildArchType + ".ssi";
            var binariesSymfile = binsDir + "\\" + ssiName;
            var buildTypeSymfile = sourceDir + "\\" + ssiName;

                // do not de-index a non-indexed directory
            if ((createrequestAction == createrequestRemove) && !FSOFileExists(binariesSymfile))
                continue;
            
            _createrequest(binsDir, branch + "_" + buildArchType, createrequestAction, ssiConfigFileName);

                // copy binaries symfile to build-root, so indexed dirs are easily 'seen'
            if (!(createrequestAction == createrequestRemove)) {
                FSOCopyFile(binariesSymfile, buildTypeSymfile + ".symbols_indexed", "FORCE");
                FSOCopyFile(binariesSymfile + ".log", buildTypeSymfile + ".log", "FORCE");
            }
            else { /* (createrequestAction == createrequestRemove) */
                // for remove requests delete the .ssi 
                // and rename the 'easily seen' files
                FSODeleteFile(binariesSymfile, "FORCE");
                FSOMoveFileIfExists(buildTypeSymfile + ".symbols_indexed",
                            buildTypeSymfile + ".symbols_deleted", "FORCE");
                FSOMoveFileIfExists(buildTypeSymfile + ".log",
                            buildTypeSymfile + ".symbols_deleted.log", "FORCE");
            }

        }
        catch(e) {
            logMsg(LogSymbols, LogError, "Failed to " + submitOrRemove + " symbols from '"
                    + binsDir + "' to \\\\symbols\\symbols.\n");
            logMsg(LogScript, LogInfo, "Error "+e.number&0xFFFF+" : "+e.description+"\n");
        }
    }

    return 0;
}


/*********************************************************************************/
/* This function runs createrequest.cmd. */
function _createrequest(dirToIndex, uniqueTag, commandOptions, ssiConfigFile) {

    if (dirToIndex == undefined) {
        logMsg(LogSymbols, LogError, "The directory to index symbols for was not found. " +
                "Nothing can be indexed.\n");
        throw Error(1, "_createrequest: The directory to index symbols for was not given");
     }

    if (uniqueTag == undefined) {
        logMsg(LogSymbols, LogError, "A unique (per-build) tag is needed to create " +
                "a symbols indexing request (for ex., 'x86chk').\n");
        throw Error(1, "_createrequest: A unique string is needed for a symbols request\n");
     }

    if (commandOptions == undefined) {
        logMsg(LogSymbols, LogError, "No command-line options to symbols request. " +
                "A symbols indexing action is required.\n");
        throw Error(1, "_createrequest: No command-line parameters were specified");
    }

    if (ssiConfigFile == undefined) 
        ssiConfigFile = "";

    if (!FSOFileExists("\\\\symbols\\tools\\createrequest.cmd") ) {
        logMsg(LogSymbols, LogError, "Symbols.js: Unable to find " + 
                "\\\\symbols\\tools\\createrequest.cmd\n");
        throw Error(1, "_createrequest: Unable to find " + 
                "\\\\symbols\\tools\\createrequest.cmd");
    }

    var createrequestCall = "\\\\symbols\\tools\\createrequest.cmd";

    if (ssiConfigFile != "")
         createrequestCall += " -i " + ssiConfigFile;

    createrequestCall += " "    +commandOptions;
    createrequestCall += " -e " + uniqueTag;
    createrequestCall += " -g " + dirToIndex;
    createrequestCall += " -d " + dirToIndex;

    var run = runCmdToLog(createrequestCall, 
            runSetTimeout(1800,
            runSetNoThrow(
            runSetEnv("PATH", "\\\\symbols\\tools" + ";" + runGetEnv("PATH"),
            clrRunTemplate))));

    if (run.exitCode != 0) {
        logMsg(LogSymbols, LogError, "No symbols were indexed for '" + dirToIndex +
                "' because the  request to the symbols server failed\n");
        throw Error(1, "createrequest.cmd : error : No symbols were indexed for '" +
                dirToIndex + "'. Its return value was: ("+ run.exitCode + ")\n");
    }

	     return 0;
}


/*********************************************************************************/
/*   This function returns the contents of a symbols server indexing (SSI) file  */
function _createrequestConfigText(bldNumber, bldPhone, contacts, project,   recurse,  mail,      submitter) {

    var CRLF = "\r\n";

    var ssiConfigText = 

        "BuildId="       + bldNumber + CRLF + 
        "BuildLabPhone=" + bldPhone  + CRLF + 
        "ContactPeople=" + contacts  + CRLF +
        "Project="       + project   + CRLF +
        "Recursive="     + recurse   + CRLF +
        "StatusMail="    + mail      + CRLF + 
        "UserName="      + submitter + CRLF + 
        "";

    return ssiConfigText; 
}



/*********************************************************************************/
/*   This function creates a configuration file for "createrequest.cmd".         */
function _writeSymbolsConfig (ssiConfigFileName, fullBuildNumber, symSrvProject, submitter, sendMailToSubmitter) {

    if (FSOFileExists(ssiConfigFileName))
        FSODeleteFile(ssiConfigFileName, "FORCE");

    var statusMail = "clrgnt"; // clrgnt gets all notices, to catch error messages
    if (sendMailToSubmitter != "") {
        statusMail += ";" + submitter;
    }

          // create a file with most of the symbols server configuration settings
    FSOWriteToFile(
        _createrequestConfigText(
            fullBuildNumber,          // 'buildId' -------- build number (#####.##)
            "882-8080",               // 'BuildLabPhone' -- this number varies
            "clrgnt;vancem;dnotario;" // 'ContactPeople' -- automation owners & submitter
                    + submitter,
            symSrvProject,            // 'Project' -------- symbols server project
            "yes",                    // 'Recursive' ------ index symbols in subdirs
            statusMail,               // 'StatusMail' ----- status mail recipients
            submitter                 // 'UserName' ------- index request requestor
        ),
        ssiConfigFileName
    );
}

