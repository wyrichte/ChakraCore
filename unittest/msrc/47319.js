var obj;
var ary = new Int32Array(1048576);
function test()
{
    obj &= ('' in ary);
}

test();
test();
test();
WScript.Echo("PASS");
