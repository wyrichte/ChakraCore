// These tests are to make sure we don't restore garbage values when bailing out under debugger.

function foo1(x) 
{
    if (x) 
    {
        let q = 1;
        return 1;
    }
    return Math.sin(0);
}

function foo2(x) 
{
    if (x) 
    {
        const q = 1;
        return 1;
    }
    return Math.sin(0);
}

function foo3(x) 
{
    if (x) 
    {
        function q(){};
        return 1;
    }
    return Math.sin(0);
}

function foo4(x)
{
    if (x)
    {
        function inner1(){}
        function inner2()
        {
            inner1();
        }
        return 1;
    }
    return Math.sin(0);
}

function foo5(x)
{
    if (x)
    {
        let x;
        function inner()
        {
            return x;
        }
        return 1;
    }
    return Math.sin(0);
}

foo1(true);
foo1(false);

foo2(true);
foo2(false);

foo3(true);
foo3(false);

foo4(true);
foo4(false);

foo5(true);
foo5(false);

WScript.Echo('pass');

// -forcediagnosticsmode -MaxInterpretCount:1 -version:5
