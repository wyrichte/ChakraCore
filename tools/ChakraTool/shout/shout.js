///
/// Shout.js
/// 
/// Main function + event handler routines
///

Environment.missingFiles = [];

// Event Handlers
function showDiff(testName, groupName, id) {
    var test = Environment.failedTests[testName];
    var group = test[groupName];
    var failure = group[id];
    
    var tempFile = TempFileHelper.createNewTempFile(failure.output, Environment.fso);

    var diffProgram = "windiff.exe";

    try {
        diffProgram = Environment.variables("SDUDIFF");
    } catch (x) {
        // Ignore if the variable doesnt exist
    }

    var commandLine = "{0} {1} {2}".format(diffProgram, failure.baseline, tempFile);

    Environment.shell.Run(commandLine);
}

function copyCommandLineToClipboard(testName, groupName, id, transformer) {
    var test = Environment.failedTests[testName];
    var group = test[groupName];
    var failure = group[id];
    
    var clipboardText = failure.commandLine;

    if (!!transformer) {
        clipboardText = transformer(failure.commandLine);
    }

    if (document.parentWindow.clipboardData.setData("text", clipboardText)) {
        alert('Copied');
    } else {
        alert('Failed to copy');
    }
    
}

function copyTestCommandline(testName, groupName, id)
{
    copyCommandLineToClipboard(testName, groupName, id);
}

function copyDebugTestCommandline(testName, groupName, id)
{
    function transformToDebugCommandLine(commandLine) {
        var tokens = commandLine.split(/\s+/);
        var indexOfJsHost = -1;
        var hasAssertBreak = false;
        var hasDebugger = false;

        for (var i = 0; i < tokens.length; i++) {
            if (indexOfJsHost == -1 && tokens[i].match(/jshost(\.exe)?/i)) {
                indexOfJsHost = i;
            }
            if (tokens[i].toLowerCase() == "-AssertBreak") {
                hasAssertBreak = true;
            }
            if (tokens[i].match(/windbg(\.exe)?/i) || tokens[i].match(/ntsd(\.exe)?/i)) {
                hasDebugger = true;
            }
        }

        if (!hasAssertBreak) {
            tokens.splice(indexOfJsHost + 1, 0, "-AssertBreak");
        }

        var transformedCommandLine = tokens.join(' ');
        if (!hasDebugger) {
            transformedCommandLine = "windbg -o " + transformedCommandLine;
        }

        return transformedCommandLine;
    }
    
    copyCommandLineToClipboard(testName, groupName, id, transformToDebugCommandLine);
}

function folderProcessingComplete(context) {
    var inProgressFilePath = Environment.fso.BuildPath(Environment.utLogPath, "_currentRun.tmp");

    if (Environment.fso.FileExists(inProgressFilePath)) {
        var file = Environment.fso.OpenTextFile(inProgressFilePath, 1, 2);
	var currentVariant = file.ReadAll().trim();

	for (var i = 0; i < Environment.missingFiles.length; i++) {
	    if (Environment.missingFiles[i] == currentVariant) {
		Environment.missingFiles[i] += " (currently running)"
	    }
	}

	var rootUtLogFolder = Environment.fso.GetFolder(Environment.utLogPath);
	var fEnum = new Enumerator(rootUtLogFolder.Files);
	for (; !fEnum.atEnd(); fEnum.moveNext()) {
	    var item = fEnum.item();
	    if (item.Name.toLowerCase() == "rl.log") {
		window.processSingleItem(context, context[currentVariant],
					 currentVariant, item);
	    }
	}

	Environment.inProgressWarning = currentVariant;
    }
    
    View.render(context);
}

function loadTestResults()
{
    var utPath = Environment.variables("SDXROOT") + "\\inetcore\\jscript\\unittest\\";
    var utLogPath = Environment.utLogPath = utPath + "\\logs\\";
    var archLogs = utLogPath + Environment.variables("_BuildArch") + Environment.variables("_BuildType");

    Environment.unitTestJsFileRegexString = escapeRegExp(" " + utPath) + "([^\\s]*)\\\\([^\\s]*)\\.js\\s*$";
    Environment.unitTestJsonFileRegexString = escapeRegExp(" " + utPath) + "([^\\s]*)\\\\([^\\s]*)\\.json\\s*$";

    debugOut(Environment.unitTestJsFileRegexString);
    Environment.unitTestJsFileRegex = new RegExp(Environment.unitTestJsFileRegexString);
    Environment.unitTestJsonFileRegex = new RegExp(Environment.unitTestJsonFileRegexString);

    var utLogFolder = Environment.fso.GetFolder(archLogs);
    if (utLogFolder == null) {
        displayError("'" + utLogFolder + "' doesn't exist");
        return;
    }

    Environment.failedTests = { 
        totalFailures: 0,
        testGroups: []
    };

    Environment.failedTests.forEachTest = function(callback) {
        var context = Environment.failedTests;
        for (var i = 0; i < context.testGroups.length; i++) {
            var testName = context.testGroups[i];
            callback(testName, context[testName], context);
        }
    }

    var e = new Enumerator(utLogFolder.SubFolders);
    processAsync(processFolder, Environment.failedTests, e, folderProcessingComplete);    
}

window.onload = function() 
{
    // Init the various subsystems
    Environment.init();
    Debug.init();
    View.init();

    loadTestResults();
}

window.onunload = function() {
    TempFileHelper.clearAllTempFiles();
}
