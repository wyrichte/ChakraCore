// JavaScript source code
if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
    this.WScript.LoadScriptFile("observerProxy.js");
    var csGlobal = this.WScript.LoadScriptFile("observerProxy.js", "samethread");
    var cpGlobal = this.WScript.LoadScriptFile("observerProxy.js", "differetthread");
}
else {
    throw new Error('failed');
}

var tests = [
    {
        name: "basic isArray",
        body: function () {
            var arrayProxy = new Proxy([], {});
            assert.isTrue(Array.isArray(arrayProxy));
            var objProxy = new Proxy({}, {});
            assert.isFalse(Array.isArray(objProxy));
        }
    },
    {
        name: "isArray toString",
        body: function () {
            var arrayProxy = new Proxy([1,2,3], {});
            assert.areEqual(arrayProxy.toString(), "1,2,3");
            var objProxy = new Proxy({'a':1, 'b':2}, {});
            assert.areEqual(objProxy.toString(), "[object Object]");
        }
    },
    {
        name: "array builtin",
        body: function () {
            var arrayProxy = new Proxy([1, 2, 3], {});
            arrayProxy.push(4);
            assert.areEqual(arrayProxy.toString(), "1,2,3,4");
            assert.areEqual(arrayProxy.pop().toString(), "4");
            assert.areEqual(arrayProxy.slice().toString(), "1,2,3");
            var newResult = arrayProxy.concat([8, 9, 10]);
            assert.isTrue(Array.isArray(newResult));
            assert.areEqual(newResult, [1, 2, 3, 8, 9, 10]);
            var res1 = arrayProxy.splice(0, 2);
            assert.areEqual(res1, [1,2]);
        }
    },
    {
        name: "array JSON",
        body: function () {
            var arrayProxy = new Proxy([1, 2, 3], {});
            var result = JSON.stringify(arrayProxy);
            assert.areEqual(result, "[1,2,3]");
            var obj = JSON.parse(result);
            assert.areEqual(obj.toString(), "1,2,3");
        }
    },
    {
        name: "object JSON",
        body: function () {
            var objProxy = new Proxy({'a':1, 'b':2}, {});
            var result = JSON.stringify(objProxy);
            assert.areEqual(result, '{"a":1,"b":2}');
            var obj = JSON.parse(result);
            assert.areEqual(obj, {'a':1, 'b':2});
        }
    },

];

initialize();
testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
