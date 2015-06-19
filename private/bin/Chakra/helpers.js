var _$lg_disabled = true;
var _$lg_limit = 1000;
var _$eventListener_limit = 100;
var _$containerList = {};
var _$evaltmp;

function _$protectedEval(exp) { return eval(exp); }
(function () { this["?"] = false; })();

Object.preventExtensions = function (o) { return o; };
Object.seal = function (o) { return o; };
Object.freeze = function (o) { return o; };

// Creating a proxy that capture all property reads
function _$createCapturingProxyHolder() {
    var capture =
    {
        properties: []
    };

    var get = function (target, propertyName, proxy) {
        capture.properties.push(propertyName);

        // Returing true maximize the chance some branches are executed based on property checks
        return true;
    }

    capture.get = get;

    capture.proxy = new Proxy({}, capture)
    return capture;
}

// Define properties as non-enumerable to prevent them from interfering with user code
function _$defineProperty(obj, name, value) {
    if ((typeof obj === 'object' || typeof obj === 'function') && obj)
        return Object.defineProperty(obj, name, {
            value: value,
            enumerable: false,
            writable: true,
            configurable: true
        });
    return obj;
}
function _$defineProperties(obj) {
    if (obj) {
        for (var i = 1; i < arguments.length; i++) {
            var a = arguments[i];
            if (a) {
                for (var v in a)
                    _$defineProperty(obj, v, a[v]);
            }
        }
    }
    return obj;
}

function _$isNullOrUndefined(o) {
    return (typeof o === 'undefined' || o === null);
}

function _$executeGetter(o, p) {
    return o[p];
}

function _$executeSetter(o, p, v) {
    o[p] = v;
}

var _$propertyCompletionItem = {};
Object.defineProperty(_$propertyCompletionItem, 'value', {
    get: function () {
        return (this.parentObject !== undefined) ? this.parentObject[this.name] : undefined;
    },
    set: function (v) {
        if (this.parentObject !== undefined) {
            this.parentObject[this.name] = v;
        }
    },
    enumerable: true,
    configurable: false
});

// Doc Comments rewrite utilities

var _$value;
var _$elementValue;
var _$createDomObject;
var _$paramValue;
var _$getInstanceByType;
var _$getInstanceByTypeString;
(function () {
    var typed = {
        Int8Array: Int8Array,
        Int16Array: Int16Array,
        Int32Array: Int32Array,
        Uint8Array: Uint8Array,
        Uint16Array: Uint16Array,
        Uint32Array: Uint32Array,
        Float32Array: Float32Array,
        Float64Array: Float64Array
    };

    function ctorOf(ctor, type) {
        return ctor || (type && _$protectedEval(type));
    }

    _$getInstanceByType = function (ctor) {
        var result = ctor._$instanceProxies;
        if (!result)
        {
            // In a scenario like this:
            //
            // function Foo()
            // {
            //     ///<field name="bar" type="Foo" mayBeNull="true"></field> 
            //     this.bar = null;
            // }
            // 
            // Foo.prototype.test = function () { 
            //   this./**ml:bar**/
            // } 
            // 
            // JSLS will rewrite the code to something like this to support intellisense
            //
            // function Foo()
            // {
            //     ///<field name="bar" type="Foo" mayBeNull="true"></field> 
            //     this.bar = null;
            //     this.bar = _$getInstanceByType(Foo);
            // }
            // 
            // Foo.prototype.test = function () { 
            //   this./**ml:bar**/
            // } 
            //
            // It is important that _$getInstanceByType(Foo) will not call Foo() again, 
            // otherwise that will lead to infinite recursion.
            //
            // The proxy without target functionality is developed to support that, it 
            // make sure Foo() is not called during construction, and this.bar will
            // be a proxy to this which will give the same set of properties for 
            // intellisense.
            //
            var myProxy = _$createProxyWithoutTarget();
            ctor._$instanceProxies = myProxy;
            try {
                result = new ctor();
            }
            finally {
                ctor._$instanceProxies = null;
            }
            _$updateProxyTarget(myProxy, result);
        }

        return result;
    }

    _$getInstanceByTypeString = function (typeName)
    {
        return _$getInstanceByType(_$protectedEval(typeName));
    }

    function arrayValue(type, elementCtor, elementType) {
        var result;
        var typedCtor = typed[type];
        if (typedCtor) {
            result = new typedCtor(1);
            result[0] = 1;
        }
        else if (type == 'Array') {
            result = [];
            elementCtor = ctorOf(elementCtor, elementType);
            if (typeof elementCtor == 'function')
                result[0] = _$getInstanceByType(elementCtor);
            else if (elementType)
                result[0] = _$createDomObject(elementType);
        }
        return result;
    }

    _$createDomObject = function (typeName) {
        if (typeof document === 'object' && typeof document._$createDomObject === 'function')
            return document._$createDomObject(typeName);
        return undefined;
    }

    _$value = function (doc, defaultValue) {
        var result = !_$isNullOrUndefined(doc.value) ? doc.value : defaultValue;
        if (_$isNullOrUndefined(result)) {
            result = arrayValue(doc.type, doc.elementCtor, !doc.isUnsafeElementType ? doc.elementType : undefined) || result;
            if (_$isNullOrUndefined(result)) {
                var valueCtor = ctorOf(doc.ctor, !doc.isUnsafeType ? doc.type : undefined);
                if (typeof valueCtor == 'function') {
                    result = _$getInstanceByType(valueCtor);
                }
                else if (doc.type) {
                    result = _$createDomObject(doc.type) || result;
                }
            }
        }
        return result;
    };

    _$elementValue = function (doc) {
        if (doc) {
            if (typed[doc.type]) return 1;
            if (typeof doc.elementCtor == 'function')
                return _$getInstanceByType(doc.elementCtor);
            else if (doc.elementType)
                return _$createDomObject(doc.elementType);
        }
    };

    _$paramValue = function (type, isElement) {
        if (isElement)
            return _$createDomObject(type);
        var ctor = _$protectedEval(type);
        if (ctor && typeof ctor === "function") return new ctor();
    };
})();

