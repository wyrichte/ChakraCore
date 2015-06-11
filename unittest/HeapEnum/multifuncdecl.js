
var glo;
function test()
{
    if (true)
    {
        function func1()
        {
        }
        function func1()
        {
        }
        function nested_ref()
        {
            func1();
            return a + b;
        }
        glo = nested_ref;
    }
}

test();

Debug.dumpHeap(glo);



