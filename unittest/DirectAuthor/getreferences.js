var obj = 10;
function foo(/**getref:1**/str, a)
{
    var k = str; obj++/**getref:-3**/;
}
obj++;

