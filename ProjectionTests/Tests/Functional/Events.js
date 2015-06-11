if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var msg = "123";

    function easyPrint(myDoubleVectorString, myDoubleVector) {
        var objectDump = "\n    var " + myDoubleVectorString + "Members = [";
        for (p in myDoubleVector) {
            if (typeof myDoubleVector[p] == 'function') {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myDoubleVector[p] + '\', ' + myDoubleVector[p].length + '],'
            }
            else {
                objectDump = objectDump + '\n        [\'' + p + '\', \'' + typeof myDoubleVector[p] + '\'],';
            }
        }
        objectDump = objectDump + "\n    ];";

        return objectDump;
    }

    function verifyMembers(objectString, actualObject, expectedProperties, typeofString) {
        var objectDump = easyPrint(objectString, actualObject);
        logger.comment("typeof " + objectString + ": " + typeof actualObject);
        logger.comment("Dump of properties : " + objectDump);

        if (typeofString === undefined) {
            verify(typeof actualObject, "object", "typeof " + objectString);
        }
        else {
            verify(typeof actualObject, typeofString, "typeof " + objectString);
        }

        var propertiesIndex = 0;
        for (p in actualObject) {
            // Look in properties
            verify(p, expectedProperties[propertiesIndex][0], objectString + '["' + p + '"]');
            verify(typeof actualObject[p], expectedProperties[propertiesIndex][1], 'typeof ' + objectString + '["' + p + '"]');

            if (typeof actualObject[p] === 'function') {
                verify(actualObject[p].length, expectedProperties[propertiesIndex][2], objectString + '["' + p + '"].length');
                logger.comment('Setting length of function to be 10');
                actualObject[p].length = 10;
                verify(actualObject[p].length, expectedProperties[propertiesIndex][2], objectString + '["' + p + '"].length');
            }
            propertiesIndex++;
        }

        verify(propertiesIndex, expectedProperties.length, 'number of members of ' + objectString);
    }

    function verifyEvObjectPropertyDescriptor(evObjectString, evObject, prop, evObjectTypeString) {
        logger.comment("Verifying property descriptor for " + evObjectString + "." + prop);

        if (evObjectTypeString === undefined) {
            verify(typeof evObject, "object", "typeof " + evObjectString);
        }
        else {
            verify(typeof evObject, evObjectTypeString, "typeof " + evObjectString);
        }
        verify((prop in evObject), true, "(" + prop + " in " + evObjectString + ")");

        logger.comment("Get property descriptor from object");
        var desc = Object.getOwnPropertyDescriptor(evObject, prop);
        assert(desc !== undefined, "desc !== undefined");

        verify(desc.writable, false, "desc.writable");
        verify(desc.enumerable, true, "desc.enumerable");
        verify(desc.configurable, false, "desc.configurable");
        verify(desc.get, undefined, "desc.get");
        verify(desc.set, undefined, "desc.set");
        assert(desc.value !== undefined, "desc.value");
    }

    function createToaster() {
        var toaster = new Fabrikam.Kitchen.Toaster();
        return toaster;
    }

    var myRC1WithEventMembers = [
        ['Animals.IInterface2WithEvent.onevent2', 'function', 1],
        ['addEventListener', 'function', 2],
        ['handler1', 'function', 2],
        ['invokeDelegate', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['invokeEvent_I2E1', 'function', 1],
        ['invokeEvent_I2E3', 'function', 1],
        ['onevent1', 'object'],
        ['onevent2', 'object'],
        ['onevent21', 'object'],
        ['onevent3', 'object'],
        ['removeEventListener', 'function', 2],
        ['wasHandler1Invoked', 'boolean'],
        ['toString', 'function', 0],
    ];

    var staticRC2WithEventMembers = [
        ['addEventListener', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['onevent1', 'object'],
        ['onevent2', 'object'],
        ['removeEventListener', 'function', 2],
    ];

    var myRC2WithEventMembers = [
        ['addEventListener', 'function', 2],
        ['handler1', 'function', 2],
        ['invokeDelegate', 'function', 2],
        ['invokeEvent_I2E1', 'function', 1],
        ['invokeEvent_I2E3', 'function', 1],
        ['onevent2', 'function', 1],
        ['onevent21', 'object'],
        ['onevent3', 'object'],
        ['removeEventListener', 'function', 2],
        ['wasHandler1Invoked', 'boolean'],
        ['toString', 'function', 0],
    ];

    var myRC3WithEventMembers = [
        ['Animals.IInterface3WithEvent.addEventListener', 'function', 1],
        ['addEventListener', 'function', 2],
        ['invokeEvent_I3E1', 'function', 1],
        ['invokeEvent_I3E5', 'function', 1],
        ['onevent31', 'object'],
        ['onevent5', 'object'],
        ['removeEventListener', 'function', 2],
        ['toString', 'function', 0],
    ];

    var myRC4WithEventMembers = [
        ['Animals.IInterface2WithEvent.onevent2', 'function', 1],
        ['Animals.IInterface3WithEvent.addEventListener', 'function', 1],
        ['addEventListener', 'function', 2],
        ['handler1', 'function', 2],
        ['invokeDelegate', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['invokeEvent_I2E1', 'function', 1],
        ['invokeEvent_I2E3', 'function', 1],
        ['invokeEvent_I3E1', 'function', 1],
        ['invokeEvent_I3E5', 'function', 1],
        ['onevent1', 'object'],
        ['onevent2', 'object'],
        ['onevent21', 'object'],
        ['onevent3', 'object'],
        ['onevent31', 'object'],
        ['onevent5', 'object'],
        ['removeEventListener', 'function', 2],
        ['wasHandler1Invoked', 'boolean'],
        ['toString', 'function', 0],
    ];

    var staticRC5WithEventMembers = [
        ['Animals.IInterface3WithEvent.addEventListener', 'function', 1],
        ['addEventListener', 'function', 2],
        ['invokeEvent_I3E1', 'function', 1],
        ['invokeEvent_I3E5', 'function', 1],
        ['onevent31', 'object'],
        ['onevent5', 'object'],
        ['removeEventListener', 'function', 2],
    ];

    var myRC5WithEventMembers = [
        ['Animals.IInterface2WithEvent.onevent2', 'function', 1],
        ['addEventListener', 'function', 2],
        ['handler1', 'function', 2],
        ['invokeDelegate', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['invokeEvent_I2E1', 'function', 1],
        ['invokeEvent_I2E3', 'function', 1],
        ['onevent1', 'object'],
        ['onevent2', 'object'],
        ['onevent21', 'object'],
        ['onevent3', 'object'],
        ['removeEventListener', 'function', 2],
        ['wasHandler1Invoked', 'boolean'],
        ['toString', 'function', 0],
    ];

    var myRC6WithEventMembers = [
        ['Animals.IInterface1WithEvent.onevent1', 'object'],
        ['Animals.IInterface2WithEvent.onevent2', 'function', 1],
        ['Animals.IInterface3WithEvent.addEventListener', 'function', 1],
        ['Animals.IInterface4WithEvent.onevent1', 'object'],
        ['addEventListener', 'function', 2],
        ['handler1', 'function', 2],
        ['invokeDelegate', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['invokeEvent_I2E1', 'function', 1],
        ['invokeEvent_I2E3', 'function', 1],
        ['invokeEvent_I3E1', 'function', 1],
        ['invokeEvent_I3E5', 'function', 1],
        ['invokeEvent_I4E1', 'function', 1],
        ['onevent1', 'function', 1],
        ['onevent2', 'object'],
        ['onevent21', 'object'],
        ['onevent3', 'object'],
        ['onevent31', 'object'],
        ['onevent5', 'object'],
        ['removeEventListener', 'function', 2],
        ['wasHandler1Invoked', 'boolean'],
        ['toString', 'function', 0],
    ];
    var evToastMembers = [
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
        ['message', 'string'],
        ['toString', 'function', 0],
    ];

    var toastMembers = [
        ['message', 'string'],
        ['toString', 'function', 0],
    ];

    var toasterMembers = [
        ['addEventListener', 'function', 2],
        ['electricityReporter', 'object'],
        ['getSameToaster', 'function', 0],
        ['indirectMakeToast', 'function', 1],
        ['indirectToaster', 'object'],
        ['invokePreheatCompleteBackgroundEvents', 'function', 1],
        ['invokeRootedHandler', 'function', 2],
        ['makeToast', 'function', 1],
        ['onindirecttoastcompleteevent', 'object'],
        ['onpreheatcompletebackground', 'object'],
        ['onpreheatstart', 'object'],
        ['onrootedtoastcompleteevent', 'object'],
        ['ontoastcompleteevent', 'object'],
        ['ontoaststartevent', 'object'],
        ['preheatInBackground', 'function', 1],
        ['preheatInBackgroundWithSmuggledDelegate', 'function', 1],
        ['removeEventListener', 'function', 2],
        ['rootedHandler', 'object'],
        ['size', 'object'],
        ['toString', 'function', 0],
    ];

    var evStringMembers = [
        ['0', 'string'],
        ['1', 'string'],
        ['2', 'string'],
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
    ];

    var evDetailStringMembers = [
        ['0', 'string'],
        ['1', 'string'],
        ['2', 'string'],
    ];

    var electricityReporterMembers = [
        ['addEventListener', 'function', 2],
        ['applianceName', 'string'],
        ['getSameElectricityReporter', 'function', 0],
        ['onapplianceswitchoffevent', 'object'],
        ['onapplianceswitchonevent', 'object'],
        ['removeEventListener', 'function', 2],
        ['toString', 'function', 0],
    ];

    var evEmptyObjectMembers = [
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
    ];

    var evToastList5Members = [
        ['0', 'object'],
        ['1', 'object'],
        ['2', 'object'],
        ['3', 'object'],
        ['4', 'object'],
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
        ['append', 'function', 1],
        ['clear', 'function', 0],
        ['first', 'function', 0],
        ['getAt', 'function', 1],
        ['getMany', 'function', 2],
        ['getView', 'function', 0],
        ['indexOf', 'function', 1],
        ['insertAt', 'function', 2],
        ['removeAt', 'function', 1],
        ['removeAtEnd', 'function', 0],
        ['replaceAll', 'function', 1],
        ['setAt', 'function', 2],
        ['size', 'number'],
    ];

    var toastList5Members = [
        ['0', 'object'],
        ['1', 'object'],
        ['2', 'object'],
        ['3', 'object'],
        ['4', 'object'],
        ['append', 'function', 1],
        ['clear', 'function', 0],
        ['first', 'function', 0],
        ['getAt', 'function', 1],
        ['getMany', 'function', 2],
        ['getView', 'function', 0],
        ['indexOf', 'function', 1],
        ['insertAt', 'function', 2],
        ['removeAt', 'function', 1],
        ['removeAtEnd', 'function', 0],
        ['replaceAll', 'function', 1],
        ['setAt', 'function', 2],
        ['size', 'number'],
    ];

    var chefMembers = [
        ['addEventListener', 'function', 2],
        ['capabilities', 'number'],
        ['makeBreakfast', 'function', 1],
        ['name', 'string'],
        ['onmaketoastroundoff', 'object'],
        ['onmultipletoastcompletearray', 'object'],
        ['onmultipletoastcompletecollection', 'object'],
        ['removeEventListener', 'function', 2],
        ['role', 'number'],
        ['toString', 'function', 0],
    ];

    var chefWithEventHandlerMembers = [
        ['addEventListener', 'function', 2],
        ['capabilities', 'number'],
        ['makeBreakfast', 'function', 1],
        ['name', 'string'],
        ['onmaketoastroundoff', 'object'],
        ['onmultipletoastcompletearray', 'object'],
        ['onmultipletoastcompletecollection', 'function', 1],
        ['removeEventListener', 'function', 2],
        ['role', 'number'],
        ['toString', 'function', 0],
    ];

    var evToastArray5Members = [
        ['0', 'object'],
        ['1', 'object'],
        ['2', 'object'],
        ['3', 'object'],
        ['4', 'object'],
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
    ];

    var toastArray5Members = [
        ['0', 'object'],
        ['1', 'object'],
        ['2', 'object'],
        ['3', 'object'],
        ['4', 'object'],
    ];

    var myRC7WithEventMembers = [
        ['Animals.IInterfaceWithOnEvent1.onevent1', 'function', 1],
        ['addEventListener', 'function', 2],
        ['invokeEvent_I1E1', 'function', 1],
        ['invokeEvent_I1E2', 'function', 1],
        ['onevent1', 'object'],
        ['onevent2', 'object'],
        ['removeEventListener', 'function', 2],
        ['toString', 'function', 0],
    ];

    var evRC8WithEventMembers = [
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
        ['addEventListener', 'function', 2],
        ['invokeDelegateEvent', 'function', 1],
        ['invokeInterfaceWithTargetEvent', 'function', 0],
        ['invokeStructEvent', 'function', 1],
        ['invokeStructWithTargetEvent', 'function', 1],
        ['ondelegateevent', 'object'],
        ['oninterfacewithtargetevent', 'object'],
        ['onstructevent', 'object'],
        ['onstructwithtargetevent', 'object'],
        ['removeEventListener', 'function', 2],
        ['toString', 'function', 0],
    ];

    var myRC8WithEventMembers = [
        ['addEventListener', 'function', 2],
        ['invokeDelegateEvent', 'function', 1],
        ['invokeInterfaceWithTargetEvent', 'function', 0],
        ['invokeStructEvent', 'function', 1],
        ['invokeStructWithTargetEvent', 'function', 1],
        ['ondelegateevent', 'object'],
        ['oninterfacewithtargetevent', 'object'],
        ['onstructevent', 'object'],
        ['onstructwithtargetevent', 'object'],
        ['removeEventListener', 'function', 2],
        ['target', 'function', 1],
        ['toString', 'function', 0],
    ];

    var evStructForStructEventMembers = [
        ['structId', 'number'],
        ['structData', 'number'],
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
    ];

    var structForStructEventMembers = [
        ['structId', 'number'],
        ['structData', 'number'],
    ];

    var evStructForStructWithTargetEventMembers = [
        ['structId', 'number'],
        ['structData', 'number'],
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
    ];

    var structForStructWithTargetEventMembers = [
        ['structId', 'number'],
        ['structData', 'number'],
        ['target', 'boolean'],
    ];

    runner.addTest({
        id: 1,
        desc: 'InvalidAddAndRemoveListener',
        pri: '0',
        test: function () {
            var toaster = createToaster();

            // Add a legitimate listener
            var eventCount = 0;
            function callback() { eventCount++; }
            toaster.addEventListener("toastcompleteevent", callback);

            verify.noException(function () {
                toaster.addEventListener("toastcompleteevent", callback);
            }, 'AddEventListener with already registered function');

            verify.noException(function () {
                toaster.removeEventListener("toastcompleteevent", function () { });
            }, 'RemoveEventListener with non existant function');

            logger.comment('Calling toaster.makeToast to ensure the legitimate listener is still there');
            toaster.makeToast(msg);
            verify(eventCount, 1, 'Number of events handled');

            toaster.removeEventListener("toastcompleteevent", callback);

            // Remove the event twice
            verify.noException(function () {
                toaster.removeEventListener("toastcompleteevent", callback);
            }, 'RemoveEventListener with already removed handler');

            logger.comment('Calling toaster.makeToast to ensure that the ebent listener was removed');
            toaster.makeToast(msg);
            verify(eventCount, 1, 'Number of events handled');
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Basic',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var toastCompleteCount = 0;

            function toastCompleteCallback(ev) {
                verify(ev.message, msg, 'ev.message');
                toastCompleteCount++;
            }

            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);
            var requiredToastCount = 3;
            for (var i = 0; i < requiredToastCount; i++) {
                var toast = toaster.makeToast(msg);
            }
            verify(toastCompleteCount, requiredToastCount, 'toastCompleteCount');

        }
    });

    runner.addTest({
        id: 3,
        desc: 'MultipleListeners',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var listeners = 3;
            var eventCount = 0;

            for (var i = 0; i < listeners; i++) {
                toaster.addEventListener("toastcompleteevent", function () { eventCount++; });
            }

            logger.comment('Calling toaster.MakeToast, expecting ' + listeners + ' callbacks');
            toaster.makeToast(msg);
            verify(eventCount, listeners, 'Number of events handled');

        }
    });


    runner.addTest({
        id: 4,
        desc: 'MultipleEvents',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var completeEventCount = 0;
            var startEventCount = 0;

            function completeEventCallback(ev) {
                logger.comment('In Complete toasting event callback: toast.message is ' + ev.message);
                completeEventCount++;
            }

            function startEventCallback1(sender) {
                logger.comment('In Start toasting event callback1');
                startEventCount++;
            }

            function startEventCallback2(sender) {
                logger.comment('In Start toasting event callback2');
                startEventCount++;
            }

            logger.comment("Registerting toaststartevent callback");
            toaster.addEventListener("toaststartevent", startEventCallback1);

            logger.comment("Registerting toastcompleteevent callback1");
            toaster.addEventListener("toastcompleteevent", completeEventCallback);

            logger.comment('Calling toaster.makeToast');
            toaster.makeToast(msg);
            verify(startEventCount, 1, 'Number of times toaststartevent called');
            verify(completeEventCount, 1, 'Number of times toastcompleteevents called');

            logger.comment("Registerting toaststartevent callback");
            toaster.addEventListener("toaststartevent", startEventCallback2);

            logger.comment('Calling toaster.makeToast 4 times');
            completeEventCount = 0;
            startEventCount = 0;
            toaster.makeToast(msg);
            toaster.makeToast(msg);
            toaster.makeToast(msg);
            toaster.makeToast(msg);
            verify(startEventCount, 8, 'Number of times toaststartevent called');
            verify(completeEventCount, 4, 'Number of times toastcompleteevents called');

            logger.comment('Removing toaststartevent callback1');
            toaster.removeEventListener("toaststartevent", startEventCallback1);

            logger.comment('Calling toaster.makeToast');
            completeEventCount = 0;
            startEventCount = 0;
            toaster.makeToast(msg);
            verify(startEventCount, 1, 'Number of times toaststartevent called');
            verify(completeEventCount, 1, 'Number of times toastcompleteevents called');

            logger.comment('Removing toaststartevent callback2');
            toaster.removeEventListener("toaststartevent", startEventCallback2);

            logger.comment('Calling toaster.makeToast');
            completeEventCount = 0;
            startEventCount = 0;
            toaster.makeToast(msg);
            verify(startEventCount, 0, 'Number of times toaststartevent called');
            verify(completeEventCount, 1, 'Number of times toastcompleteevents called');

            logger.comment('Removing toastcompleteevent callback');
            toaster.removeEventListener("toastcompleteevent", completeEventCallback);

            logger.comment('Calling toaster.makeToast');
            completeEventCount = 0;
            startEventCount = 0;
            toaster.makeToast(msg);
            verify(startEventCount, 0, 'Number of times toaststartevent called');
            verify(completeEventCount, 0, 'Number of times toastcompleteevents called');
        }
    });


    runner.addTest({
        id: 5,
        desc: 'EventsOnInterfaceOnlyWinRTObjects',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            function onSwitchOn(ev) {
                logger.comment('*** OnSwitchOn');
                verify(ev.target.applianceName, "Toaster", 'ev.target.applianceName');
                verify(ev, msg, "ev : eventDetails");
            }

            function onSwitchOff(ev) {
                logger.comment('*** OnSwitchOff');
                verify(ev.target.applianceName, "Toaster", 'ev.target.applianceName');
                verify(ev, msg, "ev : eventDetails");
                verify(ev.detail[1], 5, "ev.detail[1] : unitCount");
            }

            logger.comment("var electricityReporter = toaster.electricityReporter");
            var electricityReporter = toaster.electricityReporter;
            verify(electricityReporter.applianceName, 'Toaster', "electricityReporter.applianceName");

            logger.comment("Registering onSwitchOn event handler");
            electricityReporter.addEventListener("applianceswitchonevent", onSwitchOn);

            logger.comment("Registering onSwitchOff event handler");
            electricityReporter.addEventListener("applianceswitchoffevent", onSwitchOff);

            logger.comment('Calling toaster.makeToast');
            toaster.makeToast(msg);
        }
    });

    runner.addTest({
        id: 6,
        desc: 'Event Name collision on required interfaces',
        pri: '0',
        test: function () {
            logger.comment("var myRC1WithEvent = new Animals.RC1WithEvent();");
            var myRC1WithEvent = new Animals.RC1WithEvent();
            verifyMembers("myRC1WithEvent", myRC1WithEvent, myRC1WithEventMembers);

            function callback1(ev) {
                logger.comment("*** Event Start : Interface1::Event1");
                verify(ev.target, myRC1WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface1::Event1");
            }

            var myString = "This is IInterface1WithEvent::Event1";
            myRC1WithEvent.addEventListener("event1", callback1);
            logger.comment("myRC1WithEvent.invokeEvent_I1E1(myString);");
            myRC1WithEvent.invokeEvent_I1E1(myString);

            function callback2(ev) {
                logger.comment("*** Event Start : Interface2::Event1");
                verify(ev.target, myRC1WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface2::Event1");
            }

            myString = "This is IInterface2WithEvent::Event1";
            myRC1WithEvent.addEventListener("event21", callback2);
            logger.comment("myRC1WithEvent.invokeEvent_I2E1(myString);");
            myRC1WithEvent.invokeEvent_I2E1(myString);

            function callback(ev) {
                logger.comment("*** Event Start : Generic callback");
                verify(ev.target, myRC1WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Generic callback");
            }

            myString = "This is IInterface1WithEvent::Event1";
            myRC1WithEvent.addEventListener("event1", callback);
            logger.comment("myRC1WithEvent.invokeEvent_I1E1(myString);");
            myRC1WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            myRC1WithEvent.addEventListener("event2", callback);
            logger.comment("myRC1WithEvent.invokeEvent_I1E2(myString);");
            myRC1WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event1";
            myRC1WithEvent.removeEventListener("event21", callback2);
            logger.comment("myRC1WithEvent.invokeEvent_I2E1(myString);");
            myRC1WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface1WithEvent::Event1";
            myRC1WithEvent.removeEventListener("event1", callback);
            logger.comment("myRC1WithEvent.invokeEvent_I1E1(myString);");
            myRC1WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            myRC1WithEvent.removeEventListener("event2", callback);
            logger.comment("myRC1WithEvent.invokeEvent_I1E2(myString);");
            myRC1WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface1WithEvent::Event1";
            myRC1WithEvent.removeEventListener("event1", callback1);
            logger.comment("myRC1WithEvent.invokeEvent_I1E1(myString);");
            myRC1WithEvent.invokeEvent_I1E1(myString);
        }
    });

    runner.addTest({
        id: 7,
        desc: 'Event Name collision on a static and required interface',
        pri: '0',
        test: function () {
            logger.comment("var staticRC2WithEvent = Animals.RC2WithEvent;");
            var staticRC2WithEvent = Animals.RC2WithEvent;
            verifyMembers("staticRC2WithEvent", Animals.RC2WithEvent, staticRC2WithEventMembers, "function");

            logger.comment("var myRC2WithEvent = new Animals.RC2WithEvent();");
            var myRC2WithEvent = new Animals.RC2WithEvent();
            verifyMembers("myRC2WithEvent", myRC2WithEvent, myRC2WithEventMembers);

            function callback1(ev) {
                logger.comment("*** Event Start : Interface2::Event1");
                verify(ev.target, myRC2WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface2::Event1");
            }

            var myString = "This is IInterface2WithEvent::Event1";
            myRC2WithEvent.addEventListener("event21", callback1);
            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);

            function callback2(ev) {
                logger.comment("*** Event Start : Interface1::Event1");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface1::Event1");
            }

            myString = "This is IInterface1WithEvent::Event1";
            staticRC2WithEvent.addEventListener("event1", callback2);
            logger.comment("staticRC2WithEvent.invokeEvent_I1E1(myString);");
            staticRC2WithEvent.invokeEvent_I1E1(myString);

            function callback(ev) {
                logger.comment("*** Event Start : Generic callback");
                verify(ev.target, myRC2WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Generic callback");
            }

            myString = "This is IInterface2WithEvent::Event1";
            myRC2WithEvent.addEventListener("event21", callback);
            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface2WithEvent::Event3";
            myRC2WithEvent.addEventListener("event3", callback);
            logger.comment("myRC2WithEvent.invokeEvent_I2E3(myString);");
            myRC2WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface1WithEvent::Event1";
            staticRC2WithEvent.removeEventListener("event1", callback2);
            logger.comment("staticRC2WithEvent.invokeEvent_I1E1(myString);");
            staticRC2WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            myRC2WithEvent.removeEventListener("event21", callback);
            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface2WithEvent::Event3";
            myRC2WithEvent.removeEventListener("event3", callback);
            logger.comment("myRC2WithEvent.invokeEvent_I2E3(myString);");
            myRC2WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface2WithEvent::Event1";
            myRC2WithEvent.removeEventListener("event21", callback1);
            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);
        }
    });

    runner.addTest({
        id: 8,
        desc: 'Interface with event and addEventListener method',
        pri: '0',
        test: function () {
            logger.comment("var myRC3WithEvent = new Animals.RC3WithEvent();");
            var myRC3WithEvent = new Animals.RC3WithEvent();
            verifyMembers("myRC3WithEvent", myRC3WithEvent, myRC3WithEventMembers);

            function callback1(ev) {
                logger.comment("*** Event Start : Interface3::Event1");
                verify(ev.target, myRC3WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface3::Event1");
            }

            var myString = "This is IInterface3WithEvent::Event1";
            myRC3WithEvent.addEventListener("event31", callback1);
            logger.comment("myRC3WithEvent.invokeEvent_I3E1(myString);");
            myRC3WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            verify(myRC3WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'myRC3WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');

            myString = "This is IInterface3WithEvent::Event1";
            myRC3WithEvent.removeEventListener("event31", callback1);
            logger.comment("myRC3WithEvent.invokeEvent_I3E1(myString);");
            myRC3WithEvent.invokeEvent_I3E1(myString);
        }
    });

    runner.addTest({
        id: 9,
        desc: 'Rc with interfaces with event and addEventListener method',
        pri: '0',
        test: function () {
            logger.comment("var myRC4WithEvent = new Animals.RC4WithEvent();");
            var myRC4WithEvent = new Animals.RC4WithEvent();
            verifyMembers("myRC4WithEvent", myRC4WithEvent, myRC4WithEventMembers);

            function callback(ev) {
                logger.comment("*** Event Start");
                verify(ev.target, myRC4WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete");
            }

            myRC4WithEvent.addEventListener("event1", callback);
            myRC4WithEvent.addEventListener("event21", callback);
            myRC4WithEvent.addEventListener("event31", callback);
            myRC4WithEvent.addEventListener("event2", callback);
            myRC4WithEvent.addEventListener("event3", callback);
            myRC4WithEvent.addEventListener("event5", callback);

            var myString = "This is IInterface1WithEvent::Event1";
            logger.comment("myRC4WithEvent.invokeEvent_I1E1(myString);");
            myRC4WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            logger.comment("myRC4WithEvent.invokeEvent_I2E1(myString);");
            myRC4WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface3WithEvent::Event1";
            logger.comment("myRC4WithEvent.invokeEvent_I3E1(myString);");
            myRC4WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            logger.comment("myRC4WithEvent.invokeEvent_I1E2(myString);");
            myRC4WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event3";
            logger.comment("myRC4WithEvent.invokeEvent_I2E3(myString);");
            myRC4WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface3WithEvent::Event5";
            logger.comment("myRC4WithEvent.invokeEvent_I3E5(myString);");
            myRC4WithEvent.invokeEvent_I3E5(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            verify(myRC4WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'myRC4WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');

            myRC4WithEvent.removeEventListener("event1", callback);
            myRC4WithEvent.removeEventListener("event21", callback);
            myRC4WithEvent.removeEventListener("event31", callback);
            myRC4WithEvent.removeEventListener("event2", callback);
            myRC4WithEvent.removeEventListener("event3", callback);
            myRC4WithEvent.removeEventListener("event5", callback);

            myString = "This is IInterface1WithEvent::Event1";
            logger.comment("myRC4WithEvent.invokeEvent_I1E1(myString);");
            myRC4WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            logger.comment("myRC4WithEvent.invokeEvent_I2E1(myString);");
            myRC4WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface3WithEvent::Event1";
            logger.comment("myRC4WithEvent.invokeEvent_I3E1(myString);");
            myRC4WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            logger.comment("myRC4WithEvent.invokeEvent_I1E2(myString);");
            myRC4WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event3";
            logger.comment("myRC4WithEvent.invokeEvent_I2E3(myString);");
            myRC4WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface3WithEvent::Event5";
            logger.comment("myRC4WithEvent.invokeEvent_I3E5(myString);");
            myRC4WithEvent.invokeEvent_I3E5(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            verify(myRC4WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'myRC4WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');
        }
    });

    runner.addTest({
        id: 10,
        desc: 'Event Name collision on a static interface with method addEventListner',
        pri: '0',
        test: function () {
            logger.comment("var staticRC5WithEvent = Animals.RC5WithEvent;");
            var staticRC5WithEvent = Animals.RC5WithEvent;
            if (typeof Animals._CLROnly !== 'undefined') //In C# ABI implementation, there is no way to provide a static interface like native and the static part interface name is auto generated instead
            {
                staticRC5WithEventMembers = [
                    ['Animals.IRC5WithEventStatic.addEventListener', 'function', 1],
                    ['addEventListener', 'function', 2],
                    ['invokeEvent_I3E1', 'function', 1],
                    ['invokeEvent_I3E5', 'function', 1],
                    ['onevent31', 'object'],
                    ['onevent5', 'object'],
                    ['removeEventListener', 'function', 2],
                ];
            }
            verifyMembers("staticRC5WithEvent", Animals.RC5WithEvent, staticRC5WithEventMembers, "function");

            logger.comment("var myRC5WithEvent = new Animals.RC5WithEvent();");
            var myRC5WithEvent = new Animals.RC5WithEvent();
            verifyMembers("myRC5WithEvent", myRC5WithEvent, myRC5WithEventMembers);

            function callback(ev) {
                logger.comment("*** Event Start");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete");
            }

            myRC5WithEvent.addEventListener("event1", callback);
            myRC5WithEvent.addEventListener("event21", callback);
            myRC5WithEvent.addEventListener("event2", callback);
            myRC5WithEvent.addEventListener("event3", callback);

            staticRC5WithEvent.addEventListener("event31", callback);
            staticRC5WithEvent.addEventListener("event5", callback);

            var myString = "This is IInterface1WithEvent::Event1";
            logger.comment("myRC5WithEvent.invokeEvent_I1E1(myString);");
            myRC5WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            logger.comment("myRC5WithEvent.invokeEvent_I2E1(myString);");
            myRC5WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            logger.comment("myRC5WithEvent.invokeEvent_I1E2(myString);");
            myRC5WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event3";
            logger.comment("myRC5WithEvent.invokeEvent_I2E3(myString);");
            myRC5WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface3WithEvent::Event1";
            logger.comment("staticRC5WithEvent.invokeEvent_I3E1(myString);");
            staticRC5WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface3WithEvent::Event5";
            logger.comment("staticRC5WithEvent.invokeEvent_I3E5(myString);");
            staticRC5WithEvent.invokeEvent_I3E5(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            if (typeof Animals._CLROnly === 'undefined') ////In C# ABI implementation, there is no way to provide a static interface like native and the static part interface name is auto generated instead
                verify(staticRC5WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'staticRC5WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');
            else
                verify(staticRC5WithEvent["Animals.IRC5WithEventStatic.addEventListener"](myString), myString, 'staticRC5WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');

            myRC5WithEvent.removeEventListener("event1", callback);
            myRC5WithEvent.removeEventListener("event21", callback);
            myRC5WithEvent.removeEventListener("event2", callback);
            myRC5WithEvent.removeEventListener("event3", callback);

            staticRC5WithEvent.removeEventListener("event31", callback);
            staticRC5WithEvent.removeEventListener("event5", callback);

            myString = "This is IInterface1WithEvent::Event1";
            logger.comment("myRC5WithEvent.invokeEvent_I1E1(myString);");
            myRC5WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            logger.comment("myRC5WithEvent.invokeEvent_I2E1(myString);");
            myRC5WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            logger.comment("myRC5WithEvent.invokeEvent_I1E2(myString);");
            myRC5WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event3";
            logger.comment("myRC5WithEvent.invokeEvent_I2E3(myString);");
            myRC5WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface3WithEvent::Event1";
            logger.comment("staticRC5WithEvent.invokeEvent_I3E1(myString);");
            staticRC5WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface3WithEvent::Event5";
            logger.comment("staticRC5WithEvent.invokeEvent_I3E5(myString);");
            staticRC5WithEvent.invokeEvent_I3E5(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            if (typeof Animals._CLROnly === 'undefined') //In C# ABI implementation, there is no way to provide a static interface like native and the static part interface name is auto generated instead
                verify(staticRC5WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'staticRC5WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');
            else
                verify(staticRC5WithEvent["Animals.IRC5WithEventStatic.addEventListener"](myString), myString, 'staticRC5WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');

            logger.comment("Add event handler that we wont remove and see if script context can close property");
            function callback2(ev) {
                logger.comment("*** Event1 Start");
                verify(ev, myString, "ev");
                logger.comment("*** Event1 Complete");
            }
            staticRC5WithEvent.onevent31 = callback2;

            logger.comment("Invoke method and check the event handler is set");
            myString = "This is IInterface3WithEvent::Event1";
            staticRC5WithEvent.invokeEvent_I3E1(myString);
        }
    });

    runner.addTest({
        id: 11,
        desc: 'Event Name collision over multiple level requires',
        pri: '0',
        test: function () {
            logger.comment("var myRC6WithEvent = new Animals.RC6WithEvent();");
            var myRC6WithEvent = new Animals.RC6WithEvent();
            verifyMembers("myRC6WithEvent", myRC6WithEvent, myRC6WithEventMembers);

            function callback(ev) {
                logger.comment("*** Event Start");
                verify(ev.target, myRC6WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete");
            }

            myRC6WithEvent.addEventListener("Animals.IInterface1WithEvent.event1", callback);
            myRC6WithEvent.addEventListener("event21", callback);
            myRC6WithEvent.addEventListener("event31", callback);
            myRC6WithEvent.addEventListener("Animals.IInterface4WithEvent.event1", callback);
            myRC6WithEvent.addEventListener("event2", callback);
            myRC6WithEvent.addEventListener("event3", callback);
            myRC6WithEvent.addEventListener("event5", callback);

            var myString = "This is IInterface1WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I1E1(myString);");
            myRC6WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I2E1(myString);");
            myRC6WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface3WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I3E1(myString);");
            myRC6WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            logger.comment("myRC6WithEvent.invokeEvent_I1E2(myString);");
            myRC6WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event3";
            logger.comment("myRC6WithEvent.invokeEvent_I2E3(myString);");
            myRC6WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface3WithEvent::Event5";
            logger.comment("myRC6WithEvent.invokeEvent_I3E5(myString);");
            myRC6WithEvent.invokeEvent_I3E5(myString);

            myString = "This is IInterface4WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I4E1(myString);");
            myRC6WithEvent.invokeEvent_I4E1(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            verify(myRC6WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'myRC6WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');

            myRC6WithEvent.removeEventListener("Animals.IInterface1WithEvent.event1", callback);
            myRC6WithEvent.removeEventListener("event21", callback);
            myRC6WithEvent.removeEventListener("event31", callback);
            myRC6WithEvent.removeEventListener("Animals.IInterface4WithEvent.event1", callback);
            myRC6WithEvent.removeEventListener("event2", callback);
            myRC6WithEvent.removeEventListener("event3", callback);
            myRC6WithEvent.removeEventListener("event5", callback);

            myString = "This is IInterface1WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I1E1(myString);");
            myRC6WithEvent.invokeEvent_I1E1(myString);

            myString = "This is IInterface2WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I2E1(myString);");
            myRC6WithEvent.invokeEvent_I2E1(myString);

            myString = "This is IInterface3WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I3E1(myString);");
            myRC6WithEvent.invokeEvent_I3E1(myString);

            myString = "This is IInterface1WithEvent::Event2";
            logger.comment("myRC6WithEvent.invokeEvent_I1E2(myString);");
            myRC6WithEvent.invokeEvent_I1E2(myString);

            myString = "This is IInterface2WithEvent::Event3";
            logger.comment("myRC6WithEvent.invokeEvent_I2E3(myString);");
            myRC6WithEvent.invokeEvent_I2E3(myString);

            myString = "This is IInterface3WithEvent::Event5";
            logger.comment("myRC6WithEvent.invokeEvent_I3E5(myString);");
            myRC6WithEvent.invokeEvent_I3E5(myString);

            myString = "This is IInterface4WithEvent::Event1";
            logger.comment("myRC6WithEvent.invokeEvent_I4E1(myString);");
            myRC6WithEvent.invokeEvent_I4E1(myString);

            myString = "This is IInterface3WithEvent::addListener method";
            verify(myRC6WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString), myString, 'myRC6WithEvent["Animals.IInterface3WithEvent.addEventListener"](myString)');
        }
    });

    runner.addTest({
        id: 12,
        desc: 'Vector<T>, IVectorChangedArgs<T>',
        pri: '0',
        test: function () {
            var myAnimal = new Animals.Animal(1);
            var myVector = myAnimal.getObservableVector();

            function vectorChanged(ev) {
                logger.comment("*** ObservableVector Change invoke");

                logger.comment("    ev.target: " + ev.target);
                logger.comment("    ev.index: " + ev.index);
                logger.comment("    ev.collectionChange: " + ev.collectionChange);

                logger.comment("*** ObservableVector Change End");
            }

            // Assign the event handler and get no error.
            logger.comment("myVector.addEventListener('vectorchanged', vectorChanged);");
            myVector.addEventListener('vectorchanged', vectorChanged);

            var myAnotherVector = myAnimal.getObservableStringVector();
            myAnotherVector.addEventListener('vectorchanged', vectorChanged);

            myVector[3] = 10;
            myAnotherVector[2] = "NewString"

            logger.comment("myAnotherVector : " + myAnotherVector);
            logger.comment("myVector : " + myVector);
        }
    });

    runner.addTest({
        id: 13,
        desc: 'EventHandlerInvokeFormat: <This, Sender, Runtimeclass>',
        pri: '0',
        test: function () {
            logger.comment("Create the toaster");
            var toaster = createToaster();
            var toastCompleteCount = 0;

            function toastCompleteCallback(ev) {
                logger.comment("*** toastCompleteCallback : Invoke");
                verify(ev.message, msg, 'ev.message');
                toastCompleteCount++;

                // Verify Ev
                verifyMembers("ev", ev, evToastMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, toasterMembers);
                assert(ev.target === toaster, "ev.target === toaster");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], toastMembers);
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                verify(ev.detail[0].message, ev.message, 'ev.message');
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "toastcompleteevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                // verify extensibility
                ev.myExpando = 42;
                verify(ev.myExpando, undefined, "ev.myExpando");

                ev.detail.myExpando = 42;
                verify(ev.detail.myExpando, undefined, "ev.detail.myExpando");

                logger.comment("*** toastCompleteCallback : Exit");
            }

            logger.comment("Add event listener for toastComplete event");
            toaster.addEventListener("toastcompleteevent", toastCompleteCallback);

            var toast = toaster.makeToast(msg);
            verify(toastCompleteCount, 1, 'toastCompleteCount');
        }
    });

    runner.addTest({
        id: 14,
        desc: 'EventHandlerInvokeFormat: <This, Sender, String, Arg2>',
        pri: '0',
        test: function () {
            logger.comment("Create the toaster");
            var toaster = createToaster();
            var eventInvokeCount = 0;
            function onSwitchOff(ev) {
                logger.comment('*** OnSwitchOff : Invoke');

                eventInvokeCount++;
                verify(ev.target.applianceName, "Toaster", 'ev.target.applianceName');
                verify(ev, msg, "ev : eventDetails");
                verify(ev.detail[1], 5, "ev.detail[1] : unitCount");

                // Verify Ev
                verifyMembers("ev", ev, evStringMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, electricityReporterMembers);
                assert(ev.target === electricityReporter, "ev.target === electricityReporter");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 2, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], evDetailStringMembers, "string");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev == ev.detail[0], "ev == ev.detail[0]");
                verify(ev.detail[0].message, ev.message, 'ev.message');
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "applianceswitchoffevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment('*** OnSwitchOff : Exit');
            }

            logger.comment("Get electricity reporter");
            var electricityReporter = toaster.electricityReporter;
            verify(electricityReporter.applianceName, 'Toaster', "electricityReporter.applianceName");

            logger.comment("Registering onSwitchOff event handler");
            electricityReporter.addEventListener("applianceswitchoffevent", onSwitchOff);

            logger.comment('Calling toaster.makeToast');
            toaster.makeToast(msg);

            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 15,
        desc: 'EventHandlerInvokeFormat: <This, Sender>',
        pri: '0',
        test: function () {
            logger.comment("Create the toaster");
            var toaster = createToaster();
            var eventInvokeCount = 0;

            function toastStartCallback(ev) {
                logger.comment("*** toastStartCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evEmptyObjectMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, toasterMembers);
                assert(ev.target === toaster, "ev.target === toaster");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 0, "ev.detail.length");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "toaststartevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** toastStartCallback : Exit");
            }

            logger.comment("Add event listener for toastStart event");
            toaster.addEventListener("toaststartevent", toastStartCallback);

            var toast = toaster.makeToast(msg);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 16,
        desc: 'EventHandlerInvokeFormat: <This>',
        pri: '0',
        test: function () {
            logger.comment("Create the toaster");
            var toaster = createToaster();
            var eventInvokeCount = 0;

            function toasterPreheatStartCallback(ev) {
                logger.comment("*** toasterPreheatStartCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evEmptyObjectMembers);

                // Verify target
                verify(ev.target, null, "ev.target");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 0, "ev.detail.length");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "preheatstart", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                // verify extensibility
                ev.myExpando = 42;
                verify(ev.myExpando, undefined, "ev.myExpando");

                ev.detail.myExpando = 42;
                verify(ev.detail.myExpando, undefined, "ev.detail.myExpando");

                logger.comment("*** toasterPreheatStartCallback : Exit");
            }

            logger.comment("Add event listener for toater preheat start event");
            toaster.addEventListener("preheatstart", toasterPreheatStartCallback);

            var toast = toaster.preheatInBackground(null);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 17,
        desc: 'EventHandlerInvokeFormat: <This, sender, Vecor<T>>',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = createToaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            var eventInvokeCount = 0;

            function multipleToastCompleteCollectionCallback(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evToastList5Members);

                // Verify target
                verifyMembers("ev.target", ev.target, chefMembers);
                assert(ev.target === myChef, "ev.target === myChef");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], toastList5Members);
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                var detail0 = ev.detail[0];
                for (i = 0; i < 5; i++) {
                    verify(detail0[i].message, ev[i].message, "ev.detail[0][" + i + "].message");
                }
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "multipletoastcompletecollection", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** multipleToastCompleteCollectionCallback : Exit");
            }

            logger.comment("Add event listener for multiple toast make complete event");
            myChef.addEventListener("multipletoastcompletecollection", multipleToastCompleteCollectionCallback);

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 18,
        desc: 'EventHandlerInvokeFormat: <This, sender, Array<T>>',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = createToaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            var eventInvokeCount = 0;

            function multipleToastCompleteArrayCallback(ev) {
                logger.comment("*** multipleToastCompleteArrayCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evToastArray5Members);

                // Verify target
                verifyMembers("ev.target", ev.target, chefMembers);
                assert(ev.target === myChef, "ev.target === myChef");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], toastArray5Members);
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                var detail0 = ev.detail[0];
                for (i = 0; i < 5; i++) {
                    verify(detail0[i].message, ev[i].message, "ev.detail[0][" + i + "].message");
                }
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "multipletoastcompletearray", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** multipleToastCompleteArrayCallback : Exit");
            }

            logger.comment("Add event listener for multiple toast make complete event");
            myChef.addEventListener("multipletoastcompletearray", multipleToastCompleteArrayCallback);

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 19,
        desc: 'EventHandlerInvokeFormat: <This, sender, outParams, number>',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = createToaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            var eventInvokeCount = 0;

            function makeToastRoundOffCallback(ev) {
                logger.comment("*** makeToastRoundOffCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evEmptyObjectMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, chefMembers);
                assert(ev.target === myChef, "ev.target === myChef");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], [], "number");
                verify(ev.detail[0].toString(), ev.toString(), "ev.detail[0].toString()");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev == ev.detail[0], "ev == ev.detail[0]");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "maketoastroundoff", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** makeToastRoundOffCallback : Exit");
                return ev.detail[0] == 5;
            }

            logger.comment("Add event listener for roundoff");
            myChef.addEventListener("maketoastroundoff", makeToastRoundOffCallback);

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 20,
        desc: 'EventHandlerInvokeFormat: <This, sender, struct>',
        pri: '0',
        test: function () {
            logger.comment("Create Object");
            var myRC8WithEvent = new Animals.RC8WithEvent();

            var eventInvokeCount = 0;
            function structEventCallback(ev) {
                logger.comment("*** structEventCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evStructForStructEventMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, myRC8WithEventMembers);
                assert(ev.target === myRC8WithEvent, "ev.target === myRC8WithEvent");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], structForStructEventMembers);
                verify(ev.detail[0], myStruct, "ev.detail[0]");
                verify(ev.detail[0].structId, ev.structId, "ev.detail[0].structId");
                verify(ev.detail[0].structData, ev.structData, "ev.detail[0].structData");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "structevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** structEventCallback : Exit");
            }

            logger.comment("Add event listener");
            myRC8WithEvent.addEventListener("structevent", structEventCallback);

            logger.comment("Invoke method and event");
            var myStruct = {
                structId: 10,
                structData: 100
            };
            myRC8WithEvent.invokeStructEvent(myStruct);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 21,
        desc: 'EventHandlerInvokeFormat: <This, sender, delegate>',
        pri: '0',
        test: function () {
            logger.comment("Create Object");
            var myRC8WithEvent = new Animals.RC8WithEvent();

            var eventInvokeCount = 0;
            function delegateEventCallback(ev) {
                logger.comment("*** delegateEventCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evEmptyObjectMembers, "function");
                var currentValue = myValue;
                ev(myValue);
                verify(myValue, currentValue + 1, "myValue");

                // Verify target
                verifyMembers("ev.target", ev.target, myRC8WithEventMembers);
                assert(ev.target === myRC8WithEvent, "ev.target === myRC8WithEvent");
                verifyEvObjectPropertyDescriptor("ev", ev, "target", "function");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], [], "function");
                verify(ev.detail[0], myDelegateCallback, "ev.detail[0]");
                ev.detail[0](myValue);
                verify(myValue, currentValue + 2, "myValue");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail", "function");

                // verify type
                verify(ev.type, "delegateevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type", "function");

                logger.comment("*** delegateEventCallback : Exit");
            }

            logger.comment("Add event listener");
            myRC8WithEvent.addEventListener("delegateevent", delegateEventCallback);
            var myValue = 999;

            logger.comment("Invoke method and event");
            function myDelegateCallback(inValue) {
                logger.comment("*** myDelegateCallback : Invoke");
                verify(inValue, myValue, "inValue");
                myValue++;
                logger.comment("*** myDelegateCallback : Exit");
            }
            myRC8WithEvent.invokeDelegateEvent(myDelegateCallback);
            verifyMembers("myDelegateCallback", myDelegateCallback, [], "function");
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 22,
        desc: 'EventHandlerInvokeFormat: <This, sender, delegate = null>',
        pri: '0',
        test: function () {
            logger.comment("Create Object");
            var myRC8WithEvent = new Animals.RC8WithEvent();

            var eventInvokeCount = 0;
            function nullDelegateInvokeCallback(ev) {
                logger.comment("*** nullDelegateInvokeCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evEmptyObjectMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, myRC8WithEventMembers);
                assert(ev.target === myRC8WithEvent, "ev.target === myRC8WithEvent");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verify(ev.detail[0], null, "ev.detail[0]");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "delegateevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** nullDelegateInvokeCallback : Exit");
            }

            logger.comment("Add event listener");
            myRC8WithEvent.addEventListener("delegateevent", nullDelegateInvokeCallback);

            logger.comment("Invoke method and event");
            myRC8WithEvent.invokeDelegateEvent(null);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 23,
        desc: 'EventHandlerInvokeFormat: <This, sender, NameConflictWithTarget - Inteface>',
        pri: '0',
        test: function () {
            logger.comment("Create Object");
            var myRC8WithEvent = new Animals.RC8WithEvent();

            var eventInvokeCount = 0;
            function interfaceWithTargetEventCallback(ev) {
                logger.comment("*** interfaceWithTargetEventCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evRC8WithEventMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, myRC8WithEventMembers);
                assert(ev.target === myRC8WithEvent, "ev.target === myRC8WithEvent");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], myRC8WithEventMembers);
                assert(ev.detail[0] === myRC8WithEvent, "ev.detail[0] === myRC8WithEvent");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "interfacewithtargetevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** interfaceWithTargetEventCallback : Exit");
            }

            logger.comment("Add event listener");
            myRC8WithEvent.addEventListener("interfacewithtargetevent", interfaceWithTargetEventCallback);

            logger.comment("Invoke method and event");
            myRC8WithEvent.invokeInterfaceWithTargetEvent();
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 24,
        desc: 'EventHandlerInvokeFormat: <This, sender, NameConflictWithTarget - struct>',
        pri: '0',
        test: function () {
            logger.comment("Create Object");
            var myRC8WithEvent = new Animals.RC8WithEvent();

            var eventInvokeCount = 0;
            function structWithTargetEventCallback(ev) {
                logger.comment("*** structWithTargetEventCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evStructForStructWithTargetEventMembers);

                // Verify target
                verifyMembers("ev.target", ev.target, myRC8WithEventMembers);
                assert(ev.target === myRC8WithEvent, "ev.target === myRC8WithEvent");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], structForStructWithTargetEventMembers);
                verify(ev.detail[0], myStruct, "ev.detail[0]");
                verify(ev.detail[0].structId, ev.structId, "ev.detail[0].structId");
                verify(ev.detail[0].structData, ev.structData, "ev.detail[0].structData");
                assert(ev.detail[0].target !== ev.target, "ev.detail[0].target !== ev.target");
                assert(ev.detail[0].target != ev.target, "ev.detail[0].target != ev.target");
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "structwithtargetevent", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** structWithTargetEventCallback : Exit");
            }

            logger.comment("Add event listener");
            myRC8WithEvent.addEventListener("structwithtargetevent", structWithTargetEventCallback);

            logger.comment("Invoke method and event");
            var myStruct = {
                structId: 10,
                structData: 100,
                target: true
            };
            myRC8WithEvent.invokeStructWithTargetEvent(myStruct);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 25,
        desc: 'BasicEventHandler',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = createToaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            verifyMembers("myChef", myChef, chefMembers);
            verify(myChef.onmultipletoastcompletecollection, null, "myChef.onmultipletoastcompletecollection");

            var eventInvokeCount = 0;

            function multipleToastCompleteCollectionCallback(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                verifyMembers("ev", ev, evToastList5Members);

                // Verify target
                verifyMembers("ev.target", ev.target, chefWithEventHandlerMembers);
                assert(ev.target === myChef, "ev.target === myChef");
                verifyEvObjectPropertyDescriptor("ev", ev, "target");

                // Verify Detail
                assert(Array.isArray(ev.detail), "Array.isArray(ev.detail)");
                verify(ev.detail.length, 1, "ev.detail.length");
                verifyMembers("ev.detail[0]", ev.detail[0], toastList5Members);
                assert(ev !== ev.detail[0], "ev !== ev.detail[0]");
                assert(ev != ev.detail[0], "ev != ev.detail[0]");
                var detail0 = ev.detail[0];
                for (i = 0; i < 5; i++) {
                    verify(detail0[i].message, ev[i].message, "ev.detail[0][" + i + "].message");
                }
                verifyEvObjectPropertyDescriptor("ev", ev, "detail");

                // verify type
                verify(ev.type, "multipletoastcompletecollection", "ev.type");
                verifyEvObjectPropertyDescriptor("ev", ev, "type");

                logger.comment("*** multipleToastCompleteCollectionCallback : Exit");
            }

            logger.comment("Add event handler for multiple toast complete");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');

            logger.comment("Set event handler to null");
            myChef.onmultipletoastcompletecollection = null;
            verify(myChef.onmultipletoastcompletecollection, null, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });

    runner.addTest({
        id: 26,
        desc: 'CombinationOfEventHandlerSet',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = createToaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            verifyMembers("myChef", myChef, chefMembers);
            verify(myChef.onmultipletoastcompletecollection, null, "myChef.onmultipletoastcompletecollection");

            var eventInvokeCount = 0;
            var eventInvokeCount1 = 0;
            var eventInvokeCount2 = 0;

            function multipleToastCompleteCollectionCallback1(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback1 : Invoke");
                eventInvokeCount++;
                eventInvokeCount1++;

                // Verify that event handler is being invoked correctly
                verify(ev.type, "multipletoastcompletecollection", "ev.type");

                logger.comment("*** multipleToastCompleteCollectionCallback1 : Exit");
            }

            function multipleToastCompleteCollectionCallback2(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback2 : Invoke");
                eventInvokeCount++;
                eventInvokeCount2++;

                // Verify that event handler is being invoked correctly
                verify(ev.type, "multipletoastcompletecollection", "ev.type");

                logger.comment("*** multipleToastCompleteCollectionCallback2 : Exit");
            }

            logger.comment("Add event handler for multiple toast complete - callback1");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback1;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback1, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 0, 'eventInvokeCount2');

            logger.comment("Set event handler to callback2");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback2;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback2, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 2, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 1, 'eventInvokeCount2');

            logger.comment("Set event handler to null");
            myChef.onmultipletoastcompletecollection = null;
            verify(myChef.onmultipletoastcompletecollection, null, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 2, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 1, 'eventInvokeCount2');

            logger.comment("Set event handler to callback 2");
            myChef.onmultipletoastcompletecollection = multipleToastCompleteCollectionCallback2;
            verify(myChef.onmultipletoastcompletecollection, multipleToastCompleteCollectionCallback2, "myChef.onmultipletoastcompletecollection");

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 3, 'eventInvokeCount');
            verify(eventInvokeCount1, 1, 'eventInvokeCount1');
            verify(eventInvokeCount2, 2, 'eventInvokeCount2');
        }
    });

    runner.addTest({
        id: 27,
        desc: 'NameConflictsOnEventHandler: With event name collision of event1 and event method collision onevent2',
        pri: '0',
        test: function () {
            logger.comment("Create the object;");
            var myRC1WithEvent = new Animals.RC1WithEvent();
            verify(myRC1WithEvent["onevent1"], null, 'myRC1WithEvent["onevent1"]');
            verify(myRC1WithEvent.onevent2, null, 'myRC1WithEvent.onevent2');
            verify(myRC1WithEvent["onevent21"], null, 'myRC1WithEvent["onevent21"]');
            verify(myRC1WithEvent.onevent3, null, 'myRC1WithEvent.onevent3');

            var i1e1Count = 0;
            var i1e2Count = 0;
            var i2e1Count = 0;
            var i2e3Count = 0;
            function interface1Callback1(ev) {
                logger.comment("*** interface1Callback1: Invoke");
                i1e1Count++;
                verify(ev.type, "event1", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface1Callback1: Exit");
            }

            function interface1Callback2(ev) {
                logger.comment("*** interface1Callback2: Invoke");
                i1e2Count++;
                verify(ev.type, "event2", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface1Callback2: Exit");
            }

            function interface2Callback1(ev) {
                logger.comment("*** interface2Callback1: Invoke");
                i2e1Count++;
                verify(ev.type, "event21", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface2Callback1: Exit");
            }

            function interface2Callback3(ev) {
                logger.comment("*** interface2Callback3: Invoke");
                i2e3Count++;
                verify(ev.type, "event3", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface2Callback3: Exit");
            }

            logger.comment("Set the callbacks");
            myRC1WithEvent["onevent1"] = interface1Callback1;
            verify(myRC1WithEvent["onevent1"], interface1Callback1, 'myRC1WithEvent["onevent1"]');
            myRC1WithEvent.onevent2 = interface1Callback2;
            verify(myRC1WithEvent.onevent2, interface1Callback2, 'myRC1WithEvent.onevent2');
            myRC1WithEvent["onevent21"] = interface2Callback1;
            verify(myRC1WithEvent["onevent21"], interface2Callback1, 'myRC1WithEvent["onevent21"]');
            myRC1WithEvent.onevent3 = interface2Callback3;
            verify(myRC1WithEvent.onevent3, interface2Callback3, 'myRC1WithEvent.onevent3');

            logger.comment("Invoke events and methods");
            myRC1WithEvent.invokeEvent_I1E1("event1");
            verify(i1e1Count, 1, "i1e1Count");
            myRC1WithEvent.invokeEvent_I1E2("event2");
            verify(i1e2Count, 1, "i1e2Count");
            myRC1WithEvent.invokeEvent_I2E1("event21");
            verify(i2e1Count, 1, "i2e1Count");
            myRC1WithEvent.invokeEvent_I2E3("event3");
            verify(i2e3Count, 1, "i2e3Count");
            var methodString = "This is onevent2 method"
            var result = myRC1WithEvent["Animals.IInterface2WithEvent.onevent2"](methodString);
            verify(result, methodString, 'myRC1WithEvent["Animals.IInterface2WithEvent.onevent2"]("This is onevent2 method")');

            logger.comment("Set the callbacks to null");
            myRC1WithEvent["onevent1"] = null;
            verify(myRC1WithEvent["onevent1"], null, 'myRC1WithEvent["onevent1"]');
            myRC1WithEvent.onevent2 = null;
            verify(myRC1WithEvent.onevent2, null, 'myRC1WithEvent.onevent2');
            myRC1WithEvent["onevent21"] = null;
            verify(myRC1WithEvent["onevent21"], null, 'myRC1WithEvent["onevent21"]');
            myRC1WithEvent.onevent3 = null;
            verify(myRC1WithEvent.onevent3, null, 'myRC1WithEvent.onevent3');


            logger.comment("Invoke events and methods");
            myRC1WithEvent.invokeEvent_I1E1("event1");
            verify(i1e1Count, 1, "i1e1Count");
            myRC1WithEvent.invokeEvent_I1E2("event2");
            verify(i1e2Count, 1, "i1e2Count");
            myRC1WithEvent.invokeEvent_I2E1("event21");
            verify(i2e1Count, 1, "i2e1Count");
            myRC1WithEvent.invokeEvent_I2E3("event3");
            verify(i2e3Count, 1, "i2e3Count");
            var methodString = "This is onevent2 method"
            var result = myRC1WithEvent["Animals.IInterface2WithEvent.onevent2"](methodString);
            verify(result, methodString, 'myRC1WithEvent["Animals.IInterface2WithEvent.onevent2"]("This is onevent2 method")');
        }
    });

    runner.addTest({
        id: 28,
        desc: 'NameConflictsOnEventHandler: event method collision resulting in unresolvable thunk',
        pri: '0',
        test: function () {
            logger.comment("Create the object;");
            var myRC6WithEvent = new Animals.RC6WithEvent();

            logger.comment("verify that onevent1 is unresolvable thunk for interface 4");
            var exceptionFound = false;
            verify.exception(function () {
                myRC6WithEvent["Animals.IInterface4WithEvent.onevent1"]("Hello");
            }, TypeError, "Exception from unresolvable event-method");

            logger.comment("Test to check rest of the event1 handlers can be set");
            verify(myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"], null, 'myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"]');
            verify(myRC6WithEvent.onevent2, null, 'myRC6WithEvent.onevent2');
            verify(myRC6WithEvent["onevent21"], null, 'myRC6WithEvent["onevent21"]');
            verify(myRC6WithEvent.onevent3, null, 'myRC6WithEvent.onevent3');
            verify(myRC6WithEvent["onevent31"], null, 'myRC6WithEvent["onevent31"]');
            verify(myRC6WithEvent.onevent5, null, 'myRC6WithEvent.onevent5');

            var i1e1Count = 0;
            var i1e2Count = 0;
            var i2e1Count = 0;
            var i2e3Count = 0;
            var i3e1Count = 0;
            var i3e5Count = 0;
            function interface1Callback1(ev) {
                logger.comment("*** interface1Callback1: Invoke");
                i1e1Count++;
                verify(ev.type, "Animals.IInterface1WithEvent.event1", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface1Callback1: Exit");
            }

            function interface1Callback2(ev) {
                logger.comment("*** interface1Callback2: Invoke");
                i1e2Count++;
                verify(ev.type, "event2", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface1Callback2: Exit");
            }

            function interface2Callback1(ev) {
                logger.comment("*** interface2Callback1: Invoke");
                i2e1Count++;
                verify(ev.type, "event21", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface2Callback1: Exit");
            }

            function interface2Callback3(ev) {
                logger.comment("*** interface2Callback3: Invoke");
                i2e3Count++;
                verify(ev.type, "event3", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface2Callback3: Exit");
            }

            function interface3Callback1(ev) {
                logger.comment("*** interface3Callback1: Invoke");
                i3e1Count++;
                verify(ev.type, "event31", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface3Callback1: Exit");
            }

            function interface3Callback5(ev) {
                logger.comment("*** interface3Callback5: Invoke");
                i3e5Count++;
                verify(ev.type, "event5", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface3Callback5: Exit");
            }

            logger.comment("Set the callbacks");
            myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"] = interface1Callback1;
            verify(myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"], interface1Callback1, 'myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"]');
            myRC6WithEvent.onevent2 = interface1Callback2;
            verify(myRC6WithEvent.onevent2, interface1Callback2, 'myRC6WithEvent.onevent2');
            myRC6WithEvent["onevent21"] = interface2Callback1;
            verify(myRC6WithEvent["onevent21"], interface2Callback1, 'myRC6WithEvent["onevent21"]');
            myRC6WithEvent.onevent3 = interface2Callback3;
            verify(myRC6WithEvent.onevent3, interface2Callback3, 'myRC6WithEvent.onevent3');
            myRC6WithEvent["onevent31"] = interface3Callback1;
            verify(myRC6WithEvent["onevent31"], interface3Callback1, 'myRC6WithEvent["onevent31"]');
            myRC6WithEvent.onevent5 = interface3Callback5;
            verify(myRC6WithEvent.onevent5, interface3Callback5, 'myRC6WithEvent.onevent5');

            logger.comment("Invoke events and methods");
            myRC6WithEvent.invokeEvent_I1E1("Animals.IInterface1WithEvent.event1");
            verify(i1e1Count, 1, "i1e1Count");
            myRC6WithEvent.invokeEvent_I1E2("event2");
            verify(i1e2Count, 1, "i1e2Count");
            myRC6WithEvent.invokeEvent_I2E1("event21");
            verify(i2e1Count, 1, "i2e1Count");
            myRC6WithEvent.invokeEvent_I2E3("event3");
            verify(i2e3Count, 1, "i2e3Count");
            var methodString = "This is onevent2 method"
            var result = myRC6WithEvent["Animals.IInterface2WithEvent.onevent2"](methodString);
            verify(result, methodString, 'myRC6WithEvent["Animals.IInterface2WithEvent.onevent2"]("This is onevent2 method")');
            myRC6WithEvent.invokeEvent_I3E1("event31");
            verify(i3e1Count, 1, "i3e1Count");
            myRC6WithEvent.invokeEvent_I3E5("event5");
            verify(i3e5Count, 1, "i3e5Count");

            logger.comment("Set the callbacks to null");
            myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"] = null;
            verify(myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"], null, 'myRC6WithEvent["Animals.IInterface1WithEvent.onevent1"]');
            myRC6WithEvent.onevent2 = null;
            verify(myRC6WithEvent.onevent2, null, 'myRC6WithEvent.onevent2');
            myRC6WithEvent["onevent21"] = null;
            verify(myRC6WithEvent["onevent21"], null, 'myRC6WithEvent["onevent21"]');
            myRC6WithEvent.onevent3 = null;
            verify(myRC6WithEvent.onevent3, null, 'myRC6WithEvent.onevent3');
            myRC6WithEvent["onevent31"] = null;
            verify(myRC6WithEvent["onevent31"], null, 'myRC6WithEvent["onevent31"]');
            myRC6WithEvent.onevent5 = null;
            verify(myRC6WithEvent.onevent5, null, 'myRC6WithEvent.onevent5');

            logger.comment("Invoke events and methods");
            myRC6WithEvent.invokeEvent_I1E1("Animals.IInterface1WithEvent.event1");
            verify(i1e1Count, 1, "i1e1Count");
            myRC6WithEvent.invokeEvent_I1E2("event2");
            verify(i1e2Count, 1, "i1e2Count");
            myRC6WithEvent.invokeEvent_I2E1("event21");
            verify(i2e1Count, 1, "i2e1Count");
            myRC6WithEvent.invokeEvent_I2E3("event3");
            verify(i2e3Count, 1, "i2e3Count");
            var methodString = "This is onevent2 method"
            var result = myRC6WithEvent["Animals.IInterface2WithEvent.onevent2"](methodString);
            verify(result, methodString, 'myRC6WithEvent["Animals.IInterface2WithEvent.onevent2"]("This is onevent2 method")');
            myRC6WithEvent.invokeEvent_I3E1("event31");
            verify(i3e1Count, 1, "i3e1Count");
            myRC6WithEvent.invokeEvent_I3E5("event5");
            verify(i3e5Count, 1, "i3e5Count");
        }
    });

    runner.addTest({
        id: 29,
        desc: 'NameConflictsOnEventHandler: event method name on other interface which doesnt have any events',
        pri: '0',
        test: function () {
            logger.comment("Create the object;");
            var myRC7WithEvent = new Animals.RC7WithEvent();
            verifyMembers("myRC7WithEvent", myRC7WithEvent, myRC7WithEventMembers);

            verify(myRC7WithEvent.onevent1, null, 'myRC7WithEvent.onevent1');
            verify(myRC7WithEvent.onevent2, null, 'myRC7WithEvent.onevent2');

            var event1Count = 0;
            var event2Count = 0;
            function interface1Callback1(ev) {
                logger.comment("*** interface1Callback1: Invoke");
                event1Count++;
                verify(ev.type, "event1", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface1Callback1: Exit");
            }

            function interface1Callback2(ev) {
                logger.comment("*** interface1Callback2: Invoke");
                event2Count++;
                verify(ev.type, "event2", "ev.type");
                verify(ev.detail[0], ev.type, "ev.detail[0]");
                logger.comment("*** interface1Callback2: Exit");
            }


            logger.comment("Set the callbacks");
            myRC7WithEvent.onevent1 = interface1Callback1;
            verify(myRC7WithEvent.onevent1, interface1Callback1, 'myRC7WithEvent.onevent1');
            myRC7WithEvent.onevent2 = interface1Callback2;
            verify(myRC7WithEvent.onevent2, interface1Callback2, 'myRC7WithEvent.onevent2');

            logger.comment("Invoke events and methods");
            myRC7WithEvent.invokeEvent_I1E1("event1");
            verify(event1Count, 1, "event1Count");
            myRC7WithEvent.invokeEvent_I1E2("event2");
            verify(event2Count, 1, "event2Count");
            var methodString = "This is onevent1 method"
            var result = myRC7WithEvent["Animals.IInterfaceWithOnEvent1.onevent1"](methodString);
            verify(result, methodString, 'myRC7WithEvent["Animals.IInterfaceWithOnEvent1.onevent1"]("This is onevent2 method")');

            logger.comment("Set the callbacks to null");
            myRC7WithEvent.onevent1 = null;
            verify(myRC7WithEvent.onevent1, null, 'myRC7WithEvent.onevent1');
            myRC7WithEvent.onevent2 = null;
            verify(myRC7WithEvent.onevent2, null, 'myRC7WithEvent.onevent2');

            logger.comment("Invoke events and methods");
            myRC7WithEvent.invokeEvent_I1E1("event1");
            verify(event1Count, 1, "event1Count");
            myRC7WithEvent.invokeEvent_I1E2("event2");
            verify(event2Count, 1, "event2Count");
            var methodString = "This is onevent1 method"
            var result = myRC7WithEvent["Animals.IInterfaceWithOnEvent1.onevent1"](methodString);
            verify(result, methodString, 'myRC7WithEvent["Animals.IInterfaceWithOnEvent1.onevent1"]("This is onevent2 method")');
        }
    });

    runner.addTest({
        id: 30,
        desc: 'TypeMismatch during addEventListener',
        pri: '0',
        test: function () {
            var toaster = createToaster();

            var eventCount = 0;
            verify.noException(function () {
                toaster.addEventListener("toastcompleteevent", eventCount);
            }, 'AddEventListener with eventCount');
        }
    });

    runner.addTest({
        id: 31,
        desc: 'TypeMismatch during removeEventListener',
        pri: '0',
        test: function () {
            var toaster = createToaster();

            var eventCount = 0;
            verify.noException(function () {
                toaster.removeEventListener("toastcompleteevent", eventCount);
            }, 'RemoveEventListener with eventCount');
        }
    });

    runner.addTest({
        id: 32,
        desc: 'TypeMismatch during event handler',
        pri: '0',
        test: function () {
            var toaster = createToaster();

            var eventCount = 0;
            verify.noException(function () {
                toaster.ontoastcompleteevent = eventCount;
            }, 'toaster.ontoastcompleteevent with eventCount');

            verify(toaster.ontoastcompleteevent, null, "toaster.ontoastcompleteevent");
        }
    });

    runner.addTest({
        id: 33,
        desc: 'Use delegate property and native delegate',
        pri: '0',
        test: function () {
            logger.comment("var myRC2WithEvent = new Animals.RC2WithEvent();");
            var myRC2WithEvent = new Animals.RC2WithEvent();
            verify(myRC2WithEvent.handler1.toString(), "\nfunction Interface2WithEventHandler() {\n    [native code]\n}\n", "myRC2WithEvent.handler1.toString()");

            var eventCount = 0;
            function callback1(ev) {
                logger.comment("*** Event Start : Interface2::Event1");
                eventCount++;
                verify(ev.target, myRC2WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface2::Event1");
            }

            function nativeDelegate(ev) {
                verify.exception(function () {
                    return myRC2WithEvent.handler1(ev);
                }, Error, "Too few arguments when calling native delegate with event arg");
            }
            logger.comment("myRC2WithEvent.onevent1 = nativeDelegate");
            myRC2WithEvent.onevent21 = nativeDelegate;

            logger.comment("myRC2WithEvent.invokeEvent_I2E1('NativeDelegateString');");
            try {
                myRC2WithEvent.invokeEvent_I2E1('NativeDelegateString');
            }
            catch (e) {
                if (typeof Animals._CLROnly !== 'undefined') { //CLR bug on this issue
                    logger.comment("CLR bug: 257509");
                    logger.comment(e.toString());
                }
            }
            verify(eventCount, 0, "eventCount");
            verify(myRC2WithEvent.wasHandler1Invoked, false, "native Delegate Invoked"); // We cannot invoke native delegate

            logger.comment("myRC2WithEvent.onevent1 = callback1");
            var myString = "This is IInterface2WithEvent::Event1";
            myRC2WithEvent.onevent21 = callback1;
            verify(myRC2WithEvent.onevent21, callback1, "myRC2WithEvent.onevent21");

            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);
            verify(eventCount, 1, "eventCount");
        }
    });

    runner.addTest({
        id: 34,
        desc: 'Calling event handler as delegate',
        pri: '0',
        test: function () {
            logger.comment("var myRC2WithEvent = new Animals.RC2WithEvent();");
            var myRC2WithEvent = new Animals.RC2WithEvent();

            var eventCount = 0;
            var delegateCount = 0;
            var useAsEvent = true;
            function callback1(ev, param2) {
                if (useAsEvent == true) {
                    logger.comment("*** Event : Invoke");
                    eventCount++;
                    verify(ev.target, myRC2WithEvent, "ev.target");
                    verify(ev, myString, "ev");
                    logger.comment("*** Event : Exit");
                }
                else {
                    logger.comment("*** Delegate : Invoke");
                    delegateCount++;
                    verify(ev, myRC2WithEvent, "sender");
                    verify(param2, myString, "hString");
                    logger.comment("*** Delegate : Exit");
                }
            }

            logger.comment("myRC2WithEvent.onevent21 = callback1");
            var myString = "This is IInterface2WithEvent::Event1";
            myRC2WithEvent.onevent21 = callback1;
            verify(myRC2WithEvent.onevent21, callback1, "myRC2WithEvent.onevent21");

            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);
            verify(eventCount, 1, "eventCount");
            verify(delegateCount, 0, "eventCount");

            myString = "Delegate String";
            useAsEvent = false;
            eventCount = 0;
            delegateCount = 0;
            logger.comment("myRC2WithEvent.invokeDelegate(callback1, myString)");
            myRC2WithEvent.invokeDelegate(callback1, myString);
            verify(eventCount, 0, "eventCount");
            verify(delegateCount, 1, "eventCount");

            myString = "Event String";
            useAsEvent = true;
            eventCount = 0;
            delegateCount = 0;
            logger.comment("myRC2WithEvent.invokeEvent_I2E1(myString);");
            myRC2WithEvent.invokeEvent_I2E1(myString);
            verify(eventCount, 1, "eventCount");
            verify(delegateCount, 0, "eventCount");
        }
    });

    runner.addTest({
        id: 35,
        desc: 'Multiple times event invoke and identity',
        pri: '0',
        test: function () {
            var toaster = createToaster();

            // Add a legitimate listener
            var eventCount = 0;
            var ev1;
            var ev2;
            var storeEv1 = true;
            function callback(ev) {
                logger.comment("*** callback invoke");
                eventCount++;
                if (storeEv1 == true) {
                    logger.comment("setting ev1");
                    ev1 = ev;
                    storeEv1 = false;
                }
                else {
                    logger.comment("setting ev2");
                    ev2 = ev;
                }
                logger.comment("*** callback exit");
            }

            toaster.addEventListener("toastcompleteevent", callback);

            logger.comment('Calling toaster.makeToast to ensure the legitimate listener is still there');
            toaster.makeToast(msg);
            verify(eventCount, 1, 'Number of events handled');
            assert(ev1 !== undefined, "ev1 !== undefined");
            assert(ev2 === undefined, "ev2 === undefined");

            toaster.makeToast(msg);
            verify(eventCount, 2, 'Number of events handled');
            assert(ev1 !== undefined, "ev1 !== undefined");
            assert(ev2 !== undefined, "ev2 !== undefined");

            assert(ev1 !== ev2, "ev1 !== ev2");
        }
    });

    runner.addTest({
        id: 36,
        desc: 'Setting indirect event',
        pri: '0',
        test: function () {
            var toaster = createToaster();
            var indirectToaster = createToaster();
            toaster.indirectToaster = indirectToaster;

            var eventCount = 0;
            function callback() { eventCount++; }
            toaster.addEventListener("indirecttoastcompleteevent", function () {
                eventCount++;
            });

            logger.comment("Make indirect toast");
            toaster.indirectMakeToast(msg);

            verify(eventCount, 1, "event count");
        }
    });

    runner.addTest({
        id: 37,
        desc: 'Setting indirect event and reseting toaster before invoking event, verify not collected if we do not supportWeakDelegates',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined' && TestUtilities.SupportsWeakDelegate() === false);
        },
        test: function () {
            var eventCount = 0;
            function createIndirectToaster() {
                var toaster = createToaster();
                var indirectToaster = createToaster();
                toaster.indirectToaster = indirectToaster;

                function callback() { eventCount++; }
                toaster.addEventListener("indirecttoastcompleteevent", function () {
                    eventCount++;
                });

                toaster = null;
                CollectGarbage();
                return indirectToaster;
            }

            var myToaster = createIndirectToaster();
            CollectGarbage();

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 1, "event count");
        }
    });

    runner.addTest({
        id: 38,
        desc: 'Setting indirect event and setting indirect toaster to null and then creating toast, the event should be fired',
        pri: '0',
        test: function () {

            var eventCount = 0;
            function createToasterWithIndirectToaster() {
                var toaster = createToaster();
                var indirectToaster = createToaster();
                toaster.indirectToaster = indirectToaster;

                function callback() { eventCount++; }
                toaster.addEventListener("indirecttoastcompleteevent", function () {
                    eventCount++;
                });

                indirectToaster = null;
                return toaster;
            }

            var myToaster = createToasterWithIndirectToaster();
            CollectGarbage();
            CollectGarbage();

            logger.comment("Make indirect toast");
            myToaster.indirectMakeToast(msg);

            verify(eventCount, 1, "event count");
        }
    });

    runner.addTest({
        id: 39,
        desc: 'Setting rooted delegate and making sure it isnt collected even if toaster is set to null',
        pri: '0',
        test: function () {

            var eventCount = 0;
            var toast = null;
            function createToasterWithRootedHandler() {
                var toaster = createToaster();
                var indirectToaster = createToaster();
                toaster.indirectToaster = indirectToaster;

                function callback() { eventCount++; }
                toaster.addEventListener("rootedtoastcompleteevent", function () {
                    eventCount++;
                });

                // Make toast
                toast = toaster.makeToast(msg);
                verify(eventCount, 1, "event count");

                logger.comment("Make rooted toast");
                indirectToaster.invokeRootedHandler(indirectToaster, toast);
                verify(eventCount, 2, "event count");

                toaster = null;
                return indirectToaster;
            }

            var myToaster = createToasterWithRootedHandler();
            CollectGarbage();
            CollectGarbage();

            logger.comment("Make rooted toast");
            myToaster.invokeRootedHandler(myToaster, toast);

            verify(eventCount, 3, "event count");
        }
    });

    runner.addTest({
        id: 40,
        desc: 'Setting rooted delegate and making sure it isnt collected even if toaster is set to null',
        pri: '0',
        test: function () {

            var eventCount = 0;
            var toast = null;
            function createToasterWithRootedHandler() {
                var toaster = createToaster();
                var indirectToaster = createToaster();
                toaster.indirectToaster = indirectToaster;

                function callback() { eventCount++; }
                toaster.addEventListener("rootedtoastcompleteevent", function () {
                    eventCount++;
                });

                // Make toast
                toast = toaster.makeToast(msg);
                verify(eventCount, 1, "event count");

                logger.comment("Make rooted toast");
                indirectToaster.invokeRootedHandler(indirectToaster, toast);
                verify(eventCount, 2, "event count");

                indirectToaster.rootedHandler = null;
                return toaster;
            }

            var myToaster = createToasterWithRootedHandler();
            CollectGarbage();
            CollectGarbage();

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 3, "event count");
        }
    });

    runner.addTest({
        id: 41,
        desc: 'Cross Apartment event invoke',
        pri: '0',
        test: function () {
            var eventCount = 0;
            function preheatEvent(ev) {
                eventCount++;
            }

            var toaster = new Fabrikam.Kitchen.Toaster();
            logger.comment('Fabrikam.Kitchen.Toaster created successfully');

            logger.comment('Add preheat bg complete event handler');
            toaster.onpreheatcompletebackground = preheatEvent;

            logger.comment('Calling toaster.preheatInBackground');
            toaster.preheatInBackground(null);

            logger.comment('toaster.preheatInBackground done');
            verify(eventCount, 1, 'eventCount');
        }
    });

    // This will fail fast if registry key is set
    runner.addTest({
        id: 42,
        desc: 'Setting indirect event and reseting toaster before invoking event, verify the error is thrown',
        pri: '0',
        preReq: function () {
            return (typeof TestUtilities !== 'undefined' && TestUtilities.SupportsWeakDelegate() === true);
        },
        test: function () {

            var eventCount = 0;
            function createIndirectToaster() {
                var toaster = createToaster();
                var indirectToaster = createToaster();
                toaster.indirectToaster = indirectToaster;

                function callback() { eventCount++; }
                toaster.addEventListener("indirecttoastcompleteevent", function () {
                    eventCount++;
                });

                toaster = null;
                CollectGarbage();
                return indirectToaster;
            }

            var myToaster = createIndirectToaster();
            CollectGarbage();

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 0, "event count");
        }
    });

    runner.addTest({
        id: 43,
        desc: 'Memory leak bugfix (BLUE#105816) with ForInEnumerator and CustomExternalObject',
        pri: '0',
        test: function () {
            logger.comment("Create the chef and toaster");
            var myToaster = toaster = new Fabrikam.Kitchen.Toaster();
            var myKitchen = new Fabrikam.Kitchen.Kitchen();
            var myChef = new Fabrikam.Kitchen.Chef('Bob', myKitchen, 42);
            var eventInvokeCount = 0;

            function multipleToastCompleteCollectionCallback(ev) {
                logger.comment("*** multipleToastCompleteCollectionCallback : Invoke");
                eventInvokeCount++;

                // Verify Ev
                logger.comment("-- before for() --\n");
                for (var t in ev) {
                    t;
                    //logger.comment(t + "\n");
                }
            }

            logger.comment("Add event listener for multiple toast make complete event");
            myChef.addEventListener("multipletoastcompletecollection", multipleToastCompleteCollectionCallback);

            myChef.makeBreakfast(myToaster, 5);
            verify(eventInvokeCount, 1, 'eventInvokeCount');
        }
    });
    runner.addTest({
        id: 44,
        desc: 'BLUE:436929 - UpdateRootedState AV after early collection via msReleaseWinRTObject',
        pri: '0',
        test: function () {
            logger.comment('Create a new empty toaster');
            var localToaster = new Fabrikam.Kitchen.Toaster();

            logger.comment('Add an event listener delegate');
            localToaster.addEventListener("toastcompleteevent", function(){});

            logger.comment('Cause early collection of the event listener delegate');
            msReleaseWinRTObject(localToaster);

            logger.comment('Force GC');
            CollectGarbage();

            logger.comment('GC succeeds');
        }
    });

    Loader42_FileName = 'Events tests';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
