// Set Next Statement explicitly, no exception/no ignore exception

// For-In, set next to beginning of For-In from location before loop.
function foo1()
{
  WScript.Echo("foo1");
  var x = 0; /**bp:setnext('foo1_forin')**/
  WScript.Echo("We should not get here!");

  for (var i in [0])/**loc(foo1_forin):stack()**/
  {
    WScript.Echo(i);
  }
}

// For-In, set next to beginning of For-In from inside the loop.
function foo2()
{
  WScript.Echo("foo2");
  var arr = [1, 2, 3];
  for (var i in arr)/**loc(foo2_forin):stack()**/
  {
    WScript.Echo(arr[i]);
    var x = 0; /**bp:setnext('foo2_forin')**/
  }
}

// For-In, set next to beginning of For-In from location after the loop.
function foo3()
{
  WScript.Echo("foo3");
  var arr = [4, 5];
  for (var i in arr)/**loc(foo3_forin):stack()**/
  {
    WScript.Echo(arr[i]);
  }
  WScript.Echo("foo3.after loop");
  var x = 0; /**bp:setnext('foo3_forin')**/
  // SetNext should not be allowed, so no infinite loop caused by prev BP.
}

// If-else, set next to else stmt.
function foo4()
{
  WScript.Echo("foo4");
  function foo(b)
  {
    if (b)
    {
      var x = 0; /**bp:setnext('foo4_else')**/
    }
    else
    {
      WScript.Echo("foo4.else"); /**loc(foo4_else):stack()**/
    }
  }
  foo(true);
}

function foo5()
{
  WScript.Echo("foo5"); /**bp:setnext('foo5_try')**/   // Not allowed
  var x = ""; /**bp:setnext('foo5_finally')**/         // Not allowed
  try
  {
    x += "in try."; /**loc(foo5_try):stack()**/
  }
  finally
  {
    x += "in finally."; /**loc(foo5_finally):stack()**/ 

    var y = 0; /**bp:setnext('foo5_nested_catch')**/   // Not allowed
    try
    {
      var y = 0; /**bp:setnext('foo5_nested_catch')**/ // Not allowed
      var z = 0; /**bp:setnext('foo5_finally_done')**/ // Not allowed
      x += "in nested try."; 
    }
    catch (ex)
    {
      x += "in nested catch."; /**loc(foo5_nested_catch):stack()**/
    }

    var z = 0; /**bp:setnext('foo5_finally_done')**/ // Allowed
    try
    {
      x += "in 2nd nested try."; // This should not be printed
    }
    catch (ex)
    {
    }

    x += "finally done."; /**loc(foo5_finally_done):stack()**/
  }
  WScript.Echo(x);
}

foo1();
foo2();
foo3();
foo4();
foo5();
WScript.Echo("done");
