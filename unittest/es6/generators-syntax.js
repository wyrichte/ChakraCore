// ES6 Genertors Syntax tests -- verifies function* and yield syntax spec conformance

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Simple valid syntax forms",
        body: function () {
            assert.doesNotThrow(function () { eval("function* gf() { }"); }, "Simple generator function declaration is valid syntax");
            assert.doesNotThrow(function () { eval("var gfe = function* () { }"); }, "Simple generator function expression w/o name is valid syntax");
            assert.doesNotThrow(function () { eval("var gfe = function* rgfe() { }"); }, "Simple generator function expression w/ name is valid syntax");
            assert.doesNotThrow(function () { eval("class C { *gm() { } }"); }, "Simple generator method declaration is valid syntax");
            assert.doesNotThrow(function () { eval("var o = { *gcf() { } }"); }, "Simple generator concise function declaration is valid syntax");

            assert.doesNotThrow(function () { eval("function* gf() { yield; }"); }, "yield without operand is a valid expression statement");
            assert.doesNotThrow(function () { eval("function* gf() { yield }"); }, "yield without operand is a valid expression statement with implicit semicolon insertion");
            assert.doesNotThrow(function () { eval("function* gf() { var a = yield; }"); }, "yield without operand is a valid RHS expression");
            assert.doesNotThrow(function () { eval("function* gf() { foo(yield); }"); }, "yield without operand is a valid argument expression");
            assert.doesNotThrow(function () { eval("function* gf() { foo[yield]; }"); }, "yield without operand is a valid element expression");
            assert.doesNotThrow(function () { eval("function* gf() { yield, 10; }"); }, "yield without operand is a valid LHS of the comma operator");
            assert.doesNotThrow(function () { eval("function* gf() { switch (1) { case yield: break; } }"); }, "yield without operand is a valid switch case label operand");
            assert.doesNotThrow(function () { eval("var gfe = function* () { switch (1) { case yield: break; } }"); }, "yield without operand is a valid switch case label operand inside function expression w/o name");
            assert.doesNotThrow(function () { eval("var gfe = function* rgfe() { switch (1) { case yield: break; } }"); }, "yield without operand is a valid switch case label operand inside function expression w/ name");
            assert.doesNotThrow(function () { eval("var o = { *gf() { switch (1) { case yield: break; } } }"); }, "yield without operand is a valid switch case label operand inside method definition");
            assert.doesNotThrow(function () { eval("class C { *gf() { switch (1) { case yield: break; } } }"); }, "yield without operand is a valid switch case label operand inside class method");

            assert.doesNotThrow(function () { eval("function* gf() { yield 'foo'; }"); }, "yield with operand is a valid expression statement");
            assert.doesNotThrow(function () { eval("function* gf() { yield 'foo' }"); }, "yield with operand is a valid expression statement with implicit semicolon insertion");
            assert.doesNotThrow(function () { eval("function* gf() { var a = yield 'foo'; }"); }, "yield with operand is a valid RHS expression");
            assert.doesNotThrow(function () { eval("function* gf() { foo(yield 'foo'); }"); }, "yield with operand is a valid argument expression");
            assert.doesNotThrow(function () { eval("function* gf() { foo[yield 'foo']; }"); }, "yield with operand is a valid element expression");
            assert.doesNotThrow(function () { eval("function* gf() { yield 'foo', 10; }"); }, "yield with operand is a valid LHS of the comma operator");
            assert.doesNotThrow(function () { eval("function* gf() { switch (1) { case yield 'foo': break; } }"); }, "yield with operand is a valid switch case label operand");
            assert.doesNotThrow(function () { eval("var gfe = function* () { switch (1) { case yield 'foo': break; } }"); }, "yield with operand is a valid switch case label operand inside function expression w/o name");
            assert.doesNotThrow(function () { eval("var gfe = function* rgfe() { switch (1) { case yield 'foo': break; } }"); }, "yield with operand is a valid switch case label operand inside function expression w/ name");
            assert.doesNotThrow(function () { eval("var o = { *gf() { switch (1) { case yield 'foo': break; } } }"); }, "yield with operand is a valid switch case label operand inside method definition");
            assert.doesNotThrow(function () { eval("class C { *gf() { switch (1) { case yield 'foo': break; } } }"); }, "yield with operand is a valid switch case label operand inside class method");

            assert.doesNotThrow(function () { eval("function* gf() { yield* 'foo'; }"); }, "yield* with operand is a valid expression statement");
            assert.doesNotThrow(function () { eval("function* gf() { yield* 'foo' }"); }, "yield* with operand is a valid expression statement with implicit semicolon insertion");
            assert.doesNotThrow(function () { eval("function* gf() { var a = yield* 'foo'; }"); }, "yield* with operand is a valid RHS expression");
            assert.doesNotThrow(function () { eval("function* gf() { foo(yield* 'foo'); }"); }, "yield* with operand is a valid argument expression");
            assert.doesNotThrow(function () { eval("function* gf() { foo[yield* 'foo']; }"); }, "yield* with operand is a valid element expression");
            assert.doesNotThrow(function () { eval("function* gf() { yield* 'foo', 10; }"); }, "yield* with operand is a valid LHS of the comma operator");
            assert.doesNotThrow(function () { eval("function* gf() { switch (1) { case yield* 'foo': break; } }"); }, "yield* with operand is a valid switch case label operand");
            assert.doesNotThrow(function () { eval("var gfe = function* () { switch (1) { case yield* 'foo': break; } }"); }, "yield* with operand is a valid switch case label operand inside function expression w/o name");
            assert.doesNotThrow(function () { eval("var gfe = function* rgfe() { switch (1) { case yield* 'foo': break; } }"); }, "yield* with operand is a valid switch case label operand inside function expression w/ name");
            assert.doesNotThrow(function () { eval("var o = { *gf() { yield* 'foo'; } }"); }, "yield* with an operand as a valid expression statement inside method definition");
            assert.doesNotThrow(function () { eval("class C { *gf() { switch (1) { case yield* 'foo': break; } } }"); }, "yield* with operand is a valid switch case label operand inside class method");
        }
    },
    {
        name: "Invalid syntax forms -- noteworthy forms found in testing",
        body: function () {
            assert.throws(function () { eval("class C { gm*() { } }"); }, SyntaxError, "Star does not work on RHS of method name for declaring a generator method", "Expected '('");
        }
    },
    {
        name: "Invalid syntax forms -- yield expressions cannot appear higher than assignment level precedence",
        body: function () {
            assert.throws(function () { eval("function* gf() { 1 + yield; }"); }, SyntaxError, "yield cannot appear in AdditiveExpression -- e.g. rhs operand of binary +", "Syntax error");
            assert.throws(function () { eval("function* gf() { 1 + yield 2; }"); }, SyntaxError, "yield with operand cannot appear in AdditiveExpression -- e.g. rhs of +", "Syntax error");
            assert.throws(function () { eval("function* gf() { 1 + yield* 'foo'; }"); }, SyntaxError, "yield* with operand cannot appear in AdditiveExpression -- e.g. rhs of +", "Syntax error");

            assert.throws(function () { eval("function* gf() { +yield; }"); }, SyntaxError, "yield cannot appear in UnaryExpression -- e.g. operand of unary +", "Syntax error");
            assert.throws(function () { eval("function* gf() { +yield 2; }"); }, SyntaxError, "yield with operand cannot appear in UnaryExpression -- e.g. operand of unary +", "Syntax error");
            assert.throws(function () { eval("function* gf() { +yield* 'foo'; }"); }, SyntaxError, "yield* with operand cannot appear in UnaryExpression -- e.g. operand of unary +", "Syntax error");

            assert.throws(function () { eval("function* gf() { yield++; }"); }, SyntaxError, "yield cannot appear in PostfixExpression -- e.g. operand of postfix ++", "Syntax error");
        }
    },
    {
        name: "Invalid use of yield in lvalue positions that are runtime errors",
        body: function () {
            assert.throws(function () { function* gf() { (yield) = 10; } var g = gf(); g.next(); g.next(); }, ReferenceError, "yield cannot be the LHS target of an assignment", "Invalid left-hand side in assignment");
            assert.throws(function () { function* gf() { ++(yield); } var g = gf(); g.next(); g.next(); }, ReferenceError, "yield cannot be the target of an increment operator", "Invalid left-hand side in assignment");
            assert.throws(function () { function* gf() { (yield)++; } var g = gf(); g.next(); g.next(); }, ReferenceError, "yield cannot be the target of an increment operator", "Invalid left-hand side in assignment");
        }
    },
    {
        name: "'yield' cannot be used as a BindingIdentifier for declarations in generator bodies",
        body: function () {
            assert.throws(function () { eval("let gfe = function* yield() { }"); }, SyntaxError, "Cannot name generator function expression 'yield' in any context", "The use of a keyword for an identifier is invalid");

            assert.throws(function () { eval("function* gf() { var yield; }"); }, SyntaxError, "Cannot name var variable 'yield' in generator body", "The use of a keyword for an identifier is invalid");
            assert.throws(function () { eval("function* gf() { let yield; }"); }, SyntaxError, "Cannot name let variable 'yield' in generator body", "The use of a keyword for an identifier is invalid");
            assert.throws(function () { eval("function* gf() { const yield = 10; }"); }, SyntaxError, "Cannot name const variable 'yield' in generator body", "The use of a keyword for an identifier is invalid");

            assert.throws(function () { eval("function* gf() { function yield() { } }"); }, SyntaxError, "Cannot name function 'yield' in generator body", "The use of a keyword for an identifier is invalid");
            assert.throws(function () { eval("function* gf() { function* yield() { } }"); }, SyntaxError, "Cannot name generator function 'yield' in generator body", "The use of a keyword for an identifier is invalid");
            assert.throws(function () { eval("function* gf() { var fe = function yield() { } }"); }, SyntaxError, "Cannot name function expression 'yield' in generator body", "The use of a keyword for an identifier is invalid");
            assert.throws(function () { eval("function* gf() { var gfe = function* yield() { } }"); }, SyntaxError, "Cannot name generator function expression 'yield' in generator body", "The use of a keyword for an identifier is invalid");

            assert.throws(function () { eval("function* gf() { class yield { } }"); }, SyntaxError, "Cannot name class 'yield' in generator body", "The use of a keyword for an identifier is invalid");

            // TODO: Is this correct or a spec bug that will be fixed?
            // var yield = 10; function* gf() { var o = { yield }; } gf();
            assert.throws(function () { eval("function* gf() { var o = { yield }; }"); }, SyntaxError, "Cannot name shorthand property 'yield' in generator body", "The use of a keyword for an identifier is invalid");

            // Note, reserved words are allowed for object literal and class PropertyNames, so these cases parse without error.
            assert.doesNotThrow(function () { eval("function* gf() { var o = { yield: 10 } }"); }, "Can name object literal property 'yield' in generator body");
            assert.doesNotThrow(function () { eval("function* gf() { var o = { get yield() { } } }"); }, "Can name accessor method 'yield' in generator body");
            assert.doesNotThrow(function () { eval("function* gf() { var o = { yield() { } } }"); }, "Can name concise method 'yield' in generator body");
            assert.doesNotThrow(function () { eval("function* gf() { var o = { *yield() { } } }"); }, "Can name generator concise method 'yield' in generator body");
            assert.doesNotThrow(function () { eval("function* gf() { class C { yield() { } } }"); }, "Can name method 'yield' in generator body");
            assert.doesNotThrow(function () { eval("function* gf() { class C { *yield() { } } }"); }, "Can name generator method 'yield' in generator body");
        }
    },
    {
        name: "'yield' can be used as a BindingIdentifier for declarations in non-strict, non-generator bodies",
        body: function () {
            assert.doesNotThrow(function () { eval("function f() { var yield; }"); }, "Can name var variable 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { let yield; }"); }, "Can name let variable 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { const yield = 10; }"); }, "Can name const variable 'yield' in non-generator body");

            assert.doesNotThrow(function () { eval("function f() { function yield() { } }"); }, "Can name function 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { function* yield() { } }"); }, "Can name generator function 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { var fe = function yield() { } }"); }, "Can name function expression 'yield' in non-generator body");

            assert.doesNotThrow(function () { eval("function f() { class yield { } }"); }, "Can name class 'yield' in non-generator body");

            assert.doesNotThrow(function () { eval("function f() { var o = { yield: 10 } }"); }, "Can name object literal property 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { var o = { get yield() { } } }"); }, "Can name accessor method 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { var o = { yield() { } } }"); }, "Can name concise method 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { var o = { *yield() { } } }"); }, "Can name generator concise method 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { var yield = 10; var o = { yield }; }"); }, "Can name shorthand property 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { class C { yield() { } } }"); }, "Can name method 'yield' in non-generator body");
            assert.doesNotThrow(function () { eval("function f() { class C { *yield() { } } }"); }, "Can name generator method 'yield' in non-generator body");
        }
    }

    // TODO: add test case for function* gfoo() { (yield) => { /* use yield here */ } }
    // TODO: add test cases for yield appearing as a valid identifier inside generator functions (e.g. above lambda, or e.g. o.yield, etc)
    // TODO: add test case for automatic semicolon insertion after yield keyword (requires checking value of yield operation, i.e. full implementation)
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
