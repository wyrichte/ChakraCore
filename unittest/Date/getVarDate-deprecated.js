WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js", "self");

var tests = {
  test01: {
    name: "Check that getVarDate is deprecated for HostType = Application",
    body: function () {
      assert.throws(
        function() { 
          var d = new Date(2011, 3, 24, 12, 49, 15, 478);
          d.getVarDate();
        }, TypeError);
    }
  },
};

testRunner.runTests(tests);
