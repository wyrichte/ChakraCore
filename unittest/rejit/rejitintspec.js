function test1(n) {
  var k = 0x3fffffff;
  k <<= 1;
  if(n === 1) {
    --k;
  }
  var a = k;
  while(n-- !== 0) {
    ++a;
  }
  return a;
}

function test2(n) {
  if (n.valueOf & 1) {    
    n++;
  }
  
  return n;
}

WScript.Echo("--------------- int type spec ------------------");

WScript.Echo(test1(1));
WScript.Echo(test1(1));
WScript.Echo(test1(1));
WScript.Echo("    Initial JIT");
WScript.Echo(test1(2));

WScript.Echo("    Schedule instrumented reJIT");
WScript.Echo(test1(2));
WScript.Echo("    Instrumented reJIT");
WScript.Echo(test1(2));

WScript.Echo(test1(2));
WScript.Echo(test1(2));
WScript.Echo(test1(2));
WScript.Echo(test1(2));
WScript.Echo("    Schedule reJIT here, no int type spec");	
WScript.Echo(test1(2));
WScript.Echo("    reJIT, no int type spec");
WScript.Echo(test1(2));


WScript.Echo("------------ lossy int type spec ---------------");

WScript.Echo(test2(1));
WScript.Echo(test2(1));
WScript.Echo(test2(1));
WScript.Echo("    Initial JIT");
WScript.Echo(test2(1));

WScript.Echo("    Schedule instrumented reJIT");
WScript.Echo(test2(1));

WScript.Echo("    Instrumented reJIT");
WScript.Echo(test2(1));

WScript.Echo(test2(1));
WScript.Echo(test2(1));
WScript.Echo(test2(1));
WScript.Echo(test2(1));
WScript.Echo("    Schedule reJIT here, no lossy int type spec");	
WScript.Echo(test2(1));
WScript.Echo("    reJIT, no lossy int type spec");
WScript.Echo(test2(1));
WScript.Echo(test2(1));

