if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var setTimeOutCalls = 0;
    var savedSetTimeout = undefined;
    if (typeof setTimeout !== 'undefined')
        savedSetTimeout = setTimeout;
    runner.globalSetup(function () {
        setTimeout = function () { setTimeOutCalls = setTimeOutCalls + 1; }
    });
    runner.globalTeardown(function () {
        setTimeout = savedSetTimeout;
    });


    runner.addTest({
        id: 1,
        desc: 'AsyncOut',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
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
            if (async.done == undefined) {
                fail("expected done");
            }
        }
    });

    runner.addTest({
        id: 2,
        desc: 'CompleteSuccessfully',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) {
                completeCalled = true;
                resultWas = result;
            });
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
            if (!completeCalled) {
                fail("expected completeCalled");
            }
            if (resultWas != 192) {
                fail("expected result to be 192");
            }
        }
    });

    runner.addTest({
        id: '2-done',
        desc: 'CompleteSuccessfully',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var resultWas = 0;
            var promise = winery.asyncOperationOut();

            promise
            .done(function (result) {
                completeCalled = true;
                resultWas = result;
            });
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
            if (!completeCalled) {
                fail("expected completeCalled");
            }
            if (resultWas != 192) {
                fail("expected result to be 192");
            }
        }
    });

    runner.addTest({
        id: 2.1,
        desc: 'CompleteSuccessfully-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) {
                completeCalled = true;
                resultWas = result;
            });
            promise.operation.moveToCompleted(null); // Simulate asynchronous completion
            if (!completeCalled) {
                fail("expected completeCalled");
            }
            if (resultWas != null) {
                fail("expected result to be null");
            }
        }
    });

    runner.addTest({
        id: '2.1-done',
        desc: 'CompleteSuccessfully-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) {
                completeCalled = true;
                resultWas = result;
            });
            promise.operation.moveToCompleted(null); // Simulate asynchronous completion
            if (!completeCalled) {
                fail("expected completeCalled");
            }
            if (resultWas != null) {
                fail("expected result to be null");
            }
        }
    });

    runner.addTest({
        id: 3,
        desc: 'CompleteWithCancel',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.cancel(); // Simulate asynchronous cancel
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'cancel' to be called");
            }
            if (errorResult.name !== "Canceled") {
                fail("expected error name to be 'Canceled'");
            }
            if (errorResult.message !== "Canceled") {
                fail("expected error message to be 'Canceled'");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '3-done',
        desc: 'CompleteWithCancel',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.cancel(); // Simulate asynchronous cancel
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'cancel' to be called");
            }
            if (errorResult.name !== "Canceled") {
                fail("expected error name to be 'Canceled'");
            }
            if (errorResult.message !== "Canceled") {
                fail("expected error message to be 'Canceled'");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: 4,
        desc: 'CompleteWithError',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.moveToError(); // Simulate asynchronous error
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '4-done',
        desc: 'CompleteWithError',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.moveToError(); // Simulate asynchronous error
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: 5,
        desc: 'TwoThensFollowedByCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise
            .then(function (result) {
                completeCalled = completeCalled + "second";
            });

            promise.operation.moveToCompleted(192); // Both 'thens' should be called.
            if (completeCalled != "first second") {
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: '5-done',
        desc: 'TwoDonesFollowedByCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .done(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise
            .done(function (result) {
                completeCalled = completeCalled + "second";
            });

            promise.operation.moveToCompleted(192); // Both 'thens' should be called.
            if (completeCalled != "first second") {
                fail("expected completeCalled");
            }
        }
    });


    runner.addTest({
        id: 5.1,
        desc: 'TwoThensFollowedByCompleted-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise
            .then(function (result) {
                completeCalled = completeCalled + "second";
            });

            promise.operation.moveToCompleted(null); // Both 'thens' should be called.
            if (completeCalled != "first second") {
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: '5.1-done',
        desc: 'TwoDonesFollowedByCompleted-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .done(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise
            .done(function (result) {
                completeCalled = completeCalled + "second";
            });

            promise.operation.moveToCompleted(null); // Both 'thens' should be called.
            if (completeCalled != "first second") {
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: 6,
        desc: 'ThensFollowedByCompletedFollowedByThen',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise.operation.moveToCompleted(192); // Complete before the next 'then' is attached. Second should still get called
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "second";
                });

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: '6-done',
        desc: 'DoneFollowedByCompletedFollowedByDone',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .done(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise.operation.moveToCompleted(192); // Complete before the next 'done' is attached. Second should still get called
            promise
                .done(function (result) {
                    completeCalled = completeCalled + "second";
                });

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: 6.1,
        desc: 'ThensFollowedByCompletedFollowedByThen-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise.operation.moveToCompleted(null); // Complete before the next 'then' is attached. Second should still get called
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "second";
                });

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: '6.1-done',
        desc: 'DoneFollowedByCompletedFollowedByDone-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .done(function (result) {
                    completeCalled = completeCalled + "first ";
                });
            promise.operation.moveToCompleted(null); // Complete before the next 'then' is attached. Second should still get called
            promise
                .done(function (result) {
                    completeCalled = completeCalled + "second";
                });

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: 7,
        desc: 'ThensFollowedByErrorFollowedByThen',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = "";
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = "first ";
                }
            );
            promise.operation.moveToError(); // Simulate asynchronous error
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = errorCalled + "second";
                }
            );

            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (errorCalled != "first second") {
                fail("expected errorCalled");
            }
        }
    });

    runner.addTest({
        id: '7-done',
        desc: 'DoneFollowedByErrorFollowedByDone',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = "";
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) {
                completeCalled = true;
            },
                function (error) {
                    errorCalled = "first ";
                }
            );
            promise.operation.moveToError(); // Simulate asynchronous error
            promise
            .done(function (result) {
                completeCalled = true;
            },
                function (error) {
                    errorCalled = errorCalled + "second";
                }
            );

            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (errorCalled != "first second") {
                fail("expected errorCalled");
            }
        }
    });

    runner.addTest({
        id: 8,
        desc: 'ChainedThensFollowedByCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                })
                .then(function (result) {
                    completeCalled = completeCalled + "second";
                });
            promise.operation.moveToCompleted(192); // Complete before the next 'then' is attached. Second should still get called

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: '8-done',
        desc: 'ThenDoneFollowedByCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                })
                .done(function (result) {
                    completeCalled = completeCalled + "second";
                });
            promise.operation.moveToCompleted(192); // Complete before the next 'then' is attached. Second should still get called

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: 8.1,
        desc: 'ChainedThensFollowedByCompleted-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                })
                .then(function (result) {
                    completeCalled = completeCalled + "second";
                });
            promise.operation.moveToCompleted(null); // Complete before the next 'then' is attached. Second should still get called

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: '8.1-done',
        desc: 'ThenDoneFollowedByCompleted-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                })
                .done(function (result) {
                    completeCalled = completeCalled + "second";
                });
            promise.operation.moveToCompleted(null); // Complete before the next 'then' is attached. Second should still get called

            if (completeCalled != "first second") {
                logger.comment(completeCalled);
                fail("expected completeCalled");
            }
        }
    });

    runner.addTest({
        id: 9,
        desc: 'ChainedThensFollowedByCancel',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var errorCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(null, function (result) {
                    errorCalled = errorCalled + "first";
                })
                .then(null, function (result) {
                    errorCalled = errorCalled + "second";
                })
                .cancel(); // Should cause first cancel function to be called and so the second then is not executed

            if (errorCalled != "first") {
                logger.comment(errorCalled);
                fail("expected single errorCalled");
            }
        }
    });

    runner.addTest({
        id: '9-null',
        desc: 'ThenDoneFollowedByCancel',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var errorCalled = "";
            var resultWas = 0;
            var promise = winery.asyncOperationOut();
            promise
                .then(null, function (result) {
                    errorCalled = errorCalled + "first";
                })
                .done(null, function (result) {
                    errorCalled = errorCalled + "second";
                });
            promise.cancel(); // Should cause first cancel function to be called and so the second then is not executed

            if (errorCalled != "first") {
                logger.comment(errorCalled);
                fail("expected single errorCalled");
            }
        }
    });

    runner.addTest({
        id: 10,
        desc: 'ExceptionThrownFromOnCompleteHandler',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "first"; throw "error from onComplete1"; },
                    function (result) { errorCalled = errorCalled + "first"; }
                ).then(
                    function (result) { completeCalled = completeCalled + "second"; throw "error from onComplete2"; },
                    function (result) { errorCalled = errorCalled + "second(" + result + ")"; }
                );
            promise.operation.moveToCompleted(192);

            if (completeCalled != "first") {
                logger.comment(completeCalled);
                fail("expected only first onComplete to be called");
            }
            if (errorCalled != "second(error from onComplete1)") {
                logger.comment(errorCalled);
                fail("expected only first onError to be called");
            }
        }
    });

    runner.addTest({
        id: '10-done',
        desc: 'ExceptionThrownFromOnCompleteHandler-done',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "first"; throw "error from onComplete1"; },
                    function (result) { errorCalled = errorCalled + "first"; }
                ).done(
                    function (result) { completeCalled = completeCalled + "second"; throw "error from onComplete2"; },
                    function (result) { errorCalled = errorCalled + "second(" + result + ")"; }
                );
            promise.operation.moveToCompleted(192);

            if (completeCalled != "first") {
                logger.comment(completeCalled);
                fail("expected only first onComplete to be called");
            }
            if (errorCalled != "second(error from onComplete1)") {
                logger.comment(errorCalled);
                fail("expected only first onError to be called");
            }
        }
    });

    runner.addTest({
        id: 10.1,
        desc: 'ExceptionThrownFromOnCompleteHandler-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "first"; throw "error from onComplete1"; },
                    function (result) { errorCalled = errorCalled + "first"; }
                ).then(
                    function (result) { completeCalled = completeCalled + "second"; throw "error from onComplete2"; },
                    function (result) { errorCalled = errorCalled + "second(" + result + ")"; }
                );
            promise.operation.moveToCompleted(null);

            if (completeCalled != "first") {
                logger.comment(completeCalled);
                fail("expected only first onComplete to be called");
            }
            if (errorCalled != "second(error from onComplete1)") {
                logger.comment(errorCalled);
                fail("expected only first onError to be called");
            }
        }
    });

    runner.addTest({
        id: '10.1-done',
        desc: 'ExceptionThrownFromOnCompleteHandler-done-null',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "first"; throw "error from onComplete1"; },
                    function (result) { errorCalled = errorCalled + "first"; }
                ).done(
                    function (result) { completeCalled = completeCalled + "second"; throw "error from onComplete2"; },
                    function (result) { errorCalled = errorCalled + "second(" + result + ")"; }
                );
            promise.operation.moveToCompleted(null);

            if (completeCalled != "first") {
                logger.comment(completeCalled);
                fail("expected only first onComplete to be called");
            }
            if (errorCalled != "second(error from onComplete1)") {
                logger.comment(errorCalled);
                fail("expected only first onError to be called");
            }
        }
    });

    runner.addTest({
        id: 11,
        desc: 'ExceptionThrownFromOnErrorHandler',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "first"; },
                    function (result) { errorCalled = errorCalled + "first(" + result + ") "; throw "error from onError1"; }
                ).then(
                    function (result) { completeCalled = completeCalled + "second"; },
                    function (result) { errorCalled = errorCalled + "second(" + result + ")"; }
                );
            promise.cancel();

            if (completeCalled != "") {
                logger.comment(completeCalled);
                fail("expected no call to on onComplete");
            }
            if (errorCalled != "first(Canceled: Canceled) second(error from onError1)") {
                logger.comment(errorCalled);
                fail("expected both onError handlers to be called");
            }
        }
    });

    runner.addTest({
        id: '11-done',
        desc: 'ExceptionThrownFromOnErrorHandler',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "first"; },
                    function (result) { errorCalled = errorCalled + "first(" + result + ") "; throw "error from onError1"; }
                ).done(
                    function (result) { completeCalled = completeCalled + "second"; },
                    function (result) { errorCalled = errorCalled + "second(" + result + ")"; }
                );
            promise.cancel();

            if (completeCalled != "") {
                logger.comment(completeCalled);
                fail("expected no call to on onComplete");
            }
            if (errorCalled != "first(Canceled: Canceled) second(error from onError1)") {
                logger.comment(errorCalled);
                fail("expected both onError handlers to be called");
            }
        }
    });


    runner.addTest({
        id: 12,
        desc: 'ProgressThenCancel',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var progressCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "onComplete(" + result + ")"; },
                    function (result) { errorCalled = errorCalled + "onError(" + result + ")"; },
                    function (percent) { progressCalled = progressCalled + "onProgress(" + percent + ")"; }
                );

            // Step through some progress.
            promise.operation.progress(promise.operation, 10);
            promise.operation.progress(promise.operation, 20);
            promise.operation.progress(promise.operation, 99);

            // Then cancel.
            promise.cancel();

            if (completeCalled != "") {
                logger.comment(completeCalled);
                fail("expected no call to on onComplete");
            }
            if (errorCalled != "onError(Canceled: Canceled)") {
                logger.comment(errorCalled);
                fail("expected onError be called");
            }
            if (progressCalled != "onProgress(10)onProgress(20)onProgress(99)") {
                logger.comment(progressCalled);
                fail("expected onProgress to be called");
            }
        }
    });

    runner.addTest({
        id: '12-done',
        desc: 'ProgressThenCancel',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var progressCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .done(
                    function (result) { completeCalled = completeCalled + "onComplete(" + result + ")"; },
                    function (result) { errorCalled = errorCalled + "onError(" + result + ")"; },
                    function (percent) { progressCalled = progressCalled + "onProgress(" + percent + ")"; }
                );

            // Step through some progress.
            promise.operation.progress(promise.operation, 10);
            promise.operation.progress(promise.operation, 20);
            promise.operation.progress(promise.operation, 99);

            // Then cancel.
            promise.cancel();

            if (completeCalled != "") {
                logger.comment(completeCalled);
                fail("expected no call to on onComplete");
            }
            if (errorCalled != "onError(Canceled: Canceled)") {
                logger.comment(errorCalled);
                fail("expected onError be called");
            }
            if (progressCalled != "onProgress(10)onProgress(20)onProgress(99)") {
                logger.comment(progressCalled);
                fail("expected onProgress to be called");
            }
        }
    });

    runner.addTest({
        id: 13,
        desc: 'ExceptionFromProgress',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var progressCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .then(
                    function (result) { completeCalled = completeCalled + "onComplete(" + result + ")"; },
                    function (result) { errorCalled = errorCalled + "onError(" + result + ")"; },
                    function (percent) { progressCalled = progressCalled + "onProgress(" + percent + ")"; throw "Exception from onProgress"; }
                );

            // Step through some progress.
            promise.operation.progress(promise.operation, 10);
            promise.operation.progress(promise.operation, 20);
            promise.operation.progress(promise.operation, 99);

            if (completeCalled != "") {
                logger.comment(completeCalled);
                fail("expected no call to on onComplete");
            }
            if (errorCalled != "") {
                logger.comment(errorCalled);
                fail("expected no onError call");
            }
            if (progressCalled != "onProgress(10)onProgress(20)onProgress(99)") {
                logger.comment(progressCalled);
                fail("expected onProgress to be called");
            }
        }
    });

    runner.addTest({
        id: '13-done',
        desc: 'ExceptionFromProgress',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = "";
            var errorCalled = "";
            var progressCalled = "";
            var promise = winery.asyncOperationOut();
            promise
                .done(
                    function (result) { completeCalled = completeCalled + "onComplete(" + result + ")"; },
                    function (result) { errorCalled = errorCalled + "onError(" + result + ")"; },
                    function (percent) { progressCalled = progressCalled + "onProgress(" + percent + ")"; throw "Exception from onProgress"; }
                );

            // Step through some progress.
            promise.operation.progress(promise.operation, 10);
            promise.operation.progress(promise.operation, 20);
            promise.operation.progress(promise.operation, 99);

            if (completeCalled != "") {
                logger.comment(completeCalled);
                fail("expected no call to on onComplete");
            }
            if (errorCalled != "") {
                logger.comment(errorCalled);
                fail("expected no onError call");
            }
            if (progressCalled != "onProgress(10)onProgress(20)onProgress(99)") {
                logger.comment(progressCalled);
                fail("expected onProgress to be called");
            }
        }
    });

    runner.addTest({
        id: 14,
        desc: 'ExamineThen',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();
            var myThen = promise.then();

            for (p in myThen) {
                logger.comment(p);
            }
        }
    });

    runner.addTest({
        id: '14-done',
        desc: 'ExamineDone',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();
            var myDone = promise.then();

            for (p in myDone) {
                logger.comment(p);
            }
        }
    });

    runner.addTest({
        id: 255219,
        desc: 'No Progress Property',
        pri: '0',
        test: function () {
            var oven = new Fabrikam.Kitchen.Oven()
            var async = oven.timerAsync(15);
            async.then(function (result) { });
        }
    });

    runner.addTest({
        id: '255219-done',
        desc: 'No Progress Property',
        pri: '0',
        test: function () {
            var oven = new Fabrikam.Kitchen.Oven()
            var async = oven.timerAsync(15);
            async.done(function (result) { });
        }
    });

    runner.addTest({
        id: 253937,
        desc: 'Winery server ABI issue',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var async = winery.asyncOperationOut();
        }
    });

    runner.addTest({
        id: 15,
        desc: 'ErrorBeforeDoneWithNoErrorHandler',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var immediateException = false;
            var promise = winery.asyncOperationOut();
            var started = promise.then();
            promise.operation.moveToError(); // Simulate asynchronous error
            try {
                promise.done();
            } catch (err) {
                immediateException = true;
            }
            if (immediateException) {
                fail("expected no immediate exception");
            }
        }
    });

    runner.addTest({
        id: 16,
        desc: 'CancelBeforeDoneWithNoErrorHandler',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var immediateException = false;
            var promise = winery.asyncOperationOut();
            var started = promise.cancel();
            try {
                promise.done();
            } catch (err) {
                immediateException = true; // This should not hit for 'cancel'
            }
            if (immediateException) {
                fail("expected no immediate exception for cancel");
            }
        }
    });

    runner.addTest({
        id: 17,
        desc: 'CancelAfterDoneWithNoErrorHandler',
        pri: '0',
        test: function () {
            var originalSetTimeOutCalls = setTimeOutCalls;
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();
            promise.done();
            promise.operation.cancel();
            if (setTimeOutCalls != originalSetTimeOutCalls) {
                fail("Expected setTimeOut to not be called");
            }


        }
    });


    runner.addTest({
        id: 18,
        desc: 'ErrorAfterDoneWithNoErrorHandler',
        pri: '0',
        test: function () {
            var originalSetTimeOutCalls = setTimeOutCalls;
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();
            promise.done();
            promise.operation.moveToError(); // Simulate asynchronous error
            if (setTimeOutCalls != originalSetTimeOutCalls + 1) {
                fail("Expected setTimeOut to be called");
            }
        }
    });

    var errorMessages = {};
    errorMessages.e_outofmemory = "Not enough storage is available to complete this operation.\r\n";

    runner.addTest({
        id: 413280,
        desc: 'Verify that we get an error from a chained promise',
        pri: '0',
        test: function () {

            var winery = new Winery.RWinery(1);
            var errorCalled = false;
            var progressCalled = "";
            var completeCalled = "";
            var promise = winery.asyncOperationOut();

            var chainedWinery = Winery.RWinery(1);
            var chainedPromise = chainedWinery.asyncOperationOut();

            promise
                .then(function (result) {
                    completeCalled = completeCalled + "first ";
                    //chainedPromise.operation.progress(chainedPromise.operation, 10);
                    return chainedPromise;
                })
                .then(function (result) {
                    completeCalled = completeCalled + "second";
                }, function (error) {
                    verify(error.description, errorMessages.e_outofmemory, "Expected E_OUTOFMEMORY error");
                    errorCalled = true;
                }, function (percent) {
                    progressCalled = progressCalled + "onProgress(" + percent + ")";
                    chainedPromise.operation.moveToError(); // Simulate asynchronous error
                });

            promise.operation.moveToCompleted(192); // Complete before the next 'then' is attached. Second should still get called

            chainedPromise.operation.progress(chainedPromise.operation, 10); // Progress through the chained promise


            if (errorCalled != true) {
                logger.comment(errorCalled);
                fail("expected errorCalled for chained promise");
            }
        }
    });

    // Malformed Async Operation tests, starting with id: 20

    errorMessages.noErrorInErrorState = "Status is 'error', but getResults did not return an error";
    errorMessages.invalidStatusArg = "Missing or invalid status parameter passed to completed handler";
    errorMessages.invalidSenderArg = "Missing or invalid sender parameter passed to completed handler";

    runner.addTest({
        id: 20,
        desc: 'GetResultsReturnsErrorOnCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (!(errorResult instanceof WinRTError)) {
                fail("expected WinRTError")
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '20-done',
        desc: 'GetResultsReturnsErrorOnCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (!(errorResult instanceof WinRTError)) {
                fail("expected WinRTError")
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: 21,
        desc: 'CancelReturnsError',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.cancel(); // Simulate asynchronous cancel
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'cancel' to be called");
            }
            if (!(errorResult instanceof WinRTError)) {
                fail("expected WinRTError")
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '21-done',
        desc: 'CancelReturnsError',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.cancel(); // Simulate asynchronous cancel
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'cancel' to be called");
            }
            if (!(errorResult instanceof WinRTError)) {
                fail("expected WinRTError")
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: 22,
        desc: 'GetResultsSucceedsOnError',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.operation.moveToError(); // Simulate asynchronous error
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.name != "AsyncBehaviorError") {
                fail("expected AsyncBehaviorError")
            }
            if (errorResult.message != errorMessages.noErrorInErrorState) {
                logger.comment(errorResult.message);
                fail("incorrect error message");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '22-done',
        desc: 'GetResultsSucceedsOnError',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.operation.moveToError(); // Simulate asynchronous error
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.name != "AsyncBehaviorError") {
                fail("expected AsyncBehaviorError")
            }
            if (errorResult.message != errorMessages.noErrorInErrorState) {
                logger.comment(errorResult.message);
                fail("incorrect error message");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: 23,
        desc: 'InvalidStatusPassedToCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerInvalidStatusArg();
            promise.operation.moveToError(); // Simulate asynchronous error
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.name != "AsyncBehaviorError") {
                fail("expected AsyncBehaviorError")
            }
            if (errorResult.message != errorMessages.invalidStatusArg) {
                logger.comment(errorResult.message);
                fail("incorrect error message");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '23-done',
        desc: 'InvalidStatusPassedToCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerInvalidStatusArg();
            promise.operation.moveToCompleted(23); // Simulate asynchronous completion
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.name != "AsyncBehaviorError") {
                fail("expected AsyncBehaviorError")
            }
            if (errorResult.message != errorMessages.invalidStatusArg) {
                logger.comment(errorResult.message);
                fail("incorrect error message");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: 24,
        desc: 'InvalidSenderPassedToCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .then(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerInvalidSenderArg();
            promise.operation.moveToError(); // Simulate asynchronous error
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.name != "AsyncBehaviorError") {
                fail("expected AsyncBehaviorError")
            }
            if (errorResult.message != errorMessages.invalidSenderArg) {
                logger.comment(errorResult.message);
                fail("incorrect error message");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    runner.addTest({
        id: '24-done',
        desc: 'InvalidSenderPassedToCompleted',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var errorResult = null;
            var promise = winery.asyncOperationOut();
            promise
            .done(function (result) { completeCalled = true; },
                function (error) {
                    errorCalled = true;
                    errorResult = error;
                }
            );
            promise.operation.triggerInvalidSenderArg();
            promise.operation.moveToCompleted(23); // Simulate asynchronous completion
            if (completeCalled) {
                fail("expected no 'then' call");
            }
            if (!errorCalled) {
                fail("expected 'error' to be called");
            }
            if (errorResult.name != "AsyncBehaviorError") {
                fail("expected AsyncBehaviorError")
            }
            if (errorResult.message != errorMessages.invalidSenderArg) {
                logger.comment(errorResult.message);
                fail("incorrect error message");
            }
            if (errorResult.asyncOpType != "Winery.CustomAsyncInfo") {
                logger.comment(errorResult.asyncOpType);
                fail("incorrect type name");
            }
            if (typeof errorResult.asyncOpSource !== 'undefined') {
                logger.comment(errorResult.asyncOpSource);
                fail("asyncOpSource info should not be available when not running under debugger");
            }
        }
    });

    Loader42_FileName = "Promises tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
