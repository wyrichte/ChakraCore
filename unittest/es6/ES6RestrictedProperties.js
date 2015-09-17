// ES6 restricted property tests 

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Restricted properties of Function.prototype",
        body: function () {
            var obj = Function.prototype;
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Function.prototype does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Function.prototype does not report that it has own property 'arguments'")
            
            var p = Object.getOwnPropertyDescriptor(obj, 'caller');
            assert.areEqual('{"enumerable":false,"configurable":false}', JSON.stringify(p), "Function.prototype function has 'caller' own property")
            assert.areEqual('function', typeof p.get, "Function.prototype['caller'] has get accessor function");
            assert.areEqual('function', typeof p.set, "Function.prototype['caller'] has set accessor function");
            assert.throws(function() { p.get(); }, TypeError, "Function.prototype['caller'] has get accessor which throws");
            assert.throws(function() { p.set(); }, TypeError, "Function.prototype['caller'] has set accessor which throws");
            assert.isTrue(p.get === p.set, "Function.prototype returns the same ThrowTypeError function for get/set accessor of 'caller' property");
            
            p = Object.getOwnPropertyDescriptor(obj, 'arguments');
            assert.areEqual('{"enumerable":false,"configurable":false}', JSON.stringify(p), "Function.prototype function has 'arguments' own property")
            assert.areEqual('function', typeof p.get, "Function.prototype['arguments'] has get accessor function");
            assert.areEqual('function', typeof p.set, "Function.prototype['arguments'] has set accessor function");
            assert.throws(function() { p.get(); }, TypeError, "Function.prototype['arguments'] has get accessor which throws");
            assert.throws(function() { p.set(); }, TypeError, "Function.prototype['arguments'] has set accessor which throws");
            assert.isTrue(p.get === p.set, "Function.prototype returns the same ThrowTypeError function for get/set accessor of 'arguments' property");
            
            assert.areEqual('["apply","arguments","bind","call","caller","constructor","length","name","toMethod","toString"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Function.prototype function has 'caller' and 'arguments' own properties");
            
            assert.throws(function() { obj.caller; }, TypeError, "Function.prototype throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Function.prototype throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            assert.throws(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, TypeError, "Function.prototype has 'arguments' property as non-configurable", "Cannot redefine non-configurable property 'arguments'");
            assert.throws(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, TypeError, "Function.prototype has 'caller' property as non-configurable", "Cannot redefine non-configurable property 'caller'");
        }
    },
    {
        name: "Restricted properties of non-strict function",
        body: function () {
            var obj = function() {};
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "non-strict function does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "non-strict function does not report that it has own property 'arguments'")
            
            assert.areEqual('{"value":null,"writable":false,"enumerable":false,"configurable":false}', JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "non-strict function has 'caller' own property")
            assert.areEqual('{"value":null,"writable":false,"enumerable":false,"configurable":false}', JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "non-strict function has 'arguments' own property");
            assert.areEqual('["arguments","caller","length","name","prototype"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "non-strict function has 'caller' and 'arguments' own properties");
            
            assert.areEqual(null, obj.caller, "'caller' property of non-strict function is null")
            assert.areEqual(null, obj.arguments, "'arguments' property of non-strict function is null")
            
            assert.throws(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, TypeError, "non-strict function has 'arguments' property as non-writable, non-configurable", "Cannot modify non-writable property 'arguments'");
            assert.throws(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, TypeError, "non-strict function has 'caller' property as non-writable, non-configurable", "Cannot modify non-writable property 'caller'");
        }
    },
    {
        name: "Restricted properties of strict function",
        body: function () {
            var obj = function() { 'use strict'; };
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Strict function does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Strict function does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Strict function does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Strict function does not have 'arguments' own property");
            assert.areEqual('["length","name","prototype"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Strict function does not have 'caller' and 'arguments' own properties");
            
            assert.throws(function() { obj.caller; }, TypeError, "Strict function throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Strict function throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, "Strict function has doesn't have own 'arguments' property");
            assert.areEqual(123, obj.arguments, "Strict function can have an own property defined for 'arguments'")
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, "Strict function doesn't have own 'caller' property");
            assert.areEqual(123, obj.caller, "Strict function can have an own property defined for 'caller'")
        }
    },
    {
        name: "Restricted properties of class",
        body: function () {
            var obj = class A { };
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Class does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Class does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Class does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Class does not have 'arguments' own property");
            assert.areEqual('["length","name","prototype"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Class does not have 'caller' and 'arguments' own properties");
            
            assert.throws(function() { obj.caller; }, TypeError, "Class throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Class throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, "Class has doesn't have own 'arguments' property");
            assert.areEqual(123, obj.arguments, "Class can have an own property defined for 'arguments'")
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, "Class doesn't have own 'caller' property");
            assert.areEqual(123, obj.caller, "Class can have an own property defined for 'caller'")
        }
    },
    {
        name: "Restricted properties of class static method",
        body: function () {
            class A {
                static static_method() { }
            };
            var obj = A.static_method;
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Class static method does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Class static method does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Class static method does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Class static method does not have 'arguments' own property");
            assert.areEqual('["length","name"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Class static method does not have 'caller' and 'arguments' own properties");
            
            assert.throws(function() { obj.caller; }, TypeError, "Class static method throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Class static method throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, "Class static method doesn't have own 'arguments' property");
            assert.areEqual(123, obj.arguments, "Class static method can have an own property defined for 'arguments'")
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, "Class static method doesn't have own 'caller' property");
            assert.areEqual(123, obj.caller, "Class static method can have an own property defined for 'caller'")
        }
    },
    {
        name: "Restricted properties of class method",
        body: function () {
            class A {
                method() { }
            };
            var obj = new A().method;
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Class method does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Class method does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Class method does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Class method does not have 'arguments' own property");
            assert.areEqual('["length","name"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Class method does not have 'caller' and 'arguments' own properties");
            
            assert.throws(function() { obj.caller; }, TypeError, "Class method throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Class method throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, "Class method doesn't have own 'arguments' property");
            assert.areEqual(123, obj.arguments, "Class method can have an own property defined for 'arguments'")
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, "Class method doesn't have own 'caller' property");
            assert.areEqual(123, obj.caller, "Class method can have an own property defined for 'caller'")
        }
    },
    {
        name: "Restricted properties of class with 'caller' static method",
        body: function () {
            var obj = class A { 
                static caller() { return 42; }
            };
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Class does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Class does not report that it has own property 'arguments'")
            
            assert.areEqual('{"writable":true,"enumerable":false,"configurable":true}', JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Class does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Class does not have 'arguments' own property");
            assert.areEqual('["caller","length","name","prototype"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Class does not have 'caller' and 'arguments' own properties");
            
            assert.areEqual(42, obj.caller(), "Accessing the 'caller' property is not restricted");
            assert.throws(function() { obj.arguments; }, TypeError, "Class throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
        }
    },
    {
        name: "Restricted properties of class with 'arguments' static get method",
        body: function () {
            var obj = class A { 
                static get arguments() { return 42; }
            };
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Class does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Class does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Class does not have 'caller' own property")
            assert.areEqual('{"enumerable":false,"configurable":true}', JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Class has 'arguments' own property");
            assert.areEqual('["arguments","length","name","prototype"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Class has 'arguments' own property, no 'caller' own property");
            
            assert.throws(function() { obj.caller; }, TypeError, "Class method throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.areEqual(42, obj.arguments, "Accessing the 'arguments' property is not restricted");
        }
    },
    {
        name: "Restricted properties of class with 'arguments' set method",
        body: function () {
            var my_v;
            class A { 
                set arguments(v) { my_v = v; }
            };
            var obj = A;
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Class does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Class does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Class does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Class has 'arguments' own property");
            assert.areEqual('["length","name","prototype"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Class has 'arguments' own property, no 'caller' own property");
            
            assert.throws(function() { obj.caller; }, TypeError, "Class method throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Class method throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            new A().arguments = 50;
            assert.areEqual(50, my_v, "Accessing the 'arguments' property was not restricted");
        }
    },
    {
        name: "Restricted properties of lambda",
        body: function () {
            var obj = () => { }
            
            assert.isFalse(Object.hasOwnProperty(obj, 'caller'), "Lambda does not report that it has own property 'caller'")
            assert.isFalse(Object.hasOwnProperty(obj, 'arguments'), "Lambda does not report that it has own property 'arguments'")
            
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'caller')), "Lambda does not have 'caller' own property")
            assert.areEqual(undefined, JSON.stringify(Object.getOwnPropertyDescriptor(obj, 'arguments')), "Lambda does not have 'arguments' own property");
            assert.areEqual('["length","name"]', JSON.stringify(Object.getOwnPropertyNames(obj).sort()), "Lambda does not have 'caller' and 'arguments' own properties");
            
            assert.throws(function() { obj.caller; }, TypeError, "Lambda throws on access to 'caller' property", "Accessing the 'caller' property is restricted in this context");
            assert.throws(function() { obj.arguments; }, TypeError, "Lambda throws on access to 'arguments' property", "Accessing the 'arguments' property is restricted in this context");
            
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'arguments', { value: 123 }); }, "Lambda doesn't have own 'arguments' property");
            assert.areEqual(123, obj.arguments, "Lambda can have an own property defined for 'arguments'")
            assert.doesNotThrow(function() { Object.defineProperty(obj, 'caller', { value: 123 }); }, "Lambda doesn't have own 'caller' property");
            assert.areEqual(123, obj.caller, "Lambda can have an own property defined for 'caller'")
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
