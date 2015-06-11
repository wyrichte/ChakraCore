var o = {}
function test()
{
    var i = 0;
    function simple_stackfunc() // this can be stack allocated
    {
        if (i == 0)
        {
            i++;
            return simple_stackfunc();
        }
        return i;
    }
    return simple_stackfunc();
}

WScript.Echo(test());
WScript.Echo(test());

