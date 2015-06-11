var a = 3;
function test()
{
    // Bail out point to make sure the stack walker can get the line number of the throw after bailout
    for (var i = 0; i < a; i++)
    {   
        WScript.Echo(i);
    }
        
    throw 1;
}


test();
