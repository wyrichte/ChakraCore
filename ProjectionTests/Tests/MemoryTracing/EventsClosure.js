if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
Loader42_FileName = 'Event Closure Tests';

var msg = "Toast";

function createToaster() {
    logger.comment("Create Toaster");
    return new Fabrikam.Kitchen.Toaster();
}

function createToasterAndAddEvent() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    logger.comment("Add event listner");
    toaster.addEventListener("toastcompleteevent", function (ev) {
        logger.comment("*** toastCompleteCallback: Enter");
        verify(Animals.Animal.getRefCount(toaster), 3, "toaster refCount inside event handler : toaster");
        verify(Animals.Animal.getRefCount(ev.target), 3, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");
        logger.comment("*** toastCompleteCallback: Exit");
    });
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    return toaster;
}

function createToasterAndAddEvent1() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    logger.comment("Add event listner");
    toaster.addEventListener("toastcompleteevent", function (ev) {
        logger.comment("*** toastCompleteCallback: Enter");
        verify(Animals.Animal.getRefCount(toaster), 3, "toaster refCount inside event handler : toaster");
        verify(Animals.Animal.getRefCount(ev.target), 3, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");

        logger.comment("Try breaking circular reference when this function is invoked");
        toaster = null;
        logger.comment("*** toastCompleteCallback: Exit");
    });
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    return toaster;
}

function createToasterAddEventAndInvokeEvent() {
    var toaster = createToasterAndAddEvent();

    logger.comment("Make Toast");
    toaster.makeToast("My Toast");

    return toaster;
}

function createToasterAddEventAndInvokeEvent1() {
    var toaster = createToasterAndAddEvent1();

    logger.comment("Make Toast");
    toaster.makeToast("My Toast");

    return toaster;
}

function createToasterAddEventRemoveEvent() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    function toastCompleteEvent(ev) {
        logger.comment("*** toastCompleteCallback: Enter");
        verify(Animals.Animal.getRefCount(toaster), 4, "toaster refCount inside event handler : toaster");
        verify(Animals.Animal.getRefCount(ev.target), 4, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");
        logger.comment("*** toastCompleteCallback: Exit");
    }

    logger.comment("Add event listner");
    toaster.addEventListener("toastcompleteevent", toastCompleteEvent);
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    logger.comment("Remove event listner");
    toaster.removeEventListener("toastcompleteevent", toastCompleteEvent);

    return toaster;
}

function createToasterAddEventInvokeEventRemoveEvent() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    function toastCompleteEvent(ev) {
        logger.comment("*** toastCompleteCallback: Enter");
        verify(Animals.Animal.getRefCount(toaster), 3, "toaster refCount inside event handler : toaster");
        verify(Animals.Animal.getRefCount(ev.target), 3, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");
        logger.comment("*** toastCompleteCallback: Exit");
    }

    logger.comment("Add event listner");
    toaster.addEventListener("toastcompleteevent", toastCompleteEvent);
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    logger.comment("Make Toast");
    toaster.makeToast("My Toast");

    logger.comment("Remove event listner");
    toaster.removeEventListener("toastcompleteevent", toastCompleteEvent);

    return toaster;
}

function createToasterAddEventStoreInAnimal() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    function toastCompleteEvent(ev) {
        logger.comment("*** toastCompleteCallback: Enter");
        verify(Animals.Animal.getRefCount(toaster), 4, "toaster refCount inside event handler : toaster");
        verify(Animals.Animal.getRefCount(ev.target), 4, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");
        logger.comment("*** toastCompleteCallback: Exit");
    }

    logger.comment("Add event listner");
    toaster.addEventListener("toastcompleteevent", toastCompleteEvent);
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    logger.comment("Store the toaster into Animals.Animal");
    Animals.Animal.myToaster = toaster;
}

function createToasterAddEventStoreInAnimal1() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    function toastCompleteEvent(ev) {
        logger.comment("*** toastCompleteCallback: Enter");
        verify(Animals.Animal.getRefCount(toaster), 4, "toaster refCount inside event handler : toaster");
        verify(Animals.Animal.getRefCount(ev.target), 4, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");

        logger.comment("Break circular reference by setting toaster to null");
        toaster = null;

        logger.comment("*** toastCompleteCallback: Exit");
    }

    logger.comment("Add event listner");
    toaster.addEventListener("toastcompleteevent", toastCompleteEvent);
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    logger.comment("Store the toaster into Animals.Animal");
    Animals.Animal.myToaster = toaster;
}

