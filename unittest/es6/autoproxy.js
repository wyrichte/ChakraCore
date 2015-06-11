
function test(ctor)
{
    var a = new ctor();
    WScript.Echo('first time set proxy');
    Debug.setAutoProxyName('foo');
    var b = new ctor();
    foo.has = function () { WScript.Echo('has trap'); }
    'a' in b;
    Debug.disableAutoProxy();
    var c = new ctor();
    'a' in c;
    WScript.Echo('second time set proxy');
    Debug.setAutoProxyName('foo');
    var d = new ctor();
    'a' in b;
}

var BuiltinType = [Date, Object, Array, Number];
for (i = 0 ; i < BuiltinType.length; i++) {
    test(BuiltinType[i]);
}
