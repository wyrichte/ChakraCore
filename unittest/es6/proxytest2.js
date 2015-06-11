// JavaScript source code
if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
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
    delete handler.getOwnPropertyDescriptor;
    if (handler.getOwnPropertyDescriptor)
    {
        observerHandler._getOwnPropertyDescriptor = observerHandler.getOwnPropertyDescriptor;
    }
    delete handler.getPrototypeOf;
}

var tests = [
    {
        name: "basic get test",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            assert.areEqual(observerProxy.foo, 20, "get trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trap on get");
            assert.areEqual(savedLogResult[0], "get trap foo on function receiver function", "one get trap");
            delete target.foo;
        }
    },
    {
        name: "basic get test number",
        body: function () {
            cleanupBeforeTestStarts();
            target['6'] = 6;
            assert.areEqual(observerProxy[6], 6, "get trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trap on get");
            assert.areEqual(savedLogResult[0], "get trap 6 on function receiver function", "one get trap");
            delete target['6'];
        }
    },
    {
        name: "basic get test function",
        body: function () {
            cleanupBeforeTestStarts();
            target.basicTestFunc = function () { return 4242; };
            assert.areEqual(observerProxy.basicTestFunc(), 4242, "get trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trap on get");
            assert.areEqual(savedLogResult[0], "get trap basicTestFunc on function receiver function", "one get trap");
            delete target.basicTestFunc;
        }
    },
    {
        name: "get on prototype",
        body: function () {
            cleanupBeforeTestStarts();
            Function.prototype.bar = 42;
            assert.areEqual(observerProxy.bar, 42, "get trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trp on get");
            assert.areEqual(savedLogResult[0], "get trap bar on function receiver function", "one get trap");
            delete Function.prototype.bar;
        }
    },
    {
        name: "get func on prototype",
        body: function () {
            cleanupBeforeTestStarts();
            Function.prototype.protoFunc = function () { return 'protoFunc' };
            assert.areEqual(observerProxy.protoFunc(), 'protoFunc', "get trap forward");
            assert.areEqual(savedLogResult.length, 1, "one trp on get");
            assert.areEqual(savedLogResult[0], "get trap protoFunc on function receiver function", "one get trap");
            delete Function.prototype.bar;
        }
    },
    {
        name: "get argument on object",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 21;
            handler.trapGet = function (obj, name, recv) {
                assert.areEqual(true, obj == target, "obj == target at get");
                assert.areEqual(true, recv == observerProxy, "receiver == target at get");
                assert.areEqual(name, "foo", "name == foo at get");
                return obj[name];
            }
            assert.areEqual(21, observerProxy.foo, "proxy.foo == 21");
            assert.areEqual(savedLogResult[0], "get trap foo on function receiver function", "one get trap");
            delete target.foo;
        }
    },
    {
        name: "get argument on prototype",
        body: function () {
            cleanupBeforeTestStarts();
            Function.prototype.bar = 42;
            handler.trapGet = function (obj, name, recv) {
                assert.areEqual(obj, target, "obj == Function.prototype at get");
                assert.areEqual(recv, observerProxy, "receiver == target at get");
                assert.areEqual(name, "bar", "name == bar at get");
                return obj[name];
            };
            assert.areEqual(42, observerProxy.bar, "getarg");
            assert.areEqual(savedLogResult[0], "get trap bar on function receiver function", "one get trap");
            delete Function.prototype.bar;
        }
    },
    {
        name: "get argument on prototype1",
        body: function () {
            var obj = {};
            cleanupBeforeTestStarts();
            obj.__proto__ = observerProxy;
            Function.prototype.bar = 42;
            handler.trapGet = function (obj, name, recv) {
                assert.areEqual(obj, target, "obj == Function.prototype at get");
                assert.areEqual(recv, observerProxy, "receiver == target at get");
                assert.areEqual(name, "bar", "name == bar at get");
                return obj[name];
            };
            assert.areEqual(42, observerProxy.bar, "getarg");
            assert.areEqual(savedLogResult[0], "get trap bar on function receiver function", "one get trap");
            delete Function.prototype.bar;
        }
    },
    {
        name: "get argument on prototype2",
        body: function () {
            var myObj = {};
            cleanupBeforeTestStarts();
            myObj.__proto__ = observerProxy;
            Function.prototype.bar = 42;
            handler.trapGet = function (obj, name, recv) {
                assert.areEqual(obj, target, "obj == Function.prototype at get");
                assert.areEqual(recv, myObj, "receiver == target at get");
                assert.areEqual(name, "bar", "name == bar at get");
                return obj[name];
            };
            assert.areEqual(42, myObj.bar, "getarg");
            assert.areEqual(savedLogResult[0], "get trap bar on function receiver object", "one get trap");
            delete Function.prototype.bar;
        }
    },
    {
        name: "get non-existing object",
        body: function () {
            cleanupBeforeTestStarts();
            handler.trapGet = function (obj, name, recv) {
                assert.areEqual(obj, target, "obj == Function.prototype at get");
                assert.areEqual(recv, observerProxy, "receiver == target at get");
                assert.areEqual(name, "nonexistProperty", "name == nonexistProperty at get");
                return 41;
            };
            assert.areEqual(41, observerProxy.nonexistProperty, "getarg");
            assert.areEqual(savedLogResult[0], "get trap nonexistProperty on function receiver function", "one get trap");
        }
    },
    {
        name: "get inconsistent result",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "nonConfigurable", { value: 1, configurable: false });
            handler.trapGet = function (obj, name, recv) {
                assert.areEqual(obj, target, "obj == Function.prototype at get");
                assert.areEqual(recv, observerProxy, "receiver == target at get");
                assert.areEqual(name, "nonConfigurable", "name == nonConfigurable at get");
                return undefined;
            };
            assert.throws(function () { observerProxy.nonConfigurable }, TypeError, "Invariant check failed for get proxy trap");
            assert.areEqual(savedLogResult[0], "get trap nonConfigurable on function receiver function", "one get trap");
        }
    },
    {
        name: "has trap",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            assert.areEqual(true, 'foo' in observerProxy, "we have foo in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.foo", "one has trap");
        }
    },
    {
        name: "has trap number",
        body: function () {
            cleanupBeforeTestStarts();
            target["2"] = 2;
            assert.areEqual(true, '2' in observerProxy, "we have 2 in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.2", "one has trap");
        }
    },
    {
        name: "has trap nonexisting",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            assert.areEqual(false, 'nonexisting' in observerProxy, "we dont have nonexisting in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.nonexisting", "one has trap");
        }
    },
    {
        name: "has trap nonexisting number",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            assert.areEqual(false, '56' in observerProxy, "we dont have 56 in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.56", "one has trap");
        }
    },
    {
        name: "has trap arguments",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            handler.trapHas = function (obj, name) {
                assert.areEqual(true, obj == target, "obj == target");
                assert.areEqual("nonexisting", name, "name == nonexisting");
                return name in obj;
            }
            assert.areEqual(false, 'nonexisting' in observerProxy, "we dont have nonexisting in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.nonexisting", "one has trap");
        }
    },
    {
        name: "has trap inconsistent output1",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            handler.trapHas = function (obj, name) {
                assert.areEqual(true, obj == target, "obj == target");
                assert.areEqual("nonexisting", name, "name == nonexisting");
                return !(name in obj);
            }
            assert.areEqual(true, 'nonexisting' in observerProxy, "we dont have nonexisting in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.nonexisting", "one has trap");
        }
    },
    {
        name: "has trap inconsistent output2",
        body: function () {
            cleanupBeforeTestStarts();
            target.foo = 20;
            handler.trapHas = function (obj, name) {
                assert.areEqual(true, obj == target, "obj == target");
                assert.areEqual("foo", name, "name == foo");
                return !(name in obj);
            }
            assert.areEqual(false, 'foo' in observerProxy, "we dont have foo in target");
            assert.areEqual(1, savedLogResult.length, "one has trap");
            assert.areEqual(savedLogResult[0], "has trap function.foo", "one has trap");
        }
    },
    {
        name: "has trap using with",
        body: function () {
            cleanupBeforeTestStarts();
            Object.defineProperty(target, "foo", { set: function () { print('in set'); }, get: function () { return 5; } });
            with (observerProxy) {
                eval("LogResult('eval out:' + foo);");
            }
            delete target.foo;
            assert.areEqual(savedLogResult.length, 7, "two output: has trap and eval");
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[0]); 
            assert.areEqual("has trap function.eval", savedLogResult[1]); // need to find eval in "with"
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[2]); 
            assert.areEqual("has trap function.LogResult", savedLogResult[3], "second has in eval");  // need to find logResult in eval, env contains with
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[4]); 
            assert.areEqual("get trap foo on function receiver function", savedLogResult[5]);
            assert.areEqual('eval out:5', savedLogResult[6]);
        }
    },
    {
        name: "has trap using with1",
        body: function () {
            cleanupBeforeTestStarts();
            handler.getOwnPropertyDescriptor = handler._getOwnPropertyDescriptor;
            Object.defineProperty(target, "foo1", { set: function () { print('in set'); }, get: function () { return 5; } });
            with (observerProxy) {
                eval("LogResult('eval out:' + foo1);");
            }
            delete target.foo;
            assert.areEqual(savedLogResult.length, 7, "two output: has trap and eval")
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[0]); 
            assert.areEqual("has trap function.eval", savedLogResult[1]); // need to find eval in "with"
             assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[2]); 
            assert.areEqual("has trap function.LogResult", savedLogResult[3]);  // need to find logResult in eval, env contains with
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[4]); 
            assert.areEqual("get trap foo1 on function receiver function", savedLogResult[5]);
            assert.areEqual('eval out:5', savedLogResult[6]);
        }
    },

    {
        name: "propertyIsEnumerable shouldn't call has trap",
        body: function () {
            cleanupBeforeTestStarts();
            handler.getOwnPropertyDescriptor = function (obj, key) {
                LogResult("getOwnPropertyDescriptor trap " + key.toString());
                return Object.getOwnPropertyDescriptor(obj, key);
            }
            Object.defineProperty(target, "foo1", { enumerable: false });
            Object.defineProperty(target, "foo2", { enumerable: true });
            observerProxy.propertyIsEnumerable("foo1");
            observerProxy.propertyIsEnumerable("foo2");
            assert.areEqual(savedLogResult.length, 4, "two output: has trap and eval")
            assert.areEqual("get trap propertyIsEnumerable on function receiver function", savedLogResult[0]);
            assert.areEqual("getOwnPropertyDescriptor trap foo1", savedLogResult[1]);
            assert.areEqual("get trap propertyIsEnumerable on function receiver function", savedLogResult[2]);
            assert.areEqual("getOwnPropertyDescriptor trap foo2", savedLogResult[3]);
        }
    },

    {
        name: "typeof fld; known bug of extra hastrap",
        body: function () {
            cleanupBeforeTestStarts();
            handler.getOwnPropertyDescriptor = handler._getOwnPropertyDescriptor;
            assert.areEqual('undefined', typeof observerProxy.nonexistProperty, "undefined for non-existing property")
            assert.areEqual(1, savedLogResult.length, "two output: has trap and eval");
        }
    },
    {
        name: "typeof element using with; known bug of extra has trap",
        body: function () {
            cleanupBeforeTestStarts();
            handler.getOwnPropertyDescriptor = handler._getOwnPropertyDescriptor;
            with (observerProxy) {
                assert.areEqual('undefined', typeof nonexisting, "undefined for non-existing property")
            }
            assert.areEqual(savedLogResult.length, 4, "go through env");
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[0]); 
            assert.areEqual("has trap function.assert", savedLogResult[1]);
            assert.areEqual("get trap Symbol(Symbol.unscopables) on function receiver function", savedLogResult[2]); 
            assert.areEqual("has trap function.nonexisting", savedLogResult[3]);
        }
    },
    {
        name: "no get trap",
        body: function () {
            cleanupBeforeTestStarts();
            handler._get = handler.get;
            delete handler.get;
            target.passThrough = 14;
            assert.areEqual(observerProxy.passThrough, 14, "get passthrough");
        }
    },

];

initialize();
testRunner.runTests(tests);
initialize('Reflect');
testRunner.runTests(tests);

