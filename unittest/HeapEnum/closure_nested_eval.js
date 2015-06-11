function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

var HETest = {};

HETest.outer = function()
{
	var x = 12;
	var y = "test";
	var z = 1.1;

	var inner = function(evalstr)
	{
        Dump(x);
        Dump(y);
        Dump(z);
        var aa = "aa";
        var bb = "bb";
        Dump("evalstr: " + evalstr);
        eval (evalstr);
        var inner_a = function(evalstr_a)
        {
            Dump(aa);
            Dump(eeo);
            Dump("evalstr_a: " + evalstr_a);
            Dump(HETest.bfrominner);
            eval(evalstr_a);
            with (eea)
            {
                Dump(eea);
            }
        }

        var inner_b = function(evalstr_b)
        {
            Dump(bb);
            Dump("evalstr_b: " + evalstr_b);
            eval (evalstr_b);
            HETest.bfrominner = "bfrominner";
            Dump(HETest.bfrominner);
        }

        return[inner_a, inner_b]
	}

	return inner;
}

HETest.g = function(funcs)
{
	funcs[1]('var eeb = "eeb_value"');
	funcs[0]('var eea = "eea_value"');
}

HETest.clo = HETest.outer();
HETest.clo2 = HETest.clo('var eeo = "eeo_value"');
HETest.g(HETest.clo2);

Debug.dumpHeap(HETest, true);