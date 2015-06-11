// ES6 Subclassable tests -- verifies subclass construction behavior

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Symbol[@@create] API shape",
        body: function () {
            descriptor = Object.getOwnPropertyDescriptor(Symbol, Symbol.create);
            assert.isFalse(descriptor.writable, 'Symbol[@@create].descriptor.writable == false');
            assert.isFalse(descriptor.enumerable, 'Symbol[@@create].descriptor.enumerable == false');
            assert.isTrue(descriptor.configurable, 'Symbol[@@create].descriptor.configurable == true');

            assert.areEqual('function', typeof Symbol[Symbol.create], "typeof Symbol[@@create] === 'function'");
            assert.areEqual(0, Symbol[Symbol.create].length, "Symbol[@@create].length === 0");

            var functionToString = Symbol[Symbol.create].toString();
            var actualName = functionToString.substring(9, functionToString.indexOf('('));

            assert.areEqual('[Symbol.create]', actualName, 'Symbol[@@create].name === "[Symbol.create]"');
        }
    },
    {
        name: "Symbol[@@create] always throws no matter the arguments",
        body: function () {
            assert.throws(function() { Symbol[Symbol.create](); }, TypeError, "Symbol[@@create] throws no matter the parameters", "Symbol[Symbol.create]: invalid argument");
        }
    },
    {
        name: "Promise[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Promise, Symbol.create);
            assert.isFalse(descriptor.writable, "Promise[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Promise[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Promise[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Promise[@@create] === 'function'");
            assert.areEqual(0, Promise[Symbol.create].length, "Promise[@@create].length === 0");
        }
    },
    {
        name: "Promise[@@create] behaviors",
        body: function () {
            var _create = Promise[Symbol.create];

            Promise.__defineGetter__(Symbol.create, function() { throw new TypeError('getter for @@create throws'); });
            assert.throws(function() { new Promise(function() { }); }, TypeError, "new Promise() throws when Promise[@@create] is a throwing getter", "getter for @@create throws");

            Object.defineProperty(Promise, Symbol.create, { value: 'string value' });
            assert.throws(function() { new Promise(function() { }); }, TypeError, "new Promise() throws when Promise[@@create] is not callable", "Function expected");

            Object.defineProperty(Promise, Symbol.create, { value: function() { return 123; } });
            assert.throws(function() { new Promise(function() { }); }, TypeError, "new Promise() throws when Promise[@@create] returns a non-object", "Object expected");

            Object.defineProperty(Promise, Symbol.create, { value: undefined });
            assert.throws(function() { new Promise(function() { }); }, TypeError, "new Promise() throws when Promise[@@create] returns undefined", "Promise: 'this' is not a Promise object");

            Object.defineProperty(Promise, Symbol.create, { value: _create });
        }
    },
    {
        name: "Promise[@@create] throwing behavior",
        body: function () {
            assert.throws(function() { Promise[Symbol.create].call(); }, TypeError, "Promise[@@create] throws when called with no this parameter", "Promise[@@create]: 'this' is not a Function object");
            assert.throws(function() { Promise[Symbol.create].call(undefined); }, TypeError, "Promise[@@create] throws when called with undefined this parameter", "Promise[@@create]: 'this' is not a Function object");
            assert.throws(function() { Promise[Symbol.create].call(null); }, TypeError, "Promise[@@create] throws when called with null this parameter", "Promise[@@create]: 'this' is not a Function object");
            assert.throws(function() { Promise[Symbol.create].call({}); }, TypeError, "Promise[@@create] throws when called with non-function this parameter", "Promise[@@create]: 'this' is not a Function object");
            assert.throws(function() { Promise[Symbol.create].call(Math.sin); }, TypeError, "Promise[@@create] throws when called with non-constructor this parameter", "Promise[@@create]: 'this' is not a Function object");

            var promise = Promise[Symbol.create]();
            assert.throws(function() { Promise.prototype.then.call(promise); }, TypeError, "Promise.prototype.then throws when called with an uninitialized promise this parameter", "Promise.prototype.then: 'this' is not a Promise object");
            assert.throws(function() { Promise.prototype.catch.call(promise); }, TypeError, "Promise.prototype.catch throws when called with an uninitialized promise this parameter", "Promise.prototype.then: 'this' is not a Promise object");
        }
    },
    {
        name: "Promise[@@create] constructor and prototype hookup for subclass",
        body: function() {
            class MyPromise extends Promise {
                constructor(executor) {
                    this.val = 'some value';
                    super(executor);
                }

                testMethod() {
                    return this.val;
                }
            }

            var myPromise = new MyPromise(function() {});

            assert.areEqual('some value', myPromise.testMethod(), "Subclass instance inherits class prototype methods");
        }
    },
    {
        name: "Boolean[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Boolean, Symbol.create);
            assert.isFalse(descriptor.writable, "Boolean[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Boolean[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Boolean[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Boolean[@@create] === 'function'");
            assert.areEqual(0, Boolean[Symbol.create].length, "Boolean[@@create].length === 0");
        }
    },
    {
        name: "Boolean[@@create] functionality",
        body: function () {
            assert.throws(function() { Boolean[Symbol.create].call(); }, TypeError, "Boolean[@@create] throws when called with no this parameter", "Boolean[@@create]: 'this' is not a Function object");
            assert.throws(function() { Boolean[Symbol.create].call(undefined); }, TypeError, "Boolean[@@create] throws when called with undefined this parameter", "Boolean[@@create]: 'this' is not a Function object");
            assert.throws(function() { Boolean[Symbol.create].call(null); }, TypeError, "Boolean[@@create] throws when called with null this parameter", "Boolean[@@create]: 'this' is not a Function object");
            assert.throws(function() { Boolean[Symbol.create].call({}); }, TypeError, "Boolean[@@create] throws when called with non-function this parameter", "Boolean[@@create]: 'this' is not a Function object");
            assert.throws(function() { Boolean[Symbol.create].call(Math.sin); }, TypeError, "Boolean[@@create] throws when called with non-constructor this parameter", "Boolean[@@create]: 'this' is not a Function object");

            assert.throws(function() { Boolean[Symbol.create]().toString(); }, TypeError, "Boolean[@@create] returns an uninitialized Boolean object which throws when passed as this argument to Boolean.prototype.toString", "Boolean.prototype.toString: Object internal state is not initialized");
            assert.throws(function() { Boolean[Symbol.create]().valueOf(); }, TypeError, "Boolean[@@create] returns an uninitialized Boolean object which throws when passed as this argument to Boolean.prototype.valueOf", "Boolean.prototype.valueOf: Object internal state is not initialized");

            assert.isTrue(true == Boolean.call(Boolean[Symbol.create](), true), "Creating boolean instance via Boolean[@@create] and initializing via Boolean constructor");

            var a = Boolean[Symbol.create]();
            var b = Boolean.call(a, false);
            assert.isTrue(a === b, "The same Boolean object returned from Boolean[@@create] should also be returned from Boolean constructor during initialization");
            assert.isTrue(false == a, "Initialized Boolean object has value passed to Boolean constructor");

            var c = Boolean.call(a, true);
            assert.isTrue(false == a, "Boolean object is not re-initialized even if passed to constructor a second time");
            assert.isTrue(true === c, "Boolean constructor returns value if this argument is already initialized");
        }
    },
    {
        name: "Subclass of Boolean",
        body: function () {
            class MyBoolean extends Boolean {
                constructor(val) {
                    this.prop = 'mybool';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            assert.areEqual('mybool', new MyBoolean(true).method(), "Subclass of Boolean has correct methods and properties");
            assert.isTrue(new MyBoolean(true) == true, "Subclass of Boolean object has correct boolean value");
            assert.isTrue(new MyBoolean(false) == false, "Subclass of Boolean object has correct boolean value");
        }
    },
    {
        name: "Error constructors have [@@create] properties",
        body: function () {
            function verifyErrorConstructor(constructor, constructorName) {
                var descriptor = Object.getOwnPropertyDescriptor(constructor, Symbol.create);
                assert.isFalse(descriptor.writable, constructorName + "[@@create].writable === false");
                assert.isFalse(descriptor.enumerable, constructorName + "[@@create].enumerable === false");
                assert.isTrue(descriptor.configurable, constructorName + "[@@create].configurable === true");
                assert.areEqual('function', typeof descriptor.value, "typeof " + constructorName + "[@@create] === 'function'");
                assert.areEqual(0, constructor[Symbol.create].length, constructorName + "[@@create].length === 0");

                assert.throws(function() { constructor[Symbol.create].call(); }, TypeError, constructorName + "[@@create] throws when called with no this parameter", constructorName + "[@@create]: 'this' is not a Function object");
                assert.throws(function() { constructor[Symbol.create].call(undefined); }, TypeError, constructorName + "[@@create] throws when called with undefined this parameter", constructorName + "[@@create]: 'this' is not a Function object");
                assert.throws(function() { constructor[Symbol.create].call(null); }, TypeError, constructorName + "[@@create] throws when called with null this parameter", constructorName + "[@@create]: 'this' is not a Function object");
                assert.throws(function() { constructor[Symbol.create].call({}); }, TypeError, constructorName + "[@@create] throws when called with non-function this parameter", constructorName + "[@@create]: 'this' is not a Function object");
                assert.throws(function() { constructor[Symbol.create].call(Math.sin); }, TypeError, constructorName + "[@@create] throws when called with non-constructor this parameter", constructorName + "[@@create]: 'this' is not a Function object");
            }

            verifyErrorConstructor(Error, 'Error');
            verifyErrorConstructor(EvalError, 'EvalError');
            verifyErrorConstructor(RangeError, 'RangeError');
            verifyErrorConstructor(ReferenceError, 'ReferenceError');
            verifyErrorConstructor(SyntaxError, 'SyntaxError');
            verifyErrorConstructor(TypeError, 'TypeError');
            verifyErrorConstructor(URIError, 'URIError');
        }
    },
    {
        name: "Creating Error object and initializing via @@create and the Error constructor",
        body: function () {
            function verifyErrorConstruction(constructor, constructorName) {
                assert.areEqual(constructorName + ': message', constructor.call(constructor[Symbol.create](), 'message').toString(), constructorName + " constructor can be used to create new instances");

                assert.areEqual(constructorName + ': mixed-type error', Error.call(constructor[Symbol.create](), 'mixed-type error').toString(), "Error can be used to initialize any error object allocated by @@create");
                assert.areEqual(constructorName + ': mixed-type error', EvalError.call(constructor[Symbol.create](), 'mixed-type error').toString(), "EvalError can be used to initialize any error object allocated by @@create");
                assert.areEqual(constructorName + ': mixed-type error', RangeError.call(constructor[Symbol.create](), 'mixed-type error').toString(), "RangeError can be used to initialize any error object allocated by @@create");
                assert.areEqual(constructorName + ': mixed-type error', ReferenceError.call(constructor[Symbol.create](), 'mixed-type error').toString(), "ReferenceError can be used to initialize any error object allocated by @@create");
                assert.areEqual(constructorName + ': mixed-type error', SyntaxError.call(constructor[Symbol.create](), 'mixed-type error').toString(), "SyntaxError can be used to initialize any error object allocated by @@create");
                assert.areEqual(constructorName + ': mixed-type error', TypeError.call(constructor[Symbol.create](), 'mixed-type error').toString(), "TypeError can be used to initialize any error object allocated by @@create");
                assert.areEqual(constructorName + ': mixed-type error', URIError.call(constructor[Symbol.create](), 'mixed-type error').toString(), "URIError can be used to initialize any error object allocated by @@create");
            }

            verifyErrorConstruction(Error, 'Error');
            verifyErrorConstruction(EvalError, 'EvalError');
            verifyErrorConstruction(RangeError, 'RangeError');
            verifyErrorConstruction(ReferenceError, 'ReferenceError');
            verifyErrorConstruction(SyntaxError, 'SyntaxError');
            verifyErrorConstruction(TypeError, 'TypeError');
            verifyErrorConstruction(URIError, 'URIError');
        }
    },
    {
        name: "Behavior of Error constructors with different arguments",
        body: function () {
            function verifyErrorConstruction(constructor, constructorName) {
                var e1 = constructor[Symbol.create]();
                var e2 = constructor.call(e1, "message1");

                assert.isTrue(e1 === e2, "Object returned from " + constructorName + " should be strict equal to the this argument when this argument is any uninitialized Error object");

                var e3 = constructor.call(e1, "message2");

                assert.areEqual(constructorName + ': message2', e3.toString(), constructorName + " constructor produces new object with the right message when this argument is initialized Error object");
                assert.isFalse(e1 === e3, "Object returned from " + constructorName + " should be a new object when the this argument is an initialized Error object");

                assert.areEqual(constructorName + ': message3', constructor.call(undefined, "message3").toString(), constructorName + " constructor produces new object with the right message when this argument is undefined");
                assert.areEqual(constructorName + ': message4', constructor.call(null, "message4").toString(), constructorName + " constructor produces new object with the right message when this argument is null");
                assert.areEqual(constructorName + ': message6', constructor.call({}, "message6").toString(), constructorName + " constructor produces new object with the right message when this argument is non-Error object");
                assert.areEqual(constructorName + ': message7', constructor.call('string', "message7").toString(), constructorName + " constructor produces new object with the right message when this argument is non-object");
            }

            verifyErrorConstruction(Error, 'Error');
            verifyErrorConstruction(EvalError, 'EvalError');
            verifyErrorConstruction(RangeError, 'RangeError');
            verifyErrorConstruction(ReferenceError, 'ReferenceError');
            verifyErrorConstruction(SyntaxError, 'SyntaxError');
            verifyErrorConstruction(TypeError, 'TypeError');
            verifyErrorConstruction(URIError, 'URIError');
        }
    },
    {
        name: "Subclass of Error",
        body: function () {
            function verifySubclassError(constructor, constructorName) {
                class MyError extends constructor {
                    constructor(val) {
                        this.prop = 'myerrorsubclass of ' + constructorName;
                        super('Subclass: ' + val);
                    }

                    method() {
                        return this.prop;
                    }
                }

                assert.areEqual('myerrorsubclass of ' + constructorName, new MyError('message').method(), "Subclass of " + constructorName + " has correct methods and properties");
                assert.areEqual(constructorName + ": Subclass: message", new MyError('message').toString(), "Subclass of " + constructorName + " has correct message value");
            }

            verifySubclassError(Error, 'Error');
            verifySubclassError(EvalError, 'EvalError');
            verifySubclassError(RangeError, 'RangeError');
            verifySubclassError(ReferenceError, 'ReferenceError');
            verifySubclassError(SyntaxError, 'SyntaxError');
            verifySubclassError(TypeError, 'TypeError');
            verifySubclassError(URIError, 'URIError');
        }
    },
    {
        name: "Number[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Number, Symbol.create);
            assert.isFalse(descriptor.writable, "Number[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Number[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Number[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Number[@@create] === 'function'");
            assert.areEqual(0, Number[Symbol.create].length, "Number[@@create].length === 0");
        }
    },
    {
        name: "Number[@@create] functionality",
        body: function () {
            assert.throws(function() { Number[Symbol.create].call(); }, TypeError, "Number[@@create] throws when called with no this parameter", "Number[@@create]: 'this' is not a Function object");
            assert.throws(function() { Number[Symbol.create].call(undefined); }, TypeError, "Number[@@create] throws when called with undefined this parameter", "Number[@@create]: 'this' is not a Function object");
            assert.throws(function() { Number[Symbol.create].call(null); }, TypeError, "Number[@@create] throws when called with null this parameter", "Number[@@create]: 'this' is not a Function object");
            assert.throws(function() { Number[Symbol.create].call({}); }, TypeError, "Number[@@create] throws when called with non-function this parameter", "Number[@@create]: 'this' is not a Function object");
            assert.throws(function() { Number[Symbol.create].call(Math.sin); }, TypeError, "Number[@@create] throws when called with non-constructor this parameter", "Number[@@create]: 'this' is not a Function object");

            assert.throws(function() { Number[Symbol.create]().toString(); }, TypeError, "Number[@@create] returns an uninitialized Number object which throws when passed as this argument to Number.prototype.toString", "Number.prototype.toString: 'this' is not a Number object");
            assert.throws(function() { Number[Symbol.create]().valueOf(); }, TypeError, "Number[@@create] returns an uninitialized Number object which throws when passed as this argument to Number.prototype.valueOf", "Number.prototype.valueOf: Object internal state is not initialized");
            assert.throws(function() { Number[Symbol.create]().toExponential(); }, TypeError, "Number[@@create] returns an uninitialized Number object which throws when passed as this argument to Number.prototype.toExponential", "Number.prototype.toExponential: 'this' is not a Number object");
            assert.throws(function() { Number[Symbol.create]().toFixed(); }, TypeError, "Number[@@create] returns an uninitialized Number object which throws when passed as this argument to Number.prototype.toFixed", "Number.prototype.toFixed: 'this' is not a Number object");
            assert.throws(function() { Number[Symbol.create]().toLocaleString(); }, TypeError, "Number[@@create] returns an uninitialized Number object which throws when passed as this argument to Number.prototype.toLocaleString", "Object internal state is not initialized");
            assert.throws(function() { Number[Symbol.create]().toPrecision(); }, TypeError, "Number[@@create] returns an uninitialized Number object which throws when passed as this argument to Number.prototype.toPrecision", "Number.prototype.toPrecision: 'this' is not a Number object");

            assert.isTrue(123 == Number.call(Number[Symbol.create](), 123), "Creating number instance via Number[@@create] and initializing via Number constructor");

            var a = Number[Symbol.create]();
            var b = Number.call(a, 123);
            assert.isTrue(a === b, "The same Number object returned from Number[@@create] should also be returned from Number constructor during initialization");
            assert.isTrue(123 == a, "Initialized Number object has value passed to Number constructor");

            var c = Number.call(a, 456);
            assert.isTrue(123 == a, "Number object is not re-initialized even if passed to constructor a second time");
            assert.isTrue(456 === c, "Number constructor returns value if this argument is already initialized");

            assert.isTrue(123 === Number.call(undefined, 123), "Number constructor returns value when this argument is undefined");
            assert.isTrue(123 === Number.call(null, 123), "Number constructor returns value when this argument is null");
            assert.isTrue(123 === Number.call({}, 123), "Number constructor returns value when this argument is non-Number object");
            assert.isTrue(123 === Number.call('string', 123), "Number constructor returns value when this argument is non-object");
        }
    },
    {
        name: "Subclass of Number",
        body: function () {
            class MyNumber extends Number {
                constructor(val) {
                    this.prop = 'mynumber';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            assert.areEqual('mynumber', new MyNumber(0).method(), "Subclass of Number has correct methods and properties");
            assert.isTrue(new MyNumber(123) == 123, "Subclass of Number object has correct value");
            assert.isTrue(isNaN(new MyNumber()), "MyNumber constructor calls super with a parameter even if no argument is passed to the constructor. Calling new Number(undefined) returns NaN!");
        }
    },
    {
        name: "Date[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Date, Symbol.create);
            assert.isFalse(descriptor.writable, "Date[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Date[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Date[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Date[@@create] === 'function'");
            assert.areEqual(0, Date[Symbol.create].length, "Date[@@create].length === 0");
        }
    },
    {
        name: "Date[@@create] functionality",
        body: function () {
            assert.throws(function() { Date[Symbol.create].call(); }, TypeError, "Date[@@create] throws when called with no this parameter", "Date[@@create]: 'this' is not a Function object");
            assert.throws(function() { Date[Symbol.create].call(undefined); }, TypeError, "Date[@@create] throws when called with undefined this parameter", "Date[@@create]: 'this' is not a Function object");
            assert.throws(function() { Date[Symbol.create].call(null); }, TypeError, "Date[@@create] throws when called with null this parameter", "Date[@@create]: 'this' is not a Function object");
            assert.throws(function() { Date[Symbol.create].call({}); }, TypeError, "Date[@@create] throws when called with non-function this parameter", "Date[@@create]: 'this' is not a Function object");
            assert.throws(function() { Date[Symbol.create].call(Math.sin); }, TypeError, "Date[@@create] throws when called with non-constructor this parameter", "Date[@@create]: 'this' is not a Function object");

            assert.throws(function() { Date[Symbol.create]().toString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toString", "Date.prototype.toString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().valueOf(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.valueOf", "Date.prototype.valueOf: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getDate(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getDate", "Date.prototype.getDate: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getDay(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getDay", "Date.prototype.getDay: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getFullYear(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getFullYear", "Date.prototype.getFullYear: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getHours(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getHours", "Date.prototype.getHours: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getMilliseconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getMilliseconds", "Date.prototype.getMilliseconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getMinutes(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getMinutes", "Date.prototype.getMinutes: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getMonth(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getMonth", "Date.prototype.getMonth: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getSeconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getSeconds", "Date.prototype.getSeconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getTime(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getTime", "Date.prototype.getTime: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getTimezoneOffset(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getTimezoneOffset", "Date.prototype.getTimezoneOffset: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCDate(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCDate", "Date.prototype.getUTCDate: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCFullYear(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCFullYear", "Date.prototype.getUTCFullYear: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCHours(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCHours", "Date.prototype.getUTCHours: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCMilliseconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCMilliseconds", "Date.prototype.getUTCMilliseconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCMinutes(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCMinutes", "Date.prototype.getUTCMinutes: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCMonth(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCMonth", "Date.prototype.getUTCMonth: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().getUTCSeconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.getUTCSeconds", "Date.prototype.getUTCSeconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setDate(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setDate", "Date.prototype.setDate: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setFullYear(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setFullYear", "Date.prototype.setFullYear: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setHours(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setHours", "Date.prototype.setHours: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setMilliseconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setMilliseconds", "Date.prototype.setMilliseconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setMinutes(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setMinutes", "Date.prototype.setMinutes: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setMonth(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setMonth", "Date.prototype.setMonth: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setSeconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setSeconds", "Date.prototype.setSeconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setTime(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setTime", "Date.prototype.setTime: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCDate(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCDate", "Date.prototype.setUTCDate: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCFullYear(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCFullYear", "Date.prototype.setUTCFullYear: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCHours(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCHours", "Date.prototype.setUTCHours: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCMilliseconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCMilliseconds", "Date.prototype.setUTCMilliseconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCMinutes(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCMinutes", "Date.prototype.setUTCMinutes: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCMonth(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCMonth", "Date.prototype.setUTCMonth: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().setUTCSeconds(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.setUTCSeconds", "Date.prototype.setUTCSeconds: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toDateString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toDateString", "Date.prototype.toDateString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toISOString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toISOString", "Date.prototype.toISOString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toJSON(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toJSON", "Date.prototype.toISOString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toLocaleDateString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toLocaleDateString", "Date.prototype.toLocaleDateString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toLocaleString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toLocaleString", "Date.prototype.toLocaleString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toTimeString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toTimeString", "Date.prototype.toTimeString: Object internal state is not initialized");
            assert.throws(function() { Date[Symbol.create]().toUTCString(); }, TypeError, "Date[@@create] returns an uninitialized Date object which throws when passed as this argument to Date.prototype.toUTCString", "Date.prototype.toUTCString: Object internal state is not initialized");

            var a = Date.call(Date[Symbol.create](), 1401411128000);
            var b = new Date(1401411128000);
            assert.areEqual(0, b - a, "Creating date instance via Date[@@create] and initializing via Date constructor produce same date value (single argument)");

            var a = Date.call(Date[Symbol.create](), 2012, 10);
            var b = new Date(2012, 10);
            assert.areEqual(0, b - a, "Creating date instance via Date[@@create] and initializing via Date constructor produce same date value (two arguments)");

            var a = Date[Symbol.create]();
            var b = Date.call(a, 1401411128000);
            assert.isTrue(a === b, "The same Date object returned from Date[@@create] should also be returned from Date constructor during initialization");

            var c = Date.call(a, 1301411128000);
            assert.isFalse(c === a, "Date constructor returns a different object when this argument is already initialized (single argument)");
            assert.areEqual(0, new Date(1401411128000) - a, "Date object is not re-initialized if passed to constructor a second time (single argument)");
            assert.areEqual('string', typeof c, "Date constructor returns string value if this argument is already initialized (single argument)");

            var c = Date.call(a, 2012, 10);
            assert.isFalse(c === a, "Date constructor returns a different object when this argument is already initialized (two arguments)");
            assert.areEqual(0, new Date(1401411128000) - a, "Date object is not re-initialized if passed to constructor a second time (two arguments)");
            assert.areEqual('string', typeof c, "Date constructor returns string value if this argument is already initialized (two arguments)");

            var c = Date.call(a);
            assert.isFalse(c === a, "Date constructor returns a different object when this argument is already initialized (zero arguments)");
            assert.areEqual(0, new Date(1401411128000) - a, "Date object is not re-initialized if passed to constructor a second time (zero arguments)");
            assert.areEqual('string', typeof c, "Date constructor returns string value if this argument is already initialized (zero arguments)");

            assert.areEqual('string', typeof Date.call(undefined, 1401411128000), "Date constructor returns string value when this argument is undefined (single argument)");
            assert.areEqual('string', typeof Date.call(null, 1401411128000), "Date constructor returns string value when this argument is null (single argument)");
            assert.areEqual('string', typeof Date.call({}, 1401411128000), "Date constructor returns string value when this argument is non-Date object (single argument)");
            assert.areEqual('string', typeof Date.call('string', 1401411128000), "Date constructor returns string value when this argument is non-object (single argument)");

            assert.areEqual('string', typeof Date.call(undefined, 2012, 10), "Date constructor returns string value when this argument is undefined (two arguments)");
            assert.areEqual('string', typeof Date.call(null, 2012, 10), "Date constructor returns string value when this argument is null (two arguments)");
            assert.areEqual('string', typeof Date.call({}, 2012, 10), "Date constructor returns string value when this argument is non-Date object (two arguments)");
            assert.areEqual('string', typeof Date.call('string', 2012, 10), "Date constructor returns string value when this argument is non-object (two arguments)");

            assert.areEqual('string', typeof Date.call(undefined), "Date constructor returns string value when this argument is undefined (zero arguments)");
            assert.areEqual('string', typeof Date.call(null), "Date constructor returns string value when this argument is null (zero arguments)");
            assert.areEqual('string', typeof Date.call({}), "Date constructor returns string value when this argument is non-Date object (zero arguments)");
            assert.areEqual('string', typeof Date.call('string'), "Date constructor returns string value when this argument is non-object (zero arguments)");
        }
    },
    {
        name: "Subclass of Date",
        body: function () {
            class MyDate extends Date {
                constructor(val) {
                    this.prop = 'mydate';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            assert.areEqual('mydate', new MyDate(0).method(), "Subclass of Date has correct methods and properties");
            assert.areEqual(0, new Date(1401411128000) - new MyDate(1401411128000), "Subclass of Date object has correct value");

            assert.isTrue(isNaN(new MyDate()), "MyDate constructor calls super with a parameter even if no argument is passed to the constructor. Calling new Date(undefined) returns an Invalid Date!");
            assert.areEqual('object', typeof new MyDate(), "MyDate constructor called with no parameter passes undefined to new Date");
        }
    },
    {
        name: "String[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(String, Symbol.create);
            assert.isFalse(descriptor.writable, "String[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "String[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "String[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof String[@@create] === 'function'");
            assert.areEqual(0, String[Symbol.create].length, "String[@@create].length === 0");
        }
    },
    {
        name: "String[@@create] functionality",
        body: function () {
            assert.throws(function() { String[Symbol.create].call(); }, TypeError, "String[@@create] throws when called with no this parameter", "String[@@create]: 'this' is not a Function object");
            assert.throws(function() { String[Symbol.create].call(undefined); }, TypeError, "String[@@create] throws when called with undefined this parameter", "String[@@create]: 'this' is not a Function object");
            assert.throws(function() { String[Symbol.create].call(null); }, TypeError, "String[@@create] throws when called with null this parameter", "String[@@create]: 'this' is not a Function object");
            assert.throws(function() { String[Symbol.create].call({}); }, TypeError, "String[@@create] throws when called with non-function this parameter", "String[@@create]: 'this' is not a Function object");
            assert.throws(function() { String[Symbol.create].call(Math.sin); }, TypeError, "String[@@create] throws when called with non-constructor this parameter", "String[@@create]: 'this' is not a Function object");

            assert.throws(function() { String[Symbol.create]().toString(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.toString", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().valueOf(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.valueOf", "String.prototype.valueOf: 'this' is not a String object");

            assert.throws(function() { String[Symbol.create]().charAt(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.charAt", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().charCodeAt(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.charCodeAt", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().codePointAt(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.codePointAt", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().concat(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.concat", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().includes(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.includes", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().endsWith(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.endsWith", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().indexOf(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.indexOf", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().lastIndexOf(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.lastIndexOf", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().localeCompare(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.localeCompare", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().match(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.match", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().normalize(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.normalize", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().repeat(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.repeat", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().replace(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.replace", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().search(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.search", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().slice(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.slice", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().startsWith(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.startsWith", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().substring(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.substring", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().toLocaleLowerCase(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.toLocaleLowerCase", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().toLocaleUpperCase(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.toLocaleUpperCase", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().toLowerCase(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.toLowerCase", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().toUpperCase(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.toUpperCase", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { String[Symbol.create]().trim(); }, TypeError, "String[@@create] returns an uninitialized String object which throws when passed as this argument to String.prototype.trim", "String.prototype.toString: 'this' is not a String object");

            assert.isTrue('something' == String.call(String[Symbol.create](), 'something'), "Creating string instance via String[@@create] and initializing via String constructor");

            var a = String[Symbol.create]();
            var b = String.call(a, 'something');
            assert.isTrue(a === b, "The same String object returned from String[@@create] should also be returned from String constructor during initialization");
            assert.isTrue('something' == a, "Initialized String object has value passed to String constructor");

            var c = String.call(a, 'something else');
            assert.isTrue('something' == a, "String object is not re-initialized even if passed to constructor a second time");
            assert.isTrue('something else' === c, "String constructor returns value if this argument is already initialized");

            assert.isTrue('123' === String.call(undefined, 123), "String constructor returns value when this argument is undefined");
            assert.isTrue('123' === String.call(null, 123), "String constructor returns value when this argument is null");
            assert.isTrue('123' === String.call({}, 123), "String constructor returns value when this argument is non-String object");
            assert.isTrue('123' === String.call('string', 123), "String constructor returns value when this argument is non-object");
        }
    },
    {
        name: "Subclass of String",
        body: function () {
            class MyString extends String {
                constructor(val) {
                    this.prop = 'mystring';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            assert.areEqual('mystring', new MyString(0).method(), "Subclass of String has correct methods and properties");
            assert.isTrue(new MyString(123) == '123', "Subclass of String object has correct value");
            assert.isTrue(new MyString(123).toString() === '123', "Subclass of String object inherits correct toString() method");
            assert.isTrue(new MyString(123).valueOf() === '123', "Subclass of String object inherits correct valueOf() method");
            assert.isTrue(new MyString() == 'undefined', "MyString constructor calls super with a parameter even if no argument is passed to the constructor. Calling new String(undefined) returns 'undefined'!");
            assert.isTrue(new MyString().valueOf() === 'undefined', "MyString constructor calls super with a parameter even if no argument is passed to the constructor. Calling new String(undefined) returns 'undefined'!");
        }
    },
    {
        name: "RegExp[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(RegExp, Symbol.create);
            assert.isFalse(descriptor.writable, "RegExp[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "RegExp[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "RegExp[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof RegExp[@@create] === 'function'");
            assert.areEqual(0, RegExp[Symbol.create].length, "RegExp[@@create].length === 0");
        }
    },
    {
        name: "RegExp[@@create] functionality",
        body: function () {
            assert.throws(function() { RegExp[Symbol.create].call(); }, TypeError, "RegExp[@@create] throws when called with no this parameter", "RegExp[@@create]: 'this' is not a Function object");
            assert.throws(function() { RegExp[Symbol.create].call(undefined); }, TypeError, "RegExp[@@create] throws when called with undefined this parameter", "RegExp[@@create]: 'this' is not a Function object");
            assert.throws(function() { RegExp[Symbol.create].call(null); }, TypeError, "RegExp[@@create] throws when called with null this parameter", "RegExp[@@create]: 'this' is not a Function object");
            assert.throws(function() { RegExp[Symbol.create].call({}); }, TypeError, "RegExp[@@create] throws when called with non-function this parameter", "RegExp[@@create]: 'this' is not a Function object");
            assert.throws(function() { RegExp[Symbol.create].call(Math.sin); }, TypeError, "RegExp[@@create] throws when called with non-constructor this parameter", "RegExp[@@create]: 'this' is not a Function object");

            assert.throws(function() { RegExp[Symbol.create]().toString(); }, TypeError, "RegExp[@@create] returns an uninitialized RegExp object which throws when passed as this argument to RegExp.prototype.toString", "RegExp.prototype.toString: Object internal state is not initialized");
            assert.throws(function() { RegExp[Symbol.create]().exec(); }, TypeError, "RegExp[@@create] returns an uninitialized RegExp object which throws when passed as this argument to RegExp.prototype.exec", "RegExp.prototype.exec: Object internal state is not initialized");
            assert.throws(function() { RegExp[Symbol.create]().test(); }, TypeError, "RegExp[@@create] returns an uninitialized RegExp object which throws when passed as this argument to RegExp.prototype.test", "RegExp.prototype.test: Object internal state is not initialized");

            assert.isTrue('/ab+c/' == RegExp.call(RegExp[Symbol.create](), 'ab+c').toString(), "Creating regex instance via RegExp[@@create] and initializing via RegExp constructor");

            var a = RegExp[Symbol.create]();
            var b = RegExp.call(a, 'ab+c');
            assert.isTrue(a === b, "The same RegExp object returned from RegExp[@@create] should also be returned from RegExp constructor during initialization");
            assert.isTrue('/ab+c/' === a.toString(), "Initialized RegExp object has value passed to RegExp constructor");

            var c = RegExp.call(a, 'de+f');
            assert.isTrue('/ab+c/' === a.toString(), "RegExp object is not re-initialized even if passed to constructor a second time");
            assert.isTrue('/de+f/' === c.toString(), "RegExp constructor returns value if this argument is already initialized");

            assert.isTrue('/ab+c/' === RegExp.call(undefined, 'ab+c').toString(), "RegExp constructor returns value when this argument is undefined");
            assert.isTrue('/ab+c/' === RegExp.call(null, 'ab+c').toString(), "RegExp constructor returns value when this argument is null");
            assert.isTrue('/ab+c/' === RegExp.call({}, 'ab+c').toString(), "RegExp constructor returns value when this argument is non-RegExp object");
            assert.isTrue('/ab+c/' === RegExp.call('string', 'ab+c').toString(), "RegExp constructor returns value when this argument is non-object");

            assert.throws(function() { RegExp(RegExp[Symbol.create](), {}); }, TypeError, "RegExp constructor throws when 1st argument is an uninitialized RegExp object", "Object internal state is not initialized");
            assert.throws(function() { RegExp(RegExp.call(RegExp[Symbol.create]()), {}); }, SyntaxError, "RegExp constructor throws when 1st argument is an initialized RegExp object and flags argument is not undefined and ToString(flags) is invalid", "Syntax error in regular expression");

            var a = new RegExp('ab+c');
            var b = RegExp[Symbol.create]();
            var c = RegExp.call(b, a);

            assert.isTrue(b === c, "RegExp constructor correctly initializes RegExp objects");
            assert.isTrue(a !== c, "RegExp constructor re-uses the pattern of the first argument (if it is a RegExp object) but returns a new object");
            assert.isTrue('/ab+c/' === c.toString(), "RegExp constructor initializes with the correct pattern");

            assert.throws(function() { RegExp.call({}, RegExp[Symbol.create]()).toString(); }, TypeError, "RegExp constructor clones RegExp objects (even if they aren't initialized) when the this argument is not an uninitialized RegExp object", "RegExp.prototype.toString: Object internal state is not initialized");
        }
    },
    {
        name: "Subclass of RegExp",
        body: function () {
            class MyRegExp extends RegExp {
                constructor(val) {
                    this.prop = 'myregexp';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            assert.areEqual('myregexp', new MyRegExp(0).method(), "Subclass of RegExp has correct methods and properties");
            assert.isTrue(new MyRegExp('ab+c').toString() === '/ab+c/', "Subclass of RegExp object has correct value");
            assert.isTrue(new MyRegExp().toString() === "/(?:)/", "MyRegExp constructor calls super with a parameter even if no argument is passed to the constructor. Calling new RegExp(undefined) returns '/(?:)/'!");
        }
    },
    {
        name: "Map[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Map, Symbol.create);
            assert.isFalse(descriptor.writable, "Map[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Map[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Map[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Map[@@create] === 'function'");
            assert.areEqual(0, Map[Symbol.create].length, "Map[@@create].length === 0");
        }
    },
    {
        name: "Map[@@create] functionality",
        body: function () {
            assert.throws(function() { Map[Symbol.create].call(); }, TypeError, "Map[@@create] throws when called with no this parameter", "Map[@@create]: 'this' is not a Function object");
            assert.throws(function() { Map[Symbol.create].call(undefined); }, TypeError, "Map[@@create] throws when called with undefined this parameter", "Map[@@create]: 'this' is not a Function object");
            assert.throws(function() { Map[Symbol.create].call(null); }, TypeError, "Map[@@create] throws when called with null this parameter", "Map[@@create]: 'this' is not a Function object");
            assert.throws(function() { Map[Symbol.create].call({}); }, TypeError, "Map[@@create] throws when called with non-function this parameter", "Map[@@create]: 'this' is not a Function object");
            assert.throws(function() { Map[Symbol.create].call(Math.sin); }, TypeError, "Map[@@create] throws when called with non-constructor this parameter", "Map[@@create]: 'this' is not a Function object");

            assert.throws(function() { Map[Symbol.create]().clear(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.clear", "Map.prototype.clear: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().delete(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.delete", "Map.prototype.delete: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().entries(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.entries", "Map.prototype.entries: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().forEach(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.forEach", "Map.prototype.forEach: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().get(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.get", "Map.prototype.get: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().has(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.has", "Map.prototype.has: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().keys(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.keys", "Map.prototype.keys: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().set(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.set", "Map.prototype.set: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().size; }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.size", "Map.prototype.size: Object internal state is not initialized");
            assert.throws(function() { Map[Symbol.create]().values(); }, TypeError, "Map[@@create] returns an uninitialized Map object which throws when passed as this argument to Map.prototype.values", "Map.prototype.values: Object internal state is not initialized");

            var map = Map.call(Map[Symbol.create]());
            map.set('key', 'value');
            assert.areEqual('value', map.get('key'), "Creating Map instance via Map[@@create] and initializing via Map constructor");

            assert.throws(function() { Map.call(map); }, TypeError, "Map constructor throws if this argument is an already-initialized Map object", "");
        }
    },
    {
        name: "Subclass of Map",
        body: function () {
            class MyMap extends Map {
                constructor(val) {
                    this.prop = 'mymap';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            var mymap = new MyMap([{0:'k1',1:'v1'},{0:'k2',1:'v2'}]);

            assert.areEqual('mymap', mymap.method(), "Subclass of Map has correct methods and properties");
            assert.areEqual('v2', mymap.get('k2'), "Subclass of Map object has correct values");
            assert.areEqual('v1', mymap.get('k1'), "Subclass of Map object has correct values");
        }
    },
    {
        name: "Set[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Set, Symbol.create);
            assert.isFalse(descriptor.writable, "Set[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Set[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Set[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Set[@@create] === 'function'");
            assert.areEqual(0, Set[Symbol.create].length, "Set[@@create].length === 0");
        }
    },
    {
        name: "Set[@@create] functionality",
        body: function () {
            assert.throws(function() { Set[Symbol.create].call(); }, TypeError, "Set[@@create] throws when called with no this parameter", "Set[@@create]: 'this' is not a Function object");
            assert.throws(function() { Set[Symbol.create].call(undefined); }, TypeError, "Set[@@create] throws when called with undefined this parameter", "Set[@@create]: 'this' is not a Function object");
            assert.throws(function() { Set[Symbol.create].call(null); }, TypeError, "Set[@@create] throws when called with null this parameter", "Set[@@create]: 'this' is not a Function object");
            assert.throws(function() { Set[Symbol.create].call({}); }, TypeError, "Set[@@create] throws when called with non-function this parameter", "Set[@@create]: 'this' is not a Function object");
            assert.throws(function() { Set[Symbol.create].call(Math.sin); }, TypeError, "Set[@@create] throws when called with non-constructor this parameter", "Set[@@create]: 'this' is not a Function object");

            assert.throws(function() { Set[Symbol.create]().clear(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.clear", "Set.prototype.clear: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().add(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.add", "Set.prototype.add: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().entries(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.entries", "Set.prototype.entries: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().forEach(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.forEach", "Set.prototype.forEach: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().delete(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.delete", "Set.prototype.delete: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().has(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.has", "Set.prototype.has: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().keys(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.keys", "Set.prototype.keys: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().size; }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.size", "Set.prototype.size: Object internal state is not initialized");
            assert.throws(function() { Set[Symbol.create]().values(); }, TypeError, "Set[@@create] returns an uninitialized Set object which throws when passed as this argument to Set.prototype.values", "Set.prototype.values: Object internal state is not initialized");

            var set = Set.call(Set[Symbol.create]());
            set.add('key');
            assert.isTrue(set.has('key'), "Creating Set instance via Set[@@create] and initializing via Set constructor");

            assert.throws(function() { Set.call(set); }, TypeError, "Set constructor throws if this argument is an already-initialized Set object", "");
        }
    },
    {
        name: "Subclass of Set",
        body: function () {
            class MySet extends Set {
                constructor(val) {
                    this.prop = 'myset';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            var myset = new MySet(['k1','k2']);

            assert.areEqual('myset', myset.method(), "Subclass of Set has correct methods and properties");
            assert.isTrue(myset.has('k2'), "Subclass of Set object has correct values");
            assert.isTrue(myset.has('k1'), "Subclass of Set object has correct values");
        }
    },
    {
        name: "WeakMap[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(WeakMap, Symbol.create);
            assert.isFalse(descriptor.writable, "WeakMap[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "WeakMap[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "WeakMap[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof WeakMap[@@create] === 'function'");
            assert.areEqual(0, WeakMap[Symbol.create].length, "WeakMap[@@create].length === 0");
        }
    },
    {
        name: "WeakMap[@@create] functionality",
        body: function () {
            assert.throws(function() { WeakMap[Symbol.create].call(); }, TypeError, "WeakMap[@@create] throws when called with no this parameter", "WeakMap[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakMap[Symbol.create].call(undefined); }, TypeError, "WeakMap[@@create] throws when called with undefined this parameter", "WeakMap[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakMap[Symbol.create].call(null); }, TypeError, "WeakMap[@@create] throws when called with null this parameter", "WeakMap[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakMap[Symbol.create].call({}); }, TypeError, "WeakMap[@@create] throws when called with non-function this parameter", "WeakMap[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakMap[Symbol.create].call(Math.sin); }, TypeError, "WeakMap[@@create] throws when called with non-constructor this parameter", "WeakMap[@@create]: 'this' is not a Function object");

            assert.throws(function() { WeakMap[Symbol.create]().delete(); }, TypeError, "WeakMap[@@create] returns an uninitialized WeakMap object which throws when passed as this argument to WeakMap.prototype.delete", "WeakMap.prototype.delete: Object internal state is not initialized");
            assert.throws(function() { WeakMap[Symbol.create]().get(); }, TypeError, "WeakMap[@@create] returns an uninitialized WeakMap object which throws when passed as this argument to WeakMap.prototype.get", "WeakMap.prototype.get: Object internal state is not initialized");
            assert.throws(function() { WeakMap[Symbol.create]().has(); }, TypeError, "WeakMap[@@create] returns an uninitialized WeakMap object which throws when passed as this argument to WeakMap.prototype.has", "WeakMap.prototype.has: Object internal state is not initialized");
            assert.throws(function() { WeakMap[Symbol.create]().set(); }, TypeError, "WeakMap[@@create] returns an uninitialized WeakMap object which throws when passed as this argument to WeakMap.prototype.set", "WeakMap.prototype.set: Object internal state is not initialized");

            var key = {};
            var weakmap = WeakMap.call(WeakMap[Symbol.create]());
            weakmap.set(key, 'value');
            assert.areEqual('value', weakmap.get(key), "Creating WeakMap instance via WeakMap[@@create] and initializing via WeakMap constructor");

            assert.throws(function() { WeakMap.call(weakmap); }, TypeError, "WeakMap constructor throws if this argument is an already-initialized WeakMap object", "");
        }
    },
    {
        name: "Subclass of WeakMap",
        body: function () {
            class MyWeakMap extends WeakMap {
                constructor(val) {
                    this.prop = 'mweakmap';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            var key1 = {};
            var key2 = {};
            var mweakmap = new MyWeakMap([{0:key1,1:'v1'},{0:key2,1:'v2'}]);

            assert.areEqual('mweakmap', mweakmap.method(), "Subclass of WeakMap has correct methods and properties");
            assert.areEqual('v1', mweakmap.get(key1), "Subclass of WeakMap object has correct values");
            assert.areEqual('v2', mweakmap.get(key2), "Subclass of WeakMap object has correct values");
        }
    },
    {
        name: "WeakSet[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(WeakSet, Symbol.create);
            assert.isFalse(descriptor.writable, "WeakSet[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "WeakSet[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "WeakSet[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof WeakSet[@@create] === 'function'");
            assert.areEqual(0, WeakSet[Symbol.create].length, "WeakSet[@@create].length === 0");
        }
    },
    {
        name: "WeakSet[@@create] functionality",
        body: function () {
            assert.throws(function() { WeakSet[Symbol.create].call(); }, TypeError, "WeakSet[@@create] throws when called with no this parameter", "WeakSet[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakSet[Symbol.create].call(undefined); }, TypeError, "WeakSet[@@create] throws when called with undefined this parameter", "WeakSet[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakSet[Symbol.create].call(null); }, TypeError, "WeakSet[@@create] throws when called with null this parameter", "WeakSet[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakSet[Symbol.create].call({}); }, TypeError, "WeakSet[@@create] throws when called with non-function this parameter", "WeakSet[@@create]: 'this' is not a Function object");
            assert.throws(function() { WeakSet[Symbol.create].call(Math.sin); }, TypeError, "WeakSet[@@create] throws when called with non-constructor this parameter", "WeakSet[@@create]: 'this' is not a Function object");

            assert.throws(function() { WeakSet[Symbol.create]().add(); }, TypeError, "WeakSet[@@create] returns an uninitialized WeakSet object which throws when passed as this argument to WeakSet.prototype.add", "WeakSet.prototype.add: Object internal state is not initialized");
            assert.throws(function() { WeakSet[Symbol.create]().delete(); }, TypeError, "WeakSet[@@create] returns an uninitialized WeakSet object which throws when passed as this argument to WeakSet.prototype.delete", "WeakSet.prototype.delete: Object internal state is not initialized");
            assert.throws(function() { WeakSet[Symbol.create]().has(); }, TypeError, "WeakSet[@@create] returns an uninitialized WeakSet object which throws when passed as this argument to WeakSet.prototype.has", "WeakSet.prototype.has: Object internal state is not initialized");

            var key = {};
            var weakset = WeakSet.call(WeakSet[Symbol.create]());
            weakset.add(key);
            assert.isTrue(weakset.has(key), "Creating WeakSet instance via WeakSet[@@create] and initializing via WeakSet constructor");

            assert.throws(function() { WeakSet.call(weakset); }, TypeError, "WeakSet constructor throws if this argument is an already-initialized WeakSet object", "");
        }
    },
    {
        name: "Subclass of WeakSet",
        body: function () {
            class MyWeakSet extends WeakSet {
                constructor(val) {
                    this.prop = 'mweakset';
                    super(val);
                }

                method() {
                    return this.prop;
                }
            }

            var key1 = {};
            var key2 = {};
            var mweakset = new MyWeakSet([key1,key2]);

            assert.areEqual('mweakset', mweakset.method(), "Subclass of WeakSet has correct methods and properties");
            assert.isTrue(mweakset.has(key1), "Subclass of WeakSet object has correct values");
            assert.isTrue(mweakset.has(key2), "Subclass of WeakSet object has correct values");
        }
    },
    {
        name: "Subclass passes more than 8 arguments to parent constructor",
        body: function () {
            class A extends Number {
                constructor(a,b,c,d,e,f,g,h,i) {
                    assert.areEqual(1, a, "Arguments passed to A constructor have correct values");
                    assert.areEqual(2, b, "Arguments passed to A constructor have correct values");
                    assert.areEqual(3, c, "Arguments passed to A constructor have correct values");
                    assert.areEqual(4, d, "Arguments passed to A constructor have correct values");
                    assert.areEqual(5, e, "Arguments passed to A constructor have correct values");
                    assert.areEqual(6, f, "Arguments passed to A constructor have correct values");
                    assert.areEqual(7, g, "Arguments passed to A constructor have correct values");
                    assert.areEqual(8, h, "Arguments passed to A constructor have correct values");
                    assert.areEqual(9, i, "Arguments passed to A constructor have correct values");
                    super(i);
                }
            }
            class B extends A {
                constructor(a,b,c,d,e,f,g,h,i) {
                    super(a,b,c,d,e,f,g,h,i);
                }
            }

            var b = new B(1,2,3,4,5,6,7,8,9);

            assert.isTrue(9 == b, "Subclass object gets the correct value from the constructor chain");
        }
    },
    {
        name: "Builtin Prototypes have the correct types",
        body: function () {
            assert.throws(function() { String.prototype.toString(); }, TypeError, "String.prototype is not a String object, it cannot be the this argument to String.prototype.toString", "String.prototype.toString: 'this' is not a String object");
            assert.throws(function() { Boolean.prototype.toString(); }, TypeError, "Boolean.prototype is not a Boolean object, it cannot be the this argument to Boolean.prototype.toString", "Boolean.prototype.toString: 'this' is not a Boolean object");
            assert.throws(function() { Number.prototype.toString(); }, TypeError, "Number.prototype is not a Number object, it cannot be the this argument to Number.prototype.toString", "Number.prototype.toString: 'this' is not a Number object");
            assert.throws(function() { Date.prototype.toString(); }, TypeError, "Date.prototype is not a Date object, it cannot be the this argument to Date.prototype.toString", "Date.prototype.toString: 'this' is not a Date object");
            assert.throws(function() { Promise.prototype.then(); }, TypeError, "Promise.prototype is not a Promise object, it cannot be the this argument to Promise.prototype.then", "Promise.prototype.then: 'this' is not a Promise object");

            assert.isTrue(String.prototype.__proto__ === Object.prototype, "String.prototype.__proto__ === Object.prototype");
            assert.isTrue(Boolean.prototype.__proto__ === Object.prototype, "Boolean.prototype.__proto__ === Object.prototype");
            assert.isTrue(Number.prototype.__proto__ === Object.prototype, "Number.prototype.__proto__ === Object.prototype");
            assert.isTrue(Date.prototype.__proto__ === Object.prototype, "Date.prototype.__proto__ === Object.prototype");
            assert.isTrue(Promise.prototype.__proto__ === Object.prototype, "Promise.prototype.__proto__ === Object.prototype");

            assert.isTrue(Error.prototype.__proto__ === Object.prototype, "Error.prototype.__proto__ === Object.prototype");
            assert.isTrue(EvalError.prototype.__proto__ === Error.prototype, "EvalError.prototype.__proto__ === Error.prototype");
            assert.isTrue(RangeError.prototype.__proto__ === Error.prototype, "RangeError.prototype.__proto__ === Error.prototype");
            assert.isTrue(ReferenceError.prototype.__proto__ === Error.prototype, "ReferenceError.prototype.__proto__ === Error.prototype");
            assert.isTrue(SyntaxError.prototype.__proto__ === Error.prototype, "SyntaxError.prototype.__proto__ === Error.prototype");
            assert.isTrue(TypeError.prototype.__proto__ === Error.prototype, "TypeError.prototype.__proto__ === Error.prototype");
            assert.isTrue(URIError.prototype.__proto__ === Error.prototype, "URIError.prototype.__proto__ === Error.prototype");

            assert.isTrue(Error.__proto__ === Function.prototype, "Error.__proto__ === Function.prototype");
            assert.isTrue(EvalError.__proto__ === Error, "EvalError.__proto__ === Error");
            assert.isTrue(RangeError.__proto__ === Error, "RangeError.__proto__ === Error");
            assert.isTrue(ReferenceError.__proto__ === Error, "ReferenceError.__proto__ === Error");
            assert.isTrue(SyntaxError.__proto__ === Error, "SyntaxError.__proto__ === Error");
            assert.isTrue(TypeError.__proto__ === Error, "TypeError.__proto__ === Error");
            assert.isTrue(URIError.__proto__ === Error, "URIError.__proto__ === Error");

            assert.areEqual("[object Object]", Object.prototype.toString.call(Boolean.prototype), "Boolean.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Symbol.prototype), "Symbol.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Error.prototype), "Error.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(EvalError.prototype), "EvalError.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(RangeError.prototype), "RangeError.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(ReferenceError.prototype), "ReferenceError.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(SyntaxError.prototype), "SyntaxError.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(TypeError.prototype), "TypeError.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(URIError.prototype), "URIError.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Number.prototype), "Number.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Date.prototype), "Date.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(String.prototype), "String.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(RegExp.prototype), "RegExp.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Array.prototype), "Array.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Map.prototype), "Map.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Set.prototype), "Set.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(WeakMap.prototype), "WeakMap.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(WeakSet.prototype), "WeakSet.prototype is an ordinary object");
            assert.areEqual("[object Object]", Object.prototype.toString.call(Promise.prototype), "Promise.prototype is an ordinary object");
        }
    },
    {
        name: "Array[@@create] API shape",
        body: function () {
            var descriptor = Object.getOwnPropertyDescriptor(Array, Symbol.create);
            assert.isFalse(descriptor.writable, "Array[@@create].writable === false");
            assert.isFalse(descriptor.enumerable, "Array[@@create].enumerable === false");
            assert.isTrue(descriptor.configurable, "Array[@@create].configurable === true");
            assert.areEqual('function', typeof descriptor.value, "typeof Array[@@create] === 'function'");
            assert.areEqual(0, Array[Symbol.create].length, "Array[@@create].length === 0");
        }
    },
    {
        name: "Array[@@create] functionality",
        body: function () {
            assert.throws(function() { Array[Symbol.create].call(); }, TypeError, "Array[@@create] throws when called with no this parameter", "Array[@@create]: 'this' is not a Function object");
            assert.throws(function() { Array[Symbol.create].call(undefined); }, TypeError, "Array[@@create] throws when called with undefined this parameter", "Array[@@create]: 'this' is not a Function object");
            assert.throws(function() { Array[Symbol.create].call(null); }, TypeError, "Array[@@create] throws when called with null this parameter", "Array[@@create]: 'this' is not a Function object");
            assert.throws(function() { Array[Symbol.create].call({}); }, TypeError, "Array[@@create] throws when called with non-function this parameter", "Array[@@create]: 'this' is not a Function object");
            assert.throws(function() { Array[Symbol.create].call(Math.sin); }, TypeError, "Array[@@create] throws when called with non-constructor this parameter", "Array[@@create]: 'this' is not a Function object");

            var u = Array[Symbol.create]();
            assert.isTrue(u === Array.call(u), "Array[@@create] creates uninitialized Array objects which will be initialized by the Array constructor");
            assert.isTrue(u !== Array.call(u), "Calling Array constructor with an initialized Array object as this argument returns a new Array");
            assert.areEqual(0, u.length, "Array object initialized by call to constructor with no arguments has zero length");

            var u = Array[Symbol.create]();
            assert.isTrue(u === Array.call(u, 100), "Array[@@create] creates uninitialized Array objects which will be initialized by the Array constructor");
            assert.isTrue(u !== Array.call(u, 100), "Calling Array constructor with an initialized Array object as this argument returns a new Array");
            assert.areEqual(100, u.length, "Array object initialized by call to constructor with single, numeric argument has length equal to the argument");

            var u = Array[Symbol.create]();
            assert.isTrue(u === Array.call(u, 50.0), "Array[@@create] creates uninitialized Array objects which will be initialized by the Array constructor");
            assert.isTrue(u !== Array.call(u, 50.0), "Calling Array constructor with an initialized Array object as this argument returns a new Array");
            assert.areEqual(50, u.length, "Array object initialized by call to constructor with single, float argument has length equal to the argument");

            var u = Array[Symbol.create]();
            assert.isTrue(u === Array.call(u, 'non-number'), "Array[@@create] creates uninitialized Array objects which will be initialized by the Array constructor");
            assert.isTrue(u !== Array.call(u, 'non-number'), "Calling Array constructor with an initialized Array object as this argument returns a new Array");
            assert.areEqual('non-number', u[0], "Array object initialized by call to constructor with single, non-numeric argument has that argument as [0] element")
            assert.areEqual(1, u.length, "Array object initialized by call to constructor with single, non-numeric argument has length equal to 1");

            var u = Array[Symbol.create]();
            assert.isTrue(u === Array.call(u, 0, 1, 2, 3), "Array[@@create] creates uninitialized Array objects which will be initialized by the Array constructor");
            assert.isTrue(u !== Array.call(u, 0, 1, 2, 3), "Calling Array constructor with an initialized Array object as this argument returns a new Array");
            assert.areEqual([0, 1, 2, 3], u, "Array object initialized by call to constructor with multiple arguments has elements equal to those arguments")
            assert.areEqual(4, u.length, "Array object initialized by call to constructor with multiple arguments has length equal to the number of arguments");

            var u = Array[Symbol.create]();
            u[0] = 1;
            u[1] = 2;
            u[2] = 3;
            assert.areEqual(3, u.length, "Uninitialized Array objects can still have their elements changed");
            assert.isTrue(u === Array.call(u), "Array constructor initializes any uninitialized Array object");
            assert.areEqual(0, u.length, "Array object initialized by call to constructor with no arguments has length changed to zero");

            var u = Array[Symbol.create]();
            u[0] = 1;
            u[1] = 2;
            u[2] = 3;
            assert.isTrue(u === Array.call(u, 100), "Array constructor initializes any uninitialized Array object");
            assert.areEqual(100, u.length, "Array object initialized by call to constructor with single numeric argument has length changed to that argument");
            assert.areEqual([1,2,3], u, "Array object initialized by call to constructor retains existing elements");

            var u = Array[Symbol.create]();
            u[0] = 1;
            u[1] = 2;
            u[2] = 3;
            assert.isTrue(u === Array.call(u, 50.0), "Array constructor initializes any uninitialized Array object");
            assert.areEqual(50, u.length, "Array object initialized by call to constructor with single float argument has length changed to that argument");
            assert.areEqual([1,2,3], u, "Array object initialized by call to constructor retains existing elements");

            var u = Array[Symbol.create]();
            u[0] = 1;
            u[1] = 2;
            u[2] = 3;
            assert.isTrue(u === Array.call(u, 'non-number'), "Array constructor initializes any uninitialized Array object");
            assert.areEqual(1, u.length, "Array object initialized by call to constructor with single non-numeric argument has length changed 1");
            assert.areEqual(['non-number'], u, "Array object initialized by call to constructor loses any existing elements when constructor is called with single, non-numeric argument");

            var u = Array[Symbol.create]();
            u[0] = 1;
            u[1] = 2;
            u[2] = 3;
            assert.isTrue(u === Array.call(u, 10, 20), "Array constructor initializes any uninitialized Array object");
            assert.areEqual(2, u.length, "Array object initialized by call to constructor with multiple arguments has length changed to the count of those arguments");
            assert.areEqual([10,20], u, "Array object initialized by call to constructor loses any existing elements when constructor is called with multiple arguments");
        }
    },
    {
        name: "Subclass of Array",
        body: function () {
            class MyArray extends Array {
                constructor(...val) {
                    this.prop = 'myarray';
                    super(...val);
                }

                method() {
                    return this.prop;
                }
            }

            assert.areEqual('myarray', new MyArray().method(), "Subclass of Array has correct methods and properties");
            assert.areEqual(0, new MyArray().length, "Subclass of Array object has correct length when constructor called with no arguments");
            assert.areEqual(100, new MyArray(100).length, "Subclass of Array object has correct length when constructor called with single numeric argument");
            assert.areEqual(50, new MyArray(50.0).length, "Subclass of Array object has correct length when constructor called with single float argument");
            assert.areEqual(1, new MyArray('something').length, "Subclass of Array object has correct length when constructor called with single non-numeric argument");
            assert.areEqual('something', new MyArray('something')[0], "Subclass of Array object has correct length when constructor called with single non-numeric argument");

            var a = new MyArray(1,2,3);
            assert.areEqual(3, a.length, "Subclass of Array object has correct length when constructor called with multiple arguments");
            assert.areEqual(1, a[0], "Subclass of Array object has correct values when constructor called with multiple arguments");
            assert.areEqual(2, a[1], "Subclass of Array object has correct values when constructor called with multiple arguments");
            assert.areEqual(3, a[2], "Subclass of Array object has correct values when constructor called with multiple arguments");

            assert.isTrue(Array.isArray(a), "Subclass of Array is an array as tested via Array.isArray");
        }
    },
    {
        name: "Uninitialised Array object with existing length is observable during Array constructor initialization",
        body: function () {
            var u = Array[Symbol.create]();
            var s = false;

            u[1] = 2;
            u[2] = 3;

            Object.defineProperty(Array.prototype, 0, { set:
                    function() {
                        s = true;
                        assert.areEqual(3, u.length, "Array constructor doesn't set uninitialized Array object length until after copying elements");
                    }
                }
            );

            Array.call(u, {});

            assert.isTrue(s, "Array constructor called setter when copying elements");
            assert.areEqual(1, u.length, "Array constructor sets uninitialized Array object length to 1 when initializing with a single object");
        }
    }
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
