var otherScriptContext = WScript.LoadScriptFile("crossSiteBoundFunctionRemote.js", "samethread");

function fs() { WScript.Echo("local function"); } 
var localBoundFunction = fs.bind(1);
localBoundFunction.foo = "localFoo";
localBoundFunction.bar = "localBar";

WScript.Echo("-- Test Bind");
localBoundFunction();

var remoteBoundFunction = otherScriptContext.getBoundFunction();

WScript.Echo("\n-- Test prop get");
WScript.Echo(remoteBoundFunction.foo);
WScript.Echo(localBoundFunction.foo);
WScript.Echo(remoteBoundFunction.bar);
WScript.Echo(localBoundFunction.bar);

remoteBoundFunction.foo = "remoteFoo2";

WScript.Echo("\n-- Test prop set");
WScript.Echo(remoteBoundFunction.foo);
WScript.Echo(localBoundFunction.foo);
WScript.Echo(remoteBoundFunction.bar);
WScript.Echo(localBoundFunction.bar);

WScript.Echo("\n-- Test remote call");
localBoundFunction();
remoteBoundFunction();
