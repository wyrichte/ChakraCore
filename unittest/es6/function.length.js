if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

function assertLengthIsConfigurable(f) {
    function getDescriptor() {
        return Object.getOwnPropertyDescriptor(f, "length");
    }

    var descriptor = getDescriptor();
    assert.areNotEqual(undefined, descriptor, "'length' property exists");
    assert.isTrue(descriptor.configurable, "'length' is configurable");

    Object.defineProperty(f, "length", { writable: true });
    assert.isTrue(getDescriptor().writable, "'length' property can be redefined");

    var newLength = f.length + 1;
    f.length = newLength;
    assert.areEqual(newLength, f.length, "'length' can be assigned to a number after being made writable");

    newLength = "non-number length";
    f.length = newLength;
    assert.areEqual(newLength, f.length, "'length' can be assigned to a non-number after being made writable");

    var deleted = delete f.length;
    assert.isTrue(deleted);
    assert.areEqual(undefined, getDescriptor(), "'length' can be deleted");
    assert.areEqual(Object.getPrototypeOf(f).length, f.length, "'length' falls back to prototype when it is deleted");
}

function assertBoundLength(expectedLength, originalFunctionLength, message) {
    function f(a) {}
    Object.defineProperty(f, "length", { value: originalFunctionLength });
    var bf = f.bind();
    assert.areEqual(expectedLength, bf.length, message);
}

var tests = [
    {
        name: "'length' property of a built-in function is configurable",
        body: function () {
            var builtIn = Function.prototype.apply;
            const originalDescriptor = Object.getOwnPropertyDescriptor(builtIn, "length");

            assertLengthIsConfigurable(builtIn);

            // Revert to the original descriptor in case it's needed in another test
            Object.defineProperty(builtIn, "length", originalDescriptor);
        }
    },
    {
        name: "'length' property of a script function is configurable",
        body: function () {
            function f(a) {}
            assertLengthIsConfigurable(f);
        }
    },
    {
        name: "'length' property of a bound function is initialized to zero when the original function's 'length' doesn't exist",
        body: function () {
            function f(a) {}
            delete f.length;
            var bf = f.bind();
            assert.areEqual(0, bf.length, "length");
        }
    },
    {
        name: "'length' property of a bound function is initialized using ToInteger()",
        body: function () {
            var int32Overflow = Math.pow(2, 42);
            assertBoundLength(int32Overflow, int32Overflow, "Can have values larger than maximum int32");
            assertBoundLength(Infinity, Infinity);
            assertBoundLength(3, 3.14, "floor(number)");
            assertBoundLength(0, "non-number");
            assertBoundLength(3, {valueOf: function() { return 3; }}, "ToPrimitive()");

            function assertThrows(originalLength) {
                function tryBind() {
                    function f() {}
                    Object.defineProperty(f, "length", { value: originalLength });
                    var bf = f.bind();
                    // Use the result of the "bind" so that we actually execute it.
                    bf.length;
                }
                assert.throws(tryBind, TypeError);
            }
            assertThrows(Symbol.iterator);
            assertThrows(Object.create(null));
        }
    },
    {
        name: "'length' property of a bound function should be at least zero",
        body: function () {
            var expectedLength = 0;
            var assertZero = assertBoundLength.bind(undefined, expectedLength);
            [-1, -Infinity, "-Infinity"].forEach(assertZero);
        }
    },
    {
        name: "'length' property of a bound function is configurable",
        body: function () {
            function f(a, b) {}
            var bf = f.bind();
            assertLengthIsConfigurable(bf);
        }
    },
    {
        name: "'length' property of a generator function is configurable",
        body: function () {
            function *gf(a) {}
            assertLengthIsConfigurable(gf);
        }
    }
]

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
