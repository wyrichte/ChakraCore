var obj = {};
obj.foo = function () {
    var x = "Ì≠ÄÌ∞ÅÌ∞†"
    var y = 1;/**bp:locals(2,0x100);**/
}
WScript.Attach(obj.foo);
/* hello world */
var abc = 1;
WScript.Echo("Pass");