runner.addTest({
    id: 1,
    desc: 'Simple Closure',
    pri: '0',
    test: function () {
        createToasterAndAddEvent();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 2,
    desc: 'Invoking event',
    pri: '0',
    test: function () {
        createToasterAddEventAndInvokeEvent();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});


runner.addTest({
    id: 3,
    desc: 'Remove Event',
    pri: '0',
    test: function () {
        createToasterAddEventRemoveEvent();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});


runner.addTest({
    id: 4,
    desc: 'Remove Event after Invoke',
    pri: '0',
    test: function () {
        createToasterAddEventInvokeEventRemoveEvent();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 5,
    desc: 'Create Closure and save it in ABI. Note that the object wont get collected',
    pri: '0',
    test: function () {
        createToasterAddEventStoreInAnimal();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");

        var toaster = Animals.Animal.myToaster;
        toaster.makeToast("Hello");
        toaster = null;

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");

        Animals.Animal.myToaster = null;

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 6,
    desc: 'Create Closure but inside the event handler set the toaster to null, note that the closure cannot be collected since event is not invoked',
    pri: '0',
    test: function () {
        createToasterAndAddEvent1();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 7,
    desc: 'Create Closure but inside the event handler set the toaster to null, invoke the event, note that the closure can be collected',
    pri: '0',
    test: function () {
        createToasterAddEventAndInvokeEvent1();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 8,
    desc: 'Create Closure and save it in ABI. Note that the object wont get collected',
    pri: '0',
    test: function () {
        createToasterAddEventStoreInAnimal1();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");

        var toaster = Animals.Animal.myToaster;
        toaster.makeToast("Hello");
        toaster = null;

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 9,
    desc: 'Setting indirect event',
    pri: '0',
    test: function () {
        function actualFunction() {
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

        actualFunction();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 10,
    desc: 'Setting indirect event and setting indirect toaster to null and then creating toast, the event should be fired',
    pri: '0',
    test: function () {

        function actualFunction() {
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

            logger.comment("Perform gc");
            CollectGarbage();
            logger.comment("GC complete");

            logger.comment("Make indirect toast");
            myToaster.indirectMakeToast(msg);

            verify(eventCount, 1, "event count");
        }

        actualFunction();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 11,
    desc: 'Setting rooted delegate and making sure it isnt collected even if toaster is set to null',
    pri: '0',
    test: function () {

        function actualFunction() {
            var eventCount = 0;
            function createToasterWithRootedHandler() {
                var toaster = createToaster();
                var toast = null;
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
                return {
                    _toaster: indirectToaster,
                    _toast: toast
                };
            }

            var newObject = createToasterWithRootedHandler();

            logger.comment("Perform gc");
            CollectGarbage();
            logger.comment("GC complete");

            logger.comment("Make rooted toast");
            newObject._toaster.invokeRootedHandler(newObject._toaster, newObject._toast);

            verify(eventCount, 3, "event count");

            logger.comment("Perform gc");
            CollectGarbage();
            logger.comment("GC complete");
        }

        actualFunction();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 12,
    desc: 'Setting rooted delegate and making sure it isnt collected even if toaster is set to null',
    pri: '0',
    test: function () {

        function actualFunction() {
            var eventCount = 0;
            function createToasterWithRootedHandler() {
                var toast = null;
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

            logger.comment("Perform gc");
            CollectGarbage();
            logger.comment("GC complete");

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 3, "event count");
        }

        actualFunction();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

// Fail fasts if the registry key set
runner.addTest({
    id: 13,
    desc: 'Setting indirect event and reseting toaster before invoking event, verify the error is thrown',
    pri: '0',
    preReq: function () {
        return (typeof TestUtilities !== 'undefined' && TestUtilities.SupportsWeakDelegate() === true);
    },
    test: function () {

        function actualFunction() {

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

            logger.comment("Perform gc");
            CollectGarbage();
            logger.comment("GC complete");

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 0, "event count");
        }

        actualFunction();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});



runner.addTest({
    id: "Last",
    desc: 'Setting indirect event and reseting toaster before invoking event, verify delegate is alive',
    pri: '0',
    preReq: function () {
        return (typeof TestUtilities !== 'undefined' && TestUtilities.SupportsWeakDelegate() === false);
    },
    test: function () {

        function actualFunction() {

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

            logger.comment("Perform gc");
            CollectGarbage();
            logger.comment("GC complete");

            logger.comment("Make toast");
            myToaster.makeToast(msg);

            verify(eventCount, 1, "event count");
        }

        actualFunction();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});Run() 
