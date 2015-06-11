/**exception(resume_ignore,firstchance):stack();**/

function literal() {
  0();
};

function useArgument(f) {
  f();
}

function spread() {
  1(...[1],1, ...[1]);
}

function test(f, arg)
{
  try {
    f(arg);
  } catch (ex) {
    WScript.Echo(ex);
  }
}

test(literal);
test(useArgument, 1);
test(spread);

if (Debug.debuggerEnabled)
{
  WScript.Echo("PASS"); // For debugger we don't output the exception and have no checked-in baseline.
}
// -debuglaunch -targeted -forcenative
