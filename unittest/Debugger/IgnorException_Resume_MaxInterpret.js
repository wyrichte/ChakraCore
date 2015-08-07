// Ignore exception resume to next statement. 
// These tests are expected to run with: -maxInterpretCount:1 -debuglaunch -targeted

/**exception(resume_ignore):stack();**/

if (typeof (WScript) != "undefined") {
  WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js", "self");
}

var tests = {
  // Note: each test has name (string) and body (function) properties. 
  //       Success is when the body does not throw, failure -- when it throws.

  test01: {
    name: "Ignore exception bailout is shared with implicit calls bailout. Make sure that we bail out to correct byte code offset. WinBlue 246857.",
    body: function () {
      var accumulator = "";
      var x = 1;
      var trigger;
      var newTrigger = { valueOf : function () { accumulator += 'valueOf called.'; return 1; } };
    
      function func1() {
        return trigger;
      }
    
      function func2() 
      {
        var z = (func1(--x) >= 0 ? func1() : 0) & 0xF;
      }
    
      func2();
      trigger = newTrigger;
      func2();

      assert.areEqual("valueOf called.valueOf called.", accumulator);
    }
  },

}; // tests.

testRunner.runTests(tests);
