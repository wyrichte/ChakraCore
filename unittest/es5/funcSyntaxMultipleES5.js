
function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write("</br>");
    }
    else
        WScript.Echo(a)
}

try
{
// this should not compile in ES5
eval('function a,b,c(){ write("in function m");}');
}
catch(e)
{
write("function a,b,c(){} -  compile failure in ES5" + e)
}