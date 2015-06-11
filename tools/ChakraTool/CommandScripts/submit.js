/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    function executeEntryPoint(context) {
        var shell = context.shell;
        var cmdPath = Utils.GetChakraToolRoot(shell) + "\\submit\\submit.cmd";
        shell.execute(cmdPath);
    }

    TestHost.registerCommand({
        name: "submit",
        entryPoint: executeEntryPoint,
        description: "Submit a snap checkin through IECAT."
    });
})();