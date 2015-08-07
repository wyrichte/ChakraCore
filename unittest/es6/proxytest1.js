// JavaScript source code
if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
    this.WScript.LoadScriptFile("observerProxy.js");
    var csGlobal = this.WScript.LoadScriptFile("observerProxy.js", "samethread");
    var cpGlobal = this.WScript.LoadScriptFile("observerProxy.js", "differetthread");
}
else {
    throw new Error('failed');
}

function cleanupBeforeTestStarts() {
    delete handler.trapGetOwnPropertyDescriptor;
    delete handler.trapGetPrototypeOf;
    delete handler.trapGet;
    delete handler.trapSet;
    delete handler.trapHas;
    delete handler.trapSetPrototypeOf;
    savedLogResult = [];
}

var tests = [
    {
        name: "basic Proxy object",
        body: function () {
            assert.isTrue(Proxy !== undefined, "Proxy is defined");
            assert.areEqual('function', typeof Proxy, "typeof Proxy === 'function'");
            assert.areEqual(2, Proxy.length, "Symbol.length === 2");
            assert.areEqual('function', typeof Proxy.revocable, "typeof Proxy.Revocable === 'function'");
            assert.areEqual(Proxy.__proto__, Function.__proto__, "Proxy.__proto__ == Function.__proto__");
        }
    },
    {
        name: "Proxy construction",
        body: function () {
            assert.throws(function () { Proxy({}, {}) }, TypeError, "", "");
            assert.doesNotThrow(function () { new Proxy({}, {}) }, 'basic proxy creation');
            assert.throws(function () { new Proxy() }, TypeError, "Proxy requires more than 1 arguments", "Proxy requires more than 1 arguments");
            assert.throws(function () { new Proxy({}) }, TypeError, "Proxy requires more than 1 arguments", "Proxy requires more than 1 arguments");
            assert.throws(function () { new Proxy(2, {}) }, TypeError, "Proxy argument target is not a valid object", "Proxy argument target is not a valid object");
            assert.throws(function () { new Proxy({}, "string3") }, TypeError, "Proxy argument handler is not a valid object", "Proxy argument handler is not a valid object");
            assert.areEqual('function', typeof new Proxy(function () { }, {}), 'typeof Proxy(function) === function');
            assert.areEqual('object', typeof new Proxy({}, {}), 'typeof Proxy(object) === object');
            assert.areEqual('object', typeof new Proxy([], {}), 'typeof Proxy(array) === object');
        }
    },
    {
        name: "Revocable Proxy construction",
        body: function () {
            var revocabledProxy;
            assert.doesNotThrow(function () { revocabledProxy = Proxy.revocable({}, {}); }, "basic revcable proxy");
            assert.areEqual('object', typeof revocabledProxy, "typeof revoclableProxy == 'object'");
            assert.areEqual('function', typeof revocabledProxy.revoke, "revocable proxy has revoke method.");
            assert.doesNotThrow(function () { revocabledProxy.revoke(); }, "revoke the revocable proxy");
            assert.areEqual('object', typeof revocabledProxy);
            assert.areEqual('object', typeof revocabledProxy.proxy);
            assert.throws(function () { revocabledProxy.proxy.foo }, TypeError, "", "");
        }
    },
    {
        name: "proxy equality",
        body: function () {
            var revocableProxy;
            var obj = {};
            var handler = {};
            cleanupBeforeTestStarts();
            assert.doesNotThrow(function () { revocableProxy = Proxy.revocable(obj, handler); }, "basic revcable proxy");
            assert.areEqual('object', typeof revocableProxy.proxy, "revocable proxy's proxy property");
            assert.areEqual('function', typeof revocableProxy.revoke, "revocable proxy's revoker property");
            assert.areEqual(typeof revocableProxy.proxy, 'object', "revocable proxy's proxy property");
            assert.areEqual(typeof revocableProxy.revoke, 'function', "revocable proxy's revoker property");
            assert.areEqual(new Proxy(obj, handler) == new Proxy(obj, handler), false, "two instances of proxies are different");
            assert.areEqual(new Proxy(obj, handler) === new Proxy(obj, handler), false, "two instances of proxies are different");
            assert.areEqual(new Proxy(obj, handler) == obj, false, "proxy is different from obj");
        }
    },
    {
        name: "remote proxy equality",
        body: function () {
            var csRevocableProxy;
            var obj = {};
            var handler = {};
            cleanupBeforeTestStarts();
            assert.doesNotThrow(function () { csRevocableProxy = csGlobal.Proxy.revocable(obj, handler); }, "basic revcable proxy");
            assert.areEqual('object', typeof csRevocableProxy.proxy, "revocable proxy's proxy property");
            assert.areEqual('function', typeof csRevocableProxy.revoke, "revocable proxy's revoker property");
            assert.areEqual(typeof csRevocableProxy.proxy, 'object', "revocable proxy's proxy property");
            assert.areEqual(typeof csRevocableProxy.revoke, 'function', "revocable proxy's revoker property");
            var tmp = csGlobal.observerProxy;
            csGlobal.tmp = tmp;
            assert.areEqual(true, csGlobal.tmp == csGlobal.observerProxy, 'keep object identity for remote proxy');
        }
    },
    {
        name: "getOwnPropertyDescriptor_basic",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "foo");
            assert.areEqual(desc.value, 20, "Object.getOwnPropertyDescriptor");
            assert.areEqual(observerProxy.foo, 20, "getOwnProperty");
            target.bar = 'hello';
            assert.areEqual(observerProxy.bar, 'hello', "property from target");
            assert.areEqual(savedLogResult.length, 3);
            assert.areEqual(savedLogResult[0], "getOwnPropertyDescriptor trap foo");
            assert.areEqual(savedLogResult[1], "get trap foo on function receiver function");
            assert.areEqual(savedLogResult[2], "get trap bar on function receiver function");

        }
    },
    {
        name: "getOwnPropertyDescriptor_item",
        body: function () {
            // reset the log.
            cleanupBeforeTestStarts();
            target[1] = undefined;
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "1");
            assert.areEqual(desc.value, undefined, "Object.getOwnPropertyDescriptor");
            assert.areEqual(observerProxy[1], undefined, "getOwnProperty");
            target[1] = 30;
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "1");
            assert.areEqual(desc.value, 30, "Object.getOwnPropertyDescriptor");
            assert.areEqual(observerProxy[1], 30, "getOwnProperty");
        }
    },
    {
        name: "getOwnPropertyDescriptor_getter",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = undefined;
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "foo");
            assert.areEqual(desc.value, undefined, "Object.getOwnPropertyDescriptor");
            assert.areEqual(observerProxy.foo, undefined, "getOwnProperty");
            assert.areEqual(observerProxy._doesntexist, undefined, "getOwnProperty");
            Object.defineProperty(target, "mygetter", { get: function () { LogResult('in mygetter'); return 20; } });
            assert.areEqual(observerProxy.mygetter, 20, "getter property");
        }
    },
    {
        name: "getOwnPropertyDescriptor_itemgetter",
        body: function () {
            // reset the log.
            cleanupBeforeTestStarts();
            target[2] = undefined;
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "2");
            assert.areEqual(desc.value, undefined, "Object.getOwnPropertyDescriptor");
            assert.areEqual(observerProxy[2], undefined, "getOwnProperty");
            Object.defineProperty(target, "2", { get: function () { LogResult('in item getter 2'); return 21; } });
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "2");
            assert.areEqual(21, observerProxy[2], "getOwnProperty");
        }
    },
    {
        name: "getOwnPropertyDescriptor change return",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapGetOwnPropertyDescriptor = function () {
                print('in trapGetOwnPropertyDescriptor');
                return {value: 42, configurable: true}
            }
            target.tada = 20;
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "tada");
            assert.areEqual(desc.value, 42, "Object.getOwnPropertyDescriptor");
            desc = Object.getOwnPropertyDescriptor(observerProxy, "whatisthis");
            assert.areEqual(42, desc.value, "getOwnProperty");
            Object.defineProperty(target, "2", { get: function () { LogResult('in item getter 2'); return 21; } });
            var desc = Object.getOwnPropertyDescriptor(observerProxy, "2");
            assert.areEqual(42, desc.value, "getOwnProperty");
        }
    },
    {
        name: "getPrototypeOf1",
        body: function () {
            var derivedObj = {};
            cleanupBeforeTestStarts();
            Function.prototype.getPrototypeOf1 = 60;
            assert.areEqual(60, observerProxy.getPrototypeOf1, "get from prototype");
            assert.areEqual(1, savedLogResult.length, "getPrototypeOf override doesn't get trapped in object access");
            assert.areEqual('get trap getPrototypeOf1 on function receiver function', savedLogResult[0], "getPrototypeOf trap");
        }
    },
    {
        name: "getPrototypeOf2",
        body: function () {
            var derivedObj = {};
            cleanupBeforeTestStarts();
            // before setting the proxy, the spec requires a verification to avoid recursion.
            derivedObj.__proto__ = observerProxy;
            assert.areEqual(true, derivedObj instanceof Function, "get from prototype");
            assert.areEqual(2, savedLogResult.length, "getPrototypeOf override with one trap");
            assert.areEqual('getPrototypeOf trap', savedLogResult[0], "getPrototypeOf trap");
            assert.areEqual('getPrototypeOf trap', savedLogResult[1], "getPrototypeOf trap");
        }
    },
    {
        name: "getPrototypeOf return null",
        body: function () {
            var derivedObj = {};
            cleanupBeforeTestStarts();
            handler.trapGetPrototypeOf = function (obj) { return null;}
            assert.areEqual(false, observerProxy instanceof Function, "instanceof from null");
            assert.areEqual(1, savedLogResult.length, "getPrototypeOf override with one trap");
            assert.areEqual('getPrototypeOf trap', savedLogResult[0], "getPrototypeOf trap");
            delete handler.trapGetPrototypeOf;
        }
    },
    {
        name: "getPrototypeOf return value",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapGetPrototypeOf = function (obj) { return Number.prototype}
            assert.areEqual(true, observerProxy instanceof Number, "prototypeof ");
            assert.areEqual(1, savedLogResult.length, "getPrototypeOf override with one trap");
            assert.areEqual('getPrototypeOf trap', savedLogResult[0], "getPrototypeOf trap");
            delete handler.trapGetPrototypeOf;
        }
    },
     {
         name: "hasOwnProperty return value",
         body: function () {
             cleanupBeforeTestStarts();
             var origObj = {xyz : "a"};
             target.__proto__ = origObj;
             handler.trapGetPrototypeOf = function (obj) { return Number.prototype }
             assert.areEqual(false, observerProxy.hasOwnProperty("xyz"), "prototypeof ");
             assert.areEqual(2, savedLogResult.length, "getPrototypeOf override with one trap");
             assert.areEqual('get trap hasOwnProperty on function receiver function', savedLogResult[0], "getPrototypeOf trap");
             assert.areEqual('getOwnPropertyDescriptor trap xyz', savedLogResult[1], "getPrototypeOf trap");
             delete handler.trapGetPrototypeOf;
         }
     },
];
initialize();
testRunner.runTests(tests);
initialize('Reflect');
testRunner.runTests(tests);

