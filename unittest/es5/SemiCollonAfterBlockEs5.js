

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
eval("if(true){};else{}");
}
catch(e)
{
write("'if(true){};else{}' compile failure in ES5" + e)
}