function _$mergeDoc(doc, other) {
    if (!doc.type && !doc.elementType) {
        doc.type = other.type;
        doc.elementType = other.elementType;
    }
    if (!doc.description && !doc.locid && !doc.externalFile && !doc.externalid) {
        doc.description = other.description;
        doc.locid = other.locid;
        doc.externalFile = other.externalFile;
        doc.externalid = other.externalid;
    }
    doc.helpKeyword = doc.helpKeyword || other.helpKeyword;
    return doc;
}

function _$initVar(value, doc) {
    if (typeof value === 'undefined')
        return _$getTrackingUndefined(_$value(doc));
    if (value === null)
        return _$getTrackingNull(_$value(doc));
    if (value._$doc && doc) {
        // merge <returns> info
        value._$doc = _$mergeDoc(doc, value._$doc);
    } else {
        value._$doc = doc;
    }
    return value;
}

// Introduce the name in order to avoid parser confusion. The will be reintroduced in each rewritten function. 
var _$retDoc;
function _$return(thisVal, v, retMeta) {
    var shouldReturnTrackedUndefined = false;
    var isValueNull = v === null;
    if (_$isNullOrUndefined(v) || v._$isExceptionObject) {
        var caller = arguments.callee.caller;
        if (!_$isNullOrUndefined(caller) && !caller._$isExceptionObject
            && (thisVal instanceof caller)) {
            v = thisVal;
        }
        else if (retMeta && retMeta.doc) {
            shouldReturnTrackedUndefined = true;
            v = _$value(retMeta.doc);
        }
    }
    if (!_$isNullOrUndefined(v) && retMeta) {
        if (retMeta.doc && typeof v == "object" && Object.isExtensible(v))
            _$defineProperty(v, '_$doc', retMeta.doc);
        if (retMeta.fields)
            _$initFields(v, retMeta);

        if (shouldReturnTrackedUndefined) {
            _$trace("_$return (doc) : returning tracking undefined/null\n");
            if (isValueNull) {
                return _$getTrackingNull(v);
            }
            return _$getTrackingUndefined(v)
        }
    }
    return v;
}

function _$initArg(v, doc) {
    if (!doc) return;
    v = _$value(doc, v);
    if (!_$isNullOrUndefined(v))
        _$defineProperty(v, '_$doc', doc);
    return v;
}
var _$globalObject = this;
// This is called once upon a rewritten function entry to set the field values so IntelliSense on this.| will work correctly.
// It is also called by _$return function to set field values on the actual returned object which may be different from 'this' object.
function _$initFields(o, funcDoc) {
    if (_$isNullOrUndefined(o) || o == _$globalObject || !funcDoc || !funcDoc.fields) return;
    for (var i = 0; i < funcDoc.fields.length; i++) {
        var fieldDoc = funcDoc.fields[i];
        if (o.hasOwnProperty(fieldDoc.name))
            o[fieldDoc.name] = _$value(fieldDoc, o[fieldDoc.name]);
        _$defineProperty(o, '_$fieldDoc$' + fieldDoc.name, fieldDoc);
    }
}

