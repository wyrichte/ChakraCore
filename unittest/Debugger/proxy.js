if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
};

var tests = [
    {
        name: "basic Proxy debugger test",
        body: function () {
            var a = new Proxy({}, {});
            /**bp:evaluate('a', 2);**/
        }
    },
    {
        name: "proxy debugger with handler",
        body: function () {
            var handler = { get: function (obj, name, receiver) { return Reflect.get(obj, name, receiver)} };
            var target = {a:1, b:2, c:3};
            target[1] = 2; target[2] = 3;
            var test2 = new Proxy(target, handler);
            /**bp:evaluate('test2', 2);**/
            /**bp:locals(2);**/
        }
    },
    {
        name: "basic Proxy debugger test",
        body: function () {
            var handler = {
                get: function (obj, name, receiver) {
                    /**bp:stack()**/
                    /**bp:locals(2);**/
                    return Reflect.get(obj, name, receiver);
                }
            };
            var target = {A:1, B:2, C:3};
            var test3 = new Proxy(target, handler);
            /**bp:evaluate('test3', 2);**/
            test3.foo;
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });



