var c = 1;
function foo(hint) {
   return c;
}
var a = {};
a[Symbol.toPrimitive] = foo;

function foobar()
{
    var b = 0;
    for(i = 0; i < 1000; i++)
    {
        b +=a;
    }
}
foobar();
c = {valueOf() { return 0}}
foobar();
WScript.Echo("PASS");