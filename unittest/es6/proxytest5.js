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

tests = [
    {
        name: "Reflect.getOwnPropertyDescriptor basic",
        body: function () {
            var obj = { a: 1 };
            assert.areEqual(Reflect.getOwnPropertyDescriptor(obj, 'a'), Object.getOwnPropertyDescriptor(obj, 'a'), "get existing property");
            assert.areEqual(Reflect.getOwnPropertyDescriptor(obj, 'b'), Object.getOwnPropertyDescriptor(obj, 'b'), "get non-existing property");
            assert.areEqual(Reflect.getOwnPropertyDescriptor(obj), Object.getOwnPropertyDescriptor(obj), "no property name");
            assert.throws(function () { Reflect.getOwnPropertyDescriptor() }, TypeError, "throw type error when no object");
        }
    },
    {
        name: "Reflect.defineProperty basic",
        body: function () {
            var obj = { a: 1 };
            assert.throws(function () { Reflect.defineProperty() }, TypeError, "requires object");
            assert.throws(function () { Reflect.defineProperty(2, '2') }, TypeError, "no descriptor");
            assert.throws(function () { Reflect.defineProperty(2); }, TypeError, "no propertyname");
        }
    },
    {
        name: "Reflect.defineProperty return",
        body: function () {
            var obj = { a: 1 };
            assert.areEqual(true, Reflect.defineProperty(obj, 'a', { value: 2 }), 'defined value');
        }
    },
    {
        name: "Reflect.deleteProperty basic",
        body: function () {
            var obj = { a: 1 };
            assert.areEqual(true, Reflect.deleteProperty(obj, 'a'), "delete property succeeded");
            assert.areEqual(false, 'a' in obj, "object deleted already");
            assert.throws(function () { Reflect.deleteProperty() }, TypeError, "need object");
            assert.areEqual(true, Reflect.deleteProperty(obj), "none existing property");
        }
    },
    {
        name: "Reflect.isExtensible basic",
        body: function () {
            var obj = { a: 1 };
            assert.areEqual(true, Reflect.isExtensible(obj), "object is extensible by default");
            assert.throws(function () { Reflect.isExtensible() }, TypeError, "need object");
            assert.throws(function () { Reflect.prevventExtension() }, TypeError, "need object");
            assert.areEqual(true, Reflect.preventExtensions(obj), "preventExtensions call succeeded");
            assert.areEqual(false, Reflect.isExtensible(obj), "no more extensible");
            assert.areEqual(false, Object.isExtensible(obj), "no more extensible");
        }
    },
    {
         name: "proxy toLocaleString for builtins",
         body: function () {
             BuiltinTypes = [Object];
             for (i = 0; i < BuiltinTypes.length; i++) {
                 var obj = new BuiltinTypes[i]();
                 var pro = new Proxy(obj, {});
                 assert.areEqual(Object.prototype.toLocaleString.call(pro), "[object Object]", "test" + i);
             }
         }
    },
    {
        name: "Reflect.Get #1",
        body: function () {
            var obj = {};
            Object.defineProperty(obj, 'getter', {
                get: function () {
                    assert.areEqual(this, Number);
                    return 42;
                }
            });
            var val = Reflect.get(obj, 'getter', Number);
            assert.areEqual(val, 42);
        }
    },
    {
        name: "Reflect.Get #2",
        body: function () {
            var obj = {};
            Object.defineProperty(obj, 'getter', {
                get: function () {
                    assert.areEqual(typeof this, "function");
                    return 20;
                }
            });
            var val = Reflect.get(obj, 'getter', Number);
            assert.areEqual(val, 20, "return from get");
        }
    },
    {
        name: "Reflect.Get #3",
        body: function () {
            var obj = {};
            Object.defineProperty(obj, 'getter', { get: function () { return this._val; } });
            var internal = {_val: 42};
            var val = Reflect.get(obj, 'getter', internal);
            assert.areEqual(val, 42, "return from get");
        }
    },
    {
        name: "Reflect.Get #4",
        body: function () {
            Object.defineProperty(Number.prototype, 'getter', {
                get: function () {
                    assert.areEqual(typeof this, "object");
                    return this._val;
                }
            });
            var internal = { _val: 41 };
            assert.throws(function () { Reflect.get(2, 'getter', internal) }, TypeError, "need object");
        }
    },
    {
        name: "Reflect.Get #5",
        body: function () {
            Object.defineProperty(Number.prototype, 'funcGetter', {
                get: function () {
                    assert.areEqual(typeof this, "function");
                    return this._val;
                }
            });
            Function._val = 'hello';
            assert.throws(function () { Reflect.get(2, 'funcGetter', Function) }, TypeError, "need object");
        }
    },
    {
        name: "Reflect.Get #6",
        body: function () {
            Object.defineProperty(Number.prototype, '2', {
                get: function () {
                    assert.areEqual(typeof this, "object");
                    assert.areEqual(this._val, 41);
                    return this._val;
                }
            });
            var internal = { _val: 41 };
            assert.throws(function () { Reflect.get(2, '2', internal) }, TypeError, "need object");
        }
    },
    {
        name: "Reflect.Set #1",
        body: function () {
            var obj = {};
            Object.defineProperty(obj, 'setter', {
                set: function (val) {
                    assert.areEqual(typeof this, "object");
                    assert.areEqual(val, 15, "value set is 15");
                    this._val = val;
                }
            });
            var val = Reflect.set(obj, 'setter', 15, obj);
            assert.areEqual(true, val, "Reflect.set of setter");
            assert.areEqual(obj._val, 15, "_val is set in setter function");
        }
    },
    {
        name: "Reflect.Set #2",
        body: function () {
            var obj = {};
            function funcInternal() { return 20; };
            funcInternal._val = 41;
            Object.defineProperty(obj, 'setter', {
                set: function (val) {
                    assert.areEqual(typeof this, "function");
                    assert.areEqual(val, 24);
                    this._val = val;
                }
            });
            var val = Reflect.set(obj, 'setter', 24, funcInternal);
            assert.areEqual(true, val, "Reflect.set of setter #2");
            assert.areEqual(funcInternal._val, 24, "setter set value");
        }
    },
    {
        name: "Reflect.Set #3",
        body: function () {
            var obj = {};
            function funcInternal() { return 20; };
            funcInternal._val = 41;
            Object.defineProperty(Number.prototype, 'setter', {
                set: function (val) {
                    assert.areEqual(typeof this, "function");
                    assert.areEqual(val, 24);
                    this._val = val;
                }
            });
            assert.throws(function () { Reflect.set(2, 'setter', 24, funcInternal) }, TypeError, "need object");
        }
    },
    {
        name: "Reflect.setPrototypeOf ",
        body: function () {
            var obj = {};
            assert.areEqual(true, Reflect.setPrototypeOf(obj, Object.prototype), "same prototype");
            var obj1 = {};
            Object.preventExtensions(obj1);
            assert.areEqual(false, Reflect.setPrototypeOf(obj1, {}), "can't set prototype if not isExtensible");
        }
    },
    {
        name: "Reflect.apply basic",
        body: function () {
            var objThis = {};
            function testFunc() {
                assert.areEqual(this, objThis, "invalid this");
                assert.areEqual(arguments.length, 0, "no arguments");
                return 42;
            }
            assert.areEqual(42, Reflect.apply(testFunc, objThis), "return value match");
        }
    },
    {
        name: "Reflect.apply arguments",
        body: function () {
            var objThis = {};
            var args = [1, 'hello', {}];
            function testFunc() {
                assert.areEqual(arguments.length, 3, "3 arguments");
                for (i = 0; i < 3; i++)
                {
                    assert.areEqual(arguments[i], args[i], "arguments are consistent");
                }
                return 42;
            }
            assert.areEqual(42, Reflect.apply(testFunc, objThis,args), "return value match");
        }
    },
    {
        name: "Reflect.apply still works after changing Function.prototype.apply",
        body: function () {
            var objThis = {};
            var oldApply = Function.prototype.apply;
            Function.prototype.apply = undefined;
            function testFunc() {
                assert.areEqual(this, objThis, "invalid this");
                assert.areEqual(arguments.length, 0, "no arguments");
                return 42;
            }
            assert.areEqual(42, Reflect.apply(testFunc, objThis), "return value match");
            Function.prototype.apply = oldApply;
        }
    },
    {
        name: "Reflect.construct basic",
        body: function () {
            function testFunc() {
                assert.areEqual(true, this instanceof testFunc, "invalid this");
                assert.areEqual(arguments.length, 0, "no arguments");
                this.foo = 20;
                return this;
            }
            assert.areEqual(true, Reflect.construct(testFunc) instanceof testFunc, "return value match");
            assert.areEqual(20, Reflect.construct(testFunc).foo, "new instance of testFunc")
        }
    },
    {
        name: "Reflect.construct arguments",
        body: function () {
            var args = [1, 'hello', {}];
            function testFunc() {
                assert.areEqual(arguments.length, 3, "3 arguments");
                for (i = 0; i < 3; i++) {
                    assert.areEqual(arguments[i], args[i], "arguments are consistent");
                }
                this.bar = 20;
                return this;
            }
            assert.areEqual(true, Reflect.construct(testFunc, args) instanceof testFunc, "return value match");
            assert.areEqual(20, Reflect.construct(testFunc, args).bar, "new instance of testFunc")
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
