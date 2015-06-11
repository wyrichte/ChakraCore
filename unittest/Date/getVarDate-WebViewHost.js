WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js", "self");

var tests = {
  test01: {
    name: "Check that getVarDate is for allowed for HostType = Web View",
    body: function () {
      var d = new Date(2011, 3, 24, 12, 49, 15, 478);
      d.getVarDate();
    }
  },
};

testRunner.runTests(tests);
