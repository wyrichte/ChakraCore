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
    if (!handler.__getOwnPropertyDescriptor) {
        handler.__getOwnPropertyDescriptor = handler.getOwnPropertyDescriptor;
    }
    delete handler.getOwnPropertyDescriptor;
    if (!handler.__getPrototypeOf) {
        handler.__getPrototypeOf = handler.getPrototypeOf;
    }
    delete handler.getPrototypeOf;
}

var tests = [
    {
        name: "basic set test",
        body: function () {
            cleanupBeforeTestStarts();
            observerProxy.test1 = 20;
            assert.areEqual(target.test1, 20, "set trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap test1")
        }
    },
    {
        name: "set argument shape",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapSet = function (obj, name, value, receiver) {
                assert.areEqual(arguments.length, 4, "three arguments in set trap");
                assert.areEqual(obj == target, true, "obj is the target");
                assert.areEqual(name, "test2", "name is test2");
                assert.areEqual(value == 22, true, "value is 20");
                assert.areEqual(receiver == observerProxy, true, "receiver is proxy");
                return obj[name] = value;
            }
            observerProxy.test2 = 22;
            assert.areEqual(target.test2, 22, "set trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap test2")
        }
    },
    {
        name: "basic delete test",
        body: function () {
            cleanupBeforeTestStarts();
            assert.areEqual(delete observerProxy.test1, true, "property deleted");
            assert.areEqual("test1" in target, false, "delete trap forward");
            assert.areEqual(savedLogResult.length, 1, "delete trap test1");
            assert.areEqual(observerProxy.test1, undefined, "deleted property");
        }
    },
    {
        name: "set in eval",
        body: function () {
            cleanupBeforeTestStarts();
            eval("observerProxy.testEval = 22");
            assert.areEqual(target.testEval, 22, "set trap forward");
            assert.areEqual(savedLogResult.length, 1, "one traps on set inside eval");
            assert.areEqual(savedLogResult[0], "set trap testEval");
        }
    },
    {
        name: "set in with",
        body: function () {
            cleanupBeforeTestStarts();
            target.testWith = 5;
            with (observerProxy) {
                testWith = 20;
            }
            assert.areEqual(target.testWith, 20, "set trap forward");
            assert.areEqual(3,savedLogResult.length, "two traps on set inside with");
            assert.areEqual("has trap function.testWith", savedLogResult[0]);
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[1]); 
            assert.areEqual("set trap testWith", savedLogResult[2]);
        }
    },
    {
        name: "set in with1",
        body: function () {
            cleanupBeforeTestStarts();
            with (observerProxy) {
                testWith1 = 20;
            }
            assert.areEqual(target.testWith1, undefined, "set trap non existing property");
            assert.areEqual(1, savedLogResult.length, "one traps on set inside with");
            assert.areEqual("has trap function.testWith1", savedLogResult[0]);
        }
    },
    {
        name: "set element i",
        body: function () {
            cleanupBeforeTestStarts();
            observerProxy[1] = 1;
            assert.areEqual(target[1], 1, "set trap forward");
            if (handler == observeHandler) {
                assert.areEqual(savedLogResult.length, 1, "one trap on set");
            }
            else {
                assert.areEqual(savedLogResult.length, 2, "one trap on set when using Reflect");
            }
            assert.areEqual(savedLogResult[0], "set trap 1")
        }
    },
    {
        name: "set element i",
        body: function () {
            cleanupBeforeTestStarts();
            observerProxy['test2'] = 12;
            assert.areEqual(target['test2'], 12, "set trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap test2");
        }
    },
    {
        name: "set readonly",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "readOnly", { value: 6, configurable: false, writable: false });
            observerProxy.readOnly = 44;
            assert.areEqual(target.readOnly, 6, "readOnly data can't be changed")
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
        }
    },
    {
        name: "set inconsistent value",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "readOnly", { value: 6, configurable: false, writable: false });
            handler.trapSet = function (obj, key, value, receiver) {
                    return true;
            }
            assert.throws(function () { observerProxy.readOnly = 44; }, TypeError);
            assert.areEqual(target.readOnly, 6, "readOnly data can't be changed")
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
        }
    },
    {
        name: "set with readonly property on proto",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(Function.prototype, "proReadOnly", { value: 8, configurable: false, writable: false });
            observerProxy.proReadOnly = 44;
            assert.areEqual(target.proReadOnly, 8, "proReadOnly data can't be chaged")
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap proReadOnly");
        }
    },
    {
        name: "set with setter",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "mySetter", { set: function (value) { target.__mySetter = value; }, get: function () { return target.__mySetter } });
            observerProxy.mySetter = 42;
            assert.areEqual(target.mySetter, 42, "setter called");
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap mySetter");
        }
    },
    {
        name: "set with setter only",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "mySetter1", { set: function (value) { target.__mySetter = value; } });
            observerProxy.mySetter1 = 43;
            assert.areEqual(target.mySetter1, undefined, "setter called");
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap mySetter1")
        }
    },
    {
        name: "set with configurable getter only",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "mySetter2", { get: function () { return 44 }, configurable: true });
            observerProxy.mySetter2 = 43;
            assert.areEqual(target.mySetter2, 44, "setter called");
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap mySetter2")
        }
    },
    {
        name: "set with getter only",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "mySetter3", { get: function () { return 44 } });
            observerProxy.mySetter3 = 43; 
            assert.areEqual(savedLogResult.length, 1, "one trp on set");
            assert.areEqual(savedLogResult[0], "set trap mySetter3")
        }
    },
    {
        name: "set on number",
        body: function() {
            cleanupBeforeTestStarts();
            var oldproto = Object.getPrototypeOf(Number.prototype);
            Object.setPrototypeOf(Number.prototype, observerProxy);
            var a = 2;
            a.foo = 24;
            assert.areEqual(savedLogResult.length, 1, "one set");
            // when we use reflect, receiver is number and we won't
            // be able to set the property on target in prototype
            // the implementation of observeHandler set on target always.
            if (handler == observeHandler) {
                assert.areEqual(target.foo, 24, "foo is set");
            }
            else {
                assert.areEqual(target.foo, undefined, "foo is set");
            }
            var foo = { a: 1, b: 2 };
            for (i in foo) {
                a[i] = 25;
            }
            Object.setPrototypeOf(Number.prototype, oldproto);
        }
    },
    {
        name: "set on proxy in forin of i",
        body: function () {
            cleanupBeforeTestStarts();
            var a = 2;
            var foo = { a: 1, b: 2 };
            target.a = 1; target.b = 2;
            for (i in foo) {
                observerProxy[i] = 24;
            }
            assert.areEqual(savedLogResult.length, 2, "two set");
            assert.areEqual(target.a, 24, "foo is set");
            assert.areEqual(target.b, 24, "foo is set");
        }
    },
    {
        name: "change set value",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapSet = function (obj, name, value, receiver) {
                obj[name] = value + 1;
            }
            observerProxy['test10'] = 15;
            assert.areEqual(savedLogResult.length, 1, "one set");
            assert.areEqual(observerProxy.test10, 16, "retrieved set value");
            assert.areEqual(savedLogResult.length, 2, "one set and one get");
        }
    },
    {
        name: "don't trap set",
        body: function () {
            cleanupBeforeTestStarts();
            var oldSet = handler.set;
            delete handler.set;
            handler.getOwnPropertyDescriptor = handler.__getOwnPropertyDescriptor;
            observerProxy.test12 = 45;
            for (i = 0; i < 20; i++)
            {
                assert.areEqual(observerProxy.test12, 45, "value is still set without trapping set");
                assert.areEqual(savedLogResult.length, 3+i, "gOPD, sOPD and get");
            }
            handler.set = oldSet;
        }
    },
    {
        name: "change set value to getter",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapSet = function (obj, name, value, receiver) {
                Object.defineProperty(obj, name, {
                    get: function () { return value + 1; },
                    set: function (value) { obj[name + '_'] = value; }
                });
            };
            observerProxy['test11'] = 15;
            assert.areEqual(savedLogResult.length, 1, "one set");
            assert.areEqual(observerProxy.test10, 16, "retrieved set value");
            assert.areEqual(savedLogResult.length, 2, "one set and one get");
        }
    },
    {
        name: "delete unexisting",
        body: function () {
            cleanupBeforeTestStarts();
            assert.areEqual(true, delete observerProxy.nonExisting, "delete nonexisting property");
            assert.areEqual(false, delete observerProxy.mySetter3, "delete getter only property");
            assert.areEqual(savedLogResult.length, 2, "one trp on delete");
            assert.areEqual(savedLogResult[0], "delete trap nonExisting");
            assert.areEqual(savedLogResult[1], "delete trap mySetter3");
        }
    },
];

initialize();
testRunner.runTests(tests);

