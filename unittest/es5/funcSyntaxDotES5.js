
function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write("</br>");
    }
    else
        WScript.Echo(a)
}

var a = { b: { c: {} } };

try
{
// this should not compile in ES5
eval('function a.b.c(){ write("in function m");}');
}
catch(e)
{
write("function a.b.c(){} -  compile failure in ES5 " + e)
}

try
{
// this syntax is now supported even in ES5, but without a host
// the a.b.c::x will fail to bind to the event
eval('function a.b.c::x(){ write("in function m");}');
}
catch(e)
{
write("function a.b.c::x(){} -  compile failure in ES5 " + e)
}