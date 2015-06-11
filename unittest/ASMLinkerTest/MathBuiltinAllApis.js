//log and abs
function mathModule(stdlib)
{
	'use asm'	
	var acos = stdlib.Math.acos;
	var asin = stdlib.Math.asin;
	var atan = stdlib.Math.atan;
	var cos = stdlib.Math.cos;
	var sin = stdlib.Math.sin;
	var tan = stdlib.Math.tan;
	var exp = stdlib.Math.exp;
	var log = stdlib.Math.log;
	var cein = stdlib.Math.ceil;
	var floor = stdlib.Math.floor;
	var sqrt = stdlib.Math.sqrt
	var abs = stdlib.Math.abs;
	var min = stdlib.Math.min;
	var max = stdlib.Math.max;
	var atan2 = stdlib.Math.atan2;
	var pow = stdlib.Math.pow;
	var imul = stdlib.Math.imul;
	//var fround = stdlib.Math.fround; // fround is not implemented yet
	var e = stdlib.Math.E;
	var ln10 = stdlib.Math.LN10;
	var ln2 = stdlib.Math.LN2;
	var log2e = stdlib.Math.LOG2E;
	var log10e = stdlib.Math.LOG10E;
	var pi = stdlib.Math.PI;
	var sqrt1_2 = stdlib.Math.SQRT1_2;
	var sqrt2 = stdlib.Math.SQRT2;
	
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




