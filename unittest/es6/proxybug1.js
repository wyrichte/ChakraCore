function test0() {
  WScript.Echo(typeof Object.create(null) == 'object');
}
test0();
Debug.setAutoProxyName(test0);
test0();
Debug.disableAutoProxy();
WScript.Echo(typeof new Date() == 'object');
function test1() {
   WScript.Echo(typeof new Function("var a = 1;") == 'function');
}
test1();
Debug.setAutoProxyName(test1);
test1();
