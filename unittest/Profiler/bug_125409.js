// Validation of bug 125409 (will be executed under forcedeferparse)

var dorun = false;

function foo()
{
    function bar()
    {
        var j = 10;
        j++;
    }
    
    if (dorun)
    {
        var k = new bar();
    }
    
    return 10;
    
}

foo();
dorun = true;
WScript.StartProfiling(foo);
