function Dump(x, dump)
{
    //dump = true;
    if (dump != undefined)
    {
        WScript.Echo(x);
    }
}

Dump("Scenario: Multiple closures, with variables that are modified in the parent function");

function write(x) { Dump(x + ""); }

var HETest = {};

HETest.f = function ()
{
    var y = "before modification";

    var ret1 = function()
    {
        Dump("1st function");
        Dump(y);
    }

    y = "after modification";

    var ret2 = function()
    {
        Dump("2nd function");
        Dump(y);
    }

    return [ret1,ret2];
}

HETest.anon1 = function() {
    function f() {
        write('In f');
    }
    function g() {
        write('In g');
    }

    var savef = f;
    f(f = g);
    f = savef;
    
    function foo() {
        write(typeof f);
        write(typeof g);
    }
};
HETest.anon1;

HETest.g = function(funcs)
{
    funcs[1]();
    funcs[0]();
}

HETest.clo = HETest.f();
HETest.g(HETest.clo);
HETest.g(HETest.clo);

// Side-effect through a closure without eval.
HETest.anon2 = function(){
    var f2 = function() { a2 = 2; return 2; }
    HETest.outer2 = f2;
    var a2 = 2;
    Dump(a2 + (f2() + a2));
}
HETest.anon2();

// Side-effect through a closure with eval.
HETest.anon3 = function(){
    var f3 = function() { a3 = 3; return 1; }
    HETest.outer3 = f3;
    var a3 = 1;
    Dump(a3 + (f3() + a3));
    eval("");
}
HETest.anon3();

// Side-effect through a closure inside eval.
HETest.anon4 = function(){
    var f4 = function() { a4 = 2; return 1; }
    HETest.outer4 = f4;
    var a4 = 1;
    eval('Dump(a4 + (f4() + a4));');
}
HETest.anon4();

// No side-effect in nested function.
HETest.anon5 = function(){
    var f5 = function() { return 1; }
    HETest.outer5 = f5;
    var a5 = 1;
    Dump(a5 + (f5() + a5));
}
HETest.anon5();

Debug.dumpHeap(HETest, true);