/*
 * WinOOB: 1139867
 * WinOOB: 1141202
 * WinOOB: 1131212
 * WinOOB: 1142531
 * Win8:   600064
 */

function f(arg)
{
    // eval forces args to be loaded from slots.
    eval();

    // now emit a CallIPut arg
    arg() = 10;
}


// Other scenarios for this case of CallIPut


function f0(arg)
{
    arg() = 10;
}

function f1(arg)
{
    arguments[0];

    // now emit a CallIPut arg
    arg() = 10;
}

function f2(arg)
{
    // now emit a CallIPut arg
    arg() = 10;
    
    function g() {
                eval("");
    }
}

try { eval("function x() { ++f(g()) }"); } catch (ex) { }
try { eval("(function x(foo){ return ++(foo.watch(new Function('','')))})"); } catch (ex) { }

function f3() {
    eval("") = function () { };
}

var x;
function f4() {
    var foo = x;
    with ({})
        foo(x) = 3;
}

// -----------------------------------------------
// 1131212
function LTNS() {
    function foo() {
    };

    function bar() {
        foo(0) = 1;
    };
};
// -----------------------------------------------


// -----------------------------------------------
// 1142531
var o = {};
var s = '';
try { eval("with(o) eval(s)--;"); } catch (e) {}

// -----------------------------------------------

// -----------------------------------------------
// Win8 600064

function test0() {
    eval("");
    try {
        nonexistentFunc() = 0;
    } catch(e) {
        WScript.Echo(e.name + ": " + e.message);
    }
}
test0();

// -----------------------------------------------

// -----------------------------------------------
// WinBlue 34087

try { 
    (x34087)(this) = x34087; 
} catch(x34087) { 
    var x34087 = 0;       // Declares a global var but assigns to the catch variable.
    WScript.Echo(x34087); // Prints the catch variable.
} 
WScript.Echo(x34087);     // Prints the (unassigned) global var.

WScript.Echo("Done");
