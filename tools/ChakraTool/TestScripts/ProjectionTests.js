/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    var rlCommand = "";
    var ccFlags = "";
    var variantsConfigObject = {
        interpretedFlags: { type: "String",
 defaultValue: "-nonative" },
        nativeFlags: { type: "String",
 defaultValue: "-forceNative -off:simpleJit -bgJitDelay:0" }
    };

    function main(testContext) {
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var config = testContext.config;
        var storage = testContext.onlyForIntellisense ? Storage(testContext.storage) : testContext.storage;

        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        shell.setStdOutWriter(logger.logTestOutput);
        shell.setStdErrWriter(logger.logTestError);
        shell.setCurrentDirectory(config.testFilesRoot);
        
        //Ensures directory exists
        config.testFilesRoot.getDirectory("Logs");
        
        var variantsConfigApplied = testContext.getConfigFromDefinitionObject(variantsConfigObject, config, shell);
        var testFailed = false;
        var variantsToUse = config.variants;
        variantsToUse.forEach(function (variant) {
            logger.startRun(variant);

            var extraCCFlags = ccFlags + " " + variantsConfigApplied["{0}Flags".format(variant)];
            shell.setEnv("EXTRA_CC_FLAGS", extraCCFlags === undefined ? "" : extraCCFlags);
            logger.logLine("EXTRA_CC_FLAGS = " + shell.getEnv("EXTRA_CC_FLAGS"), "Setup");

            var variantRLCommand = rlCommand + " -nottags:exclude_{0} -nottags:fails_{0}".format(variant);

            logger.logLine("Running variant: {0}, RL Command: \r\n{1}".format(variant, variantRLCommand), "Setup");
            logger.logLine("RL Command:\r\n{0}".format(variantRLCommand), "TestOperation");
            var exitCode = shell.execute(variantRLCommand);
            if (exitCode !== 0) {
                testFailed = true;
            }
            logger.logLine("Exit code returned: {0}".format(exitCode), "TestOperation");

            logger.logTestSummary("\r\nTest Summary for Variant: {0}\r\n".format(variant));

            var file = storage.getFile(config.defaultRLLogPath, IOModes.ForReading);

            var contents = file.readAll();

            file.deleteFile()

            var indexOfUTF8ByteMark = contents.indexOf(String.fromCharCode(239));

            while (indexOfUTF8ByteMark !== -1) {
                contents = contents.substring(0, indexOfUTF8ByteMark) + contents.substring(indexOfUTF8ByteMark + 3, contents.length);
                indexOfUTF8ByteMark = contents.indexOf(String.fromCharCode(239));
            }

            logger.logTestSummary(contents);
            logger.finishRun();
        });

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

    
    function enlistmentSetup(shell, config, logger) {
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

        shell.xCopy(
            "{0}\\jshost.exe".format(config.jsBinRoot),
            "{0}".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        shell.xCopy(
            "{0}\\chakratest.dll".format(config.jsBinRoot),
            "{0}".format(config.projectionsBinRoot),
            "/y", XCopyUNCTarget.Directory);

        logger.logLine("Finished Enlistment setup.", "Setup");
    }


    //-snapTests
    function setup(testContext) {
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var config = testContext.config;
        var configStack = testContext.configStack;
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

        if (config.os === "win7") {
            throw new Error("Win7 is not currently suppoorted.");
        }

        var notTagsArray = [
            "fail",
            "fail_{0}".format(config.os),
            "exclude_{0}".format(config.os),
            "exclude_{0}".format(config.buildType),
            "exclude_{0}".format(config.platform)
        ];
        if (config.snapTests) {
            config.snap = false;
            config.setup = false;
            notTagsArray.push("exclude_snap");
        }
        if (config.rlMode === "asmbase" || config.rlMode === "asmdiff") {
            config.variants = ["native"];
        }
        if (config.nostress) {
            notTagsArray.push("exclude_nostress");
        }
        if (config.snap) {
            config.setup = false;
            config.unregister = true;
            notTagsArray.push("exclude_snap");
        }
        
        if (config.drt) {
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
            notTagsArray.push("exclude_jshost");
            config.unregister = true;
        }
        shell.setCurrentDirectory(config.testRoot);
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

        if (!config.isSnapSetup) {
            if (config.isEnlistmentSetup) {
                enlistmentSetup(shell, config, logger);
            }
            
        }
        
        shell.setEnv("_TestsDirectory", config.testFilesRoot);
        var newPath = "";
        if (config.drt) {
            newPath = "{0};{1};{2};{3}".format(config.testFilesRoot, config.projectionsBinRoot, config.snapJSRoot, shell.getEnv("PATH"));
        } else {
            newPath = "{0};{1};{2}".format(config.testFilesRoot, config.projectionsBinRoot, shell.getEnv("PATH"));
        }

        shell.setCurrentDirectory(config.projectionsBinRoot);
        logger.logLine("Current Directory: '{0}'".format(shell.getCurrentDirectory()), "Config");
        logger.logLine("\r\nPATH:\r\n{0}\r\n".format(shell.getEnv("PATH")), "Config");
        shell.setEnv("PATH", newPath);
        shell.setEnv("WIN_JSHOST_METADATA_BASE_PATH", config.projectionsBinRoot);
        logger.logLine("\r\nPATH:\r\n{0}\r\n".format(shell.getEnv("PATH")), "Config");
        logger.logLine("WIN_JSHOST_METADATA_BASE_PATH = '{0}'".format(shell.getEnv("WIN_JSHOST_METADATA_BASE_PATH")), "Config");

        if (config.force) {
            shell.pushDirectory("{0}\\Functional".format(config.testFilesRoot));
            // For now this is not in JS, will be a task later:
            shell.execute("registerABIs.cmd -unregister");
            shell.popDirectory();
        }
        
        if (config.setup) {
            if (config.objRoot !== undefined) {
                shell.execute("copy {0}\\inetcore\\jscript\\unittest\\ut_rl\\{1}\\rl.exe {2}".format(config.objRoot, config.buildAlt, config.jsRoot));
            }
        }

        storage.attach(config.testFilesRoot);
        shell.pushDirectory(config.testFilesRoot.getDirectory("Functional"));
        // For now this is not in JS, will be a task later:
        shell.execute("registerABIs.cmd");
        shell.popDirectory();

        ccFlags += " -bvt -hosttype:2 -TargetWinRTVersion:3";

        var rlModeFlag = "-" + config.rlMode;
        if (config.rlMode === "asmbase") {
            rlModeFlag = "-asm -base";
            ccFlags += " -Off:InsertNOPs -Prejit";
        } else if (config.rlMode === "asmdiff") {
            rlModeFlag = "-asm -diff";
            ccFlags += " -Off:InsertNOPs -Prejit";
        }
        var threadsFlag = "";
        if (shell.getEnv("NUMBER_OF_PROCESSORS") === "12") {
            threadsFlag = "-threads:1";
        }

        rlCommand = "rl {9} -target:{0} {1} -binary:{2} {3} -nottags:{4} {5} {6} {7} {8}".format(
            config.platform,
            rlModeFlag,
            config.jshostPath,
            config.verbose ? "-verbose" : "",
            notTagsArray.join(',') + (config.nottags.length > 0 ? "," + config.nottags.join(',') : ""),
            config.tags.length > 0 ? "-tags:{0}".format(config.tags.join(',')) : "",
            config.dirtags.length > 0 ? "-dirtags:{0}".format(config.dirtags.join(',')) : "",
            config.dirnottags.length > 0 ? "-dirnottags:{0}".format(config.dirnottags.join(',')) : "",
            config.dirs ? "-dirs:" + config.dirs.join(",") : "-all",
            threadsFlag);

        rlCommand = String.removeDuplicateSpaces(rlCommand);

        logger.logLine("Base RL Command: " + rlCommand, "Setup");
        shell.setEnv("REGRESS", config.testFilesRoot);
        shell.setEnv("TARGET_OS", config.os);
    }

    function snapSetup(testContext) {
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
            var extraction7ZipCommandLine = "7z x -r -aoa -y {0} -o{1}".format(config.testCab, snapTargetDir);
            logger.logLine("Needs 7-zip installed on the test machine and on the %path%");
            logger.logLine(extraction7ZipCommandLine);
            shell.execute(extraction7ZipCommandLine);

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
        name: "Projection",
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

            rlMode: {
                type: 'Enum(["exe", "asmbase", "asmdiff"])',
                defaultValue: "exe",
                description: ""
            },

            variants: {
                type: 'Array',
                defaultValue: ["interpreted", "native"],
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
                defaultValue: true,
                description: ""
            },
            snapTests: {
                type: 'Boolean',
                defaultValue: true,
                description: "Use the -snapTests to run the SNAP test w/o unregistration on exit"
            },
            nostress: {
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
                description: "If this flag is specified, only select directories will run."
            },
            tags: {
                type: 'Array',
                defaultValue: [],
                description: "An array of additional tags to explicitly include."
            },
            nottags: {
                type: 'Array',
                defaultValue: [],
                description: "An array of additional tags to exclude."
            },
            dirtags: {
                type: 'Array',
                defaultValue: [],
                description: "An array of additional tags on dirs to explicitly include."
            },
            dirnottags: {
                type: 'Array',
                defaultValue: [],
                description: "An array of additional tags on dirs to exclude."
            },

            testCab: {
                type: "File",
                defaultValue: undefined,
                description: "SNAP Only: Path to Tests.cab."
            },
            snapTargetDir: {
                type: "Directory",
                defaultValue: "{0}\\ProjectionTests",
                defaultValueReferences: ["snapTargetRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory where projection tests should go."
            },
            snapMDRoot: {
                type: "Directory",
                defaultValue: "{0}",
                defaultValueReferences: ["snapTargetDir"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory where projection MD files should go."
            }
        }
    });
}());