function _$thisAssignment(thisVal, p, v, definitionRef) {
    thisVal[p] = v;
    var fieldDoc = '_$fieldDoc$' + p;
    if (!(fieldDoc in thisVal)) {
        _$defineProperty(thisVal, fieldDoc, definitionRef);
    } else {
        var existingDoc = thisVal[fieldDoc];
        if (existingDoc.fileId && (existingDoc.fileId == definitionRef.fileId) && !existingDoc.pos) {
            existingDoc.pos = definitionRef.pos;
        }
        if (!existingDoc.isDefinitionRef && (v instanceof Array)) {
            var elementValue = _$elementValue(existingDoc);
            if (elementValue != undefined) {
                _$defineProperty(v, '_$doc', existingDoc);
                _$defineProperty(v, 'forEach', function (callback, thisArg) {
                    if (this.length)
                        Array.prototype.forEach.apply(this, arguments);
                    else
                        intellisense.setCallContext(callback, { thisArg: (thisArg || _$globalObject), args: [v[0], 0] });
                });
                redirectDefinition(v.forEach, Array.prototype.forEach);
            }
        }
    }
}

// Ensure at least one iteration of the for...in statement is made because the cursor is in the for-in statement body.
function _$wrapForInOrForOfTarget(v) {
    for (var i in v) return v;
    return [{}];
}

var _$guardForInOrForOfTarget = (function () {
    var global = this;

    // Guard the enumeration of a for...in statement.
    return function _$guardForInOrForOfTarget(v, guard) {
        if (_$lg_disabled) return v;
        var g = global[guard];
        if (g >= 100) return {};
        for (var m in v) g++;
        if (g < 100) {
            this[guard] = g;
            return v;
        }
        var g = global[guard];
        var r = {};
        for (var m in v)
            if (++g < 100)
                r[m] = v[m];
        global[guard] = g;
        return r;
    }
})();

var _$callbackManager = (function () {
    var callbackList = [];
    var setTimeoutList = [];
    var nestedFunctionList = [];

    function callFunction(fun, settings) {
        if (typeof settings === 'object') {
            if (settings.thisObj)
                fun.apply(settings.thisObj, settings.args);
            else if (settings.useNew)
                new fun();
            else
                fun();
        }
        else
            fun();
        // mark the function as called
        _$defineProperty(fun, '_$called', true);
    }

    function invokeCallbacks() {
        var executionCount = 0;
        for (var i = 0; i < callbackList.length; i++) {
            var fun = callbackList[i];
            if (typeof fun === 'function' && !fun._$called) {
                callFunction(fun, fun._$settings);
                executionCount++;
            }

            // stop the execution of callbacks if they exceed a certain limit
            if (executionCount > _$eventListener_limit)
                break;
        }
        return (executionCount > 0);
    }

    function invokeSetTimeoutCalls(maxTimeout) {
        var executionCount = 0;
        for (var i = 0; i < setTimeoutList.length; i++) {
            var fun = setTimeoutList[i];
            if (typeof fun === 'function' && !fun._$called && !fun._$cleared &&
                  (!maxTimeout || (typeof fun._$timeout === 'number' && fun._$timeout <= maxTimeout))) {
                callFunction(fun, fun._$settings);
                executionCount++;
            }

            // stop the execution of callbacks if they exceed a certain limit
            if (executionCount > _$eventListener_limit)
                break;
        }
        return (executionCount > 0);
    }

    function invokeSingleNestedFunction() {
        // Execute one function from the nested list.
        // Start looking from the end to ensure inner most functions are executed first
        for (var i = nestedFunctionList.length - 1; i >= 0; i--) {
            var fun = nestedFunctionList[i];
            if (typeof fun === 'function' && !fun._$called) {
                callFunction(fun, fun._$settings);
                // return true if any work was done
                return true;
            }
        }
        return false;
    }

    function invokeAll(maxTimeout) {
        var workDone = false;
        do {
            workDone = invokeCallbacks();
            workDone = invokeSetTimeoutCalls(maxTimeout) || workDone;
            workDone = invokeSingleNestedFunction() || workDone;
        } while (workDone);
    }

    function applySettings(fun, settings) {
        if (settings && !fun._$settings) _$defineProperty(fun, '_$settings', settings);
    }

    return {
        addCallback: function (fun, settings) {
            if (typeof fun === 'function') {
                applySettings(fun, settings);
                callbackList.push(fun);
            }
        },
        addSetTimeoutCall: function (fun, timeout, settings) {
            if (typeof fun === 'function') {
                applySettings(fun, settings);
                _$defineProperty(fun, '_$timeout', timeout);
                setTimeoutList.push(fun);
                // use 1-based index to clear this call if cleartimeout was called
                return setTimeoutList.length;
            }
            return 0;
        },
        removeSetTimeoutCall: function (index) {
            var fun = setTimeoutList[index + 1];
            if (typeof fun === 'function') {
                _$defineProperty(fun, '_$cleared', true);
            }
        },
        addNested: function (fun, settings) {
            if (typeof fun === 'function') {
                applySettings(fun, settings);
                nestedFunctionList.push(fun);
            }
        },
        invokeAll: invokeAll,
        invokeImmediateSetTimeoutCalls: function () {
            // invoke settimeout calls with timeout <= 10 msec
            var workDone = false;
            do {
                workDone = invokeSetTimeoutCalls(10);
            } while (workDone);
        }
    };
})();

