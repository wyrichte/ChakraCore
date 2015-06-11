
// Test to make sure field copy prop can deal with delete
"use strict";
function f() {
    var x = new Object();
    x.y = 1;
    delete x.y;
    if (x.y === undefined)
    {
        WScript.Echo("Pass");
    }
    return x;
}

function g(o)
{
    delete o.y;
}

g(f());
