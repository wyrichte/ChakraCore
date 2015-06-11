/**exception(resume_ignore):stack();**/

function dummy() {}

function oldF(doThrow)
{
  if (doThrow)
  {
    throw "catch me!";
    WScript.Echo("oldF.after throw");
  }
}
oldF(false);

// Make sure that in case when we have a function created before attach, it still gets the DebugProfileThunk
// and installs the exception wrapper.
function test1()
{
  function newF()
  {
    oldF(true);
  }
  newF(true);
}

// External function
function test2()
{
  function test()
  {
    SCA.serialize(dummy);
    WScript.Echo("test2.test.after throw");
  }
  test();
}

function tests()
{
  test1();
  test2();
}

WScript.Attach(tests);
