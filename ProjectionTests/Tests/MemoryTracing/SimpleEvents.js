if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
Loader42_FileName = 'Simple Event Tests';

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
        verify(Animals.Animal.getRefCount(ev.target), 3, "toaster refCount inside event handler : ev.target");
        verify(Animals.Animal.getRefCount(ev.detail[0]), 7, "toast refCount inside event handler: ev.detail[0]");
        verify(Animals.Animal.getRefCount(ev), 7, "toast refCount inside event handler: ev");
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

function createToasterAddEventRemoveEvent() {
    var toaster = createToaster();
    verify(Animals.Animal.getRefCount(toaster), 2, "toaster refCount");

    function toastCompleteEvent(ev) {
        logger.comment("*** toastCompleteCallback: Enter");
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

runner.addTest({
    id: 1,
    desc: 'Simple event handler',
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
    desc: 'Add Event and Pass the object to ABI',
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

        logger.comment("Set the toaster to null and see that delegate for event handler is released");
        Animals.Animal.myToaster = null;
    }
});
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
