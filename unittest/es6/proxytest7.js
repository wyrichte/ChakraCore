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
    delete handler.trapDefineProperty;
    savedLogResult = [];
}

var tests = [
    {
        name: "basic seal",
        body: function () {
            var target = {};
            var a = new Proxy(target, {});
            Object.seal(a);
            assert.areEqual(Object.isSealed(target), true, "target is sealed");
            assert.areEqual(Object.isExtensible(target), false, "target is not extensible after seal")
            var target1 = {};
            var b = new Proxy(target1, {});
            Object.freeze(target1);
            assert.areEqual(Object.isFrozen(target1), true, "target is frozen");
            assert.areEqual(Object.isExtensible(target), false, "target is not extensible after freeze");
        }
    },
    {
        name: "basic isfrozen/issealed",
        body: function () {
            var target = {};
            var proxy = new Proxy(target, {});
            assert.areEqual(Object.isSealed(proxy), false, "proxy is not sealed yet");
            Object.seal(target);
            assert.areEqual(Object.isSealed(proxy), true, "proxy is sealed");
            var target1 = {};
            var proxy1 = new Proxy(target1, {});
            assert.areEqual(Object.isFrozen(proxy1), false, "proxy1 is not sealed yet");
            Object.freeze(target1);
            assert.areEqual(Object.isFrozen(proxy1), true, "proxy1 is frozen");
        }
    },
    {
        name: "isFrozen trap",
        body: function ()
        {
            cleanupBeforeTestStarts();
            var target = {bar: 1, foo: 2};
            var proxy = new Proxy(target, handler);
            assert.areEqual(Object.isFrozen(proxy), false, "target is not frozen");
            assert.areEqual(savedLogResult.length, 1, "stop after isExtensible check")
        }
    },
    {
        name: "isFrozen trap#2",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            Object.preventExtensions(target);
            var proxy = new Proxy(target, handler);
            // there will be two set of output: one for keys, and one for going through all the properties.
            assert.areEqual(Object.isFrozen(proxy), false, "target is not frozen");
            // extensible, ownKeys, 2*gOPD (for 2 properties) to check configurable.
            assert.areEqual(savedLogResult.length, 4, "incorrect trap count");
        }
    },
    {
        name: "isFrozen trap#3",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            Object.preventExtensions(target);
            var proxy = new Proxy(target, handler);
            assert.areEqual(Object.isFrozen(proxy), false, "target is not frozen");
            // extensible, ownKeys, 2*gOPD (for 2 properties) to check configurable.
            assert.areEqual(savedLogResult.length, 4, "incorrect trap count");
        }
    },
    {
        name: "isSealed trap",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            assert.areEqual(Object.isSealed(proxy), false, "target is not sealed");
            assert.areEqual(savedLogResult.length, 1, "stop after isExtensible check")
        }
    },
    {
        name: "isSealed trap#2",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            Object.preventExtensions(target);
            var proxy = new Proxy(target, handler);
            assert.areEqual(Object.isSealed(proxy), false, "target is not sealed");
            // extensible, ownKeys, 2*gOPD (for 2 properties) to check configurable.
            assert.areEqual(savedLogResult.length, 4, "incorrect trap count");
        }
    },
    {
        name: "freeze trap",
        body: function()
        {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            Object.freeze(proxy);
            assert.areEqual(Object.isFrozen(target), true, "target is frozen");
            assert.areEqual(Object.isFrozen(proxy), true, "proxy is frozen");
            // key (+nested 2*gOPD) + 2*(gOPD+definePropety), preventExtension; 4 from isFrozen as above;
            assert.areEqual(savedLogResult.length, 10, "incorrect trap count");
        }
    },
    {
        name: "seal trap",
        body: function()
        {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            Object.seal(proxy);
            assert.areEqual(Object.isSealed(target), true, "target is sealed");
            assert.areEqual(Object.isSealed(proxy), true, "proxy is sealed");
            // key (+nested 2*gOPD) + 2*definePropety, preventExtension; 4 from isFrozen as above;
            // seal has one less set of getOwnPropertyDescriptor because we don't need to check for
            // data/accessor.
            assert.areEqual(savedLogResult.length, 8, "incorrect trap count");
        }
    },
    {
        name: "seal throws",
        body: function()
        {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            handler.trapDefineProperty = function (obj, name, descObj) {
                if (name == 'bar') {
                    throw new Error('hello');
                }
            }
            assert.throws(function () { Object.seal(proxy) }, Error, 'one throw', 'hello');
            assert.areEqual(Object.isSealed(target), false, "target is not sealed");
            assert.areEqual(Object.isSealed(proxy), false, "proxy is not sealed");

            delete handler.trapDefineProperty;
            // seal(preventExtension + ownKey + 1*defineProperty since we throw 'hello' in 'bar') + isSeal(isExtensible + ownKey + 2*gOPD)
            assert.areEqual(savedLogResult.length, 7, "incorrect trap count");
        }
    },
    {
        name: "seal multiple throw",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            handler.trapDefineProperty = function (obj, name, descObj) {
                throw new Error(name);
            }
            assert.throws(function () { Object.seal(proxy) }, Error, "first throw wins", 'bar');
            assert.areEqual(Object.isSealed(target), false, "target is not sealed");
            assert.areEqual(Object.isSealed(proxy), false, "proxy is not sealed");
            delete handler.trapDefineProperty;
            // seal(preventExtension + ownKey + 1*defineProperty) + isSeal(isExtensible + ownKey + 2*gOPD)
            assert.areEqual(savedLogResult.length, 7, "incorrect trap count");
        }
    },
    {
        name: "freeze throws",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            handler.trapDefineProperty = function (obj, name, descObj) {
                if (name == 'bar') {
                    throw new Error('hello');
                }
            }
            assert.throws(function () { Object.freeze(proxy) }, Error, 'one throw only', "hello");
            assert.areEqual(Object.isFrozen(target), false, "target is not sealed");
            assert.areEqual(Object.isFrozen(proxy), false, "proxy is not sealed");
            delete handler.trapDefineProperty;
            // freeze(preventExtension + ownKey + +1*gOPD + 1*defineProperty) + isFrozen(isExtensible + ownKey + 2*gOPD)
            assert.areEqual(savedLogResult.length, 8, "incorrect trap count");
        }
    },
    {
        name: "freeze multiple throw",
        body: function () {
            cleanupBeforeTestStarts();
            var target = { bar: 1, foo: 2 };
            var proxy = new Proxy(target, handler);
            handler.trapDefineProperty = function (obj, name, descObj) {
                throw new Error(name);
            }
            assert.throws(function () { Object.freeze(proxy) }, Error, 'first throw wins', 'bar');
            assert.areEqual(Object.isFrozen(target), false, "target is not sealed");
            assert.areEqual(Object.isFrozen(proxy), false, "proxy is not sealed");
            delete handler.trapDefineProperty;
            // freeze(preventExtension + ownKey + +1*gOPD + 1*defineProperty) + isFrozen(isExtensible + ownKey + 2*gOPD)
            assert.areEqual(savedLogResult.length, 8, "incorrect trap count");
        }
    },
];
initialize();
testRunner.runTests(tests);
initialize('Reflect');
testRunner.runTests(tests);


