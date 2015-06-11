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

        logger.logLine("Starting JSLS run.", "Setup");
        logger.startRun("run");

        var exitCode = shell.execute(runCommand, config.timeOut);
        logger.logLine("Exit code: {0}".format(exitCode), "TestOperation");

        logger.finishRun();
        logger.logLine("Completed JSLS run.", "Setup");
        return exitCode;//0-15 is number of non passing tests
    }

    function setup(testContext) {
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var config = testContext.config;
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;


        shell.setEnv("PATH", "{0};{1}".format(shell.getEnv("PATH"), config.jsBinRoot));

        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop], "Config");
        }

        logger.logLine("Starting JSLS setup.", "Setup");

        var snapSelect = "";

        if (config.snap) {
            logger.logLine("Running in SNAP mode.", "Setup");
            snapSelect = " and (not @SNAP=\'No\')";
        }

        var logArg = "";

        if (config.logFile) {
            logger.logLine("TAEF logging will be forwarded to '{0}'.".format(config.logFile), "Setup");
            logArg = ' /enableWttLogging /logFile:"{0}"'.format(config.logFile);
        }

        if (isNaN(config.numberOfThreads)) {
            config.numberOfThreads = Math.floor(config.numberOfProcessors * 1.25);
            logger.logLine("TAEF will use {0} threads for parallel test running.".format(config.numberOfThreads), "Setup");
        }

        runCommand = 'te {0} /select:"(not @Disabled=\'Yes\'){1}" /unicodeoutput:false{2} /stacktraceonerror{3} /isolationLevel:class /logOutput:{5} {4}'.format(
            config.testDLL,
            snapSelect,
            config.numberOfThreads > 1 ? " /parallel:{0}".format(config.numberOfThreads) : "",
            config.miniDumpOnError ? " /minidumponerror" : "",
            logArg,
            config.logLevel);

        logger.logLine("Test Command: {0}".format(runCommand), "Setup");

        shell.setEnv("_NT_SYMBOL_PATH", "{0};{1}".format(shell.getEnv("_NT_SYMBOL_PATH"), config.testPath));
        logger.logLine("_NT_SYMBOL_PATH={0}".format(shell.getEnv("_NT_SYMBOL_PATH")), "Setup");

        logger.logLine("Completed JSLS setup.", "Setup");
    }

    function snapSetup(testContext) {
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        var config = testContext.config;
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        shell.setStdOutWriter(logger.log);
        shell.setStdErrWriter(logger.logError);

        logger.logLine("Started JSLS snap setup.", "Setup");
        
        var exitCode = shell.roboCopy(config.snapSourceDir, config.snapTargetDir, "*.dll *.exe *.mui *.tlb /njh /njs /ndl /purge /xx");
        if (exitCode.isError) {
            throw new Error("Robocopy encountered an error: {0}".format(exitCode));
        }
        
        exitCode = shell.roboCopy(config.snapReferenceDir, config.snapTargetDir, "*.js /njh /njs /ndl /purge /xx");
        if (exitCode.isError) {
            throw new Error("Robocopy encountered an error: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\chakrals.dll".format(config.snapBinRoot), config.snapTargetDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }

        exitCode = shell.xCopy("{0}\\JSLS\\Microsoft.VisualStudio.QualityTools.UnitTestFramework.dll".format(config.snapOwnBinRoot), config.snapTargetDir, "/y");
        if (!exitCode.isSuccess) {
            throw new Error("xcopy encountered an error, exit code: {0}".format(exitCode));
        }
    }

    this.TestHost.registerTestSuite({
        entryPoint: main,
        name: "JSLS",
        setup: setup,
        snapSetup: snapSetup,
        configDefinition: {
            testPath: {
                type: "Directory",
                defaultValue: "{0}\\jsls\\unittest",
                defaultValueReferences: ["jsBinRoot"],
                description: "The root directory of JSLS unit test DLL."
            },
            testDLL: {
                type: "File",
                defaultValue: "{0}\\DirectAuthorCheckinTests.dll",
                defaultValueReferences: ["testPath"],
                description: "The JSLS dll."
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
                defaultValue: 60 * 60 * 1000,
                description: "The time out for the test in milliseconds, default is 60 min."
            },
            snapSourceDir: {
                type: "Directory",
                defaultValue: "{0}\\jsls\\unittest",
                defaultValueReferences: ["snapBinRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the share directory containing JSLS test scripts."
            },
            snapTargetDir: {
                type: "Directory",
                defaultValue: "{0}\\jsls\\unittest",
                defaultValueReferences: ["snapJSRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the local directory where JSLS test scripts will be copied to."
            },
            snapReferenceDir: {
                type: "Directory",
                defaultValue: "{0}\\inetcore\\jscript\\references",
                defaultValueReferences: ["snapTargetRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory containing JS references."
            },
            logLevel: {
                type: 'Enum(["High", "Low", "LowWithConsoleBuffering", "Lowest"])',
                defaultValue: "Low",
                description: "Sets the output level of the logger. Valid values are:\r\n\t-High: Enables some additional console output such as printing a timestamp next to every trace.\r\n\t-Low: Emits only core events (start, end group, etc) and errors. The log file includes lower priority details preceeding any errors to provide context for failures.\r\n\t-LowWithConsoleBuffering: Same as Low, but includes the context of failures in both the log file and console output.\r\n\t-Lowest: Same as Low, but console output includes only errors, test failures, and the summary of execution."
            }
        }
    });
}());