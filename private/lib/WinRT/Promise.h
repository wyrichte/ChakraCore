//---------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
// Runtime details of promises implementation
//----------------------------------------------------------------------------

#pragma once

#define PROJECTION_PROMISE_BODY \
    _u("// Copyright (c) Microsoft Corporation.\n")  \
    _u("// All rights reserved.\n")  \
    _u("function AsyncBehaviorError(message, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    this.name = 'AsyncBehaviorError';\n")  \
    _u("    this.message = (message || '');\n")  \
    _u("    this.asyncOpType = asyncOpType;\n")  \
    _u("    this.asyncOpSource = asyncOpSource;\n")  \
    _u("    this.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("}\n")  \
    _u("AsyncBehaviorError.prototype = Object.create(Error.prototype);\n")  \
    _u("AsyncBehaviorError.prototype.constructor = AsyncBehaviorError;\n")  \
    _u("\n")  \
    _u("function doComplete(carrier, completeValue) {\n")  \
    _u("    if (carrier._state !== state_working) {\n")  \
    _u("        return;\n")  \
    _u("    }\n")  \
    _u("\n")  \
    _u("    carrier._value = completeValue;\n")  \
    _u("\n")  \
    _u("    if (typeof completeValue === 'object' && completeValue && typeof completeValue.then === 'function') {\n")  \
    _u("        carrier._state = state_waiting;\n")  \
    _u("\n")  \
    _u("        var setAsyncOpCausalityId = function (id) { carrier._asyncOpCausalityId = id; };\n")  \
    _u("        var complete = function(value) { carrier._state = state_working; doComplete(carrier, value); };\n")  \
    _u("        complete.setAsyncOpCausalityId = setAsyncOpCausalityId;\n")  \
    _u("        var error = function(value) { carrier._state = state_working; doError(carrier, value); };\n")  \
    _u("        error.setAsyncOpCausalityId = setAsyncOpCausalityId;\n")  \
    _u("        var progress = function (value) { doProgress(carrier, value); };\n")  \
    _u("        progress.setAsyncOpCausalityId = setAsyncOpCausalityId;\n")  \
    _u("\n")  \
    _u("        completeValue.then(complete, error, progress);\n")  \
    _u("    } else {\n")  \
    _u("        carrier._state = state_fulfilled_success;\n")  \
    _u("        doNotify(carrier);\n")  \
    _u("        carrier._cleanup();\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function doError(carrier, errorValue) {\n")  \
    _u("    if (carrier._state !== state_working) {\n")  \
    _u("        return;\n")  \
    _u("    }\n")  \
    _u("\n")  \
    _u("    carrier._value = errorValue;\n")  \
    _u("    carrier._state = state_fulfilled_error;\n")  \
    _u("\n")  \
    _u("    doNotify(carrier);\n")  \
    _u("\n")  \
    _u("    carrier._cleanup();\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function doProgress(carrier, progressValue) {\n")  \
    _u("    (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("    if (carrier._state > state_waiting) {\n")  \
    _u("        return;\n")  \
    _u("    }\n")  \
    _u("    if (carrier._listeners) {\n")  \
    _u("        for (var i = 0, len = carrier._listeners.length; i < len; i++) {\n")  \
    _u("            var onProgress = carrier._listeners[i].onProgress;\n")  \
    _u("            var traceAsync = true;\n")  \
    _u("            if (onProgress && typeof onProgress === 'function') {\n")  \
    _u("                if (onProgress.setAsyncOpCausalityId && typeof onProgress.setAsyncOpCausalityId === 'function') {\n")  \
    _u("                    traceAsync = false;\n")  \
    _u("                    onProgress.setAsyncOpCausalityId(carrier._asyncOpCausalityId);\n")  \
    _u("                }\n")  \
    _u("                if (traceAsync) {\n")  \
    _u("                    try {\n")  \
    _u("                        traceAsyncCallbackStart(carrier._asyncOpCausalityId, work_type_progress, log_level_verbose);\n")  \
    _u("                        onProgress(progressValue);\n")  \
    _u("                    } catch (e) {\n")  \
    _u("                        updateAsyncCallbackRelation(carrier._asyncOpCausalityId, callback_status_error, log_level_verbose);\n")  \
    _u("                    } finally {\n")  \
    _u("                        traceAsyncCallbackComplete(carrier._asyncOpCausalityId, log_level_verbose);\n")  \
    _u("                    }\n")  \
    _u("                } else {\n")  \
    _u("                    try {\n")  \
    _u("                        onProgress(progressValue);\n")  \
    _u("                    } catch (e) {\n")  \
    _u("                    }\n")  \
    _u("                }\n")  \
    _u("            }\n")  \
    _u("        }\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function notifySuccess(listeners, value, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("    for (var i = 0, len = listeners.length; i < len; i++) {\n")  \
    _u("        var listener = listeners[i];\n")  \
    _u("        var carrier = listener.carrier;\n")  \
    _u("        var onComplete = listener.onComplete;\n")  \
    _u("        var callbackCompleted = false;\n")  \
    _u("        var traceAsync = true;\n")  \
    _u("        var onCompletePresent = onComplete && typeof onComplete === 'function';\n")  \
    _u("        if (onCompletePresent && onComplete.setAsyncOpCausalityId && typeof onComplete.setAsyncOpCausalityId === 'function') {\n")  \
    _u("            traceAsync = false;\n")  \
    _u("            onComplete.setAsyncOpCausalityId(asyncOpCausalityId);\n")  \
    _u("        }\n")  \
    _u("        try {\n")  \
    _u("            var temp = value;\n")  \
    _u("            if (onCompletePresent) {\n")  \
    _u("                if (traceAsync) {\n")  \
    _u("                    traceAsyncCallbackStart(asyncOpCausalityId, work_type_completion, log_level_required);\n")  \
    _u("                }\n")  \
    _u("                temp = onComplete(value);\n")  \
    _u("                if (traceAsync) {\n")  \
    _u("                    traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n")  \
    _u("                }\n")  \
    _u("                callbackCompleted = true;\n")  \
    _u("            }\n")  \
    _u("            doComplete(carrier, temp);\n")  \
    _u("        } catch (exception) {\n")  \
    _u("            if (onCompletePresent && traceAsync === true && callbackCompleted === false) {\n")  \
    _u("                updateAsyncCallbackRelation(asyncOpCausalityId, callback_status_error, log_level_verbose);\n")  \
    _u("                traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n")  \
    _u("            }\n")  \
    _u("            if (!exception.asyncOpType) {\n")  \
    _u("                exception.originatedFromHandler = true;\n")  \
    _u("                exception.asyncOpType = asyncOpType;\n")  \
    _u("                exception.asyncOpSource = asyncOpSource;\n")  \
    _u("                exception.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("            }\n")  \
    _u("            doError(carrier, exception);\n")  \
    _u("        }\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function notifyError(listeners, value, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("    for (var i = 0, len = listeners.length; i < len; i++) {\n")  \
    _u("        var listener = listeners[i];\n")  \
    _u("        var carrier = listener.carrier;\n")  \
    _u("        var onError = listener.onError;\n")  \
    _u("        var callbackCompleted = false;\n")  \
    _u("        var traceAsync = true;\n")  \
    _u("        var onErrorPresent = onError && typeof onError === 'function';\n")  \
    _u("        if (onErrorPresent && onError.setAsyncOpCausalityId && typeof onError.setAsyncOpCausalityId === 'function') {\n")  \
    _u("            traceAsync = false;\n")  \
    _u("            onError.setAsyncOpCausalityId(asyncOpCausalityId);\n")  \
    _u("        }\n")  \
    _u("        try {\n")  \
    _u("            if (onErrorPresent) {\n")  \
    _u("                if (traceAsync) {\n")  \
    _u("                    traceAsyncCallbackStart(asyncOpCausalityId, work_type_completion, log_level_required);\n")  \
    _u("                }\n")  \
    _u("                var temp = onError(value);\n")  \
    _u("                if (traceAsync) {\n")  \
    _u("                    traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n")  \
    _u("                }\n")  \
    _u("                callbackCompleted = true;\n")  \
    _u("                doComplete(carrier, temp);\n")  \
    _u("            } else {\n")  \
    _u("                doError(carrier, value);\n")  \
    _u("            }\n")  \
    _u("        } catch (exception) {\n")  \
    _u("            if (onErrorPresent && traceAsync === true && callbackCompleted === false) {\n")  \
    _u("                updateAsyncCallbackRelation(asyncOpCausalityId, callback_status_error, log_level_verbose);\n")  \
    _u("                traceAsyncCallbackComplete(asyncOpCausalityId, log_level_required);\n")  \
    _u("            }\n")  \
    _u("            if (!exception.asyncOpType) {\n")  \
    _u("                exception.originatedFromHandler = true;\n")  \
    _u("                exception.asyncOpType = asyncOpType;\n")  \
    _u("                exception.asyncOpSource = asyncOpSource;\n")  \
    _u("                exception.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("            }\n")  \
    _u("            doError(carrier, exception);\n")  \
    _u("        }\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("var state_working = 0;\n")  \
    _u("var state_waiting = 1;\n")  \
    _u("var state_fulfilled_error = 2;\n")  \
    _u("var state_fulfilled_success = 3;\n")  \
    _u("var log_level_required = 0;\n")  \
    _u("var log_level_important = 1;\n")  \
    _u("var log_level_verbose = 2;\n")  \
    _u("var work_type_completion = 0;\n")  \
    _u("var work_type_progress = 1;\n")  \
    _u("var work_type_operation = 2;\n")  \
    _u("var invalid_async_operation_id = 0;\n")  \
    _u("var op_status_success = 1;\n")  \
    _u("var op_status_canceled = 2;\n")  \
    _u("var op_status_error = 3;\n")  \
    _u("var callback_status_assign_delegate = 0;\n")  \
    _u("var callback_status_join = 1;\n")  \
    _u("var callback_status_cancel = 3;\n")  \
    _u("var callback_status_error = 4;\n")  \
    _u("var traceAsyncCallbackStart = function() { };\n")  \
    _u("var traceAsyncOperationComplete = function() { };\n")  \
    _u("var updateAsyncCallbackRelation = function() { };\n")  \
    _u("var debug_msTraceAsyncCallbackCompleted = function() { };\n")  \
    _u("var debug = Debug;\n")  \
    _u("if (Debug != undefined) {\n")  \
    _u("    op_status_success = Debug.MS_ASYNC_OP_STATUS_SUCCESS || 1;\n")  \
    _u("    op_status_canceled = Debug.MS_ASYNC_OP_STATUS_CANCELED || 2;\n")  \
    _u("    op_status_error = Debug.MS_ASYNC_OP_STATUS_ERROR || 3;\n")  \
    _u("    callback_status_assign_delegate = Debug.MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE || 0;\n")  \
    _u("    callback_status_join = Debug.MS_ASYNC_CALLBACK_STATUS_JOIN || 1;\n")  \
    _u("    callback_status_cancel = Debug.MS_ASYNC_CALLBACK_STATUS_CANCEL || 3;\n")  \
    _u("    callback_status_error = Debug.MS_ASYNC_CALLBACK_STATUS_ERROR || 4;\n")  \
    _u("    if (Debug.msTraceAsyncCallbackStarting && typeof Debug.msTraceAsyncCallbackStarting === 'function') {\n")  \
    _u("        traceAsyncCallbackStart = Debug.msTraceAsyncCallbackStarting;\n")  \
    _u("    }\n")  \
    _u("    if (Debug.msTraceAsyncOperationCompleted && typeof Debug.msTraceAsyncOperationCompleted === 'function') {\n")  \
    _u("        traceAsyncOperationComplete = Debug.msTraceAsyncOperationCompleted;\n")  \
    _u("    }\n")  \
    _u("    if (Debug.msTraceAsyncCallbackCompleted && typeof Debug.msTraceAsyncCallbackCompleted === 'function') {\n")  \
    _u("        debug_msTraceAsyncCallbackCompleted = Debug.msTraceAsyncCallbackCompleted;\n")  \
    _u("    }\n")  \
    _u("    if (Debug.msUpdateAsyncCallbackRelation && typeof Debug.msUpdateAsyncCallbackRelation === 'function') {\n")  \
    _u("        updateAsyncCallbackRelation = Debug.msUpdateAsyncCallbackRelation;\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("function traceAsyncCallbackComplete(asyncOpCausalityId, logLevel) {\n")  \
    _u("    if (invalid_async_operation_id != asyncOpCausalityId) {\n")  \
    _u("        debug_msTraceAsyncCallbackCompleted(logLevel);\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function doCancel(carrier) {\n")  \
    _u("    if (carrier._state === state_waiting) {\n")  \
    _u("        if (typeof carrier._value.cancel === 'function') {\n")  \
    _u("            carrier._value.cancel();\n")  \
    _u("        }\n")  \
    _u("    }\n")  \
    _u("    carrier._cleanup();\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function doNotify(carrier) {\n")  \
    _u("    var listeners = carrier._listeners;\n")  \
    _u("    carrier._listeners = [];\n")  \
    _u("    if (listeners && (listeners.length > 0)) {\n")  \
    _u("        switch (carrier._state) {\n")  \
    _u("            case state_fulfilled_success:\n")  \
    _u("                notifySuccess(listeners, carrier._value, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);\n")  \
    _u("                break;\n")  \
    _u("\n")  \
    _u("            case state_fulfilled_error:\n")  \
    _u("                notifyError(listeners, carrier._value, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);\n")  \
    _u("                break;\n")  \
    _u("        }\n")  \
    _u("    }\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function doThen(carrier, complete, error, progress, creatorPromise) {\n")  \
    _u("    carrier._listeners = carrier._listeners || [];\n")  \
    _u("\n")  \
    _u("    var p = createThenPromise(creatorPromise, carrier._asyncOpType, carrier._asyncOpSource, carrier._asyncOpCausalityId);\n")  \
    _u("\n")  \
    _u("    carrier._listeners.push({\n")  \
    _u("        promise: p.promise,\n")  \
    _u("        carrier: p.carrier,\n")  \
    _u("        onComplete: complete,\n")  \
    _u("        onError: error,\n")  \
    _u("        onProgress: progress\n")  \
    _u("    });\n")  \
    _u("\n")  \
    _u("    if (carrier._state > state_waiting) {\n")  \
    _u("        doNotify(carrier);\n")  \
    _u("    }\n")  \
    _u("    return p.promise;\n")  \
    _u("}\n")  \
    _u("function isCanceled(err) {\n")  \
    _u("    return err instanceof Error && err.name === 'Canceled';\n")  \
    _u("}\n")  \
    _u("function createDebugStack(err) {\n")  \
    _u("    var stack = [];\n")  \
    _u("    stack.push('WinRT Promise.done()');\n")  \
    _u("    if (err && err.originatedFromHandler) {\n")  \
    _u("        stack.push('Promise handler');\n")  \
    _u("    }\n")  \
    _u("    if (err && err.asyncOpType) {\n")  \
    _u("        if (err.originatedFromHandler) {\n")  \
    _u("            stack.push('WinRT Promise object for ' + err.asyncOpType);\n")  \
    _u("        } else {\n")  \
    _u("            stack.push(err.asyncOpType);\n")  \
    _u("        }\n")  \
    _u("    } else {\n")  \
    _u("        stack.push('WinRT Promise object');\n")  \
    _u("    }\n")  \
    _u("\n")  \
    _u("    var s = 'var o = {};';\n")  \
    _u("    var wrap = function (n) {\n")  \
    _u("        return '[\"' + n + '\"]';\n")  \
    _u("    }\n")  \
    _u("\n")  \
    _u("    for (var i = 1; i < stack.length; i++) {\n")  \
    _u("        s += 'o' + wrap(stack[i]) + ' = function () { o' + wrap(stack[i - 1]) + '(); };';\n")  \
    _u("    }\n")  \
    _u("    s += 'o' + wrap(stack[0]) + ' = function () { rethrowPromiseError(); };';\n")  \
    _u("    s += 'o' + wrap(stack[stack.length - 1]);\n")  \
    _u("    return s;\n")  \
    _u("}\n")  \
    _u("function postError(err) {\n")  \
    _u("    if (isCanceled(err)) {\n")  \
    _u("        return;\n")  \
    _u("    }\n")  \
    _u("    var s = createDebugStack(err);\n")  \
    _u("    function rethrowPromiseError() {\n")  \
    _u("        throw err;\n")  \
    _u("    }\n")  \
    _u("    setTimeout(eval(s), 0);\n")  \
    _u("}\n")  \
    _u("function doDone(carrier, complete, error, progress, creatorPromise) {\n")  \
    _u("    if (carrier._state === state_fulfilled_success) {\n")  \
    _u("        if (complete) {\n")  \
    _u("            try {\n")  \
    _u("                traceAsyncCallbackStart(carrier._asyncOpCausalityId, work_type_completion, log_level_required);\n")  \
    _u("                complete(carrier._value);\n")  \
    _u("            } finally {\n")  \
    _u("                traceAsyncCallbackComplete(carrier._asyncOpCausalityId, log_level_required);\n")  \
    _u("            }\n")  \
    _u("        }\n")  \
    _u("        return;\n")  \
    _u("    }\n")  \
    _u("    if (carrier._state === state_fulfilled_error) {\n")  \
    _u("        if (error) {\n")  \
    _u("            try {\n")  \
    _u("                traceAsyncCallbackStart(carrier._asyncOpCausalityId, work_type_completion, log_level_required);\n")  \
    _u("                error(carrier._value);\n")  \
    _u("                return;\n")  \
    _u("            } finally {\n")  \
    _u("                traceAsyncCallbackComplete(carrier._asyncOpCausalityId, log_level_required);\n")  \
    _u("            }\n")  \
    _u("        }\n")  \
    _u("        if (!isCanceled(carrier._value)) {\n")  \
    _u("            postError(carrier._value);\n")  \
    _u("        }\n")  \
    _u("        return;\n")  \
    _u("    }\n")  \
    _u("    doThen(carrier, complete, error, progress, creatorPromise)\n")  \
    _u("    .then(null, postError, null);\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function getResultsOfAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("    return op.getResults();\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("function cancelAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("    return op.cancel();\n")  \
    _u("}\n")  \
    _u("\n")  \
    _u("var AsyncOpPromise = function (op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    var that = this;\n")  \
    _u("    var carrier = {\n")  \
    _u("        _listeners: null,\n")  \
    _u("        _state: state_working,\n")  \
    _u("        _value: null,\n")  \
    _u("        _cleanup: function () { op = null; },\n")  \
    _u("        _asyncOpType: asyncOpType,\n")  \
    _u("        _asyncOpSource: asyncOpSource,\n")  \
    _u("        _asyncOpCausalityId: asyncOpCausalityId\n")  \
    _u("    };\n")  \
    _u("\n")  \
    _u("    this.cancel = function () {\n")  \
    _u("        (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("        try {\n")  \
    _u("            if (op) {\n")  \
    _u("                cancelAsyncOp(op, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("            }\n")  \
    _u("            doCancel(carrier);\n")  \
    _u("        } catch (exception) {\n")  \
    _u("            exception.asyncOpType = asyncOpType;\n")  \
    _u("            exception.asyncOpSource = asyncOpSource;\n")  \
    _u("            exception.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("            doError(carrier, exception);\n")  \
    _u("        }\n")  \
    _u("    };\n")  \
    _u("\n")  \
    _u("    this.then = function (complete, error, progress) {\n")  \
    _u("        return doThen(carrier, complete, error, progress, that);\n")  \
    _u("    };\n")  \
    _u("\n")  \
    _u("    this.done = function (complete, error, progress) {\n")  \
    _u("        doDone(carrier, complete, error, progress, that);\n")  \
    _u("    };\n")  \
    _u("\n")  \
    _u("    var progressProperty = Object.getOwnPropertyDescriptor(Object.getPrototypeOf(op), 'progress');\n")  \
    _u("    if (progressProperty) {\n")  \
    _u("        if ('set' in progressProperty) {\n")  \
    _u("            op.progress = function (op2, progressInfo) {\n")  \
    _u("                doProgress(carrier, progressInfo);\n")  \
    _u("            };\n")  \
    _u("        };\n")  \
    _u("    }\n")  \
    _u("\n")  \
    _u("    op.completed = function (op2, status) {\n")  \
    _u("        (Debug && (Debug.setNonUserCodeExceptions = true));\n")  \
    _u("        if (!op2 || !op2.operation) {\n")  \
    _u("            if (carrier._state <= state_waiting) {\n")  \
    _u("                traceAsyncOperationComplete(asyncOpCausalityId, op_status_error, log_level_required);\n")  \
    _u("            }\n")  \
    _u("\n")  \
    _u("            var invalidSenderArgMsg = errorStrings[2];\n")  \
    _u("            doError(carrier, new AsyncBehaviorError(invalidSenderArgMsg, asyncOpType, asyncOpSource, asyncOpCausalityId));\n")  \
    _u("            return;\n")  \
    _u("        }\n")  \
    _u("        switch (status) {\n")  \
    _u("            case op_status_success:\n")  \
    _u("                if (carrier._state <= state_waiting) {\n")  \
    _u("                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_success, log_level_required);\n")  \
    _u("                }\n")  \
    _u("\n")  \
    _u("                var result;\n")  \
    _u("                try {\n")  \
    _u("                    result = getResultsOfAsyncOp(op2.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("                    doComplete(carrier, result);\n")  \
    _u("                } catch(exception) {\n")  \
    _u("                    exception.asyncOpType = asyncOpType;\n")  \
    _u("                    exception.asyncOpSource = asyncOpSource;\n")  \
    _u("                    exception.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("                    doError(carrier, exception);\n")  \
    _u("                }\n")  \
    _u("\n")  \
    _u("                break;\n")  \
    _u("\n")  \
    _u("            case op_status_error:\n")  \
    _u("                if (carrier._state <= state_waiting) {\n")  \
    _u("                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_error, log_level_required);\n")  \
    _u("                }\n")  \
    _u("\n")  \
    _u("                var error;\n")  \
    _u("                try {\n")  \
    _u("                    getResultsOfAsyncOp(op2.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("                    var noErrorInErrorStateMsg = errorStrings[0];\n")  \
    _u("                    error = new AsyncBehaviorError(noErrorInErrorStateMsg, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("                } catch (exception) {\n")  \
    _u("                    exception.asyncOpType = asyncOpType;\n")  \
    _u("                    exception.asyncOpSource = asyncOpSource;\n")  \
    _u("                    exception.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("                    error = exception;\n")  \
    _u("                }\n")  \
    _u("                doError(carrier, error);\n")  \
    _u("\n")  \
    _u("                break;\n")  \
    _u("\n")  \
    _u("            case op_status_canceled:\n")  \
    _u("                if (carrier._state <= state_waiting) {\n")  \
    _u("                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_canceled, log_level_required);\n")  \
    _u("                }\n")  \
    _u("\n")  \
    _u("                var err = new Error('Canceled');\n")  \
    _u("                err.name = err.message;\n")  \
    _u("                err.asyncOpType = asyncOpType;\n")  \
    _u("                err.asyncOpSource = asyncOpSource;\n")  \
    _u("                err.asyncOpCausalityId = asyncOpCausalityId;\n")  \
    _u("                doError(carrier, err);\n")  \
    _u("\n")  \
    _u("                break;\n")  \
    _u("\n")  \
    _u("            default:\n")  \
    _u("                if (carrier._state <= state_waiting) {\n")  \
    _u("                    traceAsyncOperationComplete(asyncOpCausalityId, op_status_error, log_level_required);\n")  \
    _u("                }\n")  \
    _u("\n")  \
    _u("                var invalidStatusArgMsg = errorStrings[1];\n")  \
    _u("                doError(carrier, new AsyncBehaviorError(invalidStatusArgMsg, asyncOpType, asyncOpSource, asyncOpCausalityId));\n")  \
    _u("        }\n")  \
    _u("    };\n")  \
    _u("};\n")  \
    _u("AsyncOpPromise.prototype = {};\n")  \
    _u("\n")  \
    _u("var thenPromisePrototype = {};\n")  \
    _u("Object.preventExtensions(thenPromisePrototype);\n")  \
    _u("var createThenPromise = function (creator, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    var carrier = {\n")  \
    _u("        _listeners: null,\n")  \
    _u("        _state: state_working,\n")  \
    _u("        _value: null,\n")  \
    _u("        _cleanup: function () { creator = null; },\n")  \
    _u("        _asyncOpType: asyncOpType,\n")  \
    _u("        _asyncOpSource: asyncOpSource,\n")  \
    _u("        _asyncOpCausalityId: asyncOpCausalityId\n")  \
    _u("    };\n")  \
    _u("    var thenPromise =\n")  \
    _u("        Object.create(thenPromisePrototype, {\n")  \
    _u("            then: { writable: false, enumerable: true, configurable: false, value: function (complete, error, progress) {\n")  \
    _u("                return doThen(carrier, complete, error, progress, this);\n")  \
    _u("            }\n")  \
    _u("            },\n")  \
    _u("            cancel: { writable: false, enumerable: true, configurable: false, value: function () {\n")  \
    _u("                if (creator) {\n")  \
    _u("                    creator.cancel();\n")  \
    _u("                }\n")  \
    _u("                doCancel(carrier);\n")  \
    _u("            }\n")  \
    _u("            },\n")  \
    _u("            done: { writable: false, enumerable: true, configurable: false, value: function (complete, error, progress) {\n")  \
    _u("                doDone(carrier, complete, error, progress, this);\n")  \
    _u("            }\n")  \
    _u("            }\n")  \
    _u("        });\n")  \
    _u("    Object.preventExtensions(thenPromise);\n")  \
    _u("    return { promise: thenPromise, carrier: carrier };\n")  \
    _u("};\n")  \
    _u("\n")  \
    _u("var asyncOpWrapperPrototype = {}\n")  \
    _u("Object.preventExtensions(asyncOpWrapperPrototype);\n")  \
    _u("function AsyncOpWrapper(op, asyncOpType, asyncOpSource, asyncOpCausalityId) {\n")  \
    _u("    var promise = null;\n")  \
    _u("    var thenFunction = function (complete, error, progress) {\n")  \
    _u("                promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("                return promise.then(complete, error, progress);\n")  \
    _u("            };\n")  \
    _u("    Object.preventExtensions(thenFunction);\n")  \
    _u("    var cancelFunction = function () {\n")  \
    _u("                promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("                promise.cancel();\n")  \
    _u("            };\n")  \
    _u("    var doneFunction = function (complete, error, progress) {\n")  \
    _u("                promise = promise || new AsyncOpPromise(this.operation, asyncOpType, asyncOpSource, asyncOpCausalityId);\n")  \
    _u("                promise.done(complete, error, progress);\n")  \
    _u("            };\n")  \
    _u("    Object.preventExtensions(cancelFunction);\n")  \
    _u("    var wrapper =\n")  \
    _u("        Object.create(asyncOpWrapperPrototype, {\n")  \
    _u("            operation: { writable: false, enumerable: true, configurable: false, value: op },\n")  \
    _u("            then: { writable: false, enumerable: true, configurable: false, value: thenFunction },\n")  \
    _u("            cancel: { writable: false, enumerable: true, configurable: false, value: cancelFunction },\n")  \
    _u("            done: { writable: false, enumerable: true, configurable: false, value: doneFunction }\n")  \
    _u("        });\n")  \
    _u("    Object.preventExtensions(wrapper);\n")  \
    _u("    return wrapper;\n")  \
    _u("};\n")  \
    _u("return AsyncOpWrapper;")