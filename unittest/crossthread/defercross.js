var child = WScript.LoadScriptFile("defercrosschild.js", "samethread");
var childFoo = child.Foo;
var childBar = child.Bar;
WScript.Shutdown(child);

function timeOutFunc() {
  succeeded = false;
  try {
  var c = childBar();
  } catch(e)
  {
    succeeded = true;
    WScript.Echo(e);
  }
  if (succeeded) {
    try {
      var b = new childFoo();
    } catch(e)
    {
      succeeded = true;
      WScript.Echo(e);
    }
  }
  if (!succeeded) {
    WScript.Echo("FAILED");
  }
}

WScript.SetTimeout(timeOutFunc, 500);