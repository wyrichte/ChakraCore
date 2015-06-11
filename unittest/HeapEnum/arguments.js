function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

function DumpTestResult(s, fcn)
{
    Dump(s);
    var arr = (HETest[s] = fcn());
    for (i=0; i < arr.length; i++)
    {
        Dump(i + ": " + arr[i]);
    }
    Dump("");
}

var HETest = {};

HETest.fcn_NoParams = function()
{
    return arguments;
}

DumpTestResult("tst_NoParams_with_0_arguments", function() { return HETest.fcn_NoParams(); } );
DumpTestResult("tst_NoParams_with_1_argument", function() { return HETest.fcn_NoParams("t_1_1_arg1"); } );

HETest.fcn_TwoParams = function(p1, p2)
{
    return arguments;
}

DumpTestResult("tst_TwoParams_with_0_arguments", function() { return HETest.fcn_TwoParams(); } );
DumpTestResult("tst_TwoParams_with_1_argument", function() { return HETest.fcn_TwoParams("t_2_1_arg1"); } );
DumpTestResult("tst_TwoParams_with_2_argument", function() { return HETest.fcn_TwoParams("t_2_2_arg1", "t_2_2_arg2"); } );
DumpTestResult("tst_TwoParams_with_3_argument", function() { return HETest.fcn_TwoParams("t_2_3_arg1", "t_2_3_arg2", "t_2_3_arg3"); } );

HETest.fcn_FiveParams = function(p1, p2, p3, p4, p5)
{
    var arr = arguments;
    delete arguments[7];
    return arguments;
}

DumpTestResult("tst_FiveParams_with_10_arguments_delete_8th", function() { return HETest.fcn_FiveParams(11, 22, 33, 44, 55, 66, 77, 88, 99, 10); } );

Debug.dumpHeap(HETest, true);
