// ES6 Genertors APIs tests -- verifies built-in API objects and properties

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var globObj = this;

function checkAttributes(name, o, p, a) {
    var desc = Object.getOwnPropertyDescriptor(o, p);

    var msgPrefix = "Property " + p.toString() + " on " + name + " is ";

    assert.isTrue(!!desc, msgPrefix + " with a descriptor");

    assert.areEqual(a.writable, desc.writable, msgPrefix + (a.writable ? "" : "not") + " writable");
    assert.areEqual(a.enumerable, desc.enumerable, msgPrefix + (a.enumerable ? "" : "not") + " enumerable");
    assert.areEqual(a.configurable, desc.configurable, msgPrefix + (a.configurable ? "" : "not") + " configurable");
}

var tests = [
    {
        name: "GeneratorFunction is not exposed on the global object",
        body: function () {
            assert.isFalse(globObj.hasOwnProperty("GeneratorFunction"), "Global object does not have property named GeneratorFunction");
        }
    },
    {
        name: "Generator function object instances have length, arguments, caller, and prototype properties",
        body: function () {
            function* gf() { }

            assert.isTrue(gf.hasOwnProperty("length"), "Generator function objects have a 'length' property");
            assert.isTrue(gf.hasOwnProperty("arguments"), "Generator function objects have an 'arguments' property");
            assert.isTrue(gf.hasOwnProperty("caller"), "Generator function objects have a 'caller' property");
            assert.isTrue(gf.hasOwnProperty("prototype"), "Generator function objects have a 'prototype' property");

            checkAttributes("gf", gf, "length", { writable: false, enumerable: false, configurable: true });
            checkAttributes("gf", gf, "arguments", { enumerable: false, configurable: true });
            checkAttributes("gf", gf, "caller", { enumerable: false, configurable: true });
            checkAttributes("gf", gf, "prototype", { writable: false, enumerable: false, configurable: true });

            assert.isFalse(gf.prototype.hasOwnProperty("constructor"), "Generator function prototype objects do not get a 'constructor' property");
        }
    },
    {
        name: "arguments and caller properties are poisoned; they throw and are there to prevent confusion with legacy behavior",
        body: function () {
            function* gf() { }

            assert.throws(function () { gf.arguments; }, TypeError, "Get operation on 'arguments' property throws", "Object doesn't support this action");
            assert.throws(function () { gf.arguments = 0; }, TypeError, "Set operation on 'arguments' property throws", "Object doesn't support this action");
            assert.throws(function () { gf.caller; }, TypeError, "Get operation on 'caller' property throws", "Object doesn't support this action");
            assert.throws(function () { gf.caller = 0; }, TypeError, "Set operation on 'caller' property throws", "Object doesn't support this action");

            function* gfstrict() { "use strict"; }

            assert.throws(function () { gfstrict.arguments; }, TypeError, "Get operation on 'arguments' property throws", "Object doesn't support this action");
            assert.throws(function () { gfstrict.arguments = 0; }, TypeError, "Set operation on 'arguments' property throws", "Object doesn't support this action");
            assert.throws(function () { gfstrict.caller; }, TypeError, "Get operation on 'caller' property throws", "Object doesn't support this action");
            assert.throws(function () { gfstrict.caller = 0; }, TypeError, "Set operation on 'caller' property throws", "Object doesn't support this action");
        }
    },
    {
        name: "Generator functions' length property is the number of formal parameters",
        body: function () {
            function* gf0() { }
            function* gf1(a) { }
            function* gf5(a,b,c,d,e) { }

            assert.areEqual(0, gf0.length, "Generator function with no formal parameters has length 0");
            assert.areEqual(1, gf1.length, "Generator function with one formal parameter has length 1");
            assert.areEqual(5, gf5.length, "Generator function with five formal parameters has length 5");
        }
    },
    {
        name: "Generator function instances have GeneratorFunction.prototype as their prototype and it has the specified properties and prototype",
        body: function () {
            function* gf() { }
            var generatorFunctionPrototype = Object.getPrototypeOf(gf);

            assert.areEqual(Function.prototype, Object.getPrototypeOf(generatorFunctionPrototype), "GeneratorFunction.prototype's prototype is Function.prototype");

            assert.isTrue(generatorFunctionPrototype.hasOwnProperty("constructor"), "GeneratorFunction.prototype has 'constructor' property");
            assert.isTrue(generatorFunctionPrototype.hasOwnProperty("prototype"), "GeneratorFunction.prototype has 'prototype' property");
            assert.isTrue(generatorFunctionPrototype.hasOwnProperty(Symbol.toStringTag), "GeneratorFunction.prototype has [Symbol.toStringTag] property");

            checkAttributes("GeneratorFunction.prototype", generatorFunctionPrototype, "constructor", { writable: false, enumerable: false, configurable: true });
            checkAttributes("GeneratorFunction.prototype", generatorFunctionPrototype, "prototype", { writable: false, enumerable: false, configurable: true });
            checkAttributes("GeneratorFunction.prototype", generatorFunctionPrototype, Symbol.toStringTag, { writable: false, enumerable: false, configurable: true });

            assert.areEqual("GeneratorFunction", generatorFunctionPrototype[Symbol.toStringTag], "GeneratorFunction.prototype's [Symbol.toStringTag] property is 'GeneratorFunction'");
        }
    },
    {
        name: "GeneratorFunction constructor is the value of the constructor property of GeneratorFunction.prototype and has the specified properties and prototype",
        body: function () {
            function* gf() { }
            var generatorFunctionPrototype = Object.getPrototypeOf(gf);
            var generatorFunctionConstructor = generatorFunctionPrototype.constructor;

            assert.areEqual(Function, Object.getPrototypeOf(generatorFunctionConstructor), "GeneratorFunction's prototype is Function");

            assert.isTrue(generatorFunctionConstructor.hasOwnProperty("length"), "GeneratorFunction has 'length' property");
            assert.isTrue(generatorFunctionConstructor.hasOwnProperty("prototype"), "GeneratorFunction has 'prototype' property");

            checkAttributes("GeneratorFunction", generatorFunctionConstructor, "length", { writable: false, enumerable: false, configurable: true });
            checkAttributes("GeneratorFunction", generatorFunctionConstructor, "prototype", { writable: false, enumerable: false, configurable: false });

            assert.areEqual(generatorFunctionPrototype, generatorFunctionConstructor.prototype, "GeneratorFunction's 'prototype' property is GeneratorFunction.prototype");
            assert.areEqual(1, generatorFunctionConstructor.length, "GeneratorFunction's 'length' property is 1");
        }
    },
    {
        name: "Generator prototype is the value of the prototype property of GeneratorFunction.prototype and has the specified properties and prototype",
        body: function () {
            function* gf() { }
            var generatorFunctionPrototype = Object.getPrototypeOf(gf);
            var generatorPrototype = generatorFunctionPrototype.prototype;

            assert.areEqual(Object.prototype, Object.getPrototypeOf(generatorPrototype), "Generator prototype's prototype is Object.prototype");

            assert.isTrue(generatorPrototype.hasOwnProperty("constructor"), "Generator prototype has 'constructor' property");
            assert.isTrue(generatorPrototype.hasOwnProperty("next"), "Generator prototype has 'next' property");
            assert.isTrue(generatorPrototype.hasOwnProperty("throw"), "Generator prototype has 'throw' property");
            assert.isTrue(generatorPrototype.hasOwnProperty("return"), "Generator prototype has 'return' property");
            assert.isTrue(generatorPrototype.hasOwnProperty(Symbol.iterator), "Generator prototype has [Symbol.iterator] property");
            assert.isTrue(generatorPrototype.hasOwnProperty(Symbol.toStringTag), "Generator prototype has [Symbol.toStringTag] property");

            checkAttributes("Generator prototype", generatorPrototype, "constructor", { writable: false, enumerable: false, configurable: true });
            checkAttributes("Generator prototype", generatorPrototype, "next", { writable: true, enumerable: false, configurable: true });
            checkAttributes("Generator prototype", generatorPrototype, "throw", { writable: true, enumerable: false, configurable: true });
            checkAttributes("Generator prototype", generatorPrototype, "return", { writable: true, enumerable: false, configurable: true });
            checkAttributes("Generator prototype", generatorPrototype, Symbol.iterator, { writable: true, enumerable: false, configurable: true });
            checkAttributes("Generator prototype", generatorPrototype, Symbol.toStringTag, { writable: false, enumerable: false, configurable: true });

            assert.areEqual(generatorFunctionPrototype, generatorPrototype.constructor, "Generator prototype's 'constructor' property is GeneratorFunction.prototype");
            assert.areEqual("Generator", generatorPrototype[Symbol.toStringTag], "Generator prototype's [Symbol.toStringTag] property is 'Generator'");
        }
    },
    {
        name: "Generator object prototype by default is the function's .prototype property value whose prototype is the Generator prototype",
        body: function () {
            function* gf() { }
            var generatorFunctionPrototype = Object.getPrototypeOf(gf);
            var generatorPrototype = generatorFunctionPrototype.prototype;

            var g = gf();
            assert.areEqual(generatorPrototype, Object.getPrototypeOf(Object.getPrototypeOf(g)), "Generator object's prototype is an object whose prototype is Generator prototype");
            assert.areEqual(Object.getPrototypeOf(g), gf.prototype, "Generator object's prototype comes from generator function's .prototype property");
        }
    },
    {
        name: "Generator function's arguments.callee should be equal to the generator function object itself",
        body: function () {
            function* gf() {
                assert.isTrue(arguments.callee === gf, "arguments.callee should be the same function object pointed to by gf");

                var a = arguments;
                assert.throws(function () { a.callee.arguments; }, TypeError, "Sanity check: arguments should throw", "Object doesn't support this action");
                assert.throws(function () { a.callee.caller; }, TypeError, "Sanity check: caller should throw", "Object doesn't support this action");
            }

            gf().next();
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