function _$callClassMethod(ctor, name, isStatic) {
    if (typeof ctor !== 'function' || typeof name !== 'string') { return; }
    if (isStatic) {
        _$callbackManager.addNested(ctor[name], { thisObj: ctor });
    } else if (ctor.prototype) {
        _$prototypeCall(ctor.prototype[name], ctor);
    }
}

function _$callGetterSetter(obj, prop, isGetter, protoCall) {
    if (typeof prop !== 'string') { return; }
    var desc = Object.getOwnPropertyDescriptor((protoCall && obj) ? obj.prototype : obj, prop);
    var f;
    if (desc && (f = (isGetter ? desc.get : desc.set)) && (typeof f === 'function')) {
        if (protoCall && obj) {
            _$prototypeCall(f, obj);
        } else {
            _$callbackManager.addNested(f, (obj ? { thisObj: obj } : undefined));
        }
    }
}

var _$inCall = false;
function _$prototypeCall(f, ctor) {
    // Avoid artifically introduced infinite recursion if the function is nested inside the constructor.
    if (_$inCall) return;
    _$inCall = true;
    // The try-finally is not strictly necessary since exceptions are never allowed to propagate in
    // language service mode, but if that changes this code is unlikely to be reviewed and the 
    // try-finally will keep this helper function working.
    try {
        var thisObj = (ctor && ctor.call && new ctor()) || ctor.prototype;
        _$callbackManager.addNested(f, { thisObj: thisObj });
    } finally {
        _$inCall = false;
    }
}

// Used when we cannot find a better way to force a nested function be called. This will try
// to determine if the call should be called using new or just called normally.
// It is called with new if prototype of the function has members or the first letter is a capital 
// ASCII letter. Otherwise it is called normally.
function _$callNested(f, name, isGenerator, thisObj, forceNew) {
    if (isGenerator) {
        var tempGenObj = f.call(thisObj);
        _$callbackManager.addNested(tempGenObj.next, { thisObj: tempGenObj });
    }
    else if (typeof thisObj === 'object' && thisObj) {
        _$callbackManager.addNested(f, { thisObj: thisObj });
    }
    else if (forceNew || (f && f.prototype && Object.keys(f.prototype).length) || (name && name.length && name[0] >= 'A' && name[0] <= 'Z')) {
        _$callbackManager.addNested(f, { useNew: true });
    }
    else {
        _$callbackManager.addNested(f);
    }
}

// Call the global _$ls function, if it exists or any _$ls method on a global object.
function _$callLss() {
    if (typeof _$ls === 'function')
        _$ls();
    for (var n in this) {
        var o = this[n];
        if (o && typeof o._$ls === 'function') o._$ls();
    }

    // invoke callbacks and nested functions
    _$callbackManager.invokeAll();
}

// Event listener support for Dom and WebWorker 
var _$createEventManager = function (getEventObject) {
    if (getEventObject && typeof getEventObject == 'function') {
        var listeners = [];
        function removeOnPrefix(name) {
            return (name.length > 2 && name[0] == 'o' && name[1] == 'n') ? name.substring(2, name.length) : name;
        }
        function addEventListener(object, type, handler, attach, ignoreEventNameCase) {
            if (!object || !type || typeof type !== 'string' || !handler) return;
            if (attach)
                type = removeOnPrefix(type);
            _$defineProperty(object, '_$' + type, handler);
            if (typeof handler === 'function') {
                _$callbackManager.addCallback(function () { handler.call(object, getEventObject(type, attach, object, ignoreEventNameCase)); });
            }
        }
        function getEventListener(object, type) {
            return object ? object['_$' + type] : undefined;
        }
        function defineEventAttribute(object, name) {
            if (!object.hasOwnProperty(name)) {
                var eventName = removeOnPrefix(name);
                Object.defineProperty(object, name,
                {
                    set: function (handler) {
                        addEventListener(this, eventName, handler, false, true);
                    },
                    get: function () {
                        return getEventListener(this, eventName);
                    }
                });
            }
        }
        return {
            add: addEventListener,
            createEventProperties: function (o) {
                if (!o) return;
                for (var i = 1; i < arguments.length; i++) {
                    defineEventAttribute(o, arguments[i]);
                }
            }
        };
    }
};

function _$nonRemovable(o) {
    if (o) _$defineProperty(o, '_$isNonRemovable', true);
}

