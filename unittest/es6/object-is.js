// ES6 Object.is(x,y) API extension tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Object.is should exist and have length 2",
        body: function () {
            assert.isTrue(Object.hasOwnProperty('is'), "Object should have an is method");
            assert.areEqual(2, Object.is.length, "is method takes two arguments");
        }
    },
    {
        name: "Object.is(undefined, y) returns true for y = undefined, false otherwise",
        body: function () {
            assert.isTrue(Object.is(undefined, undefined), "Object.is(undefined, y) returns true for Type(y) = Undefined");
            assert.isFalse(Object.is(undefined, null), "Object.is(undefined, y) returns false for Type(y) = Null");
            assert.isFalse(Object.is(undefined, false), "Object.is(undefined, y) returns false for Type(y) = Boolean");
            assert.isFalse(Object.is(undefined, ""), "Object.is(undefined, y) returns false for Type(y) = String");
            assert.isFalse(Object.is(undefined, Symbol()), "Object.is(undefined, y) returns false for Type(y) = Symbol");
            assert.isFalse(Object.is(undefined, 0), "Object.is(undefined, y) returns false for Type(y) = Number");
            assert.isFalse(Object.is(undefined, { }), "Object.is(undefined, y) returns false for Type(y) = Object");
        }
    },
    {
        name: "Object.is(null, y) returns true for y = null, false otherwise",
        body: function () {
            assert.isFalse(Object.is(null, undefined), "Object.is(null, y) returns false for Type(y) = Undefined");
            assert.isTrue(Object.is(null, null), "Object.is(null, y) returns true for Type(y) = Null");
            assert.isFalse(Object.is(null, false), "Object.is(null, y) returns false for Type(y) = Boolean");
            assert.isFalse(Object.is(null, ""), "Object.is(null, y) returns false for Type(y) = String");
            assert.isFalse(Object.is(null, Symbol()), "Object.is(null, y) returns false for Type(y) = Symbol");
            assert.isFalse(Object.is(null, 0), "Object.is(null, y) returns false for Type(y) = Number");
            assert.isFalse(Object.is(null, { }), "Object.is(null, y) returns false for Type(y) = Object");
        }
    },
    {
        name: "Object.is(x, y), where Type(x) is Number, returns true for y = x (bitwise), false otherwise",
        body: function () {
            assert.isFalse(Object.is(0, undefined), "Object.is(0, y) returns false for Type(y) = Undefined");
            assert.isFalse(Object.is(0, null), "Object.is(0, y) returns false for Type(y) = Null");
            assert.isFalse(Object.is(0, false), "Object.is(0, y) returns false for Type(y) = Boolean");
            assert.isFalse(Object.is(0, ""), "Object.is(0, y) returns false for Type(y) = String");
            assert.isFalse(Object.is(0, Symbol()), "Object.is(0, y) returns false for Type(y) = Symbol");
            assert.isTrue(Object.is(0, 0), "Object.is(0, y) returns true for Type(y) = Number");
            assert.isFalse(Object.is(0, { }), "Object.is(0, y) returns false for Type(y) = Object");

            assert.isTrue(Object.is(NaN, NaN), "Object.is(NaN, NaN) returns true");
            assert.isFalse(Object.is(+0, -0), "Object.is(+0, -0) returns false");
            assert.isFalse(Object.is(-0, +0), "Object.is(+0, -0) returns false");

            assert.isTrue(Object.is(10, 10), "Object.is(10, 10) returns true");
            assert.isFalse(Object.is(10, -10), "Object.is(10, 10) returns false");

            assert.isTrue(Object.is(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY), "Object.is(+Infinity, +Infinity) returns true");
            assert.isTrue(Object.is(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY), "Object.is(-Infinity, -Infinity) returns true");
            assert.isFalse(Object.is(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY), "Object.is(+Infinity, -Infinity) returns false");
        }
    },
    {
        name: "Object.is(x, y), where Type(x) is String, returns true when Type(y) is String and x and y have the same sequence of code points, false otherwise",
        body: function () {
            assert.isFalse(Object.is("", undefined), "Object.is('', y) returns false for Type(y) = Undefined");
            assert.isFalse(Object.is("", null), "Object.is('', y) returns false for Type(y) = Null");
            assert.isFalse(Object.is("", false), "Object.is('', y) returns false for Type(y) = Boolean");
            assert.isTrue(Object.is("", ""), "Object.is('', y) returns true for Type(y) = String where x == y");
            assert.isFalse(Object.is("", Symbol()), "Object.is('', y) returns false for Type(y) = Symbol");
            assert.isFalse(Object.is("", 0), "Object.is('', y) returns false for Type(y) = Number");
            assert.isFalse(Object.is("", { }), "Object.is('', y) returns false for Type(y) = Object");

            assert.isTrue(Object.is("abc", "abc"), "Object.is('abc', 'abc') returns true");
            assert.isFalse(Object.is("abc", "xyz"), "Object.is('abc', 'xyz') returns false");
        }
    },
    {
        name: "Object.is(x, y), where Type(x) is Boolean, returns true when Type(y) is Boolean and x = y, false otherwise",
        body: function () {
            assert.isFalse(Object.is(false, undefined), "Object.is(false, y) returns false for Type(y) = Undefined");
            assert.isFalse(Object.is(false, null), "Object.is(false, y) returns false for Type(y) = Null");
            assert.isTrue(Object.is(false, false), "Object.is(false, y) returns true for Type(y) = Boolean where x == y");
            assert.isFalse(Object.is(false, ""), "Object.is(false, y) returns false for Type(y) = String");
            assert.isFalse(Object.is(false, Symbol()), "Object.is(false, y) returns false for Type(y) = Symbol");
            assert.isFalse(Object.is(false, 0), "Object.is(false, y) returns false for Type(y) = Number");
            assert.isFalse(Object.is(false, { }), "Object.is(false, y) returns false for Type(y) = Object");

            assert.isTrue(Object.is(true, true), "Object.is(true, true) returns true");
            assert.isFalse(Object.is(false, true), "Object.is(false, true) returns false");
        }
    },
    {
        name: "Object.is(x, y), where Type(x) is Symbol, returns true when Type(y) is Symbol and x and y are the same symbol, false otherwise",
        body: function () {
            var sym = Symbol();

            assert.isFalse(Object.is(sym, undefined), "Object.is(Symbol(), y) returns false for Type(y) = Undefined");
            assert.isFalse(Object.is(sym, null), "Object.is(Symbol(), y) returns false for Type(y) = Null");
            assert.isFalse(Object.is(sym, false), "Object.is(Symbol(), y) returns false for Type(y) = Boolean");
            assert.isFalse(Object.is(sym, ""), "Object.is(Symbol(), y) returns false for Type(y) = String");
            assert.isTrue(Object.is(sym, sym), "Object.is(x, y) returns true when x and y are the same symbol");
            assert.isFalse(Object.is(sym, 0), "Object.is(Symbol(), y) returns false for Type(y) = Number");
            assert.isFalse(Object.is(sym, { }), "Object.is(Symbol(), y) returns false for Type(y) = Object");

            assert.isFalse(Object.is(sym, Symbol()), "Object.is(x, y) returns false where x and y are different symbols");
        }
    },
    {
        name: "Object.is(x, y), where Type(x) is Symbol, returns true when Type(y) is Symbol and x and y are the same symbol, false otherwise",
        body: function () {
            var o = { };

            assert.isFalse(Object.is(o, undefined), "Object.is(Symbol(), y) returns false for Type(y) = Undefined");
            assert.isFalse(Object.is(o, null), "Object.is(Symbol(), y) returns false for Type(y) = Null");
            assert.isFalse(Object.is(o, false), "Object.is(Symbol(), y) returns false for Type(y) = Boolean");
            assert.isFalse(Object.is(o, ""), "Object.is(Symbol(), y) returns false for Type(y) = String");
            assert.isFalse(Object.is(o, Symbol()), "Object.is(x, y) returns false when Type(y) = Symbol");
            assert.isFalse(Object.is(o, 0), "Object.is(Symbol(), y) returns false for Type(y) = Number");
            assert.isTrue(Object.is(o, o), "Object.is(Symbol(), y) returns true for Type(y) = Object where x and y are the same object");

            assert.isFalse(Object.is(o, { }), "Object.is(x, y) returns false when x and y are different objects");
        }
    },
    {
        name: "Object.is called with less or more than 2 arguments",
        body: function () {
            assert.isTrue(Object.is(), "Object.is() is the same as Object.is(undefined, undefined) which should return true");

            assert.isTrue(Object.is(undefined), "Object.is(undefined) is the same as Object.is(undefined, undefined) and returns true");
            assert.isFalse(Object.is(null), "Object.is(null) is the same as Object.is(null, undefined) and returns false");
            assert.isFalse(Object.is(false), "Object.is(false) is the same as Object.is(false, undefined) and returns false");
            assert.isFalse(Object.is(""), "Object.is('') is the same as Object.is('', undefined) and returns false");
            assert.isFalse(Object.is(Symbol()), "Object.is(Symbol()) is the same as Object.is(Symbol(), undefined) and returns false");
            assert.isFalse(Object.is(0), "Object.is(0) is the same as Object.is(0, undefined) and returns false");
            assert.isFalse(Object.is({ }), "Object.is({ }) is the same as Object.is({ }, undefined) and returns false");

            assert.isTrue(Object.is(0, 0, 1), "Object.is ignores arguments after the first two; ignores the 1");
            assert.isFalse(Object.is("", 0, false), "Object.is ignores arguments after the first two; ignores the false");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
