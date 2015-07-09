function f() 
{
    var v0 = 1;
    var o = { [v0++] : v0 };
    if (o[1] !== 2)
    {
        WScript.Echo('fail1: o[1] === ', o[1]);
    }
}
f();

function g() 
{
    var v0 = 1;
    var o = { [v0++] : v0 };
    function h() { return v0; }
    if (o[1] !== 2)
    {
        WScript.Echo('fail2: o[1] ===', o[1]);
    }
}
g();

WScript.Echo('pass');    