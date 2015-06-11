/**exception(resume_ignore):stack();**/

// External function
function testExceptionFromInsideStartCall()
{
  function test()
  {
    // Here LdRootFld for non-existing var will be inside StartCall as part of the ArgOut.
    SCA.serialize(SomeNonExistingVarToGenerateError);
    WScript.Echo("PASS"); // If we don't assert, we are fine.
  }
  test();
}

WScript.Attach(testExceptionFromInsideStartCall);
