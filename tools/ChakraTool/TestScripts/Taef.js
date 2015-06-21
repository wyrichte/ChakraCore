/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {    
    var teCommand = "";
    var teJsEtwConsoleCommand = "";

    // List of message prefix that we ignore from the taef unittests
    var consolePrefixFilters = ["StartGroup:", "TAEF: Data[", "INFO: Command:",
        "INFO: Exit Code:", "INFO: Diff Command:", "NotRun: ", "Non-passing Tests",
        "Summary: Total=", "Test Authoring and Execution Framework v",
        "Other Errors:", "    The selection criteria did not match any tests.",
        "    No test cases were executed.", "Error: TAEF: The selection criteria did not match any tests.",
        "Error: TAEF: No test cases were executed."];
    var endGroupRegex = /^EndGroup: (.*)\:\:(.*)\#(.*)\:\:(.*)\#(.*) \[(.*)\]$/;
    var nonPassingTestRegex = /^    (.*)\:\:(.*)\#(.*)\:\:(.*)\#(.*) \[(.*)\]$/;
    var allSpace = /^\s*$/;
    function filterConsoleOutput(config, str) {
        for (var i = 0; i < consolePrefixFilters.length; i++) {
            if (str.indexOf(consolePrefixFilters[i]) === 0) { return true; }
        }
        if (!config.showBlocked) {
            if (str.indexOf("Blocked: This test belongs to an execution group") == 0) { return true; }
        }
        return allSpace.exec(str) !== null || nonPassingTestRegex.exec(str) !== null;
    }
    
    function main(testContext) {
        
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var config = testContext.config;        
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var testFailed = false;
        var result = {};
        var total = 0;
        var failedTest = [];
        var blockedTest = [];        
        var testStarted = new Date();
        var lastStatus = testStarted;
        var consoleLog = function (str, isError) {
            // Strip the last \r\n
            var str = str.substring(0, str.length - 2);
            var a = endGroupRegex.exec(str);
            if (a == null) {
                if (!config.traceTestOutput && (isError || !filterConsoleOutput(config, str))) { WScript.Echo(str); }
                return;
            }
            var test = a[1];
            var testDir = a[2];
            var testName = a[3];
            var variantFunction = a[4];
            var variant = a[5];
            var state = a[6];
            var testVariant = test + "::" + variant;
            switch (state) {
                case "Passed":
                    if (!result[testVariant]) { result[testVariant] = { passed: 0, failed: 0, blocked: 0 }; }
                    result[testVariant].passed++;
                    total++;
                    break;
                case "Failed":
                    if (!result[testVariant]) { result[testVariant] = { passed: 0, failed: 0, blocked: 0 }; }
                    result[testVariant].failed++;
                    testFailed = true;
                    total++;
                    var fullTestName = test + "::" + testDir + "#" + testName + "::" + variantFunction + "#" + variant;
                    failedTest.push(fullTestName);
                    if (!config.traceTestOutput) {
                        WScript.Echo(fullTestName + " [" + state + "]");
                        WScript.Echo("------------------------------------------------------------------------------------");
                    }
                    break;
                case "Blocked":
                    if (config.showBlocked) {
                        if (!result[testVariant]) { result[testVariant] = { passed: 0, failed: 0, blocked: 0 }; }
                        result[testVariant].blocked++;
                        //testFailed = true;    // Blocked test should be failed test too?
                        total++;
                        var fullTestName = test + "::" + testDir + "#" + testName + "::" + variantFunction + "#" + variant;
                        blockedTest.push(fullTestName);
                        if (!config.traceTestOutput) {
                            WScript.Echo(fullTestName + " [" + state + "]");
                            WScript.Echo("------------------------------------------------------------------------------------");
                        }
                        break;
                    }                    
                default:
                    // Don't output anything or other states
                    return;
            }

            
            if (!config.traceTestOutput) {                
                // Print status every 15s
                var currentDate = new Date();
                if ((currentDate - lastStatus) > 15000) {
                    lastStatus = currentDate;
                    var totalStr = "" + total;
                    while (totalStr.length < 6) { totalStr = " " + totalStr; }
                    var timeStr = "" + (((currentDate - testStarted) / 1000) | 0);
                    while (timeStr.length < 4) { timeStr = " " + timeStr; }
                    WScript.Echo("    " + totalStr + " Tests Executed - Time Elapsed " + timeStr + "s");
                }
            }
        }
        shell.setStdOutWriter(function (str) {
            consoleLog(str, false);
            logger.logTestOutput(str);
        });
        shell.setStdErrWriter(function (str) {
            consoleLog(str, true);
            logger.logTestError(str);
        });
        
        
        logger.startRun("Taef");
        if (config.runProjection) {
            //Ensures directory exists
            config.testFilesRoot.getDirectory("Logs");
            // Non projection test doesn't need to run this
            logger.logLine("TE Command: \r\n{0}".format(teJsEtwConsoleCommand), "TestOperation");
            var exitCode = shell.execute(teJsEtwConsoleCommand);
            logger.logLine("Exit code returned: {0}".format(exitCode), "TestOperation"); // TAEF doesn't give 0 exit code
        }

        logger.logLine("TE Command: \r\n{0}".format(teCommand), "TestOperation");
        var exitCode = shell.execute(teCommand);
        logger.logLine("Exit code returned: {0}".format(exitCode), "TestOperation"); // TAEF doesn't give 0 exit code

        if (failedTest.length != 0) {
            logger.logTestSummary("\r\nFailed tests:\r\n");
            for (var i = 0 ; i < failedTest.length; i++) {
                logger.logTestSummary("    " + failedTest[i] + "\r\n");
            }
        }
        if (blockedTest.length != 0) {
            logger.logTestSummary("\r\nBlocked tests:\r\n");
            for (var i = 0 ; i < blockedTest.length; i++) {
                logger.logTestSummary("    " + blockedTest[i] + "\r\n");
            }
        }
        logger.logTestSummary(String.format("\r\nTests Executed: {0} Total - Time Elapsed {1}\r\n", total, (((new Date() - testStarted) / 1000) | 0)));
        for (var i in result) {
            if (result[i].blocked != 0) {
                logger.logTestSummary(String.format("  {0}: {1} passed, {2} failed, {3} blocked\r\n", i, result[i].passed, result[i].failed, result[i].blocked));
            } else {
                logger.logTestSummary(String.format("  {0}: {1} passed, {2} failed\r\n", i, result[i].passed, result[i].failed));
            }
        }

        if (!testFailed) { logger.logTestSummary("ALL TEST PASSED"); }
        logger.finishRun();

        //Cleanup
        shell.setStdOutWriter(logger.logRaw);
        shell.setStdErrWriter(logger.logErrorRaw);
        if (config.unregister) {
            shell.pushDirectory("{0}\\Functional".format(config.testFilesRoot));
            // For now this is not in JS, will be a task later:
            shell.execute("registerABIs.cmd -unregister");
            shell.popDirectory();
        }
        return testFailed ? 1 : 0;
    }

    
    function projectionEnlistmentSetup(shell, config, logger) {
        logger.logLine("Starting Enlistment setup.", "Setup");
        if (String(config.sdRoot) !== String(config.testRoot)) {
            shell.execute("robocopy {0} {1}".format(String(config.sdRoot), String(config.testRoot)));
        }

        shell.xCopy(
            "{0}\\*.winmd".format(config.mdRoot),
            "{0}".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\pdm.dll".format(config.jsBinRoot),
            "{0}\\jshost.exe.local".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\msdbg2.dll".format(config.jsBinRoot),
            "{0}\\jshost.exe.local".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\pdmproxy100.dll".format(config.jsBinRoot),
            "{0}\\jshost.exe.local".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\Tests\\projectionsGlue.js".format(config.testRoot),
            "{0}\\Tests\\Perf".format(config.testRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\Tests\\projectionsGlue.js".format(config.testRoot),
            "{0}\\..".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\rl.exe".format(config.jsBinRoot),
            "{0}".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        logger.logLine("Finished Enlistment setup.", "Setup");
    }

    function setupEnlistmentUnitTest(config, shell, logger) {
        var exitCode = shell.execute("call {0}\\runjs setupJsGlass".format(config.jsToolsRoot));
        if (exitCode !== 0) {
            throw new Error("Failed to call setupJsGlass");
        } else {
            logger.logLine("Successfully called setupJsGlass", "Setup");
        }
        var exitCode = shell.execute("call {0}\\runjs setupTesthost".format(config.jsToolsRoot));
        if (exitCode !== 0) {
            throw new Error("Failed to call setupTesthost");
        } else {
            logger.logLine("Successfully called setupTesthost", "Setup");
        }
        var exitCode = shell.execute("call {0}\\runjs setupJdTest".format(config.jsToolsRoot));
        if (exitCode !== 0) {
            throw new Error("Failed to call setupJdTest");
        } else {
            logger.logLine("Successfully called setupJdTest", "Setup");
        }
        var exitCode = shell.execute("call {0}\\runjs setupJsHostTest".format(config.jsToolsRoot));
        if (exitCode !== 0) {
            throw new Error("Failed to call setupJsHostTest");
        } else {
            logger.logLine("Successfully called setupJsHostTest", "Setup");
        }
        var exitCode = shell.execute("call {0}\\runjs setupWindowsGlobalization".format(config.jsToolsRoot));
        if (exitCode !== 0) {
            throw new Error("Failed to call setupWindowsGlobalization");
        } else {
            logger.logLine("Successfully called setupWindowsGlobalization", "Setup");
        }
    }

    //-snapTests
    function setup(testContext) {
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var config = testContext.config;        
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var storage = testContext.onlyForIntellisense ? Storage(testContext.storage) : testContext.storage;

        logger.setFlushPerWrite(true);
        shell.setStdOutWriter(logger.logRaw);
        shell.setStdErrWriter(logger.logErrorRaw);
        //:OutputInfo kind of
        logger.logLine("Raw Config::", "Config");
        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop], "Config");
        }
        config.isEnlistmentSetup = false;
        config.isSnapSetup = false;

        if (config.unitOnly === undefined && config.projectionOnly === undefined && config.selectTest === undefined) {
            config.runUnit = true;
            config.runProjection = true;
        }

        if (config.os === "win7") {
            // From projection
            throw new Error("Win7 is not currently suppoorted.");
        }

        if (config.drt) {
            // TODO
            throw new Error("DRT not supported");

            // From unittest
            if (config.runUnit) {
                notTagsArray.push("exclude_drt");
            }

            if (config.runProjection) {
                logger.logLine();
                logger.logLine("Redirrecting dirs for SNAP mode (DRT).", "Setup");
                logger.logLine();
                storage.attach(config.snapMDRoot);
                storage.attach(config.snapTargetDir);

                config.mdRoot = config.snapMDRoot;
                config.sdRoot = config.snapTargetDir;
                config.testRoot = config.snapTargetDir;
                config.testFilesRoot = config.snapTargetDir.getDirectory("Tests");
                config.projectionsBinRoot = config.snapTargetDir.getDirectory("JsHost");
                config.jshostPath = config.projectionsBinRoot + "\\jshost.exe";
                config.defaultRLLogPath = config.testFilesRoot + "\\logs\\rl.log";

                config.setup = false;
                notTagsArray.push("exclude_drt");
                config.unregister = true;
            }
        }
        else
        {
            if (config.runUnit) {
                setupEnlistmentUnitTest(config, shell, logger);
            }
        }
        
        if (!config.generate) {
            if (config.snap || config.drt) {
                config.isSnapSetup = true;
            } else {
                config.isEnlistmentSetup = true;
            }
        }


        //:OutputInfo kind of
        logger.logLine("Processed Config::", "Config");
        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop], "Config");
        }
        
        if (config.runProjection) {
           
            if (!config.isSnapSetup) {
                if (config.isEnlistmentSetup) {
                    projectionEnlistmentSetup(shell, config, logger);
                }
            
            }               
            shell.setEnv("_TestsDirectory", config.testFilesRoot);
            var newPath = "";
            if (config.drt) {
                newPath = "{0};{1};{2};{3}".format(config.testFilesRoot, config.projectionsBinRoot, config.snapJSRoot, shell.getEnv("PATH"));
            } else {
                newPath = "{0};{1};{2}".format(config.testFilesRoot, config.projectionsBinRoot, shell.getEnv("PATH"));
            }
                        
            shell.setEnv("WIN_JSHOST_METADATA_BASE_PATH", config.projectionsBinRoot);            
            logger.logLine("WIN_JSHOST_METADATA_BASE_PATH = '{0}'".format(shell.getEnv("WIN_JSHOST_METADATA_BASE_PATH")), "Config");

            if (config.force) {
                shell.pushDirectory("{0}\\Functional".format(config.testFilesRoot));
                // For now this is not in JS, will be a task later:
                shell.execute("registerABIs.cmd -unregister");
                shell.popDirectory();
            }

            storage.attach(config.testFilesRoot);
            shell.pushDirectory(config.testFilesRoot.getDirectory("Functional"));
            // For now this is not in JS, will be a task later:
            shell.execute("registerABIs.cmd");
            shell.popDirectory();
        }
        constructCommandLine(config, logger);
    }

    function constructCommandLine(config, logger)
    {

        // select query

        var propArray = [];
        if (config.snapTests) {
            config.snap = false;
            config.setup = false;
            propArray.push("/p:Snap=1");
        }

        if (config.html) {
            propArray.push("/p:html=1");
        }

        if (config.recyclerStressTests) {
            propArray.push("/p:RecyclerStress=1");
        }

        if (config.snap) {
            config.setup = false;
            config.unregister = true;
            propArray.push("/p:Snap=1");
        }

        // select query
        var selectQuery = [];        
        if (config.runUnit) {
            if (!config.runProjection) {
                selectQuery.push("@Name='UnitTest::*");
            }
        } else if (config.runProjection) {
            selectQuery.push("@Name='ProjectionTest::*");
        }

        var testSelect = [];
        if (config.selectTest) {
            config.selectTest.forEach(function (test) {
                testSelect.push("@Name='" + test + "'");
            });            
        }
        if (config.runUnit && config.dirs && config.dirs.length != 0) {            
            config.dirs.forEach(function (dir) {
                var query 
                // Only dynamic profile variants  when -dir is specified and -variant is not specified
                if (!config.variant || config.variant.length === 0) {                    
                    query = String.format("@Name='UnitTest::{0}#*::DynamicProfileVariants#*'", dir);
                }
                else
                {
                    query = String.format("@Name='UnitTest::{0}#*'", dir);
                }
                
                testSelect.push(query);
            });            
        }
        if (config.runProjection && config.projectionDirs && config.projectionDirs.length != 0) {            
            config.projectionDirs.forEach(function (dir) {
                var query = String.format("@Name='ProjectionTest::{0}#*'", dir);
                testSelect.push(query);
            });            
        }

        if (testSelect.length != 0) {
            selectQuery.push(testSelect.join(" OR "));
        }

        if (config.selectTags) {
            var tagSelect = [];
            config.selectTags.forEach(function (tag) {
                tagSelect.push(String.format("@{0}=1", tag));
            });
            selectQuery.push(tagSelect.join(" OR "));
        }

        if (config.variant) {
            if (config.variant && config.variant.length != 0) {
                var variantsQuery = [];
                config.variant.forEach(function (variant) {
                    var query = String.format("@Name='*#{0}'", variant);
                    variantsQuery.push(query);
                });

                selectQuery.push(variantsQuery.join(" OR "));
            }
        }

        if (isNaN(config.numberOfThreads)) {
            config.numberOfThreads = Math.floor(config.numberOfProcessors * 1.25);
            logger.logLine("TAEF will use {0} threads for parallel test running.".format(config.numberOfThreads), "Setup");
        }

        if (config.verbose) {
            propArray.push("/p:Verbose=1");
        }

        var teCommonCommand = String.format(
            "te {0}\\Taef\\ChakraNativeTaefTests.dll /console:flushWrites /p:\"TestBinDir={0}\" /p:\"TestRootDir={1}\" /p:Arch={2} /p:Flavor={3} {4}",
            config.jsBinRoot,
            config.jsRoot,
            config.platform,
            config.buildType,
            propArray.join(' '));

        // Command line for non-JsEtwConsole tests
        teCommand = teCommonCommand + " /parallel:" + config.numberOfThreads + (selectQuery.length != 0 ? " /select:\"({0})\"".format(selectQuery.join(") AND (")) : "");
        teCommand = String.removeDuplicateSpaces(teCommand);

        // Command line for JsEtwConsole tests
        selectQuery.push("@Parallel=false");  // JsEtwConsole tests are mared as non parallel
        teJsEtwConsoleCommand = teCommonCommand + " /p:JsEtwConsole=1 /RunAs=Elevated" + (selectQuery.length != 0 ? " /select:\"({0})\"".format(selectQuery.join(") AND (")) : "");
        teJsEtwConsoleCommand = String.removeDuplicateSpaces(teJsEtwConsoleCommand);

        logger.logLine("Base TE Command: " + teCommand, "Setup");
        logger.logLine("Base TE JsEtwConsole Command: " + teJsEtwConsoleCommand, "Setup");
    }
    // TODO: Not finish yet
    function snapSetup(testContext) {
        unittestSnapSetup(testContext);
        projectionSnapSetup(testContext);
    }
    function unittestSnapSetup(testContext) {
        
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var config = testContext.config;
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var storage = testContext.onlyForIntellisense ? Storage(testContext.storage) : testContext.storage;
        shell.setStdOutWriter(logger.log);
        shell.setStdErrWriter(logger.logError);

        logger.logLine("Started Unit snap setup.", "Setup");

        var snapTargetDir = testContext.onlyForIntellisense ? Storage.Directory(config.snapTargetDir) : config.snapTargetDir;
        snapTargetDir.attach(storage);
        snapTargetDir.cleanDirectory(true);

        var jshostLocalDir = storage.getDirectory("{0}\\jshost.exe.local".format(config.snapJSRoot));

        if (config.testCab === undefined) {
            config.testCab = "{0}\\JScriptTestCollateral.cab".format(config.snapBinRoot);
            logger.logLine("No path was specified for switch 'testCab', defaulting to '{0}'.".format(config.testCab), "Setup");
        }

        logger.logLine("Copying binaries", "Setup");
        shell.xCopy("{0}\\pdm.dll".format(config.snapBinRoot), jshostLocalDir, "/y");
        shell.xCopy("{0}\\msdbg2.dll".format(config.snapBinRoot), jshostLocalDir, "/y");
        shell.xCopy("{0}\\pdmproxy100.dll".format(config.snapBinRoot), jshostLocalDir, "/y");

        if (config.testCab !== undefined) {
            logger.logLine("Extracting test collateral.", "Setup");
            shell.execute("{0}\\cabarc.exe -o -p X {1} {2}\\".format(config.snapOwnBinRoot, config.testCab, snapTargetDir));

            if (!Storage.DoesFileExist("{0}\\Tools\\runjs.bat".format(snapTargetDir))) {
                throw new Error("Expected files are not found after extracting unit tests cab, aborting!");
            }
        } else {
            logger.logErrorLine("No test collateral was provided!", "Setup");
        }

        logger.logLine("Setting up test hosts.", "Setup");
        
        var exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupJsGlass {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupJsGlass");
        }
    
        exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupTesthost {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupJsGlass");
        }

        exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupJdTest {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupJsGlass");
        }

        exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupJsHostTest {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupJsGlass");
        }

        logger.logLine("Completed Unit snap setup.", "Setup");     
    }
    function projectionSnapSetup(testContext) {
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var config = testContext.config;
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var storage = testContext.onlyForIntellisense ? Storage(testContext.storage) : testContext.storage;
        shell.setStdOutWriter(logger.log);
        shell.setStdErrWriter(logger.logError);

        logger.logLine("Started Projection snap setup.", "Setup");

        var snapTargetDir = testContext.onlyForIntellisense ? Storage.Directory(config.snapTargetDir) : config.snapTargetDir;
        snapTargetDir.attach(storage);

        snapTargetDir.cleanDirectory(true);
        
        var jshostDir = snapTargetDir.getDirectory("JsHost");
        var jshostLocalDir = jshostDir.getDirectory("JsHost.exe.local");
        

        if (config.testCab === undefined) {
            config.testCab = "{0}\\Projection\\Tests.cab".format(config.snapBinRoot);
            logger.logLine("No path was specified for switch 'testCab', defaulting to '{0}'.".format(config.testCab), "Setup");
        }

        
        if (config.testCab !== undefined) {
            logger.logLine("Extracting test collateral.", "Setup");
            shell.execute("{0}\\cabarc.exe -o -p X {1} {2}\\".format(config.snapOwnBinRoot, config.testCab, snapTargetDir));

            if (!Storage.DoesFileExist("{0}\\Tests\\Functional\\AsyncDebug.js".format(snapTargetDir))) {
                throw new Error("Expected files are not found after extracting Projection tests cab, aborting!");
            }
        } else {
            logger.logErrorLine("No test collateral was provided!", "Setup");
        }

        
        logger.logLine("Copying ABIs", "Setup");
        var exitCode = shell.xCopy("{0}\\Projection\\winrt\\*.*".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        
        exitCode = shell.xCopy("{0}\\*.winmd".format(config.snapMDRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\*.winmd".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\Configurable".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.roboCopy("{0}\\JsHost\\".format(snapTargetDir), "{0}\\Tests\\Functional".format(snapTargetDir), "*.* /njh /njs /ndl /purge /xx");
        if (exitCode.isError) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\HeapDump".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\Perf".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\MemoryTracing".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\MetadataParsing".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\RecyclerStress".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\ScriptErrorTests".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        exitCode = shell.xCopy("{0}\\JsHost\\*.*".format(snapTargetDir), "{0}\\Tests\\Versioning".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        logger.logLine("Copying test binaries", "Setup");

        exitCode = shell.xCopy("{0}\\chakratest.dll".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\chakradiagtest.dll".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\jshost.exe".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\jdtest.exe".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\JsEtwConsole.exe".format(config.snapBinRoot), jshostDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\pdm.dll".format(config.snapBinRoot), jshostLocalDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        exitCode = shell.xCopy("{0}\\msdbg2.dll".format(config.snapBinRoot), jshostLocalDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        
        exitCode = shell.xCopy("{0}\\pdmproxy100.dll".format(config.snapBinRoot), jshostLocalDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\rl.exe".format(config.snapBinRoot), "{0}\\Tests".format(snapTargetDir), "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
        //TODO
        //    call :MakeConfigs JsHost "%_defaultConfig%"

        logger.logLine("Completed Projection snap setup.", "Setup");
    }

    this.TestHost.registerTestSuite({
        entryPoint: main,
        name: "Taef",
        setup: setup,
        snapSetup: snapSetup,
        configDefinition: {
            mdRoot: {
                type: 'Directory',
                defaultValue: "{0}\\sdpublic\\sdk\\winmetadata",
                defaultValueReferences: ["sdxRoot"]
            },
            sdRoot: {
                type: 'Directory',
                defaultValue: "{0}\\ProjectionTests",
                defaultValueReferences: ["jsRoot"],
                description: "The root directory for projection tests."
            },
            testRoot: {
                type: 'Directory',
                defaultValue: "{0}",
                defaultValueReferences: ["sdRoot"],
                description: "The root directory for projection tests ."
            },
            testFilesRoot: {
                type: 'Directory',
                defaultValue: "{0}\\Tests",
                defaultValueReferences: ["testRoot"],
                description: "The root directory for projection test files. "
            },
            projectionsBinRoot: {
                type: 'Directory',
                defaultValue: "{0}\\projection\\winrt",
                defaultValueReferences: ["binRoot"]
            },
            binPlaceRoot: {
                type: 'Directory',
                defaultValue: "{0}\\Projection\\localRun",
                defaultValueReferences: ["binRoot"]
            },
            defaultRLLogPath: {
                type: 'File',
                defaultValue: "{0}\\logs\\rl.log",
                defaultValueReferences: ["testFilesRoot"]
            },
            jshostPath: {
                type: 'File',
                defaultValue: "{0}\\jshost.exe",
                defaultValueReferences: ["projectionsBinRoot"],
                description: "JsHost binary path"
            },
            binary: {
                type: 'File',
                defaultValue: "{0}",
                defaultValueReference: ["jshostPath"],
                description: "The binary to use."
            },
            variants: {
                type: 'Array',
                defaultValue: [],
                description: "Variants to run."
            },
            generate: {
                type: 'Boolean',
                defaultValue: false
            },
            setup: {
                type: 'Boolean',
                defaultValue: false
            },
            verbose: {
                type: 'Boolean',
                defaultValue: false,
                description: ""
            },
            snapTests: {
                type: 'Boolean',
                defaultValue: false,
                description: "Use the -snapTests to run the SNAP test w/o unregistration on exit"
            },
            recyclerStressTests: {
                type: 'Boolean',
                defaultValue: false,
                description: ""
            },
            force: {
                type: 'Boolean',
                defaultValue: false,
                description: "Use -force to force ABI re-registration"
            },
            dirs: {
                type: 'Array',
                defaultValue: undefined,
                description: "If this flag is specified, only select unittest directories will run."
            },           
            projectionDirs: {
                type: 'Array',
                defaultValue: undefined,
                description: "If this flag is specified, only select projection directories will run."
            },
            testCab: {
                type: "File",
                defaultValue: undefined,
                description: "SNAP Only: Path to Tests.cab."
            },
            /* TODO: unittest and projection conflict */
            snapTargetDir: {
                    type: "Directory",
                    defaultValue: "{0}\\inetcore\\jscript",
                    defaultValueReferences: ["snapTargetRoot"],
                    description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory where unit tests should go."
            },
            snapTargetDir: {
                type: "Directory",
                defaultValue: "{0}\\ProjectionTests",
                defaultValueReferences: ["snapTargetRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory where projection tests should go."
            },
            snapMDRoot: {
                type: "Directory",
                defaultValue: "{0}\\winmetadata",
                defaultValueReferences: ["snapTargetDir"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory where projection MD files should go."
            },
            numberOfThreads: {
                type: "Number",
                defaultValue: undefined,
                description: "The number of threads to for running unit tests, if not provided defaults to number of processes * 1.25."
            },
            showBlocked: {
                type: "Boolean",
                defaultValue: false,
                description: "Show blocked tests"
            },
            unitOnly: {
                type: "Boolean",
                defaultValue: undefined,
                description: "Run unit test only"
            },
            projectionOnly: {
                type: "Boolean",
                defaultValue: undefined,
                description: "Run projection test only"
            },
            selectTest: {
                type: "Array",
                defaultValue: undefined,
                description: "Selected test to run"
            },
            selectTags: {
                type: "Array",
                defaultValue: undefined,
                description: "Select tests tagged with one of given tags to run"
            },
            html: { 
                type: "Boolean",
                defaultValue: false,
                description: "Specifies whether HTML tests will be run."
            }
        }
    });
}());

