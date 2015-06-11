function check(cond, test)
{
	if(!cond)
	{
		WScript.Echo("Failed test: " + test);
	}
}

var f = new Boolean(false);

check( (f==false), "f equals false");
check( (f!==false), "f strict-not-equals false");
check( (!f==false), "!f equals false");  // Conformant to the ES3 and ES5 specifications!


WScript.Echo("done");