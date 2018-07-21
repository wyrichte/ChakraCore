/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    function executeEntryPoint(context) {
        test(context, false);
    }

    function setupEntryPoint(context) {
        test(context, true);
    }

    function setupSnapEntryPoint(context) {
        var logger = context.logger;
        var testsToRun = undefined;
        var testSuites = context.testSuites;
        var config = context.config;

        if (config.optin) {
            testsToRun = testSuites.switchedOn;
        } else {
            testsToRun = {};
            for (var test in testSuites.configDefault) {
                if (!testSuites.switchedOff.hasOwnProperty(test)) {
                    testsToRun[test] = testSuites.configDefault[test];
                }
            }
        }

        var atLeastOne = false;
        for (var prop in testsToRun) {
            atLeastOne = true;
            logger.logLine("Selected test script: {0}".format(prop), "Setup");
        }
        if (!atLeastOne) {
            logger.logErrorLine("No test scripts were selected, please select at least one. Exiting. ", "Setup");
            return;
        }
        if (!config.snap) {
            config.snap = true;
            logger.logLine("Forcing SNAP flag on because setupSnap entry point is being used.", "Setup");
        }

        if (!config.drt) {
            config.drt = true;
            logger.logLine("Forcing DRT flag on because setupSnap entry point is being used.", "Setup");
        }

        setupSnap(context, testsToRun);
    }

    function setupSnap(context, testsToRun) {
        var logger = context.onlyForIntellisense ? Logger(context.logger) : context.logger;
        var storage = context.onlyForIntellisense ? Storage(context.storage) : context.storage;
        var shell = context.onlyForIntellisense ? Shell(context.shell) : context.shell;
        var config = context.config;
        var testSuites = context.testSuites;
        var configStack = context.configStack;

        shell.setStdOutWriter(logger.log);
        shell.setStdErrWriter(logger.logError);

        logger.logLine("Setting up for SNAP.", "Setup");

        config.snapBucketID -= 49;

        if (config.snapBinRoot === undefined) {
            throw new Error("The switch 'snapBinRoot' wasn't set and is required.");
        }

        // Based on bucket index, override test scripts to setup (turns on additional ones, as opposed to turning any off)
        if (config.snapBucketIDTestMap.hasOwnProperty(String(config.snapBucketID))) {
            var value = String(config.snapBucketIDTestMap[config.snapBucketID]).toLowerCase();
            if (value != "") {
                if (!testSuites.registered.hasOwnProperty(value.toLowerCase())) {
                    logger.logErrorLine("The mapping of bucket ID '{0}' doesn't result in a valid Test Script - '{1}'.".format(config.snapBucketID, value));
                } else {
                    logger.logLine("Turning on '{0}' test script based on Bucket ID '{1}'".format(value, config.snapBucketID), "Setup");
                    testsToRun[value] = testSuites.registered[value];
                }
            }
        }
        else if (config.snapBucketIDTestMap.hasOwnProperty("default")) {
            var value = String(config.snapBucketIDTestMap["default"]).toLowerCase();
            if (value != "") {
                if (!testSuites.registered.hasOwnProperty(value.toLowerCase())) {
                    logger.logErrorLine("The default mapping for bucket ID's doesn't result in a valid Test Script - '{0}'.".format(value));
                } else {
                    logger.logLine("Turning on '{0}' test script based on default mapping.".format(value), "Setup");
                    testsToRun[value] = testSuites.registered[value];
                }
            }
        } else {
            logger.logErrorLine("No mapping found for Bucket ID '{0}', and default mapping wasn't set. Map: {1}\r\nFor more information, call 'chakra help -infoOn:test.snapBucketIDTestMap'".format(config.snapBucketID, config.snapBucketIDTestMap));
        }
        
        var jsRootDir = context.onlyForIntellisense ? Storage.Directory(config.snapJSRoot) : config.snapJSRoot;

        jsRootDir.attach(storage);
        jsRootDir.cleanDirectory(true);

        var exitCode = shell.roboCopy(config.snapBinRoot, jsRootDir, "*.dll *.exe *.mui *.tlb /njh /njs /ndl /purge /xx");
        if (exitCode.isError) {
            throw new Error("Robocopy encountered an error, exit code: {0}".format(exitCode));
        }

        if (config.snapAdditionalBinRoot !== undefined) {
            logger.logLine("snapAdditionalBinRoot switch was set, copying those files as well.");
            var exitCode = shell.roboCopy(config.snapAdditionalBinRoot, jsRootDir, "*.dll *.exe *.mui *.tlb /njh /njs /ndl /purge /xx");
            if (exitCode.isError) {
                throw new Error("Robocopy encountered an error, exit code: {0}".format(exitCode));
            }
        }
        
        logger.logLine("Modifying config values, and reapplying references.", "Setup");

        config.sdxRoot = config.snapTargetRoot;
        config.binRoot = config.snapTargetRoot;

        configStack.reApplyReferenceResolution(configStack.definition, config, configStack.resolvedReferences, configStack.argumentValues);
        
        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop] +
                (configStack.definition[prop].defaultValueReferences !== undefined
                    ? "; Resolved Default Value References: [" + configStack.definition[prop].defaultValueReferences.join(",") + "]"
                    : ""), "Config");
        }

        shell.setEnv("_NT_SYMBOL_PATH", config.snapTargetRoot);

        logger.logLine("Starting individual test script SNAP setup.", "Setup");
        for (var prop in testsToRun) {
            var testDefinition = testsToRun[prop];
            if (!testDefinition.hasOwnProperty("snapSetup") || testDefinition.snapSetup === undefined) {
                throw new Error("The Test Script '{0}' doesn't have a registered 'snapSetup' function.".format(prop));
            }
            var testContext = testSuites.getTestContextFor(testDefinition);

            logger.logLine();
            logger.logLine("Calling {0} test script SNAP setup.".format(prop), "Setup");
            logger.logLine();
            testDefinition.snapSetup(testContext);
            logger.logLine();
            logger.logLine("Completed call to {0} test script SNAP setup.".format(prop), "Setup");
            logger.logLine();
        }
        logger.logLine("Completed individual test script SNAP setup.", "Setup");

        var snapTargetDir = context.onlyForIntellisense ? Storage.Directory(config.snapTargetRoot) : config.snapTargetRoot;
        snapTargetDir.attach(storage);

        var projectionsDir = snapTargetDir.getDirectory("ProjectionTests");
        var testsDir = projectionsDir.getDirectory("Tests");

        var buildTypeFile = testsDir.getFile("BuildType.cmd", IOModes.ForWriting);

        buildTypeFile.write("set build.type={0}".format(config.buildType));
        logger.logLine("Written Build type to {0}".format(buildTypeFile));

        buildTypeFile.dispose();
        projectionsDir.dispose();
        testsDir.dispose();

        logger.logLine("Completed SNAP setup.", "Setup");
    }

    function test(context, setupOnly) {
        var logger = context.logger;
        var testsToRun = undefined;
        var testSuites = context.testSuites;
        var shell = context.shell;
        var config = context.config;
        
        if (!config.optin) {
            for(var test in testSuites.switchedOn) {
                if (!testSuites.configDefault.hasOwnProperty(test)) {
                    config.optin = true;
                    break;
                }
            }
        }

        if (config.optin) {
            testsToRun = testSuites.switchedOn;
        } else {
            testsToRun = {};
            for (var test in testSuites.configDefault) {
                if (!testSuites.switchedOff.hasOwnProperty(test)) {
                    testsToRun[test] = testSuites.configDefault[test];
                }
            }
        }

        var atLeastOne = false;
        for (var prop in testsToRun) {
            atLeastOne = true;
            logger.logLine("Selected test script: {0}".format(prop), "Setup");
        }
        if (!atLeastOne) {
            logger.logErrorLine("No test scripts were selected, please select at least one. Exiting. ", "Setup");
            return;
        }

        // Setup for snap/drt
        if (config.snap && config.drt) {
            if (config.doSnapSetup) {
                setupSnap(context, testsToRun);
            } else {
                logger.logLine("Skipping snap setup due to override switch 'doSnapSetup'.", "Setup");
            }
        }

        if (setupOnly) {
            logger.logLine();
            logger.logLine("IMPORTANT: Only running test suite setup.", "Setup");
            logger.logLine();
        }
        
        logger.logLine("Register jshost.exe to get correct browser emulation mode", "Setup");
        shell.execute("REG ADD \"HKEY_LOCAL_MACHINE\\SOFTWARE\\MICROSOFT\\INTERNET EXPLORER\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION\" /t REG_DWORD /v jshost.exe /d 11000 /f ");
        
        var atLeastOneFailed = false;

        var combinedSummaries = "";

        var testScriptsThatFailed = [];

        for (var prop in testsToRun) {
            var testDefinition = testsToRun[prop];
            var testContext = testSuites.getTestContextFor(testDefinition);

            try {
                logger.logLine();
                if (testDefinition.setup) {
                    logger.logLine("Calling '{0}' test script setup.".format(prop), "Operation");
                    testDefinition.setup(testContext);
                } else {
                    logger.logLine("Test Script '{0}' didn't register a setup function.".format(prop), "Operation");
                }
            }
            catch (ex) {
                throw ex;
                logger.logException("{0}.setup".format(prop), ex);
                testContext.storage.dispose();
                continue;
                testScriptsThatFailed.push(prop);
                atLeastOneFailed = true;
            }
            if (setupOnly) {
                continue;
            }

            try {
                logger.logLine("Calling '{0}' test script entry point.".format(prop), "Operation");
                var returnCode = testDefinition.entryPoint(testContext);
                if (returnCode !== undefined && returnCode !== 0) {
                    atLeastOneFailed = true;
                    testScriptsThatFailed.push(prop);
                }
                combinedSummaries += testContext.logger.getCompleteTestSummary();
            }
            catch (ex) {
                logger.logException("{0}.execute".format(prop), ex);
                testScriptsThatFailed.push(prop);
                atLeastOneFailed = true;
            }
            logger.logLine();
            testContext.storage.dispose();
        }
        if (!setupOnly) {
            logger.logLine("================ TEST SUMMARIES ================")
            logger.logLine(combinedSummaries);
            logger.logLine("============ END OF TEST SUMMARIES =============")
        } else {
            logger.logLine();
            logger.logLine("================ SETUP COMPLETE ================");
        }
        if (atLeastOneFailed) {
            throw new Error("Test scripts '{0}' failed!".format(testScriptsThatFailed.join(', ')));
        }
    }

    TestHost.registerCommand({
        name: "test",
        entryPoint: executeEntryPoint,
        namedEntryPoints: {
            setup: setupEntryPoint,
            setupSnap: setupSnapEntryPoint
        },
        configDefinition: {
            doSnapSetup: {
                type: "Boolean",
                defaultValue: "{0}",
                defaultValueReferences: ["drt"],
                description: "An override to disable snap setup when '-drt' and '-snap' is provided in order to support seperate setup/run command lines."
            },
            snapOwnBinRoot: {
                type: "Directory",
                defaultValue: "\\\\iesnap\\SNAP\\bin",
                description: "SNAP Only: The root folder for SNAP binaries."
            },
            snapBinRoot: {
                type: "Directory",
                defaultValue: undefined,
                description: "SNAP Only: A full path to the binaries to setup on the DRT machine. The binaries may be built as part of a SNAP job, enlistment, official build or otherwise."
            },
            snapBucketID: {
                type: "Number",
                defaultValue: 0,
                description: "SNAP Only: A JS DRT bucket index as defined in the IESNAP model file: \\\\iesnap\\SNAP\\model\\fbl_ie_script_dev.model.xml"
            },
            snapTargetRoot: {
                type: "Directory",
                defaultValue: "{0}\\jscript",
                defaultValueReferences: ["systemDrive"],
                description: "SNAP Only: A path to the location on the local machine into which we will copy the test collateral."
            },
            snapJSRoot: {
                type: "Directory",
                defaultValue: "{0}\\jscript",
                defaultValueReferences: ["snapTargetRoot"],
                description: "SNAP Only: A path to the location on the local machine into which we will copy the jscript binaries."
            },
            snapAdditionalBinRoot: {
                type: "Directory",
                defaultValue: undefined,
                description: ""
            },
            snapBucketIDTestMap: {
                type: "Dictionary",
                defaultValue: { "0": "", "7": "JSRT", "9": "Projection", "default": "Unit" },
                description: "The mapping between Bucket ID and required tests."
            },
            testOutputRelativePath: {
                type: String,
                defaultValue: "{0}{1}",
                defaultValueReferences: ["platform", "buildType"],
                description: "The relative path of where logs for the current session should be stored, under root logging directory of the test script."
            },
            testOutputFileNameFormat: {
                type: String,
                defaultValue: "{0}.output.log",
                description: "The format to be used for naming output file for a run."
            },
            testErrorLogFileNameFormat: {
                type: String,
                defaultValue: "{0}.errors.log",
                description: "The format to be used for naming error file for a run."
            },
            testSummaryLogFileNameFormat: {
                type: String,
                defaultValue: "{0}.summary.log",
                description: "The format to be used for naming summary file for a run."
            }
        }
    });
}());
