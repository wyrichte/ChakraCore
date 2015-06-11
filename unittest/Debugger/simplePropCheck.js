
function foo(a1,b1)
{
    var c = {};
    c.__proto__ = ["x"];
    c[0] = 20;
    function bar () {}
    WScript.Echo("Pass");                /**:bp:locals()**/
}
WScript.Attach(foo);
