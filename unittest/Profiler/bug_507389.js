// Validation of function compiled event sent. 

var globCreatFunction = undefined;
function Foo()
{
    globCreatFunction = function createFunction () {
    function fun() {
       WScript.Echo("blah");
    }

    return fun;
  }
  
  globCreatFunction().prop = "str";
}

Foo();
function Repro()
{
  var func1 = globCreatFunction();
  func1();
}

WScript.StartProfiling(Repro);