// Common functions for dom.js and dedicatedworker.js
function _$inherit(o) {
    return Object.create(o);
}
function _$implement(o, type) {
    if (!o || !type) return;
    var props = Object.getOwnPropertyNames(type);
    for (var i = 0; i < props.length; i++) {
        var prop = props[i];
        Object.defineProperty(o, prop, Object.getOwnPropertyDescriptor(type, prop));
    }
}

// SetTimeout / SetInterval support
function _$setTimeout(expression, msec, language) {
    if (typeof expression == 'function') {
        var args = [];
        if (arguments && arguments.length > 2) {
            for (var i = 2; i < arguments.length; i++)
                args.push(arguments[i]);
        }
        return _$callbackManager.addSetTimeoutCall(expression, msec, { thisObj: this, args: args });
    }
    else if (typeof expression == 'string') {
        return _$callbackManager.addSetTimeoutCall(function () { _$protectedEval(expression); }, msec);
    }
    return 0;
}

function _$clearTimeout(timeoutId) {
    if (typeof timeoutId === 'number')
        _$callbackManager.removeSetTimeoutCall(timeoutId);
}

function _$invokeImmediateSetTimeoutCalls() {
    _$callbackManager.invokeImmediateSetTimeoutCalls();
}

var _$analyzeClasses;

function _$callFunctionWithSettings(functionValue) {
    var instance;
    try {
        var functionPrototype = functionValue.prototype;
        var settings = functionValue._$settings;
        if (settings && settings.args && functionPrototype) {
            // Simulate new with the given arguments.
            // We cannot use __proto__ here because we might be executing in ES5 mode.
            instance = Object.create(functionPrototype);
            functionValue.apply(instance, settings.args);
        }
        else {
            instance = new functionValue();
        }
    }
    catch (e) {
        instance = undefined;
    }
    return instance;
}

