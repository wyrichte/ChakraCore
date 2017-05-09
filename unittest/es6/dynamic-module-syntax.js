//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// ES6 Module syntax tests -- verifies syntax of import and export statements

WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");

function testDynamicImport(importFunc, thenFunc, catchFunc, _asyncEnter, _asyncExit) {
    var promise = importFunc();
    assert.isTrue(promise instanceof Promise);
    promise.then((v)=>{
        _asyncEnter();
        thenFunc(v);
        _asyncExit();
    }).catch((err)=>{
        _asyncEnter();
        catchFunc(err);
        _asyncExit();
    });
}

var testDynamicImportSyntaxError = (function() {
    var moduleIndex = 0;
    return function (source, message, explicitAsync = false) {
        var moduleSourceFile = `moduleSource${moduleIndex}.js`;
        WScript.RegisterModuleSource(moduleSourceFile, source);
        var testCall = `
                testDynamicImport(
                    ()=>{ return import('${moduleSourceFile}'); },
                    (v)=>{
                        assert.isTrue(false, "Promise from import() expected to be rejected due to early error");
                    },
                    (err)=>{
                        assert.areEqual(SyntaxError, err.constructor, "Expect SyntaxError object to be caught");
                        assert.isTrue(err instanceof Error, "Expect Error object to be caught");
                    }, _asyncEnter, _asyncExit);

            `;
        let testFuncScript = () => testRunner.LoadScript(testCall, 'samethread', false, explicitAsync);
        assert.doesNotThrow(testFuncScript, message);
        let testFuncModule = () => testRunner.LoadModule(testCall, 'samethread', false, explicitAsync);
        assert.doesNotThrow(testFuncModule, message);
        moduleIndex++;
    };
})();


