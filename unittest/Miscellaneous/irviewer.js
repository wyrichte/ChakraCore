function foo() {
  return 1;
}

function blah(a,b) {
  return (a+b)*2+42;
}


// parseIR
WScript.Echo("--- parseIR ---");
var x = parseIR("function bar() {return 42;}");  // <<< CALL parseIR


// functionList
WScript.Echo("--- functionList ---");
var fnlist = functionList();  // <<< CALL functionList


// rejitFunction
WScript.Echo("--- rejitFunction ---");

var fn, fnSrcInfo, fnId;
fn = fnlist[0];
fnSrcInfo = fn.utf8SrcInfoPtr;
fnId = fn.funcId;

rejitFunction(fnSrcInfo, fnId);  // <<< CALL rejitFunction
