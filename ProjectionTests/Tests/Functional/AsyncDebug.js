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
        id: '1',
        desc: 'AsyncOperationOutStatic',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
        }
    });
    
    runner.addTest({
        id: '2',
        desc: 'AsyncOperationOutStaticNotFastPath',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStaticNotFastPath(1, 2, 3);
            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
        }
    });

    runner.addTest({
        id: '3',
        desc: 'AsyncOperationOut',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();
            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion
        }
    });

    runner.addTest({
        id: '4',
        desc: 'AsyncOperationOut-Failure',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOut();

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.triggerIncorrectReturnValues();
            promise.operation.moveToError(); // Simulate asynchronous completion
        }
    });

    runner.addTest({
        id: '5',
        desc: 'AsyncOperationOutAfterExecuteDelegate-Simple',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var promise2 = winery2.asyncOperationOut();

                    promise2
                    .done(
                        function (result) { },
                        function (error) { }
                    );

                    promise2.operation.moveToError(); // status=3
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '6',
        desc: 'AsyncOperationOutAfterExecuteDelegate-Deeper',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var promise2 = winery2.asyncOperationOutAfterExecuteDelegate(
                        function(val) { 
                            var winery3 = new Winery.RWinery(2);
                            var promise3 = winery3.asyncOperationOut();

                            promise3
                            .done(
                                function (result) { },
                                function (error) { }
                            );

                            promise3.operation.moveToError(); // status=3
                        }
                    );

                    promise2
                    .done(
                        function (result) { },
                        function (error) { }
                    );

                    promise2.operation.cancel(); // status=2
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '7',
        desc: 'SimpleDelegateWithAsyncInParameter',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            winery.asyncOperationViaDelegate(
                function(promise) {
                    promise.done( function(result) { completeCalled = true; }, function(error) { errorCalled = true; } );
                    promise.operation.moveToCompleted(192); // Simulate asynchronous completion
                }
            );
            if (completeCalled == false) {
                fail("expected completeCalled")
            }
            if (errorCalled == true) {
                fail("expected !errorCalled")
            }
        }
    });

    runner.addTest({
        id: '8',
        desc: 'SimpleDelegateWithAsyncInParameter-Nested',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            function errorHandler(error) { errorCalled = true; }
            function completeHandler(result) { completeCalled = true; }
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var promise2 = winery2.asyncOperationOutAfterExecuteDelegate(
                        function(val) { 
                            var winery3 = new Winery.RWinery(2);
                            winery3.asyncOperationViaDelegate(
                                function(promise3) {
                                    promise3.done( completeHandler, errorHandler );
                                    promise3.operation.moveToError(); // status=3
                                }
                            );
                            if (completeCalled == true) {
                                fail("!completeCalled")
                            }
                            if (errorCalled == false) {
                                fail("errorCalled")
                            }
                        }
                    );

                    promise2
                    .done(
                        function (result) { },
                        function (error) { }
                    );

                    promise2.operation.cancel(); // status=2
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '9',
        desc: 'AsyncOperationOutAfterExecuteDelegateWithAsyncInParameter',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var promise2 = winery2.asyncOperationOutAfterExecuteDelegateWithAsyncInParameter(
                        function(promise3) {
                            promise3.done( function(result) { completeCalled = true; }, function(error) { errorCalled = true; } );
                            promise3.operation.moveToError(); // status=3
                        }
                    );

                    promise2
                    .done(
                        function (result) { completeCalled = true; },
                        function (error) { errorCalled = true; }
                    );

                    promise2.operation.cancel(); // status=2
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '10',
        desc: 'AsyncOperationOutAfterExecuteDelegateWithAsyncInParameterUseSameAsyncObject',
        pri: '0',
        test: function () {
            // For test #12 able to run in wwahost, capture the unhandled exception
            if (Utils.isWWAHost()) {
                var old_onerr = window.onerror;
                window.onerror = function (e) {
                    logger.comment(e);
                    window.onerror = old_onerr;
                    return true;
                }
            }

            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var promise2 = winery2.asyncOperationOutAfterExecuteDelegateWithAsyncInParameterUseSameAsyncObject(
                        function(promise3) {
                            promise3.done( function(result) { completeCalled = true; }, function(error) { errorCalled = true; } );
                            promise3.operation.moveToError(); // status=3
                        }
                    );

                    promise2
                    .done(
                        function (result) { completeCalled = true; },
                        function (error) { errorCalled = true; }
                    );

                    // This will cause a JS runtime error - expected.
                    promise2.operation.cancel(); // status=2
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );

            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '11',
        desc: 'AsyncOperationWithMultipleAsyncOutParameters',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var promise2and3 = winery2.asyncOperationWithMultipleAsyncOutParameters();

                    var promise2 = promise2and3['asyncOp1'];
                    promise2.done(
                        function (result) { completeCalled = true; },
                        function (error) { errorCalled = true; }
                    );

                    promise2.operation.moveToError(); // status=3

                    var promise3 = promise2and3['asyncOp2'];
                    promise3.done(
                        function (result) { completeCalled = true; },
                        function (error) { errorCalled = true; }
                    );

                    promise3.operation.cancel(); // status=2
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '12',
        desc: 'AsyncOperationWithMultipleOutParameters-Delegate',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;
            var promise = winery.asyncOperationOutAfterExecuteDelegate(
                function(val) { 
                    var winery2 = new Winery.RWinery(2);
                    var outparams = winery2.asyncOperationWithMultipleOutParameters();

                    var promise2 = outparams['asyncOp'];
                    promise2.done(
                        function (result) { completeCalled = true; },
                        function (error) { errorCalled = true; }
                    );
                    promise2.operation.moveToError(); // status=3
                }
            );

            promise
            .done(
                function (result) { },
                function (error) { }
            );
            promise.operation.moveToCompleted(192); // status=1
        }
    });

    runner.addTest({
        id: '13',
        desc: 'AsyncOperationWithMultipleOutParameters',
        pri: '0',
        test: function () {
            var winery = new Winery.RWinery(1);
            var completeCalled = false;
            var errorCalled = false;

            var winery2 = new Winery.RWinery(2);
            var outparams = winery2.asyncOperationWithMultipleOutParameters();

            var promise2 = outparams['asyncOp'];
            promise2.done(
                function (result) { completeCalled = true; },
                function (error) { errorCalled = true; }
            );
            promise2.operation.moveToError(); // status=3

        }
    });

    runner.addTest({
        id: '14',
        desc: 'AsyncOperationCompleteAfterCallbacksAdded',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise.then(
                function(result) { then1success = true; },
                function(error) { then1error = true; });
            promise.then(
                function(result) { then2success = true; },
                function(error) { then2error = true; });
            promise.done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion

            if (then1error == true || then2error == true || doneerror == true) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == false || donesuccess == false) {
                fail("success handlers not called")
            }
        }
    });

    runner.addTest({
        id: '15',
        desc: 'AsyncOperationCompleteBeforeCallbacksAdded',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; },
                function(error) { then1error = true; });

            promise.operation.moveToCompleted(192); // Simulate asynchronous completion

            promise.then(
                function(result) { then2success = true; },
                function(error) { then2error = true; });
            promise.done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });

            if (then1error == true || then2error == true || doneerror == true) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == false || donesuccess == false) {
                fail("success handlers not called")
            }
        }
    });

    runner.addTest({
        id: '16',
        desc: 'AsyncOperationCompleteBeforeCallbacksAdded-Chained',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise.then(
                function(result) { then1success = true; },
                function(error) { then1error = true; })
            .then(
                function(result) { then2success = true; },
                function(error) { then2error = true; })
            .done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });
            promise.operation.moveToCompleted(192); // Simulate asynchronous completion

            if (then1error == true || then2error == true || doneerror == true) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == false || donesuccess == false) {
                fail("success handlers not called")
            }
        }
    });

    runner.addTest({
        id: '17',
        desc: 'AsyncOperationCompleteBeforeCallbacksAdded-Chained',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            var chain_promise = promise.then(
                function(result) { then1success = true; },
                function(error) { then1error = true; });

            promise.operation.moveToCompleted(192); // Simulate asynchronous completion

            chain_promise.then(
                function(result) { then2success = true; },
                function(error) { then2error = true; })
            .done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });

            if (then1error == true || then2error == true || doneerror == true) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == false || donesuccess == false) {
                fail("success handlers not called")
            }
        }
    });

    runner.addTest({
        id: '18',
        desc: 'AsyncOperationCancel',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; },
                function(error) { then1error = true; });
            promise.then(
                function(result) { then2success = true; },
                function(error) { then2error = true; });
            promise.done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });

            promise.cancel();

            logger.comment('errors: ' + then1error + ' ' + then2error + ' ' + doneerror);
            logger.comment('successes: ' + then1success + ' ' + then2success + ' ' + donesuccess);

            if (then1error == false || then2error == false || doneerror == false) {
                fail("error handlers not called")
            }
            if (then1success == true || then2success == true || donesuccess == true) {
                fail("success handlers called")
            }
        }
    });

    runner.addTest({
        id: '19',
        desc: 'AsyncOperationErrorInSuccessCallback',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; eval('0 = 0'); },
                function(error) { then1error = true; })
            .then(
                function(result) { then2success = true; },
                function(error) { then2error = true; });
            promise.done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });

            promise.operation.moveToCompleted(192);

            logger.comment('errors: ' + then1error + ' ' + then2error + ' ' + doneerror);
            logger.comment('successes: ' + then1success + ' ' + then2success + ' ' + donesuccess);

            if (then1error == true || then2error == false || doneerror == true) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == true || donesuccess == false) {
                fail("success handlers called")
            }
        }
    });

    runner.addTest({
        id: '20',
        desc: 'AsyncOperationErrorInSuccessAndErrorCallbacks',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; eval('0 = 0'); },
                function(error) { then1error = true; })
            .then(
                function(result) { then2success = true; },
                function(error) { then2error = true; eval('0 = 0'); })
            .done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });

            promise.operation.moveToCompleted(192);

            logger.comment('errors: ' + then1error + ' ' + then2error + ' ' + doneerror);
            logger.comment('successes: ' + then1success + ' ' + then2success + ' ' + donesuccess);

            if (then1error == true || then2error == false || doneerror == false) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == true || donesuccess == true) {
                fail("success handlers called")
            }
        }
    });

    runner.addTest({
        id: '21',
        desc: 'AsyncOperationErrorInErrorCallbackWithDone',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; eval('0 = 0'); },
                function(error) { then1error = true; eval('0 = 0'); })
            .then(
                function(result) { then2success = true; eval('0 = 0'); },
                function(error) { then2error = true; eval('0 = 0'); });

            promise.operation.moveToError();

            promise.done(
                function(result) { donesuccess = true; },
                function(error) { doneerror = true; });

            logger.comment('errors: ' + then1error + ' ' + then2error + ' ' + doneerror);
            logger.comment('successes: ' + then1success + ' ' + then2success + ' ' + donesuccess);

            if (then1error == false || then2error == false || doneerror == false) {
                fail("error handlers not called")
            }
            if (then1success == true || then2success == true || donesuccess == true) {
                fail("success handlers called")
            }
        }
    });

    runner.addTest({
        id: '22',
        desc: 'AsyncOperationErrorInErrorCallbackWithErrorInDone',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; eval('0 = 0'); },
                function(error) { then1error = true; eval('0 = 0'); })
            .then(
                function(result) { then2success = true; eval('0 = 0'); },
                function(error) { then2error = true; eval('0 = 0'); });

            promise.operation.moveToError();

            try {
                promise.done(
                    function(result) { donesuccess = true; eval('0 = 0'); },
                    function(error) { doneerror = true; eval('0 = 0'); });
                fail("expected exception");
            } catch(e) {
                verify.instanceOf(e, Error);
            }

            logger.comment('errors: ' + then1error + ' ' + then2error + ' ' + doneerror);
            logger.comment('successes: ' + then1success + ' ' + then2success + ' ' + donesuccess);

            if (then1error == false || then2error == false || doneerror == false) {
                fail("error handlers not called")
            }
            if (then1success == true || then2success == true || donesuccess == true) {
                fail("success handlers called")
            }
        }
    });

    runner.addTest({
        id: '23',
        desc: 'AsyncOperationMalformedSuccessCallbackReturn',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var then2success = false;
            var then2error = false;
            var donesuccess = false;
            var doneerror = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; return { then : function() { then2success = true; eval('0 = 0'); }, somethingelse : undefined }; },
                function(error) { then1error = true; return { then : function() { then2error = true; eval('0 = 0'); }, somethingelse : undefined }; });

            promise.operation.moveToCompleted(192);

            logger.comment('errors: ' + then1error + ' ' + then2error);
            logger.comment('successes: ' + then1success + ' ' + then2success);

            if (then1error == true || then2error == true) {
                fail("error handlers called")
            }
            if (then1success == false || then2success == false) {
                fail("success handlers not called")
            }
        }
    });

    runner.addTest({
        id: '24',
        desc: 'AsyncOperationMultipleCompletionCallbacksDoesNotCauseMultipleETWEvents',
        pri: '0',
        test: function () {
            var then1success = false;
            var then1error = false;
            var promise = Winery.RWinery.asyncOperationOutStatic();

            promise.then(
                function(result) { then1success = true; },
                function(error) { then1error = true; });

            promise.operation.completed();

            logger.comment('errors: ' + then1error);
            logger.comment('successes: ' + then1success);

            if (then1error == false) {
                fail("error handlers not called")
            }
            if (then1success == true) {
                fail("success handlers called")
            }

            promise.operation.completed();
        }
    });
    
    runner.addTest({
        id: '25',
        desc: 'ChainedPromiseWithSecondLevelPromiseInternalThen',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            logger.comment('calling chained promise');
            promise
            .then(function () { 
                logger.comment('inside first-level then function');
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                promise2.then(function () {
                   logger.comment('inside second-level then function');
                });
            }
            );

            promise.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '26',
        desc: 'ChainedPromiseWithSecondLevelPromiseReturnedFromThen',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var thenPromise = promise
            .then(function () { 
                logger.comment('inside first-level then function');
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                return promise2;
            }
            );
            var thenPromise2 = thenPromise.then(function () {
                logger.comment('inside second-level then function');
            })
            ;

            promise.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '27',
        desc: 'ChainedPromiseWithManyLevels',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var promise3;
            var promise4;
            promise
            .then(function () { 
                logger.comment('inside first-level then function');
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                return promise2;
            })
            .then(function () {
                logger.comment('inside second-level then function');
                promise3 = Winery.RWinery.asyncOperationOutStatic();
                return promise3;
            })
            .then(function () {
                logger.comment('inside third-level then function');
                promise4 = Winery.RWinery.asyncOperationOutStatic();
                return promise4;
            })
            .then(function () {
                logger.comment('inside fourth-level then function');
            })
            ;

            promise.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
            promise3.operation.moveToCompleted(192);
            promise4.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '28',
        desc: 'ChainedPromiseWithNestedPromises',
        pri: '0',
        test: function () {
            logger.comment('calling chained/nested promise');
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var promise1a;
            var promise1b;
            var promise2;
            var promise2a;
            var promise3;
            var promise3a;
            var promise3b;
            var promise3c;
            var promise4;
            promise1
            .then(function () { 
                logger.comment('inside promise1.then');
                promise1a = Winery.RWinery.asyncOperationOutStatic();
                promise1a
                .then(function() {
                    logger.comment('inside promise1a.then');
                    promise1b = Winery.RWinery.asyncOperationOutStatic();
                    return promise1b;
                })
                .then(function() {
                    logger.comment('inside promise1a.promise1b.then');
                });
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                return promise2;
            })
            .then(function () {
                logger.comment('inside promise1.promise2.then');
                promise2a = Winery.RWinery.asyncOperationOutStatic();
                promise2a
                .then(function() {
                    logger.comment('inside promise2a.then');
                });
                promise3 = Winery.RWinery.asyncOperationOutStatic();
                return promise3;
            })
            .then(function () {
                logger.comment('inside promise1.promise2.promise3 then');
                promise3a = Winery.RWinery.asyncOperationOutStatic();
                promise3a
                .then(function() {
                    logger.comment('inside promise3a.then');
                    promise3b = Winery.RWinery.asyncOperationOutStatic();
                    return promise3b;
                })
                .then(function() {
                    logger.comment('inside promise3a.promise3b.then');
                    promise3c = Winery.RWinery.asyncOperationOutStatic();
                    return promise3c;
                })
                .then(function() {
                    logger.comment('inside promise3a.promise3b.promise3c.then');
                });
                promise4 = Winery.RWinery.asyncOperationOutStatic();
                return promise4;
            })
            .then(function () {
                logger.comment('inside promise1.promise2.promise3.promise4 then');
            })
            ;

            promise1.operation.moveToCompleted(192);
            promise1a.operation.moveToCompleted(192);
            promise1b.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
            promise2a.operation.moveToCompleted(192);
            promise3.operation.moveToCompleted(192);
            promise3a.operation.moveToCompleted(192);
            promise3b.operation.moveToCompleted(192);
            promise3c.operation.moveToCompleted(192);
            promise4.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '29',
        desc: 'ChainedSinglePromise',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(function () { 
                logger.comment('inside first-level then function');
            })
            .then(function () {
                logger.comment('inside second-level then function');
            })
            .then(function () {
                logger.comment('inside third-level then function');
            })
            .then(function () {
                logger.comment('inside fourth-level then function');
            })
            ;

            promise.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '30',
        desc: 'ChainedPromiseReturningThenPromise',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var promise1a;
            var promise1b;
            var promise2a;
            var promise3;
            var promise3b;
            var promise1aThenPromise;
            var promise3ThenPromise;
            var promise1ThenPromise;
            var promise1ThenThenPromise;
            var promise1ThenThenThenPromise;
            var promise1ThenThenThenThenPromise;
            promise1ThenPromise = promise1
            .then(function () { 
                logger.comment('inside promise1.then');
                promise1a = Winery.RWinery.asyncOperationOutStatic();
                promise1aThenPromise = promise1a
                .then(function() {
                    logger.comment('inside promise1a.then');
                    promise1b = Winery.RWinery.asyncOperationOutStatic();
                    return promise1b;
                });
                promise1aThenPromise
                .then(function() {
                    logger.comment('inside promise1a.promise1b.then');
                });
            });
            promise1ThenThenPromise = promise1ThenPromise.then(function () {
                logger.comment('inside promise1.thenPromise.then');
                promise2a = Winery.RWinery.asyncOperationOutStatic();
                promise2a
                .then(function() {
                    logger.comment('inside promise2a.then');
                });
            });
            promise1ThenThenThenPromise = promise1ThenThenPromise.then(function () {
                logger.comment('inside promise1.then - second one');
                promise3 = Winery.RWinery.asyncOperationOutStatic();
                promise3ThenPromise = promise3
                .then(function() {
                    logger.comment('inside promise3.then');
                    promise3b = Winery.RWinery.asyncOperationOutStatic();
                    return promise3b;
                });
                return promise3ThenPromise;
            });
            promise1ThenThenThenThenPromise = promise1ThenThenThenPromise.then(function () {
                logger.comment('inside promise3b.then');
            })
            ;

            promise1.operation.moveToCompleted(192);
            promise1a.operation.moveToCompleted(192);
            promise1b.operation.moveToCompleted(192);
            promise2a.operation.moveToCompleted(192);
            promise3.operation.moveToCompleted(192);
            promise3b.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '31',
        desc: 'ChainedPromiseWithMultipleCompleteCallbacks',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var promise3;
            var promise4;
            var promise5;
            promise3 = promise1
            .then(function () { 
                logger.comment('inside promise1.then');
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                return promise2;
            });
            promise1.then(function () {
                logger.comment('inside second promise1.then');
            });
            promise3.then(function () {
                logger.comment('inside promise3.then');
            });
            promise4 = promise3.then(function () {
                logger.comment('inside second promise3.then');
                promise5 = Winery.RWinery.asyncOperationOutStatic();
                return promise5.then(function () {
                    logger.comment('inside promise5.then');
                });
            });
            promise4.then(function () {
                logger.comment('inside promise4.then');
            })
            ;
            
            promise1.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
            promise5.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '32',
        desc: 'TwoLevelChainPromise',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var parentPromise = Winery.RWinery.asyncOperationOutStatic();
            var parentPromiseThenPromise;
            var childPromise;
            var childPromiseThenPromise;
            var childPromiseChildPromise;
            parentPromiseThenPromise = parentPromise
            .then(function () { 
                logger.comment('inside parentPromise.then');
                childPromise = Winery.RWinery.asyncOperationOutStatic();
                childPromiseThenPromise = childPromise.then(function () {
                    logger.comment('inside childPromise.then');
                    childPromiseChildPromise = Winery.RWinery.asyncOperationOutStatic();
                    return childPromiseChildPromise;
                });
                return childPromiseThenPromise;
            });
            parentPromiseThenPromiseThenPromise = parentPromiseThenPromise.then(function () {
                logger.comment('inside parentPromiseThenPromise.then');
            });
            
            parentPromise.operation.moveToCompleted(192);
            childPromise.operation.moveToCompleted(192);
            childPromiseChildPromise.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '33',
        desc: 'TwoLevelChainPromiseReusePromiseAfterComplete',
        pri: '0',
        test: function () {
            logger.comment('calling chained promise');
            var parentPromise = Winery.RWinery.asyncOperationOutStatic();
            var parentPromiseThenPromise;
            var childPromise;
            var childPromiseThenPromise;
            var childPromiseChildPromise;
            parentPromiseThenPromise = parentPromise
            .then(function () { 
                logger.comment('inside parentPromise.then');
                childPromise = Winery.RWinery.asyncOperationOutStatic();
                childPromiseThenPromise = childPromise.then(function () {
                    logger.comment('inside childPromise.then');
                    childPromiseChildPromise = Winery.RWinery.asyncOperationOutStatic();
                    return childPromiseChildPromise;
                });
                return childPromiseThenPromise;
            });
            parentPromiseThenPromiseThenPromise = parentPromiseThenPromise.then(function () {
                logger.comment('inside parentPromiseThenPromise.then');
            });
            
            parentPromise.operation.moveToCompleted(192);
            childPromise.operation.moveToCompleted(192);
            childPromiseChildPromise.operation.moveToCompleted(192);
            
            var latentParentPromiseThenPromise = parentPromise
            .then(function () {
                logger.comment('inside latentParentPromiseThenPromise.then');
            });
        }
    });
    
    runner.addTest({
        id: '39',
        desc: 'SimpleProgressHandler',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler: ' + progress + '%');
                }
            );
            
            for (i = 1; i < 10; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '41',
        desc: 'SimpleNestedProgressHandlers',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler 1: ' + progress + '%');
                }
            ).then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler 2: ' + progress + '%');
                }
            )
            ;
            
            for (i = 1; i < 10; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '42',
        desc: 'SimpleProgressHandlerInteruptedByCompletion',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler: ' + progress + '%');
                }
            );
            
            for (i = 1; i < 5; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToCompleted(192);

            try {
                promise.operation.triggerProgress(50);
                fail('Succeeded to trigger progress after operation completion.');
            } catch(e) {
                logger.comment('Failed to trigger progress after operation completion.');
            }
        }
    });
    
    runner.addTest({
        id: '43',
        desc: 'SimpleProgressHandlerAfterCompletion',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler: ' + progress + '%');
                }
            );
            
            for (i = 1; i < 5; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToCompleted(192);

            try {
                promise.operation.allowProgressCalledAfterCompletion();
                promise.operation.triggerProgress(50);
                logger.comment('Succeeded to trigger progress after operation completion.');
            } catch(e) {
                fail('Failed to trigger progress after operation completion.');
            }
        }
    });
    
    runner.addTest({
        id: '44',
        desc: 'SimpleProgressHandlerInteruptedByError',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler: ' + progress + '%');
                }
            );
            
            for (i = 1; i < 5; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToError();

            try {
                promise.operation.triggerProgress(50);
                fail('Succeeded to trigger progress after operation error.');
            } catch(e) {
                logger.comment('Failed to trigger progress after operation error.');
            }
        }
    });
    
    runner.addTest({
        id: '45',
        desc: 'SimpleProgressHandlerAfterError',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler: ' + progress + '%');
                }
            );
            
            for (i = 1; i < 5; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToError();

            try {
                promise.operation.allowProgressCalledAfterCompletion();
                promise.operation.triggerProgress(50);
                logger.comment('Succeeded to trigger progress after operation error.');
            } catch(e) {
                fail('Failed to trigger progress after operation error.');
            }
        }
    });

    runner.addTest({
        id: '46',
        desc: 'SimpleNonNestedProgressHandlers',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler 1: ' + progress + '%');
                }
            );
            promise
            .then(
                function (result) { },
                function (error) { },
                function (progress) {
                    logger.comment('Progress handler 2: ' + progress + '%');
                }
            )
            ;
            
            for (i = 1; i < 10; i++) {
                promise.operation.triggerProgress(i*10);
            }
            
            promise.operation.moveToCompleted(192);
        }
    });

    Loader42_FileName = "AsyncDebug tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
