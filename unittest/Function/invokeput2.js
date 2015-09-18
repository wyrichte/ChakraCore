var Count = 0;
function test() {
    var x = 100;
    Object.prototype['y'] = x; 
    var e = 8;
    try {
       e.y('u89B9') = 10;
    } catch (e) {
        Count++;
    }        
}
test();
test();
test();

if (Count === 3)
{
    WScript.Echo("Passed");
}
else
{
    WScript.Echo("FAILED\n");
}
