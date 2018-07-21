/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    var runCommand = "";
    function main(testContext) {
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var config = testContext.config;
        shell.setStdOutWriter(logger.logTestSummary);
        shell.setStdErrWriter(logger.logTestError);

        logger.logLine("Starting JSRT run.", "Setup");
        logger.startRun("run");

        var exitCode = shell.execute(runCommand, config.timeOut);
        logger.logLine("Exit code: {0}".format(exitCode), "TestOperation");

        logger.finishRun();
        logger.logLine("Completed JSRT run.", "Setup");

        return exitCode;//0-15 is number of non passing tests
    }

    function setup(testContext) {
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var config = testContext.config;
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;

        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop], "Config");
        }

        logger.logLine("Starting JSRT setup.", "Setup");

        var testList = config.testPath + "\\UnitTest.JsRT.API.dll ";
        testList += config.testPath + "\\UnitTest.JsRT.ComProjection.dll ";
        testList += config.testPath + "\\UnitTest.JsRT.RentalThreading.dll ";

        var runManagedTests = true;
        var logFile = undefined;

        if (config.snap) {
            logger.logLine("Running in SNAP mode.", "Setup");
            runManagedTests = false;
        }

        var logArg = "";

        if (config.logFile) {
            logger.logLine("TAEF logging will be forwarded to '{0}'.".format(config.logFile), "Setup");
            logArg = ' /enableWttLogging /logFile:"{0}"'.format(config.logFile);
        }

        if (runManagedTests) {
            if (shell.getOSVersion().isWin7) setupJsrtUnitTests();
            testList += config.testPath + "\\UnitTest.JsRT.Managed.API.dll ";
            testList += config.testPath + "\\UnitTest.JsRT.WinRT.dll ";
        }

        runCommand = 'te {0} /select:"(not @Disabled=\'Yes\')" /unicodeoutput:false{1} /isolationLevel:class /logOutput:lowest {2}'.format(
            testList,
            " /parallel:1", //This was hard coded in EzeAutomation.js, so leaving it as 1. config.numberOfThreads > 1 ? " /parallel:{0}".format(config.numberOfThreads) : "",
            logArg);
        logger.logLine("Test Command: {0}".format(runCommand), "Setup");
        logger.logLine("Completed JSRT setup.", "Setup");
    }

    function snapSetup(testContext) {
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;

        var config = testContext.config;
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        shell.setStdOutWriter(logger.logRaw);
        shell.setStdErrWriter(logger.logErrorRaw);
        logger.logLine("Started JSRT snap setup.", "Setup");

        //robocopy %_sourceDir% %_targetDir% *.dll *.exe *.mui *.tlb *.js /njh /njs /ndl /purge /xx
        logger.logLine("Executing robocopy {0} {1} *.dll *.exe *.mui *.tlb *.js /njh /njs /ndl /purge /xx".format(config.snapSourceDir, config.snapTargetDir), "Setup");
        var exitCode = shell.execute("robocopy {0} {1} *.dll *.exe *.mui *.tlb *.js /njh /njs /ndl /purge /xx".format(config.snapSourceDir, config.snapTargetDir));
        switch (exitCode) {
            case 0:
            case 1:
            case 2:
                logger.logLine("Robocopy succeeded with exit code {0}".format(exitCode), "Setup");
                break;
            default:
                throw new Error("Robocopy encountered an error, exit code: {0}".format(exitCode));
        }

        logger.logLine("Completed JSRT snap setup.", "Setup");
    }

    this.TestHost.registerTestSuite({
        entryPoint: main,
        name: "JSRT",
        setup: setup,
        snapSetup: snapSetup,
        configDefinition: {
            testPath: {
                type: "Directory",
                defaultValue: "{0}\\jsrt\\unittest",
                defaultValueReferences: ["jsBinRoot"],
                description: "The root directory of JSRT unit test DLL."
            },
            numberOfThreads: {
                type: "Number",
                defaultValue: undefined,
                description: "The number of threads to for running unit tests, if not provided defaults to number of processes * 1.25."
            },
            miniDumpOnError: {
                type: "Boolean",
                defaultValue: "{0}",
                defaultValueReferences: ["snap"],
                description: "An override for whether mini dumps should be produced on error, by default true if it is a snap run."
            },
            logFile: {
                type: "File",
                defaultValue: undefined,
                description: "The file sink for logging produced by TAEF."
            },
            timeOut: {
                type: "Number",
                defaultValue: 5 * 60 * 1000,
                description: "The time out for the test in milliseconds, default is 5 min."
            },
            snapSourceDir: {
                type: "Directory",
                defaultValue: "{0}\\jsrt\\unittest",
                defaultValueReferences: ["snapBinRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the share directory containing JSRT test scripts."
            },
            snapTargetDir: {
                type: "Directory",
                defaultValue: "{0}\\jsrt\\unittest",
                defaultValueReferences: ["snapJSRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the local directory where JSRT test scripts will be copied to."
            }
        }
    });
}());
