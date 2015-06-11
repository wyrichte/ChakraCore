// Verifying setnext after exception break.

/**exception(resume_break):setnext('A')**/

function foo() {
  var x1 = a;
  for(var x in {}) {}
  WScript.Echo("Pass"); /**loc(A):stack()**/
}
foo();
WScript.Echo("Pass");
