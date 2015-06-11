
var obj =
{
    get: function() {},
    set: function(v){}
};

var obj2 =
{
    get getter() {},
    set setter(v){}
};

var obj3 =
{
    get: function foo() {},
    set:  function bar(v){}
};

function f(){};

var a = f();

var b = function() {};

WScript.Echo("PASS");