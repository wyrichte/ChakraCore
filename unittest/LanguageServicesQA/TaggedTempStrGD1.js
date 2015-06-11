function /**target:foo**/foo(callsite, ...substitutions) {
	return "Tagged Template";
}

function /**target:get10**/get10() {
	return 10;
}

WScript.Echo(foo/**gd:foo**/`Got ${10 + get10/**gd:get10**/()}`);