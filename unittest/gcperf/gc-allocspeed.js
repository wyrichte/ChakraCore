

function test(count)
{
    var d = new Date();
    for (var i = 0; i< count; i++)
    {
        new Object();
    }
    return new Date() - d;
}

for (var i = 100000; i < 1000000000; i *= 10)
{
    var sum = 0;
    for (var j = 0; j < 3; j++)
    {
        sum += test(i);
        CollectGarbage();
    }
    WScript.Echo(i + ": " + ((sum / 3) * 100 | 0) / 100);
}
