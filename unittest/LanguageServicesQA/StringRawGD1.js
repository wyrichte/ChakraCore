function /**target:get10**/get10() {
	return 10;
}

WScript.Echo(String.raw`Got ${10 + get10/**gd:get10**/()}`);