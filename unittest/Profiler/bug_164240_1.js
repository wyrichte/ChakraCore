//Validation of bug 164240, where the duplication of function compiled event sent, due to two object sharing the same function body.
// This reproed with forcedeferparse mode only.

function Repro()
{
  function createFunction () {
    function fun() {
       WScript.Echo("blah");
    }

    return fun;
  }

  var func1 = createFunction();
  var func2 = createFunction();

  func1();
  func2();
}

WScript.StartProfiling(Repro);
