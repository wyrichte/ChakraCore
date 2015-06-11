// Ignore exception resume to next statement.

/**exception(resume_ignore):stack();**/

// Ignore exception before for-in enumerator, resume next. See WinBlue 231880.
// ForIn's child Emit doesn't create new statement.
function foo1()
{
  cause.exception;
  for (var i in [1])
  {
    WScript.Echo(i);
  }
  WScript.Echo("foo1.done");
}

// Ignore exception before for-in enumerator, resume next. See WinBlue 231880.
// ForIn's child Emit creates a new statement.
function foo2()
{
  cause.exception;
  for (var i in new Object({"1": 1}))
  {
    WScript.Echo(i);
  }
  WScript.Echo("foo2.done");
}

// Ignore exception inside if with else block immediatly following the throw, resume next
function foo3()
{
  function foo(b)
  {
    if (b)
    {
      throw "some exception";
    }
    else
    {
      WScript.Echo("foo3.else");
    }
    WScript.Echo("foo3.done");
  }
  foo (true);
}

// Ignore exception thrown in delay-manner (via ThrownExceptionObject::DefaultEntryPoint)
function foo4() 
{
  function foo()
  {
    (new Object).unexistingMethod();
    return; 
  }
  foo();
  WScript.Echo("foo4.done");
}

// Ignore exception thrown from a helper in place where there are tmp local vars but no non-temp vars.
// WinBlue 218483.
function foo5() 
{
  function foo()
  {
    (new UnexistingObject).getTime();
    return; 
  }
  foo();
  WScript.Echo("foo5.done");
}

// Ignore exception and catch in inner script funciton on top on helper call on stack. 
function foo6()
{
  function foo()
  {
    throw new Error("boo");
    WScript.Echo("resume in inner func, good");
  }
  foo.call();
  WScript.Echo("foo6.done");
}

foo1();
foo2();
foo3();
foo4();
foo5();
foo6();
WScript.Echo("done");
