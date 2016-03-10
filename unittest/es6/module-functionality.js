//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// ES6 Module functionality tests -- verifies functionality of import and export statements

WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");

function testModuleScript(source, message, shouldFail) {
    let testfunc = () => WScript.LoadModule(source, 'samethread');

    if (shouldFail) {
        let caught = false;

        // We can't use assert.throws here because the SyntaxError used to construct the thrown error
        // is from a different context so it won't be strictly equal to our SyntaxError.
        try {
            testfunc();
        } catch(e) {
            caught = true;

            // Compare toString output of SyntaxError and other context SyntaxError constructor.
            assert.areEqual(e.constructor.toString(), SyntaxError.toString(), message);
        }

        assert.isTrue(caught, `Expected error not thrown: ${message}`);
    } else {
        assert.doesNotThrow(testfunc, message);
    }
}

var tests = [
    {
        name: "Validate a simple module export",
        body: function () {
            let functionBody = 
                `import { foo } from 'ModuleSimpleExport.js';
                assert.areEqual('foo', foo(), 'Failed to import foo from ModuleSimpleExport.js');`;
            testModuleScript(functionBody, "Test importing a simple exported function", false);
        }
    },
    {
        name: "Validate importing from multiple modules",
        body: function () {
            let functionBody = 
                `import { foo } from 'ModuleSimpleExport.js';
                assert.areEqual('foo', foo(), 'Failed to import foo from ModuleSimpleExport.js');
                import { foo2 } from 'ModuleComplexExports.js';
                assert.areEqual('foo', foo2(), 'Failed to import foo2 from ModuleComplexExports.js');`;
            testModuleScript(functionBody, "Test importing from multiple modules", false);
        }
    },
    {
        name: "Validate a variety of more complex exports",
        body: function () {
            let functionBody = 
                `import { foo, foo2 } from 'ModuleComplexExports.js';
                assert.areEqual('foo', foo(), 'Failed to import foo from ModuleComplexExports.js');
                assert.areEqual('foo', foo2(), 'Failed to import foo2 from ModuleComplexExports.js');
                import { bar, bar2 } from 'ModuleComplexExports.js';
                assert.areEqual('bar', bar(), 'Failed to import bar from ModuleComplexExports.js');
                assert.areEqual('bar', bar2(), 'Failed to import bar2 from ModuleComplexExports.js');
                import { let2, let3, let4, let5 } from 'ModuleComplexExports.js';
                assert.areEqual('let2', let2, 'Failed to import let2 from ModuleComplexExports.js');
                assert.areEqual('let3', let3, 'Failed to import let3 from ModuleComplexExports.js');
                assert.areEqual('let2', let4, 'Failed to import let4 from ModuleComplexExports.js');
                assert.areEqual('let3', let5, 'Failed to import let5 from ModuleComplexExports.js');
                import { const2, const3, const4, const5 } from 'ModuleComplexExports.js';
                assert.areEqual('const2', const2, 'Failed to import const2 from ModuleComplexExports.js');
                assert.areEqual('const3', const3, 'Failed to import const3 from ModuleComplexExports.js');
                assert.areEqual('const2', const4, 'Failed to import const4 from ModuleComplexExports.js');
                assert.areEqual('const3', const5, 'Failed to import const5 from ModuleComplexExports.js');
                import { var2, var3, var4, var5 } from 'ModuleComplexExports.js';
                assert.areEqual('var2', var2, 'Failed to import var2 from ModuleComplexExports.js');
                assert.areEqual('var3', var3, 'Failed to import var3 from ModuleComplexExports.js');
                assert.areEqual('var2', var4, 'Failed to import var4 from ModuleComplexExports.js');
                assert.areEqual('var3', var5, 'Failed to import var5 from ModuleComplexExports.js');
                import { class2, class3, class4, class5 } from 'ModuleComplexExports.js';
                assert.areEqual('class2', class2.static_member(), 'Failed to import class2 from ModuleComplexExports.js');
                assert.areEqual('class2', new class2().member(), 'Failed to create intance of class2 from ModuleComplexExports.js');
                assert.areEqual('class2', class3.static_member(), 'Failed to import class3 from ModuleComplexExports.js');
                assert.areEqual('class2', new class3().member(), 'Failed to create intance of class3 from ModuleComplexExports.js');
                assert.areEqual('class4', class4.static_member(), 'Failed to import class4 from ModuleComplexExports.js');
                assert.areEqual('class4', new class4().member(), 'Failed to create intance of class4 from ModuleComplexExports.js');
                assert.areEqual('class4', class5.static_member(), 'Failed to import class4 from ModuleComplexExports.js');
                assert.areEqual('class4', new class5().member(), 'Failed to create intance of class4 from ModuleComplexExports.js');
                import _default from 'ModuleComplexExports.js';
                assert.areEqual('default', _default(), 'Failed to import default from ModuleComplexExports.js');
                `;
            testModuleScript(functionBody, "Test importing a variety of exports", false);
        }
    },
    {
        name: "Import an export as a different binding identifier",
        body: function () {
            let functionBody = 
                `import { foo as foo3 } from 'ModuleSimpleExport.js';
                assert.areEqual('foo', foo3(), 'Failed to import foo from ModuleSimpleExport.js');
                import { foo2 as foo4 } from 'ModuleComplexExports.js';
                assert.areEqual('foo', foo4(), 'Failed to import foo4 from ModuleComplexExports.js');`;
            testModuleScript(functionBody, "Test importing as different binding identifiers", false);
        }
    },
    {
        name: "Import the same export under multiple local binding identifiers",
        body: function () {
            let functionBody = 
                `import { foo as foo3, foo as foo4 } from 'ModuleSimpleExport.js';
                assert.areEqual('foo', foo3(), 'Failed to import foo from ModuleSimpleExport.js');
                assert.areEqual('foo', foo4(), 'Failed to import foo from ModuleSimpleExport.js');
                assert.isTrue(foo3 === foo4, 'Export has the same value even if rebound');`;
            testModuleScript(functionBody, "Test importing the same export under multiple binding identifier", false);
        }
    },
    {
        name: "Exporting module changes exported value",
        body: function () {
            let functionBody = 
                `import { target, changeTarget } from 'ModuleComplexExports.js';
                assert.areEqual('before', target(), 'Failed to import target from ModuleComplexExports.js');
                assert.areEqual('ok', changeTarget(), 'Failed to import changeTarget from ModuleComplexExports.js');
                assert.areEqual('after', target(), 'changeTarget failed to change export value');`;
            testModuleScript(functionBody, "Changing exported value", false);
        }
    },
    {
        name: "Simple re-export forwards import to correct slot",
        body: function () {
            let functionBody = 
                `import { foo } from 'ModuleSimpleReexport.js';
                assert.areEqual('foo', foo(), 'Failed to import foo from ModuleSimpleReexport.js');`;
            testModuleScript(functionBody, "Simple re-export from one module to another", false);
        }
    },
    {
        name: "Import of renamed re-export forwards import to correct slot",
        body: function () {
            let functionBody = 
                `import { foo as baz } from 'ModuleSimpleReexport.js';
                assert.areEqual('foo', baz(), 'Failed to import foo from ModuleSimpleReexport.js');`;
            testModuleScript(functionBody, "Rename simple re-export", false);
        }
    },
    {
        name: "Renamed re-export and renamed import",
        body: function () {
            let functionBody = 
                `import { foo as baz } from 'ModuleComplexReexports.js';
                assert.areEqual('bar', baz(), 'Failed to import foo from ModuleComplexReexports.js');`;
            testModuleScript(functionBody, "Rename already renamed re-export", false);
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
