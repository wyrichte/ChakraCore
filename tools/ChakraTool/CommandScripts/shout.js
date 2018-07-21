/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

/// 
/// Implementation of the "shout" command
///
/// shout == ShoUT 
/// Displays a UI with the unit test results from the last run
///
///

(function () {
     function executeEntryPoint(context) {
         var shell = context.shell;
         var shoutPath = Utils.GetChakraToolRoot(shell) + "\\shout\\shout.hta";

         shell.execute("start " + shoutPath);
     }

    TestHost.registerCommand({
        name: "shout",
        entryPoint: executeEntryPoint,
        description: "shout == ShoUT \r\nDisplays a UI with the unit test results from the last run."
     });
}()); 