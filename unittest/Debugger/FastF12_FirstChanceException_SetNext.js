function foo()
{
/**exception(firstchance,resume_break):setnext('loc1');**/

  // Scenario to repro issue from WinBlue 569775:
  // Create a few statements to make sure throw has fairly high bytecode offset
  // (which is incorrectly recorded into on 1st chance exception/set next due to WinBlue 569775),
  // so that when we return to global and check for ignore exception bailout after call, we continue
  // to that big offset which doesn't exist in global (it's pass over the end.
  ["a", "b"].join();
  ["c", "d"].join();
  ["e", "f"].join();

  // Generate first-chance exception (we catch it in the caller so that this is not ignore exception scenario).
  throw 0;

  WScript.Echo("Should get here");  /**loc(loc1)**/ // Both old and new F12 allows setting setNext in case of exception
}

// Note: this runs in interpreted mode as we are under debugger and it has try stmt.
function bar()
{
  try
  {
    foo();
  }
  catch (ex)
  {
    WScript.Echo("Should not get here");
  }
}

bar();
WScript.Echo("done");
