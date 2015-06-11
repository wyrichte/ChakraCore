function findObjectInHeap(obj)
{
    for (var i=0; i < HD.length; i++)
    {
        if (HD[i].object == obj) 
        {
            return HD[i];
        }
    }
}

function dumpSnapshot()
{
    for (var i = 0; i < HD.length; i++)
    {
        dumpObject(HD[i]);
    }
}

var testFailed = false;

function setTestFailed()
{
    testFailed = true;
}

function testExpectation(eval_expr)
{
    if (! eval(eval_expr))
    {
        setTestFailed();
        WScript.Echo("***Expection failed: " + eval_expr);
    }
}

function displayResult()
{
    if (testFailed)
    {
        WScript.Echo("*** test failed ****");
    }
    else
    {
        WScript.Echo("Pass");
    }
}

function dumpObject(obj)
{
    WScript.Echo("Dumping " + obj);
    for (p in obj)
    {
        WScript.Echo(p + ": " + obj.p);
    }
}