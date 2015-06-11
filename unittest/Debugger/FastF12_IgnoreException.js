// Set global handler for unhandled exceptions.
/**exception(resume_ignore)**/

// Library/built-in function that throws.
function foo1()
{
  var obj = undefined;
  Object.prototype.hasOwnProperty.call(obj, "foo");
}

// Helper that throws.
function foo2()
{
  var x = undefined;
  delete x.y;
}

// Explicit throw statement.
function foo3()
{
  throw new Error("catch me if you can");
  WScript.Echo("foo3.after throw");
}

// Exception in script function with Intl (script/internal) on stack
function foo4() 
{
  var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");
  WScript.Echo("foo4.after Intl call");
}

// Helper call right after script call
function foo5()
{
  function m() {};
  function foo5_inner(c)
  {
    return m(c).unexistingProp > 0;
  }

  foo5_inner();
}

foo1();
foo2();
foo3();
foo4();
foo5();
WScript.Echo("done");
