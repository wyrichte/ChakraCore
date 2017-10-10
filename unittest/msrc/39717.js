WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");

var tests = [
  {
    name: "Use of new.target in coroutines called through Proxy",
    body: function () {
        var gf1 = function*(){
            assert.areEqual(undefined, new.target, "Generator is not called as a constructor");
        }
        var p0 = new Proxy(gf1, {});
        var g1 = p0.call({} , {});
        g1.next();

        var af = async function(){
            assert.areEqual(undefined, new.target, "Async function is not called as a constructor");
        };
        var p1 = new Proxy(af, {});
        p1.call({} , {});

        var gf2 = function*(){
            yield 1;
            assert.areEqual(undefined, new.target, "New.target is used after a yield");
        }
        var p2 = new Proxy(gf2, {});
        var g2 = p2.call({} , {});
        g2.next();
        g2.next();
    }
  },
  {
    name: "Use of new.target in regular functions that are called through Proxy",
    body: function () {
        function foo() {
            assert.areEqual(undefined, new.target, "Function is not called as a constructor");
        }
        var p = new Proxy(foo, {});
        p.call({});
    }
  },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });