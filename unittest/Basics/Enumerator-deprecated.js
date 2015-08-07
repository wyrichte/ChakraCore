WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js", "self");

var tests = {
  test01: {
    name: "Check that Enumerator is deprecated for HostType = Application",
    body: function () {
      assert.throws(
        function() { 
          var arr = ["x", "y"];
          var enu = new Enumerator(arr);
          // for (enu.moveFirst(); !enu.atEnd(); enu.moveNext()) helpers.writeln(enu.item());
        }, ReferenceError);
    }
  },
};

testRunner.runTests(tests);
