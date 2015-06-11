function test0(){
  var obj1 = {};
  var sc4Code = "with (obj0) {\n\
}\n\
;\n\
  ";
  var sc4 = WScript.LoadScriptFile('DummyFileForCctx.js', 'samethread');
  sc4.obj0 = obj1;
  var sc4_cctx = sc4.Debug.parseFunction(sc4Code);
  sc4_cctx();
};
test0();
WScript.Echo("PASS");
