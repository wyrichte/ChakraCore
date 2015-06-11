function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

var HETest = {};

HETest.outer = function ()
{
	var x = 12;
	var y = "test";
	var z = 1.1;

	var inner = function()
	{
        Dump(x);
        Dump(y);
        Dump(z);
        var aa = "aa";
        var bb = "bb";
        var inner_a = function()
        {
            Dump(aa);
        }

        var inner_b = function()
        {
            Dump(bb);
        }

        return[inner_a, inner_b]
	}

	return inner;
}

HETest.g = function (funcs)
{
	funcs[1]();
	funcs[0]();
}

HETest.clo = HETest.outer();
HETest.clo2 = HETest.clo();
HETest.g(HETest.clo2);

Debug.dumpHeap(HETest, true);