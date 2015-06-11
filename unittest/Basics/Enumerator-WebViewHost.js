WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js", "self");

var tests = {
  test01: {
    name: "Check that Enumerator is allowed for HostType = Web View",
    body: function () {
      var arr = ["x", "y"];
      var enu = new Enumerator(arr);
      for (enu.moveFirst(); !enu.atEnd(); enu.moveNext()) helpers.writeln(enu.item());
    }
  },
};

testRunner.runTests(tests);
