// Locals inspection validate on the top frame when the bailout happens from the jit code.

function foo(a, b)
{
    var c = 10;
    var m2 = {}; var k = new Date;
    var j = 39.1
    c += 31;

    function bar1(a, b)
    {
        j += 0.2;
        var s = [3,5,7];
        var s2 = function () {}
        if (a > b)
        {
            debugger;
        }
        return s;
    }
    bar1(a,b);
    
    var y = 10;

    if (a > b)
    {
        debugger;
    }
    
    function bar2(a, b)
    {
        var z1 = a|b;
        var z3 = arguments.length
        var z2 = a | 31;
        if (a > b)
        {
            debugger;
        }
        
        return z1;
    }
    bar2(a*2, b*2)
    return [y];
}

foo(3 ,6);
foo(8,2);

WScript.Echo("Pass");
