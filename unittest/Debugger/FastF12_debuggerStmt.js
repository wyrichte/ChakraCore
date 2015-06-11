function foo()
{
  var x = 1;
  debugger;
  WScript.Echo("foo done"); // Inject helper block right after explicit bailout.
  return x;
}

foo();
