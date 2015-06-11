if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  // jc/jshost
  this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

function foo() {
  function bar() {
    [a] = this;
  }
  let a;
  bar();
}
assert.throws(function () { foo(); }, ReferenceError, "Invalid assignment to array throws runtime reference error when destructuring is disabled", "Invalid left-hand side in assignment");

WScript.Echo("PASS");