//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// Object Rest unit tests

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Test serialization with simple statement",
        body: function() {
            let {a, b, ...rest} = {a: 1, b: 2, c: 3, d: 4};
            assert.areEqual(1, a);
            assert.areEqual(2, b);
            assert.areEqual({c: 3, d: 4}, rest);
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });