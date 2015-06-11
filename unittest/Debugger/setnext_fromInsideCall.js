// -targeted -debuglaunch
function foo(){
  function func1() { /**bp:resume('step_out');setnext('loc0')**/ }
  function func2() {}
  x = 1;
  x = Math.log(func1());
  /**bp(loc0)**/
  x = Math.log(func2()); 
};

foo();
WScript.Echo("PASS");
