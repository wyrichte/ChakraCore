//log and abs
function mathModule(stdlib)
{
	'use asm'	
	var floor = stdlib.Math.floor;		
	function bar(x,y){
		x = +x;
		y = +y;
		return +(+floor(10.9) + +(x+y)) ;
	}
	return bar;
}
var foo = mathModule({Math:Math});
WScript.Echo(Debug.getAsmJSModuleLinkVal(mathModule));
WScript.Echo(foo(12,20));
Math = {floor:Math.ceil}; 
var foo = mathModule({Math:Math});
WScript.Echo(Debug.getAsmJSModuleLinkVal(mathModule));