(function () {
    var objectProto = Object.prototype,
        arrayProto = Array.prototype,
        functionInstanceMembers = Object.getOwnPropertyNames(function () { }),
        recursionMark = "_$recursionMark$_";

    function isFunction(value) {
        return value && typeof value == "function" && value.prototype;
    }

    function isObject(value) {
        return typeof value == "object";
    }

    var indentLevel = 0;

    function analyzeValue(scope, value, includeMembers, skipContainerListItems) {
        /// <param name="scope" type="Array" />
        /// <param name="value" type="Object" />
        /// <param name="includeMembers" type="Boolean" />
        /// <param name="skipContainerListItems" type="Boolean" />

        function analysisOfMember(instance, name) {
            _$trace(getPrintIndent() + "analysisOfMember (" + name + ") {\n");
            indentLevel++;
            var obj = analysisOfMember_1(instance, name);
            indentLevel--;
            _$trace(getPrintIndent() + "}\n");
            return obj
        }
        function analysisOfMember_1(instance, name) {

            try {
                var descriptor = Object.getOwnPropertyDescriptor(instance, name) || {};
            }
            catch (e) {
                descriptor = {};
            }
            if (descriptor.hasOwnProperty("get")) {
                _$trace(getPrintIndent() + "hasOwnProperty -- get\n");
                return {
                    name: name,
                    type: "field",
                    value: descriptor.get || descriptor.set,
                    auxValue: descriptor.get && descriptor.set,
                    instance: instance
                };
            }
            else {
                var type = isFunction(descriptor.value) ? "method" : "field";
                _$trace(getPrintIndent() + "type : " + type + "\n");
                    return {
                        name: name,
                        type: type,
                        value: descriptor.value,
                        instance: instance
                    };
            }
        }
        function printContainer(container, containerName)
        {
            _$trace(getPrintIndent() + containerName + "{\n");
            indentLevel++;
            for (var i = 0; i < container.length; i++) {
                _$trace(getPrintIndent() + JSON.stringify(container[i]) + "\n");
            }
            indentLevel--;
            _$trace(getPrintIndent() + "}\n");
        }

        printContainer(functionInstanceMembers, "functionInstanceMembers");

        function getPrintIndent() {
            var str = ""
            for (var i = 0 ; i < indentLevel; i++) {
                str += "    "
            }
            return str;
        }

        function analyzeFunction(functionName, functionValue, glyph) {
            /// <param name="functionName" type="String" />
            /// <param name="functionValue" type="Function" />
            /// <param name="containingObject" type="Object" />

            _$trace(getPrintIndent() + "analyzeFunction {\n");
            indentLevel++;
            // Add the instance members from the prototype
            var prototypeChildNames = [],
                functionPrototype = functionValue.prototype,
                children = Object.getOwnPropertyNames(functionPrototype)
                            .filter(function (name) { return name != "constructor"; })
                            .map(function (name) {
                                prototypeChildNames.push(name);
                                return analysisOfMember(functionPrototype, name);
                            });

            // Add static members, noting whether any static members exist beyond what are applied to all function objects.
            var hasStaticFunctions = false;
            Object.getOwnPropertyNames(functionValue)
                .forEach(function (staticMemberName) {
                    if (!hasStaticFunctions &&
                        functionInstanceMembers.indexOf(staticMemberName) < 0 &&
                        isFunction(functionValue[staticMemberName])) {
                        hasStaticFunctions = true;
                    }
                    children.push(analysisOfMember(functionValue, staticMemberName));
                });

            printContainer(prototypeChildNames, "prototypeChildNames");
            printContainer(children, "children");

            if (prototypeChildNames.length || hasStaticFunctions || _$hasThisStmt(functionValue)) {
                var instance;
                try {
                    var settings = functionValue._$settings;
                    if (settings && settings.args && functionPrototype) {
                        // Simulate new with the given arguments.
                        // We cannot use __proto__ here because we might be executing in ES5 mode.
                        _$trace(getPrintIndent() + "Creating instance by apply (setting args) : " + JSON.stringify(settings.args) + "\n");

                        instance = Object.create(functionPrototype);
                        functionValue.apply(instance, settings.args);
                    }
                    else {
                        _$trace(getPrintIndent() + "Creating instance \n");
                        instance = new functionValue();
                    }
                }
                catch (e) {
                    _$trace(getPrintIndent() + "Exception while creating instance \n");
                    instance = {};
                }

                // Add the instance members.
                Object.getOwnPropertyNames(instance)
                    .forEach(function (intstanceVariableName) {
                        var child = analysisOfMember(instance, intstanceVariableName);

                        // Properties defined in the constructor are added in place of prototype properties of the same name
                        var overrideIdx = prototypeChildNames.indexOf(intstanceVariableName);
                        if (overrideIdx >= 0 && overrideIdx < children.length) {
                            children[overrideIdx] = child;
                        } else {
                            children.push(child);
                        }
                    });
                printContainer(children, "children");

                _$trace(getPrintIndent() + "functionName : " + functionName + "\n");

                // Add constructor function
                children.push({
                    name: functionName,
                    type: "method",
                    value: functionValue,
                    instance: instance,
                });

                scope.push({
                    name: functionName,
                    glyph: glyph,
                    type: "class",
                    value: functionValue,
                    instance: instance,
                    children: children
                });
            }
            else if (includeMembers) {
                _$trace(getPrintIndent() + " (no child names, static function or this statement) functionName : " + functionName + "\n");

                    scope.push({
                        name: functionName,
                        type: "method",
                        value: functionValue,
                        instance: value
                    });
            }

            indentLevel--;
            _$trace(getPrintIndent() + "}\n");
        }


        function analyzeObject(objectName, objectValue, glyph) {
            /// <param name="objectName" type="String" />
            /// <param name="objectValue" type="Object" />
            _$trace(getPrintIndent() + "analyzeObject " + includeMembers + " {\n");
            indentLevel++;
            var objectScope = [],
                objectPrototypeScope = [],
                objectValueProto = Object.getPrototypeOf(objectValue),
                objectCtor = objectValueProto ? objectValueProto.constructor : null;

            function analyzeChildren(childScope, childContainer) {
                /// <param name="childScope" type="Array" />
                /// <param name="childContainer" type="Object" />
                _$trace(getPrintIndent() + "analyzeChildren {\n");
                indentLevel++;
                // Only analyze this object if it is not an instance of a class.
                if (objectCtor === Object || objectCtor === Array) {

                    // TODO: Use a Map instead, once available, because we are not guarenteed to be able to set properties (e.g. the object is frozen).
                    childContainer[recursionMark] = true;
                    analyzeValue(childScope, childContainer, true, false);
                    delete childContainer[recursionMark];
                }

                var hasChildren = !!childScope.length;
                _$trace(getPrintIndent() + "}\n");
                indentLevel--;
                return hasChildren;
            }

            // if object has prototype children then it is a class, 
            // likely created by Object.create({mems..})
            if (objectValueProto !== objectProto &&
                objectValueProto !== arrayProto &&
                analyzeChildren(objectPrototypeScope, objectValueProto)) {

                _$trace(getPrintIndent() + "protoobject != objectProto and != arrayProto\n");

                // Add Instance Members
                analyzeChildren(objectPrototypeScope, objectValue);
                scope.push({
                    name: objectName,
                    glyph: glyph,
                    type: "class",
                    value: objectValueProto,
                    instance: value,
                    children: objectPrototypeScope
                });
            }
            // otherwise if the object has instance members we consider it a namespace
            // Only record the object as a namespace if it had something in it (e.g. another namespace or a class).
            else if (analyzeChildren(objectScope, objectValue)) {
                _$trace(getPrintIndent() + "passed analyzeChildren(objectScope, objectValue)\n");

                scope.push({
                    name: objectName,
                    glyph: glyph,
                    type: "namespace",
                    value: objectValue,
                    children: objectScope,
                    instance: value
                });
            }
            else if (includeMembers) {
                scope.push({
                    name: objectName,
                    glyph: glyph,
                    type: "field",
                    value: objectValue,
                    instance: value
                });
            }
            indentLevel--
            _$trace(getPrintIndent() + "}\n");
        }

        function analyzeProperty(propertyName) {
            /// <param name="propertyName" type="String" />
            _$trace(getPrintIndent() + "analyzeProperty, propertyName_0 : " + propertyName + "\n");
            if (propertyName) {

                if (propertyName.substr(0, 2) == "_$") {
                    return;
                }

                var propertyValue = value[propertyName];
                _$trace(getPrintIndent() + "analyzeProperty, propertyName : " + propertyName + "\n");

                if (_$isProxyObject(propertyValue, propertyName)) {
                    _$trace(getPrintIndent() + "     Proxy object\n");
                    return;
                }

                _$trace(getPrintIndent() + "     is recursion marked " + propertyValue[recursionMark] + "\n");

                if (propertyValue != null && !propertyValue[recursionMark]) {
                    if (propertyValue["_$inContainerList"] && skipContainerListItems) {
                        return;
                    }
                    var glyph = propertyValue["_$glyph"];

                    if (isFunction(propertyValue)) {
                        indentLevel++;
                        analyzeFunction(propertyName, propertyValue, glyph);
                        indentLevel--;
                    }

                    else if (isObject(propertyValue)) {
                        indentLevel++;
                        analyzeObject(propertyName, propertyValue, glyph);
                        indentLevel--;
                    }
                    else if (includeMembers) {
                        scope.push({
                            name: propertyName,
                            glyph: glyph,
                            type: "field",
                            value: propertyValue,
                            instance: value
                        });
                    }
                }
                else if (includeMembers) {
                    scope.push({
                        name: propertyName,
                        type: "field",
                        value: propertyValue,
                        instance: value
                    });
                }
            }
        }

        _$trace(getPrintIndent() + "analyzeValue {\n");
        indentLevel++;
        Object.getOwnPropertyNames(value).forEach(analyzeProperty);
        indentLevel--;
        _$trace(getPrintIndent() + "}\n");
    }

    function analyzeClasses() {
        var globalScope = [],
            globalObject = this;
        _$trace("Start analyzeClasses\n");
        try {
            _$trace("Analyzing container list\n");
            analyzeValue(globalScope, _$containerList, false, false);

            globalObject[recursionMark] = true;
            
            _$trace("Analyzing global scope\n");
            analyzeValue(globalScope, globalObject, false, true);
            
            delete globalObject[recursionMark];
        }
        catch (e) {
        }

        _$trace("End analyzeClasses\n");
        return globalScope;
    }

    _$analyzeClasses = analyzeClasses;
})();

