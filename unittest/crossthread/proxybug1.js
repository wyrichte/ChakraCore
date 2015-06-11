function test0() {
  var obj0 = {};
  var func3 = function () {
    return ary.reverse();
  };
  obj0.method0 = func3;
  var ary = Array();
  var sc6 = WScript.LoadScriptFile('DummyFileForCctx.js', 'samethread');
  sc6.obj0 = obj0;
  var sc6_cctx = sc6.Debug.parseFunction('function v0()\n{\n\tthis.v1 = 1;\n\tthis.v2 = 1;\n\tthis.v3 = 1;\n\tthis.v4 = (new obj0.method0()).prop0 ;\n\tthis.v3= 1;\n}\nfunction v5()\n{\n\tv0.prototype = {};\n\tObject.defineProperty(v0.prototype,"v4",{get:function(){return 100},configurable:false });\n;\n;\n\tvar v6 = new v0();\n}\nv5();\nv5();\n;\n  ');
  sc6_cctx();
}
Debug.setAutoProxyName(test0);
test0();
WScript.Echo('PASS');

