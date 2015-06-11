/*********************************************************************************/
/*                              runJScript.js                                 */
/*********************************************************************************/

/* This file contains all main command parsing and help logic for runjs.wsf */

// Author: Vance Morrison
// Date 1/1/2003

/*********************************************************************************/


var LogJS = logNewFacility("runJS");

/**********************************************************************/
/* Helper function to print replay arguments correctly, e.g., string arguments
   with surrounding quotes. A subset of logCall(). */

function canonicalizeArgs(args) {

    var ret = "";
    for (var i = 0; i < args.length; i++) {
        var arg = args(i);
        var argType = typeof(arg);
        if (arg == undefined)
            ret += "_";
        else if (argType == "string" && !arg.match(/^[\w\.\\\:\-]+$/))
            ret += "\"" + arg + "\"";
        else if (argType == "object" && argType.length > 0) 
            ret += "[ARRAY]";
        else 
            ret += arg;
        if (i + 1 < args.length)
            ret += " ";
    }
    return ret;
}

/***********************************************************************/
/* This code tests try/catch/finally clauses. If you were to run 
   "runjs testTryCatchFinally", you might expect it to output something
   like this:

      00.00: Logging started at Fri Oct 31 18:42:07 PDT 2008
      00.00: TO REPLAY: runjs testTryCatchFinally

      Try clause is running...
      Try clause preparing to throw: error#1
      Catch clause caught exception: error#1
      Catch clause preparing to throw: error#2
      Finally clause is running...
      C:\dd\clr\src\ndp\clr\bin\runjs.wsf(239, 6) (null): error#2

   Unfortunately, there is a "feature" in JScript (cscript.exe) where 
   finally clauses are not guaranteed to execute if the exception is
   never caught (in this example, the catch clause is throwing an
   exception "error#2" which does not get caught). Thus, the output 
   for "runjs testTryCatchFinally" might actually look something like
   this:

      00.00: Logging started at Fri Oct 31 18:42:07 PDT 2008
      00.00: TO REPLAY: runjs testTryCatchFinally
      Try clause is running...
      Try clause preparing to throw: error#1
      Catch clause caught exception: error#1
      Catch clause preparing to throw: error#2
      C:\dd\clr\src\ndp\clr\bin\runjs.wsf(239, 6) (null): error#2

   This behavior is undesirable, since many developers assume that
   the contents of the finally clause will always be executed. To
   fix this problem, a try/catch has been added to the function 
   runJS() to guarantee that finally clauses will always be executed.
   This is now what the output should look like:

      00.00: Logging started at Fri Oct 31 18:42:07 PDT 2008
      00.00: TO REPLAY: runjs testTryCatchFinally
      Try clause is running...
      Try clause preparing to throw: error#1
      Catch clause caught exception: error#1
      Catch clause preparing to throw: error#2
      Finally clause is running...
      C:\dd\clr\src\ndp\clr\bin\runjs.wsf(239, 6) (null): error#2

*/

function testTryCatchFinally()
{
    try 
    {
        WScript.Echo("Try clause is running...");
        var ex1 = new Error(1, "error#1");
        WScript.Echo("Try clause preparing to throw: " + ex1.description);
        throw ex1;
    }
    catch(e) 
    {
        WScript.Echo("Catch clause caught exception: " + e.description);
        var ex2 = new Error(1, "error#2");
        WScript.Echo("Catch clause preparing to throw: " + ex2.description);
        throw ex2;
    }
    finally 
    {
        WScript.Echo("Finally clause is running...");
    }   
}

/***********************************************************************/
/* This script is just a generic wrapper mechanism over the real 
   operations you want done.  Basically this routine simply passes
   the command line to 'eval' for it, thus it is quite generic.
   If you set up your scripts properly, this allows you to run the
   main program of your script, but also many interesting intermediate
   places as well (you just need to be able to pass strings to it).
*/

