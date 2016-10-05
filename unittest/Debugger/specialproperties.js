
var testVector = [
	"Array.prototype",
	"Boolean.prototype",
	"Function.prototype",
	"Number.prototype",
	"Error.prototype",
	"String.prototype",
	"RegExp.prototype",
    "Map.prototype",
    "Set.prototype",	
	"Uint8Array.prototype",	
	"Array.prototype.forEach",
    "Math.sin",
	"Intl",
	"JSON",
	"Math",
	"Array",
	"Boolean",
	"Number",
	"ArrayBuffer",
	"Uint8Array",
	"Error",
	"Date",
	"String",
	"Map",
	"Set",
	"new ArrayBuffer()",
	"new Array()",
	"new String()",
	"new RegExp()",
	"new Boolean()",
	"new Number()",
	"new Error()",
	"new Float32Array()",
	"new Map()",
	"new Set()",
	"(new Map()).__proto__",
	"var a = function() {}; a",
	"new Function()",
	// "this", -- global "this" doesn't have special properties to display, remove it to reduce noise
	"arguments",
	"arguments.callee",
	"arguments.callee.caller"
]


testVector.forEach(function(val){
	var f = eval(val);
	/**bp:evaluate('val')**/
	f;
	/**bp:evaluate('f',1)**/
});


WScript.Echo('Pass');

