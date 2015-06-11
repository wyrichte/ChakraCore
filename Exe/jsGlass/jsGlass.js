// Copyright (C) 2009 by Microsoft Corporation.  All rights reserved.

// jsGlass testing harness / Dev scaffolding
//
// This is a simple interpreter that
// uses JSON to record a debugging session.
// The recording can be played back to validate that
// another run has results that meet the validation rules.

// Usage Scenario #1: Command Line debugger
// Usage Scenario #2: Regression test harness

// JsGlass  relies on being run from within a native
// host that provides access to IActiveScriptDebugging and other
// primitives via the 'Eze' global Javascript object.

// enclose everything in a function scope so nothing bleeds back 
// into the global namespace. 

(function () {

    //////////////////////////////////////////////////////////
    // The follow allows us to use both cscript and exe hosts

    var console = {
        stdIn: { ReadLine: Eze.ReadLine },
        stdOut: { Write: function (x) { Eze.Write(x, "stdOut"); } },

        // redirect stdErr to std Out so errors break the baseline
        stdErr: { Write: function (x) { Eze.Write(x, "stdOut"); } },

        // if I have rebase defined here and replace it later the version here gets used instead
        // i believe that may indicate a bug in Eze .. but I'm not sure: javascript is sometimes surprising.
        //                    rebase: {Write: function(x) { Eze.Write(x, "stdOut"); }},
        Arguments: { Length: Eze.ArgumentLength,
            Item: Eze.ArgumentItem
        }
    };

    // Eze always throws even if this is wrapped in a try/catch ..
    // avoiding debugging the issue for now...
    // This blocks making the jgGlass.js work in cscript w/o modification
    // (however the debugger commands will do that in time ... so maybe it doesn't matter?
    //filesys = new ActiveXObject("Scripting.FileSystemObject");

    var filesys = {
        FileExists: Eze.FileExists,
        OpenTextFile: Eze.OpenTextFile
    };

    // I'm exposing everything directly on the Eze object and moving
    // them around to other objects in javascript. It's easier to play with getting
    // them in the right location for consumption this way.
    Eze.Debug = {
        OpenScript: Eze.OpenScript,
        SetBreakpoint: Eze.SetBreakpoint,
        RunScript: Eze.RunScript,
        ResumeScript: Eze.ResumeScript,
        GetLocation: Eze.GetLocation,
        GetCallstack: Eze.GetCallstack,
        GetLocals: Eze.GetLocals,
        EvaluateExpr: Eze.EvaluateExpr,
        EditLocalValue: Eze.EditLocalValue,
        EnableFirstChanceException: Eze.EnableFirstChanceException
    };

    Eze.Authoring = {
        GetTokenRanges: Eze.GetTokenRanges,
        GetRegions: Eze.GetRegions,
        GetCompletions: Eze.GetCompletions,
        GetErrors: Eze.GetErrors,
        GetAst: Eze.GetAst,
        GetQuickInfo: Eze.GetQuickInfo,
        ProcessCompletionsSession: Eze.ProcessCompletionsSession,
        GetFunctionHelp: Eze.GetFunctionHelp,
        SplatterSession: Eze.SplatterSession,
        MultipleHostTypeCompletion: Eze.MultipleHostTypeCompletion
    };

    /////////////////////////////////////////////////////////

    /*  enable this definition of Assert to reduce assert overhead // (Check: does Eze inline and realize No-op???)
    function  assertMsg(){}
    // */

    // /*  enable this definition of AssertMsg for extra validation
    function assertMsg(expr, msg) {
        if (!expr) {
            console.stdErr("assert: " + msg + "\n");
            debugger;
        }
    }
    // */

    // non-interactive command handling
    function CommandArrayIter(cmdArray) {
        var i = -1;
        var array = cmdArray;
        function moveNext() {
            i++;
            return i < array.length;
        }
        function current() {
            return array[i];
        }
        return { moveNext: moveNext,
            current: current
        };
    }

    // interactive command handling
    var interactiveCmds = (function (console) {
        var line;
        function moveNext() {
            console.stdOut.Write("\n>");
            line = console.stdIn.ReadLine();

            return line !== "quit";
        }
        function getInteractiveCmd() {
            var tokens = line.split(" ");
            return { action: tokens[0], args: tokens.slice(1) }
        }
        return { moveNext: moveNext,
            current: getInteractiveCmd
        };
    })(console);

    function extend(base, extension) {
        var NewObj = function () { };
        NewObj.prototype = base;
        var extended = new NewObj();

        for (name in extension) {
            assertMsg(typeof extension[name] === "function", "Exension cannot have data members");
            extended[name] = extension[name];
        }
        return extended;
    }

    var interpreter = (function () {

        var dispatchBase = (function (base) {
            // baseline interactive commands


            function loadPkg(session, args) {
                // Attach a cmdpkg to the interpreter
                // args[0] should be a jscript file
                //var cmdPkg = args[0];
                //session.dispatch = cmdPkg(session.dispatch);
            }
            loadPkg.helpMsg = "Not Implemented";


            function helpGeneral(that, session, args) {

                function displayName(name, length) {
                    while (name.length < length) {
                        name += " ";
                    }
                    return name;
                }

                var commands = [];
                var length = 0;
                if (console) {
                    for (name in that) {
                        commands.push(name);
                        length = length < name.length ? name.length : length;
                    }
                    commands.sort();

                    for (var i = 0; i < commands.length; i++) {
                        console.stdOut.Write(displayName(commands[i], length) + ": " + that[commands[i]].helpMsg + "\n");
                    }
                }
            }

            function helpSpecific(session, args) {
                if (this[args[0]]) {
                    if (this[args[0]].specificHelpMsg) {
                        console.stdOut.Write(this[args[0]].specificHelpMsg + "\n");
                    } else if (this[args[0]].helpMsg) {
                        console.stdOut.Write(this[args[0]].helpMsg + "\n");
                    }
                }
            }

            function help(session, args) {
                if (args.length < 1) {
                    // Hack to remove use of apply which appears to be busted in Eze
                    //helpGeneral.apply(this, [session, args]);
                    helpGeneral(this, session, args);
                }
                else {
                    helpSpecific.apply(this, [session, args]);
                }
            }
            help.helpMsg = "Get information on commands.";
            help.specificHelpMsg = "help [command]";


            var _previousCommands = null;
            function interact(session, args) {
                // swap to the interactive command line if it exists
                _previousCommands = session.commands;
                session.commands = interactiveCmds;
                console.stdOut.Write("Entering interactive mode:");
            }
            interact.helpMsg = "Switch to interactive mode.  Helpful for debugging tests.";


            function resume(session, args) {
                // resume the non-interactive commands if it exists
                if (_previousCommands) {
                    session.commands = _previousCommands;
                    _previousCommands = null;
                }
            }
            resume.helpMsg = "Continue non-interactive commands, after use of the interact command.";


            function quit(session, args) {
                Eze.Quit();
                // do quit command
            }
            quit.helpMsg = "exit jsGlass";

            function echo(session, args) {
                console.stdOut.Write(args[0]);
            }
            echo.helpMsg = "prints arg to the console window";

            return extend(base,
                           { help: help,
                               interact: interact,
                               resume: resume,
                               quit: quit,
                               echo: echo
                           });
        })({});

        var dispatchDebugger = (function (base) {
            var _target;

            // Debugger based commands
            var _testCode;

            function open(session, args) {
                if (args.length < 1) {
                    this.help(session, ["open"]);
                    return;
                }
                var forReading = "rb";

                //TODO debug why the FileSystemObject doesn't work..
                //var filesys = new ActiveXObject("Scripting.FileSystemObject");
                // For now use my hacked filesys object
                var file = filesys.FileExists(session.defaultDir + args[0]);
                file = file ? file : filesys.FileExists(args[0]);
                var fileStream = filesys.OpenTextFile(file, forReading);
                var _testCode = fileStream.ReadAll();
                _target = args[0];

                // TODO setup an ActiveDebug Session for this script
                // TODO: TEMP TEMP TEMP
                Eze.Debug.OpenScript(_testCode, args[0]);
            }
            open.helpMsg = "Open a jscript file to debug.";

            function setBp(session, args) {
                // Todo .. line,offset rather than offset?
                // TODO .. resolve file! rather than have a single implicit file
                // TODO factor the debugger aspects to an Eze.debugger object
                // rather than having them on Eze

                // SDK ISSUE: args[1] is a Number ... but I have little control over
                // how it gets presented to Native..
                // Is there any way to make it easier from this side..?
                // Should I force it to be a string?
                if (args.length < 2) {
                    this.help(session, ["setBp"]);
                    return;
                }
                Eze.Debug.SetBreakpoint(args[0], args[1].toString());
            }
            setBp.helpMsg = "Set a breakpoint at the specified character offset.";
            setBp.specificHelpMsg = "setBp file charOffset";

            function enableFirstChance(session, args) {
                // This will enable the first chance exception in the script.
                Eze.Debug.EnableFirstChanceException(args[0]);
            }
            enableFirstChance.helpMsg = "Enable/Disable first chance exception in the script";

            function launch(session, args) {
                //TODO: combine launch/go for amore usable cmd line debugger?
                // or leave separate for orthogonal test surface?
                Eze.Debug.RunScript();
            }
            launch.helpMsg = "Begin debug execution of a script";

            function go(session, args) {
                if (args[0]) {
                    Eze.Debug.ResumeScript("continue", args[0]);
                }
                else {
                    // TODO differentiate 1st run from continue
                    Eze.Debug.ResumeScript("continue");
                }
            }
            go.helpMsg = "Run the previous opened jscript file, or continue from a stop or breakpoint.";


            function step(session, args) {
                var steptype = "over";
                if (args[0]) {
                    steptype = args[0];
                }
                Eze.Debug.ResumeScript(steptype);
            }
            step.helpMsg = "step [over*,in,out]";

            function locals(session, args) {
                var expandLevel = "0";
                if (args[0]) {
                    expandLevel = args[0].toString();
                }
                // do locals command
                Eze.Debug.GetLocals(expandLevel);
            }
            locals.helpMsg = "List the locals for the current frame.";

            function evaluate(session, args) {
                // do locals command
                if (args.length < 1) {
                    this.help(session, ["evaluate, expression, depth, checkorder"]);
                }
                else if (args.length == 1) {
                    Eze.Debug.EvaluateExpr(args[0], "0", false);
                }
                else if (args.length == 2) {
                    Eze.Debug.EvaluateExpr(args[0], args[1].toString(), false);
                }
                else if (args.length == 3) {
                    Eze.Debug.EvaluateExpr(args[0], args[1].toString(), args[2]);
                }
            }

            evaluate.helpMsg = "Evaluate passed expression as it is done in watch/console window";

            function editLocal(session, args) {
                // do locals command
                if (args.length < 2) {
                    this.help(session, ["editLocal , rootnode, childnode, value"]);
                }
                else if (args.length == 2) {
                    Eze.Debug.EditLocalValue(args[0], "", args[1]);
                }
                else if (args.length == 3) {
                    Eze.Debug.EditLocalValue(args[0], args[1], args[2]);
                }
            }

            editLocal.helpMsg = "Edit a local, provided as argument, with some value";

            function location(session, args) {
                Eze.Debug.GetLocation();
            }
            location.helpMsg = "Get the current location";

            function callstack(session, args) {
                Eze.Debug.GetCallstack();
            }
            callstack.helpMsg = "Get the current callstack";

            return extend(base,
                           { open: open,
                               setBp: setBp,
                               enableFirstChance: enableFirstChance,
                               launch: launch,
                               go: go,
                               step: step,
                               location: location,
                               callstack: callstack,
                               locals: locals,
                               evaluate: evaluate,
                               editLocal: editLocal
                           });
        })(dispatchBase);

        var dispatchAuthoring = (function (base) {

            function callAuthoringMethod(session, args, name, f) {
                if (args.length < 1) {
                    this.help(session, [name]);
                }
                var forReading = "rb";

                var file = filesys.FileExists(session.defaultDir + args[0]);
                file = file ? file : filesys.FileExits(args[0]);
                var fileStream = filesys.OpenTextFile(file, forReading);
                var textCode = fileStream.ReadAll();

                f(textCode, session.defaultDir);
            }

            function callMultiFileAuthoringMethod(session, args, name, f) {
                if (args.length < 1)
                    this.help(session, [name]);
                var forReading = "rb";
                var files = [];
                for (var i = 0; i < args.length; i++) {
                    var file = filesys.FileExists(session.defaultDir + args[i]) || filesys.FileExists(args[i]);
                    var fileStream = filesys.OpenTextFile(file, forReading);
                    files.push(fileStream.ReadAll());
                }
                f(files.length, files[0], files[1], files[2], files[3], files[4], files[5], files[6], files[7], files[8], files[9]);
            }

            function getTokenRanges(session, args) {
                callAuthoringMethod(session, args, "getTokenRanges", Eze.Authoring.GetTokenRanges);
            }
            getTokenRanges.helpMsg = "Get the text color range information for the javascript file";

            function getRegions(session, args) {
                callAuthoringMethod(session, args, "getRegions", Eze.Authoring.GetRegions);
            }
            getRegions.helpMsg = "Get the locations of all the methods in the javascript file";

            function getCompletions(session, args) {
                callMultiFileAuthoringMethod(session, args, "getCompletions", Eze.Authoring.GetCompletions);
            }
            getCompletions.helpMsg = "Get the completions for all '|' characters in the source";

            function getErrors(session, args) {
                callAuthoringMethod(session, args, "getErrors", Eze.Authoring.GetErrors);
            }
            getErrors.helpMsg = "Get the errors (if any) in the source";

            function getAst(session, args) {
                callAuthoringMethod(session, args, "getAst", Eze.Authoring.GetAst);
            }
            getAst.helpMsg = "Get the ast of the source as JSON";

            function getQuickInfo(session, args) {
                callAuthoringMethod(session, args, "getQuickInfo", Eze.Authoring.GetQuickInfo);
            }
            getQuickInfo.helpMsg = "Get the ast quick info for symbols marked with a |";

            function processCompletionsSession(session, args) {
                callAuthoringMethod(session, args, "processCompletionsSession", Eze.Authoring.ProcessCompletionsSession);
            }
            processCompletionsSession.helpMsg = "Process a script driven completions session";

            function getFunctionHelp(session, args) {
                callMultiFileAuthoringMethod(session, args, "getFunctionHelp", Eze.Authoring.GetFunctionHelp);
            }
            getFunctionHelp.helpMsg = "Get the function help for all '|' characters in the source";

            function splatterSession(session, args) {
                callAuthoringMethod(session, args, "splatterSession", Eze.Authoring.SplatterSession);
            }
            splatterSession.helpMsg = "Splatter completions requests and modifications all over the given source";
            return extend(base,
                            {
                                getTokenRanges: getTokenRanges,
                                getRegions: getRegions,
                                getCompletions: getCompletions,
                                getErrors: getErrors,
                                getAst: getAst,
                                getQuickInfo: getQuickInfo,
                                processCompletionsSession: processCompletionsSession,
                                getFunctionHelp: getFunctionHelp,
                                splatterSession: splatterSession,
                                multipleHostTypeCompletion: multipleHostTypeCompletion
                            });
            function multipleHostTypeCompletion(session, args) {
                callMultiFileAuthoringMethod(session, args, "multipleHostTypeCompletion", Eze.Authoring.MultipleHostTypeCompletion);
            }
            multipleHostTypeCompletion.helpMsg = "Get the completions for all '|' characters in the source for all supported host types";

        })(dispatchDebugger);

        var _currentRecordObj = { "action": "",
            "args": [],
            "events": []
        };
        var _betweenCommands = "";

        function printObjectErr(error) {
            console.stdErr.Write(JSON.stringify(error));
        }


        function recordCommandStart(command) {
            _currentRecordObj["action"] = command.action;
            _currentRecordObj["args"] = command.args;
        }

        function recordCommandEnd() {
            //TODO pass real results
            console.rebase.Write(_betweenCommands);
            _betweenCommands = ",\n";
            console.rebase.Write(JSON.stringify(_currentRecordObj,
                                                function (x, y) { return y; },
                                                2));
            _currentRecordObj = { "action": "",
                "args": [],
                "events": []
            };
        }

        function recordEvent() {
            var arrayArgs = [];
            for (var i = 0; i < arguments.length; i++) {
                arrayArgs[i] = arguments[i];
            }
            _currentRecordObj["events"] = arrayArgs;

        }

        function emitTestHeader(header) {
            console.rebase.Write("{\n");
            for (name in header) {
                if (name !== "validated") {
                    console.rebase.Write("  \"" + name + "\" : " + JSON.stringify(header[name]) + ",\n");
                }
                else {
                    console.rebase.Write("  \"validated\" : false,\n");
                }
            }
            console.rebase.Write("  \"commands\" :\n  [\n");
        }
        function emitTestFooter() {
            console.rebase.Write("\n  ]\n}\n");
            console.stdOut.Write("Finished. If the test failed compare <test>.json to <test>.json.rebase\n");
        }
        return { dispatch: dispatchAuthoring,
            recordEvent: recordEvent,
            recordError: printObjectErr,
            recordCommandStart: recordCommandStart,
            recordCommandEnd: recordCommandEnd,
            emitTestHeader: emitTestHeader,
            emitTestFooter: emitTestFooter
        };
    })();


    function glassRunner(session) {
        assertMsg(session.commands, "No commands provided.");

        while (session.commands.moveNext()) {
            (function (command) {

                function matchEvent(e1, e2) {
                    if (typeof e1 !== "object") {
                        return e1 === e2;
                    }
                    for (name in e1) {
                        if (e2[name] === undefined) {
                            return false;
                        }
                        else if (!matchEvent(e1[name], e2[name])) {
                            return false;
                        }
                    }
                    for (name in e2)
                        if (e1[name] === undefined)
                            return false;
                    return true;
                }

                function validateExpectedEvent(event) {
                    for (var i = 0; i < _expectedEvents.length; i++) {
                        if (matchEvent(event, _expectedEvents[i])) {
                            _expectedEvents[i] = null;
                            return true;
                        }
                    }
                    return false;
                }
                var _validateExpectedEvent = command.events ? validateExpectedEvent : function () { return true; };

                // Events from the current commands will be dispatched here
                // In interactive mode we capture the events for the next
                // 150ms as a Heuristic ... 
                function eventCallback(eventJSON) {
                    var event = JSON.parse(eventJSON);

                    if (!_validateExpectedEvent(event)) {
                        session.recordError({ UnexpectedEvent: event });
                        console.stdOut.Write("\n");
                    }
                    session.recordEvent(event)
                }

                function eventsExist(events) {
                    for (var i = 0; i < events.length; i++) {
                        if (events[i] != null) {
                            return true;
                        }
                    }
                    return false;
                }

                var _expectedEvents = command.events;

                // Set the callback Eze uses for debugger events
                Eze.callback = eventCallback;

                if (session.dispatch[command.action]) {

                    session.recordCommandStart(command);

                    if (interpreter.timeout) {
                        var timeoutStart = new Date();
                    }

                    session.dispatch[command.action](session, command.args);

                    if (!_expectedEvents) {
                        // In interactive mode pump some messages to allow
                        // for events to arrive
                        var interactive_start = new Date();
                        do {
                            Eze.PumpMessages();
                            var interactive_interval = new Date() - interactive_start;
                        }
                        while (interactive_interval < 150);
                    }
                    while (_expectedEvents && eventsExist(_expectedEvents)) {
                        Eze.PumpMessages();
                        if (interpreter.timeout) {
                            var interval = new Date() - timeoutStart;
                            if (interval > interpreter.timeout) {
                                session.recordError({ "Timeout": interval, "Command": command });
                                session.recordCommandEnd();
                                // Set interpreter timeout to something small since we already failed
                                interpreter.timeout = 150;
                                return;
                            }
                        }
                    }
                    session.recordCommandEnd();
                }
                else {
                    session.recordError({ missingCommand: command });
                }

            })(session.commands.current());
        } // ends the while
    } // ends glassRunner

    var runAuthoringTest = false;
    var targetHost = "";
    var pdmPath = "";

    var test = null;
    if (console.Arguments.Length > 0) {
        var forReading = "rb";
        // We don't currently process any jscript arguments
        // but just work around this by detecting the first json file
        var filename;
        var fileStream;
        for (var i = 0; i < console.Arguments.Length; i++) {
            filename = console.Arguments.Item(i);
            fileStream = filesys.OpenTextFile(filename, forReading);
            if (fileStream != undefined) {
                break;
            }
        }

        // I wanted to use "Scripting.FileSystemObject, but the resulting filesys
        // object failed on OpenTextFile. I did a workaround filesys to move on.
        // I didn't bother exposing the same interface...  mine is a thin wrapper over fopen.
        //var filesys = new ActiveXObject("Scripting.FileSystemObject");

        var testStr = fileStream.ReadAll();

        var defaultDir = function (path) {
            var dirs = path.split("\\");
            var newpath = "";
            for (var i = 0; i < dirs.length - 1; i++) {
                newpath += dirs[i] + "\\";
            }
            return newpath;
        } (filename);

        test = JSON.parse(testStr);

        var rebasefile = filename + ".rebase";
        var forWriting = "w";
        var rebaseStream = filesys.OpenTextFile(rebasefile, forWriting);

        console.rebase = { Write: function (x) { Eze.Write(x, rebaseStream); } };

        // Determine the right Target JScript*.dll and PDM paths:
        if (typeof test.binRootEnv === "string" && typeof test.targetHost === "string") {
            var rootBin = Eze.GetEnvironmentVariable(test.binRootEnv);
            if (rootBin) {

                if (filesys.FileExists(rootBin + "\\" + test.targetHost)) {
                    targetHost = filesys.FileExists(rootBin + "\\" + test.targetHost);
                }

                // We should use a default pdm
                // for the case where the dev did not do a full IE build:

                if (filesys.FileExists(rootBin + "\\" + "pdm.dll")) {

                    // If the dev did a full IE build, use the matching pdm:

                    pdmPath = filesys.FileExists(rootBin + "\\" + "pdm.dll");
                }
            }
        }

        // If there is no envrionment set, try to find out if there is any .local redirection.
        if (pdmPath == "" || targetHost == "") {
            var localFolderPath = Eze.GetLocalFolderPath();
            if (localFolderPath) {
                if (pdmPath == "") {
                    pdmPath = filesys.FileExists(localFolderPath + "\\" + "pdm.dll");
                }
                if (targetHost == "") {
                    var file = "\\" + test.targetHost;
                    targetHost = filesys.FileExists(localFolderPath + file);
                }
            }
        }

        if (typeof test.runAuthoringTest == "boolean") {
            runAuthoringTest = test.runAuthoringTest;
        }

        if (typeof test.version == "number") {
            Eze.SetVersion(test.version);
        }
    }

    if (runAuthoringTest) {
        Eze.CreateAuthoringEngine(targetHost);
    }
    else {
        Eze.StartTargetHost(targetHost, pdmPath);
    }

    if (test) {
        var commands = test.commands;
        delete test.commands;
        interpreter.emitTestHeader(test);
        test.commands = commands;
        interpreter.commands = CommandArrayIter(test.commands);
        interpreter.timeout = test.timeout;
        interpreter.defaultDir = defaultDir;

    }
    else {
        console.rebase = { Write: function (x) { Eze.Write(x, "stdOut"); } };
        interpreter.emitTestHeader({ testname: "-",
            owner: "-",
            validated: false,
            timeout: 1000
        });
        interpreter.commands = interactiveCmds;
    }
    glassRunner(interpreter);
    interpreter.emitTestFooter();
})();
