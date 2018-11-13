//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in ch
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Using correct environment when boxing stack function",
        body: function () {
            // Set up the bug:
            // Function 1 has a scope slot.
            // Function 2 has two deferred functions nested within.
            // Functions 1 and 2 are created on-stack because we don't know yet that something will escape.
            // Upon running Function 3, we discover an escape and box the enclosing functions.
            // However, Function 2 had already fetched its environment into a register, and that register is not updated to point to the newly boxed environment.
            // Function 4 is created pointing to the stack-allocated environment from Function 1 rather than the new boxed version.
            var a, b;
            "" + function () { // Function 1
                var local = 1;
                "" + function () { // Function 2
                    "" + function () { // Function 3
                        a = function () { return local; };
                    }();
                    "" + function () { // Function 4
                        b = function () {
                            var result = local; // read from scope
                            local = 2; // write to scope
                            return result;
                        };
                    }();
                }();
            }();

            // Exercising the bug:
            // Create a similar-shaped call stack so we can read and write a variable we shouldn't be able to access
            "" + function () {
                var local = 3;
                "" + function () {
                    "" + function () {
                        function c() { return local; };
                    }();
                    assert.areEqual(1, b(), "should read closure value not stack value");
                    assert.areEqual(3, local, "should have not written to stack value");
                }();
            }();

            assert.areEqual(2, b(), "should read closure value not invoke undefined behavior");
        }
    },
];

testRunner.runTests(tests, { verbose: false /*so no need to provide baseline*/ });