var x = WScript.LoadScriptFile("cross_site_async_break_child.js", "samethread");
var e = x.obj
var stop = false;

/* Stop the loop in 0.5 seconds */
WScript.SetTimeout(function() { stop = true }, 500);

function run()
{ 
	e.foo(3500); 
	if (!stop)
	{
		WScript.SetTimeout(run, 10);
	}
	else
	{
		WScript.Echo("Pass");
	}
}

run();