function runJS(srcCodeArray) {

    var scriptDir  = WScript.ScriptFullName.match(/^(.*)\\/)[1];
    var args = WScript.Arguments;

    var origArgs = canonicalizeArgs(args);

    var i = logCountArgs(args);         // See if the user asked for extra logging
    var rawJS = false;
    var incFiles = [];

    // Include any local .js files in the home directory of the current computer.
    var defaultInclude = Env("HOMEDRIVE") + Env("HOMEPATH") + "\\runjs.local.js";
    if (FSOFileExists(defaultInclude)) 
        incFiles.push(defaultInclude);

    // Include any local .js files from the shared user script repository.
    defaultInclude = scriptDir + "\\scriptlib\\user\\" + Env("USERNAME") + "\\runjs.local.js";
    if (FSOFileExists(defaultInclude)) 
        incFiles.push(defaultInclude);

    while (i < args.length) {
        var arg = args(i)
        if (!arg.match(/^[\/]/))
            break;

        if (arg.match(/^\/js/i))
            rawJS = true;
        else if (arg.match(/^\/incUser:(.*)/i))
            incFiles.push(scriptDir + "\\scriptlib\\user\\" + RegExp.$1 + "\\runjs.local.js");
        else if (arg.match(/^\/inc:(.*)/i)) {
			var incFile = RegExp.$1;
			incFile = incFile.replace(/^%/, scriptDir + "\\scriptlib");
            incFiles.push(incFile);
		}
        else if (arg.match(/^\/q$/i)) {
            logSetFacilityLevel(LogJS, LogWarn);
            logSetFacilityLevel(_LogLog, LogWarn);
			logSetPrefixTime(false);
        }
        else if (arg.match(/^\/\?/i)) {
            var helpArg = undefined
            if (i+1 < args.length)  {
                helpArg = args(i+1);
                WScript.Echo("The following functions match the pattern \"" + helpArg + "\":");
            }
            else 
                runJSPrintHelpHeader();

            for (var j = 0; j < srcCodeArray.length; j++) 
                runJSPrintHelp(scriptDir + "\\" + srcCodeArray[j], helpArg);
            for (var j = 0; j < incFiles.length; j++) 
                runJSPrintHelp(incFiles[j], helpArg);
            if (helpArg == undefined) {
                WScript.Echo("");
                WScript.Echo("For help on specific functions: runjs /? <funcPat>");
            }
            return 0;
        }
        else if (!arg.match(/^\/log/))  {
            WScript.Echo("Unknown qualifier " + arg);
            return 1;
        }
        i++;
    }

    if (i >= args.length) {
        WScript.Echo("Usage: runjs <command>.  Do runjs /? for help");
        return 1;
    }

    logMsg(LogJS, LogInfo, "TO REPLAY: runjs ", origArgs, "\n");

    // include any user specified functionality
    for (var j = 0; j < incFiles.length; j++) {
        logMsg(LogJS, LogInfo, "Including: ", incFiles[j], "\n");
        var fileData = FSOReadFromFile(incFiles[j]);
        eval(fileData);
    }

    // Now that we've included all files, we can parse the logging arguments.
    logParseArgs(args);

    var commandLine = "";
    if (rawJS) {
        while(i < args.length)  {
            commandLine += args(i++) + " ";
        }
    }
    else {
        var ftn = args(i++); 
        try {
            eval(ftn);
        } catch(e) {
            WScript.Echo("Error: the function '" + ftn + "' is not defined.  Use runjs /? to get a list.");
            WScript.Echo("       Note that the function name is case sensitive.\n"); 
            return 1;
        }

        var byNameArgs = [];
        var commandLineArgs = [];

        if (i < args.Length) {
            for(;;) {
                var arg = args(i);
                if (arg == "_") 
                    commandLineArgs.push("undefined");
                else if (arg.match(/^\/(.*?)=(.*)/i)) {
                    // Named parameter value. For all arguments of the format /<name>=<value>, store
                    // the pair in byNameArgs, match the pair names after parsing all arguments.
                    var pair = {};
                    pair.name = RegExp.$1;
                    pair.value = RegExp.$2;
                    if (!(arg.match(/^(\d+)$/) || arg.match(/^['"].*["']$/))) {
                        pair.value = "'" + pair.value + "'";   // Wrap with quotes if not already quoted and not a number
                    }
                    byNameArgs.push(pair);
                }
                else if (arg.match(/^(\d+)$/) || arg.match(/^['"].*["']$/))
                    commandLineArgs.push(arg);                 // It's a number or already quoted 
                else {
                    commandLineArgs.push("'" + arg.replace(/\\/g, "\\\\") + "'");    // It's a string, quote it
                }
                i++;
                if (i >= args.Length)
                    break;
            }
        }

        if (byNameArgs.length > 0) {
            // There are by-name parameters. First extract the function
            // argument names (using the same technique as the help functionality below) and for
            // each match substitute the argument value at that position (if one has not already
            // been supplied; otherwise throw an error).
            // Note: there does not appear to be a way of extract parameter names from the
            // jscript function object itself, otherwise that would be the preferred approach.
            var ftnSig = null;
            for (var j = 0; j < srcCodeArray.length && ftnSig == null; j++) {
                ftnSig = _findFunctionSignature(scriptDir + "\\" + srcCodeArray[j], ftn);
            }
            for (var j = 0; j < incFiles.length && ftnSig == null; j++) {
                ftnSig = _findFunctionSignature(incFiles[j], ftn);
            }
            if (ftnSig == null) {
                throw new Error("Cannot find signature for function '" + ftn + "'.");
            }

            // Flesh out the array of arguments to make sure all array indexes are valid.
            while (commandLineArgs.length < ftnSig.args.length) {
                commandLineArgs.push("undefined");
            }

            // For each named parameter, iterate each parameter name, and when a match is found
            // substitute the pair's value if the current value is undefined; if a value already
            // exists or the named parameter is not found throw an error.
            for (var j = 0; j < byNameArgs.length; ++j) {
                var argNameRE = new RegExp(byNameArgs[j].name, "i");
                for (var k = 0; k < ftnSig.args.length; ++k) {
                    if (ftnSig.args[k].match(argNameRE)) {
                        if (commandLineArgs[k] != "undefined") {
                            logMsg(LogJS, LogWarn, 
                                "Value for parameter '" + ftnSig.args[k] + "' already provided; by name parameter value " +
                                byNameArgs[j].value + " will be ignored.\n");
                        }
                        else {
                            // Found a matching parameter, set the value.
                            commandLineArgs[k] = byNameArgs[j].value;
                        }
                        break;
                    }
                }
                if (k == ftnSig.args.length) {
                    throw new Error("Parameter '" + byNameArgs[j].name + "' not found.");
                }
            }
        }
        commandLine = ftn + "(" + commandLineArgs.join(",") + ")";
    }

    var start = (new Date()).getTime();

    try {
        ret = eval(commandLine);
    } catch (e) {
        throw e;
    }
    var durationMin = ((new Date()).getTime() - start) / 60000;

    if (typeof(ret) != "number") {
        if (typeof(ret) != "string")
            ret = dump(ret, 1);
        logMsg(LogJS, LogInfo, "runJS eval returns: ", ret, " assuming success, duration = ", durationMin.toFixed(2), " min\n");
        ret = 0;
    }
    if (ret != 0)
        logMsg(LogJS, LogWarn, "runJS ret: ", ret, " duration = ", durationMin.toFixed(2), " min\n");
    return ret;
}


