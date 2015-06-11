(function (window) {
    ///////////////////////////////////////////////////////////////////////////////
    // Utils object for helper functions
    ///////////////////////////////////////////////////////////////////////////////
    var Utils = {};

    //Doc Mode
    Utils.IE7STANDARDSMODE = 7;
    Utils.IE8STANDARDSMODE = 8;
    Utils.IE9STANDARDSMODE = 9;
    Utils.IE10STANDARDSMODE = 10;

    //IE Architectures 
    Utils.X86MODE = "x86";
    Utils.X64MODE = "x64";
    Utils.ARMMODE = "arm";

    Utils.getHOSTMode = function () {
        var documentMode = this.IE10STANDARDSMODE; // Making default to be IE10 standards mode
        //See if document.documentMode is defined
        try {
            if ((document) && (document.documentMode)) {
                documentMode = document.documentMode;
            }
        } catch (ex) {
            //We are not running in IE, need to put logic of versioning for other hosts later
        }
        if (documentMode < 8)
            return this.IE7STANDARDSMODE;
        else if (documentMode == 8)
            return this.IE8STANDARDSMODE;
        else if (documentMode == 9)
            return this.IE9STANDARDSMODE;
        else if (documentMode == 10)
            return this.IE10STANDARDSMODE;
        else
            return this.IE10STANDARDSMODE;
    }

    Utils.getIEArchitecture = function () {
        //get the user agent and split the strings and identify t he IE mode that we are running 
        var mode = navigator.userAgent;
        var arr_mode = mode.toLowerCase().split(";");

        for (var i = 0; i < arr_mode.length; i++) {
            if (arr_mode[i] === " wow64") {
                return this.X86MODE;
            }
            else if (arr_mode[i] === " x64") {
                return this.X64MODE;
            }
            else if (arr_mode[i] === " arm") {
                return this.ARMMODE;
            }
        }
        //default to x86
        return this.X86MODE;
    }

    //Get the host of script engine
    Utils.WWAHOST = "WWA";
    Utils.IEHOST = "Internet Explorer";
    Utils.getHOSTType = function () {
        //navigator.appName will return the host name, e.g. "WWAHost/1.0" or "Microsoft Internet Explorer" 
        if (navigator.appName.indexOf(this.WWAHOST) >= 0)
            return this.WWAHOST;
        return this.IEHOST;
    }

    //Get localized error message
    Utils.getLocalizedError = function (ID, substitution) {
        if (substitution == undefined) {
            var localizedString = apGlobalObj.apGetLocalizedString(ID);
        } else {
            var localizedString = apGlobalObj.apGetLocalizedStringWithSubstitution(ID, substitution);
        }
        return localizedString;
    }

    var loggers = [];    
    ///////////////////////////////////////////////////////////////////////////////
    // WScriptLogger:
    // Log to the console using WScript.
    ////////////////////////////////////////////////////////////////////////////////
    function WScriptLogger() {
        var passCount = 0;
        var failCount = 0;
        var verifications = [];

        // Initialize the projection related stuff in JsHost
        if (Utils.getHOSTType() == Utils.WWAHOST)
            WScript.InitializeProjection();

        this.start = function (filename, priority) {
            if (priority === "all") {
                WScript.Echo(filename);
            } else {
                WScript.Echo(filename + " Priority " + priority);
            }
        }

        this.testStart = function (test) {
            verifications = [];
        }

        this.verify = function (test, passed, act, exp, msg) {
            if (passed)
                verifications.push("\tPass: " + msg + "\n\t\t  Actual: " + act + "\n");
            else
                verifications.push("\n\tFail: " + msg + "\n\t\tExpected: " + exp + "\n\t\t  Actual: " + act + "\n");
        }

        this.pass = function (test) {
            passCount++;
            WScript.Echo("PASS\t" + test.id + ": " + test.desc);
            //Needed for baselines
            WScript.Echo(verifications.join("\n"));
        }

        this.fail = function (test) {
            failCount++;
            WScript.Echo("");
            WScript.Echo("");
            WScript.Echo("FAIL\t" + test.id + ": " + test.desc);
            WScript.Echo("");
            WScript.Echo(verifications.join("\n"));
            WScript.Echo("");
            WScript.Echo("");
        }

        this.error = function (test, error) {
            failCount++;
            WScript.Echo("");
            WScript.Echo("");
            WScript.Echo("FAIL\t" + test.id + ": " + test.desc);
            WScript.Echo("\n\tError: " + error.name + " - " + error.description);
            if (verifications.length > 0) {
                WScript.Echo("");
                WScript.Echo(verifications.join("\n"));
                WScript.Echo("");
            }
            WScript.Echo("");
            WScript.Echo("");
        }

        this.end = function () {
            WScript.Echo("");
            WScript.Echo("Passed: " + passCount);
            WScript.Echo("Failed: " + failCount);
            if ((passCount + failCount) === 0) {
                WScript.Echo("");
                WScript.Echo("No tests were run! There's probably a SyntaxError in the test file.")
            }
        }

        this.comment = function (str) {
            verifications.push("\tComment: " + str);
        };
    }


    WScriptLogger.shouldEnable = function () {
        return typeof WScript !== "undefined" && typeof WScript.Echo !== "undefined" && typeof window.location === "undefined";
    }

    loggers.push(WScriptLogger);


    // A simple logger object exposed on the window, giving the user access to the comment method.
    // Simply publishes a comment event.
    var windowLogger = {};
    windowLogger.comment = function (str) {
        runner.publish('comment', str);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // End Loggers
    ///////////////////////////////////////////////////////////////////////////////
    //scheduler in charge of giving us tasks
    var scheduler = null;
    var runner = (function () {
        //holds onto objects that wish to be notified of pub/sub events
        var subscribers = [];
        //callbacks that get run before any tests execute
        var globalSetups = [];
        //callbacks that get run after all tests execute
        var globalTeardowns = [];
        //how many start calls we are waiting for
        var waitCount = 0;
        //current handle returned from setTimeout
        var waitHandle = null;
        //valid states
        //init - initial state
        //       => running
        //       => error
        //running - running test
        //          => waiting
        //          => ending
        //          => error
        //waiting - waiting for async op
        //          => waiting
        //          => resuming
        //          => timeout
        //          => error
        //resuming - resuming from async op
        //          => error
        //          => running
        //timeout - timed out during async op
        //          => resuming
        //          => error
        //ending - final state
        //error - error in the test
        //        => running
        //        => ending
        //current state of the runner
        var currentState = "init";

        //name of the currently executing file/suite. This is set via
        //setting Loader42_FileName on the global scope
        var fileName = null;
        //priority to run for filtering tests. Valid values are "all" or a number
        var priority = "all";
        //default timeout for async tasks
        var DEFAULT_TIMEOUT = 10000;
        //flag for the main run loop
        var ended = false;
        //object holding valid states and the action to take each iteration while running in them
        var states = {
            init: function () {
                if (window.runsExecuted == 0) {    // In MaxInterprentCount case do it only on first iteration
                    runner.publish('start', fileName, priority);
                }
                scheduler.prepend(globalSetups);
                scheduler.push(globalTeardowns);
                transitionTo("running");
            },
            resuming: function () {
                transitionTo("running");
            },
            running: function () {
                var task = scheduler.next();
                if (task) {
                    try {
                        task();
                    } catch (ex) {
                        runner.publish('error', ex);
                        transitionTo("error");
                    }
                } else {
                    transitionTo('ending');
                }
            },
            ending: function () {
                if (++window.runsExecuted < window.runsToExecute) {
                    transitionTo("restarting");
                }
                else {
                    runner.publish("end", fileName, priority);
                    ended = true;
                }
            },
            restarting: function () {
                tasks = ranTasks;
                ranTasks = [];
                transitionTo("init");
            },
            waiting: function () { },
            timeout: function () {
                if (waitHandle === null)
                    return;
                waitHandle = null;
                currentTest.error = new Error("Timed out before runner.start() being called");
                testPassed = false;
                waitCount = 0;
                transitionTo("resuming");
            },
            error: function () {
                ended = true;
            }
        };

        // Subscribes to an event named prop for each property of obj, with the callback being the
        // value.
        function subscribeObject(obj) {
            for (var prop in obj)
                if (typeof obj[prop] === "function")
                    subscribeToEvent(prop, obj[prop]);
        }

        // Subscribes to an event of a given name with the given callback.
        function subscribeToEvent(e, callback) {
            if (typeof subscribers[e] === "undefined")
                subscribers[e] = [];

            subscribers[e].push(callback);
        }

        //transitions the runner to a new state and validates that the transition is allowed
        //parameters:
        // * newState: string representing the new state to transition to
        function transitionTo(newState) {
            function transitionAssert() {
                var res = false;
                for (var i = 0; i < arguments.length; i++)
                    if (arguments[i] === newState)
                        res = true;
                runnerAssert(res, "invalid state transition: " + currentState + "->" + newState)
            }
            switch (currentState) {
                case "init":
                    transitionAssert("running", "error");
                    break;
                case "running":
                    transitionAssert("waiting", "ending", "error");
                    break;
                case "waiting":
                    transitionAssert("resuming", "waiting", "error", "timeout");
                    break;
                case "timeout":
                    transitionAssert("resuming", "error");
                    break;
                case "resuming":
                    transitionAssert("running", "error");
                    break;
                case "error":
                    transitionAssert("running", "ending");
                    break;
                case "ending":
                    transitionAssert("restarting", "error");
                    break;
                case "restarting":
                    transitionAssert("init", "error");
                    break;
                default:
                    runnerAssert(false, "Invalid state");
                    break;
            }
            currentState = newState;
        }

        //internal check to validate assumptions at given points in the code
        //along the lines of the C ASSERT macro or C# asserts
        //parameters:
        // * bool - boolean value that is assumed to be true
        // * msg - message to publish if the bool is false
        //returns:
        // the bool passed in (for use in conditionals or assignment)
        function runnerAssert(bool, msg) {
            if (!bool) {
                runner.publish('comment', "Internal failure: " + msg);
                transitionTo("error");
                currentTest.error = new Error("Internal failure: " + msg);
            }
            return bool;
        }

        //main runloop for the runner. Loops continuously unless state
        //is waiting or the test is ended. Each iteration, it runs the action
        //associated with the current state
        function runTasks() {
            while (!ended) {
                states[currentState]();
                if (currentState === "waiting")
                    return;
            }
        }

        //wraps globalSetup or globalTeardown operations in a try catch block to allow logging
        function wrapGlobalOperation(type, task) {
            return function () {
                try {
                    task();
                } catch (e) {
                    runner.publish('comment', "Error during " + type + " - " + e.name + ": " + e.description);
                    runner.publish('end', fileName, priority);
                    transitionTo("error");
                }
            }
        }


        // Runner object is responsible for controlling the execution of tests. It is exposed on the global
        // object to external code can call any method on runner.
        return {
            // Subscribe to an event by passing an eventName and a callback function. Subscribe to
            // multiple events by passing an object with keys that correspond to the event names you
            // want to subscribe to.
            subscribe: function () {
                if (arguments.length == 1)
                    subscribeObject(arguments[0]);
                else
                    subscribeToEvent(arguments[0], arguments[1]);
            },

            // Publish an event. Calls all subscribed callbacks with the provided arguments.
            publish: function (e) {
                var args = Array.prototype.slice.call(arguments, 1);

                if (typeof subscribers[e] !== "undefined")
                    for (var i = 0; i < subscribers[e].length; i++)
                        subscribers[e][i].apply(undefined, args);
            },

            mutators: [],


            //Runs list of tests once.
            //parameters:
            // * testFileName - name of the test file/suite
            run: function (testFileName) {
                runnerAssert(scheduler !== null, "no scheduler set. This only happens if setScheduler is not called with a scheduler");
                runnerAssert(currentState === "init", "run called while runner in bad state. Probably means multiple calls to run were made");
                if (typeof testFileName === "undefined") {
                    try {
                        testFileName = Loader42_FileName;  // Loader42_FileName may be undefined.
                    } catch (e) {
                        //noop
                    } finally {
                        testFileName = testFileName || "No filename given";
                    }
                }

                fileName = testFileName;
                runTasks();
            },

            //Decrements the wait counter and instructs the runner to resume running after a wait call
            // if the counter is 0. If the timeout has fired, this is a noop.
            //parameters:
            // * testWaitHandle - waithandle corresponding to the return value from start
            start: function (testWaitHandle) {
                if (typeof testWaitHandle !== "undefined" && testWaitHandle !== waitHandle)
                    return;

                waitCount--;
                if (waitCount === 0) {
                    clearTimeout && clearTimeout(testWaitHandle);
                    transitionTo("resuming");
                    waitHandle = null;
                    runTasks();
                }
            },

            //Increments the wait counter and instructs the runner to wait for an async op to finish
            //if the counter is > 0. If count is passed, the wait counter is incremented by that amount.
            //parameters:
            // * timeout - time (in milliseconds) to wait before timing out. Defaults to DEFAULT_TIMEOUT
            // * count - amount to increment counter by. Defaults to 1
            //returns:
            // * the waitHandle produced by setTimeout
            wait: function (timeout, count) {
                count = count || 1;
                timeout = timeout || DEFAULT_TIMEOUT;
                waitCount += count;
                if (waitCount > 0) {
                    transitionTo("waiting");
                    waitHandle = setTimeout && setTimeout(function () {
                        transitionTo("timeout");
                        runTasks();
                    }, timeout);
                    return waitHandle;
                }
            },

            // Create a new test object with the properties specified in obj.
            addTest: function (obj) {
                var test = new TestCase();

                for (var prop in obj) {
                    test[prop] = obj[prop]
                }

                test.AddTest();
            },

            globalSetup: function (callback) {
                globalSetups.push(wrapGlobalOperation("global setup", callback));
            },

            globalTeardown: function (callback) {
                globalTeardowns.push(wrapGlobalOperation("global teardown", callback));
            },

            //sets the scheduler for the runner. Note that this doesn't (currently) unregister the old scheduler.
            //As part of registering the scheduler, the 'schedule' and 'prepend' events are registered for pub/sub
            //parameters:
            // * sched - new scheduler object
            setScheduler: function (sched) {
                scheduler = sched;
                runner.subscribe("schedule", sched.schedule);
            }
        };
    })();

    for (var i = 0; i < loggers.length; i++)
        if (loggers[i].shouldEnable())
            runner.subscribe(new loggers[i]());

    // holds the list of tasks that need to be scheduled. Running is currently destructive to this list
    var tasks = [];
    // holds the list of tasks that have been run already, so that tasks[] list can be restored after the run
    var ranTasks = [];
    window.runsExecuted = 0;
    window.runsToExecute = getMaxInterpretCount() + 1;

    // read "MaxInterpretCount" environment variable to know how many time to run test suite
    function getMaxInterpretCount() {
        try {
            var GetEnvVariable = function (variable) {
                var wscriptShell = new ActiveXObject("WScript.Shell");
                var wshEnvironment = wscriptShell.Environment("Process");
                return wshEnvironment(variable);
            }
            var result = GetEnvVariable("MaxInterpretCount");
            if (result === "")
                return 0;
            else
                return Math.floor(result);
        }
        catch (e) {
            return 0;
        }
    }

    // populate and return default scheduler object
    var defaultScheduler = (function () {
        return {
            //adds an item to the tail of the list. If the item has a tasks property that is a function
            //the return value of that function is used instead of this item
            //parameters:
            // * item - item to schedule
            schedule: function (item) {
                var items;
                if (item.tasks && typeof item.tasks === "function")
                    items = item.tasks();
                else
                    items = [item];
                for (var i = 0; i < items.length; i++)
                    tasks.push(items[i]);
            },
            //prepends an item or array to the list. If the item is an array, it is concatenated onto
            //the front of the list (i.e. single level flatten). Otherwise it is just put into the list
            //parameters:
            // * obj - item to put onto the front of the list
            prepend: function (obj) {
                var type = hoozit(obj);
                if (type === "array") {
                    for (var i = obj.length - 1; i >= 0; i--) {
                        tasks.unshift(obj[i]);
                    }
                } else {
                    tasks.unshift(obj);
                }
            },
            //appends an item or array to the list. If the item is an array, it is concatenated onto
            //the end of the list (i.e. single level flatten). Otherwise it is just put into the list
            //parameters:
            // * obj - item to put onto the end of the list
            push: function (obj) {
                var type = hoozit(obj);
                if (type === "array") {
                    for (var i = 0; i < obj.length; i++) {
                        tasks.push(obj[i]);
                    }
                } else {
                    tasks.push(obj);
                }
            },
            //returns the next task to run
            //returns:
            //the next task to run or null if there are no more tasks
            next: function () {
                var task = null;
                if (tasks.length > 0) {
                    task = tasks.shift();
                    ranTasks.push(task);
                }

                return task;
            }
        };
    })();
    runner.setScheduler(defaultScheduler);

    ///////////////////////////////////////////////////////////////////////////////
    // Test Runner
    ///////////////////////////////////////////////////////////////////////////////
    var currentTest;
    var testPassed;

    // A test case object
    var TestCase = function () {
        //close over this for the tasks method
        var testCase = this;

        // Flag to execute the test in global scope
        this.useGlobalThis = false;
        this.baselineFile = false;
        this.baselineHandler = 0;
        this.baselineCounter = 0;
        this.pass = true;

        this.AddTest = function () {
            // Function to add use test case to testCases list
            if ((this.id === undefined) || (this.id === "")) {
                runner.publish('comment', "Test case id is not valid ");
            } else if (this.desc === undefined || this.desc === "") {
                runner.publish('comment', "Test case description not specified");
            } else if (this.preReq && (!(typeof (this.preReq) === "function"))) {
                runner.publish('comment', "Invalid preReq function");
            } else if ((this.test === undefined) || (!(typeof (this.test) === "function"))) {
                runner.publish('comment', "Invalid test function");
            } else {
                runner.publish('schedule', this);
            }
        };

        //splits the test work into setup, test and cleanup. This allows the runner to pause
        //between the test task and the cleanup task in order to wait for async operations
        //returns:
        // - an array of functions
        this.tasks = function () {
            function setup() {
                testPassed = true;
                currentTest = testCase;

                runner.publish('testStart', currentTest);
            }

            function test() {
                if (!currentTest.preReq || currentTest.preReq()) {
                    try {
                        if (currentTest.useGlobalThis) {
                            testFunc = currentTest.test;
                            testFunc();
                        } else {
                            currentTest.test();
                        }
                    } catch (e) {
                        currentTest.error = e;
                    }
                } else {
                    runner.publish('comment', "Test prereq returned false. Skipping");
                }
            }

            function cleanup() {
                if (currentTest.error !== undefined) {
                    runner.publish('error', currentTest, currentTest.error);
                } else if (testPassed) {
                    runner.publish('pass', currentTest);
                } else {
                    runner.publish('fail', currentTest);
                }
                runner.publish('testEnd', currentTest);
            }

            return [setup, test, cleanup];
        }
    }

    /*
    * Baseline test cases
    */

    function addEscapeCharacter(txt) {
        txt = txt.replace(/\\/g, "\\\\");
        txt = txt.replace(/\"/g, "\\\"");
        txt = txt.replace(/\'/g, "\\\'");
        txt = txt.replace(/\r/g, "\\r");
        txt = txt.replace(/\n/g, "\\n");

        return txt;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Verification functions.
    ///////////////////////////////////////////////////////////////////////////////
    //the wrapping function is really just to allow folding in editors
    var verify = (function () {
        function verify(act, exp, msg) {
            var result = equiv(act, exp);
            testPassed = testPassed && result;
            runner.publish('verify', currentTest, result, act, exp, msg);
        }
        verify.equal = verify;
        verify.notEqual = function (act, exp, msg) {
            var result = !equiv(act, exp);
            testPassed = testPassed && result;
            runner.publish('verify', currentTest, result, act, "not " + exp, msg);
        };

        verify.strictEqual = function (act, exp, msg) {
            var result = act === exp;
            testPassed = testPassed && result;
            runner.publish('verify', currentTest, result, act, exp, msg);
        }

        verify.notStrictEqual = function (act, exp, msg) {
            var result = act !== exp;
            testPassed = testPassed && result;
            runner.publish('verify', currentTest, result, act, "not " + exp, msg);
        }

        verify.noException = function (callback, msg) {
            try {
                var res = callback();
                verify.equal(res, res, msg);
            } catch (e) {
                verify.equal(res, "no exception", msg);
            }
        };

        verify.exception = function (callback, exception, msg) {
            try {
                var res = callback();
                verify.equal(res, "exception", msg);
            } catch (e) {
                verify.instanceOf(e, exception)
            }
        };

        verify.defined = function (obj, msg) {
            return verify.notStrictEqual(obj, undefined, msg + " should be defined");
        };

        verify.notDefined = function (obj, msg) {
            return verify.strictEqual(obj, undefined, msg + " should not be defined");
        };

        verify.typeOf = function (obj, expected) {
            return verify.equal(typeof obj, expected, "typeof " + obj);
        };

        verify.instanceOf = function (obj, expected) {
            return verify.equal(obj instanceof expected, true, "instanceof " + obj);
        };

        return verify;
    })();

    function fail(msg) {
        verify(true, false, msg);
    }

    function assert(condition, msg) {
        verify(condition, true, msg);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Below here is code to support the old test case format. These old test     
    // cases call a series of ap* functions directly. The object below basically  
    // proxies calls to ap* functions to the appropriate logger. Because the old  
    // test case format doesn't instantiate a test case object, the code below    
    // creates a stub to pass to the loggers.                                     
    ///////////////////////////////////////////////////////////////////////////////

    function OldGlueProxy() {
        var test;
        var testStarted = false;
        var scenarioStarted = false;
        var scenarioPassed = true;

        function finishScenario() {
            if (scenarioPassed)
                runner.publish('pass', test);
            else
                runner.publish('fail', test);
        }

        this.apInitTest = function (name) {
            if (window.runsExecuted == 0) {    // In MaxInterprentCount case do it only on first iteration
                if (testStarted)
                    runner.publish('end');

                testStarted = true;

                if (typeof Loader42_FileName == 'undefined')
                    runner.publish('start', name);
                else
                    runner.publish('start', Loader42_FileName);
            }
        }

        this.apInitScenario = function (name) {
            if (scenarioStarted)
                finishScenario();

            var id = /^\d+/.exec(name);
            if (id === null)
                id = "";

            test = { id: id, desc: name, test: Function() };

            runner.publish('testStart', test);
            scenarioStarted = true;
            scenarioPassed = true;
        }

        this.apEndScenario = function () {
            finishScenario();
            scenarioStarted = false;
        }

        this.apLogFailInfo = function (msg, exp, act) {
            runner.publish('verify', test, false, act, exp, msg);
            scenarioPassed = false;
        }

        this.apEndTest = function () {
            if (window.runsExecuted >= window.runsToExecute - 1) {    // In MaxInterprentCount case do it only on last iteration
                if (scenarioStarted)
                    finishScenario();
                runner.publish('end');
                testStarted = false;
            }
        }

        this.apWriteDebug = function (msg) {
            runner.publish('comment', msg);
        }

        this.VBS_apglobal_init = function () {
            try {
                window.apGlobalObj = new ActiveXObject("apgloballib.apglobal");
            } catch (e) {
                // Can't create ActiveX, so make a stub object with the same methods.
                window.apGlobalObj = {
                    apInitTest: apInitTest,
                    apInitScenario: apInitScenario,
                    apLogFailInfo: apLogFailInfo,
                    apEndScenario: apEndScenario,
                    apEndTest: apEndTest,
                    apGetLangExt: function (Lcid) {
                        var LangExt = "";
                        switch (apGlobalObj.apPrimaryLang(Lcid)) {
                            case 0x9:
                                LangExt = "EN"; // LANG_ENGLISH
                                break;
                            case 0xC:
                                LangExt = "FR"; // LANG_FRENCH
                                break;
                            case 0xA:
                                LangExt = "ES"; // LANG_SPANISH
                                break;
                            case 0x7:
                                LangExt = "DE"; // LANG_GERMAN
                                break;
                            case 0x10:
                                LangExt = "IT"; // LANG_ITALIAN
                                break;
                            case 0x16:
                                LangExt = "PT"; // LANG_PORTUGUESE
                                break;
                            case 0x1D:
                                LangExt = "SV"; // LANG_SWEDISH
                                break;
                            case 0x14:
                                LangExt = "NO"; // LANG_NORWEGIAN
                                break;
                        }
                        if (LangExt == "") { //Hack for DBCS
                            switch (Lcid) {
                                case 0x411:
                                    LangExt = "JP"; // LANG_JAPANESE
                                    break;
                                case 0x412:
                                    LangExt = "KO"; // LANG_KOREAN
                                    break;
                                case 0x404:
                                    LangExt = "CHT"; // Chinese
                                    break;
                                case 0x804:
                                    LangExt = "CHS"; // PRC
                                    break;
                            }
                        }
                        return LangExt;
                    },
                    apPrimaryLang: function (Lcid) {
                        return Lcid & 0x3FF;
                    },
                    apGetLocFormatDate: function (Lcid, Fmt) {
                        var strToken = "";
                        switch (Fmt) // am not sure whenstr.toUpper will be supported..
                        {
                            case "LONG DATE":
                            case "long date":
                                strToken = "datelong";
                                break;
                            case "MEDIUM DATE":
                            case "medium date":
                                strToken = "datemed";
                                break;
                            case "SHORT DATE":
                            case "short date":
                                strToken = "dateshort";
                                break;
                            case "GENERAL DATE":
                            case "general date":
                                strToken = "gendate";
                                break;
                            case "LONG TIME":
                            case "long time":
                                strToken = "timelong";
                                break;
                            case "MEDIUM TIME":
                            case "medium time":
                                strToken = "timemed";
                                break;
                            case "SHORT TIME":
                            case "short time":
                                strToken = "timeshort";
                                break;
                        }
                        return apGlobalObj.apGetToken(Lcid, "fmt_named_" + strToken, "OLB");
                    },
                    apGetToken: function (Lcid, strTokenName, strFileName) {
                        // not needed right now..
                        var strToken = "";
                        return (strToken == "") ? "[[Token Not Found]]" : strToken;
                    },
                    apGetLocInfo: function (Lcid, lctype, ProjectOverride, Flags) { },
                    apGetOSVer: function () {
                        //not needed
                        return "NT;";
                    },
                    apGetPathName: function () {
                        //not needed
                        apGlobalObj.apLogFailInfo("FAILURE: TC requested function apGetPathName", "", "", "");
                        apGlobalObj.apEndTest();
                        return "";
                    },
                    apGetPlatform: function () {
                        //not needed
                        return "NT;";
                    },
                    apGetVolumeName: function (OS) {
                        //not needed
                        return "NT;";
                    },
                    LangHost: function () {
                        //  return GetUserDefaultLCID();
                        return 1033;
                    },
                    apGetLocalizedString: function (resID) {
                        return "No localized string returned";
                    },
                    apGetLocalizedStringWithSubstitution: function (resID, stringSubstitution) {
                        return "No localized string returned";
                    }
                }
            }
            window.apPlatform = apGlobalObj.apGetPlatform();
        }

        this.apGetLocale = function () {
            return apGlobalObj.LangHost();
        }

    }

    ///////////////////////////////////////////////////////////////////////////////
    // Object Comparison functions.
    ///////////////////////////////////////////////////////////////////////////////

    // Determine what is o.
    function hoozit(o) {
        if (typeof o === "undefined") {
            return "undefined";
        } else if (o === null) {
            return "null";
        } else if (o.constructor === String) {
            return "string";

        } else if (o.constructor === Boolean) {
            return "boolean";

        } else if (o.constructor === Number) {

            if (isNaN(o)) {
                return "nan";
            } else {
                return "number";
            }

            // consider: typeof [] === object
        } else if (o instanceof Array) {
            return "array";

            // consider: typeof new Date() === object
        } else if (o instanceof Date) {
            return "date";

            // consider: /./ instanceof Object;
            //           /./ instanceof RegExp;
            //          typeof /./ === "function"; // => false in IE and Opera,
            //                                          true in FF and Safari
        } else if (o instanceof RegExp) {
            return "regexp";

        } else if (typeof o === "object") {
            return "object";

        } else if (o instanceof Function) {
            return "function";
        } else {
            return undefined;
        }
    }

    // Call the o related callback with the given arguments.
    function bindCallbacks(o, callbacks, args) {
        var prop = hoozit(o);
        if (prop) {
            if (hoozit(callbacks[prop]) === "function") {
                return callbacks[prop].apply(callbacks, args);
            } else {
                return callbacks[prop]; // or undefined
            }
        }
    }
    // Test for equality any JavaScript type.
    // Discussions and reference: http://philrathe.com/articles/equiv
    // Test suites: http://philrathe.com/tests/equiv
    // Author: Philippe Rath? <prathe@gmail.com>
    var equiv = function () {

        var innerEquiv; // the real equiv function
        var callers = []; // stack to decide between skip/abort functions


        var callbacks = function () {

            // for string, boolean, number and null
            function useStrictEquality(b, a) {
                if (b instanceof a.constructor || a instanceof b.constructor) {
                    // to catch short annotaion VS 'new' annotation of a declaration
                    // e.g. var i = 1;
                    //      var j = new Number(1);
                    return a == b;
                } else {
                    return a === b;
                }
            }

            return {
                "string": useStrictEquality,
                "boolean": useStrictEquality,
                "number": useStrictEquality,
                "null": useStrictEquality,
                "undefined": useStrictEquality,

                "nan": function (b) {
                    return isNaN(b);
                },

                "date": function (b, a) {
                    return hoozit(b) === "date" && a.valueOf() === b.valueOf();
                },

                "regexp": function (b, a) {
                    return hoozit(b) === "regexp" &&
                    a.source === b.source && // the regex itself
                    a.global === b.global && // and its modifers (gmi) ...
                    a.ignoreCase === b.ignoreCase &&
                    a.multiline === b.multiline;
                },

                // - skip when the property is a method of an instance (OOP)
                // - abort otherwise,
                //   initial === would have catch identical references anyway
                "function": function () {
                    var caller = callers[callers.length - 1];
                    return caller !== Object &&
                        typeof caller !== "undefined";
                },

                "array": function (b, a) {
                    var i;
                    var len;

                    // b could be an object literal here
                    if (!(hoozit(b) === "array")) {
                        return false;
                    }

                    len = a.length;
                    if (len !== b.length) { // safe and faster
                        return false;
                    }
                    for (i = 0; i < len; i++) {
                        if (!innerEquiv(a[i], b[i])) {
                            return false;
                        }
                    }
                    return true;
                },

                "object": function (b, a) {
                    var i;
                    var eq = true; // unless we can proove it
                    var aProperties = [], bProperties = []; // collection of strings

                    // comparing constructors is more strict than using instanceof
                    if (a.constructor !== b.constructor) {
                        return false;
                    }

                    // stack constructor before traversing properties
                    callers.push(a.constructor);

                    for (i in a) { // be strict: don't ensures hasOwnProperty and go deep

                        aProperties.push(i); // collect a's properties

                        if (!innerEquiv(a[i], b[i])) {
                            eq = false;
                        }
                    }

                    callers.pop(); // unstack, we are done

                    for (i in b) {
                        bProperties.push(i); // collect b's properties
                    }

                    // Ensures identical properties name
                    return eq && innerEquiv(aProperties, bProperties);
                }
            };
        } ();

        innerEquiv = function () { // can take multiple arguments
            var args = Array.prototype.slice.apply(arguments);
            if (args.length < 2) {
                return true; // end transition
            }

            return (function (a, b) {
                if (a === b) {
                    return true; // catch the most you can
                } else if (a === null || b === null || typeof a === "undefined" || typeof b === "undefined" || hoozit(a) !== hoozit(b)) {
                    return false; // don't lose time with error prone cases
                } else {
                    return bindCallbacks(a, callbacks, [b, a]);
                }

                // apply transition with (1..n) arguments
            })(args[0], args[1]) && arguments.callee.apply(this, args.splice(1, args.length - 1));
        };

        return innerEquiv;

    } ();

    ///////////////////////////////////////////////////////////////////////////////
    //
    //   Assign globals to the global object.
    //   These are underscore prefixed because they are called by functions of the same name outside the
    //   framework closure. This is done because window.verify will not be overwritten by a
    //   function verify() {} in global scope, which many old tests depend on.
    //
    ///////////////////////////////////////////////////////////////////////////////
    window._verify = verify;
    window._assert = assert;
    window._fail = fail;
    window.getLocalizedError = Utils.getLocalizedError;

    window.Run = runner.run;
    window.runner = runner;
    window.TestCase = TestCase;
    window.logger = windowLogger;
    // globals for versioning
    window.IE7STANDARDSMODE = Utils.IE7STANDARDSMODE;
    window.IE8STANDARDSMODE = Utils.IE8STANDARDSMODE;
    window.IE9STANDARDSMODE = Utils.IE9STANDARDSMODE;
    window.IE10STANDARDSMODE = Utils.IE10STANDARDSMODE;
    window.getHOSTMode = Utils.getHOSTMode;

    window.X86MODE = Utils.X86MODE;
    window.X64MODE = Utils.X64MODE;
    window.ARMMODE = Utils.ARMMODE;
    window.getIEArchitecture = Utils.getIEArchitecture;

    window.Utils = Utils;

    // Function returns true/false based on equality, same as verify, but excludes logging. 
    Utils.equiv = equiv;



    // An object to hold various implementational details of our engine that our test cases rely upon
    window.Chakra = {
        // The amount of properties that have to be declared to transition object from path to
        // dictionary type.
        PathToDictionaryTransitionThreshold: 16
    }

    // if this is called, set up globals required for old test cases.
    window.VBS_apglobal_init = function () {
        // Old test cases require conditional compilation in a few cases, so turn it on.
        var proxy = new OldGlueProxy();
        window.apInitTest = proxy.apInitTest;
        window.apInitScenario = proxy.apInitScenario;
        window.apEndScenario = proxy.apEndScenario;
        window.apLogFailInfo = proxy.apLogFailInfo;
        window.apEndTest = proxy.apEndTest;
        window.apWriteDebug = proxy.apWriteDebug;
        window.VBS_apglobal_init = proxy.VBS_apglobal_init;
        window.apGetLocale = proxy.apGetLocale;
        proxy.VBS_apglobal_init();
    }

    // It used to be defined via 'function VBS_apglobal_init()'
    // 'window.apGlobalObj' has to be defined
    if (typeof window.apGlobalObj == "undefined") {
        try {
            // Define the object
            window.apGlobalObj = new ActiveXObject("apgloballib.apglobal");
        } catch (e) {
            // Cannot instantiate ActiveXObject
            window.apGlobalObj = null;
        }
    }

})(this);

// Default version of these verification functions simply call the framework version of them.
verify = _verify;
assert = _assert;
fail = _fail;
