// ES6 Species Built-In APIs tests -- verifies the shape and basic behavior of the built-in [@@species] property

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "AV when calling slice on an array when using es6all flag",
        body: function () {
            var c = [];
            c[0] = 1;
            c[4294967294] = 2;
            Object.defineProperty(Array, Symbol.species, {enumerable: false, configurable: true, writable: true});
            assert.areEqual(c, c.slice(0));
        }
    },
    {
        name: "Flag 'isNotPathTypeHandlerOrHasUserDefinedCtor' should propagate in PathTypeHandler chain",
        body: function () {
            var arr = [1,2,3,4,5,6];
            arr.constructor = null;
            arr.x = 1;
            assert.throws(function() { Array.prototype.splice.call(arr, 0, 3); }, TypeError, "TypeError when [@@species] is not constructor", "Function '[@@species]' is not a constructor");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
