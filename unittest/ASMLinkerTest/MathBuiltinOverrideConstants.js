//log and abs
function mathModule(stdlib)
{
	'use asm'	
	var e = stdlib.Math.E;	
	function bar(x,y){
		x = x|0;
		y = y|0;
		return (x + y)|0;
	}
	return bar;
}
var foo = mathModule({Math:Math});
WScript.Echo(Debug.getAsmJSModuleLinkVal(mathModule));
WScript.Echo(foo(22,20));
Math = {E:2}; 
var foo = mathModule({Math:Math});
WScript.Echo(Debug.getAsmJSModuleLinkVal(mathModule));





