if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
Loader42_FileName = 'Promise Tests';

function asyncWithoutThenHandler() {
    var winery = new Winery.RWinery(1);
    logger.comment("Get the promise object");
    var async = winery.asyncOperationOut();
    if (async.then == undefined) {
        fail("expected then");
    }
    if (async.cancel == undefined) {
        fail("expected cancel");
    }
    if (async.operation == undefined) {
        fail("expected operation");
    }
}

function completeSuccessfully() {
    var winery = new Winery.RWinery(1);
    var completeCalled = false;
    var resultWas = 0;
    logger.comment("Get the promise object");
    var promise = winery.asyncOperationOut();

    logger.comment("Add Completed Handler");
    promise
            .then(function (result) {
                completeCalled = true;
                resultWas = result;
            });

    logger.comment("Simulate the completion");
    promise.operation.moveToCompleted(192); // Simulate asynchronous completion
    if (!completeCalled) {
        fail("expected completeCalled");
    }
    if (resultWas != 192) {
        fail("expected result to be 192");
    }
}

function completeWithCancel() {
    var winery = new Winery.RWinery(1);
    var completeCalled = false;
    var errorCalled = false;
    var errorResult = null;
    logger.comment("Get the promise object");
    var promise = winery.asyncOperationOut();

    logger.comment("Add Completed and Error handler");
    promise.then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );

    logger.comment("Simulate asynchronous cancel");
    promise.operation.cancel(); // Simulate asynchronous cancel
    if (completeCalled) {
        fail("expected no 'then' call");
    }
    if (!errorCalled) {
        fail("expected 'cancel' to be called");
    }
    if (errorResult.message !== "Canceled") {
        fail("expected error message to be 'Canceled'");
    }
}

function completeWithError() {
    var winery = new Winery.RWinery(1);
    var completeCalled = false;
    var errorCalled = false;
    var errorResult = null;

    logger.comment("Get the promise object");
    var promise = winery.asyncOperationOut();

    logger.comment("Add Completed and Error handler");
    promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );

    logger.comment("Simulate asynchronous error");
    promise.operation.moveToError(); // Simulate asynchronous error
    if (completeCalled) {
        fail("expected no 'then' call");
    }
    if (!errorCalled) {
        fail("expected 'error' to be called");
    }
}

function getPromiseWithoutCleanup() {
    logger.comment("Get the AsyncInfo from the vector");
    return new Animals.PropertyValueTests().receiveVectorOfAsyncInfo_InspectableOut()[0];
}

function promiseWithoutCleanup() {
    var promise = getPromiseWithoutCleanup();

    logger.comment("Perform gc");
    CollectGarbage();
    logger.comment("GC complete");

    var completeCalled = false;
    var resultWas = 0;

    promise.then(function (result) {
        completeCalled = true;
        resultWas = result;
    });

    logger.comment("Simulate the completion");
    promise.operation.moveToCompleted(192); // Simulate asynchronous completion
    verify(completeCalled, true, "complete called");
    verify(resultWas, 192, "result from the completion");
}

runner.addTest({
    id: 1,
    desc: 'Without Any Then Handler',
    pri: '0',
    test: function () {
        asyncWithoutThenHandler();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 2,
    desc: 'Complete successfully',
    pri: '0',
    test: function () {
        completeSuccessfully();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 3,
    desc: 'Complete with Cancel',
    pri: '0',
    test: function () {
        completeWithCancel();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

runner.addTest({
    id: 4,
    desc: 'Complete with error',
    pri: '0',
    test: function () {
        completeWithError();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

// This test creates a promise object which doesn't clean up handlers in WinRT side.
// It causes a real memory leak. Leak reporting is turned off for Promise.js due to this test.
runner.addTest({
    id: 5,
    desc: 'Promise that doesnt cleanup its own handlers in close',
    pri: '0',
    test: function () {
        promiseWithoutCleanup();

        logger.comment("Perform gc");
        CollectGarbage();
        logger.comment("GC complete");
    }
});

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
