
var escape;

function test(param)
{
    function outter()
    {
        function inner()
        {
            escape = nested;
        }
        inner();
    }


    function nested()
    {
        return param;
    }

    outter();
}
    

test("test1");
WScript.Echo(escape());
test("test2");
WScript.Echo(escape());
