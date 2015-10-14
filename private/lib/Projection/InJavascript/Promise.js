//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
"use strict";
(function (EngineInterface) {
    var platform = EngineInterface.Promise;

    var Error = platform.Error;

    var ObjectGetPrototypeOf = platform.builtInJavascriptObjectEntryGetPrototypeOf;
    var ObjectPreventExtensions = platform.builtInJavascriptObjectPreventExtensions;
    var ObjectCreate = platform.builtInJavascriptObjectCreate;
    var ObjectGetOwnPropertyDescriptor = platform.builtInJavascriptObjectGetOwnPropertyDescriptor;

    var GlobalObjectEval = platform.builtInGlobalObjectEval;

    var DebugTraceAsyncCallbackStarting = platform.msTraceAsyncCallbackStarting;
    var DebugTraceAsyncCallbackCompleted = platform.msTraceAsyncCallbackCompleted;
    var DebugTraceAsyncOperationCompleted = platform.msTraceAsyncOperationCompleted;
    var DebugUpdateAsyncCallbackRelation = platform.msUpdateAsyncCallbackRelation;
    var DebugSetNonUserCodeExceptions = platform.setNonUserCodeExceptions;

    var NoErrorInErrorState = 0x800A1450;
    var InvalidStatusArg = 0x800A1451;
    var InvalidSenderArg = 0x800A1452;

    var state_working = 0;
    var state_waiting = 1;
    var state_fulfilled_error = 2;
    var state_fulfilled_success = 3;

    var log_level_required = 0;
    var log_level_important = 1;
    var log_level_verbose = 2;

    var work_type_completion = 0;
    var work_type_progress = 1;
    var work_type_operation = 2;

    var promise_on_success = "Promise.onSuccess";
    var promise_on_error = "Promise.onError";
    var promise_on_progress = "Promise.onProgress";

    function AsyncBehaviorError(message, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        this.name = 'AsyncBehaviorError';
        this.message = (message || '');
        this.asyncOpType = asyncOpType;
        this.asyncOpSource = asyncOpSource;
        this.asyncOpCausalityId = asyncOpCausalityId;
    }

    AsyncBehaviorError.prototype = ObjectCreate(Error.prototype);
    AsyncBehaviorError.prototype.constructor = AsyncBehaviorError;

    function doComplete(carrier, completeValue) {
        if (carrier._state !== state_working) {
            return;
        }

        carrier._value = completeValue;

        if (typeof completeValue === 'object' && completeValue && typeof completeValue.then === 'function') {
            carrier._state = state_waiting;

            var setAsyncOpCausalityId = function (id) { carrier._asyncOpCausalityId = id; };
            var complete = function (value) { carrier._state = state_working; doComplete(carrier, value); };
            complete.setAsyncOpCausalityId = setAsyncOpCausalityId;
            var error = function (value) { carrier._state = state_working; doError(carrier, value); };
            error.setAsyncOpCausalityId = setAsyncOpCausalityId;
            var progress = function (value) { doProgress(carrier, value); };
            progress.setAsyncOpCausalityId = setAsyncOpCausalityId;

            completeValue.then(complete, error, progress);
        } else {
            carrier._state = state_fulfilled_success;
            doNotify(carrier);
            carrier._cleanup();
        }
    }

    function doError(carrier, errorValue) {
        if (carrier._state !== state_working) {
            return;
        }

        carrier._value = errorValue;
        carrier._state = state_fulfilled_error;

        doNotify(carrier);

        carrier._cleanup();
    }

    function doProgress(carrier, progressValue) {
        DebugSetNonUserCodeExceptions(true);
        if (carrier._state > state_waiting) {
            return;
        }
        if (carrier._listeners) {
            for (var i = 0, len = carrier._listeners.length; i < len; i++) {
                var onProgress = carrier._listeners[i].onProgress;
                var traceAsync = true;
                if (onProgress && typeof onProgress === 'function') {
                    if (onProgress.setAsyncOpCausalityId && typeof onProgress.setAsyncOpCausalityId === 'function') {
                        traceAsync = false;
                        onProgress.setAsyncOpCausalityId(carrier._asyncOpCausalityId);
                    }
                    if (traceAsync) {
                        try {
                            DebugTraceAsyncCallbackStarting(carrier._asyncOpCausalityId, work_type_progress, log_level_verbose);
                            onProgress(progressValue);
                        } catch (e) {
                            DebugUpdateAsyncCallbackRelation(carrier._asyncOpCausalityId, platform.MS_ASYNC_CALLBACK_STATUS_ERROR, log_level_verbose);
                        } finally {
                            DebugTraceAsyncCallbackCompleted(log_level_verbose, carrier._asyncOpCausalityId);
                        }
                    } else {
                        try {
                            onProgress(progressValue);
                        } catch (e) {
                        }
                    }
                }
            }
        }
    }
    platform.tagPublicLibraryCode(doProgress, promise_on_progress);

    function notifySuccess(listeners, value, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        DebugSetNonUserCodeExceptions(true);
        for (var i = 0, len = listeners.length; i < len; i++) {
            var listener = listeners[i];
            var carrier = listener.carrier;
            var onComplete = listener.onComplete;
            var callbackCompleted = false;
            var traceAsync = true;
            var onCompletePresent = onComplete && typeof onComplete === 'function';
            if (onCompletePresent && onComplete.setAsyncOpCausalityId && typeof onComplete.setAsyncOpCausalityId === 'function') {
                traceAsync = false;
                onComplete.setAsyncOpCausalityId(asyncOpCausalityId);
            }
            try {
                var temp = value;
                if (onCompletePresent) {
                    if (traceAsync) {
                        DebugTraceAsyncCallbackStarting(asyncOpCausalityId, work_type_completion, log_level_required);
                    }
                    temp = onComplete(value);
                    if (traceAsync) {
                        DebugTraceAsyncCallbackCompleted(log_level_required, asyncOpCausalityId);
                    }
                    callbackCompleted = true;
                }
                doComplete(carrier, temp);
            } catch (exception) {
                if (onCompletePresent && traceAsync === true && callbackCompleted === false) {
                    DebugUpdateAsyncCallbackRelation(asyncOpCausalityId, platform.MS_ASYNC_CALLBACK_STATUS_ERROR, log_level_verbose);
                    DebugTraceAsyncCallbackCompleted(log_level_required, asyncOpCausalityId);
                }
                if (!exception.asyncOpType) {
                    exception.originatedFromHandler = true;
                    exception.asyncOpType = asyncOpType;
                    exception.asyncOpSource = asyncOpSource;
                    exception.asyncOpCausalityId = asyncOpCausalityId;
                }
                doError(carrier, exception);
            }
        }
    }
    platform.tagPublicLibraryCode(notifySuccess, promise_on_success);

    function notifyError(listeners, value, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        DebugSetNonUserCodeExceptions(true);
        for (var i = 0, len = listeners.length; i < len; i++) {
            var listener = listeners[i];
            var carrier = listener.carrier;
            var onError = listener.onError;
            var callbackCompleted = false;
            var traceAsync = true;
            var onErrorPresent = onError && typeof onError === 'function';
            if (onErrorPresent && onError.setAsyncOpCausalityId && typeof onError.setAsyncOpCausalityId === 'function') {
                traceAsync = false;
                onError.setAsyncOpCausalityId(asyncOpCausalityId);
            }
            try {
                if (onErrorPresent) {
                    if (traceAsync) {
                        DebugTraceAsyncCallbackStarting(asyncOpCausalityId, work_type_completion, log_level_required);
                    }
                    var temp = onError(value);
                    if (traceAsync) {
                        DebugTraceAsyncCallbackCompleted(log_level_required, asyncOpCausalityId);
                    }
                    callbackCompleted = true;
                    doComplete(carrier, temp);
                } else {
                    doError(carrier, value);
                }
            } catch (exception) {
                if (onErrorPresent && traceAsync === true && callbackCompleted === false) {
                    DebugUpdateAsyncCallbackRelation(asyncOpCausalityId, platform.MS_ASYNC_CALLBACK_STATUS_ERROR, log_level_verbose);
                    DebugTraceAsyncCallbackCompleted(log_level_required, asyncOpCausalityId);
                }
                if (!exception.asyncOpType) {
                    exception.originatedFromHandler = true;
                    exception.asyncOpType = asyncOpType;
                    exception.asyncOpSource = asyncOpSource;
                    exception.asyncOpCausalityId = asyncOpCausalityId;
                }
                doError(carrier, exception);
            }
        }
    }
    platform.tagPublicLibraryCode(notifyError, promise_on_error);

    function doCancel(carrier) {
        if (carrier._state === state_waiting) {
            if (typeof carrier._value.cancel === 'function') {
                carrier._value.cancel();
            }
        }
        carrier._cleanup();
    }

    function doNotify(carrier) {
        var listeners = carrier._listeners;
        carrier._listeners = [];
        if (listeners && (listeners.length > 0)) {
            switch (carrier._state) {
                case state_fulfilled_success:
                    notifySuccess(listeners, carrier._value, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);
                    break;

                case state_fulfilled_error:
                    notifyError(listeners, carrier._value, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);
                    break;
            }
        }
    }

    function doThen(carrier, complete, error, progress, creatorPromise) {
        carrier._listeners = carrier._listeners || [];

        var p = createThenPromise(creatorPromise, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);

        carrier._listeners.push({
            promise: p.promise,
            carrier: p.carrier,
            onComplete: complete,
            onError: error,
            onProgress: progress
        });

        if (carrier._state > state_waiting) {
            doNotify(carrier);
        }
        return p.promise;
    }

    function isCanceled(err) {
        return err instanceof Error && err.name === 'Canceled';
    }

    function createDebugStackFunction(err) {
        var stack = [];
        stack.push('WinRT Promise.done()');
        if (err && err.originatedFromHandler) {
            stack.push('Promise handler');
        }
        if (err && err.asyncOpType) {
            if (err.originatedFromHandler) {
                stack.push('WinRT Promise object for ' + err.asyncOpType);
            } else {
                stack.push(err.asyncOpType);
            }
        } else {
            stack.push('WinRT Promise object');
        }

        var s = '(function() { return function(err) { var o = {};';
        var wrap = function (n) {
            return '[\"' + n + '\"]';
        }

        for (var i = 1; i < stack.length; i++) {
            s += 'o' + wrap(stack[i]) + ' = function () { o' + wrap(stack[i - 1]) + '(); };';
        }
        s += 'o' + wrap(stack[0]) + ' = function () { throw err; };';
        s += 'return o' + wrap(stack[stack.length - 1]);
        s += '}})()';
        return GlobalObjectEval(s)(err);
    }

    function postError(err) {
        if (isCanceled(err)) {
            return;
        }
        platform.enqueueTask(createDebugStackFunction(err));
    }

    function doDoneSuccess(carrier, complete) {
        if (complete) {
            try {
                DebugTraceAsyncCallbackStarting(carrier._asyncOpCausalityId, work_type_completion, log_level_required);
                complete(carrier._value);
            } finally {
                DebugTraceAsyncCallbackCompleted(log_level_required, carrier._asyncOpCausalityId);
            }
        }
    }
    platform.tagPublicLibraryCode(doDoneSuccess, promise_on_success);

    function doDoneError(carrier, error) {
        if (error) {
            try {
                DebugTraceAsyncCallbackStarting(carrier._asyncOpCausalityId, work_type_completion, log_level_required);
                error(carrier._value);
                return;
            } finally {
                DebugTraceAsyncCallbackCompleted(log_level_required, carrier._asyncOpCausalityId);
            }
        }
        if (!isCanceled(carrier._value)) {
            postError(carrier._value);
        }
    }
    platform.tagPublicLibraryCode(doDoneError, promise_on_error);

    function doDone(carrier, complete, error, progress, creatorPromise) {
        if (carrier._state === state_fulfilled_success) {
            doDoneSuccess(carrier, complete);
            return;
        }
        if (carrier._state === state_fulfilled_error) {
            doDoneError(carrier, error);
            return;
        }
        doThen(carrier, complete, error, progress, creatorPromise)
        .then(null, postError, null);
    }

    function getResultsOfAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        DebugSetNonUserCodeExceptions(true);
        return op.getResults();
    }

    function cancelAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        DebugSetNonUserCodeExceptions(true);
        return op.cancel();
    }

    var AsyncOpPromise = function (op, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        var that = this;
        var carrier = {
            _listeners: null,
            _state: state_working,
            _value: null,
            _cleanup: function () { op = null; },
            _asyncOpType: asyncOpType,
            _asyncOpSource: asyncOpSource,
            _asyncOpCausalityId: asyncOpCausalityId
        };

        this.cancel = function () {
            DebugSetNonUserCodeExceptions(true);
            try {
                if (op) {
                    cancelAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId);
                }
                doCancel(carrier);
            } catch (exception) {
                exception.asyncOpType = asyncOpType;
                exception.asyncOpSource = asyncOpSource;
                exception.asyncOpCausalityId = asyncOpCausalityId;
                doError(carrier, exception);
            }
        };

        this.then = function (complete, error, progress) {
            return doThen(carrier, complete, error, progress, that);
        };

        this.done = function (complete, error, progress) {
            doDone(carrier, complete, error, progress, that);
        };

        var progressProperty = ObjectGetOwnPropertyDescriptor(ObjectGetPrototypeOf(op), 'progress');
        if (progressProperty) {
            if ('set' in progressProperty) {
                op.progress = function (op2, progressInfo) {
                    doProgress(carrier, progressInfo);
                };
            };
        }

        op.completed = function (op2, status) {
            DebugSetNonUserCodeExceptions(true);
            if (!op2 || !op2.operation) {
                if (carrier._state <= state_waiting) {
                    DebugTraceAsyncOperationCompleted(asyncOpCausalityId, platform.MS_ASYNC_OP_STATUS_ERROR, log_level_required);
                }

                doError(carrier, new AsyncBehaviorError(platform.getErrorMessage(InvalidSenderArg), asyncOpType, asyncOpSource, asyncOpCausalityId));
                return;
            }
            switch (status) {
                case platform.MS_ASYNC_OP_STATUS_SUCCESS:
                    if (carrier._state <= state_waiting) {
                        DebugTraceAsyncOperationCompleted(asyncOpCausalityId, platform.MS_ASYNC_OP_STATUS_SUCCESS, log_level_required);
                    }

                    var result;
                    try {
                        result = getResultsOfAsyncOp(op2.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);
                        doComplete(carrier, result);
                    } catch (exception) {
                        exception.asyncOpType = asyncOpType;
                        exception.asyncOpSource = asyncOpSource;
                        exception.asyncOpCausalityId = asyncOpCausalityId;
                        doError(carrier, exception);
                    }

                    break;

                case platform.MS_ASYNC_OP_STATUS_ERROR:
                    if (carrier._state <= state_waiting) {
                        DebugTraceAsyncOperationCompleted(asyncOpCausalityId, platform.MS_ASYNC_OP_STATUS_ERROR, log_level_required);
                    }

                    var error;
                    try {
                        getResultsOfAsyncOp(op2.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);
                        error = new AsyncBehaviorError(platform.getErrorMessage(NoErrorInErrorState), asyncOpType, asyncOpSource, asyncOpCausalityId);
                    } catch (exception) {
                        exception.asyncOpType = asyncOpType;
                        exception.asyncOpSource = asyncOpSource;
                        exception.asyncOpCausalityId = asyncOpCausalityId;
                        error = exception;
                    }
                    doError(carrier, error);

                    break;

                case platform.MS_ASYNC_OP_STATUS_CANCELED:
                    if (carrier._state <= state_waiting) {
                        DebugTraceAsyncOperationCompleted(asyncOpCausalityId, platform.MS_ASYNC_OP_STATUS_CANCELED, log_level_required);
                    }

                    var err = new Error('Canceled');
                    err.name = err.message;
                    err.asyncOpType = asyncOpType;
                    err.asyncOpSource = asyncOpSource;
                    err.asyncOpCausalityId = asyncOpCausalityId;
                    doError(carrier, err);

                    break;

                default:
                    if (carrier._state <= state_waiting) {
                        DebugTraceAsyncOperationCompleted(asyncOpCausalityId, platform.MS_ASYNC_OP_STATUS_ERROR, log_level_required);
                    }

                    doError(carrier, new AsyncBehaviorError(platform.getErrorMessage(InvalidStatusArg), asyncOpType, asyncOpSource, asyncOpCausalityId));
            }
        };
    };
    AsyncOpPromise.prototype = {};

    var thenPromisePrototype = {};
    ObjectPreventExtensions(thenPromisePrototype);
    var createThenPromise = function (creator, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        var carrier = {
            _listeners: null,
            _state: state_working,
            _value: null,
            _cleanup: function () { creator = null; },
            _asyncOpType: asyncOpType,
            _asyncOpSource: asyncOpSource,
            _asyncOpCausalityId: asyncOpCausalityId
        };
        var thenFunction = function (complete, error, progress) {
            return doThen(carrier, complete, error, progress, this);
        };
        ObjectPreventExtensions(thenFunction);
        var cancelFunction = function () {
            if (creator) {
                creator.cancel();
            }
            doCancel(carrier);
        };
        ObjectPreventExtensions(cancelFunction);
        var doneFunction = function (complete, error, progress) {
            doDone(carrier, complete, error, progress, this);
        };
        ObjectPreventExtensions(doneFunction);
        var thenPromise =
            ObjectCreate(thenPromisePrototype, {
                then: { writable: false, enumerable: true, configurable: false, value: thenFunction },
                cancel: { writable: false, enumerable: true, configurable: false, value: cancelFunction },
                done: { writable: false, enumerable: true, configurable: false, value: doneFunction }
            });
        ObjectPreventExtensions(thenPromise);
        return { promise: thenPromise, carrier: carrier };
    };

    var asyncOpWrapperPrototype = {}
    ObjectPreventExtensions(asyncOpWrapperPrototype);
    function AsyncOpWrapper(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {
        var promise = null;
        var thenFunction = function (complete, error, progress) {
            promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);
            return promise.then(complete, error, progress);
        };
        ObjectPreventExtensions(thenFunction);
        var cancelFunction = function () {
            promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);
            promise.cancel();
        };
        ObjectPreventExtensions(cancelFunction);
        var doneFunction = function (complete, error, progress) {
            promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);
            promise.done(complete, error, progress);
        };
        ObjectPreventExtensions(doneFunction);
        var wrapper =
            ObjectCreate(asyncOpWrapperPrototype, {
                operation: { writable: false, enumerable: true, configurable: false, value: op },
                then: { writable: false, enumerable: true, configurable: false, value: thenFunction },
                cancel: { writable: false, enumerable: true, configurable: false, value: cancelFunction },
                done: { writable: false, enumerable: true, configurable: false, value: doneFunction }
            });
        ObjectPreventExtensions(wrapper);
        return wrapper;
    };

    return AsyncOpWrapper;
});
