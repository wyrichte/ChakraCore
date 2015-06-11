function f() { WScript.Echo("remote function"); }

function getBoundFunction()
{
    var x = f.bind(1);
    x.foo = 'remoteFoo';
    x.bar = 'remoteBar';
    return x;
}