// Extensions support
(function () {
    // Copy documentation data from one object to another to allow for comments.
    function annotate(item, doc) {
        if (typeof item === 'function' && typeof doc === 'function')
            _$defineProperty(item, '_$doc', doc._$doc || doc);
        else {
            for (var m in doc) {
                var data = doc[m];
                var d = '_$fieldDoc$' + m;
                var dataFieldDoc = doc[d];
                if (dataFieldDoc) {
                    var member = item[d];
                    if (member) dataFieldDoc.original = member;
                    item[d] = dataFieldDoc;
                    dataFieldDoc.annotation = true;
                    _$defineProperty(member, '_$doc', dataFieldDoc);
                }
                if (typeof data === 'function') {
                    var member = item[m];
                    if (member && typeof member === 'function') {
                        _$defineProperty(member, '_$doc', data._$doc || data);
                        data.annotation = true;
                    }
                }
            }
        }
    }
    // Redirect definition of one function to another function
    function redirectDefinition(item, definition) {
        if (typeof item === 'function' && typeof definition === 'function') {
            _$defineProperty(item, '_$doc', definition._$doc || definition);
            _$defineProperty(item, '_$def', definition._$def || definition);
        }
    }

    function callerDefines(item, sourceObject) {
        var defLocProperty = '_$defLoc';
        if (item) {

            var type = typeof item;

            if ((type === 'function' || type === 'object') &&
                !item.hasOwnProperty(defLocProperty)) {

                var location = _$getCallerLocation();

                _$defineProperty(item, defLocProperty, location);
            }

            if (sourceObject && sourceObject[defLocProperty]) {
                _$defineProperty(item, defLocProperty, sourceObject[defLocProperty]);
            }
        }
    }

    function declareNavigationContainer(object, displayName, glyph) {
        if (object) {
            // If the display name isn't valid attempt to get the caller name instead
            var name = displayName;
            if (!_$isValidDisplayName(name)) {
                name = _$getCallerName();

                // If the name still isn't valid don't add the object to the container list
                if (!_$isValidDisplayName(name)) {
                    return;
                }
            }

            name = (name.length <= 100) ? name : name.substr(0, 100);

            _$defineProperty(object, '_$inContainerList', true);
            _$defineProperty(object, '_$glyph', (typeof glyph === 'string') ? glyph : 'vs:GlyphGroupModule');
            _$containerList[name] = object;
        }
    }

    function _$isValidDisplayName(name) {
        return (typeof name === 'string' && name !== '');
    }

    var extensions = { _$msgs: [] };
    function addHandler(event, handler) {
        if (!handler) return;
        var handlers = extensions[event];
        if (!handlers) {
            handlers = [];
            handlers.fire = function (arg) {
                for (var i = 0; i < this.length; i++) {
                    try {
                        this[i](arg);
                    }
                    catch (e) { }
                }
            };
            extensions[event] = handlers;
        }
        handlers.push(handler);
    }
    extensions.addEventListener = function (type, handler) {
        switch (type) {
            case 'statementcompletion': addHandler('_onCompletion', handler); break;
            case 'signaturehelp': addHandler('_onParameterHelp', handler); break;
            case 'statementcompletionhint': addHandler('_onCompletionHint', handler); break;
        }
    };

    extensions.undefinedWithCompletionsOf = _$getTrackingUndefined;
    extensions.nullWithCompletionsOf = _$getTrackingNull;
    extensions.annotate = annotate;
    extensions.redirectDefinition = redirectDefinition;
    extensions.callerDefines = callerDefines;
    extensions.declareNavigationContainer = declareNavigationContainer;
    extensions.logMessage = function (msg) {
        if (extensions._$msgs.length < 1000)
            extensions._$msgs.push(msg);
    };
    extensions.logStack = function (maxFrameCount) {
        if (!maxFrameCount) {
            // the default should value should be the best case.
            maxFrameCount = 10;
        }
        else if (maxFrameCount > 1000 || maxFrameCount < 0) {
            // user probably want the whole stack. 1000 items will still be sufficient
            maxFrameCount = 1000;
        }
        _$logStack(maxFrameCount);
    }
    // As of now the intention is to not show these functions - that's why _$
    // Below functions will enable logging call graph.
    extensions._$startCallGraph = function () {
        _$enableCallGraph(true);
    }
    extensions._$endCallGraph = function () {
        _$enableCallGraph(false);
    }
    extensions.getFunctionComments = function (f) { return extensions._$langSvcMethods.getFunctionComments(f); };
    extensions.version = '11';
    extensions.setCallContext = function (func, context) {
        function copyProperty(source, sourceProperty, target, targetProperty) {
            var desc = Object.getOwnPropertyDescriptor(source, sourceProperty);
            if (desc && desc.get) {
                Object.defineProperty(target, targetProperty, { get: desc.get, enumerable: true, configurable: true });
            } else {
                target[targetProperty] = source[sourceProperty];
            }
        }
        if (typeof func !== 'function' || typeof context !== 'object') return;
        var settings = {};
        copyProperty(context, 'thisArg', settings, 'thisObj');
        copyProperty(context, 'args', settings, 'args');
        func._$settings = settings;
    };
    extensions.progress = function () { _$progress(); };
    extensions._$fire = function (event, langSvcMethods, arg) {
        var handlers = extensions[event];
        if (!handlers) return;
        extensions._$langSvcMethods = langSvcMethods;
        handlers.fire(arg);
        extensions._$langSvcMethods = undefined;
    };
    Object.defineProperty(extensions, "executingScriptFileName", {
        get: _$getExecutingScriptFileName,
        enumerable: true, configurable: true
    });
    this.intellisense = extensions;
})();

// Script Loaders support

this._$asyncRequests = {
    _asyncRequestList: [],
    getItems: function () { return this._asyncRequestList; },
    add: function (o) {
        if (o)
            this._asyncRequestList.push(o);
    },
    insertBefore: function (newObject, refObject) {
        if (newObject) {
            var index = 0;
            for (var request in this._asyncRequestList) {
                if (this._asyncRequestList[request] == refObject)
                    break;
                index++;
            }
            this._asyncRequestList.splice(index, 0, newObject);
        }
    },
    replace: function (newObject, oldObject) {
        if (newObject) {
            for (var request in this._asyncRequestList) {
                if (this._asyncRequestList[request] == oldObject) {
                    this._asyncRequestList[request] = newObject;
                    break;
                }
            }
        }
    },
    init: function () {
        var root = _$createDomObject('HTMLScriptElement') || { src: '' };
        this._asyncRequestList = [root];
    }
};
_$asyncRequests.init();
