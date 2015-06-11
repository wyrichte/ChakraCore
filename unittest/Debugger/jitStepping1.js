// Test: make sure that step out works when stepping out intepreter -> jitted func
// WinBlue 336936.
function stepOutFromInterpretedToJitFunc()
{
  //baz is not JITted. The way we force it to be interpreted is by using try-catch.
  function baz() 
  {   
      try 
      {
        WScript.Echo("baz: isInJit =", Debug.isInJit());/**bp:resume('step_out');stack();**/
      } catch (ex) {
      }
  }

  // this one is JITted
  function bar() 
  {
      WScript.Echo("bar: isInJit =", Debug.isInJit());
      // Use quite a few local vars to make sure this interpreter stack frame is bigger than one for baz.
      var a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,c0,c1,c2,c3,c4,c5,c6,c7,c8,c9;
      baz();
  }

  bar();
}

// Test: step out of a function called implicitly.
// WinBlue 596731
function stepOutOfValueOf()
{
  var litObj0 = {};
  litObj0.prop0 = {
    valueOf : function () {
      return 1; /**bp:resume('step_out');stack();**/
    }
  };

  var func2 = function (o) {
    return o.prop0 + 1;
  }

  func2(litObj0);
}

stepOutFromInterpretedToJitFunc();
stepOutOfValueOf();

WScript.Echo("done");
