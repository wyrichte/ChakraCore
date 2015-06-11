function Run() {
var testVector = [
	"Array.prototype",
	"Boolean.prototype",
	"Function.prototype",	
	"Error.prototype",
	"String.prototype",
	"RegExp.prototype",
    "Map.prototype",   
	"Uint8Array.prototype",	
	"Array.prototype.forEach",
    "Math.sin",
	"Intl",	
	"Math",
	"Array",	
	"Number",
	"ArrayBuffer",	
	"Error",
	"Date",
	"String",	
	"Set",
	"new ArrayBuffer()",
	"new Array()",
	"new String()",
	"new RegExp()",
	"new Boolean()",
	"new Number()",	
	"new Float32Array()",
	"new Map()",	
	"(new Map()).__proto__",	
	"new Function()",
	// "this", -- global "this" doesn't have special properties to display, remove it to reduce noise
	"arguments",
	"arguments.callee",
	"arguments.callee.caller"
]


testVector.forEach(function(val){
	var f = eval(val);
	/**bp:evaluate('val', LOCALS_FULLNAME)**/
	f;
	/**bp:evaluate('f', 1, LOCALS_FULLNAME)**/
});


WScript.Echo('Pass');

}
WScript.Attach(Run);