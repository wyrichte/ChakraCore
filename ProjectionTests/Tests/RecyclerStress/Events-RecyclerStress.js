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
    ];

    var evToastMembers = [
        ['target', 'object'],
        ['detail', 'object'],
        ['type', 'string'],
        ['message', 'string'],
    ];

    var toastMembers = [
        ['message', 'string'],
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
        id: 2,
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
        id: 3,
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
        id: 4,
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
        id: 5,
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
                logger.comment("*** Event Start : Interface2::Event21");
                verify(ev.target, myRC1WithEvent, "ev.target");
                verify(ev, myString, "ev");
                logger.comment("*** Event Complete : Interface2::Event1");
            }

            myString = "This is IInterface2WithEvent::Event21";
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

            myString = "This is IInterface2WithEvent::Event21";
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
        id: 6,
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
        id: 7,
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
        id: 8,
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
        id: 9,
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
        id: 10,
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
        id: 11,
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
        id: 12,
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
        id: 13,
        desc: 'NameConflictsOnEventHandler: With event name collision of event1 and event method collision onevent2',
        pri: '0',
        test: function () {
            logger.comment("Create the object;");
            var myRC1WithEvent = new Animals.RC1WithEvent();
            verify(myRC1WithEvent.onevent1, null, 'myRC1WithEvent.onevent1');
            verify(myRC1WithEvent.onevent2, null, 'myRC1WithEvent.onevent2');
            verify(myRC1WithEvent.onevent21, null, 'myRC1WithEvent.onevent21');
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
            myRC1WithEvent.onevent1 = interface1Callback1;
            verify(myRC1WithEvent.onevent1, interface1Callback1, 'myRC1WithEvent.onevent1');
            myRC1WithEvent.onevent2 = interface1Callback2;
            verify(myRC1WithEvent.onevent2, interface1Callback2, 'myRC1WithEvent.onevent2');
            myRC1WithEvent.onevent21 = interface2Callback1;
            verify(myRC1WithEvent.onevent21, interface2Callback1, 'myRC1WithEvent.onevent21');
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
            myRC1WithEvent.onevent1 = null;
            verify(myRC1WithEvent.onevent1, null, 'myRC1WithEvent.onevent1');
            myRC1WithEvent.onevent2 = null;
            verify(myRC1WithEvent.onevent2, null, 'myRC1WithEvent.onevent2');
            myRC1WithEvent.onevent21 = null;
            verify(myRC1WithEvent.onevent21, null, 'myRC1WithEvent.onevent21');
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
        id: 14,
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
        id: 15,
        desc: 'Setting indirect event and reseting toaster before invoking event, verify the delegate is alive',
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
                return indirectToaster;
            }

            var myToaster = createIndirectToaster();
            CollectGarbage();
            CollectGarbage();

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 1, "event count");
        }
    });

    runner.addTest({
        id: 16,
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
        id: 17,
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
        id: 18,
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
        id: 19,
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

    // Fail fasts if set in the registry key

    runner.addTest({
        id: 20,
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
                return indirectToaster;
            }

            var myToaster = createIndirectToaster();
            CollectGarbage();
            CollectGarbage();

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 0, "event count");
        }
    });

    Loader42_FileName = 'Recycler Stress Selected Scenarios from events.js';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
