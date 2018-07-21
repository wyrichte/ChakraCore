/// <reference path="Utils.js" /> 
/// <reference path="JSExtensions.js" /> 

var onlyForIntelisense = true;
this.TestHost = {};
var main = undefined;
(function () {
    //A function used for writing out to StdErr/StdOut
    var stdErrWriter = function (message) { WScript.StdErr.Write(message); }
    var stdOutWriter = function (message) { WScript.StdOut.Write(message); }

    var loadedTestSuites = {};
    var availableCommands = {};

    // Registers a test suite that will be used to run a certain kind of tests as part of this Test Harness.
    // TestSuite Description must contain an object with the following properties:
    // - entryPoint: function
    // - name: String
    // - configDefinition: An object literal that describes the accepted options/switches by the test suite, or
    //      a String denoting the name of the JSON file with the object literal.
    // In addition, the following are optional:
    // - setup: function
    this.TestHost.registerTestSuite = function (testSuiteDescription) {
        try {
            if (testSuiteDescription === undefined || testSuiteDescription === null) {
                stdErrWriter("A test script registration can't be undefined or null.\r\n");
            } else if (testSuiteDescription.name === undefined || testSuiteDescription.entryPoint === undefined ||
                testSuiteDescription.name === null || testSuiteDescription.entryPoint === null) {
                stdErrWriter("A test script registration object must contain a name and an entryPoint that is not null or undefined.\r\n");
            } else if (testSuiteDescription.configDefinition === undefined || testSuiteDescription.configDefinition === null) {
                stdErrWriter("A test script registration object must contain a configDefiniton literal object, or a path to it.\r\n");
            } else if (loadedTestSuites[testSuiteDescription.name] !== undefined) {
                stdErrWriter("A test script with the name '" + testSuiteDescription.name + "' has already been registered.\r\n");
            } else {
                loadedTestSuites[testSuiteDescription.name.toLowerCase()] = testSuiteDescription;
            }
        } catch (ex) {
            stdErrWriter("Error occured when registering test script, message: " + ex.message + "\r\n");
        }
    };

    //This function is really a no-op, but thanks to the way our intelisense works, this will provide the command functions with intelisense
    function provideIntelisense(commandDescription) {
        if (!WScript.Arguments) {
            var toReturn = {};
            for (var prop in commandDescription.configDefinition) {
                toReturn[prop] = {};
            }
            commandDescription.entryPoint({
                logger: new Logger(),
                storage: new Storage(),
                shell: new Shell(),
                config: toReturn
            });
            for (var entryPoint in commandDescription.namedEntryPoints) {
                commandDescription.namedEntryPoints[entryPoint]({
                    logger: new Logger(),
                    storage: new Storage(),
                    shell: new Shell(),
                    config: toReturn
                });
            }
        }
    }


    // Registers a command that could be executed as part of this Test Harness.
    // Command description is an object that has the following properties:
    // - name: the unique name representing the action, can't contain spaces or '-'
    // - entryPoint: the default entry point for this action
    // - [optional] namedEntryPoints: An object map between names of entry points and their functions. 
    //      For example 'chakra test-setup' will look up the command 'test', and call the entry point named 'setup' on it (if available).
    // - [optional] configDefinition:  An object literal that describes the accepted options/switches by the command, or
    //      a String denoting the name of the JSON file with the object literal.
    this.TestHost.registerCommand = function (commandDescription) {
        provideIntelisense(commandDescription);
        try {
            if (commandDescription === undefined || commandDescription === null) {
                stdErrWriter("A command description can't be undefined or null.\r\n");
            } else if (commandDescription.name === undefined || commandDescription.entryPoint === undefined ||
                commandDescription.name === null || commandDescription.entryPoint === null) {
                stdErrWriter("A command description object must contain a name and an entryPoint that is not null or undefined.\r\n");
            } else if (availableCommands[commandDescription.name] !== undefined) {
                stdErrWriter("A command description '{0}' has already been registered.\r\n".format(commandDescription.name));
            } else {
                if (commandDescription.hasOwnProperty("namedEntryPoints")) {
                    var lowerCaseKeyedEntryPoints = {};
                    for (var prop in commandDescription.namedEntryPoints) {
                        lowerCaseKeyedEntryPoints[prop.toLowerCase()] = commandDescription.namedEntryPoints[prop];
                    }
                    commandDescription.namedEntryPoints = lowerCaseKeyedEntryPoints;
                }
                availableCommands[commandDescription.name.toLowerCase()] = commandDescription;

            }
        } catch (ex) {
            stdErrWriter("Error occured when registering command, message: {0}\r\n".format(ex.message));
        }
    }

    // Sets up a context for the command to operate on, the context contains the following:
    // - shell: A shell that wraps the WShell.Object with some extra functionality.
    // - storage: An assortment of functions for working with the file system
    // - args: An object of parsed command line arguments
    // - config: The parsed and resolved options/switches values for the command
    // - configStack: The unflattened stack of options/switches with the values for switches at various stages of config resolution
    // - logger: A helper object that manages logging
    // - testSuites: An object containing an array of registered, turned on/off, and on by default TestSuites
    // - commandDefinitions: The raw availableCommands object
    function configureCommandContext(commandDescription) {
        var globalConfigFile = Storage.GetNonTrackedFile(Storage.FromRelativePath("TestHost.config.def.json"), IOModes.ForReading);

        var shell = new Shell();
        var args = Utils.GetArgumentsStartingAt(1);
        var config = {};
        var configDefinitionObject = Utils.ParseJSON(globalConfigFile.readAll());
        globalConfigFile.dispose();

        // If the command registered it's own config object (either as object literal, or a String path to the json file)
        // then merge it with the base config definition object.
        if (commandDescription.hasOwnProperty("configDefinition")) {
            var commandConfigDefinition = commandDescription.configDefinition;
            //If it's a string, open the file and reassign it to the parsed JSON object
            if (typeof commandDescription.configDefinition === "string") {
                var commandConfigFile = Storage.GetNonTrackedFile(Storage.FromRelativePath(commandDescription.configDefinition), IOModes.ForReading);
                // Pass the config file
                commandConfigDefinition = Utils.ParseJSON(commandConfigFile.readAll());
                commandConfigFile.dispose();
            }

            // Blindly re-assign the property values
            for (var prop in commandConfigDefinition) {
                configDefinitionObject[prop] = commandConfigDefinition[prop];
            }
        }
        // Now apply the merged definition object, and resolve the option/switch values.
        var configStack = Utils.ApplyConfigOn(configDefinitionObject, config, args, shell);

        var storage = config.workingDir ? new Storage(config.workingDir, true) : new Storage(Storage.TestHostRootDirectory, true);;
        var logger = new Logger(config, storage, undefined, stdOutWriter, stdErrWriter, false);
        logger.setFlushPerWrite(true);
        var registredTestSuites = loadedTestSuites;
        var configuredDefaultTestSuites = {};
        var switchedOnTestSuites = {};
        var switchedOffTestSuites = {};

        config.defaultTestsToRun.forEach(function (test) {
            var loweredName = test.toLowerCase();
            if (registredTestSuites.hasOwnProperty(loweredName)) {
                configuredDefaultTestSuites[loweredName] = registredTestSuites[loweredName];
            } else {
                logger.logErrorLine("Test suite '{0}' not found.".format(loweredName), "Config");
            }
        });

        for (var prop in args) {
            if (configStack.definition.hasOwnProperty(prop)) {
                continue;
            }
            else if (registredTestSuites.hasOwnProperty(prop)) {
                if (args[prop]) {
                    switchedOnTestSuites[prop] = registredTestSuites[prop];
                } else if (configuredDefaultTestSuites.hasOwnProperty(prop)) {
                    switchedOffTestSuites[prop] = registredTestSuites[prop];
                }
            } else {
                logger.logLine("Unrecognized argument: '{0}' with value '{1}'".format(prop, args[prop]), "Config");
            }
        }

        for (var prop in config) {
            logger.logLine(prop + " = " + config[prop] +
                (configStack.definition[prop].defaultValueReferences !== undefined
                    ? "; Resolved Default Value References: [" + configStack.definition[prop].defaultValueReferences.join(",") + "]"
                    : ""), "Config");
        }

        return {
            args: args,
            shell: shell,
            config: config,
            configStack: configStack,
            storage: storage,
            logger: logger,
            testSuites: {
                registered: registredTestSuites,
                configDefault: configuredDefaultTestSuites,
                switchedOn: switchedOnTestSuites,
                switchedOff: switchedOffTestSuites,
                getTestContextFor: function (testSuite) {
                    if (testSuite.__testContext__ === undefined) {
                        testSuite.__testContext__ = getTestContextFor(testSuite, config, configStack.definition, storage, args, shell);
                    }
                    return testSuite.__testContext__;
                }
            },
            commandDefinitions: availableCommands,
            inflateCommandContext: configureCommandContext,
            cleanupCommandContext: cleanupCommandContext
        };
    }

    // This function creates a context for individual test suites.
    // Similar to the command context, this returns an object with the following properties:
    // - explicitArgs: An object of parsed command line arguments for this test suite. In the example 'chakra test -unit:"-dirs:ES6 -trace:*", the command line value would be: '-dirs:ES6 -trace:*'
    // - config: The parsed and resolved options/switches values for this specific test suite
    // - configStack: The unflattened stack of options/switches with the values for switches at various stages of config resolution
    // - shell: A shell that wraps the WShell.Object with some extra functionality.
    // - storage: An assortment of functions for working with the file system
    // - logger: A helper object that manages logging
    function getTestContextFor(testSuite, config, configDefinition, storage, args, shell) {
        var testConfig = Utils.CreateObject(config);
        var testSuiteName = testSuite.name.toLowerCase();
        // Extract the command line for the unit test (it is not the entire command line)
        var explicitArgs = (args.hasOwnProperty(testSuiteName) && typeof args[testSuiteName] === "string") ? Utils.ParseArguments(args[testSuiteName], "'") : undefined;

        var configDefinitionObject = testSuite.configDefinition;

        // The configDefinition value could be a path to the JSON file containing the config definition object, in such a case it would be a string.
        if (typeof configDefinitionObject === "string") {
            var file = Storage.GetNonTrackedFile(Storage.FromRelativePath("Packages\\{0}TestSuite\\{1}".format(testSuite.name, configDefinitionObject)), IOModes.ForReading);
            configDefinitionObject = Utils.ParseJSON(file.readAll());
            file.dispose();
        }

        var testConfigStack = Utils.ApplyConfigOn(configDefinitionObject, testConfig, explicitArgs, shell, configDefinition);

        return {
            explicitArgs: explicitArgs,
            config: testConfig,
            configStack: testConfigStack,
            shell: new Shell(),
            storage: new Storage(storage.getDirectory("TestScripts\\".format(testSuite.name)), true),
            logger: new Logger(testConfig, storage, testSuite.name.toLowerCase(), stdOutWriter, stdErrWriter, true),
            getConfigFromDefinitionObject: function (configDefinition, baseConfig, shell) {
                var toReturn = Utils.CreateObject(baseConfig);
                Utils.ApplyConfigOn(configDefinition, toReturn, {}, shell);
                return toReturn;
            }
        };
    }

    // This function calls dispose on the storage object for the context.
    // The Disposal action will close all open files held by the storage object, and delete all temporary files and directories that were created by the storage object.
    function cleanupCommandContext(context) {
        context.storage.dispose();
    }

    function mainEntryPoint() {
        onlyForIntelisense = false;
        if (WScript.Arguments.length < 1) {
            stdErrWriter("Missing extension argument, call syntax:\r\n\tchakra <command[-<Named Entry Point>]> [<arguments>]\r\n\r\nFor help, call: 'chakra help'\r\n")
            WScript.Quit(1);
        }

        // The first argument is the 'command' argument. It is of the following form:
        //      <name>[-<namedEntryPoint>]
        // - The command description is looked up by 'name' value. 
        // - If 'namedEntryPoint' is present, it's value is used for lookup of entryPoint on 'commandDescription.namedEntryPoints' object of the found command,
        //      otherwise the 'commandDescription.entryPoint' is used as the entry point.
        // An example of a use case is calling:
        //      'chakra test-setup' //Will only execute setup part of the test scripts, shares common code with 'chakra test'
        //          as opposed to 
        //      'chakra test'
        var commandArg = WScript.Arguments(0).toLowerCase();
        var commandArgParts = commandArg.split("-");

        if (commandArgParts.length > 2) {
            stdErrWriter("An extra '-' was encountered in the command argument: {0}.\r\n".format(commandArg)).
            WScript.Quit(1);
        }

        if (!availableCommands.hasOwnProperty(commandArgParts[0])) {
            stdErrWriter("The command '{0}' is not found.\r\n".format(commandArgParts[0]));
            WScript.Quit(1);
        }

        var commandDescription = availableCommands[commandArgParts[0]];
        var entryPoint = commandDescription.entryPoint;

        // Look up the named entry point, if it was specified.
        if (commandArgParts.length == 2) {
            if (commandDescription.namedEntryPoints === undefined || commandDescription.namedEntryPoints === null ||
                !commandDescription.namedEntryPoints.hasOwnProperty(commandArgParts[1])) {
                stdErrWriter("The command '{0}' does not have an entry point with the name '{1}'.\r\n".format(commandArgParts[0], commandArgParts[1]));
                WScript.Quit(1);
            }
            entryPoint = commandDescription.namedEntryPoints[commandArgParts[1]];
        }

        // Create the context object for the command
        var commandContext = configureCommandContext(commandDescription);

        try {
            entryPoint(commandContext);
        } catch (ex) {
            throw ex;
            stdErrWriter("The command '{0}' failed with an error: {1}\r\n".format(commandArgParts[0], ex.message));
            cleanupCommandContext(commandContext);
            WScript.Quit(1);
        }

        // Clean up calls dispose, which performs certain clean-up operations.
        cleanupCommandContext(commandContext);
        WScript.Quit(0);
    };
    this.main = mainEntryPoint;
}());