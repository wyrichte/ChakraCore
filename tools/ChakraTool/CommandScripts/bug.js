/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

/// 
/// Implementation of the "bug" command
///
/// Invokations:
///  chakra bug -id:<bug number>
///  chakra bug-list
///  chakra bug-count
///  chakra bug-team
///  chakra bug-debug -id:<bug number>
///
(function () {
     function getPowershellScriptsRoot(shell) {
         return Utils.GetChakraToolRoot(shell) + "\\powershell"; 
     }

     function getPowershellCommandLine(shell, file) {
         return "powershell -File {0}\\{1}.ps1".format(getPowershellScriptsRoot(shell), file);
     }

     function execCommand(shell, file, args, noOutput) {
         var cmd = getPowershellCommandLine(shell, file);

         if (!!args)
             cmd = cmd + " " + args;

         var result = shell.executeToObject(cmd);
         
         if (!!noOutput) {
             return result;
         }

         WScript.Echo(result);
     }

    function executeEntryPoint(context) {
        var shell = context.shell;
        var logger = context.logger;
        var config = context.config;

         var bugNumber = config.id;
         if (!bugNumber) {
             logger.logError("Bug number missing, use -id:<bug number> to specify.\n", "Command");
             return;
         }

         execCommand(shell, "bugs", "-id " + bugNumber);         
     }


     function openDumpEntryPoint(context) {
         var shell = context.shell;         
         var config = context.config;

         var id = config.id;
         if (!id) {
             logger.logError("Bug number missing, use -id:<bug number> to specify.", "Command");
             return;
         }
         var dumpId = -1;

         if (!!context.args.dumpId) {
             dumpId = parseInt(context.args.dumpId);
         }
         execCommand(shell, "opendump", "-id {0} -dumpId {1}".format(id, dumpId));
     }

     function openBugEntryPoint(context) {
         var shell = context.shell;
         var config = context.config;

         var bugNumber = config.id;
         if (!bugNumber) {
             logger.logError("Bug number missing, use -id:<bug number> to specify.", "Command");
             return;
         }
         execCommand(shell, "bugs",  " -id {0} -open".format(bugNumber));
     }

     function listBugsEntryPoint(context) {
         var shell = context.shell;

         execCommand(shell, "bugs");
     }

     function listTeamBugsEntryPoint(context) {
         var shell = context.shell;

         execCommand(shell, "bugs", " -team");
     }

     function countBugsEntryPoint(context) {
         var shell = context.shell;
         var result = execCommand(shell, "bugs", '', true /* no output */);
         
         // Subtract one to account for trailing newline
         var length = result.output.split("\n").length;
         WScript.Echo(length - 1);
     }

     TestHost.registerCommand({
         name: "bug",
         entryPoint: executeEntryPoint,
         namedEntryPoints: {
             debug: openDumpEntryPoint,
             open: openBugEntryPoint,
             list: listBugsEntryPoint,
             team:  listTeamBugsEntryPoint,
             count: countBugsEntryPoint
         },
         description: "A tool to open/list/debug/count bugs from VisualStudio Online database, ping hiteshk for any questions.",
         configDefinition: {
             id: {
                 type: "Number",
                 defaultValue: undefined,
                 description: "The id of the bug you would like to lookup."
             }
         }
     });
 }());
