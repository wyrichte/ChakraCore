
var a = new Object();

function replacer(k, v)
{
    return v;
}

for (var i = 0; i < 1290; i++)
{
    a[i + 10] = 0;
}

WScript.Echo(JSON.stringify(a, replacer).substring(0,20));
