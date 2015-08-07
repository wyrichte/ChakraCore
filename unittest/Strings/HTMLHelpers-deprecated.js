WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js", "self");

var tests = {
  test01: {
    name: "Check that HTMLWrappers on string.prototype are deprecated for HostType = Application",
    body: function () {
      var wrappers = ["anchor", "big", "blink", "bold", "fixed", "fontcolor",  
                      "fontsize", "italics", "small", "strike", "sub", "sup"];
      for (var i in wrappers) {
        helpers.writeln("trying: ", wrappers[i], "...");
        assert.throws(function() { "foo"[wrappers[i]](); }, TypeError);
      }
    }
  },
};

testRunner.runTests(tests);
