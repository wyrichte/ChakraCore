if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    runner.globalSetup(function () { });
    runner.globalTeardown(function () { });

    runner.addTest({
        id: '1',
        desc: 'Promise handler throws non-object',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { throw "some string"; },
                function (error) { logger.comment(`Shouldn't call into error handler 1: ${error}`); }
            )
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 2: ${result}`); },
                function (error) { 
                    logger.comment(`Error handler 2 got argument of type ${typeof error}: ${error}`);
                }
            );
            promise.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '2',
        desc: 'Promise success handler throws object',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { throw {}; },
                function (error) { logger.comment(`Shouldn't call into error handler 1: ${error}`); }
            )
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 2: ${result}`); },
                function (error) { 
                    logger.comment(`Error handler 2 got argument of type ${typeof error} (error instanceof Error == ${error instanceof Error}): ${error}`);
                    logger.comment(`error.originatedFromHandler = ${error.originatedFromHandler}`);
                    logger.comment(`error.asyncOpType = ${error.asyncOpType}`);
                    logger.comment(`error.asyncOpSource = ${error.asyncOpSource}`);
                    logger.comment(`error.asyncOpCausalityId = ${error.asyncOpCausalityId}`);
                }
            );
            promise.operation.moveToCompleted(192);
        }
    });

    runner.addTest({
        id: '3',
        desc: 'Promise error handler throws object',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 1: ${result}`); },
                function (error) { throw {}; }
            )
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 2: ${result}`); },
                function (error) { 
                    logger.comment(`Error handler 2 got argument of type ${typeof error} (error instanceof Error == ${error instanceof Error}): ${error}`);
                    logger.comment(`error.originatedFromHandler = ${error.originatedFromHandler}`);
                    logger.comment(`error.asyncOpType = ${error.asyncOpType}`);
                    logger.comment(`error.asyncOpSource = ${error.asyncOpSource}`);
                    logger.comment(`error.asyncOpCausalityId = ${error.asyncOpCausalityId}`);
                }
            );
            promise.operation.moveToError();
        }
    });

    runner.addTest({
        id: '4',
        desc: 'Promise error handler throws pathological object',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 1: ${result}`); },
                function (error) { 
                    var new_error = { 
                        get asyncOpType() { throw 'ignored'; },
                        foo: 'foo'
                    }; 
                    throw new_error;
                }
            )
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 2: ${result}`); },
                function (error) { 
                    logger.comment(`Error handler 2 got argument of type ${typeof error} (error instanceof Error == ${error instanceof Error}): ${error}`);
                    logger.comment(`error.originatedFromHandler = ${error.originatedFromHandler}`);
                    logger.comment(`error.asyncOpSource = ${error.asyncOpSource}`);
                    logger.comment(`error.asyncOpCausalityId = ${error.asyncOpCausalityId}`);
                    logger.comment(`error.foo = ${error.foo}`);
                }
            );
            promise.operation.moveToError();
        }
    });

    runner.addTest({
        id: '5',
        desc: 'Promise error handler throws frozen object',
        pri: '0',
        test: function () {
            var promise = Winery.RWinery.asyncOperationOutStatic();
            promise
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 1: ${result}`); },
                function (error) { 
                    var new_error = { 
                        foo: 'foo'
                    }; 
                    Object.freeze(new_error);
                    throw new_error;
                }
            )
            .then(
                function (result) { logger.comment(`Shouldn't call into success handler 2: ${result}`); },
                function (error) { 
                    logger.comment(`Error handler 2 got argument of type ${typeof error} (error instanceof Error == ${error instanceof Error}): ${error}`);
                    logger.comment(`error.originatedFromHandler = ${error.originatedFromHandler}`);
                    logger.comment(`error.asyncOpType = ${error.asyncOpType}`);
                    logger.comment(`error.asyncOpSource = ${error.asyncOpSource}`);
                    logger.comment(`error.asyncOpCausalityId = ${error.asyncOpCausalityId}`);
                    logger.comment(`error.foo = ${error.foo}`);
                }
            );
            promise.operation.moveToError();
        }
    });

    Loader42_FileName = "Tests for promise handlers throwing error";
})();

if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
