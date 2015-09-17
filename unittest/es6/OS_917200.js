if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in
  // jc/jshost
  this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
}

function foo() {
  function bar() {
    eval('[a] = this;');
  }
  let a;
  bar();
}
assert.throws(function () { foo(); }, ReferenceError, "Invalid assignment to array throws runtime reference error when destructuring is disabled", "Invalid left-hand side in assignment");

WScript.Echo("PASS");