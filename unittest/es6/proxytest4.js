// JavaScript source code
if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
    this.WScript.LoadScriptFile("..\\es6\\observerProxy.js");
    var csGlobal = this.WScript.LoadScriptFile("..\\es6\\observerProxy.js", "samethread");
    var cpGlobal = this.WScript.LoadScriptFile("..\\es6\\observerProxy.js", "differetthread");
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
    delete handler.trapIsExtensible;
    delete handler.trapApply;
    delete handler.trapConstruct;
    delete handler.trapOwnKeys;
    delete handler.trapEnumerate;
    savedLogResult = [];
}

var tests = [
    {
        name: "isExtensible trap",
        body: function () {
            cleanupBeforeTestStarts();
            var isExtensible = Object.isExtensible(observerProxy);
            assert.areEqual(isExtensible, Object.isExtensible(target));
            assert.areEqual(savedLogResult.length, 1, "one trap");
        }
    },
    {
        name: "isExtensible trap on nonextensible object",
        body: function () {
            cleanupBeforeTestStarts();
            var nonExtensionObj = {};
            Object.preventExtensions(nonExtensionObj);
            var nonExtProxy = new Proxy(nonExtensionObj, handler);
            var isExtensible = Object.isExtensible(nonExtProxy);
            assert.areEqual(isExtensible, Object.isExtensible(nonExtensionObj));
            assert.areEqual(isExtensible, false, "nonextensible object");
            assert.areEqual(savedLogResult.length, 1, "one trap");
        }
    },
    {
        name: "change isExtensible",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapIsExtensible = function (obj) {
                return !Object.isExtensible(obj);
            }
            assert.throws(function () { Object.isExtensible(observerProxy) }, TypeError, "", "");
            assert.areEqual(true, Object.isExtensible(target));
            assert.areEqual(savedLogResult.length, 1, "one trap");
        }
    },
    {
        name: "change isExtensible",
        body: function () {
            cleanupBeforeTestStarts();
            var nonExtensionObj = {};
            Object.preventExtensions(nonExtensionObj);
            var nonExtProxy = new Proxy(nonExtensionObj, handler);
            handler.trapIsExtensible = function (obj) {
                return !Object.isExtensible(obj);
            }
            assert.throws(function () { Object.isExtensible(nonExtProxy) }, TypeError, "", "");
            assert.areEqual(false, Object.isExtensible(nonExtensionObj));
            assert.areEqual(savedLogResult.length, 1, "one trap");
        }
    },
    {
        name: "preventExtensions",
        body: function () {
            cleanupBeforeTestStarts();
            var nonExtensionObj = {};
            var nonExtProxy = new Proxy(nonExtensionObj, handler);
            Object.preventExtensions(nonExtProxy);
            assert.areEqual(false, Object.isExtensible(nonExtensionObj), "nonExtensionObject is not extensible");
            assert.areEqual(false, Object.isExtensible(nonExtProxy), "nonExtProxy is not extensible");
            assert.areEqual(savedLogResult.length, 2, "preventExtensions + isExtensible");
            Object.preventExtensions(nonExtProxy);
            assert.areEqual(false, Object.isExtensible(nonExtensionObj), "nonExtensionObject is not extensible");
            assert.areEqual(false, Object.isExtensible(nonExtProxy), "nonExtProxy is not extensible");
            assert.areEqual(savedLogResult.length, 4, "preventExtensions + isExtensible");
            if (IsUsingReflect()) {
                assert.throws(function () { nonExtProxy.nonExtObj = 20 }, TypeError);
            }
            else {
                nonExtProxy.nonExtObj = 20;
            }
            assert.areEqual(nonExtProxy.nonExtObj, undefined, "can't set");
        }
    },
    {
        name: "object.keys",
        body: function () {
            cleanupBeforeTestStarts();
            var newkey = Object.keys(observerProxy);
            assert.areEqual(savedLogResult.length, 6, "keys");
            assert.areEqual(newkey, Object.keys(target), "keys value");

        }
    },
    {
        name: "object.keys return undefined",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapOwnKeys = function() {
                return undefined;
            }
            assert.throws(function(){var newKey = Object.keys(observerProxy)}, TypeError);
            assert.areEqual(savedLogResult.length, 1, "keys");
        }
    },
    {
        name: "object.getOwnPropertyNames",
        body: function () {
            cleanupBeforeTestStarts();
            var gOPN = Object.getOwnPropertyNames(observerProxy);
            assert.areEqual(savedLogResult.length, 1, "getOwnPropertyNames");
            print("from proxy");
            for (var i in gOPN) print(gOPN[i]);
            print("from target");
            var targetGOPN = Object.getOwnPropertyNames(target);
            for (var i in targetGOPN) print(targetGOPN[i]);
            assert.areEqual(gOPN, targetGOPN);
        }
    },
    {
        name: "object.getOwnPropertySymbols",
        body: function () {
            cleanupBeforeTestStarts();
            target[Symbol('hello')] = 'world';
            var gOPS = Object.getOwnPropertySymbols(observerProxy);
            assert.areEqual(savedLogResult.length, 1, "getOwnPropertySymbols trap count");
            for (i in gOPS) {
                print(gOPS[i].toString());
            }
            assert.areEqual(gOPS.length, 1, "getOwnPropertySymbols");
            var targetGOPS = Object.getOwnPropertySymbols(target);
            for (i in targetGOPS) {
                print(targetGOPS[i].toString());
            }
            assert.areEqual(gOPS, targetGOPS);
        }
    },
    {
        name: "enumerate with no property",
        body: function () {
            cleanupBeforeTestStarts();
            var count = 0;
            var inCount = 0;
            for (var i in observerProxy) {
                assert.areEqual(observerProxy[i] == target[i], true, "verify enum result");
                print(i);
                if (i in observerProxy) {
                    inCount++;
                }
                count++;
            }
            assert.areEqual(inCount, 0, "don't have enumerable property")
            assert.areEqual(inCount, count, "enumerate all")
            assert.areEqual(savedLogResult.length, 2 * count + 2, "enumerate + getPrototypeOf + n*has");
        }
    },
    {
        name: "enumerate",
        body: function () {
            cleanupBeforeTestStarts();
            var count = 0;
            var inCount = 0;
            target.bar = 20;
            for (var i in observerProxy) {
                assert.areEqual(observerProxy[i] == target[i], true, "verify enum result");
                if (i in observerProxy) {
                    inCount++;
                }
                count++;
            }
            assert.areEqual(inCount, count, "enumerate all")
            assert.areEqual(savedLogResult.length, 2*count + 2, "enumerate + getPrototypeOf + n*has");
        }
    },
    {
        name: "enumerate with prototype property",
        body: function () {
            cleanupBeforeTestStarts();
            var count = 0;
            var inCount = 0;
            target.__proto__.protoObject = 'hello';
            for (var i in observerProxy) {
                print('in ' + i);
                if (i in observerProxy) {
                    assert.areEqual(observerProxy[i] == target[i], true, "verify enum result");
                    inCount++;
                }
                count++;
            }
            assert.areEqual(inCount, count, "enumerate all")
            assert.areEqual(savedLogResult.length, 2 * count + 2, "enumerate + getPrototypeOf + n*has");
            delete target.__proto__.protoObject;
        }
    },
    {
        name: "enumerate with null",
        body: function () {
            cleanupBeforeTestStarts();
            var count = 0;
            var inCount = 0;
            function test() {
                for (var i in observerProxy) {
                    print('in ' + i);
                    if (i in observerProxy) {
                        assert.areEqual(observerProxy[i] == target[i], true, "verify enum result");
                        inCount++;
                    }
                    count++;
                }
            }
            handler.trapEnumerate = function () { return null;}
            assert.throws(test, TypeError, "", "");
            assert.areEqual(inCount, count, "enumerate all");
        }
    },
    {
        name: "Call function",
        body: function () {
            cleanupBeforeTestStarts();
            observerProxy();
            assert.areEqual(savedLogResult.length, 1, "call");
        }
    },
    {
        name: "Call function arguments",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapApply = function (func, thisArg, args) {
                assert.areEqual(true, func === target, "same target");
                assert.areEqual(args.length, 4, "passed in arguments");
                assert.areEqual(args[0], 1, "first arg");
                assert.areEqual(args[1], 2.25, "second arg");
                assert.areEqual(args[2], undefined, "third arg");
                assert.areEqual(args[3], 'hello', "forth arg");
            }
            observerProxy(1, 2.25, undefined, 'hello');
            assert.areEqual(savedLogResult.length, 1, "call");
        }
    },
    {
        name: "Call non-callable object",
        body: function () {
            cleanupBeforeTestStarts();
            var foo = {};
            var fooProxy = new Proxy(foo, handler);
            assert.throws(function () { fooProxy() }, TypeError);
            assert.areEqual(savedLogResult.length, 0, "can't call");
        }
    },
    {
        name: "Call builtin function",
        body: function () {
            cleanupBeforeTestStarts();
            var absProxy = new Proxy(Math.abs, handler);
            assert.areEqual(1, absProxy(-1), "abs(-1) = 1");
            assert.areEqual(savedLogResult.length, 1, "trap builtin");
        }
    },
    {
        name: "Call non-callable function",
        body: function () {
            cleanupBeforeTestStarts();
            var myProxy = new Proxy(Proxy, handler);
            assert.throws(function () { myProxy({}, {}) }, TypeError);
            assert.areEqual(savedLogResult.length, 1, "trap builtin");
        }
    },
    {
        name: "construct",
        body: function () {
            cleanupBeforeTestStarts();
            var newProxy = new observerProxy();
            assert.areEqual(savedLogResult.length, 1, "trap construct");
        }
    },
    {
        name: "construct with argument",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapConstruct = function (func, args) {
                assert.areEqual(true, func === target, "same target");
                assert.areEqual(args.length, 4, "passed in arguments");
                assert.areEqual(args[0], 1, "first arg");
                assert.areEqual(args[1], 2.25, "second arg");
                assert.areEqual(args[2], undefined, "third arg");
                assert.areEqual(args[3], 'hello', "forth arg");
                return [args[0], args[1], args[2], args[3]];
            }
            var newProxy = new observerProxy(1, 2.25, undefined, 'hello');
            assert.areEqual(savedLogResult.length, 1, "construct");
            assert.areEqual(newProxy, [1, 2.25, undefined, 'hello']);
        }
    },
    {
        name: "new on non-constructable function",
        body: function () {
            cleanupBeforeTestStarts();
            var myProxy = new Proxy(Math.abs, handler);
            assert.throws(function () { new myProxy()}, TypeError);
            assert.areEqual(savedLogResult.length, 1, "trap non-constructable");
        }
    },
    {
        name: "new on builtin",
        body: function () {
            cleanupBeforeTestStarts();
            var myProxy = new Proxy(Object, handler);
            var newObj = new myProxy();
            assert.areEqual(savedLogResult.length, 1, "trap builtin");
            assert.areEqual(newObj instanceof Object, true, "isObject");
        }
    },
    {
        name: "new return undefined",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapConstruct = function () {
                return undefined;
            }
            assert.throws(function () { var newObj = new observerProxy(); }, TypeError, "", "");
            assert.areEqual(savedLogResult.length, 1, "fail object");
        }
    },
];

function runTest() {
  initialize();
  testRunner.runTests(tests);
  initialize('Reflect');
  testRunner.runTests(tests);
}

if (this.inProfilerTest) {
  WScript.StartProfiling(runTest);
}
else {
  runTest();
}