/*****************************************************************************************/
function runJSPrintHelpHeader() {

    WScript.Echo("RunJS: Run a JScript function from the command line");
    WScript.Echo("     RunJS is infrastructure that allows JScript to be used as a replacement");
    WScript.Echo("     for command scripts.  Given a command line which specifies the function to");
    WScript.Echo("     call and the arguments to pass, RunJS invokes the specified function.");
    WScript.Echo("");
    WScript.Echo("           functionName arg1 arg2 ... ");
    WScript.Echo("");
    WScript.Echo("     The arguments can be numbers, quoted strings, or simply words (which are ");
    WScript.Echo("     implicitly quoted).  The symbol '_' is treated as an undefined value.");
    WScript.Echo("");
    WScript.Echo("     There is a logging facility that can be used to print out detailed");
    WScript.Echo("     information about what the script is doing.  Some useful facilities ");
    WScript.Echo("     include 'run', 'proc', 'CLRAutomation', 'fso' (file system object). ");
    WScript.Echo("     The default level is 3 and generally each level and each level increases");
    WScript.Echo("     the amount of data by a factor of 10.");
    WScript.Echo("");
    WScript.Echo("     In addition to the built in functionality, runjs also looks for a file in");
    WScript.Echo("     %HOMEPATH%\\runjs.local.js and scriptlib\\user\\<user>\\runjs.local.js for");
    WScript.Echo("     user specific scripts.");
    WScript.Echo("");
    WScript.Echo("Qualifiers:");
    WScript.Echo("          /?                  Print this help.");
    WScript.Echo("          /? <ftnPat>         Print help about a particular function. ftnPat");
    WScript.Echo("                              can be any regular expression");
    WScript.Echo("          /inc:<file>         Include <file> as JScript source.  Additional");
    WScript.Echo("                              functionality can be included in this way");
    WScript.Echo("                              a leading % in <file> is the scriptlib dir");
    WScript.Echo("          /incUser:<user>     Include scriptlib\\user\\<user>\\runjs.local.js");
    WScript.Echo("                              (another user's local script)");
    WScript.Echo("                              functionality can be included in this way");
    WScript.Echo("          /q                  quiet: don't print function called or ret val");
    WScript.Echo("          /js                 JScript syntax.  The rest of the line is");
    WScript.Echo("                              passed to the JScript evaluator unaltered");
    WScript.Echo("          /logT(ranscript):<file> Send logging messages to 'file' as well");
    WScript.Echo("          /log:<pat>=<level>  Set logging for facility matching pat to level");
    WScript.Echo("                              <pat> can be a regular expression");
    WScript.Echo("");
    WScript.Echo("Arguments:\n" +
                 "          /<param>=<value>    Provide function argument value by name, where \n" +
                 "                              <value> is the value to supply for parameter\n" + 
                 "                              <param>. Parameter values supplied explicitly will\n" + 
                 "                              override those supplied by name.\n\n" +
                 "                              Example: 'runjs razzleBuild /bldArch=amd64'");
    WScript.Echo("");
}


