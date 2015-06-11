/**exception(resume_ignore):stack();**/

// Ignore exception inside if, resume next
function foo3()
{
  function foo(b)
  {
    if (b)
    {
      throw "some exception"; // when we ignore this throw, we should go to after "if" but not inside "else"
    }
    else
    {
      WScript.Echo("foo3.else");
    }
    WScript.Echo("foo3.done");
  }
  foo (true);
}

foo3();