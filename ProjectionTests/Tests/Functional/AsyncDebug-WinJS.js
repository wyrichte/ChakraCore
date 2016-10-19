if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var setTimeOutCalls = 0;
    var savedSetTimeout = undefined;
    if (typeof setTimeout !== 'undefined')
        savedSetTimeout = setTimeout;
    runner.globalSetup(function () {
        setTimeout = function () { setTimeOutCalls = setTimeOutCalls + 1; }
        if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile(".\\WinJS.10.base.js"); } 
    });
    runner.globalTeardown(function () {
        setTimeout = savedSetTimeout;
    });

    runner.addTest({
        id: '34',
        desc: 'SimpleWinJSPromise',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;

            promise.then(
            function() {
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                return WinJS.Promise.wrap(promise2);
            }
            ).then(
            function() {
                logger.comment('inside winjs-wrapped.then');
            }
            );

            promise.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '35',
        desc: 'ChainedWinJSPromise',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var promise3;
            var promise4;
            var winjspromise;

            promise.then(
            function() {
                logger.comment('promise.then');
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then');
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            }
            ).then(
            function() {
                logger.comment('latent.then');
            }
            );

            promise.operation.moveToCompleted(192);
            promise2.operation.moveToCompleted(192);
            promise4.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '36',
        desc: 'ChainedWinJSPromiseWithErrorHandlers',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var promise3;
            var promise4;
            var winjspromise;
            var error1Called = false;
            var complete1Called = false;
            var error2Called = false;
            var complete2Called = false;
            var error3Called = false;
            var complete3Called = false;

            promise.then(
            function() {
                logger.comment('promise.then(complete)');
                complete1Called = true;
                
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then(complete)');
                    complete2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                },
                function() { 
                    logger.comment('winjspromise.then(error)');
                    error2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            },
            function() {
                logger.comment('promise.then(error)');
                error1Called = true;
                
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then(complete)');
                    complete2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                },
                function() { 
                    logger.comment('winjspromise.then(error)');
                    error2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            }
            ).then(
            function() {
                logger.comment('latent.then(complete)');
                complete3Called = true;
            },
            function() {
                logger.comment('latent.then(error)');
                error3Called = true;
            }
            );

            promise.operation.moveToError();
            promise2.operation.moveToError();
            promise4.operation.moveToError();
            
            logger.comment('Success handlers: ' + complete1Called + ' ' + complete2Called + ' ' + complete3Called);
            logger.comment('Error handlers: ' + error1Called + ' ' + error2Called + ' ' + error3Called);
            
            if (complete1Called == true || complete2Called == true || complete3Called == true) {
                fail("success handlers called")
            }
            if (error1Called == false || error2Called == false || error3Called == false) {
                fail("error handlers not called")
            }
        }
    });
    
    runner.addTest({
        id: '37',
        desc: 'ChainedWinJSPromiseWithErrorHandlers2',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var promise3;
            var promise4;
            var winjspromise;
            var error1Called = false;
            var complete1Called = false;
            var error2Called = false;
            var complete2Called = false;
            var error3Called = false;
            var complete3Called = false;

            promise.then(
            function() {
                logger.comment('promise.then(complete)');
                complete1Called = true;
                
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then(complete)');
                    complete2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                },
                function() { 
                    logger.comment('winjspromise.then(error)');
                    error2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            },
            function() {
                logger.comment('promise.then(error)');
                error1Called = true;
                
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then(complete)');
                    complete2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                },
                function() { 
                    logger.comment('winjspromise.then(error)');
                    error2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            }
            ).then(
            function() {
                logger.comment('latent.then(complete)');
                complete3Called = true;
            },
            function() {
                logger.comment('latent.then(error)');
                error3Called = true;
            }
            );

            promise.operation.moveToError();
            promise2.operation.moveToCompleted(1);
            promise4.operation.moveToError();
            
            logger.comment('Success handlers: ' + complete1Called + ' ' + complete2Called + ' ' + complete3Called);
            logger.comment('Error handlers: ' + error1Called + ' ' + error2Called + ' ' + error3Called);
            
            if (complete1Called == true || complete2Called == false || complete3Called == true) {
                fail("success handlers called")
            }
            if (error1Called == false || error2Called == true || error3Called == false) {
                fail("error handlers not called")
            }
        }
    });
    
    runner.addTest({
        id: '38',
        desc: 'ChainedWinJSPromiseWithErrorHandlers3',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            var promise2;
            var promise3;
            var promise4;
            var winjspromise;
            var error1Called = false;
            var complete1Called = false;
            var error2Called = false;
            var complete2Called = false;
            var error3Called = false;
            var complete3Called = false;

            promise.then(
            function() {
                logger.comment('promise.then(complete)');
                complete1Called = true;
                
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then(complete)');
                    complete2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                },
                function() { 
                    logger.comment('winjspromise.then(error)');
                    error2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            },
            function() {
                logger.comment('promise.then(error)');
                error1Called = true;
                
                promise2 = Winery.RWinery.asyncOperationOutStatic();
                winjspromise = WinJS.Promise.wrap(promise2);

                promise3 = winjspromise.then(
                function() {
                    logger.comment('winjspromise.then(complete)');
                    complete2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                },
                function() { 
                    logger.comment('winjspromise.then(error)');
                    error2Called = true;
                    
                    promise4 = Winery.RWinery.asyncOperationOutStatic();
                    return promise4;
                }
                );
                
                return promise3;
            }
            ).then(
            function() {
                logger.comment('latent.then(complete)');
                complete3Called = true;
            },
            function() {
                logger.comment('latent.then(error)');
                error3Called = true;
            }
            );

            promise.operation.moveToCompleted(1);
            promise2.operation.moveToError();
            promise4.operation.moveToCompleted(1);
            
            logger.comment('Success handlers: ' + complete1Called + ' ' + complete2Called + ' ' + complete3Called);
            logger.comment('Error handlers: ' + error1Called + ' ' + error2Called + ' ' + error3Called);
            
            if (complete1Called == false || complete2Called == true || complete3Called == false) {
                fail("success handlers called")
            }
            if (error1Called == true || error2Called == false || error3Called == true) {
                fail("error handlers not called")
            }
        }
    });
    
    runner.addTest({
        id: '47',
        desc: 'WinJS.Promise.Join',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var promise2 = Winery.RWinery.asyncOperationOutStatic();
            var promise3;
            var winjspromise = WinJS.Promise.join( [ promise1, promise2 ] );
            
            promise1.then(function() { logger.comment('promise1.then'); });
            promise2.then(function() { logger.comment('promise2.then'); });

            winjspromise.then(
            function() {
                logger.comment('winjspromise.then');
                promise3 = Winery.RWinery.asyncOperationOutStatic();
                return promise3;
            }
            ).then(
            function() {
                logger.comment('promise3.then');
            }
            );

            logger.comment('promise1 completing...');
            promise1.operation.moveToCompleted(192);
            logger.comment('promise2 completing...');
            promise2.operation.moveToCompleted(192);
            logger.comment('promise3 completing...');
            promise3.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '48',
        desc: 'WinJS.Promise.As',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var winjspromise = WinJS.Promise.as(promise1);
            
            promise1.then(function() { logger.comment('promise1.then'); });
            winjspromise.then(function() { logger.comment('winjspromise.then'); });

            logger.comment('winjspromise completing...');
            winjspromise.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '49',
        desc: 'WinJS.Promise.Any',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var promise2 = Winery.RWinery.asyncOperationOutStatic();
            var promise3;
            var winjspromise = WinJS.Promise.any( [ promise1, promise2 ] );
            
            promise1.then(function() { logger.comment('promise1.then'); });
            promise2.then(function() { logger.comment('promise2.then'); });

            winjspromise.then(
            function() {
                logger.comment('winjspromise.then');
                promise3 = Winery.RWinery.asyncOperationOutStatic();
                return promise3;
            }
            ).then(
            function() {
                logger.comment('promise3.then');
            }
            );

            logger.comment('promise1 completing...');
            promise1.operation.moveToCompleted(192);
            logger.comment('promise3 completing...');
            promise3.operation.moveToCompleted(192);
            logger.comment('promise2 completing...');
            promise2.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '50',
        desc: 'WinJS.Promise.Any.2',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var promise2 = Winery.RWinery.asyncOperationOutStatic();
            var promise3;
            var winjspromise = WinJS.Promise.any( [ promise1, promise2 ] );
            
            promise1.then(function() { logger.comment('promise1.then'); });
            promise2.then(function() { logger.comment('promise2.then'); });

            winjspromise.then(
            function() {
                logger.comment('winjspromise.then');
                promise3 = Winery.RWinery.asyncOperationOutStatic();
                return promise3;
            }
            ).then(
            function() {
                logger.comment('promise3.then');
            }
            );

            logger.comment('promise2 completing...');
            promise2.operation.moveToCompleted(192);
            logger.comment('promise1 completing...');
            promise1.operation.moveToCompleted(192);
            logger.comment('promise3 completing...');
            promise3.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '51',
        desc: 'WinJS.Promise.Any.3',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var winjspromise = WinJS.Promise.any( [promise1] );
            
            promise1.then(function() { logger.comment('promise1.then'); });

            winjspromise.then(
            function() {
                logger.comment('winjspromise.then');
            }
            );

            logger.comment('promise1 completing...');
            promise1.operation.moveToCompleted(192);
        }
    });
    
    runner.addTest({
        id: '52',
        desc: 'WinJS.Promise.Cancel',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            
            promise1.then(
            function() { logger.comment('promise1.then (Complete)'); },
            function() { logger.comment('promise1.then (Error)'); }
            );
                        
            var winjspromise = WinJS.Promise.wrap(promise1);
            
            winjspromise.cancel();
        }
    });
    
    runner.addTest({
        id: '53',
        desc: 'WinJS.Promise.Cancel.2',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            var winjspromise = WinJS.Promise.wrap(promise1);
            
            winjspromise.then(
            function() { logger.comment('promise1.then (Complete)'); },
            function() { logger.comment('promise1.then (Error)'); }
            );
            
            promise1.cancel();
        }
    });
    
    runner.addTest({
        id: '54',
        desc: 'WinJS.Promise.Is',
        pri: '0',
        test: function () {
            var promise1 = Winery.RWinery.asyncOperationOutStatic();
            verify(WinJS.Promise.is(promise1), true, 'WinJS.Promise.is(promise1)');
        }
    });

    Loader42_FileName = "AsyncDebug-WinJS tests";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
