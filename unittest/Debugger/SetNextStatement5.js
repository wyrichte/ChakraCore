// BLUE bug 248443

// For-In: bypass initialization of loop var moving forward.
function forward()
{
  WScript.Echo("forward");
  // Set next should not be allowed to bypass ForIn initialization
  var arr = [4, 5]; /**bp:setnext('forward_inner')**/
  return;
  for (var i in arr)
  {
    WScript.Echo(arr[i]); /**loc(forward_inner):resume('step_over')**/
  }
  WScript.Echo("forward fail after loop");
}

//For-In: bypass initialization of loop var moving backward.
function backward()
{
  WScript.Echo("backward");
  // Set next here is OK
  var arr = [4, 5]; /**bp:setnext('backward_after');resume('step_over');**/
  
  for (var i in arr) { /** bp:resume('step_over');**/
    WScript.Echo(arr[i]); /**loc(backward_inner):resume('step_over');**/
  }
  WScript.Echo("backward fail");
  var x = 5; /**loc(backward_after):setnext('backward_inner');**/
  // Set next should not be allowed to bypass ForIn initialization
}

forward();
backward();