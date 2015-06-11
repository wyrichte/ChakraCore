/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    var standardBinaries = ["jc.exe", "jshost.exe", "chakra.dll.fre", "chakratest.dll"];
    var diagBinaries = ["jdtest.exe", "jd.dll", "ut_jscript.dll", "ut_jsdiag.dll", "chakradiag.dll.fre", "chakradiagtest.dll"]
    var otherBinaries = ["jsetwconsole.exe", "jscript9internalcounters.dll", "nativeunittests.dll", "jsgen.exe", "jsglass.exe", "chakrals.dll.fre", "rl.exe", "testHost.exe"];

    function entryPoint(context) {
        var shell = context.shell;
        var logger = context.logger;
        var storage = context.storage;
        var config = context.config;

        shell.setStdOutWriter(logger.logRaw);
        shell.setStdErrWriter(logger.logError);

        if (config.shareTag === undefined) {
            throw new Error("The option -shareTag was not set. Run chakra help -infoOn:share for more information.");
        }

        var directory = undefined;
        try {
            directory = storage.getDirectory(config.fullSharePath);
            directory = directory.getDirectory(config.shareTag).getDirectory("{0}{1}".format(config.platform, config.buildType));
        }
        catch (ex) {
            throw new Error("Encountered an error when trying to open the share path '{0}'. Error message: '{1}'.".format(config.fullSharePath, ex.message));
        }

        try{
            directory.cleanDirectory(true);
        }
        catch (ex) {
            logger.logError("Failed to clean the target directory '{0}', will try and continue. Error message: {1}".format(directory, ex), "IO");
        }

        function copyJScriptItem(name) {
            shell.xCopy("{0}\\{1}".format(config.jsBinRoot, name), "{0}\\{1}".format(directory, name), "/y", XCopyUNCTarget.File);
            if (config.symbols) {
                copySymbols(name, "jscript");
            }
        }
        function copyFreItem(name) {
            shell.xCopy("{0}\\{1}".format(config.binRoot, name), "{0}\\{1}".format(directory, name), "/y", XCopyUNCTarget.File);
            if (config.symbols) {
                copySymbols(name, "retail");
            }
        }
        function copySymbols(name, retailOrJscript) {
            var parts = name.split('.');
            name = parts[0];
            var dllOrExe = parts[1];
            shell.xCopy("{0}\\Symbols.pri\\{1}\\{2}\\{3}.pdb".format(config.binRoot, retailOrJscript, dllOrExe, name), "{0}\\{1}.pdb".format(directory, name), "/y", XCopyUNCTarget.File);
        }

        function copyItem(item) {
            var parts = item.split('.');
            var func = parts.length === 3 ? copyFreItem : copyJScriptItem;
            if (config[parts[0]]) {
                func("{0}.{1}".formatGivenArray(parts));
            }
        }

        standardBinaries.forEach(copyItem);
        diagBinaries.forEach(copyItem);
        otherBinaries.forEach(copyItem);

        logger.log("\r\nCopied to clip board:\r\n{0}\r\n".format(directory));
        shell.execute("echo {0} | clip".format(directory));
    }
    var configDef = {
        hostName: {
            type: "String",
            defaultValue: undefined,
            description: "The name of the host on which the shared location is located.",
            environmentKey: "COMPUTERNAME",
            lookupFromEnvironment: true
        }, shareFolderPath: {
            type: "String",
            defaultValue: "public",
            description: "The path (excluding the host name) of the shared folder."
        }, fullSharePath: {
            type: "String",
            defaultValue: "\\\\{0}\\{1}",
            defaultValueReferences: ["hostName", "shareFolderPath"],
            description: "The full path of the share, note, that shareTag will add a folder under it. So ideally use is -hostName:\"<machine>\" -shareFolderPath:\"<user>\"."
        }, shareTag: {
            type: "String",
            defaultValue: undefined,
            description: "The tag of the current item to share."
        }, symbols: {
            type: "Boolean",
            defaultValue: true,
            description: "A flag to include symbols for each of the binaries being copied."
        }, standardBinaries: {
            type: "Boolean",
            defaultValue: true,
            description: "A group flag for JC, JSHost, Chakra and ChakraTest."
        }, jc: {
            type: "Boolean",
            defaultValue: "{0}",
            defaultValueReferences: ["standardBinaries"],
            description: "A flag to include the JC binary in the copy."
        }, jshost: {
            type: "Boolean",
            defaultValue: "{0}",
            defaultValueReferences: ["standardBinaries"],
            description: "A flag to include the JSHost binary in the copy."
        }, jscript9test: {
            type: "Boolean",
            defaultValue: "{0}",
            defaultValueReferences: ["standardBinaries"],
            description: "A flag to include the chakratest binary in the copy."
        }, jscript9: {
            type: "Boolean",
            defaultValue: "{0}",
            defaultValueReferences: ["standardBinaries"],
            description: "A flag to include the chakra binary in the copy."
        }, chakratest: {
            type: "Boolean",
            defaultValue: "{0}",
            defaultValueReferences: ["standardBinaries"],
            description: "A flag to include the chakratest binary in the copy."
        }, chakra: {
            type: "Boolean",
            defaultValue: "{0}",
            defaultValueReferences: ["standardBinaries"],
            description: "A flag to include the chakra binary in the copy."
        }, diagBinaries: {
            type: "Boolean",
            defaultValue: false,
            description: "A group flag for ChakraDiag, ChakraDiagTest, JDTest."
        }
    };

    function addConfig(bins, defaultValue, defaultValueReference) {
        bins.forEach(function (item) {
            var item = item.split('.')[0];
            configDef[item] = {
                type: "Boolean",
                defaultValue: defaultValue,
                description: "A flag to include the {0} binary in the copy.".format(item)
            };
            if (defaultValueReference !== undefined) {
                configDef[item].defaultValueReferences = defaultValueReference;
            }
        });
    };

    addConfig(standardBinaries, "{0}", ["standardBinaries"]);
    addConfig(diagBinaries, "{0}", ["diagBinaries"]);
    addConfig(otherBinaries, false, undefined);

    TestHost.registerCommand({
        name: "share", entryPoint: entryPoint, 
        description: "A small command which allows quick sharing of binaries. There are three main components: \r\n\r\n    hostName: \t\tthe server name with the shares, defaults to the \r\n\t\t\tcurrent machine.\r\n\r\n    shareFolderPath: \tThe share path which must exist and is accesible by \r\n\t\t\t\\\\<hostName>\\<shareFolderPath>\\<shareTag>. \r\n\t\t\tDefaults to 'public'.\r\n\r\n    shareTag: \t\tThe folder that will hold your share files (grouped by \r\n\t\t\tflavour folders). If it doesn't exist, it will be \r\n\t\t\tcreated.\r\n\r\nThe reason for the splits is to allow for very quick shares:\r\n\tchakra share -shareTag:Bug12345Fix\r\n\t\tWill share to '\\\\<hostName>\\public\\Bug12345Fix' folder.\r\n\r\n\tchakra share -shareTag:BuddyTest1 -shareFolderPath:anborod\r\n\t\tWill share to '\\\\<hostName>\\anborod\\BuddyTest1' folder.",
        configDefinition: configDef
    });
}());