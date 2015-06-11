function mathModule(global, foreign, buffer)
{
	'use asm'	
	var abs = global.Math.abs
	var e = foreign.i1|0
	var f = foreign.fun1;
	function bar(x,y){
		x = x|0;
		y = y|0;		
		return (x + y + e + (abs(10)|0)) |0;
	}
	return bar;
}
Math.Infinity = Infinity;
var buffer = new ArrayBuffer(1<<20);
var env = {fun1:function(x){print(x);},i1:155,i2:658,d1:68.25,d2:3.14156,f1:48.1523,f2:14896.2514}
var global = {Math:Math,Int8Array:Int8Array};
var module = mathModule(global, env,buffer);
WScript.Echo("Ams Linker Succeded: " +Debug.getAsmJSModuleLinkVal(mathModule));
env.toString = function(){WScript.Echo("called")};
env.valueOf = function(){WScript.Echo("called")};
global.toString = function(){WScript.Echo("global called")};
global.valueOf = function(){WScript.Echo("global called")};
buffer.toString = function(){WScript.Echo("buffer called")};
buffer.valueOf = function(){WScript.Echo("buffer called")};
var module = mathModule(global, env,buffer);
WScript.Echo("Ams Linker Succeded: " +Debug.getAsmJSModuleLinkVal(mathModule));