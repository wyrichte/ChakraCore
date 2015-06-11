function foo(callsite, ...substitutions) {
	return "Tagged Template";
}
var str1 = "String Template";
WScript.Echo(foo`Hello ${/**ml:str1**/str1}`);