/*****************************************************************************************/
/* Implements help functionality.  Scrapes the source code 'srcFileName' for things to output */
  
function runJSPrintHelp(srcFileName, ftn) {

    if (!FSOFileExists(srcFileName))
        return;
    var srcFileData = FSOReadFromFile(srcFileName);
    var ftnPat = undefined;
    if (ftn != undefined) 
        ftnPat = new RegExp(ftn, "i");

    var output = "";

            // match a function body, and the comment before it */
    while (srcFileData.match(/(\/\*\s+(([^*]|\*[^\/])*?)\s+\*\/)?\s*function\s*(\w+)\s*(\(.*?\))\s*{((.|\n)*?)\n}/)) {
        var before = RegExp.leftContext
        var comment = RegExp.$2;
        var ftnName = RegExp.$4;
        var ftnSig = RegExp.$5;
        var ftnBody = RegExp.$6;
        srcFileData = RegExp.rightContext;

        ftnSig = ftnSig.replace(/, */g, ",");       // compress spaces

            // See if there is header information before the comment of the form /**** HEADER ****/
        if (ftn == undefined && before.match(/\/\*\*\*+\s+(.*)\s+\*+\//)) {
            var header = RegExp.$1;
            output += header + "\r\n";
        }

        if (ftnName.match(/^_/))
            continue;

        if (ftnPat == undefined)
            output += "     " + ftnName + ftnSig + "\r\n";
        else if(ftnName.match(ftnPat)) {
            output += ftnName + ftnSig + "\r\n";
            comment = comment.replace(/^\s?\s?\s?/, "   ");
            comment = comment.replace(/^\t/gm, "    ");
            if (!comment.match(/^\s*$/))
                output += comment + "\r\n";

            var defaults = "";
            for (;;) {
                    // Is it code that sets the default value
                if (ftnBody.match(/^\s*if\s*\(\s*(\w+)\s*==\s*undefined\s*\)\s*(\w+)\s*=\s*(.*?);/) ||
                    ftnBody.match(/^\s*if\s*\(\s*(\w+)\s*==\s*undefined\s*\)\s*{\s*(\w+)\s*=\s*(.*?);(.|\n)*?}/)) {
                    var condVar = RegExp.$1;
                    var assignVar = RegExp.$2;
                    var defValue = RegExp.$3;
                    ftnBody = RegExp.rightContext;

                    if (condVar == assignVar) {
                        defValue = defValue.replace(/\\\\/g, "\\");
                        defaults += "       " + condVar + " = " + defValue + "\n";
                    }
                }
                    // Is it code that throws on a required value?
                else if (ftnBody.match(/^\s*if\s*\(\s*(\w+)\s*==\s*undefined\s*\)\s*throw.*?;/) ||
                         ftnBody.match(/^\s*if\s*\(\s*(\w+)\s*==\s*undefined\s*\)\s*{\s*throw.*?;(.|\n)*?}/)) {
                    ftnBody = RegExp.rightContext;
                }
                else 
                    break;
            }
            if (defaults != "")
                output += "\r\n     Default parameter values: (runjs shorthand '_')\r\n";
            output += defaults + "\r\n";
        }
    }

    if (output != "") {
        WScript.Echo("-------------------------------------------------------------------------------");
        WScript.Echo("Functionality in " + srcFileName);
        WScript.Echo("-------------------------------------------------------------------------------");
        WScript.Echo(output);
    }
}

function _findFunctionSignature(srcFileName, ftnName) {
    if (!FSOFileExists(srcFileName))
        return;

    var srcFileData = FSOReadFromFile(srcFileName);

    // match a function body, and the comment before it */
    while (srcFileData.match(/function\s*(\w+)\s*\((.*?)\)/)) {
        var ftnSig = {};
        srcFileData = RegExp.rightContext;
        ftnSig.name = RegExp.$1;
        ftnSig.args = RegExp.$2.replace(/, */g, ",").split(",");

        if (ftnName.match(ftnSig.name)) {
            return ftnSig;
        }
    }

}

