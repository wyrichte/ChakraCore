// jshost -DiagnosticsEngine

if (typeof (WScript) != "undefined") {
  WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js", "self");
}

var x = 10;

var tests = {
  test01: {
    name: "no args",
    body: function () {
      assert.areEqual(undefined, diagnosticsScript.debugEval());
    }
  },
  test02: {
    name: "wrong this",
    body: function () {
      assert.areEqual(undefined, diagnosticsScript.debugEval.call(1, "", true));
    }
  },
  test03: {
    name: "missing isNonUserCode",
    body: function () {
      assert.areEqual(undefined, diagnosticsScript.debugEval("x"));
    }
  },
  test04: {
    name: "Wrong isNonUserCode",
    body: function () {
      assert.areEqual(undefined, diagnosticsScript.debugEval("x"), 1);
    }
  },
  test05: {
    name: "isNonUserCode=true",
    body: function () {
      assert.areEqual(x, diagnosticsScript.debugEval("function foo() { return x; }; foo()", true));
    }
  },
  test06: {
    name: "isNonUserCode=false",
    body: function () {
      assert.areEqual(x + 1, diagnosticsScript.debugEval("var y = 1; function foo() { return x + y; }; foo()", false));
    }
  },
};

testRunner.run(tests);
