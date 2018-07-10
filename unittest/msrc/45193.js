WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");

var tests = [
    {
        name: "Await keyword usage in the param scope of async functions",
        body: function() {
            assert.throws(function () { eval("async function af1(a = class b { [await 1]() { } }) { }"); }, SyntaxError, "await cannot be used in the param scope of an async function in a class", "'await' expression not allowed in this context");
            assert.throws(function () { eval("async function af2(a = { [await 1]() { } }) { }"); }, SyntaxError, "await cannot be used in the param scope of an async function in an object literal", "'await' expression not allowed in this context");
            assert.throws(function () { eval("async function af3(a = class b extends class { [await 1]() { } } { }) { }"); }, SyntaxError, "await cannot be used in the param scope of an async function in a class with extends clause", "'await' expression not allowed in this context");
            assert.throws(function () { eval("async function af4({a = class b { [await 1]() { } } }) { }"); }, SyntaxError, "await cannot be used in the destructured param scope of an async function in a class", "'await' expression not allowed in this context");

            async function af5() {
                class c {
                    [await 1]() { return 10; }
                }
                assert.areEqual(10, (new c())[1](), "Computed function name with await in it should return the right value");
            }
            af5();

            async function af6(a = function await() { return 11; }) {
                print(a());
            }
            af6();
        }
    },
    {
        name: "Yield keword usage in the param scope of generator functions",
        body: function () {
            assert.throws(function () { eval("function *gf1(a = class b { [yield 1]() { } }) { }"); }, SyntaxError, "yield cannot be used in the param scope of a generator function in a class", "Syntax error");
            assert.throws(function () { eval("function *gf2(a = { [yield 1]() { } }) { }"); }, SyntaxError, "yield cannot be used in the param scope of a generator function in an object literal", "Syntax error");
            assert.throws(function () { eval("function *gf3(a = class b extends class { [yield 1]() { } } { }) { }"); }, SyntaxError, "yield cannot be used in the param scope of a generator function in a class with extends clause", "Syntax error");
            assert.throws(function () { eval("function *gf4({a = class b { [yield 1]() { } } }) { }"); }, SyntaxError, "yield cannot be used in the destructured param scope of a generator function in a class", "Syntax error");

            function *gf5() {
                class c {
                    [yield 10]() { return 12; }
                }
                return (new c())[11]();
            }
            var g5 = gf5();
            print(g5.next().value);
            print(g5.next(11).value);

            function *gf6(a = function yield() { return 13; }) {
                print(a());
            }
            gf6().next();
        }
    },
    {
        name: "Usage of await keyword in generator functions",
        body: function () {
            function *gf1(a = function await() { return 11; }) {
                print(a());
            }
            gf1().next();

            function *gf2() {
                var a = function await() { return 12; };
                print(a());
            }
            gf2().next();

            function *gf3() {
                function await() {
                    return 13;
                }
                print(await());
            }
            gf3().next();
        }
    },
    {
        name: "Usage of yield keyword in async functions",
        body: function () {
            async function af1(a = function yield() { return 11; }) {
                print(a());
            }
            af1();

            async function af2() {
                var a = function yield() { return 12; };
                print(a());
            }
            af2();

            async function af3() {
                function yield() {
                    return 13;
                }
                print(yield());
            }
            af3();
        }
    }
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });