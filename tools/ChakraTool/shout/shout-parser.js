///
/// Parsing layet for shout
/// Opens rl.log, parses the output into a JS object
///

(function() {
     // Regexes
     var threadLine = /^(\d+)\>/;
     var testOutOpenFailedRegex = /(?:\d+>)?Error: Unable to open file /;
     var errorLineRegex = /(?:\d+\>)?ERROR\: /;
     var outputEndLineRegex = /(?:\d+\>)?ERROR\: end of bad output file/;
     var outputLineRegex = /^(?:\d+\>)?(.*)/;
     var nameOfOutputFileRegex = /(?:\d+\>)?ERROR\: name of output file\: (.*)\\(testout\d+)\;.*/;
     var couldntDumpErrorRegex = /(\d+>)?Error: ERROR: couldn\'t open file to dump it/;
     var testFailedRegex = /(\d+\>)?ERROR: Test failed to run correctly/;
     var baselineDiffRegex = /diffs from baseline \((.*)\)\:/;
     var cmdRegex = /(?:\d+\>)?\s*(.*)\s+\>testout.* /;
     var baselineFileNameRegex = /jscript\\unittest\\(.*)\\(.*)\.baseline/;
     var summaryLineRegex = /(?:\d+\>)?Summary\: .*; (\d+) failures?$/;
     var deleteFileErrorRegex = /(\d+\>)?Error\: DeleteFileIfFound\: Unable to delete file /;

     window.processFolder = function(context, item) {
	 var name = item.Name;
         displayMessage("Processing " + name);

         var failures = { count: 0, groups: [] };
         context[name] = failures;
         context.testGroups.push(name);

         // TODO: There has to be a nicer way..
         var fEnum = new Enumerator(item.Files);
         var found = false;
         for (; !fEnum.atEnd(); fEnum.moveNext()) {
             if (fEnum.item().Name.toLowerCase() == "rl.log") { found = true; }
         }

         if (!found) {
             Environment.missingFiles.push(item.Name);
             return false;
         }

	 var name = item.Name;
         var rlLog = item.Files.Item("rl.log");
 	 window.processSingleItem(context, failures, item.Name, rlLog);
     }

     window.processSingleItem = function(context, failures, name, rlLog) {
         function addFailure(failure) {
             if (!failures[failure.group]) {
                 failures[failure.group] = [] 
                 failures.groups.push(failure.group);
             };

             debugOut("Adding " + failure.group + ", " + failure.test);
             failures[failure.group].push(failure);
             failures.count++;
             context.totalFailures++;
         }

         if (rlLog != null) {
	     var filePath = rlLog.Path;
             var file = Environment.fso.OpenTextFile(filePath, 1, 2);
             debugOut("Opening " + filePath);

             var States = { 
                 AcceptTestFailed: 1, 
                 AcceptCmd: 2, 
                 AcceptOutputFile: 3, 
                 AcceptBadOutputLine: 4, 
                 AcceptOutput: 5 
             };

             var threads = {};

             function updateStateForThread(threadNumber, line, lineNumber) {
                 if (!threads[threadNumber]) {
                     threads[threadNumber] = { 
                         currentState: States.AcceptTestFailed, 
                         previousState: -1,
                         currentFailure: {},
                         number: threadNumber
                     }
                 }

                 var thread = threads[threadNumber];
                 function updateState(state) { 
                     thread.previousState = thread.currentState;
                     thread.currentState = state;
                 }

                 // Stop processing once we've reached the summary section
                 if (summaryLineRegex.test(line)) {
                     debugOut("Summary seen- setting state to acceptFailed");
                     var m = summaryLineRegex.exec(line);
                     var failureCount = parseInt(m[1]);
                     debugOut("summary seen, failure count is " + failureCount);
                     if (failureCount != 0)
                     {
                         updateState(States.AcceptTestFailed);
                         debugOut("Previous state was " + thread.previousState + ", Current state is " + thread.currentState + ", line is " + line + "(" + lineNumber + ")");
                     }
                     else
                     {
                         debugOut("No failures in this summary- moving on");
                     }
                     return;
                 }

                 if (deleteFileErrorRegex.test(line)) {
                     debugOut("Delete file error seen- setting state to acceptFailed");
                     updateState(States.AcceptTestFailed);
                     debugOut("Previous state was " + thread.previousState + ", Current state is " + thread.currentState + ", line is " + line + "(" + lineNumber + ")");
                     return;
                 }

                 if (testOutOpenFailedRegex.test(line)) {
                     debugOut("TestOut file open error seen- setting state to acceptFailed");
                     updateState(States.AcceptTestFailed);
                     debugOut("Previous state was " + thread.previousState + ", Current state is " + thread.currentState + ", line is " + line + "(" + lineNumber + ")");
                     return;
                 }

                 debugOut("[" + thread.number + "] Previous state was " + thread.previousState + ", Current state is " + thread.currentState + ", line is " + line + "(" + lineNumber + ")");

                 if (thread.currentState == States.AcceptTestFailed) {
                     var m = baselineDiffRegex.exec(line);
                     if (m) {
                         thread.currentFailure.baseline = m[1];
                     } else {
                         thread.currentFailure.baseline = "<unknown>";
                     }
                     debugOut("handled acceptFailed, moving to acceptCmd after line " + line);

                     updateState(States.AcceptCmd);
                 } else if (thread.currentState == States.AcceptCmd) {
                     var m = cmdRegex.exec(line);
                     debugOut("Running cmdRegex on " + line)
                     thread.currentFailure.commandLine = m[1];
                     
                     debugOut("Regex is '" + Environment.unitTestJsFileRegexString +
                              "'<br />" + thread.currentFailure.commandLine);
                     
                     m = Environment.unitTestJsFileRegex.exec(thread.currentFailure.commandLine);
                     if (m) {
                         thread.currentFailure.test = m[2];
                     } else {
                         m = Environment.unitTestJsonFileRegex.exec(thread.currentFailure.commandLine);
                         thread.currentFailure.test = m[2];
                     }

                     updateState(States.AcceptOutputFile);
                 } else if (thread.currentState == States.AcceptOutputFile) {
		     // We couldn't dump the error to an output file, so
		     // RL logged that error. Since the output is lost,
		     // just set the RL message to the output since that's
		     // the best we can do, and then set the state to
		     // continue processing the log file
		     if (couldntDumpErrorRegex.test(line)) {
                         thread.currentFailure.output = line;
                         addFailure(thread.currentFailure);
                         updateState(States.AcceptTestFailed);
                         thread.currentFailure = {};			 
		     } else {
			 var m = nameOfOutputFileRegex.exec(line);
			 var dir = m[1]; 
			 var tokens = dir.split('\\');
			 thread.currentFailure.group = tokens[tokens.length - 1];
			 updateState(States.AcceptBadOutputLine);			 
		     }
                 } else if (thread.currentState == States.AcceptBadOutputLine) {
                     updateState(States.AcceptOutput);
                     thread.currentFailure.output = [];
                 } else if (thread.currentState == States.AcceptOutput) {
                     if (outputEndLineRegex.test(line)) {
                         thread.currentFailure.output = thread.currentFailure.output.join('\n');
                         addFailure(thread.currentFailure);
                         updateState(States.AcceptTestFailed);
                         thread.currentFailure = {};
                     } else {
                         var m = outputLineRegex.exec(line);
                         thread.currentFailure.output.push(m[1]);
                     }
                 } else {
                     displayError("Unexpected state: " + thread.currentState + ", previous state: " + previousState); 
                     return true;
                 }
             }

             var numLines = 0;
             var singleThreadedMode = false;

             while (!file.AtEndOfStream) {
                 var line = file.ReadLine();
                 debugOut(line + "(" + numLines + ")");

                 var m = threadLine.exec(line);
                 if (!m) { 
                     if (numLines == 0) {
                         // Assume that we're running in single-threaded mode
                         singleThreadedMode = true;
                     }
                 }

                 var threadNumber = 0;
                 if (!m) {
                     if (!singleThreadedMode) break;
                 } else {
                     threadNumber = m[1];
                 }

                 updateStateForThread(threadNumber, line, numLines);             
                 numLines++;
             }
             debugOut("Processed " + numLines + " lines");
         }

         return false;
     }
 })();