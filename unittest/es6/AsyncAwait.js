// ES6 Async Await tests -- verifies the API shape and basic functionality

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
    {
        name: "Async and Await keyword as identifier",
        body: function () {
            var async = [2, 3, 4];
            var await = 3;
            var o = { async };
            o.async = 0;

            assert.areEqual(2, async[0], "async[0] === 2");
            assert.areEqual(3, await, "await === 3");
            assert.areEqual(0, o.async, "o.async === 0");
        }
    },
    {
        name: "Async keyword with a lambda expressions",
        body: function () {
            var x = 12;
            var y = 14;
            var lambdaParenNoArg = async () => x < y;
            var lambdaSingleArgNoParen = async x => x;
            var lambdaSingleArg = async (x) => x;
            var lambdaArgs = async (a, b ,c) => a + b + c;

            assert.isTrue(lambdaParenNoArg(), "'async' keyword doesn't work with lambda expression with no arguments");
            assert.areEqual(12, lambdaSingleArgNoParen(12), "'async' keyword doesn't work with lambda expression with a single argument and no paren");
            assert.areEqual(12, lambdaSingleArg(12), "'async' keyword doesn't work with lambda expression with with a single argument");
            assert.areEqual(60, lambdaArgs(10, 20, 30), "'async' keyword doesn't work with lambda expression with with several arguments");
       }
    },
    {
        name: "Async keyword as generator",
        body: function () {
            assert.throws(function () { eval("async function* badFunction() { }"); }, SyntaxError, "'async' keyword is not allowed with a generator in a statement", "Syntax error");
            assert.throws(function () { eval("var badVaribale = async function*() { }"); }, SyntaxError, "'async' keyword is not allowed with a generator in an expression", "Syntax error");
            assert.throws(function () { eval("var o { async *badFunction() { } };"); }, SyntaxError, "'async' keyword is not allowed with a generator in a object literal member", "Expected ';'");
            assert.throws(function () { eval("class C { async *badFunction() { } };"); }, SyntaxError, "'async' keyword is not allowed with a generator in a class member", "Syntax error");
        }
    },
    {
        name: "Async function in a statement",
        body: function () {
            {
                var namedNonAsyncMethod = function async (x, y) { return x + y; }
                var unamedAsyncMethod = async function (x, y) { return x + y; }
                async function async(x, y) { return x - y; } 

                assert.areEqual(30, namedNonAsyncMethod(10, 20), "'async' keyword doesn't work with a non async function called 'async'");
                assert.areEqual(30, unamedAsyncMethod(10, 20), "'async' keyword doesn't work with unamed function");
                assert.areEqual(-10, async(10, 20), "'async' keyword doesn't work with a function called 'async'");
            };
            {
                 async function async() { return 12; }

                 assert.areEqual(12, async(), "a async function called 'async' with no argument parses and produces a normal function");
            };
            {
                function async() { return 12; }

                assert.areEqual(12, async(), "a non-async function called 'async' with no argument doesn't work");
            };
       }
    },
    {
        name: "Async function in an object",
        body: function () {
            var object = {
                async async() { return 12; }
            };

            var object2 = {
                async() { return 12; }
            };

            var object3 = {
                async "a"() { return 12; },
                async 0() { return 12; },
                async 3.14() { return 12; },
                async else() { return 12; },
            };

            assert.areEqual(12, object.async(), "'async' keyword doesn't work within a dynamic object");
            assert.areEqual(12, object2.async(), "In a dynamic object, a non-async function called 'async' is not supported");
            assert.areEqual(12, object3.a(), "In a dynamic object, a async function called '\"a\"' is not supported");
            assert.areEqual(12, object3['0'](), "In a dynamic object, a async function called '0' is not supported");
            assert.areEqual(12, object3['3.14'](), "In a dynamic object, a async function called '3.14' is not supported");
            assert.areEqual(12, object3.else(), "In a dynamic object, a async function called 'else' (a keyword) is not supported");
       }
    },
    {
        name: "Async classes",
        body: function () {
            class MyClass {
                async asyncMethod(a) { return a; }
                async async(a) { return a; }
                static async staticAsyncMethod(a) { return a; }
                async "a"() { return 12; }
                async 0() { return 12; }
                async 3.14() { return 12; }
                async else() { return 12; }
            }

            class MySecondClass {
                async(a) { return a; }
            }

            class MyThirdClass {
                static async(a) { return a; }
            }

            var x = "foo";
            class MyFourthClass {
                async [x](a) { return a; }
            }

            var myClassInstance = new MyClass();
            var mySecondClassInstance = new MySecondClass();
            var myFourthClassInstance = new MyFourthClass();

            assert.areEqual(10, myClassInstance.asyncMethod(10), "'async' keyword doesn't work with a function declaration in a class");
            assert.areEqual(10, myClassInstance.async(10), "'async' keyword doesn't work with a function declaration called 'async' in a class");
            assert.areEqual(12, myClassInstance.a(), "In a class, a async function called '\"a\"' is not supported");
            assert.areEqual(12, myClassInstance['0'](), "In a class, a async function called '0' is not supported");
            assert.areEqual(12, myClassInstance['3.14'](), "In a class, a async function called '3.14' is not supported");
            assert.areEqual(12, myClassInstance.else(), "In a class, a async function called 'else' (a keyword) is not supported");
            assert.areEqual(10, MyClass.staticAsyncMethod(10), "'async' keyword doesn't work with a static function declaration in a class");
            assert.areEqual(10, mySecondClassInstance.async(10), "A 'async' function declaration in a class is not supported");
            assert.areEqual(10, MyThirdClass.async(10), "A 'async' static function declaration in a class is not supported");
            assert.areEqual(10, myFourthClassInstance.foo(10), "'async' keyword doesn't work with an iterator function declaration in a class is not supported");

            assert.throws(function () { eval("class A { async constructor() {} }"); }, SyntaxError, "'async' keyword is not allowed with a constructor", "Syntax error");
            assert.throws(function () { eval("class A { async get foo() {} }"); }, SyntaxError, "'async' keyword is not allowed with a getter", "Syntax error");
            assert.throws(function () { eval("class A { async set foo() {} }"); }, SyntaxError, "'async' keyword is not allowed with a setter", "Syntax error");
            assert.throws(function () { eval("class A { async static staticAsyncMethod() {} }"); }, SyntaxError, "'async' keyword is not allowed before a static keyword in a function declaration", "Expected '('");
            assert.throws(function () { eval("class A { static async prototype() {} }"); }, SyntaxError, "static async method cannot be named 'prototype'", "Syntax error");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
