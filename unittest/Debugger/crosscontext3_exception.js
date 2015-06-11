// Cross context validation in the exception scenario

/**exception(resume_ignore):stack();locals()**/


var x = WScript.LoadScriptFile("crosscontext3_sub.js", "samethread");
function test()
{
    var a = "in test";
    x.testFunction(foo); 
    
    // an exception
    info.info1();
    return a;
}

function foo()
{
    var str = "in main file";
    str; 
}

test();

WScript.Echo("Pass");
