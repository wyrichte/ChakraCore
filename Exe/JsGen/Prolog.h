//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2011 by Microsoft Corporation.  All rights reserved.
//
// Runtime details of promises implementation
//----------------------------------------------------------------------------

#pragma once

#define JSGEN_PROLOG_BODY \
    L"(function () {\n" \
    L"var rootNamespace = this;\n" \
    L"var dp = null;\n" \
    L"var dpg = null;\n" \
    L"var eventParamOf = null;\n" \
    L"if (Object.defineProperty == undefined) {\n" \
    L"    var shadowableMembers = ['apply', 'call', 'hasOwnProperty', 'isPrototypeOf', 'propertyIsEnumerable', 'toLocalString', 'toString', 'valueOf'];\n" \
    L"    shadowableMembers.contains = function (name) {\n" \
    L"        for (var i = 0; i < shadowableMembers.length; i++) {\n" \
    L"            if (shadowableMembers[i] == name) { return true; }\n" \
    L"        }\n" \
    L"        return false;\n" \
    L"    };\n" \
    L"    dp = function (object, propertyname, value) {\n" \
    L"        if ((object[propertyname] != undefined) && (!shadowableMembers.contains(propertyname))) {\n" \
    L"            WScript.Echo('Attempted redefinition of property ' + propertyname);\n" \
    L"        }\n" \
    L"        object[propertyname] = value;\n" \
    L"    };\n" \
    L"    dpg = function (object, propertyname) {\n" \
    L"        if ((object[propertyname] != undefined) && (!shadowableMembers.contains(propertyname))) {\n" \
    L"            WScript.Echo('Attempted redefinition of property ' + propertyname);\n" \
    L"        }\n" \
    L"        object[propertyname] = new Object(); /* loss of Intellisense */\n" \
    L"    };\n" \
    L"} else {\n" \
    L"    dp = function (object, propertyname, value) {\n" \
    L"        Object.defineProperty(object, propertyname, { writable: false, enumerable: true, configurable: false, value: value });\n" \
    L"    };\n" \
    L"    dpg = function (object, propertyname, getter, setter) {\n" \
    L"        Object.defineProperty(object, propertyname, { enumerable: true, configurable: false, get: getter, set: setter });\n" \
    L"    };\n" \
    L"    var ensureNamespace = function (namespace) {\n" \
    L"        var fn;\n" \
    L"        while (fn = rootNamespace._$deferred[namespace]) {\n" \
    L"            rootNamespace._$deferred[namespace] = undefined;\n" \
    L"            fn();\n" \
    L"        }\n" \
    L"    };\n" \
    L"}\n" \
    L"\n" \
    L"var preventExtensions = Object.preventExtensions || function (object) { return object; };\n" \
    L"\n" \
    L"if(rootNamespace.intellisense) {\n" \
    L"    rootNamespace.intellisense.addEventListener('statementcompletion', function (ev) {\n" \
    L"        ev.items = ev.items.filter(function (item) {\n" \
    L"            return !(item.value && item.value._$hidden);\n" \
    L"        });\n" \
    L"    });\n" \
    L"}\n" \
    L"\n" \
    L"// --------------------------------------------------------------------------------------------------------------------------------\n" \
    L"// This defines the rules for special event parameter projection.\n" \
    L"// There are n parameters and the first two are named a and b.\n" \
    L"// The value of 'b', if it exists, is expected to be an array with two instances of the same type\n" \
    L"// --------------------------------------------------------------------------------------------------------------------------------\n" \
    L"eventParamOf = function (a, b) {\n" \
    L"    var result;\n" \
    L"    if (arguments.length < 2) {\n" \
    L"        result = new Object();\n" \
    L"    } else {\n" \
    L"        result = new Object(b[0]); // The 'b' parameter is doubled so that an unmodified version remains for 'detail' \n" \
    L"    }\n" \
    L"    result.target = a;\n" \
    L"    result.detail = [];\n" \
    L"    for (var i = 1; i < arguments.length; ++i) {\n" \
    L"        if (i == 1) {\n" \
    L"            result.detail[i - 1] = arguments[i][1]; // This is the second of the doubled 'b' parameter\n" \
    L"        } else {\n" \
    L"            result.detail[i - 1] = arguments[i];\n" \
    L"        }\n" \
    L"    }\n" \
    L"    result.type = '';\n" \
    L"    preventExtensions(result);\n" \
    L"    return result;\n" \
    L"}\n" \
    L"\n" \
    L"// --------------------------------------------------------------------------------------------------------------------------------\n" \
    L"// This begins the promise implementation\n" \
    L"// --------------------------------------------------------------------------------------------------------------------------------\n" \
    L"function doComplete(carrier, completeValue) {\n" \
    L"    if (carrier._state !== state_working) {\n" \
    L"        return;\n" \
    L"    }\n" \
    L"\n" \
    L"    if (typeof completeValue === 'object' && completeValue && typeof completeValue.then === 'function') {\n" \
    L"        carrier._state = state_waiting;\n" \
    L"\n" \
    L"        completeValue.then(\n" \
    L"        function (value) {\n" \
    L"            carrier._state = state_working;\n" \
    L"            carrier._value = value;\n" \
    L"            doComplete(carrier, value);\n" \
    L"        },\n" \
    L"        function (value) { carrier._state = state_working; doError(carrier, value); },\n" \
    L"        function (value) { doProgress(carrier, value); }\n" \
    L"    );\n" \
    L"\n" \
    L"    } else {\n" \
    L"        carrier._state = state_fulfilled_success;\n" \
    L"        doNotify(carrier);\n" \
    L"        carrier._cleanup();\n" \
    L"    }\n" \
    L"}\n" \
    L"\n" \
    L"function doError(carrier, errorValue) {\n" \
    L"    if (carrier._state !== state_working) {\n" \
    L"        return;\n" \
    L"    }\n" \
    L"\n" \
    L"    carrier._state = state_fulfilled_error;\n" \
    L"\n" \
    L"    doNotify(carrier);\n" \
    L"    carrier._cleanup();\n" \
    L"}\n" \
    L"\n" \
    L"function doProgress(carrier, progressValue) {\n" \
    L"    if (carrier._listeners) {\n" \
    L"        for (var i = 0, len = carrier._listeners.length; i < len; i++) {\n" \
    L"            var onProgress = carrier._listeners[i].onProgress;\n" \
    L"            try {\n" \
    L"                if (onProgress) {\n" \
    L"                    onProgress(progressValue);\n" \
    L"                }\n" \
    L"            } catch (e) {\n" \
    L"                // Swallow exception thrown from user progress handler\n" \
    L"            }\n" \
    L"        }\n" \
    L"    }\n" \
    L"}\n" \
    L"\n" \
    L"function notifySuccess(listeners, value) {\n" \
    L"    for (var i = 0, len = listeners.length; i < len; i++) {\n" \
    L"        var listener = listeners[i];\n" \
    L"        var carrier = listener.carrier;\n" \
    L"        var onComplete = listener.onComplete;\n" \
    L"        try {\n" \
    L"            if (onComplete) {\n" \
    L"                var result = onComplete(value);\n" \
    L"                if (carrier._state === state_working) {\n" \
    L"                    carrier._value = result;\n" \
    L"                }\n" \
    L"                doComplete(carrier, result);\n" \
    L"            } else {\n" \
    L"                if (carrier._state === state_working) {\n" \
    L"                    carrier._value = value;\n" \
    L"                }\n" \
    L"                doComplete(carrier, value);\n" \
    L"            }\n" \
    L"        } catch (exception) {\n" \
    L"            doError(carrier, exception);\n" \
    L"        }\n" \
    L"    }\n" \
    L"}\n" \
    L"function notifyError(listeners, value) {\n" \
    L"    for (var i = 0, len = listeners.length; i < len; i++) {\n" \
    L"        var listener = listeners[i];\n" \
    L"        var carrier = listener.carrier;\n" \
    L"        var onError = listener.onError;\n" \
    L"        try {\n" \
    L"            if (onError) {\n" \
    L"                doComplete(carrier, onError(value));\n" \
    L"            } else {\n" \
    L"                doError(carrier, value);\n" \
    L"            }\n" \
    L"        } catch (exception) {\n" \
    L"            doError(carrier, exception);\n" \
    L"        }\n" \
    L"    }\n" \
    L"}\n" \
    L"\n" \
    L"var state_working = 0;\n" \
    L"var state_waiting = 1;\n" \
    L"var state_fulfilled_error = 2;\n" \
    L"var state_fulfilled_success = 3;\n" \
    L"\n" \
    L"function doCancel(carrier) {\n" \
    L"    if (carrier._state === state_waiting) {\n" \
    L"        if (typeof carrier._value.cancel === 'function') {\n" \
    L"            carrier._value.cancel();\n" \
    L"        }\n" \
    L"    }\n" \
    L"    carrier._cleanup();\n" \
    L"}\n" \
    L"\n" \
    L"function doNotify(carrier) {\n" \
    L"    var listeners = carrier._listeners;\n" \
    L"    carrier._listeners = null;\n" \
    L"    if (listeners) {\n" \
    L"        notifySuccess(listeners, carrier._value);\n" \
    L"        notifyError(listeners, new Error());\n" \
    L"    }\n" \
    L"}\n" \
    L"\n" \
    L"\n" \
    L"function doThen(carrier, complete, error, progress, creatorPromise) {\n" \
    L"    carrier._listeners = carrier._listeners || [];\n" \
    L"    var p = createThenPromise(creatorPromise);\n" \
    L"\n" \
    L"    carrier._listeners.push({\n" \
    L"        promise: p.promise,\n" \
    L"        carrier: p.carrier,\n" \
    L"        onComplete: complete,\n" \
    L"        onError: error,\n" \
    L"        onProgress: progress\n" \
    L"    });\n" \
    L"\n" \
    L"    var progressInfo = 100;\n" \
    L"    doProgress(carrier, progressInfo);\n" \
    L"    if (carrier._state > state_waiting) {\n" \
    L"        doNotify(carrier);\n" \
    L"    }\n" \
    L"    return p.promise;\n" \
    L"}\n" \
    L"function isExceptionButNotCanceled(err) {\n"  \
    L"    return err instanceof Error && err.message !== 'Canceled';\n" \
    L"}\n"  \
    L"function postError(err) {\n"  \
    L"    if (isExceptionButNotCanceled(err)) {\n" \
    L"        setTimeout(function() {throw err;}, 0);\n" \
    L"    }\n" \
    L"}\n"  \
    L"function doDone(carrier, complete, error, progress, creatorPromise) {\n"  \
    L"    if (carrier._state === state_fulfilled_success) {\n" \
    L"        if (complete) {\n" \
    L"            complete(carrier._value);\n" \
    L"        }\n" \
    L"        return;\n" \
    L"    }\n" \
    L"    if (carrier._state === state_fulfilled_error) {\n" \
    L"        if (error) {\n" \
    L"            error(carrier._value);\n" \
    L"            return;\n" \
    L"        }\n" \
    L"        if (isExceptionButNotCanceled(carrier._value)) {\n" \
    L"            throw carrier._value;\n" \
    L"        }\n" \
    L"        return;\n" \
    L"    }\n" \
    L"    doThen(carrier, complete, error, progress, creatorPromise)\n" \
    L"    .then(null, postError, null);\n" \
    L"}\n"  \
    L"\n" \
    L"var AsyncOpPromise = function (op) {\n" \
    L"    var that = this;\n" \
    L"    var carrier = {\n" \
    L"        _listeners: null,\n" \
    L"        _state: state_working,\n" \
    L"        _value: null,\n" \
    L"        _cleanup: function () { op = null; }\n" \
    L"    };\n" \
    L"\n" \
    L"    this.cancel = function () {\n" \
    L"        if (op) {\n" \
    L"            op.cancel();\n" \
    L"        }\n" \
    L"        doCancel(carrier);\n" \
    L"    };\n" \
    L"\n" \
    L"    this.then = function (complete, error, progress) {\n" \
    L"        return doThen(carrier, complete, error, progress, that);\n" \
    L"    };\n" \
    L"\n" \
    L"    this.done = function (complete, error, progress) {\n"  \
    L"        doDone(carrier, complete, error, progress, that);\n"  \
    L"    };\n"  \
    L"\n" \
    L"    var result = op.getResults();\n" \
    L"    if (carrier._state === state_working) {\n" \
    L"        carrier._value = result;\n" \
    L"    }\n" \
    L"    doComplete(carrier, result);\n" \
    L"\n" \
    L"};\n" \
    L"AsyncOpPromise.prototype = {};\n" \
    L"\n" \
    L"var thenPromisePrototype = {};\n" \
    L"preventExtensions(thenPromisePrototype);\n" \
    L"var createThenPromise = function (creator) {\n" \
    L"    var carrier = {\n" \
    L"        _listeners: null,\n" \
    L"        _state: state_working,\n" \
    L"        _value: null,\n" \
    L"        _cleanup: function () { creator = null; }\n" \
    L"    };\n" \
    L"    var thenPromise =\n" \
    L"    Object.create(thenPromisePrototype, {\n" \
    L"        then: { writable: false, enumerable: true, configurable: false, value: function (complete, error, progress) {\n" \
    L"            return doThen(carrier, complete, error, progress, this);\n" \
    L"        }\n" \
    L"        },\n" \
    L"        cancel: { writable: false, enumerable: true, configurable: false, value: function () {\n" \
    L"            if (creator) {\n" \
    L"                // When we are canceled we need to propagate that up the chain.\n" \
    L"                creator.cancel();\n" \
    L"            }\n" \
    L"            doCancel(carrier);\n" \
    L"        }\n" \
    L"        },\n"  \
    L"        done: { writable: false, enumerable: true, configurable: false, value: function (complete, error, progress) {\n"  \
    L"            doDone(carrier, complete, error, progress, this);\n"  \
    L"        }\n"  \
    L"        }\n" \
    L"    });\n" \
    L"    preventExtensions(thenPromise);\n" \
    L"    return { promise: thenPromise, carrier: carrier }; // Carrier transmits private listener information\n" \
    L"};\n" \
    L"\n" \
    L"var asyncOpWrapperPrototype = {}\n" \
    L"preventExtensions(asyncOpWrapperPrototype);\n" \
    L"function AsyncOpWrapper(op) {\n" \
    L"    var promise = null;\n" \
    L"    var thenFunction = function (complete, error, progress) {\n" \
    L"        promise = promise || new AsyncOpPromise(this.operation);\n" \
    L"        return promise.then(complete, error, progress);\n" \
    L"    };\n" \
    L"    preventExtensions(thenFunction);\n" \
    L"    var cancelFunction = function () {\n" \
    L"        promise = promise || new AsyncOpPromise(this.operation);\n" \
    L"        promise.cancel();\n" \
    L"    };\n" \
    L"    var doneFunction = function (complete, error, progress) {\n"  \
    L"                promise = promise || new AsyncOpPromise(this.operation);\n"  \
    L"                promise.done(complete, error, progress);" \
    L"            };\n"  \
    L"    preventExtensions(cancelFunction);\n" \
    L"    var wrapper =\n" \
    L"    Object.create(asyncOpWrapperPrototype, {\n" \
    L"        operation: { writable: false, enumerable: true, configurable: false, value: op },\n" \
    L"        then: { writable: false, enumerable: true, configurable: false, value: thenFunction },\n" \
    L"        cancel: { writable: false, enumerable: true, configurable: false, value: cancelFunction },\n" \
    L"        done: { writeable: false, enumerable: true, configurable: false, value: doneFunction }\n" \
    L"    });\n" \
    L"    preventExtensions(wrapper);\n" \
    L"    return wrapper;\n" \
    L"};\n" \

