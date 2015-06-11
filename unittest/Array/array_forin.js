function DumpObject(o)
{
    for (var i in o)
    {
    	WScript.Echo(i + " = " + o[i]);
    }
}

var a = new Array(0xFFFFFFFF);
WScript.Echo(a.length);
a[0xFFFFFFFE] = 1;

DumpObject(a);


function Blah()
{
}


Blah.prototype[3] = 101;

var o = new Blah();
o[3] = 3;
o[4] = 4;

DumpObject(o);

