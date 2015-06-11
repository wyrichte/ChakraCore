/// <reference path="..\Utils.js" /> 
/// <reference path="..\JSExtensions.js" /> 
/// <reference path="..\main.js" />

(function () {
    function entryPoint(context) {
        var args = context.args;
        var registeredTestSuites = context.testSuites.registered;
        var commands = context.commandDefinitions;
        var enabledByDefault = context.testSuites.configDefault;
        var askedArgs = context.config.infoOn;
        var definitions = context.configStack.definition;
        var logger = context.logger;

        var argInfo = {};
        var testSuiteInfo = {};

        var inflatedCommandContexts = {};

        function createArgumentInfo(key, definition) {
            var defaultValue = definition.defaultValue;
            if (definition.defaultValueReferences !== undefined && definition.defaultValue !== undefined) {
                defaultValue = String.formatGivenArray(definition.defaultValue, definition.defaultValueReferences.map(function (item) { return "{{0}}".format(item); }));
            }

            return "Option '{0}':\r\n\tType: {1}\r\n\tDescription: {2}\r\n\tDefault Value: {3}{4}\r\n\tBacked By Environment: {5}{6}\r\n".format(
                key,
                definition.type.name,
                definition.description ? definition.description : "No description available",
                defaultValue,
                "",
                definition.lookupFromEnvironment ? "Yes" : "No",
                definition.lookupFromEnvironment ? "\r\n\tEnvironment Key: {0}".format(definition.environmentKey ? definition.environmentKey : arg) : "");
        }

        if (askedArgs === undefined || askedArgs.length === 0) {
            logger.logLine("Welcome to the Chakra Tool - if you have any questions, ping 'anborod'.");
            logger.logLine();
            logger.logLine("The syntax of the tool is as follows:");
            logger.logLine("\tchakra <command[-<Named Entry Point>]> [<arguments>]");
            logger.logLine();
            logger.logLine("The following command's are registered with the tool, call 'chakra help -infoOn:<command>' for more info:")
            for (var prop in commands) {
                logger.logLine("\t{0}".format(prop));
            }
            logger.logLine();
            logger.logLine("The following test suite's are registered with the system, call 'chakra help -infoOn:<test suite>' for more info:")
            for (var suite in registeredTestSuites) {
                logger.logLine("\t{0}".format(registeredTestSuites[suite].name));
            }
            logger.logLine("\r\nThe system has the following options, call 'chakra help -infoOn:<option>' for more info:");
            for (var arg in definitions) {
                logger.logLine("\t{0}".format(arg));
            }
        } else {
            for (var i = 0; i < askedArgs.length; i++) {
                var argParts = askedArgs[i].split('.');
                var arg = argParts[0];
                var found = false;
                if (definitions.hasOwnProperty(arg)) {
                    logger.logLine(createArgumentInfo(arg, definitions[arg]));
                    found = true;
                }
                if (commands.hasOwnProperty(arg)) {
                    arg.toLowerCase();
                    var command = commands[arg];

                    if (argParts.length === 1) {
                        logger.logLine("Command '{0}':\r\n\r\n{1}".format(arg, command.description ? command.description : "No description available."));
                        if (command.namedEntryPoints) {
                            logger.logLine("The command has the following variations: \r\n\t(called by 'chakra {0}-<variation> <arguments>'): ".format(arg));
                            for (var entryPoint in command.namedEntryPoints) {
                                logger.logLine("\t\t{0}".format(entryPoint));
                            }
                        }

                        if(command.configDefinition)
                        {
                            logger.logLine("\r\nHas the following options: ");
                            for (var arg in command.configDefinition) {
                                logger.logLine("\t\t{0}".format(arg));
                            }
                        }
                        
                    } else if (argParts.length === 2) {
                        var option = argParts[1];
                        if (command.configDefinition && command.configDefinition[option]) {
                            if (!inflatedCommandContexts.hasOwnProperty(arg) && arg !== "help") {
                                inflatedCommandContexts[arg] = context.inflateCommandContext(command);
                            }
                            var commandContext = inflatedCommandContexts[arg];

                            logger.logLine(createArgumentInfo(option, commandContext.configStack.definition[option]));
                        }
                    } else {
                        logger.logErrorLine("Malformed argument to optIn  '{0}'.".format(askedArgs[i]), "Help");
                    }
                    found = true;
                }
                if (registeredTestSuites.hasOwnProperty(arg.toLowerCase())) {
                    arg = arg.toLowerCase();
                    if (argParts.length === 1) {
                        var testSuite = registeredTestSuites[arg];
                        var enabledByDefault = enabledByDefault.hasOwnProperty(arg);
                        var descritpion = testSuite.description ? testSuite.description : "No description available.";
                        logger.logLine("Test Suite '{0}' - {1} \r\n\tIt is {2} by default, and has the following options: \r\n\t    (call 'chakra help -infoOn:{0}.<option>')".format(arg, descritpion, enabledByDefault ? "enabled" : "disbabled"));
                        var testContext = context.testSuites.getTestContextFor(testSuite);
                        var testArgDefinitions = testContext.configStack.definition;
                        for (var arg in testArgDefinitions) {
                            logger.logLine("\t\t{0}".format(arg));
                        }
                        testContext.storage.dispose();
                    } else if (argParts.length === 2) {
                        var testSuite = registeredTestSuites[arg];
                        var testContext = context.testSuites.getTestContextFor(testSuite);
                        var testArgDefinitions = testContext.configStack.definition;
                        if (testArgDefinitions[argParts[1]]) {
                            logger.logLine("{0} Test Suite - {1}".format(testSuite.name, createArgumentInfo(argParts[1], testArgDefinitions[argParts[1]])));
                        }

                        testContext.storage.dispose();
                    } else {
                        logger.logErrorLine("Malformed argument to optIn  '{0}'.".format(askedArgs[i]), "Help");
                    }

                    logger.logLine();
                    found = true;
                }
                if (!found) {
                    logger.logLine("'{0}' is not recognized to be an option or a test suite. Note, arguments are case sensitive!\r\n".format(arg));
                }
            }
        }

        for(var prop in inflatedCommandContexts){
            context.cleanupCommandContext(inflatedCommandContexts[prop]);
        }
    }

    TestHost.registerCommand({
        name: "help", entryPoint: entryPoint, configDefinition: {
            infoOn: {
                type: "Array",
                defaultValue: undefined,
                description: "Used by the help command to identify which arguments to show help for."
            },
            dummy: {
                type: "Number",
                defaultValue: undefined,
                description:"Test"
            }
        }
    });
}());