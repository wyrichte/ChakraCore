// Detach while step is in progress, then attach again.

function test0() {
  var g = 1;
  /**bp:resume('step_out');resume('step_out');stack();**/
};

function DebuggerAttachDetachFunc() {
  test0();
}

WScript.Attach(DebuggerAttachDetachFunc);
WScript.Detach(DebuggerAttachDetachFunc);
WScript.Attach(DebuggerAttachDetachFunc);

WScript.Echo("PASS");
