/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    var rlCommand = "";
    var ccFlags = " -bvt";

    var variantsConfigObject = {
        interpretedFlags: {
            type: "String",
            defaultValue: "-maxInterpretCount:1 -maxSimpleJitRunCount:1 -BaselineMode -bgjit- {0}", defaultValueReferences: ["dynamicProfileCacheFlag"]
        },
        interpretedRLFlags: {
            type: "String",
            defaultValue: "-appendtestnametoextraccflags"
        },

        nonativeFlags: {
            type: "String",
            defaultValue: "-nonative"
        },
        nonativeRLFlags: {
            type: "String",
            defaultValue: "-nottags:exclude_interpreted -nottags:fails_interpreted"
        },

        dynapogoFlags: {
            type: "String",
            defaultValue: "-forceNative -off:simpleJit -BaselineMode -bgJitDelay:0 {0}", defaultValueReferences: ["dynamicProfileInputFlag"]
        },
        dynapogoRLFlags: {
            type: "String",
            defaultValue: "-appendtestnametoextraccflags"
        },

        forcedeferparseFlags: {
            type: "String",
            defaultValue: "-forceDeferParse{0}", defaultValueReferences: ["dynamicProfileCacheFlag"]
        },
        forcedeferparseRLFlags: {
            type: "String",
            defaultValue: "-appendtestnametoextraccflags -nottags:exclude_forcedeferparse "
        },

        nodeferparseFlags: {
            type: "String",
            defaultValue: "-noDeferParse", defaultValueReferences: ["dynamicProfileCacheFlag"]
        },
        nodeferparseRLFlags: {
            type: "String",
            defaultValue: "-appendtestnametoextraccflags -nottags:exclude_nodeferparse"
        },

        forceundodeferFlags: {
            type: "String",
            defaultValue: "-forceUndoDefer -nottags:exclude_forceundodefer {0}", defaultValueReferences: ["dynamicProfileCacheFlag"]
        },
        forceundodeferRLFlags: {
            type: "String",
            defaultValue: "-appendtestnametoextraccflags"
        },

        bytecodeserializedFlags: {
            type: "String",
            defaultValue: "-recreatebytecodefile -serialized:{0}\ByteCode", defaultValueReferences: ["tempRoot"]
        },
        bytecodeserializedRLFlags: {
            type: "String",
            defaultValue: "-nottags:exclude_serialized"
        },

        forceserializedFlags: {
            type: "String",
            defaultValue: "-forceserialized"
        },
        forceserializedRLFlags: {
            type: "String",
            defaultValue: "-nottags:exclude_serialized"
        }
    };

    //Returns 0 on success
    function main(testContext) {
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var config = testContext.config;
        var storage = testContext.onlyForIntellisense ? Storage(testContext.storage) : testContext.storage;

        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;
        shell.setStdOutWriter(logger.logTestOutput);
        shell.setStdErrWriter(logger.logTestError);

        var testFailed = false;

        var variantsConfigApplied = testContext.getConfigFromDefinitionObject(variantsConfigObject, config, shell);

        var variantsToUse = (config.dirs && config.dirs.length) > 0 ? config.variantsWhenDirs : config.variants;
        variantsToUse.forEach(function (variant) {
            logger.startRun(variant);

            var extraCCFlags = ccFlags + " " + variantsConfigApplied["{0}Flags".format(variant)];
            shell.setEnv("EXTRA_CC_FLAGS", extraCCFlags === undefined ? "" : extraCCFlags);
            logger.logLine("EXTRA_CC_FLAGS = " + shell.getEnv("EXTRA_CC_FLAGS"), "Setup");
            rlVariantFlags = variantsConfigApplied["{0}RLFlags".format(variant)];
            rlFlags = "-nottags:exclude_{0},fails_{0} {1}".format(variant, rlVariantFlags === undefined ? "" : rlVariantFlags);
            var variantRLCommand = rlCommand + " " + rlFlags;

            logger.logLine("Running variant: {0}, RL Command: \r\n{1}".format(variant, variantRLCommand), "TestOperation");
            var exitCode = shell.execute(variantRLCommand);
            if (exitCode !== 0) {
                testFailed = true;
            }
            logger.logLine("Exit code returned: {0}".format(exitCode), "TestOperation");

            logger.logTestSummary("\r\nTest Summary for Variant: {0}\r\n".format(variant));
            var file = storage.getFile(config.defaultRLLogPath, IOModes.ForReading);
            logger.logTestSummary(file);
            file.deleteFile()

            logger.finishRun();
        });

        //Cleanup
        shell.setStdOutWriter(logger.logRaw);
        shell.setStdErrWriter(logger.logErrorRaw);
        shell.execute("del {0}\\ByteCode*.bc".format(config.tempRoot));
        shell.execute("del {0}\\NativeCode*.dll".format(config.tempRoot));
        return testFailed ? 1 : 0;
    }

    function setup(testContext) {
        var shell = testContext.onlyForIntellisense ? Shell(testContext.shell) : testContext.shell;
        var config = testContext.config;
        var logger = testContext.onlyForIntellisense ? Logger(testContext.logger) : testContext.logger;

        logger.setFlushPerWrite(true);
        shell.setCurrentDirectory(config.testRoot);
        shell.setStdOutWriter(logger.logRaw);
        shell.setStdErrWriter(logger.logErrorRaw);

        shell.setEnv("PATH", "{0};{1}".format(shell.getEnv("PATH"), config.jsBinRoot));

        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop], "Config");
        }

        if (config.platform === "fre" && !config.freTest) {
            confing.dynamicProfileCacheFlag = "";
            config.dynamicProfileInputFlag = "";
        }

        var notTagsArray = [
            "fail",
            "edit",
            "fail_{0}".format(config.os),
            "exclude_{0}".format(config.os),
            "exclude_{0}".format(config.buildType),
            "exclude_{0}".format(config.platform)
        ];

        var tagsArray = [];

        if (config.snap) {
            notTagsArray.push("exclude_snap");
        }

        if (config.html) {
            tagsArray.push("html");
        } else {
            notTagsArray.push("html");
        }

        if (config.drt) {
            notTagsArray.push("exclude_drt");
            notTagsArray.push("exclude_jshost");
        } else {
            var exitCode = shell.execute("call {0}\\runjs setupTesthost".format(config.jsToolsRoot));
            if(exitCode !== 0){
                throw new Error("Failed to call setupTesthost");
            } else {
                logger.logLine("Successfully called setupTesthost", "Setup");
            }
            var exitCode = shell.execute("call {0}\\runjs setupJdTest".format(config.jsToolsRoot));
            if(exitCode !== 0){
                throw new Error("Failed to call setupJdTest");
            } else {
                logger.logLine("Successfully called setupJdTest", "Setup");
            }
            var exitCode = shell.execute("call {0}\\runjs setupJsHostTest".format(config.jsToolsRoot));
            if(exitCode !== 0){
                throw new Error("Failed to call setupJsHostTest");
            } else {
                logger.logLine("Successfully called setupJsHostTest", "Setup");
            }
            var exitCode = shell.execute("call {0}\\runjs setupWindowsGlobalization".format(config.jsToolsRoot));
            if(exitCode !== 0){
                throw new Error("Failed to call setupWindowsGlobalization");
            } else {
                logger.logLine("Successfully called setupWindowsGlobalization", "Setup");
            }
        }

        if (config.buildType === "chk") {
            ccFlags += " -DumpOnCrash ";
        }
        var rlModeFlag = "-" + config.rlMode;
        if (config.rlMode === "asmbase") {
            rlModeFlag = "-asm -base";
        } else if (config.rlMode === "asmdiff") {
            rlModeFlag = "-asm -diff";
        }

        rlCommand = String.format("rl -target:{0} -nottags:{4} {5} {6} {7} {1} {2} {3} {8}",
            config.platform,
            rlModeFlag,
            config.verbose ? "-verbose" : "",
            config.dirs ? "-dirs:" + config.dirs.join(",") : "-all",
            notTagsArray.join(',') + (config.nottags.length > 0 ? "," + config.nottags.join(',') : ""),
            config.tags.length > 0 ? "-tags:{0}".format(config.tags.join(',')) : "",
            config.dirtags.length > 0 ? "-dirtags:{0}".format(config.dirtags.join(',')) : "",
            config.dirnottags.length > 0 ? "-dirnottags:{0}".format(config.dirnottags.join(',')) : "",
            tagsArray.length > 0 ? "-tags:{0}".format(tagsArray.join(',')) : "");

        rlCommand = String.removeDuplicateSpaces(rlCommand);

        shell.setEnv("REGRESS", config.testRoot);
        shell.setEnv("TARGET_OS", config.os);
        logger.logLine("Base RL Command: " + rlCommand, "Setup");
        shell.execute("del /s /q profile.dpl.* > nul");
    }

    function snapSetup(testContext) {
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
            var extraction7ZipCommandLine = "7z x -r -aoa -y {0} -o{1} *.*".format(config.testCab, snapTargetDir);
            logger.logLine("Needs 7-zip installed on the test machine and on the %path%");
            logger.logLine(extraction7ZipCommandLine);
            shell.execute(extraction7ZipCommandLine);

            if (!Storage.DoesFileExist("{0}\\Tools\\runjs.bat".format(snapTargetDir))) {
                throw new Error("Expected files are not found after extracting unit tests cab, aborting!");
            }
        } else {
            logger.logErrorLine("No test collateral was provided!", "Setup");
        }

        logger.logLine("Setting up test hosts.", "Setup");
        
        var exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupTesthost {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupTesthost");
        }

        exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupJdTest {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupJdTest");
        }

        exitCode = shell.execute("call {0}\\Tools\\runjs.bat setupJsHostTest {1} {2}".format(snapTargetDir, config.snapBinRoot, config.snapJSRoot));
        if(exitCode === 1){
            throw new Error("Failed to call setupJsHostTest");
        }

        logger.logLine("Completed Unit snap setup.", "Setup");
    }

    this.TestHost.registerTestSuite({
        entryPoint: main,
        name: "Unit",
        setup: setup,
        snapSetup: snapSetup,
        configDefinition: {
            dirs: {
                type: 'Array',
                defaultValue: undefined,
                description: "If this flag is specified, only select directories will run."
            },
            html: { 
                type: "Boolean",
                defaultValue: false,
                description: "Specifies whether HTML tests will be run."
            },
            rlMode: {
                type: 'Enum(["exe", "asmbase", "asmdiff"])',
                defaultValue: "exe",
                description: ""
            },
            verbose: {
                type: 'Boolean',
                defaultValue: true,
                description: ""
            },
            testRoot: {
                type: 'Directory',
                defaultValue: "{0}\\unittest",
                defaultValueReferences: ["jsRoot"],
                description: "The root directory for unittest files."
            },
            defaultRLLogPath: {
                type: 'String',
                defaultValue: "{0}\\logs\\rl.log",
                defaultValueReferences: ["testRoot"]
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
            variants: {
                type: 'Array',
                defaultValue: ["interpreted", "dynapogo"],
                description: "Variants to run."
            },
            variantsWhenDirs: {
                type: 'Array',
                defaultValue: ["interpreted", "dynapogo"],
                description: "Variants to run when 'dirs' flag is specified."
            },
            freTest: {
                type: 'Boolean',
                defaultValue: true, decription: "Overwriting global freTest flag."
            },
            dynamicProfileCacheFlag: {
                type: 'String',
                defaultValue: "-dynamicprofilecache:profile.dpl",
                description: "The flag that will be used if dynamicProfile collection is turned."
            },
            dynamicProfileInputFlag: {
                type: 'String',
                defaultValue: "-dynamicprofileinput:profile.dpl",
                description: "The flag that will be used if dynamicProfile input is turned."
            },
            snapTargetDir: {
                type: "Directory",
                defaultValue: "{0}\\inetcore\\jscript",
                defaultValueReferences: ["snapTargetRoot"],
                description: "SNAP Only: When calling SNAP setup, this will be resolved to the directory where unit tests should go."
            },
            testCab: {
                type: "File",
                defaultValue: undefined,
                description: "SNAP Only: Path to JScriptTestCollateral.cab."
            }
        }
    });
}());