var tests = [
    {
        name: "Syntax error export statements",
        body: function () {
            testDynamicImportSyntaxError('export const const1;', 'Syntax error if const decl is missing initializer', true);
            testDynamicImportSyntaxError('function foo() { }; export foo;', "Syntax error if we're trying to export an identifier without default or curly braces", true);
            testDynamicImportSyntaxError('export function () { }', 'Syntax error if function declaration is missing binding identifier', true);
            testDynamicImportSyntaxError('export function* () { }', 'Syntax error if generator declaration is missing binding identifier', true);
            testDynamicImportSyntaxError('export class { }', 'Syntax error if class declaration is missing binding identifier', true);
            testDynamicImportSyntaxError('function foo() { }; export [ foo ];', 'Syntax error if we use brackets instead of curly braces in export statement', true);
            testDynamicImportSyntaxError('function foo() { export default function() { } }', 'Syntax error if export statement is in a nested function', true);
            testDynamicImportSyntaxError('function foo() { }; export { , foo };', 'Syntax error if named export list contains an empty element', true);
            testDynamicImportSyntaxError('function foo() { }; () => { export { foo }; }', 'Syntax error if export statement is in arrow function', true);
            testDynamicImportSyntaxError('function foo() { }; try { export { foo }; } catch(e) { }', 'Syntax error if export statement is in try catch statement', true);
            testDynamicImportSyntaxError('function foo() { }; { export { foo }; }', 'Syntax error if export statement is in any block', true);
            testDynamicImportSyntaxError('export default 1, 2, 3;', "Export default takes an assignment expression which doesn't allow comma expressions", true);
            testDynamicImportSyntaxError('export 12;', 'Syntax error if export is followed by non-identifier', true);
            testDynamicImportSyntaxError("export 'string_constant';", 'Syntax error if export is followed by string constant', true);
            testDynamicImportSyntaxError('export ', 'Syntax error if export is followed by EOF', true);
            testDynamicImportSyntaxError('function foo() { }; export { foo as 100 };', 'Syntax error in named export clause if trying to export as numeric constant', true);
        }
    },
    {
        name: "Syntax error import statements",
        body: function () {
            testDynamicImportSyntaxError('function foo() { import foo from "ValidExportStatements.js"; }', 'Syntax error if import statement is in nested function', true);
            testDynamicImportSyntaxError('import foo, bar from "ValidExportStatements.js";', 'Syntax error if import statement has multiple default bindings', true);
            testDynamicImportSyntaxError('import { foo, foo } from "ValidExportStatements.js";', 'Redeclaration error if multiple imports have the same local name', true);
            testDynamicImportSyntaxError('import { foo, bar as foo } from "ValidExportStatements.js";', 'Redeclaration error if multiple imports have the same local name', true);
            testDynamicImportSyntaxError('const foo = 12; import { foo } from "ValidExportStatements.js";', 'Syntax error if module body has a const declaration bound to the same name as a module import', true);
            testDynamicImportSyntaxError('function foo() { }; import { foo } from "ValidExportStatements.js";', 'Syntax error if module body has a function declaration bound to the same name as a module import', true);
            testDynamicImportSyntaxError('import foo;', 'Syntax error if import statement is missing from clause', true);
            testDynamicImportSyntaxError('import * as foo, from "ValidExportStatements.js";', 'Syntax error if import statement has comma after namespace import', true);
            testDynamicImportSyntaxError('import * as foo, bar from "ValidExportStatements.js";', 'Syntax error if import statement has default binding after namespace import', true);
            testDynamicImportSyntaxError('import * as foo, { bar } from "ValidExportStatements.js";', 'Syntax error if import statement has named import list after namespace import', true);
            testDynamicImportSyntaxError('import { foo }, from "ValidExportStatements.js";', 'Syntax error if import statement has comma after named import list', true);
            testDynamicImportSyntaxError('import { foo }, bar from "ValidExportStatements.js";', 'Syntax error if import statement has default binding after named import list', true);
            testDynamicImportSyntaxError('import { foo }, * as ns1 from "ValidExportStatements.js";', 'Syntax error if import statement has namespace import after named import list', true);
            testDynamicImportSyntaxError('import { foo }', 'Syntax error if import statement is missing from clause', true);
            testDynamicImportSyntaxError('import [ foo ] from "ValidExportStatements.js";', 'Syntax error if named import clause uses brackets', true);
            testDynamicImportSyntaxError('import * foo from "ValidExportStatements.js";', 'Syntax error if namespace import is missing "as" keyword', true);
            testDynamicImportSyntaxError('import * as "foo" from "ValidExportStatements.js";', 'Syntax error if namespace imported binding name is not identifier', true);
            testDynamicImportSyntaxError('import { , foo } from "ValidExportStatements.js";', 'Syntax error if named import list contains an empty element', true);
            testDynamicImportSyntaxError('import foo from "ValidExportStatements.js"; import foo from "ValidExportStatements.js";', 'Default import cannot be bound to the same symbol', true);
            testDynamicImportSyntaxError('import { foo } from "ValidExportStatements.js"; import { foo } from "ValidExportStatements.js";', 'Multiple named imports cannot be bound to the same symbol', true);
            testDynamicImportSyntaxError('import * as foo from "ValidExportStatements.js"; import * as foo from "ValidExportStatements.js";', 'Multiple namespace imports cannot be bound to the same symbol', true);
            testDynamicImportSyntaxError('import { foo as bar, bar } from "ValidExportStatements.js";', 'Named import clause may not contain multiple binding identifiers with the same name', true);
            testDynamicImportSyntaxError('import foo from "ValidExportStatements.js"; import * as foo from "ValidExportStatements.js";', 'Imported bindings cannot be overwritten by later imports', true);
            testDynamicImportSyntaxError('() => { import arrow from ""; }', 'Syntax error if import statement is in arrow function', true);
            testDynamicImportSyntaxError('try { import _try from ""; } catch(e) { }', 'Syntax error if import statement is in try catch statement', true);
            testDynamicImportSyntaxError('{ import in_block from ""; }', 'Syntax error if import statement is in any block', true);
            testDynamicImportSyntaxError('import {', 'Named import clause which has EOF after left curly', true);
            testDynamicImportSyntaxError('import { foo', 'Named import clause which has EOF after identifier', true);
            testDynamicImportSyntaxError('import { foo as ', 'Named import clause which has EOF after identifier as', true);
            testDynamicImportSyntaxError('import { foo as bar ', 'Named import clause which has EOF after identifier as identifier', true);
            testDynamicImportSyntaxError('import { foo as bar, ', 'Named import clause which has EOF after identifier as identifier comma', true);
            testDynamicImportSyntaxError('import { switch } from "module";', 'Named import clause which has non-identifier token as the first token', true);
            testDynamicImportSyntaxError('import { foo bar } from "module";', 'Named import clause missing "as" token', true);
            testDynamicImportSyntaxError('import { foo as switch } from "module";', 'Named import clause with non-identifier token after "as"', true);
            testDynamicImportSyntaxError('import { foo, , } from "module";', 'Named import clause with too many trailing commas', true);
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
