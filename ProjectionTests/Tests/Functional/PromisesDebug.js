/**exception(nonusercode):stack()**/

// This test can't be executed by runner since the non-user code exceptions flag is not honored.
function test0() {
    if (typeof WScript === 'undefined') {
        return;
    }
    
    WScript.InitializeProjection();
    
    var testname = "Test 0: BLUE:454536 - Non-user code exceptions are not swallowed by promise code.";
    
    WScript.Echo("=================================================\nStarting [" + testname + "]");

    var promise = Winery.RWinery.asyncOperationOutStatic();
   
    try {
    var thenpromise = promise.then(
        function(results) {
            WScript.Echo("about to throw exception in completion handler");
            results.blahblah(); // we should break here instead of promise library code catching exception
        }
    );
    } catch(e) {
        WScript.Echo(e);
    }

    promise.operation.moveToCompleted(0);
    
    WScript.Echo("PASS [" + testname + "]\n=================================================\n");
}
test0();

if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    var setTimeOutCalls = 0;
    var savedSetTimeout = undefined;
    if (typeof setTimeout !== 'undefined')
        savedSetTimeout = setTimeout;
    runner.globalSetup(function () {
        setTimeout = function () { setTimeOutCalls = setTimeOutCalls + 1; }
        if (typeof window === 'undefined') { 
            window = {};
        }
        window.setTimeout = setTimeout;
    });
    runner.globalTeardown(function () {
        setTimeout = savedSetTimeout;
        window.setTimeout = savedSetTimeout;
    });

    runner.addTest({
        id: 1,
        desc: 'Missing window.setTimeout',
        pri: '0',
        test: function () {
            var temp = window.setTimeout;
            window.setTimeout = undefined;
            
            var winery = new Winery.RWinery(1);
            var immediateException = false;
            var promise = winery.asyncOperationOut();
            var started = promise.then();
            promise.operation.moveToError(); // Simulate asynchronous error
            try {
                promise.done();
            } catch (err) {
                immediateException = true;
                logger.comment("error: " + err.message + "\n\t" + err.stack);
            }
            if (immediateException) {
                fail("expected no immediate exception");
            }
            
            window.setTimeout = temp;
        }
    });

    runner.addTest({
        id: 2,
        desc: 'Missing window',
        pri: '0',
        test: function () {
            var temp = window;
            window = undefined;
            
            var winery = new Winery.RWinery(1);
            var immediateException = false;
            var promise = winery.asyncOperationOut();
            var started = promise.then();
            promise.operation.moveToError(); // Simulate asynchronous error
            try {
                promise.done();
            } catch (err) {
                immediateException = true;
                logger.comment("error: " + err.message + "\n\t" + err.stack);
            }
            if (immediateException) {
                fail("expected no immediate exception");
            }
            
            window = temp;
        }
    });
    
     var _debug;
     var _error;
     var _object;
     var _eval;
     var _window;
     
     function removeGlobals(replacement) {
          _debug = Debug;
          Debug = replacement;
          _error = Error;
          Error = replacement;
          _object = Object;
          Object = replacement;
          _eval = eval;
          eval = replacement;
          _window = window;
          window = replacement;
     }
     
     function resetGlobals() {
          Debug = _debug;
          Error = _error;
          Object = _object;
          eval = _eval;
          window = _window;
     }

    runner.addTest({
        id: 3,
        desc: 'Missing Debug - successfully completed',
        pri: '0',
        test: function () {
               var temp = Debug;
               Debug = undefined;
               
            var promise = Winery.RWinery.asyncOperationOutStatic();
               var promise2;
               
               var thenpromise = promise.then(
                    function() {
                         logger.comment("outer promise then handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                         
                         return promise2;
                    },
                    function (e) {
                         logger.comment("outer promise error handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                    },
                    function (p) {
                         logger.comment("outer promise progress handler.");
                    }
               );
               
               promise.operation.moveToCompleted(0);
               promise2.operation.moveToCompleted(0);
               
               promise.done(
                    function() {
                         logger.comment("outer promise final done success handler");
                    },
                    function() {
                         logger.comment("outer promise final done error handler");
                    }
               );
               promise2.done(
                    function() {
                         logger.comment("inner promise final done success handler");
                    },
                    function() {
                         logger.comment("inner promise final done error handler");
                    }
               );
               
               Debug = temp;
        }
    });
     
     runner.addTest({
        id: 4,
        desc: 'Missing Debug - error',
        pri: '0',
        test: function () {
               var temp = Debug;
               Debug = undefined;
               
            var promise = Winery.RWinery.asyncOperationOutStatic();
               var promise2;
               
               var thenpromise = promise.then(
                    function() {
                         logger.comment("outer promise then handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                         
                         return promise2;
                    },
                    function (e) {
                         logger.comment("outer promise error handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                    },
                    function (p) {
                         logger.comment("outer promise progress handler.");
                    }
               );
               
               promise.operation.moveToError();
               promise2.operation.moveToError();
               
               promise.done(
                    function() {
                         logger.comment("outer promise final done success handler");
                    },
                    function() {
                         logger.comment("outer promise final done error handler");
                    }
               );
               promise2.done(
                    function() {
                         logger.comment("inner promise final done success handler");
                    },
                    function() {
                         logger.comment("inner promise final done error handler");
                    }
               );
               
               Debug = temp;
        }
    });
     
     runner.addTest({
        id: 5,
        desc: 'Missing Debug - progress',
        pri: '0',
        test: function () {
               var temp = Debug;
               Debug = undefined;
               
            var promise = Winery.RWinery.asyncOperationOutStatic();
               var promise2;
               
               var thenpromise = promise.then(
                    function() {
                         logger.comment("outer promise then handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                         
                         return promise2;
                    },
                    function (e) {
                         logger.comment("outer promise error handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                    },
                    function (p) {
                         logger.comment("outer promise progress handler.");
                    }
               );
               
            promise.operation.triggerProgress(50);
               promise.operation.moveToCompleted(0);
               promise2.operation.triggerProgress(50);
               promise2.operation.moveToError();
               
               promise.done(
                    function() {
                         logger.comment("outer promise final done success handler");
                    },
                    function() {
                         logger.comment("outer promise final done error handler");
                    }
               );
               promise2.done(
                    function() {
                         logger.comment("inner promise final done success handler");
                    },
                    function() {
                         logger.comment("inner promise final done error handler");
                    }
               );
               
               Debug = temp;
        }
    });
     
     runner.addTest({
        id: 6,
        desc: 'All globals replaced with {} coverage case.',
        pri: '0',
        test: function () {
               removeGlobals({});
               
            var promise = Winery.RWinery.asyncOperationOutStatic();
               var promise2;
               
               var thenpromise = promise.then(
                    function() {
                         logger.comment("outer promise then handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                         
                         return promise2;
                    },
                    function (e) {
                         logger.comment("outer promise error handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                    },
                    function (p) {
                         logger.comment("outer promise progress handler.");
                    }
               );
               
            promise.operation.triggerProgress(50);
               promise.operation.moveToCompleted(0);
               promise2.operation.triggerProgress(50);
               promise2.operation.moveToError();
               
               promise.done(
                    function() {
                         logger.comment("outer promise final done success handler");
                    },
                    function() {
                         logger.comment("outer promise final done error handler");
                    }
               );
               promise2.done(
                    function() {
                         logger.comment("inner promise final done success handler");
                    },
                    function() {
                         logger.comment("inner promise final done error handler");
                    }
               );
               
               resetGlobals();
        }
    });
     
     runner.addTest({
        id: 7,
        desc: 'All globals replaced with undefined coverage case.',
        pri: '0',
        test: function () {
               removeGlobals();
               
            var promise = Winery.RWinery.asyncOperationOutStatic();
               var promise2;
               
               var thenpromise = promise.then(
                    function() {
                         logger.comment("outer promise then handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                         
                         return promise2;
                    },
                    function (e) {
                         logger.comment("outer promise error handler.");
                         promise2 = Winery.RWinery.asyncOperationOutStatic();
                         
                         promise2.then(
                              function() {
                                   logger.comment("inner promise then handler.");
                              },
                              function(e) {
                                   logger.comment("inner promise error handler.");
                              },
                              function(p) {
                                   logger.comment("inner promise progress handler.");
                              }
                         );
                    },
                    function (p) {
                         logger.comment("outer promise progress handler.");
                    }
               );
               
            promise.operation.triggerProgress(50);
               promise.operation.moveToCompleted(0);
               promise2.operation.triggerProgress(50);
               promise2.operation.moveToError();
               
               promise.done(
                    function() {
                         logger.comment("outer promise final done success handler");
                    },
                    function() {
                         logger.comment("outer promise final done error handler");
                    }
               );
               promise2.done(
                    function() {
                         logger.comment("inner promise final done success handler");
                    },
                    function() {
                         logger.comment("inner promise final done error handler");
                    }
               );
               
               resetGlobals();
        }
    });

    Loader42_FileName = "Promises tests";
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
