//---------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
// Runtime details of promises implementation
//----------------------------------------------------------------------------

#pragma once

#define PROJECTION_PROMISE_BODY \
    L"// Copyright (c) Microsoft Corporation.\n"  \
    L"// All rights reserved.\n"  \
    L"function AsyncBehaviorError(message, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    this.name = 'AsyncBehaviorError';\n"  \
    L"    this.message = (message || '');\n"  \
    L"    this.asyncOpType = asyncOpType;\n"  \
    L"    this.asyncOpSource = asyncOpSource;\n"  \
    L"    this.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"}\n"  \
    L"AsyncBehaviorError.prototype = Object.create(Error.prototype);\n"  \
    L"AsyncBehaviorError.prototype.constructor = AsyncBehaviorError;\n"  \
    L"\n"  \
    L"function doComplete(carrier, completeValue) {\n"  \
    L"    if (carrier._state !== state_working) {\n"  \
    L"        return;\n"  \
    L"    }\n"  \
    L"\n"  \
    L"    carrier._value = completeValue;\n"  \
    L"\n"  \
    L"    if (typeof completeValue === 'object' && completeValue && typeof completeValue.then === 'function') {\n"  \
    L"        carrier._state = state_waiting;\n"  \
    L"\n"  \
    L"        var setAsyncOpCausalityId = function (id) { carrier._asyncOpCausalityId = id; };\n"  \
    L"        var complete = function(value) { carrier._state = state_working; doComplete(carrier, value); };\n"  \
    L"        complete.setAsyncOpCausalityId = setAsyncOpCausalityId;\n"  \
    L"        var error = function(value) { carrier._state = state_working; doError(carrier, value); };\n"  \
    L"        error.setAsyncOpCausalityId = setAsyncOpCausalityId;\n"  \
    L"        var progress = function (value) { doProgress(carrier, value); };\n"  \
    L"        progress.setAsyncOpCausalityId = setAsyncOpCausalityId;\n"  \
    L"\n"  \
    L"        completeValue.then(complete, error, progress);\n"  \
    L"    } else {\n"  \
    L"        carrier._state = state_fulfilled_success;\n"  \
    L"        doNotify(carrier);\n"  \
    L"        carrier._cleanup();\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"\n"  \
    L"function doError(carrier, errorValue) {\n"  \
    L"    if (carrier._state !== state_working) {\n"  \
    L"        return;\n"  \
    L"    }\n"  \
    L"\n"  \
    L"    carrier._value = errorValue;\n"  \
    L"    carrier._state = state_fulfilled_error;\n"  \
    L"\n"  \
    L"    doNotify(carrier);\n"  \
    L"\n"  \
    L"    carrier._cleanup();\n"  \
    L"}\n"  \
    L"\n"  \
    L"function doProgress(carrier, progressValue) {\n"  \
    L"    (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"    if (carrier._state > state_waiting) {\n"  \
    L"        return;\n"  \
    L"    }\n"  \
    L"    if (carrier._listeners) {\n"  \
    L"        for (var i = 0, len = carrier._listeners.length; i < len; i++) {\n"  \
    L"            var onProgress = carrier._listeners[i].onProgress;\n"  \
    L"            var traceAsync = true;\n"  \
    L"            if (onProgress && typeof onProgress === 'function') {\n"  \
    L"                if (onProgress.setAsyncOpCausalityId && typeof onProgress.setAsyncOpCausalityId === 'function') {\n"  \
    L"                    traceAsync = false;\n"  \
    L"                    onProgress.setAsyncOpCausalityId(carrier._asyncOpCausalityId);\n"  \
    L"                }\n"  \
    L"                if (traceAsync) {\n"  \
    L"                    try {\n"  \
    L"                        traceAsyncCallbackStart(carrier._asyncOpCausalityId, work_type_progress, log_level_verbose);\n"  \
    L"                        onProgress(progressValue);\n"  \
    L"                    } catch (e) {\n"  \
    L"                        updateAsyncCallbackRelation(carrier._asyncOpCausalityId, callback_status_error, log_level_verbose);\n"  \
    L"                    } finally {\n"  \
    L"                        traceAsyncCallbackComplete(carrier._asyncOpCausalityId, log_level_verbose);\n"  \
    L"                    }\n"  \
    L"                } else {\n"  \
    L"                    try {\n"  \
    L"                        onProgress(progressValue);\n"  \
    L"                    } catch (e) {\n"  \
    L"                    }\n"  \
    L"                }\n"  \
    L"            }\n"  \
    L"        }\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"\n"  \
    L"function notifySuccess(listeners, value, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"    for (var i = 0, len = listeners.length; i < len; i++) {\n"  \
    L"        var listener = listeners[i];\n"  \
    L"        var carrier = listener.carrier;\n"  \
    L"        var onComplete = listener.onComplete;\n"  \
    L"        var callbackCompleted = false;\n"  \
    L"        var traceAsync = true;\n"  \
    L"        var onCompletePresent = onComplete && typeof onComplete === 'function';\n"  \
    L"        if (onCompletePresent && onComplete.setAsyncOpCausalityId && typeof onComplete.setAsyncOpCausalityId === 'function') {\n"  \
    L"            traceAsync = false;\n"  \
    L"            onComplete.setAsyncOpCausalityId(asyncOpCausalityId);\n"  \
    L"        }\n"  \
    L"        try {\n"  \
    L"            var temp = value;\n"  \
    L"            if (onCompletePresent) {\n"  \
    L"                if (traceAsync) {\n"  \
    L"                    traceAsyncCallbackStart(asyncOpCausalityId, work_type_completion, log_level_required);\n"  \
    L"                }\n"  \
    L"                temp = onComplete(value);\n"  \
    L"                if (traceAsync) {\n"  \
    L"                    traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n"  \
    L"                }\n"  \
    L"                callbackCompleted = true;\n"  \
    L"            }\n"  \
    L"            doComplete(carrier, temp);\n"  \
    L"        } catch (exception) {\n"  \
    L"            if (onCompletePresent && traceAsync === true && callbackCompleted === false) {\n"  \
    L"                updateAsyncCallbackRelation(asyncOpCausalityId, callback_status_error, log_level_verbose);\n"  \
    L"                traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n"  \
    L"            }\n"  \
    L"            if (!exception.asyncOpType) {\n"  \
    L"                exception.originatedFromHandler = true;\n"  \
    L"                exception.asyncOpType = asyncOpType;\n"  \
    L"                exception.asyncOpSource = asyncOpSource;\n"  \
    L"                exception.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"            }\n"  \
    L"            doError(carrier, exception);\n"  \
    L"        }\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"\n"  \
    L"function notifyError(listeners, value, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"    for (var i = 0, len = listeners.length; i < len; i++) {\n"  \
    L"        var listener = listeners[i];\n"  \
    L"        var carrier = listener.carrier;\n"  \
    L"        var onError = listener.onError;\n"  \
    L"        var callbackCompleted = false;\n"  \
    L"        var traceAsync = true;\n"  \
    L"        var onErrorPresent = onError && typeof onError === 'function';\n"  \
    L"        if (onErrorPresent && onError.setAsyncOpCausalityId && typeof onError.setAsyncOpCausalityId === 'function') {\n"  \
    L"            traceAsync = false;\n"  \
    L"            onError.setAsyncOpCausalityId(asyncOpCausalityId);\n"  \
    L"        }\n"  \
    L"        try {\n"  \
    L"            if (onErrorPresent) {\n"  \
    L"                if (traceAsync) {\n"  \
    L"                    traceAsyncCallbackStart(asyncOpCausalityId, work_type_completion, log_level_required);\n"  \
    L"                }\n"  \
    L"                var temp = onError(value);\n"  \
    L"                if (traceAsync) {\n"  \
    L"                    traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n"  \
    L"                }\n"  \
    L"                callbackCompleted = true;\n"  \
    L"                doComplete(carrier, temp);\n"  \
    L"            } else {\n"  \
    L"                doError(carrier, value);\n"  \
    L"            }\n"  \
    L"        } catch (exception) {\n"  \
    L"            if (onErrorPresent && traceAsync === true && callbackCompleted === false) {\n"  \
    L"                updateAsyncCallbackRelation(asyncOpCausalityId, callback_status_error, log_level_verbose);\n"  \
    L"                traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n"  \
    L"            }\n"  \
    L"            if (!exception.asyncOpType) {\n"  \
    L"                exception.originatedFromHandler = true;\n"  \
    L"                exception.asyncOpType = asyncOpType;\n"  \
    L"                exception.asyncOpSource = asyncOpSource;\n"  \
    L"                exception.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"            }\n"  \
    L"            doError(carrier, exception);\n"  \
    L"        }\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"\n"  \
    L"var state_working = 0;\n"  \
    L"var state_waiting = 1;\n"  \
    L"var state_fulfilled_error = 2;\n"  \
    L"var state_fulfilled_success = 3;\n"  \
    L"var log_level_required = 0;\n"  \
    L"var log_level_important = 1;\n"  \
    L"var log_level_verbose = 2;\n"  \
    L"var work_type_completion = 0;\n"  \
    L"var work_type_progress = 1;\n"  \
    L"var work_type_operation = 2;\n"  \
    L"var invalid_async_operation_id = 0;\n"  \
    L"var op_status_success = 1;\n"  \
    L"var op_status_canceled = 2;\n"  \
    L"var op_status_error = 3;\n"  \
    L"var callback_status_assign_delegate = 0;\n"  \
    L"var callback_status_join = 1;\n"  \
    L"var callback_status_cancel = 3;\n"  \
    L"var callback_status_error = 4;\n"  \
    L"var traceAsyncCallbackStart = function() { };\n"  \
    L"var traceAsyncOperationComplete = function() { };\n"  \
    L"var updateAsyncCallbackRelation = function() { };\n"  \
    L"var debug_msTraceAsyncCallbackCompleted = function() { };\n"  \
    L"var debug = Debug;\n"  \
    L"if (Debug != undefined) {\n"  \
    L"    op_status_success = Debug.MS_ASYNC_OP_STATUS_SUCCESS || 1;\n"  \
    L"    op_status_canceled = Debug.MS_ASYNC_OP_STATUS_CANCELED || 2;\n"  \
    L"    op_status_error = Debug.MS_ASYNC_OP_STATUS_ERROR || 3;\n"  \
    L"    callback_status_assign_delegate = Debug.MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE || 0;\n"  \
    L"    callback_status_join = Debug.MS_ASYNC_CALLBACK_STATUS_JOIN || 1;\n"  \
    L"    callback_status_cancel = Debug.MS_ASYNC_CALLBACK_STATUS_CANCEL || 3;\n"  \
    L"    callback_status_error = Debug.MS_ASYNC_CALLBACK_STATUS_ERROR || 4;\n"  \
    L"    if (Debug.msTraceAsyncCallbackStarting && typeof Debug.msTraceAsyncCallbackStarting === 'function') {\n"  \
    L"        traceAsyncCallbackStart = Debug.msTraceAsyncCallbackStarting;\n"  \
    L"    }\n"  \
    L"    if (Debug.msTraceAsyncOperationCompleted && typeof Debug.msTraceAsyncOperationCompleted === 'function') {\n"  \
    L"        traceAsyncOperationComplete = Debug.msTraceAsyncOperationCompleted;\n"  \
    L"    }\n"  \
    L"    if (Debug.msTraceAsyncCallbackCompleted && typeof Debug.msTraceAsyncCallbackCompleted === 'function') {\n"  \
    L"        debug_msTraceAsyncCallbackCompleted = Debug.msTraceAsyncCallbackCompleted;\n"  \
    L"    }\n"  \
    L"    if (Debug.msUpdateAsyncCallbackRelation && typeof Debug.msUpdateAsyncCallbackRelation === 'function') {\n"  \
    L"        updateAsyncCallbackRelation = Debug.msUpdateAsyncCallbackRelation;\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"function traceAsyncCallbackComplete(asyncOpCausalityId, logLevel) {\n"  \
    L"    if (invalid_async_operation_id != asyncOpCausalityId) {\n"  \
    L"        debug_msTraceAsyncCallbackCompleted(logLevel);\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"\n"  \
    L"function doCancel(carrier) {\n"  \
    L"    if (carrier._state === state_waiting) {\n"  \
    L"        if (typeof carrier._value.cancel === 'function') {\n"  \
    L"            carrier._value.cancel();\n"  \
    L"        }\n"  \
    L"    }\n"  \
    L"    carrier._cleanup();\n"  \
    L"}\n"  \
    L"\n"  \
    L"function doNotify(carrier) {\n"  \
    L"    var listeners = carrier._listeners;\n"  \
    L"    carrier._listeners = [];\n"  \
    L"    if (listeners && (listeners.length > 0)) {\n"  \
    L"        switch (carrier._state) {\n"  \
    L"            case state_fulfilled_success:\n"  \
    L"                notifySuccess(listeners, carrier._value, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);\n"  \
    L"                break;\n"  \
    L"\n"  \
    L"            case state_fulfilled_error:\n"  \
    L"                notifyError(listeners, carrier._value, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);\n"  \
    L"                break;\n"  \
    L"        }\n"  \
    L"    }\n"  \
    L"}\n"  \
    L"\n"  \
    L"function doThen(carrier, complete, error, progress, creatorPromise) {\n"  \
    L"    carrier._listeners = carrier._listeners || [];\n"  \
    L"\n"  \
    L"    var p = createThenPromise(creatorPromise, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);\n"  \
    L"\n"  \
    L"    carrier._listeners.push({\n"  \
    L"        promise: p.promise,\n"  \
    L"        carrier: p.carrier,\n"  \
    L"        onComplete: complete,\n"  \
    L"        onError: error,\n"  \
    L"        onProgress: progress\n"  \
    L"    });\n"  \
    L"\n"  \
    L"    if (carrier._state > state_waiting) {\n"  \
    L"        doNotify(carrier);\n"  \
    L"    }\n"  \
    L"    return p.promise;\n"  \
    L"}\n"  \
    L"function isCanceled(err) {\n"  \
    L"    return err instanceof Error && err.name === 'Canceled';\n"  \
    L"}\n"  \
    L"function createDebugStack(err) {\n"  \
    L"    var stack = [];\n"  \
    L"    stack.push('WinRT Promise.done()');\n"  \
    L"    if (err && err.originatedFromHandler) {\n"  \
    L"        stack.push('Promise handler');\n"  \
    L"    }\n"  \
    L"    if (err && err.asyncOpType) {\n"  \
    L"        if (err.originatedFromHandler) {\n"  \
    L"            stack.push('WinRT Promise object for ' + err.asyncOpType);\n"  \
    L"        } else {\n"  \
    L"            stack.push(err.asyncOpType);\n"  \
    L"        }\n"  \
    L"    } else {\n"  \
    L"        stack.push('WinRT Promise object');\n"  \
    L"    }\n"  \
    L"\n"  \
    L"    var s = 'var o = {};';\n"  \
    L"    var wrap = function (n) {\n"  \
    L"        return '[\"' + n + '\"]';\n"  \
    L"    }\n"  \
    L"\n"  \
    L"    for (var i = 1; i < stack.length; i++) {\n"  \
    L"        s += 'o' + wrap(stack[i]) + ' = function () { o' + wrap(stack[i - 1]) + '(); };';\n"  \
    L"    }\n"  \
    L"    s += 'o' + wrap(stack[0]) + ' = function () { rethrowPromiseError(); };';\n"  \
    L"    s += 'o' + wrap(stack[stack.length - 1]);\n"  \
    L"    return s;\n"  \
    L"}\n"  \
    L"function postError(err) {\n"  \
    L"    if (isCanceled(err)) {\n"  \
    L"        return;\n"  \
    L"    }\n"  \
    L"    var s = createDebugStack(err);\n"  \
    L"    function rethrowPromiseError() {\n"  \
    L"        throw err;\n"  \
    L"    }\n"  \
    L"    setTimeout(eval(s), 0);\n"  \
    L"}\n"  \
    L"function doDone(carrier, complete, error, progress, creatorPromise) {\n"  \
    L"    if (carrier._state === state_fulfilled_success) {\n"  \
    L"        if (complete) {\n"  \
    L"            try {\n"  \
    L"                traceAsyncCallbackStart(carrier._asyncOpCausalityId, work_type_completion, log_level_required);\n"  \
    L"                complete(carrier._value);\n"  \
    L"            } finally {\n"  \
    L"                traceAsyncCallbackComplete(carrier._asyncOpCausalityId, log_level_required);\n"  \
    L"            }\n"  \
    L"        }\n"  \
    L"        return;\n"  \
    L"    }\n"  \
    L"    if (carrier._state === state_fulfilled_error) {\n"  \
    L"        if (error) {\n"  \
    L"            try {\n"  \
    L"                traceAsyncCallbackStart(carrier._asyncOpCausalityId, work_type_completion, log_level_required);\n"  \
    L"                error(carrier._value);\n"  \
    L"                return;\n"  \
    L"            } finally {\n"  \
    L"                traceAsyncCallbackComplete(carrier._asyncOpCausalityId, log_level_required);\n"  \
    L"            }\n"  \
    L"        }\n"  \
    L"        if (!isCanceled(carrier._value)) {\n"  \
    L"            postError(carrier._value);\n"  \
    L"        }\n"  \
    L"        return;\n"  \
    L"    }\n"  \
    L"    doThen(carrier, complete, error, progress, creatorPromise)\n"  \
    L"    .then(null, postError, null);\n"  \
    L"}\n"  \
    L"\n"  \
    L"function getResultsOfAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"    return op.getResults();\n"  \
    L"}\n"  \
    L"\n"  \
    L"function cancelAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"    return op.cancel();\n"  \
    L"}\n"  \
    L"\n"  \
    L"var AsyncOpPromise = function (op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    var that = this;\n"  \
    L"    var carrier = {\n"  \
    L"        _listeners: null,\n"  \
    L"        _state: state_working,\n"  \
    L"        _value: null,\n"  \
    L"        _cleanup: function () { op = null; },\n"  \
    L"        _asyncOpType: asyncOpType,\n"  \
    L"        _asyncOpSource: asyncOpSource,\n"  \
    L"        _asyncOpCausalityId: asyncOpCausalityId\n"  \
    L"    };\n"  \
    L"\n"  \
    L"    this.cancel = function () {\n"  \
    L"        (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"        try {\n"  \
    L"            if (op) {\n"  \
    L"                cancelAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"            }\n"  \
    L"            doCancel(carrier);\n"  \
    L"        } catch (exception) {\n"  \
    L"            exception.asyncOpType = asyncOpType;\n"  \
    L"            exception.asyncOpSource = asyncOpSource;\n"  \
    L"            exception.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"            doError(carrier, exception);\n"  \
    L"        }\n"  \
    L"    };\n"  \
    L"\n"  \
    L"    this.then = function (complete, error, progress) {\n"  \
    L"        return doThen(carrier, complete, error, progress, that);\n"  \
    L"    };\n"  \
    L"\n"  \
    L"    this.done = function (complete, error, progress) {\n"  \
    L"        doDone(carrier, complete, error, progress, that);\n"  \
    L"    };\n"  \
    L"\n"  \
    L"    var progressProperty = Object.getOwnPropertyDescriptor(Object.getPrototypeOf(op), 'progress');\n"  \
    L"    if (progressProperty) {\n"  \
    L"        if ('set' in progressProperty) {\n"  \
    L"            op.progress = function (op2, progressInfo) {\n"  \
    L"                doProgress(carrier, progressInfo);\n"  \
    L"            };\n"  \
    L"        };\n"  \
    L"    }\n"  \
    L"\n"  \
    L"    op.completed = function (op2, status) {\n"  \
    L"        (Debug && (Debug.setNonUserCodeExceptions = true));\n"  \
    L"        if (!op2 || !op2.operation) {\n"  \
    L"            if (carrier._state <= state_waiting) {\n"  \
    L"                traceAsyncOperationComplete(asyncOpCausalityId, op_status_error, log_level_required);\n"  \
    L"            }\n"  \
    L"\n"  \
    L"            var invalidSenderArgMsg = errorStrings[2];\n"  \
    L"            doError(carrier, new AsyncBehaviorError(invalidSenderArgMsg, asyncOpType, asyncOpSource, asyncOpCausalityId));\n"  \
    L"            return;\n"  \
    L"        }\n"  \
    L"        switch (status) {\n"  \
    L"            case op_status_success:\n"  \
    L"                if (carrier._state <= state_waiting) {\n"  \
    L"                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_success, log_level_required);\n"  \
    L"                }\n"  \
    L"\n"  \
    L"                var result;\n"  \
    L"                try {\n"  \
    L"                    result = getResultsOfAsyncOp(op2.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"                    doComplete(carrier, result);\n"  \
    L"                } catch(exception) {\n"  \
    L"                    exception.asyncOpType = asyncOpType;\n"  \
    L"                    exception.asyncOpSource = asyncOpSource;\n"  \
    L"                    exception.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"                    doError(carrier, exception);\n"  \
    L"                }\n"  \
    L"\n"  \
    L"                break;\n"  \
    L"\n"  \
    L"            case op_status_error:\n"  \
    L"                if (carrier._state <= state_waiting) {\n"  \
    L"                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_error, log_level_required);\n"  \
    L"                }\n"  \
    L"\n"  \
    L"                var error;\n"  \
    L"                try {\n"  \
    L"                    getResultsOfAsyncOp(op2.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"                    var noErrorInErrorStateMsg = errorStrings[0];\n"  \
    L"                    error = new AsyncBehaviorError(noErrorInErrorStateMsg, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"                } catch (exception) {\n"  \
    L"                    exception.asyncOpType = asyncOpType;\n"  \
    L"                    exception.asyncOpSource = asyncOpSource;\n"  \
    L"                    exception.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"                    error = exception;\n"  \
    L"                }\n"  \
    L"                doError(carrier, error);\n"  \
    L"\n"  \
    L"                break;\n"  \
    L"\n"  \
    L"            case op_status_canceled:\n"  \
    L"                if (carrier._state <= state_waiting) {\n"  \
    L"                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_canceled, log_level_required);\n"  \
    L"                }\n"  \
    L"\n"  \
    L"                var err = new Error('Canceled');\n"  \
    L"                err.name = err.message;\n"  \
    L"                err.asyncOpType = asyncOpType;\n"  \
    L"                err.asyncOpSource = asyncOpSource;\n"  \
    L"                err.asyncOpCausalityId = asyncOpCausalityId;\n"  \
    L"                doError(carrier, err);\n"  \
    L"\n"  \
    L"                break;\n"  \
    L"\n"  \
    L"            default:\n"  \
    L"                if (carrier._state <= state_waiting) {\n"  \
    L"                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_error, log_level_required);\n"  \
    L"                }\n"  \
    L"\n"  \
    L"                var invalidStatusArgMsg = errorStrings[1];\n"  \
    L"                doError(carrier, new AsyncBehaviorError(invalidStatusArgMsg, asyncOpType, asyncOpSource, asyncOpCausalityId));\n"  \
    L"        }\n"  \
    L"    };\n"  \
    L"};\n"  \
    L"AsyncOpPromise.prototype = {};\n"  \
    L"\n"  \
    L"var thenPromisePrototype = {};\n"  \
    L"Object.preventExtensions(thenPromisePrototype);\n"  \
    L"var createThenPromise = function (creator, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    var carrier = {\n"  \
    L"        _listeners: null,\n"  \
    L"        _state: state_working,\n"  \
    L"        _value: null,\n"  \
    L"        _cleanup: function () { creator = null; },\n"  \
    L"        _asyncOpType: asyncOpType,\n"  \
    L"        _asyncOpSource: asyncOpSource,\n"  \
    L"        _asyncOpCausalityId: asyncOpCausalityId\n"  \
    L"    };\n"  \
    L"    var thenPromise =\n"  \
    L"        Object.create(thenPromisePrototype, {\n"  \
    L"            then: { writable: false, enumerable: true, configurable: false, value: function (complete, error, progress) {\n"  \
    L"                return doThen(carrier, complete, error, progress, this);\n"  \
    L"            }\n"  \
    L"            },\n"  \
    L"            cancel: { writable: false, enumerable: true, configurable: false, value: function () {\n"  \
    L"                if (creator) {\n"  \
    L"                    creator.cancel();\n"  \
    L"                }\n"  \
    L"                doCancel(carrier);\n"  \
    L"            }\n"  \
    L"            },\n"  \
    L"            done: { writable: false, enumerable: true, configurable: false, value: function (complete, error, progress) {\n"  \
    L"                doDone(carrier, complete, error, progress, this);\n"  \
    L"            }\n"  \
    L"            }\n"  \
    L"        });\n"  \
    L"    Object.preventExtensions(thenPromise);\n"  \
    L"    return { promise: thenPromise, carrier: carrier };\n"  \
    L"};\n"  \
    L"\n"  \
    L"var asyncOpWrapperPrototype = {}\n"  \
    L"Object.preventExtensions(asyncOpWrapperPrototype);\n"  \
    L"function AsyncOpWrapper(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n"  \
    L"    var promise = null;\n"  \
    L"    var thenFunction = function (complete, error, progress) {\n"  \
    L"                promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"                return promise.then(complete, error, progress);\n"  \
    L"            };\n"  \
    L"    Object.preventExtensions(thenFunction);\n"  \
    L"    var cancelFunction = function () {\n"  \
    L"                promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"                promise.cancel();\n"  \
    L"            };\n"  \
    L"    var doneFunction = function (complete, error, progress) {\n"  \
    L"                promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n"  \
    L"                promise.done(complete, error, progress);\n"  \
    L"            };\n"  \
    L"    Object.preventExtensions(cancelFunction);\n"  \
    L"    var wrapper =\n"  \
    L"        Object.create(asyncOpWrapperPrototype, {\n"  \
    L"            operation: { writable: false, enumerable: true, configurable: false, value: op },\n"  \
    L"            then: { writable: false, enumerable: true, configurable: false, value: thenFunction },\n"  \
    L"            cancel: { writable: false, enumerable: true, configurable: false, value: cancelFunction },\n"  \
    L"            done: { writable: false, enumerable: true, configurable: false, value: doneFunction }\n"  \
    L"        });\n"  \
    L"    Object.preventExtensions(wrapper);\n"  \
    L"    return wrapper;\n"  \
    L"};\n"  \
    L"return AsyncOpWrapper;"