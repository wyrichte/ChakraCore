// Ignore exception, set next statement expicitly, if-else statement.

/**exception(resume_ignore):setnext('foo1_else')**/

// Set next explicitly to else stmt
function foo1()
{
  function foo(b)
  {
    if (b)
    {
      throw("some exception"); /**bp:setnext('foo2_else')**/
      WScirpt.Echo("We should not get here!");
    }
    else
    {
      WScript.Echo("foo2.else"); /**loc(foo2_else):stack()**/
    }
  }
  foo(true);
}

foo1();
WScript.Echo("done");
